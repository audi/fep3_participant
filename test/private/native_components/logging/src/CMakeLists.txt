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
add_executable(tester_logging_service
    tester_rpc_log.cpp
    tester_console_log.cpp
    tester_file_log.cpp
	tester_logging_config.cpp
)

add_test(NAME tester_logging_service 
    COMMAND tester_logging_service
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_logging_service PRIVATE 
    GTest::Main
    a_util
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_logging_service PROPERTIES FOLDER "test/private/native_components")