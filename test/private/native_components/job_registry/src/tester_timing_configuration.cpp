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
#include <gmock/gmock.h>

#include <a_util/filesystem/path.h>
#include <common/gtest_asserts.h>

#include <fep3/native_components/job_registry/timing_configuration.h>
#include <fep3/fep3_duration.h>
#include <fep3/core/job.h>

using namespace fep3;
using namespace native;
using namespace timing_configuration;

namespace 
{
constexpr auto config_no_participants = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
    </participants>
</timing>)";
constexpr auto config_participant_without_name = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
        </participant>
    </participants>
</timing>)";
constexpr auto config_empty_job_name_node = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
            <name>Participant</name>
            <jobs>
                <job>
                    <name></name>
                </job>
            </jobs>
        </participant>
    </participants>
</timing>)";
constexpr auto config_job_without_name = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
            <name>Participant</name>
            <jobs>
                <job>
                </job>
            </jobs>
        </participant>
    </participants>
</timing>)";
constexpr auto config_invalid_cycle_time = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
            <name>Participant</name>
            <jobs>
                <job>
                    <name>JobToBeReconfigured</name>
                    <cycle_time>0</cycle_time>
                </job>
            </jobs>
        </participant>
    </participants>
</timing>)";
constexpr auto config_invalid_delay_time = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
            <name>Participant</name>
            <jobs>
                <job>
                    <name>JobToBeReconfigured</name>
                    <cycle_time>10</cycle_time>
                    <cycle_delay_time>-1</cycle_delay_time>
                </job>
            </jobs>
        </participant>
    </participants>
</timing>)";
constexpr auto config_invalid_max_runtime = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
            <name>Participant</name>
            <jobs>
                <job>
                    <name>JobToBeReconfigured</name>
                    <cycle_time>10</cycle_time>
                    <cycle_delay_time>0</cycle_delay_time>
                    <max_run_realtime>-1</max_run_realtime>
                </job>
            </jobs>
        </participant>
    </participants>
</timing>)";
constexpr auto config_invalid_runtime_violation_strategy = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
            <name>Participant</name>
            <jobs>
                <job>
                    <name>JobToBeReconfigured</name>
                    <cycle_time>10</cycle_time>
                    <cycle_delay_time>20</cycle_delay_time>
                    <max_run_realtime>0</max_run_realtime>
                    <run_realtime_violation>unknown</run_realtime_violation>
                </job>
            </jobs>
        </participant>
    </participants>
</timing>)";
constexpr auto config_invalid_input_queue_size = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
            <name>Participant</name>
            <jobs>
                <job>
                    <name>JobToBeReconfigured</name>
                    <cycle_time>10</cycle_time>
                    <cycle_delay_time>20</cycle_delay_time>
                    <max_run_realtime>30</max_run_realtime>
                    <run_realtime_violation>warn_about_runtime_violation</run_realtime_violation>
                    <data_references>
                        <inputs>
                            <input_reference>
                                <name>InputA</name>
                                <queue_size>-1</queue_size>
                            </input_reference>
                        </inputs>
                    </data_references>
                </job>
            </jobs>
        </participant>
    </participants>
</timing>)";
constexpr auto config_invalid_output_queue_size = R"(
<?xml version="1.0" encoding="utf-8"?>
<timing xmlns:timing="fep/xsd/timing">
    <participants>
        <participant>
            <name>Participant</name>
            <jobs>
                <job>
                    <name>JobToBeReconfigured</name>
                    <cycle_time>10</cycle_time>
                    <cycle_delay_time>20</cycle_delay_time>
                    <max_run_realtime>30</max_run_realtime>
                    <run_realtime_violation>warn_about_runtime_violation</run_realtime_violation>
                    <data_references>
                        <inputs>
                            <input_reference>
                                <name>InputA</name>
                                <queue_size>0</queue_size>
                            </input_reference>
                        </inputs>
                        <outputs>
                            <output_reference>
                                <name>OutputA</name>
                                <queue_size>-1</queue_size>
                            </output_reference>
                        </outputs>
                    </data_references>
                </job>
            </jobs>
        </participant>
    </participants>
</timing>)";
}

