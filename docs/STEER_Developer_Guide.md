<p style="text-align:center;"><img src="./images/steer-blue-logo.png" width="50%"></p><a name = "top"></a>

## The STandard Entropy Evaluation Report (STEER) Framework Developer Guide

##### 08-09-2024: Draft release - <span>A</span><span>l</span><span>e</span><span>x</span><span>a</span><span>n</span><span>d</span><span>e</span><span>r</span><span>@</span><span>a</span><span>n</span><span>a</span><span>m</span><span>e</span><span>t</span><span>r</span><span>i</span><span>c</span><span>.</span><span>c</span><span>o</span><span>m</span>

# Setting up Docker

## Download Docker Desktop
Download [docker desktop.](https://www.docker.com/products/docker-desktop/) 
Note that when installing docker you have to allocate some disk space for it. This can be done during startup but if not you can do it later in settings. 
The settings used shown in the screenshot below, but in no way are these configurations necessary. Lesser values can be used depending on your system limitations.
A virual disk limit of 128 gigs is probably overkill.
32 GB and above are recommended; you can probably get away with 16, you'll just have to clear out stale containers more often.
Note for Windows users: Windows prefers to manage the storage allocation settings.  So, although you can set these, you have to choose a 'non-recommended' option during installation or afterwards via the settings.  I allowed Windows to manage the storage allocation, so did not manually adjust these settings.

## Building and Running your Docker container
Keeping it simple, Docker is essentially a mini terminal-based VM.
In reality it's a level of hardware abstraction that's not as serious as a VM. 
This means that Docker containers are much more portable and their configurations can be easily shared regardless of operating system.

Essentially a Docker container must be built and then it can be ran.

1. Make sure the Docker desktop is running, i.e. the application is just running on your desktop
2. Download the [Armadillo zip file](https://sourceforge.net/projects/arma/files/armadillo-12.6.7.tar.xz/download)
3. Download the steer library and move the Dockerfile out of the STEER framework directory such that it's at the same directory level (we can change this later to make a more seemlesss process). Directory should look like the below screenshot:
4. In terminal, navigate to the directory where your Dockerfile is stored and run the following command `DOCKER_BUILDKIT=0 docker build -t <INSERT_DESIRED_CONTAINER_TAG> -f Dockerfile .` The DOCKER_BUILDKIT=0 just gives you better interactibility, i cant remember the exact reason i added it, but in theory it works without this parameter. The -t followed by the tag name is just an identifier. The -f followed by Dockerfile is indicating to use the file called Dockerfile. And then the `.` indicates to look in the current directory. Example of what this command looks like: `DOCKER_BUILDKIT=0 docker build -t docker_steer_test -f Dockerfile .`
Note for Windows users: Leave off the `DOCKER_BUILDKIT` bit.  So, your command should follow this example: `docker build -t docker_steer_test -f Dockerfile .`
5. Let this build and hopefully it goes smoothly
6. Next you have to run the container that you just made. You do that with the following command: `docker run -it --name <INSERT_NAME_FOR_THIS_RUNTIME> -v $(pwd):/src <INSERT_CONTAINER_TAG>`. The -it is so that you can actually use terminal in the container. the --name is to name the runtime. Then the $(pwd):/src binds your working directory to the docker container. This allows you to edit files your desired editor is, with the changes seamlessly propogating into the container. Example of this command: `docker run -it --name runtime-test -v $(pwd):/src docker_steer_test`
Note for Windows users: Swap `$(pwd)` for `"%cd%"`; Windows does not use the standard `pwd` command.  
7. Now your container should be running. Run `cd src` and then if you run `ls` you should see your steer directory. 

## Check that STEER is working
- `./build.sh --check-env`
- `./build.sh --clean --debug --with-validation`
Note for Windows users: If the above commands fail with somewhat cryptic errors, open the `build.sh` file using an editor and change all of the 'Windows newlines' to Unix newlines, either via find/replace (replace "\r\n" with "\n") or the "Edit" menu that allows for globally switching to Unix-style newlines.  If you encounter further cryptic errors, navigate to steer-framework/third-party/bash-utilities, and change the Windows newlines in every one of the bash scripts there, as well.

# Adding Test Instructions

Before starting, verify that the Docker container is set up properly. Start the Docker container and ensure that its file structure has access or is linked to the local STEER framework repository

1. Tests can be automatically added via the test-adder utility. To run the 
test-adder first build steer with validation:

```
./build.sh --debug --with-validation
```

You can use the output of the test validation to ensure that the tests were 
added properly. Take note of the total tests spawned/completed. We should 
see this value increase once the tests have been added.
```
******************                                          
*** VALIDATION ***                                          
******************                                          
                                                            
Running validation tests...                                 
                                                            
            Total test programs spawned: 75                 
          Total test programs completed: 75                 
        Total test programs with errors: 0                  
                   Total execution time: 3 seconds          
                                                            
Checking validation test results...                         
                                                            
            Total validation programs spawned: 75           
          Total validation programs completed: 75           
        Total validation programs with errors: 0            
                         Total execution time: 1 seconds   
                                                          
```

2. Run the test adder application:

```
Usage: steer_test_adder <arguments>                                                         
                                                                                            
        Available command line arguments are:                                              
                                                                                           
        -s, --build-folder-path <path>  Path to the steer parent directory.                 
        -t, --test-name <test-name>     Name of the new test.                               
        -f, --test-folder <test-folder> Folder to place the test in. Defaults to test name. 
        -h, --help                      Prints this usage notice.   
```

For the test name, we can use spaces. To follow the recommended naming 
convention, use all lower case letters for the test name. For our example, 
we will add "dummy test".

```
/Projects/steer/steer-framework# ./bin/linux/x64/Debug/steer_test_adder -t "dummy test" -s . 
                                                                                 
        New folders created:                                                    
                ./src/nist-sts/dummy-test                                      
                ./test/validation/nist-sts/dummy-test                            
        New files created:                                                       
                ./test/validation/nist-sts/dummy-test/expected_results_e.json 
                ./test/validation/nist-sts/dummy-test/parameters_e.json        
                ./test/validation/nist-sts/dummy-test/expected_results_pi.json  
                ./test/validation/nist-sts/dummy-test/parameters_pi.json        
                ./test/validation/nist-sts/dummy-test/expected_results_sha1.json 
                ./test/validation/nist-sts/dummy-test/parameters_sha1.json       
                ./test/validation/nist-sts/dummy-test/expected_results_sqrt2.json
                ./test/validation/nist-sts/dummy-test/parameters_sqrt2.json      
                ./test/validation/nist-sts/dummy-test/expected_results_sqrt3.json
                ./test/validation/nist-sts/dummy-test/parameters_sqrt3.json      
        Modified files:                                                          
                ./src/run-validations/validation_checks.json                     
                ./src/test-scheduler/validation_test_schedule.json              
                ./build_files/test_names.txt                                     
                ./build_files/test_folder_names.txt 
```

3. The test adder adds all necessary functionality to run a test, including the 
build settings in ./build.sh, the modification to test scheduler, and the 
modification to the validation checks. The a new test template is create in 
/src/testname. This includes a single-thread and multi-thread template which is
a replica of the 'approximate-entropy' test. 

4. We can verify the success of the test generation by rerunning the build script
with validation. In the build output, we can see our new test being built:

```
Building nist_sts_dummy_test_test...               
        Build of nist_sts_dummy_test_test succeeded.
```

We can also see the new tests being run and completed:
```
******************                                       
*** VALIDATION ***                                     
******************                                     
                                                       
Running validation tests...                            
                                                       
            Total test programs spawned: 80            
          Total test programs completed: 80            
        Total test programs with errors: 0             
                   Total execution time: 2 seconds     
                                                       
Checking validation test results...                    
                                                      
            Total validation programs spawned: 80      
          Total validation programs completed: 80      
        Total validation programs with errors: 0       
                         Total execution time: 2 seconds
   
```

To customize the tests, modified the files in ./src/nist-sts/test-name.

If you are only coding in C, you can stop here.

5. If you are coding in C++. 
- In the src/nist-sts/test-name you'll make two new files: a `cpp_c_connector.cpp` and `cpp_c_connector.h`. The `.h` file will look something like the following with the most likely deviation being the parameters you list. Note that `ourBuffer` will hold the actual raw data for the test:
```
#include <stdint.h>
#ifndef CPP_C_CONNECTOR_H 
#define CPP_CONNECTOR_H 

#ifdef __cplusplus
extern "C" {
#endif

void c_wrapper(int bitstreamLength,  uint8_t* ourBuffer);

#ifdef __cplusplus
}
#endif

#endif
```
The `.c` file will look something like the following. Your parameters might deviate from this template. Your function call from within the wrapper should have a different name matching the function which will discussed in the next step:
```
#include <cstdlib>

#include "cpp_c_connector.h"
#include "granger_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

// Inside this "extern C" block, I can implement functions in C++, which will externally 
//   appear as C functions (which means that the function IDs will be their names, unlike
//   the regular C++ behavior, which allows defining multiple functions with the same name
//   (overloading) and hence uses function signature hashing to enforce unique IDs),


void c_wrapper(int bitstreamLength, uint8_t* ourBuffer) {
    // lazyAAA();
    grangerCausalityTest(ourBuffer, bitstreamLength);

}

#ifdef __cplusplus
}
#endif
```
- In the same directory you can add `.h` and `.cpp` files for your custom test C++ test. The main function should match the function call in the `c_wrapper function` from above. For example in `granger_driver.h` and `granger_driver.cpp` for the Granger causality test, the main function sits inside of `granger_driver` files and is called `grangerCausalityTest`.
- Now you have to compile the C++ code and link it with the connector code. These steps can be combined in a script that is called for the Makefile, _or_ they may be performed manually.
- As the script makes things simpler and less error-prone, we recommend that route and describe it first:
- This example is from the added Granger Causality test; the bash script compiles the C++ and puts it in a shared object such that it can accessed via the C STEER test. There will be two `.so` files; they will be moved into the appropriate debug/release directory. We advise creating a bash script similar to this:
```
#!/bin/bash

# Compile the files into shared libraries
g++ -fPIC -shared final_granger_result.h granger_driver.cpp granger_causality.cpp granger_causality.h granger_matrix_experiment.cpp granger_matrix_experiment.h irls.cpp irls.h granger_result.h -o libgranger.so -larmadillo -lboost_system -Wl,-rpath,$PWD
g++ -fPIC -shared cpp_c_connector.cpp -L$PWD -lgranger -o libcpp_c_connector.so -Wl,-rpath,$PWD

# Check if the compilation succeeded
if [ $? -eq 0 ]; then
    echo "Compilation successful."
    # Create the target directory if it does not exist
    mkdir -p ../../../bin/linux/x64/Debug

    # Copy the .so files to the target directory
    cp libgranger.so ../../../bin/linux/x64/Debug/
    cp libcpp_c_connector.so ../../../bin/linux/x64/Debug/

    echo "Files copied successfully."
else
    echo "Compilation failed."
fi
```
- This bash script should then be executed from your STEER test's makefile. We define a `RUNSCRIPT` variable which is `BUILD_CFG` dependent and is executed prior to the link as illustrated below:
```
ifeq ($(BUILD_CFG),Release)
RUNSCRIPT=./cpp_compile_link_release.sh
```
```
$(OUTFILE): $(OUTDIR)  $(OBJ)
	$(RUNSCRIPT)
	$(LINK)

```

- If you use the above script method, you do _not_ need to continue with the following steps. But to instead perform the C++ compilation and linking manually, use the example of a sample test class:
  - `g++ -fPIC -shared test.cpp -o libtest.so -Wl,-rpath,$PWD`
  - `g++ -fPIC -shared test_c_connector.cpp -L$PWD -ltest -o libtest_c_connector.so -Wl,-rpath,$PWD`
- Here is what these commands would look like when you have a lot of files and libraries to link; this example is used for the Granger causality test:
  - `g++ -fPIC -shared granger_driver.cpp granger_causality.cpp granger_causality.h granger_matrix_experiment.cpp granger_matrix_experiment.h irls.cpp irls.h granger_result.h -o libgranger.so -larmadillo -lboost_system -Wl,-rpath,$PWD`
  - `g++ -fPIC -shared test_c_connector.cpp -L$PWD -lgranger -o libtest_c_connector.so -Wl,-rpath,$PWD`
- Take the new `cpp_c_connector.so` (in our example above it was `libtest_c_connector.so`) and move it into the `bin/linux/x64/Debug` Directory.
- Do the same with the `libtest.so` or whatever you called it. In the Granger causality implementation, it is called `libgranger.so`.
- In the C file for your test (the one originally copied from block_frequency) include the `cpp_c_connector .h` file.
- Then in the `RunTest()` function, call your wrapper function; in our example, this is `c_wrapper`, which will make the call to the C++ code.
- Now you have to modify the Makefile in our test directory to correctly link the code. This will occur the Configuration: Debug and Configuration: Release sections. Here, we paste examples of what these 8 link lines should be modified to look like for our test to add the new `.so` files:
  - `LINK=$(CC) -Wall "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.a $(BINDIR)/libcpp_c_connector.so $(BINDIR)/libgranger.so $(CFG_LIB) -Wl,-rpath,$(BINDIR)`
  - `LINK=$(CC) -Wall -pg "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.a $(BINDIR)/libcpp_c_connector.so $(BINDIR)/libgranger.so $(CFG_LIB) -Wl,-rpath,$(BINDIR)`
  - `LINK=$(CC) -Wall "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.so $(BINDIR)/libcpp_c_connector.so $(BINDIR)/libgranger.so $(CFG_LIB) -Wl,-rpath,$(BINDIR)`
  - `LINK=$(CC) -Wall -pg "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.so $(BINDIR)/libcpp_c_connector.so $(BINDIR)/libgranger.so $(CFG_LIB) -Wl,-rpath,$(BINDIR)`
  - `LINK=$(CC) -Wall "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.a $(CFG_LIB) $(BINDIR)/libcpp_c_connector.so $(BINDIR)/libgranger.so $(CFG_LIB) -Wl,-rpath,$(BINDIR)`
  - `LINK=$(CC) -Wall -pg "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.a $(CFG_LIB) $(BINDIR)/libcpp_c_connector.so $(BINDIR)/libgranger.so $(CFG_LIB) -Wl,-rpath,$(BINDIR)`
  - `LINK=$(CC) -Wall "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.so $(CFG_LIB) $(BINDIR)/libtest_c_connector.so $(BINDIR)/libgranger.so $(CFG_LIB) -Wl,-rpath,$(BINDIR)`
  - `LINK=$(CC) -Wall -pg "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.so $(CFG_LIB) $(BINDIR)/libcpp_c_connector.so $(BINDIR)/libgranger.so $(CFG_LIB) -Wl,-rpath,$(BINDIR)`

# Adding Data Instructions
1. Convert your data to binary form.
This is straightforward if your data is being collected/processed in Python.
If the data is stored as a Numpy array, for example, you can use the `tofile()` function.

2. Place your test file in ./data/validation/nists-sts/

3. Navigate to STEER-FRAMEWORK/build_files.
Add the name of the data file to test_data_names.txt.

4. Save and leave test_data_names.txt.

5. Navigate to STEER-FRAMEWORK/test/validation/nist-sts.
For _each_ test that you would like to run on your new data file, navigate into that test's folder.
Then, copy each of the two .json files ("expected_results_dataname.json" and "parameters_dataname.json"),
replacing the existing dataname (_e.g._ "e") with the name of your new data.  (Again, what you added in Step 2.)
If you want the tests to meaningfully run on your data, these two files will need to be updated with the expected results 
and accurate parameters for the new data.
But if you are just trying to verify that you have correctly added the data, so that the tests have the ability to run
on it, the only change is in "parameters_dataname.json": update the "parameter set name."
Save and close the files.

6. Navigate to STEER-FRAMEWORK/src/test-scheduler/validation_test_schedule.json.
For _each_ test ("program name" in this file) that you would like to run on your new data file, copy one of the existing "profiles"
blocks, and in that copy, replace each instance of the pre-existing data name with your new data name.  So, for example, if you
copied the block associated with "e", every instance of "e" as data name should be replaced with your new data name.
Save and close the file.

7. Navigate to STEER-FRAMEWORK/src/run_validations/validation_checks.json.
Again for _each_ test that you would like to run on your new data file, copy one of the 'blocks' of the form

```
{
	"measured": "./results/validation/nist-sts/testname/test_report_dataname.json",
	"expected": "./test/validation/nist-sts/testname/expected_results_dataname.json"
}
```

Then, replace "testname" with the name of whichever test you want to run on your data,
and replace "dataname" with the name of your data (from Step 2.).
Save and close the file.

8. At this point, the test(s) that you set up should run on your new data file, if you use the command:
`./build.sh --clean --debug --with-validation`


### Todo suggestions
1. Throughout the repository, all tests are referred to as "NIST STS" tests. 
We will soon have tests like Granger Causality, which are not part of the NIST STS
suite.

2. In the block_frequency test, and, specifically, `block_frequency.c`, the
line being indicating the test success is:
`*passed = (privateData->probabilityValue >= privateData->probabilityValue);`
Shoudn't this be:
`*passed = (privateData->probabilityValue >= privateData->significanceLevel);`?


---

STEER is an open source development project developed and managed by [Anametric, Inc.](https://www.anametric.com/) and the [Darwin Deason Institute of Cyber Security at Southern Methodist University](https://www.smu.edu/Lyle/Centers-and-Institutes/DDI).

<div class="row">
        <img src="./images/anametric-logo.png" width="35%" float="left"> 
        <img src="./images/ddics-logo.png" width="50%" float="right">
</div>

---

Copyright © 2022 Anametric, Inc.
