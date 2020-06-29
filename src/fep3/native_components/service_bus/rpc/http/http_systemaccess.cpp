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
#include "http_systemaccess.h"
#include "http_server.h"
#include "http_client.h"
#include <../3rdparty/lssdp-cpp/src/url/cxx_url.h>

#include <../3rdparty/lssdp-cpp/src/lssdpcpp/lssdpcpp.h>
#include <mutex>

#include <a_util/strings.h>
#include <a_util/concurrency.h>

#include "../../service_bus_logger.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

namespace fep3
{
namespace native
{

struct ServiceVec
{
public:
    void update(const lssdp::ServiceFinder::ServiceUpdateEvent& update_event,
        const std::string& system_name)
    {
        //special system_name
        const bool search_all_systems = (system_name == fep3::IServiceBus::ISystemAccess::_discover_all_systems);
        //the unique service name will be server@system in FEP 3
        std::string received_service_name;
        std::string received_system_name;
        if (search_all_systems)
        {
            received_service_name = update_event._service_description.getUniqueServiceName();
        }
        else
        {
            auto service_at_system = a_util::strings::split(update_event._service_description.getUniqueServiceName(), "@", true);
            if (service_at_system.size() >= 2)
            {
                received_service_name = service_at_system[0];
                received_system_name = service_at_system[1];
            }
        }

        //we only have a look on the received_system_name if is equal to the system_name
        if (system_name == received_system_name 
            || search_all_systems)
        {
            if (update_event._event_id == update_event.notify_alive
                || update_event._event_id == update_event.response)
            {
                _services[received_service_name]
                    = { system_clock::now(), update_event._service_description };
            }
            else if (update_event._event_id == update_event.notify_byebye)
            {
                _services.erase(received_service_name);
            }
        }
        else
        {
            //this is a server belongs to another system 
        }
    }
    void removeOldDevices()
    {
        decltype(_services)::const_iterator current = _services.cbegin();
        while (current != _services.cend())
        {
            if ((system_clock::now() - current->second.first) > 20s)
            {
                _services.erase(current->first);
                current = _services.cbegin();
            }
            else
            {
                ++current;
            }
        }
    }

    std::multimap<std::string, std::string> getDiscoveredServices() const
    {
        std::multimap<std::string, std::string> result_map = {};
        for (const auto& current : _services)
        {
            result_map.emplace(current.first, current.second.second.getLocationURL());
        }
        return result_map;
    }

private:

    std::map<std::string,
        std::pair<system_clock::time_point,
        lssdp::ServiceDescription>> _services;
};

struct HttpSystemAccess::Impl
{
    Impl() = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    //TODO: Make it more robust and return exceptions while init
    //If exception raises while loop the loop will immediately stop
    Impl(const std::string& system_url,
        const std::string& system_name,
        std::chrono::seconds interval) : _system_name(system_name),
        _system_url(system_url),
        _interval(interval),
        _services()
    {
        startDiscovering();
    }
    ~Impl()
    {
        //we only wait if the service finder was created
        _wait_for_at_least_one_msearch_call.notify();
        if (_service_finder)
        {
            _stop_loop = true;
            if (_loop.joinable())
            {
                _loop.join();
            }
        }
    }

    std::multimap<std::string, std::string> getDiscoveredServices(std::chrono::milliseconds timeout)
    {
        if (timeout.count() != 0)
        {
            //wait that at least one msearch is done
            if (timeout.count() <= 0)
            {
                timeout = 100ms;
            }
            //we do not care if we have a timeout or not we return the discovered
            _wait_for_at_least_one_msearch_call.wait_for(timeout);
        }
        {
            std::unique_lock<std::recursive_mutex> lo(_my_mutex);
            return _services.getDiscoveredServices();
        }
    }

