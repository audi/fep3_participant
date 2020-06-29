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

#include <fep3/core/data/data_writer.h>

namespace fep3
{
namespace core
{
namespace arya
{

DataWriter::DataWriter() :
    _stream_type(fep3::arya::meta_type_raw),
    _queue_size(DATA_WRITER_QUEUE_SIZE_DYNAMIC)
{
}

DataWriter::DataWriter(std::string name, const StreamType & stream_type) :
    _name(std::move(name)),
    _stream_type(stream_type),
    _queue_size(DATA_WRITER_QUEUE_SIZE_DYNAMIC)
{
}

DataWriter::DataWriter(std::string name, const StreamType & stream_type, size_t queue_size) :
    _name(std::move(name)),
    _stream_type(stream_type),
    _queue_size(queue_size)
{
}

DataWriter::DataWriter(const DataWriter& other) :
    _name(other._name),
    _stream_type(other._stream_type),
    _connected_writer(),
    _queue_size(other._queue_size)
{
}

DataWriter& DataWriter::operator=(const DataWriter& other)
{
    _name = other._name;
    _stream_type = other._stream_type;
    _queue_size = other._queue_size;
    _connected_writer.reset();
    return *this;
}

fep3::Result DataWriter::addToDataRegistry(IDataRegistry& data_registry)
{
    if (DATA_WRITER_QUEUE_SIZE_DYNAMIC == _queue_size)
    {
        _connected_writer = addDataOut(data_registry, _name.c_str(), _stream_type);
    }
    else
    {
        _connected_writer = addDataOut(data_registry, _name.c_str(), _stream_type, _queue_size);
    }

    if (_connected_writer)
    {
        return {};
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_DEVICE_NOT_READY, "could not register data writer");
    }
}

fep3::Result DataWriter::addClock(IClockService& clock)
{
    _clock = &clock;
    return {};
}

fep3::Result DataWriter::removeFromDataRegistry()
{
    _connected_writer.reset();
    return {};
}

fep3::Result DataWriter::removeClock()
{
    _clock = nullptr;
    return {};
}

struct SampleWithAddedTimeAndCounter
    : public IDataSample
{
    const IDataSample&         _data_sample;
    const uint32_t             _counter;
    fep3::Optional<Timestamp>  _time;
    explicit SampleWithAddedTimeAndCounter(const fep3::IClockService* clock,
                                           const IDataSample& data_sample,
                                           const uint32_t counter) : _data_sample(data_sample),
                                                                     _counter(counter)
    {
        if (clock)
        {
            _time = clock->getTime();
        }
    }
    Timestamp getTime() const override
    {
        if (_time)
        {
            if (_data_sample.getTime().count() == 0)
            {
                return *_time;
            }
        }
        return _data_sample.getTime();

    }
    size_t getSize() const override
    {
        return _data_sample.getSize();
    }

    uint32_t getCounter() const override
    {
        return _counter;
    }

    size_t read(IRawMemory& writeable_memory) const override
    {
        return _data_sample.read(writeable_memory);
    }

    void setTime(const Timestamp&) override
    {
        //invalid call
    }
    void setCounter(uint32_t) override
    {
        //invalid call
    }
    
    size_t write(const IRawMemory&) override
    {
        //invalid call
        return 0;
    }
};

fep3::Result DataWriter::write(const IDataSample& data_sample)
{
    if (_connected_writer)
    {
        SampleWithAddedTimeAndCounter sample_wrap(_clock, data_sample, _counter++);
        return _connected_writer->write(sample_wrap);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_CONNECTED, "not connected");
    }
}

fep3::Result DataWriter::write(const IStreamType & stream_type)
{
    if (_connected_writer)
    {
        _stream_type = stream_type;
        return _connected_writer->write(_stream_type);
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_CONNECTED, "not connected");
    }
}

fep3::Result DataWriter::write(Timestamp time, const void * data, size_t data_size)
{
    DataSampleRawMemoryRef ref_sample(time, data, data_size);
    return write(ref_sample);
}

fep3::Result DataWriter::flushNow(Timestamp )
{
    return _connected_writer->flush();
}


std::string DataWriter::getName() const
{
    return _name;
}

fep3::Result addToComponents(DataWriter& writer, const IComponents& components)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry)
    {
        auto res = writer.addToDataRegistry(*data_registry);
        if (isOk(res))
        {
            auto clock = components.getComponent<IClockService>();
            if (clock)
            {
                return writer.addClock(*clock);
            }
            else
            {
                return {};
            }
        }
        else
        {
            return res;
        }
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "%s is not part of the given component registry", getComponentIID<IDataRegistry>().c_str());
    }
}

fep3::Result removeFromComponents(DataWriter& writer, const IComponents& )
{
    return writer.removeFromDataRegistry();
}

}
}
}


