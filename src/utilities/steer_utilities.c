// =================================================================================================
//! @file steer_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements utility functions for the STandard Entropy Evaluation Report (STEER) 
//! framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-15
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_utilities.h"
#include "steer_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_string_utilities_private.h"
#include <math.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include "cephes.h"

// =================================================================================================
//  STEER_ConvertHexToAscii
// =================================================================================================
int32_t STEER_ConvertHexToAscii(uint8_t hexValue,
                                char* asciiHexValue,
                                uint8_t* numZeros,
                                uint8_t* numOnes)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    switch (hexValue)
    {
        case 0:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x30;    // '0'
            *numZeros = 4;
            *numOnes = 0;
            break;
        case 1:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x31;    // '1'
            *numZeros = 3;
            *numOnes = 1;
            break;
        case 2:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x32;    // '2'
            *numZeros = 3;
            *numOnes = 1;
            break;
        case 3:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x33;    // '3'
            *numZeros = 2;
            *numOnes = 2;
            break;
        case 4:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x34;    // '4'
            *numZeros = 3;
            *numOnes = 1;
            break;
        case 5:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x35;    // '5'
            *numZeros = 2;
            *numOnes = 2;
            break;
        case 6:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x36;    // '6'
            *numZeros = 2;
            *numOnes = 2;
            break;
        case 7:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x37;    // '7'
            *numZeros = 1;
            *numOnes = 3;
            break;
        case 8:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x38;    // '8'
            *numZeros = 3;
            *numOnes = 1;
            break;
        case 9:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x39;    // '9'
            *numZeros = 2;
            *numOnes = 2;
            break;
        case 10:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x41;    // 'A'
            *numZeros = 2;
            *numOnes = 2;
            break;
        case 11:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x42;    // 'B'
            *numZeros = 1;
            *numOnes = 3;
            break;
        case 12:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x43;    // 'C'
            *numZeros = 2;
            *numOnes = 2;
            break;
        case 13:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x44;    // 'D'
            *numZeros = 1;
            *numOnes = 3;
            break;
        case 14:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x45;    // 'E'
            *numZeros = 1;
            *numOnes = 3;
            break;
        case 15:
            if (asciiHexValue != NULL)
                asciiHexValue[0] = 0x46;    // 'F'
            *numZeros = 0;
            *numOnes = 4;
            break;
        default:
            result = STEER_CHECK_ERROR(EINVAL);
            break;
    }
    return result;
}

