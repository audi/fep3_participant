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
#include <fep3/base/streamtype/default_streamtype.h>

#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/native_components/configuration/configuration_service.h>

#include <fep3/participant/component_factories/cpp/component_factory_cpp_plugins.h>

#include <gtest/gtest.h>
#include <random>
#include <iostream>

using namespace fep3;

class ReaderWriterTestClass 
    : public testing::Test
{
protected:

    virtual void SetUp()
    {
        if (!_simulation_bus)
        {
            std::vector<std::string> plugins = { FEP3_RTI_DDS_HTTP_SERVICE_BUS_SHARED_LIB };

            ASSERT_NO_THROW
            (
                _factory = std::make_unique<arya::ComponentFactoryCPPPlugin>(plugins);
            );

            _domain_id = randomDomainId();
            std::cout << "Domain ID " << _domain_id << std::endl;
            
            _simulation_bus = createSimulationBus(_domain_id, "simbus_participant_1");
            _simulation_bus_2 = createSimulationBus(_domain_id, "simbus_participant_2");
        }

        std::string topic = findFreeTopic();
        _writer = getSimulationBus2()->getWriter(topic, StreamTypePlain<uint32_t>());
        _reader = getSimulationBus()->getReader(topic, StreamTypePlain<uint32_t>());

        ASSERT_TRUE(_writer);
        ASSERT_TRUE(_reader);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::unique_ptr<IComponent> createSimulationBus(uint32_t domain_id, std::string participant_name)
    {
        auto simulation_bus = _factory->createComponent(ISimulationBus::getComponentIID());
        if (!simulation_bus) 
        {
            return nullptr;
        }

        std::shared_ptr<Components> components = std::make_shared<Components>();
        simulation_bus->createComponent(components);

        auto property_node = components->_configuration_service->getNode("rti_dds_simulation_bus");
        if (!property_node) 
        {
            return nullptr;
        }

        if (auto property = std::dynamic_pointer_cast<fep3::arya::IPropertyWithExtendedAccess>(property_node->getChild("participant_domain")))
        {
            property->setValue(std::to_string(domain_id));
            property->updateObservers();
        }

        if (auto pProperty = std::dynamic_pointer_cast<fep3::arya::IPropertyWithExtendedAccess>(property_node->getChild("participant_name")))
        {
            pProperty->setValue(participant_name);
            pProperty->updateObservers();
        }

        EXPECT_EQ(fep3::Result(), simulation_bus->initialize());
        EXPECT_EQ(fep3::Result(), simulation_bus->tense());
        EXPECT_EQ(fep3::Result(), simulation_bus->start());

        return simulation_bus;
    }

    uint32_t randomDomainId()
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> dist(1.0, 200.0);

        return static_cast<unsigned int>(dist(mt)) % 200;
    }

    void TearDownComponent(IComponent & component)
    {
        EXPECT_EQ(fep3::Result(), component.stop());
        EXPECT_EQ(fep3::Result(), component.relax());
        EXPECT_EQ(fep3::Result(), component.deinitialize());
    }

    void TearDown()
    {
        _writer.reset();
        _reader.reset();

        if (_simulation_bus)
        {
            TearDownComponent(*_simulation_bus);
            _simulation_bus.reset();
        }

        if (_simulation_bus_2)
        {
            TearDownComponent(*_simulation_bus_2);
            _simulation_bus_2.reset();
        }
    }

    ISimulationBus* getSimulationBus()
    {
        return dynamic_cast<ISimulationBus*>(_simulation_bus.get());
    }
    ISimulationBus* getSimulationBus2()
    {
        return dynamic_cast<ISimulationBus*>(_simulation_bus_2.get());
    }

    std::string findFreeTopic()
    {
        return std::string("test_") + std::to_string(1);
    }

    class Components: public IComponents 
    {
    public:
        std::unique_ptr<fep3::native::ConfigurationService> _configuration_service = std::make_unique<fep3::native::ConfigurationService>();

    public:
        Components()
        {
            _configuration_service->create();
            _configuration_service->initialize();
            _configuration_service->tense();
            _configuration_service->start();
        }

        IComponent* findComponent(const std::string& fep_iid) const
        {
            if (fep_iid == IConfigurationService::getComponentIID())
            {
                return _configuration_service.get();
            }
            return nullptr;
        }
    };
   
    uint32_t GetDomainId()
    {
        return _domain_id;
    }

protected:
    std::unique_ptr<ISimulationBus::IDataWriter> _writer;
    std::unique_ptr<ISimulationBus::IDataReader> _reader;

    std::unique_ptr<IComponent> _simulation_bus;
    std::unique_ptr<IComponent> _simulation_bus_2;

    std::unique_ptr<arya::ComponentFactoryCPPPlugin> _factory;

    uint32_t _domain_id;
};

