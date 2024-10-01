// =================================================================================================
//! @file run_validations.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements a program to automate STEER test validation.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-05-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_file_system_utilities.h"
#include "steer_file_system_utilities_private.h"
#include "steer_json_utilities.h"
#include "steer_report_utilities.h"
#include "steer_report_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include <libgen.h>
#include <time.h>

// =================================================================================================
//  Private constants
// =================================================================================================

//  Command line strings
#define CONFIG_FILE_PATH_CMD    "config-file-path"
#define CONFIG_JSON_CMD         "config-json"
#define HELP_CMD                "help"

//  Command line options
#define CONFIG_FILE_PATH_OPTION 0
#define CONFIG_JSON_OPTION      1
#define HELP_OPTION             2

//  Short command line strings
#define CONFIG_FILE_PATH_SHORT_CMD  'c'
#define CONFIG_JSON_SHORT_CMD       'C'
#define HELP_SHORT_CMD              'h'
#define UNKNOWN_SHORT_CMD           '?'

//  Options array
static const char kShortOptions[] = { CONFIG_FILE_PATH_SHORT_CMD, ':',
                                      CONFIG_JSON_SHORT_CMD, ':',
                                      HELP_SHORT_CMD,
                                      0x00 };

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tvalidation
{
    char*   measuredPath;
    char*   expectedPath;
}
tValidation;

typedef struct tvalidationlist
{
    uint32_t    count;
    tValidation validation[];
}
tValidationList;

// =================================================================================================
//  Private prototypes
// =================================================================================================

static void PrintCmdLineHelp (const char* programName);

static int32_t ReadValidations (cJSON* validationsJson,
                                tValidationList** validations);

static int32_t SpawnCheck (char* programDirectory,
                           char* programName,
                           char* measuredPath,
                           char* expectedPath,
                           tSTEER_ProcessList** processList);

// =================================================================================================
//  PrintCmdLineHelp
// =================================================================================================
void PrintCmdLineHelp (const char* programName)
{
    printf("\nUsage: %s <arguments>\n\n", basename((char*)programName));
    printf("\tAvailable command line arguments are:\n\n");
    printf("\t-%c, --%s <path>\tPath to a file containing a JSON validation configuration.\n", 
           CONFIG_FILE_PATH_SHORT_CMD, 
           CONFIG_FILE_PATH_CMD);
    printf("\t-%c, --%s <json>\tJSON validation configuration.\n", 
           CONFIG_JSON_SHORT_CMD, 
           CONFIG_JSON_CMD);
    printf("\t-%c, --%s\t\t\tPrints this usage notice.\n", 
           HELP_SHORT_CMD, 
           HELP_CMD);
    return;
}

