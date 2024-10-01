// =================================================================================================
//! @file random_excursions.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the NIST STS random excursions test for the STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-19
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_json_utilities.h"
#include "steer_nist_sts_utilities_private.h"
#include "steer_parameters_info_utilities.h"
#include "steer_report_utilities.h"
#include "steer_report_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_string_utilities_private.h"
#include "steer_test_info_utilities.h"
#include "steer_utilities.h"
#include "steer_utilities_private.h"
#include "steer_value_utilities.h"
#include "cephes.h"
#include "defs.h"
#include <math.h>
#include <errno.h>

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_randomexcursionsprivatedata
{
    tSTEER_ReportPtr            report;
    tSTEER_ConfigurationState   configurationState[CONFIGURATION_COUNT];
    uint64_t                    ones;
    uint64_t                    zeros;
    double                      chiSquared;
    double                      probabilityValue;
    int32_t                     x;
}
tNIST_RandomExcursionsPrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_ParameterInfoList gParameterInfoList = {
    3,
    {
        // Required parameter
        {
            STEER_JSON_TAG_BITSTREAM_COUNT,
            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITSTREAMS,
            "1",
            "1",
            NULL
        },

        // Required parameter
        {
            STEER_JSON_TAG_BITSTREAM_LENGTH,
            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITS,
            "1000000",
            "1000000",      // Verified (NIST SP 800-22 Rev 1a, 2.14.7)
            NULL
        },

        // Required parameter
        {
            STEER_JSON_TAG_SIGNIFICANCE_LEVEL,
            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION,
            NULL,
            STEER_JSON_VALUE_DEFAULT_SIGNIFICANCE_LEVEL,    // Verified (NIST SP 800-22 Rev 1a, 1.1.5)
            STEER_JSON_VALUE_MINIMUM_SIGNIFICANCE_LEVEL,    // Verified (NIST SP 800-22 Rev 1a, section 4.3(f))
            STEER_JSON_VALUE_MAXIMUM_SIGNIFICANCE_LEVEL     // Verified (NIST SP 800-22 Rev 1a, 1.1.5)
        }
    }
};

static tSTEER_ParametersInfo gParametersInfo = {
    TEST_NAME,
    &gParameterInfoList
};

// =================================================================================================
//  GetTestInfo
// =================================================================================================
char* GetTestInfo (void) 
{ 
    // Convert test info to JSON and return it
    char* json = NULL;
    int32_t result = STEER_TestInfoToJson(&gTestInfo, &json);
    if (result == STEER_RESULT_SUCCESS)
        return json;
    else
        return NULL;
}

// =================================================================================================
//  GetParametersInfo
// =================================================================================================
char* GetParametersInfo (void)
{
    // Convert parameters info to JSON and return it
    char* json = NULL;
    int32_t result = STEER_ParametersInfoToJson(&gParametersInfo, &json);
    if (result == STEER_RESULT_SUCCESS)
        return json;
    else
        return NULL;
}

// =================================================================================================
//  InitTest
// =================================================================================================
int32_t InitTest (tSTEER_CliArguments* cliArguments,
                  tSTEER_ParameterSet* parameters,
                  void** testPrivateData,
                  uint64_t* bufferSizeInBytes)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_RandomExcursionsPrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;
    gCommonData.cliArguments = cliArguments;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tNIST_RandomExcursionsPrivateData),
                                  (void**)&privData);
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        void* nativeValue = NULL;

        // Get parameters from parameter set
        for (i = 0; i < parameters->count; i++)
        {
            // Required parameter
            if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_BITSTREAM_COUNT) == 0)
            {
                // Convert value from text to native type
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    gCommonData.bitstreamCount = *((uint64_t*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                }
            }

            // Required parameter
            else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_BITSTREAM_LENGTH) == 0)
            {
                // Convert value from text to native type
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    gCommonData.bitstreamLength = *((uint64_t*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                }
            }

            // Required parameter
            else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_SIGNIFICANCE_LEVEL) == 0)
            {
                // Convert value from text to native type
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    gCommonData.significanceLevel = *((double*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                }

                // Get the precision
                if (result == STEER_RESULT_SUCCESS)
                {
                    if (parameters->parameter[i].precision != NULL)
                        result = STEER_ConvertStringToUnsigned32BitInteger(parameters->parameter[i].precision,
                                                                           &(gCommonData.significanceLevelPrecision));
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                break;
        }

        // Make sure we got the required parameters and that they're good
        if ((gCommonData.bitstreamCount < MINIMUM_BITSTREAM_COUNT) || 
            (gCommonData.bitstreamLength < MINIMUM_BITSTREAM_LENGTH) ||
            ((gCommonData.bitstreamLength % 8) != 0) ||
            (gCommonData.significanceLevel <= MINIMUM_SIGNIFICANCE_LEVEL) ||
            (gCommonData.significanceLevel >= MAXIMUM_SIGNIFICANCE_LEVEL))
        {
            result = STEER_CHECK_ERROR(EINVAL);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Calculate the number of tests required 
            // to derive statistically meaningful results
            result = STEER_GetMinimumTestCount(gCommonData.significanceLevel,
                                               gCommonData.bitstreamCount,
                                               &(gCommonData.minimumTestCountRequiredForSignificance),
                                               &(gCommonData.predictedPassedTestCount),
                                               &(gCommonData.predictedFailedTestCount));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Setup
            gCommonData.maxNumberOfCycles = MAX(1000, gCommonData.bitstreamLength/100);
            gCommonData.rejectionConstraint = MAX(0.005 * pow(gCommonData.bitstreamLength, 0.5), 500);

            result = STEER_AllocateMemory(gCommonData.bitstreamLength * sizeof(int32_t),
                                          (void**)&(gCommonData.S_k));
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_AllocateMemory((MAX(1000, gCommonData.bitstreamLength/100) * sizeof(int32_t)),
                                              (void**)&(gCommonData.cycle));
            }
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Return private data and buffer size
            *testPrivateData = (void*)privData;
            *bufferSizeInBytes = gCommonData.bitstreamLength / 8;
        }
    }

    // Check status
    if (result != STEER_RESULT_SUCCESS)
    {
        // Clean up
        STEER_FreeMemory((void**)&privData);
    }
    return result;
}

