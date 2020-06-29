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
find_package(pkg_rpc)

jsonrpc_generate_client_stub(${PROJECT_SOURCE_DIR}/include/fep3/rpc_services/data_registry/data_registry.json
                             test::rpc_stubs::TestDataRegistryClient
                             ${CMAKE_CURRENT_BINARY_DIR}/test_data_registry_client_stub.h)

add_executable(test_data_registry tester_data_registry.cpp
               ${CMAKE_CURRENT_BINARY_DIR}/test_data_registry_client_stub.h
)

add_test(NAME test_data_registry 
    COMMAND test_data_registry 
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)

target_link_libraries(test_data_registry PRIVATE 
    pkg_rpc
    GTest::Main
    GMock::GMock
    a_util
    fep3_participant_private_lib
)

target_include_directories(test_data_registry PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(test_data_registry PROPERTIES FOLDER "test/private/native_components")

#fep3_participant_deploy(test_data_registry)