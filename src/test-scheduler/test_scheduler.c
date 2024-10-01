// =================================================================================================
//! @file test_scheduler.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements a program to automate STEER test schedules.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-05-19
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_file_system_utilities.h"
#include "steer_file_system_utilities_private.h"
#include "steer_json_utilities.h"
#include "steer_schedule_utilities.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include <dirent.h>
#include <libgen.h>
#include <time.h>

// =================================================================================================
//  Private constants
// =================================================================================================

//  Command line options
#define HELP_CMD                "help"
#define SCHEDULE_FILE_PATH_CMD  "schedule-file-path"
#define SCHEDULE_JSON_CMD       "schedule-json"

//  Command line options
#define HELP_OPTION                 0
#define SCHEDULE_FILE_PATH_OPTION   2
#define SCHEDULE_JSON_OPTION        3

//  Short command line strings
#define HELP_SHORT_CMD                  'h'
#define SCHEDULE_FILE_PATH_SHORT_CMD    's'
#define SCHEDULE_JSON_SHORT_CMD         'S'
#define UNKNOWN_SHORT_CMD               '?'

//  Options array
static const char kShortOptions[] = { HELP_SHORT_CMD,
                                      SCHEDULE_FILE_PATH_SHORT_CMD, ':',
                                      SCHEDULE_JSON_SHORT_CMD, ':',
                                      0x00 };

//  Default file extensions
static const char* kDefaultRandomDataFileExtension      = STEER_BINARY_FILE_NAME_EXTENSION;
static const char* kDefaultTestParametersFileExtension  = STEER_JSON_FILE_NAME_EXTENSION;
static const char* kDefaultTestReportFileExtension      = STEER_JSON_FILE_NAME_EXTENSION;

// =================================================================================================
//  Private prototypes
// =================================================================================================

static void PrintCmdLineHelp (const char* programName);

static int32_t CreateTestReportPath (const char* scheduleId,
                                     uint32_t testInputIndex,
                                     const char* testReportDirectory,
                                     const char* testProgramName,
                                     const char* testProfileId,
                                     const char* timestamp,
                                     char** testReportPath);

static int32_t SpawnTest (const char* scheduleId,
                          const char* testProgramDirectory,
                          const char* testProgramName,
                          const char* testInputPath,
                          const char* testParametersPath,
                          const char* testReportPath,
                          const char* testConductor,
                          const char* testNotes,
                          tSTEER_ReportLevel testLevel,
                          bool reportProgress,
                          tSTEER_ProcessList** processList);

// =================================================================================================
//  PrintCmdLineHelp
// =================================================================================================
void PrintCmdLineHelp (const char* programName)
{
    printf("\nUsage: %s <arguments>\n\n", basename((char*)programName));
    printf("\tAvailable command line arguments are:\n\n");
    printf("\t-%c, --%s <path>\tPath to a file containing a JSON test schedule.\n", 
           SCHEDULE_FILE_PATH_SHORT_CMD, 
           SCHEDULE_FILE_PATH_CMD);
    printf("\t-%c, --%s <json>\tJSON test schedule.\n", 
           SCHEDULE_JSON_SHORT_CMD, 
           SCHEDULE_JSON_CMD);
    printf("\t-%c, --%s\t\t\tPrints this usage notice.\n", 
           HELP_SHORT_CMD, 
           HELP_CMD);
    return;
}

