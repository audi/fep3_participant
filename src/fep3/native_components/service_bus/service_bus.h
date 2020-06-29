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

#include "fep3/components/service_bus/service_bus_intf.h"
#include "fep3/components/base/component_base.h"

#include "fep3/components/logging/logging_service_intf.h"

#include "service_bus_logger.hpp"

namespace fep3
{
//not yet sure if this is really necessary within the implementation
namespace native
{
    //servicebus implementation supporting the arya service bus
    class ServiceBus : public fep3::ComponentBase<fep3::arya::IServiceBus>,
                       public service_bus_helper::ILogSink
    {
        public:
            ServiceBus();
            virtual ~ServiceBus() ;
            ServiceBus(const ServiceBus&) = delete;
            ServiceBus(ServiceBus&&) = delete;
            ServiceBus& operator=(const ServiceBus&) = delete;
            ServiceBus& operator=(ServiceBus&&) = delete;

        public: //the arya ServiceBus interface
            fep3::Result createSystemAccess(const std::string& system_name,
                                            const std::string& system_discovery_url,
                                            bool is_default=false) override;
            fep3::Result releaseSystemAccess(const std::string& system_name) override;

            std::shared_ptr<IParticipantServer> getServer() const override;
            std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_server_name) const override;

            std::shared_ptr<ISystemAccess> getSystemAccess(const std::string& system_name) const override;
            std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_server_url, bool is_url) const override;
        
        public: //override ComponentBase
            fep3::Result create() override;
            fep3::Result destroy() override;

        public:
            void internalLog(const std::string& logmessage) override;

        private:
            class Impl;
            std::unique_ptr<Impl> _impl;
            std::shared_ptr<fep3::ILoggingService::ILogger> _logger;
            void logError(const fep3::Result& res) const;
            void logError(const std::string& log_message) const;
    };
}
}