// =================================================================================================
//! @file steer_commands.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the command line program commands that must be implemented by all
//! STEER test programs.
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

#ifndef __STEER_COMMANDS_H__
#define __STEER_COMMANDS_H__

// =================================================================================================
//  Macros
// =================================================================================================

//  Test command line strings
#define STEER_CONDUCTOR_CMD                     "conductor"
#define STEER_ENTROPY_FILE_PATH_CMD             "entropy-file-path"
#define STEER_HELP_CMD                          "help"
#define STEER_NOTES_CMD                         "notes"
#define STEER_PARAMETERS_CMD                    "parameters"
#define STEER_PARAMETERS_FILE_PATH_CMD          "parameters-file-path"
#define STEER_PARAMETERS_INFO_CMD               "parameters-info"
#define STEER_REPORT_FILE_PATH_CMD              "report-file-path"
#define STEER_REPORT_LEVEL_CMD                  "report-level"
#define STEER_REPORT_PROGRESS_CMD               "report-progress"
#define STEER_SCHEDULE_ID_CMD                   "schedule-id"
#define STEER_TEST_INFO_CMD                     "test-info"
#define STEER_VERBOSE_CMD                       "verbose"
#define STEER_VERSION_CMD                       "version"

//  Test command line options
#define STEER_CONDUCTOR_OPTION                  0
#define STEER_ENTROPY_FILE_PATH_OPTION          1
#define STEER_HELP_OPTION                       2
#define STEER_NOTES_OPTION                      3
#define STEER_PARAMETERS_OPTION                 4
#define STEER_PARAMETERS_FILE_PATH_OPTION       5
#define STEER_PARAMETERS_INFO_OPTION            6
#define STEER_REPORT_FILE_PATH_OPTION           7
#define STEER_REPORT_LEVEL_OPTION               8
#define STEER_REPORT_PROGRESS_OPTION            9
#define STEER_SCHEDULE_ID_OPTION                10
#define STEER_TEST_INFO_OPTION                  11
#define STEER_VERBOSE_OPTION                    12
#define STEER_VERSION_OPTION                    13

//  Test short command line strings
#define STEER_CONDUCTOR_SHORT_CMD               'c'
#define STEER_ENTROPY_FILE_PATH_SHORT_CMD       'e'
#define STEER_HELP_SHORT_CMD                    'h'
#define STEER_NOTES_SHORT_CMD                   'n'
#define STEER_PARAMETERS_SHORT_CMD              'P'
#define STEER_PARAMETERS_FILE_PATH_SHORT_CMD    'p'
#define STEER_PARAMETERS_INFO_SHORT_CMD         'i'
#define STEER_REPORT_FILE_PATH_SHORT_CMD        'r'
#define STEER_REPORT_LEVEL_SHORT_CMD            'l'
#define STEER_REPORT_PROGRESS_SHORT_CMD         'R'
#define STEER_SCHEDULE_ID_SHORT_CMD             's'
#define STEER_TEST_INFO_SHORT_CMD               't'
#define STEER_VERBOSE_SHORT_CMD                 'v'
#define STEER_VERSION_SHORT_CMD                 'V'
#define STEER_UNKNOWN_OR_MISSING_OPTION         '?'
#define STEER_MISSING_ARGUMENT                  ':'

// =================================================================================================
#endif	// __STEER_COMMANDS_H__
// =================================================================================================
