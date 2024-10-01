# =================================================================================================
#
#   bash_dotnet_sdk_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash build support functions.
#
# =================================================================================================

# Function to check for .NET SDK
function hasDotNetSdk () {
	if cmdInstalled "dotnet"; then
		true
	else
		false
	fi	
}

# Function to get .NET SDK version
function dotNetSdkVersion () {
	if hasDotNetSdk 
	then
		DOTNETSDK_VER="$(dotnet --version)"
		echo "$DOTNETSDK_VER"
	else
		echo "N/A"
	fi
}

# =================================================================================================

