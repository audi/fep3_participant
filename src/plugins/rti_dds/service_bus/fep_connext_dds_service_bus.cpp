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
#include "fep_connext_dds_service_bus.h"
#include <fep3/native_components/service_bus/rpc/http/http_systemaccess.h>
#include <fep3/native_components/service_bus/rpc/http/http_server.h>
#include <fep3/native_components/service_bus/rpc/http/http_client.h>

#include "rti_dds_server.h"
#include "rti_dds_client.h"
#include "rti_dds_system_access.h"

#include <../3rdparty/lssdp-cpp/src/url/cxx_url.h>
#include <fep3/fep3_errors.h>


class ServiceBusDDS_HTTPDefaults : public fep3::base::SystemAccessBase::ISystemAccessBaseDefaultUrls
{
public:
    std::string getDefaultSystemUrl() const override
    {
        //we do following in this order
        //1. we have a look in a Environment variable
        //   FEP3_SERVICEBUS_DEFAULT_SYSTEM_URL=
        //if this is empty then
        //2. here we usually open the service_bus.configuration file to have a look into it
        //
        //if this file does not exist we use this bus default url which is from HTTP Server
        return {};
    }
    std::string getDefaultServerUrl() const override
    {
        //we do following in this order
        //1. we have a look in a Environment variable
        //   FEP3_SERVICEBUS_DEFAULT_SERVER_URL=
        //if this is empty then
        //2. here we usually open the service_bus.configuration file to have a look into it
        //
        //if this file does not exist we use this bus default url which is from HTTP Server
        return fep3::rti_dds::DDSServer::_use_default_url;
    }
};

class ServicBusDDS_HTTP::Impl
{
public:
    Impl()
    {
        _locked = false;
    }
    ~Impl()
    {
    }

    fep3::Result createSystemAccess(const std::string& system_name,
        const std::string& system_url,
        bool set_as_default)
    {
        enum
        {
            rti_dds,
            http
        } system_access_scheme_to_use = rti_dds;

        std::string used_system_url = system_url;
        auto server_found = findSystemAccess(system_name);
        if (server_found)
        {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG,
                "service bus: can not create system access point '%s'. System name '%s' already exists",
                system_name.c_str(),
                system_name.c_str());
        }
        std::shared_ptr<fep3::base::SystemAccessBase::ISystemAccessBaseDefaultUrls> service_bus_system_default 
            = std::make_shared<ServiceBusDDS_HTTPDefaults>();
        try
        {
            if (system_url == IServiceBus::ISystemAccess::_use_default_url)
            {
                used_system_url = service_bus_system_default->getDefaultSystemUrl();
            }
            else if (system_url.empty())
            {
                //is valid we do not want to use discovery 
                //we know all addresses from outside!
                used_system_url = {};
            }
            //we do not open any url 
            if (!used_system_url.empty())
            {
                fep3::helper::Url url(system_url);
                if (url.scheme() == "http")
                {
                    system_access_scheme_to_use = http;
                }
                else if (url.scheme() == "rti_dds")
                {
                    system_access_scheme_to_use = rti_dds;
                }
                else
                {
                    RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG,
                        "service bus: can not create system access '%s'. This service bus does only support 'http' protocol, but it is called with '%s'",
                        system_name.c_str(),
                        system_url.c_str());
                }
                used_system_url = system_url;
            }
        }
        catch (const std::exception& exc)
        {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG,
                "service bus: can not create system access '%s'. url '%s' is not well formed. %s",
                system_name.c_str(),
                system_url.c_str(),
                exc.what());
        }

        std::shared_ptr<IServiceBus::ISystemAccess> system_access;
        if (system_access_scheme_to_use == http)
        {
            system_access = std::make_shared<fep3::native::HttpSystemAccess>(system_name,
                used_system_url,
                service_bus_system_default);
        }
        else //(system_access_scheme_to_use == rti_dds)
        {
            system_access = std::make_shared<fep3::rti_dds::DDSSystemAccess>(system_name,
                used_system_url,
                service_bus_system_default);
        }

        _system_access.push_back(system_access);
        if (set_as_default)
        {
            _default_system_access = system_access;
        }

        return {};
    }

    fep3::Result releaseSystemAccess(const std::string& system_name)
    {
        for (decltype(_system_access)::iterator it = _system_access.begin();
            it != _system_access.end();
            ++it)
        {
            if ((*it)->getName() == system_name)
            {
                _system_access.erase(it);
                if (_default_system_access && _default_system_access->getName() == system_name)
                {
                    _default_system_access.reset();
                }
                return {};
            }
        }
        RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG, "service bus: can not find system access '%s' to destroy it",
            system_name.c_str());

    }
    std::shared_ptr<IServiceBus::ISystemAccess> findSystemAccess(const std::string& system_name) const
    {
        for (const auto& current_access : _system_access)
        {
            if (current_access->getName() == system_name)
            {
                return current_access;
            }
        }
        return {};
    }
    std::shared_ptr<IServiceBus::ISystemAccess> getDefaultAccess()
    {
        return _default_system_access;
    }

    std::vector<std::shared_ptr<IServiceBus::ISystemAccess>> _system_access;
    std::shared_ptr<IServiceBus::ISystemAccess> _default_system_access;
    std::atomic<bool> _locked;
};


