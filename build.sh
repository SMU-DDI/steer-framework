#!/bin/bash
# =================================================================================================
#
#   build.sh
#
#   Copyright (c) 2024 Anametric, Inc. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This script builds the various components of the STEER framework.
#
# =================================================================================================

# Clear screen
clear

# Set built-ins
# set -o errexit
set -o pipefail
set -o nounset

# Stash original working directory 
STARTING_WORKING_DIR="$PWD"

# Stash start date/time
SECONDS=0
BUILD_START="$(date '+%a %b %d %Y %T %Z')"

# Include bash utilities
cd "./third-party/bash-utilities"
source "./bash_utilities.sh"
cd $STARTING_WORKING_DIR

# Set background color 
setBackgroundColor $NORMAL_BACKCOLOR

# Set foreground color 
setForegroundColor $NORMAL_FORECOLOR

# =================================================================================================
#	Constants
# =================================================================================================

STEER_VERSION="0.1.0"

# Commands
ANALYZE_CMD="--analyze"
CHECK_ENV_CMD="--check-env"
CLEAN_CMD="--clean"
DEBUG_CMD="--debug"
HELP_CMD="--help"
PACKAGE_CMD="--package"
RELEASE_CMD="--release"
VERBOSE_CMD="--verbose"
WITH_CONSOLE_LOGGING_CMD="--with-console-logging"
WITH_DOCUMENTATION_CMD="--with-documentation"
WITH_MULTI_THREADING_CMD="--with-multi-threading"
WITH_NIST_STS_BUILD_CMD="--with-nist-sts-build"
WITH_PROFILING_CMD="--with-profiling"
WITH_SHARED_LIBS_CMD="--with-shared-libs"
WITH_UNIT_TEST_CMD="--with-unit-test"
WITH_VALIDATION_CMD="--with-validation"

# Analyze options
ANALYZE_OPTION_CPPCHECK="cppcheck"
ANALYZE_OPTION_FULL="full"
ANALYZE_OPTION_NONE="none"
ANALYZE_OPTION_SCAN_BUILD="scan-build"
ANALYZE_OPTION_VALGRIND="valgrind"

# Operating environments
OPERATING_ENV_DARWIN="darwin"
OPERATING_ENV_LINUX="linux"
OPERATING_ENV_UNKNOWN="unknown"

# Architectures
ARCHITECTURE_X64="x64"
ARCHITECTURE_ARM64="arm64"
ARCHITECTURE_AARCH64="aarch64"
ARCHITECTURE_ARMHF="armhf"
ARCHITECTURE_UNKNOWN="unknown"

# =================================================================================================
#	Functions
# =================================================================================================

# Print summary
function printSummary () {
	BUILD_DURATION=$SECONDS
	BUILD_STOP="$(date '+%a %b %d %Y %T %Z')"
    INDENT_LEN=29

	printBanner "STEER BUILD SUMMARY"
	printWithRightJustification "Build started at: " $INDENT_LEN
    printIt "$BUILD_START"
	printWithRightJustification "Build finished at: " $INDENT_LEN
    printIt "$BUILD_STOP"
	printDuration $BUILD_DURATION $INDENT_LEN

    printWithRightJustification "Configuration: " $INDENT_LEN
	if [ $BUILD_OPERATING_ENV == $OPERATING_ENV_DARWIN ]
	then
		if [ $BUILD_ARCH == $ARCHITECTURE_X64 ]
		then
            printIt "Darwin $ARCHITECTURE_X64"
        elif [ $BUILD_ARCH == $ARCHITECTURE_ARM64 ]
        then
            printIt "Darwin $ARCHITECTURE_ARM64"
		elif [ $BUILD_ARCH == $ARCHITECTURE_ARMHF ]
		then
			printIt "Darwin $ARCHITECTURE_ARMHF"
		else
			printIt "Darwin $ARCHITECTURE_UNKNOWN"
		fi
	elif [ $BUILD_OPERATING_ENV == $OPERATING_ENV_LINUX ]
		then
		if [ $BUILD_ARCH == $ARCHITECTURE_X64 ]
		then
			printIt "Linux $ARCHITECTURE_X64"
        elif [ $BUILD_ARCH == $ARCHITECTURE_ARM64 ] || [ $BUILD_ARCH == $ARCHITECTURE_AARCH64 ]
        then
            printIt "Linux $ARCHITECTURE_ARM64"
		elif [ $BUILD_ARCH == $ARCHITECTURE_ARMHF ]
		then
			printIt "Linux $ARCHITECTURE_ARMHF"
		else
			printIt "Linux $ARCHITECTURE_UNKNOWN"
		fi
	else
		printIt "Unknown"
	fi

	if [ $BUILD_DEBUG -eq 1 ]
	then
		if [ $BUILD_CLEAN -eq 1 ]
		then
			if [ $BUILD_SHARED_LIB -eq 1 ]
			then
                printWithIndent "Clean debug build with shared libraries\n" $INDENT_LEN
			else
				printWithIndent "Clean debug build with static libraries\n" $INDENT_LEN
			fi
		else
			if [ $BUILD_SHARED_LIB -eq 1 ]
			then
				printWithIndent "Incremental debug build with shared libraries\n" $INDENT_LEN
			else
				printWithIndent "Incremental debug build with static libraries\n" $INDENT_LEN
			fi
		fi
	else
		if [ $BUILD_CLEAN -eq 1 ]
		then
            if [ $BUILD_SHARED_LIB -eq 1 ]
            then
				printWithIndent "Clean release build with shared libraries\n" $INDENT_LEN
            else
				printWithIndent "Clean release build with static libraries\n" $INDENT_LEN
            fi
		else
            if [ $BUILD_SHARED_LIB -eq 1 ]
            then
				printWithIndent "Incremental release build with shared libraries\n" $INDENT_LEN
            else
				printWithIndent "Incremental release build with static libraries\n" $INDENT_LEN
            fi
		fi
	fi

    if [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_CPPCHECK ] || [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_FULL ]
    then
        if hasCppcheck
        then
            printWithIndent "With cppcheck code quality analysis\n" $INDENT_LEN
        else
            printWithIndent "Without cppcheck code quality analysis\n" $INDENT_LEN
        fi
    else
        printWithIndent "Without cppcheck code quality analysis\n" $INDENT_LEN
    fi

    if [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_SCAN_BUILD ] || [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_FULL ]
    then
        if hasScanBuild
        then
            printWithIndent "With scan-build code quality analysis\n" $INDENT_LEN
        else
            printWithIndent "Without scan-build code quality analysis\n" $INDENT_LEN
        fi
    else
        printWithIndent "Without scan-build code quality analysis\n" $INDENT_LEN
    fi

    if [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_VALGRIND ] || [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_FULL ]
    then
        if hasValgrind
        then
            printWithIndent "With valgrind memory usage analysis\n" $INDENT_LEN
        else
            printWithIndent "Without valgrind memory usage analysis\n" $INDENT_LEN
        fi
    else
        printWithIndent "Without valgrind memory usage analysis\n" $INDENT_LEN
    fi

    if [ $BUILD_NIST_STS -eq 1 ]
    then
        printWithIndent "With NIST STS reference source code build\n" $INDENT_LEN
    else
        printWithIndent "Without NIST STS reference source code build\n" $INDENT_LEN
    fi

	if [ $BUILD_PROFILE -eq 1 ]
	then
		printWithIndent "With profiling\n" $INDENT_LEN
	else
        printWithIndent "Without profiling\n" $INDENT_LEN
	fi

    if [ $BUILD_VALIDATE -eq 1 ]
    then
        printWithIndent "With validation testing\n" $INDENT_LEN
    else
        printWithIndent "Without validation testing\n" $INDENT_LEN
    fi

	if [ $BUILD_DOCS -eq 1 ]
	then
		printWithIndent "With documenation\n" $INDENT_LEN
	else
		printWithIndent "Without documentation\n" $INDENT_LEN
	fi

    if [ $BUILD_UNIT_TEST -eq 1 ]
    then
        printWithIndent "With unit testing\n" $INDENT_LEN
    else
        printWithIndent "Without unit testing\n" $INDENT_LEN
    fi

    if [ $BUILD_PACKAGE -eq 1 ]
    then
        printWithIndent "With packaging\n" $INDENT_LEN
    else
        printWithIndent "Without packaging\n" $INDENT_LEN
    fi

	if [ $BUILD_WITH_CONSOLE_LOGGING -eq 1 ]
	then
		printWithIndent "With console logging\n" $INDENT_LEN
	else
		printWithIndent "Without console logging\n" $INDENT_LEN
	fi

    if [ $BUILD_MULTI_THREADED -eq 1 ]
    then
        printWithIndent "With multi-threaded tests\n" $INDENT_LEN
    else
        printWithIndent "With single-threaded tests\n" $INDENT_LEN
    fi

	printIt " "
	resetConsoleAttributes
}

