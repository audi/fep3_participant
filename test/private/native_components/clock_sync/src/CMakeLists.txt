##################################################################
# @file 
# @copyright Audi AG
#            All right reserved.
# 
# This Source Code Form is subject to the terms of the 
# Mozilla Public License, v. 2.0. 
# If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
##################################################################

##################################################################
# Test of the interpolation time implementation
##################################################################
add_executable(tester_interpolation_time 
               tester_interpolation_time.cpp
)

add_test(NAME tester_interpolation_time
    COMMAND tester_interpolation_time
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"        
)
target_link_libraries(tester_interpolation_time PRIVATE
    GTest::Main
    GMock::GMock
    fep3_participant_private_lib
)

set_target_properties(tester_interpolation_time PROPERTIES FOLDER "test/private/native_components/clock_sync/unit")

##################################################################
# Test of the native clock sync service implementation
##################################################################
add_executable(tester_clock_sync_service 
               tester_clock_sync_service.cpp
)

add_test(NAME tester_clock_sync_service
    COMMAND tester_clock_sync_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"        
)
target_link_libraries(tester_clock_sync_service PRIVATE
    GTest::Main
    GMock::GMock
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_clock_sync_service PROPERTIES FOLDER "test/private/native_components/clock_sync/unit")

##################################################################
# Test of the clock sync client implementation
##################################################################
add_executable(tester_clock_sync_client 
               tester_clock_sync_client.cpp
)

add_test(NAME tester_clock_sync_client
    COMMAND tester_clock_sync_client
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"        
)
target_link_libraries(tester_clock_sync_client PRIVATE
    GTest::Main
    GMock::GMock
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_clock_sync_client PROPERTIES FOLDER "test/private/native_components/clock_sync/unit")

##################################################################
# Test of the clock sync master implementation
##################################################################
add_executable(tester_clock_sync_master 
               tester_clock_sync_master.cpp
)

add_test(NAME tester_clock_sync_master
    COMMAND tester_clock_sync_master
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"        
)
target_link_libraries(tester_clock_sync_master PRIVATE
    GTest::Main
    GMock::GMock
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_clock_sync_master PROPERTIES FOLDER "test/private/native_components/clock_sync/unit")

##################################################################
# Integration test of the clock sync service
##################################################################
add_executable(tester_clock_synchronization
               tester_clock_synchronization.cpp
)

add_test(NAME tester_clock_synchronization
    COMMAND tester_clock_synchronization
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"        
)
target_link_libraries(tester_clock_synchronization PRIVATE
    GTest::Main
    GMock::GMock
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_clock_synchronization PROPERTIES FOLDER "test/private/native_components/clock_sync")