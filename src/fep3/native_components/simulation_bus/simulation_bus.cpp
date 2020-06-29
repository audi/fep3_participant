/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include "simulation_bus.h"

#include <a_util/result.h>

#include "fep3/base/streamtype/default_streamtype.h"
#include "simbus_datareader.h"
#include "simbus_datawriter.h"

#include <algorithm>
#include <vector>
#include <set>
#include <unordered_map>

#include <mutex>
#include <future>

namespace fep3
{
namespace native
{

class SimulationBus::Impl
{
    std::vector<StreamMetaType> _supported_meta_types;
    std::set<std::string> _registered_readers;
    std::set<std::string> _registered_writers;

    using Transmitters = std::unordered_map<std::string, std::shared_ptr<Transmitter>>;
    static Transmitters& getTransmitters()
    {
        static Transmitters transmitters;
        return transmitters;
    }

public:

    Impl()
    {
        using namespace fep3::arya;
        _supported_meta_types.emplace_back(meta_type_plain);
        _supported_meta_types.emplace_back(meta_type_plain_array);
        _supported_meta_types.emplace_back(meta_type_string);
        _supported_meta_types.emplace_back(meta_type_video);
        _supported_meta_types.emplace_back(meta_type_audio);
        _supported_meta_types.emplace_back(meta_type_raw);
        _supported_meta_types.emplace_back(meta_type_ddl);
    }
    ~Impl()
    {
    }

    bool isSupported(const IStreamType& stream_type) const
    {
        auto result = std::find(_supported_meta_types.begin(), _supported_meta_types.end(),
                stream_type);

        return result != _supported_meta_types.end();
    }

    std::unique_ptr<IDataReader> getReader(const std::string& name, const IStreamType&,
            size_t queue_capacity)
    {
        return getReader(name, queue_capacity);
    }

    std::unique_ptr<IDataReader> getReader(const std::string& name, size_t queue_capacity)
    {
        if (registerAndCheckIfExists(_registered_readers, name))
        {
            return nullptr;
        }

        auto receive_queue = std::make_shared<DataItemQueue<>>(queue_capacity);

        getTransmitters()[name]->add(name, receive_queue);

        auto reader = std::make_unique<DataReader>(receive_queue);
        return reader;
    }

    std::unique_ptr<IDataWriter> getWriter(const std::string& name, size_t queue_capacity)
    {
        if (registerAndCheckIfExists(_registered_writers, name))
        {
            return nullptr;
        }

        auto writer = std::make_unique<DataWriter>(name, queue_capacity, getTransmitters()[name]);
        return writer;
    }

private:
    bool registerAndCheckIfExists(std::set<std::string>& registry, const std::string& name)
    {
        if (registry.find(name) != registry.end())
        {
            return true;
        }

        if (getTransmitters().find(name) == getTransmitters().end())
        {
            getTransmitters()[name] = std::make_shared<Transmitter>();
        }

        registry.emplace(name);

        return false;
    }
};

SimulationBus::SimulationBus() :
        _impl(std::make_unique<SimulationBus::Impl>())
{

}

SimulationBus::~SimulationBus()
{
}

bool SimulationBus::isSupported(const IStreamType& stream_type) const
{
    return _impl->isSupported(stream_type);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader(const std::string& name,
        const IStreamType& stream_type)
{
    return _impl->getReader(name, stream_type, 1);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader(const std::string& name,
        const IStreamType& stream_type, size_t queue_capacity)
{
    return _impl->getReader(name, stream_type, queue_capacity);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader(const std::string& name)
{
    return _impl->getReader(name, 1);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader(const std::string& name,
        size_t queue_capacity)
{
    return _impl->getReader(name, queue_capacity);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name,
        const IStreamType&)
{
    return _impl->getWriter(name, 1);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name,
        const IStreamType&,
        size_t queue_capacity)
{
    return _impl->getWriter(name, queue_capacity);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name)
{
    return _impl->getWriter(name, 1);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name,
        size_t queue_capacity)
{
    return _impl->getWriter(name, queue_capacity);
}

} // namespace native
} // namespace fep3