# Check host enviroment
function checkHostEnvironment () {

    if isDarwin
    then
        if isIntel64Architecture
        then
	        printBanner "DARWIN $ARCHITECTURE_X64 HOST ENVIRONMENT CHECK"
        elif isARM64Architecture
        then
            printBanner "DARWIN $ARCHITECTURE_ARM64 HOST ENVIRONMENT CHECK"
        else
            printBanner "DARWIN HOST ENVIRONMENT CHECK"
        fi
    elif isLinux
    then
        if isIntel64Architecture
        then
            printBanner "LINUX $ARCHITECTURE_X64 HOST ENVIRONMENT CHECK"
        elif isARM64Architecture
        then
            printBanner "LINUX $ARCHITECTURE_ARM64 HOST ENVIRONMENT CHECK"
        else
	        printBanner "LINUX HOST ENVIRONMENT CHECK"
        fi
    else
	    printBanner "HOST ENVIRONMENT CHECK"
    fi

    LABEL_WIDTH=20

    aptCheck $LABEL_WIDTH "required"

    if hasBash && hasZsh
    then
        bashCheck $LABEL_WIDTH "required"
        zshCheck $LABEL_WIDTH "required"
    elif hasZsh
    then
        zshCheck $LABEL_WIDTH "required"
    elif hasBash
    then
        bashCheck $LABEL_WIDTH "required"
    else
        bashCheck $LABEL_WIDTH "required"
        zshCheck $LABEL_WIDTH "required"
    fi

    if hasHomebrew && hasMacPorts
    then
        homebrewCheck $LABEL_WIDTH "required"
        macPortsCheck $LABEL_WIDTH "required"
    elif hasHomebrew
    then
        homebrewCheck $LABEL_WIDTH "required"
    elif hasMacPorts
    then
        macPortsCheck $LABEL_WIDTH "required"
    else
        homebrewCheck $LABEL_WIDTH "required"
        macPortsCheck $LABEL_WIDTH "required"
    fi

    if hasClang && hasGcc
    then
        clangCheck $LABEL_WIDTH "required"
        clangC99Check $LABEL_WIDTH "required"
        gccCheck $LABEL_WIDTH "required"
        gccC99Check $LABEL_WIDTH "required"
    elif hasClang
    then
        clangCheck $LABEL_WIDTH "required"
        clangC99Check $LABEL_WIDTH "required"
    elif hasGcc
    then
        gccCheck $LABEL_WIDTH "required"
        gccC99Check $LABEL_WIDTH "required"
    else
        clangCheck $LABEL_WIDTH "required"
        clangC99Check $LABEL_WIDTH "required"
        gccCheck $LABEL_WIDTH "required"
        gccC99Check $LABEL_WIDTH "required"
    fi

    gslCheck $LABEL_WIDTH "optional"

    makeCheck $LABEL_WIDTH "required"
    dotNetSdkCheck $LABEL_WIDTH "required"
    gitCheck $LABEL_WIDTH "required"
    
    cunitCheck $LABEL_WIDTH "optional"
    xmllintCheck $LABEL_WIDTH "optional"

    gprofCheck $LABEL_WIDTH "optional"
    cppcheckCheck $LABEL_WIDTH "optional"
    scanBuildCheck $LABEL_WIDTH "optional"
    valgrindCheck $LABEL_WIDTH "optional"

    doxygenCheck $LABEL_WIDTH "optional"

    printIt " "
	exit 1
}

# Function to handle error
function handleError () {
	printIt "\n"
	printError "$1"
	printIt "\n"
	if hasNotifySend
	then
		notify-send "STEER Build" "Build failed!"
	fi
	printSummary
	exit 1
}

