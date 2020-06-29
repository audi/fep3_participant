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

TEST_F(ReaderWriterTestClass, TestParticipantDetection)
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
