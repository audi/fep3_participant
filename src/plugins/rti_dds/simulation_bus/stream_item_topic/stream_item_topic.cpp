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

#include <fep3/base/streamtype/default_streamtype.h>
#include "stream_item_topic.h"
#include "stream_item_reader.h"
#include "stream_item_writer.h"

using namespace fep3;
using namespace dds::domain;

StreamItemTopic::StreamItemTopic(DomainParticipant & participant
    , const std::string & topic_name
    , const IStreamType& stream_type
    , const std::shared_ptr<dds::core::QosProvider> & qos_provider) 
    : _participant(participant)
    , _topic_name(topic_name)
    , _stream_type(stream_type)
    , _qos_provider(qos_provider)
{
    _qos_profile = findQosProfile(stream_type);
    _sample_topic = std::make_unique<dds::topic::Topic<fep3::ddstypes::Sample>>( participant, topic_name );
    _streamtype_topic = std::make_unique<dds::topic::Topic<fep3::ddstypes::StreamType>>( participant, topic_name + "_streamtype" );
}

std::string StreamItemTopic::GetTopic()
{
    return _topic_name;
}

std::string StreamItemTopic::findQosProfile(const fep3::IStreamType& stream_type)
{
    std::string profile = FEP3_QOS_DEFAULT_SAMPLE;

    if (stream_type.getMetaTypeName() == fep3::arya::meta_type_video.getName())
    {
        profile = "fep3::video";
    }
    else if (stream_type.getMetaTypeName() == fep3::arya::meta_type_audio.getName())
    {
        profile = "fep3::audio";
    }
    else if (stream_type.getMetaTypeName() == fep3::arya::meta_type_ddl.getName())
    {
        profile = "fep3::ddl";
    }
    else if (stream_type.getMetaTypeName() == fep3::arya::meta_type_plain.getName())
    {
        profile = "fep3::plain_ctype";
    }
    else if (stream_type.getMetaTypeName() == fep3::arya::meta_type_plain.getName())
    {
        profile = "fep3::plain_ctype_array";
    }
    else if (stream_type.getMetaTypeName() == fep3::arya::meta_type_raw.getName())
    {
        profile = "fep3::raw";
    }

    return profile;
}

std::unique_ptr<ISimulationBus::IDataReader> StreamItemTopic::createDataReader(size_t queue_capacity)
{
    return std::make_unique<StreamItemDataReader>(this->shared_from_this(), queue_capacity, _qos_provider);
}

std::unique_ptr<ISimulationBus::IDataWriter> StreamItemTopic::createDataWriter(size_t queue_capacity)
{
    auto writer = std::make_unique<StreamItemDataWriter>(this->shared_from_this(), queue_capacity, _qos_provider);
    writer->write(_stream_type);
    return writer;
}

dds::domain::DomainParticipant & StreamItemTopic::getDomainParticipant()
{
    return _participant;
}

dds::topic::Topic<fep3::ddstypes::Sample> StreamItemTopic::getSampleTopic()
{
    return *_sample_topic;
}

dds::topic::Topic<fep3::ddstypes::StreamType> StreamItemTopic::getStreamTypeTopic()
{
    return *_streamtype_topic;
}

std::shared_ptr<dds::core::QosProvider> StreamItemTopic::getQosProvider()
{
    return _qos_provider;
}

std::string StreamItemTopic::getQosProfile()
{
    return _qos_profile;
}
