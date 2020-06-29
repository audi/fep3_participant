##################################################################
# @file 
# @copyright AUDI AG
#            All right reserved.
# 
# This Source Code Form is subject to the terms of the 
# Mozilla Public License, v. 2.0. 
# If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
##################################################################
cmake_minimum_required(VERSION 3.13.1 FATAL_ERROR)
cmake_policy(SET CMP0011 NEW)

# Disable extensions here and require the chosen CMAKE_CXX_STANDARD (coming from Conan)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

# Use shared libraries from current directory on Linux (same behavior as on Windows)
SET(CMAKE_INSTALL_RPATH "$ORIGIN")

if(CMAKE_CXX_STANDARD LESS 14)
    message(FATAL_ERROR "The FEP Participant library requires CMAKE_CXX_STANDARD >= 14")
endif()

project(fep3-participant-library VERSION 3.0.0)
set(FEP3_PARTICIPANT_LIBRARY fep3_participant)

set(FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(DOXYGEN_WARN_IF_UNDOCUMENTED "No")

set(FEP3_PARTICIPANT_LIBRARY_VERSION "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH}")

# Enable strict compiler warnings
if(MSVC)    
    # TODO /WD4100 should be removed when ODAUTIL-167 is fixed
    # 4251 is currently deactivated because some dll exported classes use std types within their interface (e.g. ComponentRegistry)
    add_compile_options(/W4 /WX /wd4251 /wd4100)
else()
    # TODO -Wno-comment should be removed when ODAUTIL-169 is fixed
    add_compile_options(-Wall -Wno-unknown-pragmas -Wno-reorder -Werror -Wextra -pedantic -Wno-comment)   
endif()

# check for conan
if(NOT CONAN_COMPILER)
    message(WARNING "Very good hint: FEP3 Participant Library should not be built without conan!")
else()
    message(STATUS "Including Conan build info")
    if ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
        include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
        include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo.cmake)
    elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
        include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
    elseif ( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo_multi.cmake)
        include(${CMAKE_CURRENT_BINARY_DIR}/../conanbuildinfo_multi.cmake)
    else()
        message(FATAL_ERROR "Conan build info can't be found.")
    endif()

    if(CORTEX_WORKSPACE)
        conan_basic_setup(TARGETS)
    else()
        conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
    endif()

    set(pkg_rpc_DIR ${CONAN_PKG_RPC_ROOT})
    set(DOXYGEN_ROOT ${CONAN_DOXYGEN_ROOT})
    set(gtest_search_mode CONFIG)
endif()

include(scripts/cmake/enable_multicore_compilation.cmake)
include(scripts/cmake/use_integrated_debug_symbols.cmake)

# Enable project folder structure for Visual Studio IDE
set_property(GLOBAL PROPERTY USE_FOLDERS true)

### Product specific
set(fep3_participant_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_FEP3_PARTICIPANT_LIBRARY true)
set(BETA_BUILD false CACHE BOOL "Mark as beta")
set(REFERENCE_VERSION ${CONAN_PACKAGE_VERSION})

# some settings need to be set explicitly for QNX
if (UNIX)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
    set(CMAKE_SKIP_BUILD_RPATH  OFF)
endif(UNIX)

find_package(a_util REQUIRED)
find_package(pkg_rpc REQUIRED)

if (FEP3_USE_RTIDDS)
    set(fep3_participant_use_rtidds ON) 
else()
    set(fep3_participant_use_rtidds OFF) 
endif()

option(fep3_participant_cmake_enable_documentation
       "If enabled, generate the source code documentation -\
 requires doxygen and sphinx-build (default: ON)" ON)
option(fep3_participant_cmake_enable_tests
       "Enable functional tests - requires googletest (default: OFF)" OFF)

################################################################################
### Setting up packages
################################################################################

# compensate for the missing platform if building locally
if(NOT DEFINED PLATFORM)
    set(PLATFORM "developer")
endif(NOT DEFINED PLATFORM)

# add subdirectories core
#this is needed for teh pdb file while installed
set(fep3_participant_pdb_version_str ${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR})
include(src/fep3/fep3_participant-macros.cmake)
add_subdirectory(src)
if (fep3_participant_cmake_enable_documentation)
    add_subdirectory(doc)
endif()

#we always need to install licenses
add_subdirectory(doc/license)

if(fep3_participant_cmake_enable_tests)
    # we have private tests that are built/run during the product build
    enable_testing()
    set(fep3_participant_cmake_integrated_tests ON)
    add_subdirectory(test)
endif()

# install content from include directory
install(
    DIRECTORY
        include
    DESTINATION
        ./
)
