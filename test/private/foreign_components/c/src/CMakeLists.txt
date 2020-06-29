##################################################################
# @file
# Copyright &copy; AUDI AG. All rights reserved.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
##################################################################

##################################################################
#  test plugin 1
##################################################################

set(TEST_FOREIGN_COMPONENTS_C_PLUGIN_1_TARGET test_foreign_components_c_plugin_1)

set(BUILD_INTERFACE_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

add_library(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_1_TARGET} SHARED
    test_plugins/plugin_1/test_plugin_1.h
    test_plugins/plugin_1/test_plugin_1.cpp
    test_plugins/plugin_1/mock_component_a.h
    test_plugins/plugin_1/component_a_intf.h
    test_plugins/plugin_1/component_a_c_intf.h
    test_plugins/plugin_1/component_a_c_access_wrapper.h
    test_plugins/plugin_1/component_a_c_access_wrapper.cpp
    test_plugins/plugin_1/mock_component_b.h
    test_plugins/plugin_1/component_b_intf.h
    test_plugins/plugin_1/component_b_c_intf.h
    test_plugins/plugin_1/component_b_c_access_wrapper.h
    test_plugins/plugin_1/component_b_c_access_wrapper.cpp
    )
target_include_directories(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_1_TARGET} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
    )
target_compile_definitions(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_1_TARGET} PRIVATE _FEP3_PARTICIPANT_INCLUDED_STATIC)
target_link_libraries(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_1_TARGET} PUBLIC a_util_result)
target_link_libraries(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_1_TARGET} PRIVATE
    GTest::Main
    GMock::GMock
)
set_target_properties(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_1_TARGET} PROPERTIES FOLDER "test/private/foreign_components/c/test_plugin_1")

##################################################################
#  test plugin 2
##################################################################

set(TEST_FOREIGN_COMPONENTS_C_PLUGIN_2_TARGET test_foreign_components_c_plugin_2)

set(BUILD_INTERFACE_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    )

add_library(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_2_TARGET} SHARED
    test_plugins/plugin_2/test_plugin_2.h
    test_plugins/plugin_2/test_plugin_2.cpp
    test_plugins/plugin_2/mock_component_c.h
    test_plugins/plugin_2/component_c_intf.h
    test_plugins/plugin_2/component_c_c_intf.h
    test_plugins/plugin_2/component_c_c_access_wrapper.h
    test_plugins/plugin_2/component_c_c_access_wrapper.cpp
    )
target_include_directories(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_2_TARGET} PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
    )
target_compile_definitions(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_2_TARGET} PRIVATE _FEP3_PARTICIPANT_INCLUDED_STATIC)
target_link_libraries(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_2_TARGET} PUBLIC a_util_result)
target_link_libraries(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_2_TARGET} PRIVATE
    GTest::Main
    GMock::GMock
)
set_target_properties(${TEST_FOREIGN_COMPONENTS_C_PLUGIN_2_TARGET} PROPERTIES FOLDER "test/private/foreign_components/c/test_plugin_2")

##################################################################
#  test application
##################################################################

project(tester_foreign_components_c_plugin)

add_executable(${PROJECT_NAME}
    tester_foreign_components.cpp
    )
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::Main
    GMock::Main
    ${CMAKE_DL_LIBS}
    fep3_participant_private_lib
    participant_private_test_utils
    pkg_rpc
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    a_util
    a_util_result
)

add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/foreign_components/c")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PLUGIN_1="$<TARGET_FILE:${TEST_FOREIGN_COMPONENTS_C_PLUGIN_1_TARGET}>"
    PLUGIN_2="$<TARGET_FILE:${TEST_FOREIGN_COMPONENTS_C_PLUGIN_2_TARGET}>"
    TEST_BUILD_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
