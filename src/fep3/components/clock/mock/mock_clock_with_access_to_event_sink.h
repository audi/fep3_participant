/**
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

#include "mock_clock_service.h"

namespace fep3
{
namespace mock 
{

struct ClockWithAccessToEventSink : public Clock
{
private:
    void start(const std::weak_ptr<IEventSink>& event_sink) override
    {
        _event_sink = event_sink;
        // call the mocked method to enable setting of expectations
        return Clock::start(event_sink);
    }
    
public:
    std::weak_ptr<IClock::IEventSink> getEventSink() const
    {
        return _event_sink;
    }
private:
    std::weak_ptr<IClock::IEventSink> _event_sink;
};

}
} 
