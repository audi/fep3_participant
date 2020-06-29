/**

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
 */

#include <fep3/core/data/data_reader.h>

namespace fep3
{
namespace core
{
namespace arya
{

DataReader::DataReader() :
    _stream_type(fep3::arya::meta_type_raw),
    DataReaderBacklog(1, StreamType(fep3::arya::meta_type_raw))
{
}

DataReader::DataReader(std::string name, const StreamType & stream_type) :
    _name(std::move(name)),
    _stream_type(stream_type),
    DataReaderBacklog(1, stream_type)
{
}

DataReader::DataReader(std::string name, const StreamType & stream_type, size_t queue_size) :
    _name(std::move(name)),
    _stream_type(stream_type),
    DataReaderBacklog(queue_size, stream_type)
{
}

DataReader::DataReader(const DataReader & other) :
    _name(other._name),
    _stream_type(other._stream_type),
    DataReaderBacklog(other.capacity(), other._stream_type)
{
}

DataReader& DataReader::operator=(const DataReader& other)
{
    _name = other._name;
    _stream_type = other._stream_type;
    _connected_reader.reset();

    return *this;
}

fep3::Result DataReader::addToDataRegistry(IDataRegistry& data_registry)
{
    _connected_reader = addDataIn(data_registry, _name.c_str(), _stream_type, capacity());
    if (_connected_reader)
    {
        return {};
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY, "could not register Data Reader");
    }
}

fep3::Result DataReader::removeFromDataRegistry()
{
    _connected_reader.reset();
    return {};
}

void DataReader::receiveNow(Timestamp time_of_update)
{
    if (_connected_reader)
    {
        while (_connected_reader->getFrontTime()
            && *_connected_reader->getFrontTime() < time_of_update)
        {
            if (isFailed(_connected_reader->pop(*this)))
            {
                //something went wrong
                break;
            }
        }
    }
}


std::string DataReader::getName() const
{
    return _name;
}

fep3::Result addToDataRegistry(IDataRegistry& registry, DataReader& reader)
{
    return reader.addToDataRegistry(registry);
}

fep3::Result addToComponents(DataReader& reader, const IComponents& components)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry)
    {
        return addToDataRegistry(*data_registry, reader);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "%s is not part of the given component registry", getComponentIID<IDataRegistry>().c_str());
    }
}

fep3::Result removeFromComponents(DataReader& reader, const IComponents& components)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry)
    {
        return reader.removeFromDataRegistry();
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "%s is not part of the given component registry", getComponentIID<IDataRegistry>().c_str());
    }
}

}
}
}

