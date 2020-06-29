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

#include "local_clock_service_master.h"

#include <limits>

#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>
#include <a_util/result/error_def.h>

using namespace a_util::strings;
using namespace std::chrono;

namespace
{
    constexpr nanoseconds minimum_safety_timeout = nanoseconds(1000000000);
}

namespace fep3
{

namespace rpc
{

nanoseconds calculateSafetyTimeout(const nanoseconds rpc_timeout)
{   
    auto timeout = rpc_timeout * 2;
    if (timeout < minimum_safety_timeout)
    {
        timeout = minimum_safety_timeout;
    }

    return timeout;
}

fep3::Result validateTimeouts(const nanoseconds rpc_timeout, const nanoseconds safety_timeout)
{
    if (safety_timeout < rpc_timeout)
    {
        const auto message = format(
            "rpc timeout has to be smaller than synchronization safety timeout. rpc timeout is %d ms, safety timeout is %d ms",
            rpc_timeout,
            safety_timeout);

        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, message.c_str());
    }

    if (rpc_timeout.count() > std::numeric_limits<long long>::max())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, "rpc timeout exceeds max of long long");
    }

    return{};
}

ClockSlave::ClockSlave(const std::string& name, const std::shared_ptr<IRPCRequester>& rpc_requester, const int event_id_flag)
    : RPCServiceClient<RPCClockSyncSlaveClientStub, IRPCClockSyncSlaveDef>(IRPCClockSyncSlaveDef::getRPCDefaultName(), rpc_requester)
    , _active(false)
    , _event_id_flag(event_id_flag)
    , _name(name)
{
}

void ClockSlave::activate()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _active = true;
}

void ClockSlave::deactivate()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _active = false;
}

bool ClockSlave::isActive()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _active;
}

bool ClockSlave::isSet(IRPCClockSyncMasterDef::EventIDFlag flag)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return ((_event_id_flag & static_cast<int>(flag)) == static_cast<int>(flag));
}

void ClockSlave::setEventIDFlag(const int event_id_flag)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _event_id_flag = event_id_flag;
}

std::string ClockSlave::getName()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _name;
}

ClockMaster::ClockMaster(const std::shared_ptr<const ILoggingService::ILogger>& logger
    , nanoseconds rpc_timeout
    , const std::function<fep3::Result()>& set_participant_to_error_state
    , const std::function<const std::shared_ptr<IRPCRequester>(const
        std::string& service_participant_name)> get_rpc_requester_by_name)
    : _logger(logger)
    , _rpc_timeout(rpc_timeout)
    , _slaves_synchronizer(calculateSafetyTimeout(_rpc_timeout), logger)
    , _set_participant_to_error_state(set_participant_to_error_state)
    , _get_rpc_requester_by_name(get_rpc_requester_by_name)

{
    const auto result = validateTimeouts(_rpc_timeout, _slaves_synchronizer._safety_timeout);
    if (isFailed(result))
    {
        throw std::runtime_error(result.getDescription());
    }

    createUpdateFunctions();
}

void ClockMaster::createUpdateFunctions()
{
    _func_time_update_begin = [](ClockSlave& slave, const Timestamp new_time, const Timestamp old_time) ->void
    {
        toInt64(
            slave.syncTimeEvent(
                static_cast<int>(IRPCClockSyncMasterDef::EventID::time_update_before),
                toString(new_time.count()),
                toString(old_time.count())));
    };

    _func_time_updating = [](ClockSlave& slave, const Timestamp new_time) ->void
    {
        toInt64(
            slave.syncTimeEvent(
                static_cast<int>(IRPCClockSyncMasterDef::EventID::time_updating),
                toString(new_time.count()),
                toString(0)));
    };

    _func_time_update_end = [](ClockSlave& slave, const Timestamp new_time) ->void
    {
        toInt64(
            slave.syncTimeEvent(
                static_cast<int>(IRPCClockSyncMasterDef::EventID::time_update_after),
                toString(new_time.count()),
                toString(0)));
    };

    _func_time_reset_begin = [](ClockSlave& slave, const Timestamp new_time, const Timestamp old_time) ->void
    {
        toInt64(
            slave.syncTimeEvent(
                static_cast<int>(IRPCClockSyncMasterDef::EventID::time_reset),
                toString(new_time.count()),
                toString(old_time.count())));
    };
}