// =================================================================================================
//  STEER_ConvertBytes
// =================================================================================================
int32_t STEER_ConvertBytes (uint8_t* byteBuffer,
                            uint64_t byteCount,
                            bool bitFormat,
                            bool asciiFormat,
                            uint8_t** conversionBuffer,
                            uint64_t* conversionBufferSizeInBytes,
                            uint64_t* numZeros,
                            uint64_t* numOnes)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(byteBuffer);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((byteCount > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(conversionBuffer);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(conversionBufferSizeInBytes);
    
    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint8_t* buf = NULL;
        uint64_t bufSize = 0;

        if (bitFormat)
            bufSize = byteCount * 8;
        else
            bufSize = byteCount * 2;
        
        if (asciiFormat)
            bufSize++;
        
        // Setup
        *conversionBuffer = NULL;
        if (numZeros != NULL)
            *numZeros = 0;
        if (numOnes != NULL)
            *numOnes = 0;
        
        // Allocate space for bitstream buffer
        result = STEER_AllocateMemory(bufSize * sizeof(uint8_t),
                                      (void**)&buf);
        if (result == STEER_RESULT_SUCCESS)
        {
            uint_fast64_t i = 0;
            uint_fast8_t j = 0;
            uint8_t bit = 0;
            uint8_t hexDigit = 0;
            uint64_t zeros = 0;
            uint64_t ones = 0;
            uint8_t mask = 0;
            
            // Loop over input bytes
            for (i = 0; i < byteCount; i++)
            {
                if (bitFormat)
                {
                    // Initialize bit mask
                    mask = 0x80;
                    
                    // Loop over bits in byte
                    for (j = 0; j < 8; j++)
                    {
                        if ((byteBuffer[i] & mask) != 0)
                        {
                            if (asciiFormat)
                                bit = 0x31;
                            else
                                bit = 1;
                            ones++;
                        }
                        else
                        {
                            if (asciiFormat)
                                bit = 0x30;
                            else
                                bit = 0;
                            zeros++;
                        }
                    
                        // Shift mask to next bit
                        mask >>= 1;
                        
                        buf[(i * 8) + j] = bit;
                    }
                }
                else 
                {
                    // Process the first hex digit in the byte
                    uint8_t zerosInDigit = 0;
                    uint8_t onesInDigit = 0;
                    hexDigit = (byteBuffer[i] & 0xF0) >> 4;
                    result = STEER_ConvertHexToAscii(hexDigit, 
                                                     (!asciiFormat && !bitFormat) ? NULL : (char*)&(buf[(i * 2)]), 
                                                     &zerosInDigit, 
                                                     &onesInDigit);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Update zeros and ones counts
                        zeros += zerosInDigit;
                        ones += onesInDigit;

                        // Process the second hex digit in the byte
                        hexDigit = (byteBuffer[i] & 0x0F);
                        result = STEER_ConvertHexToAscii(hexDigit, 
                                                         (!asciiFormat && !bitFormat) ? NULL : (char*)&(buf[(i * 2) + 1]), 
                                                         &zerosInDigit, 
                                                         &onesInDigit);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Update zeros and ones counts
                            zeros += zerosInDigit;
                            ones += onesInDigit;
                        }
                    }
                }
            }
            
            // Return values
            if (bitFormat || (!bitFormat && asciiFormat))
            {
                *conversionBuffer = buf;
                *conversionBufferSizeInBytes = bufSize;
            }
            else
                STEER_FreeMemory((void**)&buf);

            if (numZeros != NULL)
                *numZeros = zeros;
            if (numOnes != NULL)
                *numOnes = ones;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetRfc3339Timestamp
// =================================================================================================
int32_t STEER_GetRfc3339Timestamp (struct timeval* timestamp, 
                                   char** rfc3339FormattedTimestamp)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(timestamp);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(rfc3339FormattedTimestamp);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        char ts[STEER_STRING_MAX_LENGTH] = { 0 };
        char temp[128] = { 0 };
        char zone[64] = { 0 };
        struct tm localTime;
        time_t curTime = 0;
        size_t len = 0;

        memset(ts, 0, STEER_STRING_MAX_LENGTH);
        memset(temp, 0, 128);
        memset(zone, 0, 64);

        curTime = timestamp->tv_sec;
        (void)localtime_r(&curTime, &localTime);
        (void)strftime(temp, 128, "%Y-%m-%dT%H:%M:%S.", &localTime);
        (void)strftime(zone, 64, "%z", &localTime);
        sprintf(ts, "%s%.6ld%s\n", temp, (long)timestamp->tv_usec/1000, zone);
        len = strlen(ts);
        if (len > 1)
        {
            char min[] = { ts[len-3], ts[len-2], '\0' };
            sprintf(ts + len-3, ":%s", min);
        }
        result = STEER_DuplicateString(ts, rfc3339FormattedTimestamp);
    }
    return result;
}

