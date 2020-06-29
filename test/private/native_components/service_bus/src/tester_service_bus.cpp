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
#include <a_util/process.h>
#include <list>

#include <fep3/native_components/service_bus/service_bus.h>


/**
 * @detail Test the registration, unregistration and memorymanagment of the ServiceBus
 * @req_id FEPSDK-ServiceBus
 * 
 */
TEST(ServciceBusServer, testCreationAndDestroyingOfSystemAccess)
{
    fep3::native::ServiceBus bus;
    ASSERT_TRUE(fep3::isOk(bus.createSystemAccess("sysname",
        fep3::IServiceBus::ISystemAccess::_use_default_url)));

    auto sys_access = bus.getSystemAccess("sysname");
    ASSERT_TRUE(sys_access);

    // not yet created
    auto sys_access2 = bus.getSystemAccess("sysname2");
    ASSERT_FALSE(sys_access2);

    ASSERT_TRUE(fep3::isOk(bus.createSystemAccess("sysname2",
        fep3::IServiceBus::ISystemAccess::_use_default_url,
                                                   true)));

    // now the second one is created
    sys_access2 = bus.getSystemAccess("sysname2");
    ASSERT_TRUE(sys_access2);

    // failure because already exists
    ASSERT_FALSE(fep3::isOk(bus.createSystemAccess("sysname2",
        fep3::IServiceBus::ISystemAccess::_use_default_url)));

    // failure because invalid scheme
    ASSERT_FALSE(fep3::isOk(bus.createSystemAccess("name_of_system_invalid_scheme",
                                                   "foo://0.0.0.0:9091")));

    // failure because invalid url
    ASSERT_FALSE(fep3::isOk(bus.createSystemAccess("name_of_sys_invalid_url",
        "0.0.0.0:9091")));

    //destroy it
    ASSERT_TRUE(fep3::isOk(bus.releaseSystemAccess("sysname2")));

    // not accessible anymore!
    sys_access2 = bus.getSystemAccess("sysname2");
    ASSERT_FALSE(sys_access2);

    //can not destroy it ... it does not exist anymore
    ASSERT_TRUE(fep3::isFailed(bus.releaseSystemAccess("sysname2")));
}

/**
 * @detail Test the registration, unregistration and memorymanagment of the ServiceBus
 * @req_id FEPSDK-ServiceBus
 *
 */
TEST(ServciceBusServer, testCreationAndDestroyingOfServer)
{
    fep3::native::ServiceBus bus;

    //no default server set
    auto server = bus.getServer();
    ASSERT_FALSE(server);

    ASSERT_TRUE(fep3::isOk(bus.createSystemAccess("sysname",
        fep3::IServiceBus::ISystemAccess::_use_default_url,
        true)));

    auto sys_access = bus.getSystemAccess("sysname");
    ASSERT_TRUE(sys_access);

    //no default server set yet
    server = bus.getServer();
    ASSERT_FALSE(server);

    //now create the server
    ASSERT_TRUE(fep3::isOk(sys_access->createServer("name_of_server",
                                                    "http://localhost:9900")));

    //default server set now
    server = bus.getServer();
    ASSERT_TRUE(server);

    //default server is the same like in "sysname" system access
    auto server_same = sys_access->getServer();
    ASSERT_TRUE(server_same);

    ASSERT_EQ(server_same->getName(), server->getName());
    ASSERT_EQ(server_same->getUrl(), server->getUrl());

    //just make sure another system access will not override the default
    ASSERT_TRUE(fep3::isOk(bus.createSystemAccess("sysname_for_failure_tests",
        fep3::IServiceBus::ISystemAccess::_use_default_url,
        false)));

    //default server is still set (because we use it from the first system access)
    server = bus.getServer();
    ASSERT_TRUE(server);
    ASSERT_EQ(server_same->getName(), server->getName());
    ASSERT_EQ(server_same->getUrl(), server->getUrl());

    // failure test: because invalid scheme in server url for native service bus impl
    auto sys_access_for_failure_tests = bus.getSystemAccess("sysname_for_failure_tests");
    ASSERT_TRUE(sys_access);
    ASSERT_FALSE(fep3::isOk(sys_access_for_failure_tests->createServer("name_of_system_invalid_scheme",
                                                                       "foo://0.0.0.0:9091")));

    // failure because invalid scheme in url
    ASSERT_FALSE(fep3::isOk(sys_access_for_failure_tests->createServer("name_of_server_invalid_url",
                                                                       "//0.0.0.0:9091")));
}

/**
 * @detail Test the registration, unregistration and memorymanagment of the ServiceBus
 * @req_id FEPSDK-ServiceBus
 *
 */
