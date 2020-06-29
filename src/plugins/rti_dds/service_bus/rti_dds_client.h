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

namespace fep3
{
namespace rti_dds
{

class DDSRequester : public fep3::rpc::arya::IRPCRequester
{
    public:
        explicit DDSRequester(const std::string& server_address);
        virtual ~DDSRequester();
        fep3::Result sendRequest(const std::string& service_name,
                                 const std::string& request_message,
                                 fep3::rpc::arya::IRPCRequester::IRPCResponse& response_callback) const override;

    private:
        std::string _server_address;
};

}
}
