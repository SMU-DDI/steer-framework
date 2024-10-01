# =================================================================================================
#
#   bash_architecture_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash architecture support functions.
#
# =================================================================================================

# Function to check for ARM architecture
function isARMArchitecture () {
	if [[ "$(uname -m)" == "arm"* ]]; then
		true
	else
		false
	fi
}

# Function to check for ARM hardware floating point (armhf) architecture
function isARMHFArchitecture() {
    if [[ "$(uname -m)" == "arm6l"* ]] || [[ "$(uname -m)" == "arm7l"* ]]; then
        true
    else
        false
    fi
}

# Function to check for ARM 64-bit architecture
function isARM64Architecture () {
	if [[ "$(uname -m)" == "aarch64"* ]] || [[ "$(uname -m)" == "arm64"* ]]; then
		true
	else
		false
	fi
}

# Function to check for Intel 32-bit architecture
function isIntel32Architecture () {
	if [[ "$(uname -m)" == "i386"* ]] || [[ "$(uname -m)" == "i686"* ]]; then
		true
	else
		false
	fi
}

# Function to check for Intel 64-bit architecture
function isIntel64Architecture () {
	if [[ "$(uname -m)" == "x86_64"* ]]; then
		true
	else
		false
	fi
}

# Function to check for RISC-V architecture
function isRISCVArchitecture () {
	if [[ "$(uname -m)" == "riscv"* ]]; then
		true
	else
		false
	fi
}

# Function to check for RISC-V 32-bit architecture
function isRISCV32Architecture () {
	if [[ "$(uname -m)" == "riscv32"* ]]; then
		true
	else
		false
	fi
}

# Function to check for RISC-V 64-bit architecture
function isRISCV64Architecture () {
	if [[ "$(uname -m)" == "riscv64"* ]]; then
		true
	else
		false
	fi
}

# =================================================================================================
