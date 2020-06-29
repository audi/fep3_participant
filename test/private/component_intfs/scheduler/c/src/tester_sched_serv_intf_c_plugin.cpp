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

#include <fep3/fep3_errors.h>
#include <fep3/plugin/c/c_host_plugin.h>
#include <fep3/participant/component_factories/c/component_creator_c_plugin.h>
#include <fep3/components/scheduler/mock/mock_transferable_scheduler_service_with_access_to_schedulers.h>
#include <fep3/components/scheduler/mock/mock_scheduler.h>
#include <fep3/components/scheduler/mock/mock_scheduler_with_access_to_jobs.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/job_registry/mock/mock_job.h>
#include <fep3/components/job_registry/mock/mock_job_registry.h>
#include <fep3/components/scheduler/c_access_wrapper/scheduler_service_c_access_wrapper.h>
#include <helper/component_c_plugin_helper.h>

const std::string test_plugin_1_path = PLUGIN;

using namespace fep3::plugin::c::arya;
using namespace fep3::plugin::c::access::arya;

ACTION_P(CheckScheduler, pointer_to_reference_scheduler)
{
    if(arg0->getName() != pointer_to_reference_scheduler->getName())
    {
        RETURN_ERROR_DESCRIPTION(::fep3::ERR_INVALID_ARG, "scheduler does not match");
    }
    return {};
}

struct Plugin1PathGetter
{
    std::string operator()() const
    {
        return test_plugin_1_path;
    }
};
struct SetMockComponentFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockSchedulerService";
    }
};

/**
 * Test fixture loading a mocked scheduler service from within a C plugin
 */
using SchedulerServiceLoader = MockedComponentCPluginLoader
    <::fep3::ISchedulerService
    , fep3::mock::SchedulerService<fep3::plugin::c::TransferableComponentBase>
    , ::fep3::plugin::c::access::arya::SchedulerService
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
using SchedulerServiceLoaderFixture = MockedComponentCPluginLoaderFixture<SchedulerServiceLoader>;

/**
 * Test method fep3::ISchedulerService::getActiveSchedulerName of a scheduler service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(SchedulerServiceLoaderFixture, testMethod_getActiveSchedulerName)
{
    const auto& test_scheduler_name = std::string("test_scheduler");

    // setting of expectations
    {
        auto& mock_scheduler_service = getMockComponent();
        
        EXPECT_CALL(mock_scheduler_service, getActiveSchedulerName())
            .WillOnce(::testing::Return(test_scheduler_name));
    }
    ::fep3::arya::ISchedulerService* scheduler_service = getComponent();
    ASSERT_NE(nullptr, scheduler_service);
    EXPECT_EQ(test_scheduler_name, scheduler_service->getActiveSchedulerName());
}

/**
 * Test method fep3::ISchedulerRegistry::registerScheduler of a scheduler service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(SchedulerServiceLoaderFixture, testMethod_registerScheduler)
{
    // Note: We are not testing the IScheduler interface in this test, so we use NiceMock and don't set expectations.
    auto mock_scheduler = std::make_unique<::testing::NiceMock<::fep3::mock::Scheduler>>();
    // setting of expectations
    {
        auto& mock_scheduler_service = getMockComponent();
        
        // Note: Using a gMock matcher would suit better here than an action, but a matcher 
        // must not have any side effects and invoking a mock method has a side effect.
        EXPECT_CALL(mock_scheduler_service, registerScheduler_(::testing::_))
            .WillOnce(CheckScheduler(mock_scheduler.get()));
    }
    ::fep3::arya::ISchedulerService* scheduler_service = getComponent();
    ASSERT_NE(nullptr, scheduler_service);
    EXPECT_EQ(::fep3::Result{}, scheduler_service->registerScheduler(std::move(mock_scheduler)));
}

/**
 * Test method fep3::ISchedulerRegistry::unregisterScheduler of a scheduler service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(SchedulerServiceLoaderFixture, testMethod_unregisterScheduler)
{
    const auto& test_scheduler_name = std::string("test_scheduler");
    
    // setting of expectations
    {
        auto& mock_scheduler_service = getMockComponent();

        EXPECT_CALL(mock_scheduler_service, unregisterScheduler(test_scheduler_name))
            .WillOnce(::testing::Return(::fep3::Result{}));
    }
    ::fep3::arya::ISchedulerService* scheduler_service = getComponent();
    ASSERT_NE(nullptr, scheduler_service);
    EXPECT_EQ(::fep3::Result{}, scheduler_service->unregisterScheduler(test_scheduler_name));
}

/**
 * Test method fep3::ISchedulerRegistry::getSchedulerNames of a scheduler service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(SchedulerServiceLoaderFixture, testMethod_getSchedulerNames)
{
    const auto& test_scheduler_names = std::list<std::string>
        {"test_scheduler_1"
        , "test_scheduler_2"
        , "test_scheduler_3"
        };

    // setting of expectations
    {
        auto& mock_scheduler_service = getMockComponent();
        
        EXPECT_CALL(mock_scheduler_service, getSchedulerNames())
            .WillOnce(::testing::Return(test_scheduler_names));
    }
    ::fep3::arya::ISchedulerService* scheduler_service = getComponent();
    ASSERT_NE(nullptr, scheduler_service);
    EXPECT_EQ(test_scheduler_names, scheduler_service->getSchedulerNames());
}

/**
 * Test fixture loading a mocked scheduler service from within a C plugin
 * with additional access to registered schedulers
 */
