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

#include <fep3/base/sample/data_sample.h>

#include <chrono>

/**
 * @detail Test the registration, unregistration and memorymanagment of the ComponentRegistry
 * @req_id FEPSDK-Sample
 */
TEST(DataSampleClassTest, testSampleClass)
{  
    fep3::DataSample sample;  
}

/**
 * @detail Test the copying of StdVectorSampleType through IDataSample
 * @req_id FEPSDK-Sample
 */
TEST(StdVectorSampleTypeTest, testCopy)
{
    using namespace std::literals::chrono_literals;
    const size_t element_count = 10;
    const fep3::Timestamp timestamp = 123ns;
    const uint32_t counter = 123;

    struct TestVector{
        bool valid;
        int x;
        double length;
        float y;
        char padding;
    };

    auto compare_two_test_vectors = [](const TestVector &l, const TestVector &r) {
        bool is_equal = l.valid == r.valid;
        is_equal &= l.x == r.x;
        is_equal &= l.length == r.length;
        is_equal &= l.y == r.y;
        is_equal &= l.padding == r.padding;
        return is_equal;
    };

    // Prepare std vector
    std::vector<TestVector> my_data;
    my_data.reserve(element_count);

    for (int i = element_count; i > 0; i--)
    {
        TestVector v{
            i % 2 ? true : false,
            i,
            i * 1.24,
            static_cast<float>(i) / 1.24f,
            static_cast<char>(i)
        };
        my_data.emplace_back(v);
    }

    // Prepare sample
    fep3::StdVectorSampleType<TestVector> array_sample{my_data};
    array_sample.setTime(timestamp);
    array_sample.setCounter(counter);
    fep3::IDataSample* intf_sample = static_cast<fep3::IDataSample*>(&array_sample);

    // Copy sample via IDataSample interface
    fep3::DataSample sample_raw_copy{*intf_sample};

    std::vector<TestVector> my_copied_data;
    fep3::StdVectorSampleType<TestVector> copied_array_sample{my_copied_data};

    // Test if sample is equal to its copy with IRawMemory interface
    copied_array_sample.write(sample_raw_copy);
    ASSERT_TRUE(std::equal(my_data.begin(), my_data.end(), my_copied_data.begin(), compare_two_test_vectors));
    ASSERT_EQ(array_sample.getSize(), copied_array_sample.getSize());

    // Test if sample is equal to its copy via IDataSample assignment
    copied_array_sample = sample_raw_copy;
    ASSERT_TRUE(std::equal(my_data.begin(), my_data.end(), my_copied_data.begin(), compare_two_test_vectors));
    ASSERT_EQ(array_sample.getSize(), copied_array_sample.getSize());
    ASSERT_EQ(array_sample.getCounter(), copied_array_sample.getCounter());
    ASSERT_EQ(array_sample.getTime(), copied_array_sample.getTime());
}
