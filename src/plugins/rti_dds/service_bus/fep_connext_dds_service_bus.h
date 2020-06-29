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

#include <fep3/components/base/component_base.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/components/logging/logging_service_intf.h>

using namespace fep3::arya;

class ServicBusDDS_HTTP : public fep3::ComponentBase<IServiceBus>
{
    public:
        ServicBusDDS_HTTP();
        virtual ~ServicBusDDS_HTTP();

    public: //the arya ServiceBus interface
        fep3::Result createSystemAccess(const std::string& system_name,
                                        const std::string& system_discovery_url,
                                        bool is_default = false) override;
        fep3::Result releaseSystemAccess(const std::string& system_name) override;

        std::shared_ptr<IParticipantServer> getServer() const override;
        std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_participant_name) const override;
        std::shared_ptr<ISystemAccess> getSystemAccess(const std::string& system_name) const override;
        std::shared_ptr<IParticipantRequester> getRequester(const std::string& far_server_url, bool is_url) const override;

    private:
        class Impl;
        std::unique_ptr<Impl> _impl;
        std::shared_ptr<ILoggingService::ILogger> _logger;
        void logError(const fep3::Result& res) const;
        void logError(const std::string& logmessage) const;
};

