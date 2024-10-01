// =================================================================================================
//! @file non_overlapping_template_matching_st.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the single-threaded version of the NIST STS non overlapping 
//! template matchings test for the STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-21
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer_file_system_utilities.h"
#include "steer_json_utilities.h"
#include "steer_nist_sts_utilities_private.h"
#include "steer_parameters_info_utilities.h"
#include "steer_report_utilities.h"
#include "steer_report_utilities_private.h"
#include "steer_string_utilities.h"
#include "steer_string_utilities_private.h"
#include "steer_test_info_utilities.h"
#include "steer_utilities.h"
#include "steer_utilities_private.h"
#include "steer_value_utilities.h"
#include "cephes.h"
#include "defs.h"
#include "cJSON.h"
#include <math.h>

// =================================================================================================
//  Private constants
// =================================================================================================

#include "non_overlapping_template_matching_templates.c"

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_nonoverlappingtemplatematchingprivatedata
{
    tSTEER_ReportPtr            report;
    char*                       templateDataPtr;
    uint64_t                    ones;
    uint64_t                    zeros;
    double                      chiSquared;
    double                      probabilityValue;
    uint8_t*                    sequence;
    char*                       template;
    uint32_t*                   templateFrequencies;        // Wj
    tSTEER_ConfigurationState   configurationState[];
}
tNIST_NonOverlappingTemplateMatchingPrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_ParameterInfoList gParameterInfoList = {
    6,
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
            "1000000",  // Verified (NIST SP 800-22 Rev 1a, 2.7.7)
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
        },

        // Test specific parameter
        {
            STEER_JSON_TAG_BLOCK_LENGTH,
            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITS,
            "9",        // Value of 9 or 10 recommended (NIST SP 800-22 Rev 1a, 2.7.7)
            "2",
            "21"
        },

        // Test specific parameter
        {
            STEER_JSON_TAG_DEGREES_OF_FREEDOM,
            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
            NULL,
            NULL,
            "5",
            NULL,
            NULL
        },

        // Test specific parameter
        {
            STEER_JSON_TAG_NUMBER_OF_INDEPENDENT_BLOCKS,
            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BLOCKS,
            "8",        // Default value (NIST SP 800-22 Rev 1a, 2.7.7)
            NULL,
            "100"       // Maximum value of 100 recommended (NIST SP 800-22 Rev 1a, 2.7.7)
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
int32_t RunTest (tNIST_NonOverlappingTemplateMatchingPrivateData* privateData,
                 uint8_t* bitstreamBuffer,
                 bool* passed)
{
    int32_t result = STEER_RESULT_SUCCESS;
	uint32_t bit = 0;
    uint32_t observedTemplateFrequency = 0; // W_obs
	int_fast32_t i = 0;
    int_fast32_t j = 0;
    int_fast32_t k = 0;
    int32_t match = 0;

    // Get template
    for (i = 0; i < gCommonData.blockLength; i++) 
    {
        sscanf(privateData->templateDataPtr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, &bit);
        privateData->sequence[i] = bit;
        if (bit == 0)
            privateData->template[i] = 0x30;
        else if (bit == 1)
            privateData->template[i] = 0x31;
        privateData->templateDataPtr += 2;
    }

    for (i = 0; i < gCommonData.numberOfIndependentBlocks; i++) 
    {
        observedTemplateFrequency = 0;
        for (j = 0; j < gCommonData.substringLength - gCommonData.blockLength + 1; j++) 
        {
            match = 1;
            for (k = 0; k < gCommonData.blockLength; k++) 
            {
                if ((int32_t)privateData->sequence[k] != (int32_t)bitstreamBuffer[(i * gCommonData.substringLength) + j + k]) 
                {
                    match = 0;
                    break;
                }
            }
            if (match == 1) 
            {
                observedTemplateFrequency++;
                j += gCommonData.blockLength - 1;
            }
        }
        privateData->templateFrequencies[i] = observedTemplateFrequency;
    }

    // Compute chi squared
    privateData->chiSquared = 0.0;                                   
    for (i = 0; i < gCommonData.numberOfIndependentBlocks; i++) 
    {
        privateData->chiSquared += 
            pow(((double)privateData->templateFrequencies[i] - 
                gCommonData.theoreticalMean)/pow(gCommonData.varWj, 0.5), 2);
    }

    // Calculate p value
    privateData->probabilityValue = 
        cephes_igamc((double)(gCommonData.numberOfIndependentBlocks/2.0), privateData->chiSquared/2.0);

    *passed = (privateData->probabilityValue >= gCommonData.significanceLevel);

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
    tNIST_NonOverlappingTemplateMatchingPrivateData* privData = NULL;

    // Setup
    *testPrivateData = NULL;
    *bufferSizeInBytes = 0;
    memset((void*)&gCommonData, 0, sizeof(tNIST_NonOverlappingTemplateMatchingCommon));
    gCommonData.cliArguments = cliArguments;

    // Allocate private data
    result = STEER_AllocateMemory(sizeof(tNIST_NonOverlappingTemplateMatchingPrivateData),
                                  (void**)&privData);
    if (result == STEER_RESULT_SUCCESS)
    {
        uint_fast32_t i = 0;
        void* nativeValue = NULL;

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
                    gCommonData.bitstreamCount = *((uint64_t*)nativeValue);
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
                    gCommonData.bitstreamLength = *((uint64_t*)nativeValue);
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
                    gCommonData.significanceLevel = *((double*)nativeValue);
                    STEER_FreeMemory(&nativeValue);
                }

                // Get the precision
                if (result == STEER_RESULT_SUCCESS)
                {
                    if (parameters->parameter[i].precision != NULL)
                        result = STEER_ConvertStringToUnsigned32BitInteger(parameters->parameter[i].precision,
                                                                           &(gCommonData.significanceLevelPrecision));
                }
            }

            // Test specific parameter
            else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_BLOCK_LENGTH) == 0)
            {
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    gCommonData.blockLength = *((int32_t*)nativeValue);
                    STEER_FreeMemory((void**)&nativeValue);
                }
            }

            // Test specific parameter
            else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_DEGREES_OF_FREEDOM) == 0)
            {
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    gCommonData.degreesOfFreedom = *((int32_t*)nativeValue);
                    STEER_FreeMemory((void**)&nativeValue);
                }
            }

            // Test specific parameter
            else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_NUMBER_OF_INDEPENDENT_BLOCKS) == 0)
            {
                result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                              parameters->parameter[i].value,
                                              &nativeValue);
                if (result == STEER_RESULT_SUCCESS)
                {
                    gCommonData.numberOfIndependentBlocks = *((int32_t*)nativeValue);
                    STEER_FreeMemory((void**)&nativeValue);
                }
            }

            if (result != STEER_RESULT_SUCCESS)
                break;
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            gCommonData.substringLength = gCommonData.bitstreamLength/gCommonData.numberOfIndependentBlocks;

            // Make sure we got the required parameters and that they're good
            if ((gCommonData.bitstreamCount < MINIMUM_BITSTREAM_COUNT) || 
                (gCommonData.bitstreamLength < MINIMUM_BITSTREAM_LENGTH) ||
                ((gCommonData.bitstreamLength % 8) != 0) ||
                (gCommonData.significanceLevel <= MINIMUM_SIGNIFICANCE_LEVEL) ||
                (gCommonData.significanceLevel >= MAXIMUM_SIGNIFICANCE_LEVEL) ||
                (gCommonData.blockLength < MINIMUM_BLOCK_LENGTH) ||
                (gCommonData.blockLength > MAXIMUM_BLOCK_LENGTH) ||
                (gCommonData.numberOfIndependentBlocks > MAXIMUM_NUMBER_OF_INDEPENDENT_BLOCKS) ||
                (gCommonData.substringLength < (0.01 * gCommonData.bitstreamLength)))
            {
                result = STEER_CHECK_ERROR(EINVAL);
            }
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Calculate the number of tests required 
            // to derive statistically meaningful results
            result = STEER_GetMinimumTestCount(gCommonData.significanceLevel,
                                               gCommonData.bitstreamCount,
                                               &(gCommonData.minimumTestCountRequiredForSignificance),
                                               &(gCommonData.predictedPassedTestCount),
                                               &(gCommonData.predictedFailedTestCount));
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // More setup
            gCommonData.theoreticalMean = 
                (gCommonData.substringLength - gCommonData.blockLength + 1) / 
                pow(2, gCommonData.blockLength);
            result = STEER_CHECK_CONDITION((!isNegative(gCommonData.theoreticalMean) && !isZero(gCommonData.theoreticalMean)), 
                STEER_RESULT_OUT_OF_RANGE);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // More setup
            gCommonData.varWj = gCommonData.substringLength * 
                (1.0/pow(2.0, gCommonData.blockLength) - ((2.0 * gCommonData.blockLength) - 1.0) /
                pow(2.0, 2.0 * gCommonData.blockLength));

            privData->templateDataPtr = GetTemplatePtr(gCommonData.blockLength);
            gCommonData.numConfigurations = kNonPeriodicTemplateCount[gCommonData.blockLength];

            // Compute probabilities
            gCommonData.sum = 0.0;
            memset((void*)gCommonData.pi, 0, sizeof(double) * 6);

            for (i = 0; i < 2; i++)
            {
                gCommonData.pi[i] = 
                    exp(-(gCommonData.theoreticalMean) + 
                        (i * log(gCommonData.theoreticalMean)) - cephes_lgam((double)(i + 1)));
                gCommonData.sum += gCommonData.pi[i];
            }
            gCommonData.pi[0] = gCommonData.sum;
            for (i = 2; i < gCommonData.degreesOfFreedom; i++)
            {
                gCommonData.pi[i - 1] = 
                    exp(-(gCommonData.theoreticalMean) + 
                        (i * log(gCommonData.theoreticalMean)) - cephes_lgam(i + 1));
                gCommonData.sum += gCommonData.pi[i + 1];
            }
            gCommonData.pi[gCommonData.degreesOfFreedom] = 1 - gCommonData.sum;
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Reallocate private data to account for correct number of configurations
            result = STEER_ReallocateMemory(sizeof(tNIST_NonOverlappingTemplateMatchingPrivateData),
                                            sizeof(tNIST_NonOverlappingTemplateMatchingPrivateData) +
                                            (gCommonData.numConfigurations * sizeof(tSTEER_ConfigurationState)),
                                            (void**)&privData);
        }

        if (result == STEER_RESULT_SUCCESS)
        {
            // Allocate space for template
            result = STEER_AllocateMemory((gCommonData.blockLength + 1) * sizeof(char),
                                          (void**)&(privData->template));
        }
   
        if (result == STEER_RESULT_SUCCESS)
        {
            // Return private data and buffer size
            *testPrivateData = (void*)privData;
            *bufferSizeInBytes = gCommonData.bitstreamLength / 8;
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
    return gCommonData.numConfigurations;
}

