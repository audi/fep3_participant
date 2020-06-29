/**
 *
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include "timing_configuration.h"

#include <a_util/xml/dom.h>
#include "a_util/filesystem.h"

using namespace a_util::xml;

namespace 
{

using namespace fep3;
using namespace native;
using namespace timing_configuration;

constexpr auto timing_cfg_node_path_schema_version= "/timing/schema_version";
constexpr auto timing_cfg_node_path_header = "/timing/header";
constexpr auto timing_cfg_node_author = "author";
constexpr auto timing_cfg_node_date_creation = "date_creation";
constexpr auto timing_cfg_node_date_change = "date_change";
constexpr auto timing_cfg_node_description = "description";
constexpr auto timing_cfg_node_path_participant = "/timing/participants/participant";
constexpr auto timing_cfg_node_path_job = "jobs/job";
constexpr auto timing_cfg_node_path_input = "data_references/inputs/input_reference";
constexpr auto timing_cfg_node_path_output = "data_references/outputs/output_reference";
constexpr auto timing_cfg_node_name = "name";
constexpr auto timing_cfg_node_job_cycle_time = "cycle_time";
constexpr auto timing_cfg_node_job_cycle_delay_time = "cycle_delay_time";
constexpr auto timing_cfg_node_job_max_run_realtime = "max_run_realtime";
constexpr auto timing_cfg_node_job_run_realtime_violation = "run_realtime_violation";
constexpr auto timing_cfg_node_input_queue_size = "queue_size";

std::string err_msg_node_missing{ "Invalid timing configuration. Missing %s subnode \"%s\"." };
std::string err_msg_node_empty{ "Invalid timing configuration. Invalid %s node value \"%s\". Node may not be empty." };
std::string err_msg_node_invalid{ "Invalid timing configuration. Invalid %s node value \"%s\". %s." };


Result reconfigureJobInfoByJobConfiguration(JobInfo& job_info,
    const JobConfiguration& job_configuration)
{
    using namespace std::chrono;

    JobConfiguration config(Duration{ duration_cast<nanoseconds>(milliseconds{100}) });
    if (job_configuration._cycle_sim_time.count() <= 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format("Invalid timing configuration. Invalid %s node value. Value has to be > 0.",
                timing_cfg_node_job_cycle_time).c_str());
    }
    config._cycle_sim_time = job_configuration._cycle_sim_time;

    if (job_configuration._delay_sim_time.count() < 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format("Invalid timing configuration. Invalid %s node value. Value has to be >= 0.",
                timing_cfg_node_job_cycle_delay_time).c_str());
    }
    config._delay_sim_time = job_configuration._delay_sim_time;

    if (job_configuration._max_runtime_real_time.value().count() < 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format("Invalid timing configuration. Invalid %s node value. Value has to be >= 0.",
                timing_cfg_node_job_max_run_realtime).c_str());
    }
    else if (job_configuration._max_runtime_real_time.value().count() == 0)
    {
        config._max_runtime_real_time = Optional<Duration>{};
    }
    else
    {
        config._max_runtime_real_time = job_configuration._max_runtime_real_time;
    }

    if (JobConfiguration::TimeViolationStrategy::unknown == job_configuration._runtime_violation_strategy)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format("Invalid timing configuration. Invalid %s node value. Value has to be a valid violation strategy.",
                timing_cfg_node_job_run_realtime_violation).c_str());
    }
    config._runtime_violation_strategy = job_configuration._runtime_violation_strategy;

    FEP3_RETURN_IF_FAILED(job_info.reconfigure(config));

    return {};
}

Result getChildData(const DOMElement& dom_element,
    const std::string& child_element_name,
    const std::string& parent_element_type,
    std::string& child_element_value,
    const std::string& err_msg_missing_child_node,
    const std::string& err_msg_empty_child_node)
{
    const auto child_element = dom_element.getChild(child_element_name);

    if (child_element.isNull())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_FILE, a_util::strings::format(
            err_msg_missing_child_node.c_str(), parent_element_type.c_str(),
            child_element_name.c_str()).c_str());
    }

    child_element_value = child_element.getData();

    if (child_element_value.empty())
    {
        RETURN_ERROR_DESCRIPTION(ERR_EMPTY, a_util::strings::format(
            err_msg_empty_child_node.c_str(), parent_element_type.c_str(),
            child_element_name.c_str()).c_str());
    }

    return {};
}

Result convertToDurationIfValidValue(const std::string& string_value,
    const std::function<bool(Duration)>& validity_check, Duration& duration_value, const std::string& error_message)
{
    duration_value = Duration{ std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::microseconds(a_util::strings::toInt64(string_value))) };

    if (!validity_check(duration_value))
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, error_message.c_str());
    }

    return {};
}

Result convertToIntegerIfValidValue(const std::string& string_value,
    const std::function<bool(int)>& validity_check, int& int_value, const std::string& error_message)
{
    int_value = a_util::strings::toInt32(string_value);

    if (!validity_check(int_value))
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, error_message.c_str());
    }

    return {};
}

Result parseNameNode(const DOMElement& dom_element,
    const std::string& child_element_name,
    std::string& child_element_value)
{
    FEP3_RETURN_IF_FAILED(getChildData(dom_element, child_element_name, "job", child_element_value,
        err_msg_node_missing, err_msg_node_empty));

    return {};
}

Result parseNodeDurationValue(const DOMElement& dom_element, const std::string& child_element_name,
    Duration& configuration_value,
    const std::function<bool(Duration)>& validity_check, const std::string& err_invalid_value_msg)
{
    std::string tmp_value;

    FEP3_RETURN_IF_FAILED(getChildData(dom_element, child_element_name, "job", tmp_value,
        err_msg_node_missing, err_msg_node_empty));
    FEP3_RETURN_IF_FAILED(convertToDurationIfValidValue(tmp_value, validity_check,
        configuration_value, a_util::strings::format(err_msg_node_invalid.c_str(), "job", child_element_name.c_str(), err_invalid_value_msg.c_str())));

    return {};
}

Result parseCycleTimeNode(const DOMElement& dom_element, const std::string& child_element_name,
    Duration& configuration_value)
{
    FEP3_RETURN_IF_FAILED(parseNodeDurationValue(dom_element, child_element_name, configuration_value,
        [](const Duration duration) {return duration.count() > 0; }, "Value has to be > 0"));

    return {};
}

Result parseDelayTimeNode(const DOMElement& dom_element, const std::string& child_element_name,
    Duration& configuration_value)
{
    FEP3_RETURN_IF_FAILED(parseNodeDurationValue(dom_element, child_element_name, configuration_value,
        [](const Duration duration) {return duration.count() >= 0; }, "Value has to be >= 0"));

    return {};
}

Result parseMaxRunTimeNode(const DOMElement& dom_element, const std::string& child_element_name,
    Duration& configuration_value)
{
    FEP3_RETURN_IF_FAILED(parseNodeDurationValue(dom_element, child_element_name, configuration_value,
        [](const Duration duration) {return duration.count() >= 0; }, "Value has to be >= 0"));

    return {};
}

Result parseRunTimeViolationStrategyNode(const DOMElement& dom_element, const std::string& child_element_name,
    JobConfiguration::TimeViolationStrategy& configuration_value)
{
    std::string tmp_value;

    FEP3_RETURN_IF_FAILED(getChildData(dom_element, child_element_name, "job", tmp_value, err_msg_node_missing, err_msg_node_empty));
    configuration_value = JobConfiguration::timeViolationStrategyFromString(tmp_value);
    if (configuration_value == JobConfiguration::TimeViolationStrategy::unknown)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, a_util::strings::format("Invalid timing configuration. Invalid job node value \"%s\".",
            timing_cfg_node_job_run_realtime_violation).c_str());
    }

    return {};
}

Result readInputInformationFromJobDOMElement(const DOMElement& job_dom_element,
    DataJobConfiguration& data_job_configuration)
{
    DOMElementList inputs_element_list;
    if (job_dom_element.findNodes(timing_cfg_node_path_input, inputs_element_list))
    {
        std::string input_name;
        std::string element_value;
        int queue_size;
        for (const auto& input_element : inputs_element_list)
        {
            FEP3_RETURN_IF_FAILED(getChildData(input_element, timing_cfg_node_name, "input", input_name, err_msg_node_missing, err_msg_node_empty));

            FEP3_RETURN_IF_FAILED(getChildData(input_element, timing_cfg_node_input_queue_size, "input", element_value, err_msg_node_missing, err_msg_node_empty));
            FEP3_RETURN_IF_FAILED(convertToIntegerIfValidValue(element_value, [](const int value) {return value >= 0; },
                queue_size, a_util::strings::format(err_msg_node_invalid.c_str(), "input", timing_cfg_node_input_queue_size, "Value has to be >= 0")));

            data_job_configuration._job_input_configurations.insert(std::make_pair(input_name, JobInputConfiguration{ queue_size }));
        }
    }

    return {};
}

Result readOutputInformationFromJobDOMElement(const DOMElement& job_dom_element,
    DataJobConfiguration& data_job_configuration)
{
    DOMElementList outputs_element_list;
    if (job_dom_element.findNodes(timing_cfg_node_path_output, outputs_element_list))
    {
        std::string output_name;
        std::string element_value;
        int queue_size;
        for (const auto& output_element : outputs_element_list)
        {
            FEP3_RETURN_IF_FAILED(getChildData(output_element, timing_cfg_node_name, "output", output_name, err_msg_node_missing, err_msg_node_empty));

            FEP3_RETURN_IF_FAILED(getChildData(output_element, timing_cfg_node_input_queue_size, "output", element_value, err_msg_node_missing, err_msg_node_empty));
            FEP3_RETURN_IF_FAILED(convertToIntegerIfValidValue(element_value, [](const int value) {return value >= 0; },
                queue_size, a_util::strings::format(err_msg_node_invalid.c_str(), "output", timing_cfg_node_input_queue_size, "Value has to be >= 0")));

            data_job_configuration._job_output_configurations.insert(std::make_pair(output_name, JobOutputConfiguration{ queue_size }));
        }
    }

    return {};
}

Result readJobInformationFromDOM(const DOMElement& participant_element,
    ParticipantConfiguration& participant_config)
{
    using namespace a_util::strings;
    using namespace std::chrono_literals;
    using namespace std::chrono;

    DOMElementList jobs_element_list;
    if (participant_element.findNodes(timing_cfg_node_path_job, jobs_element_list))
    {
        JobConfiguration job_configuration(Duration{ duration_cast<nanoseconds>(100ms) });
        std::string job_name;

        for (const auto& job_element : jobs_element_list)
        {
            FEP3_RETURN_IF_FAILED(parseNameNode(job_element, timing_cfg_node_name, job_name));

            FEP3_RETURN_IF_FAILED(parseCycleTimeNode(job_element, timing_cfg_node_job_cycle_time,
                job_configuration._cycle_sim_time));

            FEP3_RETURN_IF_FAILED(parseDelayTimeNode(job_element, timing_cfg_node_job_cycle_delay_time,
                job_configuration._delay_sim_time));

            job_configuration._max_runtime_real_time = Duration{};
            FEP3_RETURN_IF_FAILED(parseMaxRunTimeNode(job_element, timing_cfg_node_job_max_run_realtime,
                job_configuration._max_runtime_real_time.value()));

            FEP3_RETURN_IF_FAILED(parseRunTimeViolationStrategyNode(job_element, timing_cfg_node_job_run_realtime_violation,
                job_configuration._runtime_violation_strategy));

            DataJobConfiguration data_job_configuration{ job_configuration };

            FEP3_RETURN_IF_FAILED(readInputInformationFromJobDOMElement(job_element, data_job_configuration));
            FEP3_RETURN_IF_FAILED(readOutputInformationFromJobDOMElement(job_element, data_job_configuration));
            participant_config._data_job_configurations.insert(std::make_pair(job_name, data_job_configuration));
        }
    }

    return {};
}

Result readParticipantInformationFromDOM(const DOM& dom,
    TimingConfiguration& timing_configuration)
{
    DOMElementList participants_element_list;
    if (!dom.findNodes(timing_cfg_node_path_participant, participants_element_list))
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_FILE, "Timing configuration does not contain a participant node.");
    }

    for (const auto& participant_element : participants_element_list)
    {
        const auto participant_name = participant_element.getChild(timing_cfg_node_name).getData();
        if (participant_name.empty())
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_FILE, "Timing configuration contains a participant entry without name.");
        }

        ParticipantConfiguration participant_config;
        FEP3_RETURN_IF_FAILED(readJobInformationFromDOM(participant_element, participant_config));
        timing_configuration._participants.insert(std::make_pair(participant_name, participant_config));
    }

    return {};
}

Result readHeaderInformationFromDOM(const DOM& dom,
    TimingConfiguration& timing_configuration)
{
    DOMElement dom_element;
    if (dom.findNode(timing_cfg_node_path_schema_version, dom_element))
    {
        if (!dom_element.isNull())
        {
            timing_configuration._schema_version = dom_element.getData();
        }
    }

    DOMElement header_element;
    if (dom.findNode(timing_cfg_node_path_header, header_element))
    {
        dom_element = header_element.getChild(timing_cfg_node_author);
        if (!dom_element.isNull())
        {
            timing_configuration._header._author = dom_element.getData();
        }

        dom_element = header_element.getChild(timing_cfg_node_date_creation);
        if (!dom_element.isNull())
        {
            timing_configuration._header._date_creation = dom_element.getData();
        }

        dom_element = header_element.getChild(timing_cfg_node_date_change);
        if (!dom_element.isNull())
        {
            timing_configuration._header._date_change = dom_element.getData();
        }

        dom_element = header_element.getChild(timing_cfg_node_description);
        if (!dom_element.isNull())
        {
            timing_configuration._header._description = dom_element.getData();
        }
    }
    return {};
}

Result readTimingConfigFromDOM(const DOM& dom,
    TimingConfiguration& timing_configuration)
{
    timing_configuration = TimingConfiguration();

    FEP3_RETURN_IF_FAILED(readHeaderInformationFromDOM(dom, timing_configuration));
    FEP3_RETURN_IF_FAILED(readParticipantInformationFromDOM(dom, timing_configuration));

    return {};
}

}

namespace fep3
{
namespace native
{
namespace timing_configuration
{

Result readTimingConfigFromFile(const std::string& file_path, TimingConfiguration& timing_configuration)
{
    using namespace a_util::filesystem;

    DOM dom;
    if (!dom.load(Path(file_path).toString()))
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_FILE, a_util::strings::format(
            "Failed to parse timing configuration file. No valid file at path '%s'", file_path.c_str()).c_str());
    }

    FEP3_RETURN_IF_FAILED(readTimingConfigFromDOM(dom, timing_configuration));

    return {};
}

Result readTimingConfigFromString(const std::string& xml_string, TimingConfiguration& timing_configuration)
{
    DOM dom;
    if (!dom.fromString(xml_string))
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_FILE, "Failed to parse file");
    }

    FEP3_RETURN_IF_FAILED(readTimingConfigFromDOM(dom, timing_configuration));

    return {};
}

Result configureJobsByTimingConfiguration(Jobs& jobs,
    const std::string& participant_name,
    const TimingConfiguration& timing_configuration)
{
    const auto participant_configuration = timing_configuration._participants.find(participant_name);
    if (timing_configuration._participants.end() == participant_configuration)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
            "Invalid timing configuration. No participant entry '%s' exists.",
            participant_name.c_str());
    }

    for (auto& job : jobs)
    {
        const auto data_job_configuration = participant_configuration->second._data_job_configurations.find(job.second.job_info.getName());
        if (participant_configuration->second._data_job_configurations.end() == data_job_configuration)
        {
            RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                "Invalid timing configuration. The participant entry '%s' does not contain a job '%s'.",
                participant_name.c_str(),
                job.second.job_info.getName().c_str());
        }
        FEP3_RETURN_IF_FAILED(reconfigureJobInfoByJobConfiguration(job.second.job_info, data_job_configuration->second._job_configuration));
    }

    return {};
}

} // namespace timing_configuration
} // namespace native
} // namespace fep3