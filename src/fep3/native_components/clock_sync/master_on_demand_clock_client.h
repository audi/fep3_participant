/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
*/

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>

#include <fep3/components/clock/clock_base.h>
#include <fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h>
#include <fep3/rpc_services/clock_sync/clock_sync_slave_service_stub.h>
#include <fep3/rpc_services/clock_sync/clock_sync_master_client_stub.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include "interpolation_time.h"

namespace fep3
{
namespace rpc
{
namespace arya
{

class IRPCRequester;

class FarClockUpdater
    : public RPCService<fep3::rpc_stubs::RPCClockSyncSlaveServiceStub, IRPCClockSyncSlaveDef>
    , public std::enable_shared_from_this<FarClockUpdater>
{
protected:
    explicit FarClockUpdater(
        Duration on_demand_step_size,
        const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
        const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
        bool before_and_after_event,
        const std::shared_ptr<const ILoggingService::ILogger>& logger,
        const std::string& local_participant_name);

    ~FarClockUpdater();
public:
    void startRPC();
    void stopRPC();

protected:
    virtual void updateTime(Timestamp new_time, Duration round_trip_time) = 0;
    virtual Timestamp masterTimeEvent(
        IRPCClockSyncMasterDef::EventID event_id,
        Timestamp new_time,
        Timestamp old_time) = 0;

    void startWorking();
    bool stopWorkingIfStarted();

    bool isClientRegistered() const;
    void registerToRPC();
    void unregisterFromRPC() const;
   
    void registerToMaster();
    void unregisterFromMaster();

protected:
    std::mutex _update_mutex;
    std::mutex _thread_mutex;
    std::condition_variable _cycle_wait_condition;
    bool _before_and_after_event;

private:
    std::string syncTimeEvent(int event_id,
                              const std::string& new_time,
                              const std::string& old_time) override;
    void work();

private:
    RPCServiceClient<rpc_stubs::RPCClockSyncMasterClientStub, IRPCClockSyncMasterDef>
        _far_clock_master;

    std::thread _worker;
    std::atomic_bool _stop;
    std::atomic_bool _started;
    int _master_type;   

    Duration _on_demand_step_size;
    std::chrono::time_point<std::chrono::steady_clock> _next_request_gettime;
    std::shared_ptr<IServiceBus::IParticipantServer> _participant_server;    

    const std::shared_ptr<const ILoggingService::ILogger> _logger;
    std::string _local_participant_name;
};

class MasterOnDemandClockInterpolating : public FarClockUpdater, public base::ContinuousClock
{
public:
    explicit MasterOnDemandClockInterpolating(
        Duration on_demand_step_size,
        const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
        const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
        const std::shared_ptr<const ILoggingService::ILogger>& logger,
        std::unique_ptr<fep3::IInterpolationTime> interpolation_time,
        const std::string& local_participant_name);

    Timestamp getNewTime() const override;
    Timestamp resetTime() override;

    void start(const std::weak_ptr<IEventSink>& event_sink) override;
    void stop() override;

private:
    void updateTime(Timestamp new_time, Duration roundtrip_time) override;
    Timestamp masterTimeEvent(
        IRPCClockSyncMasterDef::EventID event_id,
        Timestamp new_time,
        Timestamp old_time) override;

private:
    mutable std::unique_ptr<fep3::IInterpolationTime> _current_interpolation_time;
};

class MasterOnDemandClockDiscrete : public FarClockUpdater, public base::DiscreteClock
{
public:
    explicit MasterOnDemandClockDiscrete(
        Duration on_demand_step_size,
        const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
        const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
        bool beforeAndAfterEvent,
        const std::shared_ptr<const ILoggingService::ILogger>& logger,
        const std::string& local_participant_name);

    void start(const std::weak_ptr<IEventSink>& event_sink) override;
    void stop() override;  
    void updateTime(Timestamp new_time, Duration roundtrip_time) override;
    Timestamp masterTimeEvent(
        IRPCClockSyncMasterDef::EventID event_id,
        Timestamp new_time,
        Timestamp old_time) override;

private:
    void resetOnEvent();
};

} // namespace arya
using arya::MasterOnDemandClockDiscrete;
using arya::MasterOnDemandClockInterpolating;
using arya::FarClockUpdater;
} // namespace rpc
} // namespace fep3