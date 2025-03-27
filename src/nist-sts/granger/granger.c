// =================================================================================================
//! @file granger.c
//! @author %author% (%email%)
//! @brief This file implements the NIST STS granger test for the STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-19
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_test_shell.h"
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
#include <math.h>
#include <stdio.h>

// =================================================================================================
//  Private constants
// =================================================================================================


#define PROGRAM_NAME        "nist_sts_granger_test"
#define PROGRAM_VERSION     "0.1.0"
#define TEST_NAME           "granger"
#define TEST_DESCRIPTION \
"A default test description that has yet to completed"
#define CONFIGURATION_COUNT         1

#define MINIMUM_BITSTREAM_COUNT     1
#define MINIMUM_BITSTREAM_LENGTH    1024 
#define MINIMUM_SIGNIFICANCE_LEVEL  0.0
#define MAXIMUM_SIGNIFICANCE_LEVEL  1.0

#ifndef GRANGER_PY_LOCATION
#   define GRANGER_PY_LOCATION         "./granger.py"
#endif
// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_grangercommon
{
    tSTEER_CliArguments*    cliArguments;
    uint64_t                bitstreamCount;
    uint64_t                bitstreamLength;
    double                  significanceLevel;
    uint32_t                significanceLevelPrecision; 
    uint32_t                bitsPerWindow;         
    uint32_t                windowOffset;         
    double                  log2pt0;
    uint64_t                minimumTestCountRequiredForSignificance;
    uint64_t                predictedPassedTestCount;
    uint64_t                predictedFailedTestCount;
}
tNIST_GrangerCommon;

