// =================================================================================================
//! @file steer_value_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements value functions for the STandard Entropy Evaluation Report (STEER) 
//! framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-28
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_json_utilities.h"
#include "steer_json_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_string_utilities_private.h"
#include "steer_utilities.h"
#include "steer_value_utilities.h"
#include "steer_value_utilities_private.h"

// =================================================================================================
//  STEER_AddChildValue
// =================================================================================================
int32_t STEER_AddChildValue (cJSON* parentArray,
                             tSTEER_Value* value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentArray);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* valueObj = NULL;

        // Create an object
        valueObj = cJSON_CreateObject();
        result = STEER_CHECK_CONDITION((valueObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add the name
            result = STEER_AddChildObjectString(valueObj, STEER_JSON_TAG_NAME, value->name);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the data type
                result = STEER_AddChildObjectString(valueObj, STEER_JSON_TAG_DATA_TYPE, value->dataType);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the units (optional)
                if (value->units != NULL)
                    result = STEER_AddChildObjectString(valueObj, STEER_JSON_TAG_UNITS, value->units);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the precision (optional)
                if (value->precision != NULL)
                    result = STEER_AddChildObjectString(valueObj, STEER_JSON_TAG_PRECISION, value->precision);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the value
                result = STEER_AddChildObjectString(valueObj, STEER_JSON_TAG_VALUE, value->value);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the object to the array
                cJSON_bool jsonResult = cJSON_AddItemToArray(parentArray, valueObj);
                result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddChildValueSet
// =================================================================================================
int32_t STEER_AddChildValueSet (cJSON* parentArray,
                                tSTEER_ValueSet* valueSet)
{
    int32_t result = STEER_RESULT_SUCCESS;
    
    // Check arguments
    result = STEER_CHECK_POINTER(parentArray);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(valueSet);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* valueSetObj = NULL;
        cJSON_bool jsonResult = 0;

        // Create an object
        valueSetObj = cJSON_CreateObject();
        result = STEER_CHECK_CONDITION((valueSetObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add the name
            result = STEER_AddChildObjectString(valueSetObj, STEER_JSON_TAG_NAME, valueSet->name);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the data type
                result = STEER_AddChildObjectString(valueSetObj, STEER_JSON_TAG_DATA_TYPE, valueSet->dataType);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the units (optional)
                if (valueSet->units != NULL)
                    result = STEER_AddChildObjectString(valueSetObj, STEER_JSON_TAG_UNITS, valueSet->units);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the precision (optional)
                if (valueSet->precision != NULL)
                    result = STEER_AddChildObjectString(valueSetObj, STEER_JSON_TAG_PRECISION, valueSet->precision);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                cJSON* valuesArray = NULL;

                // Create a values array
                valuesArray = cJSON_CreateArray();
                result = STEER_CHECK_CONDITION((valuesArray != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    cJSON* valueObj = NULL;
                    uint_fast32_t i = 0;

                    // Add the values
                    for (i = 0; i < valueSet->count; i++)
                    {
                        valueObj = cJSON_CreateObject();
                        result = STEER_CHECK_CONDITION((valueObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Add the label
                            result = STEER_AddChildObjectString(valueObj, STEER_JSON_TAG_LABEL, valueSet->item[i].label);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Add the value
                                result = STEER_AddChildObjectString(valueObj, STEER_JSON_TAG_VALUE, valueSet->item[i].value);
                            }
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Add the object
                                jsonResult = cJSON_AddItemToArray(valuesArray, valueObj);
                                result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                            }
                        }
                    }

                    // Check status
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Add the values array
                        jsonResult = cJSON_AddItemToObject(valueSetObj, STEER_JSON_TAG_VALUES, valuesArray);
                        result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                    }
                }
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the value set object
                jsonResult = cJSON_AddItemToArray(parentArray, valueSetObj);
                result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetChildValue
// =================================================================================================
int32_t STEER_GetChildValue (cJSON* parentObject,
                             tSTEER_Value* value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the name
        result = STEER_GetChildObjectString(parentObject,
                                            STEER_JSON_TAG_NAME,
                                            (char**)&(value->name));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the data type
            result = STEER_GetChildObjectString(parentObject,
                                                STEER_JSON_TAG_DATA_TYPE,
                                                (char**)&(value->dataType));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the precision (optional)
            result = STEER_GetChildObjectString(parentObject,
                                                STEER_JSON_TAG_PRECISION,
                                                (char**)&(value->precision));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the units (optional)
            result = STEER_GetChildObjectString(parentObject,
                                                STEER_JSON_TAG_UNITS,
                                                (char**)&(value->units));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the value
            result = STEER_GetChildObjectString(parentObject,
                                                STEER_JSON_TAG_VALUE,
                                                (char**)&(value->value));
        }
    }
    return result;    
}

// =================================================================================================
//  STEER_GetChildValueSet
// =================================================================================================
int32_t STEER_GetChildValueSet (cJSON* parentObject,
                                tSTEER_ValueSet** valueSet)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(valueSet);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Allocate a value set
        result = STEER_AllocateMemory(sizeof(tSTEER_ValueSet), 
                                      (void**)valueSet);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the name
            result = STEER_GetChildObjectString(parentObject,
                                                STEER_JSON_TAG_NAME,
                                                (char**)&((*valueSet)->name));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the data type
            result = STEER_GetChildObjectString(parentObject,
                                                STEER_JSON_TAG_DATA_TYPE,
                                                (char**)&((*valueSet)->dataType));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the precision (optional)
            result = STEER_GetChildObjectString(parentObject,
                                                STEER_JSON_TAG_PRECISION,
                                                (char**)&((*valueSet)->precision));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the units (optional)
            result = STEER_GetChildObjectString(parentObject,
                                                STEER_JSON_TAG_UNITS,
                                                (char**)&((*valueSet)->units));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* valuesArray = NULL;
            uint32_t valuesArraySize = 0;

            // Get the values array
            result = STEER_GetChildArray(parentObject, STEER_JSON_TAG_VALUES, 
                                         &valuesArray, &valuesArraySize);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate space
                result = STEER_ReallocateMemory(sizeof(tSTEER_ValueSet),
                                                sizeof(tSTEER_ValueSet) + (sizeof(tSTEER_ValueItem) * valuesArraySize),
                                                (void**)valueSet);
                if (result == STEER_RESULT_SUCCESS)
                {
                    cJSON* valueItemObj = NULL;
                    uint_fast32_t i = 0;

                    for (i = 0; i < valuesArraySize; i++)
                    {
                        valueItemObj = cJSON_GetArrayItem(valuesArray, i);
                        result = STEER_CHECK_CONDITION((valueItemObj != NULL),
                                                       STEER_RESULT_JSON_OPERATION_FAILURE);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            result = STEER_GetChildObjectString(valueItemObj,
                                                                STEER_JSON_TAG_LABEL,
                                                                (char**)&((*valueSet)->item[(*valueSet)->count].label));
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                result = STEER_GetChildObjectString(valueItemObj,
                                                                    STEER_JSON_TAG_VALUE,
                                                                    (char**)&((*valueSet)->item[(*valueSet)->count].value));
                            }
                            if (result == STEER_RESULT_SUCCESS)
                                (*valueSet)->count++;
                        }

                        if (result != STEER_RESULT_SUCCESS)
                            break;
                    }
                }
            }
        }

        if (result != STEER_RESULT_SUCCESS)
            (void)STEER_FreeValueSet(valueSet);
    }
    return result;    
}

