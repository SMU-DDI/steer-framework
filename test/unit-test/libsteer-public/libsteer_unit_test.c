// =================================================================================================
//! @file libsteer_unit_test.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements a unit test of the libsteer public interfaces.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-05
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
#include "steer_json_constants.h"
#include "steer_json_utilities.h"
#include "steer_parameters_info_utilities.h"
#include "steer_report_utilities.h"
#include "steer_schedule_utilities.h"
#include "steer_test_info_utilities.h"
#include "steer_test_shell.h"
#include "steer_utilities.h"
#include "steer_value_utilities.h"

// =================================================================================================
//  Private globals
// =================================================================================================

#define DUMMY_TEST_NAME     "dummy test"
#define DUMMY_PROGRAM_NAME  "dummy_test"
#define NOT_JSON            "This is not JSON."

static const char* kTestJson = "{ \
                                \"test json\": { \
                                \"test tag\": \"test tag value\", \
                                \"test specific\": [ \
{ \
    \"name\": \"test object\" \
} \
    ] \
    } \
    }";

static const char* kDummyBadJsonTagScheduleJson = "{ \
                                                   \"the schedule\": { \
                                                   \"test conductor\": \"Unit tester\", \
                                                   \"test notes\": \"Used for unit testing.\", \
                                                   \"report progress\": false, \
                                                   \"tests\": [ \
                                                        { \
                                                            \"program name\": \"dummy_test\", \
                                                                \"profiles\": [ \
                                                                { \
                                                                    \"profile id\": \"1\", \
                                                                        \"input\": \"/dev/urandom\", \
                                                                        \"parameters\": \"./test/parameters.json\", \
                                                                        \"report\": \"./results/test_report.json\" \
                                                                } \
                                                            ] \
                                                        } \
                                                    ] \
                                                    } \
                                                }";

static const char* kDummyMalformedJsonScheduleJson = "{ \
                                                      \"schedule\": { \
                                                      \"test conductor\": \"Unit tester\", \
                                                      \"test notes\": \"Used for unit testing.\", \
                                                      \"report progress\": false, \
                                                      \"tests\": [ \
                                                        { \
                                                            \"program name\": \"dummy_test\", \
                                                                \"profiles\": [ \
                                                                { \
                                                                    \"profile id\": \"1\", \
                                                                        \"input\": \"/dev/urandom\", \
                                                                        \"parameters\": \"./test/parameters.json\", \
                                                                        \"report\": \"./results/test_report.json\" \
                                                                } \
                                                            ] \
                                                        } \
                                                    } \
                                                    }";

static const char* kDummyScheduleJson = "{ \
                                         \"schedule\": { \
                                         \"test conductor\": \"Unit tester\", \
                                         \"test notes\": \"Used for unit testing.\", \
                                         \"report progress\": false, \
                                         \"tests\": [ \
                                            { \
                                                \"program name\": \"dummy_test\", \
                                                    \"profiles\": [ \
                                                    { \
                                                        \"profile id\": \"1\", \
                                                            \"input\": \"/dev/urandom\", \
                                                            \"parameters\": \"./test/parameters.json\", \
                                                            \"report\": \"./results/test_report.json\" \
                                                    } \
                                                ] \
                                            } \
                                            ] \
                                        } \
                                        }";


static const char* kDummyMalformedJsonReportJson = "{ \
                                                    \"report\": \
{ \
    \"test name\": \"Dummy test\", \
        \"test description\": \"A valid dummy test\", \
        \"report level\": \"summary\", \
        \"program name\": \"dummy\", \
        \"program version\": \"1.0\", \
        \"operating system\": \"linux\", \
        \"architecture\": \"dummy\", \
        \"entropy source\": \"rng\", \
        \"start time\": \"2024-01-01 00:00:00 UTC\", \
        \"test duration\": \"999\", \
        \"parameters\": [ \
        { \
            \"name\": \"dummy\", \
                \"data type\": \"float\", \
                \"value\": \"0\" \
        } \
    ], \
        \"criteria\": [ \
        { \
            \"criterion\": \"foo\", \
                \"result\": \"1\" \
        } \
    ], \
        \"configurations\": [ \
        { \
            \"configuration id\": \"0\", \
                \"evaluation\": \"fail\", \
                \"criteria\": [ \
                { \
                    \"criterion\": \"foo\", \
                        \"result\": true \
                } ]\
            ], \
                \"evaluation\" : \"fail\" \
        } \
}";


static const char* kDummyBadJsonTagReportJson = "{ \
                                                 \"bad report\": \
{ \
    \"test name\": \"Dummy test\", \
        \"test description\": \"A valid dummy test\", \
        \"report level\": \"summary\", \
        \"program name\": \"dummy\", \
        \"program version\": \"1.0\", \
        \"operating system\": \"linux\", \
        \"architecture\": \"dummy\", \
        \"entropy source\": \"rng\", \
        \"start time\": \"2024-01-01 00:00:00 UTC\", \
        \"test duration\": \"999\", \
        \"parameters\": [ \
        { \
            \"name\": \"dummy\", \
                \"data type\": \"float\", \
                \"value\": \"0\" \
        } \
    ], \
        \"criteria\": [ \
        { \
            \"criterion\": \"foo\", \
                \"result\": true \
        } \
    ], \
        \"configurations\": [ \
        { \
            \"configuration id\": \"0\", \
                \"evaluation\": \"fail\", \
                \"criteria\": [ \
                { \
                    \"criterion\": \"foo\", \
                        \"result\": true \
                } ]\
        } \
    ], \
        \"evaluation\" : \"fail\" \
} \
    }";
static const char* kDummyReportJson = "{ \
                                       \"report\": \
{ \
    \"test name\": \"Dummy test\", \
        \"test description\": \"A valid dummy test\", \
        \"report level\": \"summary\", \
        \"program name\": \"dummy\", \
        \"program version\": \"1.0\", \
        \"operating system\": \"linux\", \
        \"architecture\": \"dummy\", \
        \"entropy source\": \"rng\", \
        \"start time\": \"2024-01-01 00:00:00 UTC\", \
        \"test duration\": \"999\", \
        \"parameters\": [ \
        { \
            \"name\": \"dummy\", \
                \"data type\": \"float\", \
                \"value\": \"0\" \
        } \
    ], \
        \"criteria\": [ \
        { \
            \"criterion\": \"foo\", \
                \"result\": true \
        } \
    ], \
        \"configurations\": [ \
        { \
            \"configuration id\": \"0\", \
                \"evaluation\": \"pass\", \
                \"criteria\": [ \
                { \
                    \"criterion\": \"foo\", \
                        \"result\": true \
                } ],\
            \"tests\": [ \
            { \
                \"test id\": \"0\", \
                    \"evaluation\": \"pass\", \
                    \"criteria\": [ \
                    { \
                        \"criterion\": \"foo\", \
                            \"result\": true \
                    } ]\
            } ] \
        } \
    ], \
        \"evaluation\" : \"pass\" \
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
        void** privateData,
        uint64_t* bufferSizeInBytes)
{
    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  DummyGetConfigurationCountFunction
// =================================================================================================
uint32_t DummyGetConfigurationCountFunction (void* testPrivateData)
{
    return 1;
}

// =================================================================================================
//  DummySetReportFunction
// =================================================================================================
int32_t DummySetReportFunction (void* testPrivateData,
        tSTEER_ReportPtr report)
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
//  STEER_Lib_SuiteInit
// =================================================================================================
int STEER_Lib_SuiteInit (void)
{
    return CUE_SUCCESS;
}

// =================================================================================================
//  STEER_Lib_SuiteCleanup
// =================================================================================================
int STEER_Lib_SuiteCleanup (void)
{
    return CUE_SUCCESS;
}

// =================================================================================================
//  STEER_FileExists_Test
// =================================================================================================
void STEER_FileExists_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL path
    CU_ASSERT_FALSE(STEER_FileExists(NULL));

    // Test with empty path
    CU_ASSERT_FALSE(STEER_FileExists(""));

    // Test with non-existent file
    CU_ASSERT_FALSE(STEER_FileExists("/foo/test.txt"));

    // Test with existing empty file
    result = CreateTestFile("/tmp/test.txt", NULL);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_TRUE(STEER_FileExists("/tmp/test.txt"));

    // Test with existing non-empty file
    result = CreateTestFile("/tmp/test.txt", "This is a test file.");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_TRUE(STEER_FileExists("/tmp/test.txt"));
}

// =================================================================================================
//  STEER_FileSize_Test
// =================================================================================================
void STEER_FileSize_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL path
    size_t fileSize = 0;
    result = STEER_FileSize(NULL, &fileSize);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(fileSize, 0);

    // Test with empty path
    result = STEER_FileSize("", &fileSize);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_EQUAL(fileSize, 0);

    // Test with NULL size
    result = STEER_FileSize("/tmp/test.txt", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with non-existent file
    result = STEER_FileSize("/foo/test.txt", &fileSize);
    CU_ASSERT_EQUAL(result, ENOENT);
    CU_ASSERT_EQUAL(fileSize, 0);

    // Test with existing empty file
    result = CreateTestFile("/tmp/test.txt", NULL);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_FileSize("/tmp/test.txt", &fileSize);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(fileSize, 0);

    // Test with existing non-empty file
    result = CreateTestFile("/tmp/test.txt", "This is a test file.");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_FileSize("/tmp/test.txt", &fileSize);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(fileSize, 20);
}

// =================================================================================================
//  STEER_IsReadableDevicePath_Test
// =================================================================================================
void STEER_IsReadableDevicePath_Test (void)
{
    // Test with NULL path
    CU_ASSERT_FALSE(STEER_IsReadableDevicePath(NULL));

    // Test with empty path
    CU_ASSERT_FALSE(STEER_IsReadableDevicePath(""));

    // Test with non-existent device
    CU_ASSERT_FALSE(STEER_IsReadableDevicePath("/dev/foo"));

    // Test with existing device
    CU_ASSERT_TRUE(STEER_IsReadableDevicePath("/dev/urandom"));
}

