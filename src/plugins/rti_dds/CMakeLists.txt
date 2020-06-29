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

# These two variables are needed so that the ndds find script can find the libraries we want to use.
# The ndds recipe will set environment variables that will be written to the cache when CMake is
# initially instrumented by conan. If afterwards tooling is invoking CMake without conans virtualenv
# the information is still available.
set(CONNEXTDDS_DIR  $ENV{NDDSHOME}        CACHE STRING "Path to RTI Connext DDS installation")
set(CONNEXTDDS_ARCH $ENV{CONNEXTDDS_ARCH} CACHE STRING "RTI platform string RTI Connext shall be using")

include(rti-macros.cmake)

# Hack (*shiver*) to convince RTI Connext DDS find script to support native armv8 compiling
set(CONNEXTDDS_DISABLE_VERSION_CHECK True)
set(TEMPORARY_CMAKE_HOST_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(CMAKE_HOST_SYSTEM_PROCESSOR "x86_64")
endif()

# Execute (now hacked) find package script
find_package(RTIConnextDDS EXACT "6.0.1" REQUIRED)

# Undo hack
set(CMAKE_HOST_SYSTEM_PROCESSOR ${TEMPORARY_CMAKE_HOST_SYSTEM_PROCESSOR})

set(PLUGIN_NAME fep3_connext_dds_plugin)
add_library(${PLUGIN_NAME} SHARED 
            fep_connext_dds_plugin.cpp
            
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_client.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_client.cpp
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_server.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_server.cpp
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_systemaccess.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_systemaccess.cpp

            service_bus/rti_dds_client.cpp
            service_bus/rti_dds_client.h
            service_bus/rti_dds_server.cpp
            service_bus/rti_dds_server.h
            service_bus/rti_dds_system_access.cpp
            service_bus/rti_dds_system_access.h
            service_bus/fep_connext_dds_service_bus.cpp
            service_bus/fep_connext_dds_service_bus.h

            types/stream_types.cxx
            types/stream_typesPlugin.cxx
            
            simulation_bus/topic_intf.h
            simulation_bus/converter.h            
            simulation_bus/converter.cpp
            simulation_bus/vector_raw_memory.hpp

            simulation_bus/stream_item_topic/stream_item_reader.h
            simulation_bus/stream_item_topic/stream_item_reader.cpp
            simulation_bus/stream_item_topic/stream_item_topic.h
            simulation_bus/stream_item_topic/stream_item_topic.cpp
            simulation_bus/stream_item_topic/stream_item_writer.h
            simulation_bus/stream_item_topic/stream_item_writer.cpp

			simulation_bus/internal_topic/internal_topic.h
			simulation_bus/internal_topic/internal_topic.cpp

            simulation_bus/fep_connext_dds_simulation_bus.cpp
            simulation_bus/fep_connext_dds_simulation_bus.h
            
            simulation_bus/bus_info/bus_info.h
            simulation_bus/bus_info/bus_info.cpp

            ${PROJECT_SOURCE_DIR}/src/fep3/base/binary_info/binary_info.cpp
            ${PROJECT_SOURCE_DIR}/src/fep3/base/environment_variable/environment_variable.cpp

            ${PROJECT_SOURCE_DIR}/3rdparty/lssdp-cpp/src/url/cxx_url.h
            ${PROJECT_SOURCE_DIR}/3rdparty/lssdp-cpp/src/url/cxx_url.cpp

            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/find_free_port.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/find_free_port.cpp
            ${PROJECT_SOURCE_DIR}/3rdparty/lssdp-cpp/src/lssdpcpp/lssdpcpp.h
            ${PROJECT_SOURCE_DIR}/3rdparty/lssdp-cpp/src/lssdpcpp/lssdpcpp.cpp
    
            USER_QOS_PROFILES.xml)

set_target_properties(${PLUGIN_NAME} PROPERTIES FOLDER "plugins/cpp")

target_link_libraries(${PLUGIN_NAME} PRIVATE 
            fep3_participant_cpp_plugin 
            pkg_rpc 
            RTIConnextDDS::cpp2_api
            a_util_filesystem
            a_util_system 
            a_util_concurrency)

if(MSVC)
    target_link_libraries(${PLUGIN_NAME} PRIVATE "Iphlpapi.lib")
    install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}> DESTINATION lib/rti OPTIONAL)
endif()

rti_deploy(${PLUGIN_NAME})
rti_install(lib/rti)

install(TARGETS ${PLUGIN_NAME}
        EXPORT ${PLUGIN_NAME}_targets
        LIBRARY NAMELINK_SKIP DESTINATION lib/rti
        RUNTIME DESTINATION lib/rti)

install(EXPORT ${PLUGIN_NAME}_targets DESTINATION lib/cmake)