# Function to print usage
function printUsage {
	printColor $CONSOLE_GREEN "USAGE:  build.sh <args>"
	printIt " "
	printIt "\tAll arguments are optional. With no arguments, the default behavior is:"
	printIt " "
    printIt "\t• No code analysis"
	printIt "\t• Incremental debug build of programs and static libraries"
    printIt "\t• No packaging"
    printIt "\t• No verbose output"
    printIt "\t• Without console logging"
	printIt "\t• Without documentation build"
    printIt "\t• With single-threaded tests"
    printIt "\t• Without NIST STS reference source code build"
	printIt "\t• Without profiling"
    printIt "\t• Without unit testing"
    printIt "\t• Without validation testing"
	printIt " "
	printIt "\tPossible argument values are:"
	printIt " "
    printIt "\t$ANALYZE_CMD=<$ANALYZE_OPTION_FULL|$ANALYZE_OPTION_CPPCHECK|$ANALYZE_OPTION_SCAN_BUILD|$ANALYZE_OPTION_VALGRIND>\tAnalyzes the source code with the specified tools."
	printIt "\t$CHECK_ENV_CMD\t\t\t\t\tChecks the build support on the host environment."
	printIt "\t$CLEAN_CMD\t\t\t\t\t\tForces a clean build instead of an incremental build."
	printIt "\t$DEBUG_CMD\t\t\t\t\t\tBuilds debug version."
	printIt "\t$HELP_CMD\t\t\t\t\t\tPrints this usage notice."
    printIt "\t$PACKAGE_CMD=<path>\t\t\t\tCreates a distribution archive at the specified path."
	printIt "\t$RELEASE_CMD\t\t\t\t\tBuilds release version."
    printIt "\t\t\t\t\t\t\tSTEER source code directory (defaults to the user's"
    printIt "\t\t\t\t\t\t\thome directory)."
	printIt "\t$VERBOSE_CMD\t\t\t\t\tPrints all shell log output to console."
    printIt "\t$WITH_CONSOLE_LOGGING_CMD\t\t\t\tLog progress and debug information to the console."
	printIt "\t$WITH_DOCUMENTATION_CMD\t\t\t\tBuilds documentation using Doxygen."
    printIt "\t$WITH_MULTI_THREADING_CMD\t\t\t\tBuilds multi-threaded tests (if available)."
    printIt "\t$WITH_NIST_STS_BUILD_CMD\t\t\t\tBuilds the NIST STS 2.1.2 reference source code."
	printIt "\t$WITH_PROFILING_CMD\t\t\t\tBuilds with profiling enabled (Linux only)."
	printIt "\t$WITH_SHARED_LIBS_CMD\t\t\t\tBuild and link with shared libraries instead of"
    printIt "\t\t\t\t\t\t\tstatic libraries."
    printIt "\t$WITH_VALIDATION_CMD\t\t\t\tValidates test programs against the NIST STS test"
    printIt "\t\t\t\t\t\t\tvectors."
    printIt "\t$WITH_UNIT_TEST_CMD\t\t\t\tExecutes the libsteer unit tests."
	printIt " "
	printIt "\tPrerequisites for running this script include:"
	printIt " "
	printIt "\t• bash compatible shell"
    printIt "\t• clang or gcc with C99 or later support"
	printIt "\t• cppcheck"
    printIt "\t• CUnit"
	printIt "\t• Doxygen"
    printIt "\t• gprof (used with --with-profiling option)"
    printIt "\t• make"
    printIt "\t• .NET SDK 6.0 or later"
    printIt "\t• scan-build"
    printIt "\t• valgrind"
	printIt " "
	exit 1
}

# Function to parse command line arguments
function parseCommandLineArgument {
    if stringBeginsWithSubstring "$CMD_LINE_ARG" "$ANALYZE_CMD"
    then
        BUILD_ANALYZE_OPTION=$(removeLeadingSubstring "$CMD_LINE_ARG" "$ANALYZE_CMD""=")
	elif [ "$CMD_LINE_ARG" == $CHECK_ENV_CMD ]
	then
		checkHostEnvironment
	elif [ "$CMD_LINE_ARG" == $CLEAN_CMD ]
	then
		BUILD_CLEAN=1
	elif [ "$CMD_LINE_ARG" == $DEBUG_CMD ]
	then
		BUILD_DEBUG=1
		if [ $BUILD_RELEASE -eq 1 ]
		then
			BUILD_RELEASE=0
		fi
	elif [ "$CMD_LINE_ARG" == $WITH_DOCUMENTATION_CMD ]
	then
		if hasDoxygen
		then
			BUILD_DOCS=1
		else
			BUILD_DOCS=0
			printWarning "Doxygen doesn't appear to be installed; overriding $WITH_DOCUMENTATION_CMD."
		fi
	elif [ "$CMD_LINE_ARG" == $HELP_CMD ]
	then
		printUsage
    elif stringBeginsWithSubstring "$CMD_LINE_ARG" "$PACKAGE_CMD"
	then
		BUILD_PRODUCTS_PACKAGE_ROOT=$(removeLeadingSubstring "$CMD_LINE_ARG" $PACKAGE_CMD"=")
		if [ "$BUILD_PRODUCTS_PACKAGE_ROOT" == "" ]
		then
			BUILD_PRODUCTS_PACKAGE_ROOT="$HOME"
		fi
        if ! directoryExists "$BUILD_PRODUCTS_PACKAGE_ROOT"
        then
            handleError "Package directory path must exist!"
        fi
        BUILD_PRODUCTS_PACKAGE_DIR="$BUILD_PRODUCTS_PACKAGE_ROOT/STEER-$STEER_VERSION/"
        BUILD_PACKAGE=1
    elif [ "$CMD_LINE_ARG" == $WITH_NIST_STS_BUILD_CMD ]
    then
        BUILD_NIST_STS=1
	elif [ "$CMD_LINE_ARG" == $WITH_PROFILING_CMD ]
	then
		if hasGprof
		then
			BUILD_PROFILE=1
		else
			BUILD_PROFILE=0
			printWarning "gprof not available; overriding $WITH_PROFILING_CMD."
		fi
	elif [ "$CMD_LINE_ARG" == $RELEASE_CMD ]
	then
		BUILD_RELEASE=1
		if [ $BUILD_DEBUG -eq 1 ]
		then
			BUILD_DEBUG=0
		fi
    elif [ "$CMD_LINE_ARG" == $WITH_UNIT_TEST_CMD ]
    then
        BUILD_UNIT_TEST=1
    elif [ "$CMD_LINE_ARG" == $WITH_VALIDATION_CMD ]
    then
        BUILD_VALIDATE=1
	elif [ "$CMD_LINE_ARG" == $VERBOSE_CMD ]
	then
		BUILD_VERBOSE=1
    elif [ "$CMD_LINE_ARG" == $WITH_CONSOLE_LOGGING_CMD ]
    then
        BUILD_WITH_CONSOLE_LOGGING=1
	elif [ "$CMD_LINE_ARG" == $WITH_SHARED_LIBS_CMD ]
	then	
		BUILD_SHARED_LIB=1
    elif [ "$CMD_LINE_ARG" == $WITH_MULTI_THREADING_CMD ]
    then
        BUILD_MULTI_THREADED=1
	else
		printError "Unrecognized argument: '$CMD_LINE_ARG'."
		printIt ""
		printUsage
		exit 1
	fi
}

# =================================================================================================
#	Setup
# =================================================================================================

# Set build defaults
if isDarwin 
then
    BUILD_OPERATING_ENV=$OPERATING_ENV_DARWIN
elif isLinux
then
    BUILD_OPERATING_ENV=$OPERATING_ENV_LINUX
else
	BUILD_OPERATING_ENV=$OPERATING_ENV_UNKNOWN
fi

if isARM64Architecture
then
	BUILD_ARCH=$ARCHITECTURE_ARM64
elif isARMArchitecture
then
    BUILD_ARCH=$ARCHITECTURE_ARMHF
elif isIntel64Architecture
then
    BUILD_ARCH=$ARCHITECTURE_X64
else
	BUILD_ARCH=$ARCHITECTURE_UNKNOWN
fi

BUILD_PRODUCTS_DIR_NAME="steer-framework"
#BUILD_PRODUCTS_DIR_NAME="steer-framework"
BUILD_ROOT="$STARTING_WORKING_DIR"
if ! stringEndsWithSubstring "$BUILD_ROOT" "/"
then
    BUILD_ROOT="$BUILD_ROOT""/"
