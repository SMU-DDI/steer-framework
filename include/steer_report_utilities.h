// =================================================================================================
//! @file steer_report_utilities.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the public report utilities used by the STandard Entropy Evaluation 
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-03-31
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_REPORT_UTILITIES_H__
#define __STEER_REPORT_UTILITIES_H__

#include "steer.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_FreeReport (tSTEER_ReportPtr* report);

    int32_t STEER_AddAttributeToConfiguration (tSTEER_ReportPtr report,
                                               uint64_t configurationId,
                                               const char* attributeName,
                                               const char* attributeDataType,
                                               const char* attributePrecision,
                                               const char* attributeUnits,
                                               const char* attributeValue);

    bool STEER_ConfigurationHasAttribute (tSTEER_ReportPtr report,
                                          uint64_t configurationId,
                                          const char* attributeName);

    int32_t STEER_AddMetricToConfiguration (tSTEER_ReportPtr report,
                                            uint64_t configurationId,
                                            const char* metricName,
                                            const char* metricDataType,
                                            const char* metricPrecision,
                                            const char* metricUnits,
                                            const char* metricValue);


    int32_t STEER_AddConfusionMatrixMetricsToConfiguration (tSTEER_ReportPtr report,
                                                            uint64_t configurationId,
                                                            uint64_t minimumTestCount,
                                                            uint64_t actualTestCount,
                                                            uint64_t actualPassedTestCount,
                                                            uint64_t actualFailedTestCount,
                                                            uint64_t predictedPassedTestCount,
                                                            uint64_t predictedFailedTestCount);

    int32_t STEER_AddCriterionToConfiguration (tSTEER_ReportPtr report,
                                               uint64_t configurationId,
                                               const char* criterion,
                                               bool theResult);

    int32_t STEER_AddEvaluationToConfiguration (tSTEER_ReportPtr report,
                                                uint64_t configurationId);

    int32_t STEER_AddCalculationToTest (tSTEER_ReportPtr report,
                                        uint64_t configurationId,
                                        uint64_t testId,
                                        const char* calculationName,
                                        const char* calculationDataType,
                                        const char* calculationPrecision,
                                        const char* calculationUnits,
                                        const char* calculationValue);

    int32_t STEER_AddCalculationSetToTest (tSTEER_ReportPtr report,
                                           uint64_t configurationId,
                                           uint64_t testId,
                                           tSTEER_ValueSet* calculationSet);

    int32_t STEER_AddCriterionToTest (tSTEER_ReportPtr report,
                                      uint64_t configurationId,
                                      uint64_t testId,
                                      const char* criterion,
                                      bool theResult);

    int32_t STEER_AddEvaluationToTest (tSTEER_ReportPtr report,
                                       uint64_t configurationId,
                                       uint64_t testId,
                                       bool* testPassed);

    int32_t STEER_AddCriterionToReport (tSTEER_ReportPtr report,
                                        const char* criterion,
                                        bool theResult);
                                        
    //! @fn int32_t STEER_GetMinimumTestCount (double alpha, uint64_t* minimumTestCount, 
    //! uint64_t* predictedPassCount, uint64_t* predictedFailCount)
    //! @brief Call this function to determine the minimum number of tests required to pass given an alpha
    //! value (significance level).
    //! @param[in] signficanceLevel The signficance level.
    //! @param[in] bitstreamCount The number of bitstreams to test.
    //! @param[out] minimumTestCount A pointer to the minimum number of tests that should be executed.
    //! @param[out] predictedPassCount A pointer to the number of tests predicted to pass.
    //! @param[out] predictedFailCount A pointer to the number of tests predicted to fail.
    //! @return A status code indicating whether the function call succeeded. 
    //! @note
    //! A return value of __STEER_RESULT_SUCCESS__ indicates success.\n
    //! A return value of __EINVAL__ indicates an invalid argument was provided by the caller.\n
    //! A return value of __EFAULT__ indicates a __NULL__ pointer was used as an argument by the caller.
    int32_t STEER_GetMinimumTestCount (double signficanceLevel,
                                       uint64_t bitstreamCount,
                                       uint64_t* minimumTestCount,
                                       uint64_t* predictedPassCount,
                                       uint64_t* predictedFailCount);

    int32_t STEER_GetConfusionMatrix (uint64_t minimumTestCount,
                                      uint64_t actualTestCount,
                                      uint64_t actualPassCount,
                                      uint64_t actualFailCount,
                                      uint64_t predictedPassCount,
                                      uint64_t predictedFailCount,
                                      tSTEER_ConfusionMatrix* confusionMatrix);

    int32_t STEER_GetConfusionMatrixStatistics (uint64_t actualTestCount,
                                                tSTEER_ConfusionMatrix* confusionMatrix,
                                                tSTEER_ConfusionMatrixStatistics* statistics);

    int32_t STEER_JsonToReport (const char* reportJson,
                                tSTEER_ReportPtr* report);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif	// __STEER_REPORT_UTILITIES_H__
// =================================================================================================
