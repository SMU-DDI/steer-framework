# =================================================================================================
#
#   bash_python_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash Python support functions.
#
# =================================================================================================

# Function to check for Python
function hasPython () {
	if cmdInstalled "python"; then
		true
	else
		false
	fi	
}

# Function to check for Python 2.x
function hasPython2 () {
	if cmdInstalled "python2"; then
		true
	else
		false
	fi
}

# Function to check for Python 3.x
function hasPython3 () {
	if cmdInstalled "python3"; then
		true
	else
		false
	fi	
}

# Function to get Python version
function pythonVersion () {
	if cmdInstalled "$1"; then
		echo "$($1 -c 'import sys; print(".".join(map(str, sys.version_info[:3])))')"
	fi
}

# Function to check for installation of Python module
function hasPythonModule () {
	IMPORT_CMD="import $1"
	python -c "$IMPORT_CMD" >/dev/null 2>&1
	MODULE_INSTALLED="$(echo $?)"
	if [ "$MODULE_INSTALLED" -eq "0" ]
	then
		true
	else
		false
	fi
}

# Function to check for pip module
function hasPipModule () {
	if hasPython
	then
		PIP_MODULES="$(pip freeze)"
		if stringHasSubstring "$PIP_MODULES" "$1"
		then
			true
		else
			false
		fi
	else
		false
	fi
}

# =================================================================================================
