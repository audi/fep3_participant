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

#include "simbus_datareader.h"

namespace {

template <typename tuple_type>
void dispatch (tuple_type& data, fep3::arya::ISimulationBus::IDataReceiver& onReceive)
{
    auto data_sample = std::get<0>(data);
    if (data_sample)
    {
        onReceive(data_sample);
    }

    auto stream_type = std::get<1>(data);
    if (stream_type)
    {
        onReceive(stream_type);
    }
}

} // namespace

namespace fep3
{
namespace native
{


size_t SimulationBus::DataReader::size() const
{
    std::unique_lock<std::mutex> queue_is_in_use(_exitSignal.data_triggered_reception_mutex, std::try_to_lock);

    if (!queue_is_in_use.owns_lock()) {
        // data triggered reception is currently running, so my queue is always empty
        return 0;
    }

    return _item_queue->size();
}

size_t SimulationBus::DataReader::capacity() const
{
    return _item_queue->capacity();
}

bool SimulationBus::DataReader::pop(ISimulationBus::IDataReceiver& onReceive)
{
    std::unique_lock<std::mutex> queue_is_in_use(_exitSignal.data_triggered_reception_mutex, std::try_to_lock);

    if (!queue_is_in_use.owns_lock()) {
        // data triggered reception is currently running, so my queue is always empty
        return false;
    }

    if (_item_queue->size() == 0) {
        return false;
    }

    auto res = _item_queue->pop();
    queue_is_in_use.unlock();

    dispatch<decltype(res)>(res, onReceive);

    return true;
}

void SimulationBus::DataReader::receive(ISimulationBus::IDataReceiver& onReceive)
{
    std::unique_lock<std::mutex> lock(_exitSignal.data_triggered_reception_mutex);

    std::future<void> futureObj = _exitSignal.trigger.get_future();
    while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
    {
        auto res = _item_queue->pop();
        dispatch<decltype(res)>(res, onReceive);
    }
}

void SimulationBus::DataReader::stop()
{
    _exitSignal.trigger.set_value();

    {
        std::unique_lock<std::mutex> lock(_exitSignal.data_triggered_reception_mutex);
        _exitSignal.trigger = std::promise<void> {};
    }
}

Optional<Timestamp> SimulationBus::DataReader::getFrontTime() const
{
    return _item_queue->getFrontTime();
}


} // namespace native
} // namespace fep3
