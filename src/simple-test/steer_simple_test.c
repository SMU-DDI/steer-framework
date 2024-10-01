// =================================================================================================
//! @file steer_simple_test.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file contains private constants, function prototypes and function implementations
//! for a STEER simple test.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-07-02
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_json_utilities.h"
#include "steer_string_utilities.h"
#include "steer_parameters_info_utilities.h"
#include "steer_report_utilities.h"
#include "steer_test_info_utilities.h"
#include "steer_test_shell.h"
#include "steer_utilities.h"
#include "steer_value_utilities.h"

// =================================================================================================
//  Private constants
// =================================================================================================

#define SIMPLE_TEST_NAME        "simple test"
#define SIMPLE_CALCULATION_NAME "bytes read"
#define SIMPLE_METRIC_NAME      "total bytes read"
#define SIMPLE_NUM_CONFIGS      1

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tsimpleprivatedata
{
    tSTEER_CliArguments*    cliArguments;
    tSTEER_ReportPtr        report;
    uint64_t                bitstreamCount;
    uint64_t                bitstreamLength;
    uint64_t                testsRun;
    uint64_t                testsPassed;
    uint64_t                testsFailed;
    uint64_t                accumulatedBytesRead;
}
tSimplePrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

// Define references
static tSTEER_InfoList gReferences = {
    1, { "STEER Developers Guide" }
};

// Define authors
static tSTEER_InfoList gAuthors = {
    1, { STEER_AUTHOR_GARY_WOODCOCK }
};

// Define maintainers
static tSTEER_InfoList gMaintainers = {
    2, 
    { 
        STEER_MAINTAINER_ANAMETRIC,
        STEER_MAINTAINER_SMU_DARWIN_DEASON 
    }
};

// Define test info
static tSTEER_TestInfo gTestInfo = {
    SIMPLE_TEST_NAME,
    NULL,
    "This is a simple STEER test written in C.",
    eSTEER_Complexity_Simple,
    &gReferences,
    "steer_simple_test",
    "0.1.0",
    eSTEER_InputFormat_Bitstream,
    STEER_REPOSITORY_URI,
    &gAuthors,
    NULL,
    &gMaintainers,
    STEER_CONTACT
};

// Define parameter info
static tSTEER_ParameterInfoList gParameterInfoList = {
    2, 
    {
        // Required parameter
        {
            STEER_JSON_TAG_BITSTREAM_COUNT,
            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITSTREAMS,
            "1",
            "1",
            "1024"
        },

        // Required parameter
        {
            STEER_JSON_TAG_BITSTREAM_LENGTH,
            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITS,
            "1000000",
            "1024",
            "10000000"
        }
    }
};

static tSTEER_ParametersInfo gParametersInfo = {
    SIMPLE_TEST_NAME,
    &gParameterInfoList
};

