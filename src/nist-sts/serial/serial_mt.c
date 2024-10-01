// =================================================================================================
//! @file serial_mt.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the multi-threaded version of the NIST STS serial test for the 
//! STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-07-18
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
#include <pthread.h>

// =================================================================================================
//  Private constants
// =================================================================================================

#define SLEEP_INTERVAL_IN_MICROSECONDS  1000

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_serialtestdata
{
    uint8_t*    bitstreamBuffer;
    int32_t     result;                     // TODO: Need to check this!
    uint64_t    numOnes;
    uint64_t    numZeros;
    double      generalizedSerialStatistic1;    // del1
    double      generalizedSerialStatistic2;    // del2
    double      serialStatisticM;               // psi (m)
    double      serialStatisticMminus1;         // psi (m - 1)
    double      serialStatisticMminus2;         // psi (m - 2)
    double      probabilityValue[CONFIGURATION_COUNT];
    bool        passed[CONFIGURATION_COUNT];
}
tNIST_SerialTestData;

typedef struct tnist_serialthread
{
    pthread_t               threadId;
    uint64_t                config1Id;
    uint64_t                config1TestId;
    uint64_t                config2Id;
    uint64_t                config2TestId;
    tNIST_SerialTestData    testData;
}
tNIST_SerialThread;

typedef struct tnist_serialprivatedata
{
    tSTEER_ReportPtr            report;
    tSTEER_ConfigurationState   configurationState[CONFIGURATION_COUNT];
    uint32_t                    threadCount;
    tNIST_SerialThread          thread[];
}
tNIST_SerialPrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_ParameterInfoList gParameterInfoList = {
    5,
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
        },

        // Test specific parameter
        {
            STEER_JSON_TAG_THREAD_COUNT,
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_THREADS,
            "32",
            "1",
            "128"
        }
    }
};

static tSTEER_ParametersInfo gParametersInfo = {
    TEST_NAME,
    &gParameterInfoList
};

volatile uint32_t gRunningThreadCount = 0;
pthread_mutex_t gRunningThreadCountMutex = PTHREAD_MUTEX_INITIALIZER;

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
//  UpdateReport
// =================================================================================================
int32_t UpdateReport (tSTEER_ReportPtr report,
                      uint64_t configId,
                      uint64_t testId,
                      uint32_t probabilityValueIndex,
                      tNIST_SerialTestData* testData)
{
    int32_t result = STEER_RESULT_SUCCESS;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char tagStr[STEER_STRING_MAX_LENGTH] = { 0 };
    tSTEER_ValueSet* valueSet = NULL;

    // Add calculations to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Add ones
        memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf((void*)calculationStr, "%" PRIu64 "", testData->numOnes);
        result = STEER_AddCalculationToTest(report, configId, testId,
                                            STEER_JSON_TAG_ONES,
                                            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BITS,
                                            calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Add zeros
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, "%" PRIu64 "", testData->numZeros);
        result = STEER_AddCalculationToTest(report, configId, testId,
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
                STEER_DEFAULT_FLOATING_POINT_PRECISION, 
                testData->probabilityValue[probabilityValueIndex]);
        result = STEER_AddCalculationToTest(report, configId, testId,
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
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->generalizedSerialStatistic1);
            result = STEER_AddValueToValueSet(tagStr, calculationStr, &valueSet);
        } 

        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)tagStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(tagStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 2);
            memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->generalizedSerialStatistic2);
            result = STEER_AddValueToValueSet(tagStr, calculationStr, &valueSet);
        }

        if (result == STEER_RESULT_SUCCESS)
            result = STEER_AddCalculationSetToTest(report, configId, testId, valueSet);
        
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
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->serialStatisticM);
            result = STEER_AddValueToValueSet(STEER_JSON_TAG_M, calculationStr, &valueSet);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->serialStatisticMminus1);
            result = STEER_AddValueToValueSet(STEER_JSON_TAG_M_MINUS_ONE, calculationStr, &valueSet);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->serialStatisticMminus2);
            result = STEER_AddValueToValueSet(STEER_JSON_TAG_M_MINUS_TWO, calculationStr, &valueSet);
        }

        if (result == STEER_RESULT_SUCCESS)
            result = STEER_AddCalculationSetToTest(report, configId, testId, valueSet);
        
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
                testData->probabilityValue[probabilityValueIndex],
                STEER_DEFAULT_FLOATING_POINT_PRECISION, 0.0);
        result = STEER_AddCriterionToTest(report, configId, testId, criterionStr,
                                          (testData->probabilityValue[probabilityValueIndex] > 0.0) ? true : false);
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(criterionStr, "%s of %.*f <= %.*f",
                    STEER_JSON_TAG_PROBABILITY_VALUE,
                    STEER_DEFAULT_FLOATING_POINT_PRECISION,
                    testData->probabilityValue[probabilityValueIndex],
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, 1.0);
            result = STEER_AddCriterionToTest(report, configId, testId, criterionStr,
                                              (testData->probabilityValue[probabilityValueIndex] <= 1.0) ? true : false);
        }
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Probability value
        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "%s of %.*f >= %s of %.*f",
                STEER_JSON_TAG_PROBABILITY_VALUE,
                STEER_DEFAULT_FLOATING_POINT_PRECISION,
                testData->probabilityValue[probabilityValueIndex],
                STEER_JSON_TAG_SIGNIFICANCE_LEVEL,
                gCommonData.significanceLevelPrecision,
                gCommonData.significanceLevel);
        result = STEER_AddCriterionToTest(report, configId, testId, criterionStr,
                                          (testData->probabilityValue[probabilityValueIndex] >= gCommonData.significanceLevel) ? true : false);
    }

    // Add evaluation to current test
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_AddEvaluationToTest(report, configId, testId, 
                                           &(testData->passed[probabilityValueIndex]));

    return result;
}

