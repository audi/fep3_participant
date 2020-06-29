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
#include <thread>
#include <future>

#include <fep3/native_components/scheduler/clock_based/timer_scheduler_impl.h>
#include <fep3/native_components/scheduler/clock_based/local_clock_based_scheduler.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/job_registry/mock/mock_job.h>
#include <fep3/components/job_registry/mock/mock_job_registry.h>

#include <helper/job_registry_helper.h>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ::testing;
using namespace fep3;
using namespace fep3::native;

using Strategy = fep3::JobConfiguration::TimeViolationStrategy;

struct SchedulerTestEnv
{
    SchedulerTestEnv()       
        : _logger(std::make_shared<fep3::mock::Logger>())
        , _set_participant_to_error_state(
            [&]() -> fep3::Result {return _set_participant_to_error_state_mock.Call(); })
    {
      
    }

    template <typename T = fep3::native::JobRunner, typename... Args>
    std::unique_ptr<T> makeChecker(Args&&... args) {
        std::unique_ptr<T> runtime_checker = std::make_unique<fep3::native::JobRunner>(
            std::forward<Args>(args)..., *_logger, _set_participant_to_error_state);
        return runtime_checker;
    }

    std::unique_ptr<fep3::native::JobRunner> makeDefaultChecker()
    {
        auto runtime_checker = std::make_unique<fep3::native::JobRunner>(
            "my_runtime_checker", Strategy::ignore_runtime_violation, 10ms, _logger, _set_participant_to_error_state);
        return runtime_checker;
    }

    std::shared_ptr<fep3::mock::Logger> _logger;
    
    /// necessary because MockFunction.AsStdFunction seems not be available with our gtest
    std::function<fep3::Result()> _set_participant_to_error_state{};
    MockFunction<fep3::Result()> _set_participant_to_error_state_mock;
};


