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

#include <../3rdparty/lssdp-cpp/src/url/cxx_url.h>
#include <fep3/components/service_bus/service_registry_base.hpp>
#include <map>
#include <memory>
#include <mutex>

namespace fep3
{
namespace rti_dds
{

class DDSServer : public fep3::base::arya::ServiceRegistryBase
{
    public:
        struct DDSReceiverToRPCServiceWrapper
        {
            public:
                explicit DDSReceiverToRPCServiceWrapper(const std::shared_ptr<IRPCService>& service);
                virtual ~DDSReceiverToRPCServiceWrapper() = default;
                a_util::result::Result HandleCall(const char* strRequest,
                                                  size_t nRequestSize);
                std::shared_ptr<IServiceBus::IParticipantServer::IRPCService> getService() const;
            private:
                std::shared_ptr<IServiceBus::IParticipantServer::IRPCService> _service;
        };

    public:
        DDSServer(const std::string& name,
                  const std::string& url,
                  const std::string& system_name);
        virtual ~DDSServer();

    public: //rpc service ... needs to override it
        std::vector<std::string> getRegisteredServiceNames() const override;
        std::shared_ptr<rpc::arya::IRPCServer::IRPCService> getServiceByName(const std::string& service_name) const override;

    public: //internal rpc server... needs to override it
        fep3::Result registerService(const std::string& service_name,
            const std::shared_ptr<IRPCService>& service) override;
        fep3::Result unregisterService(const std::string& service_name) override;
        std::string getUrl() const override;

    private:
        std::map<std::string, std::shared_ptr<DDSReceiverToRPCServiceWrapper>> _service_wrappers;
        std::string _url;
        mutable std::recursive_mutex _sync_list;
};

}
}
