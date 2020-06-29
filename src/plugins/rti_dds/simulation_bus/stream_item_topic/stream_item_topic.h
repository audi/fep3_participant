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

#include <plugins/rti_dds/simulation_bus/rti_conext_dds_include.h>
#include <plugins/rti_dds/types/stream_types.hpp>
#include <plugins/rti_dds/simulation_bus/topic_intf.h>

#define FEP3_QOS_STREAM_TYPE "fep3::stream_type"
#define FEP3_QOS_PARTICIPANT "fep3::participant"
#define FEP3_QOS_DEFAULT_SAMPLE "fep3::default_profile"

class StreamItemTopic :
    public std::enable_shared_from_this<StreamItemTopic>,
    public ITopic
{

public:
    StreamItemTopic(dds::domain::DomainParticipant & participant
        , const std::string & topic_name
        , const fep3::IStreamType& stream_type
        , const std::shared_ptr<dds::core::QosProvider> & qos_provider);

    std::string GetTopic();

    std::string findQosProfile(const fep3::IStreamType& stream_type);

    std::unique_ptr<fep3::ISimulationBus::IDataReader> createDataReader(size_t queue_capacity);
    std::unique_ptr<fep3::ISimulationBus::IDataWriter> createDataWriter(size_t queue_capacity);

    dds::domain::DomainParticipant & getDomainParticipant();
    dds::topic::Topic<fep3::ddstypes::Sample> getSampleTopic();
    dds::topic::Topic<fep3::ddstypes::StreamType> getStreamTypeTopic();
    std::shared_ptr<dds::core::QosProvider> getQosProvider();
    std::string getQosProfile();

private:
    dds::domain::DomainParticipant & _participant;
    std::unique_ptr<dds::topic::Topic<fep3::ddstypes::Sample>> _sample_topic;
    std::unique_ptr<dds::topic::Topic<fep3::ddstypes::StreamType>> _streamtype_topic;
    std::string _topic_name;
    std::string _qos_profile;
    fep3::StreamType  _stream_type;
    std::shared_ptr<dds::core::QosProvider> _qos_provider;
};
