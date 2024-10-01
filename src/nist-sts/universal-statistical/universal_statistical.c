// =================================================================================================
//! @file universal_statistics.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the NIST STS universal statistics test for the STEER framework.
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

#define PROGRAM_NAME        "nist_sts_universal_statistical_test"
#define PROGRAM_VERSION     "0.1.0"
#define TEST_NAME           "universal statistical"
#define TEST_DESCRIPTION \
"The focus of this test is the number of bits between matching patterns (a measure that is \
related to the length of a compressed sequence). The purpose of the test is to detect whether \
or not the sequence can be significantly compressed without loss of information. A significantly \
compressible sequence is considered to be non-random."
#define CONFIGURATION_COUNT         1

#define MINIMUM_BITSTREAM_COUNT     1
#define MINIMUM_SIGNIFICANCE_LEVEL  0.0
#define MAXIMUM_SIGNIFICANCE_LEVEL  1.0

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_universalstatisticalprivatedata
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
    uint64_t                    suppliedNumberOfBitstreams;
    tSTEER_ConfigurationState   configurationState[CONFIGURATION_COUNT];
    uint64_t                    ones;
    uint64_t                    zeros;
    uint64_t                    bitsDiscarded;      
    double                      preComputedExpectedValue;
    int32_t                     numNonOverlappingBlocksInTestSegment;   // K
    int32_t                     blockLength;                            // L
    int32_t                     minBlockLength;                         // minL
    int32_t                     maxBlockLength;                         // maxL
    double                      testStatistic;                          // phi
    double                      probabilityValue;
    int32_t                     numBlocksInInitSequence;                // Q
    double                      minNumBlocksInInitSequence;             // minQ
    double                      theoreticalStandardDeviation;           // sigma
    double                      sum;
    double                      preComputedVariance;
}
tNIST_UniversalStatisticalPrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_InfoList gReferences = {
    5,
    { 
        "NIST Special Publication 800-22 Rev. 1a, Section 1.1.5",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.9",
        "NIST Special Publication 800-22 Rev. 1a, Section 3.9",
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
            NULL,
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
int32_t RunTest (tNIST_UniversalStatisticalPrivateData* privateData,
                 uint8_t* bitstreamBuffer,
                 bool* passed)
{
    int32_t result = STEER_RESULT_SUCCESS;
	int_fast32_t i = 0;
    int_fast32_t j = 0;
    int32_t p = 0;
	double arg = 0.0;
    double sqrt2 = 0.0;
    double c = 0.0;
	uint32_t* T = NULL;
    uint32_t decRep = 0;
	double preComputedExpectedValue[17] = { 0, 0, 0, 0, 0, 0, 5.2177052, 6.1962507, 7.1836656,
				                            8.1764248, 9.1723243, 10.170032, 11.168765,
				                            12.168070, 13.167693, 14.167488, 15.167379 };
	double preComputedVariance[17] = { 0, 0, 0, 0, 0, 0, 2.954, 3.125, 3.238, 3.311, 3.356, 3.384,
				                       3.401, 3.410, 3.416, 3.419, 3.421 };

    // Setup
    privateData->bitsDiscarded = 0;
    privateData->blockLength = 0;
    privateData->numBlocksInInitSequence = 0;
    privateData->numNonOverlappingBlocksInTestSegment = 0;
    privateData->sum = 0.0;
    privateData->theoreticalStandardDeviation = 0.0;
    privateData->preComputedVariance = 0.0;
    privateData->preComputedExpectedValue = 0.0;
    privateData->testStatistic = 0.0;
    privateData->probabilityValue = 0.0;
    *passed = false;
	
    /* * * * * * * * * ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    * THE FOLLOWING REDEFINES L, SHOULD THE CONDITION:     n >= 1010*2^L*L       *
    * NOT BE MET, FOR THE BLOCK LENGTH L.                                        *
    * * * * * * * * * * ** * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    privateData->blockLength = 5;
    if (privateData->bitstreamLength >= 387840)     privateData->blockLength = 6;
    if (privateData->bitstreamLength >= 904960)     privateData->blockLength = 7;
    if (privateData->bitstreamLength >= 2068480)    privateData->blockLength = 8;
    if (privateData->bitstreamLength >= 4654080)    privateData->blockLength = 9;
    if (privateData->bitstreamLength >= 10342400)   privateData->blockLength = 10;
    if (privateData->bitstreamLength >= 22753280)   privateData->blockLength = 11;
    if (privateData->bitstreamLength >= 49643520)   privateData->blockLength = 12;
    if (privateData->bitstreamLength >= 107560960)  privateData->blockLength = 13;
    if (privateData->bitstreamLength >= 231669760)  privateData->blockLength = 14;
    if (privateData->bitstreamLength >= 496435200)  privateData->blockLength = 15;
    if (privateData->bitstreamLength >= 1059061760) privateData->blockLength = 16;

    privateData->minBlockLength = 6;
    privateData->maxBlockLength = 16;
    privateData->numBlocksInInitSequence = 10 * (int)pow(2, privateData->blockLength);
    privateData->minNumBlocksInInitSequence = 10 * pow(2, privateData->blockLength);

    // Blocks to test
    privateData->numNonOverlappingBlocksInTestSegment = 
        (int32_t) (floor(privateData->bitstreamLength/privateData->blockLength) - 
        (double)privateData->numBlocksInInitSequence);	 		 

    p = (int32_t)pow(2, privateData->blockLength);
    result = STEER_CHECK_CONDITION((privateData->blockLength >= privateData->minBlockLength), 
        NIST_RESULT_BLOCK_LENGTH_L_LT_MIN);
    if (result == STEER_RESULT_SUCCESS)
    {
        result = STEER_CHECK_CONDITION((privateData->blockLength <= privateData->maxBlockLength), 
            NIST_RESULT_BLOCK_LENGTH_L_GT_MAX);
        if (result == STEER_RESULT_SUCCESS)
            result = STEER_CHECK_CONDITION(((double)privateData->numBlocksInInitSequence >= privateData->minNumBlocksInInitSequence), 
                NIST_RESULT_NUM_BLOCKS_IN_INIT_SEQ_LT_MIN);
    }

    if (result == STEER_RESULT_SUCCESS)
        result = STEER_AllocateMemory(p * sizeof(uint32_t), (void**)&T);
    
    if (result == STEER_RESULT_SUCCESS)
    {
        // Compute the expected: Formula 16, in Marsaglia's Paper
        c = 0.7 - (0.8/(double)privateData->blockLength) + 
            (4 + (32/(double)privateData->blockLength)) * 
            pow(privateData->numNonOverlappingBlocksInTestSegment, -3/(double)privateData->blockLength)/15;
        privateData->theoreticalStandardDeviation = 
            c * sqrt(preComputedVariance[privateData->blockLength]/(double)privateData->numNonOverlappingBlocksInTestSegment);
        sqrt2 = sqrt(2);
        privateData->sum = 0.0;
        for (i = 0; i < p; i++)
            T[i] = 0;

        // Initialize table
        for (i = 1; i <= privateData->numBlocksInInitSequence; i++) 
        {		
            decRep = 0;
            for (j = 0; j < privateData->blockLength; j++)
                decRep += bitstreamBuffer[((i - 1) * privateData->blockLength) + j] * (uint32_t)pow(2, privateData->blockLength - 1 - j);
            T[decRep] = i;
        }

        // Process blocks
        for (i = privateData->numBlocksInInitSequence + 1; i <= privateData->numBlocksInInitSequence + privateData->numNonOverlappingBlocksInTestSegment; i++) 
        { 	
            decRep = 0;
            for (j = 0; j < privateData->blockLength; j++)
                decRep += bitstreamBuffer[((i - 1) * privateData->blockLength) + j] * (uint32_t)pow(2, privateData->blockLength - 1 - j);
            privateData->sum += log(i - T[decRep])/log(2);
            T[decRep] = i;
        }
        privateData->testStatistic = (double)(privateData->sum/(double)privateData->numNonOverlappingBlocksInTestSegment);

        arg = fabs(privateData->testStatistic - preComputedExpectedValue[privateData->blockLength])/(sqrt2 * privateData->theoreticalStandardDeviation);
        privateData->probabilityValue = erfc(arg);

        privateData->bitsDiscarded = privateData->bitstreamLength - 
            ((privateData->numBlocksInInitSequence + privateData->numNonOverlappingBlocksInTestSegment) * privateData->blockLength);
        privateData->preComputedVariance = preComputedVariance[privateData->blockLength];
        privateData->preComputedExpectedValue = preComputedExpectedValue[privateData->blockLength];
        *passed = (privateData->probabilityValue >= privateData->significanceLevel);
    }

    // Clean up
    STEER_FreeMemory((void**)&T);

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
    tNIST_UniversalStatisticalPrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tNIST_UniversalStatisticalPrivateData),
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
    ((tNIST_UniversalStatisticalPrivateData*)testPrivateData)->report = report;
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
    tNIST_UniversalStatisticalPrivateData* privData = (tNIST_UniversalStatisticalPrivateData*)testPrivateData;
    bool passed = false;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    uint64_t testId = 0;
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
        // Add bits discarded
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, "%" PRIu64 "", privData->bitsDiscarded);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_BITS_DISCARDED,
                                            STEER_JSON_VALUE_UNSIGNED_64_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BITS, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Block length
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->blockLength);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_BLOCK_LENGTH,
                                            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BITS, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Number of blocks in initialization sequence
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->numBlocksInInitSequence);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_NUMBER_OF_BLOCKS_IN_INITIALIZATION_SEQUENCE,
                                            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BLOCKS, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Number of non overlapping blocks in test segment
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, privData->numNonOverlappingBlocksInTestSegment);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_NUMBER_OF_NON_OVERLAPPING_BLOCKS_IN_TEST_SEGMENT,
                                            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                            NULL, STEER_JSON_VALUE_BLOCKS, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Pre computed expected value
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->preComputedExpectedValue);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_PRE_COMPUTED_EXPECTED_VALUE,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Pre computed variance
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->preComputedVariance);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_PRE_COMPUTED_VARIANCE,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Sum
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->sum);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_SUM,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Test statistic
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->testStatistic);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_TEST_STATISTIC,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Theoretical standard deviation
        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->theoreticalStandardDeviation);
        result = STEER_AddCalculationToTest(privData->report, 0, testId,
                                            STEER_JSON_TAG_THEORETICAL_STANDARD_DEVIATION,
                                            STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                            STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                            NULL, calculationStr);
    }

    // Add criteria to current test
    if (result == STEER_RESULT_SUCCESS)
    {
        // Minimum block length
        memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
        sprintf(criterionStr, "%s of %d >= %s minimum of %d",
                STEER_JSON_TAG_BLOCK_LENGTH,
                privData->blockLength,
                STEER_JSON_TAG_BLOCK_LENGTH,
                privData->minBlockLength);
        result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                                          (privData->blockLength >= privData->minBlockLength) ? true : false);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Maximum block length
        memset((void*)criterionStr, 0, 256);
        sprintf(criterionStr, "%s of %d <= %s maximum of %d",
                STEER_JSON_TAG_BLOCK_LENGTH,
                privData->blockLength,
                STEER_JSON_TAG_BLOCK_LENGTH,
                privData->maxBlockLength);
                result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                                                  (privData->blockLength <= privData->maxBlockLength) ? true : false);
    }

    if (result == STEER_RESULT_SUCCESS)
    {
        // Minimum number of blocks in initialization sequence
        memset((void*)criterionStr, 0, 256);
        sprintf(criterionStr, "%s of %d >= %s minimum of %.*f",
                STEER_JSON_TAG_NUMBER_OF_BLOCKS_IN_INITIALIZATION_SEQUENCE,
                privData->numBlocksInInitSequence,
                STEER_JSON_TAG_NUMBER_OF_BLOCKS_IN_INITIALIZATION_SEQUENCE,
                STEER_DEFAULT_FLOATING_POINT_PRECISION,
                privData->minNumBlocksInInitSequence);
                result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                                                  (privData->numBlocksInInitSequence >= privData->minNumBlocksInInitSequence) ? true : false);
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
    tNIST_UniversalStatisticalPrivateData* privData = (tNIST_UniversalStatisticalPrivateData*)(*testPrivateData);

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