/**
* @brief A valid timing configuration is parsed correctly.
*
*/
TEST(TimingConfigurationParsing, ValidTimingConfiguration)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_NOERROR(readTimingConfigFromFile(
        a_util::filesystem::Path(TEST_FILES_DIRECTORY).append("valid_timing_configuration.xml"),
        timing_configuration));

    EXPECT_EQ("1.0", timing_configuration._schema_version);
    EXPECT_EQ("fep_team", timing_configuration._header._author);
    EXPECT_EQ("10.10.2017", timing_configuration._header._date_creation);
    EXPECT_EQ("12.10.2017", timing_configuration._header._date_change);
    EXPECT_EQ("Test Timing Configuration", timing_configuration._header._description);

    const auto participant_configuration{ timing_configuration._participants["Participant"] };
    const auto data_job_configuration{ participant_configuration._data_job_configurations.at("my_job") };
    const auto job_configuration{ data_job_configuration._job_configuration };

    EXPECT_EQ(job_configuration._cycle_sim_time, fep3::Duration{ 100000000 });
    EXPECT_EQ(job_configuration._delay_sim_time, fep3::Duration{ 200000000 });
    EXPECT_EQ(job_configuration._max_runtime_real_time.value(), fep3::Duration{ 300000000 });
    EXPECT_EQ(job_configuration._runtime_violation_strategy, fep3::JobConfiguration::TimeViolationStrategy::set_stm_to_error);

    EXPECT_EQ(data_job_configuration._job_input_configurations.at("InputA")._queue_size, 10);
    EXPECT_EQ(data_job_configuration._job_output_configurations.at("OutputA")._queue_size, 10);

    const auto participant_configuration_2{ timing_configuration._participants["Participant2"] };
    const auto data_job_configuration_2{ participant_configuration_2._data_job_configurations.at("my_job2") };
    const auto job_configuration_2{ data_job_configuration_2._job_configuration };

    EXPECT_EQ(job_configuration_2._cycle_sim_time, fep3::Duration{ 400000000 });
    EXPECT_EQ(job_configuration_2._delay_sim_time, fep3::Duration{ 500000000 });
    EXPECT_EQ(job_configuration_2._max_runtime_real_time.value(), fep3::Duration{ 600000000 });
    EXPECT_EQ(job_configuration_2._runtime_violation_strategy, fep3::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation);

    const auto data_job_configuration_3{ participant_configuration_2._data_job_configurations.at("my_job3") };
    const auto job_configuration_3{ data_job_configuration_3._job_configuration };

    EXPECT_EQ(job_configuration_3._cycle_sim_time, fep3::Duration{ 700000000 });
    EXPECT_EQ(job_configuration_3._delay_sim_time, fep3::Duration{ 800000000 });
    EXPECT_EQ(job_configuration_3._max_runtime_real_time.value(), fep3::Duration{ 900000000 });
    EXPECT_EQ(job_configuration_3._runtime_violation_strategy, fep3::JobConfiguration::TimeViolationStrategy::skip_output_publish);

    EXPECT_EQ(data_job_configuration_3._job_input_configurations.at("InputC")._queue_size, 10);
    EXPECT_EQ(data_job_configuration_3._job_input_configurations.at("InputD")._queue_size, 10);
    EXPECT_EQ(data_job_configuration_3._job_output_configurations.at("OutputE")._queue_size, 10);
    EXPECT_EQ(data_job_configuration_3._job_output_configurations.at("OutputF")._queue_size, 10);
}

/**
* @brief Parsing a timing configuration from an invalid path shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorInvalidPath)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromFile("", timing_configuration),
        fep3::ERR_INVALID_FILE,
        ".*No valid file at path.*");
}

/**
* @brief Parsing a timing configuration which does not contain any participant nodes shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigNoParticipants)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_no_participants, timing_configuration),
        fep3::ERR_INVALID_FILE,
        ".*not contain a participant node.");
}

/**
* @brief Parsing a timing configuration containing a participant node which contains no name node shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigParticipantWithoutName)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_participant_without_name, timing_configuration),
        fep3::ERR_INVALID_FILE,
        ".*participant entry without name.");
}

/**
* @brief Parsing a timing configuration specifying a job with an empty name node
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigJobEmptyName)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_empty_job_name_node, timing_configuration),
        fep3::ERR_EMPTY,
        ".*\"name\". Node may not be empty.");
}

/**
* @brief Parsing a timing configuration containing a job node which contains no name node shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigJobWithoutName)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_job_without_name, timing_configuration),
        fep3::ERR_INVALID_FILE,
        ".*Missing job subnode \"name\".");
}

/**
* @brief Parsing a timing configuration specifying an invalid cycle time shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigInvalidCycleTime)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_invalid_cycle_time, timing_configuration),
        fep3::ERR_INVALID_ARG,
        ".*value \"cycle_time\". Value has to be > 0.");
}

/**
* @brief Parsing a timing configuration specifying an invalid delay time shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigInvalidDelayTime)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_invalid_delay_time, timing_configuration),
        fep3::ERR_INVALID_ARG,
        ".*value \"cycle_delay_time\". Value has to be >= 0.");
}

/**
* @brief Parsing a timing configuration specifying an invalid max runtime shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigInvalidMaxRuntime)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_invalid_max_runtime, timing_configuration),
        fep3::ERR_INVALID_ARG,
        ".*value \"max_run_realtime\". Value has to be >= 0.");
}

/**
* @brief Parsing a timing configuration specifying an invalid runtime violation strategy shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigInvalidRuntimeViolationStrategy)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_invalid_runtime_violation_strategy, timing_configuration),
        fep3::ERR_INVALID_ARG,
        ".*value \"run_realtime_violation\".");
}

/**
* @brief Parsing a timing configuration specifying an invalid input queue size shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigInvalidInputQueueSize)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_invalid_input_queue_size, timing_configuration),
        fep3::ERR_INVALID_ARG,
        ".*input node value \"queue_size\". Value has to be >= 0.");
}

/**
* @brief Parsing a timing configuration specifying an invalid output queue size shall return
* the corresponding error.
*
*/
TEST(TimingConfigurationParsing, ErrorConfigInvalidOutputQueueSize)
{
    TimingConfiguration timing_configuration;
    ASSERT_FEP3_RESULT_WITH_MESSAGE(readTimingConfigFromString(config_invalid_output_queue_size, timing_configuration),
        fep3::ERR_INVALID_ARG,
        ".*output node value \"queue_size\". Value has to be >= 0.");
}

