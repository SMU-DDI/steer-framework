// =================================================================================================
//! @file steer_parameter_set_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private parameter set utilities used by the STandard Entropy 
//! Evaluation Report (STEER) framework.
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

#ifndef __STEER_PARAMETER_SET_UTILITIES_PRIVATE_H__
#define __STEER_PARAMETER_SET_UTILITIES_PRIVATE_H__

#include "steer_parameters_info_utilities.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_ValidateParameterSet (const char* parameterSet,
                                        cJSON** rootObject);

    int32_t STEER_GetNextParameter (cJSON* parameterObject,
                                                tSTEER_Value* parameter);

    int32_t STEER_DefaultParameterCheck (tSTEER_ParameterInfo* parameterInfo,
                                         tSTEER_ParameterSet** parameterSet);

    int32_t STEER_JsonToParameterSet (const char* parameterSetJson,
                                      tSTEER_ParametersInfo* parametersInfo,
                                      tSTEER_ParameterSet** parameterSet);

    int32_t STEER_ParameterSetToJson (tSTEER_ParameterSet* parameterSet,
                                      char** parameterSetJson);

    int32_t STEER_FreeParameterSet (tSTEER_ParameterSet** parameterSet);

    int32_t STEER_GetParameterValue (tSTEER_ParameterSet* parameterSet,
                                     const char* parameterName,
                                     void** parameterValue);
#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_PARAMETER_SET_UTILITIES_PRIVATE_H__
// =================================================================================================