// =================================================================================================
//  SetReport
// =================================================================================================
int32_t SetReport (void* testPrivateData,
                   tSTEER_ReportPtr report)
{
    int32_t result = STEER_RESULT_SUCCESS;
    tNIST_NonOverlappingTemplateMatchingPrivateData* privData = NULL;
    uint32_t bit = 0;
    char* templateDataPtr = GetTemplatePtr(gCommonData.blockLength);
    uint_fast32_t i = 0;
    uint_fast32_t j = 0;
    
    privData = (tNIST_NonOverlappingTemplateMatchingPrivateData*) testPrivateData;
    privData->report = report;

    for (i = 0; i < gCommonData.numConfigurations; i++)
    {
        // Set up configuration attributes
        for (j = 0; j < gCommonData.blockLength; j++)
        {
            // Read a bit from the template file
            sscanf(templateDataPtr, 
                    STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, &bit);
            if (bit == 0)
                privData->template[j] = 0x30;
            else if (bit == 1)
                privData->template[j] = 0x31;
            templateDataPtr += 2;
        }

        privData->configurationState[i].configurationId = i;

        // Add attribute
        if (!STEER_ConfigurationHasAttribute(privData->report, i, STEER_JSON_TAG_TEMPLATE))
            result = STEER_AddAttributeToConfiguration(privData->report, i,
                                                       STEER_JSON_TAG_TEMPLATE,
                                                       STEER_JSON_VALUE_UTF_8_STRING,
                                                       NULL, NULL, privData->template);

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
    tNIST_NonOverlappingTemplateMatchingPrivateData* privData = (tNIST_NonOverlappingTemplateMatchingPrivateData*)testPrivateData;
    bool passed = false;
    char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
    char progressStr[STEER_STRING_MAX_LENGTH] = { 0 };
    tSTEER_ValueSet* valueSet = NULL;
    uint64_t testId = 0;
    char* end = NULL;

    testId = strtoull(bitstreamId, &end, 0) - 1;

    result = STEER_AllocateMemory(gCommonData.numberOfIndependentBlocks * sizeof(uint32_t),
                                  (void**)&(privData->templateFrequencies));
    if (result == STEER_RESULT_SUCCESS)
    {
        result = STEER_AllocateMemory(gCommonData.blockLength * sizeof(uint8_t),
                                      (void**)&(privData->sequence));
    }

    if (result == STEER_RESULT_SUCCESS)
    {  
        uint_fast32_t i = 0;
        uint_fast32_t j = 0;

        for (i = 0; i < gCommonData.numConfigurations; i++)
        {
            privData->ones = numOnes;
            privData->zeros = numZeros;
            privData->configurationState[i].accumulatedOnes += numOnes;
            privData->configurationState[i].accumulatedZeros += numZeros;

            // Run test
            if (gCommonData.cliArguments->reportProgress)
            {
                memset((void*)progressStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(progressStr, "Testing with template %s...", privData->template);
                STEER_REPORT_PROGRESS(gCommonData.cliArguments->programName, progressStr);
            }
            result = RunTest(privData, buffer, &passed);

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
                // Template frequencies
                result = STEER_NewValueSet(STEER_JSON_TAG_TEMPLATE_FREQUENCIES,
                                            STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                            NULL, NULL, &valueSet);
                if (result == STEER_RESULT_SUCCESS)
                {
                    char tagStr[STEER_STRING_MAX_LENGTH] = { 0 };

                    for (j = 0; j < gCommonData.numberOfIndependentBlocks; j++)
                    {
                        memset((void*)tagStr, 0, STEER_STRING_MAX_LENGTH);
                        sprintf(tagStr, "%s %u", STEER_JSON_TAG_BLOCK, (uint32_t)(j + 1));
                        memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                        sprintf(calculationStr, STEER_DEFAULT_UNSIGNED_INTEGER_STRING_FORMAT, 
                                privData->templateFrequencies[j]);
                        result = STEER_AddValueToValueSet(tagStr, calculationStr, &valueSet);
                        if (result != STEER_RESULT_SUCCESS)
                            break;
                    }
                }

                if (result == STEER_RESULT_SUCCESS)
                    result = STEER_AddCalculationSetToTest(privData->report, i, testId, valueSet);
                
                (void)STEER_FreeValueSet(&valueSet);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Theoretical mean
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
                        STEER_DEFAULT_FLOATING_POINT_PRECISION, gCommonData.theoreticalMean);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_THEORETICAL_MEAN,
                                                    STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                                                    STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                                                    NULL, calculationStr);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Substring length
                memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
                sprintf(calculationStr, STEER_DEFAULT_SIGNED_INTEGER_STRING_FORMAT, 
                        gCommonData.substringLength);
                result = STEER_AddCalculationToTest(privData->report, i, testId,
                                                    STEER_JSON_TAG_SUBSTRING_LENGTH,
                                                    STEER_JSON_VALUE_SIGNED_32_BIT_INTEGER,
                                                    NULL, STEER_JSON_VALUE_BITS, calculationStr);
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
                        gCommonData.significanceLevelPrecision,
                        gCommonData.significanceLevel);
                result = STEER_AddCriterionToTest(privData->report, i, testId, criterionStr,
                                                    (privData->probabilityValue >= gCommonData.significanceLevel) ? true : false);
            }

            // Add evaluation to current test
            if (result == STEER_RESULT_SUCCESS)
            {
                bool passed = false;
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

            #if STEER_ENABLE_CONSOLE_LOGGING
                if (i == 0)
                    fprintf(stdout, "\t\t%d template tested with non overlapping template matching.\n", i + 1);
                else
                    fprintf(stdout, "\t\t%d templates tested with non overlapping template matching.\n", i + 1);
            #endif
        }
    }

    // Clean up
    STEER_FreeMemory((void**)&(privData->sequence));
    STEER_FreeMemory((void**)&(privData->templateFrequencies));

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
    uint_fast32_t i = 0;

    tNIST_NonOverlappingTemplateMatchingPrivateData* privData = 
        (tNIST_NonOverlappingTemplateMatchingPrivateData*)(*testPrivateData);

    if (privData != NULL)
    {
        double probabilityValueUniformity = 0.0;
        uint64_t proportionThresholdMinimum = 0;
        uint64_t proportionThresholdMaximum = 0;

        for (i = 0; i < gCommonData.numConfigurations; i++)
        {
            // Add required metrics to configuration
            result = STEER_NistStsAddRequiredMetricsToConfiguration(privData->report, i,
                                                                    suppliedNumberOfBitstreams,
                                                                    gCommonData.minimumTestCountRequiredForSignificance,
                                                                    privData->configurationState[i].testsPassed,
                                                                    gCommonData.predictedPassedTestCount,
                                                                    privData->configurationState[i].accumulatedOnes,
                                                                    privData->configurationState[i].accumulatedZeros);

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add test specific metrics
                result = STEER_NistStsAddMetricsToConfiguration(privData->report, i, false,
                                                                suppliedNumberOfBitstreams,
                                                                gCommonData.significanceLevel,
                                                                &probabilityValueUniformity,
                                                                &proportionThresholdMinimum,
                                                                &proportionThresholdMaximum);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add optional confusion matrix metrics to configuration
                result = STEER_AddConfusionMatrixMetricsToConfiguration(privData->report, i,
                                                                        gCommonData.minimumTestCountRequiredForSignificance,
                                                                        privData->configurationState[i].testsRun,
                                                                        privData->configurationState[i].testsPassed,
                                                                        privData->configurationState[i].testsFailed,
                                                                        gCommonData.predictedPassedTestCount,
                                                                        gCommonData.predictedFailedTestCount);
            }

            if (result == STEER_RESULT_SUCCESS)
            {
                // Add required criteria to configuration
                result = STEER_NistStsAddRequiredCriterionToConfiguration(privData->report, i,
                                                                          suppliedNumberOfBitstreams,
                                                                          privData->configurationState[i].testsPassed,
                                                                          gCommonData.significanceLevel,
                                                                          gCommonData.significanceLevelPrecision,
                                                                          gCommonData.minimumTestCountRequiredForSignificance);
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
        }

        // Clean up
        STEER_FreeMemory((void**)&(privData->template));
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