// =================================================================================================
//  GetConfigurationCount
// =================================================================================================
uint32_t GetConfigurationCount (void* testPrivateData)
{
    return CONFIGURATION_COUNT;
}

// =================================================================================================
//  SetReport
// =================================================================================================
int32_t SetReport (void* testPrivateData,
                   tSTEER_ReportPtr report)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_RandomExcursionsPrivateData* privData = NULL;
    char attributeStr[STEER_STRING_MAX_LENGTH] = { 0 };
    uint_fast32_t i = 0;

    privData = (tNIST_RandomExcursionsPrivateData*)testPrivateData;
    privData->report = report;

    // Walk configurations
    for (i = 0; i < CONFIGURATION_COUNT; i++)
    {
        privData->configurationState[i].configurationId = i;

        // Add random excursion state attribute to configuration
        if (!STEER_ConfigurationHasAttribute(privData->report, i,
                                             STEER_JSON_TAG_RANDOM_EXCURSION_STATE))
        {
            memset((void*)attributeStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(attributeStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, gCommonData.stateX[i]);
            result = STEER_AddAttributeToConfiguration(privData->report, i,
                                                       STEER_JSON_TAG_RANDOM_EXCURSION_STATE,
                                                       STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                                       NULL, NULL, attributeStr);
        }

        if (result != STEER_RESULT_SUCCESS)
            break;
    }
    return result;
}

