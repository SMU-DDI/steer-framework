// =================================================================================================
//! @file linear_complexity_mt.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the multi-threaded version of the NIST STS linear complexity test 
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
#include <errno.h>
#include <pthread.h>

// =================================================================================================
//  Private constants
// =================================================================================================

#define SLEEP_INTERVAL_IN_MICROSECONDS  1000

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_linearcomplexitytestdata
{
    uint8_t*    bitstreamBuffer;
    int32_t     result;                 // TODO: Need to check this!
    uint64_t    numOnes;
    uint64_t    numZeros;
    uint64_t    bitsDiscarded;
    double      chiSquared;
    int32_t     numberOfSubstrings;     // TODO: should make this uint32_t
    int32_t     frequencies[7];         // nu
    double      probabilityValue;
    bool        passed;
}
tNIST_LinearComplexityTestData;

typedef struct tnist_linearcomplexitythread
{
    pthread_t                       threadId;
    uint64_t                        testId;
    tNIST_LinearComplexityTestData  testData;
}
tNIST_LinearComplexityThread;

typedef struct tnist_linearcomplexityprivatedata
{
    tSTEER_ReportPtr                report;
    tSTEER_ConfigurationState       configurationState;
    uint32_t                        threadCount;
    tNIST_LinearComplexityThread    thread[];
}
tNIST_LinearComplexityPrivateData;

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
                      tNIST_LinearComplexityTestData* testData)
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
        // Bits discarded
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, "%" PRIu64 "", testData->bitsDiscarded);
        result = STEER_AddCalculationToTest(report, 0, testId,
                                            STEER_JSON_TAG_BITS_DISCARDED,
                                            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                            NULL, NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Chi squared
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
            sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 
                    testData->frequencies[0]);
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
                    sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 
                            testData->frequencies[i]);
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
            result = STEER_AddCalculationSetToTest(report, 0, testId, valueSet);

        (void)STEER_FreeValueSet(&valueSet);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Number of substrings
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 
                testData->numberOfSubstrings);
        result = STEER_AddCalculationToTest(report, 0, testId,
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
int32_t GetNextAvailableThread (tNIST_LinearComplexityPrivateData* privateData)
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
bool ThreadsReady (tNIST_LinearComplexityPrivateData* privateData)
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
    tNIST_LinearComplexityTestData* testData = (tNIST_LinearComplexityTestData*)ptr;
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
    testData->result = STEER_RESULT_SUCCESS;
    testData->numberOfSubstrings = 0;
    memset((void*)&(testData->frequencies), 0, 7 * sizeof(uint32_t));
    testData->chiSquared = 0.0;
    testData->probabilityValue = 0.0;
    testData->passed = false;
	
    testData->numberOfSubstrings = 
        (int32_t)floor(gCommonData.bitstreamLength/gCommonData.blockLength);
    testData->bitsDiscarded = 
        gCommonData.bitstreamLength % testData->numberOfSubstrings;

    // Allocate memory
    testData->result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t), 
                                            (void**)&B_);
    if (testData->result == STEER_RESULT_SUCCESS)
        testData->result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t), 
                                                (void**)&C);

    if (testData->result == STEER_RESULT_SUCCESS)
        testData->result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t), 
                                                (void**)&P);

    if (testData->result == STEER_RESULT_SUCCESS)
        testData->result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t), 
                                                (void**)&T);

    if (testData->result == STEER_RESULT_SUCCESS)
    {
        for (i = 0; i < kDegreesOfFreedom + 1; i++)
            testData->frequencies[i] = 0.00;

        for (ii = 0; ii < testData->numberOfSubstrings; ii++) 
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
                d = (int32_t)testData->bitstreamBuffer[ii * gCommonData.blockLength + N_];
                for (i = 1; i <= L; i++)
                    d += C[i] * testData->bitstreamBuffer[(ii * gCommonData.blockLength) + N_ - i];
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
                testData->frequencies[0]++;
            else if (linearComplexityStatistic > -2.5 && linearComplexityStatistic <= -1.5)
                testData->frequencies[1]++;
            else if (linearComplexityStatistic > -1.5 && linearComplexityStatistic <= -0.5)
                testData->frequencies[2]++;
            else if (linearComplexityStatistic > -0.5 && linearComplexityStatistic <= 0.5)
                testData->frequencies[3]++;
            else if (linearComplexityStatistic > 0.5 && linearComplexityStatistic <= 1.5)
                testData->frequencies[4]++;
            else if (linearComplexityStatistic > 1.5 && linearComplexityStatistic <= 2.5)
                testData->frequencies[5]++;
            else
                testData->frequencies[6]++;
        }
        testData->chiSquared = 0.00;

        for (i = 0; i < kDegreesOfFreedom + 1; i++)
            testData->chiSquared += (pow(testData->frequencies[i] - 
                                     (testData->numberOfSubstrings * kPreComputedProbabilities[i]), 2) / 
                                     (testData->numberOfSubstrings * kPreComputedProbabilities[i]));
        testData->probabilityValue = 
            cephes_igamc((double)(kDegreesOfFreedom/2.0), testData->chiSquared/2.0);

        testData->passed = (testData->probabilityValue >= gCommonData.significanceLevel);
    }

    // Clean up
    STEER_FreeMemory((void**)&B_);
    STEER_FreeMemory((void**)&C);
    STEER_FreeMemory((void**)&P);
    STEER_FreeMemory((void**)&T);

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
    tNIST_LinearComplexityPrivateData* privData = NULL;
    uint32_t threadCount = 0;
    uint_fast32_t i = 0;
    void* nativeValue = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;
    memset((void*)&gCommonData, 0, sizeof(tNIST_LinearComplexityCommon));
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
        result = STEER_AllocateMemory(sizeof(tNIST_LinearComplexityPrivateData) +
                                    (threadCount * sizeof(tNIST_LinearComplexityThread)),
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
    uint64_t testId = 0;
    int32_t threadIndex = -1;
    uint_fast32_t i = 0;
    char* end = NULL;

    testId = strtoull(bitstreamId, &end, 0) - 1;

    // Setup
    privData->configurationState.accumulatedOnes += numOnes;
    privData->configurationState.accumulatedZeros += numZeros;

    // Get next available thread
    threadIndex = GetNextAvailableThread(privData);
    if (threadIndex > -1)
    {
        // Queue it up
        memset((void*)&(privData->thread[threadIndex].testData), 0, sizeof(tNIST_LinearComplexityTestData));
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
    tNIST_LinearComplexityPrivateData* privData = (tNIST_LinearComplexityPrivateData*)(*privateData);

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
