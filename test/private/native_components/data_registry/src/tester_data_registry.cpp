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

#include "test_data_registry_client_stub.h"

#include "fep3/components/base/component_registry.h"
#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include <fep3/base/streamtype/mock/mock_streamtype.h>
#include "fep3/components/simulation_bus/simulation_bus_intf.h" // Used only to override IDataReceiver
#include "fep3/native_components/service_bus/service_bus.h"
#include "fep3/native_components/service_bus/testing/service_bus_testing.hpp"
#include "fep3/native_components/data_registry/data_registry.h"
#include "fep3/native_components/simulation_bus/simulation_bus.h"
#include "fep3/components/simulation_bus/mock/mock_simulation_bus.h"
#include "fep3/rpc_services/base/fep_rpc_client.h"
#include "fep3/rpc_services/data_registry/data_registry_rpc_intf_def.h"

#include "fep3/base/streamtype/default_streamtype.h"
#include "fep3/base/sample/data_sample.h"


bool containsVector(const std::vector<std::string>& source_vec,
                    const std::vector<std::string>& contain_vec)
{
    for (const auto& item : contain_vec)
    {
        bool found = false;
        for (const auto& item_source : source_vec)
        {
            if (item_source == item)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

class TestClient : public fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestDataRegistryClient, fep3::rpc::IRPCDataRegistryDef>
{
private:
    typedef fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestDataRegistryClient, fep3::rpc::IRPCDataRegistryDef> base_type;

public:
    using base_type::GetStub;

    TestClient(const char* server_object_name,
        std::shared_ptr<fep3::rpc::IRPCRequester> rpc) : base_type(server_object_name, rpc)
    {
    }

    std::vector<std::string> getSignalInNames() const
    {
        try
        {
            return a_util::strings::split(GetStub().getSignalInNames(), ",");
        }
        catch (jsonrpc::JsonRpcException&)
        {
            ADD_FAILURE();
        }
        return std::vector<std::string>{};
    }

    std::vector<std::string> getSignalOutNames() const
    {
        try
        {
            return a_util::strings::split(GetStub().getSignalOutNames(), ",");
        }
        catch (jsonrpc::JsonRpcException&)
        {
            ADD_FAILURE();
        }
        return std::vector<std::string>{};
    }

    fep3::StreamType getStreamType(const std::string& signal_name) const
    {
        try
        {
            Json::Value json_value = GetStub().getStreamType(signal_name);
            fep3::StreamType streamtype(fep3::StreamMetaType(json_value["meta_type"].asString()));
            std::vector<std::string> property_names = a_util::strings::split(json_value["properties"]["names"].asString(), ",");
            std::vector<std::string> property_values = a_util::strings::split(json_value["properties"]["values"].asString(), ",", true);
            std::vector<std::string> property_types = a_util::strings::split(json_value["properties"]["types"].asString(), ",");
            for (decltype(property_names)::size_type i = 0; i < property_names.size(); ++i)
            {
                streamtype.setProperty(property_names.at(i), property_values.at(i), property_types.at(i));
            }
            return streamtype;
        }
        catch (jsonrpc::JsonRpcException&)
        {
            ADD_FAILURE();
            return fep3::StreamType{ fep3::StreamMetaType{"void"} };
        }
    }
};

// Dummy class
struct TestDataReceiver : public fep3::ISimulationBus::IDataReceiver
{
    fep3::data_read_ptr<const fep3::IStreamType> _last_type;
    fep3::data_read_ptr<const fep3::IDataSample> _last_sample;

    void operator()(const fep3::data_read_ptr<const fep3::IStreamType>& rec_type) override
    {
        _last_type = rec_type;
    };
    void operator()(const fep3::data_read_ptr<const fep3::IDataSample>& rec_sample) override
    {
        _last_sample = rec_sample;
    };

    void reset()
    {
        _last_type.reset();
        _last_sample.reset();
    }
    bool waitForSampleUpdate(int trycount)
    {
        while (trycount > 0)
        {
            if (_last_sample)
            {
                break;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                trycount--;
            }
        }
        return static_cast<bool>(_last_sample);
    }
};

template <typename sim_bus=fep3::mock::SimulationBus<>>
struct EasyPart
{
    EasyPart() :
        _registry{ std::make_shared<fep3::native::DataRegistry>() },
        _service_bus{ std::make_shared<fep3::native::ServiceBus>() },
        _simulation_bus{ std::make_shared<sim_bus>() },
        _component_registry{ std::make_shared<fep3::ComponentRegistry>() },
        _test_participant_name_default(fep3::native::testing::test_participant_name),
        _test_participant_url_default(fep3::native::testing::test_participant_url)
    {
    }
    EasyPart(const std::string& test_participant_name_default,
             const std::string& test_participant_url_default) :
        _registry{ std::make_shared<fep3::native::DataRegistry>() },
        _service_bus{ std::make_shared<fep3::native::ServiceBus>() },
        _simulation_bus{ std::make_shared<sim_bus>() },
        _component_registry{ std::make_shared<fep3::ComponentRegistry>() },
        _test_participant_name_default(test_participant_name_default),
        _test_participant_url_default(test_participant_url_default)
    {}

    void SetUp()
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus,
            _test_participant_name_default,
            _test_participant_url_default));
        ASSERT_EQ(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->registerComponent<fep3::ISimulationBus>(_simulation_bus), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->registerComponent<fep3::IDataRegistry>(_registry), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->create(), fep3::ERR_NOERROR);
    }

    std::shared_ptr<fep3::native::DataRegistry> _registry{};
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{};
    std::shared_ptr<sim_bus> _simulation_bus{};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
    
    std::string _test_participant_name_default;
    std::string _test_participant_url_default;

};

