// =================================================================================================
//! @file steer_nist_sts_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the report utilities for the STandard Entropy Evaluation Report
//! (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-23
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_nist_sts_utilities_private.h"
#include "steer_report_utilities.h"
#include "steer_report_utilities_private.h"
#include "steer_utilities.h"
#include "steer_value_utilities.h"
#include "cephes.h"
#include <math.h>

// =================================================================================================
//  Private constants
// =================================================================================================

#define NUM_P_VALUE_FREQ_BINS                                           10
#define MINIMUM_NUMBER_OF_TESTS_FOR_PROBABILITY_UNIFORMITY_ASSESSMENT   55

// =================================================================================================
//  STEER_NistStsAddRequiredMetricsToConfiguration
// =================================================================================================
int32_t STEER_NistStsAddRequiredMetricsToConfiguration (tSTEER_ReportPtr report,
                                                        uint64_t configurationId,
                                                        uint64_t testCount,
                                                        uint64_t minimumTestCountRequiredForSignificance,
                                                        uint64_t passedTestCount,
                                                        uint64_t minimumTestCountRequiredToPass,
                                                        uint64_t totalOnes,
                                                        uint64_t totalZeros)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((minimumTestCountRequiredForSignificance > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((minimumTestCountRequiredToPass > 0), EINVAL);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        char metricStr[STEER_STRING_MAX_LENGTH] = { 0 };

        // Add bitstreams tested metric
        memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(metricStr, "%" PRIu64 "", testCount);
        result = STEER_AddMetricToConfiguration(report, configurationId,
                                                STEER_JSON_TAG_BITSTREAMS_TESTED,
                                                STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                NULL,
                                                STEER_JSON_VALUE_BITSTREAMS,
                                                metricStr);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add bitstream test required for significance metric
            memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(metricStr, "%" PRIu64 "", minimumTestCountRequiredForSignificance);
            result = STEER_AddMetricToConfiguration(report, configurationId,
                                                    STEER_JSON_TAG_MINIMUM_BITSTREAM_TESTS_REQUIRED_FOR_SIGNIFICANCE,
                                                    STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                    NULL,
                                                    STEER_JSON_VALUE_TESTS,
                                                    metricStr);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add bitstream tests passed metric
            memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(metricStr, "%" PRIu64 "", passedTestCount);
            result = STEER_AddMetricToConfiguration(report, configurationId,
                                                    STEER_JSON_TAG_BITSTREAM_TESTS_PASSED,
                                                    STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                    NULL,
                                                    STEER_JSON_VALUE_TESTS,
                                                    metricStr);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add minimum bitstream tests required to pass metric
            memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(metricStr, "%" PRIu64 "", minimumTestCountRequiredToPass);
            result = STEER_AddMetricToConfiguration(report, configurationId,
                                                    STEER_JSON_TAG_MINIMUM_BITSTREAM_TESTS_REQUIRED_TO_PASS,
                                                    STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                    NULL,
                                                    STEER_JSON_VALUE_TESTS,
                                                    metricStr);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add total ones metric
            memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(metricStr, "%" PRIu64 "", totalOnes);
            result = STEER_AddMetricToConfiguration(report, configurationId,
                                                    STEER_JSON_TAG_TOTAL_ONES,
                                                    STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                    NULL,
                                                    STEER_JSON_VALUE_BITS,
                                                    metricStr);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add total zeros metric
            memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(metricStr, "%" PRIu64 "", totalZeros);
            result = STEER_AddMetricToConfiguration(report, configurationId,
                                                    STEER_JSON_TAG_TOTAL_ZEROS,
                                                    STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                    NULL,
                                                    STEER_JSON_VALUE_BITS,
                                                    metricStr);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_NistStsAddMetricsToConfiguration
// =================================================================================================
int32_t STEER_NistStsAddMetricsToConfiguration (tSTEER_ReportPtr report,
                                                uint64_t configurationId,
                                                bool isRandomExcursionOrRandomExcursionVariantTest,
                                                uint64_t bitstreamCount,
                                                double significanceLevel,
                                                double* probabilityValueUniformity,
                                                uint64_t* proportionThresholdMinimum,
                                                uint64_t* proportionThresholdMaximum)
{
    int32_t result = STEER_RESULT_SUCCESS;
    uint64_t probabilityValueCount = 0;
    double* probabilityValues = NULL;
    uint64_t histogram[NUM_P_VALUE_FREQ_BINS];
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_CONDITION((bitstreamCount > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId <= reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(probabilityValueUniformity);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(proportionThresholdMinimum);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(proportionThresholdMaximum);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *probabilityValueUniformity = 0.0;
        *proportionThresholdMinimum = 0;
        *proportionThresholdMaximum = 0;

        // Get the probability values for this configuration
        result = STEER_NistStsGetProbabilityValuesFromConfiguration(report, configurationId,
                                                                    &probabilityValueCount,
                                                                    &probabilityValues);
    }
    if (result == STEER_RESULT_SUCCESS)
    {
        uint32_t pos = 0;
        uint64_t numTests = bitstreamCount;    // sampleSize
        uint64_t count = 0;
        int64_t expCount = 0;
        uint64_t proportion_threshold_min = 0;
        uint64_t proportion_threshold_max = 0;
        double chi2 = 0.0;     
        double p_hat = 0.0;
        double uniformity = 0.0;    
        double* T = NULL;
        double* A = NULL;
        uint_fast64_t j = 0;

        // Setup
        memset((void*)histogram, 0, sizeof(uint64_t) * NUM_P_VALUE_FREQ_BINS);

        // Compute proportion of passing sequences
        if (isRandomExcursionOrRandomExcursionVariantTest == true)
        {
            result = STEER_AllocateMemory(sizeof(double) * bitstreamCount, (void**)&T);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (numTests <= probabilityValueCount)
                {
                    for (j = 0; j < numTests; j++)
                    {
                        if (probabilityValues[j] > 0.000000)
                            T[count++] = probabilityValues[j];
                    }

                    if (count > 0)
                    {
                        result = STEER_AllocateMemory(sizeof(double) * count, (void**)&A);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            for (j = 0; j < count; j++)
                                A[j] = T[j];

                            numTests = count;
                            count = 0;
                            for (j = 0; j < numTests; j++)
                                if (A[j] < significanceLevel)
                                    count++;
                        }
                    }

                    // Clean up
                    STEER_FreeMemory((void**)&T);
                }
            }
        }
        else
        {
            result = STEER_AllocateMemory(sizeof(double) * bitstreamCount, (void**)&A);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (numTests <= probabilityValueCount)
                {
                    for (j = 0; j < numTests; j++)
                    {
                        if (probabilityValues[j] < significanceLevel)
                            count++;
                        A[j] = probabilityValues[j];
                    }
                }
            }
        }

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            if (A != NULL)
            {
                p_hat = 1.0 - significanceLevel;
                proportion_threshold_max = 
                    (p_hat + 3.0 * sqrt((p_hat * significanceLevel) / numTests)) * numTests;
                proportion_threshold_min = 
                    (p_hat - 3.0 * sqrt((p_hat * significanceLevel) / numTests)) * numTests;

                // Compute histogram
                qsort((void*)A, numTests, sizeof(double), (void*)STEER_CompareDoubles);
                for (j = 0; j < numTests; j++)
                {
                    pos = (uint32_t)floor(A[j] * 10);
                    if (pos == 10)
                        pos--;
                    histogram[pos]++;
                }

                chi2 = 0.0;
                expCount = numTests/10;
                if (expCount == 0)
                    uniformity = 0.0;
                else
                {
                    for (j = 0; j < 10; j++)
                        chi2 += pow((double)histogram[j] - (double)expCount, 2.0)/expCount;
                    uniformity = cephes_igamc(9.0/2.0, chi2/2.0);
                }
            }
            else
            {
                // Calculate confidence interval (SP 800-22 Rev. 1a Section 4.2.1)
                p_hat = 1.0 - significanceLevel;
                proportion_threshold_max = 
                    (p_hat + 3.0 * sqrt((p_hat * significanceLevel) / bitstreamCount)) * bitstreamCount;
                proportion_threshold_min = 
                    (p_hat - 3.0 * sqrt((p_hat * significanceLevel) / bitstreamCount)) * bitstreamCount;
            }
        }   

        // Clean up
        STEER_FreeMemory((void**)&A);

        if (result == STEER_RESULT_SUCCESS)
        {
            uint_fast32_t i = 0;
            char labelStr[STEER_STRING_MAX_LENGTH] = { 0 };
            char valueStr[STEER_STRING_MAX_LENGTH] = { 0 };

            // Add probability value frequencies metric to configuration
            tSTEER_ValueSet* valueSet = NULL;
            result = STEER_NewValueSet(STEER_JSON_TAG_PROBABILITY_VALUE_FREQUENCIES,
                                       STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                       NULL, STEER_JSON_VALUE_PROBABILITY_VALUES, &valueSet);
            if (result == STEER_RESULT_SUCCESS)
            {
                for (i = 0; i < NUM_P_VALUE_FREQ_BINS; i++)
                {
                    memset((void*)labelStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(labelStr, "between %.1f and %.1f", 
                            i * 0.1, (i + 1) * 0.1);
                    memset((void*)valueStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(valueStr, "%" PRIu64 "", histogram[i]);
                    result = STEER_AddValueToValueSet(labelStr, valueStr, &valueSet);
                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    result = STEER_AddMetricSetToConfiguration(report, configurationId,
                                                               valueSet);
                }

                (void)STEER_FreeValueSet(&valueSet);
            }

            // Add probability value uniformity metric to configuration
            if (result == STEER_RESULT_SUCCESS)
            {
                memset((void*)valueStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(valueStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, uniformity);
                result = STEER_AddMetricToConfiguration(report, configurationId,
                                                        STEER_JSON_TAG_PROBABILITY_VALUE_UNIFORMITY,
                                                        STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                                        STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                                        NULL, (const char*)valueStr);
            }

            // Add confidence interval metric to configuration
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_NewValueSet(STEER_JSON_TAG_CONFIDENCE_INTERVAL,
                                           STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                           NULL, STEER_JSON_VALUE_TESTS, &valueSet);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add lower bound
                    memset((void*)valueStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(valueStr, "%" PRIu64 "", proportion_threshold_min);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_LOWER_BOUND,
                                                      valueStr, &valueSet);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Add upper bound
                        memset((void*)valueStr, 0, STEER_STRING_MAX_LENGTH);
                        sprintf(valueStr, "%" PRIu64 "", proportion_threshold_max);
                        result = STEER_AddValueToValueSet(STEER_JSON_TAG_UPPER_BOUND,
                                                          valueStr, &valueSet);
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    result = STEER_AddMetricSetToConfiguration(report, configurationId,
                                                               valueSet);
                }

                (void)STEER_FreeValueSet(&valueSet);
            }
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            *probabilityValueUniformity = uniformity;
            *proportionThresholdMinimum = proportion_threshold_min;
            *proportionThresholdMaximum = proportion_threshold_max;
        }

        STEER_FreeMemory((void**)&probabilityValues);
    }
    return result;
}