// =================================================================================================
//  ExecuteTest
// =================================================================================================
int32_t ExecuteTest (void* testPrivateData,
                     const char* bitstreamId,
                     uint8_t* buffer,
                     uint64_t bufferSizeInBytes,
                     uint64_t bytesInBuffer,
                     uint64_t numZeros,
                     uint64_t numOnes)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_RandomExcursionsPrivateData* privData = (tNIST_RandomExcursionsPrivateData*)testPrivateData;
    bool passed = false;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char progressStr[STEER_STRING_MAX_LENGTH] = { 0 };
    int32_t b = 0;
    int_fast32_t i = 0;
    int_fast32_t j = 0;
    int_fast32_t k = 0;
    uint64_t testId = 0;
    char* end = NULL;
    int32_t cycleStart = 0;
    int32_t cycleStop = 0;
    int32_t counter[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    testId = strtoull(bitstreamId, &end, 0) - 1;

    // Setup
    privData->ones = numOnes;
    privData->zeros = numZeros;

    // Determine cycles
    gCommonData.numberOfCycles = 0;
    gCommonData.S_k[0] = (2 * (int)buffer[0]) - 1;
    for (i = 1; i < gCommonData.bitstreamLength; i++) 
    {
        gCommonData.S_k[i] = gCommonData.S_k[i - 1] + (2 * buffer[i]) - 1;
        if (gCommonData.S_k[i] == 0) 
        {
            gCommonData.numberOfCycles++;
            result = STEER_CHECK_CONDITION((gCommonData.numberOfCycles <= gCommonData.maxNumberOfCycles), 
                                           NIST_RESULT_NUM_CYCLES_GT_MAX);
            if (result == STEER_RESULT_SUCCESS)
                gCommonData.cycle[gCommonData.numberOfCycles] = i;
            else
                break;
        }
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        if (gCommonData.S_k[gCommonData.bitstreamLength - 1] != 0)
            gCommonData.numberOfCycles++;
        gCommonData.cycle[gCommonData.numberOfCycles] = gCommonData.bitstreamLength;

        cycleStart = 0;
        cycleStop  = gCommonData.cycle[1];
        for (k = 0; k < 6; k++)
            for (i = 0; i < 8; i++)
                gCommonData.nu[k][i] = 0.;

        // For each cycle
        for (j = 1; j <= gCommonData.numberOfCycles; j++) 
        {                          
            for (i = 0; i < 8; i++)
                counter[i] = 0;
            for (i = cycleStart; i < cycleStop; i++) 
            {
                if ((gCommonData.S_k[i] >= 1 && gCommonData.S_k[i] <= 4) || 
                    (gCommonData.S_k[i] >= -4 && gCommonData.S_k[i] <= -1)) 
                {
                    if (gCommonData.S_k[i] < 0)
                        b = 4;
                    else
                        b = 3;
                    counter[gCommonData.S_k[i] + b]++;
                }
            }
            cycleStart = gCommonData.cycle[j] + 1;
            if (j < gCommonData.numberOfCycles)
                cycleStop = gCommonData.cycle[j + 1];
            
            for (i = 0; i < 8; i++) 
            {
                if ((counter[i] >= 0) && (counter[i] <= 4))
                    gCommonData.nu[counter[i]][i]++;
                else if (counter[i] >= 5)
                    gCommonData.nu[5][i]++;
            }
        }
        
        for (i = 0; i < CONFIGURATION_COUNT; i++) 
        {
            privData->configurationState[i].accumulatedOnes += numOnes;
            privData->configurationState[i].accumulatedZeros += numZeros;

            if (gCommonData.cliArguments->reportProgress)
            {
                memset((void*)progressStr, 0, STEER_STRING_MAX_LENGTH);
                if (i < 4)
                    sprintf(progressStr, "Testing excursion state %d...", 
                            i - 4);
                else
                    sprintf(progressStr, "Testing excursion state %d...", 
                            i - 3);
                STEER_REPORT_PROGRESS(gCommonData.cliArguments->programName, progressStr);
            }

            privData->x = gCommonData.stateX[i];
            privData->chiSquared = 0.0;
            for (k = 0; k < 6; k++)
            {
                privData->chiSquared += 
                    pow(gCommonData.nu[k][i] - 
                    gCommonData.numberOfCycles * gCommonData.pi[(int32_t)abs(privData->x)][k], 2) / 
                    (gCommonData.numberOfCycles * gCommonData.pi[(int32_t)abs(privData->x)][k]);
            }
            privData->probabilityValue = cephes_igamc(2.5, privData->chiSquared/2.0);

            passed = (privData->probabilityValue >= gCommonData.significanceLevel);

            // Add calculations to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add ones
                memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf((void*)calculationStr, "%" PRIu64 "", numOnes);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_ONES,
                                                    STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                    NULL, STEER_JSON_VALUE_BITS,
                                                    calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add zeros
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, "%" PRIu64 "", numZeros);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_ZEROS,
                                                    STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                    NULL, STEER_JSON_VALUE_BITS,
                                                    calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add probability value
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->probabilityValue);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_PROBABILITY_VALUE,
                                                    STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                                    STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                                    NULL, calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Chi squared
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->chiSquared);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_CHI_SQUARED,
                                                    STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                                    STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                                    NULL, calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Number of cycles
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 
                        gCommonData.numberOfCycles);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_NUMBER_OF_CYCLES,
                                                    STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                                    NULL, STEER_JSON_VALUE_CYCLES, calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Rejection constraint
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, gCommonData.rejectionConstraint);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_REJECTION_CONSTRAINT,
                                                    STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                                    STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                                    NULL, calculationStr);
            }

            // Add criteria to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                // Number of cycles
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %d <= %s %s of %d",
                        STEER_JSON_TAG_NUMBER_OF_CYCLES,
                        gCommonData.numberOfCycles, "maximum",
                        STEER_JSON_TAG_NUMBER_OF_CYCLES,
                        gCommonData.maxNumberOfCycles);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                  (gCommonData.numberOfCycles <= gCommonData.maxNumberOfCycles) ? true : false);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Probability value in range
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %.*f > %.*f",
                        STEER_JSON_TAG_PROBABILITY_VALUE,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION,
                        privData->probabilityValue,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, 0.0);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                  (privData->probabilityValue > 0.0) ? true : false);
                if (result == STEER_RESULT_SUCCESS)
                {
                    memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(criterionStr, "%s of %.*f <= %.*f",
                            STEER_JSON_TAG_PROBABILITY_VALUE,
                            STEER_DEFAULT_FLOATING_POINT_PRECISION,
                            privData->probabilityValue,
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, 1.0);
                    result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                      (privData->probabilityValue <= 1.0) ? true : false);
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Probability value
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %.*f >= %s of %.*f",
                        STEER_JSON_TAG_PROBABILITY_VALUE,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION,
                        privData->probabilityValue,
                        STEER_JSON_TAG_SIGNIFICANCE_LEVEL,
                        gCommonData.significanceLevelPrecision,
                        gCommonData.significanceLevel);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                  (privData->probabilityValue >= gCommonData.significanceLevel) ? true : false);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Rejection constraint
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %d >= %s of %.*f",
                        STEER_JSON_TAG_NUMBER_OF_CYCLES,
                        gCommonData.numberOfCycles,
                        STEER_JSON_TAG_REJECTION_CONSTRAINT,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION,
                        gCommonData.rejectionConstraint);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                  (gCommonData.numberOfCycles >= gCommonData.rejectionConstraint) ? true : false);
            }

            // Add evaluation to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_AddEvaluationToTest(privData->report, i, testId, &passed);
                if (result == STEER_RESULT_SUCCESS)
                {
                    privData->configurationState[i].testsRun++;
                    if (passed)
                        privData->configurationState[i].testsPassed++;
                    else
                        privData->configurationState[i].testsFailed++;
                }
            }
        }
    }

    // Clean up
    STEER_FreeMemory((void**)&buffer);

    return result;
}

