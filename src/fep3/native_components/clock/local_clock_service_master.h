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

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <condition_variable>
#include <future>
#include <queue>

#include <fep3/rpc_services/clock_sync/clock_sync_slave_client_stub.h>
#include <fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include "fep3/components/clock/clock_service_intf.h"
#include <fep3/components/logging/logging_service_intf.h>

#include "fep3/components/service_bus/service_bus_intf.h"

namespace fep3
{
namespace rpc
{

class AsyncExecutor
{
public:
    AsyncExecutor();
    ~AsyncExecutor()
    {
        _stop = true;
        _condition_sync_start.notify_one();
        if (_sync_thread.joinable())
        {
            _sync_thread.join();
        }
    }
    AsyncExecutor(AsyncExecutor&) = delete;
    AsyncExecutor(AsyncExecutor&&) = delete;
    AsyncExecutor& operator=(AsyncExecutor&) = delete;
    AsyncExecutor& operator=(AsyncExecutor&&) = delete;

    template<typename F, typename R = std::result_of_t<F && ()>>
    std::future<void> enqueueTask(F&& f)
    {
        auto task = std::packaged_task<R()>(std::forward<F>(f));
        auto future = task.get_future();

        {
            std::unique_lock<std::mutex> lock(_tasks_mutex);
            _tasks.push(std::move(task));
        }
        _condition_sync_start.notify_one();

        return future;
    }    

private:
    void executionLoop();   

private:
    std::thread _sync_thread{};
    std::condition_variable _condition_sync_start{};
    std::mutex _tasks_mutex{};
    std::queue<std::packaged_task<void()>> _tasks;
    std::atomic_bool _stop{ false };
};

class ClockSlave
    : public RPCServiceClient<rpc_stubs::RPCClockSyncSlaveClientStub, IRPCClockSyncSlaveDef>
{
public:
    ClockSlave(const std::string& name, const std::shared_ptr<IRPCRequester>& rpc_requester, int event_id_flag);
    ~ClockSlave() = default;

    void activate();
    void deactivate();
    bool isActive();

    bool isSet(IRPCClockSyncMasterDef::EventIDFlag flag);
    void setEventIDFlag(int event_id_flag);   
    std::string getName();

private:
    bool _active;
    int _event_id_flag;
    std::string _name;
    std::mutex _mutex;
};

class ClockMaster : public IClock::IEventSink
{
public:
    ClockMaster(const std::shared_ptr<const ILoggingService::ILogger>& logger
    , std::chrono::nanoseconds rpc_timeout
    , const std::function<Result()>& set_participant_to_error_state
    , std::function<const std::shared_ptr<IRPCRequester>(const
        std::string& service_participant_name)> get_rpc_requester_by_name);

    virtual ~ClockMaster();

public:
    Result registerSlave(const std::string& slave_name, int event_id_flag);
    Result unregisterSlave(const std::string& slave_name);
    Result receiveSlaveSyncedEvent(const std::string& slave_name, Timestamp time);
    Result updateTimeout(std::chrono::nanoseconds rpc_timeout);

public:
    void timeUpdateBegin(Timestamp old_time, Timestamp new_time) override;
    void timeUpdating(Timestamp new_time) override;
    void timeUpdateEnd(Timestamp new_time) override;
    void timeResetBegin(Timestamp old_time, Timestamp new_time) override;
    void timeResetEnd(Timestamp new_time) override;

    class SlaveEntry
    {
    public:
        explicit SlaveEntry(std::shared_ptr<ClockSlave> slave)
            : _slave(std::move(slave))
        {
        }

        ~SlaveEntry() = default;            
         
        SlaveEntry(SlaveEntry&) = delete;
        SlaveEntry(SlaveEntry&&) = delete;
        SlaveEntry& operator=(SlaveEntry&) = delete;
        SlaveEntry& operator=(SlaveEntry&&) = delete;     

    public:
        std::shared_ptr<ClockSlave> _slave;      
        AsyncExecutor _async_executor;
    };

    class MultipleSlavesSynchronizer
    {
    public:
        MultipleSlavesSynchronizer(std::chrono::nanoseconds timeout
            , const std::shared_ptr<const ILoggingService::ILogger>& logger);

        void synchronize(const std::map<std::string, std::unique_ptr<ClockMaster::SlaveEntry>>& slaves
            , std::function<void(ClockSlave&)> sync_func
            , IRPCClockSyncMasterDef::EventIDFlag event_id_flag) const;

        MultipleSlavesSynchronizer(MultipleSlavesSynchronizer&) = delete;
        MultipleSlavesSynchronizer(MultipleSlavesSynchronizer&&) = delete;
        MultipleSlavesSynchronizer& operator=(MultipleSlavesSynchronizer&) = delete;
        MultipleSlavesSynchronizer& operator=(MultipleSlavesSynchronizer&&) = delete;

        private:            
            void waitUntilSyncFinish(
                std::vector<std::pair<SlaveEntry&, std::future<void>>>& current_synchronizations) const;        
        public:
            std::chrono::nanoseconds _safety_timeout;
        
        private:
            std::shared_ptr<const ILoggingService::ILogger> _logger;
    };

private:    
    void createUpdateFunctions();
    void synchronizeEvent(const std::function<void(ClockSlave&)>& sync_func
        , const IRPCClockSyncMasterDef::EventIDFlag event_id_flag
        , const std::string& message) const;

private:
    std::shared_ptr<IServiceBus> _service_bus;
    std::shared_ptr<const ILoggingService::ILogger> _logger;
    std::map<std::string, std::unique_ptr<SlaveEntry>> _slaves;
    std::chrono::nanoseconds _rpc_timeout;
    MultipleSlavesSynchronizer _slaves_synchronizer;
    std::mutex _slaves_mutex;
    std::function<Result()> _set_participant_to_error_state;
    const std::function<const std::shared_ptr<IRPCRequester>(
        const std::string& service_participant_name)> _get_rpc_requester_by_name;

    std::function<void(ClockSlave&, Timestamp, Timestamp)> _func_time_update_begin;
    std::function<void(ClockSlave&, Timestamp)> _func_time_updating;
    std::function<void(ClockSlave&, Timestamp)> _func_time_update_end;
    std::function<void(ClockSlave&, Timestamp, Timestamp)> _func_time_reset_begin;    
};

} // namespace rpc
} // namespace fep3
