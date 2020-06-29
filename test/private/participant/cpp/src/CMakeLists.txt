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
# teser_job
##################################################################

add_executable(tester_job tester_job.cpp tester_datajob.cpp)
add_test(NAME tester_job
    COMMAND tester_job
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)
target_link_libraries(tester_job PRIVATE GTest::Main ${CMAKE_DL_LIBS} participant_private_test_utils fep3_participant_cpp)
set_target_properties(tester_job PROPERTIES FOLDER "test/private/participant/cpp")
fep3_participant_deploy(tester_job)
