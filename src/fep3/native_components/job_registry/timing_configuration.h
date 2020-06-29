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

#pragma once

#include <map> 

#include <fep3/fep3_errors.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <functional>

namespace fep3
{
namespace native
{
namespace timing_configuration
{

/// Timing configuration header
struct Header
{
    /// Author element
    std::string _author;
    /// Date of creation 
    std::string _date_creation;
    /// Date of latest change
    std::string _date_change;
    /// Description
    std::string _description;
};

/// Configuration Item for job input configuration
struct JobInputConfiguration
{
    /// Input queue size
    int32_t _queue_size;
};

/// Configuration Item for job output configuration
struct JobOutputConfiguration
{
    /// Output queue size
    int32_t _queue_size;
};

/// Participant timing configuration
struct DataJobConfiguration
{
    DataJobConfiguration(JobConfiguration job_configuration)
        : _job_configuration(std::move(job_configuration))
    {
    }

    /// Configuration of job 
    JobConfiguration _job_configuration;
    /// Configuration of job inputs 
    std::map<std::string, JobInputConfiguration> _job_input_configurations{};
    /// Configuration of job outputs 
    std::map<std::string, JobOutputConfiguration> _job_output_configurations{};
};

/// Participant timing configuration
struct ParticipantConfiguration
{
    /// Configuration of data jobs
    std::map<std::string, DataJobConfiguration> _data_job_configurations{};
};

/// Timing configuration root element
struct TimingConfiguration
{
    /// Schema version
    std::string _schema_version;
    /// Header 
    Header _header;
    /// Participant elements to be configured
    std::map<std::string, ParticipantConfiguration> _participants{};
};

/**
* @brief Read timing configuration from file
*
* @param [in] file_path Path to a timing configuration file
* @param [out] timing_configuration Timing configuration output
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine
* @retval ERR_INVALID_FILE The timing configuration is not correct.
* @retval ERR_EMPTY The timing configuration is missing an essential node.
* @retval ERR_INVALID_ARG The timing configuration contains invalid configuration values.
*/
Result readTimingConfigFromFile(const std::string& file_path,
    TimingConfiguration& timing_configuration);

/**
* @brief Read timing 3 configuration from string
*
* @param [in] xml_string String with configuration in xml format
* @param [out] timing_configuration Timing configuration output
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine
* @retval ERR_INVALID_FILE The timing configuration is not correct.
* @retval ERR_EMPTY The timing configuration is missing an essential node.
* @retval ERR_INVALID_ARG The timing configuration contains invalid configuration values.
*/
Result readTimingConfigFromString(const std::string& xml_string,
    TimingConfiguration& timing_configuration);

/**
* @brief Configure jobs by a timing configuration
*
* @param [in] jobs To be configured
* @param [in] participant_name Local name of the participant
* @param [in] timing_configuration Timing configuration to use
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine
* @retval ERR_NOT_FOUND The timing configuration lacks a required entry (e.g. job/participant)
* @retval ERR_INVALID_ARG The timing configuration contains invalid configuration values
*/
Result configureJobsByTimingConfiguration(Jobs& jobs,
    const std::string& participant_name,
    const TimingConfiguration& timing_configuration);
}

} // namespace native
} // namespace fep3


