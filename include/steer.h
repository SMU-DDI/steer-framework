// =================================================================================================
//! @file steer.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the top level interface for the STandard Entropy Evaluation Report
//! (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-03-28
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_H__
#define __STEER_H__

#include "steer_types.h"
#include "steer_config.h"
#include "steer_commands.h"
#include "steer_json_constants.h"
#include "steer_string_utilities.h"

// =================================================================================================
//  Constants
// =================================================================================================

//! @def STEER_VERSION
//! @brief The STEER implementation version represented by this release.
#define STEER_VERSION   "0.1.0"

// =================================================================================================
//  Macros
// =================================================================================================

#if STEER_ENABLE_CONSOLE_LOGGING
    #define STEER_CHECK_CONDITION(condition,errcode)            \
    ({                                                          \
        int32_t status = STEER_RESULT_SUCCESS;                  \
        if (condition == false)                                 \
        {                                                       \
            status = errcode;                                   \
            fprintf(stderr,                                     \
                    "[PID %d] At line %d in file %s: %s\n",     \
                    getpid(),                                   \
                    __LINE__,                                   \
                    __FILE__,                                   \
                    STEER_ErrorString(status));                 \
        }                                                       \
        status;                                                 \
    })

    #define STEER_CHECK_POINTER(ptr)                            \
    ({                                                          \
        int32_t status = STEER_RESULT_SUCCESS;                  \
        if (ptr == NULL)                                        \
        {                                                       \
            status = EFAULT;                                    \
            fprintf(stderr,                                     \
                    "[PID %d] At line %d in file %s: %s\n",     \
                    getpid(),                                   \
                    __LINE__,                                   \
                    __FILE__,                                   \
                    STEER_ErrorString(status));                 \
        }                                                       \
        status;                                                 \
    })

    #define STEER_CHECK_STRING(str)                             \
    ({                                                          \
        int32_t status = STEER_RESULT_SUCCESS;                  \
        if (str != NULL)                                        \
        {                                                       \
            if (strlen(str) == 0)                               \
            {                                                   \
                status = STEER_RESULT_EMPTY_STRING;             \
                fprintf(stderr,                                 \
                        "%d] At line %d in file %s: %s\n", \
                        getpid(),                               \
                        __LINE__,                               \
                        __FILE__,                               \
                        STEER_ErrorString(status));             \
            }                                                   \
        }                                                       \
        else                                                    \
        {                                                       \
            status = EFAULT;                                    \
            fprintf(stderr,                                     \
                    "[PID %d] At line %d in file %s: %s\n",     \
                    getpid(),                                   \
                    __LINE__,                                   \
                    __FILE__,                                   \
                    STEER_ErrorString(status));                 \
        }                                                       \
        status;                                                 \
    })

    #define STEER_CHECK_ERROR(errcode)                          \
    ({                                                          \
        if (errcode != STEER_RESULT_SUCCESS)                    \
        {                                                       \
            fprintf(stderr,                                     \
                    "[PID %d] At line %d in file %s: %s\n",     \
                    getpid(),                                   \
                    __LINE__,                                   \
                    __FILE__,                                   \
                    STEER_ErrorString(errcode));                \
        }                                                       \
        errcode;                                                \
    })
#else
    #define STEER_CHECK_CONDITION(condition,errcode)            \
    ({                                                          \
        int32_t status = STEER_RESULT_SUCCESS;                  \
        if (condition == false)                                 \
            status = errcode;                                   \
        status;                                                 \
    })

    #define STEER_CHECK_POINTER(ptr)                            \
    ({                                                          \
        int32_t status = STEER_RESULT_SUCCESS;                  \
        if (ptr == NULL)                                        \
            status = EFAULT;                                    \
        status;                                                 \
    })

    #define STEER_CHECK_STRING(str)                             \
    ({                                                          \
        int32_t status = STEER_RESULT_SUCCESS;                  \
        if (str != NULL)                                        \
        {                                                       \
            if (strlen(str) == 0)                               \
                status = STEER_RESULT_EMPTY_STRING;             \
        }                                                       \
        else                                                    \
            status = EFAULT;                                    \
        status;                                                 \
    })

    #define STEER_CHECK_ERROR(errcode)                          \
    ({                                                          \
        errcode;                                                \
    })
#endif

#define STEER_REPORT_PROGRESS(programName,progressStr)          \
({                                                              \
    int32_t status = STEER_CHECK_STRING(programName);           \
    if (status == STEER_RESULT_SUCCESS)                         \
    {                                                           \
        status = STEER_CHECK_STRING(progressStr);               \
        if (status == STEER_RESULT_SUCCESS)                     \
            fprintf(stdout, "[PID %d] %s: %s\n", getpid(),      \
                    programName, progressStr);                  \
    }                                                           \
})


// =================================================================================================
#endif	// __STEER_H__
// =================================================================================================