// =================================================================================================
//  STEER_GetRfc3339Duration
// =================================================================================================
int32_t STEER_GetRfc3339Duration (struct timeval* startTimeValue, 
                                  struct timeval* stopTimeValue,
                                  char** duration)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(startTimeValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(stopTimeValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(duration);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        #define WEEKS_PER_YEAR          52ULL
        #define DAYS_PER_WEEK           7ULL
        #define HOURS_PER_DAY           24ULL
        #define MINUTES_PER_HOUR        60ULL
        #define SECONDS_PER_MINUTE      60ULL
        #define MICROSECONDS_PER_SECOND 1000000ULL
        #define MICROSECONDS_PER_MINUTE (MICROSECONDS_PER_SECOND * SECONDS_PER_MINUTE)
        #define MICROSECONDS_PER_HOUR   (MICROSECONDS_PER_MINUTE * MINUTES_PER_HOUR)
        #define MICROSECONDS_PER_DAY    (MICROSECONDS_PER_HOUR * HOURS_PER_DAY)
        #define MICROSECONDS_PER_WEEK   (MICROSECONDS_PER_DAY * DAYS_PER_WEEK)
        #define MICROSECONDS_PER_YEAR   (MICROSECONDS_PER_WEEK * WEEKS_PER_YEAR)

        *duration = NULL;

        uint64_t diffUsecs = 0;
        uint64_t diffSecs = 0;
        uint64_t numHours = 0;
        uint64_t numMinutes = 0;
        uint64_t numSeconds = 0;
        uint64_t numMicroseconds = 0;
        if (stopTimeValue->tv_usec < startTimeValue->tv_usec) 
        {
            diffSecs = stopTimeValue->tv_sec - startTimeValue->tv_sec - 1;
            diffUsecs = stopTimeValue->tv_usec - startTimeValue->tv_usec + MICROSECONDS_PER_SECOND;
        }
        else
        {
            diffSecs = stopTimeValue->tv_sec - startTimeValue->tv_sec;
            diffUsecs = stopTimeValue->tv_usec - startTimeValue->tv_usec;
        }
        numMicroseconds = diffUsecs + (diffSecs * MICROSECONDS_PER_SECOND);
        
        // Limit to weeks
        numHours = numMicroseconds / (uint64_t) MICROSECONDS_PER_HOUR;
        numMicroseconds -= (numHours * (uint64_t) MICROSECONDS_PER_HOUR);
        numMinutes = numMicroseconds / (uint64_t) MICROSECONDS_PER_MINUTE;
        numMicroseconds -= (numMinutes * (uint64_t) MICROSECONDS_PER_MINUTE);
        numSeconds = numMicroseconds / (uint64_t) MICROSECONDS_PER_SECOND;
        numMicroseconds -= (numSeconds * (uint64_t) MICROSECONDS_PER_SECOND);

        char ts[STEER_STRING_MAX_LENGTH] = { 0 };
        sprintf(ts, "%02d:%02d:%02d.%06d", (uint32_t)numHours, (uint32_t)numMinutes, 
                (uint32_t)numSeconds, (uint32_t)numMicroseconds);
        result = STEER_DuplicateString(ts, duration);
    }
    return result;
}

// =================================================================================================
//  STEER_AllocateMemory
// =================================================================================================
int32_t STEER_AllocateMemory (size_t bufferSizeInBytes,
                              void** buffer)
{
    // NOTE: Calling calloc with a size of zero is implementation-dependent.
    // This is why it's flagged as an invalid argument.

    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_CONDITION((bufferSizeInBytes > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(buffer);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {   
        void* buf = NULL;

        // Setup
        *buffer = NULL;

        // Allocate the buffer
        buf = calloc(1, bufferSizeInBytes); // calloc initializes memory to all zeros
        result = STEER_CHECK_CONDITION((buf != NULL), errno);
        if (result == STEER_RESULT_SUCCESS)
            *buffer = buf;
    }   // cppcheck-suppress memleak
    return result;
}

// =================================================================================================
//  STEER_ReallocateMemory
// =================================================================================================
int32_t STEER_ReallocateMemory (size_t currentBufferSizeInBytes,
                                size_t desiredBufferSizesInBytes,
                                void** buffer)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // NOTE: Calling realloc with a size of zero is implementation-dependent.
    // This is why it's flagged as an invalid argument.

    // Check arguments
    result = STEER_CHECK_CONDITION((desiredBufferSizesInBytes > 0), EINVAL);   
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(buffer);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {   
        // If the buffer has never been allocated, just allocate it
        if ((*buffer == NULL) && (currentBufferSizeInBytes == 0))
            result = STEER_AllocateMemory(desiredBufferSizesInBytes, buffer);

        else if (*buffer != NULL)
        {
            // Note that realloc does *not* initialize new memory
            // to all zeros, unlike calloc
            void* curBuffer = *buffer;
            void* newBuffer = NULL;

            newBuffer = realloc(curBuffer, desiredBufferSizesInBytes);
            result = STEER_CHECK_CONDITION((newBuffer != NULL), errno);
            if (result == STEER_RESULT_SUCCESS)
            {
                *buffer = newBuffer;

                // If we increased the buffer size, initialize the new bytes
                if (desiredBufferSizesInBytes > currentBufferSizeInBytes)
                {
                    void* initBuffer = 
                        (void*)(newBuffer + currentBufferSizeInBytes);
                    memset(initBuffer, 0, (desiredBufferSizesInBytes - currentBufferSizeInBytes));
                }
            }
        }   // cppcheck-suppress memleak
        else
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}
                                
// =================================================================================================
//  STEER_FreeMemory
// =================================================================================================
void STEER_FreeMemory (void** buffer)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(buffer);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Free the buffer
        if (*buffer != NULL)
        {
            free(*buffer);
            *buffer = NULL;
        }
    }
    return;
}

