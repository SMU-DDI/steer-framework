// =================================================================================================
//! @file serial_st.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the single-threaded version of the NIST STS serial test for the 
//! STEER framework.
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

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_serialprivatedata
{
    tSTEER_ReportPtr            report;
    tSTEER_ConfigurationState   configurationState[CONFIGURATION_COUNT];
    uint64_t                    suppliedNumberOfBitstreams;
    uint64_t                    ones;
    uint64_t                    zeros;
    double                      generalizedSerialStatistic1;    // del1
    double                      generalizedSerialStatistic2;    // del2
    double                      serialStatisticM;       // psi (m)
    double                      serialStatisticMminus1; // psi (m - 1)
    double                      serialStatisticMminus2; // psi (m - 2)
    double                      probabilityValue[CONFIGURATION_COUNT];
    bool                        passed[CONFIGURATION_COUNT];
}
tNIST_SerialPrivateData;

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
            NULL,      
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
            "16",
            NULL,
            NULL
        }
    }
};

static tSTEER_ParametersInfo gParametersInfo = {
    TEST_NAME,
    &gParameterInfoList
};

// =================================================================================================
//  psi2
// =================================================================================================
double psi2 (int32_t m, 
             int32_t n,
             uint8_t* bitstreamBuffer)
{
	int_fast32_t i = 0;
    int_fast32_t j = 0;
    int32_t k = 0;
    int32_t powLen = 0;
	double sum = 0.0;
    double numOfBlocks = 0.0;
	uint32_t* P = NULL;
    int32_t result = STEER_RESULT_SUCCESS;
	
	if ((m == 0) || (m == -1))
        sum = 0.0;
    else
    {
        numOfBlocks = n;
        powLen = (int32_t)pow(2, m + 1) - 1;

        result = STEER_AllocateMemory(powLen * sizeof(uint32_t),
                                      (void**)&P);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Initialize nodes
            for (i = 1; i < powLen - 1; i++)
                P[i] = 0;	  
                
            // Compute frequency
            for (i = 0; i < numOfBlocks; i++) 		
            {
                k = 1;
                for (j = 0; j < m; j++) 
                {
                    if (bitstreamBuffer[(i + j) % n] == 0)
                        k *= 2;
                    else if (bitstreamBuffer[(i + j) % n] == 1)
                        k = 2 * k + 1;
                }
                P[k - 1]++;
            }
            sum = 0.0;
            for (i = (int32_t)pow(2, m) - 1; i < (int32_t)pow(2, m + 1) - 1; i++)
                sum += pow(P[i], 2);

            sum = (sum * pow(2, m)/(double)n) - (double)n;

            STEER_FreeMemory((void**)&P);
        }
    }
	return sum;
}