typedef struct tnist_grangerprivatedata
{
    tSTEER_ReportPtr            report;
    tSTEER_ConfigurationState   configurationState[CONFIGURATION_COUNT];
    double                      granger;
    double                      chiSquared;
    double                      probabilityValue;
    uint32_t                    recommendedWindowSize;
    uint32_t                    recommendedWindowOffset;
    uint64_t                    ones;
    uint64_t                    zeros;
    const char*                 id;
}
tNIST_GrangerPrivateData;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_InfoList gReferences = {
    5,
    { 
        "NIST Special Publication 800-22 Rev. 1a, Section 1.1.5",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.12",
        "NIST Special Publication 800-22 Rev. 1a, Section 3.12",
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
        STEER_CONTRIBUTOR_ALEX_MAGYARI        
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
    eSTEER_Complexity_Moderate,
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

static tNIST_GrangerCommon gCommonData;
static tSTEER_ParameterInfoList gParameterInfoList = {
    5,
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
            "1024",
            NULL
        },

        // Test specific parameter
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
            STEER_JSON_TAG_WINDOW_OFFSET,
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITS,
            "250",
            0,
            NULL
        },

        // Test specific parameter
        {
            STEER_JSON_TAG_BITS_PER_WINDOW,
            STEER_JSON_VALUE_UNSIGNED_32_BIT_INTEGER,
            NULL,
            STEER_JSON_VALUE_BITS,
            "1000",
            0,
            NULL
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
int32_t RunTest (tNIST_GrangerPrivateData* privateData,
    uint8_t* bitstreamBuffer,
    bool* passed)
{
int32_t result = STEER_RESULT_SUCCESS;
int_fast32_t i = 0;
int_fast32_t j = 0;
int32_t k = 0;
int32_t r = 0;
int32_t blockSize = 0;
int32_t seqLength = 0;
int32_t powLen = 0;
int32_t index = 0;
double sum= 0.0;
double numOfBlocks = 0.0;
double entropyDistribution[2] = {0, 0};  // ApEn
uint32_t* P = NULL;

privateData->chiSquared = 0.0;
privateData->granger = 0.0;
privateData->probabilityValue = 0.0;
privateData->recommendedWindowSize = 20;
privateData->recommendedWindowOffset = 10;
privateData->ones = 0;
privateData->zeros = 0;
*passed = false;
seqLength = gCommonData.bitstreamLength;
r = 0;

FILE *fp;

const char * flags[] = {"-l", "-n", "-o"};
char filePath[20];
sprintf(filePath, "./%08d.bin", getpid()) ;
char * pythonBuf;
char ** args;

result = STEER_AllocateMemory(6*sizeof(char*), &args);
if (result == STEER_RESULT_SUCCESS) {
for (i = 0; i < 6; i ++)
result = STEER_AllocateMemory(sizeof(char *), &args[i]);
}
if (result == STEER_RESULT_SUCCESS)
result = STEER_OpenFile(filePath, true, true, &fp);
if (result == STEER_RESULT_SUCCESS)
{
size_t numWrites = fwrite(bitstreamBuffer, sizeof(uint8_t), seqLength, fp);

result = STEER_CloseFile(&fp);
result = STEER_CHECK_CONDITION(numWrites == (seqLength), STEER_RESULT_NOT_ENOUGH_BYTES_WRITTEN);
if (result == STEER_RESULT_SUCCESS)
{
char *intString;
int intStringLen;
if (result == STEER_RESULT_SUCCESS)
{
   for (i = 0; i < 3; i ++)
   {
       result = STEER_DuplicateString(flags[i], &args[2*i]);
       if (i == 0)
           result = STEER_DuplicateString(filePath, &args[1]);
       if (i == 1){
           intStringLen = (int)((ceil(log10(gCommonData.bitsPerWindow))+1)*sizeof(char));
           STEER_AllocateMemory(intStringLen, &intString);
           sprintf(intString, "%d", gCommonData.bitsPerWindow);
           result = STEER_DuplicateString(intString, &args[3]);
           STEER_FreeMemory((void **) &intString);
       }
       if (i == 2){
           intStringLen = (int)((ceil(log10(gCommonData.windowOffset))+1)*sizeof(char));
           STEER_AllocateMemory(intStringLen, &intString);
           sprintf(intString, "%d", gCommonData.windowOffset);
           result = STEER_DuplicateString(intString,  &args[5]);
           STEER_FreeMemory((void **) &intString);
       }
       if (result != STEER_RESULT_SUCCESS)
           break;
   }
}
}
}

if (result == STEER_RESULT_SUCCESS)
STEER_RunPython(GRANGER_PY_LOCATION, args, 6, &pythonBuf);

remove(filePath);
for (i = 0; i < 6; i ++)
STEER_FreeMemory((void **) &args[i]);
STEER_FreeMemory((void **)  &args);

if (result == STEER_RESULT_SUCCESS)
{
int numResults = 0;
char ** subStrings = STEER_ExplodeString(pythonBuf, " ", &numResults, &result);
result = STEER_CHECK_CONDITION(numResults == 6, STEER_RESULT_OUT_OF_RANGE);
if (result == STEER_RESULT_SUCCESS)
result = STEER_CHECK_CONDITION(strcmp(subStrings[0], "success") == 0, STEER_RESULT_FAILURE);
STEER_FreeMemory((void **) &pythonBuf);

// Check status
if (result == STEER_RESULT_SUCCESS)
{
result = STEER_ConvertStringToDoublePrecisionFloatingPoint (subStrings[1], &(privateData->granger));

if (result == STEER_RESULT_SUCCESS)
   result = STEER_ConvertStringToDoublePrecisionFloatingPoint (subStrings[2], &(privateData->chiSquared));
if (result == STEER_RESULT_SUCCESS)
   result = STEER_ConvertStringToDoublePrecisionFloatingPoint (subStrings[3], &(privateData->probabilityValue));
if (result == STEER_RESULT_SUCCESS)
   result = STEER_ConvertStringToUnsigned64BitInteger (subStrings[4], &(privateData->ones));
if (result == STEER_RESULT_SUCCESS)
   result = STEER_ConvertStringToUnsigned64BitInteger (subStrings[5], &(privateData->zeros));
printf("G: %f X2: %f p: %f: 1s: %d 0s: %d\n", (float)(privateData->granger), (float)(privateData->chiSquared), (float)(privateData->probabilityValue),(privateData->ones), (privateData->zeros));
if (result == STEER_RESULT_SUCCESS)
   *passed = (privateData->probabilityValue >= gCommonData.significanceLevel);
}
for (int i = 0; i < numResults; i ++)
STEER_FreeMemory((void **) &subStrings[i]);
STEER_FreeMemory((void **) &subStrings);
}
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
tNIST_GrangerPrivateData* privData = NULL;

// Setup
*testPrivateData = NULL;
*bufferSizeInBytes = 0;
memset((void*)&gCommonData, 0, sizeof(tNIST_GrangerCommon));
gCommonData.cliArguments = cliArguments;
gCommonData.log2pt0 = log(2.0);

// Allocate private data
result = STEER_AllocateMemory(sizeof(tNIST_GrangerPrivateData), (void**)&privData);

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

else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_WINDOW_OFFSET) == 0)
{
   result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                 parameters->parameter[i].value,
                                 &nativeValue);
   if (result == STEER_RESULT_SUCCESS)
   {
       gCommonData.windowOffset = *((int32_t*)nativeValue);
       STEER_FreeMemory((void**)&nativeValue);
   }
}