// =================================================================================================
//  STEER_WaitForProcessesToComplete
// =================================================================================================
int32_t STEER_WaitForProcessesToComplete (tSTEER_ProcessList* processList,
                                          uint32_t waitIntervalInMicroseconds,
                                          uint32_t* processSuccessCount,
                                          uint32_t* processFailureCount)
{
    int32_t result = STEER_RESULT_SUCCESS;
    uint32_t i = 0;

    // Check arguments
    result = STEER_CHECK_POINTER(processList);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((processList->count > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(processSuccessCount);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(processFailureCount);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Make sure none of the supplied process IDs are zero
        for (i = 0; i < processList->count; i++)
        {
            if (processList->process[i].pid == 0)
            {
                result = STEER_CHECK_ERROR(EINVAL);
                break;
            }
        }
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        pid_t checkPID = 0;
        int32_t status = 0;
        uint32_t processCount = processList->count;
        uint32_t failureCount = 0;
        uint32_t waitMicrosecs = waitIntervalInMicroseconds;

        if (waitMicrosecs == 0)
            waitMicrosecs = 1000;

        *processSuccessCount = 0;
        *processFailureCount = 0;

        // While there are still processes to wait for...
        while (processCount > 0)
        {
            // Loop over processes
            for (i = 0; i < processList->count; i++)
            {
                // Is this process valid?
                if (processList->process[i].pid != 0)
                {
                    // Wait on it to complete
                    checkPID = waitpid(processList->process[i].pid, &status, WNOHANG);
                    if (checkPID == processList->process[i].pid)
                    {
                        // Remove this process from the list
                        processList->process[i].pid = 0;
                        processCount -= 1;

                        // Did it fail?
                        if ((int8_t)WEXITSTATUS(status) != STEER_RESULT_SUCCESS)
                        {
                            // Yes
                            failureCount += 1;
                            #if STEER_ENABLE_CONSOLE_LOGGING
                            fprintf(stdout, "\t%s %s failed with status %d!\n", 
                                    processList->process[i].programName,
                                    processList->description, (int8_t)WEXITSTATUS(status));
                            #endif
                        }
                        else
                        {
                            // No
                            #if STEER_ENABLE_CONSOLE_LOGGING
                            fprintf(stdout, "\t%s %s succeeded.\n", 
                                    processList->process[i].programName,
                                    processList->description);
                            #endif
                        }
                    }
                }
            }

            // Sleep for a bit
            usleep(waitMicrosecs);

            if (result != STEER_RESULT_SUCCESS)
                break;
        }

        // Return success and failure counts
        *processSuccessCount = processList->count - failureCount;
        *processFailureCount = failureCount;
    }
    return result;
}
                                        
// =================================================================================================