using SchedulerServiceLoaderWithAccessToSchedulers = MockedComponentCPluginLoader
    <::fep3::ISchedulerService
    , fep3::mock::TransferableSchedulerServiceWithAccessToSchedulers
    , ::fep3::plugin::c::access::arya::SchedulerService
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
    
/**
 * Test fixture class loading a single mocked component from within a C plugin
 */
class SchedulerServiceWithAccessToSchedulersLoaderFixture 
    : public ::testing::Test
    , public SchedulerServiceLoaderWithAccessToSchedulers
{
protected:
    void SetUp() override
    {
        SchedulerServiceLoaderWithAccessToSchedulers::SetUp();
        _get_schedulers_function = getPlugin()->get<fep3::IScheduler*(size_t)>("getScheduler");
        ASSERT_NE(nullptr, _get_schedulers_function);
    }
    
    ::fep3::IScheduler* getScheduler(size_t index) const
    {
        return _get_schedulers_function(index);
    }
private:
    ::fep3::IScheduler*(*_get_schedulers_function)(size_t);
};

/**
 * Test the interface fep3::IScheduler of a scheduler that resides in a C plugin
 * @req_id TODO
 */
TEST_F(SchedulerServiceWithAccessToSchedulersLoaderFixture, testSchedulerInterface)
{
    const auto& test_scheduler_name = std::string("test_scheduler");
    // Note: We are not testing the IJob interface in this test, so we use NiceMock and don't set expectations.
    fep3::arya::Jobs test_jobs
        {{"test_job_1", ::fep3::arya::JobEntry
            {std::make_shared<::testing::NiceMock<::fep3::mock::Job>>()
            , ::fep3::arya::JobInfo
                {"test_job_1"
                , ::fep3::arya::JobConfiguration
                    {::fep3::Duration(1)
                    , ::fep3::Duration(2)
                    , ::fep3::Duration(3)
                    , ::fep3::JobConfiguration::TimeViolationStrategy::unknown
                    , std::vector<std::string>{"other_job_a, other_job_b"}
                    }
                }
            }}
        , {"test_job_2", ::fep3::arya::JobEntry
            {std::make_shared<::testing::NiceMock<::fep3::mock::Job>>()
            , ::fep3::arya::JobInfo
                {"test_job_2"
                , ::fep3::arya::JobConfiguration
                    {::fep3::Duration(11)
                    , ::fep3::Duration(22)
                    , {}
                    , ::fep3::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation
                    , std::vector<std::string>{}
                    }
                }
            }}
        };
    auto mock_scheduler = std::make_unique<::testing::StrictMock<::fep3::mock::Scheduler>>();
    // setting of expectations
    {
        auto& mock_scheduler_service = getMockComponent();
        ::testing::InSequence call_sequence;
        
        EXPECT_CALL(mock_scheduler_service, registerScheduler_(::testing::_))
            .WillOnce(::testing::Return(::fep3::Result{}));
            
        EXPECT_CALL(*mock_scheduler.get(), getName())
            .WillOnce(::testing::Return(test_scheduler_name));
        EXPECT_CALL(*mock_scheduler.get(), initialize(::testing::_, fep3::mock::JobsMatcher(test_jobs)))
            .WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(*mock_scheduler.get(), start())
            .WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(*mock_scheduler.get(), stop())
            .WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(*mock_scheduler.get(), deinitialize())
            .WillOnce(::testing::Return(::fep3::Result{}));
    }
    ::fep3::arya::ISchedulerService* scheduler_service = getComponent();
    ASSERT_NE(nullptr, scheduler_service);
    EXPECT_EQ(::fep3::Result{}, scheduler_service->registerScheduler(std::move(mock_scheduler)));

    const auto& scheduler = getScheduler(0);
    // we registered one scheduler before
    ASSERT_NE(nullptr, scheduler);
    
    // now test the interface IScheduler of a scheduler that resides in a C plugin
    EXPECT_EQ(test_scheduler_name, scheduler->getName());
    fep3::mock::ClockService<fep3::plugin::c::TransferableComponentBase> mock_clock_service;
    EXPECT_EQ(::fep3::Result{}, scheduler->initialize(mock_clock_service, test_jobs));
    EXPECT_EQ(::fep3::Result{}, scheduler->start());
    EXPECT_EQ(::fep3::Result{}, scheduler->stop());
    EXPECT_EQ(::fep3::Result{}, scheduler->deinitialize());
}