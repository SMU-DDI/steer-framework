# =================================================================================================
#
#   bash_docs_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash documentation support functions.
#
# =================================================================================================

# Function to check for Doxygen
function hasDoxygen () {
	if cmdInstalled "doxygen"; then
		true
	else
		false
	fi
}

# Function to check Doxygen version
function doxygenVersion () {
	if hasDoxygen
	then
		DOXYGEN_VER="$(doxygen --version)"
		echo $DOXYGEN_VER
	else
		echo "N/A"
	fi
}

# =================================================================================================
