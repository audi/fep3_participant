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

#include <json/json.h>
#include <rti/core/ListenerBinder.hpp>
#include <map>

/*
* Collect and provide bus information via build_in topic from conext dds
*/
class BusInfo
{
public:
    struct Version
    {
        int major = 0;
        int minor = 0;
        int patch = 0;
    };

    class ParticipantInfo
    {
    public:
        ParticipantInfo() = default;
        ParticipantInfo(const std::string & participant_name);
        ~ParticipantInfo() = default;

        bool parse(const std::string & json);
        std::string asJson() const;

        std::string getParticipantName() const;
        Version getFepVersion() const;

        void setParticipantName(const std::string& participant_name);
        void setFepVersion(Version version);

    private:
        std::string _participant_name;
        Version _fep_version;
    };


public:
    BusInfo();
    ~BusInfo();

    std::map<std::string, std::shared_ptr< ParticipantInfo > > getParticipantInfos() const;
    ParticipantInfo* getOwnParticipantInfo() const;

    void registerParticipant(dds::domain::DomainParticipant & participant);
    void unregisterParticipant(dds::domain::DomainParticipant& participant);

    void registerUserData(dds::domain::qos::DomainParticipantQos & qos);

    void setUpdateCallback(const std::function<void()>& callback);

    std::string asJson() const;

    void onUserDataReceived(const std::string& user_data);

private:
    std::map<std::string, std::shared_ptr< ParticipantInfo > > _participant_infos;
    std::unique_ptr<rti::core::ListenerBinder<dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData>>> _listener_binder;
    std::shared_ptr<ParticipantInfo> _own_participant_info;
    std::function<void()> _callback;
};