// =================================================================================================
//  SimpleTestGetInfo
// =================================================================================================
char* SimpleTestGetInfo (void)
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
//  SimpleTestGetParametersInfo
// =================================================================================================
char* SimpleTestGetParametersInfo (void)
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
//  SimpleTestInit
// =================================================================================================
int32_t SimpleTestInit (tSTEER_CliArguments* cliArguments,
                        tSTEER_ParameterSet* parameters,
                        void** testPrivateData,
                        uint64_t* bufferSizeInBytes)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSimplePrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tSimplePrivateData), (void**)&privData);
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        void* nativeValue = NULL;

        // Cache CLI arguments and report pointer
        privData->cliArguments = cliArguments;

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
                    privData->bitstreamCount = *((uint64_t*)nativeValue);
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
                    privData->bitstreamLength = *((uint64_t*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                break;
        }

        // Make sure we got the required parameters and that they're good
        if ((privData->bitstreamCount == 0) || 
            (privData->bitstreamLength == 0) ||
            ((privData->bitstreamLength % 8) != 0))
        {
            result = STEER_CHECK_ERROR(EINVAL);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Return private data and bitstream buffer size
            *testPrivateData = (void*)privData;
            *bufferSizeInBytes = (privData->bitstreamLength / 8);
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
//  SimpleTestGetConfigurationCount
// =================================================================================================
uint32_t SimpleTestGetConfigurationCount (void* testPrivateData)
{
    return SIMPLE_NUM_CONFIGS;
}

// =================================================================================================
//  SimpleTestSetReport
// =================================================================================================
int32_t SimpleTestSetReport (void* testPrivateData,
                             tSTEER_ReportPtr report)
{
    tSimplePrivateData* privData = (tSimplePrivateData*)testPrivateData;

    privData->report = report;

    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  SimpleTestExecute
// =================================================================================================
int32_t SimpleTestExecute (void* testPrivateData,
                           const char* bitstreamId,
                           uint8_t* buffer,
                           uint64_t bufferSizeInBytes,
                           uint64_t bytesInBuffer,
                           uint64_t numZeros,
                           uint64_t numOnes)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSimplePrivateData* privData = (tSimplePrivateData*)testPrivateData;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char progressStr[STEER_STRING_MAX_LENGTH] = { 0 };
    uint64_t testId = 0;
    char* end = NULL;

    testId = strtoull(bitstreamId, &end, 0) - 1;

    // Update accumulated byte count
    privData->accumulatedBytesRead += bytesInBuffer;

    // Do some kind of testing here...

    // Report some progress, if asked
    if (privData->cliArguments->reportProgress == true)
    {
        memset((void*)progressStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(progressStr, "Executing test with bitstream %s...", bitstreamId);
        STEER_REPORT_PROGRESS(privData->cliArguments->programName, progressStr);
    }

    // Add calculations to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Add sample calculation
        memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, "%" PRIu64 "", bytesInBuffer);
        result = STEER_AddCalculationToTest(privData->report, 0, testId, 
                                            SIMPLE_CALCULATION_NAME,
                                            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BYTES,
                                            calculationStr);
    }

    // Add criteria to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        bool passed = false;

        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "bytes in buffer (%" PRIu64 ") > 0",  bytesInBuffer);
        if (bytesInBuffer > 0)
        {
            privData->testsPassed++;
            result = STEER_AddCriterionToTest(privData->report, 0, testId,
                                                criterionStr, STEER_JSON_VALUE_TRUE);
            // Add evaluation to current test
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_AddEvaluationToTest(privData->report, 0, testId, &passed);
        }
        else
        {
            privData->testsFailed++;
            result = STEER_AddCriterionToTest(privData->report, 0, testId,
                                              criterionStr, STEER_JSON_VALUE_FALSE);
            // Add evaluation to current test
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_AddEvaluationToTest(privData->report, 0, testId, &passed);
        }

        // Update tests run count
        if (result == STEER_RESULT_SUCCESS)
            privData->testsRun++;
    }

    // Clean up
    STEER_FreeMemory((void**)&buffer);

    return result;
}

// =================================================================================================
//  SimpleTestFinalize
// =================================================================================================
int32_t SimpleTestFinalize (void** testPrivateData,
                            uint64_t suppliedNumberOfBitstreams)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSimplePrivateData* privData = (tSimplePrivateData*)(*testPrivateData);

    if (privData != NULL)
    {
        char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
        char metricStr[STEER_STRING_MAX_LENGTH] = { 0 };

        // Add metric to configuration
        memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(metricStr, "%" PRIu64 "", privData->accumulatedBytesRead);
        result = STEER_AddMetricToConfiguration(privData->report, 0, SIMPLE_METRIC_NAME,
                                                STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                                NULL, STEER_JSON_VALUE_BYTES, metricStr);
        
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add criteria to configuration
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(criterionStr, "total bytes read (%" PRIu64 ") > 0",
                    privData->accumulatedBytesRead);
            result = STEER_AddCriterionToConfiguration(privData->report, 0, criterionStr,
                                                       (privData->accumulatedBytesRead > 0) ? 
                                                       STEER_JSON_VALUE_TRUE : STEER_JSON_VALUE_FALSE);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add evaulation to configuration
            result = STEER_AddEvaluationToConfiguration(privData->report, 0);
        }
    }

    // Clean up
    STEER_FreeMemory((void**)testPrivateData);

    return result;
}

// =================================================================================================
//  main
// =================================================================================================
int main (int argc, const char * argv[])
{
    // Run STEER program
    return STEER_Run ("steer_simple_test", argc, argv,
                      SimpleTestGetInfo, SimpleTestGetParametersInfo,
                      SimpleTestInit, SimpleTestGetConfigurationCount,
                      SimpleTestSetReport, SimpleTestExecute, SimpleTestFinalize);
}

// =================================================================================================
