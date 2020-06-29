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

#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include <fep3/components/simulation_bus/mock/mock_simulation_bus.h>
#include <fep3/native_components/simulation_bus/simulation_bus.h>
#include <fep3/native_components/simulation_bus/simbus_datareader.h>
#include <fep3/base/streamtype/default_streamtype.h>
#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/sample/data_sample.h>

#include "helper/gmock_async_helper.h"

#include <thread>
#include <array>

using namespace fep3;

namespace {

class DataSampleNumber : public DataSample {
public:
    explicit DataSampleNumber(uint32_t order) {

        auto memory = fep3::RawMemoryStandardType<uint32_t>(order);

        this->write(memory);
    }
};

MATCHER_P(StreamTypeMatcher, other, "Equality matcher for IStreamType")
{
    return arg == other;
}

ACTION_P(Notify, notification)
{
    notification->notify();
}

struct FillTheReceiverQueueData {
    std::shared_ptr<fep3::native::DataItemQueue<>> item_queue;
    std::shared_ptr<fep3::mock::DataSample> sample;
};

ACTION_P(FillTheReceiverQueue, data)
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    data.item_queue->push(data.sample);
}

/**
 * This mock is able to use the fep3::mock::DataSampleMatcher
 */
class DataReceiver: public ISimulationBus::IDataReceiver
{
public:
    MOCK_METHOD1(onSampleReceived, void(const IDataSample& type));
    MOCK_METHOD1(onStreamTypeReceived, void(const IStreamType& type));

    void operator()(const data_read_ptr<const IStreamType>& type) override
    {
        onStreamTypeReceived(*type);
    }

    void operator()(const data_read_ptr<const IDataSample>& sample) override
    {
        onSampleReceived(*sample);
    }
};

} // namespace

/**
 * @detail Test the stopping of the DataReader
 * @req_id FEPSDK-SimulationBus
 * 
 */
TEST(NativeSimulationBus, testStoppingDataReader)
{

    using ::testing::_;
    using MatchIDataSample = testing::Matcher<const data_read_ptr<const IDataSample>&>;

    auto item_queue = std::make_shared<fep3::native::DataItemQueue<>>(5);
    auto sample = std::make_shared<fep3::mock::DataSample>();
    item_queue->push(sample);
    item_queue->push(sample);
    item_queue->push(sample);
    item_queue->push(sample);
    item_queue->push(sample);

    fep3::mock::DataReceiver receiver1;
    test::helper::Notification done1;
    {
        ::testing::InSequence s1;
        EXPECT_CALL(receiver1, call(MatchIDataSample(_)))
            .WillOnce(Notify(&done1));

        EXPECT_CALL(receiver1, call(MatchIDataSample(_)))
            //Thread safty of item_queue is required
            .WillRepeatedly(FillTheReceiverQueue(FillTheReceiverQueueData {item_queue, sample}));
    }

    fep3::native::SimulationBus::DataReader reader{ item_queue };

    std::thread t1(&fep3::native::SimulationBus::DataReader::receive, &reader, std::ref(receiver1));
    done1.waitForNotification();
    reader.stop();
    t1.join();

    // Second run of:
    // start thread
    // run receive
    // stop reveive
    fep3::mock::DataReceiver receiver2;
    test::helper::Notification done2;
    {
        ::testing::InSequence s1;
        EXPECT_CALL(receiver2, call(MatchIDataSample(_)))
            .WillOnce(Notify(&done2));

        EXPECT_CALL(receiver2, call(MatchIDataSample(_)))
            //Thread safty of item_queue is required
            .WillRepeatedly(FillTheReceiverQueue(FillTheReceiverQueueData {item_queue, sample}));
    }

    std::thread t2(&fep3::native::SimulationBus::DataReader::receive, &reader, std::ref(receiver2));
    done2.waitForNotification();
    reader.stop();
    t2.join();
}

/**
 * @detail Test transmission of arbitrary data
 * @req_id FEPSDK-SimulationBus
 *
 */
