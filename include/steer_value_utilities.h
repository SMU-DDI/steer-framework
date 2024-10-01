// =================================================================================================
//! @file steer_value_utilities.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public value utilities for the STandard Entropy Evaluation Report 
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

#ifndef __STEER_VALUE_UTILITIES_H__
#define __STEER_VALUE_UTILITIES_H__

#include "steer.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_GetNativeValue (const char* dataType,
                                  const char* value,
                                  void** nativeValue);

    int32_t STEER_NewValue (const char* name,
                            const char* dataType,
                            const char* precision,
                            const char* units,
                            const char* value,
                            tSTEER_Value* theValue);

    int32_t STEER_DuplicateValue (tSTEER_Value* sourceValue,
                                  tSTEER_Value* duplicatedValue);

    int32_t STEER_FreeValue (tSTEER_Value* value);

    int32_t STEER_NewValueSet (const char* name,
                               const char* dataType,
                               const char* precision,
                               const char* units,
                               tSTEER_ValueSet** theValueSet);

    int32_t STEER_AddValueToValueSet(const char* label,
                                     const char* value,
                                     tSTEER_ValueSet** theValueSet);

    int32_t STEER_GetValueFromValueSet (tSTEER_ValueSet* valueSet,
                                        const char* label,
                                        char** value);

    int32_t STEER_DuplicateValueSet (tSTEER_ValueSet* sourceValueSet,
                                     tSTEER_ValueSet** duplicatedValueSet);

    int32_t STEER_FreeValueSet (tSTEER_ValueSet** valueSet);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_VALUE_UTILITIES_H__
// =================================================================================================