// =================================================================================================
//  STEER_FreeValues
// =================================================================================================
int32_t STEER_FreeValues (tSTEER_Values** values)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(values);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*values != NULL)
        {
            uint_fast32_t i = 0;

            for (i = 0; i < (*values)->count; i++)
            {
                result = STEER_FreeValue(&((*values)->value[i]));
            }
        }
        STEER_FreeMemory((void**)values);
    }
    return result;
}

// =================================================================================================
//  STEER_FreeValueSets
// =================================================================================================
int32_t STEER_FreeValueSets (tSTEER_ValueSets** valueSets)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(valueSets);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*valueSets != NULL)
        {
            uint_fast32_t i = 0;

            for (i = 0; i < (*valueSets)->count; i++)
            {
                result = STEER_FreeValueSet(&((*valueSets)->valueSet[i]));
            }
        }
        STEER_FreeMemory((void**)valueSets);
    }
    return result;
}

// =================================================================================================
//  STEER_AddValuesToArray
// =================================================================================================
int32_t STEER_AddValuesToArray (cJSON* parentArray,
                                tSTEER_Values* values)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentArray);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(values);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;

        // Walk the values
        for (i = 0; i < values->count; i++)
        {
            // Add value to array
            result = STEER_AddChildValue(parentArray, &(values->value[i]));
            if (result != STEER_RESULT_SUCCESS)
                break;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddValueSetsToArray
// =================================================================================================
int32_t STEER_AddValueSetsToArray (cJSON* parentArray,
                                   tSTEER_ValueSets* valueSets)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentArray);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(valueSets);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;

        // Walk the values
        for (i = 0; i < valueSets->count; i++)
        {
            // Add value set to array
            result = STEER_AddChildValueSet(parentArray, valueSets->valueSet[i]);
            if (result != STEER_RESULT_SUCCESS)
                break;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetValuesFromArray
// =================================================================================================
int32_t STEER_GetValuesFromArray (cJSON* parentArray,
                                  tSTEER_Values** values,
                                  tSTEER_ValueSets** valueSets)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentArray);
    if (result == STEER_RESULT_SUCCESS)
    {
        uint32_t valueCount = 0;
        uint32_t valueSetCount = 0;
        uint32_t arraySize = cJSON_GetArraySize(parentArray);

        if (arraySize > 0)
        {
            cJSON* valueObj = NULL;
            uint_fast32_t i = 0;

            for (i = 0; i < arraySize; i++)
            {
                valueObj = cJSON_GetArrayItem(parentArray, i);
                result = STEER_CHECK_CONDITION((valueObj != NULL),
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    if (STEER_HasChildTag(valueObj, STEER_JSON_TAG_VALUE))
                        valueCount++;
                    else
                    {
                        if (STEER_HasChildTag(valueObj, STEER_JSON_TAG_VALUES))
                            valueSetCount++;
                    }
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate space for values
                if ((valueCount > 0) && (values != NULL))
                {
                    result = STEER_AllocateMemory(sizeof(tSTEER_Values) +
                                                  (sizeof(tSTEER_Value) * valueCount),
                                                  (void**)values);
                }

                // Allocate space for value sets
                if (result == STEER_RESULT_SUCCESS)
                {
                    if ((valueSetCount > 0) && (valueSets != NULL))
                    {
                        result = STEER_AllocateMemory(sizeof(tSTEER_ValueSets) +
                                                      (sizeof(tSTEER_ValueSet) * valueSetCount),
                                                      (void**)valueSets);
                    }
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Get the values and value sets
                for (i = 0; i < arraySize; i++)
                {
                    valueObj = cJSON_GetArrayItem(parentArray, i);
                    result = STEER_CHECK_CONDITION((valueObj != NULL), 
                                                STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        if (STEER_HasChildTag(valueObj, STEER_JSON_TAG_VALUE))
                        {
                            // This is a value
                            if (values != NULL)
                            {
                                result = STEER_GetChildValue(valueObj, 
                                                             &((*values)->value[(*values)->count]));
                                (*values)->count++;
                            }
                        }
                        else
                        {
                            if (STEER_HasChildTag(valueObj, STEER_JSON_TAG_VALUES))
                            {
                                // This is a value set
                                if (valueSets != NULL)
                                {
                                    result = STEER_GetChildValueSet(valueObj, 
                                                                    &((*valueSets)->valueSet[(*valueSets)->count]));
                                    (*valueSets)->count++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetNativeValue
// =================================================================================================
int32_t STEER_GetNativeValue (const char* dataType,
                              const char* value,
                              void** nativeValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(dataType);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(value);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(nativeValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *nativeValue = NULL;

        // Check the data type
        if (strcmp(dataType, STEER_JSON_VALUE_BOOLEAN) == 0)
        {
            result = STEER_AllocateMemory(sizeof(bool), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToBoolean(value, 
                                                      (bool*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT) == 0)
        {
            result = STEER_AllocateMemory(sizeof(double), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToDoublePrecisionFloatingPoint(value, 
                                                                           (double*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_EXTENDED_PRECISION_FLOATING_POINT) == 0)
        {
            result = STEER_AllocateMemory(sizeof(long double), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToExtendedPrecisionFloatingPoint(value, 
                                                                             (long double*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_SIGNED_8_BIT_INTEGER) == 0)
        {
            result = STEER_AllocateMemory(sizeof(int8_t), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToSigned8BitInteger(value, 
                                                                (int8_t*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_SIGNED_16_BIT_INTEGER) == 0)
        {
            result = STEER_AllocateMemory(sizeof(int16_t), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToSigned16BitInteger(value, 
                                                                 (int16_t*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER) == 0)
        {
            result = STEER_AllocateMemory(sizeof(int32_t), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToSigned32BitInteger(value, 
                                                                 (int32_t*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_SIGNED_64_BIT_INTEGER) == 0)
        {
            result = STEER_AllocateMemory(sizeof(int64_t), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToSigned64BitInteger(value, 
                                                                 (int64_t*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_SINGLE_PRECISION_FLOATING_POINT) == 0)
        {
            result = STEER_AllocateMemory(sizeof(float), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToSinglePrecisionFloatingPoint(value, 
                                                                           (float*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_UNSIGNED_8_BIT_INTEGER) == 0)
        {
            result = STEER_AllocateMemory(sizeof(uint8_t), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToUnsigned8BitInteger(value, 
                                                                  (uint8_t*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_UNSIGNED_16_BIT_INTEGER) == 0)
        {
            result = STEER_AllocateMemory(sizeof(uint16_t), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToUnsigned16BitInteger(value, 
                                                                   (uint16_t*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER) == 0)
        {
            result = STEER_AllocateMemory(sizeof(uint32_t), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToUnsigned32BitInteger(value, 
                                                                   (uint32_t*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER) == 0)
        {
            result = STEER_AllocateMemory(sizeof(uint64_t), nativeValue);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_ConvertStringToUnsigned64BitInteger(value, 
                                                                   (uint64_t*)(*nativeValue));
        }
        else if (strcmp(dataType, STEER_JSON_VALUE_UTF_8_STRING) == 0)
        {
            result = STEER_DuplicateString(value, (char**)nativeValue);
        }
        else
            result = STEER_CHECK_ERROR(EINVAL);

        // Check status
        if (result != STEER_RESULT_SUCCESS)
            STEER_FreeMemory(nativeValue);
    }
    return result;
}

// =================================================================================================
//  STEER_NewValue
// =================================================================================================
int32_t STEER_NewValue (const char* name,
                        const char* dataType,
                        const char* precision,
                        const char* units,
                        const char* value,
                        tSTEER_Value* theValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(name);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(dataType);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(value);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(theValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        memset((void*)theValue, 0, sizeof(tSTEER_Value));
        result = STEER_DuplicateString(name, (char**)&(theValue->name));
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(dataType, (char**)&(theValue->dataType));
        if (result == STEER_RESULT_SUCCESS)
        {
            if (precision != NULL)
                result = STEER_DuplicateString(precision, (char**)&(theValue->precision));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            if (units != NULL)
                result = STEER_DuplicateString(units, (char**)&(theValue->units));
        }
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_DuplicateString(value, (char**)&(theValue->value));
    }
    return result;
}

// =================================================================================================
//  STEER_DuplicateValue
// =================================================================================================
int32_t STEER_DuplicateValue (tSTEER_Value* sourceValue,
                              tSTEER_Value* duplicatedValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(sourceValue);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(duplicatedValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Copy the name
        result = STEER_DuplicateString(sourceValue->name, 
                                       (char**)&(duplicatedValue->name));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the data type
            result = STEER_DuplicateString(sourceValue->dataType, 
                                           (char**)&(duplicatedValue->dataType));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the precision (optional)
            if (sourceValue->precision != NULL)
                result = STEER_DuplicateString(sourceValue->precision, 
                                               (char**)&(duplicatedValue->precision));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the units (optional)
            if (sourceValue->units != NULL)
                result = STEER_DuplicateString(sourceValue->units,
                                               (char**)&(duplicatedValue->units));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the value
            result = STEER_DuplicateString(sourceValue->value,
                                           (char**)&(duplicatedValue->value));
        }

        // Check status
        if (result != STEER_RESULT_SUCCESS)
            (void)STEER_FreeValue(duplicatedValue);
    }
    return result;
}

// =================================================================================================
//  STEER_FreeValue
// =================================================================================================
int32_t STEER_FreeValue (tSTEER_Value* value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(value);
    if (result == STEER_RESULT_SUCCESS)
    {
        STEER_FreeMemory((void**)&(value->name));
        STEER_FreeMemory((void**)&(value->dataType));
        STEER_FreeMemory((void**)&(value->precision));
        STEER_FreeMemory((void**)&(value->units));
        STEER_FreeMemory((void**)&(value->value));
    }
    return result;
}

// =================================================================================================
//  STEER_NewValueSet
// =================================================================================================
int32_t STEER_NewValueSet (const char* name,
                           const char* dataType,
                           const char* precision,
                           const char* units,
                           tSTEER_ValueSet** theValueSet)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(name);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(dataType);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (precision != NULL)
            result = STEER_CHECK_STRING(precision);
    }
    if (result == STEER_RESULT_SUCCESS)
    {
        if (units != NULL)
            result = STEER_CHECK_STRING(units);
    }

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        result = STEER_AllocateMemory(sizeof(tSTEER_ValueSet), (void**)theValueSet);
        if (result == STEER_RESULT_SUCCESS)
        {
            result = STEER_DuplicateString(name, (char**)&((*theValueSet)->name));
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_DuplicateString(dataType, (char**)&((*theValueSet)->dataType));
            if (result == STEER_RESULT_SUCCESS)
            {
                if (precision != NULL)
                    result = STEER_DuplicateString(precision, (char**)&((*theValueSet)->precision));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                if (units != NULL)
                    result = STEER_DuplicateString(units, (char**)&((*theValueSet)->units));
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddValueToValueSet
// =================================================================================================
int32_t STEER_AddValueToValueSet(const char* label,
                                 const char* value,
                                 tSTEER_ValueSet** theValueSet)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_STRING(label);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(value);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(theValueSet);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(*theValueSet);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Reallocate
        result = STEER_ReallocateMemory(sizeof(tSTEER_ValueSet) + ((*theValueSet)->count * sizeof(tSTEER_ValueItem)),
                                        sizeof(tSTEER_ValueSet) + (((*theValueSet)->count + 1) * sizeof(tSTEER_ValueItem)),
                                        (void**)theValueSet);
        if (result == STEER_RESULT_SUCCESS)
        {
            result = STEER_DuplicateString(label, 
                                           (char**)&(((*theValueSet)->item[(*theValueSet)->count].label)));
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_DuplicateString(value, 
                                               (char**)&(((*theValueSet)->item[(*theValueSet)->count].value)));
                if (result == STEER_RESULT_SUCCESS)
                    (*theValueSet)->count++;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetValueFromValueSet
// =================================================================================================
int32_t STEER_GetValueFromValueSet (tSTEER_ValueSet* valueSet,
                                    const char* label,
                                    char** value)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(valueSet);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(label);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(value);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        bool foundIt = false;

        // Setup
        *value = NULL;

        for (i = 0; i < valueSet->count; i++)
        {
            if (strcmp(valueSet->item[i].label, label) == 0)
            {
                result = STEER_DuplicateString(valueSet->item[i].value, value);
                foundIt = true;
                break;
            }
        }

        if (!foundIt)
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}

// =================================================================================================
//  STEER_DuplicateValueSet
// =================================================================================================
int32_t STEER_DuplicateValueSet (tSTEER_ValueSet* sourceValueSet,
                                 tSTEER_ValueSet** duplicatedValueSet)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(sourceValueSet);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(duplicatedValueSet);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *duplicatedValueSet = NULL;
        result = STEER_AllocateMemory(sizeof(tSTEER_ValueSet), (void**)duplicatedValueSet);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the name
            result = STEER_DuplicateString(sourceValueSet->name,
                                        (char**)&((*duplicatedValueSet)->name));
            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy the data type
                result = STEER_DuplicateString(sourceValueSet->dataType,
                                            (char**)&((*duplicatedValueSet)->dataType));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy the precision (optional)
                if (sourceValueSet->precision != NULL)
                    result = STEER_DuplicateString(sourceValueSet->precision,
                                                (char**)&((*duplicatedValueSet)->precision));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy the units (optional)
                if (sourceValueSet->units != NULL)
                    result = STEER_DuplicateString(sourceValueSet->units,
                                                (char**)&((*duplicatedValueSet)->units));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                uint_fast32_t i = 0;

                // Copy the values
                for (i = 0; i < sourceValueSet->count; i++)
                {
                    // Copy the label
                    result = STEER_AddValueToValueSet(sourceValueSet->item[i].label,
                                                      sourceValueSet->item[i].value,
                                                      duplicatedValueSet);
                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
            {
                if ((duplicatedValueSet != NULL) && (*duplicatedValueSet != NULL))
                    (*duplicatedValueSet)->count = sourceValueSet->count;
            }
            else
                (void)STEER_FreeValueSet(duplicatedValueSet);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_FreeValueSet
// =================================================================================================
int32_t STEER_FreeValueSet (tSTEER_ValueSet** valueSet)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(valueSet);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*valueSet != NULL)
        {
            uint_fast32_t i = 0;

            STEER_FreeMemory((void**)&((*valueSet)->name));
            STEER_FreeMemory((void**)&((*valueSet)->dataType));
            STEER_FreeMemory((void**)&((*valueSet)->precision));
            STEER_FreeMemory((void**)&((*valueSet)->units));

            for (i = 0; i < (*valueSet)->count; i++)
            {
                STEER_FreeMemory((void**)&(((*valueSet)->item[i].label)));
                STEER_FreeMemory((void**)&(((*valueSet)->item[i].value)));
            }
        }

        STEER_FreeMemory((void**)valueSet);
    }
    return result;
}

// =================================================================================================
