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

################################################################################
## \page page_cmake_commands
# <hr>
# <b>rti_install(\<destination\>)</b>
#
# This macro installs the rti libraries and the 'USER_QOS_PROFILES.xml' to the 
#   folder \<destination\>. The macro expects the 'USER_QOS_PROFILES.xml' to lay
#   next the the 'CMakeLists.txt' calling this macro.
#
# Arguments:
# \li \<destination\>:
# The relative path to the install subdirectory
################################################################################
macro(rti_install DESTINATION)
    if(MSVC)    
        set(RTI_CONNEXT_FILES_PATH ${CONAN_BIN_DIRS_RTI_CONNEXT_DDS})
        set(FILE_WILDCARD "*")
    else()
        if(NOT CONNEXTDDS_ARCH)
            message(FATAL_ERROR "CONNEXTDDS_ARCH variable is empty. Did you "
                "forget a 'find_package(RTIConnextDDS ...)' before "
                "calling 'rti_install'?")
        endif()
        set(RTI_CONNEXT_FILES_PATH 
            ${CONAN_LIB_DIRS_RTI_CONNEXT_DDS}/${CONNEXTDDS_ARCH})
        set(FILE_WILDCARD "*.so")
    endif()

    install(DIRECTORY ${RTI_CONNEXT_FILES_PATH}/ DESTINATION ${DESTINATION}
        FILES_MATCHING PATTERN ${FILE_WILDCARD})
    install(FILES USER_QOS_PROFILES.xml DESTINATION ${DESTINATION})

endmacro(rti_install DESTINATION)


################################################################################
## \page page_cmake_commands
# <hr>
# <b>rti_deploy(\<name\>)</b>
#
# This macro copies the rti libraries to the location next to the target 
#   \<name\>. Furthermore it copies the 'USER_QOS_PROFILES.xml' which is 
#   expected to lay next to the 'CMakeLists.txt' which called that macro. 
#
# Arguments:
# \li \<name\>:
# The name of the target that needs the rti libraries.
################################################################################
macro(rti_deploy NAME)
    if(MSVC)    
        set(RTI_CONNEXT_FILES_PATH ${CONAN_BIN_DIRS_RTI_CONNEXT_DDS})
    else()
        if(NOT CONNEXTDDS_ARCH)
            message(FATAL_ERROR "CONNEXTDDS_ARCH variable is empty. "
                "Did you forget a 'find_package(RTIConnextDDS ...)' "
                "for the target '${NAME}'?")
        endif()
        set(RTI_CONNEXT_FILES_PATH 
            ${CONAN_LIB_DIRS_RTI_CONNEXT_DDS}/${CONNEXTDDS_ARCH})
        set(FILE_WILDCARD "*.so")
    endif()

    if(MSVC)
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${RTI_CONNEXT_FILES_PATH}/rtimonitoring$<$<CONFIG:Debug>:d>.dll
                ${RTI_CONNEXT_FILES_PATH}/rticonnextmsgcpp$<$<CONFIG:Debug>:d>.dll
                ${RTI_CONNEXT_FILES_PATH}/nddsmetp$<$<CONFIG:Debug>:d>.dll
                ${RTI_CONNEXT_FILES_PATH}/nddscpp2$<$<CONFIG:Debug>:d>.dll
                ${RTI_CONNEXT_FILES_PATH}/nddscpp$<$<CONFIG:Debug>:d>.dll
                ${RTI_CONNEXT_FILES_PATH}/nddscore$<$<CONFIG:Debug>:d>.dll
                ${RTI_CONNEXT_FILES_PATH}/nddsc$<$<CONFIG:Debug>:d>.dll
                $<TARGET_FILE_DIR:${NAME}>)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_custom_command(TARGET ${NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${RTI_CONNEXT_FILES_PATH}/rtimonitoringd.pdb
                    ${RTI_CONNEXT_FILES_PATH}/rticonnextmsgcppd.pdb
                    ${RTI_CONNEXT_FILES_PATH}/nddsmetpd.pdb
                    ${RTI_CONNEXT_FILES_PATH}/nddscpp2d.pdb
                    ${RTI_CONNEXT_FILES_PATH}/nddscppd.pdb
                    ${RTI_CONNEXT_FILES_PATH}/nddscored.pdb
                    ${RTI_CONNEXT_FILES_PATH}/nddscd.pdb
                    $<TARGET_FILE_DIR:${NAME}>)
        endif()
    else()
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${RTI_CONNEXT_FILES_PATH}/${FILE_WILDCARD}
                $<TARGET_FILE_DIR:${NAME}>)
    endif()

    add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/USER_QOS_PROFILES.xml
                $<TARGET_FILE_DIR:${NAME}>)
endmacro(rti_deploy NAME)
