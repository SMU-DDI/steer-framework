// =================================================================================================
//! @file steer_string_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private string utilities for the STandard Entropy Evaluation Report 
//! (STEER) framework.
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

#ifndef __STEER_STRING_UTILITIES_PRIVATE_H__
#define __STEER_STRING_UTILITIES_PRIVATE_H__

#include "steer.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    bool STEER_StringIsPositiveInteger (const char* theString);

    bool STEER_StringIsNegativeInteger (const char* theString);

    //! @fn int32_t STEER_ConvertStringToBoolean (const char* stringValue, bool* boolValue)
    //! @brief Call this function to convert a UTF-8 string to a boolean.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] boolValue A pointer for the converted boolean value. 
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __boolValue__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToBoolean (const char* stringValue,
                                          bool* boolValue);

    //! @fn int32_t STEER_ConvertStringToDoublePrecisionFloatingPoint (const char* stringValue, 
    //! double* doubleValue)
    //! @brief Call this function to convert a UTF-8 string value to a double precision floating 
    //! point (double) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] doubleValue A pointer for the converted double value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __doubleValue__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToDoublePrecisionFloatingPoint (const char* stringValue,
                                                               double* doubleValue);

    //! @fn int32_t STEER_ConvertStringToExtendedPrecisionFloatingPoint (const char* stringValue, 
    //! long double* longDoubleValue)
    //! @brief Call this function to convert a UTF-8 string value to an extended precision floating 
    //! point (long double) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] longDoubleValue A pointer for the converted long double value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __longDoubleValue__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToExtendedPrecisionFloatingPoint (const char* stringValue,
                                                                 long double* longDoubleValue);

    //! @fn int32_t STEER_ConvertStringToSigned8BitInteger (const char* stringValue, int8_t* int8Value)
    //! @brief Call this function to convert a UTF-8 string value to a signed 8-bit integer (int8_t) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] int8Value A pointer for the converted int8_t value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __int8Value__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToSigned8BitInteger (const char* stringValue,
                                                    int8_t* int8Value);

    //! @fn int32_t STEER_ConvertStringToSigned16BitInteger (const char* stringValue, int16_t* int16Value)
    //! @brief Call this function to convert a UTF-8 string value to a signed 16-bit integer (int16_t) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] int16Value A pointer for the converted int16_t value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __int16Value__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToSigned16BitInteger (const char* stringValue,
                                                     int16_t* int16Value);

    //! @fn int32_t STEER_ConvertStringToSigned32BitInteger (const char* stringValue, int32_t* int32Value)
    //! @brief Call this function to convert a UTF-8 string value to a signed 32-bit integer (int32_t) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] int32Value A pointer for the converted int32_t value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __int32Value__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToSigned32BitInteger (const char* stringValue,
                                                     int32_t* int32Value);

    //! @fn int32_t STEER_ConvertStringToSigned64BitInteger (const char* stringValue, int64_t* int64Value)
    //! @brief Call this function to convert a UTF-8 string value to a signed 64-bit integer (int64_t) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] int64Value A pointer for the converted int64_t value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __int64Value__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToSigned64BitInteger (const char* stringValue,
                                                     int64_t* int64Value);

    //! @fn int32_t STEER_ConvertStringToSinglePrecisionFloatingPoint (const char* stringValue, 
    //! float* floatValue)
    //! @brief Call this function to convert a UTF-8 string value to a single precision floating 
    //! point (float) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] floatValue A pointer for the converted float value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __floatValue__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToSinglePrecisionFloatingPoint (const char* stringValue,
                                                               float* floatValue);

    //! @fn int32_t STEER_ConvertStringToUnsigned8BitInteger (const char* stringValue, uint8_t* uint8Value)
    //! @brief Call this function to convert a UTF-8 string value to an unsigned 8-bit integer (uint8_t) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] uint8Value A pointer for the converted uint8_t value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __uint8Value__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToUnsigned8BitInteger (const char* stringValue,
                                                      uint8_t* uint8Value);

    //! @fn int32_t STEER_ConvertStringToUnsigned16BitInteger (const char* stringValue, uint16_t* uint16Value)
    //! @brief Call this function to convert a UTF-8 string value to an unsigned 16-bit integer (uint16_t) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] uint16Value A pointer for the converted uint16_t value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __uint16Value__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToUnsigned16BitInteger (const char* stringValue,
                                                      uint16_t* uint16Value);

    //! @fn int32_t STEER_ConvertStringToUnsigned32BitInteger (const char* stringValue, uint32_t* uint32Value)
    //! @brief Call this function to convert a UTF-8 string value to an unsigned 32-bit integer (uint32_t) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] uint32Value A pointer for the converted uint32_t value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __uint32Value__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToUnsigned32BitInteger (const char* stringValue,
                                                       uint32_t* uint32Value);

    //! @fn int32_t STEER_ConvertStringToUnsigned64BitInteger (const char* stringValue, uint64_t* uint64Value)
    //! @brief Call the function to convert a UTF-8 string value to an unsigned 64-bit integer (uint64_t) value.
    //! @param[in] stringValue The UTF-8 string to convert.
    //! @param[out] uint64Value A pointer for the converted uint64_t value.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The __uint64Value__ pointer is not allocated by this function.
    int32_t STEER_ConvertStringToUnsigned64BitInteger (const char* stringValue,
                                                       uint64_t* uint64Value);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_STRING_UTILITIES_PRIVATE_H__
// =================================================================================================
