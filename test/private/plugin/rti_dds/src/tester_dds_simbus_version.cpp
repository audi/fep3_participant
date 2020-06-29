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

#include <gtest/gtest.h>
#include <fep3/base/streamtype/default_streamtype.h>
#include <fep3/fep3_participant_version.h>

#include <json/json.h>

#include "detail/test_receiver.hpp"
#include "detail/test_samples.hpp"
#include "detail/test_read_write_test_class.hpp"

using namespace fep3;

Json::Value read_json(const data_read_ptr<const IDataSample> & sample)
{
    JSONCPP_STRING err;
    Json::Value root;

    const Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    RawDataSample data;
    sample->read(data);

    const std::string json = data.getString();
    EXPECT_TRUE(reader->parse(json.c_str(), json.c_str() + json.size(), &root, &err));
    return root;
}

void check_version_info(Json::Value json, const std::string& participant_name, int major, int minor, int patch)
{
    EXPECT_EQ(json["participant_name"], participant_name);
    EXPECT_EQ(json["fep_version"]["major"].asInt(), major);
    EXPECT_EQ(json["fep_version"]["minor"].asInt(), minor);
    EXPECT_EQ(json["fep_version"]["patch"].asInt(), patch);
}

/**
 * @detail We access the internal topic _buildin_topic_businfo to test the discovered information of the simbus
 * @req_id 
 */
TEST_F(ReaderWriterTestClass, TestAvailableInformations)
{  

    // Use the hidden 
    auto simbus_1_businfo_reader =  getSimulationBus()->getReader("_buildin_topic_businfo", StreamTypePlain<uint32_t>());
    ASSERT_TRUE(simbus_1_businfo_reader);

    TestReceiver simbus_1_businfo_reciever;
    simbus_1_businfo_reader->pop(simbus_1_businfo_reciever);

    ASSERT_EQ(simbus_1_businfo_reciever._samples.size(), 1);
    auto json = read_json(simbus_1_businfo_reciever._samples[0]);
    ASSERT_EQ(json.isArray(), true);
    check_version_info(json[0], "simbus_participant_2", 
        FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR, 
        FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR, 
        FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);

    // Add a late joiner
    auto simbus3 = createSimulationBus(GetDomainId(), "simbus_participant_3");
    ASSERT_TRUE(simbus3);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    
    // Read an check information from simbus_1
    simbus_1_businfo_reciever.clear();
    simbus_1_businfo_reader->pop(simbus_1_businfo_reciever);

    ASSERT_EQ(simbus_1_businfo_reciever._samples.size(), 1);
    json = read_json(simbus_1_businfo_reciever._samples[0]);
    ASSERT_EQ(json.isArray(), true);

    check_version_info(json[0], "simbus_participant_2",
        FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);
    check_version_info(json[1], "simbus_participant_3",
        FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);

    // Read an check information from simbus_3 (late joiner)
    auto simbus_3_businfo_reader = dynamic_cast<ISimulationBus*>(simbus3.get())->getReader("_buildin_topic_businfo", StreamTypePlain<uint32_t>());
    TestReceiver simbus_3_businfo_reciever;
    // We have two updates so we take the last
    simbus_3_businfo_reader->pop(simbus_3_businfo_reciever);
    simbus_3_businfo_reader->pop(simbus_3_businfo_reciever);
    ASSERT_EQ(simbus_3_businfo_reciever._samples.size(), 2);
    json = read_json(simbus_3_businfo_reciever._samples[1]);

    // late joiner detected all participants
    check_version_info(json[0], "simbus_participant_1",
        FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);
    check_version_info(json[1], "simbus_participant_2",
        FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR,
        FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH);

    ASSERT_TRUE(fep3::isOk(simbus3->stop()));
    ASSERT_TRUE(fep3::isOk(simbus3->relax()));
    ASSERT_TRUE(fep3::isOk(simbus3->deinitialize()));
}
