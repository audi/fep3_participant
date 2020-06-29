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

#include "data_io.h"

using namespace fep3;
using namespace fep3::native;

/***************************************************************/
/* DataReader                                                  */
/***************************************************************/

DataRegistry::DataReader::~DataReader()
{
}



/***************************************************************/
/* DataWriter                                                  */
/***************************************************************/

fep3::Result DataRegistry::DataWriter::write(const IDataSample& data_sample)
{
    //usually we should NOT imidiatelly forward here but push to a queue and forward it while flush is called!
    //but this can be done only if we are able to preallocate data_write_ptr from the simbus 
    return _dataout_writer_ref.write(data_sample);
}

fep3::Result DataRegistry::DataWriter::write(const IStreamType& stream_type)
{
    //usually we should NOT imidiatelly forward here but push to a queue and forward it while flush is called!
    //but this can be done only if we are able to preallocate data_write_ptr from the simbus 
    return _dataout_writer_ref.write(stream_type);
}

fep3::Result DataRegistry::DataWriter::flush()
{
    //usually we should forward our queue content here and call transmit afterwards in a whole
    //but this can be done only if we are able to preallocate data_write_ptr from the simbus and hold them in a queue here
    //(alloc_write_sample! )
    return _dataout_writer_ref.transmit();
}

size_t DataRegistry::DataWriter::capacity() const
{
    return _queue_capacity;
}


/***************************************************************/
/* DataReaderProxy                                             */
/***************************************************************/

DataRegistry::DataReaderProxy::DataReaderProxy(std::shared_ptr<IDataRegistry::IDataReader> reader)
    : _data_reader(reader)
{
    if (!_data_reader)
    {
        throw std::runtime_error("DataReaderProxy cannot be constructed with an invalid reader object!");
    }
}

size_t DataRegistry::DataReaderProxy::size() const
{ 
    return _data_reader->size(); 
}

size_t DataRegistry::DataReaderProxy::capacity() const
{
    return _data_reader->capacity(); 
}

fep3::Result DataRegistry::DataReaderProxy::pop(IDataReceiver& receiver)
{ 
    return _data_reader->pop(receiver); 
}

fep3::Optional<Timestamp> DataRegistry::DataReaderProxy::getFrontTime() const
{
    return _data_reader->getFrontTime(); 
}

/***************************************************************/
/* DataWriterProxy                                             */
/***************************************************************/

DataRegistry::DataWriterProxy::DataWriterProxy(std::shared_ptr<IDataRegistry::IDataWriter> writer)
    : _data_writer(writer)
{
    if (!_data_writer)
    {
        throw std::runtime_error("DataWriterProxy cannot be constructed with an invalid writer object!");
    }
}

fep3::Result DataRegistry::DataWriterProxy::write(const IDataSample& data_sample)
{
    return _data_writer->write(data_sample); 
}

fep3::Result DataRegistry::DataWriterProxy::write(const IStreamType& stream_type)
{
    return _data_writer->write(stream_type); 
}

fep3::Result DataRegistry::DataWriterProxy::flush()
{
    return _data_writer->flush(); 
}