fi
if stringEndsWithSubstring "$BUILD_ROOT" "$BUILD_PRODUCTS_DIR_NAME/"
then
    BUILD_ROOT=$(removeTrailingSubstring "$BUILD_ROOT" "$BUILD_PRODUCTS_DIR_NAME/")
fi

BUILD_ANALYZE_OPTION=$ANALYZE_OPTION_NONE
BUILD_CFG=Debug
BUILD_CLEAN=0
BUILD_DEBUG=1
BUILD_DOCS=0
BUILD_MULTI_THREADED=0
BUILD_NIST_STS=0
BUILD_PACKAGE=0
BUILD_PROFILE=0
BUILD_RELEASE=0
BUILD_UNIT_TEST=0
BUILD_VALIDATE=0
BUILD_VERBOSE=0
BUILD_PRODUCTS_BIN_DIR="bin"
BUILD_PRODUCTS_PACKAGE_ROOT="$HOME"
BUILD_PRODUCTS_OBJ_DIR="obj"
BUILD_WITH_CONSOLE_LOGGING=0
BUILD_SHARED_LIB=0
CLEAN_LOG_PREFIX="_clean_log_"
BUILD_LOG_PREFIX="_build_log_"
CPPCHECK_LOG_PREFIX="_cppcheck_log_"
SCAN_BUILD_LOG_PREFIX="_scan_build_log_"
VALGRIND_LOG_PREFIX="_valgrind_log_"
LOG_POSTFIX=".txt"
LIB_EXTENSION=".a"

# Load test and data list
if [ ! -f ./build_files/test_data_names.txt ]; then
    handleError "Test data names file not found!"
fi
readarray -t NIST_STS_TEST_DATA_NAMES < ./build_files/test_data_names.txt

