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
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>

#include <fep3/native_components/scheduler/local_scheduler_registry.h>
#include <fep3/components/scheduler/mock/mock_scheduler.h>

using namespace ::testing;

using Scheduler = NiceMock<fep3::mock::Scheduler>;


struct SchedulerRegistryTest : public ::testing::Test
{
    SchedulerRegistryTest() :
        _scheduler_mock(new Scheduler()),
        _default_scheduler(new Scheduler())
    {
        ON_CALL(*_scheduler_mock.get(), getName())
            .WillByDefault(Return("my_scheduler"));

        ON_CALL(*_default_scheduler.get(), getName())
            .WillByDefault(Return("default_scheduler"));
    }

    std::unique_ptr<Scheduler> _scheduler_mock{};
    std::unique_ptr<Scheduler> _default_scheduler{};
};


/**
* @brief Set a scheduler and check that is will be set as active scheduler
* @req_id FEPSDK-2083
*/
TEST_F(SchedulerRegistryTest, SetSchedulerIsGetScheduler)
{
    auto scheduler_name = _scheduler_mock->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));

    // actual test
    {
        ASSERT_FEP3_NOERROR(scheduler_registry.registerScheduler(std::move(_scheduler_mock)));
        ASSERT_FEP3_NOERROR(scheduler_registry.setActiveScheduler(scheduler_name));

        auto current_schduler = scheduler_registry.getActiveSchedulerName();        
        ASSERT_EQ(current_schduler, scheduler_name);
    }
}


/**
* @brief Set a scheduler that is not registred
* @req_id FEPSDK-2083
*/
TEST_F(SchedulerRegistryTest, SetSchedulerThatWasNotRegistered)
{
    auto scheduler_name = _scheduler_mock->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));

    // actual test
    {
        ASSERT_FEP3_NOERROR(scheduler_registry.registerScheduler(std::move(_scheduler_mock)));        
    }
}

/**
* @brief Register and unregister a scheduler
* @req_id FEPSDK-2081, FEPSDK-2082
*/
TEST_F(SchedulerRegistryTest, RegisterUnregister)
{
    auto scheduler_name = _scheduler_mock->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));
    
    // actual test
    {
        ASSERT_FEP3_NOERROR(scheduler_registry.registerScheduler(std::move(_scheduler_mock)));        
        ASSERT_FEP3_NOERROR(scheduler_registry.unregisterScheduler(scheduler_name));             
    }
}

/**
* @brief Register two schedulers with same name
* @req_id FEPSDK-2081
*/
TEST_F(SchedulerRegistryTest, RegisterTwoSchedulersWithSameName)
{
    auto scheduler_name = _scheduler_mock->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));

    std::unique_ptr<Scheduler> _scheduler_same_name{ new Scheduler() };
    ON_CALL(*_scheduler_same_name.get(), getName())
        .WillByDefault(Return(_scheduler_mock->getName()));

    // actual test
    {
        ASSERT_FEP3_NOERROR(scheduler_registry.registerScheduler(std::move(_scheduler_mock)));
        ASSERT_FEP3_RESULT(scheduler_registry.registerScheduler(std::move(_scheduler_same_name)), fep3::ERR_RESOURCE_IN_USE);                
    }
}

/**
* @brief Register two schedulers and check scheduler list
* @req_id FEPSDK-2081, FEPSDK-2084
*/
TEST_F(SchedulerRegistryTest, GetSchedulerList)
{    
    auto default_scheduler_name = _default_scheduler->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));

    std::unique_ptr<Scheduler> _scheduler_two {new Scheduler()};
    ON_CALL(*_scheduler_two.get(), getName())
        .WillByDefault(Return("scheduler_two"));

    auto list_expected = std::list<std::string>({ default_scheduler_name, _scheduler_mock->getName(), _scheduler_two->getName() });

    // actual test
    {
        ASSERT_FEP3_NOERROR(scheduler_registry.registerScheduler(std::move(_scheduler_mock)));
        ASSERT_FEP3_NOERROR(scheduler_registry.registerScheduler(std::move(_scheduler_two)));

        auto list = scheduler_registry.getSchedulerNames();
        ASSERT_EQ(list, list_expected);        
    }
}

/**
* @brief Default scheduler is the active scheduler if no scheduler was registered
* @req_id FEPSDK-2099
*/
TEST_F(SchedulerRegistryTest, DefaultSchedulerisActiveSchedulerIfNoSchedulerWasRegistered)
{
    auto scheduler_name = _scheduler_mock->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));

    // actual test
    {
        auto current_schduler = scheduler_registry.getActiveSchedulerName();        
        ASSERT_EQ(current_schduler, "default_scheduler");
    }
}


/**
* @brief The last scheduler will be unregistered.
* @detail In this case the default scheduler has to be set
* @req_id FEPSDK-2099
*/
TEST_F(SchedulerRegistryTest, UnregisterLastScheduler)
{    
    auto scheduler_name = _scheduler_mock->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));

    // actual test
    {
        ASSERT_FEP3_NOERROR(scheduler_registry.registerScheduler(std::move(_scheduler_mock)));
        ASSERT_FEP3_NOERROR(scheduler_registry.unregisterScheduler(scheduler_name));        

        auto current_schduler = scheduler_registry.getActiveSchedulerName();
        ASSERT_EQ(current_schduler, "default_scheduler");
    }
}

/**
* @brief It will be verified that the default scheduler (which is a native scheduler) can not be unregistered
* @req_id FEPSDK-2168
*/
TEST_F(SchedulerRegistryTest, UnregisterDefaulfScheduler)
{    
    const auto default_scheduler_name = _default_scheduler->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));

    // actual test
    {      
        ASSERT_FEP3_RESULT(scheduler_registry.unregisterScheduler(default_scheduler_name), fep3::ERR_INVALID_ARG);       
    }
}

/**
* @brief A scheduler that is not existing is attempted to be unregistered. 
* @req_id FEPSDK-2082
*/
TEST_F(SchedulerRegistryTest, UnregisterNotExistingScheduler)
{    
    const auto default_scheduler_name = _default_scheduler->getName();
    fep3::native::LocalSchedulerRegistry scheduler_registry(std::move(_default_scheduler));

    // actual test
    {      
        ASSERT_FEP3_RESULT(scheduler_registry.unregisterScheduler("not_existing_scheduler"), fep3::ERR_NOT_FOUND);       
    }
}