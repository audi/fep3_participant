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

#include <fep3/plugin/c/c_host_plugin.h>
#include <fep3/participant/component_factories/c/component_creator_c_plugin.h>
#include <fep3/components/simulation_bus/mock/mock_simulation_bus.h>
#include <fep3/base/sample/mock/mock_data_sample.h>
#include <fep3/base/streamtype/mock/mock_streamtype.h>
#include <fep3/components/simulation_bus/c_access_wrapper/simulation_bus_c_access_wrapper.h>
#include <fep3/base/sample/data_sample.h>
#include <fep3/base/sample/raw_memory.h>
#include <helper/component_c_plugin_helper.h>

const std::string test_plugin_1_path = PLUGIN;

using namespace fep3::plugin::c::arya;
using namespace fep3::plugin::c::access::arya;

struct Plugin1PathGetter
{
    std::string operator()() const
    {
        return test_plugin_1_path;
    }
};
struct SetMockComponentFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockSimulationBus";
    }
};

/**
 * Test fixture class loading a mocked simulation bus from within a C plugin
 */
using SimulationBusLoader = MockedComponentCPluginLoader
    <::fep3::ISimulationBus
    , ::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>
    , ::fep3::plugin::c::access::arya::SimulationBus
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
using SimulationBusLoaderFixture = MockedComponentCPluginLoaderFixture<SimulationBusLoader>;

/**
 * Test method fep3::ISimulationBus::isSupported of a simulation bus
 * that resides in a C plugin
 * @req_id FEPSDK-1915
 */
TEST_F(SimulationBusLoaderFixture, testMethod_isSupported)
{
    const ::fep3::StreamType supported_stream_type_1(::fep3::StreamMetaType("test_stream_meta_type_1"));
    const ::fep3::StreamType supported_stream_type_2(::fep3::StreamMetaType("test_stream_meta_type_2"));
    const ::fep3::StreamType unsupported_stream_type(::fep3::StreamMetaType("test_stream_meta_type_3"));

    // setting of expectations
    {
        auto& mock_simulation_bus = getMockComponent();

        EXPECT_CALL(mock_simulation_bus, isSupported(::testing::_))
            .WillOnce(::testing::Return(true))
            .WillOnce(::testing::Return(true))
            .WillOnce(::testing::Return(false))
            ;
    }
    ::fep3::arya::ISimulationBus* simulation_bus = getComponent();
    ASSERT_NE(nullptr, simulation_bus);
    EXPECT_TRUE(simulation_bus->isSupported(supported_stream_type_1));
    EXPECT_TRUE(simulation_bus->isSupported(supported_stream_type_2));
    EXPECT_FALSE(simulation_bus->isSupported(unsupported_stream_type));
}

/**
 * Test method fep3::ISimulationBus::getReader (all overloads) of a simulation bus
 * that resides in a C plugin
 * @req_id FEPSDK-1915
 */
TEST_F(SimulationBusLoaderFixture, testMethod_getReader)
{
    const std::string signal_1_name{"signal_1"};
    const ::fep3::StreamType stream_type_1(::fep3::StreamMetaType("test_stream_meta_type_1"));

    // setting of expectations
    {
        auto& mock_simulation_bus = getMockComponent();
        ::testing::InSequence call_sequence;

        { // getReader by signal name and stream type
            auto mock_data_reader = std::make_unique<::testing::NiceMock<::fep3::mock::DataReader>>();
            EXPECT_CALL
                (mock_simulation_bus
                , getReader_
                    (signal_1_name
                    , ::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_1))
                    ))
                .WillOnce(::testing::Return(mock_data_reader.release()));
        }
        { // getReader by signal name, stream type and queue capacity
            auto mock_data_reader = std::make_unique<::testing::NiceMock<::fep3::mock::DataReader>>();
            EXPECT_CALL
                (mock_simulation_bus
                , getReader_
                    (signal_1_name
                    , ::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_1))
                    , 1
                    ))
                .WillOnce(::testing::Return(mock_data_reader.release()));
        }
        { // getReader by signal name
            auto mock_data_reader = std::make_unique<::testing::NiceMock<::fep3::mock::DataReader>>();
            EXPECT_CALL(mock_simulation_bus, getReader_(signal_1_name))
                .WillOnce(::testing::Return(mock_data_reader.release()));
        }
        { // getReader by signal name and queue capacity
            auto mock_data_reader = std::make_unique<::testing::NiceMock<::fep3::mock::DataReader>>();
            EXPECT_CALL
                (mock_simulation_bus
                , getReader_
                    (signal_1_name
                    , 2
                    ))
                .WillOnce(::testing::Return(mock_data_reader.release()));
        }
    }
    ::fep3::arya::ISimulationBus* simulation_bus = getComponent();
    ASSERT_NE(nullptr, simulation_bus);
    EXPECT_NO_THROW(simulation_bus->getReader(signal_1_name, stream_type_1));
    EXPECT_NO_THROW(simulation_bus->getReader(signal_1_name, stream_type_1, 1));
    EXPECT_NO_THROW(simulation_bus->getReader(signal_1_name));
    EXPECT_NO_THROW(simulation_bus->getReader(signal_1_name, 2));
}

