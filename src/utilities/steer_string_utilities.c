// =================================================================================================
//! @file steer_string_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements string utility functions for the STandard Entropy Evaluation 
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-09-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_string_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include <ctype.h>
#include <limits.h>
#include <time.h>

// =================================================================================================
//  Private constants
// =================================================================================================

static const char* kSTEER_ErrorStrings[] = {
    "Unknown error code",
    "Out of range",
    "Empty string",
    "Not a file",
    "Empty file",
    "Not enough bytes read",
    "Not enough bytes written",
    "Buffer too small",
    "Buffer length mismatch",
    "Invalid time",
    "Validation check failure",
    "JSON parse failure",
    "JSON tag not found",
    "JSON operation failure",
    "Invalid JSON construction"
};

static const char* kNIST_ErrorStrings[] = {
    "Block length (L) greater than recommended block length",
    "Number of cycles greater than maximum number of cycles",
    "Number of cycles less than rejection constraint",
    "Number of matrices is zero",
    "fabs (pre-test proportion of ones - 0.5) greater than 2 / sqrt (bitstream length)",
    "block length less than minimum block length",
    "block length greater than maximum block length",
    "number of blocks in initialization sequence less than minimum number of blocks"
};

// =================================================================================================
//  STEER_StringIsPositiveInteger
// =================================================================================================
bool STEER_StringIsPositiveInteger (const char* theString)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool isPositiveInteger = false;

    // Check arguments
    result = STEER_CHECK_STRING(theString);
    if (result == STEER_RESULT_SUCCESS)
    {
        size_t len = strlen(theString);
        bool foundOnlyNumbers = true;
        size_t i = 0;

        for (i = 0; i < len; i++)
        {
            if (isdigit(theString[i]) == 0)
            {
                foundOnlyNumbers = false;
                break;
            }
        }

        if (foundOnlyNumbers)
            isPositiveInteger = true;
    }
    return isPositiveInteger;
}

// =================================================================================================
//  STEER_StringIsNegativeInteger
// =================================================================================================
bool STEER_StringIsNegativeInteger (const char* theString)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool isNegativeInteger = false;

    // Check arguments
    result = STEER_CHECK_STRING(theString);
    if (result == STEER_RESULT_SUCCESS)
    {
        size_t len = strlen(theString);
        
        if (theString[0] == '-')
        {
            bool foundOnlyNumbers = true;
            size_t i = 0;

            for (i = 1; i < len; i++)
            {
                if (isdigit(theString[i]) == 0)
                {
                    foundOnlyNumbers = false;
                    break;
                }
            }

            if (foundOnlyNumbers)
                isNegativeInteger = true;
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return isNegativeInteger;
}


// =================================================================================================
// STEER_ReplaceSubstring
// =================================================================================================
int32_t STEER_ReplaceSubstring(const char * stringValue,
                               const char * oldVal,
                               const char * newVal,
                               char ** convertedString)
{
    int32_t result = STEER_RESULT_SUCCESS;
    char *strBuf = NULL;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(convertedString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(newVal);
    if (result == STEER_RESULT_EMPTY_STRING)
        result = STEER_RESULT_SUCCESS;
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(oldVal);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        const char * rdPtr = stringValue;
        size_t strLen = strlen(stringValue);
        size_t oldLen = strlen(oldVal);
        size_t newLen = strlen(newVal);
        char * pos;

        // Count occurances
        int numOccurances = 0;
        while ((pos = strstr(rdPtr, oldVal)))
        {
            numOccurances ++;
            rdPtr = pos + oldLen;
        }
        
        if (numOccurances > 0)
        {
            result = (oldLen > newLen) ? STEER_AllocateMemory(strLen + numOccurances * (oldLen - newLen) + 1, (void **) &strBuf) :
                                         STEER_AllocateMemory(strLen + numOccurances * (newLen - oldLen) + 1, (void **) &strBuf) ;
            if (result == STEER_RESULT_SUCCESS)
            {
                // Replace occurances
                char * wrPtr = strBuf;
                rdPtr = stringValue;
                
                while ((pos = strstr(rdPtr, oldVal)))
                {
                   memcpy((void *) wrPtr, (void *) rdPtr, pos - rdPtr);
                   wrPtr += pos - rdPtr;
                   memcpy((void *) wrPtr, (void *) newVal, newLen);
                   rdPtr = pos + oldLen;
                   wrPtr += newLen;
                }

                // Copy end of string
                memcpy((void *) wrPtr, (void *) rdPtr, strLen - (rdPtr - stringValue) + 1);
                STEER_DuplicateString(strBuf, convertedString);
                STEER_FreeMemory((void **) &strBuf);
            }
        } else
        {
            result = STEER_DuplicateString(stringValue, convertedString);
        }
    }
    return result;
}
// =================================================================================================
// STEER_SwapCharacters
// =================================================================================================
int32_t STEER_SwapCharacters(char * stringValue,
                             char oldChar,
                             char newChar,
                             char ** convertedString)
{
    int32_t result = STEER_RESULT_SUCCESS;
    const char *rdPtr = stringValue;
    char *strBuf = NULL;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(convertedString);
    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        result = STEER_DuplicateString(stringValue, convertedString);
        
        if (result == STEER_RESULT_SUCCESS)
        {
            char * rdPtr = *convertedString;

            while(*rdPtr != '\0')
            {
                if (*rdPtr == oldChar)
                    *rdPtr = newChar;
                rdPtr ++;
            }
        }
    }

    return result;
}


