// =================================================================================================
//! @file validate_test.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements a program to compare the test report from a STEER test program 
//! and determine whether it matches expected results.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-03-14
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_file_system_utilities_private.h"
#include "steer_json_utilities.h"
#include "steer_report_utilities.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include <libgen.h>

// =================================================================================================
//  Private constants
// =================================================================================================

//  Command line strings
#define EXPECTED_RESULTS_FILE_PATH_CMD  "expected-results-file-path"
#define HELP_CMD                        "help"
#define REPORT_FILE_PATH_CMD            "report-file-path"

//  Command line options
#define EXPECTED_RESULTS_FILE_PATH_OPTION   0
#define HELP_OPTION                         1
#define REPORT_FILE_PATH_OPTION             2

//  Short command line strings
#define EXPECTED_RESULTS_FILE_PATH_SHORT_CMD    'e'
#define HELP_SHORT_CMD                          'h'
#define REPORT_FILE_PATH_SHORT_CMD              'r'
#define UNKNOWN_SHORT_CMD                       '?'

//  Options array
static const char kShortOptions[] = { EXPECTED_RESULTS_FILE_PATH_SHORT_CMD, ':',
                                      HELP_SHORT_CMD,
                                      REPORT_FILE_PATH_SHORT_CMD, ':',
                                      0x00 };

// =================================================================================================
//  PrintCmdLineHelp
// =================================================================================================
void PrintCmdLineHelp (const char* programName)
{
    printf("\nUsage: %s <arguments>\n\n", basename((char*)programName));
    printf("\tAvailable command line arguments are:\n\n");
    printf("\t-%c, --%s\t\tPath to an expected results JSON file.\n", 
           EXPECTED_RESULTS_FILE_PATH_SHORT_CMD, 
           EXPECTED_RESULTS_FILE_PATH_CMD);
    printf("\t-%c, --%s\t\t\t\t\tPrints this usage notice.\n",
           HELP_SHORT_CMD, 
           HELP_CMD);
    printf("\t-%c, --%s\t\t\tPath to a report JSON file.\n", 
           REPORT_FILE_PATH_SHORT_CMD, 
           REPORT_FILE_PATH_CMD);
    return;
}

