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
#include <plugins/rti_dds/simulation_bus/rti_conext_dds_include.h>
#include <plugins/rti_dds/simulation_bus/stream_item_topic/stream_item_topic.h>

class StreamItemDataWriter 
    : public fep3::ISimulationBus::IDataWriter
    , public dds::pub::DataWriterListener<fep3::ddstypes::Sample>
{
public:
    StreamItemDataWriter(const std::shared_ptr<StreamItemTopic> & topic
        ,size_t queue_capacity
        ,const std::shared_ptr<dds::core::QosProvider> & qos_provider);
    ~StreamItemDataWriter();

    fep3::Result write(const fep3::IDataSample& data_sample);
    fep3::Result write(const fep3::IStreamType& stream_type);
    fep3::Result transmit();

protected:
    void on_offered_deadline_missed(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer
        ,const dds::core::status::OfferedDeadlineMissedStatus& status) override;

    void on_offered_incompatible_qos(dds::pub::DataWriter<fep3::ddstypes::Sample>& writer
        ,const dds::core::status::OfferedIncompatibleQosStatus& status) override;

    void on_liveliness_lost(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer
        ,const dds::core::status::LivelinessLostStatus& status) override;

    void on_publication_matched(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer
        ,const dds::core::status::PublicationMatchedStatus& status) override;

    void on_reliable_writer_cache_changed(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer,
        const rti::core::status::ReliableWriterCacheChangedStatus& status) override;

    void on_reliable_reader_activity_changed(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer
        ,const rti::core::status::ReliableReaderActivityChangedStatus& status) override;

    void on_instance_replaced(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer
        , const dds::core::InstanceHandle& handle) override;

    void on_application_acknowledgment(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer,
        const rti::pub::AcknowledgmentInfo& acknowledgment_info) override;

    void on_service_request_accepted(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer
        ,const rti::core::status::ServiceRequestAcceptedStatus& status) override;

    void on_destination_unreachable(
        dds::pub::DataWriter<fep3::ddstypes::Sample>&
        ,const dds::core::InstanceHandle&
        ,const rti::core::Locator&) override;

    void* on_data_request(dds::pub::DataWriter<fep3::ddstypes::Sample>&
        ,const rti::core::Cookie&) override;

    void on_data_return(
        dds::pub::DataWriter<fep3::ddstypes::Sample>&
        ,void*
        ,const rti::core::Cookie&) override;

    void on_sample_removed(
        dds::pub::DataWriter<fep3::ddstypes::Sample>& writer
        ,const rti::core::Cookie& cookie) override;

private:
    std::shared_ptr<StreamItemTopic> _topic;
    std::unique_ptr<dds::pub::DataWriter<fep3::ddstypes::StreamType>> _streamtype_writer;
    std::unique_ptr<dds::pub::DataWriter<fep3::ddstypes::Sample>> _sample_writer;

};