// =================================================================================================
// STEER_ConvertStringToCamelCase
// =================================================================================================
int32_t STEER_ConvertStringToCamelCase(const char* stringValue,
                                       char ** convertedString)
{
    int32_t result = STEER_RESULT_SUCCESS;
    const char *rdPtr = stringValue;
    char *strBuf = NULL;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(convertedString);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        size_t numCharsRemoved = 0;
        
        // Count spaces and underscores
        while (*rdPtr != '\0')
        {
            if ((*rdPtr == '_') || (*rdPtr == ' '))
                numCharsRemoved ++;
            rdPtr ++;
        }
        
        // Allocate memory for new string
        result = STEER_AllocateMemory(sizeof(char) * (strlen(stringValue) + 1 - numCharsRemoved),
                                        (void **) &strBuf);

        if (result == STEER_RESULT_SUCCESS)
        {
            char *wrPtr = strBuf;
            rdPtr = stringValue;
            bool newCap = false;
            while (*rdPtr != '\0')
            {
                if ((*rdPtr == '_') || (*rdPtr == ' '))
                {
                    newCap = true;
                } else {
                    *wrPtr++ = (newCap) ? toupper(*rdPtr) : *rdPtr; 
                    newCap = false;
                }
                rdPtr ++;
            }
            *wrPtr = '\0';
        }
        result = STEER_DuplicateString(strBuf, convertedString);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_CHECK_CONDITION((*convertedString != NULL), errno);
        
        STEER_FreeMemory((void **)&strBuf);
    }

    return result;
}

