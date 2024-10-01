// =================================================================================================
//! @file libsteer_private_unit_test.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements a unit test of the libsteer private interfaces.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any
//! @date 2022-06-09
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include <CUnit.h>
#include <Automated.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "steer_file_system_utilities.h"
#include "steer_file_system_utilities_private.h"
#include "steer_json_utilities.h"
#include "steer_json_utilities_private.h"
#include "steer_parameter_set_utilities_private.h"
#include "steer_parameters_info_utilities.h"
#include "steer_parameters_info_utilities_private.h"
#include "steer_report_utilities_private.h"
#include "steer_schedule_utilities_private.h"
#include "steer_test_info_utilities.h"
#include "steer_test_info_utilities_private.h"
#include "steer_test_shell.h"
#include "steer_test_shell_private.h"
#include "steer_utilities.h"
#include "steer_utilities_private.h"
#include "steer_value_utilities.h"
#include "steer_value_utilities_private.h"

// =================================================================================================
//  Private globals
// =================================================================================================

#define DUMMY_TEST_NAME     "dummy test"
#define DUMMY_PROGRAM_NAME  "dummy_test"
#define NOT_JSON            "This is not JSON."

static const char* kDummyBadJsonTagTestInfoJson = "{ \
    \"test information\": { \
        \"my name\": \"dummy test\", \
        \"description\": \"This is a dummy test used for unit testing.\", \
        \"complexity\": \"simple\", \
        \"program name\": \"dummy_test\", \
        \"program version\": \"0.1.0\", \
        \"input format\": \"bitstream\", \
        \"authors\": [ \"Unit test\" ] \
    } \
}";

static const char* kDummyMalformedJsonTestInfoJson = "{ \
    \"test info\": { \
        \"name\": \"dummy test\", \
        \"description\": \"This is a dummy test used for unit testing.\", \
        \"complexity\": \"simple\", \
        \"program name\": \"dummy_test\", \
        \"program version\": \"0.1.0\", \
        \"input format\": \"bitstream\", \
        \"authors\": [ \"Unit test\" ] \
}";

static const char* kDummyTestInfoJson = "{ \
    \"test info\": { \
        \"name\": \"dummy test\", \
        \"description\": \"This is a dummy test used for unit testing.\", \
        \"complexity\": \"simple\", \
        \"program name\": \"dummy_test\", \
        \"program version\": \"0.1.0\", \
        \"input format\": \"bitstream\", \
        \"authors\": [ \"Unit test\" ] \
    } \
}";

static const char* kDummyBadJsonTagParameterInfoJson = "{ \
    \"parameters information\": { \
        \"parameters\": { \
            \"bitstream count\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bitstreams\", \
                \"default value\": \"1\", \
                \"minimum value\": \"1\", \
                \"maximum value\": \"1024\" \
            }, \
            \"bitstream length\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bits\", \
                \"default value\": \"1000000\", \
                \"minimum value\": \"1024\", \
                \"maximum value\": \"10000000\" \
            }, \
            \"significance level (alpha)\": { \
                \"data type\": \"double precision floating point\", \
                \"precision\": \"6\", \
                \"default value\": \"0.01\", \
                \"minimum value\": \"0.001\", \
                \"maximum value\": \"0.01\" \
            }] \
        }, \
        \"test name\": \"dummy test\" \
    } \
}";

static const char* kDummyMalformedJsonParameterInfoJson = "{ \
    \"parameters info\": { \
        \"parameters\": { \
            \"bitstream count\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bitstreams\", \
                \"default value\": \"1\", \
                \"minimum value\": \"1\", \
                \"maximum value\": \"1024\" \
            }, \
            \"bitstream length\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bits\", \
                \"default value\": \"1000000\", \
                \"minimum value\": \"1024\", \
                \"maximum value\": \"10000000\" \
            }, \
            \"significance level (alpha)\": { \
                \"data type\": \"double precision floating point\", \
                \"precision\": \"6\", \
                \"default value\": \"0.01\", \
                \"minimum value\": \"0.001\", \
                \"maximum value\": \"0.01\" \
            }] \
        \"test name\": \"dummy test\" \
    } \
}";

static const char* kDummyParametersInfoJson = "{ \
    \"parameters info\": { \
        \"parameters\": { \
            \"bitstream count\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bitstreams\", \
                \"default value\": \"1\", \
                \"minimum value\": \"1\", \
                \"maximum value\": \"1024\" \
            }, \
            \"bitstream length\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bits\", \
                \"default value\": \"1000000\", \
                \"minimum value\": \"1024\", \
                \"maximum value\": \"10000000\" \
            }, \
            \"significance level (alpha)\": { \
                \"data type\": \"double precision floating point\", \
                \"precision\": \"6\", \
                \"default value\": \"0.01\", \
                \"minimum value\": \"0.001\", \
                \"maximum value\": \"0.01\" \
            } \
        }, \
        \"test name\": \"dummy test\" \
    } \
}";

static const char* kDummyBadJsonTagParameterSet = "{ \
    \"a parameter set\": { \
        \"test name\": \"dummy test\", \
        \"parameter set name\": \"dummy parameter set\", \
        \"parameters\": { \
            \"bitstream count\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bitstreams\", \
                \"value\": \"1\" \
            } \
        } \
    } \
}";

static const char* kDummyMalformedJsonParameterSet = "{ \
    \"parameter set\": { \
        \"test name\": \"dummy test\", \
        \"parameter set name\": \"dummy parameter set\", \
        \"parameters\": { \
            \"bitstream count\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bitstreams\", \
                \"value\": \"1\" \
        } \
    } \
}";

