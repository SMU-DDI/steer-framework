// =================================================================================================
//! @file steer_parameter_set_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the parameter set utilities used by the STandard Entropy
//! Evaluation Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-03-31
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_parameter_set_utilities_private.h"
#include "steer_json_utilities.h"
#include "steer_json_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include "steer_value_utilities.h"
#include "steer_value_utilities_private.h"

// =================================================================================================
//  STEER_ValidateParameterSet
// =================================================================================================
int32_t STEER_ValidateParameterSet (const char* parameterSet,
                                    cJSON** rootObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Parse the JSON
    result = STEER_ParseJsonString(parameterSet, rootObject);
    if (result == STEER_RESULT_SUCCESS)
    {
        // TODO: Validate this JSON against the appropriate JSON schema

        // Make sure this JSON has the parameter set tag
        cJSON* obj = NULL;
        result = STEER_GetChildObject(*rootObject, STEER_JSON_TAG_PARAMETER_SET, &obj);

        // If the tag isn't found, this isn't a parameter set JSON structure
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
//  STEER_GetNextParameter
// =================================================================================================
int32_t STEER_GetNextParameter (cJSON* parameterObject,
                                tSTEER_Value* parameter)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(parameterObject);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the name
        result = STEER_GetChildObjectString(parameterObject,
                                            STEER_JSON_TAG_NAME,
                                            (char**)&(parameter->name));

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the data type
            result = STEER_GetChildObjectString(parameterObject,
                                                STEER_JSON_TAG_DATA_TYPE,
                                                (char**)&(parameter->dataType));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the units (optional)
            result = STEER_GetChildObjectString(parameterObject,
                                                STEER_JSON_TAG_UNITS,
                                                (char**)&(parameter->units));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the precision (optional)
            result = STEER_GetChildObjectString(parameterObject,
                                                STEER_JSON_TAG_PRECISION,
                                                (char**)&(parameter->precision));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the value
            result = STEER_GetChildObjectString(parameterObject,
                                                STEER_JSON_TAG_VALUE,
                                                (char**)&(parameter->value));
        }
    }
    return result;
}

