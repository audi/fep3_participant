<!---
  Copyright @ 2020 Audi AG. All rights reserved.

      This Source Code Form is subject to the terms of the Mozilla
      Public License, v. 2.0. If a copy of the MPL was not distributed
      with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

  If it is not possible or desirable to put the notice in a particular file, then
  You may include the notice in a location (such as a LICENSE file in a
  relevant directory) where a recipient would be likely to look for such a notice.

  You may add additional accurate notices of copyright ownership.
  -->
# FEP Logging Service {#page_fep_logging_service}

The FEP Logging Service provides a convienient way to write logs to the console, a file or send them 
to other participants and automatically appends metadata like participant name, date, time and simulation time.
It can also be configured by the system to filter logs based on severity level and to choose where
they should be logged.

## Logging Service Usage

To write a log a logger object has be created first, which can be done like this:

    std::shared_ptr<ILoggingService::ILogger> logger = GetLoggingService()->createLogger("MyLogger.SubDomain.Domain");

During creation a name has to be given to the logger. The name does *not* have to be unique and will 
only be used to identify the logger for configuration and to write in the log from where the log is coming from. 
Loggers can also be grouped together by adding a domain to the logger name. To do this write a 
period `'.'` after the logger name and add a custom domain name. From left to right the name must go 
from the most to the least precise level in the hierarchy.

The actual log can then be written by calling one of the log function from the logger object:

    logger->logError("An Error has occured!");

Should the construction of the string message be very performance intensive, it's also possible to 
retrieve the severity level of the current configuration beforehand:

    if (logger->isErrorEnabled())
    {
        logger->logError(buildCostlyString());
    }

## Logging Service Configuration

Each logger can be configured individually or a group of loggers can be configured all at once. If no 
configuration has been set for a particular logger, the configuration of its domain will be used. If it
doesn't have one either the next higher domain will be used and so on. If there is no configuration at all
the default configuration will be used.

The following parameters can be configured:
* Enabling logging to the console
* Enabling logging to a file
* The file path to the logging file
* Enabling sending logs to other participants over RPC
* Severity Level Filter

To set the configuration of a logger the function `setConfiguration(std::string logger_name, LoggingConfig configuration)` 
of the FEP SDK System Library has to be called. It works as follows:
* If the logger is already configured the old configuration will be overwritten
* If the logger name is a group all members of the group will be set to the new configuration

To change the default configuration an empty string can be passed as the logger_name to the function.
Caution: Passing an empty string will set all loggers to the new configuration and overwrite all existing configurations!

## Writing logs to a file

If file logging is enabled the logging service will try to log to the file given by the configuration.
If no file has been configured but file logging is enabled, the file from the default configuration
will be used. If there is no default log file configured either, an error will be returned, but logging
to console and other participants won't be cancelled if enabled. Several loggers can use the same log file 
at the same time without interference.

## Sending logs from one participant to another

To send a log from sender A to receiver B, receiver B needs to be registered at the logging service of sender A.
For this the FEP SDK System Library provides the function `registerLoggingListener(std::string receiver_participant_name)` that will 
create a one way connection. Only loggers which have RPC enabled in their configuration will send their
logs to other participants this way.

To configure where receiving logs should be logged the logger "ListenerServer.LoggingService.Component.Participant.Sdk.Fep" 
has to be configured. This is a special logger that will use the metadata of the received log instead 
of its own participant. It also cannot be configured to send logs over RPC again to prevent infinite loops.
