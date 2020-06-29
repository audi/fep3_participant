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

project(test_component_factory_cpp_plugins)

add_executable(${PROJECT_NAME} tester_component_factory_cpp_plugins.cpp)
add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"        
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    GTest::Main
    fep3_participant_private_lib
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/component_factory/cpp")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PLUGIN_1="$<TARGET_FILE:test_cpp_plugin_1>"
    PLUGIN_2="$<TARGET_FILE:test_cpp_plugin_2>"
    TEST_BUILD_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

target_include_directories(${PROJECT_NAME} PRIVATE    
    $<TARGET_PROPERTY:test_cpp_plugin_1,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:test_cpp_plugin_2,INTERFACE_INCLUDE_DIRECTORIES>
)

# copy the plugins
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:test_cpp_plugin_1> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:test_cpp_plugin_1>
)
