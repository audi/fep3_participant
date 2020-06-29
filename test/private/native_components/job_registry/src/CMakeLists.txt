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

##################################################################
# tester_job_registry
##################################################################


add_executable(tester_job_registry_impl tester_job_registry_impl.cpp)

add_test(NAME tester_job_registry_impl
    COMMAND tester_job_registry_impl
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_job_registry_impl PRIVATE
    GTest::Main
    participant_private_test_utils
    fep3_participant_private_lib
    fep3_participant_core
)
set_target_properties(tester_job_registry_impl PROPERTIES FOLDER "test/private/native_components/job_registry/unit")

##################################################################
# tester_timing_configuration
##################################################################


add_executable(tester_timing_configuration tester_timing_configuration.cpp)

add_test(NAME tester_timing_configuration
    COMMAND tester_timing_configuration
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_timing_configuration PRIVATE
    GTest::Main
    participant_private_test_utils
    fep3_participant_private_lib
)

target_compile_definitions(tester_timing_configuration PRIVATE
    TEST_FILES_DIRECTORY="${fep3_participant_DIR}/test/private/native_components/job_registry/files/"
)

set_target_properties(tester_timing_configuration PROPERTIES FOLDER "test/private/native_components/job_registry/unit")

##################################################################
# tester_job_registry_service
##################################################################


add_executable(tester_job_registry tester_job_registry.cpp)

add_test(NAME tester_job_registry
    COMMAND tester_job_registry
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_job_registry PRIVATE
    GTest::Main
    participant_private_test_utils
    fep3_participant_private_lib
)

target_compile_definitions(tester_job_registry PRIVATE
    TEST_FILES_DIRECTORY="${fep3_participant_DIR}/test/private/native_components/job_registry/files/"
)

set_target_properties(tester_job_registry PROPERTIES FOLDER "test/private/native_components/job_registry")

##################################################################
# Test of the native job registry rpc implementation
##################################################################

find_package(pkg_rpc)

jsonrpc_generate_client_stub(${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/job_registry/job_registry.json
                             test::rpc_stubs::TestJobRegistryClientStub
                             ${CMAKE_CURRENT_BINARY_DIR}/test_job_registry_client_stub.h)

add_executable(tester_job_registry_rpc tester_job_registry_rpc.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/test_job_registry_client_stub.h
)

add_test(NAME tester_job_registry_rpc 
    COMMAND tester_job_registry_rpc 
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_job_registry_rpc PRIVATE 
    pkg_rpc
    GTest::Main
    GMock::GMock
    a_util
    fep3_participant_private_lib
)

target_include_directories(tester_job_registry_rpc PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_job_registry_rpc PROPERTIES FOLDER "test/private/native_components/job_registry")
