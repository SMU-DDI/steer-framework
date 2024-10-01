// =================================================================================================
//! @file longest_run_of_ones.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the NIST STS longest run of ones test for the STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-19
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_json_utilities.h"
#include "steer_nist_sts_utilities_private.h"
#include "steer_parameters_info_utilities.h"
#include "steer_report_utilities.h"
#include "steer_report_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_string_utilities_private.h"
#include "steer_test_info_utilities.h"
#include "steer_test_shell.h"
#include "steer_utilities.h"
#include "steer_utilities_private.h"
#include "steer_value_utilities.h"
#include "cephes.h"
#include "defs.h"
#include <math.h>

// =================================================================================================
//  Private constants
// =================================================================================================

#define PROGRAM_NAME        "nist_sts_longest_run_of_ones_test"
#define PROGRAM_VERSION     "0.1.0"
#define TEST_NAME           "longest run of ones"
#define TEST_DESCRIPTION \
"The focus of the test is the longest run of ones within M-bit blocks. The purpose of this test \
is to determine whether the length of the longest run of ones within the tested sequence is \
consistent with the length of the longest run of ones that would be expected in a random \
sequence. Note that an irregularity in the expected length of the longest run of ones implies \
that there is also an irregularity in the expected length of the longest run of zeroes. \
Therefore, only a test for ones is necessary."
#define CONFIGURATION_COUNT         1

#define MINIMUM_BITSTREAM_COUNT     1
#define MINIMUM_BITSTREAM_LENGTH    128 
#define MINIMUM_SIGNIFICANCE_LEVEL  0.0
#define MAXIMUM_SIGNIFICANCE_LEVEL  1.0

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_longestrunofonesprivatedata
{
    tSTEER_CliArguments*        cliArguments;
    tSTEER_ReportPtr            report;
    uint64_t                    bitstreamCount;
    uint64_t                    bitstreamLength;
    double                      significanceLevel;
    uint32_t                    significanceLevelPrecision;
    uint64_t                    minimumTestCountRequiredForSignificance;
    uint64_t                    predictedPassedTestCount;
    uint64_t                    predictedFailedTestCount;
    tSTEER_ConfigurationState   configurationState[CONFIGURATION_COUNT];
    uint64_t                    ones;
    uint64_t                    zeros;
    double                      chiSquared;
    int32_t                     numberOfClasses;    // K
    uint32_t                    nu[7];
    int32_t                     numberOfSubstrings; 
    double                      probabilityValue;
    int32_t                     substringLength;
}
tNIST_LongestRunOfOnesPrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_InfoList gReferences = {
    5,
    { 
        "NIST Special Publication 800-22 Rev. 1a, Section 1.1.5",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.4",
        "NIST Special Publication 800-22 Rev. 1a, Section 3.4",
        "NIST Special Publication 800-22 Rev. 1a, Section 4.3(f)",
        "NIST Special Publication 800-22 Rev. 1a, Appendix B"
    }
};

static tSTEER_InfoList gAuthors = {
    2,
    { 
        NIST_STS_AUTHOR_JUAN_SOTO,
        NIST_STS_AUTHOR_LARRY_BASSHAM
    }
};

static tSTEER_InfoList gContributors = {
    1,
    {
        STEER_CONTRIBUTOR_GARY_WOODCOCK        
    }
};

static tSTEER_InfoList gMaintainers = {
    2,
    { 
        STEER_MAINTAINER_ANAMETRIC,
        STEER_MAINTAINER_SMU_DARWIN_DEASON 
    }
};

static tSTEER_TestInfo gTestInfo = {
    TEST_NAME,
    NIST_STS_NAME,
    TEST_DESCRIPTION,
    eSTEER_Complexity_Average,
    &gReferences,
    PROGRAM_NAME,
    PROGRAM_VERSION,
    eSTEER_InputFormat_Bitstream,
    STEER_REPOSITORY_URI,
    &gAuthors,
    &gContributors,
    &gMaintainers,
    STEER_CONTACT
};

