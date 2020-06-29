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
# tester_job_runner
##################################################################

add_executable(tester_job_runner tester_job_runner.cpp)

add_test(NAME tester_job_runner
    COMMAND tester_job_runner
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_job_runner PRIVATE
    GTest::Main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_job_runner PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_scheduler_registry
##################################################################


add_executable(tester_scheduler_registry tester_scheduler_registry.cpp)

add_test(NAME tester_scheduler_registry
    COMMAND tester_scheduler_registry
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_scheduler_registry PRIVATE
    GTest::Main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_scheduler_registry PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_timer_scheduler
##################################################################


add_executable(tester_timer_scheduler tester_timer_scheduler.cpp)

add_test(NAME tester_timer_scheduler
    COMMAND tester_timer_scheduler
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_timer_scheduler PRIVATE
    GTest::Main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_timer_scheduler PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_clock_based_scheduler
##################################################################


add_executable(tester_clock_based_scheduler tester_clock_based_scheduler.cpp)

add_test(NAME tester_clock_based_scheduler
    COMMAND tester_clock_based_scheduler
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_clock_based_scheduler PRIVATE
    GTest::Main
    participant_private_test_utils
    participant_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_clock_based_scheduler PROPERTIES FOLDER "test/private/native_components/scheduler/unit")

##################################################################
# tester_scheduler_service
##################################################################


add_executable(tester_scheduler_service tester_scheduler_service.cpp)

add_test(NAME tester_scheduler_service
    COMMAND tester_scheduler_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_scheduler_service PRIVATE
    GTest::Main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_scheduler_service PROPERTIES FOLDER "test/private/native_components/scheduler")

##################################################################
# Test of the native scheduler service rpc implementation
##################################################################

find_package(pkg_rpc)

jsonrpc_generate_client_stub(${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/scheduler_service/scheduler_service.json
                             test::rpc_stubs::TestSchedulerServiceClientStub
                             ${CMAKE_CURRENT_BINARY_DIR}/test_scheduler_service_client_stub.h)

add_executable(tester_scheduler_service_rpc tester_scheduler_service_rpc.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/test_scheduler_service_client_stub.h
)

add_test(NAME tester_scheduler_service_rpc 
    COMMAND tester_scheduler_service_rpc 
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_scheduler_service_rpc PRIVATE 
    pkg_rpc
    GTest::Main
    GMock::GMock
    a_util
    fep3_participant_private_lib
)

target_include_directories(tester_scheduler_service_rpc PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_scheduler_service_rpc PROPERTIES FOLDER "test/private/native_components/scheduler")