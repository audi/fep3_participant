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

#include "service_bus.h"
#include "rpc/http/http_systemaccess.h"
#include <../3rdparty/lssdp-cpp/src/url/cxx_url.h>
#include "rpc/http/http_server.h"
#include "rpc/http/http_client.h"
#include <a_util/result.h>

namespace fep3
{
namespace native
{

class ServiceBusDefaults : public fep3::base::SystemAccessBase::ISystemAccessBaseDefaultUrls
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
        //if this file does not exist we use this bus default url which is from HTTP SystemAccess
        return fep3::native::HttpSystemAccess::_default_url;
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
        return fep3::native::HttpServer::_use_default_url;
    }
};

class ServiceBus::Impl
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
        if (_locked)
        {
            SB_LOG_AND_RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                "Can not create system access. Invalid state for creation of '%s' on '%s'",
                system_name.c_str(),
                system_url.c_str());
        }
        std::string used_system_url = system_url;
        auto server_found = findSystemAccess(system_name);
        if (server_found)
        {
            SB_LOG_AND_RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, "Can not create system access point '%s'. System name '%s' already exists",
                system_name.c_str(),
                system_name.c_str());
        }
        std::shared_ptr<base::SystemAccessBase::ISystemAccessBaseDefaultUrls> service_bus_system_default = std::make_shared<ServiceBusDefaults>();
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
            else
            {
                fep3::helper::Url url(system_url);
                if (url.scheme() != "http")
                {
                    RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                        "Can not create system access '%s'. This service bus does only support 'http' protocol, but it is called with '%s'",
                        system_name.c_str(),
                        system_url.c_str());
                }
                used_system_url = system_url;
            }
        }
        catch (const std::exception& exc)
        {
            SB_LOG_AND_RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                "Can not create system access '%s'. url '%s' is not well formed. %s",
                system_name.c_str(),
                system_url.c_str(),
                exc.what());
        }

        std::shared_ptr<HttpSystemAccess> system_access = {};
        try
        {
            system_access = std::make_shared<HttpSystemAccess>(system_name,
                used_system_url,
                service_bus_system_default);
        }
        catch (const std::exception& ex)
        {
            SB_LOG_AND_RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY,
                "Can not create system access '%s' - %s : %s",
                system_name.c_str(),
                system_url.c_str(),
                ex.what());
        }
        _system_accesses.push_back(system_access);
        if (set_as_default)
        {
            _default_system_access = system_access;
        }

        return {};
    }

    fep3::Result releaseSystemAccess(const std::string& system_name)
    {
        if (_locked)
        {
            SB_LOG_AND_RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                "Can not release system access '%s'. service bus locked",
                system_name.c_str());
        }
        for (decltype(_system_accesses)::iterator it = _system_accesses.begin();
            it != _system_accesses.end();
            ++it)
        {
            if ((*it)->getName() == system_name)
            {
                _system_accesses.erase(it);
                if (_default_system_access && _default_system_access->getName() == system_name)
                {
                    _default_system_access.reset();
                }
                return {};
            }
        }
        SB_LOG_AND_RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, "Can not find system access '%s' to destroy it",
            system_name.c_str());
        
    }
    std::shared_ptr<IServiceBus::ISystemAccess> findSystemAccess(const std::string& system_name) const
    {
        for (const auto& current_access : _system_accesses)
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

    void lock()
    {
        _locked = true;
        for (auto& sys_access : _system_accesses)
        {
            sys_access.get()->lock();
        }
    }

    void unlock()
    {
        for (auto& sys_access : _system_accesses)
        {
            sys_access.get()->unlock();
        }
        _locked = false;
    }

    std::vector<std::shared_ptr<HttpSystemAccess>> _system_accesses;
    std::shared_ptr<IServiceBus::ISystemAccess> _default_system_access;
    std::atomic<bool> _locked;
};


ServiceBus::ServiceBus() : _impl(std::make_unique<ServiceBus::Impl>())
{
}

ServiceBus::~ServiceBus()
{
}

fep3::Result ServiceBus::createSystemAccess(const std::string& system_name,
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

fep3::Result ServiceBus::releaseSystemAccess(const std::string& system_name)
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

std::shared_ptr<ServiceBus::ISystemAccess> ServiceBus::getSystemAccess(const std::string& system_name) const
{
    return _impl->findSystemAccess(system_name);
}

std::shared_ptr<IServiceBus::IParticipantServer> ServiceBus::getServer() const
{
    auto system_access = _impl->getDefaultAccess();
    if (system_access)
    {
        return system_access->getServer();
    }
    return {};
}

std::shared_ptr<IServiceBus::IParticipantRequester> ServiceBus::getRequester(const std::string& far_server_name) const
{
    auto system_access = _impl->getDefaultAccess();
    if (system_access)
    {
        return system_access->getRequester(far_server_name);
    }
    return {};
}

std::shared_ptr<IServiceBus::IParticipantRequester> ServiceBus::getRequester(const std::string& far_server_address, bool) const
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
            return std::make_shared<HttpClientConnector>(far_server_address);
        }
    }
    catch (const std::exception& exc)
    {
        logError(std::string("could not create requester for the ") + far_server_address + ": " + exc.what());
    }
    return {};
}

void ServiceBus::logError(const fep3::Result& res) const
{
    if (_logger)
    {
        if (_logger->isErrorEnabled())
        {
            _logger->logError(a_util::result::toString(res));
        }
    }
}

void ServiceBus::logError(const std::string& logmessage) const
{
    if (_logger)
    {
        if (_logger->isErrorEnabled())
        {
            _logger->logError(logmessage);
        }
    }
}

void ServiceBus::internalLog(const std::string& logmessage)
{
    if (_logger)
    {
        if (_logger->isErrorEnabled())
        {
            _logger->logError(logmessage);
        }
    }
}

fep3::Result ServiceBus::create()
{
    _impl->lock();
    service_bus_helper::Logger::get().add(this);
    return {};
}

fep3::Result ServiceBus::destroy()
{
    service_bus_helper::Logger::get().remove(this);
    _impl->unlock();
    return {};
}



}
}