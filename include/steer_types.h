// =================================================================================================
//! @file steer_types.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public types for the STandard Entropy Evaluation Report (STEER)
//! framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-15
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_TYPES_H__
#define __STEER_TYPES_H__

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// =================================================================================================
//  Constants
// =================================================================================================

#define NIST_STS_NAME                       "NIST Statistical Test Suite (STS)"
#define NIST_STS_AUTHOR_JUAN_SOTO           "Juan Soto (NIST)"
#define NIST_STS_AUTHOR_LARRY_BASSHAM       "Larry Bassham (NIST)"
#define STEER_AUTHOR_GARY_WOODCOCK          "Gary Woodcock (Anametric)"
#define STEER_MAINTAINER_ANAMETRIC          "Anametric, Inc."
#define STEER_MAINTAINER_SMU_DARWIN_DEASON  "SMU Darwin Deason Institute for Cyber Security"
#define STEER_CONTRIBUTOR_GARY_WOODCOCK     "Gary Woodcock (Anametric)"
#define STEER_CONTRIBUTOR_ALEX_MAGYARI      "Alex Magyari (Anametric)"
#define STEER_CONTRIBUTOR_JOSHUA_SYLVESTER    "Joshua Sylvester (SMU)"
#define STEER_CONTRIBUTOR_JESSIE_HENDERSON   "Jessie Henderson (SMU)"
#define STEER_CONTRIBUTOR_MICAH_THORNTON    "Micah Thornton"
#define STEER_CONTRIBUTOR_MITCH_THORNTON    "Mitch Thornton (SMU)"
#define STEER_CONTRIBUTOR_NATHAN_CONRAD     "Nathan Conrad (Anametric)"
#define STEER_REPOSITORY_URI                "https://github.com/steer-framework.git"
#define STEER_CONTACT                       "info@steer-framework.dev"

//  STEER success and failure codes
#define STEER_RESULT_SUCCESS                                            EXIT_SUCCESS
#define STEER_RESULT_RESERVED_START                                     -1
#define STEER_RESULT_FAILURE                                            STEER_RESULT_RESERVED_START
#define STEER_RESULT_OUT_OF_RANGE                                       (STEER_RESULT_RESERVED_START - 1)
#define STEER_RESULT_EMPTY_STRING                                       (STEER_RESULT_RESERVED_START - 2)
#define STEER_RESULT_NOT_A_FILE                                         (STEER_RESULT_RESERVED_START - 3)
#define STEER_RESULT_EMPTY_FILE                                         (STEER_RESULT_RESERVED_START - 4)
#define STEER_RESULT_NOT_ENOUGH_BYTES_READ                              (STEER_RESULT_RESERVED_START - 5)
#define STEER_RESULT_NOT_ENOUGH_BYTES_WRITTEN                           (STEER_RESULT_RESERVED_START - 6)
#define STEER_RESULT_BUFFER_TOO_SMALL                                   (STEER_RESULT_RESERVED_START - 7)
#define STEER_RESULT_BUFFER_LENGTH_MISMATCH                             (STEER_RESULT_RESERVED_START - 8)
#define STEER_RESULT_INVALID_TIME                                       (STEER_RESULT_RESERVED_START - 9)
#define STEER_RESULT_VALIDATION_CHECK_FAILURE                           (STEER_RESULT_RESERVED_START - 10)
#define STEER_RESULT_JSON_PARSE_FAILURE                                 (STEER_RESULT_RESERVED_START - 11)
#define STEER_RESULT_JSON_TAG_NOT_FOUND                                 (STEER_RESULT_RESERVED_START - 12)
#define STEER_RESULT_JSON_OPERATION_FAILURE                             (STEER_RESULT_RESERVED_START - 13)
#define STEER_RESULT_JSON_INVALID_CONSTRUCTION                          (STEER_RESULT_RESERVED_START - 14)
#define STEER_RESULT_RESERVED_END                                       -1023

//  NIST success and failure codes
#define STEER_RESULT_NIST_RESERVED_START                                (STEER_RESULT_RESERVED_END - 1)
#define STEER_RESULT_NIST_RESERVED_END                                  -2047
#define NIST_RESULT_BLOCK_LENGTH_GT_RECOMMENDED_BLOCK_LENGTH            STEER_RESULT_NIST_RESERVED_START
#define NIST_RESULT_NUM_CYCLES_GT_MAX                                   (STEER_RESULT_NIST_RESERVED_START - 1)
#define NIST_RESULT_NUM_CYCLES_LT_REJECTION_CONSTRAINT                  (STEER_RESULT_NIST_RESERVED_START - 2)
#define NIST_RESULT_NUM_MATRICES_IS_ZERO                                (STEER_RESULT_NIST_RESERVED_START - 3)
#define NIST_RESULT_FABS_PI_MINUS_PT_5_GT_2_OVER_SQRT_BITSTREAM_LENGTH  (STEER_RESULT_NIST_RESERVED_START - 4)
#define NIST_RESULT_BLOCK_LENGTH_L_LT_MIN                               (STEER_RESULT_NIST_RESERVED_START - 5)
#define NIST_RESULT_BLOCK_LENGTH_L_GT_MAX                               (STEER_RESULT_NIST_RESERVED_START - 6)
#define NIST_RESULT_NUM_BLOCKS_IN_INIT_SEQ_LT_MIN                       (STEER_RESULT_NIST_RESERVED_START - 7)

