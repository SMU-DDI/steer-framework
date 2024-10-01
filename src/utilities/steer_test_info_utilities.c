// =================================================================================================
//! @file steer_test_info_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the test info utilities used by the STandard Entropy Evaluation
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-03-31
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_test_info_utilities_private.h"
#include "steer_test_info_utilities.h"
#include "steer_json_utilities.h"
#include "steer_json_utilities_private.h"
#include "steer_utilities.h"
#include "steer_value_utilities.h"

// =================================================================================================
//  STEER_ValidateTestInfo
// =================================================================================================
int32_t STEER_ValidateTestInfo (const char* testInfoJson,
                                cJSON** rootObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Parse the JSON
    result = STEER_ParseJsonString(testInfoJson, rootObject);
    if (result == STEER_RESULT_SUCCESS)
    {
        // TODO: Validate this JSON against the appropriate JSON schema

        // Make sure this JSON has the test info tag
        cJSON* obj = NULL;
        result = STEER_GetChildObject(*rootObject, STEER_JSON_TAG_TEST_INFO, &obj);

        // If the tag isn't found, this isn't a test info JSON structure
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
//  STEER_GetTestInfoItemList
// =================================================================================================
int32_t STEER_GetTestInfoItemList (cJSON* parentObject,
                                   const char* tag,
                                   tSTEER_InfoList** list)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(list);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* listArray = NULL;
        uint32_t listArraySize = 0;

        // Setup
        *list = NULL;

        // Get the array
        result = STEER_GetChildArray(parentObject, tag,
                                     &listArray, &listArraySize);
        if (result == STEER_RESULT_SUCCESS)
        {
            tSTEER_InfoList* theList = NULL;

            // Allocate space
            result = STEER_AllocateMemory(sizeof(tSTEER_InfoList) + 
                                          (sizeof(const char*) * listArraySize),
                                          (void**)&theList);
            if (result == STEER_RESULT_SUCCESS)
            {
                uint_fast32_t i = 0;
                cJSON* itemObj = NULL;

                // Get the array items
                for (i = 0; i < listArraySize; i++)
                {
                    itemObj = cJSON_GetArrayItem(listArray, i);
                    result = STEER_CHECK_POINTER(itemObj);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        result = STEER_GetObjectStringValue(itemObj, 
                                                            (char**)&(theList->item[i]));
                        if (result == STEER_RESULT_SUCCESS)
                            theList->count++;
                        else
                            break;
                    }
                    else
                        break;
                }

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                    *list = theList;
                else
                    STEER_FreeMemory((void**)&theList);
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_FreeTestInfoItemList
// =================================================================================================
int32_t STEER_FreeTestInfoItemList (tSTEER_InfoList** list)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(list);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*list != NULL)
        {
            uint_fast32_t i = 0;

            for (i = 0; i < (*list)->count; i++)
            {
                STEER_FreeMemory((void**)&((*list)->item[i]));
            }
        }

        STEER_FreeMemory((void**)list);
    }
    return result;
}

