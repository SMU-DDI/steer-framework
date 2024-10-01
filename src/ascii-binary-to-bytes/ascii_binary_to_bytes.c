// =================================================================================================
//! @file ascii_binary_to_bytes.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements a program to convert ASCII binary bitstreams to bytes.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-02-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_file_system_utilities.h"
#include "steer_json_utilities.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include <libgen.h>

// =================================================================================================
//  Private constants
// =================================================================================================

//  Buffer sizes
#define BUFFER_SIZE 262144  // 256K

//  Command line options
#define HELP_CMD    "help"
#define CONVERT_CMD "convert"

//  Command line options
#define HELP_OPTION     0
#define CONVERT_OPTION  1

//  Short command line strings
#define HELP_SHORT_CMD      'h'
#define CONVERT_SHORT_CMD   'c'
#define UNKNOWN_SHORT_CMD   '?'

//  Options array
static const char kShortOptions[] = { HELP_SHORT_CMD,
                                      CONVERT_SHORT_CMD, ':',
                                      0x00 };

// =================================================================================================
//  Private prototypes
// =================================================================================================

static void PrintCmdLineHelp (const char* programName);

static int32_t ConvertASCIIBinaryToBytes (uint8_t* asciiBinaryBuffer,
                                          size_t asciiBinarySize,
                                          uint8_t* bytesBuffer,
                                          size_t* bytesSize);

// =================================================================================================
//  PrintCmdLineHelp
// =================================================================================================
void PrintCmdLineHelp (const char* programName)
{
    printf("\nUsage: %s <arguments>\n\n", basename((char*)programName));
    printf("\tAvailable command line arguments are:\n\n");
    printf("\t-%c, --%s <path>\tConvert the ASCII binary file at <path> to a file containing raw bytes.\n", 
           CONVERT_SHORT_CMD, 
           CONVERT_CMD);
    printf("\t-%c, --%s\t\t\tPrints this usage notice.\n", 
           HELP_SHORT_CMD, 
           HELP_CMD);
    return;
}

// =================================================================================================
//  main
// =================================================================================================
int32_t ConvertASCIIBinaryToBytes (uint8_t* asciiBinaryBuffer,
                                   size_t asciiBinarySize,
                                   uint8_t* bytesBuffer,
                                   size_t* bytesSize)
{
    int32_t result = STEER_RESULT_SUCCESS;
    uint8_t byteValue = 0;
    uint_fast32_t i = 0;
    uint32_t j = 0;
    uint32_t inIndex = 0;
    uint32_t outIndex = 0;
    size_t inputSize = asciiBinarySize;

    // Adjust size to be an even multiple of 8
    inputSize = asciiBinarySize - (asciiBinarySize % 8);

    for (i = 0; i < (inputSize / 8); i++)
    {
        byteValue = 0;
        for (j = 0; j < 8; j++)
        {
            inIndex = (8 * i) + j;

            if (asciiBinaryBuffer[inIndex] == 0x30)       // "0"
                byteValue |= (0 << (7 - j));
            else if (asciiBinaryBuffer[inIndex] == 0x31)  // "1"
                byteValue |= (1 << (7 - j));
        }

        bytesBuffer[outIndex] = byteValue;
        outIndex++;
    }

    *bytesSize = outIndex;

    return result;
}

// =================================================================================================
//  main
// =================================================================================================
int main (int argc, const char * argv[])
{
    int32_t result = STEER_RESULT_SUCCESS;
    char* inputFilePath = NULL;
    char* outputFilePath = NULL;

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
        
        // Define command line options
        static struct option longOptions[] =
        {
            { HELP_CMD,     no_argument,        0, 0 }, // Prints help
            { CONVERT_CMD,  required_argument,  0, 0 }, // Path to a file containing ASCII binary
            { 0,            0,                  0, 0 }
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

                    // ASCII binary file path
                    else if (optionIndex == CONVERT_OPTION)
                    {
                        if ((optarg != NULL) && (strlen(optarg) > 0))
                        {
                            if (STEER_FileExists(optarg))
                            {
                                result = STEER_AllocateMemory(strlen(optarg) + 1, 
                                                              (void**)&inputFilePath);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    strcpy(inputFilePath, optarg);
                                    result = STEER_AllocateMemory(strlen(optarg) + strlen(".bin") + 1, 
                                                                  (void**)&outputFilePath);
                                    if (result == STEER_RESULT_SUCCESS)
                                    {
                                        strcpy(outputFilePath, optarg);
                                        strcat(outputFilePath, ".bin");
                                    }
                                }
                            }
                        }
                        else
                        {
                            result = EINVAL;
                            done = true;
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

                // Help
                case HELP_SHORT_CMD:
                    PrintCmdLineHelp(argv[0]);
                    printf("\n");
                    done = true;
                    break;

                // ASCII binary file path
                case CONVERT_SHORT_CMD:
                {
                    if ((optarg != NULL) && (strlen(optarg) > 0))
                    {
                        if (STEER_FileExists(optarg))
                        {
                            result = STEER_AllocateMemory(strlen(optarg) + 1, 
                                                          (void**)&inputFilePath);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                strcpy(inputFilePath, optarg);
                                result = STEER_AllocateMemory(strlen(optarg) + strlen(".bin") + 1, 
                                                              (void**)&outputFilePath);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    strcpy(outputFilePath, optarg);
                                    strcat(outputFilePath, ".bin");
                                }
                            }
                        }
                    }
                    else
                    {
                        result = EINVAL;
                        done = true;
                    }
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
        if (done == false)
        {
            uint8_t* inputBuffer = NULL;
            uint8_t* outputBuffer = NULL;
            FILE* inputFile = NULL;
            FILE* outputFile = NULL;
            size_t inputFileSizeInBytes = 0;

            result = STEER_FileSize(inputFilePath, &inputFileSizeInBytes);
            if ((result == STEER_RESULT_SUCCESS) && (inputFileSizeInBytes > 0))
            {
                result = STEER_AllocateMemory(BUFFER_SIZE, (void**)&inputBuffer);
                if (result == STEER_RESULT_SUCCESS)
                {
                    result = STEER_AllocateMemory(BUFFER_SIZE, (void**)&outputBuffer);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        result = STEER_OpenFile(inputFilePath, false, false, &inputFile);
                        if (result == STEER_RESULT_SUCCESS)
                            result = STEER_OpenFile(outputFilePath, true, true, &outputFile);
                    }
                }
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                size_t bytesRead = 0;
                size_t bytesWritten = 0;
                size_t outputByteCount = 0;

                do
                {
                    bytesRead = fread(inputBuffer, sizeof(uint8_t), BUFFER_SIZE, inputFile);
                    if (bytesRead > 0)
                    {
                        result = ConvertASCIIBinaryToBytes(inputBuffer, bytesRead, 
                                                           outputBuffer, &outputByteCount);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            bytesWritten = fwrite(outputBuffer, sizeof(uint8_t), 
                                                  (size_t)outputByteCount, outputFile);
                            result = STEER_CHECK_CONDITION((bytesWritten == outputByteCount), errno);
                        }
                        else
                            break;
                    }
                }
                while (bytesRead > 0);
            }

            // Clean up
            (void)STEER_CloseFile(&inputFile);
            (void)STEER_CloseFile(&outputFile);
            STEER_FreeMemory((void**)&inputBuffer);
            STEER_FreeMemory((void**)&outputBuffer);
        }
    }

    // Clean up
    STEER_FreeMemory((void*)&inputFilePath);
    STEER_FreeMemory((void*)&outputFilePath);

    return result;
}

// =================================================================================================
