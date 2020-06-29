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
#include <fep3/components/job_registry/mock/mock_transferable_job_registry_with_access_to_jobs.h>
#include <fep3/components/job_registry/mock/mock_job.h>
#include <fep3/components/job_registry/c_access_wrapper/job_registry_c_access_wrapper.h>
#include <helper/component_c_plugin_helper.h>

const std::string test_plugin_1_path = PLUGIN;

using namespace fep3::plugin::c::arya;
using namespace fep3::plugin::c::access::arya;

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
        return "setMockJobRegistry";
    }
};

/**
 * Test fixture loading a mocked job registry from within a C plugin
 */
using JobRegistryLoader = MockedComponentCPluginLoader
    <fep3::IJobRegistry
    , fep3::mock::TransferableJobRegistryWithAccessToJobs
    , fep3::plugin::c::access::arya::JobRegistry
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
using JobRegistryLoaderFixture = MockedComponentCPluginLoaderFixture<JobRegistryLoader>;

/**
 * Test method fep3::IJobRegistry::addJob of a job registry that resides in a C plugin
 * @req_id TODO
 */
TEST_F(JobRegistryLoaderFixture, testMethod_addJob)
{
    const auto& test_job_name = std::string("test_job");
    const auto& test_job_configuration = fep3::arya::JobConfiguration
        {fep3::arya::Duration{1}
        , fep3::arya::Duration{2}
        , fep3::arya::Duration{3}
        , fep3::arya::JobConfiguration::TimeViolationStrategy::skip_output_publish
        , {"other_job_a", "other_job_b"}
        };
    // Note: There is no way to check if the jobs as added to the job registry (residing in a C plugin)
    // are referring to the correct original jobs except triggering them and checking if the original
    // jobs are executed correctly. Thus we use a StrictMock of the job and check if executeDataIn
    // is executed correctly (see below).
    const auto& mock_job = std::make_shared<::testing::StrictMock<fep3::mock::Job>>();
    
    auto& mock_job_registry = getMockComponent();
    // setting of expectations
    {
        ::testing::InSequence call_sequence;
        
        // Note: Using a gMock matcher would suit better here than an action, but a matcher 
        // must not have any side effects and invoking a mock method has a side effect.
        EXPECT_CALL(mock_job_registry, addJob(test_job_name, ::testing::_, test_job_configuration))
            .WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(*mock_job.get(), executeDataIn(::fep3::Timestamp(1)))
            .WillOnce(::testing::Return(::fep3::Result{}));
    }
    fep3::arya::IJobRegistry* job_registry = getComponent();
    ASSERT_NE(nullptr, job_registry);
    EXPECT_EQ(fep3::Result{}, job_registry->addJob(test_job_name, std::move(mock_job), test_job_configuration));

    const auto& jobs_of_job_registry = mock_job_registry.getJobPointers();
    ASSERT_EQ(1, jobs_of_job_registry.size());
    ::fep3::IJob* job_of_job_registry = *jobs_of_job_registry.begin();
    
    // now test the interface IJob of a job that resides in a C plugin
    EXPECT_EQ(::fep3::Result{}, job_of_job_registry->executeDataIn(::fep3::Timestamp(1)));
}

/**
 * Test method fep3::IJobRegistry::removeJob of a job registry that resides in a C plugin
 * @req_id TODO
 */
TEST_F(JobRegistryLoaderFixture, testMethod_removeJob)
{
    const auto& test_job_name = std::string("test_job");
    
    // setting of expectations
    {
        auto& mock_job_registry = getMockComponent();

        EXPECT_CALL(mock_job_registry, removeJob(test_job_name))
            .WillOnce(::testing::Return(fep3::Result{}));
    }
    fep3::arya::IJobRegistry* job_registry = getComponent();
    ASSERT_NE(nullptr, job_registry);
    EXPECT_EQ(fep3::Result{}, job_registry->removeJob(test_job_name));
}

/**
 * Test method fep3::IJobRegistry::getJobInfos of a job registry that resides in a C plugin
 * @req_id TODO
 */