// =================================================================================================
//  STEER_JsonToTestInfo
// =================================================================================================
int32_t STEER_JsonToTestInfo (const char* testInfoJson,
                              tSTEER_TestInfo** testInfo)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(testInfoJson);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testInfo);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;
        
        // Setup
        *testInfo = NULL;

        // Parse JSON
        result = STEER_ValidateTestInfo(testInfoJson, &rootObj);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Allocate space
            result = STEER_AllocateMemory(sizeof(tSTEER_TestInfo), (void**)testInfo);
            if (result == STEER_RESULT_SUCCESS)
            {
                cJSON* testInfoObj = NULL;

                // Get the test info object
                result = STEER_GetChildObject(rootObj, STEER_JSON_TAG_TEST_INFO, &testInfoObj);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the name
                    result = STEER_GetChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_NAME,
                                                        (char**)&((*testInfo)->name));
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the suite (optional)
                        result = STEER_GetChildObjectString(testInfoObj,
                                                            STEER_JSON_TAG_SUITE,
                                                            (char**)&((*testInfo)->suite));
                        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                            result = STEER_RESULT_SUCCESS;  // Eat this result
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the description
                        result = STEER_GetChildObjectString(testInfoObj,
                                                            STEER_JSON_TAG_DESCRIPTION,
                                                            (char**)&((*testInfo)->description));
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        char* complexity = NULL;

                        // Get the complexity
                        result = STEER_GetChildObjectString(testInfoObj,
                                                            STEER_JSON_TAG_COMPLEXITY,
                                                            &complexity);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            if (strcmp(complexity, STEER_JSON_VALUE_SIMPLE) == 0)
                                (*testInfo)->complexity = eSTEER_Complexity_Simple;
                            else if (strcmp(complexity, STEER_JSON_VALUE_AVERAGE) == 0)
                                (*testInfo)->complexity = eSTEER_Complexity_Average;
                            else if (strcmp(complexity, STEER_JSON_VALUE_MODERATE) == 0)
                                (*testInfo)->complexity = eSTEER_Complexity_Moderate;
                            else
                                result = STEER_CHECK_ERROR(STEER_RESULT_JSON_INVALID_CONSTRUCTION);

                            STEER_FreeMemory((void**)&complexity);
                        }
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the references (optional)
                        result = STEER_GetTestInfoItemList(testInfoObj,
                                                           STEER_JSON_TAG_REFERENCES,
                                                           &((*testInfo)->references));
                        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                            result = STEER_RESULT_SUCCESS;  // Eat this result
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the program name
                        result = STEER_GetChildObjectString(testInfoObj,
                                                            STEER_JSON_TAG_PROGRAM_NAME,
                                                            (char**)&((*testInfo)->programName));
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the program version
                        result = STEER_GetChildObjectString(testInfoObj,
                                                            STEER_JSON_TAG_PROGRAM_VERSION,
                                                            (char**)&((*testInfo)->programVersion));
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        char* inputFormat = NULL;

                        // Get the input format
                        result = STEER_GetChildObjectString(testInfoObj,
                                                            STEER_JSON_TAG_INPUT_FORMAT,
                                                            &inputFormat);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            if (strcmp(inputFormat, STEER_JSON_VALUE_ASCII_BITSTREAM) == 0)
                                (*testInfo)->inputFormat = eSTEER_InputFormat_AsciiBitstream;
                            else if (strcmp(inputFormat, STEER_JSON_VALUE_BITSTREAM) == 0)
                                (*testInfo)->inputFormat = eSTEER_InputFormat_Bitstream;
                            else
                                result = STEER_CHECK_ERROR(STEER_RESULT_JSON_INVALID_CONSTRUCTION);

                            STEER_FreeMemory((void**)&inputFormat);
                        }
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the repository (optional)
                        result = STEER_GetChildObjectString(testInfoObj,
                                                            STEER_JSON_TAG_REPOSITORY,
                                                            (char**)&((*testInfo)->repository));
                        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                            result = STEER_RESULT_SUCCESS;  // Eat this result
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the authors
                        result = STEER_GetTestInfoItemList(testInfoObj,
                                                           STEER_JSON_TAG_AUTHORS,
                                                           &((*testInfo)->authors));
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the contributors (optional)
                        result = STEER_GetTestInfoItemList(testInfoObj,
                                                           STEER_JSON_TAG_CONTRIBUTORS,
                                                           &((*testInfo)->contributors));
                        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                            result = STEER_RESULT_SUCCESS;  // Eat this result
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the maintainers (optional)
                        result = STEER_GetTestInfoItemList(testInfoObj,
                                                           STEER_JSON_TAG_MAINTAINERS,
                                                           &((*testInfo)->maintainers));
                        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                            result = STEER_RESULT_SUCCESS;  // Eat this result
                    }

                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the contact (optional)
                        result = STEER_GetChildObjectString(testInfoObj,
                                                            STEER_JSON_TAG_CONTACT,
                                                            (char**)&((*testInfo)->contact));
                        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                            result = STEER_RESULT_SUCCESS;  // Eat this result
                    }
                }

                // Check status
                if (result != STEER_RESULT_SUCCESS)
                    (void)STEER_FreeTestInfo(testInfo);
            }

            // Clean up
            (void)cJSON_Delete(rootObj);
            rootObj = NULL;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_FreeTestInfo
