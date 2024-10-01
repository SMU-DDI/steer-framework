# =================================================================================================
#
#   bash_check_env.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash console support functions.
#
# =================================================================================================

# Function to check for apt
function aptCheck () {
    if isLinux
    then
        printWithRightJustification "apt: " "${1}"
        if hasAdvancedPackagingTool 
        then
            APT_VERSION=$(advancedPackagingToolVersion)
            printColor $CONSOLE_GREEN "Installed (v$APT_VERSION)"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not installed"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not installed"
            else
                printColor $CONSOLE_RED "Not installed"
            fi
        fi
    fi
}

# Function to check for bash
function bashCheck () {
    printWithRightJustification "bash: " "${1}"
    if hasBash
    then
        BASH_VERSION=$(bashVersion)
        printColor $CONSOLE_GREEN "Installed (v$BASH_VERSION)"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for clang
function clangCheck () {
	printWithRightJustification "clang: " "${1}"
	if hasClang
	then
		CLANG_VERSION=$(clangVersion)
		printColor $CONSOLE_GREEN "Installed (v$CLANG_VERSION)"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for clang C99
function clangC99Check () {
	printWithRightJustification "clang C99 support: " "${1}"
    if hasClang
    then
        if clangSupportsC99
        then
            printColor $CONSOLE_GREEN "Available"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not available"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not available"
            else
                printColor $CONSOLE_RED "Not available"
            fi
        fi
    fi
}

# Function to check for clang C11
function clangC11Check () {
	printWithRightJustification "clang C11 support: " "${1}"
    if hasClang
    then
        if clangSupportsC11
        then
            printColor $CONSOLE_GREEN "Available"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not available"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not available"
            else
                printColor $CONSOLE_RED "Not available"
            fi
        fi
    fi
}

# Function to check for clang C17
function clangC17Check () {
	printWithRightJustification "clang C17 support: " "${1}"
    if hasClang
    then
        if clangSupportsC17
        then
            printColor $CONSOLE_GREEN "Available"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not available"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not available"
            else
                printColor $CONSOLE_RED "Not available"
            fi
        fi
    fi
}

# Function to check for cppcheck
function cppcheckCheck () {
	printWithRightJustification "cppcheck: " "${1}"
	if hasCppcheck 
	then
		CPPCHECK_VERSION=$(cppcheckVersion)
		printColor $CONSOLE_GREEN "Installed (v$CPPCHECK_VERSION)"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for CUnit
function cunitCheck () {
	printWithRightJustification "CUnit: " "${1}"
	if hasCUnit
	then
		printColor $CONSOLE_GREEN "Installed"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for .NET SDK
function dotNetSdkCheck () {
    printWithRightJustification ".NET SDK: " "${1}"
    if hasDotNetSdk
    then
        DOTNETSDK_VERSION=$(dotNetSdkVersion)
        printColor $CONSOLE_GREEN "Installed (v$DOTNETSDK_VERSION)"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for doxygen
function doxygenCheck () {
	printWithRightJustification "Doxygen: " "${1}"
	if hasDoxygen
	then
		DOXYGEN_VERSION=$(doxygenVersion)
		printColor $CONSOLE_GREEN "Installed (v$DOXYGEN_VERSION)"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for dropdmg
function dropDmgCheck() {
	printWithRightJustification "dropdmg: " "${1}"
	if hasDropDmg
	then
		DROPDMG_VERSION=$(dropDmgVersion)
		printColor $CONSOLE_GREEN "Installed (v$DROPDMG_VERSION)"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for dumpcap
function dumpcapCheck() {
	printWithRightJustification "dumpcap: " "${1}"
	if hasDumpcap
	then
		printColor $CONSOLE_GREEN "Installed"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for GCC
function gccCheck () {
	printWithRightJustification "gcc: " "${1}"
	if hasGcc
	then
		GCC_VERSION=$(gccVersion)
		printColor $CONSOLE_GREEN "Installed (v$GCC_VERSION)"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for GCC C99
function gccC99Check () {
	printWithRightJustification "gcc C99 support: " "${1}"
    if hasGcc 
    then
        if gccSupportsC99
        then
            printColor $CONSOLE_GREEN "Available"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not available"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not available"
            else
                printColor $CONSOLE_RED "Not available"
            fi
        fi
    fi
}

# Function to check for GCC C11
function gccC11Check () {
	printWithRightJustification "gcc C11 support: " "${1}"
    if hasGcc 
    then
        if gccSupportsC11
        then
            printColor $CONSOLE_GREEN "Available"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not available"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not available"
            else
                printColor $CONSOLE_RED "Not available"
            fi
        fi
    fi
}

# Function to check for GCC C17
function gccC17Check () {
	printWithRightJustification "gcc C17 support: " "${1}"
    if hasGcc 
    then
        if gccSupportsC17
        then
            printColor $CONSOLE_GREEN "Available"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not available"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not available"
            else
                printColor $CONSOLE_RED "Not available"
            fi
        fi
    fi
}

# Function to check for git
function gitCheck () {
    printWithRightJustification "git: " "${1}"
    if hasGit 
    then
        GIT_VERSION=$(gitVersion)
        printColor $CONSOLE_GREEN "Installed (v$GIT_VERSION)"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for gprof
function gprofCheck () {
	printWithRightJustification "gprof: " "${1}"
	if hasGprof
	then
        GPROF_VERSION=$(gprofVersion)
		printColor $CONSOLE_GREEN "Installed (v$GPROF_VERSION)"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for GSL
function gslCheck() {
    printWithRightJustification "GSL: " "${1}"
    if hasGsl
    then
        GSL_VERSION=$(gslVersion)
        printColor $CONSOLE_GREEN "Installed (v$GSL_VERSION)"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for Homebrew
function homebrewCheck () {
    if isDarwin
    then
        printWithRightJustification "Homebrew: " "${1}"
        if hasHomebrew
        then
            printColor $CONSOLE_GREEN "Installed"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not installed"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not installed"
            else
                printColor $CONSOLE_RED "Not installed"
            fi
        fi
    fi
}

# Function to check for jq
function jqCheck () {
    printWithRightJustification "jq: " "${1}"
    if hasJq
    then
        JQ_VERSION=$(jqVersion)
        printColor $CONSOLE_GREEN "Installed (v$JQ_VERSION)"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for MacPorts
function macPortsCheck () {
    if isDarwin
    then
        printWithRightJustification "MacPorts: " "${1}"
        if hasMacPorts
        then
            MACPORTS_VERSION=$(macPortsVersion)
            printColor $CONSOLE_GREEN "Installed (v$MACPORTS_VERSION)"
        else
            if [ "${2}" == "required" ]
            then
                printColor $CONSOLE_RED "Not installed"
            elif [ "${2}" == "optional" ]
            then
                printColor $CONSOLE_YELLOW "Not installed"
            else
                printColor $CONSOLE_RED "Not installed"
            fi
        fi
    fi
}

# Function to check for make
function makeCheck () {
	printWithRightJustification "make: " "${1}"
	if hasMake
	then
		MAKE_VERSION=$(makeVersion)
		printColor $CONSOLE_GREEN "Installed (v$MAKE_VERSION)"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for scan-build
function scanBuildCheck () {
	printWithRightJustification "scan-build: " "${1}"
	if hasScanBuild
	then
		printColor $CONSOLE_GREEN "Installed"
	else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
	fi
}

# Function to check for sqlite3
function sqlite3BuildCheck () {
    printWithRightJustification "sqlite: " "${1}"
    if hasSqlite3
    then
        SQLITE3_VERSION=$(sqlite3Version)
        printColor $CONSOLE_GREEN "Installed (v$SQLITE3_VERSION)"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for tshark
function tsharkCheck () {
    printWithRightJustification "tshark: " "${1}"
    if hasTshark
    then
        printColor $CONSOLE_GREEN "Installed"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for valgrind
function valgrindCheck () {
    printWithRightJustification "valgrind: " "${1}"
    if hasValgrind
    then
        VALGRIND_VERSION=$(valgrindVersion)
        printColor $CONSOLE_GREEN "Installed (v$VALGRIND_VERSION)"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for xmllint
function xmllintCheck () {
    printWithRightJustification "xmllint: " "${1}"
    if hasXmllint
    then
        printColor $CONSOLE_GREEN "Installed"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# Function to check for zsh
function zshCheck () {
    printWithRightJustification "zsh: " "${1}"
    if hasZsh
    then
        ZSH_VERSION=$(zshVersion)
        printColor $CONSOLE_GREEN "Installed (v$ZSH_VERSION)"
    else
        if [ "${2}" == "required" ]
        then
            printColor $CONSOLE_RED "Not installed"
        elif [ "${2}" == "optional" ]
        then
            printColor $CONSOLE_YELLOW "Not installed"
        else
            printColor $CONSOLE_RED "Not installed"
        fi
    fi
}

# =================================================================================================