struct NativeDataRegistry : public ::testing::Test,
                            public EasyPart<>
{
    NativeDataRegistry() : EasyPart<>()
    {}

    void SetUp() override
    {
        EasyPart<>::SetUp();
    }
};

struct NativeDataCommunication : public ::testing::Test
    
{
    NativeDataCommunication()
    {}

    void SetUp() override
    {
        _sender.SetUp();
        _receiver.SetUp();
    }

    void TearDown() override
    {
        stop_deinit();
    }

    void init_run()
    {
        ASSERT_TRUE(fep3::isOk(_sender._component_registry->initialize()));
        ASSERT_TRUE(fep3::isOk(_receiver._component_registry->initialize()));
        ASSERT_TRUE(fep3::isOk(_sender._component_registry->tense()));
        ASSERT_TRUE(fep3::isOk(_receiver._component_registry->tense()));
        ASSERT_TRUE(fep3::isOk(_sender._component_registry->start()));
        ASSERT_TRUE(fep3::isOk(_receiver._component_registry->start()));
        _is_running = true;
    }

    void stop_deinit()
    {
        if (_is_running)
        {
            EXPECT_TRUE(fep3::isOk(_receiver._component_registry->stop()));
            EXPECT_TRUE(fep3::isOk(_sender._component_registry->stop()));
            EXPECT_TRUE(fep3::isOk(_receiver._component_registry->relax()));
            EXPECT_TRUE(fep3::isOk(_sender._component_registry->relax()));
            EXPECT_TRUE(fep3::isOk(_receiver._component_registry->deinitialize()));
            EXPECT_TRUE(fep3::isOk(_sender._component_registry->deinitialize()));
            _is_running = false;
        }
    }

    EasyPart<fep3::native::SimulationBus> _sender{"test_sender", "http://localhost:9921"};
    EasyPart<fep3::native::SimulationBus> _receiver{ "test_receiver", "http://localhost:9922" };

    bool _is_running{false};
};