ClockMaster::~ClockMaster() = default;

fep3::Result ClockMaster::registerSlave(const std::string& slave_name, int event_id_flag)
{
    std::lock_guard<std::mutex> lock(_slaves_mutex);

    auto rpc_requester = _get_rpc_requester_by_name(slave_name);
    if (!rpc_requester)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Requester not found");
    }

    auto it = _slaves.find(slave_name);
    if (it != _slaves.end())
    {
        it->second->_slave->setEventIDFlag(event_id_flag);
        it->second->_slave->activate();
    }
    else
    {
        auto slave = std::make_unique<SlaveEntry>(
                    (std::make_shared<ClockSlave>(
                    slave_name, 
                    rpc_requester, 
                    event_id_flag)));

        _slaves[slave_name] = std::move(slave);
        _slaves[slave_name]->_slave->activate();
    }

    return{};
}

fep3::Result ClockMaster::unregisterSlave(const std::string& slave_name)
{
    std::lock_guard<std::mutex> lock(_slaves_mutex);

    auto it = _slaves.find(slave_name);
    if (it != _slaves.end())
    {
        it->second->_slave->deactivate();
        return{};
    }        

    RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, format("a slave with name '%s' was not found", slave_name.c_str()).c_str());    
}

fep3::Result ClockMaster::receiveSlaveSyncedEvent(const std::string& /*slave_name*/, Timestamp /*time*/)
{
    return {};
}

fep3::Result ClockMaster::updateTimeout(const nanoseconds rpc_timeout)
{    
    const auto safety_timeout = calculateSafetyTimeout(rpc_timeout);
    FEP3_RETURN_IF_FAILED(validateTimeouts(rpc_timeout, safety_timeout));

    _rpc_timeout = rpc_timeout;
    _slaves_synchronizer._safety_timeout = safety_timeout;
   
    createUpdateFunctions();

    return {};
}

void ClockMaster::timeUpdateBegin(Timestamp old_time, Timestamp new_time)
{
    std::lock_guard<std::mutex> lock(_slaves_mutex);

    auto func_wrapper = [&](ClockSlave& slave) {
        _func_time_update_begin(slave, new_time, old_time);
    };

    synchronizeEvent(func_wrapper
        , IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_before
        , format("an error occured during time_update_before at time %lld", new_time));
}

void ClockMaster::timeUpdating(Timestamp new_time)
{       
    std::lock_guard<std::mutex> lock(_slaves_mutex);

    auto func_wrapper = [&](ClockSlave& slave){
            _func_time_updating(slave, new_time);
    };    
        
    synchronizeEvent(func_wrapper
        , IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating
        , format("an error occured during time_updating at time %lld", new_time));
}

void ClockMaster::timeUpdateEnd(Timestamp new_time)
{
    std::lock_guard<std::mutex> lock(_slaves_mutex);

    auto func_wrapper = [&](ClockSlave& slave){
        _func_time_update_end(slave, new_time);
    };

    synchronizeEvent(func_wrapper
        , IRPCClockSyncMasterDef::EventIDFlag::register_for_time_update_after
        , format("an error occured during time_update_after at time %lld", new_time));
}

void ClockMaster::timeResetBegin(Timestamp old_time, Timestamp new_time)
{
    std::lock_guard<std::mutex> lock(_slaves_mutex);    

    auto func_wrapper = [&](ClockSlave& slave) {
        _func_time_reset_begin(slave, new_time, old_time);
    };

    synchronizeEvent(func_wrapper
        , IRPCClockSyncMasterDef::EventIDFlag::register_for_time_reset
        , format("an error occured during time_reset at old time %lld", old_time));
}

void ClockMaster::timeResetEnd(Timestamp /*new_time*/)
{
     //ignore
}

