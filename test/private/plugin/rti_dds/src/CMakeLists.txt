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

# TODO: Get rid of this hack as soon as Christoph  ##
#       knows what causes the error                 #
set(CMAKE_BUILD_TYPE ${CONAN_SETTINGS_BUILD_TYPE})  #
#####################################################

include(../../../../../src/plugins/rti_dds/rti-macros.cmake)

find_package(RTIConnextDDS EXACT "6.0.1" REQUIRED)

add_executable(test_connext_dds_plugin tester_rti_common.cpp)
add_test(NAME test_connext_dds_plugin
    COMMAND test_connext_dds_plugin
    TIMEOUT 30
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_plugin PRIVATE GTest::Main fep3_participant)
set_target_properties(test_connext_dds_plugin PROPERTIES FOLDER "test/private/plugins")

target_link_libraries(test_connext_dds_plugin PRIVATE fep3_participant)
fep3_participant_deploy(test_connext_dds_plugin)



add_executable(test_connext_dds_simulation_bus 
    tester_dds_simbus.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

add_test(NAME test_connext_dds_simulation_bus
    COMMAND test_connext_dds_simulation_bus
    TIMEOUT 60
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_simulation_bus PRIVATE GTest::Main fep3_participant a_util_memory fep3_participant_private_lib)
set_target_properties(test_connext_dds_simulation_bus PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

fep3_participant_deploy(test_connext_dds_simulation_bus)


add_executable(test_connext_dds_simulation_bus_domain_id
    tester_dds_simbus_domain_id.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus_domain_id PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

add_test(NAME test_connext_dds_simulation_bus_domain_id
    COMMAND test_connext_dds_simulation_bus_domain_id
    TIMEOUT 40
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_simulation_bus_domain_id PRIVATE GTest::Main fep3_participant a_util_memory fep3_participant_private_lib)
set_target_properties(test_connext_dds_simulation_bus_domain_id PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_domain_id PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

fep3_participant_deploy(test_connext_dds_simulation_bus_domain_id)


add_executable(test_connext_dds_simulation_bus_version 
    tester_dds_simbus_version.cpp
    USER_QOS_PROFILES.xml)

target_include_directories(test_connext_dds_simulation_bus_version PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

add_test(NAME test_connext_dds_simulation_bus_version
    COMMAND test_connext_dds_simulation_bus_version
    TIMEOUT 30
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
)
target_link_libraries(test_connext_dds_simulation_bus_version PRIVATE GTest::Main fep3_participant a_util_memory fep3_participant_private_lib)
set_target_properties(test_connext_dds_simulation_bus_version PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_version PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

fep3_participant_deploy(test_connext_dds_simulation_bus_version)
rti_deploy(test_connext_dds_simulation_bus_version)


add_executable(test_connext_dds_simulation_bus_performance 
    tester_dds_simbus_performance.cpp
    USER_QOS_PROFILES.xml)
target_compile_definitions(test_connext_dds_simulation_bus_performance PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

target_include_directories(test_connext_dds_simulation_bus_performance PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/rti_dds")

# TODO: add this test again  ################################
#add_test(NAME test_connext_dds_simulation_bus_performance  #
    #COMMAND test_connext_dds_simulation_bus_performance    #
    #TIMEOUT 30                                             #
    #WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"       #
#)                                                          #
#############################################################
target_link_libraries(test_connext_dds_simulation_bus_performance PRIVATE GTest::Main fep3_participant a_util_memory fep3_participant_private_lib)
set_target_properties(test_connext_dds_simulation_bus_performance PROPERTIES FOLDER "test/private/plugins")
target_compile_definitions(test_connext_dds_simulation_bus_performance PRIVATE FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB="$<TARGET_FILE:fep3_connext_dds_plugin>")

fep3_participant_deploy(test_connext_dds_simulation_bus_performance)