// =================================================================================================
//  CreateTestReportPath
// =================================================================================================
int32_t CreateTestReportPath (const char* scheduleId,
                              uint32_t testInputIndex,
                              const char* testReportDirectory,
                              const char* testProgramName,
                              const char* testProfileId,
                              const char* timestamp,
                              char** testReportPath)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(testReportDirectory);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testProgramName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testProfileId);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(timestamp);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testReportPath);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        size_t len = 0;
        bool needsSlash = false;
        char inputIndexStr[STEER_STRING_MAX_LENGTH] = { 0 };

        sprintf(inputIndexStr, "%u", testInputIndex);

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            len = strlen(testReportDirectory);
            needsSlash = (testReportDirectory[len - 1] != '/') ? true : false;
            len += strlen(testProgramName) + 
                strlen("_profile_") +
                strlen(testProfileId) + 
                strlen("_") +
                strlen(inputIndexStr) + 
                strlen("_") +
                strlen(timestamp) + 
                strlen(kDefaultTestReportFileExtension) + 1;
            if ((scheduleId != NULL) && (strlen(scheduleId) > 0))
                len += (strlen("schedule_") + strlen(scheduleId) + strlen("_"));    // cppcheck-suppress uninitvar
            if (needsSlash == true)
                len += strlen("/");

            result = STEER_AllocateMemory(len, (void**)testReportPath);
            if (result == STEER_RESULT_SUCCESS)
            {
                strcpy(*testReportPath, testReportDirectory);
                if (needsSlash == true)
                    strcat(*testReportPath, "/");
                if ((scheduleId != NULL) && (strlen(scheduleId) > 0))
                {
                    strcat(*testReportPath, "schedule_");
                    strcat(*testReportPath, scheduleId);
                    strcat(*testReportPath, "_");
                }
                strcat(*testReportPath, testProgramName);
                strcat(*testReportPath, "_profile_");
                strcat(*testReportPath, testProfileId);
                strcat(*testReportPath, "_");
                strcat(*testReportPath, inputIndexStr);
                strcat(*testReportPath, "_");
                strcat(*testReportPath, timestamp);
                strcat(*testReportPath, kDefaultTestReportFileExtension);
            }
        }
    }
    return result;
}

