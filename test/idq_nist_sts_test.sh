#!/bin/bash
# =================================================================================================
#
#   idq_nist_sts_test.sh
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
#set -o errexit
#set -o pipefail
set -o nounset

# Stash start date/time
SECONDS=0
BUILD_START="$(date)"

BUILD_ROOT="$HOME"
BUILD_CFG=Debug
BUILD_PRODUCTS_DIR_NAME="steer-framework"
BUILD_PRODUCTS_BIN_DIR="bin"
BUILD_PRODUCTS_INSTALL_DIR="$HOME"
BUILD_PRODUCTS_RESULTS_DIR="results"

if [ "$(uname)" == "Darwin" ] 
then
    BUILD_OPERATING_ENV="darwin"
elif [ "$(uname)" == "Linux" ]
then
    BUILD_OPERATING_ENV="linux"
else
	BUILD_OPERATING_ENV="unknown"
fi

if [[ "$(uname -m)" == "aarch64"* ]] || [[ "$(uname -m)" == "arm64"* ]]
then
	BUILD_ARCH="arm64"
elif [[ "$(uname -m)" == "arm"* ]]
then
    BUILD_ARCH="armhf"
elif [[ "$(uname -m)" == "x86_64"* ]]
then
    BUILD_ARCH="x64"
else
	BUILD_ARCH="unknown"
fi

PROG_DIR="$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/$BUILD_PRODUCTS_BIN_DIR/$BUILD_OPERATING_ENV/$BUILD_ARCH/$BUILD_CFG"
RESULTS_DIR="$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/$BUILD_PRODUCTS_RESULTS_DIR"
PARAMS_DIR="$BUILD_ROOT/$BUILD_PRODUCTS_DIR_NAME/test/multiple-bitstreams/nist-sts"

NIST_STS_TEST_NAMES=( 'approximate entropy' 'block frequency' 'cumulative sums' 'discrete fourier transform' 'frequency' 'linear complexity' 'longest run of ones' 'non overlapping template matching' 'overlapping template matching' 'random excursions' 'random excursions variant' 'rank' 'runs' 'serial' 'universal statistical' )

for NIST_STS_TEST_NAME in "${NIST_STS_TEST_NAMES[@]}"
do
    NIST_STS_TEST_NAME_DASH="${NIST_STS_TEST_NAME// /-}"
    NIST_STS_TEST_NAME_UNDERSCORE="${NIST_STS_TEST_NAME// /_}"
    PROG_NAME="$PROG_DIR/nist_sts_$NIST_STS_TEST_NAME_UNDERSCORE""_test"
    RESULTS_NAME="$RESULTS_DIR/$NIST_STS_TEST_NAME_UNDERSCORE""_idq_report.json"
    PARAMS_NAME="$PARAMS_DIR/$NIST_STS_TEST_NAME_DASH/parameters.json"
    cat /dev/qrandom0 | "$PROG_NAME" -R -r "$RESULTS_NAME" -p "$PARAMS_NAME"
done

BUILD_DURATION=$SECONDS
BUILD_STOP="$(date)"
