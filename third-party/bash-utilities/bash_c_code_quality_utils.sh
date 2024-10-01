# =================================================================================================
#
#   bash_c_code_quality_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash code quality support functions.
#
# =================================================================================================

# Function to check for cppcheck
function hasCppcheck () {
	if cmdInstalled "cppcheck"; then
		true
	else
		false
	fi
}

# Function to get cppcheck version
function cppcheckVersion () {
	if hasCppcheck
	then
		CPPCHECK_VER="$(cppcheck --version)"
		if stringBeginsWithSubstring "$CPPCHECK_VER" "Cppcheck "
		then
			CPPCHECK_VER="$(echo $CPPCHECK_VER | cut -d$' ' -f 2)"
			echo $CPPCHECK_VER
		else
			echo "unknown"
		fi
	else
		echo "N/A"
	fi
}

# Function to run cppcheck
# <name> <path> <verbose> <log> <defines>
function checkIt () {
	if hasCppcheck
	then
		printIt "Checking $1 with cppcheck..."
		pushPath "$2" "$3"
		if [ "$5" == "" ]
		then
			cppcheck --inline-suppr addon=cert.py . > "$4" 2>&1
		else
			cppcheck --inline-suppr addon=cert.py "$5" . > "$4" 2>&1
		fi
		LOG_CONTENTS=`cat "$4"`
		if stringHasSubstring "$LOG_CONTENTS" " error:"
		then
			cat "$4"
			printError "\tcppcheck of $1 generated errors!"
			printIt " "
		elif stringHasSubstring "$LOG_CONTENTS" "warning"
		then
			cat "$4"
			printWarning "\tcppcheck of $1 generated warnings!"
            printIt " "
		else
			if [ "$3" -eq 1 ]
			then
				cat "$4" 
			fi
			printSuccess "\tcppcheck of $1 is good."
			printIt " "
		fi
		popPath "$3"
	else
		printWarning "cppcheck doesn't appear to be installed."
	fi
}

# Function to check for scan-build
function hasScanBuild () {
    if cmdInstalled "scan-build"; then
        true
    else
        false
    fi
}

# Function to run scan-build
# <name> <path> <makefilename> <verbose> <log>
function scanIt () {
	if hasScanBuild
	then
		printIt "Checking $1 with scan-build..."
		pushPath "$2" "$4"
		scan-build make --makefile="$3" > "$5" 2>&1
		LOG_CONTENTS=`cat $"$5"`
		if stringHasSubstring "$LOG_CONTENTS" "error"
		then
			cat "$5"
			printError "\tscan-build of $1 generated errors!"
            printIt " "
		elif stringHasSubstring "$LOG_CONTENTS" "warning"
		then
			cat "$5"
			printWarning "\tscan-build of $1 generated warnings!"
            printIt " "
		else
			if [ "$4" -eq 1 ]
			then
				cat "$5"
			fi
			printSuccess "\tscan-build of $1 is good."
            printIt " "
		fi
		popPath "$4"
	else
		printWarning "scan-build doesn't appear to be installed."
	fi
}

# Function to check for valgrind
function hasValgrind () {
    if cmdInstalled "valgrind"; then
        true
    else
        false
    fi
}

# Function to get valgrind version
function valgrindVersion () {
	if hasValgrind
	then
		VALGRIND_VER="$(valgrind --version)"
		if stringBeginsWithSubstring "$VALGRIND_VER" "valgrind-"
		then
            VALGRIND_VER=${VALGRIND_VER%.*}
            VALGRIND_VER=${VALGRIND_VER##*-}
			echo $VALGRIND_VER
		else
			echo "unknown"
		fi
	else
		echo "N/A"
	fi
}

# =================================================================================================
