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

#include <fep3/base/sample/data_sample.h>
#include <plugins/rti_dds/simulation_bus/converter.h>

#include "stream_item_reader.h"
#include <a_util/result.h>
#include <iostream>

using namespace dds::all;
using namespace fep3;

StreamItemDataReader::StreamItemDataReader(const std::shared_ptr<StreamItemTopic> & topic
    , size_t /*queue_capacity*/
    , const std::shared_ptr<dds::core::QosProvider> & qos_provider)
    : _topic(topic)
{
    _subscriber = std::make_unique<Subscriber>(topic->getDomainParticipant(), qos_provider->subscriber_qos(FEP3_QOS_PARTICIPANT));

    auto qos = qos_provider->datareader_qos(topic->getQosProfile());
    /*
    @TODO make use of capacity limit
    if (queue_capacity > 0)
    {
        qos->resource_limits->max_samples(static_cast<int32_t>(queue_capacity));
    }*/
    _sample_reader = std::make_unique<DataReader<fep3::ddstypes::Sample>>(*_subscriber, topic->getSampleTopic(), qos, this, dds::core::status::StatusMask::none());
    _streamtype_reader = std::make_unique<DataReader<fep3::ddstypes::StreamType>>(*_subscriber, topic->getStreamTypeTopic(), qos_provider->datareader_qos(FEP3_QOS_STREAM_TYPE));

    _waitset += ReadCondition(*_sample_reader, SampleState::not_read());
    _waitset += ReadCondition(*_streamtype_reader, SampleState::not_read());
    _waitset += _gurad_condition;
}


StreamItemDataReader::~StreamItemDataReader()
{
    _waitset = nullptr;
    _gurad_condition = nullptr;
    _sample_reader->close();
    _streamtype_reader->close();
    _subscriber->close();
}

