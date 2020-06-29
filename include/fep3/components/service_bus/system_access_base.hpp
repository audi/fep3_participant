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
#include <memory>
#include <chrono>
#include <atomic>
#include <a_util/result.h>
#include <fep3/components/service_bus/service_bus_intf.h>

/**
 * the default timeout of the address discovery within the fep3::base::SystemAccessBase::getRequester call
 */
#define FEP3_SERVICE_BUS_GET_REQUESTER_TIMEOUT std::chrono::milliseconds(1000)

namespace fep3
{
namespace base
{
namespace arya
{

/**
 * Helper base implementation class to create a ISystemAccess implementation
 * 3 methods to implement:
 * \li \c SystemAccessBase::createAServer
 * \li \c SystemAccessBase::createARequester
 * \li \c SystemAccessBase::getDiscoveredServices
 * 
 */
class SystemAccessBase : public fep3::arya::IServiceBus::ISystemAccess
{
public:
    /**
     * Class interface to obtain certain default urls for system access and server
     * @see fep3::IServiceBus
     */
    class ISystemAccessBaseDefaultUrls
    {
    public:
        /**
         * retrieve the default URL for the system access
         * @see fep3::IServiceBus::createSystemAccess
         * @return std::string the default url for a system
         */
        virtual std::string getDefaultSystemUrl() const = 0;
        /**
         * retrieve the default URL for the server access
         * @see fep3::IServiceBus::ISystemAccess::createServer
         * @return std::string the default url for a server
         */
        virtual std::string getDefaultServerUrl() const = 0;
    };
protected:
    /**
     * CTOR
     * @param[in] system_name the systems name
     * @param[in] system_url the systems url
     * @param[in] default_urls the service bus implementation default urls ... implement \c ISystemAccessBaseDefaultUrls
     */
    SystemAccessBase(const std::string& system_name,
                     const std::string& system_url,
                     const std::shared_ptr<ISystemAccessBaseDefaultUrls>& default_urls) :
        _system_name(system_name), 
        _system_url(system_url),
        _access_default_urls(default_urls)
    {
        _locked = false;
    }
    /** 
     * assignment CTOR
     */
    SystemAccessBase(const SystemAccessBase&) = delete;
    /**
     * assignment CTOR
     */
    SystemAccessBase(SystemAccessBase&&) = delete;
    /**
     * assignment operator
     * @return SystemAccessBase default return value of a assignment operator
     */
    SystemAccessBase& operator=(const SystemAccessBase&) = delete;
    /**
     * move operator
     * @return SystemAccessBase default return value of a move operator
     */
    SystemAccessBase& operator=(SystemAccessBase&&) = delete;

public:
    /**
     * @brief create the server
     * 
     * @param server_name name of the server that appear in the system
     * @param server_url url of the server if necessary 
     * @return the created server. throw if error occurs.
     */
    virtual std::shared_ptr<IServiceBus::IParticipantServer> createAServer(
        const std::string& server_name,
        const std::string& server_url) = 0;
    /**
     * @brief create the requester
     * 
     * @param far_server_name name of the far server that appears in the same system
     * @param far_server_url url of the far server if necessary 
     * @return the created server. throw if error occurs.
     */
    virtual std::shared_ptr<IServiceBus::IParticipantRequester> createARequester(
        const std::string& far_server_name,
        const std::string& far_server_url) const = 0;
    /**
     * @brief retrieves a multimap with pairs of names of the server and their addresses
     * 
     * @param timeout the time waiting for the discover message answers
     * @return the multimap with pairs of names of the server and their addresses
     * 
     */
    virtual std::multimap<std::string, std::string> getDiscoveredServices(std::chrono::milliseconds timeout) const = 0;

public:
    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::createServer
     */
    fep3::Result createServer(const std::string& server_name,
        const std::string& server_url)
    {
        if (_locked)
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE,
                "service bus: Can not create server. Invalid state for creation of '%s' while creating '%s' - %s",
                _system_name.c_str(),
                server_name.c_str(),
                server_url.c_str());
        }
        try
        {
            _server.reset();
            auto server = createAServer(server_name, server_url);
            if (server)
            {
                _server = server;
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED,
                    "Could not create participant server %s with url %s",
                    server_name.c_str(), server_url.c_str());
            }
            return {};
        }
        catch (const std::exception& ex)
        {
            RETURN_ERROR_DESCRIPTION(ERR_BAD_DEVICE,
                "Could not create participant server %s with url %s - %s",
                server_name.c_str(), server_url.c_str(), ex.what() );
        }
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::releaseServer
     */
    void releaseServer() override
    {
        if (_locked)
        {
            return;
        }
        _server.reset();
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::getServer
     */
    std::shared_ptr<IServiceBus::IParticipantServer> getServer() const override
    {
        return _server;
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::getRequester
     */
    std::shared_ptr<IServiceBus::IParticipantRequester> getRequester(const std::string& far_participant_name) const override
    {
        std::string found_url;
        if (_server && far_participant_name == _server->getName())
        {
            //stay local!
            //at least this server is in the system
            found_url = _server->getUrl();
        }
        else
        {
            // look for the requester without active discovering
            auto found_services = discover(std::chrono::milliseconds(0));
            for (const auto& found_service : found_services)
            {
                if (found_service.first == far_participant_name)
                {
                    found_url = found_service.second;
                    break;
                }
            }
            //if it is still empty, discover it
            if (found_url.empty())
            {
                found_services = discover(FEP3_SERVICE_BUS_GET_REQUESTER_TIMEOUT);
                for (const auto& found_service : found_services)
                {
                    if (found_service.first == far_participant_name)
                    {
                        found_url = found_service.second;
                        break;
                    }
                }
            }
        }
        if (found_url.empty())
        {
            throw std::runtime_error("Could not create requester for " + far_participant_name);
        }
        return createARequester(far_participant_name, found_url);
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::discover
     */
    std::multimap<std::string, std::string> discover(std::chrono::milliseconds timeout) const override
    {
        return getDiscoveredServices(timeout);
    }

    /**
     * @copydoc fep3::arya::IServiceBus::ISystemAccess::getName
     */
    std::string getName() const override
    {
        return _system_name;
    }

    /**
     * @brief Get the Url of the system_access
     * 
     * @return std::string the url as string
     */
    std::string getUrl() const
    {
        return _system_url;
    }
public:
    /**
     * @brief lock creation of the server 
     *
     */
    void lock()
    {
        _locked = true;
    }

    /**
     * @brief unlock creation of the server
     *
     */
    void unlock()
    {
        _locked = false;
    }

protected:
    /**
     * @brief Get the object to retrieve the default url. 
     * 
     * @return std::shared_ptr<ISystemAccessBaseDefaultUrls> the default. see also CTOR.
     */
    std::shared_ptr<ISystemAccessBaseDefaultUrls> getDefaultUrls() const
    {
        return _access_default_urls;
    }
private:
    //the system name
    std::string _system_name;
    //the system url
    std::string _system_url;

    //the current server if created
    std::shared_ptr<IServiceBus::IParticipantServer> _server;
    //the default url object
    std::shared_ptr<ISystemAccessBaseDefaultUrls> _access_default_urls;
    //locked server creation
    std::atomic<bool> _locked;
};
}
using arya::SystemAccessBase;
}
}