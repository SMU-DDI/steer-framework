// =================================================================================================
//! @file random_excursions.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the NIST STS random excursions test for the STEER framework.
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
#include <errno.h>

// =================================================================================================
//  Private constants
// =================================================================================================

#define PROGRAM_NAME        "nist_sts_random_excursions_test"
#define PROGRAM_VERSION     "0.1.0"
#define TEST_NAME           "random excursions"
#define TEST_DESCRIPTION \
"The focus of this test is the number of cycles having exactly K visits in a cumulative sum \
random walk. The cumulative sum random walk is derived from partial sums after the (0,1) \
sequence is transferred to the appropriate (-1, +1) sequence. A cycle of a random walk consists \
of a sequence of steps of unit length taken at random that begin at and return to the origin. \
The purpose of this test is to determine if the number of visits to a particular state within \
a cycle deviates from what one would expect for a random sequence. This test is actually a \
series of eight tests (and conclusions), one test and conclusion for each of the states: \
-4, -3, -2, -1 and +1, +2, +3, +4."
#define CONFIGURATION_COUNT         8

#define MINIMUM_BITSTREAM_COUNT     1
#define MINIMUM_BITSTREAM_LENGTH    1000000 
#define MINIMUM_SIGNIFICANCE_LEVEL  0.0
#define MAXIMUM_SIGNIFICANCE_LEVEL  1.0

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_randomexcursionsprivatedata
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
    int32_t                     numberOfCycles;
    int32_t                     maxNumberOfCycles;
    double                      probabilityValue;
    double                      rejectionConstraint;
    int32_t                     x;
    int32_t*                    S_k;
    int32_t*                    cycle;
}
tNIST_RandomExcursionsPrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_InfoList gReferences = {
    7,
    { 
        "NIST Special Publication 800-22 Rev. 1a, Section 1.1.5",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.14",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.14.7",
        "NIST Special Publication 800-22 Rev. 1a, Section 3.14",
        "NIST Special Publication 800-22 Rev. 1a, Section 4.3(a)",
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
            "1000000",      // Verified (NIST SP 800-22 Rev 1a, 2.14.7)
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
    tNIST_RandomExcursionsPrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tNIST_RandomExcursionsPrivateData),
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
            // Setup
            privData->maxNumberOfCycles = MAX(1000, privData->bitstreamLength/100);
            privData->rejectionConstraint = MAX(0.005 * pow(privData->bitstreamLength, 0.5), 500);

            result = STEER_AllocateMemory(privData->bitstreamLength * sizeof(int32_t),
                                          (void**)&(privData->S_k));
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_AllocateMemory((MAX(1000, privData->bitstreamLength/100) * sizeof(int32_t)),
                                              (void**)&(privData->cycle));
            }
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
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_RandomExcursionsPrivateData* privData = NULL;
    char attributeStr[STEER_STRING_MAX_LENGTH] = { 0 };
    int32_t stateX[8] = { -4, -3, -2, -1, 1, 2, 3, 4 };
    uint_fast32_t i = 0;

    privData = (tNIST_RandomExcursionsPrivateData*)testPrivateData;
    privData->report = report;

    // Walk configurations
    for (i = 0; i < CONFIGURATION_COUNT; i++)
    {
        privData->configurationState[i].configurationId = i;

        // Add random excursion state attribute to configuration
        if (!STEER_ConfigurationHasAttribute(privData->report, i,
                                             STEER_JSON_TAG_RANDOM_EXCURSION_STATE))
        {
            memset((void*)attributeStr, 0, STEER_STRING_MAX_LENGTH);
            sprintf(attributeStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, stateX[i]);
            result = STEER_AddAttributeToConfiguration(privData->report, i,
                                                       STEER_JSON_TAG_RANDOM_EXCURSION_STATE,
                                                       STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                                       NULL, NULL, attributeStr);
        }

        if (result != STEER_RESULT_SUCCESS)
            break;
    }
    return result;
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
    tNIST_RandomExcursionsPrivateData* privData = (tNIST_RandomExcursionsPrivateData*)testPrivateData;
    bool passed = false;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char progressStr[STEER_STRING_MAX_LENGTH] = { 0 };
    int32_t b = 0;
    int_fast32_t i = 0;
    int_fast32_t j = 0;
    int_fast32_t k = 0;
    uint64_t testId = 0;
    char* end = NULL;
    int32_t cycleStart = 0;
    int32_t cycleStop = 0;
    int32_t stateX[8] = { -4, -3, -2, -1, 1, 2, 3, 4 };
    int32_t counter[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    double nu[6][8] = { {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0} };
    double pi[5][6] = { {0.0000000000, 0.00000000000, 0.00000000000, 0.00000000000, 0.00000000000, 0.0000000000}, 
                        {0.5000000000, 0.25000000000, 0.12500000000, 0.06250000000, 0.03125000000, 0.0312500000},
                        {0.7500000000, 0.06250000000, 0.04687500000, 0.03515625000, 0.02636718750, 0.0791015625},
                        {0.8333333333, 0.02777777778, 0.02314814815, 0.01929012346, 0.01607510288, 0.0803755143},
                        {0.8750000000, 0.01562500000, 0.01367187500, 0.01196289063, 0.01046752930, 0.0732727051} };

    testId = strtoull(bitstreamId, &end, 0) - 1;

    // Setup
    privData->ones = numOnes;
    privData->zeros = numZeros;

    // Determine cycles
    privData->numberOfCycles = 0;
    privData->S_k[0] = (2 * (int)buffer[0]) - 1;
    for (i = 1; i < privData->bitstreamLength; i++) 
    {
        privData->S_k[i] = privData->S_k[i - 1] + (2 * buffer[i]) - 1;
        if (privData->S_k[i] == 0) 
        {
            privData->numberOfCycles++;
            result = STEER_CHECK_CONDITION((privData->numberOfCycles <= privData->maxNumberOfCycles), 
                                           NIST_RESULT_NUM_CYCLES_GT_MAX);
            if (result == STEER_RESULT_SUCCESS)
                privData->cycle[privData->numberOfCycles] = i;
            else
                break;
        }
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        if (privData->S_k[privData->bitstreamLength - 1] != 0)
            privData->numberOfCycles++;
        privData->cycle[privData->numberOfCycles] = privData->bitstreamLength;

        cycleStart = 0;
        cycleStop  = privData->cycle[1];
        for (k = 0; k < 6; k++)
            for (i = 0; i < 8; i++)
                nu[k][i] = 0.;

        // For each cycle
        for (j = 1; j <= privData->numberOfCycles; j++) 
        {                          
            for (i = 0; i < 8; i++)
                counter[i] = 0;
            for (i = cycleStart; i < cycleStop; i++) 
            {
                if ((privData->S_k[i] >= 1 && privData->S_k[i] <= 4) || 
                    (privData->S_k[i] >= -4 && privData->S_k[i] <= -1)) 
                {
                    if (privData->S_k[i] < 0)
                        b = 4;
                    else
                        b = 3;
                    counter[privData->S_k[i] + b]++;
                }
            }
            cycleStart = privData->cycle[j] + 1;
            if (j < privData->numberOfCycles)
                cycleStop = privData->cycle[j + 1];
            
            for (i = 0; i < 8; i++) 
            {
                if ((counter[i] >= 0) && (counter[i] <= 4))
                    nu[counter[i]][i]++;
                else if (counter[i] >= 5)
                    nu[5][i]++;
            }
        }
        
        for (i = 0; i < CONFIGURATION_COUNT; i++) 
        {
            privData->configurationState[i].accumulatedOnes += numOnes;
            privData->configurationState[i].accumulatedZeros += numZeros;

            if (privData->cliArguments->reportProgress)
            {
                memset((void*)progressStr, 0, STEER_STRING_MAX_LENGTH);
                if (i < 4)
                    sprintf(progressStr, "Testing excursion state %d...", (int)(i - 4));
                else
                    sprintf(progressStr, "Testing excursion state %d...", (int)(i - 3));
                STEER_REPORT_PROGRESS(privData->cliArguments->programName, progressStr);
            }

            privData->x = stateX[i];
            privData->chiSquared = 0.0;
            for (k = 0; k < 6; k++)
            {
                privData->chiSquared += 
                    pow(nu[k][i] - 
                    privData->numberOfCycles * pi[(int32_t)abs(privData->x)][k], 2) / 
                    (privData->numberOfCycles * pi[(int32_t)abs(privData->x)][k]);
            }
            privData->probabilityValue = cephes_igamc(2.5, privData->chiSquared/2.0);

            passed = (privData->probabilityValue >= privData->significanceLevel);

            // Add calculations to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                // Add ones
                memset(calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf((void*)calculationStr, "%" PRIu64 "", numOnes);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
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
                result = STEER_AddCalculationToTest(privData->report, i, testId,
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
                result = STEER_AddCalculationToTest(privData->report, i, testId,
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
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_CHI_SQUARED,
                                                    STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                                    STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                                    NULL, calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Number of cycles
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->numberOfCycles);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_NUMBER_OF_CYCLES,
                                                    STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                                    NULL, STEER_JSON_VALUE_CYCLES, calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Rejection constraint
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->rejectionConstraint);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_REJECTION_CONSTRAINT,
                                                    STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                                    STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                                    NULL, calculationStr);
            }

            // Add criteria to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                // Number of cycles
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %d <= %s %s of %d",
                        STEER_JSON_TAG_NUMBER_OF_CYCLES,
                        privData->numberOfCycles, "maximum",
                        STEER_JSON_TAG_NUMBER_OF_CYCLES,
                        privData->maxNumberOfCycles);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                    (privData->numberOfCycles <= privData->maxNumberOfCycles) ? true : false);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Probability value in range
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %.*f > %.*f",
                        STEER_JSON_TAG_PROBABILITY_VALUE,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION,
                        privData->probabilityValue,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, 0.0);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                    (privData->probabilityValue > 0.0) ? true : false);
                if (result == STEER_RESULT_SUCCESS)
                {
                    memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                    sprintf(criterionStr, "%s of %.*f <= %.*f",
                            STEER_JSON_TAG_PROBABILITY_VALUE,
                            STEER_DEFAULT_FLOATING_POINT_PRECISION,
                            privData->probabilityValue,
                            STEER_DEFAULT_FLOATING_POINT_PRECISION, 1.0);
                    result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
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
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                    (privData->probabilityValue >= privData->significanceLevel) ? true : false);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Rejection constraint
                memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(criterionStr, "%s of %d >= %s of %.*f",
                        STEER_JSON_TAG_NUMBER_OF_CYCLES,
                        privData->numberOfCycles,
                        STEER_JSON_TAG_REJECTION_CONSTRAINT,
                        STEER_DEFAULT_FLOATING_POINT_PRECISION,
                        privData->rejectionConstraint);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                    (privData->numberOfCycles >= privData->rejectionConstraint) ? true : false);
            }

            // Add evaluation to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                result = STEER_AddEvaluationToTest(privData->report, i, testId, &passed);
                if (result == STEER_RESULT_SUCCESS)
                {
                    privData->configurationState[i].testsRun++;
                    if (passed)
                        privData->configurationState[i].testsPassed++;
                    else
                        privData->configurationState[i].testsFailed++;
                }
            }
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
    tNIST_RandomExcursionsPrivateData* privData =  
        (tNIST_RandomExcursionsPrivateData*)(*testPrivateData);

    if (privData != NULL)
    {
        double probabilityValueUniformity = 0.0;
        uint64_t proportionThresholdMinimum = 0;
        uint64_t proportionThresholdMaximum = 0;
        uint_fast32_t i = 0;

        for (i = 0; i < CONFIGURATION_COUNT; i++)
        {            
            // Add required metrics to configuration
            result = STEER_NistStsAddRequiredMetricsToConfiguration(privData->report, i,
                                                                    suppliedNumberOfBitstreams,
                                                                    privData->minimumTestCountRequiredForSignificance,
                                                                    privData->configurationState[i].testsPassed,
                                                                    privData->predictedPassedTestCount,
                                                                    privData->configurationState[i].accumulatedOnes,
                                                                    privData->configurationState[i].accumulatedZeros);

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add test specific metrics
                result = STEER_NistStsAddMetricsToConfiguration(privData->report, i, false,
                                                                suppliedNumberOfBitstreams,
                                                                privData->significanceLevel,
                                                                &probabilityValueUniformity,
                                                                &proportionThresholdMinimum,
                                                                &proportionThresholdMaximum);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add optional confusion matrix metrics to configuration
                result = STEER_AddConfusionMatrixMetricsToConfiguration(privData->report, i,
                                                                        privData->minimumTestCountRequiredForSignificance,
                                                                        privData->configurationState[i].testsRun,
                                                                        privData->configurationState[i].testsPassed,
                                                                        privData->configurationState[i].testsFailed,
                                                                        privData->predictedPassedTestCount,
                                                                        privData->predictedFailedTestCount);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add required criteria to configuration
                result = STEER_NistStsAddRequiredCriterionToConfiguration(privData->report, i,
                                                                          suppliedNumberOfBitstreams,
                                                                          privData->configurationState[i].testsPassed,
                                                                          privData->significanceLevel,
                                                                          privData->significanceLevelPrecision,
                                                                          privData->minimumTestCountRequiredForSignificance);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add test specific criteria to configuration
                result = STEER_NistStsAddCriteriaToConfiguration(privData->report, i,
                                                                 probabilityValueUniformity,
                                                                 proportionThresholdMinimum,
                                                                 proportionThresholdMaximum,
                                                                 privData->configurationState[i].testsRun, 
                                                                 privData->configurationState[i].testsPassed);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add evaluation to configuration
                result = STEER_AddEvaluationToConfiguration(privData->report, i);
            }

            // Clean up
            STEER_FreeMemory((void**)&(privData->S_k));
            STEER_FreeMemory((void**)&(privData->cycle));
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