size_t StreamItemDataReader::size() const
{
    try
    {
        return _sample_reader->extensions().datareader_cache_status().sample_count();
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return 0;
}

size_t StreamItemDataReader::capacity() const
{
    try
    {
        return _sample_reader->qos().delegate().history.depth();
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return 0;
}

bool StreamItemDataReader::pop(fep3::ISimulationBus::IDataReceiver& receiver)
{
    try
    {
        if (!_sample_reader->is_nil())
        {
            CoherentAccess coherent_access(*_subscriber);

            std::vector<AnyDataReader> readers;
            int num_readers =
                find(*_subscriber
                    , dds::sub::status::DataState::any()
                    , std::back_inserter(readers));
            
            // readers contains a list of reader, i.e.:
            // _sample_reader
            // _sample_reader
            // _streamtype_reader
            // _sample_reader
            // _sample_reader
            // depending on the recieve order

            if(num_readers > 0)
            {
                
                if (readers[0] == *_sample_reader)
                {
                    for (auto sample : _sample_reader->select()
                        .max_samples(1)
                        .take())
                    {
                       receiver(createSample(sample, sample.info()));
                    }
                }
                else
                {
                    for (auto streamtype : _streamtype_reader->select()
                        .max_samples(1)
                        .take())
                    {
                        receiver(createStreamType(streamtype, streamtype.info()));
                    }
                }
                return true;
            }
        }
    }
    catch (Exception & exception)
    {
        logError(convertExceptionToResult(exception));
    }
    return false;
}

void StreamItemDataReader::receive(fep3::arya::ISimulationBus::IDataReceiver& receiver)
{
    _gurad_condition->trigger_value(false);
    _running = true;

    // Run until ::stop was called
    while (_running)
    {
        // Block until one condition was emited:
        // * ReadCondition(*_sample_reader, SampleState::not_read());
        // * ReadCondition(*_streamtype_reader, SampleState::not_read());
        // * dds::core::cond::GuardCondition;
        // * or timeout 1s
        WaitSet::ConditionSeq conditions = _waitset.wait(Duration(1));
        for (Condition const & condition : conditions)
        {
            if (condition != _gurad_condition)
            {
                while(pop(receiver));
            }
        }
    }
}

void StreamItemDataReader::stop()
{
    _running = false;
    _gurad_condition->trigger_value(true);
}

Optional<Timestamp> StreamItemDataReader::getFrontTime() const
{
    CoherentAccess coherent_access(*_subscriber);

    std::vector<AnyDataReader> readers;
    int num_readers =
        find(*_subscriber
            , dds::sub::status::DataState::new_data()
            , std::back_inserter(readers));

    if (num_readers > 0)
    {
        if (readers[0] == *_sample_reader)
        {
            auto sample = *_sample_reader->select()
                .max_samples(1)
                .read().begin();
            
            return convertTimestamp(sample.info().source_timestamp());
        }
        else
        {
            auto streamtype = *_streamtype_reader->select()
                .max_samples(1)
                .read().begin();
            
            return convertTimestamp(streamtype.info().source_timestamp());
        } 
    }
    return {};
}

void StreamItemDataReader::logError(const fep3::Result& res) const
{
    if (_logger)
    {
        if (_logger->isErrorEnabled())
        {
            _logger->logError(a_util::result::toString(res));
        }
    }
}

void StreamItemDataReader::on_data_available(
    dds::sub::DataReader<fep3::ddstypes::Sample>& /*reader*/)
{
}

void StreamItemDataReader::on_requested_deadline_missed(
    dds::sub::DataReader<fep3::ddstypes::Sample>& /*reader*/,
    const dds::core::status::RequestedDeadlineMissedStatus& /*status*/)
{
    //std::cout << "           on_requested_deadline_missed" << std::endl;
}

void StreamItemDataReader::on_requested_incompatible_qos(
    dds::sub::DataReader<fep3::ddstypes::Sample>& /*reader*/,
    const dds::core::status::RequestedIncompatibleQosStatus& /*status*/)
{
    //std::cout << "           on_requested_incompatible_qos" << std::endl;
}

void StreamItemDataReader::on_sample_rejected(
    dds::sub::DataReader<fep3::ddstypes::Sample>& /*reader*/,
    const dds::core::status::SampleRejectedStatus& /*status*/)
{
    //std::cout << "           on_requested_incompatible_qos" << std::endl;
}

void StreamItemDataReader::on_liveliness_changed(
    dds::sub::DataReader<fep3::ddstypes::Sample>& /*reader*/,
    const dds::core::status::LivelinessChangedStatus& /*status*/)
{
    //std::cout << "           on_requested_incompatible_qos" << std::endl;
}

void StreamItemDataReader::on_subscription_matched(
    dds::sub::DataReader<fep3::ddstypes::Sample>& /*reader*/,
    const dds::core::status::SubscriptionMatchedStatus& /*status*/)
{
    //std::cout << "           on_subscription_matched" << std::endl;
}

void StreamItemDataReader::on_sample_lost(
    dds::sub::DataReader<fep3::ddstypes::Sample>& /*reader*/,
    const dds::core::status::SampleLostStatus& /*status*/)
{
    //std::cout << "           on_sample_lost" << std::endl;
}

std::shared_ptr<IStreamType> createStreamType(const fep3::ddstypes::StreamType& dds_streamtype, const dds::sub::SampleInfo& /*sample_info*/)
{
    auto streamtype = std::make_shared<fep3::StreamType>(dds_streamtype.metatype());
    for (auto dds_property : dds_streamtype.properties())
    {
        streamtype->setProperty(dds_property.name(), dds_property.value(), dds_property.type());
    }
    return streamtype;
}

std::shared_ptr<IDataSample> createSample(const fep3::ddstypes::Sample& dds_sample, const dds::sub::SampleInfo& sample_info)
{
    auto sample = std::make_shared<DataSample>();
    sample->set(dds_sample.data().data(), dds_sample.data().size());
    sample->setTime(convertTimestamp(sample_info.source_timestamp()));
    sample->setCounter(static_cast<uint32_t>(sample_info.extensions().publication_sequence_number().value()));
    return sample;
}
