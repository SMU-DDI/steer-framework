# =================================================================================================
#
#   bash_c_build_utils.sh
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

# Function to check for clang
function hasClang () {
	if cmdInstalled "clang"; then
		true
	else
		false
	fi
}

# Function to get clang version
function clangVersion () {
	if hasClang
	then
		CLANG_VER="$(clang --version)"
		if isDarwin
		then
			if stringBeginsWithSubstring "$CLANG_VER" "Apple clang version "
			then
                CLANG_VER=${CLANG_VER#*clang version *}
                CLANG_VER=${CLANG_VER% (*}
				echo $CLANG_VER
			else
				echo "unknown"
			fi
		else
			if stringBeginsWithSubstring "$CLANG_VER" "clang version "
            then
                CLANG_VER=${CLANG_VER#*clang version *}
                CLANG_VER=${CLANG_VER::6}
                echo $CLANG_VER
            else
                echo "unknown"
            fi
		fi
	else
		echo "N/A"
	fi
}

# Function to get Clang standard C version
function clangStdCVersion () {
	if hasClang
	then
		CLANG_DEFS=$(clang -dM -E - < /dev/null)
		C_STD_VERSION=${CLANG_DEFS#*__STDC_VERSION__}
		C_STD_VERSION=${C_STD_VERSION::7}
	else
		echo "N/A"
	fi
}

# Function to check for C standard support
function clangSupportsCStandard () {
	if hasClang
	then
		C_STD_VERSION=0
		clangStdCVersion
		C_STD_YEAR=${C_STD_VERSION::5}
		C_STD_MONTH=${C_STD_VERSION:5:2}
		if [ "$C_STD_YEAR" -ge "$1" ]; then
			true
		else
			false
		fi
	else
		false
	fi
}

# Function to check for clang C99 support
function clangSupportsC99 () {
    clangSupportsCStandard "1999"
}

# Function to check for clang C11 support
function clangSupportsC11 () {
    clangSupportsCStandard "2011"
}

# Function to check for clang C17 support
function clangSupportsC17 () {
    clangSupportsCStandard "2017"
}

# Function to check for G++
function hasG++ () {
	if cmdInstalled "g++"; then
		true
	else
		false
	fi
}

# Function to check for GCC
function hasGcc () {
	if ! isDarwin
	then
		if cmdInstalled "gcc"; then
			true
		else
			false
		fi
	else
		false
	fi
}

# Function to get GCC version
function gccVersion () {
	if hasGcc
	then
		GCC_VER="$(gcc --version)"
		if stringBeginsWithSubstring "$GCC_VER" "gcc "
		then
			GCC_VER="$(echo $GCC_VER | cut -d$' ' -f 4)"
			echo $GCC_VER
		else
			echo "unknown"
		fi
	else
		echo "N/A"
	fi
}

# Function to get GCC standard C version
function gccStdCVersion () {
	if hasGcc
	then
		GCC_DEFS=$(gcc -dM -E - < /dev/null)
		C_STD_VERSION=${GCC_DEFS#*__STDC_VERSION__}
		C_STD_VERSION=${C_STD_VERSION::7}
	else
		echo "N/A"
	fi
}

# Function to check for C standard support
function gccSupportsCStandard () {
	if hasGcc
	then
		C_STD_VERSION=0
		gccStdCVersion
		C_STD_YEAR=${C_STD_VERSION::5}
		C_STD_MONTH=${C_STD_VERSION:5:2}
		if [ "$C_STD_YEAR" -ge "$1" ]; then
			true
		else
			false
		fi
	else
		false
	fi
}

# Function to check for GCC C99 support
function gccSupportsC99 () {
    gccSupportsCStandard "1999"
}

# Function to check for GCC C11 support
function gccSupportsC11 () {
    gccSupportsCStandard "2011"
}

# Function to check for GCC C17 support
function gccSupportsC17 () {
    gccSupportsCStandard "2017"
}

# Function to check for GNU ARM embedded toolchain
function hasGnuArmEmbeddedToolchain () {
	if cmdInstalled "arm-none-eabi-gcc"; then
		true
	else
		false
	fi
}

# Function to get GNU ARM embedded GCC version
function gnuArmEmbeddedGccStdCVersion () {
	GNU_ARM_DEFS=$(arm-none-eabi-gcc -dM -E - < /dev/null)
	GNU_ARM_STD_C_VERSION=${GNU_ARM_DEFS#*__STDC_VERSION__}
	GNU_ARM_STD_C_VERSION=${GNU_ARM_STD_C_VERSION::6}
}

# Function to check for GNU ARM embedded C standard support
function gnuArmEmbeddedGccSupportsCStandard () {
	GNU_ARM_STD_C_VERSION=0
	gnuArmEmbeddedGccStdCVersion
	GNU_ARM_STD_C_YEAR=${GNU_ARM_STD_C_VERSION:0:5}
	GNU_ARM_STD_C_MONTH=${GNU_ARM_STD_C_VERSION:5:7}
	if [ "$GNU_ARM_STD_C_YEAR" -ge "$1" ]; then
		true
	else
		false
	fi
}

# Function to check for GSL
function hasGsl () {
    if cmdInstalled "gsl-config"; then
        true
    else
        false
    fi
}

# Function to get GSL version
function gslVersion () {
    if hasGsl
    then
        GSL_VER="$(gsl-config --version)"
        echo $GSL_VER
    else
        echo "N/A"
    fi
}

# Function to check for make
function hasMake () {
	if cmdInstalled "make"; then
		true
	else
		false
	fi
}

# Function to get make version
function makeVersion () {
	if hasMake
	then
		MAKE_VER="$(make --version)"
		if stringBeginsWithSubstring "$MAKE_VER" "GNU Make "
		then
            MAKE_VER=${MAKE_VER%Built for*}
            MAKE_VER=${MAKE_VER%Copyright*}
            MAKE_VER=${MAKE_VER#GNU Make}
			echo $MAKE_VER
		else
			echo "unknown"
		fi
	else
		echo "N/A"
	fi
}

# Function to check for Git
function hasGit () {
	if cmdInstalled "git"; then
		true
	else
		false
	fi
}

# Function to get git version
function gitVersion () {
    if hasGit
    then
        GIT_VER="$(git version)"
        if stringBeginsWithSubstring "$GIT_VER" "git version "
        then
            GIT_VER=${GIT_VER#git version}
            echo $GIT_VER
        else
            echo "unknown"
        fi
    else
        echo "N/A"
    fi
}

# Function to check for gprof
function hasGprof () {
	if cmdInstalled "gprof"; then
		true
	else
		false
	fi
}

# Function to get gprof version
function gprofVersion () {
    if hasGprof
    then
        GPROF_VER="$(gprof --version)"
        if stringBeginsWithSubstring "$GPROF_VER" "GNU gprof"
        then   
            GPROF_VER=${GPROF_VER%Based on*}
            GPROF_VER=${GPROF_VER#*) }
            echo $GPROF_VER
        else
            echo "Unknown"
        fi
    else
        echo "N/A"
    fi
}

# Function to clean 
# <name> <path> <makefilename> <verbose> <log>
function cleanIt () {
	printIt "Cleaning $1..."
	pushPath "$2" "$4"
	make clean --makefile="$3" > "$5" 2>&1
	if [ "$?" -ne 0 ]
	then
		cat "$5"
		popPath "$4"
		printIt " "
		printError "\tClean of $1 failed!\n"
		exit 1
	else
		if [ "$4" -eq 1 ]
		then
			cat "$5"
		fi
        printSuccess "\tClean of $1 succeeded.\n"
	fi
	popPath "$4"
}

# Function to build
# <name> <path> <makefilename> <verbose> <log> <defines>
function buildIt () {
	printIt "Building $1..."
	pushPath "$2" "$4"
	if [ "$6" == "" ]
	then
		make --makefile="$3" > "$5" 2>&1
	else
		make "$6" --makefile="$3" > "$5" 2>&1
	fi
	if [ "$?" -ne 0 ]
	then	
		cat "$5"
		popPath "$4"
		printIt " "
		printError "\tBuild of $1 failed!\n"
		exit 1
	else
		if [ "$4" -eq 1 ]
		then
			cat "$5"
		fi
        printSuccess "\tBuild of $1 succeeded.\n"
	fi
	popPath "$4"
}

# =================================================================================================
