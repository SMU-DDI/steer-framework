// =================================================================================================
//! @file linear_complexity.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the NIST STS linear complexity test for the STEER framework.
//! @remarks Requires ANSI C99 (or better) compliant compilers.
//! @remarks Supported host operating systems: Any *nix
//! @date 2021-02-20
//! @copyright Copyright (c) 2024 Anametric, Inc. All rights reserved.
//! 
//  Includes
// =================================================================================================
#include "steer.h"
#include "steer_test_shell.h"

// =================================================================================================
//  Private constants
// =================================================================================================

#define PROGRAM_NAME        "nist_sts_linear_complexity_test"
#define PROGRAM_VERSION     "0.1.0"
#define TEST_NAME           "linear complexity"
#define TEST_DESCRIPTION \
"The focus of this test is the length of a linear feedback shift register (LFSR). The purpose \
of this test is to determine whether or not the sequence is complex enough to be considered \
random. Random sequences are characterized by longer LFSRs. An LFSR that is too short implies \
non-randomness."
#define CONFIGURATION_COUNT         1

#define MINIMUM_BITSTREAM_COUNT     1
#define MINIMUM_BITSTREAM_LENGTH    1000000 
#define MINIMUM_SIGNIFICANCE_LEVEL  0.0
#define MAXIMUM_SIGNIFICANCE_LEVEL  1.0
#define MINIMUM_BLOCK_LENGTH        500
#define MAXIMUM_BLOCK_LENGTH        5000

static const double kPreComputedProbabilities[7] = { 
    0.01047, 0.03125, 0.12500, 0.50000, 0.25000, 0.06250, 0.020833 };    // pi

static const int32_t kDegreesOfFreedom = 6;   // K

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_linearcomplexitycommon
{
    tSTEER_CliArguments*    cliArguments;
    uint64_t                bitstreamCount;
    uint64_t                bitstreamLength;
    double                  significanceLevel;
    uint32_t                significanceLevelPrecision;
    int32_t                 blockLength;                // TODO: should make this uint32_t
    uint64_t                minimumTestCountRequiredForSignificance;
    uint64_t                predictedPassedTestCount;
    uint64_t                predictedFailedTestCount;
}
tNIST_LinearComplexityCommon;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_InfoList gReferences = {
    6,
    { 
        "NIST Special Publication 800-22 Rev. 1a, Section 1.1.5",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.10",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.10.7",
        "NIST Special Publication 800-22 Rev. 1a, Section 3.10",
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

static tNIST_LinearComplexityCommon gCommonData;

// =================================================================================================
//  Test source
// =================================================================================================

#if STEER_BUILD_MULTI_THREADED_TESTS
    #include "linear_complexity_mt.c"
#else
    #include "linear_complexity_st.c"
#endif

// =================================================================================================
