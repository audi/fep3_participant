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

#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/base/properties/properties.h>
#include "logging_sink_common.hpp"

#include <a_util/concurrency/mutex.h>

#include <memory>
#include <iostream>

namespace fep3
{
/**
* @brief Implementation of the console logging. Can be used as a base class for a custom sink.
*        This sink will write fatal and error to stderr and everything else to stdout.
*/
class LoggingSinkConsole : public Properties<ILoggingService::ILoggingSink>
{
public:
    LoggingSinkConsole() {}
    fep3::Result log(logging::LogMessage log) const override
    {
        std::string log_msg;
        native::formatLoggingString(log_msg, log);

        std::unique_lock<std::mutex> guard(getConsoleMutex());
        if (logging::Severity::error == log._severity || logging::Severity::fatal == log._severity)
        {
            std::cerr << log_msg << std::endl;
#ifdef WIN32
            ::OutputDebugString(log_msg.c_str());
#endif
        }
        else
        {
#ifdef WIN32
#ifdef _DEBUG
            ::OutputDebugString(log_msg.c_str());
#endif
#endif
            std::cout << log_msg << std::endl;
        }
        return{};
    }

protected:
    /**
    * @brief Mutex for the console to ensure single point access (Meyers' singleton)
    * @return The static mutex variable
    */
    static a_util::concurrency::mutex& getConsoleMutex()
    {
        static a_util::concurrency::mutex _console_mutex;
        return _console_mutex;
    }
};
} // namespace fep3