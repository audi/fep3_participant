/**
 * @file
 * @copyright AUDI AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */
#pragma once
#include "./../service_bus.h"

namespace fep3
{
namespace native
{
namespace testing
{

static constexpr const char* test_participant_name = "test_participant_name";
static constexpr const char* test_participant_url = "http://localhost:9090";
inline bool prepareServiceBusForTestingDefault(ServiceBus& service_bus,
    const std::string& test_participant_name_default = test_participant_name,
    const std::string& test_participant_url_default = test_participant_url)
{
    auto res = service_bus.createSystemAccess("test_with_service_bus_default", "", true);
    if (fep3::isOk(res))
    {
        auto sysaccess = service_bus.getSystemAccess("test_with_service_bus_default");
        res = sysaccess->createServer(test_participant_name_default, test_participant_url_default);
        if (fep3::isOk(res))
        {
            return true;
        }
    }
    return false;
}

}
}
}