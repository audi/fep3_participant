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

#include "fep3/components/simulation_bus/simulation_bus_intf.h"
#include "data_item_queue.h"
#include "simulation_bus.h"

#include <memory>
#include <vector>
#include <unordered_map>

namespace fep3
{
namespace native
{

/**
 * Transmitter which supports SIMO (Single Input Multiple Output) broadcasting of samples to several queues
 */
class SimulationBus::Transmitter
{
public:
    using DataItemQueuePtr = std::shared_ptr<DataItemQueue<> >;

    template <class TYPE>
    void transmit(const std::string& name, const data_read_ptr<const TYPE>& sample);

    /**
     * Add a receiver queue to which samples will be added on transmit
     *
     * @param name signal name does not have to be unique
     * @param receive_queue Queue to push name samples
     */
    void add(const std::string& name, DataItemQueuePtr receive_queue);

private:
    std::unordered_multimap<std::string, DataItemQueuePtr> _receiver_queues;
};

class SimulationBus::DataWriter : public arya::ISimulationBus::IDataWriter
{
public:
    DataWriter(const std::string& name, size_t transmit_buffer_capacity, const std::shared_ptr<SimulationBus::Transmitter>& transmitter);

    virtual ~DataWriter() {}
    DataWriter(const DataWriter&) = delete;
    DataWriter(DataWriter&&) = delete;
    DataWriter& operator=(const DataWriter&) = delete;
    DataWriter& operator=(DataWriter&&) = delete;

    virtual fep3::Result write(const IDataSample& data_sample);
    virtual fep3::Result write(const IStreamType& stream_type);

    virtual fep3::Result transmit();

private:
    using DataItemQueuePtr = std::shared_ptr<DataItemQueue<>>;
    std::unique_ptr<DataItemQueue<>> _transmit_buffer { nullptr };

    std::string _name;
    std::shared_ptr<SimulationBus::Transmitter> _transmitter { nullptr };
};

} // namespace native
} // namespace fep3
