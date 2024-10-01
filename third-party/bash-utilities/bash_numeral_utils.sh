# =================================================================================================
#
#   bash_numeral_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash numeral support functions.
#
# =================================================================================================

# Function to test whether input is an integer
function isInteger () {
	if [ "$1" -eq "$1" 2>/dev/null ]
	then
		true
	else
		false
	fi
}

# Function to test whether input is a positive integer
function isPositiveInteger () {
	if isInteger "$1"
	then
		if [ "$1" -ge "0" ]
		then
			true
		else
			false
		fi
	else
		false
	fi
}

# Function to test whether input is an integer between x and y
function isIntegerBetween () {
	if isInteger "$1"
	then
		if isInteger "$2" 
		then
			if isInteger "$3"
			then
				if [ "$1" -ge "$2" ]
				then
					if [ "$1" -le "$3" ]
					then
						true
					else
						false
					fi
				else
					false
				fi
			else
				false
			fi
		else
			false
		fi
	else
		false
	fi
}

# =================================================================================================

