/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <fep3/fep3_errors.h>
#include <fep3/components/base/component_iid.h>
#include <memory>
#include <string>
#include <map>
#include <chrono>
#include "rpc/rpc_intf.h"

namespace fep3
{
namespace arya
{
    /**
     * @brief Service Bus component interface definition
     * 
     */
    class FEP3_PARTICIPANT_EXPORT IServiceBus
    {
    protected:
        /**
         * @brief DTOR
         * @note This DTOR is explicitly protected to prevent destruction via this interface.
         */
        ~IServiceBus() = default;

    public:
        /**
         * @brief definition of the component interface identifier for the IServiceBus
         * @see IComponents
         */
        FEP_COMPONENT_IID("service_bus.arya.fep3.iid");

    public:
        /**
         * @brief Alias class that represent a Participant in the System
         */
        using IParticipantServer       = rpc::arya::IRPCServer;
        /**
         * @brief Alias class that represents the possibility to request service functionality of a far participant
         */
        using IParticipantRequester    = rpc::arya::IRPCRequester;
        /**
         * @brief the system access may represent one participant within one system.
         * A participant will only appear within this system if the \c createServer was called! 
         */
        class ISystemAccess
        {
        public:
            /**
             * @brief DTOR
             *
             */
            virtual ~ISystemAccess() = default;
        public:
            /**
             * @brief create a server object
             * The server itself is one access point to the service bus, where services can be registered and unregistered.
             *
             * @param server_name name of the server (within FEP 3 this is the participant name to appear)
             *                    within one service bus instance this name has to be unique. Usually the implementation
             *                    should also prevent to create a server with the same name twice within the discovery_network_url
             *                    (but this depends on the used protocol if this is possible)
             * @param server_url address of the server :
             *                   \li for http this will be http://localhost:9090 or http://0.0.0.0:9090
             *                                             to appear in all networks
             *                   \li for http this will be http://192.168.1.2:9090 to appear only in http://192.168.1.2.x networks
             *                   \li for rti dds this will be rti-dds://fep3_rpc_topic\@system_name:23 to appear on domain 23 
                                                           to use fep3_rpc_topic_system_name as topic on domain 23
             *
             * @return if an error occurs the server object of the system access is empty
             * @retval ERR_NOERROR successfully created
             * 
             * @remark if this function is called while a server was already created before, the current server will be release. 
             *         if creating of the new server fails the server object will be empty.
             * 
             */
            virtual fep3::Result createServer(const std::string& server_name,
                                              const std::string& server_url) = 0;
            /**
             * @brief releases the server
             * every service connection is stopped!
             *
             */
            virtual void releaseServer() = 0;

            /**
             * @brief get the participant server with the given name
             *
             * @return the server if created already
             * @retval empty std::shared_ptr<IRPCServer>() no server created yet. call \c createServer before!
             */
            virtual std::shared_ptr<IParticipantServer> getServer() const = 0;

            /**
             * @brief get a requester to request service call at the participant with \c far_participant_name by within the system access
             * @remark the participant must belong to the same system otherwise no communication is possible
             *
             * @param far_participant_name name of the far server
             * @return a requester to request messages from
             * @retval empty std::shared_ptr<IRPCRequester>() if far_participant_name address can not be retrieved
             */
            virtual std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_participant_name) const = 0;

            /**
             * @brief discover servers on the given systems address and system name
             * @param timeout the timeout waiting on the discover message answers
             *
             * @return returns a set of server name/server address pairs
             */
            virtual std::multimap<std::string, std::string> discover(std::chrono::milliseconds timeout) const = 0;

            /**
             * @brief retrieves the name of the system access (used as system_name)
             *
             * @return returns the anme of the system access
             */
            virtual std::string getName() const = 0;

            /**
             * @brief default option for addresses within the functions:
             * \li ISystemAccess::discover
             * \li ISystemAccess::createServer
             */
            static constexpr const char* const _use_default_url = "use_default_url";
            /**
             * @brief default option for service_discovery of ALL System within the functions:
             * \li IServiceBus::createSystemAccess
             * \li will influence the result of ISystemAccess::discover
             */
            static constexpr const char* const _discover_all_systems = "fep3:search_all_systems";
        };
        /**
         * @brief create a system access point to create a server (to be part of the system) or to discover and request within this system. 
         * The server itself is one access point to the service bus, where services can registered and unregistered to.
         * 
         * @param system_name the name of the system this server is belonging to
         *                    \li leave empty if there is no system this participant belongs to
         *                    \li set the name of the system. this usually is to discover only servers with the same system name
         * @param system_discovery_url the network address url to communicate with if discovery is used
         *                    \li for http we could set up i.e. a multicast address and a port
         *                    \li for rti dds we could set up rti-dds://fep3_default_discovery:0 to appear on domain 0
         *                        and answer a discovery call (this is usually a functionality already supported by rti)
         *                    \li if no discovery is used leave it empty
         * @param is_default create this access point as default system access to have easy access via IServiceBus::getServer 
         *                    and IServiceBus::getRequester which is used within the RPC implementation templates
         *                    
         * @retval ERR_NOERROR no error occured 
         * @return depending on the service bus solution it will return with an error. see details of that error
         */
        virtual fep3::Result createSystemAccess(const std::string& system_name,
                                                const std::string& system_discovery_url,
                                                bool is_default=false) = 0;
        /**
         * @brief releases the system access with the given \p system_name 
         * 
         * 
         * @param system_name name of the system access
         * @retval ERR_NOERROR no error occured
         * 
         */
        virtual fep3::Result releaseSystemAccess(const std::string& system_name) = 0;


        /**
         * @brief get the participants server at the default system access
         * @return the server if it does exist
         * @retval empty std::shared_ptr<IRPCServer>() if the server is not set
         * @remark this will only return a valid value if a systemaccess  is created with 
         *         \c createSystemAccess(..., ..., true) and within this access the server was created
         */
        virtual std::shared_ptr<IParticipantServer> getServer() const = 0;
        
        /**
         * @brief get a requester to connect a \p far_participant_server_name within the default system access
         * depending on the implementation this is used only together with discovery switched on
         *
         * @param far_participant_server_name name of the far server
         * @return a requester to request messages from
         */
        virtual std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_participant_server_name) const = 0;

        /**
        * @brief get the default server with the given name
        * @param system_name name of the server
        * @return the server if it does exist
        * @retval empty std::shared_ptr<ISystemAccess>() the server if it does not exist
        */
        virtual std::shared_ptr<ISystemAccess> getSystemAccess(const std::string& system_name) const = 0;

        /**
         * @brief get a requester to connect a server with the full URL
         *
         * @param far_server_url address of the far server to get a request for
         * @param is_url mark the function to be the is_url version (this param will not be used!)
         * @return a requester to request messages from
         * @retval empty std::shared_ptr<IRPCRequester>() if it could not be created
         * @throws std::runtime_error if url is not supported
         * @throws other for url parse error 
         */
        virtual std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_server_url, bool is_url) const = 0;
    };
}

using arya::IServiceBus;
}


