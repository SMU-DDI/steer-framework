// =================================================================================================
//! @file steer_test_shell.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the test shell functions for the STandard Entropy Evaluation Report
//! (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-09-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_test_shell.h"
#include "steer_test_shell_private.h"
#include "steer_commands.h"
#include "steer_file_system_utilities.h"
#include "steer_file_system_utilities_private.h"
#include "steer_json_utilities.h"
#include "steer_parameter_set_utilities_private.h"
#include "steer_parameters_info_utilities_private.h"
#include "steer_report_utilities.h"
#include "steer_report_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_string_utilities_private.h"
#include "steer_test_info_utilities_private.h"
#include "steer_utilities.h"
#include "steer_utilities_private.h"
#include "steer_value_utilities.h"
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>

// =================================================================================================
//  Private constants
// =================================================================================================

//  Options array
static const char kSTEER_ShortOptions[] = { ':',
                                            STEER_CONDUCTOR_SHORT_CMD,':',
                                            STEER_ENTROPY_FILE_PATH_SHORT_CMD,':',
                                            STEER_HELP_SHORT_CMD,
                                            STEER_NOTES_SHORT_CMD,':',
                                            STEER_PARAMETERS_SHORT_CMD,':',
                                            STEER_PARAMETERS_FILE_PATH_SHORT_CMD,':',
                                            STEER_PARAMETERS_INFO_SHORT_CMD,
                                            STEER_REPORT_FILE_PATH_SHORT_CMD,':',
                                            STEER_REPORT_LEVEL_SHORT_CMD, ':',
                                            STEER_REPORT_PROGRESS_SHORT_CMD,
                                            STEER_SCHEDULE_ID_SHORT_CMD, ':',
                                            STEER_TEST_INFO_SHORT_CMD,
                                            STEER_VERBOSE_SHORT_CMD,
                                            STEER_VERSION_SHORT_CMD,
                                            0x00 };

// =================================================================================================
//  Private globals
// =================================================================================================
static tSTEER_CliArguments gCliArguments;

// =================================================================================================
//  STEER_HandleConductorCmd
// =================================================================================================
int32_t STEER_HandleConductorCmd (const char* programName,
                                  char* optarg,
                                  bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;

    if (optarg != NULL)
        result = STEER_DuplicateString(optarg, (char**)&(gCliArguments.testConductor));
    else
    {
        (void)STEER_HandleHelpCmd(programName);
        printf("\n");
        *exitProgram = true;
    }
    return result;
}

// =================================================================================================
//  STEER_HandleEntropyFilePathCmd
// =================================================================================================
int32_t STEER_HandleEntropyFilePathCmd (const char* programName,
                                        char* optarg,
                                        bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;

    if (optarg != NULL)
    {
        char* expandedStr = NULL;

        result = STEER_ExpandTildeInPath(optarg, &expandedStr);
        if (result == STEER_RESULT_SUCCESS)
        {
            if (expandedStr == NULL)
                result = STEER_ValidateFileString(optarg, (char**)&(gCliArguments.inputFilePath));
            else
            {
                result = STEER_ValidateFileString(expandedStr, (char**)&(gCliArguments.inputFilePath));
                STEER_FreeMemory((void**)&expandedStr);
            }
        }
    }
    else
    {
        (void)STEER_HandleHelpCmd(programName);
        printf("\n");
        *exitProgram = true;
    }
    return result;
}

