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

#include "fep3/components/logging/logging_service_intf.h"

#include <a_util/concurrency/mutex.h>

#include <map>
#include <memory>   // unique_ptr
#include <vector>

namespace fep3
{
namespace native
{
struct LoggerFilterConfig
{
    logging::Severity _severity;
    std::map<std::string, std::shared_ptr<ILoggingService::ILoggingSink>> _logging_sinks;
};

class LoggingConfigTree
{
private:
    class Node
    {
    public:
        Node(const LoggerFilterConfig& config) : _config(config) {}
        Node(std::vector<std::string>& name, const LoggerFilterConfig& config, const Node& parent);

        void setLoggerConfig(std::vector<std::string>& name, const LoggerFilterConfig& config);
        const LoggerFilterConfig& getLoggerConfig(std::vector<std::string>& name) const;

    private:
        LoggerFilterConfig _config;
        std::map<std::string, Node> _child_nodes;
    };

public:
    LoggingConfigTree();

    /**
    * @brief Sets the configuration for a logger domain.
    *
    * This will also overwrite all already existing configurations with a lower hierarchy level than this domain.
    *
    * @param [in] logger_name Name of the logger domain which configuration shall be set.
    * @param [in] config      The parameters of the configuration.
    */
    void setLoggerConfig(const std::string& logger_name, const LoggerFilterConfig& config);

    /**
    * @brief Returns a reference to the logging configuration set for the logger name.
    *
    * If no specific configuration has been set, it will return the configuration of the next higher hierarchy level and so on.
    * If there is no configuration for any level it will return the default.
    *
    * @param [in] logger_name The logger domain name for which configuration will be returned
    *
    * @return A reference to a logging configuration.
    */
    const LoggerFilterConfig& getLoggerConfig(const std::string& logger_name) const;

private:
    /// The root node holds the default configuration and has no name
    std::unique_ptr<Node> _root_node;

};
} // namespace native
} // namespace fep
