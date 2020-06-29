/**
*
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
*/

#pragma once

#include <gmock/gmock.h>

#include <fep3/native_components/clock_sync/interpolation_time.h>
#include <fep3/rpc_services/clock_sync/clock_sync_master_service_stub.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>

namespace fep3
{
namespace mock 
{

struct InterpolationTime : public fep3::IInterpolationTime
{
    MOCK_CONST_METHOD0(getTime, Timestamp());
    MOCK_METHOD2(setTime, void(Timestamp, Duration));
    MOCK_METHOD1(resetTime, void(Timestamp));
};

struct RPCClockSyncMaster : public fep3::rpc::arya::RPCService<fep3::rpc_stubs::RPCClockSyncMasterServiceStub, fep3::rpc::arya::IRPCClockSyncMasterDef>
{
    MOCK_METHOD2(registerSyncSlave, int(int, const std::string&));
    MOCK_METHOD1(unregisterSyncSlave, int(const std::string&));
    MOCK_METHOD2(slaveSyncedEvent, int(const std::string&, const std::string&));
    MOCK_METHOD0(getMasterTime, std::string());
    MOCK_METHOD0(getMasterType, int());
};

}
} 
