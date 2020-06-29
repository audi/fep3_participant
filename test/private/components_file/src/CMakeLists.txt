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
#  the test itself
##################################################################

add_definitions(-DCURRENT_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../") 

add_executable(test_components_file tester_components_file.cpp)

set_target_properties(test_components_file PROPERTIES FOLDER "test/private/participant/core")
target_link_libraries(test_components_file PRIVATE 
    fep3_participant_private_lib
    GTest::Main                                              
)

add_test(NAME test_components_file COMMAND test_components_file WORKING_DIRECTORY "..")
set_target_properties(test_components_file PROPERTIES TIMEOUT 10)