static tSTEER_ParameterInfoList gParameterInfoList = {
    3,
    {
        // Required parameter
        {
            STEER_JSON_TAG_BITSTREAM_COUNT,
            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITSTREAMS,
            "1",
            "1",
            NULL
        },

        // Required parameter
        {
            STEER_JSON_TAG_BITSTREAM_LENGTH,
            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITS,
            "1000000",
            "128",
            NULL
        },

        // Required parameter
        {
            STEER_JSON_TAG_SIGNIFICANCE_LEVEL,
            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION,
            NULL,
            STEER_JSON_VALUE_DEFAULT_SIGNIFICANCE_LEVEL,    // Verified (NIST SP 800-22 Rev 1a, 1.1.5)
            STEER_JSON_VALUE_MINIMUM_SIGNIFICANCE_LEVEL,    // Verified (NIST SP 800-22 Rev 1a, section 4.3(f))
            STEER_JSON_VALUE_MAXIMUM_SIGNIFICANCE_LEVEL     // Verified (NIST SP 800-22 Rev 1a, 1.1.5)
        }
    }
};

static tSTEER_ParametersInfo gParametersInfo = {
    TEST_NAME,
    &gParameterInfoList
};

// =================================================================================================
//  RunTest
// =================================================================================================
int32_t RunTest (tNIST_LongestRunOfOnesPrivateData* privateData,
                 uint8_t* bitstreamBuffer,
                 bool* passed)
{
    int32_t result = STEER_RESULT_SUCCESS;
    double theoreticalProbabilities[7] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };    // pi
	int32_t run = 0;
    int32_t observedLongestRunOfOnesFrequency = 0;    // v_n_obs
    int_fast32_t i = 0;
    int_fast32_t j = 0;
    int32_t longestRunOfOnesFrequencies[7] = { 0, 0, 0, 0, 0, 0, 0 };   // V

    memset((void*)&(privateData->nu), 0, 7 * sizeof(uint32_t));

    if (privateData->bitstreamLength < 6272) 
    {
        privateData->numberOfClasses = 3;   
        privateData->substringLength = 8;
        longestRunOfOnesFrequencies[0] = 1; 
        longestRunOfOnesFrequencies[1] = 2; 
        longestRunOfOnesFrequencies[2] = 3; 
        longestRunOfOnesFrequencies[3] = 4;
        theoreticalProbabilities[0] = 0.21484375;
        theoreticalProbabilities[1] = 0.3671875;
        theoreticalProbabilities[2] = 0.23046875;
        theoreticalProbabilities[3] = 0.1875;
    }
    else if (privateData->bitstreamLength < 750000) 
    {
        privateData->numberOfClasses = 5;   
        privateData->substringLength = 128;
        longestRunOfOnesFrequencies[0] = 4; 
        longestRunOfOnesFrequencies[1] = 5; 
        longestRunOfOnesFrequencies[2] = 6; 
        longestRunOfOnesFrequencies[3] = 7; 
        longestRunOfOnesFrequencies[4] = 8; 
        longestRunOfOnesFrequencies[5] = 9;
        theoreticalProbabilities[0] = 0.1174035788;
        theoreticalProbabilities[1] = 0.242955959;
        theoreticalProbabilities[2] = 0.249363483;
        theoreticalProbabilities[3] = 0.17517706;
        theoreticalProbabilities[4] = 0.102701071;
        theoreticalProbabilities[5] = 0.112398847;
    }
    else 
    {
        privateData->numberOfClasses = 6;   
        privateData->substringLength = 10000;
        longestRunOfOnesFrequencies[0] = 10; 
        longestRunOfOnesFrequencies[1] = 11; 
        longestRunOfOnesFrequencies[2] = 12; 
        longestRunOfOnesFrequencies[3] = 13; 
        longestRunOfOnesFrequencies[4] = 14; 
        longestRunOfOnesFrequencies[5] = 15; 
        longestRunOfOnesFrequencies[6] = 16;
        theoreticalProbabilities[0] = 0.0882;
        theoreticalProbabilities[1] = 0.2092;
        theoreticalProbabilities[2] = 0.2483;
        theoreticalProbabilities[3] = 0.1933;
        theoreticalProbabilities[4] = 0.1208;
        theoreticalProbabilities[5] = 0.0675;
        theoreticalProbabilities[6] = 0.0727;
    }

    privateData->numberOfSubstrings = privateData->bitstreamLength/privateData->substringLength;
    for (i = 0; i < privateData->numberOfSubstrings; i++) 
    {
        observedLongestRunOfOnesFrequency = 0;
        run = 0;
        for (j = 0; j < privateData->substringLength; j++) 
        {
            if (bitstreamBuffer[(i * privateData->substringLength) + j] == 1) 
            {
                run++;
                if (run > observedLongestRunOfOnesFrequency)
                    observedLongestRunOfOnesFrequency = run;
            }
            else
                run = 0;
        }

        if (observedLongestRunOfOnesFrequency < longestRunOfOnesFrequencies[0])
            privateData->nu[0]++;

        for (j = 0; j <= privateData->numberOfClasses; j++) 
        {
            if (observedLongestRunOfOnesFrequency == longestRunOfOnesFrequencies[j])
                privateData->nu[j]++;
        }

        if (observedLongestRunOfOnesFrequency > longestRunOfOnesFrequencies[privateData->numberOfClasses])
            privateData->nu[privateData->numberOfClasses]++;
    }

    privateData->chiSquared = 0.0;
    for (i = 0; i <= privateData->numberOfClasses; i++)
    {
        privateData->chiSquared += ((privateData->nu[i] - 
            (privateData->numberOfSubstrings * theoreticalProbabilities[i])) * (privateData->nu[i] - 
            (privateData->numberOfSubstrings * theoreticalProbabilities[i]))) / 
            (privateData->numberOfSubstrings * theoreticalProbabilities[i]);
    }

    privateData->probabilityValue = cephes_igamc((double)(privateData->numberOfClasses/2.0), privateData->chiSquared / 2.0);
    *passed = (privateData->probabilityValue >= privateData->significanceLevel);

    return result;
}