// =================================================================================================
//  STEER_ConvertStringToBoolean
// =================================================================================================
int32_t STEER_ConvertStringToBoolean (const char* stringValue,
                                      bool* boolValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(boolValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *boolValue = false;

        if (strcmp(stringValue, STEER_JSON_VALUE_TRUE) == 0)
            *boolValue = true;
        else if (strcmp(stringValue, STEER_JSON_VALUE_FALSE) == 0)
            *boolValue = false;
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToDoublePrecisionFloatingPoint
// =================================================================================================
int32_t STEER_ConvertStringToDoublePrecisionFloatingPoint (const char* stringValue,
                                                           double* doubleValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(doubleValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        double temp = 0.0;

        // Setup
        *doubleValue = 0.0;
        errno = 0;

        temp = strtod(stringValue, NULL);
        result = STEER_CHECK_CONDITION((errno == 0), errno);
        if (result == STEER_RESULT_SUCCESS)
            *doubleValue = temp;
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToExtendedPrecisionFloatingPoint
// =================================================================================================
int32_t STEER_ConvertStringToExtendedPrecisionFloatingPoint (const char* stringValue,
                                                             long double* longDoubleValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(longDoubleValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        long double temp = 0.0;

        // Setup
        *longDoubleValue = 0.0;
        errno = 0;

        temp = strtold(stringValue, NULL);
        result = STEER_CHECK_CONDITION((errno == 0), errno);
        if (result == STEER_RESULT_SUCCESS)
            *longDoubleValue = temp;
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToSigned8BitInteger
// =================================================================================================
int32_t STEER_ConvertStringToSigned8BitInteger (const char* stringValue,
                                                int8_t* int8Value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(int8Value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        bool isValidInteger = false;

        isValidInteger = STEER_StringIsNegativeInteger(stringValue);
        if (!isValidInteger)
            isValidInteger = STEER_StringIsPositiveInteger(stringValue);
        if (isValidInteger)
        {
            long temp = 0;
            char* end = NULL;

            // Setup
            *int8Value = 0;
            errno = 0;

            temp = strtol(stringValue, &end, 0);
            result = STEER_CHECK_CONDITION((errno == 0), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                if ((temp >= SCHAR_MIN) && (temp <= SCHAR_MAX))
                    *int8Value = (int8_t)temp;
                else
                    result = STEER_CHECK_ERROR(ERANGE);
            }
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToSigned16BitInteger
// =================================================================================================
int32_t STEER_ConvertStringToSigned16BitInteger (const char* stringValue,
                                                 int16_t* int16Value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(int16Value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        bool isValidInteger = false;

        isValidInteger = STEER_StringIsNegativeInteger(stringValue);
        if (!isValidInteger)
            isValidInteger = STEER_StringIsPositiveInteger(stringValue);
        if (isValidInteger)
        {
            long temp = 0;
            char* end = NULL;

            // Setup
            *int16Value = 0;
            errno = 0;

            temp = strtol(stringValue, &end, 0);
            result = STEER_CHECK_CONDITION((errno == 0), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                if ((temp >= SHRT_MIN) && (temp <= SHRT_MAX))
                    *int16Value = (int16_t)temp;
                else
                    result = STEER_CHECK_ERROR(ERANGE);
            }
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToSigned32BitInteger
// =================================================================================================
int32_t STEER_ConvertStringToSigned32BitInteger (const char* stringValue,
                                                 int32_t* int32Value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(int32Value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        bool isValidInteger = false;

        isValidInteger = STEER_StringIsNegativeInteger(stringValue);
        if (!isValidInteger)
            isValidInteger = STEER_StringIsPositiveInteger(stringValue);
        if (isValidInteger)
        {
            long temp = 0;
            char* end = NULL;

            // Setup
            *int32Value = 0;
            errno = 0;

            temp = strtol(stringValue, &end, 0);
            result = STEER_CHECK_CONDITION((errno == 0), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                if ((temp >= -2147483648) && (temp <= 2147483647))
                    *int32Value = (int32_t)temp;
                else
                    result = STEER_CHECK_ERROR(ERANGE);
            }
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToSigned64BitInteger
// =================================================================================================
int32_t STEER_ConvertStringToSigned64BitInteger (const char* stringValue,
                                                 int64_t* int64Value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(int64Value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        bool isValidInteger = false;

        isValidInteger = STEER_StringIsNegativeInteger(stringValue);
        if (!isValidInteger)
            isValidInteger = STEER_StringIsPositiveInteger(stringValue);
        if (isValidInteger)
        {
            long long temp = 0;
            char* end = NULL;

            // Setup
            *int64Value = 0;
            errno = 0;

            temp = strtoll(stringValue, &end, 0);
            result = STEER_CHECK_CONDITION((errno == 0), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                if ((temp >= (int64_t)(9223372036854775808ULL * -1)) && (temp <= 9223372036854775807LL))
                    *int64Value = (int64_t)temp;
                else
                    result = STEER_CHECK_ERROR(ERANGE);
            }
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToSinglePrecisionFloatingPoint
// =================================================================================================
int32_t STEER_ConvertStringToSinglePrecisionFloatingPoint (const char* stringValue,
                                                           float* floatValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(floatValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        float temp = 0.0;

        // Setup
        *floatValue = 0.0;
        errno = 0;

        temp = strtof(stringValue, NULL);
        result = STEER_CHECK_CONDITION((errno == 0), errno);
        if (result == STEER_RESULT_SUCCESS)
            *floatValue = temp;
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToUnsigned8BitInteger
// =================================================================================================
int32_t STEER_ConvertStringToUnsigned8BitInteger (const char* stringValue,
                                                  uint8_t* uint8Value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(uint8Value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        bool isPositiveInteger = false;
        
        isPositiveInteger = STEER_StringIsPositiveInteger(stringValue);
        if (isPositiveInteger)
        {
            unsigned long temp = 0;
            char* end = NULL;

            // Setup
            *uint8Value = 0;
            errno = 0;

            temp = strtoul(stringValue, &end, 0);
            result = STEER_CHECK_CONDITION((errno == 0), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (temp <= UCHAR_MAX)
                    *uint8Value = (uint8_t)temp;
                else
                    result = STEER_CHECK_ERROR(ERANGE);
            }
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToUnsigned16BitInteger
// =================================================================================================
int32_t STEER_ConvertStringToUnsigned16BitInteger (const char* stringValue,
                                                   uint16_t* uint16Value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(uint16Value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        bool isPositiveInteger = false;

        isPositiveInteger = STEER_StringIsPositiveInteger(stringValue);
        if (isPositiveInteger)
        {
            unsigned long temp = 0;
            char* end = NULL;

            // Setup
            *uint16Value = 0;
            errno = 0;

            temp = strtoul(stringValue, &end, 0);
            result = STEER_CHECK_CONDITION((errno == 0), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (temp <= USHRT_MAX)
                    *uint16Value = (uint16_t)temp;
                else
                    result = STEER_CHECK_ERROR(ERANGE);
            }
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToUnsigned32BitInteger
// =================================================================================================
int32_t STEER_ConvertStringToUnsigned32BitInteger (const char* stringValue,
                                                   uint32_t* uint32Value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(uint32Value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        bool isPositiveInteger = false;

        isPositiveInteger = STEER_StringIsPositiveInteger(stringValue);
        if (isPositiveInteger)
        {
            unsigned long temp = 0;
            char* end = NULL;

            // Setup
            *uint32Value = 0;
            errno = 0;

            temp = strtoul(stringValue, &end, 0);
            result = STEER_CHECK_CONDITION((errno == 0), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (temp <= 4294967295)
                    *uint32Value = (uint32_t)temp;
                else
                    result = STEER_CHECK_ERROR(ERANGE);
            }
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertStringToUnsigned64BitInteger
// =================================================================================================
int32_t STEER_ConvertStringToUnsigned64BitInteger (const char* stringValue,
                                                   uint64_t* uint64Value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(stringValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(uint64Value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        bool isPositiveInteger = false;

        isPositiveInteger = STEER_StringIsPositiveInteger(stringValue);
        if (isPositiveInteger)
        {
            unsigned long long temp = 0;
            char* end = NULL;

            // Setup
            *uint64Value = 0;
            errno = 0;

            temp = strtoull(stringValue, &end, 0);
            result = STEER_CHECK_CONDITION((errno == 0), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (temp <= 18446744073709551615ULL)
                    *uint64Value = (uint64_t)temp;
                else
                    result = STEER_CHECK_ERROR(ERANGE);
            }
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_ErrorString
// =================================================================================================
const char* STEER_ErrorString(int32_t errorCode)
{
    const char* errorString = NULL;

    if (errorCode > STEER_RESULT_SUCCESS)
        errorString = strerror(errorCode);
    else if ((errorCode <= STEER_RESULT_RESERVED_START) && 
             (errorCode >= STEER_RESULT_RESERVED_END))
    {
        uint32_t index = (uint32_t)((-1.0 * errorCode) + STEER_RESULT_RESERVED_START);
        errorString = kSTEER_ErrorStrings[index];
    }
    else if ((errorCode <= STEER_RESULT_NIST_RESERVED_START) &&
             (errorCode >= STEER_RESULT_NIST_RESERVED_END))
    {
        uint32_t index = (uint32_t)((-1.0 * errorCode) + STEER_RESULT_NIST_RESERVED_START);
        errorString = kNIST_ErrorStrings[index];
    }
    else
        errorString = kSTEER_ErrorStrings[0];

    return errorString;
}

// =================================================================================================
//  STEER_DuplicateString
// =================================================================================================
int32_t STEER_DuplicateString (const char* sourceString,
                               char** duplicatedString)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(sourceString);
    if ((result == STEER_RESULT_SUCCESS) || (result == STEER_RESULT_EMPTY_STRING))
        result = STEER_CHECK_POINTER(duplicatedString);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*duplicatedString != NULL)
            STEER_FreeMemory((void**)duplicatedString);
        *duplicatedString = strdup(sourceString);
        result = STEER_CHECK_CONDITION((*duplicatedString != NULL), errno);
    }
    return result;
}

// =================================================================================================
//  STEER_ConcatenateString
// =================================================================================================
int32_t STEER_ConcatenateString (char** theString,
                                 const char* stringToConcatenate)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(theString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(*theString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(stringToConcatenate);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        size_t curLen = strlen(*theString) + 1;
        size_t addLen = strlen(stringToConcatenate);

        // Reallocate memory
        result = STEER_ReallocateMemory(curLen * sizeof(char),
                                        (curLen + addLen) * sizeof(char),
                                        (void**)theString);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Catenate
            strcat(*theString, stringToConcatenate);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetTimestampString
// =================================================================================================
int32_t STEER_GetTimestampString (bool forFileName,
                                  bool useUTC,
                                  char** timestamp)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(timestamp);
    if (result == STEER_RESULT_SUCCESS)
    {
        char* ts = NULL;
        int32_t count = 128;

        // Setup
        *timestamp = NULL;

        // Allocate memory
        result = STEER_AllocateMemory(count * sizeof(char),
                                      (void**)&ts);
        if (result == STEER_RESULT_SUCCESS)
        {
            time_t rawTime = -1;
            struct tm* convertedTime;

            // Get raw time
            rawTime = time(&rawTime);
            result = STEER_CHECK_CONDITION((rawTime != -1), STEER_RESULT_INVALID_TIME);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (useUTC)
                {
                    // Convert to GMT
                    convertedTime = gmtime(&rawTime);
                    result = STEER_CHECK_POINTER(convertedTime);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Convert to string
                        if (forFileName)
                            strftime(ts, count, "%Y-%m-%d-%H-%M-%S-UTC", convertedTime);
                        else
                            strftime(ts, count, "%Y-%m-%d %H:%M:%S UTC", convertedTime);

                        // Return value
                        *timestamp = ts;
                    }
                }
                else
                {
                    convertedTime = localtime(&rawTime);
                    result = STEER_CHECK_POINTER(convertedTime);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Convert to string
                        if (forFileName)
                        {
                            strftime(ts, count, "%Y-%m-%d-%H-%M-%S-", convertedTime);
                            strcat(ts, convertedTime->tm_zone);
                        }
                        else
                        {
                            strftime(ts, count, "%Y-%m-%d %H:%M:%S ", convertedTime);
                            strcat(ts, convertedTime->tm_zone);
                        }

                        // Return value
                        *timestamp = ts;
                    }
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                STEER_FreeMemory((void**)&ts);
        }   
    }
    return result;
}

// =================================================================================================
