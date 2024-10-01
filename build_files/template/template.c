// =================================================================================================
//! @file __testnameus__.c
//! @author %%author%% (%%email%%)
//! @brief This file implements the NIST STS __testname__ test for the STEER framework.
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

#define PROGRAM_NAME        "nist_sts___testnameus___test"
#define PROGRAM_VERSION     "0.1.0"
#define TEST_NAME           "__testnamefull__"
#define TEST_DESCRIPTION \
"A default test description that has yet to completed"
#define CONFIGURATION_COUNT         1

#define MINIMUM_BITSTREAM_COUNT     1
#define MINIMUM_BITSTREAM_LENGTH    1024 
#define MINIMUM_SIGNIFICANCE_LEVEL  0.0
#define MAXIMUM_SIGNIFICANCE_LEVEL  1.0

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist___testname__common
{
    tSTEER_CliArguments*    cliArguments;
    uint64_t                bitstreamCount;
    uint64_t                bitstreamLength;
    double                  significanceLevel;
    uint32_t                significanceLevelPrecision;
    int32_t                 blockLength;               
    double                  log2pt0;
    uint64_t                minimumTestCountRequiredForSignificance;
    uint64_t                predictedPassedTestCount;
    uint64_t                predictedFailedTestCount;
}
tNIST___testnamecaps__Common;

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
        STEER_CONTRIBUTOR_GARY_WOODCOCK,
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

static tNIST___testnamecaps__Common gCommonData;

// =================================================================================================
//  Test source
// =================================================================================================

#if STEER_BUILD_MULTI_THREADED_TESTS
    #include "__testnameus___mt.c"
#else
    #include "__testnameus___st.c"
#endif

// =================================================================================================
