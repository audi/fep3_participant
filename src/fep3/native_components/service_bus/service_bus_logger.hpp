/**
 * @file
 * @copyright AUDI AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */
#pragma once


#include <mutex>
#include <list>
#include <string>
#include <iostream>

namespace service_bus_helper
{
class ILogSink
{
protected:
    virtual ~ILogSink() = default;
public:
    virtual void internalLog(const std::string& message) = 0;
};

class Logger
{
private:
    Logger() = default;

public:
    
    static Logger& get()
    {
        static Logger logger;
        return logger;
    }

    void add(ILogSink* log_sink)
    {
        std::lock_guard<std::recursive_mutex> locked(_sync_sink);
        _log_sinks.push_back(log_sink);
    }
    void remove(ILogSink* log_sink)
    {
        std::lock_guard<std::recursive_mutex> locked(_sync_sink);
        _log_sinks.remove(log_sink);
    }
    void internalLog(const std::string& message)
    {
        std::lock_guard<std::recursive_mutex> locked(_sync_sink);
        if (_log_sinks.size() > 0)
        {
            for (auto& cur : _log_sinks)
            {
                cur->internalLog(message);
            }
        }
        else
        {
            std::cout << message << std::endl;
        }
    }
private: 
    std::list<ILogSink*> _log_sinks; 
    std::recursive_mutex _sync_sink;
};

#define SB_LOG_AND_RETURN_ERROR_DESCRIPTION(_errcode, ...) \
{ \
    std::string message_to_log = a_util::strings::format(__VA_ARGS__); \
    service_bus_helper::Logger::get().internalLog(message_to_log); \
    RETURN_ERROR_DESCRIPTION(_errcode, \
        message_to_log.c_str()); \
}

}