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

#include "master_on_demand_clock_client.h"

#include <exception>
#include <string>

#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>

#include <fep3/native_components/clock_sync/clock_sync_service.h>

using namespace std::chrono;

namespace fep3
{
namespace rpc
{
namespace arya
{

int getEventIDFlags(const bool before_and_after_event)
{
    if (before_and_after_event)
    {
        return static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_before) |
               static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating) |
               static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_after) |
               static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset);
    }
    else
    {
        return static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating) |
               static_cast<int>(IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset);
    }
}

FarClockUpdater::FarClockUpdater(
    Duration on_demand_step_size,
    const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
    const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
    bool before_and_after_event,
    const std::shared_ptr<const ILoggingService::ILogger>& logger,
    const std::string& local_participant_name)
        : _before_and_after_event(before_and_after_event)
        , _far_clock_master(IRPCClockSyncMasterDef::getRPCDefaultName(), participant_requester)
        , _stop(true)
        , _started(false)
        , _on_demand_step_size(on_demand_step_size)
        , _next_request_gettime(std::chrono::time_point<std::chrono::steady_clock>{Timestamp{ 0 }})
        , _participant_server(participant_server)
        , _master_type(-1)
        , _logger(logger)
        , _local_participant_name(local_participant_name)
{
}

FarClockUpdater::~FarClockUpdater()
{
    stopWorkingIfStarted();
}

void FarClockUpdater::startRPC()
{
    registerToRPC();
    registerToMaster();

    if (_master_type != static_cast<int>(IClock::ClockType::discrete))
    {
        startWorking();
    }
}

void FarClockUpdater::stopRPC()
{
    stopWorkingIfStarted();
    unregisterFromMaster();
    unregisterFromRPC();
}

void FarClockUpdater::registerToRPC()
{
    _participant_server->registerService(IRPCClockSyncSlaveDef::getRPCDefaultName(), shared_from_this());
}

void FarClockUpdater::unregisterFromRPC() const
{
    _participant_server->unregisterService(IRPCClockSyncSlaveDef::getRPCDefaultName());
}

void FarClockUpdater::startWorking()
{
    stopWorkingIfStarted(); //i will start afterwards

    {
        std::lock_guard<std::mutex> guard(_thread_mutex);

        _stop = false;
        _started = true;
        _next_request_gettime = time_point<steady_clock>{ Timestamp{ 0 } };
        _worker = std::thread([this] { work();  });
    }
}

bool FarClockUpdater::stopWorkingIfStarted()
{
    std::lock_guard<std::mutex> guard(_thread_mutex);

    _stop = true;   
    if (_started)
    {       
        if (_worker.joinable())
        {
            _worker.join();
        }
       
        _started = false;
        return true;
    }
    else
    {
        return false;
    }
}

void FarClockUpdater::registerToMaster()
{
    try
    {
        _master_type = _far_clock_master.getMasterType();
    }
    catch (const std::exception& exception)
    {
       _logger->logWarning(exception.what());
    }

    try
    {
        _far_clock_master.registerSyncSlave(
            getEventIDFlags(_before_and_after_event),
            _local_participant_name);
    }
    catch (const std::exception& exception)
    {
        _master_type = -1;
        _logger->logWarning(exception.what());
    }
}

void FarClockUpdater::unregisterFromMaster()
{
    try
    {
        _far_clock_master.unregisterSyncSlave(_local_participant_name);
    }
    catch (const std::exception& exception)
    {
        _logger->logWarning(exception.what());
    }
}

std::string FarClockUpdater::syncTimeEvent(int event_id,
                                           const std::string& new_time,
                                           const std::string& old_time)
{  
    const auto time = masterTimeEvent(
        static_cast<rpc::IRPCClockSyncMasterDef::EventID>(event_id),
        Timestamp{ a_util::strings::toInt64(new_time) },
        Timestamp{ a_util::strings::toInt64(old_time) });

    return a_util::strings::toString(time.count());  
}

bool FarClockUpdater::isClientRegistered() const
{
    return _master_type != -1;
}