// =================================================================================================
//  GetNextAvailableThread
// =================================================================================================
int32_t GetNextAvailableThread (tNIST_SerialPrivateData* privateData)
{
    int32_t index = -1;
    uint_fast32_t i = 0;

    for (i = 0; i < privateData->threadCount; i++)
    {
        if (privateData->thread[i].testData.bitstreamBuffer == NULL)
        {
            index = i;
            break;
        }
    }
    return index;
}

// =================================================================================================
//  ThreadsReady
// =================================================================================================
bool ThreadsReady (tNIST_SerialPrivateData* privateData)
{
    bool ready = true;
    uint_fast32_t i = 0;

    for (i = 0; i < privateData->threadCount; i++)
    {
        if (privateData->thread[i].testData.bitstreamBuffer == NULL)
        {
            ready = false;
            break;
        }
    }
    return ready;
}

// =================================================================================================
//  RunTest
// =================================================================================================
void* RunTest (void* ptr)
{
    tNIST_SerialTestData* testData = (tNIST_SerialTestData*)ptr;
	
    // Setup
    testData->result = STEER_RESULT_SUCCESS;

    testData->serialStatisticM = 
        psi2(gCommonData.blockLength, gCommonData.bitstreamLength, testData->bitstreamBuffer);
    testData->serialStatisticMminus1 = 
        psi2(gCommonData.blockLength-1, gCommonData.bitstreamLength, testData->bitstreamBuffer);
    testData->serialStatisticMminus2 = 
        psi2(gCommonData.blockLength-2, gCommonData.bitstreamLength, testData->bitstreamBuffer);
    testData->generalizedSerialStatistic1 = 
        testData->serialStatisticM - testData->serialStatisticMminus1;
    testData->generalizedSerialStatistic2 = 
        testData->serialStatisticM - 
        (2.0 * testData->serialStatisticMminus1) + 
        testData->serialStatisticMminus2;

    testData->probabilityValue[0] = 
        cephes_igamc(pow(2.0, (double)(gCommonData.blockLength - 1))/2, testData->generalizedSerialStatistic1/2.0);
    testData->probabilityValue[1] = 
        cephes_igamc(pow(2.0, (double)(gCommonData.blockLength - 2))/2, testData->generalizedSerialStatistic2/2.0);

    testData->passed[0] = (testData->probabilityValue[0] >= gCommonData.significanceLevel);
    testData->passed[1] = (testData->probabilityValue[1] >= gCommonData.significanceLevel);

    // Decrement running thread count
    (void)STEER_CHECK_ERROR(pthread_mutex_lock(&gRunningThreadCountMutex));
    gRunningThreadCount--;
    (void)STEER_CHECK_ERROR(pthread_mutex_unlock(&gRunningThreadCountMutex));

    return NULL;
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
    uint32_t threadCount = 0;
    uint_fast32_t i = 0;
    void* nativeValue = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;
    memset((void*)&gCommonData, 0, sizeof(tNIST_SerialCommon));
    gCommonData.cliArguments = cliArguments;

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
                {
                    result = STEER_ConvertStringToUnsigned32BitInteger(parameters->parameter[i].precision,
                                                                       &(gCommonData.significanceLevelPrecision));
                }
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

        // Test specific parameter
        else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_THREAD_COUNT) == 0)
        {
            result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                          parameters->parameter[i].value,
                                          &nativeValue);
            if (result == STEER_RESULT_SUCCESS)
            {
                threadCount = *((uint32_t*)nativeValue);
                STEER_FreeMemory((void**)&nativeValue);
            }
        }

        if (result != STEER_RESULT_SUCCESS)
            break;
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Allocate private data
        result = STEER_AllocateMemory(sizeof(tNIST_SerialPrivateData) +
                                      (threadCount * sizeof(tNIST_SerialThread)),
                                      (void**)&privData);

        if (result == STEER_RESULT_SUCCESS)
        {
            // More setup
            privData->threadCount = threadCount;

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

                // Return private data and buffer size
                *testPrivateData = (void*)privData;
                *bufferSizeInBytes = gCommonData.bitstreamLength / 8;
            }
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
    uint64_t config1Id = 0;
    uint32_t config2Id = 1;
    uint64_t config1TestId = 0;
    uint64_t config2TestId = 0;
    int32_t threadIndex = -1;
    uint_fast32_t i = 0;
    char* end = NULL;

    config1TestId = strtoull(bitstreamId, &end, 0) - 1;
    config2TestId = config1TestId;

    // Setup
    privData->configurationState[0].accumulatedOnes += numOnes;
    privData->configurationState[0].accumulatedZeros += numZeros;
    privData->configurationState[1].accumulatedOnes += numOnes;
    privData->configurationState[1].accumulatedZeros += numZeros;

    // Get next available thread
    threadIndex = GetNextAvailableThread(privData);
    if (threadIndex > -1)
    {
        // Queue it up
        memset((void*)&(privData->thread[threadIndex].testData), 0, sizeof(tNIST_SerialTestData));
        privData->thread[threadIndex].config1Id = config1Id;
        privData->thread[threadIndex].config1TestId = config1TestId;
        privData->thread[threadIndex].config2Id = config2Id;
        privData->thread[threadIndex].config2TestId = config2TestId;
        privData->thread[threadIndex].testData.bitstreamBuffer = buffer;
        privData->thread[threadIndex].testData.numOnes = numOnes;
        privData->thread[threadIndex].testData.numZeros = numZeros;
        result = STEER_CHECK_ERROR(pthread_create(&(privData->thread[threadIndex].threadId), 
                                                    NULL, &RunTest,
                                                    (void*)&(privData->thread[threadIndex].testData)));
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Are all threads ready for execution?
        if (ThreadsReady(privData))
        {
            // Yes, start them up
            for (i = 0; i < privData->threadCount; i++)
            {
                (void)STEER_CHECK_ERROR(pthread_mutex_lock(&gRunningThreadCountMutex));
                gRunningThreadCount++;
                (void)STEER_CHECK_ERROR(pthread_mutex_unlock(&gRunningThreadCountMutex));

                result = STEER_CHECK_ERROR(pthread_join(privData->thread[i].threadId, NULL));
                if (result != STEER_RESULT_SUCCESS)
                    break;  
            }

            if (result != STEER_RESULT_SUCCESS)
            {
                // Decrement thread count
                (void)STEER_CHECK_ERROR(pthread_mutex_lock(&gRunningThreadCountMutex));
                gRunningThreadCount--;
                (void)STEER_CHECK_ERROR(pthread_mutex_unlock(&gRunningThreadCountMutex));
            }

            // Wait for threads to complete
            while (gRunningThreadCount > 0)
            {
                usleep(SLEEP_INTERVAL_IN_MICROSECONDS);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                uint_fast32_t j = 0;

                // Update report and clean up
                for (i = 0; i < privData->threadCount; i++)
                {
                    result = UpdateReport(privData->report, 
                                          privData->thread[i].config1Id,
                                          privData->thread[i].config1TestId, 0,
                                          &(privData->thread[i].testData));
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        result = UpdateReport(privData->report,
                                              privData->thread[i].config2Id,
                                              privData->thread[i].config2TestId, 1,
                                              &(privData->thread[i].testData));
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        for (j = 0; j < CONFIGURATION_COUNT; j++)
                        {
                            privData->configurationState[j].testsRun++;
                            if (privData->thread[i].testData.passed[j])
                                privData->configurationState[j].testsPassed++;
                            else
                                privData->configurationState[j].testsFailed++;
                        }
                    }
                }
            }

            // Clean up
            for (i = 0; i < privData->threadCount; i++)
            {
                STEER_FreeMemory((void**)&(privData->thread[i].testData.bitstreamBuffer));
            }
        }
    }
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
        double probabilityValueUniformity = 0.0;
        uint64_t proportionThresholdMinimum = 0;
        uint64_t proportionThresholdMaximum = 0;
        uint_fast32_t i = 0;

        // Are there any pending threads that haven't been processed?
        for (i = 0; i < privData->threadCount; i++)
        {
            if (privData->thread[i].testData.bitstreamBuffer != NULL)
            {
                (void)STEER_CHECK_ERROR(pthread_mutex_lock(&gRunningThreadCountMutex));
                gRunningThreadCount++;
                (void)STEER_CHECK_ERROR(pthread_mutex_unlock(&gRunningThreadCountMutex));

                result = STEER_CHECK_ERROR(pthread_join(privData->thread[i].threadId, NULL));
                if (result != STEER_RESULT_SUCCESS)
                    break;  
            }
        }

        if (result != STEER_RESULT_SUCCESS)
        {
            // Decrement thread count
            (void)STEER_CHECK_ERROR(pthread_mutex_lock(&gRunningThreadCountMutex));
            gRunningThreadCount--;
            (void)STEER_CHECK_ERROR(pthread_mutex_unlock(&gRunningThreadCountMutex));
        }

        // Wait for any running threads to finish
        while (gRunningThreadCount > 0)
        {
            usleep(SLEEP_INTERVAL_IN_MICROSECONDS);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            uint_fast32_t j = 0;

            // Update report and clean up
            for (i = 0; i < privData->threadCount; i++)
            {
                if (privData->thread[i].testData.bitstreamBuffer != NULL)
                {
                    result = UpdateReport(privData->report, 
                                          privData->thread[i].config1Id,
                                          privData->thread[i].config1TestId, 0,
                                          &(privData->thread[i].testData));
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        result = UpdateReport(privData->report,
                                              privData->thread[i].config2Id,
                                              privData->thread[i].config2TestId, 1,
                                              &(privData->thread[i].testData));
                    }

                    for (j = 0; j < CONFIGURATION_COUNT; j++)
                    {
                        privData->configurationState[j].testsRun++;
                        if (privData->thread[i].testData.passed[j])
                            privData->configurationState[j].testsPassed++;
                        else
                            privData->configurationState[j].testsFailed++;
                    }
                }
            }
        }

        // Clean up
        for (i = 0; i < privData->threadCount; i++)
        {
            STEER_FreeMemory((void**)&(privData->thread[i].testData.bitstreamBuffer));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            for (i = 0; i < CONFIGURATION_COUNT; i++)
            {
                // Add required metrics to configuration
                result = STEER_NistStsAddRequiredMetricsToConfiguration(privData->report, 
                                                                        privData->configurationState[i].configurationId,
                                                                        suppliedNumberOfBitstreams,
                                                                        gCommonData.minimumTestCountRequiredForSignificance,
                                                                        privData->configurationState[i].testsPassed,
                                                                        gCommonData.predictedPassedTestCount,
                                                                        privData->configurationState[i].accumulatedOnes,
                                                                        privData->configurationState[i].accumulatedZeros);

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add test specific metrics
                    result = STEER_NistStsAddMetricsToConfiguration(privData->report, 
                                                                    privData->configurationState[i].configurationId, false,
                                                                    suppliedNumberOfBitstreams,
                                                                    gCommonData.significanceLevel,
                                                                    &probabilityValueUniformity,
                                                                    &proportionThresholdMinimum,
                                                                    &proportionThresholdMaximum);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add optional confusion matrix metrics to configuration
                    result = STEER_AddConfusionMatrixMetricsToConfiguration(privData->report, 
                                                                            privData->configurationState[i].configurationId,
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
                    result = STEER_NistStsAddRequiredCriterionToConfiguration(privData->report, 
                                                                              privData->configurationState[i].configurationId,
                                                                              suppliedNumberOfBitstreams,
                                                                              privData->configurationState[i].testsPassed,
                                                                              gCommonData.significanceLevel,
                                                                              gCommonData.significanceLevelPrecision,
                                                                              gCommonData.minimumTestCountRequiredForSignificance);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add test specific criteria to configuration
                    result = STEER_NistStsAddCriteriaToConfiguration(privData->report, 
                                                                     privData->configurationState[i].configurationId,
                                                                     probabilityValueUniformity,
                                                                     proportionThresholdMinimum,
                                                                     proportionThresholdMaximum,
                                                                     privData->configurationState[i].testsRun, 
                                                                     privData->configurationState[i].testsPassed);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add evaluation to configuration
                    result = STEER_AddEvaluationToConfiguration(privData->report, 
                                                                privData->configurationState[i].configurationId);
                }
            }
        }
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
