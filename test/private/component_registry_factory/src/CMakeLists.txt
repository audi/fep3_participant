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

project(test_component_registry_factory)

add_executable(${PROJECT_NAME} tester_component_registry_factory.cpp)
add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    GTest::Main 
    fep3_participant_private_lib
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/component_registry_factory")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    TEST_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

target_include_directories(${PROJECT_NAME} PRIVATE    
    $<TARGET_PROPERTY:test_cpp_plugin_1,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:test_cpp_plugin_2,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:test_foreign_components_c_plugin_1,INTERFACE_INCLUDE_DIRECTORIES>
)

# copy the plugins and component configuration files
add_dependencies(${PROJECT_NAME} copy_files)
add_custom_target(copy_files)
add_custom_command(TARGET copy_files
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:test_cpp_plugin_1> ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:test_cpp_plugin_1>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:test_cpp_plugin_2> ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:test_cpp_plugin_2>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:test_component_intfs_sim_bus_c_plugin> ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:test_component_intfs_sim_bus_c_plugin>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../files ${CMAKE_CURRENT_BINARY_DIR}/files/
)

set_target_properties(copy_files PROPERTIES FOLDER "test/private/component_registry_factory")