TEST_F(NativeDataRegistry, testRegisterSignals)
{
    TestClient client(fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(),
                     _service_bus->getRequester(fep3::native::testing::test_participant_name));

    ASSERT_EQ(_registry->registerDataIn("signal_in_1", fep3::StreamType{ fep3::StreamMetaType{"meta_type_raw"} }), fep3::ERR_NOERROR);
    //we do not care which meta type is use ... we support everything in data registry (all kind)
    //we do not check any special support for types becuse we can deal with every thing and
    // we will deal with special types like DDL for mapping and something like that
    ASSERT_EQ(_registry->registerDataIn("signal_in_2", fep3::StreamType{ fep3::StreamMetaType{"unknown_type"} }), fep3::ERR_NOERROR);
    //we can not register it a second time with a different type
    ASSERT_EQ(_registry->registerDataIn("signal_in_1", fep3::StreamType{ fep3::StreamMetaType{"meta_type_ddl"} }), fep3::ERR_INVALID_TYPE);

    ASSERT_EQ(_registry->registerDataOut("signal_out_1", fep3::StreamType{ fep3::StreamMetaType{"meta_type_raw"} }), fep3::ERR_NOERROR);
    //we also support unknown types
    ASSERT_EQ(_registry->registerDataOut("signal_out_2", fep3::StreamType{ fep3::StreamMetaType{"unknown_type"} }), fep3::ERR_NOERROR);
    //we can not register it a second time with a different type
    ASSERT_EQ(_registry->registerDataOut("signal_out_1", fep3::StreamType{ fep3::StreamMetaType{"meta_type_ddl"} }), fep3::ERR_INVALID_TYPE);

    ASSERT_EQ(client.getSignalInNames().size(), 2);
    ASSERT_TRUE(containsVector(client.getSignalInNames(), 
                               { "signal_in_1" , "signal_in_2"}));
    ASSERT_EQ(client.getSignalOutNames().size(), 2);
    ASSERT_TRUE(containsVector(client.getSignalOutNames(), 
                               { "signal_out_1" , "signal_out_2"}));

    ASSERT_EQ(_registry->unregisterDataIn("signal_in_1"), fep3::ERR_NOERROR);
    ASSERT_EQ(_registry->unregisterDataIn("signal_in_2"), fep3::ERR_NOERROR);
    ASSERT_EQ(_registry->unregisterDataIn("signal_in_3"), fep3::ERR_NOT_FOUND);

    ASSERT_EQ(_registry->unregisterDataOut("signal_out_1"), fep3::ERR_NOERROR);
    ASSERT_EQ(_registry->unregisterDataOut("signal_out_2"), fep3::ERR_NOERROR);
    ASSERT_EQ(_registry->unregisterDataOut("signal_out_3"), fep3::ERR_NOT_FOUND);

    ASSERT_EQ(client.getSignalInNames().size(), 0);
    ASSERT_EQ(client.getSignalOutNames().size(), 0);
}

TEST_F(NativeDataRegistry, testWriter)
{
    ASSERT_EQ(_registry->registerDataOut("signal_out", fep3::StreamType{ fep3::StreamMetaType{"meta_type_raw"} }), fep3::ERR_NOERROR);
    auto writer = _registry->getWriter("signal_out");
    ASSERT_TRUE(writer);
    ASSERT_FALSE(_registry->getWriter("unknown_signal"));
}

TEST_F(NativeDataRegistry, testReader)
{
    ASSERT_EQ(_registry->registerDataOut("signal_in", fep3::StreamType{ fep3::StreamMetaType{"meta_type_raw"} }), fep3::ERR_NOERROR);

    ASSERT_EQ(_registry->registerDataIn("signal_in", fep3::StreamType{ fep3::StreamMetaType{"meta_type_raw"} }), fep3::ERR_NOERROR);
    auto reader1 = _registry->getReader("signal_in");
    ASSERT_TRUE(reader1);
    auto reader2 = _registry->getReader("signal_in");
    ASSERT_TRUE(reader2);
    ASSERT_FALSE(_registry->getReader("unknown_signal"));
}

