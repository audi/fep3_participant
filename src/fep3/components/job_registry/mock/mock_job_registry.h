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

#include <gmock/gmock.h>

#include <fep3/components/base/component_base.h>
#include <fep3/components/job_registry/job_registry_intf.h>

namespace fep3
{
namespace mock
{

struct JobRegistry : public IJobRegistry
{
    MOCK_METHOD3(addJob, fep3::Result(const std::string&, const std::shared_ptr<IJob>&, const JobConfiguration&));
    MOCK_METHOD1(removeJob, fep3::Result(const std::string&));
    MOCK_CONST_METHOD0(getJobInfos, std::list<JobInfo>());
    MOCK_CONST_METHOD0(getJobs, Jobs());
};

struct JobRegistryComponentBase : public ComponentBase<IJobRegistry>
{
	MOCK_METHOD3(addJob, fep3::Result(const std::string&, const std::shared_ptr<IJob>&, const JobConfiguration&));
	MOCK_METHOD1(removeJob, fep3::Result(const std::string&));
	MOCK_CONST_METHOD0(getJobInfos, std::list<JobInfo>());
	MOCK_CONST_METHOD0(getJobs, Jobs());
};
	
template<template<typename...> class component_base_type = fep3::ComponentBase>
struct JobRegistryComponent
    : public component_base_type<IJobRegistry>
{
    MOCK_METHOD3(addJob, fep3::Result(const std::string&, const std::shared_ptr<IJob>&, const JobConfiguration&));
    MOCK_METHOD1(removeJob,  fep3::Result(const std::string&));
    MOCK_CONST_METHOD0(getJobInfos, std::list<JobInfo>());
    MOCK_CONST_METHOD0(getJobs, Jobs());
};

} // namespace mock
} // namespace fep3
