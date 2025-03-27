# =================================================================================================
#
#   bash_shell_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash shell support functions.
#
# =================================================================================================

# Function to test for definition of shell variable
function shellVarDefined () {
	if [ -z ${1+x} ]; then
		false
	else
		if [ "$1" = "" ]; then
			false
		else
			true
		fi
	fi
}

# Function to check for installation of command
function cmdInstalled () {
	if command -v "$1" >/dev/null 2>&1; then
		true 
	else
		false
	fi
}

# Function to check for existence of command
function cmdExists () {
	if [ -x "$1" ] >/dev/null 2>&1; then
		true
	else
		false
	fi
}

# Function to check for bash
function hasBash () {
    if cmdInstalled "bash"; then
        true
    else
        false
    fi
}

# Function to get bash version
function bashVersion () {
    if hasBash
    then
        BASH_VER="$(bash --version)"
        if stringBeginsWithSubstring "$BASH_VER" "GNU bash"
        then
            BASH_VER=${BASH_VER%-release*}
            BASH_VER=${BASH_VER#*version }
            BASH_VER=${BASH_VER%(*}
            echo $BASH_VER
        else
            echo "Unknown"
        fi
    else
        echo "N/A"
    fi
}

# Function to check for zsh
function hasZsh () {
    if cmdInstalled "zsh"; then
        true
    else
        false
    fi
}

# Function to get zsh version
function zshVersion () {
    if hasZsh
    then
        ZSH_VER="$(zsh --version)"
        if stringBeginsWithSubstring "$ZSH_VER" "zsh "
        then
            ZSH_VER=${ZSH_VER% (*}
            ZSH_VER=${ZSH_VER#*zsh }
            echo $ZSH_VER
        else
            echo "Unknown"
        fi
    else
        echo "N/A"
    fi
}

# Function to check for python3
function hasPython3 () {
    if cmdInstalled "python3"; then
        true
    else
        false
    fi
}

# Function to get python3 version
function python3Version () {
    if hasPython3
    then
        PYTHON3_VER="$(python3 --version)"
        if stringBeginsWithSubstring "$PYTHON3_VER" "Python"
        then
            PYTHON3_VER=${PYTHON3_VER% (*}
            PYTHON3_VER=${PYTHON3_VER#*Python }
            echo $PYTHON3_VER
        else
            echo "Unknown"
        fi
    else
        echo "N/A"
    fi
}

# Function to check for python
function hasPython () {
    if cmdInstalled "python"; then
        true
    else
        false
    fi
}

function pythonVersion () {
    if hasPython
    then
        PYTHON_VER="$(python --version)"
        if stringBeginsWithSubstring "$PYTHON_VER" "Python"
        then
            PYTHON_VER=${PYTHON_VER% (*}
            PYTHON_VER=${PYTHON_VER#*Python }
            echo $PYTHON_VER
        else
            echo "Unknown"
        fi
    else
        echo "N/A"
    fi
}

function hasPythonPackage () {
    if hasPython3
    then
        if python3 -c "import $1" &> /dev/null; then
            true
        else
            false
        fi
    elif hasPython
    then
        if python -c "import $1" &> /dev/null; then
            true
        else
            false
        fi
    else
        false
    fi
}
# =================================================================================================
