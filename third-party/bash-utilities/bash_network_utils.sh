# =================================================================================================
#
#   bash_network_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash network support functions.
#
# =================================================================================================

# Function to check for cURL
function hasCurl () {
	if isDarwin
	then
		if hasMacPorts
		then
			CURL_INSTALLED=$(port installed | grep curl)
			if stringHasSubstring "$CURL_INSTALLED" "curl"; then
				true
			else
				false
			fi
		else
			false
		fi
	elif isLinux
	then
		CURL_INSTALLED=$(ldconfig -p | grep libcurl)
		if stringHasSubstring "$CURL_INSTALLED" "libcurl"; then
			true
		else
			false
		fi
	else
		false
	fi
}

# Function to check for ping
function hasPing () {
	if cmdInstalled "ping"; then
		true
	else
		false
	fi
}

# Function to check for iperf3
function hasIperf3 () {
	if cmdInstalled "iperf3"; then
		true
	else
		false
	fi
}

# Function to check IPv4 address with ping
function pingCheck () {

	CHECK=$(ping -c 1 "$1")
	if stringHasSubstring "$CHECK" "100.0% packet loss"
	then
		false
	else
		true
	fi
}

# Test an IP address for validity:
# From https://www.linuxjournal.com/content/validating-ip-address-bash-script
# Usage:
#      valid_ip IP_ADDRESS
#      if [[ $? -eq 0 ]]; then echo good; else echo bad; fi
#   OR
#      if valid_ip IP_ADDRESS; then echo good; else echo bad; fi
#
function valid_ip () {

    local  ip=$1
    local  stat=1

    if [[ $ip =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
        OIFS=$IFS
        IFS='.'
        ip=($ip)
        IFS=$OIFS
        [[ ${ip[0]} -le 255 && ${ip[1]} -le 255 \
            && ${ip[2]} -le 255 && ${ip[3]} -le 255 ]]
        stat=$?
    fi
    return $stat
}

# Function to check for dumpcap
function hasDumpcap () {
	if cmdInstalled "dumpcap"; then
		true
	else
		false
	fi
}

# Function to check for tshark
function hasTshark () {
	if cmdInstalled "tshark"; then
		true
	else
		false
	fi
}

# =================================================================================================
