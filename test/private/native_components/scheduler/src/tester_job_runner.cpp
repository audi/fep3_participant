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
#include <thread>
#include <common/gtest_asserts.h>

#include <fep3/native_components/scheduler/job_runner.h>
#include <fep3/components/job_registry/mock/mock_job.h>
#include <fep3/components/job_registry/mock/mock_job_registry.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <helper/job_registry_helper.h>
#include <fep3/fep3_duration.h>

using namespace std::chrono;
using namespace ::testing;
using namespace std::chrono_literals;
using namespace fep3::test;
using namespace fep3;


using Strategy = fep3::JobConfiguration::TimeViolationStrategy;


struct RuntimeJobEnv
{
    RuntimeJobEnv()
        : _logger(std::make_shared<NiceMock<fep3::mock::Logger>>())
        , _set_participant_to_error_state(
            [this]() -> fep3::Result {return _set_participant_to_error_state_mock.Call(); })
    {     
    }

    template <typename T = fep3::native::JobRunner, typename... Args>
    std::unique_ptr<T> makeChecker(Args&&... args) 
    {
        std::unique_ptr<T> runtime_checker = std::unique_ptr<T>(
            new T(std::forward<Args>(args)..., _logger, _set_participant_to_error_state));
        return runtime_checker;
    }

    std::shared_ptr<NiceMock<fep3::mock::Logger>> _logger;

    /// necessary because MockFunction.AsStdFunction seems not be available with our gtest
    std::function<fep3::Result()> _set_participant_to_error_state{}; 
    MockFunction<fep3::Result()> _set_participant_to_error_state_mock{};
};