// =================================================================================================
//  STEER_NistStsAddRequiredCriterionToConfiguration
// =================================================================================================
int32_t STEER_NistStsAddRequiredCriterionToConfiguration (tSTEER_ReportPtr report,
                                                          uint64_t configurationId,
                                                          uint64_t testCount,
                                                          uint64_t passTestCount,
                                                          double significanceLevel,
                                                          uint32_t significanceLevelPrecision,
                                                          uint64_t minimumTestCountRequiredForSignificance)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
    {
        char critieronStr[STEER_STRING_MAX_LENGTH] = { 0 };

        memset((void*)critieronStr, 0, STEER_STRING_MAX_LENGTH);
        if (testCount == 1)
            sprintf(critieronStr,
                    "%" PRIu64 " bitstream tested >= minimum number of %" PRIu64 " bitstreams to test, as determined by %s of %.*f",
                    testCount, 
                    minimumTestCountRequiredForSignificance, 
                    STEER_JSON_TAG_SIGNIFICANCE_LEVEL, 
                    significanceLevelPrecision,
                    significanceLevel);
        else
            sprintf(critieronStr,
                    "%" PRIu64 " bitstreams tested >= minimum number of %" PRIu64 " bitstreams to test, as determined by %s of %.*f",
                    testCount, 
                    minimumTestCountRequiredForSignificance, 
                    STEER_JSON_TAG_SIGNIFICANCE_LEVEL, 
                    significanceLevelPrecision,
                    significanceLevel);

        result = STEER_AddCriterionToConfiguration(report, configurationId, critieronStr,
                                                   (testCount >= minimumTestCountRequiredForSignificance) ? true : false);
    }
    return result;
}

