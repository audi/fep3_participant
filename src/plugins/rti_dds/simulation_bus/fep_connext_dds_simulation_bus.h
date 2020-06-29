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

#include <fep3/components/base/component_base.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <plugins/rti_dds/simulation_bus/rti_conext_dds_include.h>
#include <fep3/components/configuration/propertynode.h>

using namespace fep3::arya;

/**
* Implements a simulation bus based on the Conext DDS implementation of the Data Distribution Service (DDS) standard from RTI.
* 
* The Data Distribution Service (DDS) for real-time systems a middleware standard that aims high performance, interoperable, 
* real-time, scalable data exchange.
* 
* This implementation requires a predefined USER_QOS_PROFILES.xml. In the QOS Profiles you can define your demands on QOS for
* each streamtype or topic. Please read the documentation of RTI on https://community.rti.com/.
* The USER_QOS_PROFILES.xml need to be located beside the fep3_connext_dds_plugin, your application or you can use the
* environment variable NDDS_QOS_PROFILES (a list of ';' seperated paths).
* 
*/
class ConnextDDSSimulationBus : public fep3::ComponentBase<fep3::arya::ISimulationBus>
{
    public:
        ConnextDDSSimulationBus();
        ~ConnextDDSSimulationBus();
        ConnextDDSSimulationBus(const ConnextDDSSimulationBus&) = delete;
        ConnextDDSSimulationBus(ConnextDDSSimulationBus&&) = delete;
        ConnextDDSSimulationBus& operator=(const ConnextDDSSimulationBus&) = delete;
        ConnextDDSSimulationBus& operator=(ConnextDDSSimulationBus&&) = delete;

    public: //the ComponentBase statemachine
        fep3::Result create() override;
        fep3::Result destroy() override;
        fep3::Result initialize() override;
        fep3::Result deinitialize() override;

    public: //the arya SimulationBus interface
        bool isSupported(const IStreamType& stream_type) const override;

        std::unique_ptr<IDataReader> getReader
            (const std::string& name
            , const IStreamType& stream_type
            ) override;

        std::unique_ptr<IDataReader> getReader
            (const std::string& name
            , const IStreamType& stream_type
            , size_t queue_capacity
            ) override;

        std::unique_ptr<IDataReader> getReader(const std::string& name) override;
        std::unique_ptr<IDataReader> getReader(const std::string& name, size_t queue_capacity) override;
        std::unique_ptr<IDataWriter> getWriter
            (const std::string& name
            , const IStreamType& stream_type
            ) override;
        std::unique_ptr<IDataWriter> getWriter
            (const std::string& name
            , const IStreamType& stream_type
            , size_t queue_capacity
            ) override;
        std::unique_ptr<IDataWriter> getWriter(const std::string& name) override;
        std::unique_ptr<IDataWriter> getWriter(const std::string& name, size_t queue_capacity) override;

    public:
        std::shared_ptr<dds::core::QosProvider> getQOSProfile() const;

    private:
        class ConnextDDSSimulationBusConfiguration : public Configuration
        {
        public:
            ConnextDDSSimulationBusConfiguration();
            ~ConnextDDSSimulationBusConfiguration() = default;

        public:
            fep3::Result registerPropertyVariables() override;
            fep3::Result unregisterPropertyVariables() override;
            
        public:
            PropertyVariable<int32_t>     _participant_domain{ 5 };
            PropertyVariable<std::string> _participant_name{ "default_participant_name" };
        };

    private:
        class Impl;
        std::unique_ptr<Impl> _impl;
        std::shared_ptr<fep3::ILoggingService::ILogger> _logger;
        void logError(const fep3::Result& res);

        ConnextDDSSimulationBusConfiguration _simulation_bus_configuration;
};