// =================================================================================================
//  GetTestInfo
// =================================================================================================
char* GetTestInfo (void) 
{ 
    // Convert test info to JSON and return it
    char* json = NULL;
    int32_t result = STEER_TestInfoToJson(&gTestInfo, &json);
    if (result == STEER_RESULT_SUCCESS)
        return json;
    else
        return NULL;
}

// =================================================================================================
//  GetParametersInfo
// =================================================================================================
char* GetParametersInfo (void)
{
    // Convert parameters info to JSON and return it
    char* json = NULL;
    int32_t result = STEER_ParametersInfoToJson(&gParametersInfo, &json);
    if (result == STEER_RESULT_SUCCESS)
        return json;
    else
        return NULL;
}

// =================================================================================================
//  InitTest
// =================================================================================================
int32_t InitTest (tSTEER_CliArguments* cliArguments,
                  tSTEER_ParameterSet* parameters,
                  void** testPrivateData,
                  uint64_t* bufferSizeInBytes)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_LongestRunOfOnesPrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tNIST_LongestRunOfOnesPrivateData),
                                  (void**)&privData);

    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        void* nativeValue = NULL;

        // Cache CLI arguments and report pointer
        privData->cliArguments = cliArguments;

        // Get parameters from parameter set
        for (i = 0; i < parameters->count; i++)
        {
            // Required parameter
            if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_BITSTREAM_COUNT) == 0)
            {
                // Convert value from text to native type
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    privData->bitstreamCount = *((uint64_t*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                }
            }

            // Required parameter
            else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_BITSTREAM_LENGTH) == 0)
            {
                // Convert value from text to native type
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    privData->bitstreamLength = *((uint64_t*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                }
            }

            // Required parameter
            else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_SIGNIFICANCE_LEVEL) == 0)
            {
                // Convert value from text to native type
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    privData->significanceLevel = *((double*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                }

                // Get the precision
                if (result == STEER_RESULT_SUCCESS)
                {
                    if (parameters->parameter[i].precision != NULL)
                        result = STEER_ConvertStringToUnsigned32BitInteger(parameters->parameter[i].precision,
                                                                           &(privData->significanceLevelPrecision));
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                break;
        }

        // Make sure we got the required parameters and that they're good
        if ((privData->bitstreamCount < MINIMUM_BITSTREAM_COUNT) || 
            (privData->bitstreamLength < MINIMUM_BITSTREAM_LENGTH) ||
            ((privData->bitstreamLength % 8) != 0) ||
            (privData->significanceLevel <= MINIMUM_SIGNIFICANCE_LEVEL) ||
            (privData->significanceLevel >= MAXIMUM_SIGNIFICANCE_LEVEL))
        {
            result = STEER_CHECK_ERROR(EINVAL);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Calculate the number of tests required 
            // to derive statistically meaningful results
            result = STEER_GetMinimumTestCount(privData->significanceLevel,
                                               privData->bitstreamCount,
                                               &(privData->minimumTestCountRequiredForSignificance),
                                               &(privData->predictedPassedTestCount),
                                               &(privData->predictedFailedTestCount));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Set the configuration ID
            result = STEER_DuplicateString("1", (char**)&(privData->configurationState[0].configurationId));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Return private data and buffer size
            *testPrivateData = (void*)privData;
            *bufferSizeInBytes = privData->bitstreamLength / 8;
        }
    }

    // Check status
    if (result != STEER_RESULT_SUCCESS)
    {
        // Clean up
        STEER_FreeMemory((void**)&privData);
    }
    return result;
}

