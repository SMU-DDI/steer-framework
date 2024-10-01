# =================================================================================================
#
#   bash_unit_test_utils.sh
#
#   Copyright (c) 2019 Unthinkable Research LLC. All rights reserved.
#
#	Author: Gary Woodcock
#
#   Supported host operating systems:
#       *nix systems capable of running bash shell.
#
#	Description:
#		This file contains a collection of bash unit test support functions.
#
# =================================================================================================

# Function to check for CUnit
function hasCUnit () {
    if isDarwin
    then
        if hasMacPorts || hasHomebrew
        then
            if directoryExists "/opt/local/include/CUnit"
            then
                true
            else
                false
            fi
        else
            false
        fi
	elif isLinux
    then
		CUNIT_INSTALLED=$(ldconfig -p | grep libcunit)
		if stringHasSubstring "$CUNIT_INSTALLED" "libcunit"; then
			true
		else
			false
		fi
    else
        false
	fi
}

# Function to parse CUnit test results XML file and print a summary
function parseCUnitResults()
{
    SUITES_TOTAL=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Suites ']]/TOTAL/text()" -)
    SUITES_TOTAL=$(trimLeadingAndTrailingWhitespace "$SUITES_TOTAL")
    SUITES_RUN=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Suites ']]/RUN/text()" -)
    SUITES_RUN=$(trimLeadingAndTrailingWhitespace "$SUITES_RUN")
    SUITES_SUCCEEDED=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Suites ']]/SUCCEEDED/text()" -)
    SUITES_SUCCEEDED=$(trimLeadingAndTrailingWhitespace "$SUITES_SUCCEEDED")
    SUITES_FAILED=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Suites ']]/FAILED/text()" -)
    SUITES_FAILED=$(trimLeadingAndTrailingWhitespace "$SUITES_FAILED")
    
    TEST_CASES_TOTAL=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Test Cases ']]/TOTAL/text()" -)
    TEST_CASES_TOTAL=$(trimLeadingAndTrailingWhitespace "$TEST_CASES_TOTAL")
    TEST_CASES_RUN=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Test Cases ']]/RUN/text()" -)
    TEST_CASES_RUN=$(trimLeadingAndTrailingWhitespace "$TEST_CASES_RUN")
    TEST_CASES_SUCCEEDED=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Test Cases ']]/SUCCEEDED/text()" -)
    TEST_CASES_SUCCEEDED=$(trimLeadingAndTrailingWhitespace "$TEST_CASES_SUCCEEDED")
    TEST_CASES_FAILED=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Test Cases ']]/FAILED/text()" -)
    TEST_CASES_FAILED=$(trimLeadingAndTrailingWhitespace "$TEST_CASES_FAILED")

    if [ $TEST_CASES_SUCCEEDED -eq $TEST_CASES_TOTAL ]
    then
        if [ $TEST_CASES_SUCCEEDED -eq 1 ]
        then
            printSuccess "\t$TEST_CASES_SUCCEEDED test of $TEST_CASES_TOTAL test total succeeded."
        else
            printSuccess "\t$TEST_CASES_SUCCEEDED tests of $TEST_CASES_TOTAL tests total succeeded."
        fi
    else
        if [ $TEST_CASES_FAILED -eq 1 ]
        then
            if [ $TEST_CASES_TOTAL -eq 1 ]
            then
                printError "\t$TEST_CASES_FAILED test of $TEST_CASES_TOTAL test total failed."
            else
                printError "\t$TEST_CASES_FAILED test of $TEST_CASES_TOTAL tests total failed."
            fi
        else
            printError "\t$TEST_CASES_FAILED tests of $TEST_CASES_TOTAL tests total failed."
        fi
    fi

    ASSERTIONS_TOTAL=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Assertions ']]/TOTAL/text()" -)
    ASSERTIONS_TOTAL=$(trimLeadingAndTrailingWhitespace "$ASSERTIONS_TOTAL")
    ASSERTIONS_RUN=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Assertions ']]/RUN/text()" -)
    ASSERTIONS_RUN=$(trimLeadingAndTrailingWhitespace "$ASSERTIONS_RUN")
    ASSERTIONS_SUCCEEDED=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Assertions ']]/SUCCEEDED/text()" -)
    ASSERTIONS_SUCCEEDED=$(trimLeadingAndTrailingWhitespace "$ASSERTIONS_SUCCEEDED")
    ASSERTIONS_FAILED=$(printf '%s\n' "$1" |
        xmllint --xpath "//CUNIT_RUN_SUMMARY_RECORD[TYPE[text()=' Assertions ']]/FAILED/text()" -)
    ASSERTIONS_FAILED=$(trimLeadingAndTrailingWhitespace "$ASSERTIONS_FAILED")
}

# =================================================================================================
