<!---
  Copyright @ 2019 Audi AG. All rights reserved.

      This Source Code Form is subject to the terms of the Mozilla
      Public License, v. 2.0. If a copy of the MPL was not distributed
      with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

  If it is not possible or desirable to put the notice in a particular file, then
  You may include the notice in a location (such as a LICENSE file in a
  relevant directory) where a recipient would be likely to look for such a notice.

  You may add additional accurate notices of copyright ownership.
  -->

FEP Participant library
=======================

## Description ##

This installed package contains the FEP Participant library.

* FEP Participant library
  * Middleware Abstraction for distributed simulation systems
  * a_util platform abstraction library
  * DDL library (data description language with codec API)
  * RPC library with JSON-RPC code generator
* FEP Participant library Examples
* FEP Participant library Documentation (see fep3_participant/doc/fep3-participant.html)

## How to use ###

The FEP SDK provides a CMake >= 3.5 configuration. Here's how to use it from your own CMake projects:

To build against the fep participant library only: 

    find_package(fep3_participant REQUIRED)

After this instruction, you can create CMake executable targets linking against the FEP Participant library using the following command:

    add_executable(my_participant_target source_file1.cpp source_file2.cpp)

You need to append the *fep3_participant* target to your existing targets to add the dependency:

    target_link_libraries(my_participant_target PRIVATE fep3_participant)
    fep3_participant_install(my_participant_target)

The convenience macro *fep3_participant_install* will help to install all DLL/SO to the target directory of *my_participant_target*.
The convenience macro *fep3_participant_deploy* will help to add DLL/SO dependencies to the build target directory of *my_participant_target*.

To build against the fep participant core library with convenience class to build your own elements as Tool integration, use the following:

    find_package(fep3_participant_core REQUIRED)
    add_executable(my_tool_element my_tool_element.cpp)
    target_link_libraries(my_tool_element PRIVATE fep3_participant_core)
    fep3_participant_install(my_tool_element)

To build against the fep participant cpp library with convenience class to build your own FEP CPP Interfaces use the following:

    find_package(fep3_participant_cpp REQUIRED)
    add_executable(my_cpp_element my_new_element.cpp)
    target_link_libraries(my_cpp_element PRIVATE fep3_participant_cpp)
    fep3_participant_install(my_cpp_element)


### Build Environment ####

The libraries are built and tested only under following compilers and operating systems: 
* Windows 10 x64 with Visual Studio C++ 2015 Update 3.1 (Update 3 and KB3165756)
* Linux Ubuntu 16.04 LTS x64 with GCC 5.4 and libstdc++11 (C++11 ABI)

## How to build the examples ###

Simply point CMake to the examples directory (containing the CMakeLists.txt file) and generate a project.
Choose "Visual Studio 14 2015 Win64", "Visual Studio 15 2017 Win64" together with the v140 toolset or "Unix Makefiles" generator, depending on your platform.

CMake might ask for the CMAKE_BUILD_TYPE variable to be defined. Possible values are Debug, Release or RelWithDebInfo
