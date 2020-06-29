/**
 *
 * Declaration of class easy logging macros
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

#include "logging_service_intf.h"
#include <fep3/components/base/component_intf.h>
#include <a_util/result.h>

namespace fep3
{
namespace logging
{
namespace arya
{
/**
 * @brief Easy helper class for logging. 
 * Derive from this class to use the logging macros: 
 * @li FEP3_LOG_DEBUG
 * @li FEP3_LOG_FATAL
 * @li FEP3_LOG_ERROR
 * @li FEP3_LOG_WARNING
 * @li FEP3_LOG_INFO
 * @li FEP3_LOG_RESULT
 * 
 * Use as follows:
 * @code
class MyComponent : public EasyLogging
{
//...
    fep3::Result someInitMethod()
    {
        return initLogger("component.my_use_of_easy_logger",*getComponents());
    }
     fep3::Result someDeinitMethod()
    {
        return deinitLogger();
    }
};
 * @endcode
 * 
 */
class EasyLogging
{
public:
    /**
     * @brief retrieves the logging service and creates a logger to log to
     * 
     * @param components the components where the ILoggingService is registered to
     * @param logger_name the name of the logger
     * @return fep3::Result 
     * @retval ERR_NOERROR if succeeded
     * @retval ERR_UNEXPECTED if the creation of logger fails
     */
    fep3::Result initLogger(const IComponents& components,
        const std::string& logger_name)
    {
        auto logging_service = components.getComponent<fep3::arya::ILoggingService>();
        if (logging_service)
        {
            try
            {
                _logger = logging_service->createLogger(logger_name);
            }
            catch (const std::exception& ex)
            {
                RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Can not create logger %s : %s",
                    logger_name.c_str(),
                    ex.what());
            }
        }
        else
        {
            deinitLogger();
        }
        return {};
    }
    /**
     * @brief deinit the logger and reset the logger member
     * 
     */
    void deinitLogger()
    {
        _logger.reset();
    }
    /**
     * @brief Get the Logger object created while initLogger
     * 
     * @return std::shared_ptr<fep3::arya::ILoggingService::ILogger> 
     */
    std::shared_ptr<fep3::arya::ILoggingService::ILogger> getLogger() const
    {
        return _logger;
    }

private:
    /**
     * @brief the logger created
     * 
     */
    std::shared_ptr<fep3::arya::ILoggingService::ILogger> _logger;
};
}
using arya::EasyLogging;
}
}

/***************************************************************
 * versioned macros
 ***************************************************************/
/**
 * Logs a debug message to the logger given.
 * It checks if given_logger is valid and debug is enabled.
 * @param given_logger the logger
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOGGER_LOG_DEBUG(given_logger, message_to_log) \
{ \
    if(given_logger && given_logger->isDebugEnabled()) \
    { \
        given_logger->logDebug(message_to_log); \
    } \
}

/**
 * Logs a fatal message to the logger given.
 * It checks if given_logger is valid and fatal is enabled.
 * @param given_logger the logger
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOGGER_LOG_FATAL(given_logger, message_to_log) \
{ \
    if(given_logger && given_logger->isFatalEnabled()) \
    { \
        given_logger->logFatal(message_to_log); \
    } \
}

/**
 * Logs a error message to the logger given.
 * It checks if given_logger is valid and error is enabled.
 * @param given_logger the logger
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOGGER_LOG_ERROR(given_logger, message_to_log) \
{ \
    if(given_logger && given_logger->isErrorEnabled()) \
    { \
        given_logger->logError(message_to_log); \
    } \
}

/**
 * Logs a warning message to the logger given.
 * It checks if given_logger is valid and warning is enabled.
 * @param given_logger the logger
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOGGER_LOG_WARNING(given_logger, message_to_log) \
{ \
    if(given_logger && given_logger->isWarningEnabled()) \
    { \
        given_logger->logWarning(message_to_log); \
    } \
}

/**
 * Logs a info message to the logger given.
 * It checks if given_logger is valid and info is enabled.
 * @param given_logger the logger
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOGGER_LOG_INFO(given_logger, message_to_log) \
{ \
    if(given_logger && given_logger->isInfoEnabled()) \
    { \
        given_logger->logInfo(message_to_log); \
    } \
}

/**
 * Logs a error message to the logger given based on a error code
 * It checks if given_logger is valid and error is enabled and the result isFailed
 * @param given_logger the logger
 * @param result_to_log the result
 */
