// =================================================================================================
//! @file steer_schedule_utilities.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public schedule utilities used by the STandard Entropy Evaluation 
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-03-31
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_SCHEDULE_UTILITIES_H__
#define __STEER_SCHEDULE_UTILITIES_H__

#include "steer.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_JsonToSchedule (const char* scheduleJson,
                                  tSTEER_Schedule** schedule);

    int32_t STEER_FreeSchedule (tSTEER_Schedule** schedule);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_SCHEDULE_UTILITIES_H__
// =================================================================================================