// =================================================================================================
//  CompareTestObjects
// =================================================================================================
int32_t CompareTestObjects (cJSON* expectedResultsTestObject,
                            cJSON* reportTestObject,
                            bool* testsMatch)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(expectedResultsTestObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(reportTestObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testsMatch);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* expectedResultsCalculationsArray = NULL;
        uint32_t expectedResultsCalculationsArraySize = 0;
        cJSON* reportCalculationsArray = NULL;
        uint32_t reportCalculationsArraySize = 0;
        uint_fast32_t i = 0;
        uint_fast32_t j = 0;

        // Setup
        *testsMatch = false;

        // Get the calculations array from the expected results test object
        result = STEER_GetChildArray(expectedResultsTestObject, STEER_JSON_TAG_CALCULATIONS,
                                     &expectedResultsCalculationsArray, 
                                     &expectedResultsCalculationsArraySize);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the calculations array from the report test object
            result = STEER_GetChildArray(reportTestObject, STEER_JSON_TAG_CALCULATIONS,
                                         &reportCalculationsArray,
                                         &reportCalculationsArraySize);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* expectedResultsCalculationObj = NULL;
            cJSON* reportCalculationObj = NULL;
            bool foundCalculation = false;
            bool calculationMatches = false;
            char* expectedResultsCalculationName = NULL;
            char* reportCalculationName = NULL;
            cJSON_bool jsonResult = 0;

            // Walk the calculations in the expected results config
            for (i = 0; i < expectedResultsCalculationsArraySize; i++)
            {
                calculationMatches = false;

                // Get an expected results calculation
                expectedResultsCalculationObj = cJSON_GetArrayItem(expectedResultsCalculationsArray, i);
                result = STEER_CHECK_CONDITION((expectedResultsCalculationObj != NULL),
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the name
                    result = STEER_GetChildObjectString(expectedResultsCalculationObj,
                                                        STEER_JSON_TAG_NAME, 
                                                        &expectedResultsCalculationName);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Find this calculation in the report
                        foundCalculation = false;
                        for (j = 0; j < reportCalculationsArraySize; j++)
                        {
                            reportCalculationObj = cJSON_GetArrayItem(reportCalculationsArray, j);
                            result = STEER_CHECK_CONDITION((reportCalculationObj != NULL),
                                                           STEER_RESULT_JSON_OPERATION_FAILURE);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Get the name
                                result = STEER_GetChildObjectString(reportCalculationObj,
                                                                    STEER_JSON_TAG_NAME,
                                                                    &reportCalculationName);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    // Do the names match?
                                    if (strcmp(expectedResultsCalculationName, reportCalculationName) == 0)
                                    {
                                        foundCalculation = true;

                                        // Yes - compare the JSON objects
                                        jsonResult = cJSON_Compare(expectedResultsCalculationObj,
                                                                   reportCalculationObj, cJSON_True);
                                        if (jsonResult == 1)
                                            calculationMatches = true;

                                        break;
                                    }

                                    // Clean up
                                    STEER_FreeMemory((void**)&reportCalculationName);
                                }
                            }

                            if ((result != STEER_RESULT_SUCCESS) || foundCalculation)
                                break;
                        }

                        // Clean up
                        STEER_FreeMemory((void**)&expectedResultsCalculationName);
                    }
                }

                if ((result != STEER_RESULT_SUCCESS) || !calculationMatches)
                    break;
            }

            *testsMatch = calculationMatches;
        }
    }
    return result;
}

// =================================================================================================
//  CompareConfigurationObjects
// =================================================================================================
int32_t CompareConfigurationObjects (cJSON* expectedResultsConfigObject,
                                     cJSON* reportConfigObject,
                                     bool* configsMatch)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(expectedResultsConfigObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(reportConfigObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(configsMatch);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* expectedResultsTestsArray = NULL;
        uint32_t expectedResultsTestsArraySize = 0;
        cJSON* reportTestsArray = NULL;
        uint32_t reportTestsArraySize = 0;
        uint_fast32_t i = 0;
        uint_fast32_t j = 0;

        // Setup
        *configsMatch = false;

        // Get the tests array from the expected results config object
        result = STEER_GetChildArray(expectedResultsConfigObject, STEER_JSON_TAG_TESTS,
                                     &expectedResultsTestsArray, &expectedResultsTestsArraySize);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the tests array from the report config object
            result = STEER_GetChildArray(reportConfigObject, STEER_JSON_TAG_TESTS,
                                         &reportTestsArray, &reportTestsArraySize);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Make sure the counts match
                result = STEER_CHECK_CONDITION((expectedResultsTestsArraySize == reportTestsArraySize),
                                               STEER_RESULT_FAILURE);
            }
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* expectedResultsTestObj = NULL;
            cJSON* reportTestObj = NULL;
            char* expectedResultsTestId = NULL;
            char* reportTestId = NULL;
            bool foundTest = false;
            bool testMatches = false;

            // Walk the tests in the expected results config
            for (i = 0; i < expectedResultsTestsArraySize; i++)
            {
                testMatches = false;

                // Get an expected results test
                expectedResultsTestObj = cJSON_GetArrayItem(expectedResultsTestsArray, i);
                result = STEER_CHECK_CONDITION((expectedResultsTestObj != NULL),
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get its test ID
                    result = STEER_GetChildObjectString(expectedResultsTestObj,
                                                        STEER_JSON_TAG_TEST_ID,
                                                        &expectedResultsTestId);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Find this test in the report
                        foundTest = false;
                        for (j = 0; j < reportTestsArraySize; j++)
                        {
                            reportTestObj = cJSON_GetArrayItem(reportTestsArray, j);
                            result = STEER_CHECK_CONDITION((reportTestObj != NULL),
                                                           STEER_RESULT_JSON_OPERATION_FAILURE);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Get the report test ID
                                result = STEER_GetChildObjectString(reportTestObj,
                                                                    STEER_JSON_TAG_TEST_ID,
                                                                    &reportTestId);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    // Do the test IDs match?
                                    if (strcmp(expectedResultsTestId, reportTestId) == 0)
                                    {
                                        foundTest = true;

                                        // Yes - continue comparing
                                        result = CompareTestObjects(expectedResultsTestObj,
                                                                    reportTestObj,
                                                                    &testMatches);

                                        break;
                                    }

                                    // Clean up
                                    STEER_FreeMemory((void**)&reportTestId);
                                }
                            }

                            if ((result != STEER_RESULT_SUCCESS) || foundTest)
                                break;
                        }

                        // Clean up
                        STEER_FreeMemory((void**)&expectedResultsTestId);
                    }
                }

                if ((result != STEER_RESULT_SUCCESS) || !testMatches)
                    break;
            }

            *configsMatch = testMatches;
        }
    }
    return result;
}

