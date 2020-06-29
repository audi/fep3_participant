/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <a_util/datetime.h>
#include <fep3/base/logging/logging_types.h>

namespace fep3
{
namespace native
{

/**
* @brief (Optional) Helper function to ensure a consistent format for all kind of logs
*
* @param [out] log_msg The formatted logging string. The function will only append data to this parameter.
* @param [in] log The source log with all the information data.
*/
inline void formatLoggingString(std::string& log_msg, const logging::LogMessage& log)
{
    log_msg.append(a_util::strings::format("[%s - %s]: ",
        a_util::datetime::getCurrentLocalDate().format("%d.%m.%Y").c_str(),
        a_util::datetime::getCurrentLocalTime().format("%H:%M:%S").c_str()));

    log_msg.append(a_util::strings::format("%s@%s ", log._participant_name.c_str(), log._logger_name.c_str()));
    log_msg.append(a_util::strings::format(" ST: %d[us]  ", log._timestamp));

    switch (log._severity)
    {
    case logging::Severity::info:
        log_msg.append("Info");
        break;
    case logging::Severity::warning:
        log_msg.append("Warning");
        break;
    case logging::Severity::fatal:
        log_msg.append("Fatal");
        break;
    case logging::Severity::error:
        log_msg.append("Error");
        break;
    case logging::Severity::debug:
        log_msg.append("Debug");
        break;
    default:
        log_msg.append("<Unknown>");
        break;
    }

    log_msg.append(a_util::strings::format(" %s",
        log._message.c_str()));
}
}
} // namespace fep3