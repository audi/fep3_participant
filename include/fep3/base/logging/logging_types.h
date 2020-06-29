/**
 * @file 
 * @copyright Audi AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */
 //Guideline - FEP System Library API Exception
#ifndef __FEP_BASE_LOGGING_TYPES_H
#define __FEP_BASE_LOGGING_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

namespace fep3
{
namespace logging
{
/// Version namespace
namespace arya
{
/// Filter for logging events, the smaller the number the less events will be given
enum class Severity : uint8_t
{
    /// Intended to turn off logging
    off = 0,
    /// Very severe error event that will presumably lead the application to abort
    fatal = 1,
    /// Error event that might still allow the application to continue running
    error = 2,
    /// Designates potentially harmful situations
    warning = 3,
    /// Informational message that highlights the progress of the application
    info = 4,
    /// Fine-grained informational event that is most useful to debug an application
    debug = 5
};

/**
 * Struct for a log message
 * is used within the ILogger interface.
 */
struct LogMessage
{
    /// The timestamp of the simulation time
    std::string _timestamp;
    /// The level of importance of the event
    Severity _severity;
    /// The name of the participant
    std::string _participant_name;
    /// Name of the logger that created the log
    std::string _logger_name;
    /// The actual message text of the log
    std::string _message;
};

/**
* Struct to describe a logging configuration
*/
struct LoggerFilter
{
    /// The maximum level that should be logged
    Severity _severity;
    /// List of all enabled logging sinks. Natively supported are "file", "console" and "rpc"
    std::vector<std::string> _enabled_logging_sinks;
};

} // namespace arya
using arya::Severity;
using arya::LogMessage;
using arya::LoggerFilter;
} // namespace logging
} // namespace fep3

#endif //__FEP_BASE_LOGGING_TYPES_H