// =================================================================================================
//  main
// =================================================================================================
int main (int argc, const char * argv[])
{
    int32_t result = STEER_RESULT_SUCCESS;
    char* testReportFilePath = NULL;
    char* expectedResultsFilePath = NULL;

    // Check for no arguments 
    if (argc == 1)
    {
        PrintCmdLineHelp(argv[0]);
        printf("\n");
    }
    else
    {
        int32_t cmdLineOption = 0;
        int32_t optionIndex = 0;
        bool done = false;
        size_t len = 0;
        char* expandedPath = NULL;
        
        // Define command line options
        static struct option longOptions[] =
        {
            { EXPECTED_RESULTS_FILE_PATH_CMD,   required_argument,  0, 0 }, // Path to an expected results JSON file
            { HELP_CMD,                         no_argument,        0, 0 }, // Prints help
            { REPORT_FILE_PATH_CMD,             required_argument,  0, 0 }, // Path to a test report JSON file
            { 0,                                0,                  0, 0 }
        };

        // Parse command line arguments
        while ((cmdLineOption != -1) && (result == STEER_RESULT_SUCCESS))
        {
            cmdLineOption = getopt_long(argc,
                                        (char *const *)argv,
                                        kShortOptions,
                                        longOptions,
                                        &optionIndex);
            
            // Case on command line option
            switch (cmdLineOption)
            {
                case 0: // Long option found

                    // Expected results file path
                    if (optionIndex == EXPECTED_RESULTS_FILE_PATH_OPTION)
                    {
                        len = strlen(optarg);
                        if (len > 0)
                        {
                            result = STEER_ExpandTildeInPath(optarg, &expandedPath);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                if (expandedPath == NULL)
                                    result = STEER_DuplicateString(optarg, &expectedResultsFilePath);
                                else
                                {
                                    result = STEER_DuplicateString(expandedPath, &expectedResultsFilePath);
                                    STEER_FreeMemory((void**)&expandedPath);
                                }
                            }
                        }
                    }

                    // Help
                    else if (optionIndex == HELP_OPTION)
                    {
                        PrintCmdLineHelp(argv[0]);
                        printf("\n");
                        done = true;
                    }

                    // Report file path
                    else if (optionIndex == REPORT_FILE_PATH_OPTION)
                    {
                        len = strlen(optarg);
                        if (len > 0)
                        {
                            result = STEER_ExpandTildeInPath(optarg, &expandedPath);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                if (expandedPath == NULL)
                                    result = STEER_DuplicateString(optarg, &testReportFilePath);
                                else
                                {
                                    result = STEER_DuplicateString(expandedPath, &testReportFilePath);
                                    STEER_FreeMemory((void**)&expandedPath);
                                }
                            }
                        }
                    }

                    // Unknown
                    else
                    {
                        PrintCmdLineHelp(argv[0]);
                        printf("\n");
                        done = true;
                    }
                    break;

                // Expected results file path
                case EXPECTED_RESULTS_FILE_PATH_SHORT_CMD:
                    len = strlen(optarg);
                    if (len > 0)
                    {
                        result = STEER_ExpandTildeInPath(optarg, &expandedPath);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            if (expandedPath == NULL)
                                result = STEER_DuplicateString(optarg, &expectedResultsFilePath);
                            else
                            {
                                result = STEER_DuplicateString(expandedPath, &expectedResultsFilePath);
                                STEER_FreeMemory((void**)&expandedPath);
                            }
                        }
                    }
                    break;

                // Help
                case HELP_SHORT_CMD:
                    PrintCmdLineHelp(argv[0]);
                    printf("\n");
                    done = true;
                    break;

                // Report file path
                case REPORT_FILE_PATH_SHORT_CMD:
                    len = strlen(optarg);
                    if (len > 0)
                    {
                        result = STEER_ExpandTildeInPath(optarg, &expandedPath);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            if (expandedPath == NULL)
                                result = STEER_DuplicateString(optarg, &testReportFilePath);
                            else
                            {
                                result = STEER_DuplicateString(expandedPath, &testReportFilePath);
                                STEER_FreeMemory((void**)&expandedPath);
                            }
                        }
                    }
                    break;

                // Unknown
                case UNKNOWN_SHORT_CMD:
                    PrintCmdLineHelp(argv[0]);
                    printf("\n");
                    done = true;
                    break;

                default:
                    break;
            }
        }

        // Continue?
        if ((result == STEER_RESULT_SUCCESS) && !done)
        {
            if ((testReportFilePath != NULL) && (expectedResultsFilePath != NULL))
            {
                cJSON* reportRoot = NULL;
                cJSON* expectedResultsRoot = NULL;

                result = STEER_ReadJsonFromFile(testReportFilePath, &reportRoot);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_ReadJsonFromFile(expectedResultsFilePath, &expectedResultsRoot);

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                {
                    cJSON* reportObj = NULL;
                    cJSON* expectedResultsObj = NULL;
                    bool configsMatch = true;

                    reportObj = cJSON_GetObjectItemCaseSensitive(reportRoot, 
                                                                 STEER_JSON_TAG_REPORT);
                    result = STEER_CHECK_CONDITION((reportObj != NULL),
                                                   STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        expectedResultsObj = cJSON_GetObjectItemCaseSensitive(expectedResultsRoot,
                                                                              STEER_JSON_TAG_EXPECTED_RESULTS);
                        result = STEER_CHECK_CONDITION((expectedResultsObj != NULL),
                                                       STEER_RESULT_JSON_OPERATION_FAILURE);
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        cJSON* expectedResultsConfigArray = NULL;
                        uint32_t expectedResultsConfigArraySize = 0;
                        cJSON* reportConfigArray = NULL;
                        uint32_t reportConfigArraySize = 0;
                        cJSON* expectedResultsConfigObj = NULL;
                        cJSON* reportConfigObj = NULL;
                        char* expectedResultsConfigId = NULL;
                        char* reportConfigId = NULL;
                        uint_fast32_t i = 0;
                        uint_fast32_t j = 0;

                        // Get the configuration array from the expected results
                        result = STEER_GetChildArray(expectedResultsObj, STEER_JSON_TAG_CONFIGURATIONS,
                                                     &expectedResultsConfigArray, &expectedResultsConfigArraySize);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Get the configuration array from the report
                            result = STEER_GetChildArray(reportObj, STEER_JSON_TAG_CONFIGURATIONS,
                                                          &reportConfigArray, &reportConfigArraySize);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Make sure configuration count matches
                                result = STEER_CHECK_CONDITION((expectedResultsConfigArraySize == reportConfigArraySize),
                                                               STEER_RESULT_FAILURE);
                            }
                        }

                        if (result == STEER_RESULT_SUCCESS)
                        {
                            bool foundConfig = false;
                            bool configMatches = false;

                            // Walk the configurations in the expected results
                            for (i = 0; i < expectedResultsConfigArraySize; i++)
                            {
                                configMatches = false;

                                // Get an expected results config
                                expectedResultsConfigObj = cJSON_GetArrayItem(expectedResultsConfigArray, i);
                                result = STEER_CHECK_CONDITION((expectedResultsConfigObj != NULL), 
                                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    // Get its configuration ID
                                    result = STEER_GetChildObjectString(expectedResultsConfigObj,
                                                                        STEER_JSON_TAG_CONFIGURATION_ID,
                                                                        &expectedResultsConfigId);
                                    if (result == STEER_RESULT_SUCCESS)
                                    {
                                        // Find this configuration in the report
                                        foundConfig = false;
                                        for (j = 0; j < reportConfigArraySize; j++)
                                        {
                                            reportConfigObj = cJSON_GetArrayItem(reportConfigArray, j);
                                            result = STEER_CHECK_CONDITION((reportConfigObj != NULL),
                                                                           STEER_RESULT_JSON_OPERATION_FAILURE);
                                            if (result == STEER_RESULT_SUCCESS)
                                            {
                                                // Get the report configuration ID
                                                result = STEER_GetChildObjectString(reportConfigObj,
                                                                                    STEER_JSON_TAG_CONFIGURATION_ID,
                                                                                    &reportConfigId);
                                                if (result == STEER_RESULT_SUCCESS)
                                                {
                                                    // Do the configuration IDs match?
                                                    if (strcmp(expectedResultsConfigId, reportConfigId) == 0)
                                                    {
                                                        foundConfig = true;

                                                        // Yes - continue comparing
                                                        result = CompareConfigurationObjects(expectedResultsConfigObj,
                                                                                             reportConfigObj,
                                                                                             &configMatches);
                                                        break;
                                                    }

                                                    // Clean up
                                                    STEER_FreeMemory((void**)&reportConfigId);
                                                }
                                            }
                                            
                                            if ((result != STEER_RESULT_SUCCESS) || foundConfig)
                                                break;
                                        }

                                        // Clean up
                                        STEER_FreeMemory((void**)&expectedResultsConfigId);
                                    }
                                }
                                
                                if ((result != STEER_RESULT_SUCCESS) || !configMatches)
                                    break;
                            }

                            configsMatch = configMatches;
                        }
                    }

                    if (configsMatch)
                    {
                        #if STEER_ENABLE_CONSOLE_LOGGING
                            if ((testReportFilePath != NULL) && (expectedResultsFilePath != NULL))
                            {
                                fprintf(stdout, "%s validated against %s.\n", 
                                        testReportFilePath, expectedResultsFilePath);

                            }
                        #endif
                    }
                    else
                    {
                        result = STEER_CHECK_ERROR(STEER_RESULT_VALIDATION_CHECK_FAILURE);
                        #if STEER_ENABLE_CONSOLE_LOGGING
                            if ((testReportFilePath != NULL) && (expectedResultsFilePath != NULL))
                            {
                                fprintf(stdout, "%s failed to validate against %s!\n", 
                                        testReportFilePath, expectedResultsFilePath);

                            }
                        #endif
                    }
                }
                
                // Clean up
                if (reportRoot != NULL)
                {
                    cJSON_Delete(reportRoot);
                    reportRoot = NULL;
                }
                if (expectedResultsRoot != NULL)
                {
                    cJSON_Delete(expectedResultsRoot);
                    expectedResultsRoot = NULL;
                }
            }
        }
    }

    // Clean up
    STEER_FreeMemory((void**)&testReportFilePath);
    STEER_FreeMemory((void**)&expectedResultsFilePath);

    return result;
}

// =================================================================================================
