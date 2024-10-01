// =================================================================================================
//! @file steer_json_utilities.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public JSON utilities used by the STandard Entropy Evaluation 
//! Report (STEER) framework.
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

#ifndef __STEER_JSON_UTILITIES_H__
#define __STEER_JSON_UTILITIES_H__

#include "steer.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_ReadJsonFromFile (const char* jsonFilePath,
                                    cJSON** rootObject);

    int32_t STEER_WriteJsonToFile (cJSON* rootObject,
                                   const char* jsonFilePath);

    int32_t STEER_ParseJsonString (const char* jsonString,
                                   cJSON** rootObject);

    bool STEER_HasChildTag (cJSON* theObject,
                            const char* tag);

    int32_t STEER_GetChildObjectString (cJSON* parentObject,
                                        const char* childTag,
                                        char** childString);

    int32_t STEER_GetChildArray (cJSON* parentObject,
                                 const char* childTag,
                                 cJSON** childArray,
                                 uint32_t* childArraySize);

    int32_t STEER_GetArrayItemIndexWithName (cJSON* array,
                                             const char* name,
                                             uint32_t* index);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_JSON_UTILITIES_H__
// =================================================================================================
