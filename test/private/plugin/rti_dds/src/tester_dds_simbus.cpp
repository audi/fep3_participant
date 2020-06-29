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
#include <fep3/base/streamtype/default_streamtype.h>
#include <fep3/base/sample/data_sample.h>
#include <fep3/base/sample/raw_memory.h>

#include "detail/test_receiver.hpp"
#include "detail/test_samples.hpp"
#include "detail/test_read_write_test_class.hpp"

/**
 * @detail Test send and receive of one sample
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SendAndReceiveSample)
{  
    BlockingTestReceiver sample_receiver(*_reader);

    uint32_t value = 6;
    _writer->write(DataSampleType<uint32_t>(value));
    _writer->transmit();

    sample_receiver.waitFor(1, NO_CHECK);

    uint32_t received_value = 0;
    DataSampleType<uint32_t> received_value_type(received_value);
    sample_receiver._samples.at(0)->read(received_value_type);
    EXPECT_EQ(value, received_value);
}


/**
 * @detail Test send and receive of one streamtype change
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SendAndReceiveStreamType)
{
    BlockingTestReceiver sample_receiver(*_reader);

    _writer->write(StreamTypeDDL("tStruct", "ddl_description"));
    _writer->transmit();

    sample_receiver.waitFor(0, 2);

    EXPECT_EQ(sample_receiver._stream_types.size(), 2);

    auto received_stream_type = sample_receiver._stream_types.at(1);
    EXPECT_EQ(received_stream_type->getMetaTypeName(), "ddl");
    EXPECT_EQ(received_stream_type->getProperty("ddlstruct"), "tStruct");
    EXPECT_EQ(received_stream_type->getProperty("ddldescription"), "ddl_description");
}


/**
 * @detail Test change of streamtype during sample transmition
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, ChangeStreamType)
{
    BlockingTestReceiver sample_receiver(*_reader);

    uint8_t value1 = 6;
    _writer->write(DataSampleType<uint8_t>(value1));
    _writer->write(StreamTypePlain<uint64_t>());
    uint64_t value2 = 600000000;
    _writer->write(DataSampleType<uint64_t>(value2));
    _writer->transmit();

    sample_receiver.waitFor(2, 2, std::chrono::seconds(10));
    
    uint8_t received_value1 = 0;
    DataSampleType<uint8_t> received_value_type1(received_value1);
    sample_receiver._samples.at(0)->read(received_value_type1);
    EXPECT_EQ(value1, received_value1);

    uint64_t received_value2 = 0;
    DataSampleType<uint64_t> received_value_type2(received_value2);
    sample_receiver._samples.at(1)->read(received_value_type2);
    EXPECT_EQ(value2, received_value2);
}


/**
 * @detail Test send and receive from sampel with timestamp
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SampleTimestamp)
{
    BlockingTestReceiver sample_receiver(*_reader);

    uint32_t value = 6;
    _writer->write(TimeDataSampleType<uint32_t>(value, Timestamp(3)));
    _writer->transmit();

    sample_receiver.waitFor(1, NO_CHECK);

    uint32_t received_value = 0;
    TimeDataSampleType<uint32_t> sample(received_value);
    sample_receiver._samples.at(0)->read(sample);
    
    EXPECT_EQ(sample_receiver._samples.at(0)->getTime(), Timestamp(3));
} 

/**
 * @detail Test send and receive from sampel with timestamp
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, getFrontTime)
{
    uint32_t value = 6;
    _writer->write(TimeDataSampleType<uint32_t>(value, Timestamp(3)));
    _writer->transmit();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Pop stream_type
    CountSampleTestReceiver receiver;
    _reader->pop(receiver);
        
    // Now pop the real sample
    EXPECT_EQ(_reader->getFrontTime().value_or(Timestamp(0)), Timestamp(3));

    TestReceiver sample_receiver;
    _reader->pop(sample_receiver);

    ASSERT_EQ(sample_receiver._samples.size(), 1);
    EXPECT_EQ(sample_receiver._samples.at(0)->getTime(), Timestamp(3));
}

/**
 * @detail Test the sample counter
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, SampleCounter)
{
    BlockingTestReceiver sample_receiver(*_reader);

    uint8_t value = 6;
    _writer->write(DataSampleType<uint8_t>(value));
    _writer->write(DataSampleType<uint8_t>(value));
    _writer->write(DataSampleType<uint8_t>(value));
    _writer->transmit();

    sample_receiver.waitFor(3, NO_CHECK);

    uint32_t sample_counter = sample_receiver._samples.at(0)->getCounter();
    EXPECT_EQ(sample_receiver._samples.at(1)->getCounter(), sample_counter + 1);
    EXPECT_EQ(sample_receiver._samples.at(2)->getCounter(), sample_counter + 2);
    
}


/**
 * @detail Test a large sample (video)
 * @req_id FEPSDK-Sample
 */
