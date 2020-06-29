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

#include <algorithm>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gtest_asserts.h>

#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/components/logging/logging_service_intf.h>

#include <fep3/components/configuration/propertynode_helper.h>
#include <fep3/core.h>

namespace fep3
{
namespace test
{
namespace scenario
{

enum class ParticipantState
{
    UNLOADED,
    LOADED,
    INITIALIZED,
    RUNNING,
    PAUSED,
    FINAL
};  

inline fep3::Optional<std::string> getStateName(ParticipantState state_name)
{  
    auto state_map = std::map<ParticipantState, std::string>{
        { ParticipantState::UNLOADED, "UNLOADED"},
        { ParticipantState::LOADED, "LOADED"},
        { ParticipantState::INITIALIZED, "INITIALIZED"},
        { ParticipantState::RUNNING, "RUNNING"},
        { ParticipantState::PAUSED, "PAUSED"},
        { ParticipantState::FINAL, "FINAL"}  };
       

    if(state_map.count(state_name) == 0)
    {
        return {};
    }
    return {state_map[state_name]};    
}

class IStateMachine
{
    virtual void Running() = 0;
    virtual void Initialized() = 0;
};

inline fep3::Result configureParticipant(const std::vector<std::pair<std::string, std::string>>& pairs_of_properties, fep3::Participant& participant)
{
    const auto configuration_service = participant.getComponent<fep3::IConfigurationService>();
    if (!configuration_service)
    {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ADDRESS, "configuration service unavailable");
    }
    for (const auto& pair_of_properties : pairs_of_properties)
    {
        FEP3_RETURN_IF_FAILED(fep3::arya::setPropertyValue<std::string>(*configuration_service,
            pair_of_properties.first,
            pair_of_properties.second))
    }
    return {};
}

inline fep3::Result configureParticipants(const std::vector<std::pair<std::string, std::string>>& pairs_of_properties,  std::vector<std::shared_ptr<fep3::Participant>> participants)
{
     std::for_each(participants.begin(), participants.end(), [pairs_of_properties](std::shared_ptr<fep3::Participant>& participant)
        {
            configureParticipant(pairs_of_properties, *participant);
        });

    return {};        
}

MATCHER(logIsError, "") 
{
     return (arg._severity == fep3::logging::Severity::error
        || arg._severity == fep3::logging::Severity::fatal); 
}

}
}
}