// =================================================================================================
int32_t STEER_FreeTestInfo (tSTEER_TestInfo** testInfo)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(testInfo);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Clean up
        if (*testInfo != NULL)
        {
            STEER_FreeMemory((void**)&((*testInfo)->name));
            STEER_FreeMemory((void**)&((*testInfo)->suite));
            STEER_FreeMemory((void**)&((*testInfo)->description));
            STEER_FreeMemory((void**)&((*testInfo)->programName));
            STEER_FreeMemory((void**)&((*testInfo)->programVersion));
            STEER_FreeMemory((void**)&((*testInfo)->repository));
            STEER_FreeMemory((void**)&((*testInfo)->contact));

            (void)STEER_FreeTestInfoItemList(&((*testInfo)->references));
            (void)STEER_FreeTestInfoItemList(&((*testInfo)->authors));
            (void)STEER_FreeTestInfoItemList(&((*testInfo)->contributors));
            (void)STEER_FreeTestInfoItemList(&((*testInfo)->maintainers));
        }

        STEER_FreeMemory((void**)testInfo);
    }
    return result;
}

// =================================================================================================
//  STEER_TestInfoToJson
// =================================================================================================
int32_t STEER_TestInfoToJson (tSTEER_TestInfo* testInfo,
                              char** testInfoJson)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(testInfo);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testInfoJson);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;

        // Setup
        *testInfoJson = NULL;

        // Create a root object
        rootObj = cJSON_CreateObject();
        result = STEER_CHECK_CONDITION((rootObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* testInfoObj = NULL;
            cJSON* infoArray = NULL;
            cJSON* infoString = NULL;
            cJSON_bool jsonResult = 0;
            uint_fast32_t i = 0;

            // Create a test info object
            testInfoObj = cJSON_CreateObject();
            result = STEER_CHECK_CONDITION((testInfoObj != NULL),
                                           STEER_RESULT_JSON_OPERATION_FAILURE);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the test info object to the root
                jsonResult = cJSON_AddItemToObject(rootObj,
                                                   STEER_JSON_TAG_TEST_INFO,
                                                   testInfoObj);
                result = STEER_CHECK_CONDITION((jsonResult == 1), 
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add name
                result = STEER_AddChildObjectString(testInfoObj,
                                                    STEER_JSON_TAG_NAME,
                                                    testInfo->name);
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add suite (optional)
                if (testInfo->suite != NULL)
                    result = STEER_AddChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_SUITE,
                                                        testInfo->suite);
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add description
                result = STEER_AddChildObjectString(testInfoObj,
                                                    STEER_JSON_TAG_DESCRIPTION,
                                                    testInfo->description);
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add complexity
                if (testInfo->complexity == eSTEER_Complexity_Simple)
                    result = STEER_AddChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_COMPLEXITY,
                                                        STEER_JSON_VALUE_SIMPLE);
                else if (testInfo->complexity == eSTEER_Complexity_Average)
                    result = STEER_AddChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_COMPLEXITY,
                                                        STEER_JSON_VALUE_AVERAGE);
                else if (testInfo->complexity == eSTEER_Complexity_Moderate)
                    result = STEER_AddChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_COMPLEXITY,
                                                        STEER_JSON_VALUE_MODERATE);
                else
                    result = STEER_CHECK_ERROR(EINVAL);
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add references (optional)
                if (testInfo->references != NULL)
                {
                    // Create an array
                    infoArray = cJSON_CreateArray();
                    result = STEER_CHECK_CONDITION((infoArray != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Add it to the test info 
                        jsonResult = cJSON_AddItemToObject(testInfoObj,
                                                           STEER_JSON_TAG_REFERENCES,
                                                           infoArray);
                        result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            for (i = 0; i < testInfo->references->count; i++)
                            {
                                // Add the reference
                                infoString = cJSON_CreateString(testInfo->references->item[i]);
                                result = STEER_CHECK_CONDITION((infoString != NULL), 
                                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    jsonResult = cJSON_AddItemToArray(infoArray, infoString);
                                    result = STEER_CHECK_CONDITION((jsonResult == 1),
                                                                    STEER_RESULT_JSON_OPERATION_FAILURE);
                                }
                                if (result != STEER_RESULT_SUCCESS)
                                    break;
                            }
                        }
                    }
                }
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add program name
                result = STEER_AddChildObjectString(testInfoObj,
                                                    STEER_JSON_TAG_PROGRAM_NAME,
                                                    testInfo->programName);
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add program version
                result = STEER_AddChildObjectString(testInfoObj,
                                                    STEER_JSON_TAG_PROGRAM_VERSION,
                                                    testInfo->programVersion);
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add input format
                if (testInfo->inputFormat == eSTEER_InputFormat_AsciiBitstream)
                    result = STEER_AddChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_INPUT_FORMAT,
                                                        STEER_JSON_VALUE_ASCII_BITSTREAM);
                else if (testInfo->inputFormat == eSTEER_InputFormat_Bitstream)
                    result = STEER_AddChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_INPUT_FORMAT,
                                                        STEER_JSON_VALUE_BITSTREAM);
                else
                    result = STEER_CHECK_ERROR(EINVAL);
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add repository (optional)
                if (testInfo->repository != NULL)
                    result = STEER_AddChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_REPOSITORY,
                                                        testInfo->repository);
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Create an array
                infoArray = cJSON_CreateArray();
                result = STEER_CHECK_CONDITION((infoArray != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add it to the test info 
                    jsonResult = cJSON_AddItemToObject(testInfoObj,
                                                       STEER_JSON_TAG_AUTHORS,
                                                       infoArray);
                    result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        for (i = 0; i < testInfo->authors->count; i++)
                        {
                            // Add the author
                            infoString = cJSON_CreateString(testInfo->authors->item[i]);
                            result = STEER_CHECK_CONDITION((infoString != NULL), 
                                                           STEER_RESULT_JSON_OPERATION_FAILURE);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                jsonResult = cJSON_AddItemToArray(infoArray, infoString);
                                result = STEER_CHECK_CONDITION((jsonResult == 1),
                                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                            }
                            if (result != STEER_RESULT_SUCCESS)
                                break;
                        }
                    }
                }
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add contributors (optional)
                if (testInfo->contributors != NULL)
                {
                    // Create an array
                    infoArray = cJSON_CreateArray();
                    result = STEER_CHECK_CONDITION((infoArray != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Add it to the test info 
                        jsonResult = cJSON_AddItemToObject(testInfoObj,
                                                           STEER_JSON_TAG_CONTRIBUTORS,
                                                           infoArray);
                        result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            for (i = 0; i < testInfo->contributors->count; i++)
                            {
                                // Add the contributor
                                infoString = cJSON_CreateString(testInfo->contributors->item[i]);
                                result = STEER_CHECK_CONDITION((infoString != NULL), 
                                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    jsonResult = cJSON_AddItemToArray(infoArray, infoString);
                                    result = STEER_CHECK_CONDITION((jsonResult == 1),
                                                                    STEER_RESULT_JSON_OPERATION_FAILURE);
                                }
                                if (result != STEER_RESULT_SUCCESS)
                                    break;
                            }
                        }
                    }
                }
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add maintainers (optional)
                if (testInfo->maintainers != NULL)
                {
                    // Create an array
                    infoArray = cJSON_CreateArray();
                    result = STEER_CHECK_CONDITION((infoArray != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Add it to the test info 
                        jsonResult = cJSON_AddItemToObject(testInfoObj,
                                                           STEER_JSON_TAG_MAINTAINERS,
                                                           infoArray);
                        result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            for (i = 0; i < testInfo->maintainers->count; i++)
                            {
                                // Add the maintainer
                                infoString = cJSON_CreateString(testInfo->maintainers->item[i]);
                                result = STEER_CHECK_CONDITION((infoString != NULL), 
                                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    jsonResult = cJSON_AddItemToArray(infoArray, infoString);
                                    result = STEER_CHECK_CONDITION((jsonResult == 1),
                                                                    STEER_RESULT_JSON_OPERATION_FAILURE);
                                }
                                if (result != STEER_RESULT_SUCCESS)
                                    break;
                            }
                        }
                    }
                }
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add contact (optional)
                if (testInfo->contact != NULL)
                    result = STEER_AddChildObjectString(testInfoObj,
                                                        STEER_JSON_TAG_CONTACT,
                                                        testInfo->contact);
            }
            
            // Check status
            if (result == STEER_RESULT_SUCCESS)
                *testInfoJson = cJSON_Print(rootObj);

            // Clean up
            (void)cJSON_Delete(rootObj);
        }
    }
    return result;
}

// =================================================================================================

