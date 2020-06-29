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
#pragma once

#include <a_util/memory.h>
#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_timestamp.h>
#include <fep3/fep3_optional.h>
#include "data_sample_intf.h"
#include "raw_memory.h"


namespace fep3
{
namespace arya
{

/**
 * Implementation helper class for a data sample used within ISimulationBus or IDataRegistry
 * 
 */
class DataSample : public IDataSample,
                   public IRawMemory
{
public:
    /**
     * @brief Construct a new data sample
     * 
     */
    DataSample() : _fixed_size(false), _time(0), _counter(0), _current_size(0)
    {}
    /**
     * @brief Construct a new data sample
     * 
     * @param pre_allocated_capacity will prealocate the raw memory data area of the data sample
     * @param fixed_size marks the sample to have no dynamic memory. if capacity size is reached no realocation is possible.
     */
    DataSample(size_t pre_allocated_capacity, bool fixed_size)
        : _fixed_size(fixed_size), _time(0), _counter(0), _current_size(0), _buffer(pre_allocated_capacity)
    {}
    /**
     * @brief Construct a new data sample
     * 
     * @param time preset timestamp
     * @param counter preset counter
     * @param from_memory the memory to copy the content from 
     */
    DataSample(timestamp_t time, uint32_t counter, const IRawMemory& from_memory)
        : _fixed_size(false), _time(time), _counter(counter)
    {
        write(from_memory);
    }

    /**
     * @brief copy construct a new data sample
     * 
     * @param other the sample to copy from
     */
    DataSample(const DataSample& other)
        : _fixed_size(false), _time(other.getTime()), _counter(other.getCounter())
    {
        other.read(*this);
    }
    /**
     * @brief copy construct a new data sample
     * 
     * @param other the sample interface to move from
     */
    DataSample(const IDataSample& other)
        : _fixed_size(false), _time(other.getTime()), _counter(other.getCounter())
    {
        other.read(*this);
    }
    /**
     * @brief move construct a new data sample
     *
     * @param other the sample to move from
     */
    DataSample(DataSample&& other)
        : _fixed_size(other._fixed_size)
        , _counter(other._counter)
        , _current_size(other._current_size)
        , _time(other._time)
        , _buffer(std::move(other._buffer))
    {}
    /**
     * @brief copy the content of the given data sample
     * 
     * @param other the sample to copy from
     * @return DataSample& the copied sample content
     */
    DataSample& operator=(const DataSample& other)
    {
        setTime(other.getTime());
        setCounter(other.getCounter());
        other.read(*this);
        return *this;
    }
    /**
     * @brief copy the content of the given data sample interface
     * 
     * @param other the sample content to copy from
     * @return DataSample& the copied sample content
     */
    DataSample& operator=(const IDataSample& other)
    {
        setTime(other.getTime());
        setCounter(other.getCounter());
        other.read(*this);
        return *this;
    }
     /**
     * @brief move the content of the given data sample
     * 
     * @param other the sample content to move from
     * @return DataSample& the copied sample content
     */
    DataSample& operator=(DataSample&& other)
    {
        std::swap(_fixed_size, other._fixed_size);
        std::swap(_time, other._time);
        std::swap(_counter, other._counter);
        std::swap(_current_size, other._current_size);
        std::swap(_buffer, other._buffer);
        return *this;
    }
    /**
     * @brief DTOR
     * 
     */
    virtual ~DataSample() = default;

    /**
     * @brief set the content of the sample
     * 
     * @param time timestamp
     * @param counter counter of the sample
     * @param from_memory the memory to copy from
     * @return size_t the size copied
     */
    size_t update(const Timestamp& time, uint32_t counter, const IRawMemory& from_memory)
    {
        setTime(time);
        setCounter(counter);
        return write(from_memory);
    }

public:
    size_t capacity() const override
    {
        return _buffer.getSize();
    }
    const void* cdata() const override
    {
        return _buffer.getPtr();
    }
    size_t size() const override
    {
        return _current_size;
    }

    size_t set(const void* data, size_t data_size) override
    {
        if (_fixed_size && capacity() < data_size)
        {
            a_util::memory::copy(_buffer, data, capacity());
            _current_size = capacity();
        }
        else
        {
            a_util::memory::copy(_buffer, data, data_size);
            _current_size = data_size;
        }
        return _current_size;
    }
    size_t resize(size_t data_size) override
    {
        if (_fixed_size && capacity() < data_size)
        {
            _current_size = capacity();
        }
        else
        {
            _current_size = data_size;
        }
        return _current_size;
    }

public:
    Timestamp getTime() const override
    {
        return _time;
    }
    size_t   getSize() const override
    {
        return _current_size;
    }
    uint32_t getCounter() const override
    {
        return _counter;
    }
    size_t read(IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(cdata(), size());
    }

    void setTime(const Timestamp& time) override
    {
        _time = time;
    }
    size_t write(const IRawMemory& from_memory) override
    {
        return set(from_memory.cdata(), from_memory.size());
    }
    void setCounter(uint32_t counter) override
    {
        _counter = counter;
    }

private:
    bool        _fixed_size;
    Timestamp   _time;
    uint32_t    _counter;
    size_t      _current_size;
    a_util::memory::MemoryBuffer _buffer;
};

/**
 * @brief Data sample helper class to wrap a raw memory pointer \p data and a size \p data_size in bytes.
 */
struct DataSampleRawMemoryRef : public IDataSample
{
    /**
     * CTOR
     * @param time the timestamp to preset
     * @param data the data pointer to reference to
     * @param data_size the size of the memory given in \p data
     */
    explicit DataSampleRawMemoryRef(Timestamp& time, const void* data, size_t data_size)
        : _time(time), _raw_memory_ref(data, data_size)
    {}
private:
    Timestamp& _time;
    RawMemoryRef _raw_memory_ref;

