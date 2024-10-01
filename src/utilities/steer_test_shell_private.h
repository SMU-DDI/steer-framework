// =================================================================================================
//! @file steer_test_shell_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private test shell functions for the STandard Entropy Evaluation 
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-09
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_TEST_SHELL_PRIVATE_H__
#define __STEER_TEST_SHELL_PRIVATE_H__

#include "steer_types_private.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_HandleConductorCmd (const char* programName,
                                      char* optarg,
                                      bool* exitProgram);

    int32_t STEER_HandleEntropyFilePathCmd (const char* programName,
                                            char* optarg,
                                            bool* exitProgram);

    int32_t STEER_HandleHelpCmd (const char* programName);

    int32_t STEER_HandleNotesCmd (const char* programName,
                                  char* optarg,
                                  bool* exitProgram);

    int32_t STEER_HandleParametersCmd (const char* programName,
                                       char* optarg,
                                       tSTEER_PrivateData* privateData,
                                       bool* exitProgram);

    int32_t STEER_HandleParametersFilePathCmd (const char* programName,
                                               char* optarg,
                                               tSTEER_PrivateData* privateData,
                                               bool* exitProgram);

    int32_t STEER_HandleParametersInfoCmd (tSTEER_PrivateData* privateData);

    int32_t STEER_HandleReportFilePathCmd (const char* programName,
                                           char* optarg,
                                           bool* exitProgram);
    
    int32_t STEER_HandleReportLevelCmd (const char* programName,
                                        char* optarg,
                                        bool* exitProgram);

    int32_t STEER_HandleReportProgressCmd (void);

    int32_t STEER_HandleScheduleIdCmd (const char* programName,
                                       char* optarg,
                                       bool* exitProgram);

    int32_t STEER_HandleTestInfoCmd (tSTEER_PrivateData* privateData);

    int32_t STEER_HandleVerboseCmd (void);

    int32_t STEER_HandleVersionCmd (const char* programName,
                                    tSTEER_PrivateData* privateData);

    int32_t STEER_ParseCommandLineArguments (int argc, 
                                             const char * argv[], 
                                             tSTEER_PrivateData* privateData,
                                             bool* exitProgram);

    int32_t STEER_OpenDataSource (char* inputFilePath,
                                  tSTEER_DataSourceReference* dataSourceRef);

    int32_t STEER_ReadFromDataSource (tSTEER_DataSourceReference* dataSourceRef,
                                      tSTEER_InputFormat inputFormat,
                                      uint64_t bytesToRead, 
                                      uint64_t* bytesRead,
                                      uint64_t* bufferSizeInBytes,
                                      uint8_t** buffer,
                                      uint64_t* numZeros,
                                      uint64_t* numOnes);

    int32_t STEER_CloseDataSource (tSTEER_DataSourceReference* dataSourceRef);

    int32_t STEER_Initialize (tSTEER_PrivateData* privateData,
                              const char* programName,
                              tSTEER_TestGetInfo testGetInfoFunction,
                              tSTEER_TestGetParametersInfo testGetParametersInfoFunction,
                              tSTEER_InputFormat* inputFormat);

    int32_t STEER_Terminate (tSTEER_PrivateData** instance);

    int32_t STEER_EvaluateData (tSTEER_PrivateData* privateData,
                                void* testPrivateData,
                                tSTEER_InputFormat inputFormat,
                                tSTEER_TestExecute testExecuteFunction, 
                                tSTEER_TestFinalize testFinalizeFunction);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_TEST_SHELL_PRIVATE_H__
// =================================================================================================
