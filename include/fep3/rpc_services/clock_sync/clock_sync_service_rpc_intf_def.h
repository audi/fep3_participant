/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
*/

#ifndef _FEP3_RPC_CLOCK_SYNC_SERVICE_INTF_DEF_H_
#define _FEP3_RPC_CLOCK_SYNC_SERVICE_INTF_DEF_H_

//very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3
{
namespace rpc
{
namespace arya
{
    /**
     * @brief definition of the external service interface of the clock service as clock master
     * @see delivered clock_sync_master.json file
     */
    class IRPCClockSyncMasterDef
    {
    public:
        /// definition of the rpc propagated time events
        enum class EventID : uint8_t
        {
            ///time update before
            ///@see fep3::arya::IClock::IEventSink::timeUpdateBegin
            time_update_before = 1,
            ///time updating
            ///@see fep3::arya::IClock::IEventSink::timeUpdating
            time_updating = 2,
            ///time update after
            ///@see fep3::arya::IClock::IEventSink::timeUpdateEnd
            time_update_after = 3,
            ///time reset
            ///@see fep3::arya::IClock::IEventSink::timeResetEnd
            time_reset = 4
        };
        /// definition of the rpc propagated time events registration
        /// by default only register_for_time_updating and register_for_time_reset is used.
        enum class EventIDFlag : uint8_t
        {
            /// register to get a IRPCClockSyncMasterDef::EventID::time_update_before event
            register_for_time_update_before = 0x01,
            /// register to get a IRPCClockSyncMasterDef::EventID::time_updating event
            register_for_time_updating = 0x02,
            /// register to get a IRPCClockSyncMasterDef::EventID::time_update_after event
            register_for_time_update_after = 0x04,
            /// register to get a IRPCClockSyncMasterDef::EventID::time_reset event
            register_for_time_reset = 0x08
        };
    public:
        ///definition of the FEP rpc service iid for a clock synchronization master
        FEP_RPC_IID("clock_sync_master.arya.fep3.iid", "clock_sync_master");
    };

    /**
     * @brief definition of the external service interface of the clock synchronisation service as clock slave
     * @see delivered clock_sync_slave.json file
     */
    class IRPCClockSyncSlaveDef
    {
    public:
        ///definiton of the FEP rpc service iid for a clock synchronization slave
        FEP_RPC_IID("clock_sync_slave.arya.fep3.iid", "clock_sync_slave");
    };
} // namespace arya
using arya::IRPCClockSyncMasterDef;
using arya::IRPCClockSyncSlaveDef;
} // namespace rpc
} // namespace fep3

#endif // _FEP3_RPC_CLOCK_SYNC_SERVICE_INTF_DEF_H_