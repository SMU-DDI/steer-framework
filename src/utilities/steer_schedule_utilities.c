// =================================================================================================
//! @file steer_schedule_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the schedule utilities used by the STandard Entropy Evaluation
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-03-31
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_schedule_utilities_private.h"
#include "steer_schedule_utilities.h"
#include "steer_json_utilities.h"
#include "steer_json_utilities_private.h"
#include "steer_utilities.h"

// =================================================================================================
//  STEER_ValidateSchedule
// =================================================================================================
int32_t STEER_ValidateSchedule (const char* scheduleJson,
                                cJSON** rootObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Parse the JSON
    result = STEER_ParseJsonString(scheduleJson, rootObject);
    if (result == STEER_RESULT_SUCCESS)
    {
        // TODO: Validate this JSON against the appropriate JSON schema

        // Make sure this JSON has the schedule tag
        cJSON* obj = NULL;
        result = STEER_GetChildObject(*rootObject, STEER_JSON_TAG_SCHEDULE, &obj);

        // If the tag isn't found, this isn't a schedule JSON structure
        if (result != STEER_RESULT_SUCCESS)
        {
            // Clean up
            cJSON_Delete(*rootObject);
            *rootObject = NULL;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetScheduledTests
// =================================================================================================
int32_t STEER_GetScheduledTests (cJSON* testsArray,
                                 tSTEER_Schedule* schedule)
{
    int32_t result = STEER_RESULT_SUCCESS;  
    uint_fast32_t i = 0;
    uint_fast32_t j = 0;
    cJSON* testObj = NULL;
    cJSON* profilesArray = NULL;
    uint32_t profilesCount = 0;
    cJSON* profileObj = NULL;
    cJSON* obj = NULL;

    // Walk the tests
    for (i = 0; i < schedule->count; i++)
    {
        testObj = cJSON_GetArrayItem(testsArray, i);
        result = STEER_CHECK_POINTER(testObj);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the profiles array
            result = STEER_GetChildArray(testObj, 
                                         STEER_JSON_TAG_PROFILES, 
                                         &profilesArray,
                                         &profilesCount);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the program name
                result = STEER_GetChildObjectString(testObj,
                                                    STEER_JSON_TAG_PROGRAM_NAME,
                                                    (char**)&(schedule->tests[i].programName));
                if (result == STEER_RESULT_SUCCESS)
                {
                    result = STEER_AllocateMemory(sizeof(tSTEER_Profiles) +
                                                  (profilesCount * sizeof(tSTEER_Profile)),
                                                  (void**)&(schedule->tests[i].profiles));
                    // Walk the profiles
                    for (j = 0; j < profilesCount; j++)
                    {
                        profileObj = cJSON_GetArrayItem(profilesArray, j);
                        result = STEER_CHECK_POINTER(profileObj);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Get the profile id
                            result = STEER_GetChildObjectString(profileObj,
                                                                STEER_JSON_TAG_PROFILE_ID,
                                                                (char**)&(schedule->tests[i].profiles->profile[j].id));
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Is there an inputs directory?
                                result = STEER_GetChildObject(profileObj, 
                                                              STEER_JSON_TAG_INPUTS_DIRECTORY,
                                                              &obj);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    // Yes, this is a directory specification
                                    schedule->tests[i].profiles->profile[j].specifiesDirectories = true;
                                    
                                    // Get the inputs directory
                                    result = STEER_GetChildObjectString(profileObj,
                                                                        STEER_JSON_TAG_INPUTS_DIRECTORY,
                                                                        (char**)&(schedule->tests[i].profiles->profile[j].input));

                                    if (result == STEER_RESULT_SUCCESS)
                                    {
                                        // Get the parameters directory (optional)
                                        result = STEER_GetChildObjectString(profileObj,
                                                                            STEER_JSON_TAG_PARAMETERS_DIRECTORY,
                                                                            (char**)&(schedule->tests[i].profiles->profile[j].parameters));
                                        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                                            result = STEER_RESULT_SUCCESS;  // Eat this result
                                    }

                                    if (result == STEER_RESULT_SUCCESS)
                                    {
                                        // Get the reports directory
                                        result = STEER_GetChildObjectString(profileObj,
                                                                            STEER_JSON_TAG_REPORTS_DIRECTORY,
                                                                            (char**)&(schedule->tests[i].profiles->profile[j].report));
                                    }
                                    
                                    if (result == STEER_RESULT_SUCCESS)
                                        schedule->tests[i].profiles->count++;
                                    else
                                        break;
                                }
                                else if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                                {
                                    // Check for an input tag
                                    result = STEER_GetChildObject(profileObj,
                                                                  STEER_JSON_TAG_INPUT,
                                                                  &obj);
                                    if (result == STEER_RESULT_SUCCESS)
                                    {
                                        schedule->tests[i].profiles->profile[j].specifiesDirectories = false;

                                        // Get the input file
                                        result = STEER_GetChildObjectString(profileObj,
                                                                            STEER_JSON_TAG_INPUT,
                                                                            (char**)&(schedule->tests[i].profiles->profile[j].input));
                                        if (result == STEER_RESULT_SUCCESS)
                                        {
                                            // Get the parameters file (optional)
                                            result = STEER_GetChildObjectString(profileObj,
                                                                                STEER_JSON_TAG_PARAMETERS,
                                                                                (char**)&(schedule->tests[i].profiles->profile[j].parameters));
                                            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                                                result = STEER_RESULT_SUCCESS;  // Eat this result
                                        }

                                        if (result == STEER_RESULT_SUCCESS)
                                        {
                                            // Get the report path
                                            result = STEER_GetChildObjectString(profileObj,
                                                                                STEER_JSON_TAG_REPORT,
                                                                                (char**)&(schedule->tests[i].profiles->profile[j].report));
                                        }
                                        
                                        if (result == STEER_RESULT_SUCCESS)
                                            schedule->tests[i].profiles->count++;
                                        else
                                            break;
                                    }
                                    else
                                        break;
                                }
                                else
                                    break;
                            }
                            else
                                break;
                        }
                        else
                            break;
                    }

                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
                else
                    break;
            }
            else
                break;
        }
        else   
            break;
    }
    return result;
}