/**
 * Test method fep3::ISimulationBus::getWriter (all overloads) of a simulation bus
 * that resides in a C plugin
 * @req_id FEPSDK-1915
 */
TEST_F(SimulationBusLoaderFixture, testMethod_getWriter)
{
    const std::string signal_1_name{"signal_1"};
    const ::fep3::StreamType stream_type_1(::fep3::StreamMetaType("test_stream_meta_type_1"));

    // setting of expectations
    {
        auto& mock_simulation_bus = getMockComponent();
        ::testing::InSequence call_sequence;

        { // getWriter by signal name and stream type
            auto mock_data_writer = std::make_unique<::testing::NiceMock<::fep3::mock::DataWriter>>();
            EXPECT_CALL
                (mock_simulation_bus
                , getWriter_
                    (signal_1_name
                    , ::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_1))
                    ))
                .WillOnce(::testing::Return(mock_data_writer.release()));
        }
        { // getWriter by signal name, stream type and queue capacity
            auto mock_data_writer = std::make_unique<::testing::NiceMock<::fep3::mock::DataWriter>>();
            EXPECT_CALL
                (mock_simulation_bus
                , getWriter_
                    (signal_1_name
                    , ::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_1))
                    , 3
                    ))
                .WillOnce(::testing::Return(mock_data_writer.release()));
        }
        { // getWriter by signal name
            auto mock_data_writer = std::make_unique<::testing::NiceMock<::fep3::mock::DataWriter>>();
            EXPECT_CALL(mock_simulation_bus, getWriter_(signal_1_name))
                .WillOnce(::testing::Return(mock_data_writer.release()));
        }
        { // getWriter by signal name and queue capacity
            auto mock_data_writer = std::make_unique<::testing::NiceMock<::fep3::mock::DataWriter>>();
            EXPECT_CALL
                (mock_simulation_bus
                , getWriter_
                    (signal_1_name
                    , 4
                    ))
                .WillOnce(::testing::Return(mock_data_writer.release()));
        }
    }
    
    ::fep3::arya::ISimulationBus* simulation_bus = getComponent();
    ASSERT_NE(nullptr, simulation_bus);
    EXPECT_NO_THROW(simulation_bus->getWriter(signal_1_name, stream_type_1));
    EXPECT_NO_THROW(simulation_bus->getWriter(signal_1_name, stream_type_1, 3));
    EXPECT_NO_THROW(simulation_bus->getWriter(signal_1_name));
    EXPECT_NO_THROW(simulation_bus->getWriter(signal_1_name, 4));
}

// action popping a stream type
ACTION_P(Pop, shared_pointer_to_item)
{
    arg0.operator()(shared_pointer_to_item);
    return true; // receiver was invoked, so return true
}
// action receiving a stream type
ACTION_P(Receive, shared_pointer_to_item)
{
    arg0.operator()(shared_pointer_to_item);
}

/**
 * Test ISimulationBus::IDataReader interface through C plugin
 * @req_id FEPSDK-1915
 */