TEST_F(JobRegistryLoaderFixture, testMethod_getJobInfos)
{
    const auto& test_job_infos = std::list<fep3::JobInfo>
        {fep3::JobInfo
            {"test_job_1"
            , fep3::JobConfiguration
                {fep3::arya::Duration{1}
                , fep3::arya::Duration{1}
                , fep3::arya::Duration{111}
                , fep3::arya::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation
                , {"other_job_1", "other_job_11"}
                }
            }
        , fep3::JobInfo
            {"test_job_2"
            , fep3::JobConfiguration
                {fep3::arya::Duration{2}
                , fep3::arya::Duration{22}
                , fep3::arya::Duration{222}
                , fep3::arya::JobConfiguration::TimeViolationStrategy::warn_about_runtime_violation
                , {"other_job_2", "other_job_22"}
                }
            }
        , fep3::JobInfo
            {"test_job_3"
            , fep3::JobConfiguration
                {fep3::arya::Duration{3}
                , fep3::arya::Duration{33}
                , fep3::arya::Duration{333}
                , fep3::arya::JobConfiguration::TimeViolationStrategy::skip_output_publish
                , {"other_job_3", "other_job_33"}
                }
            }
        };

    // setting of expectations
    {
        auto& mock_job_registry = getMockComponent();
        
        EXPECT_CALL(mock_job_registry, getJobInfos())
            .WillOnce(::testing::Return(test_job_infos));
    }
    fep3::arya::IJobRegistry* job_registry = getComponent();
    ASSERT_NE(nullptr, job_registry);
    EXPECT_EQ(test_job_infos, job_registry->getJobInfos());
}

/**
 * Test method fep3::IJobRegistry::getJobss of a job registry that resides in a C plugin
 * @req_id TODO
 */
TEST_F(JobRegistryLoaderFixture, testMethod_getJobs)
{
    // Note: There is no way to check if the jobs as returned from the job registry (residing in a C plugin)
    // are referring to the correct original jobs except triggering them and checking if the original
    // jobs are executed correctly. Thus we use a StrictMock of the job and check if executeDataIn
    // is executed correctly (see below).
    const auto& mock_job_1 = std::make_shared<::testing::StrictMock<fep3::mock::Job>>();
    const auto& mock_job_2 = std::make_shared<::testing::StrictMock<fep3::mock::Job>>();
    const auto& mock_job_3 = std::make_shared<::testing::StrictMock<fep3::mock::Job>>();
    const auto& test_reference_jobs = fep3::Jobs
        {fep3::Jobs::value_type
            {"test_job_1"
            , fep3::JobEntry
                {mock_job_1
                , fep3::JobInfo
                    {"test_job_1"
                    , fep3::JobConfiguration
                        {fep3::arya::Duration{1}
                        , fep3::arya::Duration{11}
                        , fep3::arya::Duration{111}
                        , fep3::arya::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation
                        , {"other_job_1", "other_job_11"}
                        }
                    }
                }
            }
        , fep3::Jobs::value_type
            {"test_job_2"
            , fep3::JobEntry
                {mock_job_2
                , fep3::JobInfo
                    {"test_job_2"
                    , fep3::JobConfiguration
                        {fep3::arya::Duration{2}
                        , fep3::arya::Duration{22}
                        , fep3::arya::Duration{222}
                        , fep3::arya::JobConfiguration::TimeViolationStrategy::warn_about_runtime_violation
                        , {"other_job_2", "other_job_22"}
                        }
                    }
                }
            }
        , fep3::Jobs::value_type
            {"test_job_3"
            , fep3::JobEntry
                {mock_job_3
                , fep3::JobInfo
                    {"test_job_3"
                    , fep3::JobConfiguration
                        {fep3::arya::Duration{3}
                        , fep3::arya::Duration{33}
                        , fep3::arya::Duration{333}
                        , fep3::arya::JobConfiguration::TimeViolationStrategy::skip_output_publish
                        , {"other_job_3", "other_job_33"}
                        }
                    }
                }
            }
        };

    // setting of expectations
    {
        ::testing::InSequence call_sequence;
        
        auto& mock_job_registry = getMockComponent();
        
        EXPECT_CALL(mock_job_registry, getJobs())
            .WillOnce(::testing::Return(test_reference_jobs));

        EXPECT_CALL(*mock_job_1.get(), executeDataIn(::fep3::Timestamp(1)))
            .WillOnce(::testing::Return(::fep3::Result{}));
        
        EXPECT_CALL(*mock_job_2.get(), executeDataIn(::fep3::Timestamp(2)))
            .WillOnce(::testing::Return(::fep3::Result{}));

        EXPECT_CALL(*mock_job_3.get(), executeDataIn(::fep3::Timestamp(3)))
            .WillOnce(::testing::Return(::fep3::Result{}));
    }

    fep3::arya::IJobRegistry* job_registry = getComponent();
    ASSERT_NE(nullptr, job_registry);
    const auto& jobs = job_registry->getJobs();
    ASSERT_EQ(test_reference_jobs.size(), jobs.size());
    auto test_reference_job_iter = test_reference_jobs.begin();
    auto job_iter = jobs.begin();
    for
        (uint32_t job_index = 0
        ; test_reference_job_iter != test_reference_jobs.end() && job_iter != jobs.end()
        ; ++job_index, ++test_reference_job_iter, ++job_iter
        )
    {
        EXPECT_EQ(test_reference_job_iter->first, job_iter->first);
        const fep3::JobEntry& test_reference_job_entry = test_reference_job_iter->second;
        const fep3::JobEntry& job_entry = job_iter->second;
        // note: the pointers to the job are actually differing, because one side is the pointer to the wrapped job
        //EXPECT_EQ(test_reference_job_entry.job, job_entry.job)
        // instead we trigger the jobs to check if the corresponding wrapped job is executed correclty (via expectations, see above)
        EXPECT_EQ(::fep3::Result{}, job_entry.job->executeDataIn(::fep3::Timestamp(job_index + 1)));
        const fep3::JobInfo& test_reference_job_info = test_reference_job_entry.job_info;
        const fep3::JobInfo& job_info = job_entry.job_info;
        EXPECT_EQ(test_reference_job_info, job_info);
    }

    // break reference cylces of mock jobs
    testing::Mock::VerifyAndClearExpectations(mock_job_1.get());
    testing::Mock::VerifyAndClearExpectations(mock_job_2.get());
    testing::Mock::VerifyAndClearExpectations(mock_job_3.get());
}

