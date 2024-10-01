# =================================================================================================
#
#   bash_file_system_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash file system support functions.
#
# =================================================================================================

# Function to check for existence of file
function fileExists () {
	if [ -f "$1" ]; then
		true
	else
		false
	fi
}

# Function to delete file
function deleteFile () {
	if fileExists "$1"
	then
		rm "$1"
	fi
}

# Functio to force delete file
function forceDeleteFile () {
	if fileExists "$1"
	then
		rm -f "$1"
	fi
}

# Function to create directory
function createDirectory () {
	mkdir "$1"
	chmod 0755 "$1"
}

# Function to check for existence of directory
function directoryExists () {
	if [ -d "$1" ] && [ -x "$1" ]; then
		true
	else
		false
	fi
}

# Function to delete directory and its contents
function deleteDirectory () {
	if directoryExists "$1" 
	then
		rm -r "$1"
	fi
}

# Function to force delete directory and its contents
function forceDeleteDirectory () {
	if directoryExists "$1"
	then
		rm -rf "$1"
	fi
}

# Function to delete files with file extension from directory
function deleteFilesWithExtensionFromDirectory () {
    if directoryExists "$2"
    then
        cd "$2"
        rm -rf *"$1"
    fi
}

# Function to push path
function pushPath () {
	if [ "$2" -eq 1 ]
	then
		pushd "$1"
	else
		pushd "$1" >/dev/null 
	fi
}

# Function to pop path
function popPath () {
	if [ "$1" -eq 1 ]
	then
		popd
	else
		popd >/dev/null 
	fi
}

# Function to get file size in bytes
function fileSizeInBytes () {
	if fileExists "$1"
	then
		FSZ=$(wc -c "$1" | awk '{print $1}' )
	else
		FSZ=0
	fi
	echo $FSZ
}

# =================================================================================================
