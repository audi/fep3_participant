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
# <b>fep3_participant_install(NAME \<name\> DESTINATION \<destination\>)</b>
#
# This macro installs the target \<name\>, together with the FEP SDK Participant libraries (if neccessary)
#   to the folder \<destination\>
# Arguments:
# \li \<name\>:
# The name of the library to install.
# \li \<destination\>:
# The relative path to the install subdirectory
################################################################################
macro(fep3_participant_install NAME DESTINATION)
    install(TARGETS ${NAME} DESTINATION ${DESTINATION})
    install(
        FILES
            $<TARGET_FILE:fep3_participant>
        DESTINATION ${DESTINATION}
    )
    
    if(MSVC)
        install(FILES $<TARGET_FILE_DIR:fep3_participant>/fep3_participant$<$<CONFIG:Debug>:d>${fep3_participant_pdb_version_str}.pdb
                DESTINATION ${DESTINATION} OPTIONAL)
    endif(WIN32)

    if(fep3_participant_use_rtidds)
        install(FILES
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rtimonitoring$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rticonnextmsgcpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsmetp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp2$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE:fep3_connext_dds_plugin>
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/USER_QOS_PROFILES.xml
            DESTINATION ${DESTINATION}/rti
        )

        if(MSVC)
            install(FILES
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/rtimonitoringd.pdb
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/rticonnextmsgcppd.pdb
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddsmetpd.pdb
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscpp2d.pdb
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscppd.pdb
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscored.pdb
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscd.pdb
                DESTINATION ${DESTINATION}/rti
                CONFIGURATIONS Debug
            )
        endif()

        #now we need to install the components file to load the rti plugin
        install(
            FILES
                $<TARGET_FILE_DIR:fep3_participant>/fep3_participant.fep_components
            DESTINATION ${DESTINATION}
        )

    endif()
endmacro(fep3_participant_install NAME DESTINATION)

################################################################################
## \page page_cmake_commands
# <hr>
# <b>fep3_participant_deploy(NAME \<name\>)</b>
#
# This macro deploys the participant library to the same target folder as the target with \<name\>.
# Arguments:
# \li \<name\>: 
# The name of the target to obtain the folder where to copy the participant library 
# binaries to.
################################################################################
macro(fep3_participant_deploy NAME)
    # no need to copy in build directory on linux since linker rpath takes care of that
    if (WIN32)
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep3_participant> $<TARGET_FILE_DIR:${NAME}>
        )
    endif()
    
    if(MSVC)
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE_DIR:fep3_participant>/fep3_participant$<$<CONFIG:Debug>:d>${fep3_participant_pdb_version_str}.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                $<TARGET_FILE_DIR:${NAME}>
        )
    endif()

    if(fep3_participant_use_rtidds)

        add_custom_command(TARGET ${NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    $<TARGET_FILE_DIR:${NAME}>/rti
        )
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rtimonitoring$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rticonnextmsgcpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsmetp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp2$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
                $<TARGET_FILE:fep3_connext_dds_plugin>
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/USER_QOS_PROFILES.xml
                $<TARGET_FILE_DIR:${NAME}>/rti
        )

        if(MSVC)
            add_custom_command(TARGET ${NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/rtimonitoring$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                    $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/rticonnextmsgcpp$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                    $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddsmetp$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                    $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscpp2$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                    $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscpp$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                    $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscore$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                    $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddsc$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                    $<TARGET_FILE_DIR:${NAME}>/rti
            )
        endif()

        #now we need to install the components file to load the rti plugin
        add_custom_command(TARGET ${NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE_DIR:fep3_participant>/fep3_participant.fep_components
                $<TARGET_FILE_DIR:${NAME}>
        )
    endif()
    set_target_properties(${NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
endmacro(fep3_participant_deploy NAME)