// =================================================================================================
//  ReadValidations
// =================================================================================================
int32_t ReadValidations (cJSON* validationsJson,
                         tValidationList** validations)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(validationsJson);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(validations);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* validationsArray = NULL;
        uint32_t validationsArraySize = 0;

        // Setup
        *validations = NULL;
        
        result = STEER_GetChildArray(validationsJson, STEER_JSON_TAG_VALIDATIONS,
                                     &validationsArray, &validationsArraySize);
        if (result == STEER_RESULT_SUCCESS)
        {
            result = STEER_AllocateMemory(sizeof(tValidationList) +
                                          (sizeof(tValidation) * validationsArraySize),
                                          (void**)validations);
            if (result == STEER_RESULT_SUCCESS)
            {
                cJSON* obj = NULL;
                uint_fast32_t i = 0;

                for (i = 0; i < validationsArraySize; i++)
                {
                    obj = cJSON_GetArrayItem(validationsArray, i);
                    result = STEER_CHECK_CONDITION((obj != NULL),
                                                   STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        result = STEER_GetChildObjectString(obj, STEER_JSON_TAG_MEASURED,
                                                            (char**)&((*validations)->validation[i].measuredPath));
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        result = STEER_GetChildObjectString(obj, STEER_JSON_TAG_EXPECTED,
                                                            (char**)&((*validations)->validation[i].expectedPath));
                    }

                    if (result == STEER_RESULT_SUCCESS)
                        (*validations)->count++;
                    else
                        break;
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  SpawnCheck
// =================================================================================================
int32_t SpawnCheck (char* programDirectory,
                    char* programName,
                    char* measuredPath,
                    char* expectedPath,
                    tSTEER_ProcessList** processList)
{
    int32_t result = STEER_RESULT_SUCCESS;
    size_t fileSize = 0;
    char* theProgramName = NULL;

    // Check arguments
    result = STEER_CHECK_STRING(programDirectory);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(programName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(measuredPath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(expectedPath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(processList);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Does the file exist?
        if (STEER_FileExists(measuredPath))
        {
            // Is the file not empty?
            result = STEER_FileSize(measuredPath, &fileSize);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_CHECK_CONDITION((fileSize > 0), STEER_RESULT_FAILURE);
        }
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Does the file exist?
        if (STEER_FileExists(expectedPath))
        {
            // Is the file not empty?
            result = STEER_FileSize(expectedPath, &fileSize);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_CHECK_CONDITION((fileSize > 0), STEER_RESULT_FAILURE);
        }
    }

    // Get name of test program whose report is being validated
    if (result == STEER_RESULT_SUCCESS)
    {
        char* reportJson = NULL;

        result = STEER_ReadTextFile(measuredPath, (uint8_t**)&reportJson);
        if (result == STEER_RESULT_SUCCESS)
        {
            tSTEER_ReportPtr report = NULL;

            result = STEER_JsonToReport(reportJson, &report);
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_DuplicateString(((tSTEER_ReportPrivate*)report)->programName, 
                                               &theProgramName);
                (void)STEER_FreeReport(&report);
            }

            STEER_FreeMemory((void**)&reportJson);
        }
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        char* programPath = NULL;
        size_t len = 0;
        tSTEER_ProcessList* curList = *processList;

        len = strlen(programDirectory) + 
              strlen("/") + 
              strlen(programName) + 1;
        result = STEER_AllocateMemory(len, (void**)&programPath);
        if (result == STEER_RESULT_SUCCESS)
        {
            strcpy(programPath, programDirectory);
            strcat(programPath, "/");
            strcat(programPath, programName);

            if (curList == NULL)
            {
                // Allocate memory for PID list
                result = STEER_AllocateMemory(sizeof(tSTEER_ProcessList) + sizeof(tSTEER_Process),
                                              (void**)&curList);
                if (result == STEER_RESULT_SUCCESS)
                    strcpy(curList->description, "validation check");
            }
            else
            {
                // Reallocate
                len = sizeof(tSTEER_ProcessList) + (sizeof(tSTEER_Process) * curList->count + 1);
                result = STEER_ReallocateMemory(len, len + sizeof(tSTEER_Process), (void**)&curList);
            }   

            if (result == STEER_RESULT_SUCCESS)
            {
                pid_t pid = fork();
                if (pid == -1)
                    result = STEER_CHECK_ERROR(STEER_RESULT_FAILURE);

                else if (pid == 0)
                {
                    result = STEER_CHECK_ERROR(execl(programPath, programPath,
                                                     "-r", measuredPath,
                                                     "-e", expectedPath,
                                                     (char*)0));
                }
                else
                {
                    // The PID is the PID of the forked process; the context is the parent process
                    curList->process[curList->count].pid = pid;
                    strcpy(curList->process[curList->count].programName, theProgramName);
                    curList->count += 1;
                    *processList = curList;

                    STEER_FreeMemory((void**)&theProgramName);
                }
            }

            // Clean up
            STEER_FreeMemory((void**)&programPath);
        }
    }   // cppcheck-suppress memleak
    return result;
}

// =================================================================================================
//  main
// =================================================================================================
int main (int argc, const char * argv[])
{
    int32_t result = STEER_RESULT_SUCCESS;

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
        cJSON* rootObj = NULL;
        char* expandedPath = NULL;
        
        // Define command line options
        static struct option longOptions[] =
        {
            { CONFIG_FILE_PATH_CMD, required_argument,  0, 0 }, // Path to a file containing JSON validation configuration
            { CONFIG_JSON_CMD,      required_argument,  0, 0 }, // JSON validation configuration
            { HELP_CMD,             no_argument,        0, 0 }, // Prints help
            { 0,                    0,                  0, 0 }
        };

        // Parse command line arguments
        while ((cmdLineOption != -1) && (result == STEER_RESULT_SUCCESS))
        {
            cmdLineOption = getopt_long(argc, (char *const *)argv,
                                        kShortOptions, longOptions, &optionIndex);
            
            // Case on command line option
            switch (cmdLineOption)
            {
                case 0: // Long option found

                    // Test config file path
                    if (optionIndex == CONFIG_FILE_PATH_OPTION)
                    {
                        if (strlen(optarg) > 0)
                        {
                            result = STEER_ExpandTildeInPath(optarg, &expandedPath);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                if (expandedPath == NULL)
                                    result = STEER_ReadJsonFromFile(optarg, &rootObj);
                                else
                                {
                                    result = STEER_ReadJsonFromFile(expandedPath, &rootObj);
                                    STEER_FreeMemory((void**)&expandedPath);
                                }
                            }
                        }
                    }

                    // Test config JSON
                    else if (optionIndex == CONFIG_JSON_OPTION)
                        result = STEER_ParseJsonString(optarg, &rootObj);

                    // Help
                    else if (optionIndex == HELP_OPTION)
                    {
                        PrintCmdLineHelp(argv[0]);
                        printf("\n");
                        done = true;
                    }

                    // Unknown
                    else
                    {
                        PrintCmdLineHelp(argv[0]);
                        printf("\n");
                        done = true;
                    }
                    break;

                // Test config file path
                case CONFIG_FILE_PATH_SHORT_CMD:
                {
                    if (strlen(optarg) > 0)
                    {
                        result = STEER_ExpandTildeInPath(optarg, &expandedPath);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            if (expandedPath == NULL)
                                result = STEER_ReadJsonFromFile(optarg, &rootObj);
                            else
                            {
                                result = STEER_ReadJsonFromFile(expandedPath, &rootObj);
                                STEER_FreeMemory((void**)&expandedPath);
                            }
                        }
                    }
                    break;
                }

                // Test config JSON
                case CONFIG_JSON_SHORT_CMD:
                {
                    result = STEER_ParseJsonString(optarg, &rootObj);
                    break;
                }

                // Help
                case HELP_SHORT_CMD:
                    PrintCmdLineHelp(argv[0]);
                    printf("\n");
                    done = true;
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
            if (rootObj != NULL)
            {
                char* programDir = NULL;
                uint32_t programCount = 0;
                tSTEER_FileName* testPrograms = NULL;
                bool available = false;
                tSTEER_ProcessList* processList = NULL;
                uint_fast32_t i = 0;

                // Get the test program directory
                result = STEER_GetProgramDirectory(&programDir);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Scan the directory
                    result = STEER_ScanProgramDirectory(programDir, &programCount,
                                                        &testPrograms);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Is the test program available?
                        available = STEER_ProgramAvailable("steer_validate", 
                                                           programCount,
                                                           testPrograms);
                    }
                }

                // Check status
                if ((result == STEER_RESULT_SUCCESS) && (available == true))
                {
                    time_t startTime = 0;
                    tValidationList* validationList = NULL;

                    startTime = time(&startTime);

                    // Get the validations
                    result = ReadValidations(rootObj, &validationList);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Walk the list
                        for (i = 0; i < validationList->count; i++)
                        {
                            result = SpawnCheck(programDir, "steer_validate",
                                                validationList->validation[i].measuredPath,
                                                validationList->validation[i].expectedPath,
                                                &processList);
                        }
                    }

                    // Check to see whether spawned processes are finished
                    if (processList != NULL)
                    {
                        uint32_t successCount = 0;
                        uint32_t failureCount = 0;
                        time_t stopTime = 0;
                        double diffTimeInSeconds = 0.0;

                        result = STEER_WaitForProcessesToComplete(processList, 1000, &successCount, &failureCount);

                        stopTime = time(&stopTime);
                        diffTimeInSeconds = difftime(stopTime, startTime);

                        fprintf(stdout, "\n");
                        fprintf(stdout, "\t    Total validation programs spawned: %u\n", processList->count);
                        fprintf(stdout, "\t  Total validation programs completed: %u\n", successCount);
                        fprintf(stdout, "\tTotal validation programs with errors: %u\n", failureCount);
                        fprintf(stdout, "\t                 Total execution time: %.0f seconds\n", diffTimeInSeconds);
                        fprintf(stdout, "\n");

                        STEER_FreeMemory((void**)&processList);
                    }

                    if (validationList != NULL)
                    {
                        for (i = 0; i < validationList->count; i++)
                        {
                            STEER_FreeMemory((void**)&(validationList->validation[i].measuredPath));
                            STEER_FreeMemory((void**)&(validationList->validation[i].expectedPath));
                        }
                        STEER_FreeMemory((void**)&validationList);
                    }
                }

                // Clean up
                STEER_FreeMemory((void**)&programDir);
                STEER_FreeMemory((void**)&testPrograms);
                cJSON_Delete(rootObj);
            }
        }
    }
    return result;
}

// =================================================================================================
