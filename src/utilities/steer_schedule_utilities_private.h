// =================================================================================================
//! @file steer_schedule_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private schedule utilities used by the STandard Entropy Evaluation 
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

#ifndef __STEER_SCHEDULE_UTILITIES_PRIVATE_H__
#define __STEER_SCHEDULE_UTILITIES_PRIVATE_H__

#include "steer_schedule_utilities.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_ValidateSchedule (const char* scheduleJson,
                                    cJSON** rootObject);

    int32_t STEER_GetScheduledTests (cJSON* testsObject,
                                     tSTEER_Schedule* schedule);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_SCHEDULE_UTILITIES_PRIVATE_H__
// =================================================================================================
