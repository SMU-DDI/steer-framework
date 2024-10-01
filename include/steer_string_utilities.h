// =================================================================================================
//! @file steer_string_utilities.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public string utilities for the STandard Entropy Evaluation Report 
//! (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-09-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_STRING_UTILITIES_H__
#define __STEER_STRING_UTILITIES_H__

#include "steer.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    //! @fn const char* STEER_ErrorString (int32_t errorCode)
    //! @brief Call this function to convert a STEER error code to a descriptive string.
    //! @return A string that describes the error code.
    const char* STEER_ErrorString(int32_t errorCode);

    //! @fn int32_t STEER_DuplicateString (const char* sourceString, char** duplicatedString)
    //! @brief Call this function to duplicate a string.
    //! @param[in] sourceString A pointer to a UTF-8 source string.
    //! @param[out] duplicatedString A pointer to a newly allocated buffer containing a copy of the source string.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The caller of this function is responsible for freeing the allocated buffer
    //! with [STEER_FreeMemory](@ref STEER_FreeMemory).
    int32_t STEER_DuplicateString (const char* sourceString,
                                   char** duplicatedString);

    int32_t STEER_ConcatenateString (char** theString,
                                     const char* stringToConcatenate);

    //! @fn int32_t STEER_GetTimestampString (bool forFileName, bool useUTC, char** timestamp)
    //! @brief Call this function to get a formatted timestamp string.
    //! @param[in] forFileName A boolean indicating whether to format the string for use in a filename.
    //! @param[in] useUTC A boolean indicating whether to format the string using coordinated universal time.
    //! @param[out] timestamp A pointer to a newly allocated buffer containing the formatted timestamp.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The caller of this function is responsible for freeing the allocated buffer
    //! with [STEER_FreeMemory](@ref STEER_FreeMemory).
    int32_t STEER_GetTimestampString (bool forFileName,
                                      bool useUTC,
                                      char** timestamp);

    
    //! @fn int32_t STEER_ConvertStringToCamelCase (const char* stringValue, char ** convertedString)
    //! @brief Call this function to get a convert an input string with spaces and underscores to camelcase
    //! @param[in] stringValue The reference string
    //! @param[out] convertedString A pointer to the newly formatted string.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    //! @warning The caller of this function is responsible for freeing the allocated buffer
    //! with [STEER_FreeMemory](@ref STEER_FreeMemory).
    int32_t STEER_ConvertStringToCamelCase (const char* stringValue,
                                            char** convertedString);

    //! @fn int32_t STEER_SwapCharacters (const char* stringValue, char oldChar, char newChar, char ** convertedString)
    //! @brief Call this function to get a convert all occurances of oldChar into newChar within stringValue
    //! @param[in] stringValue The reference string
    //! @param[in] oldChar The character to replace
    //! @param[in] newChar The character to replace oldChar
    //! @param[out] convertedString A pointer to the newly formatted string.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    int32_t STEER_SwapCharacters (char * stringValue,
                                  char oldChar,
                                  char newChar,
                                  char ** convertedString);


    //! @fn int32_t STEER_ReplaceSubstring (char ** stringValue, char* oldVal, char* newVal, char ** convertedString)
    //! @brief Call this function to replace all values of oldVal with newVal in string stringValue
    //! @param[in] stringValue A pointer to the reference string
    //! @param[in] oldVal The substring to replace
    //! @param[in] newVal The substring to replace oldVal
    //! @param[out] convertedString A pointer to the newly formatted string.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    int32_t STEER_ReplaceSubstring (const char * stringValue,
                                    const char * oldVal,
                                    const char * newVal,
                                    char ** convertedString);


#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_STRING_UTILITIES_H__
// =================================================================================================
