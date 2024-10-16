## STEER Framework SDK README
##### 2022.07.13 - Gary Woodcock, <span>g</span><span>a</span><span>r</span><span>y</span><span>@</span><span>a</span><span>n</span><span>a</span><span>m</span><span>e</span><span>t</span><span>r</span><span>i</span><span>c</span><span>.</span><span>c</span><span>o</span><span>m</span>
---
## Table of Contents
+ [About](#about)
+ [Getting Started](#getting_started)
+ [Usage](#usage)

## About <a name = "about"></a>
The primary goal of the STandard Entropy Evaluation Report (STEER) Framework is to improve the accessibility, usability and scalability of the NIST Statistical Test Suite (STS) while maintaining and extending its importance in evaluating entropy sources.

## Usage <a name = "usage"></a>

The following usage examples assume that you've extacted the STEER package to your home directory.

The first thing you should do is to validate the source code by running a known set of test vectors and comparing the results to known results (as published in NIST SP 800-22 Rav. 1a, Appendix B). There is a predefined test schedule that will run 5 different entropy files of one million bits through each of the 15 NIST STS tests (and each standard configuration of a test), or 75 test runs with a total of 940 unique test configurations. You can run this test using the `steer_test_scheduler` program with the predefined test schdule, as shown below:

    cd ~/STEER-SDK-0.1.0
    ./bin/steer_test_scheduler -s ./schedules/validation_test_schedule.json

This will output a summary like this to the console:

            Total test programs spawned: 75
          Total test programs completed: 75
        Total test programs with errors: 0
                   Total execution time: 4 seconds

If everything is working properly, you should see that 0 test programs had errors.

To check the test output generated by the `steer_test_scheduler` with the NIST STS test vectors, you can run the `steer_run_validations` program with a predefined validation check, as shown below:

    cd ~/STEER-SDK-0.1.0
    ./bin/steer_run_validations -c ./test/validation/nist-sts/validation_checks.json

This will enter a summary like this to the console:

            Total validation programs spawned: 75
          Total validation programs completed: 75
        Total validation programs with errors: 0
                         Total execution time: 73 seconds

If all is well, you should see that 0 validations had errors.

There is also a predefined test schedule that will run 100 different entropy files of one million bits through each of the 15 NIST STS tests (and each standard configuration of a test), or 15 test runs with a total of 18800 unique test configurations. You can run this test using the `steer_test_scheduler` program with the predefined test schdule, as shown below:

    cd ~/STEER-SDK-0.1.0
    ./bin/steer_test_scheduler -s ./schedules/benchmark_test_schedule.json

A summary of the test run will be printed to the console:

            Total test programs spawned: 15
          Total test programs completed: 15
        Total test programs with errors: 0
                   Total execution time: 151 seconds

To benchmark the NIST STS with the same data, use these commands:

    cd ./STEER-SDK-0.1.0/sts-2.1.2-reference
    ./benchmark-sts.sh

This will produce the output, shown below:

                G E N E R A T O R    S E L E C T I O N
                ______________________________________

         [0] Input File                 [1] Linear Congruential
         [2] Quadratic Congruential I   [3] Quadratic Congruential II
         [4] Cubic Congruential         [5] XOR
         [6] Modular Exponentiation     [7] Blum-Blum-Shub
         [8] Micali-Schnorr             [9] G Using SHA-1

        Enter Choice:

                     User Prescribed Input File:
                     S T A T I S T I C A L   T E S T S
                     _________________________________

         [01] Frequency                       [02] Block Frequency
         [03] Cumulative Sums                 [04] Runs
         [05] Longest Run of Ones             [06] Rank
         [07] Discrete Fourier Transform      [08] Nonperiodic Template Matchings
         [09] Overlapping Template Matchings  [10] Universal Statistical
         [11] Approximate Entropy             [12] Random Excursions
         [13] Random Excursions Variant       [14] Serial
         [15] Linear Complexity

              INSTRUCTIONS
                 Enter 0 if you DO NOT want to apply all of the
                 statistical tests to each sequence and 1 if you DO.

        Enter Choice:
             P a r a m e t e r   A d j u s t m e n t s
             -----------------------------------------
         [1] Block Frequency Test - block length(M):         128
         [2] NonOverlapping Template Test - block length(m): 9
         [3] Overlapping Template Test - block length(m):    9
         [4] Approximate Entropy Test - block length(m):     10
         [5] Serial Test - block length(m):                  16
         [6] Linear Complexity Test - block length(M):       500

        Select Test (0 to continue):
        How many bitstreams?
        Input File Format:
         [0] ASCII - A sequence of ASCII 0's and 1's
         [1] Binary - Each byte in data file contains 8 bits of data

        Select input mode:
         Statistical Testing In Progress.........

         Statistical Testing Complete!!!!!!!!!!!!

           Start: Wed Jul 13 16:50:47 CDT 2022
            Stop: Wed Jul 13 16:53:03 CDT 2022
    Elapsed time: 136 seconds

To learn more about using STEER, please refer to the STEER User Guide, included in this SDK.

To learn more about how to develop tests with STEER, please refer to the STEER Developers Guide, included in this SDK.

---
###### Copyright (c) 2024 Anametric, Inc. All rights reserved.