#define FEP3_ARYA_LOGGER_LOG_RESULT(given_logger, result_to_log) \
do \
{ \
    if(given_logger && given_logger->isErrorEnabled() && fep3::isFailed(result_to_log)) \
    { \
        fep3::Result tmp_res(result_to_log); \
        given_logger->logError(tmp_res.getDescription()); \
    } \
} while(false)

/***************************************************************
* convenience macros
***************************************************************/
/**
 * Logs a debug message to the logger retrieving by getLogger()
 * It checks if getLogger() is valid and debug is enabled.
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOG_DEBUG(message_to_log) \
    FEP3_ARYA_LOGGER_LOG_DEBUG(getLogger(), message_to_log)

/**
 * Logs a fatal message to the logger retrieving by getLogger()
 * It checks if getLogger() is valid and fatal is enabled.
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOG_FATAL(message_to_log) \
    FEP3_ARYA_LOGGER_LOG_FATAL(getLogger(), message_to_log)
/**
 * Logs a error message to the logger retrieving by getLogger()
 * It checks if getLogger() is valid and error is enabled.
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOG_ERROR(message_to_log) \
    FEP3_ARYA_LOGGER_LOG_ERROR(getLogger(), message_to_log)

/**
 * Logs a warning message to the logger retrieving by getLogger()
 * It checks if getLogger() is valid and warning is enabled.
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOG_WARNING(message_to_log) \
    FEP3_ARYA_LOGGER_LOG_WARNING(getLogger(), message_to_log)

/**
 * Logs a info message to the logger retrieving by getLogger()
 * It checks if getLogger() is valid and info is enabled.
 * @param message_to_log the log message
 */
#define FEP3_ARYA_LOG_INFO(message_to_log) \
    FEP3_ARYA_LOGGER_LOG_INFO(getLogger(), message_to_log)

/**
 * Logs a error message to the logger retrieving by getLogger() based on a error code
 * It checks if getLogger() is valid and error is enabled and the given error code isFailed()
 * @param result_to_log the error code
 */
#define FEP3_ARYA_LOG_RESULT(result_to_log) \
    FEP3_ARYA_LOGGER_LOG_RESULT(getLogger(), result_to_log)

/***************************************************************
* convenience macros no versioning
***************************************************************/
///@copydoc FEP3_ARYA_LOG_DEBUG
#define FEP3_LOG_DEBUG(message_to_log) \
    FEP3_ARYA_LOG_DEBUG(message_to_log)

///@copydoc FEP3_ARYA_LOG_FATAL
#define FEP3_LOG_FATAL(message_to_log) \
    FEP3_ARYA_LOG_FATAL(message_to_log)

///@copydoc FEP3_ARYA_LOG_ERROR
#define FEP3_LOG_ERROR(message_to_log) \
    FEP3_ARYA_LOG_ERROR(message_to_log)

///@copydoc FEP3_ARYA_LOG_WARNING
#define FEP3_LOG_WARNING(message_to_log) \
    FEP3_ARYA_LOG_WARNING(message_to_log)

///@copydoc FEP3_ARYA_LOG_INFO
#define FEP3_LOG_INFO(message_to_log) \
    FEP3_ARYA_LOG_INFO(message_to_log)

///@copydoc FEP3_ARYA_LOG_RESULT
#define FEP3_LOG_RESULT(result_to_log) \
    FEP3_ARYA_LOG_RESULT(result_to_log)