// =================================================================================================
//  GetConfigurationCount
// =================================================================================================
uint32_t GetConfigurationCount (void* testPrivateData)
{
    return CONFIGURATION_COUNT;
}

// =================================================================================================
//  SetReport
// =================================================================================================
int32_t SetReport (void* testPrivateData,
                   tSTEER_ReportPtr report)
{
    ((tNIST_LongestRunOfOnesPrivateData*)testPrivateData)->report = report;
    return STEER_RESULT_SUCCESS;
}

// =================================================================================================
//  ExecuteTest
// =================================================================================================
int32_t ExecuteTest (void* testPrivateData,
                     const char* bitstreamId,
                     uint8_t* buffer,
                     uint64_t bufferSizeInBytes,
                     uint64_t bytesInBuffer,
                     uint64_t numZeros,
                     uint64_t numOnes)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_LongestRunOfOnesPrivateData* privData = (tNIST_LongestRunOfOnesPrivateData*)testPrivateData;
    bool passed = false;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    uint64_t testId = 0;
    tSTEER_ValueSet* valueSet = NULL;
    char* end = NULL;

    testId = strtoull(bitstreamId, &end, 0) - 1;

    // Setup
    privData->ones = numOnes;
    privData->zeros = numZeros;
    privData->configurationState[0].accumulatedOnes += numOnes;
    privData->configurationState[0].accumulatedZeros += numZeros;

    // Run the test
    result = RunTest(privData, buffer, &passed);

    // Add calculations to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Add ones
        memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf((void*)calculationStr, "%" PRIu64 "", numOnes);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_ONES,
                                            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BITS,
                                            calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Add zeros
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, "%" PRIu64 "", numZeros);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_ZEROS,
                                            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BITS,
                                            calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Add probability value
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->probabilityValue);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_PROBABILITY_VALUE,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Chi squared
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->chiSquared);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_CHI_SQUARED,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Number of substrings
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->numberOfSubstrings);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_NUMBER_OF_SUBSTRINGS,
                                            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_SUBSTRINGS, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Substring length
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->substringLength);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_NUMBER_OF_SUBSTRINGS,
                                            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BITS, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Theoretical probabilities
        result = STEER_NewValueSet(STEER_JSON_TAG_THEORETICAL_PROBABILITIES,
                                   STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
                                   NULL, NULL, &valueSet);
        if (result == STEER_RESULT_SUCCESS)
        {
            char tagStr[STEER_STRING_MAX_LENGTH] = { 0 };
            uint32_t bins = 0;
            uint32_t offset = 0;
            uint_fast32_t i = 0;

            if (privData->numberOfClasses == 3)
            {
                bins = 4;
                offset = 1;
            }
            else if (privData->numberOfClasses == 5)
            {
                bins = 6;
                offset = 4;
            }
            else
            {
                bins = 7;
                offset = 10;
            }

            for (i = 0; i < bins; i++)
            {
                memset((void*)tagStr, 0, STEER_STRING_MAX_LENGTH);
                if (i == 0)
                    sprintf(tagStr, "%s %s %u", STEER_JSON_TAG_PROBABILITIES, "<=", (uint32_t)(i + offset));
                else if (i < (bins - 1))
                    sprintf(tagStr, "%s %s %u", STEER_JSON_TAG_PROBABILITIES, "=", (uint32_t)(i + offset));
                else if (i == (bins - 1))
                    sprintf(tagStr, "%s %s %u", STEER_JSON_TAG_PROBABILITIES, ">=", (uint32_t)(i + offset));
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_UNSIGNED_INTEGER_STRING_FORMAT, privData->nu[i]);
                result = STEER_AddValueToValueSet(tagStr, calculationStr, &valueSet);
                if (result != STEER_RESULT_SUCCESS)
                    break;
            }

            if (result == STEER_RESULT_SUCCESS)
                result = STEER_AddCalculationSetToTest(privData->report,  0, testId, valueSet);
            
            (void)STEER_FreeValueSet(&valueSet);
        }
    }

    // Add criteria to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Probability value in range
        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "%s of %.*f > %.*f",
                STEER_JSON_TAG_PROBABILITY_VALUE,
                STEER_DEFAULT_FLOATING_POINT_PRECISION,
                privData->probabilityValue,
                STEER_DEFAULT_FLOATING_POINT_PRECISION, 0.0);
        result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                                          (privData->probabilityValue > 0.0) ? true : false);
        if (result == STEER_RESULT_SUCCESS)
        {
            memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(criterionStr, "%s of %.*f <= %.*f",
                    STEER_JSON_TAG_PROBABILITY_VALUE,
                    STEER_DEFAULT_FLOATING_POINT_PRECISION,
                    privData->probabilityValue,
                    STEER_DEFAULT_FLOATING_POINT_PRECISION, 1.0);
            result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                                              (privData->probabilityValue <= 1.0) ? true : false);
        }
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Probability value
        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "%s of %.*f >= %s of %.*f",
                STEER_JSON_TAG_PROBABILITY_VALUE,
                STEER_DEFAULT_FLOATING_POINT_PRECISION,
                privData->probabilityValue,
                STEER_JSON_TAG_SIGNIFICANCE_LEVEL,
                privData->significanceLevelPrecision,
                privData->significanceLevel);
        result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                                          (privData->probabilityValue >= privData->significanceLevel) ? true : false);
    }

    // Add evaluation to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        bool passed = false;
        result = STEER_AddEvaluationToTest(privData->report, 0, testId, &passed);
        if (result == STEER_RESULT_SUCCESS)
        {
            privData->configurationState[0].testsRun++;
            if (passed)
                privData->configurationState[0].testsPassed++;
            else
                privData->configurationState[0].testsFailed++;
        }
    }

    // Clean up
    STEER_FreeMemory((void**)&buffer);

    return result;
}

