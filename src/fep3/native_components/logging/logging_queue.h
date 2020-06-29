/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <atomic>   //std::atomic<int32_t>
#include <string>

#include "a_util/concurrency.h"
#include "a_util/memory.h"
#include "a_util/system.h"
#include "fep3/base/queue/fep3_locked_queue.h"
#include "fep3/fep3_errors.h"

namespace fep3
{
namespace native
{
namespace arya
{
class LoggingQueue
{

public:
    /**
     * Default constructor
     * Using this to create the timer will assume a default cycle time of 1ms.
     */
    LoggingQueue();

    /// Default destructor
    ~LoggingQueue();

private: // override the worker function
    /**
     * Timer function which is being called periodically upon each timer expiration.
     */
    void timerFunc();

public:
    /**
     * adds a function that will be executed from the queue
     * @param [in] std::function the function itself
     * @retval ERR_NOERROR The message could be queued.
     * @retval ERR_MEMORY too many items in queue,
     * number of characters too high or the queue is congested.
     */
    fep3::Result add(const std::function<void()>& fcn);



private:
    /**
    * Method called by the TimerFunc to publish a single queued message.
    * @retval ERR_NOERROR Message has been logged successfully.
    * @retval ERR_EMPTY Queue is empty.
    */
    fep3::Result collectAndExecute();
    /// Maximum number of queue-slots
    int _max_queue_slot_num;
    /// Maximum number of characters per message.
    int _max_message_length;
    /// Total size of the memory allocated for the queue.
    size_t _max_queue_mem_size;

    /// The actual memory behind the queue.
    a_util::memory::MemoryBuffer _message_mem;
    /// The timer used to pipeline the physical logging
    a_util::system::Timer _log_timer;
    /// Mutex to guard the enqueues.
    a_util::concurrency::recursive_mutex _queue_guard;
    /// String holding internal messages
    std::string _internal_msg;

    /// Slot index to be used next
    /// (note that his queue internally behaves like a circular buffer).
    std::atomic<int32_t> _next_slot;
    /// Number of slots currently in use by the queue.
    std::atomic<int32_t> _queue_lvl;

    /// The actual lock free queue
    base::LockedQueue<std::function<void()>> _function_queue;
};
} // namespace arya
using arya::LoggingQueue;
} // namespace native
} // namespace fep3