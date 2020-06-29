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

#include "scenario_participant.h"

namespace fep3
{
namespace test
{
namespace scenario
{

struct SystemStateMachine
    : public IStateMachine  
{
    std::vector<ParticipantStateMachine> _participant_state_machines;
    const std::chrono::seconds _transition_timeout {5};

    SystemStateMachine(std::vector<std::shared_ptr<fep3::core::ParticipantExecutor>> executors)        
    {
         std::for_each(executors.begin(), executors.end(), [this](std::shared_ptr<fep3::core::ParticipantExecutor> executor)
         {
                _participant_state_machines.push_back(ParticipantStateMachine(executor));
         });
    }  

    void changeStateInParallel(const std::string& error_message, const std::function <void (ParticipantStateMachine&)>& change_func)
	{
      std::vector<std::future<void>> futures;
        std::for_each(_participant_state_machines.begin(), _participant_state_machines.end(), [&futures, &change_func](ParticipantStateMachine& state_machine)
            {                        
                auto handle = std::async(std::launch::async, [&]()                            
                    {
                        change_func(state_machine);
                    });
                futures.push_back(std::move(handle));
            });

        std::for_each(futures.begin(), futures.end(), [this, &error_message](std::future<void>& future)
            {
                auto result = future.wait_for(_transition_timeout);
                if(result != std::future_status::ready)
                {
                    throw std::runtime_error(error_message);
                }
                future.get();
            });        
	}

    void Running() override
     {
		changeStateInParallel("Timeout bringing participant to state RUNNING",
			[](ParticipantStateMachine& state_machine)
			{
				state_machine.Running();
			}
		);
     }

    void Initialized() override
    {
		changeStateInParallel("Timeout bringing participant to state INITIALIZED",
			[](ParticipantStateMachine& state_machine)
		{
			state_machine.Initialized();
		}
		);
    }
};

}
}
}