else if (strcmp(parameters->parameter[i].name, STEER_JSON_TAG_BITS_PER_WINDOW) == 0)
{
   result = STEER_GetNativeValue(parameters->parameter[i].dataType,
                                 parameters->parameter[i].value,
                                 &nativeValue);
   if (result == STEER_RESULT_SUCCESS)
   {
       gCommonData.bitsPerWindow = *((int32_t*)nativeValue);
       STEER_FreeMemory((void**)&nativeValue);
   }
}

if (result != STEER_RESULT_SUCCESS)
   break;
}

if (result == STEER_RESULT_SUCCESS)
{
// Make sure we got the required parameters and that they're good
if ((gCommonData.bitstreamCount < MINIMUM_BITSTREAM_COUNT) || 
   (gCommonData.bitstreamLength < MINIMUM_BITSTREAM_LENGTH) ||
   ((gCommonData.bitstreamLength % 8) != 0) ||
   (gCommonData.significanceLevel <= MINIMUM_SIGNIFICANCE_LEVEL) ||
   (gCommonData.significanceLevel >= MAXIMUM_SIGNIFICANCE_LEVEL))
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

// Set the configuration ID
privData->configurationState[0].configurationId = 1;

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
return CONFIGURATION_COUNT;
}

// =================================================================================================
//  SetReport
// =================================================================================================
int32_t SetReport (void* testPrivateData,
      tSTEER_ReportPtr report)
{
((tNIST_GrangerPrivateData*)testPrivateData)->report = report;
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
tNIST_GrangerPrivateData* privData = (tNIST_GrangerPrivateData*)testPrivateData;
bool passed = false;
char calculationStr[STEER_STRING_MAX_LENGTH] = { 0 };
char criterionStr[STEER_STRING_MAX_LENGTH] = { 0 };
tSTEER_ValueSet* valueSet = NULL;
uint64_t testId = 0;
char* end = NULL;

// Set test ID
testId = strtoull(bitstreamId, &end, 0) - 1;

// Setup
privData->id = bitstreamId;
privData->chiSquared = 0;
privData->probabilityValue = 0;

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
// Add granger
memset((void*)calculationStr, 0, STEER_STRING_MAX_LENGTH);
sprintf(calculationStr, STEER_DEFAULT_FLOATING_POINT_STRING_FORMAT, 
   STEER_DEFAULT_FLOATING_POINT_PRECISION, privData->granger);
result = STEER_AddCalculationToTest(privData->report, 0, testId,
                               STEER_JSON_TAG_APPROXIMATE_ENTROPY,
                               STEER_JSON_VALUE_DOUBLE_PRECISION_FLOATING_POINT,
                               STEER_JSON_VALUE_DEFAULT_FLOATING_POINT_PRECISION, 
                               NULL, calculationStr);
}

