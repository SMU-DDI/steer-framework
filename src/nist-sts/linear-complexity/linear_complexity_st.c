// =================================================================================================
//! @file linear_complexity_st.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the single-threaded version of the NIST STS linear complexity test 
//! for the STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-20
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

typedef struct tnist_linearcomplexityprivatedata
{
    tSTEER_ReportPtr            report;
    tSTEER_ConfigurationState   configurationState[CONFIGURATION_COUNT];
    uint64_t                    ones;
    uint64_t                    zeros;
    uint64_t                    bitsDiscarded;
    double                      chiSquared;
    int32_t                     numberOfSubstrings; // TODO: should make this uint32_t
    int32_t                     frequencies[7];     // nu
    double                      probabilityValue;
}
tNIST_LinearComplexityPrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_ParameterInfoList gParameterInfoList = {
    4,
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
            "1000000",  // Verified (NIST SP 800-22 Rev 1a, 2.10.7)
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
        },

        // Test specific parameter
        {
            STEER_JSON_TAG_BLOCK_LENGTH,
            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITS,
            "500",
            "500",      // Verified (NIST SP 800-22 Rev 1a, 2.10.7)
            "5000"      // Verified (NIST SP 800-22 Rev 1a, 2.10.7)
        }
    }
};

static tSTEER_ParametersInfo gParametersInfo = {
    TEST_NAME,
    &gParameterInfoList
};

