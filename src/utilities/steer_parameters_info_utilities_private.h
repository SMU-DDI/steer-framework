// =================================================================================================
//! @file steer_parameters_info_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private parameters info utilities used by the STandard Entropy 
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

#ifndef __STEER_PARAMETERS_INFO_UTILITIES_PRIVATE_H__
#define __STEER_PARAMETERS_INFO_UTILITIES_PRIVATE_H__

#include "steer.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_ValidateParametersInfo (const char* parametersInfoJson,
                                          cJSON** rootObject);

    int32_t STEER_GetNextParameterInfo (cJSON* parameterInfoJSON,
                                        tSTEER_ParameterInfo* parameterInfo);

    int32_t STEER_AddChildParameterInfo (cJSON* testSpecificArray,
                                         tSTEER_ParameterInfo* parameterInfo);

    int32_t STEER_JsonToParametersInfo (const char* parametersInfoJson,
                                        tSTEER_ParametersInfo** parametersInfo);

    int32_t STEER_FreeParametersInfo (tSTEER_ParametersInfo** parametersInfo);
                                                     
#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_PARAMETERS_INFO_UTILITIES_PRIVATE_H__
// =================================================================================================