static const char* kDummyParameterSet = "{ \
    \"parameter set\": { \
        \"test name\": \"dummy test\", \
        \"parameter set name\": \"dummy parameter set\", \
        \"parameters\": { \
            \"bitstream count\": { \
                \"data type\": \"unsigned 64-bit integer\", \
                \"units\": \"bitstreams\", \
                \"value\": \"1\" \
            } \
        } \
    } \
}";

static tSTEER_InfoList gAuthor = {
    1, { "Unit tester" }
};

static tSTEER_TestInfo gDummyTestInfo = {
    DUMMY_TEST_NAME,
    NULL,
    "This is a dummy test used for unit testing.",
    eSTEER_Complexity_Simple,
    NULL,
    DUMMY_PROGRAM_NAME,
    "0.1.0",
    eSTEER_InputFormat_Bitstream,
    NULL,
    &gAuthor,
    NULL,
    NULL,
    NULL
};

static tSTEER_ParameterInfoList gDummyParameterInfoList = {
    3, {
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
        },

        // Required parameter
        {
            STEER_JSON_TAG_SIGNIFICANCE_LEVEL,
            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION,
            NULL,
            STEER_JSON_VALUE_DEFAULT_SIGNIFICANCE_LEVEL,
            STEER_JSON_VALUE_MINIMUM_SIGNIFICANCE_LEVEL,
            STEER_JSON_VALUE_MAXIMUM_SIGNIFICANCE_LEVEL
        }
    }
};

static tSTEER_ParametersInfo gDummyParametersInfo = {
    DUMMY_TEST_NAME,
    &gDummyParameterInfoList
};

// =================================================================================================
//  CreateTestFile
// =================================================================================================
int32_t CreateTestFile (const char* path,
                        const char* contents)
{
    int32_t result = STEER_RESULT_SUCCESS;
    FILE* fp = NULL;

    fp = fopen(path, "w");
    if (fp != NULL)
    {
        if (contents != NULL)
        {
            if (fputs(contents, fp) == EOF)
                result = errno;
        }

        (void)fclose(fp);
        fp = NULL;
    }
    else
        result = errno;

    return result;
}

// =================================================================================================
//  DummyGetInfoFunction
// =================================================================================================
char* DummyGetInfoFunction (void)
{
    char* json = NULL;
    int32_t result = STEER_TestInfoToJson(&gDummyTestInfo, &json);
    if (result == STEER_RESULT_SUCCESS)
        return json;
    else
        return NULL;
}

// =================================================================================================
//  DummyGetParametersInfoFunction
// =================================================================================================
char* DummyGetParametersInfoFunction (void)
{
    char* json = NULL;
    int32_t result = STEER_ParametersInfoToJson(&gDummyParametersInfo, &json);
    if (result == STEER_RESULT_SUCCESS)
        return json;
    else
        return NULL;
}

// =================================================================================================
//  DummyInitFunction
// =================================================================================================
int32_t DummyInitFunction (tSTEER_CliArguments* cliArguments,
                           tSTEER_ParameterSet* parameters,
                           tSTEER_ReportPtr report,
                           void** privateData,
                           uint64_t* bufferSizeInBytes)
{
    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  DummyExecuteFunction
// =================================================================================================
int32_t DummyExecuteFunction (void* testPrivateData,
                              const char* bitstreamId,
                              uint8_t* buffer,
                              uint64_t bufferSizeInBytes,
                              uint64_t bytesInBuffer,
                              uint64_t numZeros,
                              uint64_t numOnes)
{
    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  DummyFinalizeFunction
// =================================================================================================
int32_t DummyFinalizeFunction (void** testPrivateData,
                               uint64_t suppliedNumberOfBitstreams)
{
    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  STEER_LibPrivate_SuiteInit
// =================================================================================================
int STEER_LibPrivate_SuiteInit (void)
{
    return CUE_SUCCESS;
}

// =================================================================================================
//  STEER_LibPrivate_SuiteCleanup
// =================================================================================================
int STEER_LibPrivate_SuiteCleanup (void)
{
    return CUE_SUCCESS;
}

// =================================================================================================
//  STEER_PathHasFileExtension_Test
// =================================================================================================
void STEER_PathHasFileExtension_Test (void)
{
    CU_FAIL("TODO: Implement STEER_PathHasFileExtension_Test");
}

// =================================================================================================
//  STEER_IsFilePath_Test
// =================================================================================================
void STEER_IsFilePath_Test (void)
{
    CU_FAIL("TODO: Implement STEER_IsFilePath_Test");
}

// =================================================================================================
//  STEER_IsFileEmpty_Test
// =================================================================================================
void STEER_IsFileEmpty_Test (void)
{
    CU_FAIL("TODO: Implement STEER_IsFileEmpty_Test");
}

// =================================================================================================
//  STEER_ExpandTildeInPath_Test
// =================================================================================================
void STEER_ExpandTildeInPath_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ExpandTildeInPath_Test");
}

// =================================================================================================
//  STEER_IsDirectoryPath_Test
// =================================================================================================
void STEER_IsDirectoryPath_Test (void)
{
    CU_FAIL("TODO: Implement STEER_IsDirectoryPath_Test");
}

// =================================================================================================
//  STEER_DirectoryExists_Test
// =================================================================================================
void STEER_DirectoryExists_Test (void)
{
    CU_FAIL("TODO: Implement STEER_DirectoryExists_Test");
}

// =================================================================================================
//  STEER_CreateDirectory_Test
// =================================================================================================
void STEER_CreateDirectory_Test (void)
{
    CU_FAIL("TODO: Implement STEER_CreateDirectory_Test");
}

// =================================================================================================
//  STEER_DirectoryHasTrailingSlash_Test
// =================================================================================================
void STEER_DirectoryHasTrailingSlash_Test (void)
{
    CU_FAIL("TODO: Implement STEER_DirectoryHasTrailingSlash_Test");
}

// =================================================================================================
//  STEER_TrimFileExtensionFromPath_Test
// =================================================================================================
void STEER_TrimFileExtensionFromPath_Test (void)
{
    CU_FAIL("TODO: Implement STEER_TrimFileExtensionFromPath_Test");
}

// =================================================================================================
//  STEER_ValidateFileString_Test
// =================================================================================================
void STEER_ValidateFileString_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ValidateFileString_Test");
}

// =================================================================================================
//  STEER_ValidatePathString_Test
// =================================================================================================
void STEER_ValidatePathString_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ValidatePathString_Test");
}

