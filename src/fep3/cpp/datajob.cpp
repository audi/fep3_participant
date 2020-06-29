/**

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
 */
#include <fep3/cpp/datajob.h>

namespace fep3
{
namespace cpp
{
namespace arya
{

DataJob::DataJob(const std::string& name, fep3::arya::Duration cycle_time) :
    Configuration("job_" + name),
    Job(name, cycle_time,
        [&](Timestamp time_of_ex) { return process(time_of_ex); })
{
}

DataJob::DataJob(const std::string& name, fep3::arya::JobConfiguration job_config) :
    Configuration("job_" + name),
    Job(name, job_config,
        [&](Timestamp time_of_ex) { return process(time_of_ex); })
{
}

DataJob::DataJob(const std::string& name, fep3::arya::Duration cycle_time, DataJob::ExecuteCallback fc) :
    Configuration("job_" + name),
    Job(name, cycle_time, fc)
{
}

DataJob::DataJob(const std::string& name, fep3::arya::JobConfiguration job_config, DataJob::ExecuteCallback fc) :
    Configuration("job_" + name),
    Job(name, job_config, fc)
{
}

DataReader * DataJob::addDataIn(const std::string& name, const IStreamType & type)
{
    _readers.push_back(DataReader(name, type));
    return &_readers.back();
}

DataReader * DataJob::addDataIn(const std::string& name, const IStreamType & type, size_t queue_size)
{
    _readers.push_back(DataReader(name, type, queue_size));
    return &_readers.back();
}

DataWriter * DataJob::addDataOut(const std::string& name, const IStreamType & type)
{
    _writers.push_back(DataWriter(name, type, core::arya::DATA_WRITER_QUEUE_SIZE_DEFAULT));
    return &_writers.back();
}

DataWriter * DataJob::addDynamicDataOut(const std::string& name, const IStreamType & type)
{
    _writers.push_back(DataWriter(name, type, core::arya::DATA_WRITER_QUEUE_SIZE_DYNAMIC));
    return &_writers.back();
}

fep3::Result DataJob::reconfigureDataIn(const std::string& name, size_t queue_size)
{
    for (auto& reader : _readers)
    {
        if (reader.getName() == name)
        {
            reader.resize(queue_size);
            return {};
        }
    }
    return fep3::Result(ERR_NOT_FOUND);
}

DataWriter * DataJob::addDataOut(const std::string& name, const IStreamType & type, size_t queue_size)
{
    if (0 >= queue_size)
    {
        throw std::runtime_error("a queue size <= 0 is not valid");
    }

    _writers.push_back(DataWriter(name, type, queue_size));
    return &_writers.back();
}

fep3::Result DataJob::process(Timestamp /*time_of_execution*/)
{
    return {};
}

fep3::Result DataJob::reset()
{
    return Job::reset();
}

fep3::Result DataJob::executeDataIn(Timestamp time_of_execution)
{
    for (auto& current : _readers)
    {
        current.receiveNow(time_of_execution);
    }
    return {};
}

fep3::Result DataJob::executeDataOut(Timestamp time_of_execution)
{
    for (auto& current : _writers)
    {
        //this will empty the data writer queues and write the data usually to the simulation bus
        //usually it will also call the transimission of the queue content
        //this will usually NOT wait for data really transmitted and 
        //it will usually NOT wait for the response of the receivers of the data 
        current.flushNow(time_of_execution);
    }
    return {};

}

fep3::Result DataJob::addDataToComponents(const IComponents& components)
{
    bool rollback = false;
    fep3::Result res;

    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry == nullptr)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "Datajob needs IDataRegistry, but not found in component");
    }
    auto clock = components.getComponent<IClockService>();
    if (clock == nullptr)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "Datajob needs IClockService, but not found in component");
    }

    if (!rollback)
    {
        res = initLogger(components, "element.job." + getJobInfo().getName());
        if (isFailed(res))
        {
            rollback = true;
        }
    }

    if (!rollback)
    {
        for (auto& reader : _readers)
        {
            res = reader.addToDataRegistry(*data_registry);
            if (isFailed(res))
            {
                rollback = true;
                break;
            }
        }
    }

    if (!rollback)
    {
        for (auto& writer : _writers)
        {
            res = writer.addToDataRegistry(*data_registry);
            res |= writer.addClock(*clock);
            if (isFailed(res))
            {
                rollback = true;
                break;
            }
        }
    }

    if (rollback)
    {
        removeDataFromComponents();
        return res;
    }
    else
    {
        return {};
    }
}

fep3::Result DataJob::removeDataFromComponents()
{

    for (auto& reader : _readers)
    {
        reader.removeFromDataRegistry();
    }
    for (auto& writer : _writers)
    {
        writer.removeFromDataRegistry();
        writer.removeClock();
    }

    deinitLogger();

    return {};
}

}
}
}
