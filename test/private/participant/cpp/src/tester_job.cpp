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

#include <fep3/core/job.h>
#include <fep3/components/job_registry/mock/mock_job_registry.h>
#include <fep3/components/base/component_registry.h>

using namespace ::testing;
using namespace fep3::arya;
using namespace fep3::core::arya;


struct JobComponentRegistryWithJobRegistry : ::testing::Test
{
    void SetUp() override {
        _component_registry = std::make_shared<fep3::ComponentRegistry>();

        createComponents();
        setComponents();

        ASSERT_FEP3_RESULT(_component_registry->create(), fep3::ERR_NOERROR);
    }

    void createComponents()
    {    
        auto job_registry = std::make_unique<fep3::mock::JobRegistryComponent<>>();
        ASSERT_FEP3_RESULT(_component_registry->registerComponent<fep3::IJobRegistry>(std::move(job_registry)), fep3::ERR_NOERROR);
    }

    void setComponents()
    {
        _job_registry_mock = _component_registry->getComponent<fep3::mock::JobRegistry>();
        ASSERT_NE(_job_registry_mock, nullptr);
    }
  
    fep3::mock::JobRegistry* _job_registry_mock;
    std::shared_ptr<fep3::ComponentRegistry> _component_registry;  
};

struct JobComponentRegistryWithoutJobRegistry : ::testing::Test
{
    void SetUp() override {
        _component_registry = std::make_shared<fep3::ComponentRegistry>();    

        ASSERT_FEP3_RESULT(_component_registry->create(), fep3::ERR_NOERROR);
    }

    std::shared_ptr<fep3::ComponentRegistry> _component_registry;
};


/**
* @brief Jobs will be added and removed
*
*/
TEST_F(JobComponentRegistryWithJobRegistry, JobRegistryWillBeCalled)
{
    { // add
        EXPECT_CALL(*_job_registry_mock, addJob(_, _, _)).Times(2).WillRepeatedly(::testing::Return(fep3::Result{}));

        const std::vector<std::shared_ptr<Job>> jobs{
            std::make_shared<Job>("Job1", Duration(1)),
            std::make_shared<Job>("Job2", Duration(1))
        };
        ASSERT_FEP3_NOERROR(fep3::core::arya::addToComponents(jobs, *_component_registry));
    }

    { // remove
        EXPECT_CALL(*_job_registry_mock, removeJob(_)).Times(2).WillRepeatedly(::testing::Return(fep3::Result{}));

        const std::vector<std::string> job_names{ "Job1","Job2" };
        ASSERT_FEP3_NOERROR(removeFromComponents(job_names, *_component_registry));
    }
}

/**
* @brief JobRegistry can not be found, therefore an error is returned
*
*/
TEST_F(JobComponentRegistryWithoutJobRegistry, ErrorRetrievingJobRegistry)
{
    { // add     
        const std::vector<std::shared_ptr<Job>> jobs{
            std::make_shared<Job>("Job1", Duration(1)),
            std::make_shared<Job>("Job2", Duration(1))
        };
        ASSERT_EQ(addToComponents(jobs, *_component_registry), fep3::ERR_NO_INTERFACE);
    }

    { // remove
        const std::vector<std::string> job_names{ "Job1","Job2" };
        ASSERT_EQ(removeFromComponents(job_names, *_component_registry), fep3::ERR_NO_INTERFACE);
    }
}

/**
 * @brief Two jobs are added with success
 * 
 */
TEST(Job, AddJobWithSuccesss)
{
    auto job_registry = std::make_unique<fep3::mock::JobRegistry>();

    EXPECT_CALL(*job_registry, addJob(_,_,_)).Times(2).WillRepeatedly(::testing::Return(fep3::Result{}));

    const std::vector<std::shared_ptr<Job>> jobs{
        std::make_shared<Job>("Job1", Duration(1)), 
        std::make_shared<Job>("Job2", Duration(1)) 
        };
    ASSERT_FEP3_NOERROR(fep3::core::arya::addJobsToJobRegistry(jobs, *job_registry));
}

/**
* @brief Three jobs are added. Adding fails with 2 second job
* @details Since adding Job2 returns an error Job2 and Job3 are not added
*/
TEST(Job, AddJobsWithError)
{
    auto job_registry = std::make_unique<fep3::mock::JobRegistry>();

    EXPECT_CALL(*job_registry, addJob(_, _, _))
        .Times(2)
        .WillOnce(::testing::Return(fep3::Result{}))
        .WillOnce(::testing::Return(CREATE_ERROR_DESCRIPTION(fep3::ERR_FAILED, "error adding Job2")));
    
    const std::vector<std::shared_ptr<Job>> jobs{
            std::make_shared<Job>("Job1", Duration(1)),
            std::make_shared<Job>("Job2", Duration(1)),
            std::make_shared<Job>("Job3", Duration(1))
            };
    ASSERT_FEP3_RESULT_WITH_MESSAGE(addJobsToJobRegistry(jobs, *job_registry), fep3::ERR_FAILED, "error adding Job2");
}

/**
* @brief Two jobs are added with success
*
*/
TEST(Job, RemoveJobsWithSuccess)
{
    auto job_registry = std::make_unique<fep3::mock::JobRegistry>();

    EXPECT_CALL(*job_registry, removeJob(_)).Times(2).WillRepeatedly(::testing::Return(fep3::Result{}));

    const std::vector<std::string> job_names{"Job1","Job2"};
    ASSERT_FEP3_NOERROR(removeJobsFromJobRegistry(job_names, *job_registry));
}

/**
* @brief Four jobs are removed. Removing fails for Job2 and Job4
* @details Job1 and Job3 will be removed. 
*               The last error code will be returned.
*               A message containing of error description for Job2 and Job4 will be created.
*/
TEST(Job, RemoveJobsWithError)
{
    auto job_registry = std::make_unique<fep3::mock::JobRegistry>();

    EXPECT_CALL(*job_registry, removeJob(_))
        .Times(4)
        .WillOnce(::testing::Return(fep3::Result{}))
        .WillOnce(::testing::Return(CREATE_ERROR_DESCRIPTION(fep3::ERR_FAILED, "error adding Job2")))
        .WillOnce(::testing::Return(fep3::Result{}))
        .WillOnce(::testing::Return(CREATE_ERROR_DESCRIPTION(fep3::ERR_CANCELLED, "error adding Job4")));

    const std::vector<std::string> job_names{ "Job1","Job2","job3","job4" };
    ASSERT_FEP3_RESULT_WITH_MESSAGE(removeJobsFromJobRegistry(job_names, *job_registry), fep3::ERR_FAILED, "error adding Job2; error adding Job4");
}