// =================================================================================================
//  STEER_GetObjectStringValue_Test
// =================================================================================================
void STEER_GetObjectStringValue_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetObjectStringValue_Test");
}

// =================================================================================================
//  STEER_GetChildObject_Test
// =================================================================================================
void STEER_GetChildObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetChildObject_Test");
}

// =================================================================================================
//  STEER_GetChildObjectBoolean_Test
// =================================================================================================
void STEER_GetChildObjectBoolean_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetChildObjectBoolean_Test");
}

// =================================================================================================
//  STEER_AddChildObjectString_Test
// =================================================================================================
void STEER_AddChildObjectString_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddChildObjectString_Test");
}

// =================================================================================================
//  STEER_AddChildObjectBoolean_Test
// =================================================================================================
void STEER_AddChildObjectBoolean_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddChildObjectBoolean_Test");
}

// =================================================================================================
//  STEER_AddEmptyNamedChildArray_Test
// =================================================================================================
void STEER_AddEmptyNamedChildArray_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddEmptyNamedChildArray_Test");
}

// =================================================================================================
//  STEER_NistStsAddRequiredMetricsToConfiguration_Test
// =================================================================================================
void STEER_NistStsAddRequiredMetricsToConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NistStsAddRequiredMetricsToConfiguration_Test");
}

// =================================================================================================
//  STEER_NistStsAddMetricsToConfiguration_Test
// =================================================================================================
void STEER_NistStsAddMetricsToConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NistStsAddMetricsToConfiguration_Test");
}

// =================================================================================================
//  STEER_NistStsAddRequiredCriterionToConfiguration_Test
// =================================================================================================
void STEER_NistStsAddRequiredCriterionToConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NistStsAddRequiredCriterionToConfiguration_Test");
}

// =================================================================================================
//  STEER_NistStsAddCriteriaToConfiguration_Test
// =================================================================================================
void STEER_NistStsAddCriteriaToConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NistStsAddCriteriaToConfiguration_Test");
}

// =================================================================================================
//  STEER_NistStsGetProbabilityValueFromTest_Test
// =================================================================================================
void STEER_NistStsGetProbabilityValueFromTest_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NistStsGetProbabilityValueFromTest_Test");
}

// =================================================================================================
//  STEER_NistStsGetProbabilityValuesFromConfiguration_Test
// =================================================================================================
void STEER_NistStsGetProbabilityValuesFromConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NistStsGetProbabilityValuesFromConfiguration_Test");
}

// =================================================================================================
//  STEER_ValidateParameterSet_Test
// =================================================================================================
void STEER_ValidateParameterSet_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ValidateParameterSet_Test");
}

// =================================================================================================
//  STEER_GetNextParameter_Test
// =================================================================================================
void STEER_GetNextParameter_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetNextParameter_Test");
}

// =================================================================================================
//  STEER_DefaultParameterCheck_Test
// =================================================================================================
void STEER_DefaultParameterCheck_Test (void)
{
    CU_FAIL("TODO: Implement STEER_DefaultParameterCheck_Test");
}

// =================================================================================================
//  STEER_JsonToParameterSet_Test
// =================================================================================================
void STEER_JsonToParameterSet_Test (void)
{
    CU_FAIL("TODO: Implement STEER_JsonToParameterSet_Test");
}

// =================================================================================================
//  STEER_ParameterSetToJson_Test
// =================================================================================================
void STEER_ParameterSetToJson_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ParameterSetToJson_Test");
}

// =================================================================================================
//  STEER_FreeParameterSet_Test
// =================================================================================================
void STEER_FreeParameterSet_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeParameterSet_Test");
}

// =================================================================================================
//  STEER_GetParameterValue_Test
// =================================================================================================
void STEER_GetParameterValue_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetParameterValue_Test");
}

// =================================================================================================
//  STEER_ValidateParametersInfo_Test
// =================================================================================================
void STEER_ValidateParametersInfo_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ValidateParametersInfo_Test");
}

// =================================================================================================
//  STEER_GetNextParameterInfo_Test
// =================================================================================================
void STEER_GetNextParameterInfo_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetNextParameterInfo_Test");
}

// =================================================================================================
//  STEER_AddChildParameterInfo_Test
// =================================================================================================
void STEER_AddChildParameterInfo_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddChildParameterInfo_Test");
}

// =================================================================================================
//  STEER_JsonToParametersInfo_Test
// =================================================================================================
void STEER_JsonToParametersInfo_Test (void)
{
    CU_FAIL("TODO: Implement STEER_JsonToParametersInfo_Test");
}

// =================================================================================================
//  STEER_FreeParametersInfo_Test
// =================================================================================================
void STEER_FreeParametersInfo_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeParametersInfo_Test");
}

// =================================================================================================
//  STEER_CompareDoubles_Test
// =================================================================================================
void STEER_CompareDoubles_Test (void)
{
    CU_FAIL("TODO: Implement STEER_CompareDoubles_Test");
}

