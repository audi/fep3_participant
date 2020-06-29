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

#include <iterator>

#include "test_plugin.h"

void fep3_plugin_getPluginVersion
    (void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "scheduler interfaces test c plugin 0.0.1");
}

fep3::mock::TransferableSchedulerServiceWithAccessToSchedulers* g_mock_scheduler_service = nullptr;

void setMockSchedulerService(fep3::mock::TransferableSchedulerServiceWithAccessToSchedulers* mock_scheduler_service)
{
    g_mock_scheduler_service = mock_scheduler_service;
}

fep3::IScheduler* getScheduler(size_t index)
{
    const auto& schedulers = g_mock_scheduler_service->getRegisteredSchedulers();
    if(index < schedulers.size())
    {
        auto scheduler_iterator = schedulers.cbegin();
        std::advance(scheduler_iterator, index);
        return *scheduler_iterator;
    }
    else
    {
        return {};
    }
}