void ClockMaster::synchronizeEvent(const std::function<void(ClockSlave&)>& sync_func
    , const IRPCClockSyncMasterDef::EventIDFlag event_id_flag
    , const std::string& message) const
{
    try
    {
        _slaves_synchronizer.synchronize(_slaves, sync_func, event_id_flag);
    }
    catch (const std::exception& ex)
    {
        const auto actual_message = std::string(message + ": " + ex.what());
        _logger->logError(actual_message);

        _set_participant_to_error_state();
    }
}

AsyncExecutor::AsyncExecutor()
{
    _sync_thread = std::thread(&AsyncExecutor::executionLoop, this);
}


void AsyncExecutor::executionLoop()
{
    while (!_stop)
    {           
        {
            std::unique_lock<std::mutex> lock(_tasks_mutex);
            /// using while because of spurious wakeups
            if (_tasks.empty())
            {           
                _condition_sync_start.wait(lock);
            }
        }              

        /// we check here again because we can be woken up by destructor        
        if (!_stop)
        {                      
            std::packaged_task<void()> task;

            {
                std::unique_lock<std::mutex> lock(_tasks_mutex);
                if (!_tasks.empty())
                {                
                    task = std::move(_tasks.front());
                    _tasks.pop();                   
                }
            }            

            if (task.valid())
            {
                (task)();
            }                        
        }
    }
}

ClockMaster::MultipleSlavesSynchronizer::MultipleSlavesSynchronizer(
    nanoseconds timeout,
    const std::shared_ptr<const ILoggingService::ILogger>& logger)
    : _safety_timeout(timeout)
    , _logger(logger)
{
}

void ClockMaster::MultipleSlavesSynchronizer::synchronize(
    const std::map<std::string,
    std::unique_ptr<ClockMaster::SlaveEntry>>& slaves,
    std::function<void(ClockSlave&)> sync_func,
    const IRPCClockSyncMasterDef::EventIDFlag event_id_flag) const
{ 
    std::vector<std::pair<SlaveEntry&, std::future<void>>> synchronizations;

    for (const auto& it : slaves)
    {
        const auto& slave_entry = it.second;
        auto clock_slave = it.second->_slave;
        auto& slave_synchronizer = it.second->_async_executor;

        if (!clock_slave->isActive())
        {
            continue;
        }

        if (clock_slave->isSet(event_id_flag))
        {          
            auto sync_func_slave_binded = [clock_slave, sync_func]()
            {
                sync_func(*clock_slave);
            };

            synchronizations.emplace_back(
                *slave_entry, slave_synchronizer.enqueueTask(sync_func_slave_binded));            
        }
    }

    waitUntilSyncFinish(synchronizations);
}

void ClockMaster::MultipleSlavesSynchronizer::waitUntilSyncFinish(
    std::vector<std::pair<SlaveEntry&, std::future<void>>>& current_synchronizations) const
{
    const auto timeout_until = system_clock::now() + _safety_timeout;

    for (auto& synchronization : current_synchronizations)
    {
        const auto slave_name = synchronization.first._slave->getName();     

        const auto status = synchronization.second.wait_until(timeout_until);
        if (status == std::future_status::timeout)
        {
            const auto message = format(
                "a safety timeout (not rpc) occured while synchronizing the slave '%s'. "
                    "This points to an internal error."
                , slave_name.c_str());            

            _logger->logError(message);
        }
        else if (status == std::future_status::ready)
        {
            try
            {
                synchronization.second.get();
            }          
            catch (const jsonrpc::JsonRpcException& ex)
            {
                const auto message = format(
                    "an error occured during synchronization of slave '%s'. "
                        "Could be a timeout. Slave will be deactivated: %s"
                    , slave_name.c_str()
                    , ex.what());

                _logger->logError(message);

                synchronization.first._slave->deactivate();
            }         
        }
        else
        {
            const auto message = format(
                "synchronization thread for slave '%s' was deferred."
                , slave_name.c_str());
            throw std::runtime_error(message);            
        }
    }
}

} // namespace rpc
} // namespace fep3