// =================================================================================================
//  STEER_HandleHelpCmd
// =================================================================================================
int32_t STEER_HandleHelpCmd (const char* programName)
{
    printf("\nUsage: %s <arguments>\n\n", basename((char*)programName));
    printf("\tAvailable command line arguments are:\n\n");
    printf("\t-%c, --%s <conductor>\t\t\tSets the test conductor running this test.\n", 
           STEER_CONDUCTOR_SHORT_CMD, STEER_CONDUCTOR_CMD);
    printf("\t-%c, --%s <path>\t\t\tSets the path to a file containing test data.\n", 
           STEER_ENTROPY_FILE_PATH_SHORT_CMD, STEER_ENTROPY_FILE_PATH_CMD);
    printf("\t-%c, --%s\t\t\t\t\tPrints this usage notice.\n", 
           STEER_HELP_SHORT_CMD, STEER_HELP_CMD);
    printf("\t-%c, --%s <notes>\t\t\t\tSets caller specified notes for this test run.\n",
           STEER_NOTES_SHORT_CMD, STEER_NOTES_CMD);
    printf("\t-%c, --%s <json>\t\t\t\tSets the test parameters via a JSON structure.\n", 
           STEER_PARAMETERS_SHORT_CMD, STEER_PARAMETERS_CMD);
    printf("\t-%c, --%s <path>\t\tSets the path to a JSON file containing test parameters.\n", 
           STEER_PARAMETERS_FILE_PATH_SHORT_CMD, STEER_PARAMETERS_FILE_PATH_CMD);
    printf("\t-%c, --%s\t\t\t\tGets a JSON structure describing test parameters.\n", 
           STEER_PARAMETERS_INFO_SHORT_CMD, STEER_PARAMETERS_INFO_CMD);
    printf("\t-%c, --%s <path>\t\t\tSets the path for the test report file.\n", 
           STEER_REPORT_FILE_PATH_SHORT_CMD, STEER_REPORT_FILE_PATH_CMD);
    printf("\t-%c, --%s <summary|standard|full>\tSets the detail level for the test report.\n",
           STEER_REPORT_LEVEL_SHORT_CMD, STEER_REPORT_LEVEL_CMD);
    printf("\t-%c, --%s\t\t\t\tPrints testing progress.\n", 
           STEER_REPORT_PROGRESS_SHORT_CMD, STEER_REPORT_PROGRESS_CMD);
    printf("\t-%c, --%s <id>\t\t\t\tSets an optional schedule ID.\n", 
           STEER_SCHEDULE_ID_SHORT_CMD, STEER_SCHEDULE_ID_CMD);
    printf("\t-%c, --%s\t\t\t\t\tGets a JSON structure describing test.\n", 
           STEER_TEST_INFO_SHORT_CMD, STEER_TEST_INFO_CMD);
    printf("\t-%c, --%s\t\t\t\t\tPrints all available status and diagnostic information.\n", 
           STEER_VERBOSE_SHORT_CMD, STEER_VERBOSE_CMD);
    printf("\t-%c, --%s\t\t\t\t\tPrints program version.\n", 
           STEER_VERSION_SHORT_CMD, STEER_VERSION_CMD);
    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  STEER_HandleNotesCmd
// =================================================================================================
int32_t STEER_HandleNotesCmd (const char* programName,
                              char* optarg,
                              bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;

    if (optarg != NULL)
        result = STEER_DuplicateString(optarg, (char**)&(gCliArguments.testNotes));
    else
    {
        (void)STEER_HandleHelpCmd(programName);
        printf("\n");
        *exitProgram = true;
    }
    return result;
}

// =================================================================================================
//  STEER_HandleParametersCmd
// =================================================================================================
int32_t STEER_HandleParametersCmd (const char* programName,
                                   char* optarg,
                                   tSTEER_PrivateData* privateData,
                                   bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;

    if (optarg != NULL)
    {
        result = STEER_DuplicateString(optarg, (char**)&(gCliArguments.parametersJson));
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(optarg, &(privateData->parametersJson));
    }
    else
    {
        (void)STEER_HandleHelpCmd(programName);
        printf("\n");
        *exitProgram = true;
    }
    return result;
}

// =================================================================================================
//  STEER_HandleParametersFilePathCmd
// =================================================================================================
int32_t STEER_HandleParametersFilePathCmd (const char* programName,
                                           char* optarg,
                                           tSTEER_PrivateData* privateData,
                                           bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;

    if (optarg != NULL)
    {
        char* expandedStr = NULL;

        result = STEER_ExpandTildeInPath(optarg, &expandedStr);
        if (result == STEER_RESULT_SUCCESS)
        {
            if (expandedStr == NULL)
                result = STEER_DuplicateString(optarg, (char**)&(gCliArguments.parametersFilePath));
            else
            {
                result = STEER_DuplicateString(expandedStr, (char**)&(gCliArguments.parametersFilePath));
                STEER_FreeMemory((void**)&expandedStr);
            }
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            result = STEER_ReadTextFile(gCliArguments.parametersFilePath, 
                                        (uint8_t**)&(privateData->parametersJson));
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_DuplicateString(privateData->parametersJson, 
                                               (char**)&(gCliArguments.parametersJson));
            }
        }
    }
    else
    {
        (void)STEER_HandleHelpCmd(programName);
        printf("\n");
        *exitProgram = true;
    }
    return result;
}

// =================================================================================================
//  STEER_HandleParametersInfoCmd
// =================================================================================================
int32_t STEER_HandleParametersInfoCmd (tSTEER_PrivateData* privateData)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    if (privateData->parametersInfoJson != NULL)
        printf("%s\n", privateData->parametersInfoJson);

    return result;
}

// =================================================================================================
//  STEER_HandleReportFilePathCmd
// =================================================================================================
int32_t STEER_HandleReportFilePathCmd (const char* programName,
                                       char* optarg,
                                       bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    if (optarg != NULL)
    {
        char* expandedStr = NULL;

        result = STEER_ExpandTildeInPath(optarg, &expandedStr);
        if (result == STEER_RESULT_SUCCESS)
        {
            if (expandedStr == NULL)
                result = STEER_ValidatePathString(optarg, true, 
                                                  (char**)&(gCliArguments.reportFilePath));
            else
            {
                result = STEER_ValidatePathString(expandedStr, true, 
                                                  (char**)&(gCliArguments.reportFilePath));
                STEER_FreeMemory((void**)&expandedStr);
            }
        }
    }
    else
    {
        (void)STEER_HandleHelpCmd(programName);
        printf("\n");
        *exitProgram = true;
    }
    return result;
}

// =================================================================================================
//  STEER_HandleReportLevelCmd
// =================================================================================================
int32_t STEER_HandleReportLevelCmd (const char* programName,
                                    char* optarg,
                                    bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;

    if (optarg != NULL)
    {
        if (strcmp(optarg, STEER_JSON_VALUE_SUMMARY) == 0)
            gCliArguments.reportLevel = eSTEER_ReportLevel_Summary;

        else if (strcmp(optarg, STEER_JSON_VALUE_STANDARD) == 0)
            gCliArguments.reportLevel = eSTEER_ReportLevel_Standard;

        else if (strcmp(optarg, STEER_JSON_VALUE_FULL) == 0)
            gCliArguments.reportLevel = eSTEER_ReportLevel_Full;

        else
        {
            (void)STEER_HandleHelpCmd(programName);
            printf("\n");
            *exitProgram = true;
        }
    }
    else
    {
        (void)STEER_HandleHelpCmd(programName);
        printf("\n");
        *exitProgram = true;
    }
    return result;
}

