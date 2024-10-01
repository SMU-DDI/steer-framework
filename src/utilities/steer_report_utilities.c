// =================================================================================================
//! @file steer_report_utilities.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the report utilities for the STandard Entropy Evaluation Report
//! (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-03-31
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_report_utilities_private.h"
#include "steer_report_utilities.h"
#include "steer_parameter_set_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_utilities.h"
#include "steer_utilities_private.h"
#include "steer_value_utilities.h"
#include "steer_value_utilities_private.h"
#include "cephes.h"
#include <math.h>

// =================================================================================================
//  STEER_CompareDoubles
// =================================================================================================
int STEER_CompareDoubles (const double* a,
                          const double* b)
{
    if (*a < *b)
        return -1;
    if (*a == *b)
        return 0;
    return 1;
}

// =================================================================================================
//  STEER_FreeCriteria
// =================================================================================================
int32_t STEER_FreeCriteria (tSTEER_Criteria** criteria)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER (criteria);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*criteria != NULL)
        {
            uint_fast32_t i = 0;

            for (i = 0; i < (*criteria)->count; i++)
            {
                STEER_FreeMemory((void**)&((*criteria)->criterion[i].basis));
            }
        }

        STEER_FreeMemory((void**)criteria);
    }
    return result;
}

// =================================================================================================
//  STEER_FreeTest
// =================================================================================================
int32_t STEER_FreeTest (tSTEER_Test* test)
{
    (void)STEER_FreeValues(&(test->calculations));
    (void)STEER_FreeValueSets(&(test->calculationSets));
    (void)STEER_FreeCriteria(&(test->criteria));

    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  STEER_FreeTests
// =================================================================================================
int32_t STEER_FreeTests (tSTEER_Tests** tests)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER(tests);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*tests != NULL)
        {
            uint_fast32_t i = 0;

            for (i = 0; i < (*tests)->count; i++)
            {
                (void)STEER_FreeTest(&((*tests)->test[i]));
            }
        }

        STEER_FreeMemory((void**)tests);
    }
    return result;
}

// =================================================================================================
//  STEER_FreeConfigurations
// =================================================================================================
int32_t STEER_FreeConfigurations (tSTEER_Configurations** configurations)
{
    int32_t result = STEER_RESULT_SUCCESS;

    result = STEER_CHECK_POINTER(configurations);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*configurations != NULL)
        {
            uint_fast32_t i = 0;

            for (i = 0; i < (*configurations)->count; i++)
            {
                (void)STEER_FreeValues(&((*configurations)->configuration[i].attributes));
                (void)STEER_FreeTests(&((*configurations)->configuration[i].tests));
                (void)STEER_FreeValues(&((*configurations)->configuration[i].metrics));
                (void)STEER_FreeValueSets(&((*configurations)->configuration[i].metricSets));
                (void)STEER_FreeCriteria(&((*configurations)->configuration[i].criteria));
            }
        }
        STEER_FreeMemory((void**)configurations);
    }
    return result;
}

// =================================================================================================
//  STEER_NewEmptyReport
// =================================================================================================
int32_t STEER_NewEmptyReport (tSTEER_ReportPrivate** report)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check argument
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_ReportPrivate* theReport = NULL;

        // Setup
        *report = NULL;

        // Allocate empty report 
        result = STEER_AllocateMemory(sizeof(tSTEER_ReportPrivate), 
                                      (void**)&theReport);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Allocate empty parameters set
            result = STEER_AllocateMemory(sizeof(tSTEER_ParameterSet),
                                          (void**)&(theReport->parameters));
            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate empty configurations
                result = STEER_AllocateMemory(sizeof(tSTEER_Configurations),
                                              (void**)&(theReport->configurations));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate empty criteria
                result = STEER_AllocateMemory(sizeof(tSTEER_Criteria),
                                              (void**)&(theReport->criteria));
            }

            // Check status
            if (result == STEER_RESULT_SUCCESS)
                *report = theReport;
            else
                (void)STEER_FreeReport((tSTEER_ReportPtr*)&theReport);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_NewReport