    Timestamp getTime() const override
    {
        return _time;
    }
    size_t getSize() const override
    {
        return _raw_memory_ref.size();
    }
    uint32_t getCounter() const override
    {
        return 0;
    }

    size_t read(IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(_raw_memory_ref.cdata(), _raw_memory_ref.size());
    }
    void setTime(const Timestamp& time) override
    {
        _time = time;
    }
    size_t write(const IRawMemory& /*from_memory*/) override
    {
        return 0;
    }
    void setCounter(uint32_t /*counter*/) override
    {
    }

};

/**
 * @brief Data sample helper template to wrap a non standard layout type T by default
 * @tparam T the non standard layout type
 * @tparam is_standard_layout_type the standard layout type check
 */
template<typename T, typename is_standard_layout_type = void>
class DataSampleType : public IDataSample,
    public RawMemoryClassType<T>
{
public:
    ///the value type is T
    typedef T                     value_type;
    ///the base type is the super class
    typedef RawMemoryClassType<T> base_type;

public:
    /**
     * Copy CTOR 
     * @param other the other to copy from
     */
    explicit DataSampleType(value_type& other) : base_type(other)
    {
    }
    /**
     * Copy operator
     * @param other the other to copy from
     * @return this sample type
     */
    DataSampleType& operator=(const IDataSample& other)
    {
        other.read(*this);
        return *this;
    }
public:
    Timestamp getTime() const
    {
        return {};
    }
    size_t getSize() const override
    {
        return base_type::size();
    }
    uint32_t getCounter() const override
    {
        return 0;
    }

    size_t read(IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(base_type::cdata(), base_type::size());
    }
    /// @cond no_documentation
    void setTime(const Timestamp& /*time*/) override
    {
    }
    /// @endcond no_documentation
    size_t write(const IRawMemory& from_memory) override
    {
        return base_type::set(from_memory.cdata(), from_memory.size());
    }
    void setCounter(uint32_t /*counter*/) override
    {
    }
};

/**
 * @brief Data sample helper template to wrap a non standard layout type T by default
 * @tparam T the non standard layout type
 */
template <typename T>
class DataSampleType<T, typename std::enable_if<std::is_standard_layout<T>::value>::type>
    : public IDataSample,
    public RawMemoryStandardType<T>
{
public:
    /// value type of DataSampleType
    typedef T                        value_type;
    /// super type of DataSampleType
    typedef RawMemoryStandardType<T> base_type;
public:
    /**
     * CTOR
     * @param value reference to the value type
     */
    explicit DataSampleType(value_type& value) : base_type(value)
    {
    }
    /**
     * copy operator
     * @param other reference to the value type to copy from
     * @return this data sample type object
     */
    DataSampleType& operator=(const IDataSample& other)
    {
        other.read(*this);
        return *this;
    }

public:
    Timestamp getTime() const override
    {
        return {};
    }
    size_t getSize() const override
    {
        return base_type::size();
    }
    uint32_t getCounter() const override
    {
        return 0;
    }
    size_t read(IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(base_type::cdata(), base_type::size());
    }
    /// @cond no_documentation 
    void setTime(const Timestamp & /*time*/) override
    {
    }
    /// @endcond no_documentation
    size_t write(const IRawMemory& from_memory) override
    {
        return base_type::set(from_memory.cdata(), from_memory.size());
    }
    void setCounter(uint32_t /*counter*/) override
    {
    }
};

/**
 * @brief Data sample helper template to wrap a std vector of type T
 */
template <typename T>
class StdVectorSampleType : public RawMemoryClassType<std::vector<T>>, public IDataSample
{
public:
    /// value type of DataSampleType
    typedef T                                 value_type;
    /// super type of DataSampleType
    typedef RawMemoryClassType<std::vector<T>> base_type;
public:
    /**
     * CTOR
     * @param array the data pointer to reference to
     */
    explicit StdVectorSampleType(std::vector<value_type>& array)
        : base_type(array)
    {
    };

    /**
     * copy assignment
     * @param other reference to the value type to copy from
     * @return this data sample type object
     */
    StdVectorSampleType& operator=(const IDataSample& other)
    {
        setTime(other.getTime());
        setCounter(other.getCounter());
        other.read(*this);
        return *this;
    }

    Timestamp getTime() const override
    {
        return _time;
    }

    size_t getSize() const override
    {
        return base_type::size();
    }

    uint32_t getCounter() const override
    {
        return _counter;
    }

    size_t read(IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(base_type::cdata(), base_type::size());
    }

    void setTime(const Timestamp& time ) override
    {
        _time = time;
    }

    size_t write(const IRawMemory& from_memory) override
    {
        base_type::set(from_memory.cdata(), from_memory.size());
        return getSize();
    }

    void setCounter(uint32_t counter) override
    {
        _counter = counter;
    }

private:
    Timestamp   _time;
    uint32_t    _counter;
};

}
using arya::DataSample;
using arya::DataSampleType;
using arya::DataSampleRawMemoryRef;
using arya::StdVectorSampleType;

}
