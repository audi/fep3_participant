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

jsonrpc_generate_client_stub(${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json 
                             test::rpc_stubs::TestInterfaceClient
                             ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h)
jsonrpc_generate_server_stub(${CMAKE_CURRENT_SOURCE_DIR}/testinterface.json
                             test::rpc_stubs::TestInterfaceServer
                             ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h)


add_executable(test_service_bus tester_service_bus.cpp
                                tester_find_free_port.cpp
                                tester_service_registration.cpp
                                ${CMAKE_CURRENT_BINARY_DIR}/testserverstub.h
                                ${CMAKE_CURRENT_BINARY_DIR}/testclientstub.h)

set_target_properties(test_service_bus PROPERTIES FOLDER "test/private/native_components")
target_link_libraries(test_service_bus PRIVATE GTest::Main
                                               fep3_participant_private_lib
                                               a_util_process)

target_include_directories(test_service_bus PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME test_service_bus COMMAND test_service_bus WORKING_DIRECTORY "..")
set_target_properties(test_service_bus PROPERTIES TIMEOUT 10)
