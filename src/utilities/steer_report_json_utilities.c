// =================================================================================================
//! @file steer_report_json_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the report JSON utilities for the STandard Entropy Evaluation
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-04-16
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_report_utilities_private.h"
#include "steer_report_utilities.h"
#include "steer_json_utilities.h"
#include "steer_json_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_string_utilities_private.h"
#include "steer_utilities.h"
#include "steer_value_utilities.h"
#include "steer_value_utilities_private.h"

// =================================================================================================
//  STEER_ValidateReport
// =================================================================================================
int32_t STEER_ValidateReport (const char* reportJson,
                              cJSON** rootObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Parse the JSON
    result = STEER_ParseJsonString(reportJson, rootObject);
    if (result == STEER_RESULT_SUCCESS)
    {
        // TODO: Validate this JSON against the appropriate JSON schema

        // Make sure this JSON has the report tag
        cJSON* obj = NULL;
        result = STEER_GetChildObject(*rootObject, STEER_JSON_TAG_REPORT, &obj);

        // If the tag isn't found, this isn't a report JSON structure
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
//  STEER_GetReportHeaderFromReportObject
// =================================================================================================
int32_t STEER_GetReportHeaderFromReportObject (cJSON* reportObject,
                                               tSTEER_ReportPrivate* report)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(reportObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(report);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get test name
        result = STEER_GetChildObjectString(reportObject, 
                                            STEER_JSON_TAG_TEST_NAME, 
                                            (char**)&(report->testName));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get test suite (optional)
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_TEST_SUITE,
                                                (char**)&(report->testSuite));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get test description
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_TEST_DESCRIPTION,
                                                (char**)&(report->testDescription));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get schedule ID (optional)
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_SCHEDULE_ID,
                                                (char**)&(report->scheduleId));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get test conductor (optional)
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_TEST_CONDUCTOR,
                                                (char**)&(report->testConductor));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get test notes (optional)
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_TEST_NOTES,
                                                (char**)&(report->testNotes));
            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get report level (optional)
            char* reportLevel = NULL;
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_REPORT_LEVEL,
                                                (char**)&reportLevel);
            if (result == STEER_RESULT_SUCCESS)
            {
                if (strcmp(reportLevel, STEER_JSON_VALUE_SUMMARY) == 0)
                    report->level = eSTEER_ReportLevel_Summary;
                else if (strcmp(reportLevel, STEER_JSON_VALUE_STANDARD) == 0)
                    report->level = eSTEER_ReportLevel_Standard;
                else if (strcmp(reportLevel, STEER_JSON_VALUE_FULL) == 0)
                    report->level = eSTEER_ReportLevel_Full;
                else    
                    result = STEER_CHECK_ERROR(STEER_RESULT_JSON_INVALID_CONSTRUCTION);
                STEER_FreeMemory((void**)&reportLevel);
            }
            else if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                result = STEER_RESULT_SUCCESS;  // Eat this result
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get program name
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_PROGRAM_NAME,
                                                (char**)&(report->programName));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get program version
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_PROGRAM_VERSION,
                                                (char**)&(report->programVersion));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get operating system
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_OPERATING_SYSTEM,
                                                (char**)&(report->operatingSystem));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get architecture
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_ARCHITECTURE,
                                                (char**)&(report->architecture));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get entropy source
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_ENTROPY_SOURCE,
                                                (char**)&(report->entropySource));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get timestamp
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_START_TIME,
                                                (char**)&(report->startTime));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get duration
            result = STEER_GetChildObjectString(reportObject,
                                                STEER_JSON_TAG_TEST_DURATION,
                                                (char**)&(report->duration));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get criteria
            result = STEER_GetCriteriaFromParentObject(reportObject,
                                                       &(report->criteria));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get evaluation
            result = STEER_GetEvaluationFromParentObject(reportObject,
                                                         &(report->evaluation));
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetParametersFromReportObject
// =================================================================================================
int32_t STEER_GetParametersFromReportObject (cJSON* reportObject,
                                             tSTEER_ParameterSet** parameters)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(reportObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(parameters);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* paramsArray = NULL;
        uint32_t paramsArraySize = 0;

        // Get the parameters array
        result = STEER_GetChildArray(reportObject, STEER_JSON_TAG_PARAMETERS, 
                                     &paramsArray, &paramsArraySize);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Free original parameter set
            result = STEER_ReallocateMemory(sizeof(tSTEER_ParameterSet),
                                            sizeof(tSTEER_ParameterSet) + 
                                            (sizeof(tSTEER_Value) * paramsArraySize),
                                            (void**)parameters);

            if (result == STEER_RESULT_SUCCESS)
            {
                cJSON* paramObj = NULL;
                uint_fast32_t i = 0;

                // Set count
                (*parameters)->count = paramsArraySize;

                // Get the parameters
                for (i = 0; i < paramsArraySize; i++)
                {
                    paramObj = cJSON_GetArrayItem(paramsArray, i);
                    result = STEER_CHECK_POINTER(paramObj);
                    if (result == STEER_RESULT_SUCCESS)
                        result = STEER_GetChildValue(paramObj, &((*parameters)->parameter[i]));

                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetConfigurationsFromReportObject
// =================================================================================================
int32_t STEER_GetConfigurationsFromReportObject (cJSON* reportObject,
                                                 tSTEER_Configurations** configurations)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(reportObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(configurations);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* configsArray = NULL;
        uint32_t configsArraySize = 0;

        // Get the configurations array
        result = STEER_GetChildArray(reportObject, STEER_JSON_TAG_CONFIGURATIONS, 
                                     &configsArray, &configsArraySize);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Allocate space
            result = STEER_ReallocateMemory(sizeof(tSTEER_Configurations),
                                          sizeof(tSTEER_Configurations) +
                                          (configsArraySize * sizeof(tSTEER_Configuration)),
                                          (void**)configurations);
            if (result == STEER_RESULT_SUCCESS)
            {
                char* configIdStr = NULL;
                cJSON* configObj = NULL;
                uint_fast32_t i = 0;

                // Set count
                (*configurations)->count = configsArraySize;

                for (i = 0; i < configsArraySize; i++)
                {
                    configObj = cJSON_GetArrayItem(configsArray, i);
                    result = STEER_CHECK_POINTER(configObj);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Copy the configuration ID
                        result = STEER_GetChildObjectString(configObj, 
                                                            STEER_JSON_TAG_CONFIGURATION_ID,
                                                            (char**)&configIdStr);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            result = STEER_ConvertStringToUnsigned64BitInteger(configIdStr,
                                                                               &((*configurations)->configuration[i].configurationId));
                            if (result == STEER_RESULT_SUCCESS)
                                (*configurations)->configuration[i].configurationId -= 1;
                        }
                        STEER_FreeMemory((void**)&configIdStr);

                        if (result == STEER_RESULT_SUCCESS)
                        {
                            cJSON* attributesArray = NULL;
                            uint32_t attributesArraySize = 0;
                            
                            // Get the attributes (optional)
                            result = STEER_GetChildArray(configObj, STEER_JSON_TAG_ATTRIBUTES,
                                                         &attributesArray, &attributesArraySize);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                result = STEER_GetValuesFromArray(attributesArray,
                                                                  &((*configurations)->configuration[i].attributes),
                                                                  NULL);
                            }
                            else if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                                result = STEER_RESULT_SUCCESS;  // Eat this result
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Get the tests (optional)
                            result = STEER_GetTestsFromConfigurationObject(configObj,
                                                                           &((*configurations)->configuration[i].tests));
                            if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                                result = STEER_RESULT_SUCCESS;  // Eat this result
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            cJSON* metricsArray = NULL;
                            uint32_t metricsArraySize = 0;

                            // Get the metrics (optional)
                            result = STEER_GetChildArray(configObj, STEER_JSON_TAG_METRICS,
                                                         &metricsArray, &metricsArraySize);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                result = STEER_GetValuesFromArray(metricsArray,
                                                                  &((*configurations)->configuration[i].metrics),
                                                                  &((*configurations)->configuration[i].metricSets));
                            }
                            else if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                                result = STEER_RESULT_SUCCESS;  // Eat this result
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Get the criteria
                            result = STEER_GetCriteriaFromParentObject(configObj,
                                                                       &((*configurations)->configuration[i].criteria));
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Get the evaluation
                            result = STEER_GetEvaluationFromParentObject(configObj,
                                                                         &((*configurations)->configuration[i].evaluation));
                        }
                    }

                    // Check status
                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetTestsFromConfigurationObject
// =================================================================================================
int32_t STEER_GetTestsFromConfigurationObject (cJSON* configurationObject,
                                               tSTEER_Tests** tests)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(configurationObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(tests);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* testsArray = NULL;
        uint32_t testsArraySize = 0;

        // Get the tests array
        result = STEER_GetChildArray(configurationObject, STEER_JSON_TAG_TESTS,
                                     &testsArray, &testsArraySize);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Allocate space
            result = STEER_AllocateMemory(sizeof(tSTEER_Tests) +
                                          (testsArraySize * sizeof(tSTEER_Test)),
                                          (void**)tests);
            if (result == STEER_RESULT_SUCCESS)
            {
                char* testIdStr = NULL;
                cJSON* testObj = NULL;
                uint_fast32_t i = 0;

                // Set count
                (*tests)->count = testsArraySize;

                for (i = 0; i < testsArraySize; i++)
                {
                    testObj = cJSON_GetArrayItem(testsArray, i);
                    result = STEER_CHECK_POINTER(testObj);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Copy the test ID
                        result = STEER_GetChildObjectString(testObj, STEER_JSON_TAG_TEST_ID, 
                                                            &testIdStr);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            result = STEER_ConvertStringToUnsigned64BitInteger(testIdStr,
                                                                               &((*tests)->test[i].testId));
                            if (result == STEER_RESULT_SUCCESS)
                                (*tests)->test[i].testId -= 1;
                        }
                        STEER_FreeMemory((void**)&testIdStr);

                        if (result == STEER_RESULT_SUCCESS)
                        {
                            cJSON* calculationsArray = NULL;
                            uint32_t calculationsArraySize = 0;

                            // Get the calculations (optional)
                            result = STEER_GetChildArray(testObj, STEER_JSON_TAG_CALCULATIONS,
                                                         &calculationsArray, &calculationsArraySize);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                result = STEER_GetValuesFromArray(calculationsArray,
                                                                  &((*tests)->test[i].calculations),
                                                                  &((*tests)->test[i].calculationSets));
                            }
                            else if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
                                result = STEER_RESULT_SUCCESS;  // Eat this result
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Get the criteria
                            result = STEER_GetCriteriaFromParentObject(testObj, 
                                                                       &((*tests)->test[i].criteria));
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Get the evaluation
                            result = STEER_GetEvaluationFromParentObject(testObj, 
                                                                         &((*tests)->test[i].evaluation));
                        }
                    }

                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetCriteriaFromParentObject
// =================================================================================================
int32_t STEER_GetCriteriaFromParentObject (cJSON* parentObject,
                                           tSTEER_Criteria** criteria)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(criteria);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* criteriaArray = NULL;
        uint32_t criteriaArraySize = 0;

        // Get the criteria array
        result = STEER_GetChildArray(parentObject, STEER_JSON_TAG_CRITERIA,
                                     &criteriaArray, &criteriaArraySize);
        if (result == STEER_RESULT_SUCCESS)
        {
            if (*criteria == NULL)
                result = STEER_AllocateMemory(sizeof(tSTEER_Criteria) +
                                              (criteriaArraySize * sizeof(tSTEER_Criterion)),
                                              (void**)criteria);
            else
                result = STEER_ReallocateMemory(sizeof(tSTEER_Criteria),
                                                sizeof(tSTEER_Criteria) +
                                                (criteriaArraySize * sizeof(tSTEER_Criterion)),
                                                (void**)criteria);
            if (result == STEER_RESULT_SUCCESS)
            {
                cJSON* criterionObj = NULL;
                uint_fast32_t i = 0;

                // Set count
                (*criteria)->count = criteriaArraySize;
        
                for (i = 0; i < criteriaArraySize; i++)
                {
                    criterionObj = cJSON_GetArrayItem(criteriaArray, i);
                    result = STEER_CHECK_POINTER(criterionObj);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Copy the criterion
                        result = STEER_GetChildObjectString(criterionObj,
                                                            STEER_JSON_TAG_CRITERION,
                                                            (char**)&((*criteria)->criterion[i].basis));
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Copy the result
                            result = STEER_GetChildObjectBoolean(criterionObj,
                                                                 STEER_JSON_TAG_RESULT,
                                                                 &((*criteria)->criterion[i].result));
                        }
                    }

                    // Check status
                    if (result != STEER_RESULT_SUCCESS)
                        break;
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetEvaluationFromParentObject
// =================================================================================================
int32_t STEER_GetEvaluationFromParentObject (cJSON* parentObject,
                                             tSTEER_Evaluation* evaluation)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(evaluation);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        char* evalStr = NULL;

        result = STEER_GetChildObjectString(parentObject, STEER_JSON_TAG_EVALUATION, &evalStr);
        if (result == STEER_RESULT_SUCCESS)
        {
            if (strcmp(evalStr, STEER_JSON_VALUE_FAIL) == 0)
                *evaluation = eSTEER_Evaluation_Fail;
            else if (strcmp(evalStr, STEER_JSON_VALUE_INCONCLUSIVE) == 0)
                *evaluation = eSTEER_Evaluation_Inconclusive;
            else if (strcmp(evalStr, STEER_JSON_VALUE_PASS) == 0)
                *evaluation = eSTEER_Evaluation_Pass;
            else 
                result = STEER_CHECK_ERROR(STEER_RESULT_JSON_INVALID_CONSTRUCTION);

            STEER_FreeMemory((void**)&evalStr);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_NewReportObject
// =================================================================================================
int32_t STEER_NewReportObject (const char* name,
                               const char* suite,
                               const char* scheduleId,
                               const char* description,
                               const char* conductor,
                               const char* notes,
                               tSTEER_ReportLevel level,
                               const char* programName,
                               const char* programVersion,
                               const char* operatingSystem,
                               const char* architecture,
                               const char* entropySource,
                               const char* startTime,
                               cJSON** rootObject)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(rootObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(name);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(programName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(programVersion);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(operatingSystem);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(architecture);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(entropySource);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(startTime);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        *rootObject = NULL;

        // Create the top level object
        cJSON* rootObj = cJSON_CreateObject();
        result = STEER_CHECK_CONDITION((rootObj != NULL), EFAULT);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Create the report object
            cJSON* reportObj = cJSON_CreateObject();
            result = STEER_CHECK_CONDITION((reportObj != NULL), EFAULT);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the test name
                result = STEER_AddChildObjectString(reportObj,
                                                    STEER_JSON_TAG_TEST_NAME,
                                                    name);

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test suite (optional)
                    if ((suite != NULL) && (strlen(suite) > 0))
                    {
                        result = STEER_AddChildObjectString(reportObj,
                                                            STEER_JSON_TAG_TEST_SUITE,
                                                            suite); // cppcheck-suppress uninitvar
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the schedule ID (optional)
                    if ((scheduleId != NULL) && (strlen(scheduleId) > 0))
                    {
                        result = STEER_AddChildObjectString(reportObj,
                                                            STEER_JSON_TAG_SCHEDULE_ID,
                                                            scheduleId); // cppcheck-suppress uninitvar
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test description (optional)
                    if ((description != NULL) && (strlen(description) > 0))
                    {
                        result = STEER_AddChildObjectString(reportObj,
                                                            STEER_JSON_TAG_TEST_DESCRIPTION,
                                                            description); // cppcheck-suppress uninitvar
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test conductor (optional)
                    if ((conductor != NULL) && (strlen(conductor) > 0))
                    {
                        result = STEER_AddChildObjectString(reportObj,
                                                            STEER_JSON_TAG_TEST_CONDUCTOR,
                                                            conductor); // cppcheck-suppress uninitvar
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test notes (optional)
                    if ((notes != NULL) && (strlen(notes) > 0))
                    {
                        result = STEER_AddChildObjectString(reportObj,
                                                            STEER_JSON_TAG_TEST_NOTES,
                                                            notes); // cppcheck-suppress uninitvar
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the report level
                    switch (level)
                    {
                        case eSTEER_ReportLevel_Summary:
                            result = STEER_AddChildObjectString(reportObj,
                                                                STEER_JSON_TAG_REPORT_LEVEL,
                                                                STEER_JSON_VALUE_SUMMARY);
                            break;
                        case eSTEER_ReportLevel_Standard:
                            result = STEER_AddChildObjectString(reportObj,
                                                                STEER_JSON_TAG_REPORT_LEVEL,
                                                                STEER_JSON_VALUE_STANDARD);
                            break;
                        case eSTEER_ReportLevel_Full:
                            result = STEER_AddChildObjectString(reportObj,
                                                                STEER_JSON_TAG_REPORT_LEVEL,
                                                                STEER_JSON_VALUE_FULL);
                            break;
                        default:
                            break;
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test program name
                    result = STEER_AddChildObjectString(reportObj,
                                                        STEER_JSON_TAG_PROGRAM_NAME,
                                                        programName);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test program version
                    result = STEER_AddChildObjectString(reportObj,
                                                        STEER_JSON_TAG_PROGRAM_VERSION,
                                                        programVersion);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test operating system
                    result = STEER_AddChildObjectString(reportObj,
                                                        STEER_JSON_TAG_OPERATING_SYSTEM,
                                                        operatingSystem);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test architecture
                    result = STEER_AddChildObjectString(reportObj,
                                                        STEER_JSON_TAG_ARCHITECTURE,
                                                        architecture);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the test source
                    result = STEER_AddChildObjectString(reportObj,
                                                        STEER_JSON_TAG_ENTROPY_SOURCE,
                                                        entropySource);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the timestamp
                    result = STEER_AddChildObjectString(reportObj,
                                                        STEER_JSON_TAG_START_TIME,
                                                        startTime);
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    if (level == eSTEER_ReportLevel_Full)
                    {
                        // Add an empty parameters array
                        result = STEER_AddEmptyNamedChildArray(reportObj, STEER_JSON_TAG_PARAMETERS, NULL);
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    if (level == eSTEER_ReportLevel_Full)
                    {
                        // Add an empty configurations array
                        result = STEER_AddEmptyNamedChildArray(reportObj, STEER_JSON_TAG_CONFIGURATIONS, NULL);
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add test report object to root
                    cJSON_bool jsonResult = cJSON_AddItemToObject(rootObj, 
                                                                  STEER_JSON_TAG_REPORT, 
                                                                  reportObj);
                    result = STEER_CHECK_CONDITION((jsonResult == 1), 
                                                   STEER_RESULT_JSON_OPERATION_FAILURE);
                    if (result == STEER_RESULT_SUCCESS)
                        *rootObject = rootObj;
                    else
                        (void)cJSON_Delete(reportObj);
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                (void)cJSON_Delete(rootObj);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddParametersToReportObject
// =================================================================================================
int32_t STEER_AddParametersToReportObject (cJSON* reportObject,
                                           tSTEER_ParameterSet* parameters)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_CONDITION((reportObject != NULL), EFAULT);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((parameters != NULL), EFAULT);
    
    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* paramsArray = NULL;
        uint32_t paramsArraySize = 0;

        // Get the parameters array
        result = STEER_GetChildArray(reportObject, STEER_JSON_TAG_PARAMETERS, 
                                     &paramsArray, &paramsArraySize);
        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
        {
            // Create the parameters array
            result = STEER_AddEmptyNamedChildArray(reportObject,
                                                   STEER_JSON_TAG_PARAMETERS,
                                                   &paramsArray);
        }

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            uint_fast32_t i = 0;

            for (i = 0; i < parameters->count; i++)
            {
                // Add parameter to report
                result = STEER_AddChildValue(paramsArray, &(parameters->parameter[i]));
                if (result != STEER_RESULT_SUCCESS)
                    break;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddConfigurationsToReportObject
// =================================================================================================
int32_t STEER_AddConfigurationsToReportObject (cJSON* reportObject,
                                               tSTEER_ReportLevel level,
                                               tSTEER_Configurations* configurations)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(reportObject);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(configurations);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* configsArray = NULL;

        // Get the configurations array
        result = STEER_GetChildArray(reportObject, STEER_JSON_TAG_CONFIGURATIONS, 
                                     &configsArray, NULL);
        if (result == STEER_RESULT_JSON_TAG_NOT_FOUND)
        {
            result = STEER_AddEmptyNamedChildArray(reportObject,
                                                   STEER_JSON_TAG_CONFIGURATIONS,
                                                   &configsArray);
        }

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* configObj = NULL;
            cJSON* attributesArray = NULL;
            cJSON* testsArray = NULL;
            cJSON* metricsArray = NULL;
            cJSON* criteriaArray = NULL;
            cJSON_bool jsonResult = 0;
            char configIdStr[STEER_STRING_MAX_LENGTH] = { 0 };
            uint_fast32_t i = 0;

            // Walk the configurations
            for (i = 0; i < configurations->count; i++)
            {
                // Create a configuration object
                configObj = cJSON_CreateObject();
                result = STEER_CHECK_CONDITION((configObj != NULL), 
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Copy configuration ID
                    memset((void*)configIdStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(configIdStr, "%" PRIu64 "", 
                            configurations->configuration[i].configurationId + 1);
                    result = STEER_AddChildObjectString(configObj,
                                                        STEER_JSON_TAG_CONFIGURATION_ID,
                                                        configIdStr);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        if (configurations->configuration[i].attributes != NULL)
                        {
                            // Add empty attributes array
                            result = STEER_AddEmptyNamedChildArray(configObj,
                                                                   STEER_JSON_TAG_ATTRIBUTES,
                                                                   &attributesArray);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Add attributes
                                result = STEER_AddValuesToArray(attributesArray,
                                                                configurations->configuration[i].attributes);
                            }
                        }

                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Add empty tests array
                            result = STEER_AddEmptyNamedChildArray(configObj,
                                                                   STEER_JSON_TAG_TESTS,
                                                                   &testsArray);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Add tests
                                result = STEER_AddTestsToTestsArray(testsArray, level,
                                                                    configurations->configuration[i].tests);
                            }
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            if (level == eSTEER_ReportLevel_Full)
                            {
                                // Add an empty metrics array
                                result = STEER_AddEmptyNamedChildArray(configObj, 
                                                                       STEER_JSON_TAG_METRICS, 
                                                                       &metricsArray);
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    // Add metrics
                                    result = STEER_AddValuesToArray(metricsArray,
                                                                    configurations->configuration[i].metrics);
                                }
                                if (result == STEER_RESULT_SUCCESS)
                                {
                                    // Add metric sets
                                    result = STEER_AddValueSetsToArray(metricsArray,
                                                                       configurations->configuration[i].metricSets);
                                }
                            }
                        }

                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Add an empty criteria array
                            result = STEER_AddEmptyNamedChildArray(configObj, 
                                                                   STEER_JSON_TAG_CRITERIA, 
                                                                   &criteriaArray);
                            if (result == STEER_RESULT_SUCCESS)
                            {
                                // Add criteria
                                result = STEER_AddCriteriaToCriteriaArray(criteriaArray,
                                                                          configurations->configuration[i].criteria);
                            }
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Add evaluation
                            result = STEER_AddEvaluationToParentObject(configObj,
                                                                       configurations->configuration[i].evaluation);
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Add the configuration object to the array
                            jsonResult = cJSON_AddItemToArray(configsArray, configObj);
                            result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                        }
                    }
                }

                if (result != STEER_RESULT_SUCCESS)
                    break;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddTestsToTestsArray
// =================================================================================================
int32_t STEER_AddTestsToTestsArray (cJSON* testsArray,
                                    tSTEER_ReportLevel level,
                                    tSTEER_Tests* tests)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(testsArray);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(tests);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* testObj = NULL;
        cJSON* calculationsArray = NULL;
        cJSON* criteriaArray = NULL;
        cJSON_bool jsonResult = 0;
        uint_fast32_t i = 0;
        char testIdStr[STEER_STRING_MAX_LENGTH] = { 0 };

        // Walk the tests
        for (i = 0; i < tests->count; i++)
        {
            // Create a test object
            testObj = cJSON_CreateObject();
            result = STEER_CHECK_CONDITION((testObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy test ID
                memset((void*)testIdStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(testIdStr, "%" PRIu64 "", tests->test[i].testId + 1);
                result = STEER_AddChildObjectString(testObj, STEER_JSON_TAG_TEST_ID, testIdStr);

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                {
                    if (level == eSTEER_ReportLevel_Full)
                    {
                        // Add empty calculations array
                        result = STEER_AddEmptyNamedChildArray(testObj,
                                                               STEER_JSON_TAG_CALCULATIONS,
                                                               &calculationsArray);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Add calculations
                            result = STEER_AddValuesToArray(calculationsArray,
                                                            tests->test[i].calculations);
                        }
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Add calculation sets
                            result = STEER_AddValueSetsToArray(calculationsArray,
                                                               tests->test[i].calculationSets);
                        }
                    }
                }

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add empty criteria array
                    result = STEER_AddEmptyNamedChildArray(testObj,
                                                           STEER_JSON_TAG_CRITERIA,
                                                           &criteriaArray);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Add criteria
                        result = STEER_AddCriteriaToCriteriaArray(criteriaArray,
                                                                  tests->test[i].criteria);
                    }
                }

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add evaluation
                    result = STEER_AddEvaluationToParentObject(testObj, 
                                                               tests->test[i].evaluation);
                }

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add test object to array
                    jsonResult = cJSON_AddItemToArray(testsArray, testObj);
                    result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                }
            }

            // Check status
            if (result != STEER_RESULT_SUCCESS)
                break;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddCriteriaToCriteriaArray
// =================================================================================================
int32_t STEER_AddCriteriaToCriteriaArray (cJSON* criteriaArray,
                                          tSTEER_Criteria* criteria)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(criteriaArray);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(criteria);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* criterionObj = NULL;
        uint_fast32_t i = 0;
        cJSON_bool jsonResult = 0;

        // Walk the criteria
        for (i = 0; i < criteria->count; i++)
        {
            // Create a criterion object
            criterionObj = cJSON_CreateObject();
            result = STEER_CHECK_CONDITION((criterionObj != NULL), STEER_RESULT_JSON_OPERATION_FAILURE);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the criterion
                result = STEER_AddChildObjectString(criterionObj, 
                                                    STEER_JSON_TAG_CRITERION, 
                                                    criteria->criterion[i].basis);
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the result
                    result = STEER_AddChildObjectBoolean(criterionObj,
                                                         STEER_JSON_TAG_RESULT,
                                                         criteria->criterion[i].result);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        // Add the object
                        jsonResult = cJSON_AddItemToArray(criteriaArray, criterionObj);
                        result = STEER_CHECK_CONDITION((jsonResult == 1), STEER_RESULT_JSON_OPERATION_FAILURE);
                    }
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                break;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddEvaluationToParentObject
// =================================================================================================
int32_t STEER_AddEvaluationToParentObject (cJSON* parentObject,
                                           tSTEER_Evaluation evaluation)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parentObject);
    if (result == STEER_RESULT_SUCCESS)
    {
        // Add evaluation
        if (evaluation == eSTEER_Evaluation_Fail)
            result = STEER_AddChildObjectString(parentObject, 
                                                STEER_JSON_TAG_EVALUATION, 
                                                STEER_JSON_VALUE_FAIL);
        else if (evaluation == eSTEER_Evaluation_Inconclusive)
            result = STEER_AddChildObjectString(parentObject,
                                                STEER_JSON_TAG_EVALUATION,
                                                STEER_JSON_VALUE_INCONCLUSIVE);
        else if (evaluation == eSTEER_Evaluation_Pass)
            result = STEER_AddChildObjectString(parentObject,
                                                STEER_JSON_TAG_EVALUATION,
                                                STEER_JSON_VALUE_PASS);
        else
            result = STEER_CHECK_ERROR(STEER_RESULT_JSON_INVALID_CONSTRUCTION);
    }
    return result;
}

// =================================================================================================
//  STEER_ReportToJson
// =================================================================================================
int32_t STEER_ReportToJson (tSTEER_ReportPrivate* report,
                            tSTEER_ReportLevel reportLevel,
                            char** reportJson)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(reportJson);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;
        cJSON* reportObj = NULL;

        // Setup
        *reportJson = NULL;

        // Create a new report object
        result = STEER_NewReportObject(report->testName,
                                       report->testSuite,
                                       report->scheduleId,
                                       report->testDescription,
                                       report->testConductor,
                                       report->testNotes,
                                       report->level,
                                       report->programName,
                                       report->programVersion,
                                       report->operatingSystem,
                                       report->architecture,
                                       report->entropySource,
                                       report->startTime,
                                       &rootObj);

        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the report object
            result = STEER_GetChildObject(rootObj, STEER_JSON_TAG_REPORT, &reportObj);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add the completion timestamp
            if ((report->completionTime != NULL) && (strlen(report->completionTime) > 0))
                result = STEER_AddChildObjectString(reportObj, 
                                                    STEER_JSON_TAG_COMPLETION_TIME,
                                                    report->completionTime);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add the test duration
            if ((report->duration != NULL) && (strlen(report->duration) > 0))
                result = STEER_AddChildObjectString(reportObj,
                                                    STEER_JSON_TAG_TEST_DURATION,
                                                    report->duration);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            if ((reportLevel == eSTEER_ReportLevel_Standard) ||
                (reportLevel == eSTEER_ReportLevel_Full))
            {
                // Add the parameters
                result = STEER_AddParametersToReportObject(reportObj,
                                                           report->parameters);
            }
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            if ((reportLevel == eSTEER_ReportLevel_Standard) ||
                (reportLevel == eSTEER_ReportLevel_Full))
            {
                // Add the configurations
                result = STEER_AddConfigurationsToReportObject(reportObj, reportLevel,
                                                               report->configurations);
            }
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* criteriaArray = NULL;
            
            // Add the criteria
            result = STEER_AddEmptyNamedChildArray(reportObj,
                                                   STEER_JSON_TAG_CRITERIA,
                                                   &criteriaArray);
            if (result == STEER_RESULT_SUCCESS)
                result = STEER_AddCriteriaToCriteriaArray(criteriaArray, report->criteria);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add the evaluation
            result = STEER_AddEvaluationToParentObject(reportObj, report->evaluation);
        }

        // Check status
        if (result == STEER_RESULT_SUCCESS)
        {
            // Print the JSON
            *reportJson = cJSON_Print(rootObj);
        }
        
        // Clean up
        (void)cJSON_Delete(rootObj);
    }
    return result;
}

// =================================================================================================
//  STEER_JsonToReport
// =================================================================================================
int32_t STEER_JsonToReport (const char* reportJson,
                            tSTEER_ReportPtr* report)
{
    int32_t result = STEER_RESULT_SUCCESS;
    // Check arguments
    result = STEER_CHECK_STRING(reportJson);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(report);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        cJSON* rootObj = NULL;

        // Setup 
        *report = NULL;

        // Validate the JSON
        result = STEER_ValidateReport (reportJson, &rootObj);
        if (result == STEER_RESULT_SUCCESS)
        {
            cJSON* reportObj = NULL;

            // Get the report object
            result = STEER_GetChildObject(rootObj, STEER_JSON_TAG_REPORT, &reportObj);
            if (result == STEER_RESULT_SUCCESS)
            {
                tSTEER_ReportPrivate* theReport = NULL;

                // Get a new empty report
                result = STEER_NewEmptyReport(&theReport);

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get report header
                    result = STEER_GetReportHeaderFromReportObject(reportObj, theReport);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the parameters
                    result = STEER_GetParametersFromReportObject(reportObj,
                                                                 &(theReport->parameters));
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Get the configurations
                    result = STEER_GetConfigurationsFromReportObject(reportObj,
                                                                     &(theReport->configurations));
                }

                // Check status
                if (result == STEER_RESULT_SUCCESS)
                    *report = theReport;
                else
                    (void)STEER_FreeReport((tSTEER_ReportPtr*)&theReport);
            }

            // Clean up
            cJSON_Delete(rootObj);
            rootObj = NULL;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetConfigurationIndexWithId
// =================================================================================================
int32_t STEER_GetConfigurationIndexWithId (cJSON* configurationsArray,
                                           const char* id,
                                           uint32_t* index)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(configurationsArray);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(id);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(index);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint32_t arraySize = cJSON_GetArraySize(configurationsArray);

        // Setup
        *index = 0;

        if (arraySize > 0)
        {
            uint_fast32_t i = 0;
            cJSON* arrayItem = NULL;
            char* itemId = NULL;
            bool foundIt = false;

            for (i = 0; i < arraySize; i++)
            {
                arrayItem = cJSON_GetArrayItem(configurationsArray, i);
                result = STEER_CHECK_CONDITION((arrayItem != NULL),
                                               STEER_RESULT_JSON_OPERATION_FAILURE);
                if (result == STEER_RESULT_SUCCESS)
                {
                    result = STEER_GetChildObjectString(arrayItem, STEER_JSON_TAG_CONFIGURATION_ID, &itemId);
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        if (strcmp(id, itemId) == 0)
                        {
                            foundIt = true;
                            *index = i;
                        }

                        STEER_FreeMemory((void**)&itemId);
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