if (result == STEER_RESULT_SUCCESS)
{
// Add chi squared
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
// Recommended window
memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
sprintf(criterionStr, "%s of %d %s >= recommended %s of %d %s",
   STEER_JSON_TAG_BITS_PER_WINDOW,
   gCommonData.bitsPerWindow,
   STEER_JSON_VALUE_BITS,
   STEER_JSON_TAG_BITS_PER_WINDOW,
   privData->recommendedWindowSize, 
   STEER_JSON_VALUE_BITS);
result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                             (gCommonData.bitsPerWindow >= privData->recommendedWindowSize) ? true : false);
}

if (result == STEER_RESULT_SUCCESS)
{
// Recommended window
memset((void*)criterionStr, 0, STEER_STRING_MAX_LENGTH);
sprintf(criterionStr, "%s of %d %s >= recommended %s of %d %s",
STEER_JSON_TAG_WINDOW_OFFSET,
   gCommonData.windowOffset,
   STEER_JSON_VALUE_BITS,
   STEER_JSON_TAG_WINDOW_OFFSET,
   privData->recommendedWindowOffset,
   STEER_JSON_VALUE_BITS);
result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                             (gCommonData.windowOffset >= privData->recommendedWindowOffset) ? true : false);
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
   gCommonData.significanceLevelPrecision,
   gCommonData.significanceLevel);
result = STEER_AddCriterionToTest(privData->report, 0, testId, criterionStr,
                             (privData->probabilityValue >= gCommonData.significanceLevel) ? true : false);
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
tNIST_GrangerPrivateData* privData = (tNIST_GrangerPrivateData*)(*testPrivateData);

if (privData != NULL)
{
double probabilityValueUniformity = 0.0;
uint64_t proportionThresholdMinimum = 0;
uint64_t proportionThresholdMaximum = 0;

// Add required NIST metrics to configuration
result = STEER_NistStsAddRequiredMetricsToConfiguration(privData->report, 0,
                                                   suppliedNumberOfBitstreams,
                                                   gCommonData.minimumTestCountRequiredForSignificance,
                                                   privData->configurationState[0].testsPassed,
                                                   gCommonData.predictedPassedTestCount,
                                                   privData->configurationState[0].accumulatedOnes,
                                                   privData->configurationState[0].accumulatedZeros);

if (result == STEER_RESULT_SUCCESS)
{
// Add test specific metrics
result = STEER_NistStsAddMetricsToConfiguration(privData->report, 0, false,
                                               suppliedNumberOfBitstreams,
                                               gCommonData.significanceLevel,
                                               &probabilityValueUniformity,
                                               &proportionThresholdMinimum,
                                               &proportionThresholdMaximum);
}

if (result == STEER_RESULT_SUCCESS)
{
// Add optional confusion matrix metrics to configuration
result = STEER_AddConfusionMatrixMetricsToConfiguration(privData->report, 0,
                                                       gCommonData.minimumTestCountRequiredForSignificance,
                                                       privData->configurationState[0].testsRun,
                                                       privData->configurationState[0].testsPassed,
                                                       privData->configurationState[0].testsFailed,
                                                       gCommonData.predictedPassedTestCount,
                                                       gCommonData.predictedFailedTestCount);
}

if (result == STEER_RESULT_SUCCESS)
{
// Add required criteria to configuration
result = STEER_NistStsAddRequiredCriterionToConfiguration(privData->report, 0,
                                                         suppliedNumberOfBitstreams,
                                                         privData->configurationState[0].testsPassed,
                                                         gCommonData.significanceLevel,
                                                         gCommonData.significanceLevelPrecision,
                                                         gCommonData.minimumTestCountRequiredForSignificance);
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
if (result != STEER_RESULT_SUCCESS)
printf("Granger failed with error code %d", result);
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