/**
* @brief Tests that incident is thrown if error in executeDataIn occurs
*
*/
TEST(JobRunner, IncidentOnExecuteInError)
 {
    RuntimeJobEnv runtime_job_env;

    NiceMock<fep3::mock::Job> my_job{};
    ON_CALL(my_job, execute(_))
        .WillByDefault(Return(a_util::result::Result()));
    ON_CALL(my_job, executeDataOut(_))
        .WillByDefault(Return(a_util::result::Result()));

    // actual test
    {
        auto runtime_checker = runtime_job_env.makeChecker("my_runtime_checker", Strategy::ignore_runtime_violation, 10ms);

        fep3::Timestamp call_time = 2us;
        EXPECT_CALL(my_job, executeDataIn(call_time)).WillOnce(Return(::fep3::ERR_FAILED));
        EXPECT_CALL(*runtime_job_env._logger, logWarning(_)).WillOnce(Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(runtime_checker->runJob(call_time, my_job), a_util::result::Result());       
    }
}


/**
* @brief Tests that the jobs execute.. functions are called correctly
* @req_id FEPSDK-2095, FEPSDK-2096, FEPSDK-2097
*/
TEST(JobRunner, JobIsCalledCorrectly)
{
    RuntimeJobEnv runtime_job_env;

    NiceMock<fep3::mock::Job> my_job{};

    // actual test
    {        
        auto runtime_checker = runtime_job_env.makeChecker("my_runtime_checker", Strategy::set_stm_to_error, 10ms);

        EXPECT_CALL(runtime_job_env._set_participant_to_error_state_mock, Call()).Times(0);

        fep3::Timestamp call_time = 2ms;
        EXPECT_CALL(my_job, executeDataIn(call_time)).WillOnce(Return(::fep3::Result{}));
        EXPECT_CALL(my_job, execute(call_time)).WillOnce(Return(::fep3::Result{}));
        EXPECT_CALL(my_job, executeDataOut(call_time)).WillOnce(Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(runtime_checker->runJob(call_time, my_job), fep3::Result{});      
    }
}

/**
* @brief Tests that the max_runtime check actually works if job runs shorter than max_runtime
* @req_id FEPSDK-2089
*/
TEST(JobRunner, RunsShorterThanMaxRuntime)
{
    auto max_runtime = 1s;
    auto actual_runtime = 1ms;    
    ASSERT_LT(actual_runtime, max_runtime);

    RuntimeJobEnv runtime_job_env;

    // actual test
    {
        helper::SleepingJob my_job("my_job", duration_cast<Duration>(1us), duration_cast<Duration>(actual_runtime));
        auto runtime_checker = runtime_job_env.makeChecker("my_runtime_checker", Strategy::set_stm_to_error, max_runtime);
        
        EXPECT_CALL(runtime_job_env._set_participant_to_error_state_mock, Call()).Times(0);

        EXPECT_CALL(*runtime_job_env._logger, logError(_)).Times(0);
        EXPECT_CALL(*runtime_job_env._logger, logWarning(_)).Times(0);

        ASSERT_FEP3_RESULT(runtime_checker->runJob(2ms, my_job), a_util::result::Result());        
    }
}


/**
* @brief Tests that the max_runtime check actually works if job runs longer than max_runtime
* @req_id FEPSDK-2089
*/
TEST(JobRunner, RunsLongerThanMaxRuntime)
{   
    auto max_runtime = 1ms;
    auto actual_runtime = 10ms;
    ASSERT_GT(actual_runtime, max_runtime);

    RuntimeJobEnv runtime_job_env;

    // actual test
    {        
        helper::SleepingJob my_job("my_job", duration_cast<Duration>(1ms), duration_cast<Duration>(actual_runtime));
        auto runtime_checker = runtime_job_env.makeChecker("my_runtime_checker", Strategy::set_stm_to_error, max_runtime);        

        EXPECT_CALL(*runtime_job_env._logger, logError(_)).WillOnce(Return(::fep3::Result{}));
        EXPECT_CALL(runtime_job_env._set_participant_to_error_state_mock, Call()).WillOnce(Return(fep3::Result{}));

        ASSERT_FEP3_RESULT_WITH_MESSAGE(runtime_checker->runJob(2ms, my_job), fep3::ERR_FAILED, ".*Computation time .* exceeded configured maximum runtime.*");       
    }
}


/**
* @brief Tests that a Warning is issued if strategy warn_about_runtime_violation is used
* @req_id FEPSDK-2092
*/
TEST(JobRunner, RuntimeViolationWarningEmitted)
{  
    auto max_runtime = 1ms;
    auto actual_runtime = 10ms;
    ASSERT_GT(actual_runtime, max_runtime);

    RuntimeJobEnv runtime_job_env;

    // actual test
    {
        helper::SleepingJob my_job("my_job", duration_cast<Duration>(1ms), duration_cast<Duration>(actual_runtime));
        auto runtime_checker = runtime_job_env.makeChecker("my_runtime_checker", Strategy::warn_about_runtime_violation, max_runtime);

        EXPECT_CALL(runtime_job_env._set_participant_to_error_state_mock, Call()).Times(0);
        EXPECT_CALL(*runtime_job_env._logger,
            logWarning(ContainsRegex("Computation time .* exceeded configured maximum runtime."))).WillOnce(Return(::fep3::Result{}));

        ASSERT_EQ(runtime_checker->runJob(2ms, my_job), a_util::result::Result());      
    }
}

/**
* @brief Tests that a runtime violation is ignored if TS_IGNORE_RUNTIME_VIOLATION is used
* @req_id FEPSDK-2091
*/
TEST(JobRunner, RuntimeViolationIgnore)
{  
    auto max_runtime = 1ms;
    auto actual_runtime = 10ms;
    ASSERT_GT(actual_runtime, max_runtime);

    RuntimeJobEnv runtime_job_env;

    // actual test
    {
        helper::SleepingJob my_job("my_job", duration_cast<Duration>(1ms), duration_cast<Duration>(actual_runtime));
        auto runtime_checker = runtime_job_env.makeChecker("my_runtime_checker", Strategy::ignore_runtime_violation, max_runtime);

        EXPECT_CALL(runtime_job_env._set_participant_to_error_state_mock, Call()).Times(0);
        EXPECT_CALL(*runtime_job_env._logger, logWarning(_)).Times(0);
        
        ASSERT_EQ(runtime_checker->runJob(2ms, my_job), a_util::result::Result());      
    }
}


/**
* @brief Tests that to_error_func() is called if strategy set_stm_to_error is used
* @req_id FEPSDK-2094
*/
TEST(JobRunner, RuntimeViolationLeadsToStateError)
{
    auto max_runtime = 1ms;
    auto actual_runtime = 10ms;
    ASSERT_GT(actual_runtime, max_runtime);

    RuntimeJobEnv runtime_job_env;

    // actual test
    {
        helper::SleepingJob my_job("my_job", duration_cast<Duration>(1ms), duration_cast<Duration>(actual_runtime));
        auto runtime_checker = runtime_job_env.makeChecker("my_runtime_checker", Strategy::set_stm_to_error, max_runtime);        

		EXPECT_CALL(runtime_job_env._set_participant_to_error_state_mock, Call()).WillOnce(Return(fep3::Result{}));
        EXPECT_CALL(*runtime_job_env._logger,
            logError(ContainsRegex("Computation time.*exceeded configured maximum runtime."))).WillOnce(Return(::fep3::Result{}));

        auto result = runtime_checker->runJob(2ms, my_job);

        ASSERT_FEP3_RESULT_WITH_MESSAGE(result, fep3::ERR_FAILED, ".*Computation time .* exceeded configured maximum runtime.*");     
    }
}



/**
* @brief Tests that no runtime check is done, if max_runtime is not set to a duration
* @req_id FEPSDK-2090
*/
TEST(JobRunner, NoCheckOnMaxRuntimeIsZero)
{
    fep3::Optional<Duration> max_runtime = {};    
    auto actual_runtime = 1ms;    

    RuntimeJobEnv runtime_job_env;

    // actual test
    {
        helper::SleepingJob my_job("my_job", duration_cast<Duration>(1ms), duration_cast<Duration>(actual_runtime));
        
        /// this cast is used because of bug FEPSDK-2126
        auto runtime_checker = runtime_job_env.makeChecker(
            "my_runtime_checker",
            Strategy::set_stm_to_error,
            max_runtime);

        EXPECT_CALL(runtime_job_env._set_participant_to_error_state_mock, Call()).Times(0);
        EXPECT_CALL(*runtime_job_env._logger, logError(_)).Times(0);

        ASSERT_FEP3_NOERROR(runtime_checker->runJob(2ms, my_job));       
    }
}


/**
* @brief Tests that the method executeDataOut of a job is not called if TS_SKIP_OUTPUT_PUBLISH is applied
* @req_id FEPSDK-2093
*/
TEST(JobRunner, RuntimeViolationSkipPublish)
{  
    auto max_runtime = 1ms;
    auto actual_runtime = 10ms;
    ASSERT_GT(actual_runtime, max_runtime);

    RuntimeJobEnv runtime_job_env;
    NiceMock<fep3::mock::Job> my_job{};

    EXPECT_CALL(runtime_job_env._set_participant_to_error_state_mock, Call()).Times(0);
    EXPECT_CALL(*runtime_job_env._logger, logWarning(_)).Times(0);

    // actual test
    {
        auto runtime_checker = runtime_job_env.makeChecker("my_runtime_checker", Strategy::skip_output_publish, max_runtime);

		EXPECT_CALL(*runtime_job_env._logger, logError(_)).WillOnce(Return(::fep3::Result{}));
        EXPECT_CALL(my_job, executeDataIn(_)).WillOnce(Return(::fep3::Result{}));
        EXPECT_CALL(my_job, execute(_)).WillOnce(            
            InvokeWithoutArgs([&actual_runtime](){ std::this_thread::sleep_for(actual_runtime); return ::fep3::Result{}; }) );

        EXPECT_CALL(my_job, executeDataOut(_)).Times(0);
        
        ASSERT_EQ(runtime_checker->runJob(2ms, my_job), a_util::result::Result());      
    }
}