//  Lengths
#define STEER_STRING_MAX_LENGTH                                         256
#define STEER_PATH_MAX_LENGTH                                           1024
#define STEER_FILE_NAME_MAX_LENGTH                                      256
#define STEER_PROCESS_DESCRIPTION_MAX_LENGTH                            64

//  Formats and precision
#define STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT                      "%d"
#define STEER_DEFAULT_UNSIGNED_INTEGER_STRING_FORMAT                    "%u"
#define STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT                      "%.*f"
#define STEER_DEFAULT_FLOATING_POINT_PRECISION                          6

// =================================================================================================
//  Types
// =================================================================================================

//  Standard file and directory path types
typedef char tSTEER_FilePath[STEER_PATH_MAX_LENGTH];
typedef char tSTEER_FileName[STEER_FILE_NAME_MAX_LENGTH];
typedef char tSTEER_DirectoryPath[STEER_PATH_MAX_LENGTH];

//  Process information
typedef struct tsteer_process
{
    tSTEER_FileName programName;
    pid_t           pid;
}
tSTEER_Process;

//  Process list
typedef struct tsteer_processlist
{
    uint32_t        count;
    char            description[STEER_PROCESS_DESCRIPTION_MAX_LENGTH];
    tSTEER_Process  process[];
}
tSTEER_ProcessList;

//  Data types
typedef enum tsteer_datatype
{
    eSTEER_DataType_Boolean                         = 0,
    eSTEER_DataType_DoublePrecisionFloatingPoint    = 1,
    eSTEER_DataType_ExtendedPrecisionFloatingPoint  = 2,
    eSTEER_DataType_Signed8BitInteger               = 3,
    eSTEER_DataType_Signed16BitInteger              = 4,
    eSTEER_DataType_Signed32BitInteger              = 5,
    eSTEER_DataType_Signed64BitInteger              = 6,
    eSTEER_DataType_SinglePrecisionFloatingPoint    = 7,
    eSTEER_DataType_Unsigned8BitInteger             = 8,
    eSTEER_DataType_Unsigned16BitInteger            = 9,
    eSTEER_DataType_Unsigned32BitInteger            = 10,
    eSTEER_DataType_Unsigned64BitInteger            = 11,
    eSTEER_DataType_UTF8String                      = 12
}
tSTEER_DataType;

//  Test complexity
typedef enum tsteer_complexity
{
    eSTEER_Complexity_Simple    = 1,
    eSTEER_Complexity_Average   = 2,
    eSTEER_Complexity_Moderate  = 3
}
tSTEER_Complexity;

//  Bitstream input formats
typedef enum tsteer_inputformat
{
    eSTEER_InputFormat_AsciiBitstream   = 0,
    eSTEER_InputFormat_Bitstream        = 1
}
tSTEER_InputFormat;

//  Evaluations
typedef enum tsteer_evaluation
{
    eSTEER_Evaluation_Fail          = 0,
    eSTEER_Evaluation_Inconclusive  = 1,
    eSTEER_Evaluation_Pass          = 2
}
tSTEER_Evaluation;

//  Report levels
typedef enum tsteer_reportlevel
{
    eSTEER_ReportLevel_Summary  = 0,    // Top level criterion and evaluation status
    eSTEER_ReportLevel_Standard = 1,    // ...plus parameters and configuration/test criteria and evaluation status
    eSTEER_ReportLevel_Full     = 2     // ...plus all calculations and metrics
}
tSTEER_ReportLevel;

//  Value definition
typedef struct tsteer_value
{
    const char* name;
    const char* dataType;
    const char* precision;  // Optional; may be NULL
    const char* units;      // Optional; may be NULL
    const char* value;
}
tSTEER_Value;

//  Values
typedef struct tsteer_values
{
    uint32_t        count;
    tSTEER_Value    value[];
}
tSTEER_Values;

//  Value item definition
typedef struct tsteer_valueitem
{
    const char* label;
    const char* value;
}
tSTEER_ValueItem;

//  Value set definition
typedef struct tsteer_valueset
{
    const char*         name;
    const char*         dataType;
    const char*         precision;  // Optional; may be NULL
    const char*         units;      // Optional; may be NULL
    uint32_t            count;
    tSTEER_ValueItem    item[];
}
tSTEER_ValueSet;

