// =================================================================================================
//! @file steer_value_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private value utilities for the STandard Entropy Evaluation Report 
//! (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-28
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_VALUE_UTILITIES_PRIVATE_H__
#define __STEER_VALUE_UTILITIES_PRIVATE_H__

#include "steer_types_private.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_AddChildValue (cJSON* parentArray,
                                 tSTEER_Value* value);

    int32_t STEER_AddChildValueSet (cJSON* parentArray,
                                    tSTEER_ValueSet* valueSet);

    int32_t STEER_GetChildValue (cJSON* parentObject,
                                 tSTEER_Value* value);

    int32_t STEER_GetChildValueSet (cJSON* parentObject,
                                    tSTEER_ValueSet** valueSet);

    int32_t STEER_FreeValues (tSTEER_Values** values);

    int32_t STEER_FreeValueSets (tSTEER_ValueSets** valueSets);

    int32_t STEER_AddValuesToArray (cJSON* parentArray,
                                    tSTEER_Values* values);

    int32_t STEER_AddValueSetsToArray (cJSON* parentArray,
                                       tSTEER_ValueSets* valueSets);

    int32_t STEER_GetValuesFromArray (cJSON* parentArray,
                                      tSTEER_Values** values,
                                      tSTEER_ValueSets** valueSets);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_VALUE_UTILITIES_PRIVATE_H__
// =================================================================================================