// =================================================================================================
//  STEER_NistStsAddCriteriaToConfiguration
// =================================================================================================
int32_t STEER_NistStsAddCriteriaToConfiguration (tSTEER_ReportPtr report,
                                                 uint64_t configurationId,
                                                 double probabilityValueUniformity,
                                                 uint64_t proportionThresholdMinimum,
                                                 uint64_t proportionThresholdMaximum,
                                                 uint64_t testCount,
                                                 uint64_t passTestCount)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId <= reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        char criterionStr[STEER_STRING_MAX_LENGTH];

        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        if (testCount == 1)
        {
            sprintf(criterionStr, 
                    "%" PRIu64 " bitstream tested >= minimum number of %u bitstreams to test, as required for probability uniformity assessment",
                    testCount, MINIMUM_NUMBER_OF_TESTS_FOR_PROBABILITY_UNIFORMITY_ASSESSMENT);
        }
        else
        {
            sprintf(criterionStr, 
                    "%" PRIu64 " bitstreams tested >= minimum number of %u bitstreams to test, as required for probability uniformity assessment",
                    testCount, MINIMUM_NUMBER_OF_TESTS_FOR_PROBABILITY_UNIFORMITY_ASSESSMENT);
        }
        result = STEER_AddCriterionToConfiguration(report, configurationId, criterionStr, 
                                                   (testCount >= MINIMUM_NUMBER_OF_TESTS_FOR_PROBABILITY_UNIFORMITY_ASSESSMENT) ? true : false);
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(criterionStr, "probability uniformity of %.*f > 0.0000",
                    STEER_DEFAULT_FLOATING_POINT_PRECISION,
                    probabilityValueUniformity);
            result = STEER_AddCriterionToConfiguration(report, configurationId, criterionStr, 
                                                       (probabilityValueUniformity > 0) ? true : false);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(criterionStr, "probability uniformity of %.*f > 0.0001",
                    STEER_DEFAULT_FLOATING_POINT_PRECISION,
                    probabilityValueUniformity);
            result = STEER_AddCriterionToConfiguration(report, configurationId, criterionStr, 
                                                       (probabilityValueUniformity > 0.0001) ? true : false);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            if (testCount == 1)
                sprintf(criterionStr,
                        "%" PRIu64 " bitstream tested > 0 bitstreams tested",
                        testCount);
            else
                sprintf(criterionStr,
                        "%" PRIu64 " bitstreams tested > 0 bitstreams tested",
                        testCount);
            result = STEER_AddCriterionToConfiguration(report, configurationId, criterionStr,
                                                       (testCount > 0) ? true : false);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            if (passTestCount == 1)
            {
                if (proportionThresholdMaximum == 1)
                    sprintf(criterionStr,
                            "%" PRIu64 " bitstream test passed <= maximum of %" PRIu64 " bitstream test to pass, as determined by confidence interval",
                            passTestCount, proportionThresholdMaximum);
                else
                    sprintf(criterionStr,
                            "%" PRIu64 " bitstream test passed <= maximum of %" PRIu64 " bitstream tests to pass, as determined by confidence interval",
                            passTestCount, proportionThresholdMaximum);
            }
            else
            {
                if (proportionThresholdMaximum == 1)
                    sprintf(criterionStr,
                            "%" PRIu64 " bitstream tests passed <= maximum of %" PRIu64 " bitstream test to pass, as determined by confidence interval",
                            passTestCount, proportionThresholdMaximum);
                else
                    sprintf(criterionStr,
                            "%" PRIu64 " bitstream tests passed <= maximum of %" PRIu64 " bitstream tests to pass, as determined by confidence interval",
                            passTestCount, proportionThresholdMaximum);
            }
            result = STEER_AddCriterionToConfiguration(report, configurationId, criterionStr,
                                                       (passTestCount <= proportionThresholdMaximum) ? true : false);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            if (passTestCount == 1)
            {
                if (proportionThresholdMinimum == 1)
                    sprintf(criterionStr,
                            "%" PRIu64 " bitstream test passed >= minimum of %" PRIu64 " bitstream test to pass, as determined by confidence interval",
                            passTestCount, proportionThresholdMinimum);
                else
                    sprintf(criterionStr,
                            "%" PRIu64 " bitstream test passed >= minimum of %" PRIu64 " bitstream tests to pass, as determined by confidence interval",
                            passTestCount, proportionThresholdMinimum);
            }
            else
            {
                if (proportionThresholdMinimum == 1)
                    sprintf(criterionStr,
                            "%" PRIu64 " bitstream tests passed >= minimum of %" PRIu64 " bitstream test to pass, as determined by confidence interval",
                            passTestCount, proportionThresholdMinimum);
                else
                    sprintf(criterionStr,
                            "%" PRIu64 " bitstream tests passed >= minimum of %" PRIu64 " bitstream tests to pass, as determined by confidence interval",
                            passTestCount, proportionThresholdMinimum);
            }
            result = STEER_AddCriterionToConfiguration(report, configurationId, criterionStr,
                                                       (passTestCount >= proportionThresholdMinimum) ? true : false);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_NistStsGetProbabilityValueFromTest
