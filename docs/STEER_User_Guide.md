<p style="text-align:center;"><img src="./images/steer-blue-logo.png" width="50%"></p><a name="top"></a>

## The STandard Entropy Evaluation Report (STEER) Framework User Guide

##### 08-09-2024: Draft release - <span>A</span><span>l</span><span>e</span><span>x</span><span></span><span>a</span><span>n</span><span>d</span><span>e</span><span>r</span>@</span><span>a</span><span>n</span><span>a</span><span>m</span><span>e</span><span>t</span><span>r</span><span>i</span><span>c</span><span>.</span><span>c</span><span>o</span><span>m</span>

### Introduction

This user guide describes how to use the STEER (STandard Entropy Evaluation Report) Framework to evaluate entropy sources. STEER leverages the statistical tests first developed by the [National Institute of Standards and Technology](https://csrc.nist.gov/publications/detail/sp/800-22/rev-1a/final) for assessing the quality of random bit generators.

### Getting STEER
<!--
Website is currently down

Pre-built versions of STEER are available for Linux and macOS systems running on either Intel or ARM 64-bit systems. Multi-threaded versions of STEER are available for higher efficiency execution on multi-core computers. Download the appropriate version for your use at these links:

* [STEER for Linux Intel 64-bit systems](https://www.steer-framework.dev/downloads/STEER_0.1.0_linux_x64_Release.tar.gz)
* [STEER for Linux Intel 64-bit systems (multi-threaded)](https://www.steer-framework.dev/downloads/STEER_0.1.0_linux_x64_Release_mt.tar.gz)
* [STEER for Linux ARM 64-bit systems](https://www.steer-framework.dev/downloads/STEER_0.1.0_linux_arm64_Release.tar.gz)
* [STEER for Linux ARM 64-bit systems (multi-threaded)](https://www.steer-framework.dev/downloads/STEER_0.1.0_linux_arm64_Release_mt.tar.gz)
* [STEER for macOS Intel 64-bit systems](https://www.steer-framework.dev/downloads/STEER_0.1.0_macos_x64_Release.tar.gz)
* [STEER for macOS Intel 64-bit systems (multi-threaded)](https://www.steer-framework.dev/downloads/STEER_0.1.0_macos_x64_Release_mt.tar.gz)
* [STEER for macOS ARM 64-bit systems](https://www.steer-framework.dev/downloads/STEER.0.1.0_macos_arm64_Release.tar.gz)
* [STEER for macOS ARM 64-bit systems (multi-threaded)](https://www.steer-framework.dev/downloads/STEER.0.1.0_macos_arm64_Release_mt.tar.gz)

If you're interested in building STEER from source, please visit the [STEER Framework website](https://www.steer-framework.dev) for more information.
-->
There are currently no available pre-build versions of STEER. To get STEER, use:
`git clone SMU-DDI/steer-framework`

This will pull the STEER repository to your current working directory. 

### Installing STEER
<!-- 
Download is currently unavailable
To install STEER on Linux systems, open a terminal window and extract the archive to a location of your choosing, such as (for example) your home directory:

    cd ~
    tar -xvf ./STEER_0.1.0_linux_x64_Release.tar.gz

To install STEER on macOS systems, simply double-click the disk image (.dmg) file to mount it, and copy the STEER folder to your local drive.
-->

The STEER library can be build with `./build.sh`:

```
USAGE:  build.sh <args>

    All arguments are optional. With no arguments, the default behavior is:

    • Code analysis with cppcheck
    • Incremental debug build of programs and static libraries
    • No installation
    • Root directory path is '/Users/garyw'
    • No verbose output
    • Without console logging
    • Without documentation build
    • Without profiling
    • Without validation testing

    Possible argument values are:

    --analyze=<full|cppcheck|scan-build|valgrind>   Analyzes the source code with the specified tools.
    --check-env                                     Checks the build support on the host environment.
    --clean                                         Forces a clean build instead of an incremental build.
    --debug                                         Builds debug version.
    --help                                          Prints this usage notice.
    --install=<path>                                Installs release build to specified directory.
                                                    source code directory (defaults to the user's home directory).
    --release                                       Builds release version.
    --root-directory-path=<path>                    Sets the path to the root directory containing the STEER
                                                    source code directory (defaults to the user's home directory).
    --verbose                                       Prints all shell log output to console.
    --with-console-logging                          Log progress and debug information to the console.
    --with-documentation                            Builds documentation using Doxygen.
    --with-profiling                                Builds with profiling enabled (Linux only).
    --with-shared-libs                              Build and link with shared libraries instead of static libraries.
    --with-validation                               Validates test programs against known test configurations and results.

    Prerequisites for running this script include:

    • bash shell
    • clang or gcc with C99 support
    • cppcheck
    • Doxygen
    • gprof (used with --with-profiling option)
    • make
    • scan-build
    • valgrind
```

### Validating STEER Installation

First, open a terminal window and cd into the STEER directory:

    cd ./steer-framework

To validate that STEER is working properly, you can run a pre-defined test schedule, as shown below:

    ./bin/<os_platform>/steer_test_scheduler -s ./src/test-scheduler/validation_test_schedule.json

You should see this output similar to this in the terminal when the command completes:

	    Total test programs spawned: 80
	  Total test programs completed: 80
	Total test programs with errors: 0
	           Total execution time: 3 seconds

Next, run this program to validate the test program output:

    `./bin/steer_run_validations -c ./src/run-validations/validation_checks.json`

If everything is working properly, you should see output similar to this in the terminal:

	    Total validation programs spawned: 80
	  Total validation programs completed: 80
	Total validation programs with errors: 0
	                 Total execution time: 2 seconds

If any tests had errors or did not validate, please contact [bugs@steer-framework.dev](mailto:bugs@steer-framework.dev) to report the issue.

### Configuring STEER Tests

The easiest way to run STEER tests is to use the included `steer_test_scheduler` program, which uses a simple JSON format to specify test runs.

Below is a sample test schedule:

    {
        "schedule": {
            "schedule id": "sample schedule",
            "tests": [
                {
                    "program name": "nist_sts_approximate_entropy_test",
                    "profiles": [
                        {
                            "profile id": "sample profile",
                            "input": "./data/random.bin",
                            "parameters": "./test/multiple-bitstreams/nist-sts/approximate-entropy/parameters.json",
                            "report": "./results/approximate-entropy_test_report.json"
                        }
                    ]
                }
            ]   
        }
    }

A schedule always begins with a `schedule` object, which has at least two children: a `schedule id` object which identifies the schedule, and a `tests` array that contains one or more test objects.

A test object consists of two children: a `program name` object, which indicates the name of the test program executable to be run, and a `profiles` array that contains one or more profile objects. 

A profile object bundles the entropy input to be tested, the test parameters to be used, and the report output specifications for a test run. There are two forms of a profile object - one that handles files, and one that handles directories. Every profile object has a child `profile id` object, which uniquely identifies the profile within the test object, followed by three more child objects.

For file profile objects, the three child objects are:

* An `input` object, which specifies a file path to an entropy source;
* A `parameters` object, which specifies a file path to a parameter file; and
* A `report` object, which specifies a file path to write a report to.

For directory profile objects, the three child objects are:

* An `inputs directory` object, which specifies a directory containing one or more entropy source files;
* A `parameters directory` object, which specifies a directory containing one or more parameter files;
* A `reports directory` object, which specifies a directory to write reports into.

Note that when using the directory profile form, each unique combination of an input file and a parameter file will result in a report. For example, if the directory pointed to by `inputs directory` has 3 entropy files, and the directory pointed to by `parameters directory` has 4 parameter files, then a total of 12 reports (3 input files multiplied by 4 parameter files) will be produced. This construction allows for rapid scaling of test configurations.

Once you've created a schedule, you can invoke the `steer_test_scheduler` program to run it with this command:

    ./bin/steer_test_scheduler -c ./sample_schedule.json

### Interpreting STEER Reports

By default, STEER is configured to output a summary in STEER reports. These outputs are a JSON file format and can be viewed either via a text editor or a web browser. Each report consists of a test name, test descriptors such as the conductor, architecture, start time etc, test parameters, and test configurations. 

Each report lists different test ids associate with the report, as a single report can include multiple tests. A 'test' includes multiple criterion that must be met for the test to pass. These criterion are summarized with a 'pass' or 'fail' depending on the result of the test. The values of each metric used to calculate the 'pass' or 'fail' are outlined in the 'calculations' for each respective test id.


## STEER Utilities

STEER ships with an array of utilities to help the management of tests. Upon building the STEER framework, these utilities can be found in `steer-framework/bin/<os>/<build_type>/<utility>`.

These utilities inclue:
<ul>
   <li>Test Scheduler</li>
   <li>Test Validator</li>
   <!-- Validator links to steer-framework.dev which is currently a broken link <li>Schema Validator</li> -->
   <li>Automated Test Validator</li>
</ul>


### Test Scheduler

The test scheduler is used to automate the testing of vectors. After building the STEER library, the scheduler can be found in `steer-framework/bin/<os>/<build_type>/steer_test_scheduler`. The full usage of the test scheduler is as follows:

```
Usage: steer_test_scheduler <arguments>

        Available command line arguments are:

        -s, --schedule-file-path <path> Path to a file containing a JSON test schedule.
        -S, --schedule-json <json>      JSON test schedule.
        -h, --help                      Print usage help
```

The test scheduler is the recommended method for running tests, as it can be used to easily manage multiple tests and test results. Upon completion of the test schedule, the scheduler will output the number of spawned, completed, and erroneous tests, along with the total execution time.

### Test Validator
The test validator will compare an output test report from a STEER test program and determine if the the report matches the expected results. After building the STEER library, the validator can be found in `steer-framework/bin/<os>/<build-type>/steer-validate`. The fulle usage of the test validator is as follows:

```
Usage: steer-validate <arguments>

        -e, --expected-results-file-path    Path to an expected results JSON file.
        -r, --report-file-path              Path to a report JSON file.
        -h, --help                          Print usage help.
```

The output will list the validated reports and if the validation was succesful or not.

### Automated Test Validator

The automated test validator is used to automate STEER test validation. After building the STEER library, the scheduler can be found in `steer-framework/bin/<os>/<build_type>/steer-run-validations`. The full usage of the test validator is as follows:

```
Usage: steer_run_validations <arguments>

        Available command line arguments are:

        -c, --config-file-path <path>   Path to a file containing a JSON validation configuration.
        -C, --config-json <json         JSON validation configuration.
        -h, --help                      Print usage help
```

The test validator runs the `steer-validate` utility for each file in the JSON validation configuration. This is the recommended method for validating multiple tests. Upon completion of the validator, the number of spawned programs, completed programs, and erroneous programs, along with execution time, are output to the console.
 
---

STEER is an open source development project developed and managed by [Anametric, Inc.](https://www.anametric.com/) and the [Darwin Deason Institute of Cyber Security at Southern Methodist University](https://www.smu.edu/Lyle/Centers-and-Institutes/DDI).

<div class="row">
        <img src="./images/anametric-logo.png" width="35%" float="left">
        <img src="./images/ddics-logo.png" width="50%" float="right">
</div>

---

Copyright © 2022 Anametric, Inc.