/**
* @brief TimerScheduler is created and one Timer is added and afterwards removed
*
*/
TEST(TimerScheduler, AddRemoveTimer)
{
    fep3::mock::DiscreteSteppingClockService clock_service;
   
    auto timer_scheduler = std::make_shared<TimerScheduler>(clock_service);
    fep3::mock::Job my_job{};

    SchedulerTestEnv scheduler_tester;

    auto runtime_checker = scheduler_tester.makeDefaultChecker();
    auto timer = std::make_shared<TimerThread>("thread_name"
        , my_job, clock_service
        , duration_cast<Duration>(1us)
        , duration_cast<Duration>(0us)
        , *timer_scheduler
        , *runtime_checker);

    ASSERT_FEP3_RESULT(timer_scheduler->addTimer(*timer, duration_cast<Duration>(1us), duration_cast<Duration>(0us)), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(timer_scheduler->removeTimer(*timer), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(timer_scheduler->removeTimer(*timer), fep3::ERR_NOT_FOUND);
}


struct TimerSchedulerFixture : public ::testing::Test
{
    TimerSchedulerFixture()      
    {
    }   

    /**
     * Will create a scheduler and add a TimerThread that will execute @p my_job   
     */
    void setupSchedulerByJob(fep3::test::helper::TestJob& my_job)
    {            
        SchedulerTestEnv runtime_job_tester;
     
        timer_scheduler = std::make_shared<TimerScheduler>(clock_service);
        auto runtime_checker = runtime_job_tester.makeDefaultChecker();
        timer_thread = std::make_shared<TimerThread>("thread_name"
            , my_job
            , clock_service
            , duration_cast<Duration>(my_job._cycle_time)
            , duration_cast<Duration>(0us)
            , *timer_scheduler, *runtime_checker);

        ASSERT_EQ(timer_scheduler->addTimer(*timer_thread
            , my_job.getJobInfo().getConfig()._cycle_sim_time
            , duration_cast<Duration>(0us))
            , fep3::ERR_NOERROR);       
    }   
    
    /**
     * Will push the clock_service in steps of my_job._cycle_time until max_time is reached.
     * Will wait after every time push that a job is executed     
     */
    std::future<void> createSimulateJobUntilMaxTimeFuture(fep3::test::helper::TestJob& my_job, Timestamp max_time)
    {       
        std::future<void> simulate_until_max_time = std::async(std::launch::async, [this,&my_job, max_time]() {     

            // simulate t == 0
            my_job._expected_call_time = 0ns;
            my_job.waitForExpectedCallTime(1s);           

            // simulate until max_time
            auto clock_time = clock_service.getTime();
            while (clock_time < max_time)
            {                  
                ASSERT_LT(my_job._cycle_time, 1ms) << "time_increment has to be less than 1 ms. Otherwise TimerScheduler will wait forever in this Ttstcase";                
                
                clock_service.incrementTime(my_job._cycle_time);
                my_job._expected_call_time += my_job._cycle_time;

                my_job.waitForExpectedCallTime(1s);
                clock_time = clock_service.getTime();
            }
           
            timer_scheduler->stop();
        });
        return simulate_until_max_time;
    }       

     NiceMock<fep3::mock::DiscreteSteppingClockService> clock_service;
     std::shared_ptr<TimerScheduler> timer_scheduler;
     std::shared_ptr<TimerThread> timer_thread;
};

/**
* @brief One Job is added to a TimerScheduler and gets executed by the continuous interface
*
* @detail The TimerScheduler executes one job triggered by a clock mock
* that advances the clock discretly until a defined max_time is reached.
* The actual call times have to be in job_cycle_time resolution
*/
TEST_F(TimerSchedulerFixture, ExecuteOneJobContinuous)
{  
    EXPECT_CALL(clock_service, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::continuous));

    const auto max_time = 10ms;
    const auto job_cycle_time = 500us;

    fep3::test::helper::TestJob my_job("my_job"
        , duration_cast<Duration>(job_cycle_time));      

    setupSchedulerByJob(my_job);
    fep3::IClock::IEventSink& scheduler_as_event_sink = *timer_scheduler;

    // actual test
    {
        ASSERT_FEP3_NOERROR(timer_thread->start());   
        ASSERT_FEP3_NOERROR(timer_scheduler->start());        
       
        /// scheduling will only start if timeReset is emitted
        scheduler_as_event_sink.timeResetBegin(Timestamp(0),Timestamp(0)); 
        scheduler_as_event_sink.timeResetEnd(Timestamp(0)); 

        // this will start the scheduler
        std::future<void> call_execute = std::async(std::launch::async, [&]() {
            fep3::IJob& scheduler_as_job = *timer_scheduler;
            ASSERT_EQ(scheduler_as_job.execute(clock_service.getTime()), fep3::ERR_NOERROR);
        });

        // this will push the clock until 10ms in 500us steps
        std::future<void> simulate_until_max_time = createSimulateJobUntilMaxTimeFuture(my_job, max_time);

        simulate_until_max_time.get();
        call_execute.get();

        my_job.assertCallTimeResolution();
        my_job.assertNumberOfCalls(max_time);

        timer_scheduler->stop();
    }
}


/**
* @brief One Job is added to a TimerScheduler and gets executed by the discrete interface (calls to IEventSink)
*/
TEST_F(TimerSchedulerFixture, ExecuteOneJobDiscrete)
{ 
    EXPECT_CALL(clock_service, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::discrete));

    const auto max_time = 10ms;
    const auto job_cycle_time = 500us;

    fep3::test::helper::TestJob my_job("my_job"
        , duration_cast<Duration>(job_cycle_time));     

    setupSchedulerByJob(my_job);
    fep3::IClock::IEventSink& scheduler_as_event_sink = *timer_scheduler;

    // actual test
    {       
        ASSERT_FEP3_NOERROR(timer_scheduler->start());  
        ASSERT_FEP3_NOERROR(timer_thread->start());        
       
        /// scheduling will only start if timeReset is emitted
        scheduler_as_event_sink.timeResetBegin(Timestamp(0),Timestamp(0)); 
        scheduler_as_event_sink.timeResetEnd(Timestamp(0)); 

        // drive scheduler discrete by calling timeUpdating
        auto time = Timestamp(0);
        while (time < max_time)
        {            
            time += job_cycle_time;
            scheduler_as_event_sink.timeUpdating(time);                  
        }
     
        my_job.assertCallTimeResolution();
        my_job.assertNumberOfCalls(max_time);

        timer_scheduler->stop();
    }
}