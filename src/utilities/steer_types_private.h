// =================================================================================================
//! @file steer_types_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private types for the STandard Entropy Evaluation Report (STEER)
//! framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-26
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_TYPES_PRIVATE_H__
#define __STEER_TYPES_PRIVATE_H__

#include "steer.h"

// =================================================================================================
//  Types
// =================================================================================================

//  Data source reference
typedef struct tsteer_datasourcereference
{
    FILE*   fileReference;
    int     deviceReference;
    bool    usingStdin;
}
tSTEER_DataSourceReference;

//  Value sets
typedef struct tsteer_valuesets
{
    uint32_t            count;
    tSTEER_ValueSet*    valueSet[];
}
tSTEER_ValueSets;

//  Criterion
typedef struct tsteer_criterion
{
    const char* basis;
    bool        result;
}
tSTEER_Criterion;

//  Criteria
typedef struct tsteer_criteria
{
    uint32_t            count;
    tSTEER_Criterion    criterion[];
}
tSTEER_Criteria;

//  Test
typedef struct tsteer_test
{
    uint64_t            testId;
    tSTEER_Values*      calculations;       // Optional; may be NULL
    tSTEER_ValueSets*   calculationSets;    // Optional; may be NULL
    tSTEER_Criteria*    criteria;
    tSTEER_Evaluation   evaluation;
}
tSTEER_Test;

//  Tests
typedef struct tsteer_tests
{
    uint32_t    count;
    tSTEER_Test test[];
}
tSTEER_Tests;

//  Configuration
typedef struct tsteer_configuration
{
    uint64_t            configurationId;
    tSTEER_Values*      attributes;         // Optional; may be NULL
    tSTEER_Tests*       tests;	            // Optional; may be NULL
    tSTEER_Values*      metrics;            // Optional; may be NULL
    tSTEER_ValueSets*   metricSets;         // Optional; may be NULL
    tSTEER_Criteria*    criteria;
    tSTEER_Evaluation   evaluation;
}
tSTEER_Configuration;

//  Configurations
typedef struct tsteer_configurations
{
    uint32_t                count;
    tSTEER_Configuration    configuration[];
}
tSTEER_Configurations;

//  Report
typedef struct tsteer_reportprivate
{
    const char*             testName;
    const char*             testSuite;          // Optional; may be NULL
    const char*             scheduleId;         // Optional; may be NULL
    const char*             testDescription;
    const char*             testConductor;      // Optional; may be NULL
    const char*             testNotes;          // Optional; may be NULL
    tSTEER_ReportLevel      level;
    const char*             programName;
    const char*             programVersion;
    const char*             operatingSystem;
    const char*             architecture;
    const char*             entropySource;
    const char*             startTime;
    struct timeval          startTimeVal;
    const char*             completionTime;
    struct timeval          completionTimeVal;
    const char*             duration;
    tSTEER_ParameterSet*    parameters;
    tSTEER_Configurations*  configurations;     // May be NULL depending on report level
    tSTEER_Criteria*        criteria;
    tSTEER_Evaluation       evaluation;
}
tSTEER_ReportPrivate;

//  Private data
typedef struct tsteer_privatedata
{
    char*                   testConductor;      // Optional; may be NULL
    char*                   testNotes;          // Optional; may be NULL
    char*                   testInfoJson;
    tSTEER_TestInfo*        testInfo;
    char*                   parametersInfoJson;  
    char*                   parametersJson;   
    tSTEER_ParameterSet*    parameters;
    uint64_t                bufferSizeInBytes;
    uint64_t                bitstreamCount;
    uint64_t                bitstreamLength;
    uint64_t                suppliedNumberOfBitstreams;
    uint64_t                accumulatedOnes;
    uint64_t                accumulatedZeros;
    tSTEER_ReportPtr        report;
}
tSTEER_PrivateData;

//  Configuration state
typedef struct tsteer_configurationstate
{
    uint64_t    configurationId;
    uint64_t    accumulatedOnes;
    uint64_t    accumulatedZeros;
    uint64_t    testsRun;
    uint64_t    testsPassed;
    uint64_t    testsFailed;
}
tSTEER_ConfigurationState;

// =================================================================================================
#endif	// __STEER_TYPES_PRIVATE_H__
// =================================================================================================
