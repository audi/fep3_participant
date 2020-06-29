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

#include "detail/test_read_write_test_class.hpp"

#include "detail/test_submitter.hpp"
#include "detail/test_receiver.hpp"
#include "detail/test_read_write_test_class.hpp"

/**
 * @detail Test send and receive of samples with one listener on a different domain
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SendAndReceiveSamplesMultipleDomains)
{
    std::string topic("breadcrumb");

    uint32_t sparrow_domain_id = randomDomainId(); 
    uint32_t sparrow_data_sample_count = 5;
    
    uint32_t blackbird_domain_id = randomDomainId();
    
    /*----------------------------------------------------------------------------
     *  Make sure they have a different domain_id
     *----------------------------------------------------------------------------*/
    while (blackbird_domain_id == sparrow_domain_id)
    {
        blackbird_domain_id = randomDomainId();
    }

    std::cout << "Blackbird Domain ID " << blackbird_domain_id << std::endl;
    std::cout << "Sparrow Domain ID " << sparrow_domain_id << std::endl;

    /*----------------------------------------------------------------------------
     *  create the simulation_buses for the birds
     *----------------------------------------------------------------------------*/
    auto sparrow_simulation_bus = createSimulationBus(sparrow_domain_id, "Sheila");
    auto sparrow_simulation_bus2 = createSimulationBus(sparrow_domain_id, "Scot");
    auto blackbird_simulation_bus = createSimulationBus(blackbird_domain_id, "Brad");

    {
        /*----------------------------------------------------------------------------
         *  add the birds
         *----------------------------------------------------------------------------*/
        TestSubmitter sparrow_submitter(dynamic_cast<ISimulationBus*>(sparrow_simulation_bus.get()),
                                        topic,
                                        fep3::StreamTypePlain<uint32_t>());

        auto sparrow_reader = dynamic_cast<ISimulationBus*>(sparrow_simulation_bus2.get())
                                  ->getReader(topic, fep3::StreamTypePlain<uint32_t>());
        BlockingTestReceiver sparrow_receiver(*sparrow_reader.get());

        auto blackbird_reader = dynamic_cast<ISimulationBus*>(blackbird_simulation_bus.get())
                                    ->getReader(topic, fep3::StreamTypePlain<uint32_t>());
        BlockingTestReceiver blackbird_receiver(*blackbird_reader.get());

        /*----------------------------------------------------------------------------
         *  add sparrow data samples
         *----------------------------------------------------------------------------*/
        for (unsigned int value = 0; value < sparrow_data_sample_count; ++value)
        {
            sparrow_submitter.addDataSample(fep3::DataSampleType<uint32_t>(value));
        }
        sparrow_submitter.submitDataSamples();

        /*----------------------------------------------------------------------------
         *  listen for sparrow samples and make sure blackbird recieved nothing
         *----------------------------------------------------------------------------*/
        sparrow_receiver.waitFor(sparrow_data_sample_count, 1);
        blackbird_receiver.waitFor(0, 0, std::chrono::seconds(1), true, true);
    }  

    TearDownComponent(*blackbird_simulation_bus);
    TearDownComponent(*sparrow_simulation_bus2);
    TearDownComponent(*sparrow_simulation_bus);
}