// =================================================================================================
//  STEER_JsonToSchedule
// =================================================================================================
int32_t STEER_JsonToSchedule (const char* scheduleJson,
                              tSTEER_Schedule** schedule)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(scheduleJson);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(schedule);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;

        // Setup
        *schedule = NULL;

        // Parse JSON
        result = STEER_ValidateSchedule(scheduleJson, &rootObj);
        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* scheduleObj = NULL;

            // Get the schedule object
            result = STEER_GetChildObject(rootObj,
                                          STEER_JSON_TAG_SCHEDULE,
                                          &scheduleObj);
            if (result == STEER_RESULT_SUCCESS)
            {
                cJSON* testsArray = NULL;
                uint32_t testsArraySize = 0;

                // Get the tests array
                result = STEER_GetChildArray(scheduleObj, 
                                             STEER_JSON_TAG_TESTS, 
                                             &testsArray,
                                             &testsArraySize);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Allocate space
                    result = STEER_AllocateMemory(sizeof(tSTEER_Schedule) +
                                                  (testsArraySize * sizeof(tSTEER_ScheduledTest)), 
                                                  (void**)schedule);

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the schedule tests
                        (*schedule)->count = testsArraySize;
                        result = STEER_GetScheduledTests(testsArray,
                                                         *schedule);
                    }
                    
                    // Check status
                    if (result != STEER_RESULT_SUCCESS)
                        (void)STEER_FreeSchedule(schedule);
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the schedule ID (optional)
                result = STEER_GetChildObjectString(scheduleObj,
                                                    STEER_JSON_TAG_SCHEDULE_ID,
                                                    (char**)&((*schedule)->scheduleId));
                if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                    result = STEER_RESULT_SUCCESS;  // Eat this result
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the conductor (optional)
                result = STEER_GetChildObjectString(scheduleObj,
                                                    STEER_JSON_TAG_TEST_CONDUCTOR,
                                                    (char**)&((*schedule)->conductor));
                if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                    result = STEER_RESULT_SUCCESS;  // Eat this result
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the notes (optional)
                result = STEER_GetChildObjectString(scheduleObj,
                                                    STEER_JSON_TAG_TEST_NOTES,
                                                    (char**)&((*schedule)->notes));
                if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                    result = STEER_RESULT_SUCCESS;  // Eat this result
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                char* reportLevel = NULL;

                // Get the report level (optional)
                result = STEER_GetChildObjectString(scheduleObj,
                                                    STEER_JSON_TAG_REPORT_LEVEL,
                                                    (char**)&reportLevel);
                if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                {
                    result = STEER_RESULT_SUCCESS;  // Eat this result
                    (*schedule)->level = eSTEER_ReportLevel_Summary;
                }
                else if (result == STEER_RESULT_SUCCESS)
                {
                    if (strcmp(reportLevel, STEER_JSON_VALUE_FULL) == 0)
                        (*schedule)->level = eSTEER_ReportLevel_Full;
                    else if (strcmp(reportLevel, STEER_JSON_VALUE_STANDARD) == 0)
                        (*schedule)->level = eSTEER_ReportLevel_Standard;
                    else if (strcmp(reportLevel, STEER_JSON_VALUE_SUMMARY) == 0)
                        (*schedule)->level = eSTEER_ReportLevel_Summary;
                }
                STEER_FreeMemory((void**)&reportLevel);
            }
            
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the report progress flag (optional)
                result = STEER_GetChildObjectBoolean(scheduleObj,
                                                     STEER_JSON_TAG_REPORT_PROGRESS,
                                                     &((*schedule)->reportProgress));
                if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                    result = STEER_RESULT_SUCCESS;  // Eat this result
            }

            // Clean up
            cJSON_Delete(rootObj);
            rootObj = NULL;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_FreeSchedule
// =================================================================================================
int32_t STEER_FreeSchedule (tSTEER_Schedule** schedule)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(schedule);
    if (result == STEER_RESULT_SUCCESS)
    {
        if ((*schedule) != NULL)
        {
            uint_fast32_t i = 0;
            uint_fast32_t j = 0;

            // Clean up
            STEER_FreeMemory((void**)&((*schedule)->scheduleId));
            STEER_FreeMemory((void**)&((*schedule)->conductor));
            STEER_FreeMemory((void**)&((*schedule)->notes));
            for (i = 0; i < (*schedule)->count; i++)
            {
                STEER_FreeMemory((void**)&((*schedule)->tests[i].programName));

                for (j = 0; j < (*schedule)->tests[i].profiles->count; j++)
                {
                    STEER_FreeMemory((void**)&((*schedule)->tests[i].profiles->profile[j].id));
                    STEER_FreeMemory((void**)&((*schedule)->tests[i].profiles->profile[j].input));
                    STEER_FreeMemory((void**)&((*schedule)->tests[i].profiles->profile[j].parameters));
                    STEER_FreeMemory((void**)&((*schedule)->tests[i].profiles->profile[j].report));
                    
                    STEER_FreeMemory((void**)&((*schedule)->tests[i].profiles->profile[j]));
                }
                
                STEER_FreeMemory((void**)&((*schedule)->tests[i].profiles));
            }
        }

        STEER_FreeMemory((void**)schedule);
    }
    return result;
}

// =================================================================================================