// =================================================================================================
//  FinalizeTest
// =================================================================================================
int32_t FinalizeTest (void** testPrivateData,
                      uint64_t suppliedNumberOfBitstreams)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_LongestRunOfOnesPrivateData* privData = (tNIST_LongestRunOfOnesPrivateData*)(*testPrivateData);

    if (privData != NULL)
    {
        double probabilityValueUniformity = 0.0;
        uint64_t proportionThresholdMinimum = 0;
        uint64_t proportionThresholdMaximum = 0;
        
        // Add required metrics to configuration
        result = STEER_NistStsAddRequiredMetricsToConfiguration(privData->report, 0,
                                                                suppliedNumberOfBitstreams,
                                                                privData->minimumTestCountRequiredForSignificance,
                                                                privData->configurationState[0].testsPassed,
                                                                privData->predictedPassedTestCount,
                                                                privData->configurationState[0].accumulatedOnes,
                                                                privData->configurationState[0].accumulatedZeros);

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add test specific metrics
            result = STEER_NistStsAddMetricsToConfiguration(privData->report, 0, false,
                                                            suppliedNumberOfBitstreams,
                                                            privData->significanceLevel,
                                                            &probabilityValueUniformity,
                                                            &proportionThresholdMinimum,
                                                            &proportionThresholdMaximum);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add optional confusion matrix metrics to configuration
            result = STEER_AddConfusionMatrixMetricsToConfiguration(privData->report, 0,
                                                                    privData->minimumTestCountRequiredForSignificance,
                                                                    privData->configurationState[0].testsRun,
                                                                    privData->configurationState[0].testsPassed,
                                                                    privData->configurationState[0].testsFailed,
                                                                    privData->predictedPassedTestCount,
                                                                    privData->predictedFailedTestCount);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add required criteria to configuration
            result = STEER_NistStsAddRequiredCriterionToConfiguration(privData->report, 0,
                                                                      suppliedNumberOfBitstreams,
                                                                      privData->configurationState[0].testsPassed,
                                                                      privData->significanceLevel,
                                                                      privData->significanceLevelPrecision,
                                                                      privData->minimumTestCountRequiredForSignificance);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add test specific criteria to configuration
            result = STEER_NistStsAddCriteriaToConfiguration(privData->report, 0,
                                                             probabilityValueUniformity,
                                                             proportionThresholdMinimum,
                                                             proportionThresholdMaximum,
                                                             privData->configurationState[0].testsRun, 
                                                             privData->configurationState[0].testsPassed);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Add evaluation to configuration
            result = STEER_AddEvaluationToConfiguration(privData->report, 0);
        }

        // Clean up
        STEER_FreeMemory((void**)testPrivateData);
    }
    return result;
}

// =================================================================================================
//  main
// =================================================================================================
int main (int argc, const char * argv[])
{
    // Run STEER program
    return STEER_Run (PROGRAM_NAME, argc, argv,
                      GetTestInfo, GetParametersInfo,
                      InitTest, GetConfigurationCount,
                      SetReport, ExecuteTest, FinalizeTest);
}

// =================================================================================================