// =================================================================================================
//  RunTest
// =================================================================================================
int32_t RunTest (tNIST_SerialPrivateData* privateData,
                 uint8_t* bitstreamBuffer)
{
    int32_t result = STEER_RESULT_SUCCESS;
	
    privateData->serialStatisticM = 
        psi2(gCommonData.blockLength, gCommonData.bitstreamLength, bitstreamBuffer);
    privateData->serialStatisticMminus1 = 
        psi2(gCommonData.blockLength-1, gCommonData.bitstreamLength, bitstreamBuffer);
    privateData->serialStatisticMminus2 = 
        psi2(gCommonData.blockLength-2, gCommonData.bitstreamLength, bitstreamBuffer);
    privateData->generalizedSerialStatistic1 = 
        privateData->serialStatisticM - privateData->serialStatisticMminus1;
    privateData->generalizedSerialStatistic2 = 
        privateData->serialStatisticM - (2.0 * privateData->serialStatisticMminus1) + privateData->serialStatisticMminus2;

    privateData->probabilityValue[0] = 
        cephes_igamc(pow(2.0, (double)(gCommonData.blockLength - 1))/2, privateData->generalizedSerialStatistic1/2.0);
    privateData->probabilityValue[1] = 
        cephes_igamc(pow(2.0, (double)(gCommonData.blockLength - 2))/2, privateData->generalizedSerialStatistic2/2.0);

    privateData->passed[0] = (privateData->probabilityValue[0] >= gCommonData.significanceLevel);
    privateData->passed[1] = (privateData->probabilityValue[1] >= gCommonData.significanceLevel);

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
    tNIST_SerialPrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;
    memset((void*)&gCommonData, 0, sizeof(tNIST_SerialCommon));
    gCommonData.cliArguments = cliArguments;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tNIST_SerialPrivateData),
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
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                break;
        }

        // Make sure we got the required parameters and that they're good
        if ((gCommonData.bitstreamCount < MINIMUM_BITSTREAM_COUNT) || 
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
            uint_fast32_t i = 0;

            for (i = 0; i < CONFIGURATION_COUNT; i++)
            {
                privData->configurationState[i].configurationId = i;
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
    ((tNIST_SerialPrivateData*)testPrivateData)->report = report;
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
    tNIST_SerialPrivateData* privData = (tNIST_SerialPrivateData*)testPrivateData;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char tagStr[STEER_STRING_MAX_LENGTH] = { 0 };
    uint_fast32_t i = 0;
    tSTEER_ValueSet* valueSet = NULL;
    uint64_t testId = 0;
    char* end = NULL;

    testId = strtoull(bitstreamId, &end, 0) - 1;

    // Run the test
    result = RunTest(privData, buffer);
    if (result == STEER_RESULT_SUCCESS)
    {
        for (i = 0; i < CONFIGURATION_COUNT; i++)
        {
            // Setup
            privData->ones = numOnes;
            privData->zeros = numZeros;
            privData->configurationState[i].accumulatedOnes += numOnes;
            privData->configurationState[i].accumulatedZeros += numZeros;

            // Add ones
            memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf((void*)calculationStr, "%" PRIu64 "", numOnes);
            result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                STEER_JSON_TAG_ONES,
                                                STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                NULL, STEER_JSON_VALUE_BITS,
                                                calculationStr);

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
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->probabilityValue[i]);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_PROBABILITY_VALUE,
                                                    STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                                    STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                                    NULL, calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Generalized serial statistics
                result = STEER_NewValueSet(STEER_JSON_TAG_GENERALIZED_SERIAL_STATISTICS,
                                           STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                           STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, &valueSet);
                if (result == STEER_RESULT_SUCCESS)
                {
                    memset((void*)tagStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(tagStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 1);
                    memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->generalizedSerialStatistic1);
                    result = STEER_AddValueToValueSet(tagStr, calculationStr, &valueSet);
                } 

                if (result == STEER_RESULT_SUCCESS)
                {
                    memset((void*)tagStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(tagStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 2);
                    memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->generalizedSerialStatistic2);
                    result = STEER_AddValueToValueSet(tagStr, calculationStr, &valueSet);
                }

                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_AddCalculationSetToTest(privData->report, i, testId, valueSet);
                
                (void)STEER_FreeValueSet(&valueSet);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Serial statistics
                result = STEER_NewValueSet(STEER_JSON_TAG_SERIAL_STATISTICS,
                                           STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                           STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, &valueSet);
                if (result == STEER_RESULT_SUCCESS)
                {
                    memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->serialStatisticM);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_M, calculationStr, &valueSet);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->serialStatisticMminus1);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_M_MINUS_ONE, calculationStr, &valueSet);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->serialStatisticMminus2);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_M_MINUS_TWO, calculationStr, &valueSet);
                }

                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_AddCalculationSetToTest(privData->report, i, testId, valueSet);
                
                (void)STEER_FreeValueSet(&valueSet);
            }
            
            // Add criteria to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                // Probability value in range
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %.*f > %.*f",
                        STEER_JSON_TAG_PROBABILITY_VALUE,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION,
                        privData->probabilityValue[i],
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, 0.0);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                  (privData->probabilityValue[i] > 0.0) ? true : false);
                if (result == STEER_RESULT_SUCCESS)
                {
                    memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(criterionStr, "%s of %.*f <= %.*f",
                            STEER_JSON_TAG_PROBABILITY_VALUE,
                            STEER_DEFAULT_FLOATING_POINT_PRECISION,
                            privData->probabilityValue[i],
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, 1.0);
                    result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                      (privData->probabilityValue[i] <= 1.0) ? true : false);
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Probability value
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %.*f >= %s of %.*f",
                        STEER_JSON_TAG_PROBABILITY_VALUE,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION,
                        privData->probabilityValue[i],
                        STEER_JSON_TAG_SIGNIFICANCE_LEVEL,
                        gCommonData.significanceLevelPrecision,
                        gCommonData.significanceLevel);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                  (privData->probabilityValue[i] >= gCommonData.significanceLevel) ? true : false);
            }

            // Add evaluation to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_AddEvaluationToTest(privData->report, i, testId, &(privData->passed[i]));
                if (result == STEER_RESULT_SUCCESS)
                {
                    privData->configurationState[i].testsRun++;
                    if (privData->passed[i])
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
    tNIST_SerialPrivateData* privData = (tNIST_SerialPrivateData*)(*testPrivateData);

    if (privData != NULL)
    {
        char configIdStr[STEER_STRING_MAX_LENGTH] = { 0 };
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