TEST(NativeSimulationBus, testTransmission)
{
    const data_read_ptr<const IDataSample> sample = std::make_shared<DataSample>(0, true);
    const StreamTypeDDL ddltype("my_ddl_uint8", "Z:/fileref.ddl");

    const std::string signal_1_name = "signal_1";
    const size_t queue_size = 5;

    auto sim_bus = std::make_shared<fep3::native::SimulationBus>();
    auto reader = sim_bus->getReader(signal_1_name, queue_size);
    auto writer = sim_bus->getWriter(signal_1_name, queue_size);

    writer->write(*sample);
    writer->write(ddltype);
    writer->write(*sample);

    writer->transmit();

    fep3::mock::DataReceiver receiver;
    using ::testing::_;
    using MatchIDataSample = testing::Matcher<const data_read_ptr<const IDataSample>&>;
    using MatchIStreamType = testing::Matcher<const data_read_ptr<const IStreamType>&>;

    EXPECT_CALL(receiver, call(MatchIDataSample(_)))
        .Times(2);
    EXPECT_CALL(receiver, call(MatchIStreamType(_)))
        .Times(1);

    while(reader->pop(receiver));
}

/**
* @detail Test overflow of reader queue. Test sample loss.
* @req_id FEPSDK-SimulationBus
*
*/
TEST(NativeSimulationBus, testTransmissionOfStreamType)
{
    const std::string signal_1_name = "signal_1";
    const size_t queue_size = 5;

    auto sim_bus = std::make_shared<fep3::native::SimulationBus>();
    auto reader = sim_bus->getReader(signal_1_name, queue_size);
    auto writer = sim_bus->getWriter(signal_1_name, queue_size);

    StreamTypeDDL ddltype("my_ddl_uint8", "Z:/fileref.ddl");
    writer->write(ddltype);

    writer->transmit();

    DataReceiver receiver;

    {
        ::testing::InSequence sequence;
        using M = ::testing::Matcher<const IStreamType&>;
        EXPECT_CALL(receiver,
            onStreamTypeReceived(M(StreamTypeMatcher(ddltype))))
            .Times(1);

        EXPECT_CALL(receiver, onStreamTypeReceived(::testing::_))
            .Times(0);
    }

    while(reader->pop(receiver));
}

namespace {
    class SimpleDataSample : public ::testing::Test
    {
    protected:
        SimpleDataSample()
        {
            _sim_bus = std::make_shared<fep3::native::SimulationBus>();

            _samples.reserve(10);

            const uint32_t magic_number = 455;

            for (uint32_t i = magic_number; i < magic_number + _sample_number; i++) {
                _samples.push_back(DataSampleNumber{ i });
            }
        }

        const uint32_t _sample_number = 10;
        std::vector<DataSampleNumber> _samples;

        std::shared_ptr<fep3::native::SimulationBus> _sim_bus{ nullptr };
    };
}

/**
 * @detail Test overflow of reader queue. Test sample loss.
 * @req_id FEPSDK-SimulationBus
 *
 */
TEST_F(SimpleDataSample, testOverflowReaderQueue)
{
    const std::string signal_1_name{"signal_1"};

    const size_t queue_size = 5;

    auto reader = _sim_bus->getReader(signal_1_name, 1);
    auto writer = _sim_bus->getWriter(signal_1_name, queue_size);

    for (auto sample : _samples)
    {
        writer->write(sample);
    }

    writer->transmit();

    DataReceiver receiver;

    {
        ::testing::InSequence sequence;
        using M = ::testing::Matcher<const IDataSample&>;
        EXPECT_CALL(receiver,
            onSampleReceived(M(fep3::mock::DataSampleMatcher(_samples[_samples.size() - 1]))))
            .Times(1);

        EXPECT_CALL(receiver, onSampleReceived(::testing::_))
            .Times(0);
    }

    while(reader->pop(receiver));
}

/**
 * @detail Test overflow of writer queue. Test sample loss.
 * @req_id FEPSDK-SimulationBus
 *
 */
TEST_F(SimpleDataSample, testOverflowWriterQueue)
{
    const std::string signal_1_name{ "signal_1" };

    const size_t queue_size = 5;

    auto reader = _sim_bus->getReader(signal_1_name, queue_size);
    auto writer = _sim_bus->getWriter(signal_1_name, 1);

    for (auto sample : _samples)
    {
        writer->write(sample);
    }

    writer->transmit();

    DataReceiver receiver;

    {
        ::testing::InSequence sequence;
        using M = ::testing::Matcher<const IDataSample&>;
        EXPECT_CALL(receiver,
            onSampleReceived(M(fep3::mock::DataSampleMatcher(_samples[_samples.size() - 1]))))
            .Times(1);

        EXPECT_CALL(receiver, onSampleReceived(::testing::_))
            .Times(0);
    }

    while(reader->pop(receiver));
}
