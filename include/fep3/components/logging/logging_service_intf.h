/**
* @file
* Copyright &copy; AUDI AG. All rights reserved.
* 
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/base/component_iid.h>
#include <fep3/base/logging/logging_types.h>
#include <fep3/base/properties/properties_intf.h>

#include <string>
#include <memory>

/**
* @brief Main property entry of the logging service properties
*/
#define FEP3_LOGGING_SERVICE_CONFIG "logging"

/**
 * @brief The logging configuration property name for default sinks to use.
 * by native following values are possible as list: 
 * \li console
 * \li rpc
 * \li file
 * By default "rpc,console" is used.
 *
 */
#define FEP3_LOGGING_DEFAULT_SINKS_PROPERTY "default_sinks"

/**
 * @brief The logging configuration property path for default sinks to use.
 * by native following values are possible as list:
 * \li console
 * \li rpc
 * \li file
 * By default "rpc,console" is used.
 *
 */
#define FEP3_LOGGING_DEFAULT_SINKS  FEP3_LOGGING_SERVICE_CONFIG "/" FEP3_LOGGING_DEFAULT_SINKS_PROPERTY

/**
 * @brief The logging configuration property name for the default severity level
 *
 */
#define FEP3_LOGGING_DEFAULT_SEVERITY_PROPERTY "default_severity"

/**
 * @brief The logging configuration path name for the default severity level.
 * 
 *
 */
#define FEP3_LOGGING_DEFAULT_SEVERITY FEP3_LOGGING_SERVICE_CONFIG "/" FEP3_LOGGING_DEFAULT_SEVERITY_PROPERTY

/**
 * @brief The logging configuration property name for the default file used in the filesink
 *
 */
#define FEP3_LOGGING_DEFAULT_FILE_SINK_PROPERTY "default_sink_file"

/**
 * @brief The logging configuration property path for the default file used in the filesink
 *
 */
#define FEP3_LOGGING_DEFAULT_FILE_SINK FEP3_LOGGING_SERVICE_CONFIG "/" FEP3_LOGGING_DEFAULT_FILE_SINK_PROPERTY


namespace fep3
{
namespace arya
{
    /**
     * Logging service of one participant. 
     * 
     * The logging service will provide a single logging access point a within a participant as component.
     *
     */
    class FEP3_PARTICIPANT_EXPORT ILoggingService
    {

    public:
        /**
         * @brief Destroy the ILoggingService object
         */
        virtual ~ILoggingService() = default;
    public:

        /**
         * Logger interface to log messages to. 
         * @see ILoggingService::createLogger
         */
        class ILogger
        {
        protected:
            /// DTOR 
            virtual ~ILogger() {};
        public:

            /**
            * Log informational messages that highlight the progress of the application.
            * @param [in] message Info message
            *
            * @return Standard Result value
            *
            * @retval ERR_MEMORY            The internal logging queue is full (console and listener)
            * @retval ERR_POINTER           No log file has been configured or opened (only file)
            * @retval ERR_DEVICE_IO         Writing log to file failed (only file)
            * @retval ERR_EXCEPTION_RAISED  The RPC failed and threw an exception (only listener)
            */
            virtual fep3::Result logInfo(const std::string& message) const = 0;

            /**
            * Log potentially harmful situations
            * @param [in] message Warning message
            *
            * @return Standard Result value. See @ref logInfo() for all possible values.
            */
            virtual fep3::Result logWarning(const std::string& message) const = 0;

            /**
            * Log error events that might still allow the application to continue running
            * @param [in] message Error message
            *
            * @return Standard Result value. See @ref logInfo() for all possible values.
            */
            virtual fep3::Result logError(const std::string& message) const = 0;

            /**
            * Log very severe error events that will presumably lead the application to abort
            * @param [in] message Fatal message
            *
            * @return Standard Result value. See @ref logInfo() for all possible values.
            */
            virtual fep3::Result logFatal(const std::string& message) const = 0;

            /**
            * Log informational events that are most useful for debugging
            * @param [in] message Debug message
            *
            * @return Standard Result value. See @ref logInfo() for all possible values.
            */
            virtual fep3::Result logDebug(const std::string& message) const = 0;

            /**
            * Check whether the logger is enabled for INFO priority.
            * @retval INFO is enabled
            */
            virtual bool isInfoEnabled() const = 0;
            /**
            * Check whether the logger is enabled for WARNING priority.
            * @retval WARNING is enabled
            */
            virtual bool isWarningEnabled() const = 0;
            /**
            * Check whether the logger is enabled for ERROR priority.
            * @retval ERROR is enabled
            */
            virtual bool isErrorEnabled() const = 0;
            /**
            * Check whether the logger is enabled for FATAL priority.
            * @retval FATAL is enabled
            */
            virtual bool isFatalEnabled() const = 0;
            /**
            * Check whether the logger is enabled for DEBUG priority.
            * @retval DEBUG is enabled
            */
            virtual bool isDebugEnabled() const = 0;

        };
        /**
         * @brief Logging sink interface for all native as well as custom logging sinks.
         *
         * @note The derived class also needs to override the static function "static std::shared_ptr<ILoggingSink> createMethod()"
         *       that returns an object of itself to be able to register it at the logging sink factory.
         */
        class ILoggingSink : public IProperties
        {
        public:
            /**
             * DTOR public and virtual !
             */
            virtual ~ILoggingSink() = default;
            /**
            * @brief Logging function called by the logging service.
            *
            * @param [in] log The log message with description, severity level, timestamp, logger/domain name and participant name
            *
            * @return A standard result code to tell the logging service if the logging was succesful
            */
            virtual fep3::Result log(logging::arya::LogMessage log) const = 0;
        };

    public:
        /// the fep component interface identifier for the ILoggingService
        FEP_COMPONENT_IID("logging_service.arya.fep3.iid");

        /**
         * @brief Creates a logger object that can be used to emit logs. 
         *
         * @param [in] logger_name Name of the logger domain. Used to know from where the log is coming from and for configuration.
         *
         * @return Shared pointer to the created logger object.
         */
        virtual std::shared_ptr<ILogger> createLogger(const std::string& logger_name) = 0;

        /**
         * @brief Registers a logging sink with the given name.
         *
         * @param [in] name Name of the sink (must be unique)
         * @param [in] sink the shared pointer of the sink
         *             the sink pointer must live until the unregister 
         *             was called or the logging service is destroyed
         *
         * @return fep3::Result
         * @retval ERR_IN_USE a sink with the name already exists
         * @retval ERR_NO_ERROR success
         */
        virtual fep3::Result registerSink(const std::string& name, 
                                          const std::shared_ptr<ILoggingSink>& sink) = 0;
        /**
         * @brief Unregisters a logging sink with the given name.
         *
         * @param [in] name Name of the sink (must be unique and exists)
         *
         * @return fep3::Result
         * @retval ERR_NOT_FOUND the sink with the name does not exists
         * @retval ERR_NO_ERROR success
         */
        virtual fep3::Result unregisterSink(const std::string& name) = 0;

    };
} // namespace arya
using arya::ILoggingService;
} // namespace fep3