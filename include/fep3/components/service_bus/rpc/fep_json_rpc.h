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
#include <jsonrpccpp/client/iclientconnector.h>
#include <jsonrpccpp/common/exception.h>
#include <jsonrpccpp/server/abstractserverconnector.h>
#include <rpc_pkg/rpc_server.h>
#include <rpc_pkg/json_rpc.h>

#include <fep3/fep3_participant_types.h>
#include <fep3/components/service_bus/rpc/rpc_intf.h>

 ///@cond nodoc
namespace fep3
{
namespace rpc
{
namespace arya
{
namespace detail
{
    struct ClientConnectorInitializerType
    {
        ClientConnectorInitializerType(
            const std::string& service_name,
            const std::shared_ptr<IRPCRequester>& rpc) :
            _service_name(service_name),
            _rpc(rpc)
        {
        }
        std::string   _service_name;
        std::shared_ptr<IRPCRequester> _rpc;
    };

    /**
        * Connector that sends RPC messages
        */
    class JSONFEPClientConnector : public ::jsonrpc::IClientConnector
    {
        private:
            struct StringResponse : public arya::IRPCRequester::IRPCResponse
            {
                std::string& _bounded_string;
                StringResponse(std::string& string_to_bind) : _bounded_string(string_to_bind)
                {
                }
                fep3::Result set(const std::string& response)
                {
                    _bounded_string = response;
                    return {};
                }
            };
        public:
            /**
                * Constructor
                */
            JSONFEPClientConnector(const ClientConnectorInitializerType& rpc_service_client_info) :
                _init_info(rpc_service_client_info)
            {
            }
            virtual ~JSONFEPClientConnector()
            {
            }
        public:
            void SendRPCMessage(const std::string& message, std::string& result)
            {
                if (_init_info._rpc)
                {
                    auto requester = _init_info._rpc;
                    StringResponse response(result);
                    auto res = requester->sendRequest(
                        _init_info._service_name,
                        message,
                        response);
                    if (isFailed(res))
                    {
                        throw std::runtime_error(
                            "error while performing call : " + message + " - "+ std::string(res.getDescription()));
                    }
                }
            }

        protected:
            ClientConnectorInitializerType _init_info;
    };


    class  JSONFEPServerConnector : public jsonrpc::AbstractServerConnector
    {
    public:
        bool StartListening() override
        {
            return true;
        }

        bool StopListening() override
        {
            return false;
        }

        bool SendResponse(const std::string& response, void* addInfo)
        {
            ::rpc::IResponse* pResponse = static_cast<::rpc::IResponse*>(addInfo);
            pResponse->Set(response.data(), response.size());
            return true;
        }

        bool OnRequest(const std::string& request, ::rpc::IResponse* response)
        {
            std::string response_value;
            ProcessRequest(request, response_value);
            response->Set(response_value.c_str(), response_value.size());
            return true;
        }
    };

    class FEPResponseToRPCResponse : public ::rpc::IResponse
    {
    protected:
        ::fep3::rpc::IRPCRequester::IRPCResponse& _response_ref;
    public:
        FEPResponseToRPCResponse(::fep3::rpc::IRPCRequester::IRPCResponse& response_ref) :
            _response_ref(response_ref)
        {
        }

        void Set(const char* strResponse, size_t nResponseSize)
        {
            if (strResponse[nResponseSize - 1] != '\0')
            {
                std::string strNullTerminatedString(strResponse, nResponseSize);
                _response_ref.set(strNullTerminatedString);
            }
            else
            {
                _response_ref.set(strResponse);
            }
        }
    };
}   //namespace detail
} //ns arya
} //ns rpc
}//ns fep3
 ///@endcond nodoc
