// =================================================================================================
//! @file random_excursions.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the NIST STS random excursions test for the STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-07-29
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

typedef struct tnist_randomexcursionstestdata
{
    uint8_t*    bitstreamBuffer;
    int32_t     result;
    uint64_t    numOnes;
    uint64_t    numZeros;
    double      chiSquared;
    double      probabilityValue;
    uint64_t    configId;
    int32_t     x;
    bool        passed;
}
tNIST_RandomExcursionsTestData;

typedef struct tnist_randomexcursionsthread
{
    pthread_t                       threadId;
    uint64_t                        configId;
    uint64_t                        testId;
    tNIST_RandomExcursionsTestData  testData;
}
tNIST_RandomExcursionsThread;

typedef struct tnist_randomexcursionsconfiguration
{
    uint64_t    configurationId;
    uint64_t    accumulatedOnes;
    uint64_t    accumulatedZeros;
    uint64_t    testsRun;
    uint64_t    testsPassed;
    uint64_t    testsFailed;
}
tNIST_RandomExcursionsConfiguration;

typedef struct tnist_randomexcursionsprivatedata
{
    tSTEER_ReportPtr                    report;
    tNIST_RandomExcursionsConfiguration configurationState[CONFIGURATION_COUNT];
    uint32_t                            threadCount;
    tNIST_RandomExcursionsThread        thread[];
}
tNIST_RandomExcursionsPrivateData;

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
        },

        // Test specific parameter
        {
            STEER_JSON_TAG_THREAD_COUNT,
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_THREADS,
            "8",
            "1",
            "8"
        }
    }
};

static tSTEER_ParametersInfo gParametersInfo = {
    TEST_NAME,
    &gParameterInfoList
};

volatile int32_t gRunningThreadCount = 0;
pthread_mutex_t gRunningThreadCountMutex = PTHREAD_MUTEX_INITIALIZER;

