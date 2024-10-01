# =================================================================================================
#
#   bash_package_manager_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of package manager functions.
#
# =================================================================================================

# Function to check for APT
function hasAdvancedPackagingTool () {
    if isLinux 
    then
        if cmdInstalled "apt"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to get APT version
function advancedPackagingToolVersion () {
    if hasAdvancedPackagingTool
    then
        APT_VER="$(apt --version)"
        if stringBeginsWithSubstring "$APT_VER" "apt "
        then
            APT_VER=${APT_VER%(*}
            APT_VER=${APT_VER#apt }
            echo $APT_VER
        else
            echo "unknown"
        fi
    else
        echo "N/A"
    fi
}

# Function to check for Aptitude Package Manager
function hasAptitudePackageManager () {
    if isLinux 
    then
        if cmdInstalled "aptitude"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to check DNF
function hasDandifiedYum () {
    if isLinux 
    then
        if cmdInstalled "dnf"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to check for DPKG
function hasDebianPackageManagementSystem () {
    if isLinux 
    then
        if cmdInstalled "dpkg"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to check for Homebrew
function hasHomebrew () {
    if isDarwin
    then
        if cmdInstalled "brew"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to check for MacPorts
function hasMacPorts () {
	if isDarwin
	then
		if cmdInstalled "port"; then
			true
		else
			false
		fi
	else
		false
	fi
}

# Function to get MacPorts version
function macPortsVersion () {
    if hasMacPorts
    then
        MACPORTS_VER="$(port version)"
        if stringBeginsWithSubstring "$MACPORTS_VER" "Version: "
        then
            MACPORTS_VER=${MACPORTS_VER#Version: }
            echo $MACPORTS_VER
        else
            echo "unknown"
        fi
    else
        echo "N/A"
    fi

}

# Function to check for Pacman Package Manager
function hasPacmanPackageManager() {
    if isLinux 
    then
        if cmdInstalled "pacman"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to check for Portage Package Manager
function hasPortagePackageManager () {
    if isLinux 
    then
        if cmdInstalled "emerge"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to check for RPM
function hasRedHatPackageManager () {
    if isLinux 
    then
        if cmdInstalled "rpm"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to check for YUM
function hasYellowDogUpdaterModified () {
    if isLinux 
    then
        if cmdInstalled "yum"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# Function to check for Zypper Package Manager
function hasZypperPackageManager () {
    if isLinux 
    then
        if cmdInstalled "zypper"; then
            true
        else
            false
        fi
    else
        false
    fi
}

# =================================================================================================