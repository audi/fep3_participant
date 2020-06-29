/**
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

#include <gmock/gmock.h>
#include <fep3/components/logging/mock/mock_logging_service.h>

namespace fep3
{
namespace test
{
namespace env
{

struct SchedulerTestEnv
{
    inline SchedulerTestEnv()        
        : _logger(std::make_shared<fep3::mock::Logger>())
        , _set_participant_to_error_state(
            [&]() -> fep3::Result {return _set_participant_to_error_state_mock.Call(); })
    {        
    }

    std::shared_ptr<fep3::mock::Logger> _logger;

     /// necessary because MockFunction.AsStdFunction seems not be available with our gtest
    std::function<fep3::Result()> _set_participant_to_error_state{};
    ::testing::MockFunction<fep3::Result()> _set_participant_to_error_state_mock;
};

}
}
}