//  Info list
typedef struct tsteer_infolist
{
    uint32_t    count;
    const char* item[];
}
tSTEER_InfoList;

//  Test info
typedef struct tsteer_testinfo
{
    const char*         name;
    const char*         suite;              // Optional; may be NULL
    const char*         description;
    tSTEER_Complexity   complexity;
    tSTEER_InfoList*    references;         // Optional; may be NULL
    const char*         programName;
    const char*         programVersion;
    tSTEER_InputFormat  inputFormat;
    const char*         repository;         // Optional; may be NULL
    tSTEER_InfoList*    authors;
    tSTEER_InfoList*    contributors;       // Optional; may be NULL
    tSTEER_InfoList*    maintainers;        // Optional; may be NULL
    const char*         contact;            // Optional; may be NULL
}
tSTEER_TestInfo;

//  Parameter info
typedef struct tsteer_parameterinfo
{
    const char* name;
    const char* dataType;
    const char* precision;      // Optional; may be NULL
    const char* units;          // Optional; may be NULL
    const char* defaultValue;
    const char* minimumValue;   // Optional; may be NULL
    const char* maximumValue;   // Optional; may be NULL
}
tSTEER_ParameterInfo;

//  Parameter info list
typedef struct tsteer_parameterinfolist
{
    uint32_t                count;
    tSTEER_ParameterInfo    parameterInfo[];
}
tSTEER_ParameterInfoList;

//  Parameters info
typedef struct tsteer_parametersinfo
{
    const char*                 testName;
    tSTEER_ParameterInfoList*   parameterInfoList;
}
tSTEER_ParametersInfo;

//  Parameter set
typedef struct tsteer_parameterset
{
    char*           testName;
    char*           parameterSetName;
    uint32_t        count;
    tSTEER_Value    parameter[];
}
tSTEER_ParameterSet;

//  Report pointer
typedef void* tSTEER_ReportPtr;

//  Confusion matrix
typedef struct tsteer_confusionmatrix
{
    uint64_t    predictedPassCount; // PP
    uint64_t    predictedFailCount; // PN
    uint64_t    actualPassCount;    // AP 
    uint64_t    actualFailCount;    // AN 
    uint64_t    truePositives;      // TP
    uint64_t    trueNegatives;      // TN
    uint64_t    falsePositives;     // FP
    uint64_t    falseNegatives;     // FN
}
tSTEER_ConfusionMatrix;

//  Confusion matrix statistics
typedef struct tsteer_confusionmatrixstatistics
{
    double  truePositiveRate;               // TPR
    double  trueNegativeRate;               // TNR
    double  positivePredictiveValue;        // PPV
    double  negativePredictiveValue;        // NPV
    double  falseNegativeRate;              // FNR
    double  falsePositiveRate;              // FPR
    double  falseDiscoveryRate;             // FDR
    double  falseOmissionRate;              // FOR
    double  prevalenceThreshold;            // PT
    double  threatScore;                    // TS
    double  accuracy;                       // ACC
    double  errorRate;                      
    double  balancedAccuracy;               // BA
    double  f1Score;                        // F1
    double  matthewsCorrelationCoefficient; // MCC
    double  fowlkesMallowsIndex;            // FM
    double  informedness;                   // BM
    double  markedness;                     // MK
    double  prevalence;                     
    double  positiveLikelihoodRatio;        // LR+
    double  negativeLikelihoodRatio;        // LR-
    double  diagnosticOddsRatio;            // DOR
}
tSTEER_ConfusionMatrixStatistics;

//  Profile
typedef struct tsteer_profile
{
    const char* id;   
    bool        specifiesDirectories;
    const char* input;  
    const char* parameters;
    const char* report;
}
tSTEER_Profile;

//  Profiles
typedef struct tsteer_profiles
{
    uint32_t        count;
    tSTEER_Profile  profile[];
}
tSTEER_Profiles;

//  Scheduled test
typedef struct tsteer_scheduledtest
{
    const char*         programName;
    tSTEER_Profiles*    profiles;
}
tSTEER_ScheduledTest;

//  Schedule
typedef struct tsteer_schedule
{
    const char*             scheduleId;         // Optional; may be NULL
    const char*             conductor;          // Optional; may be NULL
    const char*             notes;              // Optional; may be NULL
    tSTEER_ReportLevel      level;              // Optional; defaults to eSTEER_ReportDetail_Summary
    bool                    reportProgress;     // Optional; defaults to false
    uint32_t                count;
    tSTEER_ScheduledTest    tests[];
}
tSTEER_Schedule;

// =================================================================================================
#endif	// __STEER_TYPES_H__
// =================================================================================================
