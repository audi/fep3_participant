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

set(TEST_NAME test_optional)

add_executable(${TEST_NAME} tester_optional.cpp)
set_target_properties(${TEST_NAME} PROPERTIES FOLDER "test/function/base/optional")
target_link_libraries(${TEST_NAME} PRIVATE
    fep3_participant
    GTest::Main
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
set_target_properties(${TEST_NAME} PROPERTIES TIMEOUT 10)

fep3_participant_deploy(${TEST_NAME})


