/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/base/sample/data_sample_intf.h>

namespace fep3
{
namespace mock
{

class DataSample
    : public IDataSample
{
public:
    ~DataSample() override = default;

    MOCK_CONST_METHOD0(getTime, Timestamp());
    MOCK_CONST_METHOD0(getSize, size_t());
    MOCK_CONST_METHOD0(getCounter, uint32_t());
    MOCK_CONST_METHOD1(read, size_t(IRawMemory&));
    MOCK_METHOD1(setTime, void(const Timestamp&));
    MOCK_METHOD1(setCounter, void(uint32_t));
    MOCK_METHOD1(write, size_t(const IRawMemory&));
};

// helper classes for tests
class FixedSizeRawMemory final : public ::fep3::IRawMemory
{
public:
    FixedSizeRawMemory(size_t data_size)
        : _value(data_size, 0)
    {}
    size_t capacity() const override
    {
        return _value.size();
    }
    const void* cdata() const override
    {
        return &_value;
    }
    size_t size() const override
    {
        return _value.size();
    }
    size_t set(const void* data, size_t new_data_size) override
    {
        if(new_data_size == _value.size())
        {
            //_value = *static_cast<const data_type*>(data);
            _value.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + new_data_size);
            return _value.size();
        }
        return 0;
    }
    size_t resize(size_t) override
    {
        // resize is not supported
        return _value.size();
    }
    std::vector<uint8_t> value() const
    {
        return _value;
    }
private:
    std::vector<uint8_t> _value;
};

bool areEqual(const ::fep3::IDataSample& lhs, const ::fep3::IDataSample& rhs)
{
    FixedSizeRawMemory data_sample_raw_memory(lhs.getSize());
    auto lhs_read_bytes = lhs.read(data_sample_raw_memory);
    FixedSizeRawMemory other_data_sample_raw_memory(rhs.getSize());
    auto rhs_read_bytes = rhs.read(other_data_sample_raw_memory);
    return
        lhs.getTime() == rhs.getTime()
        && lhs.getSize() == rhs.getSize()
        && lhs.getCounter() == rhs.getCounter()
        && lhs_read_bytes == rhs_read_bytes
        && data_sample_raw_memory.value() == other_data_sample_raw_memory.value()
        ;
}

// gmock matchers
MATCHER_P(DataSampleMatcher, other, "Equality matcher for IDataSample")
{
    return areEqual(arg, other);
}
MATCHER_P(DataSampleSmartPtrMatcher, other, "Equality matcher for smart pointer to IDataSample")
{
    const auto& arg_data_sample = *arg.get();
    const auto& other_data_sample = *other.get();
    return areEqual(arg_data_sample, other_data_sample);
}


} // namespace mock
} // namespace fep3