TEST(ServciceBusServer, testDefaultLoadingOfServiceBus)
{
    fep3::native::ServiceBus bus;
    ASSERT_TRUE(fep3::isOk(bus.createSystemAccess("default_system",
                                                  fep3::IServiceBus::ISystemAccess::_use_default_url)));
    auto sys_access = bus.getSystemAccess("default_system");
    ASSERT_TRUE(fep3::isOk(sys_access->createServer("default_server",
                                                  fep3::IServiceBus::ISystemAccess::_use_default_url)));
}



bool contains(const std::multimap<std::string, std::string>& servers,
              const std::list<std::string>& list_of_content_to_check)
{
    for (const auto& current_string_to_check : list_of_content_to_check)
    {
        auto found = (servers.find(current_string_to_check) != servers.cend());
        if (!found)
        {
            //did not find it 
            return false;
        }
    }
    return true;
}

/**
 * @detail Test the discovery methods of the native HTTP System Access and the creation of it
 * @req_id FEPSDK-ServiceBus
 *
 */
TEST(ServciceBusServer, testHTTPSystemAccessDiscovery)
{
#ifdef WIN32
    constexpr const char* const ADDR_USE_FOR_TEST = "http://230.231.0.0:9993";
#else
    constexpr const char* const ADDR_USE_FOR_TEST = fep3::IServiceBus::ISystemAccess::_use_default_url;
#endif
    std::stringstream ss;
    ss << std::this_thread::get_id();

    std::string system_name_for_test_1 = std::string("system_1_")
        + a_util::strings::toString(a_util::process::getCurrentProcessId())
        + std::string("_")
        + ss.str();

    std::string system_name_for_test_2 = std::string("system_2_")
        + a_util::strings::toString(a_util::process::getCurrentProcessId())
        + std::string("_")
        + ss.str();
    
    //create a system access to the named system "system_name_for_test_1" on the default URL
    fep3::native::ServiceBus bus1;
    ASSERT_TRUE(fep3::isOk(bus1.createSystemAccess(system_name_for_test_1,
        ADDR_USE_FOR_TEST)));

    //create one server within this system_name_for_test_1 (so it is discoverable)
    auto sys_access1 = bus1.getSystemAccess(system_name_for_test_1);
    ASSERT_TRUE(fep3::isOk(sys_access1->createServer("server_1",
        fep3::IServiceBus::ISystemAccess::_use_default_url)));

    //create another system access to the same system under the same discovery url in another ServiceBus instance
    fep3::native::ServiceBus bus2;
    ASSERT_TRUE(fep3::isOk(bus2.createSystemAccess(system_name_for_test_1,
        ADDR_USE_FOR_TEST)));

    auto sys_access2 = bus2.getSystemAccess(system_name_for_test_1);

    //usually this will discover the server in sys_access1, but is asyncronously
    //the discover will send a search and wait at least the given time for responses!
    auto list_of_discovered = sys_access2->discover(std::chrono::seconds(1));
    ASSERT_EQ(list_of_discovered.size(), 1);

    auto const& ref = list_of_discovered.begin();
    ASSERT_EQ(ref->first, "server_1");

    //create another server within this system_name_for_test_1 (so it is discoverable)
    ASSERT_TRUE(fep3::isOk(sys_access2->createServer("server_2",
        fep3::IServiceBus::ISystemAccess::_use_default_url)));

    //make sure this both server is now discoverable thru both access points
    //this is now the first access point on bus1
    auto list_of_discovered_at_1 = sys_access1->discover(std::chrono::seconds(1));
    ASSERT_EQ(list_of_discovered_at_1.size(), 2);

    ASSERT_TRUE(contains(list_of_discovered_at_1, { "server_1" , "server_2" }));

    //make sure this both server is now discoverable thru both access points
    //this is now the second access point
    auto list_of_discovered_at_2 = sys_access2->discover(std::chrono::milliseconds(5));
    ASSERT_EQ(list_of_discovered_at_2.size(), 2);

    ASSERT_TRUE(contains(list_of_discovered_at_2, { "server_1" , "server_2" }));

}

/**
 * @detail Test the discovery methods of the native HTTP System Access and the creation of it
 * @req_id FEPSDK-ServiceBus
 *
 */