// =================================================================================================
//  STEER_FreeCriteria_Test
// =================================================================================================
void STEER_FreeCriteria_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeCriteria_Test");
}

// =================================================================================================
//  STEER_FreeTest_Test
// =================================================================================================
void STEER_FreeTest_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeTest_Test");
}

// =================================================================================================
//  STEER_FreeTests_Test
// =================================================================================================
void STEER_FreeTests_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeTests_Test");
}

// =================================================================================================
//  STEER_FreeConfigurations_Test
// =================================================================================================
void STEER_FreeConfigurations_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeConfigurations_Test");
}

// =================================================================================================
//  STEER_NewEmptyReport_Test
// =================================================================================================
void STEER_NewEmptyReport_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NewEmptyReport_Test");
}

// =================================================================================================
//  STEER_NewReport_Test
// =================================================================================================
void STEER_NewReport_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NewReport_Test");
}

// =================================================================================================
//  STEER_AddConfigurationToReport_Test
// =================================================================================================
void STEER_AddConfigurationToReport_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddConfigurationToReport_Test");
}

// =================================================================================================
//  STEER_AddTestToConfiguration_Test
// =================================================================================================
void STEER_AddTestToConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddTestToConfiguration_Test");
}

// =================================================================================================
//  STEER_AddMetricSetToConfiguration_Test
// =================================================================================================
void STEER_AddMetricSetToConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddMetricSetToConfiguration_Test");
}

// =================================================================================================
//  STEER_GetParameterFromReport_Test
// =================================================================================================
void STEER_GetParameterFromReport_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetParameterFromReport_Test");
}

// =================================================================================================
//  STEER_GetMetricFromConfiguration_Test
// =================================================================================================
void STEER_GetMetricFromConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetMetricFromConfiguration_Test");
}

// =================================================================================================
//  STEER_GetMetricSetFromConfiguration_Test
// =================================================================================================
void STEER_GetMetricSetFromConfiguration_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetMetricSetFromConfiguration_Test");
}

// =================================================================================================
//  STEER_ValidateReport_Test
// =================================================================================================
void STEER_ValidateReport_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ValidateReport_Test");
}

// =================================================================================================
//  STEER_GetReportHeaderFromReportObject_Test
// =================================================================================================
void STEER_GetReportHeaderFromReportObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetReportHeaderFromReportObject_Test");
}

// =================================================================================================
//  STEER_GetParametersFromReportObject_Test
// =================================================================================================
void STEER_GetParametersFromReportObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetParametersFromReportObject_Test");
}

// =================================================================================================
//  STEER_GetConfigurationsFromReportObject_Test
// =================================================================================================
void STEER_GetConfigurationsFromReportObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetConfigurationsFromReportObject_Test");
}

// =================================================================================================
//  STEER_GetTestsFromConfigurationObject_Test
// =================================================================================================
void STEER_GetTestsFromConfigurationObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetTestsFromConfigurationObject_Test");
}

// =================================================================================================
//  STEER_GetCriteriaFromParentObject_Test
// =================================================================================================
void STEER_GetCriteriaFromParentObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetCriteriaFromParentObject_Test");
}

// =================================================================================================
//  STEER_GetEvaluationFromParentObject_Test
// =================================================================================================
void STEER_GetEvaluationFromParentObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetEvaluationFromParentObject_Test");
}

// =================================================================================================
//  STEER_NewReportObject_Test
// =================================================================================================
void STEER_NewReportObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_NewReportObject_Test");
}

// =================================================================================================
//  STEER_AddParametersToReportObject_Test
// =================================================================================================
void STEER_AddParametersToReportObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddParametersToReportObject_Test");
}

// =================================================================================================
//  STEER_AddConfigurationsToReportObject_Test
// =================================================================================================
void STEER_AddConfigurationsToReportObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddConfigurationsToReportObject_Test");
}

// =================================================================================================
//  STEER_AddTestsToTestsArray_Test
// =================================================================================================
void STEER_AddTestsToTestsArray_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddTestsToTestsArray_Test");
}

// =================================================================================================
//  STEER_AddCriteriaToCriteriaArray_Test
// =================================================================================================
void STEER_AddCriteriaToCriteriaArray_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddCriteriaToCriteriaArray_Test");
}

// =================================================================================================
//  STEER_AddEvaluationToParentObject_Test
// =================================================================================================
void STEER_AddEvaluationToParentObject_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddEvaluationToParentObject_Test");
}

// =================================================================================================
//  STEER_ReportToJson_Test
// =================================================================================================
void STEER_ReportToJson_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ReportToJson_Test");
}

// =================================================================================================
//  STEER_ValidateSchedule_Test
// =================================================================================================
void STEER_ValidateSchedule_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ValidateSchedule_Test");
}

// =================================================================================================
//  STEER_GetScheduledTests_Test
// =================================================================================================
void STEER_GetScheduledTests_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetScheduledTests_Test");
}

// =================================================================================================
//  STEER_StringIsPositiveInteger_Test
// =================================================================================================
void STEER_StringIsPositiveInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_StringIsPositiveInteger_Test");
}

// =================================================================================================
//  STEER_StringIsNegativeInteger_Test
// =================================================================================================
void STEER_StringIsNegativeInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_StringIsNegativeInteger_Test");
}

// =================================================================================================
//  STEER_ConvertStringToBoolean_Test
// =================================================================================================
void STEER_ConvertStringToBoolean_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToBoolean_Test");
}

// =================================================================================================
//  STEER_ConvertStringToDoublePrecisionFloatingPoint_Test
// =================================================================================================
void STEER_ConvertStringToDoublePrecisionFloatingPoint_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToDoublePrecisionFloatingPoint_Test");
}

