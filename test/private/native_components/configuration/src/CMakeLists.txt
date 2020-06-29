#######################################################################
# @file 
# Copyright &copy; AUDI AG. All rights reserved.
# 
# This Source Code Form is subject to the terms of the 
# Mozilla Public License, v. 2.0. 
# If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
#######################################################################


##################################################################
# tester_configuration_service
##################################################################

add_executable(tester_configuration_service 
    tester_configuration_service.cpp    
)

add_test(NAME tester_configuration_service
    COMMAND tester_configuration_service
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(tester_configuration_service PRIVATE
    GTest::Main
    participant_private_test_utils
    fep3_participant_private_lib
)

set_target_properties(tester_configuration_service PROPERTIES FOLDER "test/private/native_components/configuration")


##################################################################
# tester_configuration_service_rpc
##################################################################

find_package(pkg_rpc)

jsonrpc_generate_client_stub(${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/configuration/configuration.json
                             test::rpc_stubs::TestConfigurationServiceProxy
                             ${CMAKE_CURRENT_BINARY_DIR}/test_configuration_proxy_stub.h)

add_executable(tester_configuration_service_rpc 
              tester_configuration_service_rpc.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/test_configuration_proxy_stub.h
)

add_test(NAME tester_configuration_service_rpc 
    COMMAND tester_configuration_service_rpc 
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(tester_configuration_service_rpc PRIVATE 
    pkg_rpc
    GTest::Main
    GMock::GMock    
    participant_private_test_utils
    fep3_participant_private_lib
)

target_include_directories(tester_configuration_service_rpc PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(tester_configuration_service_rpc PROPERTIES FOLDER "test/private/native_components/configuration")

