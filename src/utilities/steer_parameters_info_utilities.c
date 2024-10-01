// =================================================================================================
//! @file steer_parameters_info_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the parameters info utilities used by the STandard Entropy
//! Evaluation Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-03-31
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_parameters_info_utilities_private.h"
#include "steer_parameters_info_utilities.h"
#include "steer_json_utilities.h"
#include "steer_json_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"

// =================================================================================================
//  STEER_ValidateParametersInfo
// =================================================================================================
int32_t STEER_ValidateParametersInfo (const char* parametersInfoJson,
                                      cJSON** rootObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Parse the JSON
    result = STEER_ParseJsonString(parametersInfoJson, rootObject);
    if (result == STEER_RESULT_SUCCESS)
    {
        // TODO: Validate this JSON against the appropriate JSON schema

        // Make sure this JSON has the parameters info tag
        cJSON* obj = NULL;
        result = STEER_GetChildObject(*rootObject, STEER_JSON_TAG_PARAMETERS_INFO, &obj);

        // If the tag isn't found, this isn't a parameters info JSON structure
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
//  STEER_GetNextParameterInfo
// =================================================================================================
int32_t STEER_GetNextParameterInfo (cJSON* parameterInfoObject,
                                    tSTEER_ParameterInfo* parameterInfo)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER(parameterInfoObject);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the name
        result = STEER_GetChildObjectString(parameterInfoObject,
                                            STEER_JSON_TAG_NAME,
                                            (char**)&(parameterInfo->name));

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the data type
            result = STEER_GetChildObjectString(parameterInfoObject,
                                                STEER_JSON_TAG_DATA_TYPE,
                                                (char**)&(parameterInfo->dataType));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the units (optional)
            result = STEER_GetChildObjectString(parameterInfoObject,
                                                STEER_JSON_TAG_UNITS,
                                                (char**)&(parameterInfo->units));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the default value
            result = STEER_GetChildObjectString(parameterInfoObject,
                                                STEER_JSON_TAG_DEFAULT_VALUE,
                                                (char**)&(parameterInfo->defaultValue));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the minimum value (optional)
            result = STEER_GetChildObjectString(parameterInfoObject,
                                                STEER_JSON_TAG_MINIMUM_VALUE,
                                                (char**)&(parameterInfo->minimumValue));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the maximum value (optional)
            result = STEER_GetChildObjectString(parameterInfoObject,
                                                STEER_JSON_TAG_MAXIMUM_VALUE,
                                                (char**)&(parameterInfo->maximumValue));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddChildParameterInfo
// =================================================================================================
int32_t STEER_AddChildParameterInfo (cJSON* parametersArray,
                                     tSTEER_ParameterInfo* parameterInfo)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parametersArray);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(parameterInfo);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* paramObj = NULL;
        cJSON_bool jsonResult = 0;

        paramObj = cJSON_CreateObject();
        result = STEER_CHECK_CONDITION((paramObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add name
            result = STEER_AddChildObjectString(paramObj,
                                                STEER_JSON_TAG_NAME,
                                                parameterInfo->name);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add data type
                result = STEER_AddChildObjectString(paramObj,
                                                    STEER_JSON_TAG_DATA_TYPE,
                                                    parameterInfo->dataType);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add precision (optional)
                if (parameterInfo->precision != NULL)
                    result = STEER_AddChildObjectString(paramObj,
                                                        STEER_JSON_TAG_PRECISION,
                                                        parameterInfo->precision);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add units (optional)
                if (parameterInfo->units != NULL)
                    result = STEER_AddChildObjectString(paramObj,
                                                        STEER_JSON_TAG_UNITS,
                                                        parameterInfo->units);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add default value
                result = STEER_AddChildObjectString(paramObj,
                                                    STEER_JSON_TAG_DEFAULT_VALUE,
                                                    parameterInfo->defaultValue);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add minimum value (optional)
                if (parameterInfo->minimumValue != NULL)
                    result = STEER_AddChildObjectString(paramObj,
                                                        STEER_JSON_TAG_MINIMUM_VALUE,
                                                        parameterInfo->minimumValue);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add maximum value (optional)
                if (parameterInfo->maximumValue != NULL)
                    result = STEER_AddChildObjectString(paramObj,
                                                        STEER_JSON_TAG_MAXIMUM_VALUE,
                                                        parameterInfo->maximumValue);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                jsonResult = cJSON_AddItemToArray(parametersArray, paramObj);
                result = STEER_CHECK_CONDITION((jsonResult == 1), 
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_JsonToParametersInfo
// =================================================================================================
int32_t STEER_JsonToParametersInfo (const char* parametersInfoJson,
                                    tSTEER_ParametersInfo** parametersInfo)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parametersInfoJson);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(parametersInfo);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;

        // Setup
        *parametersInfo = NULL;

        // Parse JSON
        result = STEER_ValidateParametersInfo(parametersInfoJson, 
                                              &rootObj);
        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* parametersInfoObj = NULL;

            // Get the parameters info object
            result = STEER_GetChildObject(rootObj,
                                          STEER_JSON_TAG_PARAMETERS_INFO,
                                          &parametersInfoObj);
            if (result == STEER_RESULT_SUCCESS)
            {
                cJSON* paramsArray = NULL;
                int paramsArraySize = 0;
                cJSON* paramInfoObj = NULL;

                // Get the parameters array
                result = STEER_GetChildObject(parametersInfoObj, 
                                              STEER_JSON_TAG_PARAMETERS,
                                              &paramsArray);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the size of the parameters array
                    paramsArraySize = cJSON_GetArraySize(paramsArray);
                    result = STEER_CHECK_CONDITION((paramsArraySize > 0), 
                                                   STEER_RESULT_JSON_INVALID_CONSTRUCTION);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Allocate space
                    result = STEER_AllocateMemory(sizeof(tSTEER_ParametersInfo), (void**)parametersInfo);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Allocate list
                        result = STEER_AllocateMemory(sizeof(tSTEER_ParameterInfoList) +
                                                      (paramsArraySize * sizeof(tSTEER_ParameterInfo)),
                                                      (void**)&((*parametersInfo)->parameterInfoList));
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Get the test name
                            result = STEER_GetChildObjectString(parametersInfoObj,
                                                                STEER_JSON_TAG_TEST_NAME,
                                                                (char**)&((*parametersInfo)->testName));
                        }

                        if (result == STEER_RESULT_SUCCESS)
                        {
                            paramInfoObj = paramsArray->child;
                            while (paramInfoObj != NULL)
                            {
                                result = STEER_GetNextParameterInfo(paramInfoObj,
                                                                    &((*parametersInfo)->parameterInfoList->parameterInfo[(*parametersInfo)->parameterInfoList->count]));
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    (*parametersInfo)->parameterInfoList->count++;
                                    paramInfoObj = paramInfoObj->next;
                                }
                                else
                                    break;
                            }
                        }

                        // Check status
                        if (result != STEER_RESULT_SUCCESS)
                            (void)STEER_FreeParametersInfo(parametersInfo);
                    }
                }
            }

            // Clean up
            (void)cJSON_Delete(rootObj);
            rootObj = NULL;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_FreeParametersInfo