TEST(ServciceBusServer, testHTTPSystemAccessDiscoveryAllSystems)
{
#ifdef WIN32
    constexpr const char* const ADDR_USE_FOR_TEST = "http://230.231.0.0:9993";
#else
    constexpr const char* const ADDR_USE_FOR_TEST = fep3::IServiceBus::ISystemAccess::_use_default_url;
#endif
    std::stringstream ss;
    ss << std::this_thread::get_id();

    std::string system_name_for_test_1 = std::string("system_1_")
        + a_util::strings::toString(a_util::process::getCurrentProcessId())
        + std::string("_")
        + ss.str();

    std::string system_name_for_test_2 = std::string("system_2_")
        + a_util::strings::toString(a_util::process::getCurrentProcessId())
        + std::string("_")
        + ss.str();

    //create a system access to the named system "system_name_for_test_1" on the default URL
    fep3::native::ServiceBus bus1;
    ASSERT_TRUE(fep3::isOk(bus1.createSystemAccess(system_name_for_test_1,
        ADDR_USE_FOR_TEST)));

    //create a system access to the named system "system_name_for_test_2" on the default URL
    ASSERT_TRUE(fep3::isOk(bus1.createSystemAccess(system_name_for_test_2,
        ADDR_USE_FOR_TEST)));

    //create one server within this system_name_for_test_1 (so it is discoverable)
    //so we have server1@system_name_for_test_1
    auto sys_access1 = bus1.getSystemAccess(system_name_for_test_1);
    ASSERT_TRUE(fep3::isOk(sys_access1->createServer("server_1",
        fep3::IServiceBus::ISystemAccess::_use_default_url)));

    //create one server within this system_name_for_test_2 (so it is discoverable)
    //so we have server2@system_name_for_test_2
    //       AND server1@system_name_for_test_1
    auto sys_access2 = bus1.getSystemAccess(system_name_for_test_2);
    ASSERT_TRUE(fep3::isOk(sys_access2->createServer("server_2",
        fep3::IServiceBus::ISystemAccess::_use_default_url)));

    //create another system access to the same system under the same discovery url in another ServiceBus instance
    fep3::native::ServiceBus bus2;
    ASSERT_TRUE(fep3::isOk(bus2.createSystemAccess(system_name_for_test_1,
        ADDR_USE_FOR_TEST)));

    //create one server within this system_name_for_test_1 (so it is discoverable)
    //so we have server3@system_name_for_test_1 
    //       AND server2@system_name_for_test_2
    //       AND server1@system_name_for_test_1
    auto sys_access3 = bus2.getSystemAccess(system_name_for_test_1);
    ASSERT_TRUE(fep3::isOk(sys_access3->createServer("server_3",
        fep3::IServiceBus::ISystemAccess::_use_default_url)));

    //create a system access to special discovery mode "fep3::IServiceBus::ISystemAccess::_discover_all_systems"
    // on given URL (where the above servers must be available to)
    fep3::native::ServiceBus bus3;
    ASSERT_TRUE(fep3::isOk(bus3.createSystemAccess(fep3::IServiceBus::ISystemAccess::_discover_all_systems,
        ADDR_USE_FOR_TEST)));
    //get this special discovery system name
    auto sys_access_all = bus3.getSystemAccess(fep3::IServiceBus::ISystemAccess::_discover_all_systems);

    auto list_of_discovered_at_discover_all_systems = sys_access_all->discover(std::chrono::seconds(1));
    //if we discover all we can not assure, that on other testsystem or other network nodes are no participant available
    //so we maybe discover also the others, but we make sure, that our test servers are available
    ASSERT_GE(list_of_discovered_at_discover_all_systems.size(), 3);

    ASSERT_TRUE(contains(list_of_discovered_at_discover_all_systems, 
                { std::string(std::string("server_1@") + system_name_for_test_1),
                  std::string(std::string("server_2@") + system_name_for_test_2),
                  std::string(std::string("server_3@") + system_name_for_test_1)
                }));
}

/**
 * @detail Test the discovery methods of the native HTTP System Access and the creation of it
 * This test check if create will lock the creation and changing of the service bus content somehow
 * @req_id FEPSDK-ServiceBus
 *
 */
TEST(ServciceBusServer, testServiceBusLocking)
{
#ifdef WIN32
    constexpr const char* const ADDR_USE_FOR_TEST = "http://230.231.0.0:9993";
#else
    constexpr const char* const ADDR_USE_FOR_TEST = fep3::IServiceBus::ISystemAccess::_use_default_url;
#endif

    //use a service bus 
    fep3::native::ServiceBus* bus1 = new fep3::native::ServiceBus();
    ASSERT_TRUE(fep3::isOk(bus1->create()));
    //this is not possible
    ASSERT_FALSE(fep3::isOk(bus1->createSystemAccess("test_sys",
        ADDR_USE_FOR_TEST)));
    ASSERT_TRUE(fep3::isOk(bus1->destroy()));
    //now it is possible
    ASSERT_TRUE(fep3::isOk(bus1->createSystemAccess("test_sys",
        ADDR_USE_FOR_TEST)));

    ASSERT_TRUE(fep3::isOk(bus1->create()));
    
    //this is still possible
    auto sys_access = bus1->getSystemAccess("test_sys");
    //and i can get the sys access
    ASSERT_TRUE(sys_access);

    //but the creation of createServer within this is locked!
    ASSERT_FALSE(fep3::isOk(sys_access->createServer("test_server",
                                                    fep3::IServiceBus::ISystemAccess::_use_default_url)));
    
    //call the destuctor is still possible also if everythis is locked
    ASSERT_NO_THROW(bus1->~ServiceBus());
}