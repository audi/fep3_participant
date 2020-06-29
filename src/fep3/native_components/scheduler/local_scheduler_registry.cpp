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

#include "local_scheduler_registry.h"

#include <algorithm>
#include <cassert>
#include <iterator>


namespace fep3
{
namespace native
{

LocalSchedulerRegistry::LocalSchedulerRegistry(std::unique_ptr<fep3::IScheduler> default_scheduler) 
    : _default_scheduler_name(default_scheduler->getName())
    , _active_scheduler("")
{
    registerScheduler(std::move(default_scheduler));
    
    const auto result = activateDefaultScheduler();
    if (fep3::isFailed(result))
    {
        throw std::runtime_error(result.getDescription());
    }  
    
    assert(!_active_scheduler.empty());
}

fep3::Result LocalSchedulerRegistry::activateDefaultScheduler()
{
    return setActiveScheduler(_default_scheduler_name);    
}

fep3::Result LocalSchedulerRegistry::setActiveScheduler(const std::string& scheduler_name)
{
    const auto scheduler = findScheduler(scheduler_name);
    if (!scheduler)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
            "Setting scheduler failed. A scheduler with the name '%s' is not registered.", scheduler_name.c_str());
    }
    _active_scheduler = scheduler_name;       
    
    return {};
}

std::string LocalSchedulerRegistry::getActiveSchedulerName() const
{
    return _active_scheduler;
}

fep3::Result LocalSchedulerRegistry::deinitializeActiveScheduler() const
{
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");
    }   
        
    return active_scheduler->deinitialize();   
}


fep3::Result LocalSchedulerRegistry::startActiveScheduler() const
{
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");
    }
    return active_scheduler->start();
}

fep3::Result LocalSchedulerRegistry::stopActiveScheduler() const
{    
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");        
    }
   
    return active_scheduler->stop();
}

std::string LocalSchedulerRegistry::getDefaultSchedulerName() const
{
    return _default_scheduler_name;
}

fep3::Result LocalSchedulerRegistry::initializeActiveScheduler(fep3::IClockService& clock, const fep3::Jobs& jobs) const
{
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");
    }

    return active_scheduler->initialize(clock, jobs);    
}


fep3::IScheduler * LocalSchedulerRegistry::getActiveScheduler() const
{
    return findScheduler(_active_scheduler);
}

fep3::IScheduler* LocalSchedulerRegistry::findScheduler(const std::string& scheduler_name) const
{
    for (const auto& current_scheduler : _schedulers)
    {
        if (current_scheduler->getName() == scheduler_name)
        {
            return current_scheduler.get();
        }
    }

    return nullptr;
}


fep3::Result LocalSchedulerRegistry::registerScheduler(std::unique_ptr<fep3::IScheduler> scheduler)
{
    const auto find_result = findScheduler(scheduler->getName());
    if (find_result != nullptr)
    {
        RETURN_ERROR_DESCRIPTION(ERR_RESOURCE_IN_USE,
            "Registering scheduler failed. A scheduler with the name '%s' is already registered.", scheduler->getName().c_str());
    }  

    _schedulers.push_back(std::move(scheduler));
    return{};    
}

fep3::Result LocalSchedulerRegistry::unregisterScheduler(const std::string& scheduler_name)
{        
    if (_default_scheduler_name == scheduler_name)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            "Unregistering the default scheduler is not possible");                    
    }

    const auto it = std::remove_if(_schedulers.begin(), _schedulers.end(),
        [scheduler_name](const std::unique_ptr<fep3::IScheduler>& scheduler){return scheduler_name == scheduler->getName();});

    if (it == _schedulers.end())
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
            "Unregisterung scheduler failed. A scheduler with the name '%s' is not registered.", scheduler_name.c_str());
    }

    _schedulers.erase(it);
    if (scheduler_name == getActiveSchedulerName())
    {
        activateDefaultScheduler();
    }

    return {};
}

std::list<std::string> LocalSchedulerRegistry::getSchedulerNames() const
{
    std::list<std::string> scheduler_list;

    std::transform(
        _schedulers.begin(), 
        _schedulers.end(), 
        std::back_inserter(scheduler_list),
         [](const std::unique_ptr<fep3::IScheduler>& scheduler){return scheduler->getName();});

    return scheduler_list;
}

} // namespace native
} // namespace fep3