/**
 * Test the interface fep3::IJob of a job that resides in a C plugin
 * @req_id TODO
 */
TEST_F(JobRegistryLoaderFixture, testJobInterface)
{
    const auto& test_job_name = std::string("test_job");
    const auto& test_job_configuration = fep3::arya::JobConfiguration
        {fep3::arya::Duration{1}
        , fep3::arya::Duration{2}
        , fep3::arya::Duration{3}
        , fep3::arya::JobConfiguration::TimeViolationStrategy::skip_output_publish
        , {"other_job_a", "other_job_b"}
        };

    const auto& mock_job = std::make_shared<::testing::StrictMock<fep3::mock::Job>>();

    auto& mock_job_registry = getMockComponent();
    // setting of expectations
    {
        ::testing::InSequence call_sequence;
        
        // Note: Using a gMock matcher would suit better here than an action, but a matcher 
        // must not have any side effects and invoking a mock method has a side effect.
        EXPECT_CALL(mock_job_registry, addJob(test_job_name, ::testing::_, test_job_configuration))
            .WillOnce(::testing::Return(::fep3::Result{}));

        EXPECT_CALL(*mock_job.get(), executeDataIn(::fep3::Timestamp(1)))
            .WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(*mock_job.get(), execute(::fep3::Timestamp(2)))
            .WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(*mock_job.get(), executeDataOut(::fep3::Timestamp(3)))
            .WillOnce(::testing::Return(::fep3::Result{}));
    }
    fep3::arya::IJobRegistry* job_registry = getComponent();
    ASSERT_NE(nullptr, job_registry);
    EXPECT_EQ(fep3::Result{}, job_registry->addJob(test_job_name, std::move(mock_job), test_job_configuration));

    const auto& jobs_of_job_registry = mock_job_registry.getJobPointers();
    ASSERT_EQ(1, jobs_of_job_registry.size());
    ::fep3::IJob* job_of_job_registry = *jobs_of_job_registry.begin();
    
    // now test the interface IJob of a job that resides in a C plugin
    EXPECT_EQ(::fep3::Result{}, job_of_job_registry->executeDataIn(::fep3::Timestamp(1)));
    EXPECT_EQ(::fep3::Result{}, job_of_job_registry->execute(::fep3::Timestamp(2)));
    EXPECT_EQ(::fep3::Result{}, job_of_job_registry->executeDataOut(::fep3::Timestamp(3)));
}