// =================================================================================================
//  STEER_ConvertStringToExtendedPrecisionFloatingPoint_Test
// =================================================================================================
void STEER_ConvertStringToExtendedPrecisionFloatingPoint_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToExtendedPrecisionFloatingPoint_Test");
}

// =================================================================================================
//  STEER_ConvertStringToSigned8BitInteger_Test
// =================================================================================================
void STEER_ConvertStringToSigned8BitInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToSigned8BitInteger_Test");
}

// =================================================================================================
//  STEER_ConvertStringToSigned16BitInteger_Test
// =================================================================================================
void STEER_ConvertStringToSigned16BitInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToSigned16BitInteger_Test");
}

// =================================================================================================
//  STEER_ConvertStringToSigned32BitInteger_Test
// =================================================================================================
void STEER_ConvertStringToSigned32BitInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToSigned32BitInteger_Test");
}

// =================================================================================================
//  STEER_ConvertStringToSigned64BitInteger_Test
// =================================================================================================
void STEER_ConvertStringToSigned64BitInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToSigned64BitInteger_Test");
}

// =================================================================================================
//  STEER_ConvertStringToSinglePrecisionFloatingPoint_Test
// =================================================================================================
void STEER_ConvertStringToSinglePrecisionFloatingPoint_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToSinglePrecisionFloatingPoint_Test");
}

// =================================================================================================
//  STEER_ConvertStringToUnsigned8BitInteger_Test
// =================================================================================================
void STEER_ConvertStringToUnsigned8BitInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToUnsigned8BitInteger_Test");
}

// =================================================================================================
//  STEER_ConvertStringToUnsigned16BitInteger_Test
// =================================================================================================
void STEER_ConvertStringToUnsigned16BitInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToUnsigned16BitInteger_Test");
}

// =================================================================================================
//  STEER_ConvertStringToUnsigned32BitInteger_Test
// =================================================================================================
void STEER_ConvertStringToUnsigned32BitInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToUnsigned32BitInteger_Test");
}

// =================================================================================================
//  STEER_ConvertStringToUnsigned64BitInteger_Test
// =================================================================================================
void STEER_ConvertStringToUnsigned64BitInteger_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertStringToUnsigned64BitInteger_Test");
}

// =================================================================================================
//  STEER_ValidateTestInfo_Test
// =================================================================================================
void STEER_ValidateTestInfo_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ValidateTestInfo_Test");
}

// =================================================================================================
//  STEER_GetTestInfoItemList_Test
// =================================================================================================
void STEER_GetTestInfoItemList_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetTestInfoItemList_Test");
}

// =================================================================================================
//  STEER_FreeTestInfoItemList_Test
// =================================================================================================
void STEER_FreeTestInfoItemList_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeTestInfoItemList_Test");
}

// =================================================================================================
//  STEER_JsonToTestInfo_Test
// =================================================================================================
void STEER_JsonToTestInfo_Test (void)
{
    CU_FAIL("TODO: Implement STEER_JsonToTestInfo_Test");
}

// =================================================================================================
//  STEER_FreeTestInfo_Test
// =================================================================================================
void STEER_FreeTestInfo_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    // Test with NULL test info
    result = STEER_FreeTestInfo(NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
}

// =================================================================================================
//  STEER_HandleConductorCmd_Test
// =================================================================================================
void STEER_HandleConductorCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleConductorCmd_Test");
}

// =================================================================================================
//  STEER_HandleEntropyFilePathCmd_Test
// =================================================================================================
void STEER_HandleEntropyFilePathCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleEntropyFilePathCmd_Test");
}

// =================================================================================================
//  STEER_HandleHelpCmd_Test
// =================================================================================================
void STEER_HandleHelpCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleHelpCmd_Test");
}

// =================================================================================================
//  STEER_HandleNotesCmd_Test
// =================================================================================================
void STEER_HandleNotesCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleNotesCmd_Test");
}

// =================================================================================================
//  STEER_HandleParametersCmd_Test
// =================================================================================================
void STEER_HandleParametersCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleParametersCmd_Test");
}

// =================================================================================================
//  STEER_HandleParametersFilePathCmd_Test
// =================================================================================================
void STEER_HandleParametersFilePathCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleParametersFilePathCmd_Test");
}

// =================================================================================================
//  STEER_HandleParametersInfoCmd_Test
// =================================================================================================
void STEER_HandleParametersInfoCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleParametersInfoCmd_Test");
}

// =================================================================================================
//  STEER_HandleReportFilePathCmd_Test
// =================================================================================================
void STEER_HandleReportFilePathCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleReportFilePathCmd_Test");
}

// =================================================================================================
//  STEER_HandleReportLevelCmd_Test
// =================================================================================================
void STEER_HandleReportLevelCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleReportLevelCmd_Test");
}

// =================================================================================================
//  STEER_HandleReportProgressCmd_Test
// =================================================================================================
void STEER_HandleReportProgressCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleReportProgressCmd_Test");
}

// =================================================================================================
//  STEER_HandleScheduleIdCmd_Test
// =================================================================================================
void STEER_HandleScheduleIdCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleScheduleIdCmd_Test");
}

// =================================================================================================
//  STEER_HandleTestInfoCmd_Test
// =================================================================================================
void STEER_HandleTestInfoCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleTestInfoCmd_Test");
}

// =================================================================================================
//  STEER_HandleVerboseCmd_Test
// =================================================================================================
void STEER_HandleVerboseCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleVerboseCmd_Test");
}

