// =================================================================================================
//! @file steer_json_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements JSON functions for the STandard Entropy Evaluation Report (STEER) 
//! framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-09-24
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_json_utilities_private.h"
#include "steer_file_system_utilities.h"
#include "steer_json_utilities.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include "steer_value_utilities.h"
#include <sys/stat.h>

// =================================================================================================
//  STEER_GetObjectStringValue
// =================================================================================================
int32_t STEER_GetObjectStringValue (cJSON* theObject,
                                    char** theString)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(theObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(theString);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the object's string value
        char* objStr = cJSON_GetStringValue(theObject);
        result = STEER_CHECK_STRING(objStr);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(objStr, theString);
    }
    return result;
}

// =================================================================================================
//  STEER_GetChildObject
// =================================================================================================
int32_t STEER_GetChildObject (cJSON* parentObject,
                              const char* childTag,
                              cJSON** childObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(childTag);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(childObject);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the child object
        *childObject = cJSON_GetObjectItemCaseSensitive(parentObject, childTag);
        result = STEER_CHECK_CONDITION((*childObject != NULL), STEER_RESULT_JSON_TAG_NOT_FOUND);
    }
    return result;
}

// =================================================================================================
//  STEER_GetChildObjectBoolean
// =================================================================================================
int32_t STEER_GetChildObjectBoolean (cJSON* parentObject,
                                     const char* childTag,
                                     bool* childBoolean)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(childTag);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(childBoolean);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* childObj = NULL;

        // Setup
        *childBoolean = false;

        // Get the child object
        result = STEER_GetChildObject(parentObject, childTag, &childObj);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the child object's boolean value
            *childBoolean = (childObj->type == cJSON_True) ? true : false;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddChildObjectString
// =================================================================================================
int32_t STEER_AddChildObjectString (cJSON* parentObject,
                                    const char* childTag,
                                    const char* childString)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(childTag);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(childString);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Add the string to the parent object
        result = STEER_CHECK_CONDITION((cJSON_AddStringToObject(parentObject,
                                                                childTag,
                                                                childString) != NULL),
                                        STEER_RESULT_JSON_OPERATION_FAILURE);
    }
    return result;
}

// =================================================================================================
//  STEER_AddChildObjectBoolean
// =================================================================================================
int32_t STEER_AddChildObjectBoolean (cJSON* parentObject,
                                     const char* childTag,
                                     bool childBoolean)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(childTag);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        result = STEER_CHECK_CONDITION((cJSON_AddBoolToObject(parentObject,
                                                              childTag, 
                                                              childBoolean) != NULL),
                                       STEER_RESULT_JSON_OPERATION_FAILURE);
    }
    return result;
}