// =================================================================================================
//  STEER_OpenFile_Test
// =================================================================================================
void STEER_OpenFile_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL file path
    FILE* fileRef = NULL;
    result = STEER_OpenFile(NULL, false, false, &fileRef);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(fileRef);

    // Test with empty path
    result = STEER_OpenFile("", false, false, &fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(fileRef);

    // Test with NULL file reference pointer
    result = CreateTestFile("/tmp/test.txt", "This is a test file.");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_OpenFile("/tmp/test.txt", false, false, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments (r)
    result = STEER_OpenFile("/tmp/test.txt", false, false, &fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(fileRef);
    result = STEER_CloseFile(&fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(fileRef);

    // Test with valid arguments (w)
    result = STEER_OpenFile("/tmp/test.txt", true, false, &fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(fileRef);
    result = STEER_CloseFile(&fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(fileRef);

    // Test with valid arguments (rb)
    result = STEER_OpenFile("/tmp/test.txt", false, true, &fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(fileRef);
    result = STEER_CloseFile(&fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(fileRef);

    // Test with valid arguments (wb)
    result = STEER_OpenFile("/tmp/test.txt", true, true, &fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(fileRef);
    result = STEER_CloseFile(&fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(fileRef);
}

// =================================================================================================
//  STEER_CloseFile_Test
// =================================================================================================
void STEER_CloseFile_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL file reference pointer
    result = STEER_CloseFile(NULL);
    CU_ASSERT_EQUAL(result, EFAULT);  

    // Test with valid argument
    FILE* fileRef = NULL;
    result = CreateTestFile("/tmp/test.txt", "This is a test file.");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_OpenFile("/tmp/test.txt", false, false, &fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(fileRef);
    result = STEER_CloseFile(&fileRef);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(fileRef);
}

// =================================================================================================
//  STEER_WriteTextFile_Test
// =================================================================================================
void STEER_WriteTextFile_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL file path
    char * buff= "test string";
    char * contents = NULL;
    result = STEER_WriteTextFile(NULL, buff);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty file path
    result = STEER_WriteTextFile("", buff);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(contents);

    // Test with empty contents
    result = STEER_WriteTextFile("/tmp/test.txt", "");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(contents);
    
    // Test with NULL contents
    result = STEER_WriteTextFile("/tmp/test.txt", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
    
    // Test with non-existent file path
    result = STEER_WriteTextFile("/foo/test.txt", buff);
    CU_ASSERT_EQUAL(result, ENOENT);
    CU_ASSERT_PTR_NULL(contents);

    // Test with valid arguments
    result = STEER_WriteTextFile("/tmp/test.txt", buff);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_ReadTextFile("/tmp/test.txt", (uint8_t **) &contents);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(strcmp(contents, buff), 0);
    STEER_FreeMemory((void**)&contents);
    CU_ASSERT_PTR_NULL(contents);
}

// =================================================================================================
//  STEER_ReadTextFile_Test
// =================================================================================================
void STEER_ReadTextFile_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL file path
    uint8_t* contents = NULL;
    result = STEER_ReadTextFile(NULL, &contents);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(contents);

    // Test with empty file path
    result = STEER_ReadTextFile("", &contents);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(contents);

    // Test with non-existent file path
    result = STEER_ReadTextFile("/foo/test.txt", &contents);
    CU_ASSERT_EQUAL(result, ENOENT);
    CU_ASSERT_PTR_NULL(contents);

    // Test with NULL buffer
    result = STEER_ReadTextFile("/tmp/test.txt", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = CreateTestFile("/tmp/test.txt", "This is a test file.");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_ReadTextFile("/tmp/test.txt", &contents);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(contents);
    STEER_FreeMemory((void**)&contents);
    CU_ASSERT_PTR_NULL(contents);
}

// =================================================================================================
//  STEER_ScanDirectoryForFilesWithExtension_Test
// =================================================================================================
void STEER_ScanDirectoryForFilesWithExtension_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL directory
    uint32_t count = 0;
    tSTEER_FilePath* paths = NULL;
    result = STEER_ScanDirectoryForFilesWithExtension(NULL, ".txt", &count, &paths);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(count, 0);
    CU_ASSERT_PTR_NULL(paths);

    // Test with empty directory
    result = STEER_ScanDirectoryForFilesWithExtension("", ".txt", &count, &paths);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_EQUAL(count, 0);
    CU_ASSERT_PTR_NULL(paths);

    // Test with NULL extension
    result = STEER_ScanDirectoryForFilesWithExtension("/tmp", NULL, &count, &paths);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(count, 0);
    CU_ASSERT_PTR_NULL(paths);

    // Test with empty extension
    result = STEER_ScanDirectoryForFilesWithExtension("/tmp", "", &count, &paths);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_EQUAL(count, 0);
    CU_ASSERT_PTR_NULL(paths);

    // Test with NULL count
    result = STEER_ScanDirectoryForFilesWithExtension("/tmp", ".txt", NULL, &paths);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(paths);

    // Test with NULL paths
    result = STEER_ScanDirectoryForFilesWithExtension("/tmp", ".txt", &count, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(count, 0);

    // Test with valid arguments
    result = CreateTestFile("/tmp/test.blargh", "This is a test file.");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_ScanDirectoryForFilesWithExtension("/tmp", ".blargh", &count, &paths);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_PTR_NOT_NULL(paths);
    STEER_FreeMemory((void**)&paths);
    CU_ASSERT_PTR_NULL(paths);
}

// =================================================================================================
//  STEER_GetProgramDirectory_Test
// =================================================================================================
void STEER_GetProgramDirectory_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL path
    result = STEER_GetProgramDirectory(NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid argument
    char* path = NULL;
    result = STEER_GetProgramDirectory(&path);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(path);
    STEER_FreeMemory((void**)&path);
    CU_ASSERT_PTR_NULL(path);
}

// =================================================================================================
//  STEER_ScanProgramDirectory_Test
// =================================================================================================
void STEER_ScanProgramDirectory_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL directory
    uint32_t count = 0;
    tSTEER_FileName* programs = NULL;
    result = STEER_ScanProgramDirectory(NULL, &count, &programs);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(count, 0);
    CU_ASSERT_PTR_NULL(programs);

    // Test with empty directory
    result = STEER_ScanProgramDirectory("", &count, &programs);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_EQUAL(count, 0);
    CU_ASSERT_PTR_NULL(programs);

    // Test with NULL count
    result = STEER_ScanProgramDirectory("/tmp", NULL, &programs);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(programs);

    // Test with NULL program names
    result = STEER_ScanProgramDirectory("/tmp", &count, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(count, 0);

    // Test with valid arguments
    char* path = NULL;
    result = STEER_GetProgramDirectory(&path);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(path);
    result = STEER_ScanProgramDirectory(path, &count, &programs);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT(count > 0);
    CU_ASSERT_PTR_NOT_NULL(programs);
    STEER_FreeMemory((void**)&path);
    CU_ASSERT_PTR_NULL(path);
    STEER_FreeMemory((void**)&programs);
    CU_ASSERT_PTR_NULL(programs);
}

// =================================================================================================
//  STEER_ProgramAvailable_Test
// =================================================================================================
void STEER_ProgramAvailable_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    char* path = NULL;
    uint32_t count = 0;
    tSTEER_FileName* programs = NULL;

    result = STEER_GetProgramDirectory(&path);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(path);
    result = STEER_ScanProgramDirectory(path, &count, &programs);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT(count > 0);
    CU_ASSERT_PTR_NOT_NULL(programs);

    // Test with NULL program name
    CU_ASSERT_FALSE(STEER_ProgramAvailable(NULL, count, programs));

    // Test with empty program name
    CU_ASSERT_FALSE(STEER_ProgramAvailable("", count, programs));

    // Test with zero count
    CU_ASSERT_FALSE(STEER_ProgramAvailable("test", 0, programs));

    // Test with NULL program names
    CU_ASSERT_FALSE(STEER_ProgramAvailable("test", count, NULL));

    // Test with NULL flag
    CU_ASSERT_FALSE(STEER_ProgramAvailable("test", count, programs));

    // Test with valid arguments
    CU_ASSERT_TRUE(STEER_ProgramAvailable("libsteer_unit_test", count, programs));
    STEER_FreeMemory((void**)&path);
    CU_ASSERT_PTR_NULL(path);
    STEER_FreeMemory((void**)&programs);
    CU_ASSERT_PTR_NULL(programs);
}

// =================================================================================================
//  STEER_ReadJsonFromFile_Test
// =================================================================================================
void STEER_ReadJsonFromFile_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL file path
    cJSON* obj = NULL;
    result = STEER_ReadJsonFromFile(NULL, &obj);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(obj);

    // Test with empty file path
    result = STEER_ReadJsonFromFile("", &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(obj);

    // Test with non-existent file path
    result = STEER_ReadJsonFromFile("/tmp/foo", &obj);
    CU_ASSERT_EQUAL(result, ENOENT);
    CU_ASSERT_PTR_NULL(obj);

    // Test with NULL JSON object pointer
    result = STEER_ReadJsonFromFile("/tmp/foo", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with file that doesn't contain JSON
    result = CreateTestFile("/tmp/test.json", NOT_JSON);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_ReadJsonFromFile("/tmp/test.json", &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_PARSE_FAILURE);
    CU_ASSERT_PTR_NULL(obj);

    // Test with valid arguments
    result = CreateTestFile("/tmp/test.json", kTestJson);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_ReadJsonFromFile("/tmp/test.json", &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(obj);
    if (obj != NULL)
    {
        (void)cJSON_Delete(obj);
        obj = NULL;
    }
}

// =================================================================================================
//  STEER_WriteJsonToFile_Test
// =================================================================================================
void STEER_WriteJsonToFile_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    cJSON* obj = NULL;
    result = CreateTestFile("/tmp/test.json", kTestJson);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_ReadJsonFromFile("/tmp/test.json", &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(obj);

    // Test with NULL JSON object pointer
    result = STEER_WriteJsonToFile(NULL, "/tmp/test.json");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL file path
    result = STEER_WriteJsonToFile(obj, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty file path
    result = STEER_WriteJsonToFile(obj, "");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with non-existent file
    if (STEER_FileExists("/tmp/test.json"))
    {
        result = (int32_t)remove("/tmp/test.json");
        CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    }
    result = STEER_WriteJsonToFile(obj, "/tmp/test.json");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with overwriting an existing file
    result = STEER_WriteJsonToFile(obj, "/tmp/test.json");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    if (obj != NULL)
    {
        (void)cJSON_Delete(obj);
        obj = NULL;
    }
}

// =================================================================================================
//  STEER_ParseJsonString_Test
// =================================================================================================
void STEER_ParseJsonString_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL JSON string
    cJSON* obj = NULL;
    result = STEER_ParseJsonString(NULL, &obj);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(obj);

    // Test with empty JSON string
    result = STEER_ParseJsonString("", &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(obj);

    // Test with not JSON
    result = STEER_ParseJsonString(NOT_JSON, &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_PARSE_FAILURE);
    CU_ASSERT_PTR_NULL(obj);

    // Test with malformed JSON
    result = STEER_ParseJsonString(kDummyMalformedJsonScheduleJson, &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_PARSE_FAILURE);
    CU_ASSERT_PTR_NULL(obj);

    // Test with NULL JSON object pointer
    result = STEER_ParseJsonString(kTestJson, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_ParseJsonString(kTestJson, &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(obj);
    if (obj != NULL)
    {
        (void)cJSON_Delete(obj);
        obj = NULL;
    }
}

// =================================================================================================
//  STEER_HasChildTag_Test
// =================================================================================================
void STEER_HasChildTag_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    cJSON* obj = NULL;
    result = STEER_ParseJsonString(kTestJson, &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(obj);

    // Test with NULL JSON object
    CU_ASSERT_FALSE(STEER_HasChildTag(NULL, "test tag"));

    // Test with NULL tag
    CU_ASSERT_FALSE(STEER_HasChildTag(obj, NULL));

    // Test with empty tag
    CU_ASSERT_FALSE(STEER_HasChildTag(obj, ""));

    // Test with non-existent tag
    CU_ASSERT_FALSE(STEER_HasChildTag(obj, "bad tag"));

    // Test with valid arguments
    CU_ASSERT_TRUE(STEER_HasChildTag(obj, "test json"));
    if (obj != NULL)
    {
        (void)cJSON_Delete(obj);
        obj = NULL;
    }
}

// =================================================================================================
//  STEER_GetChildObjectString_Test
// =================================================================================================
void STEER_GetChildObjectString_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    cJSON* obj = NULL;
    result = STEER_ParseJsonString(kTestJson, &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(obj);

    // Test with NULL JSON object
    char* str = NULL;
    result = STEER_GetChildObjectString(NULL, "test json", &str);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(str);

    // Test with NULL tag
    result = STEER_GetChildObjectString(obj, NULL, &str);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(str);

    // Test with empty tag
    result = STEER_GetChildObjectString(obj, "", &str);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(str);

    // Test with non-existent tag
    result = STEER_GetChildObjectString(obj, "bad tag", &str);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_TAG_NOT_FOUND);
    CU_ASSERT_PTR_NULL(str);

    // Test with tag that has no string
    result = STEER_GetChildObjectString(obj, "test json", &str);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(str);

    // Test with NULL string
    result = STEER_GetChildObjectString(obj, "test json", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid tag
    cJSON* childObj = NULL;
    childObj = cJSON_GetObjectItem(obj, "test json");
    CU_ASSERT_PTR_NOT_NULL(childObj);
    result = STEER_GetChildObjectString(childObj, "test tag", &str);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(str);
    if (str != NULL)
        CU_ASSERT_EQUAL(strcmp("test tag value", str), 0);
    if (obj != NULL)
    {
        (void)cJSON_Delete(obj);
        obj = NULL;
    }
    STEER_FreeMemory((void**)&str);
    CU_ASSERT_PTR_NULL(str);
}

// =================================================================================================
//  STEER_GetChildArray_Test
// =================================================================================================
void STEER_GetChildArray_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    cJSON* obj = NULL;
    cJSON* childObj = NULL;
    result = STEER_ParseJsonString(kTestJson, &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(obj);
    childObj = cJSON_GetObjectItem(obj, "test json");
    CU_ASSERT_PTR_NOT_NULL(childObj);

    // Test with NULL JSON object
    cJSON* arrayObj = NULL;
    uint32_t arraySize = 0;
    result = STEER_GetChildArray(NULL, "test specific", &arrayObj, &arraySize);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(arrayObj);
    CU_ASSERT_EQUAL(arraySize, 0);

    // Test with NULL tag
    result = STEER_GetChildArray(childObj, NULL, &arrayObj, &arraySize);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(arrayObj);
    CU_ASSERT_EQUAL(arraySize, 0);

    // Test with empty tag
    result = STEER_GetChildArray(childObj, "", &arrayObj, &arraySize);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(arrayObj);
    CU_ASSERT_EQUAL(arraySize, 0);

    // Test with non-existent tag
    result = STEER_GetChildArray(childObj, "bad tag", &arrayObj, &arraySize);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_TAG_NOT_FOUND);
    CU_ASSERT_PTR_NULL(arrayObj);
    CU_ASSERT_EQUAL(arraySize, 0);

    // Test with NULL array pointer
    result = STEER_GetChildArray(childObj, "test specific", NULL, &arraySize);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(arraySize, 0);

    // Test with NULL array size
    result = STEER_GetChildArray(childObj, "test specific", &arrayObj, NULL);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(arrayObj);

    // Test with valid tag
    result = STEER_GetChildArray(childObj, "test specific", &arrayObj, &arraySize);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(arrayObj);
    CU_ASSERT_EQUAL(arraySize, 1);
    if (obj != NULL)
    {
        (void)cJSON_Delete(obj);
        obj = NULL;
    }
}

// =================================================================================================
//  STEER_GetArrayItemIndexWithName_Test
// =================================================================================================
void STEER_GetArrayItemIndexWithName_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    cJSON* obj = NULL;
    cJSON* childObj = NULL;
    cJSON* arrayObj = NULL;
    uint32_t arraySize = 0;
    result = STEER_ParseJsonString(kTestJson, &obj);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(obj);
    childObj = cJSON_GetObjectItem(obj, "test json");
    CU_ASSERT_PTR_NOT_NULL(childObj);
    result = STEER_GetChildArray(childObj, "test specific", &arrayObj, &arraySize);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(arrayObj);
    CU_ASSERT_EQUAL(arraySize, 1);

    // Test with NULL array
    uint32_t index = 0;
    result = STEER_GetArrayItemIndexWithName(NULL, "test object", &index);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(index, 0);

    // Test with NULL name
    result = STEER_GetArrayItemIndexWithName(arrayObj, NULL, &index);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(index, 0);

    // Test with empty name
    result = STEER_GetArrayItemIndexWithName(arrayObj, "", &index);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_EQUAL(index, 0);

    // Test with non-existent name
    result = STEER_GetArrayItemIndexWithName(arrayObj, "bad name", &index);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_TAG_NOT_FOUND);
    CU_ASSERT_EQUAL(index, 0);

    // Test with NULL index
    result = STEER_GetArrayItemIndexWithName(arrayObj, "test object", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid name
    result = STEER_GetArrayItemIndexWithName(arrayObj, "test object", &index);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(index, 0);
    if (obj != NULL)
    {
        (void)cJSON_Delete(obj);
        obj = NULL;
    }
}

// =================================================================================================
//  STEER_ParametersInfoToJson_Test
// =================================================================================================
void STEER_ParametersInfoToJson_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL parameters info
    char* parametersInfoJson = NULL;
    result = STEER_ParametersInfoToJson(NULL, &parametersInfoJson);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(parametersInfoJson);

    // Test with NULL JSON
    result = STEER_ParametersInfoToJson(&gDummyParametersInfo, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_ParametersInfoToJson(&gDummyParametersInfo, &parametersInfoJson);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(parametersInfoJson);
    STEER_FreeMemory((void**)&parametersInfoJson);
    CU_ASSERT_PTR_NULL(parametersInfoJson);
}

// =================================================================================================
//  STEER_FreeReport_Test
// =================================================================================================
void STEER_FreeReport_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL argument
    result = STEER_FreeReport(NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL pointer
    tSTEER_ReportPtr report = NULL;
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
}

// =================================================================================================
//  STEER_AddAttributeToConfiguration_Test
// =================================================================================================
void STEER_AddAttributeToConfiguration_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL report
    result = STEER_AddAttributeToConfiguration(NULL, 0, "dummy attribute", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration id
    result = STEER_AddAttributeToConfiguration(report, 1, "dummy attribute", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL attribute name
    result = STEER_AddAttributeToConfiguration(report, 0, NULL, 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty attribute name
    result = STEER_AddAttributeToConfiguration(report, 0, "", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);


    // Test with NULL attribute data type
    result = STEER_AddAttributeToConfiguration(report, 0, "dummy attribute", 
            NULL, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty attribute data type
    result = STEER_AddAttributeToConfiguration(report, 0, "dummy attribute", 
            "", 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);


    // Test with NULL attribute value
    result = STEER_AddAttributeToConfiguration(report, 0, "dummy attribute", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty attribute value
    result = STEER_AddAttributeToConfiguration(report, 0, "dummy attribute", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);


    // Test with valid parameters
    result = STEER_AddAttributeToConfiguration(report, 0, "new dummy attribute", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "2");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    result = STEER_ConfigurationHasAttribute(report, 0, "new dummy attribute");
    CU_ASSERT_TRUE(result);

    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport((tSTEER_ReportPtr*)&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_ConfigurationHasAttribute_Test
// =================================================================================================
void STEER_ConfigurationHasAttribute_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);


    result = STEER_AddAttributeToConfiguration(report, 0, "foo", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "2");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL report
    result = STEER_ConfigurationHasAttribute(NULL, 0, "dummy attribute");
    CU_ASSERT_FALSE(result);

    // Test with out of bounds configuration id
    result = STEER_ConfigurationHasAttribute(report, 1, "dummy attribute");
    CU_ASSERT_FALSE(result);

    // Test with NULL attribute name
    result = STEER_ConfigurationHasAttribute(report, 0, NULL);
    CU_ASSERT_FALSE(result);

    // Test with empty attribute name
    result = STEER_ConfigurationHasAttribute(report, 0, "");
    CU_ASSERT_FALSE(result);

    // Test with missing attribute
    result = STEER_ConfigurationHasAttribute(report, 0, "bar");
    CU_ASSERT_FALSE(result);

    // Test with valid attribute
    result = STEER_ConfigurationHasAttribute(report, 0, "foo");
    CU_ASSERT_TRUE(result);

    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddMetricToConfiguration_Test
// =================================================================================================
void STEER_AddMetricToConfiguration_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL report
    result = STEER_AddMetricToConfiguration(NULL, 0, "dummy metric", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration id
    result = STEER_AddMetricToConfiguration(report, 1, "dummy metric", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL metric name
    result = STEER_AddMetricToConfiguration(report, 0, NULL, 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty metric name
    result = STEER_AddMetricToConfiguration(report, 0, "", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);


    // Test with NULL metric data type
    result = STEER_AddMetricToConfiguration(report, 0, "dummy metric", 
            NULL, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty metric data type
    result = STEER_AddMetricToConfiguration(report, 0, "dummy metric", 
            "", 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);


    // Test with empty metric precision (can be NULL, but not empty)
    result = STEER_AddMetricToConfiguration(report, 0, "dummy metric", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            "", NULL, "1");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);


    // Test with empty metric unit (can be NULL, but not empty)
    result = STEER_AddMetricToConfiguration(report, 0, "dummy metric", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, "", "1");

    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);


    // Test with valid parameters
    result = STEER_AddMetricToConfiguration(report, 0, "dummy metric", 
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER, 
            NULL, NULL, "1");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);


    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddConfusionMatrixMetricsToConfiguration_Test
// =================================================================================================
void STEER_AddConfusionMatrixMetricsToConfiguration_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL report
    result = STEER_AddConfusionMatrixMetricsToConfiguration(NULL, 0, 4, 4, 1, 1, 1, 1);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration ID
    result = STEER_AddConfusionMatrixMetricsToConfiguration(report, 1, 4, 4, 1, 1, 1, 1);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with out of bounds minimum test count
    result = STEER_AddConfusionMatrixMetricsToConfiguration(report, 0, 0, 4, 1, 1, 1, 1);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with out of bounds actual test count
    result = STEER_AddConfusionMatrixMetricsToConfiguration(report, 0, 4, 0, 1, 1, 1, 1);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with proper values
    result = STEER_AddConfusionMatrixMetricsToConfiguration(report, 0, 4, 4, 1, 1, 1, 1);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddCriterionToConfiguration_Test
// =================================================================================================
void STEER_AddCriterionToConfiguration_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL report
    result = STEER_AddCriterionToConfiguration(NULL, 0, "dummy criterion", false);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration id
    result = STEER_AddCriterionToConfiguration(report, 1, "dummy criterion", false);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL criterion name
    result = STEER_AddCriterionToConfiguration(report, 0, NULL, false);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty criterion name
    result = STEER_AddCriterionToConfiguration(report, 0, "", false);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with valid attribute
    result = STEER_AddCriterionToConfiguration(report, 0, "foo", false);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddEvaluationToConfiguration_Test
// =================================================================================================
void STEER_AddEvaluationToConfiguration_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL report
    result = STEER_AddEvaluationToConfiguration(NULL, 0);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration id
    result = STEER_AddEvaluationToConfiguration(report, 1);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with valid attributes
    result = STEER_AddEvaluationToConfiguration(report, 0);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);


    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddCalculationToTest_Test
// =================================================================================================
void STEER_AddCalculationToTest_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL report
    result = STEER_AddCalculationToTest(NULL, 0, 0, "test calculation", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT,
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, "0");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration id
    result = STEER_AddCalculationToTest(report, 1, 0, "test calculation",  STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT, 
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, "0");
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with out of bounds test id
    result = STEER_AddCalculationToTest(report, 0, 1, "test calculation", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT, 
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, "0");
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL calculation name
    result = STEER_AddCalculationToTest(report, 0, 0, NULL, STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT,  
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, "0");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty calculation name
    result = STEER_AddCalculationToTest(report, 0, 0, "", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT, 
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, "0");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL calculation data type
    result = STEER_AddCalculationToTest(report, 0, 0, "test calculation", NULL, 
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, "0");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty calculation data type
    result = STEER_AddCalculationToTest(report, 0, 0, "test calculation", "",  
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, "0");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with empty calculation precision
    result = STEER_AddCalculationToTest(report, 0, 0, "test calculation", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT, 
            "", NULL, "0");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with empty calculation units
    result = STEER_AddCalculationToTest(report, 0, 0, "test calculation", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT,  
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, "", "0");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL calculation value
    result = STEER_AddCalculationToTest(report, 0, 0, "test calculation", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT, 
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty calculation value
    result = STEER_AddCalculationToTest(report, 0, 0, "test calculation", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT,  
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, "");
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with valid attribute
    result = STEER_AddCalculationToTest(NULL, 0, 0, "test calculation", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT,  
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, NULL, NULL);
    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddCalculationSetToTest_Test
// =================================================================================================
void STEER_AddCalculationSetToTest_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Initialize value set for calculation
    tSTEER_ValueSet * calculationSet;
    result = STEER_NewValueSet("dummy value set", STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT,
            NULL, NULL, &calculationSet); 

    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    // Test with NULL report
    result = STEER_AddCalculationSetToTest(NULL, 0, 0, calculationSet);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration id
    result = STEER_AddCalculationSetToTest(report, 1, 0, calculationSet);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with out of bounds test id
    result = STEER_AddCalculationSetToTest(report, 0, 1, calculationSet);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL calculation set
    result = STEER_AddCalculationSetToTest(report, 0, 0, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid values
    result = STEER_AddCalculationSetToTest(report, 0, 0, calculationSet);

    CU_ASSERT_PTR_NOT_NULL(calculationSet);
    result = STEER_FreeValueSet(&calculationSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(calculationSet);

    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddCriterionToTest_Test
// =================================================================================================
void STEER_AddCriterionToTest_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);


    // Test with NULL report
    result = STEER_AddCriterionToTest(NULL, 0, 0, "dummy criterion", true);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration id
    result = STEER_AddCriterionToTest(report, 1, 0, "dummy criterion", true);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with out of bounds test id
    result = STEER_AddCriterionToTest(report, 0, 1, "dummy criterion", true);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL criterion
    result = STEER_AddCriterionToTest(report, 0, 0, NULL, true);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty criterion
    result = STEER_AddCriterionToTest(report, 0, 0, "", true);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with valid values
    result = STEER_AddCriterionToTest(report, 0, 0, "dummy criterion", true);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddEvaluationToTest_Test
// =================================================================================================
void STEER_AddEvaluationToTest_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    bool testPassed = false;
    // Test with NULL report
    result = STEER_AddEvaluationToTest(NULL, 0, 0, &testPassed);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of bounds configuration id
    result = STEER_AddEvaluationToTest(report, 1, 0, &testPassed);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with out of bounds test id
    result = STEER_AddEvaluationToTest(report, 0, 1, &testPassed);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL passed pointer
    result = STEER_AddEvaluationToTest(report, 0, 0, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid values. Original report has all passing criteria
    result = STEER_AddEvaluationToTest(report, 0, 0, &testPassed);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(testPassed, true);

    // Add failing criterion
    result = STEER_AddCriterionToTest(report, 0, 0, "failing criterion", false);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    result = STEER_AddEvaluationToTest(report, 0, 0, &testPassed);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(testPassed, false);

    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_AddCriterionToReport_Test
// =================================================================================================
void STEER_AddCriterionToReport_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPtr report;

    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL report
    result = STEER_AddCriterionToReport(NULL, "dummy criterion", false);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL criterion name
    result = STEER_AddCriterionToReport(report, NULL, false);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty criterion name
    result = STEER_AddCriterionToReport(report, "", false);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with valid attribute
    result = STEER_AddCriterionToReport(report, "foo", false);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_GetMinimumTestCount_Test
// =================================================================================================
void STEER_GetMinimumTestCount_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with zero significance level
    uint64_t minTestCount = 0;
    uint64_t predictedPassCount = 0;
    uint64_t predictedFailCount = 0;
    result = STEER_GetMinimumTestCount(0.0, 1, 
            &minTestCount, 
            &predictedPassCount, 
            &predictedFailCount);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_EQUAL(minTestCount, 0);
    CU_ASSERT_EQUAL(predictedPassCount, 0);
    CU_ASSERT_EQUAL(predictedFailCount, 0);

    // Test with one significance level
    result = STEER_GetMinimumTestCount(1.0, 1, 
            &minTestCount, 
            &predictedPassCount, 
            &predictedFailCount);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_EQUAL(minTestCount, 0);
    CU_ASSERT_EQUAL(predictedPassCount, 0);
    CU_ASSERT_EQUAL(predictedFailCount, 0);

    // Test with zero bitstream count
    result = STEER_GetMinimumTestCount(0.01, 0, 
            &minTestCount, 
            &predictedPassCount, 
            &predictedFailCount);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_EQUAL(minTestCount, 0);
    CU_ASSERT_EQUAL(predictedPassCount, 0);
    CU_ASSERT_EQUAL(predictedFailCount, 0);

    // Test with NULL minimum test count
    result = STEER_GetMinimumTestCount(0.01, 1,
            NULL,
            &predictedPassCount,
            &predictedFailCount);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(predictedPassCount, 0);
    CU_ASSERT_EQUAL(predictedFailCount, 0);

    // Test with NULL predicted pass count
    result = STEER_GetMinimumTestCount(0.01, 1,
            &minTestCount,
            NULL,
            &predictedFailCount);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(minTestCount, 0);
    CU_ASSERT_EQUAL(predictedFailCount, 0);

    // Test with NULL predicted fail count
    result = STEER_GetMinimumTestCount(0.01, 1,
            &minTestCount,
            &predictedPassCount,
            NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(minTestCount, 0);
    CU_ASSERT_EQUAL(predictedPassCount, 0);

    // Test with valid arguments
    result = STEER_GetMinimumTestCount(0.01, 1,
            &minTestCount,
            &predictedPassCount,
            &predictedFailCount);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(minTestCount, 100);
    CU_ASSERT_EQUAL(predictedPassCount, 1);
    CU_ASSERT_EQUAL(predictedFailCount, 0);
}

// =================================================================================================
//  STEER_GetConfusionMatrix_Test
// =================================================================================================
void STEER_GetConfusionMatrix_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ConfusionMatrix *confusionMatrix = NULL;

    result = STEER_AllocateMemory(sizeof(tSTEER_ConfusionMatrix), (void**) &confusionMatrix);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL confusion Matrix
    result = STEER_GetConfusionMatrix(10, 20, 15, 5, 10, 5, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with bad minimum test count
    result = STEER_GetConfusionMatrix(0, 20, 15, 5, 10, 5, confusionMatrix);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with bad actual test count
    result = STEER_GetConfusionMatrix(10, 0, 15, 5, 10, 5, confusionMatrix);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with valid values
    result = STEER_GetConfusionMatrix(10, 20, 15, 5, 10, 5, confusionMatrix);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_EQUAL(confusionMatrix->predictedPassCount, 10);
    CU_ASSERT_EQUAL(confusionMatrix->predictedFailCount, 5);
    CU_ASSERT_EQUAL(confusionMatrix->actualPassCount, 15);
    CU_ASSERT_EQUAL(confusionMatrix->actualFailCount, 5);
    CU_ASSERT_EQUAL(confusionMatrix->falsePositives, 0);
    CU_ASSERT_EQUAL(confusionMatrix->falseNegatives, 0);
    CU_ASSERT_EQUAL(confusionMatrix->truePositives, confusionMatrix->predictedPassCount);
    CU_ASSERT_EQUAL(confusionMatrix->trueNegatives, confusionMatrix->predictedFailCount);

    // Test with valid values
    result = STEER_GetConfusionMatrix(10, 20, 12, 8, 8, 12, confusionMatrix);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_EQUAL(confusionMatrix->predictedPassCount, 8);
    CU_ASSERT_EQUAL(confusionMatrix->predictedFailCount, 12);
    CU_ASSERT_EQUAL(confusionMatrix->actualPassCount, 12);
    CU_ASSERT_EQUAL(confusionMatrix->actualFailCount, 8);
    CU_ASSERT_EQUAL(confusionMatrix->falsePositives, 0);
    CU_ASSERT_EQUAL(confusionMatrix->falseNegatives, 4);
    CU_ASSERT_EQUAL(confusionMatrix->truePositives, confusionMatrix->predictedPassCount);
    CU_ASSERT_EQUAL(confusionMatrix->trueNegatives, confusionMatrix->actualFailCount);

    // Test with valid values
    result = STEER_GetConfusionMatrix(10, 20, 8, 12, 12, 8, confusionMatrix);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_EQUAL(confusionMatrix->predictedPassCount, 12);
    CU_ASSERT_EQUAL(confusionMatrix->predictedFailCount, 8);
    CU_ASSERT_EQUAL(confusionMatrix->actualPassCount, 8);
    CU_ASSERT_EQUAL(confusionMatrix->actualFailCount, 12);
    CU_ASSERT_EQUAL(confusionMatrix->falsePositives, 4);
    CU_ASSERT_EQUAL(confusionMatrix->falseNegatives, 0);
    CU_ASSERT_EQUAL(confusionMatrix->truePositives, confusionMatrix->actualPassCount);
    CU_ASSERT_EQUAL(confusionMatrix->trueNegatives, confusionMatrix->predictedFailCount);

    CU_ASSERT_PTR_NOT_NULL(confusionMatrix);
    STEER_FreeMemory((void**)&confusionMatrix);
    CU_ASSERT_PTR_NULL(confusionMatrix);
}

// =================================================================================================
//  STEER_GetConfusionMatrixStatistics_Test
// =================================================================================================
void STEER_GetConfusionMatrixStatistics_Test (void)
{
    uint32_t result = STEER_RESULT_SUCCESS;

    tSTEER_ConfusionMatrix *confusionMatrix = NULL;
    tSTEER_ConfusionMatrixStatistics *confusionMatrixStatistics = NULL;

    result = STEER_AllocateMemory(sizeof(tSTEER_ConfusionMatrix), (void**) &confusionMatrix);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_AllocateMemory(sizeof(tSTEER_ConfusionMatrixStatistics), (void**) &confusionMatrixStatistics);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Initialize confusion matrix
    result = STEER_GetConfusionMatrix(10, 20, 12, 8, 8, 12, confusionMatrix);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with bad test count
    result = STEER_GetConfusionMatrixStatistics(0, confusionMatrix, confusionMatrixStatistics);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL confusion matrix
    result = STEER_GetConfusionMatrixStatistics(1, NULL, confusionMatrixStatistics);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL confusion matrix statistics
    result = STEER_GetConfusionMatrixStatistics(1, confusionMatrix, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with good values
    result = STEER_GetConfusionMatrixStatistics(20, confusionMatrix, confusionMatrixStatistics);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // True positive: 8
    // True negative: 8
    // False positives: 0
    // False negatives: 4
    // Predicted pass count: 8
    // Predicted fail count: 12
    // Actual pass count: 12
    // Actual fail count: 8

    // Check external calculations
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->truePositiveRate, 8.0 / 12.0, 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->trueNegativeRate, 8.0 / 8.0, 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->positivePredictiveValue, 8.0 / (8.0 + 0.0), 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->negativePredictiveValue, 8.0 / (8.0 + 4.0), 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->falsePositiveRate, 
            1.0 - confusionMatrixStatistics->trueNegativeRate, 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->falseNegativeRate, 
            1.0 - confusionMatrixStatistics->truePositiveRate, 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->falseDiscoveryRate, 
            1.0 - confusionMatrixStatistics->positivePredictiveValue, 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->falseOmissionRate, 
            1.0 - confusionMatrixStatistics->negativePredictiveValue, 0.01); 
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->threatScore, 8.0 / (8.0 + 4.0 + 0.0), 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->accuracy, (8.0 + 8.0) / (12.0 + 8.0), 0.01);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->matthewsCorrelationCoefficient, (8.0 * 8.0 - 0.0 * 4.0) / 
            sqrt( (8.0 + 0.0) * (8.0 + 4.0) * (8.0 + 0.0) * (8.0 + 4.0) ), 0.1);
    CU_ASSERT_DOUBLE_EQUAL(confusionMatrixStatistics->prevalence, 12.0 / (12.0 + 8.0), 0.01);

    // Clean up
    CU_ASSERT_PTR_NOT_NULL(confusionMatrix);
    STEER_FreeMemory((void**)&confusionMatrix);
    CU_ASSERT_PTR_NULL(confusionMatrix);

    CU_ASSERT_PTR_NOT_NULL(confusionMatrixStatistics);
    STEER_FreeMemory((void**)&confusionMatrixStatistics);
    CU_ASSERT_PTR_NULL(confusionMatrixStatistics);
}

// =================================================================================================
//  STEER_JsonToReport_Test
// =================================================================================================
void STEER_JsonToReport_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    // Test with NULL JSON
    tSTEER_ReportPtr report = NULL;
    result = STEER_JsonToReport(NULL, &report);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(report);

    // Test with empty JSON
    result = STEER_JsonToReport("", report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(report);

    // Test with not JSON
    result = STEER_JsonToReport(NOT_JSON, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_PARSE_FAILURE);
    CU_ASSERT_PTR_NULL(report);

    // Test with bad JSON tag
    result = STEER_JsonToReport(kDummyBadJsonTagReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_TAG_NOT_FOUND);
    CU_ASSERT_PTR_NULL(report);

    // Test with malformed JSON
    result = STEER_JsonToReport(kDummyMalformedJsonReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_PARSE_FAILURE);
    CU_ASSERT_PTR_NULL(report);


    // Test with NULL result
    result = STEER_JsonToReport(kDummyReportJson, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);


    // Test with valid arguments
    result = STEER_JsonToReport(kDummyReportJson, &report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(report);
    result = STEER_FreeReport(&report);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(report);
}

// =================================================================================================
//  STEER_JsonToSchedule_Test
// =================================================================================================
void STEER_JsonToSchedule_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL JSON
    tSTEER_Schedule* schedule = NULL;
    result = STEER_JsonToSchedule(NULL, &schedule);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(schedule);

    // Test with empty JSON
    result = STEER_JsonToSchedule("", &schedule);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(schedule);

    // Test with not JSON
    result = STEER_JsonToSchedule(NOT_JSON, &schedule);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_PARSE_FAILURE);
    CU_ASSERT_PTR_NULL(schedule);

    // Test with bad JSON tag
    result = STEER_JsonToSchedule(kDummyBadJsonTagScheduleJson, &schedule);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_TAG_NOT_FOUND);
    CU_ASSERT_PTR_NULL(schedule);

    // Test with malformed JSON
    result = STEER_JsonToSchedule(kDummyMalformedJsonScheduleJson, &schedule);
    CU_ASSERT_EQUAL(result, STEER_RESULT_JSON_PARSE_FAILURE);
    CU_ASSERT_PTR_NULL(schedule);

    // Test with NULL schedule
    result = STEER_JsonToSchedule(kDummyScheduleJson, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_JsonToSchedule(kDummyScheduleJson, &schedule);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(schedule);
    result = STEER_FreeSchedule(&schedule);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(schedule);
}

// =================================================================================================
//  STEER_FreeSchedule_Test
// =================================================================================================
void STEER_FreeSchedule_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    tSTEER_Schedule* schedule = NULL;
    result = STEER_JsonToSchedule(kDummyScheduleJson, &schedule);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(schedule);

    // Test with NULL argument
    result = STEER_FreeSchedule(NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid argument
    result = STEER_FreeSchedule(&schedule);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(schedule);
}

// =================================================================================================
//  STEER_ErrorString_Test
// =================================================================================================
void STEER_ErrorString_Test (void)
{
    // Test with standard error code
    const char * errorString = STEER_ErrorString(EINVAL);
    CU_ASSERT_PTR_NOT_NULL(errorString);

    // Test with STEER error code
    errorString = STEER_ErrorString(STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NOT_NULL(errorString);

    // Test with undefined error code
    errorString = STEER_ErrorString(12345678);
    CU_ASSERT_PTR_NOT_NULL(errorString);

    STEER_FreeMemory((void**)&errorString);
}


// =================================================================================================
//  STEER_ConvertStringToCamelCase_Test
// =================================================================================================
void STEER_ConvertStringToCamelCase_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL source string
    char* destStr = NULL;
    result = STEER_ConvertStringToCamelCase(NULL, &destStr);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(destStr);

    // Test with empty source string
    result = STEER_ConvertStringToCamelCase("", &destStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(destStr);

    // Test with NULL destination string
    result = STEER_ConvertStringToCamelCase("This is a string.", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_ConvertStringToCamelCase("This    is_a string.", &destStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(destStr);
    CU_ASSERT_EQUAL(strcmp(destStr, "ThisIsAString."), 0);
    STEER_FreeMemory((void**)&destStr);
    CU_ASSERT_PTR_NULL(destStr);
}


// =================================================================================================
//  STEER_ReplaceSubstring_Test
// =================================================================================================
void STEER_ReplaceSubstring_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL source string
    char * sourceStr = NULL;
    char * outStr = NULL;

    STEER_DuplicateString("dummy string dummy string dummy stri", &sourceStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    result = STEER_ReplaceSubstring(NULL, "string", "word", &outStr);
    CU_ASSERT_EQUAL(result, EFAULT);


    // Test with NULL oldval string
    result = STEER_ReplaceSubstring(sourceStr, NULL, "word", &outStr);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty oldval string
    result = STEER_ReplaceSubstring(sourceStr, "", "word", &outStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL newval string
    result = STEER_ReplaceSubstring(sourceStr, "string", NULL, &outStr);
    CU_ASSERT_EQUAL(result, EFAULT);
 
    // Test with NULL outStr string
    result = STEER_ReplaceSubstring(sourceStr, "string", "word", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
    
    // Test with valid arguments
    result = STEER_ReplaceSubstring(sourceStr, "string", "word", &outStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(strcmp(outStr, "dummy word dummy word dummy stri"), 0);

    // Test with valid arguments
    result = STEER_ReplaceSubstring(sourceStr, "string", "longer string", &outStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(strcmp(outStr, "dummy longer string dummy longer string dummy stri"), 0);

    // Test with valid arguments
    result = STEER_ReplaceSubstring(sourceStr, " ", "", &outStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(strcmp(outStr, "dummystringdummystringdummystri"), 0);

    STEER_FreeMemory((void**) &sourceStr);
    CU_ASSERT_PTR_NULL(sourceStr);
    STEER_FreeMemory((void**) &outStr);
    CU_ASSERT_PTR_NULL(outStr);
}

// =================================================================================================
//  STEER_SwapCharacters_Test
// =================================================================================================
void STEER_SwapCharacters_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    char * outStr = NULL;

    // Test with NULL source string
    char sourceStr[] = "dummy test with spaces";
    result = STEER_SwapCharacters(NULL, ' ', '-', &outStr);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty source string
    result = STEER_SwapCharacters("", ' ', '-', &outStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL out string
    result = STEER_SwapCharacters(NULL, ' ', '-', NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
    
    // Test with valid arguments
    result = STEER_SwapCharacters(sourceStr, ' ', '-', &outStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(strcmp(outStr, "dummy-test-with-spaces"), 0);
    STEER_FreeMemory((void**) &outStr);
    CU_ASSERT_PTR_NULL(outStr);
}

// =================================================================================================
//  STEER_DuplicateString_Test
// =================================================================================================
void STEER_DuplicateString_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL source string
    char* destStr = NULL;
    result = STEER_DuplicateString(NULL, &destStr);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(destStr);

    // Test with empty source string
    result = STEER_DuplicateString("", &destStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(destStr);
    STEER_FreeMemory((void**)&destStr);
    CU_ASSERT_PTR_NULL(destStr);

    // Test with NULL destination string
    result = STEER_DuplicateString("This is a string.", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_DuplicateString("This is a string.", &destStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(destStr);
    CU_ASSERT_EQUAL(strcmp(destStr, "This is a string."), 0);
    STEER_FreeMemory((void**)&destStr);
    CU_ASSERT_PTR_NULL(destStr);
}

// =================================================================================================
//  STEER_ConcatenateString_Test
// =================================================================================================
void STEER_ConcatenateString_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    char* theStr = NULL;
    result = STEER_DuplicateString("1 2 3", &theStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(theStr);

    // Test with NULL string
    result = STEER_ConcatenateString(NULL, " 4 5 6");
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL string to concatenate
    result = STEER_ConcatenateString(&theStr, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_ConcatenateString (&theStr, " 4 5 6");
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(theStr);
    CU_ASSERT_STRING_EQUAL(theStr, "1 2 3 4 5 6");
    STEER_FreeMemory((void**)&theStr);
    CU_ASSERT_PTR_NULL(theStr);
}

// =================================================================================================
//  STEER_GetTimestampString_Test
// =================================================================================================
void STEER_GetTimestampString_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL timestamp
    result = STEER_GetTimestampString(false, false, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    char* timeStr = NULL;
    result = STEER_GetTimestampString(false, false, &timeStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(timeStr);
    STEER_FreeMemory((void**)&timeStr);
    CU_ASSERT_PTR_NULL(timeStr);

    result = STEER_GetTimestampString(true, false, &timeStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(timeStr);
    STEER_FreeMemory((void**)&timeStr);
    CU_ASSERT_PTR_NULL(timeStr);

    result = STEER_GetTimestampString(false, true, &timeStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(timeStr);
    STEER_FreeMemory((void**)&timeStr);
    CU_ASSERT_PTR_NULL(timeStr);

    result = STEER_GetTimestampString(true, true, &timeStr);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(timeStr);
    STEER_FreeMemory((void**)&timeStr);
    CU_ASSERT_PTR_NULL(timeStr);
}

// =================================================================================================
//  STEER_TestInfoToJson_Test
// =================================================================================================
void STEER_TestInfoToJson_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL test info
    char* testInfoJson = NULL;
    result = STEER_TestInfoToJson(NULL, &testInfoJson);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL JSON
    result = STEER_TestInfoToJson(&gDummyTestInfo, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_TestInfoToJson(&gDummyTestInfo, &testInfoJson);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(testInfoJson);
    STEER_FreeMemory((void**)&testInfoJson);
    CU_ASSERT_PTR_NULL(testInfoJson);
}

// =================================================================================================
//  STEER_ConductorCmd_Test
// =================================================================================================
void STEER_ConductorCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test conductor command
    const char* conductorCmdMissingArg[] = { DUMMY_PROGRAM_NAME, "-c" };
    const char* conductorCmdEmptyArg[] = { DUMMY_PROGRAM_NAME, "--conductor", "" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, conductorCmdMissingArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, conductorCmdEmptyArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
}

// =================================================================================================
//  STEER_EntropyFilePathCmd_Test
// =================================================================================================
void STEER_EntropyFilePathCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test entropy file path command
    const char* entropyFilePathCmdMissingArg[] = { DUMMY_PROGRAM_NAME, "-e" };
    const char* entropyFilePathCmdEmptyArg[] = { DUMMY_PROGRAM_NAME, "--entropy-file-path", "" };
    const char* entropyFilePathCmdEmptyFileArg[] = { DUMMY_PROGRAM_NAME, "-e", "/tmp/entropy.bin" };

    result = CreateTestFile("/tmp/entropy.bin", NULL);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, entropyFilePathCmdMissingArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, entropyFilePathCmdEmptyArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, entropyFilePathCmdEmptyFileArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_HelpCmd_Test
// =================================================================================================
void STEER_HelpCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test help command
    const char* helpCmdExtraArg[] = { DUMMY_PROGRAM_NAME, "-h", "dummy" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, helpCmdExtraArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_NotesCmd_Test
// =================================================================================================
void STEER_NotesCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test notes command
    const char* notesCmdMissingArg[] = { DUMMY_PROGRAM_NAME, "-n" };
    const char* notesCmdEmptyArg[] = { DUMMY_PROGRAM_NAME, "--notes", "" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, notesCmdMissingArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, notesCmdEmptyArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
}

// =================================================================================================
//  STEER_ParametersCmd_Test
// =================================================================================================
void STEER_ParametersCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test parameters command
    const char* parametersCmdMissingArg[] = { DUMMY_PROGRAM_NAME, "-P" };
    const char* parametersCmdEmptyArg[] = { DUMMY_PROGRAM_NAME, "--parameters", "" };
    const char* parametersCmdBadJsonArg[] = { DUMMY_PROGRAM_NAME, "-P", NOT_JSON };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, parametersCmdMissingArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, parametersCmdEmptyArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, parametersCmdBadJsonArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_ParametersFilePathCmd_Test
// =================================================================================================
void STEER_ParametersFilePathCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test parameters file path command
    const char* parametersFilePathCmdMissingArg[] = { DUMMY_PROGRAM_NAME, "-p" };
    const char* parametersFilePathCmdEmptyArg[] = { DUMMY_PROGRAM_NAME, "--parameters-file-path", "" };
    const char* parametersFilePathCmdEmptyFileArg[] = { DUMMY_PROGRAM_NAME, "-p", "/tmp/parameters.json" };
    const char* parametersFilePathCmdBadJsonArg[] = { DUMMY_PROGRAM_NAME, "-p", "/tmp/bad.json" };

    result = CreateTestFile("/tmp/parameters.json", NULL);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    result = CreateTestFile("/tmp/bad.json", NOT_JSON);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, parametersFilePathCmdMissingArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, parametersFilePathCmdEmptyArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, parametersFilePathCmdEmptyFileArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, parametersFilePathCmdBadJsonArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_ParametersInfoCmd_Test
// =================================================================================================
void STEER_ParametersInfoCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test parameters info command
    const char* parametersInfoCmdExtraArg[] = { DUMMY_PROGRAM_NAME, "-i", "dummy" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, parametersInfoCmdExtraArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_ReportFilePathCmd_Test
// =================================================================================================
void STEER_ReportFilePathCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test report file path command
    const char* reportFilePathCmdMissingArg[] = { DUMMY_PROGRAM_NAME, "-r" };
    const char* reportFilePathCmdEmptyArg[] = { DUMMY_PROGRAM_NAME, "--report-file-path", "" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, reportFilePathCmdMissingArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, reportFilePathCmdEmptyArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
}

// =================================================================================================
//  STEER_ReportLevelCmd_Test
// =================================================================================================
void STEER_ReportLevelCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test report level command
    const char* reportLevelCmdMissingArg[] = { DUMMY_PROGRAM_NAME, "-l" };
    const char* reportLevelCmdBadArg[] = { DUMMY_PROGRAM_NAME, "--report-level", "dummy" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, reportLevelCmdMissingArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, reportLevelCmdBadArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_ReportProgressCmd_Test
// =================================================================================================
void STEER_ReportProgressCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test report progress command
    const char* reportProgressCmdExtraArg[] = { DUMMY_PROGRAM_NAME, "-R", "dummy" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, reportProgressCmdExtraArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_TestInfoCmd_Test
// =================================================================================================
void STEER_TestInfoCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test test info command
    const char* testInfoCmdExtraArg[] = { DUMMY_PROGRAM_NAME, "-t", "dummy" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, testInfoCmdExtraArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_VerboseCmd_Test
// =================================================================================================
void STEER_VerboseCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test verbose command
    const char* verboseCmdExtraArg[] = { DUMMY_PROGRAM_NAME, "-v", "dummy" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, verboseCmdExtraArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_VersionCmd_Test
// =================================================================================================
void STEER_VersionCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test version command
    const char* versionCmdExtraArg[] = { DUMMY_PROGRAM_NAME, "-V", "dummy" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 3, versionCmdExtraArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);
}

// =================================================================================================
//  STEER_InvalidCmd_Test
// =================================================================================================
void STEER_InvalidCmd_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test invalid command
    const char* invalidCmdArg[] = { DUMMY_PROGRAM_NAME, "-F" };

    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, invalidCmdArg,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);
}

// =================================================================================================
//  STEER_Run_Test
// =================================================================================================
void STEER_Run_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    const char* dummyArgs[] = { DUMMY_PROGRAM_NAME, "test_arg" };

    // Test with NULL program name
    result = STEER_Run(NULL, 2, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty program name
    result = STEER_Run("", 2, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with bad argc
    result = STEER_Run(DUMMY_PROGRAM_NAME, 0, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);

    result = STEER_Run(DUMMY_PROGRAM_NAME, -1, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with NULL argv
    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, NULL,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL get info function
    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, dummyArgs,
            NULL, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL get parameters info function
    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, dummyArgs,
            DummyGetInfoFunction, NULL,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL initialize function
    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            NULL, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL get configuration count function
    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, NULL,
            DummySetReportFunction, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL set report function
    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            NULL, DummyExecuteFunction,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL execute function
    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, NULL,
            DummyFinalizeFunction);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL finalize function
    result = STEER_Run(DUMMY_PROGRAM_NAME, 2, dummyArgs,
            DummyGetInfoFunction, DummyGetParametersInfoFunction,
            DummyInitFunction, DummyGetConfigurationCountFunction,
            DummySetReportFunction, DummyExecuteFunction,
            NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test the shell command API
    STEER_ConductorCmd_Test();
    STEER_EntropyFilePathCmd_Test();
    STEER_HelpCmd_Test();
    STEER_NotesCmd_Test();
    STEER_ParametersCmd_Test();
    STEER_ParametersFilePathCmd_Test();
    STEER_ParametersInfoCmd_Test();
    STEER_ReportFilePathCmd_Test();
    STEER_ReportLevelCmd_Test();
    STEER_ReportProgressCmd_Test();
    STEER_TestInfoCmd_Test();
    STEER_VerboseCmd_Test();
    STEER_VersionCmd_Test();
    STEER_InvalidCmd_Test();
}

// =================================================================================================
//  STEER_AllocateMemory_Test
// =================================================================================================
void STEER_AllocateMemory_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with an empty (size zero) buffer
    void* buffer = NULL;
    result = STEER_AllocateMemory(0, &buffer);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(buffer);

    // Test with non-zero sized buffer
    result = STEER_AllocateMemory(32, &buffer);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(buffer);
    STEER_FreeMemory(&buffer);
    CU_ASSERT_PTR_NULL(buffer);

    // Test with NULL buffer
    result = STEER_AllocateMemory(16, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
}

// =================================================================================================
//  STEER_ReallocateMemory_Test
// =================================================================================================
void STEER_ReallocateMemory_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with no buffer pre-allocated
    void* buffer = NULL;
    result = STEER_ReallocateMemory(0, 16, &buffer);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(buffer);
    STEER_FreeMemory(&buffer);
    CU_ASSERT_PTR_NULL(buffer);

    // Test with no buffer pre-allocated, but with non-zero current size
    result = STEER_ReallocateMemory(16, 32, &buffer);
    CU_ASSERT_EQUAL(result, EINVAL);
    STEER_FreeMemory(&buffer);
    CU_ASSERT_PTR_NULL(buffer);

    // Test with NULL buffer
    result = STEER_ReallocateMemory(0, 16, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test growing buffer
    result = STEER_AllocateMemory(16, &buffer);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(buffer);
    result = STEER_ReallocateMemory(16, 64, &buffer);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(buffer);

    // Test shrinking buffer
    result = STEER_ReallocateMemory(64, 32, &buffer);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(buffer);

    // Reallocate to an empty (zero size) buffer
    result = STEER_ReallocateMemory(32, 0, &buffer);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NOT_NULL(buffer);
    STEER_FreeMemory(&buffer);
    CU_ASSERT_PTR_NULL(buffer);
}

// =================================================================================================
//  STEER_FreeMemory_Test
// =================================================================================================
void STEER_FreeMemory_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL buffer pointer
    STEER_FreeMemory(NULL);

    // Test with pointer to NULL buffer
    void* buffer = NULL;
    STEER_FreeMemory(&buffer);

    // Test with valid argument
    result = STEER_AllocateMemory(16, &buffer);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(buffer);
    STEER_FreeMemory(&buffer);
    CU_ASSERT_PTR_NULL(buffer);
}

// =================================================================================================
//  STEER_WaitForProcessesToComplete_Test
// =================================================================================================
void STEER_WaitForProcessesToComplete_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL process list
    tSTEER_ProcessList* processList = NULL;
    uint32_t processSuccessCount = 0;
    uint32_t processFailureCount = 0;
    result = STEER_WaitForProcessesToComplete(NULL, 1000, &processSuccessCount, &processFailureCount);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(processSuccessCount, 0);
    CU_ASSERT_EQUAL(processFailureCount, 0);

    // Test with 0 second wait interval

    // Test with 10 second wait interval

    // Test with NULL success count
    result = STEER_AllocateMemory(sizeof(tSTEER_ProcessList) + sizeof(tSTEER_Process),
            (void**)&processList);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(processList);
    processList->count = 1;
    processList->process[0].pid = getpid();
    strcpy(processList->process[0].programName, "libsteer_unit_test");
    result = STEER_WaitForProcessesToComplete(processList, 1000, NULL, &processFailureCount);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(processFailureCount, 0);

    // Test with NULL failure count
    result = STEER_WaitForProcessesToComplete(processList, 1000, &processSuccessCount, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_EQUAL(processSuccessCount, 0);

    // Test with mismatched process list
    if (processList != NULL)
        memset((void*)&(processList->process[0]), 0, sizeof(tSTEER_Process));
    result = STEER_WaitForProcessesToComplete(processList, 1000, &processSuccessCount, &processFailureCount);
    CU_ASSERT_EQUAL(result, EINVAL);

    // Test with empty process list
    if (processList != NULL)
        processList->count = 0;
    result = STEER_WaitForProcessesToComplete(processList, 1000, &processSuccessCount, &processFailureCount);
    CU_ASSERT_EQUAL(result, EINVAL);
    STEER_FreeMemory((void**)&processList);
    CU_ASSERT_PTR_NULL(processList);
}

// =================================================================================================
//  STEER_GetNativeValue_Test
// =================================================================================================
void STEER_GetNativeValue_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL data type
    void* nativeValue = NULL;
    result = STEER_GetNativeValue(NULL, "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(nativeValue);

    // Test with empty data type
    result = STEER_GetNativeValue("", "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(nativeValue);

    // Test with unknown data type
    result = STEER_GetNativeValue("faketype", "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    // Test with NULL value
    result = STEER_GetNativeValue(STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
            NULL, &nativeValue);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(nativeValue);

    // Test with empty value
    result = STEER_GetNativeValue(STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
            "", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(nativeValue);

    // Test with NULL native value
    result = STEER_GetNativeValue(STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
            "384.25", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with out of range values
    result = STEER_GetNativeValue(STEER_JSON_VALUE_BOOLEAN,
            "foo", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            "-129", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            "128", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_16_BIT_INTEGER,
            "-32769", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_16_BIT_INTEGER,
            "32768", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
            "-2147483649", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
            "2147483648", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_64_BIT_INTEGER,
            "-9223372036854775809", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_64_BIT_INTEGER,
            "9223372036854775808", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_8_BIT_INTEGER,
            "-1", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_8_BIT_INTEGER,
            "256", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_16_BIT_INTEGER,
            "-1", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_16_BIT_INTEGER,
            "65536", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
            "-1", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
            "4294967296", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            "-1", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            "18446744073709551616", &nativeValue);
    CU_ASSERT_EQUAL(result, ERANGE);
    CU_ASSERT_PTR_NULL(nativeValue);

    // Test with type/value mismatch
    result = STEER_GetNativeValue(STEER_JSON_VALUE_BOOLEAN,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_16_BIT_INTEGER,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_64_BIT_INTEGER,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_8_BIT_INTEGER,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_16_BIT_INTEGER,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(nativeValue);

    // Test with valid arguments
    result = STEER_GetNativeValue(STEER_JSON_VALUE_BOOLEAN,
            "true", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_TRUE(*((bool*)nativeValue));
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((double*)nativeValue), 384.25);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_EXTENDED_PRECISION_FLOATING_POINT,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((long double*)nativeValue), 384.25);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            "-100", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((int8_t*)nativeValue), -100);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_16_BIT_INTEGER,
            "-20000", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((int16_t*)nativeValue), -20000);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
            "-2000000000", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((int32_t*)nativeValue), -2000000000);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SIGNED_64_BIT_INTEGER,
            "-2000000000000000000", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((int64_t*)nativeValue), -2000000000000000000);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT,
            "384.25", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((float*)nativeValue), 384.25);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_8_BIT_INTEGER,
            "200", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((uint8_t*)nativeValue), 200);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_16_BIT_INTEGER,
            "20000", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((uint16_t*)nativeValue), 20000);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
            "2000000000", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((uint32_t*)nativeValue), 2000000000);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            "2000000000000000000", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(*((uint64_t*)nativeValue), 2000000000000000000);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);

    result = STEER_GetNativeValue(STEER_JSON_VALUE_UTF_8_STRING,
            "This is a UTF-8 string.", &nativeValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(nativeValue);
    CU_ASSERT_EQUAL(strcmp((char*)nativeValue, "This is a UTF-8 string."), 0);
    STEER_FreeMemory((void**)&nativeValue);
    CU_ASSERT_PTR_NULL(nativeValue);
}

// =================================================================================================
//  STEER_NewValue_Test
// =================================================================================================
void STEER_NewValue_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_Value value;

    // Test with NULL name
    result = STEER_NewValue(NULL, "single precision floating point", "2", "bits", "5.00", &value);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty name
    result = STEER_NewValue("", "single precision floating point", "2", "bits", "5.00", &value);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL data type
    result = STEER_NewValue("dummy", NULL, "2", "bits", "5.00", &value);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty data type
    result = STEER_NewValue("dummy", "", "2", "bits", "5.00", &value);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with empty value
    result = STEER_NewValue("dummy", "single precision floating point", "2", "bits", "", &value);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL value
    result = STEER_NewValue("dummy", "single precision floating point", "2", "bits", NULL, &value);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL STEER_value
    result = STEER_NewValue("dummy", "single precision floating point", "2", "bits", "5.00", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid values
    result = STEER_NewValue("dummy", "single precision floating point", "2", "bits", "5.00", &value);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_PTR_NOT_NULL(&value);
    result = STEER_FreeValue(&value);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
}

// =================================================================================================
//  STEER_DuplicateValue_Test
// =================================================================================================
void STEER_DuplicateValue_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_Value* valueA = NULL;
    tSTEER_Value* valueB = NULL;

    result = STEER_AllocateMemory(sizeof(tSTEER_Value),(void **) &valueA);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_AllocateMemory(sizeof(tSTEER_Value),(void **) &valueB);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with null source
    result = STEER_DuplicateValue(NULL, valueB);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with null destinatin
    result = STEER_DuplicateValue(valueA, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Initialize source
    result = STEER_NewValue("dummy", "single precision floating point", "2", "bits", "5.00", valueA);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Initialize destination
    result = STEER_NewValue("dummy two", "double precision floating point", "6", "bytes", "3.1", valueB);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    CU_ASSERT_STRING_NOT_EQUAL(valueA->name, valueB->name);
    CU_ASSERT_STRING_NOT_EQUAL(valueA->dataType, valueB->dataType);
    CU_ASSERT_STRING_NOT_EQUAL(valueA->precision, valueB->precision);
    CU_ASSERT_STRING_NOT_EQUAL(valueA->units, valueB->units);
    CU_ASSERT_STRING_NOT_EQUAL(valueA->value, valueB->value);

    // Duplicate values
    result = STEER_DuplicateValue(valueA, valueB);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_STRING_EQUAL(valueA->name, valueB->name);
    CU_ASSERT_STRING_EQUAL(valueA->dataType, valueB->dataType);
    CU_ASSERT_STRING_EQUAL(valueA->precision, valueB->precision);
    CU_ASSERT_STRING_EQUAL(valueA->units, valueB->units);
    CU_ASSERT_STRING_EQUAL(valueA->value, valueB->value);


    // Clean up
    result = STEER_FreeValue(valueA);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    STEER_FreeMemory((void**)&valueA);

    result = STEER_FreeValue(valueB);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    STEER_FreeMemory((void**)&valueB);
}

// =================================================================================================
//  STEER_FreeValue_Test
// =================================================================================================
void STEER_FreeValue_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_Value value;

    // Initialize valid value
    result = STEER_NewValue("dummy value", "single precision floating point", "2", "bits", "5.00", &value);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with null pointer
    result = STEER_FreeValue(NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    CU_ASSERT_PTR_NOT_NULL(&value);

    // Test with valid pointer
    result = STEER_FreeValue(&value);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
}

// =================================================================================================
//  STEER_NewValueSet_Test
// =================================================================================================
void STEER_NewValueSet_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL name
    tSTEER_ValueSet* valueSet = NULL;
    result = STEER_NewValueSet(NULL, STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER, 
            "6", STEER_JSON_VALUE_BLOCKS, &valueSet);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(valueSet);

    // Test with empty name
    result = STEER_NewValueSet("", STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER, 
            "6", STEER_JSON_VALUE_BLOCKS, &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(valueSet);

    // Test with NULL data type
    result = STEER_NewValueSet("test", NULL, "6",
            STEER_JSON_VALUE_BLOCKS, &valueSet);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(valueSet);

    // Test with empty data type     
    result = STEER_NewValueSet("test", "", "6",
            STEER_JSON_VALUE_BLOCKS, &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(valueSet);

    // Test with NULL precision
    result = STEER_NewValueSet("test", STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            NULL, STEER_JSON_VALUE_BLOCKS, &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(valueSet);
    result = STEER_FreeValueSet(&valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(valueSet);

    // Test with empty precision
    result = STEER_NewValueSet("test", STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            "", STEER_JSON_VALUE_BLOCKS, &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(valueSet);

    // Test with NULL units
    result = STEER_NewValueSet("test", STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            "6", NULL, &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(valueSet);
    result = STEER_FreeValueSet(&valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(valueSet);

    // Test with empty units
    result = STEER_NewValueSet("test", STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            "6", "", &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);
    CU_ASSERT_PTR_NULL(valueSet);

    // Test with NULL value set
    result = STEER_NewValueSet("test", STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER,
            "6", STEER_JSON_VALUE_BLOCKS, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);
    CU_ASSERT_PTR_NULL(valueSet);
}

// =================================================================================================
//  STEER_AddValueToValueSet_Test
// =================================================================================================
void STEER_AddValueToValueSet_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Test with NULL label
    tSTEER_ValueSet* valueSet = NULL;
    result = STEER_AddValueToValueSet(NULL, "test value", &valueSet);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty label
    result = STEER_AddValueToValueSet("", "test value", &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL value
    result = STEER_AddValueToValueSet("label", NULL, &valueSet);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty value
    result = STEER_AddValueToValueSet("label", "", &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL value set
    result = STEER_AddValueToValueSet("label", "test value", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_NewValueSet("test", STEER_JSON_VALUE_UTF_8_STRING,
            NULL, NULL, &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(valueSet);
    result = STEER_AddValueToValueSet("label", "test value", &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    result = STEER_FreeValueSet(&valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(valueSet);
}

// =================================================================================================
//  STEER_GetValueFromValueSet_Test
// =================================================================================================
void STEER_GetValueFromValueSet_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Initialize value set
    char * valueValue = NULL;
    tSTEER_ValueSet* valueSet = NULL;
    result = STEER_NewValueSet("test", STEER_JSON_VALUE_UTF_8_STRING,
            NULL, NULL, &valueSet);

    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(valueSet);
    result = STEER_AddValueToValueSet("label", "test value", &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);

    // Test with NULL pointer
    result = STEER_GetValueFromValueSet(NULL, "label", &valueValue);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL label
    result = STEER_GetValueFromValueSet(valueSet, NULL, &valueValue);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with empty label
    result = STEER_GetValueFromValueSet(valueSet, "", &valueValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_EMPTY_STRING);

    // Test with NULL value
    result = STEER_GetValueFromValueSet(valueSet, "label", NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Get missing value
    result = STEER_GetValueFromValueSet(valueSet, "missing value", &valueValue);
    CU_ASSERT_EQUAL(result, EINVAL);
    CU_ASSERT_PTR_NULL(valueValue);

    // Get correct value
    result = STEER_GetValueFromValueSet(valueSet, "label", &valueValue);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_EQUAL(strcmp(valueValue, "test value"), 0);

    STEER_FreeMemory((void**)&valueValue);
    CU_ASSERT_PTR_NULL(valueValue);

    STEER_FreeValueSet(&valueSet);
    CU_ASSERT_PTR_NULL(valueSet);
}

// =================================================================================================
//  STEER_DuplicateValueSet_Test
// =================================================================================================
void STEER_DuplicateValueSet_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    tSTEER_ValueSet* valueSetA = NULL;
    result = STEER_NewValueSet("dummy value set", STEER_JSON_VALUE_UTF_8_STRING,
            "floating point", "bits", &valueSetA);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(valueSetA);


    tSTEER_ValueSet* valueSetB = NULL;
    CU_ASSERT_PTR_NULL(valueSetB);

    // Test with NULL source
    result = STEER_DuplicateValueSet(NULL, &valueSetB);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with NULL dest
    result = STEER_DuplicateValueSet(valueSetA, NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid arguments
    result = STEER_DuplicateValueSet(valueSetA, &valueSetB);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);



    CU_ASSERT_STRING_EQUAL(valueSetA->name, valueSetB->name);
    CU_ASSERT_STRING_EQUAL(valueSetA->dataType, valueSetB->dataType);
    CU_ASSERT_STRING_EQUAL(valueSetA->precision, valueSetB->precision);
    CU_ASSERT_STRING_EQUAL(valueSetA->units, valueSetB->units);


    result = STEER_FreeValueSet(&valueSetA);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(valueSetA);


    result = STEER_FreeValueSet(&valueSetB);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(valueSetB);
}

// =================================================================================================
//  STEER_FreeValueSet_Test
// =================================================================================================
void STEER_FreeValueSet_Test (void)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Setup
    tSTEER_ValueSet* valueSet = NULL;
    result = STEER_NewValueSet("test", STEER_JSON_VALUE_UTF_8_STRING,
            NULL, NULL, &valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(valueSet);

    // Test with NULL value set
    result = STEER_FreeValueSet(NULL);
    CU_ASSERT_EQUAL(result, EFAULT);

    // Test with valid argument
    result = STEER_FreeValueSet(&valueSet);
    CU_ASSERT_EQUAL(result, STEER_RESULT_SUCCESS);
    CU_ASSERT_PTR_NULL(valueSet);
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
        // Set up file system utilities test suite
        CU_pSuite fileSystemUtilitiesTestSuite = CU_add_suite("steer_file_system_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (fileSystemUtilitiesTestSuite != NULL)
        {
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_FileExists_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_FileSize_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_IsReadableDevicePath_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_OpenFile_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_CloseFile_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_WriteTextFile_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_ReadTextFile_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_ScanDirectoryForFilesWithExtension_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_GetProgramDirectory_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_ScanProgramDirectory_Test);
            CU_ADD_TEST(fileSystemUtilitiesTestSuite, STEER_ProgramAvailable_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up JSON utilities test suite
        CU_pSuite jsonUtilitiesTestSuite = CU_add_suite("steer_json_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (jsonUtilitiesTestSuite != NULL)
        {
            CU_ADD_TEST(jsonUtilitiesTestSuite, STEER_ReadJsonFromFile_Test);
            CU_ADD_TEST(jsonUtilitiesTestSuite, STEER_WriteJsonToFile_Test);
            CU_ADD_TEST(jsonUtilitiesTestSuite, STEER_ParseJsonString_Test);
            CU_ADD_TEST(jsonUtilitiesTestSuite, STEER_HasChildTag_Test);
            CU_ADD_TEST(jsonUtilitiesTestSuite, STEER_GetChildObjectString_Test);
            CU_ADD_TEST(jsonUtilitiesTestSuite, STEER_GetChildArray_Test);
            CU_ADD_TEST(jsonUtilitiesTestSuite, STEER_GetArrayItemIndexWithName_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up parameters info utilities test suite
        CU_pSuite parametersInfoUtilitiesTestSuite = CU_add_suite("steer_parameters_info_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (parametersInfoUtilitiesTestSuite != NULL)
        {
            CU_ADD_TEST(parametersInfoUtilitiesTestSuite, STEER_ParametersInfoToJson_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up report utilities test suite
        CU_pSuite reportUtilitiesTestSuite = CU_add_suite("steer_report_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (reportUtilitiesTestSuite != NULL)
        {
            // Done
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_FreeReport_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_JsonToReport_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddAttributeToConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_ConfigurationHasAttribute_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddCriterionToConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddMetricToConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddEvaluationToConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_GetMinimumTestCount_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddCalculationToTest_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddCalculationSetToTest_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddCriterionToReport_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddCriterionToTest_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddEvaluationToTest_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_AddConfusionMatrixMetricsToConfiguration_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_GetConfusionMatrix_Test);
            CU_ADD_TEST(reportUtilitiesTestSuite, STEER_GetConfusionMatrixStatistics_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up schedule utilities test suite
        CU_pSuite scheduleUtilitiesTestSuite = CU_add_suite("steer_schedule_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (scheduleUtilitiesTestSuite != NULL)
        {
            CU_ADD_TEST(scheduleUtilitiesTestSuite, STEER_JsonToSchedule_Test);
            CU_ADD_TEST(scheduleUtilitiesTestSuite, STEER_FreeSchedule_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up string utilities test suite
        CU_pSuite stringUtilitiesTestSuite = CU_add_suite("steer_string_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (stringUtilitiesTestSuite != NULL)
        {
            CU_ADD_TEST(stringUtilitiesTestSuite, STEER_ErrorString_Test);
            CU_ADD_TEST(stringUtilitiesTestSuite, STEER_ReplaceSubstring_Test);
            CU_ADD_TEST(stringUtilitiesTestSuite, STEER_SwapCharacters_Test);
            CU_ADD_TEST(stringUtilitiesTestSuite, STEER_DuplicateString_Test);
            CU_ADD_TEST(stringUtilitiesTestSuite, STEER_ConcatenateString_Test);
            CU_ADD_TEST(stringUtilitiesTestSuite, STEER_GetTimestampString_Test);
            CU_ADD_TEST(stringUtilitiesTestSuite, STEER_ConvertStringToCamelCase_Test);

        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up test info utilities test suite
        CU_pSuite testInfoUtilitiesTestSuite = CU_add_suite("steer_test_info_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (testInfoUtilitiesTestSuite != NULL)
        {
            CU_ADD_TEST(testInfoUtilitiesTestSuite, STEER_TestInfoToJson_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up test shell test suite
        CU_pSuite testShellTestSuite = CU_add_suite("steer_test_shell test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (testShellTestSuite != NULL)
        {
            CU_ADD_TEST(testShellTestSuite, STEER_Run_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up utilities test suite
        CU_pSuite utilitiesTestSuite = CU_add_suite("steer_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (utilitiesTestSuite != NULL)
        {
            CU_ADD_TEST(utilitiesTestSuite, STEER_AllocateMemory_Test);
            CU_ADD_TEST(utilitiesTestSuite, STEER_ReallocateMemory_Test);
            CU_ADD_TEST(utilitiesTestSuite, STEER_FreeMemory_Test);
            CU_ADD_TEST(utilitiesTestSuite, STEER_WaitForProcessesToComplete_Test);
        }
        else    // CU_add_suite failed
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Set up value utilities test suite
        CU_pSuite valueUtilitiesTestSuite = CU_add_suite("steer_value_utilities test suite",
                STEER_Lib_SuiteInit,
                STEER_Lib_SuiteCleanup);
        if (valueUtilitiesTestSuite != NULL)
        {
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_GetNativeValue_Test);
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_NewValue_Test);
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_NewValueSet_Test);
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_AddValueToValueSet_Test);
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_FreeValueSet_Test);
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_FreeValue_Test); 
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_DuplicateValue_Test);
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_DuplicateValueSet_Test);
            CU_ADD_TEST(valueUtilitiesTestSuite, STEER_GetValueFromValueSet_Test);
        }
        else
        {
            result = CU_get_error();
            printf("\tCU_add_suite failed with error code %d!\n", result);
        }

        // Check for success
        if (result == CUE_SUCCESS)
        {
            CU_set_output_filename("./logs/libsteer_Unit_Test");
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

