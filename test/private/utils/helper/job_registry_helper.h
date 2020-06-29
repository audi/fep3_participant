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

#include <condition_variable>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <fep3/core/job.h>

namespace fep3
{
namespace test
{
namespace helper
{

struct SimpleJobBuilder
{
    SimpleJobBuilder(const std::string job_name = "my_job", Duration cycle_time = Duration(1)) :
        _job_name(job_name),
        _cycle_time(cycle_time)
    {
    }

    template<typename T>
    std::shared_ptr<T> makeJob(Timestamp expected_call_time) const
    {
        auto my_job = std::shared_ptr<T>(new T(_job_name, _cycle_time, expected_call_time));
        return my_job;
    }

    template<typename T>
    std::shared_ptr<T> makeJob() const
    {
        auto my_job = std::shared_ptr<T>(new T(_job_name, _cycle_time));
        return my_job;
    }

    std::shared_ptr<fep3::core::Job> makeJob() const
    {
        auto my_job = std::shared_ptr<fep3::core::Job>(new fep3::core::Job(_job_name, _cycle_time));
        return my_job;
    }

    fep3::JobInfo makeJobInfo() const
    {
        return fep3::JobInfo(_job_name, _cycle_time);
    }

    fep3::JobConfiguration makeJobConfig() const
    {
        const fep3::JobConfiguration job_config(_cycle_time);
        return job_config;
    }

    const std::string _job_name;
    const Duration _cycle_time;
};       

class TestJob : public fep3::core::Job
{
public:
    TestJob(std::string name, Duration cycle_time, Timestamp expected_call_time = Timestamp(std::chrono::nanoseconds(0)))
        : fep3::core::Job(name, cycle_time)
        , _expected_call_time(expected_call_time)
        , _cycle_time(cycle_time)
    {

    }

    virtual fep3::Result execute(Timestamp time_of_execution) override
    {
        std::unique_lock<std::mutex> lk(_my_mutex);

        _calls.push_back(time_of_execution);
        //std::cout << "Job was called with time " << time_of_execution.count() << std::endl;
        if (_calls.size() > 0 && _calls.back() >= _expected_call_time)
        {
            _expected_calls_reached.notify_all();
        }
        return{};
    }

    void assertNumberOfCalls(Timestamp max_time)
    {
        std::unique_lock<std::mutex> lk(_my_mutex);

        EXPECT_EQ(_calls.size(), (max_time / _cycle_time) + 1);
    }

    void assertNumberOfCalls(size_t expected_calls)
    {
        std::unique_lock<std::mutex> lk(_my_mutex);

        EXPECT_EQ(_calls.size(), expected_calls);
    }

    void assertCallTimeResolution(Timestamp allowed_step_deviation = Timestamp(0)) const
    {
        using namespace std::chrono;

        std::unique_lock<std::mutex> lk(_my_mutex);

        Timestamp time_expected = 0us;
        const auto cycle_time = getJobInfo().getConfig()._cycle_sim_time;

        for (const auto& time_actual : _calls)
        {
            EXPECT_NEAR(
                static_cast<double>(time_actual.count()),
                static_cast<double>(time_expected.count()),
                static_cast<double>(allowed_step_deviation.count()));

            time_expected += cycle_time;
        }
    }

    void assertSteadilyRisingCallTimes() const
    {
        std::unique_lock<std::mutex> lk(_my_mutex);

        Timestamp last_call_time{-1};     
  
        for (const auto& time_actual : _calls)
        {
            
            ASSERT_GT(time_actual.count(), last_call_time.count());

            last_call_time = time_actual;
        }
    }

    void waitForExpectedCallTime(Timestamp timeout)
    {
        using namespace std::chrono;
        std::unique_lock<std::mutex> lk(_my_mutex);

        if (!expectedCallTimeReached())
        {
            ASSERT_NE(_expected_calls_reached.wait_for(lk, timeout), std::cv_status::timeout);
        }
    }
                
public:
    Timestamp _expected_call_time;
    const Duration _cycle_time;

private:
    std::vector<Timestamp> _calls{};
 
    std::condition_variable _expected_calls_reached;
    mutable std::mutex _my_mutex;                

    bool expectedCallTimeReached()
    {
        return _calls.size() > 0 && _calls.back() >= _expected_call_time;
    }
};

class SleepingJob : public TestJob
{
public:
    SleepingJob(std::string name, Duration cycle_time, Duration sleep_time = Duration(0), Timestamp expected_call_time = Timestamp(0)) :
        TestJob(name, cycle_time, expected_call_time),
        _sleep_time(sleep_time)
    {
    }

    fep3::Result execute(Timestamp time_of_execution) override
    {
        auto result = TestJob::execute(time_of_execution);
        std::this_thread::sleep_for(_sleep_time);
        return result;
    }
private:
    Duration _sleep_time;
};

}
}
}