// =================================================================================================
//  STEER_HandleVersionCmd_Test
// =================================================================================================
void STEER_HandleVersionCmd_Test (void)
{
    CU_FAIL("TODO: Implement STEER_HandleVersionCmd_Test");
}

// =================================================================================================
//  STEER_ParseCommandLineArguments_Test
// =================================================================================================
void STEER_ParseCommandLineArguments_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ParseCommandLineArguments_Test");
}

// =================================================================================================
//  STEER_OpenDataSource_Test
// =================================================================================================
void STEER_OpenDataSource_Test (void)
{
    CU_FAIL("TODO: Implement STEER_OpenDataSource_Test");
}

// =================================================================================================
//  STEER_ReadFromDataSource_Test
// =================================================================================================
void STEER_ReadFromDataSource_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ReadFromDataSource_Test");
}

// =================================================================================================
//  STEER_CloseDataSource_Test
// =================================================================================================
void STEER_CloseDataSource_Test (void)
{
    CU_FAIL("TODO: Implement STEER_CloseDataSource_Test");
}

// =================================================================================================
//  STEER_Initialize_Test
// =================================================================================================
void STEER_Initialize_Test (void)
{
    CU_FAIL("TODO: Implement STEER_Initialize_Test");
}

// =================================================================================================
//  STEER_Terminate_Test
// =================================================================================================
void STEER_Terminate_Test (void)
{
    CU_FAIL("TODO: Implement STEER_Terminate_Test");
}

// =================================================================================================
//  STEER_EvaluateData_Test
// =================================================================================================
void STEER_EvaluateData_Test (void)
{
    CU_FAIL("TODO: Implement STEER_EvaluateData_Test");
}

// =================================================================================================
//  STEER_ConvertHexToAscii_Test
// =================================================================================================
void STEER_ConvertHexToAscii_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertHexToAscii_Test");
}

// =================================================================================================
//  STEER_ConvertBytes_Test
// =================================================================================================
void STEER_ConvertBytes_Test (void)
{
    CU_FAIL("TODO: Implement STEER_ConvertBytes_Test");
}

// =================================================================================================
//  STEER_GetRfc3339Timestamp_Test
// =================================================================================================
void STEER_GetRfc3339Timestamp_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    // Test with NULL timestamp
    char* tsStr = NULL;
    result = STEER_GetRfc3339Timestamp(NULL, &tsStr);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL string pointer
    struct timeval ts;
    memset((void*)&ts, 0, sizeof(struct timeval));
    gettimeofday(&ts, NULL);
    result = STEER_GetRfc3339Timestamp(&ts, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_GetRfc3339Timestamp(&ts, &tsStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT(tsStr != NULL);
    STEER_FreeMemory((void**)&tsStr);
}

// =================================================================================================
//  STEER_GetRfc3339Duration_Test
// =================================================================================================
void STEER_GetRfc3339Duration_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    // Test with NULL start time
    struct timeval startTS;
    struct timeval stopTS;
    char* durationStr = NULL;
    gettimeofday(&startTS, NULL);
    result = STEER_GetRfc3339Duration(NULL, &startTS, &durationStr);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL stop time
    result = STEER_GetRfc3339Duration(&startTS, NULL, &durationStr);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL string pointer
    result = STEER_GetRfc3339Duration(&startTS, &startTS, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with start and stop times equal
    result = STEER_GetRfc3339Duration(&startTS, &startTS, &durationStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT(durationStr != NULL);
    STEER_FreeMemory((void**)&durationStr);

    // Test with stop time earlier than start time
    gettimeofday(&stopTS, NULL);
    result = STEER_GetRfc3339Duration(&stopTS, &startTS, &durationStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT(durationStr != NULL);
    STEER_FreeMemory((void**)&durationStr);

    // Test with normal arguments
    result = STEER_GetRfc3339Duration(&startTS, &stopTS, &durationStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT(durationStr != NULL);
    STEER_FreeMemory((void**)&durationStr);
}

// =================================================================================================
//  STEER_AddChildValue_Test
// =================================================================================================
void STEER_AddChildValue_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddChildValue_Test");
}

// =================================================================================================
//  STEER_AddChildValueSet_Test
// =================================================================================================
void STEER_AddChildValueSet_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddChildValueSet_Test");
}

// =================================================================================================
//  STEER_GetChildValue_Test
// =================================================================================================
void STEER_GetChildValue_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetChildValue_Test");
}

// =================================================================================================
//  STEER_GetChildValueSet_Test
// =================================================================================================
void STEER_GetChildValueSet_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetChildValueSet_Test");
}

// =================================================================================================
//  STEER_FreeValues_Test
// =================================================================================================
void STEER_FreeValues_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeValues_Test");
}

// =================================================================================================
//  STEER_FreeValueSets_Test
// =================================================================================================
void STEER_FreeValueSets_Test (void)
{
    CU_FAIL("TODO: Implement STEER_FreeValueSets_Test");
}

// =================================================================================================
//  STEER_AddValuesToArray_Test
// =================================================================================================
void STEER_AddValuesToArray_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddValuesToArray_Test");
}

// =================================================================================================
//  STEER_AddValueSetsToArray_Test
// =================================================================================================
void STEER_AddValueSetsToArray_Test (void)
{
    CU_FAIL("TODO: Implement STEER_AddValueSetsToArray_Test");
}

// =================================================================================================
//  STEER_GetValuesFromArray_Test
// =================================================================================================
void STEER_GetValuesFromArray_Test (void)
{
    CU_FAIL("TODO: Implement STEER_GetValuesFromArray_Test");
}

