/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <cassert>

#include "logging_queue.h"

using namespace fep3;
using namespace fep3::native;

static const uint32_t internal_max_msg_size = 255;
static const uint64_t timer_intervall_us = 1000;

#pragma warning(disable:4355) // 'this' used in base member initializer list
LoggingQueue::LoggingQueue()
    : _log_timer(timer_intervall_us, &LoggingQueue::timerFunc, *this),
    _internal_msg(internal_max_msg_size, '\0'),
    _max_queue_slot_num(200),
    _next_slot(0),
    _queue_lvl(0)
{
    // these are for initialization only and should be statics....
    // (which they were before anyway.)
    _max_message_length = internal_max_msg_size;
                                             // message + log level
    _max_queue_mem_size = _max_queue_slot_num * (_max_message_length + sizeof(uint8_t));

    if (!_message_mem.allocate(_max_queue_mem_size))
    {
        assert(!"System out of memory!");
    }
    a_util::memory::set(_message_mem, 0, _message_mem.getSize());
    _log_timer.start();
}

LoggingQueue::~LoggingQueue()
{
    _log_timer.stop();
}

void LoggingQueue::timerFunc()
{
    collectAndExecute();
}

fep3::Result LoggingQueue::add(const std::function<void()>& fcn)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(_queue_guard);

    fep3::Result res = ERR_NOERROR;

    if (_queue_lvl == _max_queue_slot_num)
    {
        // theres not even place for the warning message.
        res = ERR_MEMORY;
    }

    if (isOk(res))
    {
        _function_queue.enqueue(std::move(fcn));

        if (++_next_slot == _max_queue_slot_num)
        {
            // Circular queue overrun.
            _next_slot = 0;
        }
        ++_queue_lvl;
    }

    return res;
}

fep3::Result LoggingQueue::collectAndExecute()
{
    std::function<void()> entry = NULL;
    if (_function_queue.tryDequeue(entry))
    {
        entry();

        --_queue_lvl;
        return ERR_NOERROR;
    }
    return ERR_EMPTY;
}