// =================================================================================================
//  STEER_DefaultParameterCheck
// =================================================================================================
int32_t STEER_DefaultParameterCheck (tSTEER_ParameterInfo* parameterInfo,
                                     tSTEER_ParameterSet** parameterSet)
{
    int32_t result = STEER_RESULT_SUCCESS;
    uint_fast32_t i = 0;
    bool parameterInSet = false;

    // Make sure all default parameters are included in the set
    for (i = 0; i < (*parameterSet)->count; i++)
    {
        if (strcmp(parameterInfo->name, (*parameterSet)->parameter[i].name) == 0)
        {
            parameterInSet = true;
            break;
        }
    }

    // Is this parameter not in the set?
    if (!parameterInSet)
    {
        // Make room for new default paramter
        result = STEER_ReallocateMemory(sizeof(tSTEER_ParameterSet) + ((*parameterSet)->count * sizeof(tSTEER_Value)),
                                        sizeof(tSTEER_ParameterSet) + (((*parameterSet)->count + 1) * sizeof(tSTEER_Value)),
                                        (void**)parameterSet);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy it
            i = (*parameterSet)->count;

            // Get the name
            result = STEER_DuplicateString(parameterInfo->name,
                                           (char**)&((*parameterSet)->parameter[i].name));
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the data type
                result = STEER_DuplicateString(parameterInfo->dataType,
                                               (char**)&((*parameterSet)->parameter[i].dataType));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the units (optional)
                if (parameterInfo->units != NULL)
                {
                    result = STEER_DuplicateString(parameterInfo->units,
                                                   (char**)&((*parameterSet)->parameter[i].units));
                }
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the precision (optional)
                if (parameterInfo->precision != NULL)
                {
                    result = STEER_DuplicateString(parameterInfo->precision,
                                                   (char**)&((*parameterSet)->parameter[i].precision));
                }
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the default value
                result = STEER_DuplicateString(parameterInfo->defaultValue,
                                               (char**)&((*parameterSet)->parameter[i].value));
            }
            if (result == STEER_RESULT_SUCCESS)
                (*parameterSet)->count++;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_JsonToParameterSet
// =================================================================================================
int32_t STEER_JsonToParameterSet (const char* parameterSetJson,
                                  tSTEER_ParametersInfo* parametersInfo,
                                  tSTEER_ParameterSet** parameters)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parametersInfo);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(parameters);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;
        cJSON* paramsArray = NULL;
        cJSON* parameterSetObj = NULL;
        cJSON* paramObj = NULL;
        int paramsArraySize = 0;
        uint_fast32_t i = 0;

        // Setup
        *parameters = NULL;

        // Were parameters provided?
        if (parameterSetJson != NULL)
        {
            // Yes, parse JSON
            result = STEER_ValidateParameterSet(parameterSetJson, &rootObj);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the parameter set object
                result = STEER_GetChildObject(rootObj,
                                              STEER_JSON_TAG_PARAMETER_SET,
                                              &parameterSetObj);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the parameters array
                    result = STEER_GetChildObject(parameterSetObj,
                                                  STEER_JSON_TAG_PARAMETERS,
                                                  &paramsArray);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the size of the parameters array
                        paramsArraySize = cJSON_GetArraySize(paramsArray);
                        result = STEER_CHECK_CONDITION((paramsArraySize > 0), 
                                                       STEER_RESULT_JSON_INVALID_CONSTRUCTION);
                    }
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate space
                result = STEER_AllocateMemory(sizeof(tSTEER_ParameterSet) +
                                              (parametersInfo->parameterInfoList->count * sizeof(tSTEER_Value)),
                                              (void**)parameters);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the test name
                    result = STEER_GetChildObjectString(parameterSetObj,
                                                        STEER_JSON_TAG_TEST_NAME,
                                                        &((*parameters)->testName));
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Get the parameter set name
                        result = STEER_GetChildObjectString(parameterSetObj,
                                                            STEER_JSON_TAG_PARAMETER_SET_NAME,
                                                            &((*parameters)->parameterSetName));
                    }

                    // Get the parameters
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        paramObj = paramsArray->child;
                        while (paramObj != NULL)
                        {
                            result = STEER_GetNextParameter(paramObj,
                                                            &((*parameters)->parameter[(*parameters)->count]));
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                (*parameters)->count++;
                                paramObj = paramObj->next;
                            }
                        }
                    }

                    // Check for all default parameters
                    for (i = 0; i < parametersInfo->parameterInfoList->count; i++)
                    {
                        result = STEER_DefaultParameterCheck(&(parametersInfo->parameterInfoList->parameterInfo[i]),
                                                             parameters);
                        if (result != STEER_RESULT_SUCCESS)
                            break;
                    }

                    // Check status
                    if (result != STEER_RESULT_SUCCESS)
                        (void)STEER_FreeParameterSet(parameters);
                }
            }

            if (rootObj != NULL)
            {
                // Clean up
                (void)cJSON_Delete(rootObj);
                rootObj = NULL;
            }
        }
        else    // No parameters were provided - use the defaults
        {
            // Allocate space
            result = STEER_AllocateMemory(sizeof(tSTEER_ParameterSet) +
                                          parametersInfo->parameterInfoList->count * sizeof(tSTEER_Value),
                                          (void**)parameters);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the test name
                result = STEER_DuplicateString(parametersInfo->testName,
                                               &((*parameters)->testName));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Set the parameter set name
                result = STEER_DuplicateString("default",
                                               &((*parameters)->parameterSetName));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                for (i = 0; i < parametersInfo->parameterInfoList->count; i++)
                {
                    // Copy name
                    result = STEER_DuplicateString(parametersInfo->parameterInfoList->parameterInfo[i].name,
                                                   (char**)&((*parameters)->parameter[i].name));
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Copy data type
                        result = STEER_DuplicateString(parametersInfo->parameterInfoList->parameterInfo[i].dataType,
                                                       (char**)&((*parameters)->parameter[i].dataType));
                    }
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Copy units (optional)
                        if (parametersInfo->parameterInfoList->parameterInfo[i].units != NULL)
                        {
                            result = STEER_DuplicateString(parametersInfo->parameterInfoList->parameterInfo[i].units,
                                                           (char**)&((*parameters)->parameter[i].units));
                        }
                    }
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Copy precision (optional)
                        if (parametersInfo->parameterInfoList->parameterInfo[i].precision != NULL)
                        {
                            result = STEER_DuplicateString(parametersInfo->parameterInfoList->parameterInfo[i].precision,
                                                           (char**)&((*parameters)->parameter[i].precision));
                        }
                    }
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Copy default value
                        result = STEER_DuplicateString(parametersInfo->parameterInfoList->parameterInfo[i].defaultValue,
                                                       (char**)&((*parameters)->parameter[i].value));
                    }
                    if (result == STEER_RESULT_SUCCESS)
                        (*parameters)->count++;
                    else
                        break;
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ParameterSetToJson
// =================================================================================================
int32_t STEER_ParameterSetToJson (tSTEER_ParameterSet* parameterSet,
                                  char** parameterSetJson)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parameterSet);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(parameterSetJson);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;

        // Setup
        *parameterSetJson = NULL;   

        // Create a root object
        rootObj = cJSON_CreateObject();
        result = STEER_CHECK_CONDITION((rootObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* paramSetObj = NULL;
            cJSON* paramsArray = NULL;
            cJSON_bool jsonResult = 0;
            uint_fast32_t i = 0;

            // Create a parameter set object
            paramSetObj = cJSON_CreateObject();
            result = STEER_CHECK_CONDITION((paramSetObj != NULL),
                                           STEER_RESULT_JSON_OPERATION_FAILURE);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the parameter set object to root
                jsonResult = cJSON_AddItemToObject(rootObj,
                                                   STEER_JSON_TAG_PARAMETER_SET,
                                                   paramSetObj);
                result = STEER_CHECK_CONDITION((jsonResult == 1), 
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add a parameters array to the parameter set object
                    paramsArray = cJSON_CreateArray();
                    result = STEER_CHECK_CONDITION((paramsArray != NULL),
                                                   STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        jsonResult = cJSON_AddItemToObject(paramSetObj,
                                                           STEER_JSON_TAG_PARAMETERS,
                                                           paramsArray);
                        result = STEER_CHECK_CONDITION((jsonResult == 1),
                                                       STEER_RESULT_JSON_OPERATION_FAILURE);
                    }
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add test name
                result = STEER_AddChildObjectString(paramSetObj,
                                                    STEER_JSON_TAG_TEST_NAME,
                                                    parameterSet->testName);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add parameter set name
                result = STEER_AddChildObjectString(paramSetObj,
                                                    STEER_JSON_TAG_PARAMETER_SET_NAME,
                                                    parameterSet->parameterSetName);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add parameters
                for (i = 0; i < parameterSet->count; i++)
                {
                    result = STEER_AddChildValue(paramsArray,
                                                 &(parameterSet->parameter[i]));
                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
            }

            if (result == STEER_RESULT_SUCCESS)
                *parameterSetJson = cJSON_Print(rootObj);

            // Clean up
            (void)cJSON_Delete(rootObj);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_FreeParameterSet
// =================================================================================================
int32_t STEER_FreeParameterSet (tSTEER_ParameterSet** parameterSet)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER(parameterSet);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*parameterSet != NULL)
        {
            uint_fast32_t i = 0;

            STEER_FreeMemory((void**)&((*parameterSet)->testName));
            STEER_FreeMemory((void**)&((*parameterSet)->parameterSetName));

            for (i = 0; i < (*parameterSet)->count; i++)
            {
                STEER_FreeMemory((void**)&((*parameterSet)->parameter[i].name));
                STEER_FreeMemory((void**)&((*parameterSet)->parameter[i].dataType));
                STEER_FreeMemory((void**)&((*parameterSet)->parameter[i].precision));
                STEER_FreeMemory((void**)&((*parameterSet)->parameter[i].units));
                STEER_FreeMemory((void**)&((*parameterSet)->parameter[i].value));
            }
        }

        STEER_FreeMemory((void**)parameterSet);
    }
    return result;
}

// =================================================================================================
//  STEER_GetParameterValue
// =================================================================================================
int32_t STEER_GetParameterValue (tSTEER_ParameterSet* parameterSet,
                                 const char* parameterName,
                                 void** parameterValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parameterSet);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(parameterName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(parameterValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;

        // Setup
        *parameterValue = NULL;

        // Look for the specified parameter in the parameter set
        for (i = 0; i < parameterSet->count; i++)
        {
            // Check the name
            if (strcmp(parameterName, parameterSet->parameter[i].name) == 0)
            {
                // Found it; get the value
                result = STEER_GetNativeValue(parameterSet->parameter[i].dataType,
                                              parameterSet->parameter[i].value,
                                              parameterValue);
                break;
            }
        }
    }
    return result;
}
                                    
// =================================================================================================