// =================================================================================================
int32_t STEER_NewReport (const char* testName,
                         const char* testSuite,
                         const char* scheduleId,
                         const char* testDescription,
                         const char* testConductor,
                         const char* testNotes,
                         tSTEER_ReportLevel level,
                         const char* programName,
                         const char* programVersion,
                         const char* operatingSystem,
                         const char* architecture,
                         const char* entropySource,
                         const char* startTime,
                         uint64_t bitstreamCount,
                         tSTEER_TestInfo* testInfo,
                         tSTEER_ParameterSet* parameters,
                         uint32_t configurationCount,
                         tSTEER_ReportPrivate** report)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(parameters);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(report);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        uint_fast32_t j = 0;

        // Setup
        *report = NULL;

        // Allocate space
        result = STEER_AllocateMemory(sizeof(tSTEER_ReportPrivate), (void**)report);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy name
            result = STEER_DuplicateString(testName, (char**)&((*report)->testName));
            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy suite (optional)
                if (testSuite != NULL)
                    result = STEER_DuplicateString(testSuite, (char**)&((*report)->testSuite));
            }   

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy schedule ID (optional)
                if (scheduleId != NULL)
                    result = STEER_DuplicateString(scheduleId, (char**)&((*report)->scheduleId));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy description
                result = STEER_DuplicateString(testDescription, (char**)&((*report)->testDescription));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy conductor (optional)
                if (testConductor != NULL)
                    result = STEER_DuplicateString(testConductor, (char**)&((*report)->testConductor));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy notes (optional)
                if (testNotes != NULL)
                    result = STEER_DuplicateString(testNotes, (char**)&((*report)->testNotes));
            }

            // Copy report level
            (*report)->level = level;

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy program name
                result = STEER_DuplicateString(programName, (char**)&((*report)->programName));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy program version
                result = STEER_DuplicateString(programVersion, (char**)&((*report)->programVersion));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy operating system
                result = STEER_DuplicateString(operatingSystem, (char**)&((*report)->operatingSystem));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy architecture
                result = STEER_DuplicateString(architecture, (char**)&((*report)->architecture));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy entropy source
                result = STEER_DuplicateString(entropySource, (char**)&((*report)->entropySource));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy timestamp
                result = STEER_DuplicateString(startTime, (char**)&((*report)->startTime));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate space to copy parameters
                result = STEER_AllocateMemory(sizeof(tSTEER_ParameterSet) + 
                                              parameters->count * sizeof(tSTEER_Value),
                                              (void**)&((*report)->parameters));
                if (result == STEER_RESULT_SUCCESS)
                {
                    result = STEER_DuplicateString(parameters->testName, 
                                                   (char**)&((*report)->parameters->testName));
                    if (result == STEER_RESULT_SUCCESS)
                    {
                        result = STEER_DuplicateString(parameters->parameterSetName,
                                                       (char**)&((*report)->parameters->parameterSetName));
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            // Copy parameters
                            for (i = 0; i < parameters->count; i++)
                            {
                                result = STEER_DuplicateValue(&(parameters->parameter[i]),
                                                              &((*report)->parameters->parameter[i]));
                                if (result == STEER_RESULT_SUCCESS)
                                    (*report)->parameters->count++;
                                else
                                    break;
                            }
                        }
                    }
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Create empty configurations list
                result = STEER_AllocateMemory(sizeof(tSTEER_Configurations),
                                              (void**)&((*report)->configurations));
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Create empty configurations with empty tests
                    for (i = 0; i < configurationCount; i++)
                    {
                        result = STEER_AddConfigurationToReport((tSTEER_ReportPtr)(*report), (uint64_t)i);
                        if (result == STEER_RESULT_SUCCESS)
                        {
                            for (j = 0; j < bitstreamCount; j++)
                            {
                                result = STEER_AddTestToConfiguration((tSTEER_ReportPtr)(*report), 
                                                                      (uint64_t)i, (uint64_t)j);
                            }
                        }
                        else
                            break;
                    }
                }
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Create empty criteria
                result = STEER_AllocateMemory(sizeof(tSTEER_Criteria),
                                              (void**)&((*report)->criteria));
            }

            // Check status
            if (result != STEER_RESULT_SUCCESS)
                (void)STEER_FreeReport((tSTEER_ReportPtr*)report);
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddConfigurationToReport
// =================================================================================================
int32_t STEER_AddConfigurationToReport (tSTEER_ReportPrivate* report,
                                        uint64_t configurationId)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId >= report->configurations->count), EINVAL);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Grow the configurations list by one
        result = STEER_ReallocateMemory(sizeof(tSTEER_Configurations) + (report->configurations->count * sizeof(tSTEER_Configuration)),
                                        sizeof(tSTEER_Configurations) + ((report->configurations->count + 1) * sizeof(tSTEER_Configuration)),
                                        (void**)&(report->configurations));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Setup
            report->configurations->configuration[configurationId].configurationId = configurationId;
            report->configurations->configuration[configurationId].attributes = NULL;

            // Allocate empty tests
            result = STEER_AllocateMemory(sizeof(tSTEER_Tests),
                                          (void**)&(report->configurations->configuration[configurationId].tests));

            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate empty metrics
                result = STEER_AllocateMemory(sizeof(tSTEER_Values),
                                              (void**)&(report->configurations->configuration[configurationId].metrics));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate empty metrics sets
                result = STEER_AllocateMemory(sizeof(tSTEER_ValueSets),
                                              (void**)&(report->configurations->configuration[configurationId].metricSets));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Allocate empty criteria
                result = STEER_AllocateMemory(sizeof(tSTEER_Criteria),
                                              (void**)&(report->configurations->configuration[configurationId].criteria));
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Bump the count
                report->configurations->count++;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddTestToConfiguration
// =================================================================================================
int32_t STEER_AddTestToConfiguration (tSTEER_ReportPrivate* report,
                                      uint64_t configurationId,
                                      uint64_t testId)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < report->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((testId >= report->configurations->configuration[configurationId].tests->count), EINVAL);;

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the configuration
        tSTEER_Configuration* configPtr = &(report->configurations->configuration[configurationId]);

        // Grow the tests list by one
        if (configPtr->tests == NULL)
            result = STEER_AllocateMemory(sizeof(tSTEER_Tests) + sizeof(tSTEER_Test),
                                          (void**)&(configPtr->tests));
        else
            result = STEER_ReallocateMemory(sizeof(tSTEER_Tests) + (configPtr->tests->count * sizeof(tSTEER_Test)),
                                            sizeof(tSTEER_Tests) + ((configPtr->tests->count + 1) * sizeof(tSTEER_Test)),
                                            (void**)&(configPtr->tests));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Bump the count
            configPtr->tests->count++;

            // Set test ID
            configPtr->tests->test[testId].testId = testId;

            // Create empty calculations list
            result = STEER_AllocateMemory(sizeof(tSTEER_Values),
                                          (void**)&(configPtr->tests->test[testId].calculations));

            if (result == STEER_RESULT_SUCCESS)
            {
                // Create empty calculation sets list
                result = STEER_AllocateMemory(sizeof(tSTEER_ValueSets),
                                              (void**)&(configPtr->tests->test[testId].calculationSets));
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Create empty criteria list
                result = STEER_AllocateMemory(sizeof(tSTEER_Criteria),
                                              (void**)&(configPtr->tests->test[testId].criteria));
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddMetricSetToConfiguration
// =================================================================================================
int32_t STEER_AddMetricSetToConfiguration (tSTEER_ReportPrivate* report,
                                           uint64_t configurationId,
                                           tSTEER_ValueSet* metricSet)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < report->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(metricSet);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the configuration
        tSTEER_Configuration* configPtr = &(report->configurations->configuration[configurationId]);

        // Grow the metrics set list by one
        result = STEER_CHECK_POINTER(configPtr->metricSets);

        // Initialize metric sets
        if (result == EFAULT)
        {
            result = STEER_AllocateMemory(sizeof(tSTEER_ValueSets*), (void **)&(configPtr->metricSets));
            result = STEER_CHECK_POINTER(&(configPtr->metrics->count));
        }

        result = STEER_ReallocateMemory(sizeof(tSTEER_ValueSets) + (configPtr->metricSets->count * sizeof(tSTEER_ValueSet*)),
                                        sizeof(tSTEER_ValueSets) + ((configPtr->metricSets->count + 1) * sizeof(tSTEER_ValueSet*)),
                                        (void**)&(configPtr->metricSets));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the metric set
            result = STEER_DuplicateValueSet(metricSet, 
                                             &(configPtr->metricSets->valueSet[configPtr->metricSets->count]));
            if (result == STEER_RESULT_SUCCESS)
            {
                // Bump the count
                configPtr->metricSets->count++;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetParameterFromReport
// =================================================================================================
int32_t STEER_GetParameterFromReport (tSTEER_ReportPrivate* report,
                                      const char* parameterName,
                                      char** parameterValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(parameterName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(parameterValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;

        for (i = 0; i < report->parameters->count; i++)
        {
            if (strcmp(report->parameters->parameter[i].name, parameterName) == 0)
            {
                result = STEER_DuplicateString(report->parameters->parameter[i].value,
                                               parameterValue);
                break;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetMetricFromConfiguration
// =================================================================================================
int32_t STEER_GetMetricFromConfiguration (tSTEER_ReportPrivate* report,
                                          uint64_t configurationId,
                                          const char* metricName,
                                          char** metricValue)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < report->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(metricName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(metricValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;

        for (i = 0; i < report->configurations->configuration[configurationId].metrics->count; i++)
        {
            if (strcmp(report->configurations->configuration[configurationId].metrics->value[i].name, metricName) == 0)
            {
                result = STEER_DuplicateString(report->configurations->configuration[configurationId].metrics->value[i].value,
                                               metricValue);
                break;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetMetricSetFromConfiguration
// =================================================================================================
int32_t STEER_GetMetricSetFromConfiguration (tSTEER_ReportPrivate* report,
                                              uint64_t configurationId,
                                              const char* metricName,
                                              tSTEER_ValueSet** metricSet)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < report->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(metricName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(metricSet);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;

        for (i = 0; i < report->configurations->configuration[configurationId].metricSets->count; i++)
        {
            if (strcmp(report->configurations->configuration[configurationId].metricSets->valueSet[i]->name, metricName) == 0)
            {
                result = STEER_DuplicateValueSet(report->configurations->configuration[configurationId].metricSets->valueSet[i],
                                                 metricSet);
                break;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_FreeReport
// =================================================================================================
int32_t STEER_FreeReport (tSTEER_ReportPtr* report)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate** reportPrivate = (tSTEER_ReportPrivate**)report;

    // Check argument
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (*reportPrivate != NULL)
        {
            STEER_FreeMemory((void**)&((*reportPrivate)->testName));
            STEER_FreeMemory((void**)&((*reportPrivate)->testSuite));
            STEER_FreeMemory((void**)&((*reportPrivate)->scheduleId));
            STEER_FreeMemory((void**)&((*reportPrivate)->testDescription));
            STEER_FreeMemory((void**)&((*reportPrivate)->testConductor));
            STEER_FreeMemory((void**)&((*reportPrivate)->testNotes)); 
            STEER_FreeMemory((void**)&((*reportPrivate)->programName));
            STEER_FreeMemory((void**)&((*reportPrivate)->programVersion));
            STEER_FreeMemory((void**)&((*reportPrivate)->operatingSystem));
            STEER_FreeMemory((void**)&((*reportPrivate)->architecture));
            STEER_FreeMemory((void**)&((*reportPrivate)->entropySource));
            STEER_FreeMemory((void**)&((*reportPrivate)->startTime));
            STEER_FreeMemory((void**)&((*reportPrivate)->completionTime));
            STEER_FreeMemory((void**)&((*reportPrivate)->duration));

            (void)STEER_FreeParameterSet(&((*reportPrivate)->parameters));
            (void)STEER_FreeConfigurations(&((*reportPrivate)->configurations));
            (void)STEER_FreeCriteria(&((*reportPrivate)->criteria));
        }

        STEER_FreeMemory((void**)report);
    }
    return result;
}

#if 0
// =================================================================================================
//  STEER_GetConfigurationIndexFromReport
// =================================================================================================
int32_t STEER_GetConfigurationIndexFromReport (tSTEER_ReportPtr report,
                                               const char* configurationId,
                                               uint32_t* configurationIndex)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(configurationId);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(configurationIndex);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        bool foundIt = false;
        
        // Setup
        *configurationIndex = 0;

        for (i = 0; i < reportPrivate->configurations->count; i++)
        {
            if (strcmp(configurationId,
                       reportPrivate->configurations->configuration[i].configurationId) == 0)
            {
                *configurationIndex = i;
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
//  STEER_GetConfigurationIdFromReport
// =================================================================================================
int32_t STEER_GetConfigurationIdFromReport (tSTEER_ReportPtr report,
                                            uint32_t configurationIndex,
                                            char** configurationId)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationIndex < reportPrivate->configurations->count), 
                                       EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(configurationId);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        result = STEER_DuplicateString(reportPrivate->configurations->configuration[configurationIndex].configurationId,
                                       configurationId);
    }
    return result;
}

// =================================================================================================
//  STEER_GetTestIndexFromReport
// =================================================================================================
int32_t STEER_GetTestIndexFromReport (tSTEER_ReportPtr report,
                                      uint32_t configurationIndex,
                                      const char* testId,
                                      uint32_t* testIndex)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationIndex < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(testId);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testIndex);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        bool foundIt = false;
        uint32_t count = reportPrivate->configurations->configuration[configurationIndex].tests->count;
        
        // Setup
        *testIndex = 0;

        for (i = 0; i < count; i++)
        {
            if (strcmp(testId,
                       reportPrivate->configurations->configuration[configurationIndex].tests->test[i].testId) == 0)
            {
                *testIndex = i;
                foundIt = true;
                break;
            }           
        }

        if (!foundIt)
            result = STEER_CHECK_ERROR(EINVAL);
    }
    return result;
}
#endif

// =================================================================================================
//  STEER_AddAttributeToConfiguration
// =================================================================================================
int32_t STEER_AddAttributeToConfiguration (tSTEER_ReportPtr report,
                                           uint64_t configurationId,
                                           const char* attributeName,
                                           const char* attributeDataType,
                                           const char* attributePrecision,
                                           const char* attributeUnits,
                                           const char* attributeValue)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(attributeName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(attributeDataType);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (attributePrecision != NULL)
            result = STEER_CHECK_STRING(attributePrecision);
    }
    if (result == STEER_RESULT_SUCCESS)
    {
        if (attributeUnits != NULL)
            result = STEER_CHECK_STRING(attributeUnits);
    }
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(attributeValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);

        // Grow the attributes list by one
        if (configPtr->attributes == NULL)
            result = STEER_AllocateMemory(sizeof(tSTEER_Values) + sizeof(tSTEER_Value),
                                          (void**)&(configPtr->attributes));
        else
            result = STEER_ReallocateMemory(sizeof(tSTEER_Values) + (configPtr->attributes->count * sizeof(tSTEER_Value)),
                                            sizeof(tSTEER_Values) + ((configPtr->attributes->count + 1) * sizeof(tSTEER_Value)),
                                            (void**)&(configPtr->attributes));

        if (result == STEER_RESULT_SUCCESS)
        {
            result = STEER_NewValue(attributeName, attributeDataType, attributePrecision,
                                    attributeUnits, attributeValue, 
                                    &(configPtr->attributes->value[(configPtr->attributes->count)]));
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Bump count
            configPtr->attributes->count++;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_ConfigurationHasAttribute
// =================================================================================================
bool STEER_ConfigurationHasAttribute (tSTEER_ReportPtr report,
                                      uint64_t configurationId,
                                      const char* attributeName)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;
    bool hasAttribute = false;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(attributeName);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_Values* attributes = reportPrivate->configurations->configuration[configurationId].attributes;
        uint_fast32_t i = 0;

        if (attributes != NULL)
        {
            for (i = 0; i < attributes->count; i++)
            {
                if (strcmp(attributes->value[i].name, attributeName) == 0)
                {
                    hasAttribute = true;
                    break;
                }
            }
        }
    }
    return hasAttribute;
}

// =================================================================================================
//  STEER_AddMetricToConfiguration
// =================================================================================================
int32_t STEER_AddMetricToConfiguration (tSTEER_ReportPtr report,
                                        uint64_t configurationId,
                                        const char* metricName,
                                        const char* metricDataType,
                                        const char* metricPrecision,
                                        const char* metricUnits,
                                        const char* metricValue)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(metricName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(metricDataType);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (metricPrecision != NULL)
            result = STEER_CHECK_STRING(metricPrecision);
    }
    if (result == STEER_RESULT_SUCCESS)
    {
        if (metricUnits != NULL)
            result = STEER_CHECK_STRING(metricUnits);
    }
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(metricValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the configuration
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);

        // Check if configurations are intialized
        result = STEER_CHECK_POINTER(&(configPtr->metrics->count));

        // Initialize configurations
        if (result == EFAULT)
        {
            result = STEER_AllocateMemory(sizeof(tSTEER_Values), (void **)&(configPtr->metrics));
            result = STEER_CHECK_POINTER(&(configPtr->metrics->count));
        }

        if (result == STEER_RESULT_SUCCESS)
        {	
            // Grow the metrics list by one
            result = STEER_ReallocateMemory(sizeof(tSTEER_Values) + (configPtr->metrics->count * sizeof(tSTEER_Value)),
                    sizeof(tSTEER_Values) + ((configPtr->metrics->count + 1) * sizeof(tSTEER_Value)),
                    (void**)&(configPtr->metrics));
            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy the metric
                result = STEER_NewValue(metricName, metricDataType, metricPrecision,
                        metricUnits, metricValue, 
                        &(configPtr->metrics->value[configPtr->metrics->count]));
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Bump the count
                    configPtr->metrics->count++;
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddConfusionMatrixMetricsToConfiguration
// =================================================================================================
int32_t STEER_AddConfusionMatrixMetricsToConfiguration (tSTEER_ReportPtr report,
                                                        uint64_t configurationId,
                                                        uint64_t minimumTestCount,
                                                        uint64_t actualTestCount,
                                                        uint64_t actualPassedTestCount,
                                                        uint64_t actualFailedTestCount,
                                                        uint64_t predictedPassedTestCount,
                                                        uint64_t predictedFailedTestCount)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_ConfusionMatrix  matrix;
        tSTEER_ConfusionMatrixStatistics stats;
        char metricStr[STEER_STRING_MAX_LENGTH] = { 0 };
        tSTEER_ValueSet* valueSet;

        memset((void*)&stats, 0, sizeof(tSTEER_ConfusionMatrixStatistics));

        // Compute the confusion matrix
        result = STEER_GetConfusionMatrix(minimumTestCount,
                                          actualTestCount,
                                          actualPassedTestCount,
                                          actualFailedTestCount,
                                          predictedPassedTestCount,
                                          predictedFailedTestCount,
                                          &matrix);
        if (result == STEER_RESULT_SUCCESS)
        {
            // Get the statistics
            result = STEER_GetConfusionMatrixStatistics(actualTestCount,
                                                        &matrix, &stats);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add confusion matrix metric
            result = STEER_NewValueSet(STEER_JSON_TAG_CONFUSION_MATRIX,
                                       STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                       NULL, STEER_JSON_VALUE_TESTS,
                                       &valueSet);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add predicted pass count
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, "%" PRIu64 "", matrix.predictedPassCount);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_PREDICTED_PASS_COUNT,
                                                  metricStr, &valueSet);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add predicted fail count
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, "%" PRIu64 "", matrix.predictedFailCount);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_PREDICTED_FAIL_COUNT,
                                                  metricStr, &valueSet);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add actual pass count
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, "%" PRIu64 "", matrix.actualPassCount);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_ACTUAL_PASS_COUNT,
                                                  metricStr, &valueSet);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add actual fail count
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, "%" PRIu64 "", matrix.actualFailCount);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_ACTUAL_FAIL_COUNT,
                                                  metricStr, &valueSet);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add true positives
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, "%" PRIu64 "", matrix.truePositives);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_TRUE_POSITIVES,
                                                  metricStr, &valueSet);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add true negatives
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, "%" PRIu64 "", matrix.trueNegatives);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_TRUE_NEGATIVES,
                                                  metricStr, &valueSet);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add false positives
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, "%" PRIu64 "", matrix.falsePositives);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_FALSE_POSITIVES,
                                                  metricStr, &valueSet);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add false negatives
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, "%" PRIu64 "", matrix.falseNegatives);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_FALSE_NEGATIVES,
                                                  metricStr, &valueSet);
            }
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add the metrics set to the configuration
                result = STEER_AddMetricSetToConfiguration(report, configurationId,
                                                           valueSet);
            }
            
            // Clean up
            (void)STEER_FreeValueSet(&valueSet);
        }
        if (result == STEER_RESULT_SUCCESS)
        {
            // Add confusion matrix statistics metric
            result = STEER_NewValueSet(STEER_JSON_TAG_CONFUSION_MATRIX_STATISTICS,
                                       STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                       STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                       NULL, &valueSet);
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add accuracy
                memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.accuracy);
                result = STEER_AddValueToValueSet(STEER_JSON_TAG_ACCURACY,
                                                  metricStr, &valueSet);

                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add balanced accuracy
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.balancedAccuracy);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_BALANCED_ACCURACY,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add diagnostic odds ratio
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.diagnosticOddsRatio);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_DIAGNOSTIC_ODDS_RATIO,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add error rate
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.errorRate);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_ERROR_RATE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add F1 score
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.f1Score);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_F1_SCORE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add false discovery rate
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.falseDiscoveryRate);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_FALSE_DISCOVERY_RATE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add false negative rate
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.falseNegativeRate);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_FALSE_NEGATIVE_RATE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add false omission rate
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.falseOmissionRate);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_FALSE_OMISSION_RATE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add false positive rate
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.falsePositiveRate);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_FALSE_POSITIVE_RATE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add Fowlkes-Mallows index
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.fowlkesMallowsIndex);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_FOWLKES_MALLOWS_INDEX,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add informedness
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.informedness);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_INFORMEDNESS,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add markedness
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.markedness);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_MARKEDNESS,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add Matthew's correlation coefficient
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.matthewsCorrelationCoefficient);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_MATTHEWS_CORRELATION_COEFFICIENT,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add negative likelihood ratio
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.negativeLikelihoodRatio);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_NEGATIVE_LIKELIHOOD_RATIO,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add negative predictive value
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.negativePredictiveValue);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_NEGATIVE_PREDICTIVE_VALUE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add positive likelihood ratio
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.positiveLikelihoodRatio);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_POSITIVE_LIKELIHOOD_RATIO,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add positive predictive value
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.positivePredictiveValue);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_POSITIVE_PREDICTIVE_VALUE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add prevalence
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.prevalence);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_PREVALENCE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add prevalence threshold
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.prevalenceThreshold);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_PREVALENCE_THRESHOLD,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add threat score
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.threatScore);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_THREAT_SCORE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add true negative rate
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.trueNegativeRate);
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_TRUE_NEGATIVE_RATE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add true positive rate
                    memset((void*)metricStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(metricStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, stats.truePositiveRate); 
                    result = STEER_AddValueToValueSet(STEER_JSON_TAG_TRUE_POSITIVE_RATE,
                                                      metricStr, &valueSet);
                }
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Add the metrics set to the configuration
                    result = STEER_AddMetricSetToConfiguration(report, configurationId,
                                                               valueSet);
                }
                
                // Clean up
                (void)STEER_FreeValueSet(&valueSet);
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddCriterionToConfiguration
// =================================================================================================
int32_t STEER_AddCriterionToConfiguration (tSTEER_ReportPtr report,
                                           uint64_t configurationId,
                                           const char* criterion,
                                           bool theResult)
{
    int32_t result = STEER_RESULT_SUCCESS; 
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(criterion);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Get the configuration
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);

        // Grow the criteria list by one
        result = STEER_ReallocateMemory(sizeof(tSTEER_Criteria) + (configPtr->criteria->count * sizeof(tSTEER_Criterion)),
                                        sizeof(tSTEER_Criteria) + ((configPtr->criteria->count + 1) * sizeof(tSTEER_Criterion)),
                                        (void**)&(configPtr->criteria));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the criterion
            result = STEER_DuplicateString(criterion,
                                           (char**)&(configPtr->criteria->criterion[configPtr->criteria->count].basis));
            if (result == STEER_RESULT_SUCCESS)
                configPtr->criteria->criterion[configPtr->criteria->count].result = theResult;

            if (result == STEER_RESULT_SUCCESS)
            {
                // Bump the count
                configPtr->criteria->count++;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddEvaluationToConfiguration
// =================================================================================================
int32_t STEER_AddEvaluationToConfiguration (tSTEER_ReportPtr report,
                                            uint64_t configurationId)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);

        // Walk the criteria
        configPtr->evaluation = eSTEER_Evaluation_Pass;
        for (i = 0; i < configPtr->criteria->count; i++)
        {
            if (configPtr->criteria->criterion[i].result == false)
            {
                configPtr->evaluation = eSTEER_Evaluation_Fail;
                break;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddCalculationToTest
// =================================================================================================
int32_t STEER_AddCalculationToTest (tSTEER_ReportPtr report,
                                    uint64_t configurationId,
                                    uint64_t testId,
                                    const char* calculationName,
                                    const char* calculationDataType,
                                    const char* calculationPrecision,
                                    const char* calculationUnits,
                                    const char* calculationValue)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((testId < reportPrivate->configurations->configuration[configurationId].tests->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(calculationName);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(calculationDataType);
    if (result == STEER_RESULT_SUCCESS)
    {
        if (calculationPrecision != NULL)
            result = STEER_CHECK_STRING(calculationPrecision);
    }
    if (result == STEER_RESULT_SUCCESS)
    {
        if (calculationUnits != NULL)
            result = STEER_CHECK_STRING(calculationUnits);
    }
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(calculationValue);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);
        tSTEER_Test* testPtr = &(configPtr->tests->test[testId]);

        // Grow the calculations list by one
        result = STEER_ReallocateMemory(sizeof(tSTEER_Values) + (testPtr->calculations->count * sizeof(tSTEER_Value)),
                                        sizeof(tSTEER_Values) + ((testPtr->calculations->count + 1) * sizeof(tSTEER_Value)),
                                        (void**)&(testPtr->calculations));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the calculation
            result = STEER_NewValue(calculationName, calculationDataType, calculationPrecision,
                                    calculationUnits, calculationValue, 
                                    &(testPtr->calculations->value[testPtr->calculations->count]));
            if (result == STEER_RESULT_SUCCESS)
            {
                // Bump the count
                testPtr->calculations->count++;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddCalculationSetToTest
// =================================================================================================
int32_t STEER_AddCalculationSetToTest (tSTEER_ReportPtr report,
                                       uint64_t configurationId,
                                       uint64_t testId,
                                       tSTEER_ValueSet* calculationSet)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((testId < reportPrivate->configurations->configuration[configurationId].tests->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(calculationSet);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);
        tSTEER_Test* testPtr = &(configPtr->tests->test[testId]);

        // Check if configurations are intialized
        result = STEER_CHECK_POINTER(&(testPtr->calculationSets->count));

        // Initialize configurations
        if (result == EFAULT)
        {
            result = STEER_AllocateMemory(sizeof(tSTEER_ValueSets), (void **)&(testPtr->calculationSets));
            result = STEER_CHECK_POINTER(&(testPtr->calculationSets->count));
        }

        if (result == STEER_RESULT_SUCCESS)
        {	
            // Grow the calculation sets list by one
            result = STEER_ReallocateMemory(sizeof(tSTEER_ValueSets) + (testPtr->calculationSets->count * sizeof(tSTEER_ValueSet*)),
                    sizeof(tSTEER_ValueSets) + ((testPtr->calculationSets->count + 1) * sizeof(tSTEER_ValueSet*)),
                    (void**)&(testPtr->calculationSets));
            if (result == STEER_RESULT_SUCCESS)
            {
                // Copy the calculation
                result = STEER_DuplicateValueSet(calculationSet, 
                        &(testPtr->calculationSets->valueSet[testPtr->calculationSets->count]));
                if (result == STEER_RESULT_SUCCESS)
                {
                    // Bump the count
                    testPtr->calculationSets->count++;
                }
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddCriterionToTest
// =================================================================================================
int32_t STEER_AddCriterionToTest (tSTEER_ReportPtr report,
                                  uint64_t configurationId,
                                  uint64_t testId,
                                  const char* criterion,
                                  bool theResult)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((testId < reportPrivate->configurations->configuration[configurationId].tests->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(criterion);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);
        tSTEER_Test* testPtr = &(configPtr->tests->test[testId]);

        // Grow the criteria list by one
        result = STEER_ReallocateMemory(sizeof(tSTEER_Criteria) + (testPtr->criteria->count * sizeof(tSTEER_Criterion)),
                                        sizeof(tSTEER_Criteria) + ((testPtr->criteria->count + 1) * sizeof(tSTEER_Criterion)),
                                        (void**)&(testPtr->criteria));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the criterion
            result = STEER_DuplicateString(criterion,
                                           (char**)&(testPtr->criteria->criterion[testPtr->criteria->count].basis));
            if (result == STEER_RESULT_SUCCESS)
                testPtr->criteria->criterion[testPtr->criteria->count].result = theResult;

            if (result == STEER_RESULT_SUCCESS)
            {
                // Bump the count
                testPtr->criteria->count++;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddEvaluationToTest
// =================================================================================================
int32_t STEER_AddEvaluationToTest (tSTEER_ReportPtr report,
                                   uint64_t configurationId,
                                   uint64_t testId,
                                   bool* testPassed)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((configurationId < reportPrivate->configurations->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((testId < reportPrivate->configurations->configuration[configurationId].tests->count), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(testPassed);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        tSTEER_Configuration* configPtr = &(reportPrivate->configurations->configuration[configurationId]);
        tSTEER_Test* testPtr = &(configPtr->tests->test[testId]);
        uint_fast32_t i = 0;

        *testPassed = true;

        // Walk the criteria
        testPtr->evaluation = eSTEER_Evaluation_Pass;
        for (i = 0; i < testPtr->criteria->count; i++)
        {
            if (testPtr->criteria->criterion[i].result == false)
            {
                testPtr->evaluation = eSTEER_Evaluation_Fail;
                *testPassed = false;
                break;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_AddCriterionToReport
// =================================================================================================
int32_t STEER_AddCriterionToReport (tSTEER_ReportPtr report,
                                    const char* criterion,
                                    bool theResult)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tSTEER_ReportPrivate* reportPrivate = (tSTEER_ReportPrivate*)report;

    // Check arguments
    result = STEER_CHECK_POINTER(report);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_STRING(criterion);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Grow the criteria list by one
        result = STEER_ReallocateMemory(sizeof(tSTEER_Criteria) + (reportPrivate->criteria->count * sizeof(tSTEER_Criterion)),
                                        sizeof(tSTEER_Criteria) + ((reportPrivate->criteria->count + 1) * sizeof(tSTEER_Criterion)),
                                        (void**)&(reportPrivate->criteria));
        if (result == STEER_RESULT_SUCCESS)
        {
            // Copy the criterion
            result = STEER_DuplicateString(criterion,
                                           (char**)&(reportPrivate->criteria->criterion[reportPrivate->criteria->count].basis));
            if (result == STEER_RESULT_SUCCESS)
                reportPrivate->criteria->criterion[reportPrivate->criteria->count].result = theResult;

            if (result == STEER_RESULT_SUCCESS)
            {
                // Bump the count
                reportPrivate->criteria->count++;
            }
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetMinimumTestCount
// =================================================================================================
int32_t STEER_GetMinimumTestCount (double significanceLevel,
                                   uint64_t bitstreamCount,
                                   uint64_t* minimumTestCount,
                                   uint64_t* predictedPassCount,
                                   uint64_t* predictedFailCount)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_CONDITION((significanceLevel > 0.0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((significanceLevel < 1.0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((bitstreamCount > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(minimumTestCount);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(predictedPassCount);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(predictedFailCount);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        double temp = significanceLevel;
        double intPart = 0.0;
        uint64_t numMultiplesOf10 = 0;
        uint64_t i = 0;
        uint64_t minPassCount = 0;
        uint64_t minFailCount = 0;

        // Setup
        *minimumTestCount = 0;
        *predictedPassCount = 0;
        *predictedFailCount = 0;

        while (modf(temp, &intPart) != 0.0)
        {
            temp *= 10;
            numMultiplesOf10 += 1;
        };
        minFailCount = (uint64_t)temp;
        
        temp = 1.0 - significanceLevel;
        for (i = 0; i < numMultiplesOf10; i++)
        {
            temp *= 10;
        }
        minPassCount = (uint64_t)temp;

        *minimumTestCount = minPassCount + minFailCount;
        *predictedFailCount = (uint64_t)((double)bitstreamCount * significanceLevel);
        *predictedPassCount = bitstreamCount - *predictedFailCount;
    }
    return result;
}

// =================================================================================================
//  STEER_GetConfusionMatrix
// =================================================================================================
int32_t STEER_GetConfusionMatrix (uint64_t minimumTestCount,
                                  uint64_t actualTestCount,
                                  uint64_t actualPassCount,
                                  uint64_t actualFailCount,
                                  uint64_t predictedPassCount,
                                  uint64_t predictedFailCount,
                                  tSTEER_ConfusionMatrix* confusionMatrix)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_CONDITION((minimumTestCount > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_CONDITION((actualTestCount > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(confusionMatrix);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Setup
        memset((void*)confusionMatrix, 0, sizeof(tSTEER_ConfusionMatrix));

        confusionMatrix->predictedPassCount = predictedPassCount;
        confusionMatrix->predictedFailCount = predictedFailCount;
        confusionMatrix->actualPassCount = actualPassCount;
        confusionMatrix->actualFailCount = actualFailCount;

        if ((actualPassCount + actualFailCount) >= (predictedFailCount + predictedPassCount))
        {
            if (actualPassCount > predictedPassCount)
                confusionMatrix->truePositives = predictedPassCount;
            else
                confusionMatrix->truePositives = actualPassCount;

            if (actualFailCount > predictedFailCount)
                confusionMatrix->trueNegatives = predictedFailCount;
            else
                confusionMatrix->trueNegatives = actualFailCount;

            confusionMatrix->falsePositives = predictedPassCount - confusionMatrix->truePositives;
            confusionMatrix->falseNegatives = predictedFailCount - confusionMatrix->trueNegatives;
        }
    }
    return result;
}

// =================================================================================================
//  STEER_GetConfusionMatrixStatistics
// =================================================================================================
int32_t STEER_GetConfusionMatrixStatistics (uint64_t actualTestCount,
                                            tSTEER_ConfusionMatrix* confusionMatrix,
                                            tSTEER_ConfusionMatrixStatistics* statistics)
{
    int32_t result = STEER_RESULT_SUCCESS;

    // Check arguments
    result = STEER_CHECK_CONDITION((actualTestCount > 0), EINVAL);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(confusionMatrix);
    if (result == STEER_RESULT_SUCCESS)
        result = STEER_CHECK_POINTER(statistics);

    // Check status
    if (result == STEER_RESULT_SUCCESS)
    {
        // Reference: https://en.wikipedia.org/wiki/Confusion_matrix

        // Setup
        memset((void*)statistics, 0, sizeof(tSTEER_ConfusionMatrixStatistics));

        // Calculate true positive rate (aka recall, sensitivity, or hit rate)
        statistics->truePositiveRate = (double)confusionMatrix->truePositives / (double)confusionMatrix->actualPassCount;

        // Calculate true negative rate (aka specificity or selectivity)
        statistics->trueNegativeRate = (double)confusionMatrix->trueNegatives / (double)confusionMatrix->actualFailCount;

        // Calculate positive predictive value (aka precision)
        statistics->positivePredictiveValue = 
            (double)confusionMatrix->truePositives / 
            (double)(confusionMatrix->truePositives + confusionMatrix->falsePositives);

        // Calculate negative predictive value
        statistics->negativePredictiveValue = 
            (double)confusionMatrix->trueNegatives / 
            (double)(confusionMatrix->trueNegatives + confusionMatrix->falseNegatives);

        // Calculate false negative rate (or miss rate)
        statistics->falseNegativeRate = 1.0 - statistics->truePositiveRate;

        // Calculate false positive rate (or fall-out)
        statistics->falsePositiveRate = 1.0 - statistics->trueNegativeRate;

        // Calculate false discovery rate
        statistics->falseDiscoveryRate = 1.0 - statistics->positivePredictiveValue;

        // Calculate false omission rate 
        statistics->falseOmissionRate = 1.0 - statistics->negativePredictiveValue;

        // Calculate prevalence threshold
        statistics->prevalenceThreshold = 
            sqrt(statistics->falsePositiveRate) / 
            (sqrt(statistics->truePositiveRate) + sqrt(statistics->falsePositiveRate));

        // Calculate threat score (aka critical success index or Jaccard index)
        statistics->threatScore = 
            (double)confusionMatrix->truePositives / 
            (double)(confusionMatrix->truePositives + confusionMatrix->falseNegatives + confusionMatrix->falsePositives);

        // Calculate accuracy
        statistics->accuracy = 
            (double)(confusionMatrix->truePositives + confusionMatrix->trueNegatives) / 
            (double)(confusionMatrix->actualPassCount + confusionMatrix->actualFailCount);

        // Calculate balanced accuracy
        statistics->balancedAccuracy = (statistics->truePositiveRate + statistics->trueNegativeRate) / 2.0;

        // Calculate F1 score
        statistics->f1Score = 
            (2.0 * statistics->positivePredictiveValue * statistics->truePositiveRate) / 
            (statistics->positivePredictiveValue + statistics->truePositiveRate);

        // Calculate Matthew's correlation coefficient (MCC)
        statistics->matthewsCorrelationCoefficient = 
            (double)((confusionMatrix->truePositives * confusionMatrix->trueNegatives) - 
                     (confusionMatrix->falsePositives * confusionMatrix->falseNegatives)) /
            sqrt((double)((confusionMatrix->truePositives + confusionMatrix->falsePositives) * 
                          (confusionMatrix->truePositives + confusionMatrix->falseNegatives) * 
                          (confusionMatrix->trueNegatives + confusionMatrix->falsePositives) * 
                          (confusionMatrix->trueNegatives + confusionMatrix->falseNegatives)));

        // Calculate Fowlkes-Mallows index
        statistics->fowlkesMallowsIndex = sqrt(statistics->positivePredictiveValue * statistics->truePositiveRate);

        // Calculate informedness (aka bookmaker informedness)
        statistics->informedness = statistics->truePositiveRate + statistics->trueNegativeRate + 1.0;

        // Calculate markedness (aka delta P)
        statistics->markedness = statistics->positivePredictiveValue + statistics->negativePredictiveValue - 1.0;

        // Calculate error rate (aka misclassification rate)
        statistics->errorRate = 1.0 - statistics->accuracy;

        // Calculate prevalence
        statistics->prevalence = 
            (double)confusionMatrix->actualPassCount / 
            (double)(confusionMatrix->actualPassCount + confusionMatrix->actualFailCount);

        // Calculate positive likelihood ratio
        statistics->positiveLikelihoodRatio = statistics->truePositiveRate / statistics->falsePositiveRate;

        // Calculate negative likelihood ratio
        statistics->negativeLikelihoodRatio = statistics->falseNegativeRate / statistics->trueNegativeRate;

        // Calculate diagnostic odds ratio
        statistics->diagnosticOddsRatio = statistics->positiveLikelihoodRatio / statistics->negativeLikelihoodRatio;
    }
    return result;
}

// =================================================================================================