// =================================================================================================
//  UpdateReport
// =================================================================================================
int32_t UpdateReport (tSTEER_ReportPtr report,
                      uint64_t configId,
                      uint64_t testId,
                      tNIST_RandomExcursionsTestData* testData)
{
    int32_t result = STEER_RESULT_SUCCESS;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };

    // Add calculations to current test

    // Add ones
    memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
    sprintf((void*)calculationStr, "%" PRIu64 "", testData->numOnes);
    result = STEER_AddCalculationToTest(report, configId, testId,
                                        STEER_JSON_TAG_ONES,
                                        STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                        NULL, STEER_JSON_VALUE_BITS,
                                        calculationStr);

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
                STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->probabilityValue);
        result = STEER_AddCalculationToTest(report, configId, testId,
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
                STEER_DEFAULT_FLOATING_POINT_PRECISION, testData->chiSquared);
        result = STEER_AddCalculationToTest(report, configId, testId,
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
        result = STEER_AddCalculationToTest(report, configId, testId,
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
        result = STEER_AddCalculationToTest(report, configId, testId,
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
        result = STEER_AddCriterionToTest(report, configId, testId, criterionStr,
                                          (gCommonData.numberOfCycles <= gCommonData.maxNumberOfCycles) ? true : false);
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
        result = STEER_AddCriterionToTest(report, configId, testId, criterionStr,
                                          (testData->probabilityValue > 0.0) ? true : false);
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(criterionStr, "%s of %.*f <= %.*f",
                    STEER_JSON_TAG_PROBABILITY_VALUE,
                    STEER_DEFAULT_FLOATING_POINT_PRECISION,
                    testData->probabilityValue,
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, 1.0);
            result = STEER_AddCriterionToTest(report, configId, testId, criterionStr,
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
        result = STEER_AddCriterionToTest(report, configId, testId, criterionStr,
                                          (testData->probabilityValue >= gCommonData.significanceLevel) ? true : false);
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
        result = STEER_AddCriterionToTest(report, configId, testId, criterionStr,
                                          (gCommonData.numberOfCycles >= gCommonData.rejectionConstraint) ? true : false);
    }

    // Add evaluation to current test
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_AddEvaluationToTest(report, configId, testId, &(testData->passed));

    return result;
}

// =================================================================================================
//  GetNextAvailableThread
// =================================================================================================
int32_t GetNextAvailableThread (tNIST_RandomExcursionsPrivateData* privateData)
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
//  RunTest
// =================================================================================================
void* RunTest (void* ptr)
{
    tNIST_RandomExcursionsTestData* testData = (tNIST_RandomExcursionsTestData*)ptr;
    int_fast32_t i = 0;

    for (i = 0; i < 6; i++)
    {
        testData->chiSquared += 
            pow(gCommonData.nu[i][testData->configId] - 
                gCommonData.numberOfCycles * gCommonData.pi[(int32_t)abs(testData->x)][i], 2) / 
                (gCommonData.numberOfCycles * gCommonData.pi[(int32_t)abs(testData->x)][i]);
    }

    testData->probabilityValue = cephes_igamc(2.5, testData->chiSquared/2.0);
    testData->passed = (testData->probabilityValue >= gCommonData.significanceLevel);

    // Decrement running thread count
    (void)STEER_CHECK_ERROR(pthread_mutex_lock(&gRunningThreadCountMutex));
    printf("Decrementing thread count\n");
    gRunningThreadCount--;
    (void)STEER_CHECK_ERROR(pthread_mutex_unlock(&gRunningThreadCountMutex));

    return NULL;
}

// =================================================================================================
//  QueueConfigurationTest
// =================================================================================================
int32_t QueueConfigurationTest (int32_t threadIndex,
                                tNIST_RandomExcursionsPrivateData* privateData,
                                uint64_t configId,
                                uint64_t testId,
                                uint8_t* bitstreamBuffer,
                                uint64_t numOnes,
                                uint64_t numZeros)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Queue it up
    privateData->thread[threadIndex].configId = configId;
    privateData->thread[threadIndex].testId = testId;
    privateData->thread[threadIndex].testData.bitstreamBuffer = bitstreamBuffer;
    privateData->thread[threadIndex].testData.result = STEER_RESULT_SUCCESS;
    privateData->thread[threadIndex].testData.numOnes = numOnes;
    privateData->thread[threadIndex].testData.numZeros = numZeros;
    privateData->thread[threadIndex].testData.chiSquared = 0.0;
    privateData->thread[threadIndex].testData.probabilityValue = 0.0;
    privateData->thread[threadIndex].testData.configId = configId;
    privateData->thread[threadIndex].testData.x = gCommonData.stateX[configId];
    privateData->thread[threadIndex].testData.passed = false;
    result = STEER_CHECK_ERROR(pthread_create(&(privateData->thread[threadIndex].threadId), 
                                              NULL, &RunTest,
                                              (void*)&(privateData->thread[threadIndex].testData)));
    if (result != STEER_RESULT_SUCCESS)
        privateData->thread[threadIndex].testData.bitstreamBuffer = NULL;
    
    return result;
}

// =================================================================================================
//  ProcessThreads
// =================================================================================================
int32_t ProcessThreads (tNIST_RandomExcursionsPrivateData* privateData)
{
    int32_t result = STEER_RESULT_SUCCESS;
    uint_fast32_t i = 0;

    // Start them up
    for (i = 0; i < privateData->threadCount; i++)
    {
        // Is the bitstream buffer valid?
        if (privateData->thread[i].testData.bitstreamBuffer != NULL)
        {
            // Increment thread count
            (void)STEER_CHECK_ERROR(pthread_mutex_lock(&gRunningThreadCountMutex));
            gRunningThreadCount++;
            (void)STEER_CHECK_ERROR(pthread_mutex_unlock(&gRunningThreadCountMutex));

            // Start this thread
            result = STEER_CHECK_ERROR(pthread_join(privateData->thread[i].threadId, NULL));
            if (result != STEER_RESULT_SUCCESS)
            {
                // Decrement thread count
                (void)STEER_CHECK_ERROR(pthread_mutex_lock(&gRunningThreadCountMutex));
                gRunningThreadCount--;
                (void)STEER_CHECK_ERROR(pthread_mutex_unlock(&gRunningThreadCountMutex));
                break;
            }
        } 
    }

    // Wait for threads to complete
    while (gRunningThreadCount != 0)
    {
        usleep(SLEEP_INTERVAL_IN_MICROSECONDS);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Update report and clean up
        for (i = 0; i < privateData->threadCount; i++)
        {
            // Is the bitstream buffer valid?
            if (privateData->thread[i].testData.bitstreamBuffer != NULL)
            {
                // Update the report for this configuration and test
                result = UpdateReport(privateData->report, 
                                      privateData->thread[i].configId,
                                      privateData->thread[i].testId,
                                      &(privateData->thread[i].testData));
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Keep track of statistics
                    privateData->configurationState[privateData->thread[i].configId].testsRun++;
                    if (privateData->thread[i].testData.passed)
                        privateData->configurationState[privateData->thread[i].configId].testsPassed++;
                    else
                        privateData->configurationState[privateData->thread[i].configId].testsFailed++;
                }
            }

            // Reset the bitstream buffer
            privateData->thread[i].testData.bitstreamBuffer = NULL;

            if (result != STEER_RESULT_SUCCESS)
                break;
        }
    }
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
    tNIST_RandomExcursionsPrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;
    gCommonData.cliArguments = cliArguments;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tNIST_RandomExcursionsPrivateData),
                                  (void*)&privData);
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
                    {
                        result = STEER_ConvertStringToUnsigned32BitInteger(parameters->parameter[i].precision,
                                                                           &(gCommonData.significanceLevelPrecision));
                    }
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
                    privData->threadCount = *((uint32_t*)nativeValue);
                    STEER_FreeMemory((void**)&nativeValue);
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                break;
        }
    }


    if (result == STEER_RESULT_SUCCESS)
    {
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
            // Reallocate private data to account for threads
            result = STEER_ReallocateMemory(sizeof(tNIST_RandomExcursionsPrivateData),
                                            sizeof(tNIST_RandomExcursionsPrivateData) +
                                            (privData->threadCount * sizeof(tNIST_RandomExcursionsThread)),
                                            (void**)&privData);
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
            sprintf(attributeStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 
                    gCommonData.stateX[i]);
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
    char progressStr[STEER_STRING_MAX_LENGTH] = { 0 };
    int32_t threadIndex = 0;
    int_fast32_t i = 0;
    uint64_t testId = 0;
    char* end = NULL;

    testId = strtoull(bitstreamId, &end, 0) - 1;

    // Walk the configurations
    for (i = 0; i < CONFIGURATION_COUNT; i++)
    {
        privData->configurationState[i].accumulatedOnes += numOnes;
        privData->configurationState[i].accumulatedZeros += numZeros;

        // Get next available thread
        threadIndex = GetNextAvailableThread(privData);
        if (threadIndex > -1)
        {
            // Queue it up
            result = QueueConfigurationTest(threadIndex, privData, i, testId,
                                            buffer, numOnes, numZeros);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (gCommonData.cliArguments->reportProgress)
                {
                    memset((void*)progressStr, 0, STEER_STRING_MAX_LENGTH);
                    if (i < 4)
                        sprintf(progressStr, "Testing excursion state %d...", i - 4);
                    else
                        sprintf(progressStr, "Testing excursion state %d...", i - 3);
                    STEER_REPORT_PROGRESS(gCommonData.cliArguments->programName, progressStr);
                }
            }
        }
        else    // Thread pool is full
        {
            result = ProcessThreads(privData);

            // Queue up this configuration
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get next available thread
                threadIndex = GetNextAvailableThread(privData);
                if (threadIndex > -1)
                {
                    result = QueueConfigurationTest(threadIndex, privData, i, testId,
                                                    buffer, numOnes, numZeros);
                }
            }
        }

        if (result != STEER_RESULT_SUCCESS)
            break;
    }

    // Make sure all pending threads have been processed
    if (result == STEER_RESULT_SUCCESS)
        result = ProcessThreads(privData);

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
