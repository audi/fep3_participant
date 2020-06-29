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

add_executable(test_component_registry tester_component_registry.cpp)
set_target_properties(test_component_registry PROPERTIES FOLDER "test/function/native_components/base")
target_link_libraries(test_component_registry PRIVATE
    fep3_participant
    GTest::Main
    GMock::GMock
)
add_test(NAME test_component_registry COMMAND test_component_registry WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../")
set_target_properties(test_component_registry PROPERTIES TIMEOUT 10)

fep3_participant_deploy(test_component_registry)

##################################################################
##################################################################

add_executable(test_data_sample tester_data_sample.cpp)
set_target_properties(test_data_sample PROPERTIES FOLDER "test/function/native_components/base")
target_link_libraries(test_data_sample PRIVATE fep3_participant
                                               a_util
                                               GTest::Main)
#target_link_libraries(test_component_registry PUBLIC a_util)
add_test(NAME test_data_sample COMMAND test_data_sample WORKING_DIRECTORY "..")
set_target_properties(test_data_sample PROPERTIES TIMEOUT 10)

fep3_participant_deploy(test_data_sample)

##################################################################
##################################################################

add_executable(test_stream_type tester_stream_type.cpp)
set_target_properties(test_stream_type PROPERTIES FOLDER "test/function/native_components/base")
target_link_libraries(test_stream_type PRIVATE fep3_participant
                                               GTest::Main)

add_test(NAME test_stream_type COMMAND test_stream_type WORKING_DIRECTORY "..")
set_target_properties(test_stream_type PROPERTIES TIMEOUT 10)

fep3_participant_deploy(test_stream_type)