// =================================================================================================
//  SpawnTest
// =================================================================================================
int32_t SpawnTest (const char* scheduleId,
                   const char* testProgramDirectory,
                   const char* testProgramName,
                   const char* testInputPath,
                   const char* testParametersPath,
                   const char* testReportPath,
                   const char* testConductor,
                   const char* testNotes,
                   tSTEER_ReportLevel testLevel,
                   bool reportProgress,
                   tSTEER_ProcessList** processList)
{
    int32_t result = STEER_RESULT_SUCCESS;
    size_t fileSize = 0;
    bool hasScheduleId = false;
    bool hasConductor = false;
    bool hasNotes = false;
    bool hasParameters = (testParametersPath != NULL);

    // Check arguments
    result = STEER_CHECK_STRING(testProgramDirectory);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testProgramName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testInputPath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testReportPath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(processList);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (hasParameters)
            result = STEER_CHECK_STRING(testParametersPath);
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        if ((scheduleId != NULL) && (strlen(scheduleId) > 0))
            hasScheduleId = true;
        if ((testConductor != NULL) && (strlen(testConductor) > 0))
            hasConductor = true;
        if ((testNotes != NULL) && (strlen(testNotes) > 0))
            hasNotes = true;
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Is this a device?
        if (!STEER_IsReadableDevicePath(testInputPath))
        {
            // No, it's a file; does the file exist?
            if (STEER_FileExists(testInputPath))
            {
                // Is the file not empty?
                result = STEER_FileSize(testInputPath, &fileSize);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_CHECK_CONDITION((fileSize > 0), STEER_RESULT_FAILURE);
            }
        }
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        if (hasParameters)
        {
            // Does the file exist?
            if (STEER_FileExists(testParametersPath))
            {
                // Is the file not empty?
                result = STEER_FileSize(testParametersPath, &fileSize);
                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_CHECK_CONDITION((fileSize > 0), STEER_RESULT_FAILURE);
            }
        }
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        char* testProgramPath = NULL;
        size_t len = 0;
        tSTEER_ProcessList* curList = *processList;

        len = strlen(testProgramDirectory) + 
              strlen("/") + 
              strlen(testProgramName) + 1;
        result = STEER_AllocateMemory(len, (void**)&testProgramPath);
        if (result == STEER_RESULT_SUCCESS)
        {
            char levelStr[16] = { 0 };

            if (testLevel == eSTEER_ReportLevel_Full)
                strcpy(levelStr, "full");
            else if (testLevel == eSTEER_ReportLevel_Standard)
                strcpy(levelStr, "standard");
            else if (testLevel == eSTEER_ReportLevel_Summary)
                strcpy(levelStr, "summary");

            strcpy(testProgramPath, testProgramDirectory);
            if (testProgramPath[strlen(testProgramPath) - 1] != '/')
                strcat(testProgramPath, "/");
            strcat(testProgramPath, testProgramName);

            if (curList == NULL)
            {
                // Allocate memory for PID list
                result = STEER_AllocateMemory(sizeof(tSTEER_ProcessList) + sizeof(tSTEER_Process),
                                              (void**)&curList);
                if (result == STEER_RESULT_SUCCESS)
                    strcpy(curList->description, "validation test run");
            }
            else
            {
                // Reallocate
                len = sizeof(tSTEER_ProcessList) + (sizeof(tSTEER_Process) * curList->count);
                result = STEER_ReallocateMemory(len, len + sizeof(tSTEER_Process), (void**)&curList);
            }   

            if (result == STEER_RESULT_SUCCESS)
            {
                pid_t pid = fork();
                if (pid == -1)
                    result = STEER_RESULT_FAILURE;

                else if (pid == 0)
                {
                    if (hasParameters)
                    {
                        if (hasConductor && hasNotes && hasScheduleId)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-n", testNotes,
                                                                 "-s", scheduleId,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-n", testNotes,
                                                                 "-s", scheduleId,
                                                                 (char*)0));
                            }
                        }
                        else if (hasConductor && hasNotes)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-n", testNotes,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-n", testNotes,
                                                                 (char*)0));
                            }
                        }
                        else if (hasConductor && hasScheduleId)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-s", scheduleId,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-s", scheduleId,
                                                                 (char*)0));
                            }
                        }
                        else if (hasNotes && hasScheduleId)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-s", scheduleId,
                                                                 "-n", testNotes,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-s", scheduleId,
                                                                 "-n", testNotes,
                                                                 (char*)0));
                            }
                        }
                        else if (hasConductor)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 (char*)0));
                            }
                        }
                        else if (hasNotes)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-n", testNotes,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-n", testNotes,
                                                                 (char*)0));
                            }
                        }
                        else if (hasScheduleId)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-s", scheduleId,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-s", scheduleId,
                                                                 (char*)0));
                            }
                        }
                        else
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-p", testParametersPath,
                                                                 "-r", testReportPath,
                                                                 (char*)0));
                            }
                        }
                    }
                    else
                    {
                        if (hasConductor && hasNotes && hasScheduleId)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-n", testNotes,
                                                                 "-s", scheduleId,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-n", testNotes,
                                                                 "-s", scheduleId,
                                                                 (char*)0));
                            }
                        }
                        else if (hasConductor && hasNotes)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-n", testNotes,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-n", testNotes,
                                                                 (char*)0));
                            }
                        }
                        else if (hasConductor && hasScheduleId)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-s", scheduleId,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-s", scheduleId,
                                                                 (char*)0));
                            }
                        }
                        else if (hasNotes && hasScheduleId)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-s", scheduleId,
                                                                 "-n", testNotes,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-s", scheduleId,
                                                                 "-n", testNotes,
                                                                 (char*)0));
                            }
                        }
                        else if (hasConductor)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-c", testConductor,
                                                                 (char*)0));
                            }
                        }
                        else if (hasNotes)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-n", testNotes,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-n", testNotes,
                                                                 (char*)0));
                            }
                        }
                        else if (hasScheduleId)
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-s", scheduleId,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-s", scheduleId,
                                                                 (char*)0));
                            }
                        }
                        else
                        {
                            if (reportProgress)
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 "-R", (char*)0));
                            }
                            else
                            {
                                result = STEER_CHECK_ERROR(execl(testProgramPath,
                                                                 testProgramPath,
                                                                 "-l", levelStr,
                                                                 "-e", testInputPath,
                                                                 "-r", testReportPath,
                                                                 (char*)0));
                            }
                        }
                    }
                }
                else
                {
                    // The PID is the PID of the forked process; the context is the parent process
                    curList->process[curList->count].pid = pid;
                    strcpy(curList->process[curList->count].programName, testProgramName);
                    curList->count += 1;
                    *processList = curList;
                }
            }

            // Clean up
            STEER_FreeMemory((void**)&testProgramPath);
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
        char* scheduleJson = NULL;
        char* expandedPath = NULL;
        
        // Define command line options
        static struct option longOptions[] =
        {
            { HELP_CMD,                 no_argument,        0, 0 }, // Prints help
            { SCHEDULE_FILE_PATH_CMD,   required_argument,  0, 0 }, // Path to a file containing JSON test schedule
            { SCHEDULE_JSON_CMD,        required_argument,  0, 0 }, // JSON test schedule
            { 0,                        0,                  0, 0 }
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

                    // Help
                    if (optionIndex == HELP_OPTION)
                    {
                        PrintCmdLineHelp(argv[0]);
                        printf("\n");
                        done = true;
                    }

                    // Test schedule file path
                    else if (optionIndex == SCHEDULE_FILE_PATH_OPTION)
                    {
                        if (strlen(optarg) > 0)
                        {
                            result = STEER_ExpandTildeInPath(optarg, &expandedPath);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                if (expandedPath == NULL)
                                    result = STEER_ReadTextFile(optarg, (uint8_t**)&scheduleJson);
                                else
                                {
                                    result = STEER_ReadTextFile(expandedPath, (uint8_t**)&scheduleJson);
                                    STEER_FreeMemory((void**)&expandedPath);
                                }
                            }
                        }
                    }

                    // Test schedule JSON
                    else if (optionIndex == SCHEDULE_JSON_OPTION)
                        result = STEER_DuplicateString(optarg, &scheduleJson);

                    // Unknown
                    else
                    {
                        PrintCmdLineHelp(argv[0]);
                        printf("\n");
                        done = true;
                    }
                    break;

                // Help
                case HELP_SHORT_CMD:
                    PrintCmdLineHelp(argv[0]);
                    printf("\n");
                    done = true;
                    break;

                // Test schedule file path
                case SCHEDULE_FILE_PATH_SHORT_CMD:
                {
                    if (strlen(optarg) > 0)
                    {
                        result = STEER_ExpandTildeInPath(optarg, &expandedPath);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            if (expandedPath == NULL)
                                result = STEER_ReadTextFile(optarg, (uint8_t**)&scheduleJson);
                            else
                            {
                                result = STEER_ReadTextFile(expandedPath, (uint8_t**)&scheduleJson);
                                STEER_FreeMemory((void**)&expandedPath);
                            }
                        }
                    }
                    break;
                }

                // Test schedule JSON
                case SCHEDULE_JSON_SHORT_CMD:
                {
                    result = STEER_DuplicateString(optarg, &scheduleJson);
                    break;
                }

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
            result = STEER_CHECK_POINTER(scheduleJson);
            if (result == STEER_RESULT_SUCCESS)
            {
                tSTEER_Schedule* schedule = NULL;

                // Convert the schedule JSON
                result = STEER_JsonToSchedule(scheduleJson, &schedule);
                if (result == STEER_RESULT_SUCCESS)
                {
                    time_t startTime = 0;
                    char* programDir = NULL;
                    uint32_t testProgramCount = 0;
                    tSTEER_FileName* testPrograms = NULL;
                    bool testProgramAvailable = false;
                    uint_fast32_t i = 0;
                    uint_fast32_t j = 0;
                    uint_fast32_t ii = 0;
                    uint_fast32_t jj = 0;
                    tSTEER_ScheduledTest* testPtr = NULL;
                    tSTEER_Profile* profilePtr = NULL;
                    tSTEER_ProcessList* processList = NULL;
                    uint32_t testInputCount = 0;
                    tSTEER_FilePath* testInputList = NULL;
                    uint32_t testParametersCount = 0;
                    tSTEER_FilePath* testParametersList = NULL;
                    char* timestamp = NULL;
                    char* testReportPath = NULL;

                    // Mark the start time
                    startTime = time(&startTime);

                    // Get the test program directory
                    result = STEER_GetProgramDirectory(&programDir);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Scan the directory
                        result = STEER_ScanProgramDirectory(programDir,
                                                            &testProgramCount,
                                                            &testPrograms);
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Walk the tests
                        for (i = 0; i < schedule->count; i++)
                        {
                            testPtr = &(schedule->tests[i]);

                            // Is the test program available?
                            testProgramAvailable = STEER_ProgramAvailable(testPtr->programName,
                                                                          testProgramCount,
                                                                          testPrograms);
                            if (testProgramAvailable)
                            {
                                // Walk the test profiles
                                for (j = 0; j < testPtr->profiles->count; j++)
                                {
                                    profilePtr = &(testPtr->profiles->profile[j]);

                                    // Are we dealing with directories or files?
                                    if (profilePtr->specifiesDirectories)
                                    {
                                        // Scan the test input directory
                                        result = STEER_ScanDirectoryForFilesWithExtension(profilePtr->input,
                                                                                          kDefaultRandomDataFileExtension,
                                                                                          &testInputCount,
                                                                                          &testInputList);
                                        if (result == STEER_RESULT_SUCCESS)
                                        {
                                            if (profilePtr->parameters != NULL)
                                            {
                                                // Scan the test parameters directory
                                                result = STEER_ScanDirectoryForFilesWithExtension(profilePtr->parameters,
                                                                                                  kDefaultTestParametersFileExtension,
                                                                                                  &testParametersCount,
                                                                                                  &testParametersList);
                                            }
                                            else
                                                testParametersCount = 1;    // We're using default parameters
                                        }

                                        if (result == STEER_RESULT_SUCCESS)
                                        {
                                            // Walk the inputs
                                            for (ii = 0; ii < testInputCount; ii++)
                                            {
                                                // Walk the parameters
                                                for (jj = 0; jj < testParametersCount; jj++)
                                                {
                                                    // Create the test report path
                                                    result = STEER_GetTimestampString(true, false, &timestamp);
                                                    if (result == STEER_RESULT_SUCCESS)
                                                    {
                                                        result = CreateTestReportPath(schedule->scheduleId, ii,
                                                                                      profilePtr->report,
                                                                                      testPtr->programName,
                                                                                      profilePtr->id,
                                                                                      timestamp,
                                                                                      &testReportPath);
                                                    }

                                                    if (result == STEER_RESULT_SUCCESS)
                                                    {
                                                        if (testParametersList != NULL)
                                                        {
                                                            result = SpawnTest(schedule->scheduleId, programDir,
                                                                               testPtr->programName, testInputList[ii],
                                                                               testParametersList[jj], testReportPath,
                                                                               schedule->conductor, schedule->notes,
                                                                               schedule->level, schedule->reportProgress,
                                                                               &processList);
                                                        }
                                                        else
                                                        {
                                                            result = SpawnTest(schedule->scheduleId, programDir,
                                                                               testPtr->programName, testInputList[ii],
                                                                               NULL, testReportPath,
                                                                               schedule->conductor, schedule->notes,
                                                                               schedule->level, schedule->reportProgress,
                                                                               &processList);
                                                        }
                                                    }

                                                    if (result != STEER_RESULT_SUCCESS)
                                                        break;
                                                }

                                                if (result != STEER_RESULT_SUCCESS)
                                                    break;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // This is a single set of files
                                        result = SpawnTest(schedule->scheduleId, programDir,
                                                           testPtr->programName, profilePtr->input,
                                                           profilePtr->parameters, profilePtr->report,
                                                           schedule->conductor, schedule->notes,
                                                           schedule->level, schedule->reportProgress,
                                                           &processList);
                                    }

                                    if (result != STEER_RESULT_SUCCESS)
                                        break;
                                }
                            }

                            if (result != STEER_RESULT_SUCCESS)
                                break;
                        }
                    }

                    // Wait for all spawned processes to complete
                    if (processList != NULL)
                    {
                        uint32_t successCount = 0;
                        uint32_t failureCount = 0;
                        time_t stopTime = 0;
                        double diffTimeInSeconds = 0.0;

                        result = STEER_WaitForProcessesToComplete(processList, 1000,
                                                                  &successCount,
                                                                  &failureCount);

                        // Mark the test stop time
                        stopTime = time(&stopTime);
                        diffTimeInSeconds = difftime(stopTime, startTime);

                        fprintf(stdout, "\n");
                        fprintf(stdout, "\t    Total test programs spawned: %u\n", processList->count);
                        fprintf(stdout, "\t  Total test programs completed: %u\n", successCount);
                        fprintf(stdout, "\tTotal test programs with errors: %u\n", failureCount);
                        fprintf(stdout, "\t           Total execution time: %.0f seconds\n", diffTimeInSeconds);
                        fprintf(stdout, "\n");

                        // Clean up
                        STEER_FreeMemory((void**)&processList);
                    }

                    // Clean up
                    STEER_FreeMemory((void**)&programDir);
                    STEER_FreeMemory((void**)&testPrograms);
                }

                // Clean up
                STEER_FreeSchedule(&schedule);
                STEER_FreeMemory((void**)&scheduleJson);
            }
        }
    }
    return result;
}

// =================================================================================================
