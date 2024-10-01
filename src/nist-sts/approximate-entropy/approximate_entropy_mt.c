// =================================================================================================
//! @file approximate_entropy_mt.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the multi-threaded version of the NIST STS approximate entropy test 
//! for the STEER framework.
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
#include <stdio.h>
#include <pthread.h>

// =================================================================================================
//  Private constants
// =================================================================================================

#define SLEEP_INTERVAL_IN_MICROSECONDS  1000

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_approximateentropytestdata
{
    uint8_t*    bitstreamBuffer;
    int32_t     result;                     // TODO: Need to check this!
    uint64_t    numOnes;
    uint64_t    numZeros;
    double      approximateEntropy;
    double      chiSquared;
    double      entropyDistributionM;       // phi (m)
    double      entropyDistributionMplus1;  // phi (m + 1)
    double      probabilityValue;           
    int32_t     recommendedBlockSize;       // TODO: Should make this uint32_t
    bool        passed;
}
tNIST_ApproximateEntropyTestData;

typedef struct tnist_approximateentropythread
{
    pthread_t                           threadId;
    uint64_t                            testId;
    tNIST_ApproximateEntropyTestData    testData;
}
tNIST_ApproximateEntropyThread;

typedef struct tnist_approximateentropyprivatedata
{
    tSTEER_ReportPtr                report;
    tSTEER_ConfigurationState       configurationState;
    uint32_t                        threadCount;
    tNIST_ApproximateEntropyThread  thread[];
}
tNIST_ApproximateEntropyPrivateData;

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
            "1024",
            NULL
        },

        // Test specific parameter
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
            "10",
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
//  UpdateReport
// =================================================================================================
int32_t UpdateReport (tSTEER_ReportPtr report,
                      uint64_t testId,
                      tNIST_ApproximateEntropyTestData* testData)
{
    int32_t result = STEER_RESULT_SUCCESS;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    tSTEER_ValueSet* valueSet = NULL;

    // Add calculations to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Add ones
        memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf((void*)calculationStr, "%" PRIu64 "", testData->numOnes);
        result = STEER_AddCalculationToTest(report, 0, testId,
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
        result = STEER_AddCalculationToTest(report, 0, testId,
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
                STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->probabilityValue);
        result = STEER_AddCalculationToTest(report, 0, testId,
                                            STEER_JSON_TAG_PROBABILITY_VALUE,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Add approximate entropy
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->approximateEntropy);
        result = STEER_AddCalculationToTest(report, 0, testId,
                                            STEER_JSON_TAG_APPROXIMATE_ENTROPY,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Add chi squared
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->chiSquared);
        result = STEER_AddCalculationToTest(report, 0, testId,
                                            STEER_JSON_TAG_CHI_SQUARED,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Add entropy distribution
        result = STEER_NewValueSet(STEER_JSON_TAG_ENTROPY_DISTRIBUTION,
                                   STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                   STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                   NULL, &valueSet);

        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->entropyDistributionM);
            result = STEER_AddValueToValueSet(STEER_JSON_TAG_M, calculationStr, &valueSet);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->entropyDistributionMplus1);
            result = STEER_AddValueToValueSet(STEER_JSON_TAG_M_PLUS_ONE, calculationStr, &valueSet);
        }

        if (result == STEER_RESULT_SUCCESS)
            result = STEER_AddCalculationSetToTest(report, 0, testId, valueSet);
        
        (void)STEER_FreeValueSet(&valueSet);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Add log2.0
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, gCommonData.log2pt0);
        result = STEER_AddCalculationToTest(report, 0, testId,
                                            STEER_JSON_TAG_LOG_E_2,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Add recommended block length
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 
                testData->recommendedBlockSize);
        result = STEER_AddCalculationToTest(report, 0, testId,
                                            STEER_JSON_TAG_RECOMMENDED_BLOCK_LENGTH,
                                            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BITS, calculationStr);
    }

    // Add criteria to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Recommended block size
        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "%s of %d %s <= recommended %s of %d %s",
                STEER_JSON_TAG_BLOCK_LENGTH,
                gCommonData.blockLength,
                STEER_JSON_VALUE_BITS,
                STEER_JSON_TAG_BLOCK_LENGTH,
                testData->recommendedBlockSize,
                STEER_JSON_VALUE_BITS);
        result = STEER_AddCriterionToTest(report, 0, testId, criterionStr,
                                          (gCommonData.blockLength <= testData->recommendedBlockSize) ? true : false);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Probability value in range
        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "%s of %.*f > %.*f",
                STEER_JSON_TAG_PROBABILITY_VALUE,
                STEER_DEFAULT_FLOATING_POINT_PRECISION,
                testData->probabilityValue, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, 0.0);
        result = STEER_AddCriterionToTest(report, 0, testId, criterionStr,
                                          (testData->probabilityValue > 0.0) ? true : false);
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(criterionStr, "%s of %.*f <= %.*f",
                    STEER_JSON_TAG_PROBABILITY_VALUE,
                    STEER_DEFAULT_FLOATING_POINT_PRECISION,
                    testData->probabilityValue, 
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, 1.0);
            result = STEER_AddCriterionToTest(report, 0, testId, criterionStr,
                                              (testData->probabilityValue <= 1.0) ? true : false);
        }
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Probability value
        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "%s of %.*f >= %s of %.*f",
                STEER_JSON_TAG_PROBABILITY_VALUE,
                STEER_DEFAULT_FLOATING_POINT_PRECISION,
                testData->probabilityValue,
                STEER_JSON_TAG_SIGNIFICANCE_LEVEL,
                gCommonData.significanceLevelPrecision,
                gCommonData.significanceLevel);
        result = STEER_AddCriterionToTest(report, 0, testId, criterionStr,
                                          (testData->probabilityValue >= gCommonData.significanceLevel) ? true : false);
    }

    // Add evaluation to current test
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_AddEvaluationToTest(report, 0, testId, &(testData->passed));

    return result;
}

