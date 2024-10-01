// =================================================================================================
//! @file steer_json_glossary.h
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file defines a glossary for the common JSON tags and values used by the STandard 
//! Entropy Evaluation Report (STEER) framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-10-11
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#ifdef __cplusplus
    #pragma once
#endif

#ifndef __STEER_JSON_GLOSSARY_H__
#define __STEER_JSON_GLOSSARY_H__

#include "steer_json_constants.h"

// =================================================================================================
//  Macros
// =================================================================================================

#define STEER_DESCRIPTION_ACCURACY                          "ACC = (TP + TN) / (TP + TN + FP + FN). Also known as the Rand index or binary accuracy."
#define STEER_DESCRIPTION_ACTUAL_FAIL_COUNT                 "N = the number of tests that failed."
#define STEER_DESCRIPTION_ACTUAL_PASS_COUNT                 "P = the number of tests that passed."
#define STEER_DESCRIPTION_BALANCED_ACCURACY                 "BA = (TPR + TNR) / 2"
#define STEER_DESCRIPTION_DIAGNOSTIC_ODDS_RATIO             "DOR = LR+ / LR-"
#define STEER_DESCRIPTION_ERROR_RATE                        "Error rate = 1 - ACC"
#define STEER_DESCRIPTION_F1_SCORE                          "F1 = (2 x TP) / ((2 x TP) + FP + FN). A value of 1.0 indicates perfect precision and recall, and a value of 0 indicates either the precision or the recall is zero."
#define STEER_DESCRIPTION_FALSE_DISCOVERY_RATE              "FDR = FP / PP"
#define STEER_DESCRIPTION_FALSE_NEGATIVE_RATE               "FNR = FN / P. Also known as miss rate."
#define STEER_DESCRIPTION_FALSE_NEGATIVES                   "FN = the number of predicted failed tests that exceed the number of actual failed tests. Also known as type II error, miss, or underestimation."
#define STEER_DESCRIPTION_FALSE_OMISSION_RATE               "FOR = FN / PN"
#define STEER_DESCRIPTION_FALSE_POSITIVE_RATE               "FPR = FP / N. Also known as probability of false alarm or fall-out."
#define STEER_DESCRIPTION_FALSE_POSITIVES                   "FP = the number of predicted passed tests that exceed the number of actual passed tests. Also known as type I error, false alarm, or overestimation."
#define STEER_DESCRIPTION_FOWLKES_MALLOWS_INDEX             "FM = SQRT ((TP / (TP + FP)) x (TP / (TP + FN)))."
#define STEER_DESCRIPTION_INFORMEDNESS                      "BM = TPR + TNR - 1. Also known as bookmaker informedness."
#define STEER_DESCRIPTION_MARKEDNESS                        "MK = PPV + NPV - 1"
#define STEER_DESCRIPTION_MATTHEWS_CORRELATION_COEFFICIENT  "MCC = SQRT (TPR x TNR x PPV x NPV) - SQRT (FNR x FPR x FOR x FDR). A value of 1.0 represents perfect prediction; 0 represents no better than random prediction, and -1.0 indicates perfect disagreement between prediction and observation."
#define STEER_DESCRIPTION_NEGATIVE_LIKELIHOOD_RATIO         "LR- = FNR / TNR"
#define STEER_DESCRIPTION_NEGATIVE_PREDICTIVE_VALUE         "NPR = TN / PN"
#define STEER_DESCRIPTION_POSITIVE_LIKELIHOOD_RATIO         "LR+ = TPR / FPR."
#define STEER_DESCRIPTION_POSITIVE_PREDICTIVE_VALUE         "PPV = TP / PP. Also known as precision."
#define STEER_DESCRIPTION_PREDICTED_FAIL_COUNT              "PN = the number of tests predicted to fail."
#define STEER_DESCRIPTION_PREDICTED_PASS_COUNT              "PP = the number of tests predicted to pass"
#define STEER_DESCRIPTION_PREVALENCE                        "Prevalence = P / (P + N)"
#define STEER_DESCRIPTION_PREVALENCE_THRESHOLD              "PT = ((SQRT (TPR x FPR)) - FPR) / (TPR - FPR)"
#define STEER_DESCRIPTION_PROBABILITY_VALUE                 "The probability of obtaining test results at least as extreme as the results actually observed, assuming the null hypothesis is correct."
#define STEER_DESCRIPTION_SIGNIFICANCE_LEVEL                "To be provided"
#define STEER_DESCRIPTION_THREAT_SCORE                      "TS = TP / (TP + FN + FP). Also known as critical success index or Jaccard index."
#define STEER_DESCRIPTION_TRUE_NEGATIVE_RATE                "TNR = TN / N. Also known as specificity or selectivity."
#define STEER_DESCRIPTION_TRUE_NEGATIVES                    "TN = the number of predicted failed tests that actually failed. Also known as correct rejection."
#define STEER_DESCRIPTION_TRUE_POSITIVE_RATE                "TPR = TP / P. Also known as recall, sensitivity, probability of detection, hit rate, or power."
#define STEER_DESCRIPTION_TRUE_POSITIVES                    "TP = the number of predicted passed tests that actually passed. Also known as hit."

// =================================================================================================
#endif	// __STEER_JSON_GLOSSARY_H__
// =================================================================================================
