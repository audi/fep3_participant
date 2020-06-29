/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef _FEP3_RPC_LOGGING_SERVICE_INTF_DEF_H_
#define _FEP3_RPC_LOGGING_SERVICE_INTF_DEF_H_

//very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3
{
namespace rpc
{
namespace arya
{
    /**
     * @brief Definition of the external service interface of the logging service to configure it
     * @see logging_service.json file
     */
    class IRPCLoggingServiceDef
    {
    protected:
        /**
        * @brief Destroy the IRPCLoggingserviceDef object
        *
        */
        virtual ~IRPCLoggingServiceDef() = default;

    public:
        ///definition of the FEP rpc service iid for the logging service interface
        FEP_RPC_IID("logging_service.arya.fep3.iid", "logging_service");
    };

    /**
     * @brief Definition of the service interface of the RPC logging sink which is provided by the 
     *        RPC Logging Sink of the Logging Service
     * @see logging_rpc_sink_service.json
     */
    class IRPCLoggingSinkServiceDef
    {
    public:
        ///definition of the FEP rpc logging sink service iid for the rpc logging sink
        FEP_RPC_IID("logging_sink_service.arya.fep3.iid", "logging_sink_service");
    };
    /**
     * @brief Definition of the client interface of the RPC logging sink which is used by the
     *        RPC Logging Sink to send log messages to.
     * @see logging_rpc_sink_client.json
     */
    class IRPCLoggingSinkClientDef
    {
    public:
        ///definition of the FEP rpc logging sink client iid for the rpc logging sink
        FEP_RPC_IID("logging_sink_client.arya.fep3.iid", "logging_sink_client");
    };
} // namespace arya
using arya::IRPCLoggingServiceDef;
using arya::IRPCLoggingSinkServiceDef;
using arya::IRPCLoggingSinkClientDef;
} // namespace rpc
} // namespace fep3

#endif //_FEP3_RPC_LOGGING_SERVICE_INTF_DEF_H_