// =================================================================================================
int32_t STEER_FreeParametersInfo (tSTEER_ParametersInfo** parametersInfo)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(parametersInfo);
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;

        // Clean up
        STEER_FreeMemory((void**)&((*parametersInfo)->testName));

        for (i = 0; i < (*parametersInfo)->parameterInfoList->count; i++)
        {
            STEER_FreeMemory((void**)&((*parametersInfo)->parameterInfoList->parameterInfo[i].name));
            STEER_FreeMemory((void**)&((*parametersInfo)->parameterInfoList->parameterInfo[i].dataType));
            STEER_FreeMemory((void**)&((*parametersInfo)->parameterInfoList->parameterInfo[i].precision));
            STEER_FreeMemory((void**)&((*parametersInfo)->parameterInfoList->parameterInfo[i].units));
            STEER_FreeMemory((void**)&((*parametersInfo)->parameterInfoList->parameterInfo[i].defaultValue));
            STEER_FreeMemory((void**)&((*parametersInfo)->parameterInfoList->parameterInfo[i].minimumValue));
            STEER_FreeMemory((void**)&((*parametersInfo)->parameterInfoList->parameterInfo[i].maximumValue));
        }

        STEER_FreeMemory((void**)&((*parametersInfo)->parameterInfoList));
        STEER_FreeMemory((void**)parametersInfo);
    }
    return result;
}
                                                    
// =================================================================================================
//  STEER_ParametersInfoToJson
// =================================================================================================
int32_t STEER_ParametersInfoToJson (tSTEER_ParametersInfo* parametersInfo,
                                    char** parametersInfoJson)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parametersInfo);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(parametersInfoJson);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;

        // Setup
        *parametersInfoJson = NULL;

        // Create a root object
        rootObj = cJSON_CreateObject();
        result = STEER_CHECK_CONDITION((rootObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* paramsInfoObj = NULL;
            cJSON* paramsArray = NULL;
            cJSON_bool jsonResult = 0;
            uint_fast32_t i = 0;

            // Create a parameters info object
            paramsInfoObj = cJSON_CreateObject();
            result = STEER_CHECK_CONDITION((paramsInfoObj != NULL),
                                           STEER_RESULT_JSON_OPERATION_FAILURE);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the parameters info object to root
                jsonResult = cJSON_AddItemToObject(rootObj,
                                                   STEER_JSON_TAG_PARAMETERS_INFO,
                                                   paramsInfoObj);
                result = STEER_CHECK_CONDITION((jsonResult == 1),
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add test name to the parameters info object
                result = STEER_AddChildObjectString(paramsInfoObj,
                                                    STEER_JSON_TAG_TEST_NAME,
                                                    parametersInfo->testName);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add a parameters array to the parameters info object
                paramsArray = cJSON_CreateArray();
                result = STEER_CHECK_CONDITION((paramsArray != NULL),
                                                STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    jsonResult = cJSON_AddItemToObject(paramsInfoObj,
                                                        STEER_JSON_TAG_PARAMETERS,
                                                        paramsArray);
                    result = STEER_CHECK_CONDITION((jsonResult == 1),
                                                    STEER_RESULT_JSON_OPERATION_FAILURE);
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add parameters to parameters array
                for (i = 0; i < parametersInfo->parameterInfoList->count; i++)
                {
                    // Add parameter info
                    result = STEER_AddChildParameterInfo(paramsArray,
                                                         &(parametersInfo->parameterInfoList->parameterInfo[i]));

                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
            }
            if (result == STEER_RESULT_SUCCESS)
                *parametersInfoJson = cJSON_Print(rootObj);

            // Clean up
            (void)cJSON_Delete(rootObj);
        }
    }
    return result;
}

// =================================================================================================
