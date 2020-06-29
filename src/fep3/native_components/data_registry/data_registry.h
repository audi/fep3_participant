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

#pragma once

#include <cstddef>
#include <list>
#include <memory>
#include <unordered_map>

#include <a_util/result.h>

#include "fep3/base/streamtype/default_streamtype.h"
#include "fep3/base/streamtype/streamtype_intf.h"
#include "fep3/components/base/component_base.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/rpc_services/data_registry/data_registry_service_stub.h"
#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include "fep3/rpc_services/data_registry/data_registry_rpc_intf_def.h"

namespace fep3
{
namespace native
{
namespace arya
{
class DataRegistry;

class RPCDataRegistryService : public rpc::RPCService<fep3::rpc_stubs::RPCDataRegistryServiceStub, fep3::rpc::IRPCDataRegistryDef>
{
public:
    explicit RPCDataRegistryService(DataRegistry& data_registry) : _data_registry(data_registry) {}

public:
    std::string getSignalInNames() override;
    std::string getSignalOutNames() override;
    Json::Value getStreamType(const std::string& signal_name) override;

private:
    DataRegistry& _data_registry;
};

/**
 * Native implementation of the data registry. Manages an internal list of
 * input and output signals which will be registered to the simulation bus
 * all at once during initialization.
 *
 * This class also provides getter functions for readers and writers to these signals.
 */
class DataRegistry : public ComponentBase<IDataRegistry>
{
public:
    DataRegistry();
    fep3::Result tense() override;
    fep3::Result relax() override;

protected:
    fep3::Result create() override;

public:
    fep3::Result registerDataIn(const std::string& name,
                                const IStreamType& type,
                                bool is_dynamic_meta_type=false) override;
    fep3::Result registerDataOut(const std::string& name,
                                 const IStreamType& type,
                                 bool is_dynamic_meta_type=false) override;
    fep3::Result unregisterDataIn(const std::string& name) override;
    fep3::Result unregisterDataOut(const std::string& name) override;

    fep3::Result registerDataReceiveListener(const std::string& name,
        const std::shared_ptr<IDataReceiver>& listener) override;
    fep3::Result unregisterDataReceiveListener(const std::string& name,
        const std::shared_ptr<IDataReceiver>& listener) override;

    std::unique_ptr<IDataRegistry::IDataReader> getReader(const std::string& name) override;
    std::unique_ptr<IDataRegistry::IDataReader> getReader(const std::string& name,
        size_t queue_capacity) override;
    std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const std::string& name) override;
    std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const std::string& name,
        size_t queue_capacity) override;

public:
    std::vector<std::string> getSignalInNames();
    std::vector<std::string> getSignalOutNames();
    StreamType getStreamType(const std::string& name);

private:
    class DataSignal;
    class DataSignalIn;
    class DataSignalOut;

    class DataReader;
    class DataWriter;
    class DataReaderProxy;
    class DataWriterProxy;

    fep3::Result addDataIn(const std::string& name,
        const IStreamType& type,
        bool is_dynamic_meta_type);
    fep3::Result addDataOut(const std::string& name,
        const IStreamType& type,
        bool is_dynamic_meta_type);
    DataSignalIn* getDataIn(const std::string& name);
    DataSignalOut* getDataOut(const std::string& name);
    bool removeDataIn(const std::string& name);
    bool removeDataOut(const std::string& name);

    std::unordered_map<std::string, std::shared_ptr<DataSignalIn>> _ins{};
    std::unordered_map<std::string, std::shared_ptr<DataSignalOut>> _outs{};
    std::shared_ptr<rpc::IRPCServer::IRPCService> _rpc_service{ nullptr };
};
} // namespace arya
using arya::RPCDataRegistryService;
using arya::DataRegistry;
} // namespace native
} // namespace fep3