struct TimingConfigurationReconfigureInvalid : public ::testing::Test
{
    Jobs _jobs;
    const std::string _participant_name = "TestParticipant";
    TimingConfiguration _timing_configuration;
};

struct TimingConfigurationReconfigureValid : public ::testing::Test
{
    TimingConfigurationReconfigureValid()
    {
        _jobs.insert(std::make_pair(job_name, _job_entry));
        ParticipantConfiguration participant_configuration;
        const JobConfiguration job_configuration{
            Duration{20},
            Duration{20},
            Duration{20},
            JobConfiguration::TimeViolationStrategy::set_stm_to_error };
        participant_configuration._data_job_configurations.insert(std::make_pair(job_name, DataJobConfiguration{ job_configuration }));
        _timing_configuration._participants.insert(std::make_pair(_participant_name, participant_configuration));
    }

    const std::string _participant_name = "TestParticipant", job_name = "TestJob";
    Jobs _jobs;
    JobEntry _job_entry{ std::make_shared <core::arya::Job>(job_name, Duration{10}), JobInfo{job_name, Duration{10}} };
    TimingConfiguration _timing_configuration;
};

/**
* @brief Check whether the timing configurator returns the approriate error if a configured timing configuration
* does not contain a corresponding participant entry.
*
*/
TEST_F(TimingConfigurationReconfigureInvalid, MissingParticipantEntry)
{
    ASSERT_FEP3_RESULT_WITH_MESSAGE(configureJobsByTimingConfiguration(_jobs, _participant_name, _timing_configuration),
        fep3::ERR_NOT_FOUND,
        ".*No participant entry.*");
}

/**
* @brief Check whether the timing configurator returns the approriate error if a configured timing configuration
* does not contain all jobs which are configured for the corresponding participant.
*
*/
TEST_F(TimingConfigurationReconfigureInvalid, MissingJobEntry)
{
    ParticipantConfiguration participant_configuration;
    _timing_configuration._participants.insert(std::make_pair(_participant_name, participant_configuration));
    const std::string job_name = "TestJob";
    JobEntry _job_entry{ std::make_shared <core::arya::Job>(job_name, Duration{10}), JobInfo{job_name, Duration{10}} };
    _jobs.insert(std::make_pair(job_name, _job_entry));

    // actual test case
    {
        ASSERT_FEP3_RESULT_WITH_MESSAGE(configureJobsByTimingConfiguration(_jobs, _participant_name, _timing_configuration),
            fep3::ERR_NOT_FOUND,
            ".*does not contain a job.*");
    }
}

/**
* @brief Check whether the timing configurator returns the approriate error if a configured timing configuration
* contains an invalid job cycle time value.
*
*/
TEST_F(TimingConfigurationReconfigureInvalid, InvalidJobCycleTime)
{
    JobConfiguration job_configuration{ Duration{0} };
    DataJobConfiguration data_job_configuration{job_configuration};
    ParticipantConfiguration participant_configuration;
    const std::string job_name = "TestJob";
    participant_configuration._data_job_configurations.insert(std::make_pair(job_name, data_job_configuration));
    _timing_configuration._participants.insert(std::make_pair(_participant_name, participant_configuration));
    JobEntry _job_entry{ std::make_shared <core::arya::Job>(job_name, Duration{10}), JobInfo{job_name, Duration{10}} };
    _jobs.insert(std::make_pair(job_name, _job_entry));

    // actual test case
    {
        ASSERT_FEP3_RESULT_WITH_MESSAGE(configureJobsByTimingConfiguration(_jobs, _participant_name, _timing_configuration),
            fep3::ERR_INVALID_ARG,
            ".*value. Value has to be > 0.*");
    }
}

