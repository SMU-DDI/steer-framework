// =================================================================================================
//! @file steer_test_info_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private test info utilities used by the STandard Entropy Evaluation 
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

#ifndef __STEER_TEST_INFO_UTILITIES_PRIVATE_H__
#define __STEER_TEST_INFO_UTILITIES_PRIVATE_H__

#include "steer_types_private.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_ValidateTestInfo (const char* testInfoJson,
                                    cJSON** rootObject);

    int32_t STEER_GetTestInfoItemList (cJSON* parentObject,
                                       const char* tag,
                                       tSTEER_InfoList** list);

    int32_t STEER_FreeTestInfoItemList (tSTEER_InfoList** list);


    int32_t STEER_JsonToTestInfo (const char* testInfoJson,
                                  tSTEER_TestInfo** testInfo);

    int32_t STEER_FreeTestInfo (tSTEER_TestInfo** testInfo);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_TEST_INFO_UTILITIES_PRIVATE_H__
// =================================================================================================