// =================================================================================================
int32_t STEER_NistStsGetProbabilityValueFromTest (tSTEER_ReportPtr report,
                                                  uint64_t configurationId,
                                                  uint64_t testId,
                                                  double* probabilityValue)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(reportPrivate);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < (reportPrivate->configurations->count)), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((testId < (reportPrivate->configurations->configuration[configurationId].tests->count)), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(probabilityValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);
        tSTEER_Tests* testsPtr = configPtr->tests;
        tSTEER_Test* testPtr = &(testsPtr->test[testId]);
        tSTEER_Value* calculationPtr = NULL;
        uint_fast32_t i = 0;
        void* nativeValue = NULL;

        // Setup
        *probabilityValue = 0.0;

        for (i = 0; i < testPtr->calculations->count; i++)
        {
            calculationPtr = &(testPtr->calculations->value[i]);
            if (strcmp(calculationPtr->name, STEER_JSON_TAG_PROBABILITY_VALUE) == 0)
            {
                result = STEER_GetNativeValue(calculationPtr->dataType,
                                              calculationPtr->value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    *probabilityValue = *((double*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                    break;
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                break;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_NistStsGetProbabilityValuesFromConfiguration
// =================================================================================================
int32_t STEER_NistStsGetProbabilityValuesFromConfiguration(tSTEER_ReportPtr report,
                                                           uint64_t configurationId,
                                                           uint64_t* probabilityValueCount,
                                                           double** probabilityValues)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(probabilityValueCount);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(probabilityValues);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);
        uint64_t numPValues = 0;
        double pValue = 0.0;
        uint_fast32_t i = 0;

        // Setup
        *probabilityValueCount = 0;
        *probabilityValues = NULL;

        // Walk the tests
        for (i = 0; i < configPtr->tests->count; i++)
        {
            result = STEER_NistStsGetProbabilityValueFromTest(report, configurationId, i, &pValue);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (*probabilityValues == NULL)
                    result = STEER_AllocateMemory(sizeof(double),
                                                (void**)probabilityValues);
                else
                    result = STEER_ReallocateMemory((sizeof(double) * numPValues),
                                                    (sizeof(double) * (numPValues + 1)),
                                                    (void**)probabilityValues);
                if (result == STEER_RESULT_SUCCESS)
                {
                    (*probabilityValues)[numPValues] = pValue;
                    numPValues++;
                }
                else
                    break;
            }
            else
                break;
        }

        if (result == STEER_RESULT_SUCCESS)
            *probabilityValueCount = numPValues;
        else
            STEER_FreeMemory((void**)probabilityValues);
    }
    return result;
}

// =================================================================================================