// =================================================================================================
//  GetNextAvailableThread
// =================================================================================================
int32_t GetNextAvailableThread (tNIST_ApproximateEntropyPrivateData* privateData)
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
bool ThreadsReady (tNIST_ApproximateEntropyPrivateData* privateData)
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
    tNIST_ApproximateEntropyTestData* testData = (tNIST_ApproximateEntropyTestData*)ptr;
	int_fast32_t i = 0;
    int_fast32_t j = 0;
    int32_t k = 0;
    int32_t r = 0;
    int32_t blockSize = 0;
    int32_t seqLength = 0;
    int32_t powLen = 0;
    int32_t index = 0;
	double sum= 0.0;
    double numOfBlocks = 0.0;
    double entropyDistribution[2] = {0, 0};  // ApEn
	uint32_t* P = NULL;

    // Setup
    testData->result = STEER_RESULT_SUCCESS;
    testData->chiSquared = 0.0;
    testData->entropyDistributionM = 0.0;
    testData->entropyDistributionMplus1 = 0.0;
    testData->approximateEntropy = 0.0;
    testData->probabilityValue = 0.0;
    testData->recommendedBlockSize = 0;
    testData->passed = false;

    seqLength = gCommonData.bitstreamLength;
    r = 0;
    
    for (blockSize = gCommonData.blockLength; blockSize <= gCommonData.blockLength + 1; blockSize++) 
    {
        if (blockSize == 0) 
        {
            entropyDistribution[0] = 0.00;
            r++;
        }
        else 
        {
            numOfBlocks = (double)seqLength;
            powLen = (int32_t)pow(2, blockSize + 1) - 1;

            // Allocate memory
            testData->result = STEER_AllocateMemory(powLen * sizeof(uint32_t), (void**)&P);

            // Check status
            if (testData->result == STEER_RESULT_SUCCESS)
            {
                for (i = 1; i < powLen - 1; i++)
                    P[i] = 0;

                // Compute frequency
                for (i = 0; i <numOfBlocks; i++) 
                {   
                    k = 1;
                    for (j = 0; j < blockSize; j++) 
                    {
                        k <<= 1;
                        if ((int32_t)testData->bitstreamBuffer[(i + j) % seqLength] == 1)
                            k++;
                    }
                    P[k - 1]++;
                }

                // Display frequency
                sum = 0.0;
                index = (int32_t)pow(2, blockSize) - 1;
                for (i = 0; i < (int32_t)pow(2, blockSize); i++) 
                {
                    if (P[index] > 0)
                        sum += P[index] * log(P[index] / numOfBlocks);
                    index++;
                }
                sum /= numOfBlocks;
                entropyDistribution[r] = sum;
                r++;

                // Clean up
                STEER_FreeMemory((void**)&P);
            }
            else    // Couldn't allocate memory
                break;
        }
    }

    // Check status
    if (testData->result == STEER_RESULT_SUCCESS)
    {
        testData->approximateEntropy = entropyDistribution[0] - entropyDistribution[1];
        testData->chiSquared = 2.0 * seqLength * (gCommonData.log2pt0 - testData->approximateEntropy);
        testData->probabilityValue = 
            cephes_igamc(pow(2.0, (double)(gCommonData.blockLength - 1)), testData->chiSquared/2.0);
        testData->recommendedBlockSize = (int32_t)((log(seqLength)/gCommonData.log2pt0) - 5);
        testData->result = STEER_CHECK_CONDITION((gCommonData.blockLength <= testData->recommendedBlockSize), 
            NIST_RESULT_BLOCK_LENGTH_GT_RECOMMENDED_BLOCK_LENGTH);
        if (testData->result == STEER_RESULT_SUCCESS)
            testData->passed = (testData->probabilityValue >= gCommonData.significanceLevel);
        testData->entropyDistributionM = entropyDistribution[0];
        testData->entropyDistributionMplus1 = entropyDistribution[1];
    }

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
    tNIST_ApproximateEntropyPrivateData* privData = NULL;
    uint32_t threadCount = 0;
    uint_fast32_t i = 0;
    void* nativeValue = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;
    memset((void*)&gCommonData, 0, sizeof(tNIST_ApproximateEntropyCommon));
    gCommonData.cliArguments = cliArguments;
    gCommonData.log2pt0 = log(2.0);

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
        result = STEER_AllocateMemory(sizeof(tNIST_ApproximateEntropyPrivateData) +
                                      (threadCount * sizeof(tNIST_ApproximateEntropyThread)),
                                      (void**)&privData);

        if (result == STEER_RESULT_SUCCESS)
        {
            // More setup
            privData->threadCount = threadCount;

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
                // Set the configuration ID
                privData->configurationState.configurationId = 0;

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
    ((tNIST_ApproximateEntropyPrivateData*)testPrivateData)->report = report;
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
    tNIST_ApproximateEntropyPrivateData* privData = (tNIST_ApproximateEntropyPrivateData*)testPrivateData;
    uint64_t testId = 0;
    int32_t threadIndex = -1;
    uint_fast32_t i = 0;
    char* end = NULL;

    // Setup
    testId = strtoull(bitstreamId, &end, 0) - 1;

    privData->configurationState.accumulatedOnes += numOnes;
    privData->configurationState.accumulatedZeros += numZeros;

    // Get next available thread
    threadIndex = GetNextAvailableThread(privData);
    if (threadIndex > -1)
    {
        // Queue it up
        memset((void*)&(privData->thread[threadIndex].testData), 0, sizeof(tNIST_ApproximateEntropyTestData));
        privData->thread[threadIndex].testId = testId;
        privData->thread[threadIndex].testData.bitstreamBuffer = buffer;
        privData->thread[threadIndex].testData.numOnes = numOnes;
        privData->thread[threadIndex].testData.numZeros = numZeros;
        result = STEER_CHECK_ERROR(pthread_create(&(privData->thread[threadIndex].threadId), NULL, &RunTest,
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
                // Update report and clean up
                for (i = 0; i < privData->threadCount; i++)
                {
                    result = UpdateReport(privData->report, 
                                          privData->thread[i].testId,
                                          &(privData->thread[i].testData));
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        privData->configurationState.testsRun++;
                        if (privData->thread[i].testData.passed)
                            privData->configurationState.testsPassed++;
                        else
                            privData->configurationState.testsFailed++;
                    }
                    else
                        break;  
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
int32_t FinalizeTest (void** privateData,
                      uint64_t suppliedNumberOfBitstreams)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_ApproximateEntropyPrivateData* privData = (tNIST_ApproximateEntropyPrivateData*)(*privateData);

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
            // Update report and clean up
            for (i = 0; i < privData->threadCount; i++)
            {
                if (privData->thread[i].testData.bitstreamBuffer != NULL)
                {
                    result = UpdateReport(privData->report, 
                                          privData->thread[i].testId,
                                          &(privData->thread[i].testData));
                    STEER_FreeMemory((void**)&(privData->thread[i].testData.bitstreamBuffer));
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        privData->configurationState.testsRun++;
                        if (privData->thread[i].testData.passed)
                            privData->configurationState.testsPassed++;
                        else
                            privData->configurationState.testsFailed++;
                    }
                    else
                        break;  
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
            // Add required metrics to configuration
            result = STEER_NistStsAddRequiredMetricsToConfiguration(privData->report, 0,
                                                                    suppliedNumberOfBitstreams,
                                                                    gCommonData.minimumTestCountRequiredForSignificance,
                                                                    privData->configurationState.testsPassed,
                                                                    gCommonData.predictedPassedTestCount,
                                                                    privData->configurationState.accumulatedOnes,
                                                                    privData->configurationState.accumulatedZeros);
        }

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
                                                                    privData->configurationState.testsRun,
                                                                    privData->configurationState.testsPassed,
                                                                    privData->configurationState.testsFailed,
                                                                    gCommonData.predictedPassedTestCount,
                                                                    gCommonData.predictedFailedTestCount);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add required criteria to configuration
            result = STEER_NistStsAddRequiredCriterionToConfiguration(privData->report, 0,
                                                                      suppliedNumberOfBitstreams,
                                                                      privData->configurationState.testsPassed,
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
                                                             privData->configurationState.testsRun, 
                                                             privData->configurationState.testsPassed);
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