while [ "${NIST_STS_TEST_DATA_NAMES[-1]}" = "" ] && [ ${#NIST_STS_TEST_DATA_NAMES[@]} -ne 0 ]; do
    unset NIST_STS_TEST_DATA_NAMES[-1]
done

if [ ${#NIST_STS_TEST_DATA_NAMES[@]} -eq "0" ]; then
    handleError "Test data names file is empty!"
fi

if [ ! -f ./build_files/test_names.txt ]; then
    handleError "Test data names file not found!"
fi
readarray -t NIST_STS_TEST_NAMES < ./build_files/test_names.txt

while [ "${NIST_STS_TEST_NAMES[-1]}" = "" ] && [ ${#NIST_STS_TEST_NAMES[@]} -ne 0 ]; do
    unset NIST_STS_TEST_NAMES[-1]
done

if [ ${#NIST_STS_TEST_NAMES[@]} -eq "0" ]; then
    handleError "Test names file is empty!"
fi


if [ ! -f ./build_files/test_folder_names.txt ]; then
    handleError "Test folder names file not found!"
fi
readarray -t NIST_STS_TEST_FOLDER_NAMES < ./build_files/test_folder_names.txt

while [ "${NIST_STS_TEST_FOLDER_NAMES[-1]}" = "" ] && [ ${#NIST_STS_TEST_FOLDER_NAMES[@]} -ne 0 ]; do
    unset NIST_STS_TEST_FOLDER_NAMES[-1]
done

if [ ${#NIST_STS_TEST_FOLDER_NAMES[@]} -eq "0" ]; then
    handleError "Test data names file is empty!"
fi

declare -i VALIDATION_SUCCESS_COUNT=0
declare -i VALIDATION_FAILURE_COUNT=0
declare -i TOTAL_VALIDATION_SUCCESS_COUNT=0
declare -i TOTAL_VALIDATION_FAILURE_COUNT=0

# If there are arguments, check them
for var in "$@"
do
	CMD_LINE_ARG=$var
	parseCommandLineArgument $CMD_LINE_ARG
done

printBanner "STEER BUILD"

printIt "Setting up..."

# Sanity check debug vs. release
if [ $BUILD_DEBUG -eq 0 ]
then
	if [ $BUILD_RELEASE -eq 0 ]
	then
		BUILD_DEBUG=1
	fi
fi

# Export debug/release/profile flag
if [ $BUILD_DEBUG -eq 1 ]
then
	BUILD_CFG=Debug
elif [ $BUILD_RELEASE -eq 1 ]
then
	BUILD_CFG=Release
fi

if fileExists "./include/steer_config.h"
then
	forceDeleteFile "./include/steer_config.h"
fi
STEER_BUILD_OS="$(uname)"
STEER_BUILD_OS_VERSION="$(uname -r)"
STEER_BUILD_ARCH="$(uname -m)"
echo "#define STEER_BUILD_OS" '"'"$STEER_BUILD_OS $STEER_BUILD_OS_VERSION"'"' >> "./include/steer_config.h"
echo "#define STEER_BUILD_ARCH" '"'"$STEER_BUILD_ARCH"'"' >> "./include/steer_config.h"
echo "#define STEER_BUILD_MULTI_THREADED_TESTS $BUILD_MULTI_THREADED" >> "./include/steer_config.h"
echo "#define STEER_ENABLE_CONSOLE_LOGGING $BUILD_WITH_CONSOLE_LOGGING" >> "./include/steer_config.h"

# Create build directory if necessary
if ! directoryExists "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/"
then
	createDirectory "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/"
fi

# Create the logs directory if necessary
BUILD_LOGS_DIR="$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/logs/"
if ! directoryExists "$BUILD_LOGS_DIR"
then
    createDirectory "$BUILD_LOGS_DIR"
fi

# Create the results directory if necessary
BUILD_RESULTS_DIR="$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/results/"
if ! directoryExists "$BUILD_RESULTS_DIR"
then
    createDirectory "$BUILD_RESULTS_DIR"
fi

# Define program directory
PROG_DIR="$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/$BUILD_PRODUCTS_BIN_DIR/$BUILD_OPERATING_ENV/$BUILD_ARCH/$BUILD_CFG"

# Define NIST STS data directory
NIST_STS_DATA_DIR="$BUILD_ROOT"/$BUILD_PRODUCTS_DIR_NAME/data/nist-sts

# Define NIST STS validation tests directory
NIST_STS_VALIDATION_TESTS_DIR="$BUILD_ROOT"/$BUILD_PRODUCTS_DIR_NAME/test/nist-sts

# Define NIST STS validation test results directory
NIST_STS_VALIDATION_RESULTS_DIR="$BUILD_RESULTS_DIR"/nist-sts
if ! directoryExists "$NIST_STS_VALIDATION_RESULTS_DIR"
then
    createDirectory "$NIST_STS_VALIDATION_RESULTS_DIR"
fi
NIST_STS_VALIDATION_RESULTS_DIR="$BUILD_RESULTS_DIR"/nist-sts/validation
if ! directoryExists "$NIST_STS_VALIDATION_RESULTS_DIR"
then
    createDirectory "$NIST_STS_VALIDATION_RESULTS_DIR"
fi

# Check library file extension
if [ $BUILD_SHARED_LIB -eq 1 ]
then
    LIB_EXTENSION=".so"
fi

# Export build environment
export BUILD_CFG
export BUILD_ROOT
export BUILD_LOGS_DIR
export BUILD_RESULTS_DIR
export BUILD_PRODUCTS_DIR_NAME
export BUILD_PRODUCTS_BIN_DIR
export BUILD_PRODUCTS_OBJ_DIR
export BUILD_OPERATING_ENV
export BUILD_ARCH
export BUILD_PROFILE
export BUILD_SHARED_LIB
export BUILD_MULTI_THREADED

printIt " "

# Make sure there are no out-of-date libs that could be accidentally linked to
forceDeleteDirectory "$PROG_DIR"/*.*

# =================================================================================================
#	Clean
# =================================================================================================

# Clean?
if [ $BUILD_CLEAN -eq 1 ]
then
	printBanner "CLEAN"

	CLEAN_TS=$(getDateTimeString)

    printIt "Cleaning docs...\n"
    if directoryExists "./docs/html"
    then
        deleteDirectory "./docs/html"
    fi
    if directoryExists "./docs/latex"
    then
        deleteDirectory "./docs/latex"
    fi

	printIt "Cleaning logs...\n"
	if directoryExists "$BUILD_LOGS_DIR"
	then
		deleteDirectory "$BUILD_LOGS_DIR"
		createDirectory "$BUILD_LOGS_DIR"
	fi

    printIt "Cleaning results...\n"
    if directoryExists "$BUILD_RESULTS_DIR"
    then
        deleteDirectory "$BUILD_RESULTS_DIR"
        createDirectory "$BUILD_RESULTS_DIR"
    fi

    cleanIt "libsteer$LIB_EXTENSION" "./src/libsteer" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/libsteer$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"

    cleanIt "steer_sample_test" "./src/sample-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_sample_test$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
    cleanIt "steer_simple_test" "./src/simple-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_simple_test$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"

    for NIST_STS_TEST_NAME in "${NIST_STS_TEST_NAMES[@]}"
    do
        NIST_STS_TEST_NAME_DASH="${NIST_STS_TEST_NAME// /-}"
        NIST_STS_TEST_NAME_UNDERSCORE="${NIST_STS_TEST_NAME// /_}"
        cleanIt "nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test" "./src/nist-sts/""$NIST_STS_TEST_NAME_DASH" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/""$NIST_STS_TEST_NAME_UNDERSCORE""$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
    done

    cleanIt "ascii_binary_to_bytes" "./src/ascii-binary-to-bytes" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/ascii_binary_to_bytes$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
    cleanIt "libsteer_unit_test" "./test/unit-test/libsteer-public" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/libsteer_unit_test$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
    cleanIt "libsteer_private_unit_test" "./test/unit-test/libsteer-private" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/libsteer_private_unit_test$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
    cleanIt "steer_run_validations" "./src/run-validations" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_run_validations$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
    cleanIt "steer_test_scheduler" "./src/test-scheduler" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_test_scheduler$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
    cleanIt "steer_validate" "./src/validate-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_validate$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
    cleanIt "steer_add_test" "./src/add-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_add_test$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
   
     # .NET programs
    if hasDotNetSdk
    then
        pushPath "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator" $BUILD_VERBOSE
	    printIt "Cleaning steer_schema_validator..."
        if [ $BUILD_DEBUG -eq 1 ]
        then
            dotnet clean --configuration Debug > "$BUILD_LOGS_DIR/steer_schema_validator$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
        elif [ $BUILD_RELEASE -eq 1 ]
        then
            dotnet clean --configuration Release > "$BUILD_LOGS_DIR/steer_schema_validator$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
        fi
        if fileExists "$BUILD_LOGS_DIR/steer_schema_validator$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
        then
            if grep -Fxq "Build succeeded." "$BUILD_LOGS_DIR/steer_schema_validator$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"
            then
                printSuccess "\tClean of steer_schema_validator succeeded.\n"
            else
                printError "\tClean of steer_schema_validator failed!\n"
            fi
        else
            printError "\tClean of steer_schema_validator failed!\n"
        fi
        popPath $BUILD_VERBOSE
    fi

    # NIST STS 2.1.2
    if [ $BUILD_NIST_STS -eq 1 ]
    then
        cleanIt "assess" "./sts-2.1.2-reference" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/assess$CLEAN_LOG_PREFIX$CLEAN_TS$LOG_POSTFIX"

        # Clean out the algorithm results
        for NIST_STS_TEST_FOLDER_NAME in "${NIST_STS_TEST_FOLDER_NAMES[@]}"
        do
            if fileExists "./sts-2.1.2-reference/experiments/AlgorithmTesting/$NIST_STS_TEST_FOLDER_NAME/results.txt"
            then
                rm "./sts-2.1.2-reference/experiments/AlgorithmTesting/$NIST_STS_TEST_FOLDER_NAME/results.txt"
            fi
            if fileExists "./sts-2.1.2-reference/experiments/AlgorithmTesting/$NIST_STS_TEST_FOLDER_NAME/stats.txt"
            then
                rm "./sts-2.1.2-reference/experiments/AlgorithmTesting/$NIST_STS_TEST_FOLDER_NAME/stats.txt"
            fi
        done
        if fileExists "./sts-2.1.2-reference/experiments/AlgorithmTesting/finalAnalysisReport.txt"
        then
            rm "./sts-2.1.2-reference/experiments/AlgorithmTesting/finalAnalysisReport.txt"
        fi
        if fileExists "./sts-2.1.2-reference/experiments/AlgorithmTesting/freq.txt"
        then
            rm "./sts-2.1.2-reference/experiments/AlgorithmTesting/freq.txt"
        fi
    fi

fi

# =================================================================================================
#	Code quality analysis
# =================================================================================================

if [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_FULL ] || [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_CPPCHECK ]
then
    printBanner "CODE QUALITY ANALYSIS: CPPCHECK"

    printIt "Checking for cppcheck..."
    if ! hasCppcheck
    then
        printWarning "\tcppcheck not available.\n"
    else
        printSuccess "\tcppcheck available.\n"

        CPPCHECK_TS=$(getDateTimeString)

        checkIt "utilities" "./src/utilities" $BUILD_VERBOSE "$BUILD_LOGS_DIR/utilities$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        checkIt "steer_sample_test" "./src/sample-test" $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_sample_test$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        checkIt "steer_simple_test" "./src/simple-test" $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_simple_test$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        checkIt "common" "./src/nist-sts/common" $BUILD_VERBOSE "$BUILD_LOGS_DIR/common$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""

        for NIST_STS_TEST_NAME in "${NIST_STS_TEST_NAMES[@]}"
        do
            NIST_STS_TEST_NAME_DASH="${NIST_STS_TEST_NAME// /-}"
            NIST_STS_TEST_NAME_UNDERSCORE="${NIST_STS_TEST_NAME// /_}"
            checkIt "nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test" "./src/nist-sts/""$NIST_STS_TEST_NAME_DASH" $BUILD_VERBOSE "$BUILD_LOGS_DIR/""$NIST_STS_TEST_NAME_UNDERSCORE""$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        done

        checkIt "ascii_binary_to_bytes" "./src/ascii-binary-to-bytes" $BUILD_VERBOSE "$BUILD_LOGS_DIR/ascii_binary_to_bytes$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        checkIt "steer_run_validations" "./src/run-validations" $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_run_validations$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        checkIt "steer_test_scheduler" "./src/test-scheduler" $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_test_scheduler$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        checkIt "steer_validate" "./src/validate-test" $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_validate$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""

        checkIt "steer_add_test" "./src/add-test" $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_add_test$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        # NIST STS 2.1.2
        if [ $BUILD_NIST_STS -eq 1 ]
        then
            checkIt "assess" "./sts-2.1.2-reference/src" $BUILD_VERBOSE "$BUILD_LOGS_DIR/assess$CPPCHECK_LOG_PREFIX$CPPCHECK_TS$LOG_POSTFIX" ""
        fi
    fi
fi

if [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_FULL ] || [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_SCAN_BUILD ]
then
    printBanner "CODE QUALITY ANALYSIS: SCAN-BUILD"

    printIt "Checking for scan-build..."
    if ! hasScanBuild
    then
        printWarning "\tscan-build not available.\n"
    else
        printSuccess "\tscan-build available.\n"
        SCAN_BUILD_TS=$(getDateTimeString)

        scanIt "libsteer" "./src/libsteer" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/libsteer$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"

        scanIt "steer_sample_test" "./src/sample-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_sample_test$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"
        scanIt "steer_simple_test" "./src/simple-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_simple_test$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"

        for NIST_STS_TEST_NAME in "${NIST_STS_TEST_NAMES[@]}"
        do
            NIST_STS_TEST_NAME_DASH="${NIST_STS_TEST_NAME// /-}"
            NIST_STS_TEST_NAME_UNDERSCORE="${NIST_STS_TEST_NAME// /_}"
            scanIt "nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test" "./src/nist-sts/""$NIST_STS_TEST_NAME_DASH" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/""$NIST_STS_TEST_NAME_UNDERSCORE""$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX" ""
        done

        scanIt "ascii_binary_to_bytes" "./src/ascii-binary-to-bytes" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/ascii_binary_to_bytes$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"
        scanIt "steer_run_validations" "./src/run-validations" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_run_validations$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"
        scanIt "steer_test_scheduler" "./src/test-scheduler" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_test_scheduler$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"
        scanIt "steer_validate" "./src/validate-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_validate$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"

        scanIt "steer_add_test" "./src/add-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_add_test$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"
        # NIST STS 2.1.2
        if [ $BUILD_NIST_STS -eq 1 ]
        then
            scanIt "assess" "./sts-2.1.2-reference/src" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/assess$SCAN_BUILD_LOG_PREFIX$SCAN_BUILD_TS$LOG_POSTFIX"
        fi

    fi
fi

# =================================================================================================
#	Build
# =================================================================================================

printBanner "BUILD"

BUILD_TS=$(getDateTimeString)

# Libraries
buildIt "libsteer$LIB_EXTENSION" "./src/libsteer" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/libsteer$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""

# Programs
buildIt "steer_sample_test" "./src/sample-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_sample_test$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
buildIt "steer_simple_test" "./src/simple-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_simple_test$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""

for NIST_STS_TEST_NAME in "${NIST_STS_TEST_NAMES[@]}"
do
    NIST_STS_TEST_NAME_DASH="${NIST_STS_TEST_NAME// /-}"
    NIST_STS_TEST_NAME_UNDERSCORE="${NIST_STS_TEST_NAME// /_}"
    buildIt "nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test" "./src/nist-sts/""$NIST_STS_TEST_NAME_DASH" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/""$NIST_STS_TEST_NAME_UNDERSCORE""$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
done

buildIt "ascii_binary_to_bytes" "./src/ascii-binary-to-bytes" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/ascii_binary_to_bytes$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
buildIt "libsteer_unit_test" "./test/unit-test/libsteer-public" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/libsteer_unit_test$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
buildIt "libsteer_private_unit_test" "./test/unit-test/libsteer-private" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/libsteer_private_unit_test$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
buildIt "steer_run_validations" "./src/run-validations" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_run_validations$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
buildIt "steer_test_scheduler" "./src/test-scheduler" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_test_scheduler$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
buildIt "steer_validate" "./src/validate-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/steer_validate$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
buildIt "steer_add_test" "./src/add-test" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/add_test$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""

# .NET programs
if hasDotNetSdk
then
    pushPath "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator" $BUILD_VERBOSE
	printIt "Building steer_schema_validator..."
    if [ $BUILD_DEBUG -eq 1 ]
    then
        dotnet build --configuration Debug > "$BUILD_LOGS_DIR/steer_schema_validator$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX"
    elif [ $BUILD_RELEASE -eq 1 ]
    then
        dotnet build --configuration Release > "$BUILD_LOGS_DIR/steer_schema_validator$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX"
    fi
    if fileExists "$BUILD_LOGS_DIR/steer_schema_validator$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX"
    then
        if grep -Fxq "Build succeeded." "$BUILD_LOGS_DIR/steer_schema_validator$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX"
        then
            printSuccess "\tBuild of steer_schema_validator succeeded.\n"
        else
            printError "\tBuild of steer_schema_validator failed!\n"
        fi
    else
        printError "\tBuild of steer_schema_validator failed!\n"
    fi
    popPath $BUILD_VERBOSE
fi

# NIST STS 2.1.2
if [ $BUILD_NIST_STS -eq 1 ]
then
    if ! directoryExists "./sts-2.1.2-reference/experiments/AlgorithmTesting"
    then
        pushPath "./sts-2.1.2-reference/experiments/" $BUILD_VERBOSE
        ./create-dir-script
        popPath $BUILD_VERBOSE
    fi

    buildIt "assess" "./sts-2.1.2-reference" makefile $BUILD_VERBOSE "$BUILD_LOGS_DIR/assess$BUILD_LOG_PREFIX$BUILD_TS$LOG_POSTFIX" ""
fi

# =================================================================================================
#	Memory usage analysis
# =================================================================================================

if [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_FULL ] || [ $BUILD_ANALYZE_OPTION == $ANALYZE_OPTION_VALGRIND ]
then
    printBanner "MEMORY USAGE ANALYSIS: VALGRIND"

    printIt "Checking for valgrind..."
    if ! hasValgrind
    then
        printWarning "\tvalgrind not available.\n"
    else
        printSuccess "\tvalgrind available.\n"

        VALGRIND_TS=$(getDateTimeString)
        
        pushPath "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME" $BUILD_VERBOSE
        for NIST_STS_TEST_NAME in "${NIST_STS_TEST_NAMES[@]}"
        do
            NIST_STS_TEST_NAME_DASH="${NIST_STS_TEST_NAME// /-}"
            NIST_STS_TEST_NAME_UNDERSCORE="${NIST_STS_TEST_NAME// /_}"

            for NIST_STS_TEST_DATA_NAME in "${NIST_STS_TEST_DATA_NAMES[@]}"
            do
                printIt "Checking nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test with memcheck using $NIST_STS_TEST_DATA_NAME.bin..."
                valgrind \
                    --tool=memcheck \
                    --show-error-list=yes \
                    --leak-check=full \
                    --show-leak-kinds=all \
                    --track-origins=yes \
                    --log-file="$BUILD_LOGS_DIR/$NIST_STS_TEST_NAME_UNDERSCORE"_"$NIST_STS_TEST_DATA_NAME""$VALGRIND_LOG_PREFIX$VALGRIND_TS$LOG_POSTFIX" \
                    "$PROG_DIR/nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test" \
                    -e "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/data/nist-sts/appendix-b/$NIST_STS_TEST_DATA_NAME/$NIST_STS_TEST_DATA_NAME.bin" \
                    -r "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/results/nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test_data_""$NIST_STS_TEST_DATA_NAME""_results.json" \
                    -p "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/test/nist-sts/$NIST_STS_TEST_NAME_DASH/appendix-b/parameters/$NIST_STS_TEST_DATA_NAME/parameters.json"   
                LOG_CONTENTS=$(<"$BUILD_LOGS_DIR/$NIST_STS_TEST_NAME_UNDERSCORE"_"$NIST_STS_TEST_DATA_NAME""$VALGRIND_LOG_PREFIX$VALGRIND_TS$LOG_POSTFIX")
                SUMMARY=${LOG_CONTENTS#*ERROR SUMMARY: *}
                SUMMARY=${SUMMARY#*ERROR SUMMARY: *}
                SUMMARY=${SUMMARY% error*}
                if [ $SUMMARY == "0" ]
                then
                    printSuccess "\tNo memory issues found in nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test using $NIST_STS_TEST_DATA_NAME.bin."
                else
                    printWarning "\t$SUMMARY memory issues found in nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test using $NIST_STS_TEST_DATA_NAME.bin."
                fi
                printIt " "
            done
        done

        printIt "Checking steer_test_scheduler with memcheck using validation_tests.json..."
        valgrind \
            --tool=memcheck \
            --show-error-list=yes \
            --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --log-file="$BUILD_LOGS_DIR/steer_test_scheduler""$VALGRIND_LOG_PREFIX$VALGRIND_TS$LOG_POSTFIX" \
            "$PROG_DIR/steer_test_scheduler" \
            -s "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/test-scheduler/validation_test_schedule.json"   
        LOG_CONTENTS=$(<"$BUILD_LOGS_DIR/steer_test_scheduler""$VALGRIND_LOG_PREFIX$VALGRIND_TS$LOG_POSTFIX")
        SUMMARY=${LOG_CONTENTS#*ERROR SUMMARY: *}
        SUMMARY=${SUMMARY#*ERROR SUMMARY: *}
        SUMMARY=${SUMMARY% error*}
        if [ $SUMMARY == "0" ]
        then
            printSuccess "\tNo memory issues found in steer_test_scheduler using validation_test_schedule.json."
        else
            printWarning "\t$SUMMARY memory issues found in steer_test_scheduler using validation_test_schedule.json."
        fi
        printIt " "

        popPath $BUILD_VERBOSE
    fi
fi

# =================================================================================================
#   Unit Test
# =================================================================================================

if [ $BUILD_UNIT_TEST -eq 1 ]
then

    printBanner "UNIT TEST"
    
    UNIT_TEST_TS=$(getDateTimeString)
    UNIT_TEST_INDENT=30

	printIt "Running libsteer public interface unit test..."
    "$PROG_DIR/libsteer_unit_test"
    mv "$BUILD_LOGS_DIR/libsteer_Unit_Test-Results.xml" "$BUILD_LOGS_DIR/libsteer_unit_test_results_$UNIT_TEST_TS.xml"
    UNIT_TEST_XML=$(cat "$BUILD_LOGS_DIR/libsteer_unit_test_results_$UNIT_TEST_TS.xml")
    parseCUnitResults "$UNIT_TEST_XML" "$UNIT_TEST_INDENT"
    printIt ""
    
	printIt "Running libsteer private interface unit test..."
    "$PROG_DIR/libsteer_private_unit_test"
    mv "$BUILD_LOGS_DIR/libsteer_private_Unit_Test-Results.xml" "$BUILD_LOGS_DIR/libsteer_private_unit_test_results_$UNIT_TEST_TS.xml"
    UNIT_TEST_XML=$(cat "$BUILD_LOGS_DIR/libsteer_private_unit_test_results_$UNIT_TEST_TS.xml")
    parseCUnitResults "$UNIT_TEST_XML" "$UNIT_TEST_INDENT"
    printIt ""
    
fi

# =================================================================================================
#	Validation
# =================================================================================================

if [ $BUILD_VALIDATE -eq 1 ]
then
	printBanner "VALIDATION"

    VALIDATION_TS=$(getDateTimeString)
    if directoryExists "$NIST_STS_VALIDATION_RESULTS_DIR"
    then
        deleteDirectory "$NIST_STS_VALIDATION_RESULTS_DIR"
        createDirectory "$NIST_STS_VALIDATION_RESULTS_DIR"
    fi

    printIt "Running validation tests..."
    pushPath "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME" $BUILD_VERBOSE
    "$PROG_DIR"/steer_test_scheduler -s "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/test-scheduler/validation_test_schedule.json"
    popPath $BUILD_VERBOSE

    printIt "Checking validation test results..."
    pushPath "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME" $BUILD_VERBOSE
    "$PROG_DIR"/steer_run_validations -c "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/run-validations/validation_checks.json"
    popPath $BUILD_VERBOSE
    
fi

# =================================================================================================
#   Documentation
# =================================================================================================

if [ $BUILD_DOCS -eq 1 ]
then
	printBanner "DOCUMENTATION"
    
	printIt "Building HTML documentation at $BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/html..."

	pushPath "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs" $BUILD_VERBOSE
	doxygen "./Doxyfile"
	if directoryExists "./html"
	then
		printIt "Opening index.html in browser..."
		if isLinux
		then
			xdg-open "./html/index.html"
		elif isDarwin
		then
			open "./html/index.html"
		fi
	else
		printError "Failed to generate HTML documentation!"
	fi
	popPath $BUILD_VERBOSE
	printIt " "

else
    printIt " "
fi

# =================================================================================================
#   Packaging
# =================================================================================================

if [ $BUILD_PACKAGE -eq 1 ]
then

	printBanner "PACKAGING"

    BUILD_MT=""
    if [ $BUILD_MULTI_THREADED -eq 1 ]
    then    
        BUILD_MT="mt"
    fi

    if fileExists "STEER-$STEER_VERSION-$BUILD_OPERATING_ENV-$BUILD_ARCH-$BUILD_CFG-$BUILD_MT.tar.gz"
    then
        deleteFile "STEER-$STEER_VERSION-$BUILD_OPERATING_ENV-$BUILD_ARCH-$BUILD_CFG-$BUILD_MT.tar.gz"
    fi

    if fileExists "STEER-$STEER_VERSION-$BUILD_OPERATING_ENV-$BUILD_ARCH-$BUILD_CFG-$BUILD_MT.dmg"
    then
        deleteFile "STEER-$STEER_VERSION-$BUILD_OPERATING_ENV-$BUILD_ARCH-$BUILD_CFG-$BUILD_MT.dmg"
    fi

    printIt "\tCreating directories..."

    if directoryExists "$BUILD_PRODUCTS_PACKAGE_DIR"
    then
        deleteDirectory "$BUILD_PRODUCTS_PACKAGE_DIR"
    fi
    if ! directoryExists "$BUILD_PRODUCTS_PACKAGE_DIR"
    then
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data/validation"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data/validation/nist-sts"
        if ! fileExists "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/STEER_User_Guide.pdf"
        then
            createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/images"
        fi
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/schedules"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/multiple-bitstreams"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/multiple-bitstreams/nist-sts"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/validation"
        createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/validation/nist-sts"
    fi

    printIt "\tCopying programs..."
    
    for NIST_STS_TEST_NAME in "${NIST_STS_TEST_NAMES[@]}"
    do
        NIST_STS_TEST_NAME_UNDERSCORE="${NIST_STS_TEST_NAME// /_}"
        cp "$PROG_DIR"/nist_sts_"$NIST_STS_TEST_NAME_UNDERSCORE"_test "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
    done

    cp "$PROG_DIR"/steer_run_validations "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
    cp "$PROG_DIR"/steer_test_scheduler "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
    cp "$PROG_DIR"/steer_validate "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
    cp "$PROG_DIR"/steer_add_test "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"

    if fileExists "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/steer_schema_validator"
    then
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/JetBrains.Annotations.dll" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/Json.More.dll" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"    
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/JsonPointer.Net.dll" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/JsonSchema.Net.dll" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/steer_schema_validator" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/steer_schema_validator.deps.json" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/steer_schema_validator.dll" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/steer_schema_validator.runtimeconfig.json" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/steer-schema-validator/bin/$BUILD_CFG/net6.0/System.Text.Json.dll" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/bin"
    fi

    printIt "\tCopying test data..."
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/data/random.bin" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data/random.bin"
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/data/validation/nist-sts/e.bin" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data/validation/nist-sts"
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/data/validation/nist-sts/pi.bin" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data/validation/nist-sts"
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/data/validation/nist-sts/sha1.bin" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data/validation/nist-sts"
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/data/validation/nist-sts/sqrt2.bin" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data/validation/nist-sts"
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/data/validation/nist-sts/sqrt3.bin" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/data/validation/nist-sts"

    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/run-validations/validation_checks.json" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/validation/nist-sts/"
    cp -R "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/test/validation/nist-sts/" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/validation/nist-sts/"
    cp -R "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/pkg-assets/non-overlapping-template-matching/validation/nist-sts/" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/validation/nist-sts/non-overlapping-template-matching/"
    cp -R "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/test/multiple-bitstreams/nist-sts/" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/multiple-bitstreams/nist-sts/"
    cp -R "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/pkg-assets/non-overlapping-template-matching/multiple-bitstreams/nist-sts/" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/test/multiple-bitstreams/nist-sts/non-overlapping-template-matching/"

    printIt "\tCopying schedules..."
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/test-scheduler/benchmark_test_schedule.json" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/schedules"
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/test-scheduler/device_test_schedule.json" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/schedules"
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/src/test-scheduler/validation_test_schedule.json" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/schedules"

    if [ $BUILD_NIST_STS -eq 1 ]
    then
        printIt "\tCopying NIST STS reference material..."
        if ! directoryExists "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference"
        then
            createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference"
            createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/data"
            createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/experiments"
            createDirectory "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/templates"
        fi

        cp -R "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/sts-2.1.2-reference/data/" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/data/"
        cp -R "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/sts-2.1.2-reference/experiments/AlgorithmTesting/" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/experiments/AlgorithmTesting/"
        cp -R "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/sts-2.1.2-reference/templates/" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/templates/"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/sts-2.1.2-reference/assess" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/sts-2.1.2-reference/benchmark_sts.sh" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/sts-2.1.2-reference/memcheck_sts.sh" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/nist/NIST SP 800-22 Rev 1a.pdf" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/sts-2.1.2-reference/NIST SP 800-22 Rev 1a.pdf"
    fi

    printIt "\tCopying documentation..."
    cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/pkg-assets/README.md" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/README.md"
    if fileExists "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/STEER_User_Guide.pdf"
    then
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/STEER_User_Guide.pdf" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/"
    else
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/STEER_User_Guide.md" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/STEER_User_Guide.md"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/images/anametric-logo.png" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/images/"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/images/ddics-logo.png" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/images/"
        cp "$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/docs/images/steer-blue-logo.png" "$BUILD_PRODUCTS_PACKAGE_DIR/STEER/images/"
    fi

    printIt "\tCreating package archive..."

    if fileExists "$BUILD_PRODUCTS_PACKAGE_ROOT/STEER-$STEER_VERSION-$BUILD_OPERATING_ENV-$BUILD_ARCH-$BUILD_CFG.tar.gz"
    then
        forceDeleteFile "$BUILD_PRODUCTS_PACKAGE_ROOT/STEER-$STEER_VERSION-$BUILD_OPERATING_ENV-$BUILD_ARCH-$BUILD_CFG.tar.gz"
    fi

    pushPath "$BUILD_PRODUCTS_PACKAGE_ROOT" $BUILD_VERBOSE
	if [ $BUILD_OPERATING_ENV == $OPERATING_ENV_DARWIN ]
    then
        hdiutil create "STEER-$STEER_VERSION-$BUILD_OPERATING_ENV-$BUILD_ARCH-$BUILD_CFG-$BUILD_MT.dmg" -srcfolder "./STEER-$STEER_VERSION"
    elif [ $BUILD_OPERATING_ENV == $OPERATING_ENV_LINUX ]
    then
        cd "./STEER-$STEER_VERSION"
        tar -zcvf "../STEER-$STEER_VERSION-$BUILD_OPERATING_ENV-$BUILD_ARCH-$BUILD_CFG-$BUILD_MT.tar.gz" "./STEER"
        cd ..
    fi
    deleteDirectory "$BUILD_PRODUCTS_PACKAGE_DIR"
    popPath $BUILD_VERBOSE

	printIt " "
fi

# =================================================================================================
#	Summary
# =================================================================================================

if hasNotifySend
then
	notify-send "STEER Build" "Build succeeded."
fi
printSummary

# =================================================================================================
