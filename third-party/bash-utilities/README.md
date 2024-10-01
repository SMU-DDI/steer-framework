## `bash-utilities` README
##### 2022.05.25 - Gary Woodcock, <span>g</span><span>a</span><span>r</span><span>y</span><span>@</span><span>u</span><span>n</span><span>t</span><span>h</span><span>i</span><span>n</span><span>k</span><span>a</span><span>b</span><span>l</span><span>e</span><span>.</span><span>c</span><span>o</span><span>m</span>
---
### Table of Contents
+ [About](#about)
+ [Usage](#usage)
+ [Contents](#contents)
+ [License](./LICENSE)

### About <a name = "about"></a>
`bash-utilities` is a collection of `bash` functions that have proven helpful in managing the development of `*nix`-compatible `C/C++` software.

### Usage <a name = "usage"></a>
To use them, simply `source` them into your `bash`-compatible shell script. For example:

    #!/bin/bash

    # Stash original working directory 
    STARTING_WORKING_DIR="$PWD"

    # Include bash utilities
    cd "./bash-utilities"
    source "./bash-utilities.sh"

    # Change back to original working directory
    cd $STARTING_WORKING_DIR

### Contents <a name = "contents"></a>
There are 19 files in this collection:

* [`bash_architecture_utils.sh`](./bash_architecture_utils.sh) implements functions that determine the type of hardware platform in use
* [`bash_c_build_utils.sh`](./bash_c_build_utils.sh) implements functions that determine what level of `C/C++` support is available
* [`bash_c_code_quality_utils.sh`](./bash_c_code_quality_utils.sh) implements functions that support `C/C++` code quality checks
* [`bash_check_env.sh`](./bash_check_env.sh) implements wrapper functions that check for specific features on a `*nix` system
* [`bash_console_utils.sh`](./bash_console_utils.sh) implements functions that support writing to the console
* [`bash_docs_utils.sh`](./bash_docs_utils.sh) implements functions that support code documentation
* [`bash_dotnet_sdk_utils.sh`](./bash_dotnet_sdk_utils.sh) implements functions that determine whether the `.NET SDK` is available
* [`bash_file_system_utils.sh`](./bash_file_system_utils.sh) implements functions that interact with files and directories
* [`bash_misc_utils.sh`](./bash_misc_utils.sh) implements a variety of miscellaneous functions
* [`bash_network_utils.sh`](./bash_network_utils.sh) implements functions that interact with networks
* [`bash_numeral_utils.sh`](./bash_numeral_utils.sh) implements various functions dealing with integers
* [`bash_os_utils.sh`](./bash_os_utils.sh) implements functions that determine the type of operating system in use
* [`bash_package_manager_utils.sh`](./bash_package_manager_utils.sh) implements functions that check for specific package managers
* [`bash_python_utils.sh`](./bash_python_utils.sh) implements functions that check for the presence of Python and its version
* [`bash_security_utils.sh`](./bash_security_utils.sh) implements functions that check for security support (e.g., `OpenSSL`)
* [`bash_shell_utils.sh`](./bash_shell_utils.sh) implements basic functions that support other script functions
* [`bash_string_utils.sh`](./bash_string_utils.sh) implements functions that can examine and manipulate strings
* [`bash_unit_test_utils.sh`](./bash_unit_test_utils.sh) implements functions that support `CUnit`
* [`bash_utilities.sh`](./bash_utilities.sh) is a script that sources all the other script files
---
###### Copyright (c) 2019 Gary Woodcock and Unthinkable Research LLC. All rights reserved.