TEST_F(ReaderWriterTestClass, VideoSample)
{
    auto video_type = StreamType(fep3::arya::meta_type_video);
    video_type.setProperty("height", "3840", "uint32_t");
    video_type.setProperty("width", "2160", "uint32_t");
    video_type.setProperty("pixelformat", "R(8)G(9)B(8)", "string");
    video_type.setProperty("max_size", "24883200", "uint32_t");

    auto writer = getSimulationBus()->getWriter("video", video_type);
    auto reader = getSimulationBus()->getReader("video", video_type);

    EXPECT_TRUE(writer);
    EXPECT_TRUE(reader);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    BlockingTestReceiver sample_receiver(*reader);

    RandomSample image_sample1(24883200);
    RandomSample image_sample2(24883200);
    RandomSample image_sample3(24883200);

    writer->write(image_sample1);
    writer->write(image_sample2);
    writer->write(image_sample3);
    writer->transmit();
    
    sample_receiver.waitFor(3, 1, std::chrono::seconds(10));

    EXPECT_TRUE(image_sample1.compare(*sample_receiver._samples.at(0)));
    EXPECT_TRUE(image_sample2.compare(*sample_receiver._samples.at(1)));
    EXPECT_TRUE(image_sample3.compare(*sample_receiver._samples.at(2)));
}

TEST_F(ReaderWriterTestClass, TestCorrectOrderOfSampleAndStreamType)
{
    OrderTestReceiver sample_receiver;

    std::thread sender_thread([&]()
    {
        for (uint8_t i = 0; i < 100; i++)
        {
            _writer->write(DataSampleType<uint8_t>(i));
            _writer->write(StreamTypePlain<uint32_t>());
            _writer->transmit();
        }
    });

    for (int i = 0; i < 100; i++)
    {
        _reader->pop(sample_receiver);
        _reader->pop(sample_receiver);
    }

    sender_thread.join();
}

TEST_F(ReaderWriterTestClass, TestPopOfDataReader)
{
    // Wait for Initalisation and discover
    CountSampleTestReceiver sample_receiver;
    
    _reader->pop(sample_receiver);
   
    for (int i = 0; i < 10; i++)
    {
        uint32_t value = 6;
        _writer->write(DataSampleType<uint32_t>(value));
        _writer->transmit();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        EXPECT_EQ(_reader->size(), 1);
        _reader->pop(sample_receiver);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_EQ(sample_receiver.getSampleCount(), 1);
    }
}

/**
 * @detail Test a large sample without knowing the streamtype
 * @req_id FEPSDK-Sample
 */
/*TEST_F(ReaderWriterTestClass, UnknownStreamType)
{
    auto writer = GetSimulationBus()->getWriter("video");
    auto reader = GetSimulationBus()->getReader("video");

    EXPECT_TRUE(writer);
    EXPECT_TRUE(reader);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    BlockingTestReceiver sample_receiver(*reader);

    RandomSample imageSample1(24883200);
    RandomSample imageSample2(24883200);
    RandomSample imageSample3(24883200);

    writer->write(imageSample1);
    writer->write(imageSample2);
    writer->write(imageSample3);
    writer->transmit();

    sample_receiver.waitFor(3, 1, std::chrono::seconds(10));

    EXPECT_TRUE(imageSample1.compare(*sample_receiver._samples.at(0)));
    EXPECT_TRUE(imageSample2.compare(*sample_receiver._samples.at(1)));
    EXPECT_TRUE(imageSample3.compare(*sample_receiver._samples.at(2)));
}
*/

/**
 * @detail Test queue_size
 * @req_id FEPSDK-Sample
 */
/*TEST_F(ReaderWriterTestClass, TestQueueSize)
{
    auto writer_limited = _participant.getWriter("limited", StreamTypePlain<uint32_t>(), 3);
    auto reader_limited = _participant.getReader("limited", StreamTypePlain<uint32_t>(), 3);

    uint8_t value = 6;
    writer_limited->write(DataSampleType<uint8_t>(value));
    writer_limited->write(DataSampleType<uint8_t>(value));
    writer_limited->write(DataSampleType<uint8_t>(value));
    writer_limited->transmit();

    writer_limited->write(DataSampleType<uint8_t>(value));
    writer_limited->write(DataSampleType<uint8_t>(value));
    writer_limited->transmit();

    BlockingTestReceiver sample_receiver(*_reader);
    sample_receiver.waitFor(3, 0);
    

    //!!! RESOURCE LIMITATIONS ARE NOT SUPPORTED AT THE MOMEMT!!!
    //EXPECT_EQ(sample_receiver._samples.size(), 3);

}*/
