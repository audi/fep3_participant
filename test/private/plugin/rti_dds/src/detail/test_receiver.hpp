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

#include <fep3/base/sample/data_sample.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>            
#include <condition_variable>

#include <gtest/gtest.h>

using namespace fep3;

#define NO_CHECK -1

/**
* @brief Basic receiver collecting all stream_types and samples
*/
struct BlockingTestReceiver : public ISimulationBus::IDataReceiver
{
public:
    std::vector< data_read_ptr<const IStreamType> > _stream_types;
    std::vector< data_read_ptr<const IDataSample> > _samples;
    ISimulationBus::IDataReader & _reader;

    int32_t _wait_for_samples = 1;
    int32_t _wait_for_streamtypes = 0;

    std::unique_ptr<std::thread> _receiver_thread;
    std::condition_variable _receiver_thread_conditional_variable;
    std::atomic<uint32_t> _running = {true};
    std::atomic<bool> _notified = { false };

    BlockingTestReceiver(ISimulationBus::IDataReader & reader)
        : _reader(reader)
    {
        _receiver_thread = std::make_unique<std::thread>([&]()
        {
            while(_running)
            {
                try
                {
                    _reader.receive(*this);
                }
                catch (std::exception exception)
                {
                    GTEST_FAIL();
                }
                _notified = true;
                _receiver_thread_conditional_variable.notify_all();
            }
        });

        // Give _receiver_thread some time to create 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    ~BlockingTestReceiver()
    {
        stop();
    }

    void operator()(const data_read_ptr<const IStreamType>& stream_type) override
    {
        _stream_types.push_back(stream_type);
        checkWaitConditions();
    }
    void operator()(const data_read_ptr<const IDataSample>& sample) override
    {
        _samples.push_back(sample);
        checkWaitConditions();
    }

    void checkWaitConditions()
    {
        if (_wait_for_samples == NO_CHECK &&
            _wait_for_streamtypes == NO_CHECK)
        {
            if (_samples.size() > 0 ||
                _stream_types.size() > 0)
            {
                _reader.stop();
            }
        }
        else if ((_wait_for_samples <= NO_CHECK || _samples.size() == static_cast<uint32_t>(_wait_for_samples)) &&
            (_wait_for_streamtypes <= NO_CHECK || _stream_types.size() == static_cast<uint32_t>(_wait_for_streamtypes)))
        {
            _reader.stop();
        }
    }

    void stop()
    {
        _running = false;
        _reader.stop();
        if (_receiver_thread)
        {
            _receiver_thread->join();
            _receiver_thread.reset();
        }
    }

    void clear()
    {
        _samples.clear();
        _stream_types.clear();
    }

    void waitFor(int32_t wait_for_samples
        , int32_t wait_for_streamtypes
        , std::chrono::seconds timeout = std::chrono::seconds(5)
        , bool stop_receiving = true
        , bool expect_timeout = false
        )
    {
        _wait_for_samples = wait_for_samples;
        _wait_for_streamtypes = wait_for_streamtypes;

        std::mutex mutex;
        {
            std::unique_lock<std::mutex> lock(mutex);
            std::cv_status lock_release_reason = std::cv_status::no_timeout;
            while (!_notified && lock_release_reason != std::cv_status::timeout)
            {
                lock_release_reason = _receiver_thread_conditional_variable.wait_for(lock, timeout);
                if (!_notified && lock_release_reason != std::cv_status::timeout)
                {
                    std::cout << "spurious wakeup." << std::endl;
                }
            }
            EXPECT_EQ(expect_timeout, lock_release_reason == std::cv_status::timeout);
        }

        if(stop_receiving)
        {
            stop();
        }

        if(_wait_for_samples != -1)
        {
            EXPECT_EQ(_samples.size(), _wait_for_samples);
        }

        if(_wait_for_streamtypes != -1)
        {
            EXPECT_EQ(_stream_types.size(), _wait_for_streamtypes);
        }
    }
};

struct TestReceiver : public ISimulationBus::IDataReceiver
{
public:
    std::vector< data_read_ptr<const IStreamType> > _stream_types;
    std::vector< data_read_ptr<const IDataSample> > _samples;

    int32_t _wait_for_samples = 1;
    int32_t _wait_for_streamtypes = 0;

    TestReceiver()
    {
    }

    virtual void operator()(const data_read_ptr<const IStreamType>& stream_type) override
    {
        _stream_types.push_back(stream_type);
    }
    
    virtual void operator()(const data_read_ptr<const IDataSample>& sample) override
    {
        _samples.push_back(sample);
    }

    void clear()
    {
        _samples.clear();
        _stream_types.clear();
    }
};

class OrderTestReceiver : public TestReceiver
{
private:
    bool _last_stream_type = false;

public:
    virtual void operator()(const data_read_ptr<const IStreamType>& /*stream_type*/) override
    {
        EXPECT_FALSE(_last_stream_type);
        _last_stream_type = true;
    }
    virtual void operator()(const data_read_ptr<const IDataSample>& /*sample*/) override
    {
        EXPECT_TRUE(_last_stream_type);
        _last_stream_type = false;
    }
};

class CountSampleTestReceiver : public TestReceiver
{
private:
    uint32_t _count_samples = 0;
    uint32_t _count_stream_type = 0;

public:
    virtual void operator()(const data_read_ptr<const IStreamType>& /*stream_type*/) override
    {
        _count_stream_type++;
    }
    virtual void operator()(const data_read_ptr<const IDataSample>& /*sample*/) override
    {
        _count_samples++;
    }

    uint32_t getSampleCount()
    {
        auto count_samples = _count_samples;
        _count_samples = 0;
        return count_samples;
    }

    uint32_t getStreamTypeCount()
    {
        auto count_stream_type = _count_stream_type;
        _count_stream_type = 0;
        return count_stream_type;
    }
};