// =================================================================================================
//  FinalizeTest
// =================================================================================================
int32_t FinalizeTest (void** testPrivateData,
                      uint64_t suppliedNumberOfBitstreams)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_RandomExcursionsPrivateData* privData =  
        (tNIST_RandomExcursionsPrivateData*)(*testPrivateData);

    if (privData != NULL)
    {
        double probabilityValueUniformity = 0.0;
        uint64_t proportionThresholdMinimum = 0;
        uint64_t proportionThresholdMaximum = 0;
        uint_fast32_t i = 0;

        for (i = 0; i < CONFIGURATION_COUNT; i++)
        {            
            // Add required metrics to configuration
            result = STEER_NistStsAddRequiredMetricsToConfiguration(privData->report, i,
                                                                    suppliedNumberOfBitstreams,
                                                                    gCommonData.minimumTestCountRequiredForSignificance,
                                                                    privData->configurationState[i].testsPassed,
                                                                    gCommonData.predictedPassedTestCount,
                                                                    privData->configurationState[i].accumulatedOnes,
                                                                    privData->configurationState[i].accumulatedZeros);

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add test specific metrics
                result = STEER_NistStsAddMetricsToConfiguration(privData->report, i, false,
                                                                suppliedNumberOfBitstreams,
                                                                gCommonData.significanceLevel,
                                                                &probabilityValueUniformity,
                                                                &proportionThresholdMinimum,
                                                                &proportionThresholdMaximum);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add optional confusion matrix metrics to configuration
                result = STEER_AddConfusionMatrixMetricsToConfiguration(privData->report, i,
                                                                        gCommonData.minimumTestCountRequiredForSignificance,
                                                                        privData->configurationState[i].testsRun,
                                                                        privData->configurationState[i].testsPassed,
                                                                        privData->configurationState[i].testsFailed,
                                                                        gCommonData.predictedPassedTestCount,
                                                                        gCommonData.predictedFailedTestCount);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add required criteria to configuration
                result = STEER_NistStsAddRequiredCriterionToConfiguration(privData->report, i,
                                                                          suppliedNumberOfBitstreams,
                                                                          privData->configurationState[i].testsPassed,
                                                                          gCommonData.significanceLevel,
                                                                          gCommonData.significanceLevelPrecision,
                                                                          gCommonData.minimumTestCountRequiredForSignificance);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add test specific criteria to configuration
                result = STEER_NistStsAddCriteriaToConfiguration(privData->report, i,
                                                                 probabilityValueUniformity,
                                                                 proportionThresholdMinimum,
                                                                 proportionThresholdMaximum,
                                                                 privData->configurationState[i].testsRun, 
                                                                 privData->configurationState[i].testsPassed);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add evaluation to configuration
                result = STEER_AddEvaluationToConfiguration(privData->report, i);
            }

            // Clean up
            STEER_FreeMemory((void**)&(gCommonData.S_k));
            STEER_FreeMemory((void**)&(gCommonData.cycle));
        }

        // Clean up
        STEER_FreeMemory((void**)testPrivateData);
    }
    return result;
}

// =================================================================================================
//  main
// =================================================================================================
int main (int argc, const char * argv[])
{
    // Run STEER program
    return STEER_Run (PROGRAM_NAME, argc, argv,
                      GetTestInfo, GetParametersInfo,
                      InitTest, GetConfigurationCount,
                      SetReport, ExecuteTest, FinalizeTest);
}

// =================================================================================================
