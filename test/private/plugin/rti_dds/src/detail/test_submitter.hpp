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

#include <fep3/base/sample/data_sample_intf.h>
#include <fep3/base/streamtype/streamtype_intf.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/base/sample/data_sample.h>
#include <fep3/base/streamtype/default_streamtype.h>

#include <vector>

/**
 * @brief Basic receiver collecting all stream_types and samples
 */
struct TestSubmitter
{
public:
    std::vector<fep3::data_read_ptr<const fep3::IStreamType> > _stream_types;
    std::vector<fep3::data_read_ptr<const fep3::IDataSample> > _samples;
    std::unique_ptr<fep3::ISimulationBus::IDataWriter> _writer;

    TestSubmitter(fep3::ISimulationBus* simulation_bus,
                  std::string topic,
                  fep3::StreamType stream_type)
    {
        _writer = simulation_bus->getWriter(topic, stream_type);
    }

    ~TestSubmitter()
    {
    }

    void addDataSample(const fep3::DataSampleType<uint32_t> data_sample)
    {
        _samples.push_back(std::make_shared<const fep3::DataSampleType<uint32_t> >(data_sample));
    }

    void addStreamType(const fep3::StreamTypeDDL stream_type)
    {
        _stream_types.push_back(std::make_shared<const fep3::StreamTypeDDL>(stream_type));
    }

    void submitStreamTypes()
    {
        for (auto stream_type : _stream_types)
        {
            _writer->write(*stream_type.get());
        }
        _writer->transmit();
        _stream_types.clear();
    }

    void submitDataSamples()
    {
        for (auto data_sample : _samples)
        {
            _writer->write(*data_sample.get());
        }
        _writer->transmit();
        _samples.clear();
    }
};

