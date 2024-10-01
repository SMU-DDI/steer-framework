// =================================================================================================
//! @file steer_test_shell.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public test shell functions for the STandard Entropy Evaluation 
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-09-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_TEST_SHELL_H__
#define __STEER_TEST_SHELL_H__

#include "steer.h"
#include "cJSON.h"
#include <sys/types.h>

// =================================================================================================
//  Types
// =================================================================================================

// Command line interface arguments
typedef struct tsteer_cliarguments
{
    const char*         testConductor;
    const char*         testNotes;
    const char*         scheduleId;
    const char*         programPath;
    const char*         programName;
    const char*         programVersion;
    const char*         inputFilePath;
    const char*         reportFilePath;
    const char*         parametersFilePath;
    const char*         parametersJson;
    pid_t               processId;
    tSTEER_ReportLevel  reportLevel;
    bool                reportProgress;
    bool                verbose;
}
tSTEER_CliArguments;

// =================================================================================================
//  Callbacks
// =================================================================================================

typedef char* (*tSTEER_TestGetInfo) (void);

typedef char* (*tSTEER_TestGetParametersInfo) (void);

typedef int32_t (*tSTEER_TestInitialize) (tSTEER_CliArguments* cliArguments,
                                          tSTEER_ParameterSet* parameters,
                                          void** testPrivateData,
                                          uint64_t* bufferSizeInBytes);

typedef uint32_t (*tSTEER_TestGetConfigurationCount) (void* testPrivateData);

typedef int32_t (*tSTEER_TestSetReport) (void* testPrivateData,
                                         tSTEER_ReportPtr report);

typedef int32_t (*tSTEER_TestExecute) (void* testPrivateData,
                                       const char* bitstreamId,
                                       uint8_t* buffer,
                                       uint64_t bufferSizeInBytes,
                                       uint64_t bytesInBuffer,
                                       uint64_t numZeros,
                                       uint64_t numOnes);

typedef int32_t (*tSTEER_TestFinalize) (void** testPrivateData,
                                        uint64_t suppliedNumberOfBitstreams);

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_Run (const char* programName,
                       int argc, 
                       const char * argv[],
                       tSTEER_TestGetInfo getInfoFunction,
                       tSTEER_TestGetParametersInfo getParametersInfoFunction,
                       tSTEER_TestInitialize initFunction,
                       tSTEER_TestGetConfigurationCount getConfigurationCountFunction,
                       tSTEER_TestSetReport setReportFunction,
                       tSTEER_TestExecute executeFunction, 
                       tSTEER_TestFinalize finalizeFunction);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_TEST_SHELL_H__
// =================================================================================================