TEST_F(SimulationBusLoaderFixture, testIDataReader)
{
    const std::string signal_1_name{"signal_1"};
    const ::fep3::Timestamp next_timestamp(999);

    auto unique_pointer_to_mock_data_reader = std::make_unique<::testing::StrictMock<::fep3::mock::DataReader>>();
    auto& mock_data_reader = *unique_pointer_to_mock_data_reader.get();

    // setting of expectations for calls to ISimulationBus methods
    {
        auto& mock_simulation_bus = getMockComponent();

        // getReader by signal name
        EXPECT_CALL(mock_simulation_bus, getReader_(signal_1_name))
            .WillOnce(::testing::Return(unique_pointer_to_mock_data_reader.release()));
    }

    ::fep3::arya::ISimulationBus* simulation_bus = getComponent();
    ASSERT_NE(nullptr, simulation_bus);
    std::unique_ptr<::fep3::ISimulationBus::IDataReader> unique_pointer_to_data_reader;
    // note: we want to test the IDataReader interface here, so we can just use any of the ISimulationBus::getReader methods
    // (i. e. no need to call all of the ISimulationBus::getReader overloads)
    EXPECT_NO_THROW(unique_pointer_to_data_reader = simulation_bus->getReader(signal_1_name));
    ASSERT_TRUE(unique_pointer_to_data_reader);

    const auto& mock_stream_type = std::make_shared<::fep3::mock::StreamType>();
    const auto& mock_data_sample = std::make_shared<::fep3::mock::DataSample>();

    // setting of expectations for calls to ISimulationBus::IDataReader methods
    {
        ::testing::InSequence sequence;
        
        EXPECT_CALL(mock_data_reader, size()).WillOnce(::testing::Return(11));
        EXPECT_CALL(mock_data_reader, capacity()).WillOnce(::testing::Return(22));
        EXPECT_CALL(mock_data_reader, pop(::testing::_))
            .WillOnce(Pop(mock_stream_type))
            .WillOnce(Pop(mock_data_sample))
            ;
        EXPECT_CALL(mock_data_reader, receive(::testing::_))
            .WillOnce(Receive(mock_stream_type))
            .WillOnce(Receive(mock_data_sample))
            ;
        EXPECT_CALL(mock_data_reader, stop()).WillOnce(::testing::Return());
        EXPECT_CALL(mock_data_reader, getFrontTime()).WillOnce(::testing::Return(::testing::ByMove(::fep3::Optional<::fep3::Timestamp>(next_timestamp))));
    }
    EXPECT_EQ(11, unique_pointer_to_data_reader->size());
    EXPECT_EQ(22, unique_pointer_to_data_reader->capacity());

    auto mock_data_receiver = std::make_unique<::testing::NiceMock<::fep3::mock::DataReceiver>>();

    EXPECT_TRUE(unique_pointer_to_data_reader->pop(*mock_data_receiver.get()));
    EXPECT_TRUE(unique_pointer_to_data_reader->pop(*mock_data_receiver.get()));
    unique_pointer_to_data_reader->receive(*mock_data_receiver.get());
    unique_pointer_to_data_reader->receive(*mock_data_receiver.get());
    unique_pointer_to_data_reader->stop();
    EXPECT_EQ(::fep3::Optional<::fep3::Timestamp>(next_timestamp), unique_pointer_to_data_reader->getFrontTime());
}

/**
 * Test ISimulationBus::IDataReceiver interface through C plugin
 * @req_id FEPSDK-1915
 */
TEST_F(SimulationBusLoaderFixture, testIDataReceiver)
{
    const std::string signal_1_name{"signal_1"};
    const auto& stream_type_1 = std::make_shared<::fep3::StreamType>(::fep3::StreamMetaType("test_stream_meta_type_1"));
    const auto& data_sample_1 = std::make_shared<::fep3::DataSample>();
    uint32_t data_sample_value{55};
    data_sample_1->update(::fep3::Timestamp(33), 44, ::fep3::RawMemoryStandardType<decltype(data_sample_value)>(data_sample_value));

    auto unique_pointer_to_mock_data_reader = std::make_unique<::testing::StrictMock<::fep3::mock::DataReader>>();
    auto& mock_data_reader = *unique_pointer_to_mock_data_reader.get();

    // setting of expectations for calls to ISimulationBus methods
    {
        auto& mock_simulation_bus = getMockComponent();

        // getReader by signal name
        EXPECT_CALL(mock_simulation_bus, getReader_(signal_1_name))
            .WillOnce(::testing::Return(unique_pointer_to_mock_data_reader.release()));
    }

    ::fep3::arya::ISimulationBus* simulation_bus = getComponent();
    ASSERT_NE(nullptr, simulation_bus);
    std::unique_ptr<::fep3::ISimulationBus::IDataReader> unique_pointer_to_data_reader;
    // note: we want to test the IDataReader interface here, so we can just use any of the ISimulationBus::getReader methods
    // (i. e. no need to call all of the ISimulationBus::getReader overloads)
    EXPECT_NO_THROW(unique_pointer_to_data_reader = simulation_bus->getReader(signal_1_name));
    ASSERT_TRUE(unique_pointer_to_data_reader);

    ::testing::StrictMock<::fep3::mock::DataReceiver> mock_data_receiver;

    EXPECT_CALL(mock_data_reader, pop(::testing::_))
        .WillOnce(Pop(stream_type_1))
        .WillOnce(Pop(data_sample_1))
        ;

    // setting of expectations for calls to ISimulationBus::IDataReceiver methods
    {
        ::testing::InSequence sequence;
        EXPECT_CALL(mock_data_receiver, call(::testing::Matcher<const ::fep3::data_read_ptr<const ::fep3::IStreamType>&>(fep3::mock::StreamTypeSmartPtrMatcher(stream_type_1))))
            .WillOnce(::testing::Return())
            ;
        EXPECT_CALL(mock_data_receiver, call(::testing::Matcher<const ::fep3::data_read_ptr<const ::fep3::IDataSample>&>(::fep3::mock::DataSampleSmartPtrMatcher(data_sample_1))))
            .WillOnce(::testing::Return())
            ;
    }

    EXPECT_TRUE(unique_pointer_to_data_reader->pop(mock_data_receiver));
    EXPECT_TRUE(unique_pointer_to_data_reader->pop(mock_data_receiver));
}

