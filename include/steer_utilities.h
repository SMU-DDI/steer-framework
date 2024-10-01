// =================================================================================================
//! @file steer_utilities.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public general utilities for the STandard Entropy Evaluation Report 
//! (STEER) framework.
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

#ifndef __STEER_UTILITIES_H__
#define __STEER_UTILITIES_H__

#include "steer.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    //! @fn int32_t STEER_AllocateMemory (size_t bufferSizeInBytes, void** buffer)
    //! @brief Call this function to dynamically allocate a memory buffer.
    //! @param[in] bufferSizeInBytes The size of the buffer to dynamically allocate.
    //! @param[out] buffer A pointer for the allocated memory buffer.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.\n
    //! @warning The caller of this function is responsible for freeing the allocated buffer
    //! with [STEER_FreeMemory](@ref STEER_FreeMemory).
    int32_t STEER_AllocateMemory (size_t bufferSizeInBytes,
                                  void** buffer);

    //! @fn int32_t STEER_ReallocateMemory (size_t bufferSizeInBytes, void** buffer)
    //! @brief Call this function to dynamically reallocate (resize) a memory buffer.
    //! @param[in] bufferSizeInBytes The size of the buffer to dynamically reallocate.
    //! @param[out] buffer A pointer for the reallocated memory buffer.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.\n
    //! @warning The caller of this function is responsible for freeing the allocated buffer
    //! with [STEER_FreeMemory](@ref STEER_FreeMemory).
    int32_t STEER_ReallocateMemory (size_t currentBufferSizeInBytes,
                                    size_t desiredBufferSizesInBytes,
                                    void** buffer);
                                        
    //! @fn int32_t STEER_FreeMemory (void** buffer)
    //! @brief Call this function to free a memory buffer.
    //! @param[in] buffer A pointer to a dynamically allocated memory buffer.
    void STEER_FreeMemory (void** buffer);

    //! @fn int32_t STEER_WaitForProcessesToComplete (tSTEER_ProcessList* processList, 
    //! uint32_t* processSuccessCount, uint32_t* processFailureCount)
    //! @brief Call this function to wait for one or more spawned processes to complete.
    //! @param[in] processList A pointer to a list of processes to wait on.
    //! @param[out] processSuccessCount A pointer to the number of processes that completed without errors.
    //! @param[out] processFailureCount A pointer to the number of processes that completed with errors.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    int32_t STEER_WaitForProcessesToComplete (tSTEER_ProcessList* processList,
                                              uint32_t waitIntervalInMicroseconds,
                                              uint32_t* processSuccessCount,
                                              uint32_t* processFailureCount);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_UTILITIES_H__
// =================================================================================================
