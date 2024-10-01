# =================================================================================================
#
#   bash_os_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash operating system support functions.
#
# =================================================================================================

# Function to check for Darwin OS
function isDarwin () {
	if [ "$(uname)" == "Darwin" ]; then
		true
	else
		false
	fi
}

# Function to check for Linux OS
function isLinux () {
	if [ "$(uname)" == "Linux" ]; then
		true
	else
		false
	fi
}

# =================================================================================================