ServicBusDDS_HTTP::ServicBusDDS_HTTP() : _impl(std::make_unique<ServicBusDDS_HTTP::Impl>())
{
}

ServicBusDDS_HTTP::~ServicBusDDS_HTTP()
{
}

fep3::Result ServicBusDDS_HTTP::createSystemAccess(const std::string& system_name,
    const std::string& system_discovery_url,
    bool is_default)
{
    auto res = _impl->createSystemAccess(system_name,
        system_discovery_url,
        is_default);
    if (fep3::isFailed(res))
    {
        logError(res);
        return res;
    }
    else
    {
        return {};
    }
}

fep3::Result ServicBusDDS_HTTP::releaseSystemAccess(const std::string& system_name)
{
    auto res = _impl->releaseSystemAccess(system_name);
    if (fep3::isFailed(res))
    {
        logError(res);
        return res;
    }
    else
    {
        return {};
    }
}

std::shared_ptr<IServiceBus::ISystemAccess> ServicBusDDS_HTTP::getSystemAccess(const std::string& system_name) const
{
    return _impl->findSystemAccess(system_name);
}

std::shared_ptr<IServiceBus::IParticipantServer> ServicBusDDS_HTTP::getServer() const
{
    auto system_access = _impl->getDefaultAccess();
    if (system_access)
    {
        return system_access->getServer();
    }
    return {};
}

std::shared_ptr<IServiceBus::IParticipantRequester> ServicBusDDS_HTTP::getRequester(const std::string& far_server_name) const
{
    auto system_access = _impl->getDefaultAccess();
    if (system_access)
    {
        return system_access->getRequester(far_server_name);
    }
    return {};
}

std::shared_ptr<IServiceBus::IParticipantRequester> ServicBusDDS_HTTP::getRequester(const std::string& far_server_address, bool) const
{
    try
    {
        fep3::helper::Url url_check(far_server_address);
        auto scheme = url_check.scheme();
        if (scheme != "http")
        {
            logError("could not create requester for the " + far_server_address + ": invalid protocol. only http supported.");
        }
        else
        {
            return std::make_shared<fep3::rti_dds::DDSRequester>(far_server_address);
        }
    }
    catch (const std::exception& exc)
    {
        logError(std::string("could not create requester for the ") + far_server_address + ": " + exc.what());
    }
    return {};
}

void ServicBusDDS_HTTP::logError(const fep3::Result& res) const
{
    if (_logger)
    {
        if (_logger->isErrorEnabled())
        {
            _logger->logError(a_util::result::toString(res));
        }
    }
}

void ServicBusDDS_HTTP::logError(const std::string& logmessage) const
{
    if (_logger)
    {
        if (_logger->isErrorEnabled())
        {
            _logger->logError(logmessage);
        }
    }
}
