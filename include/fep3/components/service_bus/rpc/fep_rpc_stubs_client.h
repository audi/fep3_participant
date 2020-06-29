/**
 * @file
 * @copyright AUDI AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */
#pragma once

#include <string>
#include <a_util/result/result_type.h>
#include <jsonrpccpp/server/abstractserverconnector.h>

#include "rpc_intf.h"
#include "fep_json_rpc.h" 
#include "fep3/rpc_services/base/fep_rpc_client_intf.h"


namespace fep3
{
namespace rpc
{
namespace arya
{

    /**
    * \brief Parent class of all rpc clients.
    * \tparam Stub Generated client stub.
    * \tparam Interface Server id interface.
    */
    template <typename rpc_stub, typename rpc_interface>
    class RPCServiceClient :
        public ::rpc::jsonrpc_remote_object<rpc_stub, detail::JSONFEPClientConnector, detail::ClientConnectorInitializerType>,
        public IRPCServiceClient
    {

    protected:
        RPCServiceClient() = delete;
    public:
        /// The json rpc base class
        typedef ::rpc::jsonrpc_remote_object<rpc_stub, detail::JSONFEPClientConnector, detail::ClientConnectorInitializerType> base_class;

        /**
         * CTOR
         *
         * @param [in] service_name The name of the RPC object / RPC component we connect to
         * @param [in] rpc_requester The RPC implementation
         */
        explicit RPCServiceClient(const std::string& service_name,
            const std::shared_ptr<IRPCRequester>& rpc_requester) :
            _service_name(service_name),
            base_class(detail::ClientConnectorInitializerType(service_name,
                                                              rpc_requester))
        {
            // Setting default timeout parameter of client to property tree
            //oModuleToBind.GetPropertyTree()->SetPropertyValue(FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, 5000);
        }

        /**
         * @retval The ID of the bound rpc server
         */
        std::string getRPCServiceIID() const override
        {
            return rpc_interface::getRPCIID();
        }

        /**
         * @retval The ID of the bound rpc server
         */
        std::string getRPCServiceDefaultName() const override
        {
            return rpc_interface::getRPCDefaultName();
        }

        /**
         * Gets the name of the service
         * @retval The Name of the bound rpc server
         */
        std::string getRPCServiceName() const override
        {
            return _service_name;
        }

    private:
        ///the name of the current service this client belongs to
        std::string _service_name;
    };

    /**
        * \brief Parent class of all rpc clients.
        * \tparam Stub Generated client stub.
        * \tparam Interface Server id interface.
        */
    template <typename rpc_stub, typename rpc_interface>
    class RPCServiceClientProxy :
        public ::rpc::jsonrpc_remote_interface<rpc_stub, rpc_interface, detail::JSONFEPClientConnector, detail::ClientConnectorInitializerType>,
        public IRPCServiceClient
    {

    protected:
        RPCServiceClientProxy() = delete;
    public:
        /// The json rpc base class
        typedef ::rpc::jsonrpc_remote_interface<rpc_stub, rpc_interface, detail::JSONFEPClientConnector, detail::ClientConnectorInitializerType> base_class;

        /**
        * CTOR
        *
        * @param [in] service_name The name of the RPC service / RPC component we connect to
        * @param [in] rpc The RPC implementation
        */
        explicit RPCServiceClientProxy(const std::string& service_name,
            const std::shared_ptr<IRPCRequester>& rpc) :
            _service_name(service_name),
            base_class(detail::ClientConnectorInitializerType(
                service_name, rpc))
        {
            // Setting default timeout parameter of client to property tree
            //oModuleToBind.GetPropertyTree()->SetPropertyValue(FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, 5000);
        }

        /**
            * @retval The ID of the bound rpc server
            */
        std::string getRPCServiceIID() const override
        {
            return fep3::rpc::getRPCIID<rpc_interface>();
        }

        /**
         * @retval The ID of the bound rpc server
         */
        std::string getRPCServiceDefaultName() const override
        {
            return fep3::rpc::getRPCDefaultName<rpc_interface>();
        }
        /**
         * Gets the name of the service
         * @retval The Name of the bound rpc server
         */
        std::string getRPCServiceName() const override
        {
            return _service_name;
        }

    protected:
        ///the name of the current service this client belongs to
        std::string _service_name;
    };
} //ns arya

using arya::RPCServiceClient;
using arya::RPCServiceClientProxy;
} //ns rpc
} //ns fep3





