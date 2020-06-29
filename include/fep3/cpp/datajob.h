/**
*
*
* @file

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.

       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.

   You may add additional accurate notices of copyright ownership.
   @endverbatim
*
*/

#pragma once

#include <list>
#include <string>

#include <fep3/core/job.h>
#include <fep3/core/data/data_reader.h>
#include <fep3/core/data/data_writer.h>
#include <fep3/components/configuration/propertynode.h>
#include <fep3/components/logging/easy_logger.h>

namespace fep3
{
namespace cpp
{
namespace arya
{

using core::arya::DataReader;
using core::arya::DataWriter;
using fep3::arya::PropertyVariable;

/**
 * A DataJob will automatically register on initialization time the data to the fep::IDataRegistry
 * It will also set up the default timing behaviour of its process method
 *
 */
class DataJob : public fep3::core::arya::Job,
    public fep3::arya::Configuration,
    public fep3::logging::arya::EasyLogging
{
public:
    ///type of an executeion function to execute if job is triggered
    using ExecuteCallback = fep3::core::arya::Job::ExecuteCallback;

public:
    /**
     * @brief CTOR. If you use this CTOR you have to override process()
     * 
     * @param name Name of the Job
     * @param cycle_time Cycle time of the job (simulation time)
     * 
     */
    DataJob(const std::string& name, fep3::arya::Duration cycle_time);
    /**
     * @brief CTOR. If you use this CTOR you have to override process()
     * 
     * @param name Name of the job
     * @param job_config default pre-configuration of the job (might be changed while runtime). 
     */
    DataJob(const std::string& name, fep3::arya::JobConfiguration job_config);
    /**
     * @brief CTOR. if using this CTOR you do NOT need to implement process(). 
     * Put you implementation into the execute function.
     *
     * @param name Name of the Job
     * @param fc execute function 
     * @param cycle_time Cycle time of the job (simulation time)
     */
    DataJob(const std::string& name, fep3::arya::Duration cycle_time, ExecuteCallback fc);
    /**
     * @brief CTOR. if using this CTOR you do NOT need to implement process(). 
     * Put you implementation into the execute function.
     *
     * @param name Name of the Job
     * @param fc execute function 
     * @param job_config default pre-configuration of the job (might be changed while runtime). 
     */
    DataJob(const std::string& name, JobConfiguration job_config, ExecuteCallback fc);

    /**
    * Creates a fep3::core::DataReader with a default queue capacity of 1
    * If you receive more than one sample before reading, these samples will be lost.
    * @param name name of signal
    * @param type type of signal
    * @return created DataReader
    */
    DataReader* addDataIn(const std::string& name, const IStreamType& type);

    /**
    * Creates a fep3::core::DataReader with a specific queue capacity
    * If you receive more samples than specified by @param queue_size before reading, these samples will be lost.
    * @param name name of signal
    * @param type type of signal
    * @param queue_size capacity of the internal DataReader item queue
    * @return created DataReader
    */
    DataReader* addDataIn(const std::string& name, const IStreamType& type, size_t queue_size);

    /**
    * Creates a fep3::core::DataWriter with a default queue capacity of 1
    * If you queue more than one sample before flushing these samples will be lost.
    * @param name name of signal
    * @param type type of signal
    * @return created DataWriter
    */
    DataWriter* addDataOut(const std::string& name, const IStreamType& type);

    /**
    * Creates a fep3::core::DataWriter with a fixed maximum queue capacity.
    * If you queue more than @param queue_size samples before flushing these samples will be lost.
    * @param name name of signal
    * @param type type of signal
    * @param queue_size maximum capacity of the queue
    * @throw throws if queue_size parameter is <= 0
    * @return created DataWriter
    */
    DataWriter* addDataOut(const std::string& name, const IStreamType& type, size_t queue_size);

    /**
    * Creates a fep3::core::DataWriter with infinite queue capacity.
    * Pushing a sample into the queue extends the queue.
    * Every sample that is queued will be written on flush.
    * @param name name of signal
    * @param type type of signal
    * @return created DataWriter
    * @remark pushing big numbers of samples into the queue might lead to out-of-memory situations
    */
    DataWriter* addDynamicDataOut(const std::string& name, const IStreamType& type);

    /**
     * Resize the DataReaderBacklog of a DataReader.
     * @param name name of the DataReader to be resized.
     * @param queue_capacity capacity of the DataReaderBacklog after resizing.
     * @return fep3::Result
     * @return ERR_NOT_FOUND found no DataReader with the given \p name.
     */
    fep3::Result reconfigureDataIn(const std::string& name, size_t queue_capacity);

    /**
     * @brief adds the created reader and writer to the data registry and clock
     *
     * @param components the components to add to
     * @return fep3::Result
     */
    fep3::Result addDataToComponents(const fep3::arya::IComponents& components);
    /**
     * @brief aremove from the data registry (where it was added).
     *
     * @return fep3::Result
     */
    fep3::Result removeDataFromComponents();

protected:
    /**
     * The process method to override.
     * implement your functionality here.
     * you do not need to lock something in here!
     * @param time_of_execution current time of execution from the ClockService
     *                          this is the beginning time of the execution in simulation time
     * @return fep::Result If you return en error here the scheduler might stop execution immediatelly!
     */
    virtual fep3::Result process(Timestamp time_of_execution);

public: //Job
    /**
     * The reset method to override.
     * implement your reset functionality here.
     * This method is called each time before the \p process method is called
     * @return fep::Result If you return en error here the scheduler might stop execution immediatelly!
     */
    fep3::Result reset() override;

private:
    ///@copydoc fep3::core::arya::Job::executeDataIn
    fep3::Result executeDataIn(Timestamp time_of_execution) override;
    ///@copydoc fep3::core::arya::Job::executeDataOut
    fep3::Result executeDataOut(Timestamp time_of_execution) override;

private:
    ///the readers
    std::list<DataReader> _readers;
    ///the writers
    std::list<DataWriter> _writers;
};

/**
* @brief Datajobs will be removed from the job registry of the @p component.
* If one job can not be removed, the function will still try to remove the following ones within @p job_names.
*
* @param jobs List of jobs to be removed
* @param components The component container in which the job registry will be looked up
* @return fep3::Result.
*           If it's an actual error it is the error code for the job that failed last.
*           If more than one remove fails, the error description contains a list of error descriptions separated by ';'.
* @retval ERR_NOERROR Everything went fine
* @retval ERR_NOT_FOUND A job with the given name is not registered
* @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
*/
inline fep3::Result removeFromComponents(const std::vector<std::shared_ptr<DataJob>>& jobs,
    const IComponents& components)
{
    std::vector<std::string> job_names;
    for (const auto& current_job_to_add : jobs)
    {
        job_names.push_back(current_job_to_add->getJobInfo().getName());
        current_job_to_add->removeDataFromComponents();
    }
    return core::removeFromComponents(job_names, components);
}
/**
* @brief Datajobs will be added to the job registry of the @p component.
*        Additionally there data reader and writer will be registered at the DataRegistery
* If one job can not be added, the function returns not adding the following ones and rolling back!
*
* @param jobs List of jobs to be added
* @param components The component container in which the job registry will be looked up
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine
* @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
* @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
*/
inline fep3::Result addToComponents(const std::vector<std::shared_ptr<DataJob>>& jobs, const IComponents& components)
{
    for (const auto& current_job_to_add : jobs)
    {
        auto res_adding_data = current_job_to_add->addDataToComponents(components);
        if (isOk(res_adding_data))
        {
            auto res_adding_job = core::addToComponents(
                current_job_to_add->getJobInfo().getName(),
                current_job_to_add,
                current_job_to_add->getJobInfo().getConfig(),
                components);
            if (isFailed(res_adding_job))
            {
                removeFromComponents(jobs, components);
                return res_adding_job;
            }
        }
        else
        {
            removeFromComponents(jobs, components);
            return res_adding_data;
        }
    }
    return {};
}
}
using arya::DataReader;
using arya::DataWriter;
using arya::DataJob;
using arya::PropertyVariable;


/**
* @brief Datajobs will be removed from the job registry of the @p component.
* If one job can not be removed, the function will still try to remove the following ones within @p job_names.
*
* @param jobs List of jobs to be removed
* @param components The component container in which the job registry will be looked up
* @return fep3::Result.
*           If it's an actual error it is the error code for the job that failed last.
*           If more than one remove fails, the error description contains a list of error descriptions separated by ';'.
* @retval ERR_NOERROR Everything went fine
* @retval ERR_NOT_FOUND A job with the given name is not registered
* @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
*/
inline fep3::Result removeFromComponents(const std::vector<std::shared_ptr<DataJob>>& jobs,
    const IComponents& components)
{
    return arya::removeFromComponents(jobs, components);
}

/**
* @brief Datajobs will be added to the job registry of the @p component.
*        Additionally there data reader and writer will be registered at the DataRegistery
* If one job can not be added, the function returns not adding the following ones and rolling back!
*
* @param jobs List of jobs to be added
* @param components The component container in which the job registry will be looked up
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine
* @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
* @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
*/
inline fep3::Result addToComponents(const std::vector<std::shared_ptr<DataJob>>& jobs, const IComponents& components)
{
    return arya::addToComponents(jobs, components);
}

}
}