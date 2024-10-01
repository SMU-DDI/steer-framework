// =================================================================================================
//! @file steer_json_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private JSON utilities used by the STandard Entropy Evaluation 
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

#ifndef __STEER_JSON_UTILITIES_PRIVATE_H__
#define __STEER_JSON_UTILITIES_PRIVATE_H__

#include "steer_types_private.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_GetObjectStringValue (cJSON* theObject,
                                        char** theString);

    int32_t STEER_GetChildObject (cJSON* parentObject,
                                  const char* childTag,
                                  cJSON** childObject);

    int32_t STEER_GetChildObjectBoolean (cJSON* parentObject,
                                         const char* childTag,
                                         bool* childBoolean);

    int32_t STEER_AddChildObjectString (cJSON* parentObject,
                                        const char* childTag,
                                        const char* childString);

    int32_t STEER_AddChildObjectBoolean (cJSON* parentObject,
                                         const char* childTag,
                                         bool childBoolean);

    int32_t STEER_AddEmptyNamedChildArray (cJSON* parentObject,
                                           const char* childTag,
                                           cJSON** childArray);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_JSON_UTILITIES_PRIVATE_H__
// =================================================================================================