void FarClockUpdater::work()
{
    using namespace std::chrono;

    while (!_stop)
    {
        if (time_point<steady_clock>{Timestamp{ 0 }} == _next_request_gettime)
        {
            // go ahead
        }
        else
        {
            std::unique_lock<std::mutex> guard(_update_mutex);

            auto current_demand_time_diff = (
                _next_request_gettime - steady_clock::now());

            if (current_demand_time_diff > Timestamp{ 0 })
            {
                if (current_demand_time_diff > Timestamp{ 5 })
                  
                {
                    _cycle_wait_condition.wait_for(guard, current_demand_time_diff);
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        }

        try
        {
            if (!isClientRegistered())
            {
                registerToMaster();
            }

            if (_master_type == static_cast<int>(IClock::ClockType::continuous)) 
            /// we only auto sync if the timing master clock is of type continuous
            {               
                time_point<steady_clock> begin_request = steady_clock::now();
                std::string master_time = _far_clock_master.getMasterTime();
                const Timestamp current_time{ a_util::strings::toInt64(master_time) };
                {
                    std::lock_guard<std::mutex> locked(_update_mutex);
                    updateTime(current_time, steady_clock::now() - begin_request);
                }
            }
            else
            {
                // Unknown type
            }
            _next_request_gettime = steady_clock::now() + _on_demand_step_size;
        }
        catch (std::exception&)
        {
            if (!_stop)
            {
                registerToMaster();
            }
        }
    }
}

MasterOnDemandClockInterpolating::MasterOnDemandClockInterpolating(
    const Duration on_demand_step_size,
    const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
    const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
    const std::shared_ptr<const ILoggingService::ILogger>& logger,
    std::unique_ptr<fep3::IInterpolationTime> interpolation_time,
    const std::string& local_participant_name)
    : FarClockUpdater(on_demand_step_size,
                        participant_server,
                        participant_requester,
                        false,
                        logger,
                        local_participant_name)
    , ContinuousClock(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND)
    , _current_interpolation_time(std::move(interpolation_time))
{
}

Timestamp MasterOnDemandClockInterpolating::getNewTime() const
{
    return _current_interpolation_time->getTime();
}

Timestamp MasterOnDemandClockInterpolating::resetTime()
{
    const bool was_stopped = stopWorkingIfStarted();
    _current_interpolation_time->resetTime(Timestamp{ 0 });
    if (was_stopped)
    {
        startWorking();
    }
    
    return Timestamp{ 0 };
}

void MasterOnDemandClockInterpolating::updateTime(const Timestamp new_time, const Duration roundtrip_time)
{
    return _current_interpolation_time->setTime(new_time, roundtrip_time);
}

Timestamp MasterOnDemandClockInterpolating::masterTimeEvent(
    const rpc::IRPCClockSyncMasterDef::EventID event_id,
    Timestamp /**new_time*/,
    Timestamp /**old_time*/)
{
    if (event_id == IRPCClockSyncMasterDef::EventID::time_reset)
    {
        reset();
    }
    return getTime();
}

void MasterOnDemandClockInterpolating::start(const std::weak_ptr<IEventSink>& event_sink)
{   
    ContinuousClock::start(event_sink);   
}

void MasterOnDemandClockInterpolating::stop()
{  
    ContinuousClock::stop();   
}

MasterOnDemandClockDiscrete::MasterOnDemandClockDiscrete(
    const Duration on_demand_step_size,
    const std::shared_ptr<IServiceBus::IParticipantServer>& participant_server,
    const std::shared_ptr<IServiceBus::IParticipantRequester>& participant_requester,
    const bool beforeAndAfterEvent,
    const std::shared_ptr<const ILoggingService::ILogger>& logger,
    const std::string& local_participant_name)
        : FarClockUpdater(on_demand_step_size, participant_server, participant_requester, beforeAndAfterEvent, logger, local_participant_name)
        , DiscreteClock(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE)
{
}

void MasterOnDemandClockDiscrete::resetOnEvent()
{
    const auto was_stopped = stopWorkingIfStarted();  

    DiscreteClock::reset();
    if (was_stopped)
    {
        startWorking();
    }
}

void MasterOnDemandClockDiscrete::updateTime(const Timestamp new_time, Duration /**roundtrip_time*/)
{
    DiscreteClock::setNewTime(new_time, true);
}

void MasterOnDemandClockDiscrete::start(const std::weak_ptr<IEventSink>& event_sink)
{   
    DiscreteClock::start(event_sink);   
}

void MasterOnDemandClockDiscrete::stop()
{  
    DiscreteClock::stop();   
}

Timestamp MasterOnDemandClockDiscrete::masterTimeEvent(
    const IRPCClockSyncMasterDef::EventID event_id,
    const Timestamp new_time,
    const Timestamp old_time)
{
    if (event_id == IRPCClockSyncMasterDef::EventID::time_reset)
    {
        if (new_time != old_time)
        {
            resetOnEvent();
        }
    }
    else if (event_id == IRPCClockSyncMasterDef::EventID::time_update_before)
    {
        std::lock_guard<std::mutex> guard(_update_mutex);
        auto sink_ptr = _event_sink.lock();
        if (sink_ptr)
        {
            sink_ptr->timeUpdateBegin(old_time, new_time);
        }
    }
    else if (event_id == IRPCClockSyncMasterDef::EventID::time_updating)
    {
        DiscreteClock::setNewTime(new_time, _before_and_after_event);
    }
    else if (event_id == IRPCClockSyncMasterDef::EventID::time_update_after)
    {
        std::lock_guard<std::mutex> guard(_update_mutex);
        auto sink_ptr = _event_sink.lock();
        if (sink_ptr)
        {
            sink_ptr->timeUpdateEnd(new_time);
        }
    }
    return getTime();
}

} // namespace arya
} // namespace rpc
} // namespace fep3
