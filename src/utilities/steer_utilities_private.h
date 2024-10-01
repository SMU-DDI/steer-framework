// =================================================================================================
//! @file steer_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private general utilities for the STandard Entropy Evaluation 
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

#ifndef __STEER_UTILITIES_PRIVATE_H__
#define __STEER_UTILITIES_PRIVATE_H__

#include "steer_types_private.h"
#include "steer_report_utilities_private.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_ConvertHexToAscii (uint8_t hexValue,
                                     char* asciiHexValue,
                                     uint8_t* numZeros,
                                     uint8_t* numOnes);

    //! @fn int32_t STEER_ConvertBytes (uint8_t* byteBuffer, uint64_t byteCount, bool bitFormat, 
    //! bool asciiFormat, uint8_t** conversionBuffer, uint64_t* conversionBufferSizeInBytes, 
    //! uint64_t* numZeros, uint64_t* numOnes)
    //! @brief Call this function to convert bytes to a specified bit format.
    //! @param[in] byteBuffer A buffer of bytes to be converted.
    //! @param[in] byteCount The number of bytes in the buffer.
    //! @param[in] bitFormat A boolean indicating whether a bit format or a hex format is required.
    //! @param[in] asciiFormat A boolean indicating whether an ascii format is required.
    //! @param[out] conversionBuffer A pointer to a buffer containing the converted bytes.
    //! @param[out] conversionBufferSizeInBytes A pointer to the size in bytes of the conversion buffer.
    //! @param[out] numZeros A pointer to the number of zero bits in the original byte buffer; pass __NULL__
    //! if this information is not required.
    //! @param[out] numOnes A pointer to the number of one bits in the original byte buffer; pass __NULL__
    //! if this information is not required.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The caller of this function is responsible for freeing the conversion buffer
    //! with [STEER_FreeMemory](@ref STEER_FreeMemory).
    int32_t STEER_ConvertBytes (uint8_t* byteBuffer,
                                uint64_t byteCount,
                                bool bitFormat,
                                bool asciiFormat,
                                uint8_t** conversionBuffer,
                                uint64_t* conversionBufferSizeInBytes,
                                uint64_t* numZeros,
                                uint64_t* numOnes);

    int32_t STEER_GetRfc3339Timestamp (struct timeval* timestamp, 
                                       char** rfc3339FormattedTimestamp);

    int32_t STEER_GetRfc3339Duration (struct timeval* startTimeValue, 
                                      struct timeval* stopTimeValue,
                                      char** duration);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_UTILITIES_PRIVATE_H__
// =================================================================================================