// =================================================================================================
//  main
// =================================================================================================
int main (int argc, const char * argv[])
{
    // Initialize CUnit
    CU_ErrorCode result = CU_initialize_registry();
    if (result == CUE_SUCCESS)
    {
        // Set up file system utilities (private) test suite
        CU_pSuite fileSystemUtilitiesPrivateTestSuite = CU_add_suite("steer_file_system_utilities_private test suite",
                                                                     STEER_LibPrivate_SuiteInit,
                                                                     STEER_LibPrivate_SuiteCleanup);
        if (fileSystemUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_PathHasFileExtension_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_IsFilePath_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_IsFileEmpty_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_ExpandTildeInPath_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_IsDirectoryPath_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_DirectoryExists_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_CreateDirectory_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_DirectoryHasTrailingSlash_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_TrimFileExtensionFromPath_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_ValidateFileString_Test);
            CU_ADD_TEST(fileSystemUtilitiesPrivateTestSuite, STEER_ValidatePathString_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up JSON utilities (private) test suite
        CU_pSuite jsonUtilitiesPrivateTestSuite = CU_add_suite("steer_json_utilities_private test suite",
                                                               STEER_LibPrivate_SuiteInit,
                                                               STEER_LibPrivate_SuiteCleanup);
        if (jsonUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(jsonUtilitiesPrivateTestSuite, STEER_GetObjectStringValue_Test);
            CU_ADD_TEST(jsonUtilitiesPrivateTestSuite, STEER_GetChildObject_Test);
            CU_ADD_TEST(jsonUtilitiesPrivateTestSuite, STEER_GetChildObjectBoolean_Test);
            CU_ADD_TEST(jsonUtilitiesPrivateTestSuite, STEER_AddChildObjectString_Test);
            CU_ADD_TEST(jsonUtilitiesPrivateTestSuite, STEER_AddChildObjectBoolean_Test);
            CU_ADD_TEST(jsonUtilitiesPrivateTestSuite, STEER_AddEmptyNamedChildArray_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up NIST STS utilities (private) test suite
        CU_pSuite nistStsUtilitiesPrivateTestSuite = CU_add_suite("steer_nist_sts_utilities_private test suite",
                                                                  STEER_LibPrivate_SuiteInit,
                                                                  STEER_LibPrivate_SuiteCleanup);
        if (nistStsUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(nistStsUtilitiesPrivateTestSuite, STEER_NistStsAddRequiredMetricsToConfiguration_Test);
            CU_ADD_TEST(nistStsUtilitiesPrivateTestSuite, STEER_NistStsAddMetricsToConfiguration_Test);
            CU_ADD_TEST(nistStsUtilitiesPrivateTestSuite, STEER_NistStsAddRequiredCriterionToConfiguration_Test);
            CU_ADD_TEST(nistStsUtilitiesPrivateTestSuite, STEER_NistStsAddCriteriaToConfiguration_Test);
            CU_ADD_TEST(nistStsUtilitiesPrivateTestSuite, STEER_NistStsGetProbabilityValueFromTest_Test);
            CU_ADD_TEST(nistStsUtilitiesPrivateTestSuite, STEER_NistStsGetProbabilityValuesFromConfiguration_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up parameter set utilities (private) test suite
        CU_pSuite parameterSetUtilitiesPrivateTestSuite = CU_add_suite("steer_parameter_set_utilities_private test suite",
                                                                       STEER_LibPrivate_SuiteInit,
                                                                       STEER_LibPrivate_SuiteCleanup);
        if (parameterSetUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(parameterSetUtilitiesPrivateTestSuite, STEER_ValidateParameterSet_Test);
            CU_ADD_TEST(parameterSetUtilitiesPrivateTestSuite, STEER_GetNextParameter_Test);
            CU_ADD_TEST(parameterSetUtilitiesPrivateTestSuite, STEER_DefaultParameterCheck_Test);
            CU_ADD_TEST(parameterSetUtilitiesPrivateTestSuite, STEER_JsonToParameterSet_Test);
            CU_ADD_TEST(parameterSetUtilitiesPrivateTestSuite, STEER_ParameterSetToJson_Test);
            CU_ADD_TEST(parameterSetUtilitiesPrivateTestSuite, STEER_FreeParameterSet_Test);
            CU_ADD_TEST(parameterSetUtilitiesPrivateTestSuite, STEER_GetParameterValue_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up parameters info utilities (private) test suite
        CU_pSuite parametersInfoUtilitiesPrivateTestSuite = CU_add_suite("steer_parameters_info_utilities_private test suite",
                                                                         STEER_LibPrivate_SuiteInit,
                                                                         STEER_LibPrivate_SuiteCleanup);
        if (parametersInfoUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(parametersInfoUtilitiesPrivateTestSuite, STEER_ValidateParametersInfo_Test);
            CU_ADD_TEST(parametersInfoUtilitiesPrivateTestSuite, STEER_GetNextParameterInfo_Test);
            CU_ADD_TEST(parametersInfoUtilitiesPrivateTestSuite, STEER_AddChildParameterInfo_Test);
            CU_ADD_TEST(parametersInfoUtilitiesPrivateTestSuite, STEER_JsonToParametersInfo_Test);
            CU_ADD_TEST(parametersInfoUtilitiesPrivateTestSuite, STEER_FreeParametersInfo_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up report utilities (private) test suite
        CU_pSuite reportUtilitiesPrivateTestSuite = CU_add_suite("steer_report_utilities_private test suite",
                                                                 STEER_LibPrivate_SuiteInit,
                                                                 STEER_LibPrivate_SuiteCleanup);
        if (reportUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_CompareDoubles_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_FreeCriteria_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_FreeTest_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_FreeTests_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_FreeConfigurations_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_NewEmptyReport_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_NewReport_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_AddConfigurationToReport_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_AddTestToConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_AddMetricSetToConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetParameterFromReport_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetMetricFromConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetMetricSetFromConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_ValidateReport_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetReportHeaderFromReportObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetParametersFromReportObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetConfigurationsFromReportObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetTestsFromConfigurationObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetCriteriaFromParentObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_GetEvaluationFromParentObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_NewReportObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_AddParametersToReportObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_AddConfigurationsToReportObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_AddTestsToTestsArray_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_AddCriteriaToCriteriaArray_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_AddEvaluationToParentObject_Test);
            CU_ADD_TEST(reportUtilitiesPrivateTestSuite, STEER_ReportToJson_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up schedule utilities (private) test suite
        CU_pSuite scheduleUtilitiesPrivateTestSuite = CU_add_suite("steer_schedule_utilities_private test suite",
                                                                   STEER_LibPrivate_SuiteInit,
                                                                   STEER_LibPrivate_SuiteCleanup);
        if (scheduleUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(scheduleUtilitiesPrivateTestSuite, STEER_ValidateSchedule_Test);
            CU_ADD_TEST(scheduleUtilitiesPrivateTestSuite, STEER_GetScheduledTests_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up string utilities (private) test suite
        CU_pSuite stringUtilitiesPrivateTestSuite = CU_add_suite("steer_string_utilities_private test suite",
                                                                 STEER_LibPrivate_SuiteInit,
                                                                 STEER_LibPrivate_SuiteCleanup);
        if (stringUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_StringIsPositiveInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_StringIsNegativeInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToBoolean_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToDoublePrecisionFloatingPoint_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToExtendedPrecisionFloatingPoint_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToSigned8BitInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToSigned16BitInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToSigned32BitInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToSigned64BitInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToSinglePrecisionFloatingPoint_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToUnsigned8BitInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToUnsigned16BitInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToUnsigned32BitInteger_Test);
            CU_ADD_TEST(stringUtilitiesPrivateTestSuite, STEER_ConvertStringToUnsigned64BitInteger_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up test info utilities (private) test suite
        CU_pSuite testInfoUtilitiesPrivateTestSuite = CU_add_suite("steer_test_info_utilities_private test suite",
                                                                   STEER_LibPrivate_SuiteInit,
                                                                   STEER_LibPrivate_SuiteCleanup);
        if (testInfoUtilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(testInfoUtilitiesPrivateTestSuite, STEER_ValidateTestInfo_Test);
            CU_ADD_TEST(testInfoUtilitiesPrivateTestSuite, STEER_GetTestInfoItemList_Test);
            CU_ADD_TEST(testInfoUtilitiesPrivateTestSuite, STEER_FreeTestInfoItemList_Test);
            CU_ADD_TEST(testInfoUtilitiesPrivateTestSuite, STEER_JsonToTestInfo_Test);
            CU_ADD_TEST(testInfoUtilitiesPrivateTestSuite, STEER_FreeTestInfo_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up test shell (private) test suite
        CU_pSuite testShellPrivateTestSuite = CU_add_suite("steer_test_shell_private test suite",
                                                           STEER_LibPrivate_SuiteInit,
                                                           STEER_LibPrivate_SuiteCleanup);
        if (testShellPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleConductorCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleEntropyFilePathCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleHelpCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleNotesCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleParametersCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleParametersFilePathCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleParametersInfoCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleReportFilePathCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleReportLevelCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleReportProgressCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleScheduleIdCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleTestInfoCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleVerboseCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_HandleVersionCmd_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_ParseCommandLineArguments_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_OpenDataSource_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_ReadFromDataSource_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_CloseDataSource_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_Initialize_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_Terminate_Test);
            CU_ADD_TEST(testShellPrivateTestSuite, STEER_EvaluateData_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up utilities (private) test suite
        CU_pSuite utilitiesPrivateTestSuite = CU_add_suite("steer_utilities_private test suite",
                                                           STEER_LibPrivate_SuiteInit,
                                                           STEER_LibPrivate_SuiteCleanup);
        if (utilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(utilitiesPrivateTestSuite, STEER_ConvertHexToAscii_Test);
            CU_ADD_TEST(utilitiesPrivateTestSuite, STEER_ConvertBytes_Test);
            CU_ADD_TEST(utilitiesPrivateTestSuite, STEER_GetRfc3339Timestamp_Test);
            CU_ADD_TEST(utilitiesPrivateTestSuite, STEER_GetRfc3339Duration_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up value utilities (private) test suite
        CU_pSuite valueUtilitiesPrivateTestSuite = CU_add_suite("steer_value_utilities_private test suite",
                                                                STEER_LibPrivate_SuiteInit,
                                                                STEER_LibPrivate_SuiteCleanup);
        if (utilitiesPrivateTestSuite != NULL)
        {
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_AddChildValue_Test);
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_AddChildValueSet_Test);
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_GetChildValue_Test);
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_GetChildValueSet_Test);
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_FreeValues_Test);
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_FreeValueSets_Test);
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_AddValuesToArray_Test);
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_AddValueSetsToArray_Test);
            CU_ADD_TEST(valueUtilitiesPrivateTestSuite, STEER_GetValuesFromArray_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Check for success
        if (result == CUE_SUCCESS)
        {
            CU_set_output_filename("./logs/libsteer_private_Unit_Test");
            CU_automated_run_tests();
        }

        // Clean up
        CU_cleanup_registry();
    }
    else    // CU_initialize_registry failed
        printf("\tCU_initialize_registry failed with error code %d!\n", result);
    
    return CU_get_error();
}

// =================================================================================================

