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
#include <string>
#include "fep_rpc_iid.h"

namespace fep3
{
namespace rpc
{
namespace arya
{
    /**
        * @brief retrieve rpc interface identifier information from type \p T
        *
        * @tparam T the type that must define an interface identifier via @ref FEP_RPC_IID
        * @return std::string the rpc identifier of the type
        */
    template<class T>
    std::string getRPCIID()
    {
        return T::getRPCIID();
    }

    /**
     * @brief retrieves the default name of this RPC object
     * this will be the default name to register to the RPC Service Registry/RPC Server (seefep3::rpc::IRPCServer)
     *
     * @tparam T  the type that must define an interface identifier via @ref FEP_RPC_IID
     * @return std::string
     */
    template<class T>
    std::string getRPCDefaultName()
    {
        return T::getRPCDefaultName();
    }

    /// Interface of a RPC Client
    class IRPCServiceClient
    {
    public:
        /**
        * DTOR
        */
        virtual ~IRPCServiceClient() {};

    public:
        /**
            * @brief retrieves the RPC ID of this RPC service
            * @retval The ID of the bound rpc server
            */
        virtual std::string getRPCServiceIID() const = 0;

        /**
         * @brief retrieves the default name of this RPC service
         * @retval The default name of the bound rpc server
         */
        virtual std::string getRPCServiceDefaultName() const = 0;

        /**
         * @brief gets the name of the service the ServiceClient belongs to
         * @retval the name of the service
         */
        virtual std::string getRPCServiceName() const = 0;
    };

} //ns arya

using arya::getRPCDefaultName;
using arya::getRPCIID;
}//ns rpc
}//ns fep3