// action writing a data sample
ACTION_P(WriteDataSample, destination_raw_memory)
{
    arg0.read(*destination_raw_memory);
    return ::fep3::Result{};
}
// action writing a stream type
ACTION_P(WriteStreamType, destination_stream_meta_type_name)
{
    *destination_stream_meta_type_name = arg0.getMetaTypeName();
    return ::fep3::Result{};
}

/**
 * Test ISimulationBus::IDataWriter interface through C plugin
 * @req_id FEPSDK-1915
 */
TEST_F(SimulationBusLoaderFixture, testIDataWriter)
{
    const std::string signal_1_name{"signal_1"};
    ::fep3::DataSample data_sample_1;
    uint32_t data_sample_value{55};
    data_sample_1.update(::fep3::Timestamp(33), 44, ::fep3::RawMemoryStandardType<decltype(data_sample_value)>(data_sample_value));
    const ::fep3::StreamType stream_type_1(::fep3::StreamMetaType("test_stream_meta_type_1"));

    auto unique_pointer_to_mock_data_writer = std::make_unique<::testing::StrictMock<::fep3::mock::DataWriter>>();
    auto& mock_data_writer = *unique_pointer_to_mock_data_writer.get();

    // setting of expectations for calls to ISimulationBus methods
    {
        auto& mock_simulation_bus = getMockComponent();

        // getWriter by signal name and stream type
        EXPECT_CALL(mock_simulation_bus, getWriter_(signal_1_name))
            .WillOnce(::testing::Return(unique_pointer_to_mock_data_writer.release()));
    }

    ::fep3::arya::ISimulationBus* simulation_bus = getComponent();
    ASSERT_NE(nullptr, simulation_bus);
    std::unique_ptr<::fep3::ISimulationBus::IDataWriter> unique_pointer_to_data_writer;
    // note: we want to test the IDataWriter interface here, so we can just use any of the ISimulationBus::getWriter methods
    // (i. e. no need to call all of the ISimulationBus::getWriter overloads)
    EXPECT_NO_THROW(unique_pointer_to_data_writer = simulation_bus->getWriter(signal_1_name));
    ASSERT_TRUE(unique_pointer_to_data_writer);

    uint32_t written_value{0};
    ::fep3::RawMemoryStandardType<decltype(written_value)> written_raw_memory(written_value);
    std::string written_stream_meta_type_name;
    // setting of expectations for calls to ISimulationBus::IDataWriter methods
    {
        ::testing::InSequence sequence;
        
        EXPECT_CALL(mock_data_writer, write(::testing::Matcher<const ::fep3::IDataSample&>(::fep3::mock::DataSampleMatcher(data_sample_1))))
            .WillOnce(WriteDataSample(&written_raw_memory))
            ;
        EXPECT_CALL(mock_data_writer, write(::testing::Matcher<const ::fep3::IStreamType&>(fep3::mock::StreamTypeMatcher(stream_type_1))))
            .WillOnce(WriteStreamType(&written_stream_meta_type_name))
            ;
        EXPECT_CALL(mock_data_writer, transmit()).WillOnce(::testing::Return(::fep3::Result{}));
    }
    EXPECT_EQ(::fep3::Result{}, unique_pointer_to_data_writer->write(data_sample_1));
    EXPECT_EQ(data_sample_value, written_value);
    EXPECT_EQ(::fep3::Result{}, unique_pointer_to_data_writer->write(stream_type_1));
    EXPECT_EQ(::fep3::Result{}, unique_pointer_to_data_writer->transmit());
}