TEST_F(NativeDataRegistry, testListenerRegistration)
{
    ASSERT_EQ(_registry->registerDataIn("signal_in", fep3::StreamType{ fep3::StreamMetaType{ "meta_type_raw" } }), fep3::ERR_NOERROR);
    auto listener = std::make_shared<TestDataReceiver>();
    ASSERT_EQ(_registry->registerDataReceiveListener("signal_in", listener), fep3::ERR_NOERROR);
    ASSERT_EQ(_registry->registerDataReceiveListener("unknown_signal", listener), fep3::ERR_NOT_FOUND);

    ASSERT_EQ(_registry->unregisterDataReceiveListener("signal_in", listener), fep3::ERR_NOERROR);
    ASSERT_EQ(_registry->unregisterDataReceiveListener("unknown_signal", listener), fep3::ERR_NOT_FOUND);
}



TEST_F(NativeDataCommunication, sendAndReceiveData)
{
    auto& data_reg_sender = _sender._registry;
    auto& data_reg_receiver = _receiver._registry;

    ASSERT_TRUE(fep3::isOk(data_reg_sender->registerDataOut("string_data", fep3::StreamTypeString(0))));
    ASSERT_TRUE(fep3::isOk(data_reg_receiver->registerDataIn("string_data", fep3::StreamTypeString(0))));

    auto listener = std::make_shared<TestDataReceiver>();
    ASSERT_EQ(data_reg_receiver->registerDataReceiveListener("string_data", listener), fep3::ERR_NOERROR);

    ASSERT_NO_THROW(
        auto readerqueuetest = data_reg_receiver->getReader("string_data");
        auto writerqueuetest = data_reg_sender->getWriter("string_data");
    );

    auto readerreceiver_dynamic_size = std::make_shared<TestDataReceiver>();
    auto readerqueue_dynamic_size = data_reg_receiver->getReader("string_data");
    auto readerreceiver_1 = std::make_shared<TestDataReceiver>();
    auto readerqueue_1 = data_reg_receiver->getReader("string_data", 1);
    auto writerqueue = data_reg_sender->getWriter("string_data");

    init_run();

    //just write one now! 
    std::string value_written = "string_written";
    //this is the time where the serialization is set at the moment
    // ... this class will serialize while writing with copy only.
    fep3::DataSampleType<std::string> value_to_write(value_written);
    ASSERT_TRUE(fep3::isOk(writerqueue->write(value_to_write)));
    
    listener->reset();
    ASSERT_TRUE(fep3::isOk(writerqueue->flush()));

    //check if it is received in an asynchronous time ;-)
    listener->waitForSampleUpdate(20);

    //data triggered
    ASSERT_TRUE(listener->_last_sample);

    //assync dynamic queue
    readerreceiver_dynamic_size->reset();
    EXPECT_TRUE(fep3::isOk(readerqueue_dynamic_size->pop(*readerreceiver_dynamic_size)));

    //check if it is received now ;-)
    ASSERT_TRUE(readerreceiver_dynamic_size->_last_sample);

    //assync queue 1
    readerreceiver_1->reset();
    EXPECT_TRUE(fep3::isOk(readerqueue_1->pop(*readerreceiver_1)));

    //check if it is received now ;-)
    ASSERT_TRUE(readerreceiver_1->_last_sample);

    //check content
    std::string value_read_from_listener;
    {
        fep3::RawMemoryClassType<std::string> string_ref(value_read_from_listener);
        //expect the string length + 1 because i know the serialization
        EXPECT_EQ(listener->_last_sample->read(string_ref), value_written.length() + 1);
    }

    std::string value_read_from_reader_dynamic_size;
    {
        fep3::RawMemoryClassType<std::string> string_ref(value_read_from_reader_dynamic_size);
        //expect the string length + 1 because i know the serialization
        EXPECT_EQ(readerreceiver_dynamic_size->_last_sample->read(string_ref), value_written.length() + 1);
    }

    std::string value_read_from_reader_1;
    {
        fep3::RawMemoryClassType<std::string> string_ref(value_read_from_reader_1);
        //expect the string length + 1 because i know the serialization
        EXPECT_EQ(readerreceiver_1->_last_sample->read(string_ref), value_written.length() + 1);
    }

    EXPECT_EQ(value_read_from_listener, value_read_from_reader_dynamic_size);
    EXPECT_EQ(value_read_from_listener, value_read_from_reader_1);
    EXPECT_EQ(value_read_from_listener, value_written);
}