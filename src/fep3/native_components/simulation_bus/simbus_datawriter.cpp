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

#include "simbus_datawriter.h"

#include "fep3/base/sample/data_sample.h"
#include "fep3/base/streamtype/streamtype.h"

namespace fep3
{
namespace native
{

template <class TYPE>
void SimulationBus::Transmitter::transmit(const std::string& name, const data_read_ptr<const TYPE>& sample)
{
    for (auto elem : _receiver_queues)
    {
        if (elem.first == name)
        {
            elem.second->push(sample);
        }
    }
}

void SimulationBus::Transmitter::add(const std::string& name, DataItemQueuePtr receive_queue)
{
    _receiver_queues.emplace(std::make_pair(name, receive_queue));
}

SimulationBus::DataWriter::DataWriter(const std::string& name, size_t transmit_buffer_capacity, const std::shared_ptr<SimulationBus::Transmitter>& transmitter)
{
    _name = name;
    _transmit_buffer = std::make_unique<DataItemQueue<>>(transmit_buffer_capacity);
    _transmitter = transmitter;
}

fep3::Result SimulationBus::DataWriter::write(const IDataSample& data_sample)
{
    auto current = std::make_shared<DataSample>(data_sample);

    _transmit_buffer->push(current);

    return {};
}

fep3::Result SimulationBus::DataWriter::write(const IStreamType& stream_type)
{

    auto current = std::make_shared<StreamType>(stream_type);

    _transmit_buffer->push(current);

    return {};
}

fep3::Result SimulationBus::DataWriter::transmit()
{
    for (auto items = _transmit_buffer->pop(); std::get<0>(items) != nullptr || std::get<1>(items) != nullptr; items = _transmit_buffer->pop())
    {
        if (std::get<0>(items) != nullptr)
        {
            _transmitter->transmit(_name, std::get<0>(items));
        }

        if (std::get<1>(items) != nullptr)
        {
            _transmitter->transmit(_name, std::get<1>(items));
        }
    }

    return {};
}


} // namespace native
} // namespace fep3
