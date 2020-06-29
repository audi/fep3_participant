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
#include <mutex>
#include <future>
#include <vector>
#include <queue>

namespace fep3
{
namespace native
{

class SimulationBus::DataReader: public arya::ISimulationBus::IDataReader
{
public:
    DataReader(std::shared_ptr< DataItemQueue<> >& item_queue)
        : _item_queue{ item_queue }
    {
    };
    virtual ~DataReader() {}
    DataReader(const DataReader&) = delete;
    DataReader(DataReader&&) = delete;
    DataReader& operator=(const DataReader&) = delete;
    DataReader& operator=(DataReader&&) = delete;

    virtual size_t size() const override;

    virtual size_t capacity() const override;

    virtual bool pop(arya::ISimulationBus::IDataReceiver& onReceive) override;

    virtual void receive(arya::ISimulationBus::IDataReceiver& onReceive) override;

    virtual void stop() override;

    virtual Optional<Timestamp> getFrontTime() const override;

private:
    std::shared_ptr<DataItemQueue<>> _item_queue{ nullptr };


    struct ExitSignal
    {
        std::promise<void> trigger;
        mutable std::mutex data_triggered_reception_mutex;
    } _exitSignal;
};


} // namespace native
} // namespace fep3
