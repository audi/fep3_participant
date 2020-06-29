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

#include <fep3/components/service_bus/rpc/rpc_intf.h>

#pragma warning( push )
#pragma warning( disable : 4290)
#include <rpc_pkg.h>
#pragma warning( pop ) 

namespace fep3
{
namespace native
{

class HttpClientConnector : public rpc::arya::IRPCRequester
{
    public:
        explicit HttpClientConnector(const std::string& server_address);
        virtual ~HttpClientConnector();
        fep3::Result sendRequest(const std::string& service_name,
                                 const std::string& request_message,
                                 IRPCRequester::IRPCResponse& response_callback) const override;
    private:
        std::string _server_address;
};

}
}
