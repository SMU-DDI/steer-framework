// =================================================================================================
//! @file steer_report_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private report utilities used by the STandard Entropy Evaluation 
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-09
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_REPORT_UTILITIES_PRIVATE_H__
#define __STEER_REPORT_UTILITIES_PRIVATE_H__

#include "steer_types_private.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int STEER_CompareDoubles (const double* a,
                              const double* b);

    int32_t STEER_FreeCriteria (tSTEER_Criteria** criteria);

    int32_t STEER_FreeTest (tSTEER_Test* test);

    int32_t STEER_FreeTests (tSTEER_Tests** tests);

    int32_t STEER_FreeConfigurations (tSTEER_Configurations** configurations);

    int32_t STEER_NewEmptyReport (tSTEER_ReportPrivate** report);

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
                             const char* timestamp,
                             uint64_t bitstreamCount,
                             tSTEER_TestInfo* testInfo,
                             tSTEER_ParameterSet* parameters,
                             uint32_t configurationCount,
                             tSTEER_ReportPrivate** report);

    int32_t STEER_AddConfigurationToReport (tSTEER_ReportPrivate* report,
                                            uint64_t configurationId);

    int32_t STEER_AddTestToConfiguration (tSTEER_ReportPrivate* report,
                                          uint64_t configurationId,
                                          uint64_t testId);

    int32_t STEER_AddMetricSetToConfiguration (tSTEER_ReportPrivate* report,
                                                uint64_t configurationId,
                                                tSTEER_ValueSet* metricSet);

    int32_t STEER_GetParameterFromReport (tSTEER_ReportPrivate* report,
                                          const char* parameterName,
                                          char** parameterValue);

    int32_t STEER_GetMetricFromConfiguration (tSTEER_ReportPrivate* report,
                                              uint64_t configurationId,
                                              const char* metricName,
                                              char** metricValue);

    int32_t STEER_GetMetricSetFromConfiguration (tSTEER_ReportPrivate* report,
                                                  uint64_t configurationId,
                                                  const char* metricName,
                                                  tSTEER_ValueSet** metricSet);

    int32_t STEER_ValidateReport (const char* reportJson,
                                  cJSON** rootObject);

    int32_t STEER_GetReportHeaderFromReportObject (cJSON* reportObject,
                                                   tSTEER_ReportPrivate* report);

    int32_t STEER_GetParametersFromReportObject (cJSON* reportObject,
                                                 tSTEER_ParameterSet** parameters);

    int32_t STEER_GetConfigurationsFromReportObject (cJSON* reportObject,
                                                     tSTEER_Configurations** configurations);

    int32_t STEER_GetTestsFromConfigurationObject (cJSON* configurationObject,
                                                   tSTEER_Tests** tests);

    int32_t STEER_GetCriteriaFromParentObject (cJSON* parentObject,
                                               tSTEER_Criteria** criteria);

    int32_t STEER_GetEvaluationFromParentObject (cJSON* parentObject,
                                                 tSTEER_Evaluation* evaluation);

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
                                   cJSON** rootObject);

    int32_t STEER_AddParametersToReportObject (cJSON* reportObject,
                                               tSTEER_ParameterSet* parameterSet);

    int32_t STEER_AddConfigurationsToReportObject (cJSON* reportObject,
                                                   tSTEER_ReportLevel level,
                                                   tSTEER_Configurations* configurations);

    int32_t STEER_AddTestsToTestsArray (cJSON* testsArray,
                                        tSTEER_ReportLevel level,
                                        tSTEER_Tests* tests);

    int32_t STEER_AddCriteriaToCriteriaArray (cJSON* criteriaArray,
                                              tSTEER_Criteria* criteria);

    int32_t STEER_AddEvaluationToParentObject (cJSON* parentObject,
                                               tSTEER_Evaluation evaluation);

    int32_t STEER_ReportToJson (tSTEER_ReportPrivate* report,
                                tSTEER_ReportLevel reportLevel,
                                char** reportJson);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_REPORT_UTILITIES_PRIVATE_H__
// =================================================================================================