/**
* @brief Check whether the timing configurator returns the approriate error if a configured timing configuration
* contains an invalid job delay cycle time value.
*
*/
TEST_F(TimingConfigurationReconfigureInvalid, InvalidJobDelayCycleTime)
{
    JobConfiguration job_configuration{ Duration{1}, Duration{-1} };
    DataJobConfiguration data_job_configuration{ job_configuration };
    ParticipantConfiguration participant_configuration;
    const std::string job_name = "TestJob";
    participant_configuration._data_job_configurations.insert(std::make_pair(job_name, data_job_configuration));
    _timing_configuration._participants.insert(std::make_pair(_participant_name, participant_configuration));
    JobEntry _job_entry{ std::make_shared <core::arya::Job>(job_name, Duration{10}), JobInfo{job_name, Duration{10}} };
    _jobs.insert(std::make_pair(job_name, _job_entry));

    // actual test case
    {
        ASSERT_FEP3_RESULT_WITH_MESSAGE(configureJobsByTimingConfiguration(_jobs, _participant_name, _timing_configuration),
            fep3::ERR_INVALID_ARG,
            ".*value. Value has to be >= 0.*");
    }
}

/**
* @brief Check whether the timing configurator returns the approriate error if a configured timing configuration
* contains an invalid job max run time value.
*
*/
TEST_F(TimingConfigurationReconfigureInvalid, InvalidJobMaxRunTime)
{
    JobConfiguration job_configuration{ Duration{1}, Duration{0},
        Optional <Duration>{Duration{-1}} };
    DataJobConfiguration data_job_configuration{ job_configuration };
    ParticipantConfiguration participant_configuration;
    const std::string job_name = "TestJob";
    participant_configuration._data_job_configurations.insert(std::make_pair(job_name, data_job_configuration));
    _timing_configuration._participants.insert(std::make_pair(_participant_name, participant_configuration));
    JobEntry _job_entry{ std::make_shared <core::arya::Job>(job_name, Duration{10}), JobInfo{job_name, Duration{10}} };
    _jobs.insert(std::make_pair(job_name, _job_entry));

    // actual test case
    {
        ASSERT_FEP3_RESULT_WITH_MESSAGE(configureJobsByTimingConfiguration(_jobs, _participant_name, _timing_configuration),
            fep3::ERR_INVALID_ARG,
            ".*value. Value has to be >= 0.*");
    }
}

/**
* @brief Check whether the timing configurator returns the approriate error if a configured timing configuration
* contains an invalid job violation strategy value.
*
*/
TEST_F(TimingConfigurationReconfigureInvalid, InvalidJobViolationStrategy)
{
    JobConfiguration job_configuration{ Duration{1}, Duration{0},
        Optional <Duration>{Duration{0}}, JobConfiguration::TimeViolationStrategy::unknown };
    DataJobConfiguration data_job_configuration{ job_configuration };
    ParticipantConfiguration participant_configuration;
    const std::string job_name = "TestJob";
    participant_configuration._data_job_configurations.insert(std::make_pair(job_name, data_job_configuration));
    _timing_configuration._participants.insert(std::make_pair(_participant_name, participant_configuration));
    JobEntry _job_entry{ std::make_shared <core::arya::Job>(job_name, Duration{10}), JobInfo{job_name, Duration{10}} };
    _jobs.insert(std::make_pair(job_name, _job_entry));

    // actual test case
    {
        ASSERT_FEP3_RESULT_WITH_MESSAGE(configureJobsByTimingConfiguration(_jobs, _participant_name, _timing_configuration),
            fep3::ERR_INVALID_ARG,
            ".*has to be a valid violation strategy.*");
    }
}

/**
* @brief Check whether set of jobs is successfully reconfigured by the timing configurator.
*
*/
TEST_F(TimingConfigurationReconfigureValid, SuccessfullConfiguration)
{
    ASSERT_FEP3_RESULT(configureJobsByTimingConfiguration(_jobs, _participant_name, _timing_configuration),
        fep3::ERR_NOERROR);

    const auto& job_configuration_to_test = _jobs.at(job_name).job_info.getConfig();
    EXPECT_EQ(job_configuration_to_test._cycle_sim_time, fep3::Duration{ 20 });
    EXPECT_EQ(job_configuration_to_test._delay_sim_time, fep3::Duration{ 20 });
    EXPECT_EQ(job_configuration_to_test._max_runtime_real_time.value(), fep3::Duration{ 20 });
    EXPECT_EQ(job_configuration_to_test._runtime_violation_strategy, fep3::JobConfiguration::TimeViolationStrategy::set_stm_to_error);
}