// =================================================================================================
//  STEER_AddEmptyNamedChildArray
// =================================================================================================
int32_t STEER_AddEmptyNamedChildArray (cJSON* parentObject,
                                       const char* childTag,
                                       cJSON** childArray)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(childTag);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        if (childArray != NULL)
            *childArray = NULL;

        cJSON* childObj = cJSON_CreateArray();
        result = STEER_CHECK_CONDITION((childObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON_bool jsonResult = cJSON_AddItemToObject(parentObject, childTag, childObj);
            result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (childArray != NULL)
                    *childArray = childObj;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ReadJsonFromFile
// =================================================================================================
int32_t STEER_ReadJsonFromFile (const char* jsonFilePath,
                                cJSON** rootObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(jsonFilePath);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(rootObject);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* fileJSON = NULL;
        FILE* fp = NULL;
        uint8_t* buffer = NULL;
        struct stat fileStat;
        size_t bytesRead = 0;

        // Setup
        *rootObject = NULL;

        // Get file size
        memset((void*)&fileStat, 0, sizeof(struct stat));
        result = stat(jsonFilePath, &fileStat);
        if (result != STEER_RESULT_SUCCESS)
            result = STEER_CHECK_ERROR(errno);

        if (result == STEER_RESULT_SUCCESS)
        {
            // Allocate space
            result = STEER_AllocateMemory((fileStat.st_size + 1) * sizeof(uint8_t),
                                          (void**)&buffer);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Open the file
                result = STEER_OpenFile(jsonFilePath, false, false, &fp);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Read the contents
                    bytesRead = fread(buffer, sizeof(uint8_t), fileStat.st_size, fp);
                    result = STEER_CHECK_CONDITION((bytesRead == fileStat.st_size), 
                                                   STEER_RESULT_NOT_ENOUGH_BYTES_READ);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Parse the JSON
                        fileJSON = cJSON_Parse((const char*)buffer);
                        result = STEER_CHECK_CONDITION((fileJSON != NULL), 
                                                       STEER_RESULT_JSON_PARSE_FAILURE);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Return JSON
                            *rootObject = fileJSON;
                        }
                    }

                    // Clean up
                    (void) STEER_CloseFile(&fp);
                }

                // Clean up
                STEER_FreeMemory((void**)&buffer);
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_WriteJsonToFile
// =================================================================================================
int32_t STEER_WriteJsonToFile (cJSON* rootObject,
                               const char* jsonFilePath)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(rootObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(jsonFilePath);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the JSON as a string
        char* jsonString = cJSON_Print(rootObject);
        result = STEER_CHECK_POINTER(jsonString);
        if (result == STEER_RESULT_SUCCESS)
        {
            FILE* fp = NULL;

            // Open/create the file
            result = STEER_OpenFile(jsonFilePath, true, false, &fp);
            if (result == STEER_RESULT_SUCCESS)
            {
                size_t bytesWritten = 0;
                size_t jsonStringLength = strlen(jsonString);

                // Write the JSON to file
                bytesWritten = fwrite(jsonString, sizeof(char), 
                                      jsonStringLength, fp);

                result = STEER_CHECK_CONDITION((bytesWritten == jsonStringLength), 
                                               STEER_RESULT_NOT_ENOUGH_BYTES_WRITTEN);

                // Clean up
                (void) STEER_CloseFile(&fp);
            }

            // Clean up
            STEER_FreeMemory((void**)&jsonString);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ParseJsonString
// =================================================================================================
int32_t STEER_ParseJsonString (const char* jsonString,
                               cJSON** rootObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(jsonString);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(rootObject);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* checkJSON = NULL;

        // Parse the JSON
        *rootObject = NULL;
        checkJSON = cJSON_Parse(jsonString);
        result = STEER_CHECK_CONDITION((checkJSON != NULL), STEER_RESULT_JSON_PARSE_FAILURE);
        if (result == STEER_RESULT_SUCCESS)
            *rootObject = checkJSON;
    }
    return result;
}

// =================================================================================================
//  STEER_HasChildTag
// =================================================================================================
bool STEER_HasChildTag (cJSON* theObject,
                        const char* tag)
{
    int32_t result = STEER_RESULT_SUCCESS;
    bool hasTag = false;

    // Check arguments
    result = STEER_CHECK_POINTER(theObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(tag);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* obj = cJSON_GetObjectItemCaseSensitive(theObject, tag);
        if (obj != NULL)
            hasTag = true;
    }
    return hasTag;
}

// =================================================================================================
//  STEER_GetChildObjectString
// =================================================================================================
int32_t STEER_GetChildObjectString (cJSON* parentObject,
                                    const char* childTag,
                                    char** childString)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(childTag);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(childString);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* childObj = NULL;

        // Get the child object
        result = STEER_GetChildObject(parentObject, childTag, &childObj);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the child object's string value
            result = STEER_GetObjectStringValue(childObj, childString);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetChildArray
// =================================================================================================
int32_t STEER_GetChildArray (cJSON* parentObject,
                             const char* childTag,
                             cJSON** childArray,
                             uint32_t* childArraySize)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(childTag);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(childArray);
    
    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* childObj = NULL;

        // Setup
        *childArray = NULL;
        if (childArraySize != NULL)
            *childArraySize = 0;

        result = STEER_GetChildObject(parentObject, childTag, &childObj);
        if (result == STEER_RESULT_SUCCESS)
        {
            *childArray = childObj;
            if (childArraySize != NULL)
                *childArraySize = cJSON_GetArraySize(childObj);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetArrayItemIndexWithName
// =================================================================================================
int32_t STEER_GetArrayItemIndexWithName (cJSON* array,
                                         const char* name,
                                         uint32_t* index)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(array);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(name);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(index);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint32_t arraySize = cJSON_GetArraySize(array);

        // Setup
        *index = 0;

        if (arraySize > 0)
        {
            uint_fast32_t i = 0;
            cJSON* arrayItem = NULL;
            char* itemName = NULL;
            bool foundIt = false;

            for (i = 0; i < arraySize; i++)
            {
                arrayItem = cJSON_GetArrayItem(array, i);
                result = STEER_CHECK_CONDITION((arrayItem != NULL),
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    result = STEER_GetChildObjectString(arrayItem, 
                                                        STEER_JSON_TAG_NAME, 
                                                        &itemName);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        if (strcmp(name, itemName) == 0)
                        {
                            foundIt = true;
                            *index = i;
                        }

                        STEER_FreeMemory((void**)&itemName);
                    }
                }

                if ((result != STEER_RESULT_SUCCESS) || foundIt)
                    break;
            }

            if ((result == STEER_RESULT_SUCCESS) && !foundIt)
                result = STEER_CHECK_ERROR(STEER_RESULT_JSON_TAG_NOT_FOUND);
        }
        else
            result = STEER_CHECK_ERROR(STEER_RESULT_JSON_TAG_NOT_FOUND);
    }
    return result;
}

// =================================================================================================
