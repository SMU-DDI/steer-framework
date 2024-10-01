# =================================================================================================
#
#   bash_string_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash string functions.
#
# =================================================================================================

# Function to check string for substring
function stringHasSubstring () {
	if [[ "$1" == *"$2"* ]]; then
		true 
	else
		false
	fi
}

# Function to check if string begins with substring
function stringBeginsWithSubstring () {
	if [[ "$1" == "$2"* ]]; then
		true
	else
		false
	fi
}

# Function to check if string ends with substring
function stringEndsWithSubstring () {
	if [[ "$1" == *"$2" ]]; then
		true
	else
		false
	fi
}

# Function to remove leading substring
function removeLeadingSubstring () {
	if stringBeginsWithSubstring "$1" "$2"
	then
		shopt -s extglob
		echo "${1##$2}"
	fi
}

# Function to remove trailing substring
function removeTrailingSubstring () {
	if stringEndsWithSubstring "$1" "$2"
	then
		shopt -s extglob
		echo "${1%%$2}"
	fi
}

# Function to convert decimal byte to hex byte
function convertDecimalByteToHexByte {
	printf "%02X" $1
}


# Function to get date in month/day/year number format
function getDateString {
	date "+%m/%d/%y"
}

# Function to get date/time in month-day-year-hour-minute-second format
function getDateTimeString {
	DTS=$(date "+%m-%d-%y at %T")
	echo "${DTS//:/$'.'}"
}

# Function to get duration string from seconds
function getDurationString {
	DURATION=$1
    SECONDS_PER_WEEK=604800
    SECONDS_PER_DAY=86400
	SECONDS_PER_HOUR=3600
	SECONDS_PER_MINUTE=60

    WEEKS=$(( $DURATION / $SECONDS_PER_WEEK ))
    WEEKS_IN_SECS=$(( $WEEKS * $SECONDS_PER_WEEK ))
    DURATION=$(( $DURATION - $WEEKS_IN_SECS ))

    DAYS=$(( $DURATION / $SECONDS_PER_DAY ))
    DAYS_IN_SECS=$(( $DAYS * $SECONDS_PER_DAY ))
    DURATION=$(( $DURATION - $DAYS_IN_SECS ))

	HOURS=$(( $DURATION / $SECONDS_PER_HOUR ))
	HOURS_IN_SECS=$(( $HOURS * $SECONDS_PER_HOUR ))
	DURATION=$(( $DURATION - $HOURS_IN_SECS ))

	MINUTES=$(( DURATION / $SECONDS_PER_MINUTE ))
	MINUTES_IN_SECS=$(( $MINUTES * $SECONDS_PER_MINUTE ))
	SECONDS=$(( $DURATION - $MINUTES_IN_SECS ))
	
    if [ $WEEKS -gt 0 ]
    then
        if [ $WEEKS -eq 0 ]
        then
            echo -n "1 week "
        else
            echo -n "$WEEKS weeks "
        fi
    fi

    if [ $DAYS -gt 0 ]
    then
        if [ $DAYS -eq 0 ]
        then
            echo -n "1 day "
        else
            echo -n "$DAYS days "
        fi
    fi

    if [ $HOURS -gt 0 ]
    then
        if [ $HOURS -eq 1 ]
        then
            echo -n "1 hour "
        else
            echo -n "$HOURS hours "
        fi
    fi

    if [ $MINUTES -gt 0 ]
    then
        if [ $MINUTES -eq 1 ]
        then
            echo -n "1 minute "
        else
            echo -n "$MINUTES minutes "
        fi
    fi

    if [ $SECONDS -gt 0 ]
    then
        if [ $SECONDS -eq 1 ]
        then
            echo "1 second"
        else
            echo "$SECONDS seconds"
        fi
    else
        echo "0 seconds"
    fi
}

# Function to remove leading and trailing whitespace from a string
function trimLeadingAndTrailingWhitespace {
    local STR_VAR="$*"

    # Remove leading whitespace
    STR_VAR="${STR_VAR#"${STR_VAR%%[![:space:]]*}"}"

    # Remove trailing whitespace
    STR_VAR="${STR_VAR%"${STR_VAR##*[![:space:]]}"}"  

    printf '%s' "$STR_VAR"
}

# Function to get indented string
function getIndentedString {
    for (( i=1; i<${2}; i++ ))
    do
        echo -n " "
    done
    echo -n "${1}"
}

# =================================================================================================