// =================================================================================================
//  RunTest
// =================================================================================================
int32_t RunTest (tNIST_LinearComplexityPrivateData* privateData,
                 uint8_t* bitstreamBuffer,
                 bool* passed)
{
    int32_t result = STEER_RESULT_SUCCESS;
	int_fast32_t i = 0;
    int_fast32_t ii = 0;
    int_fast32_t j = 0;
    int32_t d = 0;
    int32_t L = 0;
    int32_t m = 0;
    int32_t N_ = 0;
    int32_t sign = 0;
    double linearComplexityStatistic = 0.0;
    double theoreticalMean = 0.0;
	uint8_t* T = NULL;
    uint8_t* P = NULL;
    uint8_t* B_ = NULL; 
    uint8_t* C = NULL;

    // Setup
    privateData->numberOfSubstrings = 0;
    memset((void*)&(privateData->frequencies), 0, 7 * sizeof(uint32_t));
    privateData->chiSquared = 0.0;
    privateData->probabilityValue = 0.0;
    *passed = false;
	
    privateData->numberOfSubstrings = (int32_t)floor(gCommonData.bitstreamLength/gCommonData.blockLength);
    privateData->bitsDiscarded = gCommonData.bitstreamLength % privateData->numberOfSubstrings;

    result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t), (void**)&B_);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t), (void**)&C);

    if (result == STEER_RESULT_SUCCESS)
        result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t), (void**)&P);

    if (result == STEER_RESULT_SUCCESS)
        result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t), (void**)&T);

    if (result == STEER_RESULT_SUCCESS)
    {
        for (i = 0; i < kDegreesOfFreedom + 1; i++)
            privateData->frequencies[i] = 0.00;

        for (ii = 0; ii < privateData->numberOfSubstrings; ii++) 
        {
            for (i = 0; i < gCommonData.blockLength; i++) 
            {
                B_[i] = 0;
                C[i] = 0;
                T[i] = 0;
                P[i] = 0;
            }
            L = 0;
            m = -1;
            d = 0;
            (void)d;    // This line corrects a scan-build false positive
            C[0] = 1;
            B_[0] = 1;
            
            // Determine linear complexity
            N_ = 0;
            while (N_ < gCommonData.blockLength) 
            {
                d = (int32_t)bitstreamBuffer[ii * gCommonData.blockLength + N_];
                for (i = 1; i <= L; i++)
                    d += C[i] * bitstreamBuffer[(ii * gCommonData.blockLength) + N_ - i];
                d = d % 2;

                if (d == 1) 
                {
                    for (i = 0; i < gCommonData.blockLength; i++) 
                    {
                        T[i] = C[i];
                        P[i] = 0;
                    }
                    for (j = 0; j < gCommonData.blockLength; j++)
                        if (B_[j] == 1)
                            P[j + N_ - m] = 1;
                    for (i = 0; i < gCommonData.blockLength; i++)
                        C[i] = (C[i] + P[i]) % 2;
                    if (L <= N_/2) 
                    {
                        L = N_ + 1 - L;
                        m = N_;
                        for (i = 0; i < gCommonData.blockLength; i++)
                            B_[i] = T[i];
                    }
                }

                N_++;
            }

            if (((gCommonData.blockLength + 1) % 2) == 0) 
                sign = -1;
            else 
                sign = 1;

            theoreticalMean = (gCommonData.blockLength/2.0) + 
                    ((9.0 + sign)/36.0) - 
                    ((1.0/pow(2, gCommonData.blockLength)) * ((gCommonData.blockLength/3.0)) + (2.0/9.0));

            if ((gCommonData.blockLength % 2) == 0)
                sign = 1;
            else 
                sign = -1;
            linearComplexityStatistic = sign * (L - theoreticalMean) + 2.0/9.0;
            
            if (linearComplexityStatistic <= -2.5)
                privateData->frequencies[0]++;
            else if (linearComplexityStatistic > -2.5 && linearComplexityStatistic <= -1.5)
                privateData->frequencies[1]++;
            else if (linearComplexityStatistic > -1.5 && linearComplexityStatistic <= -0.5)
                privateData->frequencies[2]++;
            else if (linearComplexityStatistic > -0.5 && linearComplexityStatistic <= 0.5)
                privateData->frequencies[3]++;
            else if (linearComplexityStatistic > 0.5 && linearComplexityStatistic <= 1.5)
                privateData->frequencies[4]++;
            else if (linearComplexityStatistic > 1.5 && linearComplexityStatistic <= 2.5)
                privateData->frequencies[5]++;
            else
                privateData->frequencies[6]++;
        }
        privateData->chiSquared = 0.00;

        for (i = 0; i < kDegreesOfFreedom + 1; i++)
            privateData->chiSquared += (pow(privateData->frequencies[i] - 
                                        (privateData->numberOfSubstrings * kPreComputedProbabilities[i]), 2) / 
                                        (privateData->numberOfSubstrings * kPreComputedProbabilities[i]));
        privateData->probabilityValue = cephes_igamc((double)(kDegreesOfFreedom/2.0), privateData->chiSquared/2.0);

        *passed = (privateData->probabilityValue >= gCommonData.significanceLevel);
    }

    STEER_FreeMemory((void**)&B_);
    STEER_FreeMemory((void**)&C);
    STEER_FreeMemory((void**)&P);
    STEER_FreeMemory((void**)&T);

    return result;
}

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
    tNIST_LinearComplexityPrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;
    memset((void*)&gCommonData, 0, sizeof(tNIST_LinearComplexityCommon));
    gCommonData.cliArguments = cliArguments;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tNIST_LinearComplexityPrivateData),
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

            // Test specific parameter
            else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_BLOCK_LENGTH) == 0)
            {
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    gCommonData.blockLength = *((int32_t*)nativeValue);
                    STEER_FreeMemory((void**)&nativeValue);
                    break;
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
            (gCommonData.significanceLevel >= MAXIMUM_SIGNIFICANCE_LEVEL) ||
            (gCommonData.blockLength < MINIMUM_BLOCK_LENGTH) ||
            (gCommonData.blockLength > MAXIMUM_BLOCK_LENGTH))
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
            // Set the configuration ID
            privData->configurationState[0].configurationId = 0;

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
    ((tNIST_LinearComplexityPrivateData*)testPrivateData)->report = report;
    return STEER_RESULT_SUCCESS;
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
    tNIST_LinearComplexityPrivateData* privData = (tNIST_LinearComplexityPrivateData*)testPrivateData;
    bool passed = false;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    uint64_t testId = 0;
    tSTEER_ValueSet* valueSet = NULL;
    char* end = NULL;

    testId = strtoull(bitstreamId, &end, 0) - 1;

    // Setup
    privData->ones = numOnes;
    privData->zeros = numZeros;
    privData->configurationState[0].accumulatedOnes += numOnes;
    privData->configurationState[0].accumulatedZeros += numZeros;

    // Run the test
    result = RunTest(privData, buffer, &passed);

    // Add calculations to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Add ones
        memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf((void*)calculationStr, "%" PRIu64 "", numOnes);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
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
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
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
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_PROBABILITY_VALUE,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Bits discarded
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, "%" PRIu64 "", privData->bitsDiscarded);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_BITS_DISCARDED,
                                            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                            NULL, NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Chi squared
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT,
                STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->chiSquared);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_CHI_SQUARED,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Linear complexity statistic frequencies
        result = STEER_NewValueSet(STEER_JSON_TAG_LINEAR_COMPLEXITY_STATISTIC_FREQUENCIES,
                                   STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                   NULL, NULL, &valueSet);
        if (result == STEER_RESULT_SUCCESS)
        {
            char tagStr[STEER_STRING_MAX_LENGTH];
            uint_fast32_t i = 0;
            double upper = -1.5;
            double lower = -2.5;

            memset((void*)tagStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(tagStr, "value <= %.*f", 
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, lower);
            memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->frequencies[0]);
            result = STEER_AddValueToValueSet(tagStr, calculationStr, &valueSet);
            if (result == STEER_RESULT_SUCCESS)
            {
                for (i = 1; i < 7; i++)
                {
                    memset((void*)tagStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(tagStr, "%.*f < value <= %.*f", 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, lower, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, upper);
                    memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->frequencies[i]);
                    result = STEER_AddValueToValueSet(tagStr, calculationStr, &valueSet);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        lower += 1.0;
                        upper += 1.0;
                    }
                    else
                        break;
                }
            }
        }

        if (result == STEER_RESULT_SUCCESS)
            result = STEER_AddCalculationSetToTest(privData->report, 0, testId, valueSet);

        (void)STEER_FreeValueSet(&valueSet);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Number of substrings
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->numberOfSubstrings);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_NUMBER_OF_SUBSTRINGS,
                                            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_SUBSTRINGS, calculationStr);
    }

    // Add criteria to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Probability value in range
        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "%s of %.*f > %.*f",
                STEER_JSON_TAG_PROBABILITY_VALUE,
                STEER_DEFAULT_FLOATING_POINT_PRECISION,
                privData->probabilityValue,
                STEER_DEFAULT_FLOATING_POINT_PRECISION, 0.0);
        result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                                          (privData->probabilityValue > 0.0) ? true : false);
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(criterionStr, "%s of %.*f <= %.*f",
                    STEER_JSON_TAG_PROBABILITY_VALUE,
                    STEER_DEFAULT_FLOATING_POINT_PRECISION,
                    privData->probabilityValue,
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, 1.0);
            result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
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
        result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                                          (privData->probabilityValue >= gCommonData.significanceLevel) ? true : false);
    }

    // Add evaluation to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        bool passed = false;
        result = STEER_AddEvaluationToTest(privData->report, 0, testId, &passed);
        if (result == STEER_RESULT_SUCCESS)
        {
            privData->configurationState[0].testsRun++;
            if (passed)
                privData->configurationState[0].testsPassed++;
            else
                privData->configurationState[0].testsFailed++;
        }
    }

    // Clean up
    STEER_FreeMemory((void**)&buffer);

    return result;
}

