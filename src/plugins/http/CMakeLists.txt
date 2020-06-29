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

find_package(pkg_rpc REQUIRED)

set(PLUGIN_NAME fep3_http_service_bus)
add_library(${PLUGIN_NAME} SHARED 
            fep_http_plugin.cpp

            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus.cpp
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/service_bus_logger.hpp

            ${PROJECT_SOURCE_DIR}/3rdparty/lssdp-cpp/src/url/cxx_url.h
            ${PROJECT_SOURCE_DIR}/3rdparty/lssdp-cpp/src/url/cxx_url.cpp

            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_client.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_client.cpp
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_server.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_server.cpp
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_systemaccess.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/http_systemaccess.cpp

            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/find_free_port.h
            ${PROJECT_SOURCE_DIR}/src/fep3/native_components/service_bus/rpc/http/find_free_port.cpp
            ${PROJECT_SOURCE_DIR}/3rdparty/lssdp-cpp/src/lssdpcpp/lssdpcpp.h
            ${PROJECT_SOURCE_DIR}/3rdparty/lssdp-cpp/src/lssdpcpp/lssdpcpp.cpp)

set_target_properties(${PLUGIN_NAME} PROPERTIES FOLDER "plugins/cpp")

target_link_libraries(${PLUGIN_NAME} PRIVATE fep3_participant_cpp_plugin pkg_rpc a_util_system a_util_concurrency)

if(MSVC)
    target_link_libraries(${PLUGIN_NAME} PRIVATE "Iphlpapi.lib")
    install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}> DESTINATION lib/http OPTIONAL)
endif()

install(TARGETS ${PLUGIN_NAME}
        EXPORT ${PLUGIN_NAME}_targets
        LIBRARY NAMELINK_SKIP DESTINATION lib/http
        RUNTIME DESTINATION lib/http
)
install(EXPORT ${PLUGIN_NAME}_targets DESTINATION lib/cmake)
