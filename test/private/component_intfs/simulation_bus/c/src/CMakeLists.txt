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

set(TEST_COMPONENT_INTERFACES_SIMULATION_BUS_C_PLUGIN_TARGET test_component_intfs_sim_bus_c_plugin)

set(BUILD_INTERFACE_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
    )

add_library(${TEST_COMPONENT_INTERFACES_SIMULATION_BUS_C_PLUGIN_TARGET} SHARED
    ${PROJECT_SOURCE_DIR}/src/fep3/components/simulation_bus/mock/mock_simulation_bus.h
    test_plugin/simulation_bus.cpp
    test_plugin/test_plugin.h
    test_plugin/test_plugin.cpp
    )
target_include_directories(${TEST_COMPONENT_INTERFACES_SIMULATION_BUS_C_PLUGIN_TARGET} PUBLIC
    "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDES}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDES}>"
    )
target_link_libraries(${TEST_COMPONENT_INTERFACES_SIMULATION_BUS_C_PLUGIN_TARGET} PUBLIC a_util_result)
target_link_libraries(${TEST_COMPONENT_INTERFACES_SIMULATION_BUS_C_PLUGIN_TARGET} PRIVATE
    GTest::Main
    GMock::GMock
)
target_compile_definitions(${TEST_COMPONENT_INTERFACES_SIMULATION_BUS_C_PLUGIN_TARGET} PRIVATE _FEP3_PARTICIPANT_INCLUDED_STATIC)
set_target_properties(${TEST_COMPONENT_INTERFACES_SIMULATION_BUS_C_PLUGIN_TARGET} PROPERTIES FOLDER "test/private/component_intfs/simulation_bus/c/test_plugin")

##################################################################
#  test application
##################################################################

project(tester_component_intfs_sim_bus_c_plugin)

add_executable(${PROJECT_NAME}
    tester_sim_bus_intf_c_plugin.cpp
    )
target_include_directories(${PROJECT_NAME} PRIVATE
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PLUGIN="$<TARGET_FILE:${TEST_COMPONENT_INTERFACES_SIMULATION_BUS_C_PLUGIN_TARGET}>"
    TEST_BUILD_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::Main
    GMock::Main
    fep3_participant_private_lib
    participant_private_test_utils
    )
target_link_libraries(${PROJECT_NAME} PUBLIC
    a_util
    a_util_result
)

add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
    WORKING_DIRECTORY ".."
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/component_intfs/simulation_bus/c")