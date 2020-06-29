/**
 * Declaration of the native clock service component
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

#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <string>

#include <fep3/fep3_optional.h>
#include <fep3/components/configuration/propertynode.h>
#include <fep3/components/base/component_base.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include "fep3/components/service_bus/rpc/fep_rpc_stubs_service.h"
#include "fep3/rpc_services/clock/clock_service_stub.h"
#include "fep3/rpc_services/clock/clock_service_rpc_intf_def.h"
#include "local_clock_registry.h"
#include "local_system_clock.h"
#include "local_system_clock_discrete.h"
#include <fep3/native_components/clock/local_clock_service_master.h>

namespace fep3
{
namespace native
{

class ClockEventSinkRegistry;
class RPCClockSyncMaster;
class LocalClockService;

class RPCClockService : public rpc::RPCService<rpc_stubs::RPCClockServiceStub, rpc::IRPCClockServiceDef>
{
public:
    explicit RPCClockService(LocalClockService& service)
    : _service(service)
    {
    }

protected:
    std::string getClockNames() override;
    std::string getMainClockName() override;
    std::string getTime(const std::string& clock_name) override;
    int getType(const std::string& clock_name) override;

private:
    LocalClockService& _service;
};

/**
* @brief Configuration for the LocalClockService
*/
struct ClockServiceConfiguration : public Configuration
{
    ClockServiceConfiguration();
    ~ClockServiceConfiguration() = default;

    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;
    fep3::Result validateSimClockConfiguration(const ILoggingService::ILogger&) const;

    PropertyVariable<std::string>       _main_clock_name{ FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME };
    PropertyVariable<int32_t>           _time_update_timeout{ FEP3_TIME_UPDATE_TIMEOUT_DEFAULT_VALUE };
    PropertyVariable<double>            _clock_sim_time_time_factor{ FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE };
    PropertyVariable<int32_t>           _clock_sim_time_cycle_time{ FEP3_CLOCK_SIM_TIME_CYCLE_TIME_DEFAULT_VALUE };
};

/**
* @brief Native implementation of a clock service.
*/
class LocalClockService
    : public ComponentBase<IClockService>
{
public:
    LocalClockService();
    ~LocalClockService() = default;

public: // inherited via IClockService
    Timestamp getTime() const override;
    Optional<Timestamp> getTime(const std::string& clock_name) const override;
    IClock::ClockType getType() const override;
    Optional<IClock::ClockType> getType(const std::string& clock_name) const override;

    std::string getMainClockName() const override;

    fep3::Result registerEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink) override;
    fep3::Result unregisterEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink) override;

    /**
    * @brief Select the current main clock (this clock will provide it's time value to the @ref getTime method).
    * @remark The @p clock_name must be a registered clock
    *
    * @param clock_name The name of the clock
    * @return fep3::Result
    * @retval ERR_INVALID_STATE    Clock service is in state running in which selecting the main clock is not allowed.
    * @retval ERR_NOT_FOUND        No clock with name @p clock_name is registered.
    */
    virtual fep3::Result selectMainClock(const std::string& clock_name);

public: // inherited via IClockRegistry
    fep3::Result registerClock(const std::shared_ptr<IClock>& clock) override;
    fep3::Result unregisterClock(const std::string& clock_name) override;
    std::list<std::string> getClockNames() const override;
    std::shared_ptr<IClock> findClock(const std::string& clock_name) const override;

public: // inherited via ComponentBase
    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;
    fep3::Result tense() override;
    fep3::Result start() override;
    fep3::Result stop() override;  

public: // for Sync Master support
    fep3::Result masterRegisterSlave(const std::string& slave_name, int event_id_flag) const;
    fep3::Result masterUnregisterSlave(const std::string& slave_name) const;
    fep3::Result masterSlaveSyncedEvent(const std::string& slave_name, Timestamp time) const;

private:
    Optional<Timestamp> getTimeUnlocked(const std::string& clock_name) const;
    Optional<IClock::ClockType> getTypeUnlocked(const std::string& clock_name) const;

    fep3::Result logError(const fep3::Result& error) const;
    fep3::Result logError(const std::string& message) const;
    fep3::Result logWarning(const fep3::Result& error) const;
    fep3::Result logWarning(const std::string& message) const;

    fep3::Result setupLogger(const IComponents& components);
    fep3::Result unregisterServices(const IComponents& components);
    fep3::Result registerDefaultClocks();
    fep3::Result setupClockMaster(const IServiceBus& service_bus);
    fep3::Result setupRPCClockSyncMaster(IServiceBus::IParticipantServer& rpc_server);
    fep3::Result setupRPCClockService(IServiceBus::IParticipantServer& rpc_server);

private:
    mutable std::recursive_mutex                                _recursive_mutex;

    std::shared_ptr<const ILoggingService::ILogger>             _logger;
    LocalClockRegistry                                          _clock_registry;
    ClockServiceConfiguration                                   _configuration;

    bool                                                        _is_started;

    std::shared_ptr<LocalSystemRealClock>                       _local_system_real_clock;
    std::shared_ptr<LocalSystemSimClock>                        _local_system_sim_clock;
    std::shared_ptr<IClock>                                     _current_clock;

    std::shared_ptr<ClockEventSinkRegistry>                     _clock_event_sink_registry;

    std::shared_ptr<RPCClockSyncMaster>                         _rpc_impl_master{nullptr};
    std::shared_ptr<fep3::rpc::ClockMaster>                     _clock_master;
    std::function<fep3::Result()>                               _set_participant_to_error_state;
    std::shared_ptr<RPCClockService>                            _rpc_impl_service{ nullptr };
};

} // namespace native
} // namespace fep3
