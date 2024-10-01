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
#include "steer_test_shell.h"

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

typedef struct tnist_randomexcursionscommon
{
    tSTEER_CliArguments*    cliArguments;
    uint64_t                bitstreamCount;
    uint64_t                bitstreamLength;
    double                  significanceLevel;
    uint32_t                significanceLevelPrecision;
    uint64_t                minimumTestCountRequiredForSignificance;
    uint64_t                predictedPassedTestCount;
    uint64_t                predictedFailedTestCount;
    int32_t                 numberOfCycles;
    int32_t                 maxNumberOfCycles;
    double                  rejectionConstraint;
    int32_t                 stateX[CONFIGURATION_COUNT];
    double                  pi[5][6];
    double                  nu[6][CONFIGURATION_COUNT];
    int32_t*                S_k;
    int32_t*                cycle;
}
tNIST_RandomExcursionsCommon;

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
    3,
    { 
        NIST_STS_AUTHOR_JUAN_SOTO,
        NIST_STS_AUTHOR_LARRY_BASSHAM,
        STEER_AUTHOR_GARY_WOODCOCK
    }
};

static tSTEER_InfoList gContributors = {
    3,
    {
        STEER_CONTRIBUTOR_NATHAN_CONRAD,
        STEER_CONTRIBUTOR_MICAH_THORNTON,
        STEER_CONTRIBUTOR_MITCH_THORNTON
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

static tNIST_RandomExcursionsCommon gCommonData = {
    NULL, 
    0,
    0,
    0.0,
    0,
    0,
    0,
    0,
    0,
    0,
    0.0,
    { -4, -3, -2, -1, 1, 2, 3, 4 },
    { {0.0000000000, 0.00000000000, 0.00000000000, 0.00000000000, 0.00000000000, 0.0000000000}, 
      {0.5000000000, 0.25000000000, 0.12500000000, 0.06250000000, 0.03125000000, 0.0312500000},
      {0.7500000000, 0.06250000000, 0.04687500000, 0.03515625000, 0.02636718750, 0.0791015625},
      {0.8333333333, 0.02777777778, 0.02314814815, 0.01929012346, 0.01607510288, 0.0803755143},
      {0.8750000000, 0.01562500000, 0.01367187500, 0.01196289063, 0.01046752930, 0.0732727051} 
    },
    { {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    },
    NULL,
    NULL
};

// =================================================================================================
//  Test source
// =================================================================================================

#if STEER_BUILD_MULTI_THREADED_TESTS
    #include "random_excursions_mt.c"
#else
    #include "random_excursions_st.c"
#endif

// =================================================================================================
