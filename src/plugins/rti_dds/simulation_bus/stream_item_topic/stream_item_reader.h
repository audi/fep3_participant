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

#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <plugins/rti_dds/simulation_bus/rti_conext_dds_include.h>
#include <plugins/rti_dds/simulation_bus/stream_item_topic/stream_item_topic.h>

std::shared_ptr<fep3::arya::IStreamType> createStreamType(const fep3::ddstypes::StreamType& dds_streamtype, const dds::sub::SampleInfo& sample_info);
std::shared_ptr<fep3::arya::IDataSample> createSample(const fep3::ddstypes::Sample& dds_sample, const dds::sub::SampleInfo& sample_info);

class StreamItemDataReader 
    : public fep3::arya::ISimulationBus::IDataReader
    , public dds::sub::DataReaderListener<fep3::ddstypes::Sample>
{
private:
    std::shared_ptr<StreamItemTopic> _topic;
    std::unique_ptr<dds::sub::DataReader<fep3::ddstypes::Sample>> _sample_reader;
    std::unique_ptr<dds::sub::DataReader<fep3::ddstypes::StreamType>> _streamtype_reader;
    std::unique_ptr<dds::sub::Subscriber> _subscriber;
    dds::core::cond::WaitSet _waitset;
    dds::core::cond::GuardCondition _gurad_condition;
    std::atomic<bool> _running = { true };

public:
    StreamItemDataReader(const std::shared_ptr<StreamItemTopic> & topic
                        , size_t queue_capacity
                        , const std::shared_ptr<dds::core::QosProvider> & qos_provider);
    ~StreamItemDataReader();

    size_t size() const override;
    size_t capacity() const override;
    bool pop(fep3::ISimulationBus::IDataReceiver& receiver) override;

    void receive(fep3::arya::ISimulationBus::IDataReceiver& receiver) override;
    void stop() override;

    fep3::Optional<fep3::Timestamp> getFrontTime() const override;


protected:
    void logError(const fep3::Result& res) const;

protected:
    std::shared_ptr<fep3::ILoggingService::ILogger> _logger;

protected:
    void on_data_available(
        dds::sub::DataReader<fep3::ddstypes::Sample>& reader);

    void on_requested_deadline_missed(
        dds::sub::DataReader<fep3::ddstypes::Sample>& reader,
        const dds::core::status::RequestedDeadlineMissedStatus& status);

    void on_requested_incompatible_qos(
        dds::sub::DataReader<fep3::ddstypes::Sample>& reader,
        const dds::core::status::RequestedIncompatibleQosStatus& status);

    void on_sample_rejected(
        dds::sub::DataReader<fep3::ddstypes::Sample>& reader,
        const dds::core::status::SampleRejectedStatus& status);

    void on_liveliness_changed(
        dds::sub::DataReader<fep3::ddstypes::Sample>& reader,
        const dds::core::status::LivelinessChangedStatus& status);

    void on_subscription_matched(
        dds::sub::DataReader<fep3::ddstypes::Sample>& reader,
        const dds::core::status::SubscriptionMatchedStatus& status);

    void on_sample_lost(
        dds::sub::DataReader<fep3::ddstypes::Sample>& reader,
        const dds::core::status::SampleLostStatus& status);
};
