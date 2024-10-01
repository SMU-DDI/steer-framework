// =================================================================================================
//! @file discrete_fourier_transform.c
//! @author Gary Woodcock (gary@anametric.com)
//! @brief This file implements the NIST STS discrete fourier transform test for the STEER 
//! framework.
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

#define PROGRAM_NAME        "nist_sts_discrete_fourier_transform_test"
#define PROGRAM_VERSION     "0.1.0"
#define TEST_NAME           "discrete fourier transform"
#define TEST_DESCRIPTION \
"The focus of this test is the peak heights in the Discrete Fourier Transform of the sequence. \
The purpose of this test is to detect periodic features (i.e., repetitive patterns that are \
near each other) in the tested sequence that would indicate a deviation from the assumption \
of randomness. The intention is to detect whether the number of peaks exceeding the 95% \
threshold is significantly different than 5%."
#define CONFIGURATION_COUNT         1

#define MINIMUM_BITSTREAM_COUNT     1
#define MINIMUM_BITSTREAM_LENGTH    1000 
#define MINIMUM_SIGNIFICANCE_LEVEL  0.0
#define MAXIMUM_SIGNIFICANCE_LEVEL  1.0

// =================================================================================================
//  Private types
// =================================================================================================

typedef struct tnist_discretefouriertransformcommon
{
    tSTEER_CliArguments*    cliArguments;
    uint64_t                bitstreamCount;
    uint64_t                bitstreamLength;
    double                  significanceLevel;
    uint32_t                significanceLevelPrecision;
    uint64_t                minimumTestCountRequiredForSignificance;
    uint64_t                predictedPassedTestCount;
    uint64_t                predictedFailedTestCount;
}
tNIST_DiscreteFourierTransformCommon;

// =================================================================================================
//  Private globals
// =================================================================================================

static tSTEER_InfoList gReferences = {
    6,
    { 
        "NIST Special Publication 800-22 Rev. 1a, Section 1.1.5",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.6",
        "NIST Special Publication 800-22 Rev. 1a, Section 2.6.7",
        "NIST Special Publication 800-22 Rev. 1a, Section 3.6",
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

static tNIST_DiscreteFourierTransformCommon gCommonData;

// =================================================================================================
//  Test source
// =================================================================================================

#if STEER_BUILD_MULTI_THREADED_TESTS
    #include "discrete_fourier_transform_mt.c"
#else
    #include "discrete_fourier_transform_st.c"
#endif

// =================================================================================================
