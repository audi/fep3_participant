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

#include <fep3/components/service_bus/service_registry_base.hpp>
#include <mutex>

#pragma warning( push )
#pragma warning( disable : 4290)
#include <rpc_pkg.h>
#pragma warning( pop ) 

#include <../3rdparty/lssdp-cpp/src/lssdpcpp/lssdpcpp.h>
#include <thread>

namespace fep3
{
namespace native
{

class HttpServer : public fep3::base::arya::ServiceRegistryBase
{
    public:
        struct RPCObjectToRPCServerWrapper : public ::rpc::IRPCObject
        {
            public:
                explicit RPCObjectToRPCServerWrapper(const std::shared_ptr<IRPCService>& service);
                virtual ~RPCObjectToRPCServerWrapper() = default;
                a_util::result::Result HandleCall(const char* strRequest,
                                                size_t nRequestSize,
                                                ::rpc::IResponse& oResponse);
                std::shared_ptr<IRPCServer::IRPCService> getService() const;
            private:
                std::shared_ptr<IRPCServer::IRPCService> _service;
        };

    public:
        HttpServer(const std::string& name,
                   const std::string& url,
                   const std::string& system_name,
                   const std::string& system_url);
        virtual ~HttpServer();

    public: //implementation of ServiceRegistryBase
        fep3::Result registerService(const std::string& service_name,
                                     const std::shared_ptr<IRPCService>& service) override;
        fep3::Result unregisterService(const std::string& service_name) override;
        std::string getUrl() const override;

        std::vector<std::string> getRegisteredServiceNames() const override;
        std::shared_ptr<rpc::arya::IRPCServer::IRPCService> getServiceByName(const std::string& service_name) const override;

    public: //default url of this implementation
        static constexpr const char* const _default_url = "http://0.0.0.0:0";
        static constexpr const char* const _discovery_search_target = "fep3:servicebus:http:participant";

    private:
        ::rpc::http::cJSONRPCServer _http_server;
        std::map<std::string, std::shared_ptr<RPCObjectToRPCServerWrapper>> _service_wrappers;
        mutable std::recursive_mutex _sync_wrappers;
        bool _is_started = false;

        void checkUrlAndSetDefaultIfNecessary();
        std::string _url;
        std::string _system_url;
        std::unique_ptr<lssdp::Service> _lssdp_service;
        void startDiscovery(std::chrono::seconds interval);
        void stopDiscovery();
        std::atomic<bool>  _stop_loop;
        std::thread        _loop;
};



}
}
