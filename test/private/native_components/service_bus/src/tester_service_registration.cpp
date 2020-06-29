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
#include <fep3/components/service_bus/rpc/fep_rpc.h>
#include <fep3/rpc_services/base/fep_rpc_client.h>

#include <testclientstub.h>
#include <testserverstub.h>

#include <fep3/native_components/service_bus/service_bus.h>

class ITestInterface
{
public:
    FEP_RPC_IID("ITestInterface", "test_service");
};

class TestService : public fep3::rpc::RPCService<::test::rpc_stubs::TestInterfaceServer,
    ITestInterface>
{
    int _value = 0;
public:
    static int GetRunlevel_call_count;
    static int GetObjects_call_count;
    static int GetRPCIIDForObjects_call_count;
    static int SetRunlevel_call_count;

    virtual std::string GetObjects()
    {
        ++GetObjects_call_count;
        return "bla, blubb, bla";
    }
    virtual std::string GetRPCIIDForObject(const std::string& strObject)
    {
        ++GetRPCIIDForObjects_call_count;
        if (strObject == "bla")
        {
            return "blubb";
        }
        else if (strObject == "blubb")
        {
            return "bla";
        }
        return {};
    }
    virtual int GetRunlevel()
    {
        ++GetRunlevel_call_count;
        return _value;
    }
    virtual Json::Value SetRunlevel(int nRunLevel)
    {
        ++SetRunlevel_call_count;
        _value = nRunLevel;

        Json::Value val;
        val["ErrorCode"] = _value;
        val["Description"] = "Desc";
        val["Line"] = 1234;
        val["File"] = "File";
        val["Function"] = "Foo";
        return val;
    }
};
int TestService::GetObjects_call_count = 0;
int TestService::GetRunlevel_call_count = 0;
int TestService::SetRunlevel_call_count = 0;
int TestService::GetRPCIIDForObjects_call_count = 0;


class TestClient : public fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestInterfaceClient,
    ITestInterface>
{
public:
    TestClient(const char* service_name,
        const std::shared_ptr<fep3::rpc::IRPCRequester>& rpc) : fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestInterfaceClient,
        ITestInterface>(service_name, rpc)
    {
    }
};

/**
 * @detail Test the registration, unregistration and memorymanagment of the ServiceBus
 * @req_id FEPSDK-ServiceBus
 *
 */
TEST(ServciceBusServer, testRegistrationOfServices)
{
    constexpr const char* const test_server_url = "http://localhost:9900";
    auto test_service = std::make_shared<TestService>();
    fep3::native::ServiceBus bus;

    ASSERT_TRUE(fep3::isOk(bus.createSystemAccess("sysname",
        "",
        true)));

    auto sys_access = bus.getSystemAccess("sysname");
    ASSERT_TRUE(sys_access);

    //now create the server
    ASSERT_TRUE(fep3::isOk(sys_access->createServer("name_of_server",
        test_server_url)));

    //default server set now
    auto server = bus.getServer();
    ASSERT_TRUE(server);

    //register the service
    ASSERT_TRUE(fep3::isOk(server->registerService("test_service", test_service)));

    //register twice is not possible
    ASSERT_FALSE(fep3::isOk(server->registerService("test_service", test_service)));

    //impl test 
    fep3::rpc::RPCClient<ITestInterface> my_interface_client;

    //test the client server connections
    TestClient client(ITestInterface::getRPCDefaultName(),
    bus.getRequester(test_server_url, true));

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::SetRunlevel_call_count, 0);
        auto val = client.SetRunlevel(1234);
        ASSERT_EQ(TestService::SetRunlevel_call_count, 1);
    );

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::GetRunlevel_call_count, 0);
        auto run_level = client.GetRunlevel();
        ASSERT_EQ(TestService::GetRunlevel_call_count, 1);
        ASSERT_EQ(run_level, 1234);
    );

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::GetObjects_call_count, 0);
        auto objects = client.GetObjects();
        ASSERT_EQ(TestService::GetObjects_call_count, 1);
        ASSERT_EQ(objects, "bla, blubb, bla");
    );

    ASSERT_NO_THROW(
        ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 0);
        auto object_IID = client.GetRPCIIDForObject("bla");
        ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 1);
        ASSERT_EQ(object_IID, "blubb");
    );

    ASSERT_NO_THROW(
        auto object_IID = client.GetRPCIIDForObject("blubb");
        ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 2);
        ASSERT_EQ(object_IID, "bla");
    );

    ASSERT_NO_THROW(
        auto object_IID = client.GetRPCIIDForObject("test");
        ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 3);
        ASSERT_EQ(object_IID, "");
    );

    //unregister the service
    ASSERT_TRUE(fep3::isOk(server->unregisterService("test_service")));

    ASSERT_ANY_THROW(
        auto object_IID = client.GetRPCIIDForObject("test");
    );

    ASSERT_EQ(TestService::GetRPCIIDForObjects_call_count, 3);
}