    void startDiscovering()
    {
        if (!_system_url.empty())
        {
            _service_finder =
                std::make_unique<lssdp::ServiceFinder>(_system_url,
                    FEP3_PARTICIPANT_LIBRARY_VERSION_ID,
                    FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
                    HttpServer::_discovery_search_target);
        
            _stop_loop = false;
            _loop = std::move(std::thread(
                [&]
            {
                try
                {
                    seconds send_msearch_interval = _interval;
                    auto last_time = std::chrono::system_clock::now();
                    // send search first

                    searchNow();
                    do
                    {
                        auto now = system_clock::now();
                        bool m_search_now = ((now - last_time) >= send_msearch_interval);
                        if (m_search_now)
                        {
                            last_time = now;
                            removeOldDevices();
                            searchNow();
                        }
                        
                        checkForServices(1s);
                        std::this_thread::sleep_for(1ms);
                        if (m_search_now)
                        {
                            _wait_for_at_least_one_msearch_call.notify();
                        }
                    } while (!_stop_loop);
                }
                catch (const std::exception& ex)
                {
                    service_bus_helper::Logger::get().internalLog(ex.what());
                }
            }));
        }
    }

private:
    void searchNow()
    {
        if (!_service_finder->sendMSearch())
        {
            service_bus_helper::Logger::get().internalLog(_service_finder->getLastSendErrors());
        }
    }
    void checkForServices(std::chrono::seconds how_long)
    {
        std::unique_lock<std::recursive_mutex> lo(_my_mutex);
        _service_finder->checkForServices(
            [this](const lssdp::ServiceFinder::ServiceUpdateEvent& update_event)
        {
            _services.update(update_event, _system_name);
        }, how_long);
    }
    void removeOldDevices()
    {
        std::unique_lock<std::recursive_mutex> lo(_my_mutex);
        _services.removeOldDevices();
    }

private:
    mutable std::unique_ptr<lssdp::ServiceFinder> _service_finder;
    std::atomic<bool> _stop_loop;
    ServiceVec _services;
    std::string _system_name;
    std::string _system_url;
    std::chrono::seconds _interval;
    std::thread         _loop;
    std::recursive_mutex _my_mutex;
    a_util::concurrency::semaphore  _wait_for_at_least_one_msearch_call;
};



HttpSystemAccess::HttpSystemAccess(const std::string& system_name,
    const std::string& system_url,
    const std::shared_ptr<ISystemAccessBaseDefaultUrls>& defaults) :
    SystemAccessBase(system_name, system_url, defaults),
    _impl(std::make_unique<Impl>(system_url, system_name, std::chrono::seconds(5)))
{
}

HttpSystemAccess::~HttpSystemAccess()
{
}

std::shared_ptr<IServiceBus::IParticipantServer> HttpSystemAccess::createAServer(
    const std::string& server_name,
    const std::string& server_url)
{
    std::shared_ptr<rpc::arya::IRPCServer> server_to_set;
    {
        std::string _used_server_url = server_url;
        try
        {
            if (server_url == IServiceBus::ISystemAccess::_use_default_url)
            {
                _used_server_url = SystemAccessBase::getDefaultUrls()->getDefaultServerUrl();
            }
            else
            {
                fep3::helper::Url url(server_url);
                if (url.scheme() != "http")
                {
                    throw std::runtime_error(
                        a_util::strings::format("service bus: can not create server '%s'. Server does only support http, but it is called with '%s'",
                            server_name.c_str(),
                            server_url.c_str()));
                }
                _used_server_url = server_url;
            }
        }
        catch (const fep3::helper::Url::parse_error& exc)
        {
            throw std::runtime_error(
                a_util::strings::format("service bus: can not create server '%s'. url '%s' is not well formed. %s",
                    server_name.c_str(),
                    server_url.c_str(),
                    exc.what()));
        }
        if (_used_server_url.empty())
        {
            throw std::runtime_error(
                a_util::strings::format("service bus: can not create server '%s' with url '%s' ",
                    server_name.c_str(),
                    server_url.c_str()));
        }
        auto server = std::make_shared<HttpServer>(server_name,
            _used_server_url,
            getName(),
            getUrl());
        //very important to call!!
        server->initialize();
        return server;
    }
}

std::shared_ptr<IServiceBus::IParticipantRequester> HttpSystemAccess::createARequester(
    const std::string& far_server_name,
    const std::string& far_server_url) const
{
    try
    {
        fep3::helper::Url url_check(far_server_url);
        auto scheme = url_check.scheme();
        if (scheme != "http")
        {
            throw std::runtime_error(
                a_util::strings::format("service bus: can not create server '%s'. Server does only support http, but it is called with '%s'",
                    far_server_name.c_str(),
                    far_server_url.c_str()));
        }
        else
        {
            std::string use_url = far_server_url;
            if (url_check.host() == "0.0.0.0")
            {
                use_url = "http://127.0.0.1:" + url_check.port();
            }
            return std::make_shared<HttpClientConnector>(use_url);
        }
    }
    catch (const std::exception& exc)
    {
        throw std::runtime_error(
            a_util::strings::format("service bus: can not create server '%s'. url '%s' is not well formed. %s",
                far_server_name.c_str(),
                far_server_url.c_str(),
                exc.what()));
    }
}

std::multimap<std::string, std::string> HttpSystemAccess::getDiscoveredServices(std::chrono::milliseconds millisec) const
{
    return _impl->getDiscoveredServices(millisec);
}

}
}