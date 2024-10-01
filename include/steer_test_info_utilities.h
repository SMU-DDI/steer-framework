// =================================================================================================
//! @file steer_test_info_utilities.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public test info utilities used by the STandard Entropy Evaluation 
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-03-30
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_TEST_INFO_UTILITIES_H__
#define __STEER_TEST_INFO_UTILITIES_H__

#include "steer.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_TestInfoToJson (tSTEER_TestInfo* testInfo,
                                  char** testInfoJson);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_TEST_INFO_UTILITIES_H__
// =================================================================================================
