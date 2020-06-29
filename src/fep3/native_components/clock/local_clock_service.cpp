/**
 * Implementation of the native clock service component
 *
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include "local_clock_service.h"

#include <algorithm>

#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>

#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/base/properties/property_type.h>
#include <fep3/fep3_errors.h>

#include "fep3/components/service_bus/rpc/fep_rpc_stubs_service.h"
#include "fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h"
#include "fep3/rpc_services/clock_sync/clock_sync_master_service_stub.h"
#include "local_system_clock.h"
#include "local_system_clock_discrete.h"

#include <a_util/strings.h>

namespace fep3
{
namespace native
{

class ClockEventSinkRegistry : public IClock::IEventSink
{
public:
    ClockEventSinkRegistry() = default;

    void setLogger(const std::shared_ptr<const ILoggingService::ILogger>& logger)
    {
        _logger = logger;
    }

    ~ClockEventSinkRegistry() override = default;

    void registerSink(const std::weak_ptr<IEventSink>& sink)
    {
        const auto sink_ptr = sink.lock();
        if (sink_ptr)
        {
            std::lock_guard<std::mutex> lock_guard(_mutex);

            for (auto& current_sink : _event_sinks)
            {
                auto current_sink_ptr = current_sink.lock();
                if (!current_sink_ptr || current_sink_ptr == sink_ptr)
                {
                    return;
                }
            }
            _event_sinks.push_back(sink);
        }
    }

    void unregisterSink(const std::weak_ptr<IEventSink>& sink)
    {
        auto sink_ptr = sink.lock();
        if (sink_ptr)
        {
            std::lock_guard<std::mutex> lock_guard(_mutex);

            const auto it = std::remove_if(_event_sinks.begin(), _event_sinks.end(),
                [&sink_ptr](const std::weak_ptr<IEventSink> event_sink)
            {
                const auto event_sink_ptr = event_sink.lock();
                if (event_sink_ptr)
                {
                    return sink_ptr == event_sink_ptr;
                }
                return false;
            });

            if (it != _event_sinks.end())
            {
                _event_sinks.erase(it);;
            }
        }
    }

private:
    void timeUpdateBegin(Timestamp old_time, Timestamp new_time) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        for (auto& sink : _event_sinks)
        {
            auto sink_ptr = sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeUpdateBegin(old_time, new_time);
            }
            else
            {
                logWarning("Expired event sink addressed during 'timeUpdateBegin' event. Unregistering it from Event sink registry.");
                unregisterSink(sink);
            }
        }
    }
    void timeUpdating(Timestamp new_time) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        for (auto& sink : _event_sinks)
        {
            auto sink_ptr = sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeUpdating(new_time);
            }
            else
            {
                logWarning("Expired event sink addressed during 'timeUpdating' event. Unregistering it from Event sink registry.");
                unregisterSink(sink);
            }
        }
    }
    void timeUpdateEnd(Timestamp new_time) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        for (auto& sink : _event_sinks)
        {
            auto sink_ptr = sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeUpdateEnd(new_time);
            }
            else
            {
                logWarning("Expired event sink addressed during 'timeUpdateEnd' event. Unregistering it from Event sink registry.");
                unregisterSink(sink);
            }
        }
    }
    void timeResetBegin(Timestamp old_time, Timestamp new_time) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        for (auto& sink : _event_sinks)
        {
            auto sink_ptr = sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeResetBegin(old_time, new_time);
            }
            else
            {
                logWarning("Expired event sink addressed during 'timeResetBegin' event. Unregistering it from Event sink registry.");
                unregisterSink(sink);
            }
        }
    }
    void timeResetEnd(Timestamp new_time) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        for (auto& sink : _event_sinks)
        {
            auto sink_ptr = sink.lock();
            if (sink_ptr)
            {
                sink_ptr->timeResetEnd(new_time);
            }
            else
            {
                logWarning("Expired event sink addressed during 'timeResetEnd' event. Unregistering it from Event sink registry.");
                unregisterSink(sink);
            }
        }
    };

private:
    fep3::Result logWarning(const std::string& message) const
    {
        if (_logger && _logger->isWarningEnabled())
        {
            return _logger->logWarning(message);
        }
        else
        {
            return {};
        }
    }

private:
    std::mutex                                                  _mutex;
    std::shared_ptr<const ILoggingService::ILogger>             _logger;
    std::vector<std::weak_ptr<IEventSink>>                      _event_sinks;
};

class RPCClockSyncMaster
    : public rpc::RPCService<rpc_stubs::RPCClockSyncMasterServiceStub, rpc::arya::IRPCClockSyncMasterDef>
{
public:
    explicit RPCClockSyncMaster(LocalClockService& service) : _service(service)
    {
    }

protected:
    int registerSyncSlave(int event_id_flag, const std::string& slave_name) override
    {
        if (fep3::isOk(_service.masterRegisterSlave(slave_name, event_id_flag)))
        {
            return 0;
        }
        return -1;
    }
    int unregisterSyncSlave(const std::string& slave_name) override
    {
        if (fep3::isOk(_service.masterUnregisterSlave(slave_name)))
        {
            return 0;
        }
        return -1;
    }
    int slaveSyncedEvent(const std::string& new_time, const std::string& slave_name) override
    {
        if (fep3::isOk(
            _service.masterSlaveSyncedEvent(slave_name, Timestamp{ a_util::strings::toInt64(new_time) })))
        {
            return 0;
        }
        return -1;
    }
    std::string getMasterTime() override
    {
        return a_util::strings::toString(_service.getTime().count());
    }
    int getMasterType() override
    {
        return static_cast<int>(_service.getType());
    }

private:
    LocalClockService& _service;
};
std::string RPCClockService::getClockNames()
{
    auto return_list = _service.getClockNames();
    auto first = true;
    std::string return_string;
    for (auto& clockname : return_list)
    {
        if (first)
        {
            return_string = clockname;
            first = false;
        }
        else
        {
            return_string += "," + clockname;
        }
    }
    return return_string;
}

std::string RPCClockService::getMainClockName()
{
    return _service.getMainClockName();
}

std::string RPCClockService::getTime(const std::string& clock_name)
{
    if (clock_name.empty())
    {
        return a_util::strings::toString(_service.getTime().count());
    }
    else
    {
        auto current_time = _service.getTime(clock_name);
        if (current_time.has_value())
        {
            return a_util::strings::toString(current_time.value().count());
        }
        else
        {
            return "-1";
        }
    }
}

int RPCClockService::getType(const std::string& clock_name)
{
    if (clock_name.empty())
    {
        return static_cast<int>(_service.getType());
    }
    else
    {
        auto clock_type = _service.getType(clock_name);
        if (clock_type.has_value())
        {
            return static_cast<int>(clock_type.value());
        }
        else
        {
            return -1;
        }
    }
}

ClockServiceConfiguration::ClockServiceConfiguration()
    : Configuration(FEP3_CLOCK_SERVICE_CONFIG)
{
}

fep3::Result ClockServiceConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_main_clock_name, FEP3_MAIN_CLOCK_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_time_update_timeout, FEP3_TIME_UPDATE_TIMEOUT_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_clock_sim_time_time_factor, FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY));
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(_clock_sim_time_cycle_time, FEP3_CLOCK_SIM_TIME_CYCLE_TIME_PROPERTY));

    return {};
}

fep3::Result ClockServiceConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_main_clock_name, FEP3_MAIN_CLOCK_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_time_update_timeout, FEP3_TIME_UPDATE_TIMEOUT_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_clock_sim_time_time_factor, FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY));
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(_clock_sim_time_cycle_time, FEP3_CLOCK_SIM_TIME_CYCLE_TIME_PROPERTY));

    return {};
}

fep3::Result ClockServiceConfiguration::validateSimClockConfiguration(const ILoggingService::ILogger& logger) const
{
    if (_clock_sim_time_cycle_time < FEP3_CLOCK_SIM_TIME_CYCLE_TIME_MIN_VALUE)
    {
        const auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                "Setting main clock cycle time of %d failed. Cycle time has to be > 0. Using default value.",
                static_cast<int32_t>(_clock_sim_time_cycle_time))
            .c_str());

        if (logger.isWarningEnabled())
        {
            FEP3_RETURN_IF_FAILED(logger.logWarning(result.getDescription()));
        }
        setPropertyValue(*getNode()->getChild(FEP3_CLOCK_SIM_TIME_CYCLE_TIME_PROPERTY), FEP3_CLOCK_SIM_TIME_CYCLE_TIME_DEFAULT_VALUE);
    }

    if (_clock_sim_time_time_factor < FEP3_CLOCK_SIM_TIME_TIME_FACTOR_MIN_VALUE
        && _clock_sim_time_time_factor != FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE)
    {
        const auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                "Setting main clock time factor of %f failed. Time factor has to be >= 0,1 or = 0. Using default value.",
                static_cast<double>(_clock_sim_time_time_factor))
            .c_str());

        if (logger.isWarningEnabled())
        {
            FEP3_RETURN_IF_FAILED(logger.logWarning(result.getDescription()));
        }
        setPropertyValue(*getNode()->getChild(FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY), FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE);
    }

    return {};
}

LocalClockService::LocalClockService()
    : _is_started(false)
    , _local_system_real_clock(std::make_shared<LocalSystemRealClock>())
    , _local_system_sim_clock(std::make_shared<LocalSystemSimClock>())
    , _current_clock(_local_system_real_clock)
    , _clock_event_sink_registry(std::make_shared<ClockEventSinkRegistry>())
{
}

fep3::Result LocalClockService::create()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get logging and configuration interface");
    }

    FEP3_RETURN_IF_FAILED(setupLogger(*components));
    FEP3_RETURN_IF_FAILED(registerDefaultClocks());

    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration service is not registered");
    }

    FEP3_RETURN_IF_FAILED(_configuration.initConfiguration(*configuration_service));

    const auto service_bus = components->getComponent<IServiceBus>();
    if (!service_bus)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }

    FEP3_RETURN_IF_FAILED(setupClockMaster(*service_bus));

    const auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }

    FEP3_RETURN_IF_FAILED(setupRPCClockSyncMaster(*rpc_server));
    FEP3_RETURN_IF_FAILED(setupRPCClockService(*rpc_server));

    return {};
}

fep3::Result LocalClockService::destroy()
{
    _logger.reset();
    _clock_event_sink_registry->setLogger(_logger);
    _clock_registry.setLogger(_logger);

    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get configuration interface");
    }

    auto res = unregisterServices(*components);

    _configuration.deinitConfiguration();

    return res;
}

fep3::Result LocalClockService::initialize()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    // make sure the local clock service is in a defined state
    deinitialize();
    
    return {};
}

fep3::Result LocalClockService::tense()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    _configuration.updatePropertyVariables();

    if (_configuration._main_clock_name != getMainClockName())
    {
        FEP3_RETURN_IF_FAILED(selectMainClock(_configuration._main_clock_name));
    }

    try
    {
        FEP3_RETURN_IF_FAILED(_clock_master->updateTimeout(std::chrono::milliseconds(_configuration._time_update_timeout)));
    }
    catch(const std::exception& exception)
    {
        logError(std::string() + exception.what());
        RETURN_ERROR_DESCRIPTION(ERR_EMPTY, exception.what());
    }

    if (FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME == static_cast<std::string>(_configuration._main_clock_name))
    {
        FEP3_RETURN_IF_FAILED(_configuration.validateSimClockConfiguration(*_logger));

        _local_system_sim_clock->updateConfiguration(
            Duration(std::chrono::milliseconds(_configuration._clock_sim_time_cycle_time)),
            _configuration._clock_sim_time_time_factor);
    }

    return {};
}

fep3::Result LocalClockService::start()
{
    std::shared_ptr<IClock> current_clock;
    {
        /// locking also current_clock->reset() would lead to a deadlock, because it will call getType()
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        current_clock = _current_clock;
    }
    
    current_clock->start(_clock_event_sink_registry);    
  
    _is_started = true;

    return {};
}

fep3::Result LocalClockService::stop()
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    _current_clock->stop();
    _is_started = false;
 
    return {};
}

std::string LocalClockService::getMainClockName() const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    if (_current_clock)
    {
        return _current_clock->getName();
    }
    else
    {
        return "";
    }
}

Timestamp LocalClockService::getTime() const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    if (!_is_started)
    {
        return Timestamp(0);
    }

    return _current_clock->getTime();
}

Optional<Timestamp> LocalClockService::getTime(const std::string& clock_name) const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    return getTimeUnlocked(clock_name);
}

Optional<Timestamp> LocalClockService::getTimeUnlocked(const std::string& clock_name) const
{
    const auto clock_found = _clock_registry.findClock(std::string(clock_name));
    if (clock_found)
    {
        return { clock_found->getTime() };
    }
    else
    {
        logWarning(a_util::strings::format(
            "Receiving clock time failed. A clock with the name %s is not registered.",
            clock_name.c_str()));

        return {};
    }
}

IClock::ClockType LocalClockService::getType() const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    return _current_clock->getType();
}

Optional<IClock::ClockType> LocalClockService::getType(const std::string& clock_name) const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

        return getTypeUnlocked(clock_name);
    }

Optional<IClock::ClockType> LocalClockService::getTypeUnlocked(const std::string& clock_name) const
{
    if (const auto clock_found = _clock_registry.findClock(std::string(clock_name)))
    {
        return clock_found->getType();
    }
    else
    {
        logWarning(a_util::strings::format(
            "Receiving clock type failed. A clock with the name %s is not registered.",
            clock_name.c_str())
            );

        return {};
    }
}

fep3::Result LocalClockService::selectMainClock(const std::string& clock_name)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    if (_is_started)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
            a_util::strings::format(
                "Setting main clock %s failed. Can not reset main clock after start of clock service.",
                clock_name.c_str())
            .c_str());

        result |= logError(result);

        return result;
    }

    {
        _current_clock = _clock_registry.findClock(clock_name);
        if (!_current_clock)
        {
            _current_clock = _clock_registry.findClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME);

            auto result = CREATE_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                a_util::strings::format(
                    "Setting main clock failed. A clock with the name %s is not registered. Resetting to default.",
                    clock_name.c_str())
                .c_str());

            result |= logError(result);

            return result;
        }
        
        setPropertyValue(*_configuration.getNode()->getChild(FEP3_MAIN_CLOCK_PROPERTY), clock_name);
        
        return {};
    }
}

fep3::Result LocalClockService::registerClock(const std::shared_ptr<IClock>& clock)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    if (_is_started)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
            a_util::strings::format(
                "Registering clock %s failed. Can not register clock after start of clock service.",
                clock->getName().c_str())
            .c_str());

        result |= logError(result);

        return result;
    }
    return _clock_registry.registerClock(clock);
}

fep3::Result LocalClockService::unregisterClock(const std::string& clock_name)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    if (_is_started)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
            a_util::strings::format(
                "Unregistering clock %s failed. Can not unregister clock after start of clock service.",
                clock_name.c_str())
            .c_str());

        result |= logError(result);

        return result;
    }
    return _clock_registry.unregisterClock(clock_name);
}

std::list<std::string> LocalClockService::getClockNames() const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    return _clock_registry.getClockNames();
}

std::shared_ptr<IClock> LocalClockService::findClock(const std::string& clock_name) const
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    return _clock_registry.findClock(clock_name);
}

fep3::Result LocalClockService::registerEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink)
{   
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    if (!clock_event_sink.lock())
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_POINTER,
            a_util::strings::format(
                "Registering event sink failed. Event sink does not exist")
            .c_str());

        result |= logError(result);
        return result;
    }
    _clock_event_sink_registry->registerSink(clock_event_sink);

    return {};
}

fep3::Result LocalClockService::unregisterEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink)
{
    std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

    if (!clock_event_sink.lock())
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_POINTER,
            a_util::strings::format(
                "Unregistering event sink failed. Event sink does not exist")
            .c_str());

        result |= logError(result);
        return result;
    }
    _clock_event_sink_registry->unregisterSink(clock_event_sink);

    return {};
}

fep3::Result LocalClockService::setupLogger(const IComponents& components)
{
    auto logging_service = components.getComponent<arya::ILoggingService>();
    if (!logging_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Logging service is not registered");
    }

    _logger = logging_service->createLogger("clock_service.component");
    _clock_registry.setLogger(_logger);
    _clock_event_sink_registry->setLogger(_logger);

    return {};
}

fep3::Result LocalClockService::unregisterServices(const IComponents& components)
{
    const auto service_bus = components.getComponent<IServiceBus>();
    if (!service_bus)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Service bus is not available");
    }
    auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "RPC server is not available");
    }
    rpc_server->unregisterService(rpc::IRPCClockSyncMasterDef::getRPCDefaultName());
    rpc_server->unregisterService(rpc::IRPCClockServiceDef::getRPCDefaultName());

    return {};
}

fep3::Result LocalClockService::registerDefaultClocks()
{
    FEP3_RETURN_IF_FAILED(_clock_registry.registerClock(_local_system_real_clock));
    FEP3_RETURN_IF_FAILED(_clock_registry.registerClock(_local_system_sim_clock));

    return{};
}

fep3::Result LocalClockService::setupClockMaster(const IServiceBus& service_bus)
{
    const auto get_rpc_requester_by_name = [&service_bus](const std::string& service_participant_name) {
        return service_bus.getRequester(service_participant_name);
    };

    try
    {
        _clock_master = std::make_shared<rpc::ClockMaster>(
            _logger
            , std::chrono::milliseconds(_configuration._time_update_timeout)
            , _set_participant_to_error_state
            , get_rpc_requester_by_name);
    }
    catch (const std::runtime_error& ex)
    {
        logError(std::string() + ex.what());
        RETURN_ERROR_DESCRIPTION(ERR_EMPTY, ex.what());
    }

    _clock_event_sink_registry->registerSink(_clock_master);

    return {};
}

fep3::Result LocalClockService::setupRPCClockSyncMaster(IServiceBus::IParticipantServer& rpc_server)
{
    if (_rpc_impl_master == nullptr)
    {
        _rpc_impl_master = std::make_shared<RPCClockSyncMaster>(*this);
    }
    FEP3_RETURN_IF_FAILED(rpc_server.registerService(rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
        _rpc_impl_master));

    return {};
}

fep3::Result LocalClockService::setupRPCClockService(IServiceBus::IParticipantServer& rpc_server)
{
    if (_rpc_impl_service == nullptr)
    {
        _rpc_impl_service = std::make_shared<RPCClockService>(*this);
    }

    FEP3_RETURN_IF_FAILED(rpc_server.registerService(rpc::IRPCClockServiceDef::getRPCDefaultName(),
        _rpc_impl_service));

    return {};
}

fep3::Result LocalClockService::masterRegisterSlave(const std::string& slave_name,
    const int event_id_flag) const
{
    return _clock_master->registerSlave(slave_name, event_id_flag);
}

fep3::Result LocalClockService::masterUnregisterSlave(const std::string& slave_name) const
{
    return _clock_master->unregisterSlave(slave_name);
}

fep3::Result LocalClockService::masterSlaveSyncedEvent(const std::string& slave_name,
    const Timestamp time) const
{
    return _clock_master->receiveSlaveSyncedEvent(slave_name, time);
}


fep3::Result LocalClockService::logError(const fep3::Result& error) const
{
    if (_logger && _logger->isErrorEnabled())
    {
        return _logger->logError(error.getDescription());
    }

    return {};
}

fep3::Result LocalClockService::logError(const std::string& message) const
{
    if (_logger && _logger->isErrorEnabled())
    {
        return _logger->logError(message);
    }

    return {};

}


fep3::Result LocalClockService::logWarning(const fep3::Result& error) const
{
    if (_logger && _logger->isWarningEnabled())
    {
        return _logger->logWarning(error.getDescription());
    }

    return {};
}

fep3::Result LocalClockService::logWarning(const std::string& message) const
{
    if (_logger && _logger->isWarningEnabled())
    {
        return _logger->logWarning(message);
    }

    return {};
}

} // namespace native
} // namespace fep3