// =================================================================================================
//  FinalizeTest
// =================================================================================================
int32_t FinalizeTest (void** privateData,
                      uint64_t suppliedNumberOfBitstreams)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_LinearComplexityPrivateData* privData = (tNIST_LinearComplexityPrivateData*)(*privateData);

    if (privData != NULL)
    {
        double probabilityValueUniformity = 0.0;
        uint64_t proportionThresholdMinimum = 0;
        uint64_t proportionThresholdMaximum = 0;
        
        // Add required metrics to configuration
        result = STEER_NistStsAddRequiredMetricsToConfiguration(privData->report, 0,
                                                                suppliedNumberOfBitstreams,
                                                                gCommonData.minimumTestCountRequiredForSignificance,
                                                                privData->configurationState[0].testsPassed,
                                                                gCommonData.predictedPassedTestCount,
                                                                privData->configurationState[0].accumulatedOnes,
                                                                privData->configurationState[0].accumulatedZeros);

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add test specific metrics
            result = STEER_NistStsAddMetricsToConfiguration(privData->report, 0, false,
                                                            suppliedNumberOfBitstreams,
                                                            gCommonData.significanceLevel,
                                                            &probabilityValueUniformity,
                                                            &proportionThresholdMinimum,
                                                            &proportionThresholdMaximum);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add optional confusion matrix metrics to configuration
            result = STEER_AddConfusionMatrixMetricsToConfiguration(privData->report, 0,
                                                                    gCommonData.minimumTestCountRequiredForSignificance,
                                                                    privData->configurationState[0].testsRun,
                                                                    privData->configurationState[0].testsPassed,
                                                                    privData->configurationState[0].testsFailed,
                                                                    gCommonData.predictedPassedTestCount,
                                                                    gCommonData.predictedFailedTestCount);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add required criteria to configuration
            result = STEER_NistStsAddRequiredCriterionToConfiguration(privData->report, 0,
                                                                      suppliedNumberOfBitstreams,
                                                                      privData->configurationState[0].testsPassed,
                                                                      gCommonData.significanceLevel,
                                                                      gCommonData.significanceLevelPrecision,
                                                                      gCommonData.minimumTestCountRequiredForSignificance);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add test specific criteria to configuration
            result = STEER_NistStsAddCriteriaToConfiguration(privData->report, 0,
                                                             probabilityValueUniformity,
                                                             proportionThresholdMinimum,
                                                             proportionThresholdMaximum,
                                                             privData->configurationState[0].testsRun, 
                                                             privData->configurationState[0].testsPassed);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add evaluation to configuration
            result = STEER_AddEvaluationToConfiguration(privData->report, 0);
        }

        // Clean up
        STEER_FreeMemory((void**)privateData);
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
