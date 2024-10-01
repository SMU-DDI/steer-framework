// =================================================================================================
//! @file steer_nist_sts_utilities_private.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines the private NISTS STS utilities used by the STandard Entropy Evaluation 
//! Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2022-06-23
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_NIST_STS_UTILITIES_PRIVATE_H__
#define __STEER_NIST_STS_UTILITIES_PRIVATE_H__

#include "steer_types_private.h"
#include "cJSON.h"

// =================================================================================================
//  Prototypes
// =================================================================================================

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t STEER_NistStsAddRequiredMetricsToConfiguration (tSTEER_ReportPtr report,
                                                            uint64_t configurationId,
                                                            uint64_t testCount,
                                                            uint64_t minimumTestCountRequiredForSignificance,
                                                            uint64_t passedTestCount,
                                                            uint64_t minimumTestCountRequiredToPass,
                                                            uint64_t totalOnes,
                                                            uint64_t totalZeros);

    int32_t STEER_NistStsAddMetricsToConfiguration (tSTEER_ReportPtr report,
                                                    uint64_t configurationId,
                                                    bool isRandomExcursionOrRandomExcursionVariantTest,
                                                    uint64_t bitstreamCount,
                                                    double significanceLevel,
                                                    double* probabilityValueUniformity,
                                                    uint64_t* proportionThresholdMinimum,
                                                    uint64_t* proportionThresholdMaximum);

    int32_t STEER_NistStsAddRequiredCriterionToConfiguration (tSTEER_ReportPtr report,
                                                              uint64_t configurationId,
                                                              uint64_t testCount,
                                                              uint64_t passTestCount,
                                                              double significanceLevel,
                                                              uint32_t significanceLevelPrecision,
                                                              uint64_t minimumTestCountRequiredForSignificance);

    int32_t STEER_NistStsAddCriteriaToConfiguration (tSTEER_ReportPtr report,
                                                     uint64_t configurationId,
                                                     double probabilityValueUniformity,
                                                     uint64_t proportionThresholdMinimum,
                                                     uint64_t proportionThresholdMaximum,
                                                     uint64_t testCount,
                                                     uint64_t passTestCount);

    int32_t STEER_NistStsGetProbabilityValueFromTest (tSTEER_ReportPtr report,
                                                      uint64_t configurationId,
                                                      uint64_t testId,
                                                      double* probabilityValue);

    int32_t STEER_NistStsGetProbabilityValuesFromConfiguration(tSTEER_ReportPtr report,
                                                               uint64_t configurationId,
                                                               uint64_t* probabilityValueCount,
                                                               double** probabilityValues);

#ifdef __cplusplus
}
#endif

// =================================================================================================
#endif  // __STEER_NIST_STS_UTILITIES_PRIVATE_H__
// =================================================================================================
