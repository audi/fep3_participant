/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "logging_config.h"
#include <a_util/strings.h>

using namespace fep3;
using namespace fep3::native;

LoggingConfigTree::Node::Node(std::vector<std::string>& name, const LoggerFilterConfig& config, const Node& parent)
{
    if (name.empty())
    {
        _config = config;
    }
    else
    {
        _config = parent._config;

        std::string last_name = name.back();
        name.pop_back();
        _child_nodes.emplace(last_name, Node(name, config, *this));
    }
}

void LoggingConfigTree::Node::setLoggerConfig(std::vector<std::string>& name, const LoggerFilterConfig& config)
{
    if (name.empty())
    {
        _config = config;

        for (auto& node : _child_nodes)
        {
            node.second.setLoggerConfig(name, config);
        }
    }
    else
    {
        std::string last_name = name.back();
        name.pop_back();
        auto it = _child_nodes.find(last_name);
        if (it == _child_nodes.end())
        {
            _child_nodes.emplace(last_name, Node(name, config, *this));
        }
        else
        {
            it->second.setLoggerConfig(name, config);
        }
    }
}

const LoggerFilterConfig& LoggingConfigTree::Node::getLoggerConfig(std::vector<std::string>& name) const
{
    if (name.empty())
    {
        return _config;
    }
    else
    {
        std::string last_name = name.back();
        auto it = _child_nodes.find(last_name);
        if (it == _child_nodes.end())
        {
            return _config;
        }
        else
        {
            name.pop_back();
            return it->second.getLoggerConfig(name);
        }
    }
}


LoggingConfigTree::LoggingConfigTree()
{
    //TODO: Do not define the default hard coded here
    LoggerFilterConfig default_config = { logging::Severity::info, {} };
    _root_node = std::make_unique<Node>(default_config);
}

void LoggingConfigTree::setLoggerConfig(const std::string& logger_name, const LoggerFilterConfig& config)
{
    std::vector<std::string> name_parts = a_util::strings::split(logger_name, ".");
    _root_node->setLoggerConfig(name_parts, config);
}

const LoggerFilterConfig& LoggingConfigTree::getLoggerConfig(const std::string& logger_name) const
{
    std::vector<std::string> name_parts = a_util::strings::split(logger_name, ".");
    return _root_node->getLoggerConfig(name_parts);
}