// =================================================================================================
//! @file non_overlapping_template_matching.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the NIST STS non overlapping template matchings test for the 
//! STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-21
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_test_shell.h"

// =================================================================================================
//  Private constants
// =================================================================================================

#define PROGRAM_NAME        "nist_sts_non_overlapping_template_matching_test"
#define PROGRAM_VERSION     "0.1.0"
#define TEST_NAME           "non overlapping template matching"
#define TEST_DESCRIPTION \
"The focus of this test is the number of occurrences of pre-specified target strings. The \
purpose of this test is to detect generators that produce too many occurrences of a given \
non-periodic (aperiodic) pattern. For this test, an m-bit window is used to search for a \
specific m-bit pattern. If the pattern is not found, the window slides one bit position. \
If the pattern is found, the window is reset to the bit after the found pattern, and the \
search resumes."

#define MINIMUM_BITSTREAM_COUNT                 1
#define MINIMUM_BITSTREAM_LENGTH                1000000 
#define MINIMUM_SIGNIFICANCE_LEVEL              0.0
#define MAXIMUM_SIGNIFICANCE_LEVEL              1.0
#define MINIMUM_BLOCK_LENGTH                    2
#define MAXIMUM_BLOCK_LENGTH                    21
#define MAXIMUM_NUMBER_OF_INDEPENDENT_BLOCKS    100

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_nonoverlappingtemplatematchingcommon
{
    tSTEER_CliArguments*    cliArguments;
    uint64_t                bitstreamCount;
    uint64_t                bitstreamLength;
    double                  significanceLevel;
    uint32_t                significanceLevelPrecision;
    int32_t                 blockLength;                // TODO: should make this uint32_t
    int32_t                 degreesOfFreedom;           // K
    int32_t                 numberOfIndependentBlocks;  // N
    int32_t                 substringLength;            // M
    double                  theoreticalMean;            // lambda
    double                  varWj;
    uint32_t                numConfigurations;
    double                  sum;
    double                  pi[6];
    uint64_t                minimumTestCountRequiredForSignificance;
    uint64_t                predictedPassedTestCount;
    uint64_t                predictedFailedTestCount;
}
tNIST_NonOverlappingTemplateMatchingCommon;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_InfoList gReferences = {
    9,
    { 
        "NIST Special Publication 800-22 Rev. 1a, Section 1.1.5",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.7",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.7.7",
        "NIST Special Publication 800-22 Rev. 1a, Section 3.7",
        "NIST Special Publication 800-22 Rev. 1a, Section 4.3(f)",
        "NIST Special Publication 800-22 Rev. 1a, Section 5.5.2",
        "NIST Special Publication 800-22 Rev. 1a, Appendix A.1",
        "NIST Special Publication 800-22 Rev. 1a, Appendix A.2",
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

static tNIST_NonOverlappingTemplateMatchingCommon gCommonData;

// =================================================================================================
//  Test source
// =================================================================================================

#if STEER_BUILD_MULTI_THREADED_TESTS
    #include "non_overlapping_template_matching_mt.c"
#else
    #include "non_overlapping_template_matching_st.c"
#endif

// =================================================================================================