// =================================================================================================
//  STEER_HandleReportProgressCmd
// =================================================================================================
int32_t STEER_HandleReportProgressCmd (void)
{
    gCliArguments.reportProgress = true;
    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  STEER_HandleScheduleIdCmd
// =================================================================================================
int32_t STEER_HandleScheduleIdCmd (const char* programName,
                                   char* optarg,
                                   bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;

    if (optarg != NULL)
        result = STEER_DuplicateString(optarg, (char**)&(gCliArguments.scheduleId));
    else
    {
        (void)STEER_HandleHelpCmd(programName);
        printf("\n");
        *exitProgram = true;
    }
    return result;
}

// =================================================================================================
//  STEER_HandleTestInfoCmd
// =================================================================================================
int32_t STEER_HandleTestInfoCmd (tSTEER_PrivateData* privateData)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    if (privateData->testInfoJson != NULL)
        printf("%s\n", privateData->testInfoJson);

    return result;
}

// =================================================================================================
//  STEER_HandleVerboseCmd
// =================================================================================================
int32_t STEER_HandleVerboseCmd (void)
{
    gCliArguments.verbose = true;
    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  STEER_HandleVersionCmd
// =================================================================================================
int32_t STEER_HandleVersionCmd (const char* programName,
                                tSTEER_PrivateData* privateData)
{
    printf("%s %s (%s-%s)\n",
           basename((char*)(gCliArguments.programName)),
           privateData->testInfo->programVersion,
           STEER_BUILD_ARCH, STEER_BUILD_OS);

    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  STEER_ParseCommandLineArguments
// =================================================================================================
int32_t STEER_ParseCommandLineArguments (int argc, 
                                         const char * argv[], 
                                         tSTEER_PrivateData* privateData,
                                         bool* exitProgram)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(privateData);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Check for no arguments 
        if (argc == 1)
        {
            (void)STEER_HandleHelpCmd(argv[0]);
            printf("\n");
            *exitProgram = true;
            result = STEER_CHECK_ERROR(EINVAL);
        }
        else    // Parse arguments
        {
            int32_t cmdLineOption = 0;
            int32_t optionIndex = 0;

            // Define command line options
            static struct option longOptions[] =
            {
                { STEER_CONDUCTOR_CMD,              required_argument,  0, 0 }, // Sets the name of the test conductor for the report
                { STEER_ENTROPY_FILE_PATH_CMD,      required_argument,  0, 0 }, // Sets the path to an input file containing random data to test
                { STEER_HELP_CMD,                   no_argument,        0, 0 }, // Prints help
                { STEER_NOTES_CMD,                  required_argument,  0, 0 }, // Sets caller specified notes for this test run
                { STEER_PARAMETERS_CMD,             required_argument,  0, 0 }, // Sets the test parameters via a JSON structure
                { STEER_PARAMETERS_FILE_PATH_CMD,   required_argument,  0, 0 }, // Sets the path to a JSON file containing test parameters
                { STEER_PARAMETERS_INFO_CMD,        no_argument,        0, 0 }, // Gets a JSON structure describing test parameters
                { STEER_REPORT_FILE_PATH_CMD,       required_argument,  0, 0 }, // Sets the path to the test report file
                { STEER_REPORT_LEVEL_CMD,           required_argument,  0, 0 }, // Sets the test conductor running this test
                { STEER_REPORT_PROGRESS_CMD,        no_argument,        0, 0 }, // Prints testing progress
                { STEER_SCHEDULE_ID_CMD,            required_argument,  0, 0 }, // Sets an optional schedule ID
                { STEER_TEST_INFO_CMD,              no_argument,        0, 0 }, // Gets a JSON structure with test information
                { STEER_VERBOSE_CMD,                no_argument,        0, 0 }, // Prints all available status and diagnostic information
                { STEER_VERSION_CMD,                no_argument,        0, 0 }, // Prints program version
                { 0,                                0,                  0, 0 }
            };

            // Required function argument
            result = STEER_CHECK_POINTER(exitProgram);
            if (result == STEER_RESULT_SUCCESS)
                *exitProgram = false;

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Parse command line options
                opterr = 0;
                while ((cmdLineOption != -1) && (result == STEER_RESULT_SUCCESS))
                {
                    cmdLineOption = getopt_long(argc, (char *const *)argv, kSTEER_ShortOptions, 
                                                longOptions, &optionIndex);

                    // Case on command line option
                    switch (cmdLineOption)
                    {
                        case 0: // Long option found

                            // Set conductor
                            if (optionIndex == STEER_CONDUCTOR_OPTION)
                                result = STEER_HandleConductorCmd(argv[0], optarg, exitProgram);

                            // Set entropy file path
                            else if (optionIndex == STEER_ENTROPY_FILE_PATH_OPTION)
                                result = STEER_HandleEntropyFilePathCmd(argv[0], optarg, exitProgram);

                            // Get help
                            else if (optionIndex == STEER_HELP_OPTION)
                            {
                                result = STEER_HandleHelpCmd(argv[0]);
                                printf("\n");
                                *exitProgram = true;
                            }

                            // Set notes 
                            else if (optionIndex == STEER_NOTES_OPTION)
                                result = STEER_HandleNotesCmd(argv[0], optarg, exitProgram);

                            // Set parameters
                            else if (optionIndex == STEER_PARAMETERS_OPTION)
                                result = STEER_HandleParametersCmd(argv[0], optarg, privateData, exitProgram);

                            // Set parameters file path
                            else if (optionIndex == STEER_PARAMETERS_FILE_PATH_OPTION)
                                result = STEER_HandleParametersFilePathCmd(argv[0], optarg, privateData, exitProgram);

                            // Get parameters info
                            else if (optionIndex == STEER_PARAMETERS_INFO_OPTION)
                                result = STEER_HandleParametersInfoCmd(privateData);

                            // Set report file path
                            else if (optionIndex == STEER_REPORT_FILE_PATH_OPTION)
                                result = STEER_HandleReportFilePathCmd(argv[0], optarg, exitProgram);

                            // Set report level
                            else if (optionIndex == STEER_REPORT_LEVEL_OPTION)
                                result = STEER_HandleReportLevelCmd(argv[0], optarg, exitProgram);
                                
                            // Set report progress
                            else if (optionIndex == STEER_REPORT_PROGRESS_OPTION)
                                result = STEER_HandleReportProgressCmd();

                            // Set schedule ID
                            else if (optionIndex == STEER_SCHEDULE_ID_OPTION)
                                result = STEER_HandleScheduleIdCmd(argv[0], optarg, exitProgram);

                            // Get test info
                            else if (optionIndex == STEER_TEST_INFO_OPTION)
                                result = STEER_HandleTestInfoCmd(privateData);

                            // Set verbose
                            else if (optionIndex == STEER_VERBOSE_OPTION)
                                result = STEER_HandleVerboseCmd();

                            // Get version
                            else if (optionIndex == STEER_VERSION_OPTION)
                                result = STEER_HandleVersionCmd(argv[0], privateData);

                            // Unknown
                            else
                            {
                                (void)STEER_HandleHelpCmd(argv[0]);
                                printf("\n");
                                *exitProgram = true;
                            }
                            break;

                        // Set conductor
                        case STEER_CONDUCTOR_SHORT_CMD:
                            result = STEER_HandleConductorCmd(argv[0], optarg, exitProgram);
                            break;
                        
                        // Set entropy file path
                        case STEER_ENTROPY_FILE_PATH_SHORT_CMD:
                            result = STEER_HandleEntropyFilePathCmd(argv[0], optarg, exitProgram);
                            break;

                        // Get help
                        case STEER_HELP_SHORT_CMD:
                            result = STEER_HandleHelpCmd(argv[0]);
                            printf("\n");
                            *exitProgram = true;
                            break;

                        // Set notes
                        case STEER_NOTES_SHORT_CMD:
                            result = STEER_HandleNotesCmd(argv[0], optarg, exitProgram);
                            break;

                        // Set parameters
                        case STEER_PARAMETERS_SHORT_CMD:
                            result = STEER_HandleParametersCmd(argv[0], optarg, privateData, exitProgram);
                            break;

                        // Set parameters file path
                        case STEER_PARAMETERS_FILE_PATH_SHORT_CMD:
                            result = STEER_HandleParametersFilePathCmd(argv[0], optarg, privateData, exitProgram);
                            break;

                        // Get parameters info
                        case STEER_PARAMETERS_INFO_SHORT_CMD:
                            result = STEER_HandleParametersInfoCmd(privateData);
                            break;

                        // Set report file path
                        case STEER_REPORT_FILE_PATH_SHORT_CMD:
                            result = STEER_HandleReportFilePathCmd(argv[0], optarg, exitProgram);
                            break;

                        // Set report level
                        case STEER_REPORT_LEVEL_SHORT_CMD:
                            result = STEER_HandleReportLevelCmd(argv[0], optarg, exitProgram);
                            break;

                        // Set report progress
                        case STEER_REPORT_PROGRESS_SHORT_CMD:
                            result = STEER_HandleReportProgressCmd();
                            break;

                        // Set schedule ID
                        case STEER_SCHEDULE_ID_SHORT_CMD:
                            result = STEER_HandleScheduleIdCmd(argv[0], optarg, exitProgram);
                            break;
                            
                        // Get test info
                        case STEER_TEST_INFO_SHORT_CMD:
                            result = STEER_HandleTestInfoCmd(privateData);
                            break;

                        // Set verbose
                        case STEER_VERBOSE_SHORT_CMD:
                            result = STEER_HandleVerboseCmd();
                            break;

                        // Get version
                        case STEER_VERSION_SHORT_CMD:
                            result = STEER_HandleVersionCmd(argv[0], privateData);
                            break;

                        // Unknown
                        case STEER_UNKNOWN_OR_MISSING_OPTION:
                            (void)STEER_HandleHelpCmd(argv[0]);
                            printf("\n");
                            *exitProgram = true;
                            result = STEER_CHECK_ERROR(EINVAL);
                            break;

                        case STEER_MISSING_ARGUMENT:
                            (void)STEER_HandleHelpCmd(argv[0]);
                            printf("\n");
                            *exitProgram = true;
                            result = STEER_CHECK_ERROR(EINVAL);
                            break;

                        case -1:
                            if (optind < argc)
                            {
                                (void)STEER_HandleHelpCmd(argv[0]);
                                printf("\n");
                                *exitProgram = true;
                                result = STEER_CHECK_ERROR(EINVAL);
                            }
                            break;

                        default:
                            (void)STEER_HandleHelpCmd(argv[0]);
                            printf("\n");
                            *exitProgram = true;
                            result = STEER_CHECK_ERROR(EINVAL);
                            break;
                    }
                }

                // Check status
                if (result != STEER_RESULT_SUCCESS)
                    *exitProgram = true;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_OpenDataSource
// =================================================================================================
int32_t STEER_OpenDataSource (char* inputFilePath,
                              tSTEER_DataSourceReference* dataSourceRef)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(inputFilePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(dataSourceRef);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        dataSourceRef->fileReference = NULL;
        dataSourceRef->deviceReference = -1;
        dataSourceRef->usingStdin = false;

        // Are we reading from stdin (console)?
        if (strcmp(inputFilePath, "stdin") == 0)
        {
            // Read the stdin stream as binary
            (void)freopen(NULL, "rb", stdin);
            if (ferror(stdin))
            {
                // freopen failed
                result = STEER_CHECK_ERROR(errno);
            }
            else
                dataSourceRef->usingStdin = true;
        }

        // Are we reading from a device?
        else if (strncmp(inputFilePath, "/dev/", 5) == 0)
        {
            // Open the device
            int ref = open(inputFilePath, O_RDONLY);
            result = STEER_CHECK_CONDITION((ref >= 0), errno);
            if (result == STEER_RESULT_SUCCESS)
                dataSourceRef->deviceReference = ref;

        }

        // Reading from an input file
        else    
        {
            FILE* fp = NULL;

            // Open the input file as binary
            result = STEER_OpenFile(inputFilePath, false, true, &fp);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Return file reference
                dataSourceRef->fileReference = fp;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ReadFromDataSource
// =================================================================================================
int32_t STEER_ReadFromDataSource (tSTEER_DataSourceReference* dataSourceRef,
                                  tSTEER_InputFormat inputFormat,
                                  uint64_t bytesToRead, 
                                  uint64_t* bytesRead,
                                  uint64_t* bufferSizeInBytes,
                                  uint8_t** buffer,
                                  uint64_t* numZeros,
                                  uint64_t* numOnes)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(dataSourceRef);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((bytesToRead > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(bytesRead);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(bufferSizeInBytes);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(buffer);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(numZeros);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(numOnes);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint8_t* buf = NULL;
        uint64_t byteCount = 0;

        // Setup
        *bytesRead = 0;
        *bufferSizeInBytes = 0;
        *buffer = NULL;
        *numZeros = 0;
        *numOnes = 0;

        // Allocate read buffer
        result = STEER_AllocateMemory(bytesToRead * sizeof(uint8_t), (void**)&buf);
        if (result == STEER_RESULT_SUCCESS)
        {
            if (dataSourceRef->usingStdin)
            {
                // Read from stdin (console)
                byteCount = (uint64_t)fread(buf, sizeof(uint8_t), bytesToRead, stdin);
            }
            else if (dataSourceRef->fileReference != NULL)
            {
                // Read from the file reference
                byteCount = (uint64_t)fread(buf, sizeof(uint8_t), bytesToRead, dataSourceRef->fileReference);
            }
            else if (dataSourceRef->deviceReference != 0)
            {
                // Read from the device reference
                byteCount = (uint64_t)read(dataSourceRef->deviceReference, buf, bytesToRead);
            }

            // Did we get any data?
            if (byteCount > 0)
            {
                uint8_t* conversionBuffer = NULL;
                uint64_t bytesInConversionBuffer = 0;
                uint64_t zeros = 0;
                uint64_t ones = 0;

                if (inputFormat == eSTEER_InputFormat_Bitstream)
                {
                    // Convert to bitstream
                    result = STEER_ConvertBytes(buf, byteCount, true, false, 
                                                &conversionBuffer, 
                                                &bytesInConversionBuffer, 
                                                &zeros, &ones);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Return values
                        *bytesRead = byteCount;
                        *bufferSizeInBytes = bytesInConversionBuffer;
                        *buffer = conversionBuffer;
                        *numZeros = zeros;
                        *numOnes = ones;
                    }

                    // Clean up
                    STEER_FreeMemory((void**)&buf);
                }
                else if (inputFormat == eSTEER_InputFormat_AsciiBitstream)
                {
                    // Convert to ASCII bitstream
                    result = STEER_ConvertBytes(buf, byteCount, true, true, 
                                                &conversionBuffer, 
                                                &bytesInConversionBuffer, 
                                                &zeros, &ones);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Return values
                        *bytesRead = byteCount;
                        *bufferSizeInBytes = bytesInConversionBuffer;
                        *buffer = conversionBuffer;
                        *numZeros = zeros;
                        *numOnes = ones;
                    }

                    // Clean up
                    STEER_FreeMemory((void**)&buf);
                }
                else
                {
                    // Don't convert, but get zero and one counts
                    result = STEER_ConvertBytes(buf, byteCount, false, false, 
                                                &conversionBuffer, 
                                                &bytesInConversionBuffer, 
                                                &zeros, &ones);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Return values
                        *bytesRead = byteCount;
                        *bufferSizeInBytes = byteCount;
                        *buffer = buf;
                        *numZeros = zeros;
                        *numOnes = ones;
                    }
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_CloseDataSource
// =================================================================================================
int32_t STEER_CloseDataSource (tSTEER_DataSourceReference* dataSourceRef)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER(dataSourceRef);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (dataSourceRef->fileReference != NULL)
            result = STEER_CloseFile(&(dataSourceRef->fileReference));

        else if (dataSourceRef->deviceReference != -1)
        {
            result = close(dataSourceRef->deviceReference);
            dataSourceRef->deviceReference = -1;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_Initialize
// =================================================================================================
int32_t STEER_Initialize (tSTEER_PrivateData* privateData,
                          const char* programName,
                          tSTEER_TestGetInfo testGetInfoFunction,
                          tSTEER_TestGetParametersInfo testGetParametersInfoFunction,
                          tSTEER_InputFormat* inputFormat)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(programName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testGetInfoFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testGetParametersInfoFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(privateData);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(inputFormat);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        gCliArguments.reportProgress = false;
        gCliArguments.verbose = false;
        gCliArguments.reportLevel = eSTEER_ReportLevel_Standard;
        *inputFormat = eSTEER_InputFormat_Bitstream;
    
        // Copy the program name
        result = STEER_DuplicateString(programName, (char**)&(gCliArguments.programName));

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get test info 
            privateData->testInfoJson = testGetInfoFunction();
            result = STEER_CHECK_STRING(privateData->testInfoJson);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Parse it
                result = STEER_JsonToTestInfo(privateData->testInfoJson,
                                              &(privateData->testInfo));
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the input format
                    *inputFormat = privateData->testInfo->inputFormat;
                }
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get parameters info
                privateData->parametersInfoJson = testGetParametersInfoFunction();
                result = STEER_CHECK_STRING(privateData->parametersInfoJson);
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_Terminate
// =================================================================================================
int32_t STEER_Terminate (tSTEER_PrivateData** privateData)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    // Check argument
    result = STEER_CHECK_POINTER(privateData);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Clean up
        STEER_FreeMemory((void**)&(gCliArguments.testConductor));
        STEER_FreeMemory((void**)&(gCliArguments.testNotes));
        STEER_FreeMemory((void**)&(gCliArguments.scheduleId));
        STEER_FreeMemory((void**)&(gCliArguments.programPath));
        STEER_FreeMemory((void**)&(gCliArguments.programName));
        STEER_FreeMemory((void**)&(gCliArguments.programVersion));
        STEER_FreeMemory((void**)&(gCliArguments.inputFilePath));
        STEER_FreeMemory((void**)&(gCliArguments.reportFilePath));
        STEER_FreeMemory((void**)&(gCliArguments.parametersFilePath));
        STEER_FreeMemory((void**)&(gCliArguments.parametersJson));

        if (*privateData != NULL)
        {
            (void)STEER_FreeParameterSet(&((*privateData)->parameters));

            STEER_FreeMemory((void**)&((*privateData)->testConductor));
            STEER_FreeMemory((void**)&((*privateData)->testNotes));
            STEER_FreeMemory((void**)&((*privateData)->testInfoJson));
            STEER_FreeMemory((void**)&((*privateData)->parametersInfoJson));
            STEER_FreeMemory((void**)&((*privateData)->parametersJson));

            (void)STEER_FreeParameterSet(&((*privateData)->parameters));
            (void)STEER_FreeTestInfo(&((*privateData)->testInfo));
            
            (void)STEER_FreeReport((tSTEER_ReportPtr*)&((*privateData)->report));

            STEER_FreeMemory((void**)privateData);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_EvaluateData
// =================================================================================================
int32_t STEER_EvaluateData (tSTEER_PrivateData* privateData,
                            void* testPrivateData,
                            tSTEER_InputFormat inputFormat,
                            tSTEER_TestExecute testExecuteFunction, 
                            tSTEER_TestFinalize testFinalizeFunction)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(privateData);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testPrivateData);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testExecuteFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testFinalizeFunction);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_DataSourceReference dataSourceRef;
        char progressStr[STEER_STRING_MAX_LENGTH] = { 0 };

        // Open data source
        if (gCliArguments.inputFilePath == NULL)
            result = STEER_OpenDataSource("stdin", &dataSourceRef);
        else
            result = STEER_OpenDataSource((char*)gCliArguments.inputFilePath, &dataSourceRef);

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            uint8_t* buffer = NULL;
            uint64_t bytesToRead = privateData->bufferSizeInBytes;
            uint64_t bytesRead = 0;
            uint64_t allocatedBufSizeInBytes = 0;
            uint64_t numZeros = 0;
            uint64_t numOnes = 0;
            uint32_t bitstreamId = 0;   
            uint32_t bitstreamsTested = 0;
            char bitstreamIdStr [STEER_STRING_MAX_LENGTH] = { 0 };

            do
            {
                // Read data
                if (gCliArguments.reportProgress)
                {
                    memset((void*)progressStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(progressStr, "Reading bitstream %u...", bitstreamId + 1);
                    STEER_REPORT_PROGRESS(gCliArguments.programName, progressStr);
                }

                // Note that it's the responsibility of the test to free this buffer!
                result = STEER_ReadFromDataSource(&dataSourceRef, inputFormat,
                                                  bytesToRead, &bytesRead,
                                                  &allocatedBufSizeInBytes,
                                                  &buffer, &numZeros, &numOnes);
                if ((result == STEER_RESULT_SUCCESS) && (bytesRead == privateData->bufferSizeInBytes))
                {
                    privateData->suppliedNumberOfBitstreams++;

                    // Update accumulated ones and zeros
                    privateData->accumulatedZeros += numZeros;
                    privateData->accumulatedOnes += numOnes;

                    // Print verbose info
                    if (gCliArguments.verbose)
                    {
                        uint64_t i = 0;

                        // Print out converted bitstream
                        for (i = 0; i < bytesRead; i++)
                        {
                            fprintf(stdout, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, buffer[i]);
                            if (((i + 1) % 4) == 0)
                            {
                                if (i == (bytesRead - 1))
                                    fprintf(stdout, "\n");
                                else
                                    fprintf(stdout, " ");
                            }
                            if (((i + 1) % 64) == 0)
                                fprintf(stdout, "\n");
                        }
                    }

                    // Bump bitstream ID
                    bitstreamId++;

                    // Execute test
                    if (gCliArguments.reportProgress)
                    {
                        memset((void*)progressStr, 0, STEER_STRING_MAX_LENGTH);
                        sprintf(progressStr, "Starting test with bitstream %u...", bitstreamId);
                        STEER_REPORT_PROGRESS(gCliArguments.programName, progressStr);
                    }
                        
                    memset((void*)bitstreamIdStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(bitstreamIdStr, "%u", bitstreamId);
                    result = testExecuteFunction(testPrivateData, bitstreamIdStr,
                                                 buffer, allocatedBufSizeInBytes,
                                                 bytesRead, numZeros, numOnes);
                    if (result == STEER_RESULT_SUCCESS)
                        bitstreamsTested++;
                }
            }
            while ((result == STEER_RESULT_SUCCESS) && 
                    (bytesRead == privateData->bufferSizeInBytes) &&
                    (bitstreamsTested < privateData->bitstreamCount));

            #if STEER_ENABLE_CONSOLE_LOGGING
                if (bitstreamsTested == 1)
                    fprintf(stdout, "\t%u bitstream tested with %s.\n",
                            bitstreamsTested,
                            gCliArguments.programName);
                else
                    fprintf(stdout, "\t%u bitstreams tested with %s.\n",
                            bitstreamsTested,
                            gCliArguments.programName);
            #endif

            // Finalize test
            if (gCliArguments.reportProgress)
                STEER_REPORT_PROGRESS(gCliArguments.programName, "Finalizing test...");

            (void)testFinalizeFunction(&testPrivateData,
                                       privateData->suppliedNumberOfBitstreams);
        }

        if (gCliArguments.reportProgress)
            STEER_REPORT_PROGRESS(gCliArguments.programName, "Test completed.");

        // Close data source
        (void)STEER_CloseDataSource(&dataSourceRef);
    }
    return result;
}

// =================================================================================================
//  STEER_Run
// =================================================================================================
int32_t STEER_Run (const char* programName,
                   int argc, 
                   const char * argv[],
                   tSTEER_TestGetInfo testGetInfoFunction,
                   tSTEER_TestGetParametersInfo testGetParametersInfoFunction,
                   tSTEER_TestInitialize testInitFunction,
                   tSTEER_TestGetConfigurationCount testGetConfigurationCountFunction,
                   tSTEER_TestSetReport testSetReportFunction,
                   tSTEER_TestExecute testExecuteFunction, 
                   tSTEER_TestFinalize testFinalizeFunction)
{
    int32_t result = STEER_RESULT_SUCCESS;
    uint_fast32_t i = 0;

    // Check arguments
    result = STEER_CHECK_STRING(programName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((argc > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(argv);
    if (result == STEER_RESULT_SUCCESS)
    {
        for (i = 0; i < argc; i++)
        {
            result = STEER_CHECK_STRING(argv[i]);
            if (result != STEER_RESULT_SUCCESS)
                break;
        }
    }
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testGetInfoFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testGetParametersInfoFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testInitFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testGetConfigurationCountFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testSetReportFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testExecuteFunction);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testFinalizeFunction);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_PrivateData* privateData = NULL;
        char* timestamp = NULL;
        struct timeval startTimeVal;
        struct utsname info;
        char os[STEER_STRING_MAX_LENGTH] = { 0 };
        bool done = false;
        tSTEER_InputFormat inputFormat = eSTEER_InputFormat_Bitstream;
        void* testPrivateData = NULL;

        memset((void*)&gCliArguments, 0, sizeof(tSTEER_CliArguments));

        // Get the process ID
        gCliArguments.processId = getpid();

        // Get the start timestamp
        result = gettimeofday(&startTimeVal, NULL);
        if (result != 0)
            result = STEER_CHECK_ERROR(errno);
        else
            result = STEER_GetRfc3339Timestamp(&startTimeVal, &timestamp);

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get system info
            memset((void*)&info, 0, sizeof(struct utsname));
            result = STEER_CHECK_ERROR(uname(&info));
            if (result == STEER_RESULT_SUCCESS)
                sprintf(os, "%s %s", info.sysname, info.release);
        }

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            // Allocate space
            result = STEER_AllocateMemory(sizeof(tSTEER_PrivateData), (void**)&privateData);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Initialize the test program
                result = STEER_Initialize(privateData, programName,  
                                          testGetInfoFunction,
                                          testGetParametersInfoFunction,
                                          &inputFormat);
            }
        }

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            // Parse the command line arguments
            result = STEER_ParseCommandLineArguments(argc, argv, privateData, &done);
        }

        // Check status
        if ((result == STEER_RESULT_SUCCESS) && !done)
        {
            // Cache the program path
            result = STEER_DuplicateString(argv[0], (char**)&(gCliArguments.programPath));
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the conductor (optional)
                    if (gCliArguments.testConductor != NULL)
                        result = STEER_DuplicateString(gCliArguments.testConductor,
                                                       &(privateData->testConductor));
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the notes (optional)
                    if (gCliArguments.testNotes != NULL)
                        result = STEER_DuplicateString(gCliArguments.testNotes,
                                                       &(privateData->testNotes));
                }
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                tSTEER_ParametersInfo* paramsInfo = NULL;

                // Get the parameters
                result = STEER_JsonToParametersInfo(privateData->parametersInfoJson,
                                                    &paramsInfo);
                if (result == STEER_RESULT_SUCCESS)
                {
                    result = STEER_JsonToParameterSet(gCliArguments.parametersJson,
                                                      paramsInfo,
                                                      &(privateData->parameters));

                    // Clean up
                    (void) STEER_FreeParametersInfo(&paramsInfo);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    void* nativeValue = NULL;

                    // Get the required parameter native values
                    for (i = 0; i < privateData->parameters->count; i++)
                    {
                        if (strcmp(privateData->parameters->parameter[i].name, 
                                   STEER_JSON_TAG_BITSTREAM_COUNT) == 0)
                        {
                            result = STEER_GetNativeValue(privateData->parameters->parameter[i].dataType,
                                                          privateData->parameters->parameter[i].value,
                                                          &nativeValue);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                privateData->bitstreamCount = *((uint64_t*)nativeValue);
                                STEER_FreeMemory(&nativeValue);
                            }
                        }
                        else if (strcmp(privateData->parameters->parameter[i].name,
                                        STEER_JSON_TAG_BITSTREAM_LENGTH) == 0)
                        {
                            result = STEER_GetNativeValue(privateData->parameters->parameter[i].dataType,
                                                          privateData->parameters->parameter[i].value,
                                                          &nativeValue);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                privateData->bitstreamLength = *((uint64_t*)nativeValue);
                                STEER_FreeMemory(&nativeValue);
                            }
                        }

                        if (result != STEER_RESULT_SUCCESS)
                            break;
                    }
                }

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Initialize test
                    if (gCliArguments.reportProgress)
                    {
                        char progressStr[STEER_STRING_MAX_LENGTH] = { 0 };

                        memset((void*)progressStr, 0, STEER_STRING_MAX_LENGTH);
                        sprintf(progressStr, "Initializing...");
                        STEER_REPORT_PROGRESS(gCliArguments.programName, progressStr);
                    }
                    
                    result = testInitFunction(&gCliArguments,
                                              privateData->parameters,
                                              &testPrivateData,
                                              &(privateData->bufferSizeInBytes));
                }

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Initialize the test report
                    result = STEER_NewReport(privateData->testInfo->name,
                                             privateData->testInfo->suite,
                                             gCliArguments.scheduleId,
                                             privateData->testInfo->description,
                                             gCliArguments.testConductor,
                                             gCliArguments.testNotes,
                                             gCliArguments.reportLevel,
                                             gCliArguments.programName,
                                             privateData->testInfo->programVersion,
                                             os, info.machine,
                                             gCliArguments.inputFilePath,
                                             timestamp, 
                                             privateData->bitstreamCount,
                                             privateData->testInfo,
                                             privateData->parameters,
                                             testGetConfigurationCountFunction(testPrivateData),
                                             (tSTEER_ReportPrivate**)&(privateData->report));
                    if (result == STEER_RESULT_SUCCESS)
                        result = (testSetReportFunction)(testPrivateData, privateData->report);
                }

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Evaluate the random data
                    result = STEER_EvaluateData(privateData, testPrivateData, 
                                                inputFormat, testExecuteFunction,
                                                testFinalizeFunction);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        char* reportJson = NULL;

                        // Get the completion timestamp
                        struct timeval stopTimeVal;
                        result = gettimeofday(&stopTimeVal, NULL);
                        if (result != 0)
                            result = STEER_CHECK_ERROR(errno);

                        // Check status
                        if (result == STEER_RESULT_SUCCESS)
                            result = STEER_GetRfc3339Timestamp(&stopTimeVal, 
                                                               (char**)&(((tSTEER_ReportPrivate*)(privateData->report))->completionTime));

                        // Get the duration
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            result = STEER_GetRfc3339Duration(&startTimeVal,
                                                              &stopTimeVal,
                                                              (char**)&(((tSTEER_ReportPrivate*)(privateData->report))->duration));
                        }

                        // Get the configuration count and walk the evaluations
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)(privateData->report);
                            uint_fast32_t i = 0;
                            uint32_t configCount = reportPrivate->configurations->count;
                            uint32_t passCount = 0;
                            char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };

                            for (i = 0; i < configCount; i++)
                            {
                                if (reportPrivate->configurations->configuration[i].evaluation == eSTEER_Evaluation_Pass)
                                    passCount++;
                            }

                            if (passCount == configCount)
                                reportPrivate->evaluation = eSTEER_Evaluation_Pass;
                            else
                                reportPrivate->evaluation = eSTEER_Evaluation_Fail;
                            if (configCount == 1)
                                sprintf(criterionStr, "%u configuration passes", configCount);
                            else
                                sprintf(criterionStr, "%u configurations pass", configCount);
                            result = STEER_AddCriterionToReport(reportPrivate, criterionStr, 
                                                                (passCount == configCount));
                        }

                        // Check status
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Output test report
                            result = STEER_ReportToJson(privateData->report, gCliArguments.reportLevel, &reportJson);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                cJSON* json = NULL;
                                result = STEER_ParseJsonString(reportJson, &json);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    if (gCliArguments.reportFilePath != NULL)
                                        result = STEER_WriteJsonToFile(json, gCliArguments.reportFilePath);
                                    else
                                        fprintf(stdout, "%s\n", reportJson);

                                    // Clean up
                                    cJSON_Delete(json);
                                    json = NULL;
                                }

                                // Clean up
                                STEER_FreeMemory((void**)&reportJson);
                            }
                        }
                    }
                }
            }
        }

        // Clean up
        STEER_FreeMemory((void**)&timestamp);
        (void)STEER_Terminate(&privateData);
    }
    return result;
}

// =================================================================================================
