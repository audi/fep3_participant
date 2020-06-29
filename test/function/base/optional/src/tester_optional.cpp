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

#include <fep3/fep3_optional.h>

/**
 * @brief A move-only type
 */
class MoveOnly
{
public:
    MoveOnly() = default;
    MoveOnly(int value)
        : _value(value)
    {}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly& operator=(MoveOnly&&) = default;
    virtual ~MoveOnly() = default;

    bool operator==(const MoveOnly& rhs) const
    {
        return _value == rhs._value;
    }
    int value() const
    {
        return _value;
    }
private:
    int _value{0};
};

/**
 * @brief Another move-only type
 */
class OtherMoveOnly : public MoveOnly
{
public:
    OtherMoveOnly() = default;
    OtherMoveOnly(int value)
        : MoveOnly(value)
    {}
    OtherMoveOnly(const OtherMoveOnly&) = delete;
    OtherMoveOnly(OtherMoveOnly&&) = default;
    OtherMoveOnly& operator=(const OtherMoveOnly&) = delete;
    OtherMoveOnly& operator=(OtherMoveOnly&&) = default;
    ~OtherMoveOnly() override = default;
};

/**
 * Tests construction of fep3::Optional
 * @req_id FEPSDK-2007
*/
TEST(OptionalTester, testConstruction)
{
    using ::fep3::Optional;
    
    // default CTOR
    Optional<int> test_optional_int_1;
    EXPECT_FALSE(test_optional_int_1.has_value());
    
    Optional<int> test_optional_int_2(33);
    // copy CTOR from other optional of same type
    Optional<int> test_optional_int_3(test_optional_int_2);
    ASSERT_TRUE(test_optional_int_3.has_value());
    EXPECT_EQ(33, test_optional_int_3.value());
    
    // copy CTOR from other optional of other type
    Optional<double> test_optional_double_1(test_optional_int_2);
    ASSERT_TRUE(test_optional_double_1.has_value());
    EXPECT_EQ(33, round(test_optional_double_1.value()));
    
    short other_value(33);
    // copy CTOR from value of other (non-optional) type
    Optional<int> test_optional_int_4(other_value);
    ASSERT_TRUE(test_optional_int_4.has_value());
    EXPECT_EQ(33, test_optional_int_4.value());
    
    Optional<MoveOnly> test_optional_move_only_1(44);
    // move CTOR from other optional of same type
    Optional<MoveOnly> test_optional_move_only_2(std::move(test_optional_move_only_1));
    EXPECT_FALSE(test_optional_move_only_1.has_value());
    ASSERT_TRUE(test_optional_move_only_2.has_value());
    EXPECT_EQ(44, test_optional_move_only_2.value().value());
    
    Optional<OtherMoveOnly> test_optional_other_move_only(OtherMoveOnly(55));
    // move CTOR from other optional of other type
    Optional<MoveOnly> test_optional_move_only_3(std::move(test_optional_other_move_only));
    EXPECT_FALSE(test_optional_other_move_only.has_value());
    ASSERT_TRUE(test_optional_move_only_3.has_value());
    EXPECT_EQ(55, test_optional_move_only_3.value().value());
    
    OtherMoveOnly other_move_only(66);
    // move CTOR from value of other (non-optional) type
    Optional<MoveOnly> test_optional_move_only_4(std::move(other_move_only));
    ASSERT_TRUE(test_optional_move_only_4.has_value());
    EXPECT_EQ(66, test_optional_move_only_4.value().value());
}

/**
 * Tests construction of fep3::Optional
 * @req_id FEPSDK-2007
*/
TEST(OptionalTester, testAssignment)
{
    using ::fep3::Optional;
    
    Optional<int> test_optional_int_1(33);
    Optional<int> test_optional_int_2;
    // copy assigment from other optional of same type
    test_optional_int_2 = test_optional_int_1;
    ASSERT_TRUE(test_optional_int_2.has_value());
    EXPECT_EQ(33, test_optional_int_2.value());
    
    Optional<double> test_optional_double_1;
    // copy assignment from other optional of other type
    test_optional_double_1 = test_optional_int_2;
    ASSERT_TRUE(test_optional_double_1.has_value());
    EXPECT_EQ(33, round(test_optional_double_1.value()));
    
    short other_value(33);
    // copy assignment from value of other (non-optional) type
    test_optional_int_2 = other_value;
    ASSERT_TRUE(test_optional_int_2.has_value());
    EXPECT_EQ(33, test_optional_int_2.value());
    
    Optional<MoveOnly> test_optional_move_only_1(44);
    Optional<MoveOnly> test_optional_move_only_2;
    // move assignment from other optional of same type
    test_optional_move_only_2 = std::move(test_optional_move_only_1);
    EXPECT_FALSE(test_optional_move_only_1.has_value());
    ASSERT_TRUE(test_optional_move_only_2.has_value());
    EXPECT_EQ(44, test_optional_move_only_2.value().value());
    
    Optional<OtherMoveOnly> test_optional_other_move_only(OtherMoveOnly(55));
    Optional<MoveOnly> test_optional_move_only_3;
    // move assignment from other optional of other type
    test_optional_move_only_3 = std::move(test_optional_other_move_only);
    EXPECT_FALSE(test_optional_other_move_only.has_value());
    ASSERT_TRUE(test_optional_move_only_3.has_value());
    EXPECT_EQ(55, test_optional_move_only_3.value().value());
    
    OtherMoveOnly other_move_only(66);
    Optional<MoveOnly> test_optional_move_only_4;
    // move assignment from value of other (non-optional) type
    test_optional_move_only_4 = std::move(other_move_only);
    ASSERT_TRUE(test_optional_move_only_4.has_value());
    EXPECT_EQ(66, test_optional_move_only_4.value().value());
}

/**
 * Tests an empty fep3::Optional
 * @req_id FEPSDK-2007
*/
TEST(OptionalTester, testEmpty)
{
    using ::fep3::Optional;
    using ::fep3::BadOptionalAccess;
    Optional<int> optional_integer;
    EXPECT_FALSE(optional_integer.operator bool());
    EXPECT_FALSE(optional_integer.has_value());
    EXPECT_THROW(optional_integer.value(), BadOptionalAccess);
    EXPECT_EQ(optional_integer.value_or(1), 1);

    Optional<int> other_optional_integer;
    optional_integer.swap(other_optional_integer);
    EXPECT_FALSE(optional_integer.has_value());
}

/**
 * Tests a non-empty fep3::Optional
 * @req_id FEPSDK-2007
*/
TEST(OptionalTester, testNonEmpty)
{
    using ::fep3::Optional;
    using ::fep3::BadOptionalAccess;

    Optional<int> optional_integer(1);
    EXPECT_TRUE(optional_integer.operator bool());
    EXPECT_TRUE(optional_integer.has_value());
    EXPECT_NO_THROW(optional_integer.value());
    EXPECT_EQ(optional_integer.value(), 1);
    EXPECT_EQ(optional_integer.value_or(99), 1);

    {
        // swap with non-empty optional
        Optional<int> other_optional_integer(2);
        optional_integer.swap(other_optional_integer);
        EXPECT_TRUE(optional_integer.has_value());
        EXPECT_NO_THROW(optional_integer.value());
        EXPECT_EQ(optional_integer.value(), 2);
        EXPECT_EQ(optional_integer.value_or(99), 2);
        EXPECT_TRUE(other_optional_integer.has_value());
        EXPECT_NO_THROW(other_optional_integer.value());
        EXPECT_EQ(other_optional_integer.value(), 1);
        EXPECT_EQ(other_optional_integer.value_or(99), 1);
    }

    {
        // swap with empty optional
        Optional<int> other_optional_integer;
        optional_integer.swap(other_optional_integer);
        EXPECT_FALSE(optional_integer.has_value());
        EXPECT_THROW(optional_integer.value(), BadOptionalAccess);
        EXPECT_EQ(optional_integer.value_or(99), 99);
        EXPECT_TRUE(other_optional_integer.has_value());
        EXPECT_NO_THROW(other_optional_integer.value());
        EXPECT_EQ(other_optional_integer.value(), 2);
        EXPECT_EQ(other_optional_integer.value_or(99), 2);
    }
}

/**
 * Tests emplacing a fep3::Optional
 * @req_id FEPSDK-2007
*/
TEST(OptionalTester, testEmplace)
{
    using ::fep3::Optional;

    Optional<MoveOnly> optional_integer;
    optional_integer.emplace(1);
    EXPECT_TRUE(optional_integer.has_value());
    EXPECT_NO_THROW(optional_integer.value());
    EXPECT_EQ(optional_integer.value(), 1);
    // note value_or does not support move-only types
    //EXPECT_EQ(optional_integer.value_or(99), 1);
}

/**
 * Tests resetting of a fep3::Optional
 * @req_id FEPSDK-2007
*/
TEST(OptionalTester, testReset)
{
    using ::fep3::Optional;
    using ::fep3::BadOptionalAccess;

    Optional<int> optional_integer(1);
    optional_integer.reset();
    EXPECT_FALSE(optional_integer.has_value());
    EXPECT_THROW(optional_integer.value(), BadOptionalAccess);
    EXPECT_EQ(optional_integer.value_or(99), 99);
}

/**
 * Tests comparison of fep3::Optionals
 * @req_id FEPSDK-2007
*/
TEST(OptionalTester, testComparison)
{
    using ::fep3::Optional;

    // compare two empty optionals
    {
        Optional<int> optional_integer_1;
        Optional<int> optional_integer_2;
        EXPECT_TRUE(optional_integer_1 == optional_integer_2);
        EXPECT_FALSE(optional_integer_1 != optional_integer_2);
    }

    // compare an empty optional with a non-empty optional
    {
        Optional<int> optional_integer_1;
        Optional<int> optional_integer_2(2);
        EXPECT_TRUE(optional_integer_1 != optional_integer_2);
        EXPECT_FALSE(optional_integer_1 == optional_integer_2);
    }

    // compare two non-empty optionals with different values
    {
        Optional<int> optional_integer_1(1);
        Optional<int> optional_integer_2(2);
        EXPECT_TRUE(optional_integer_1 != optional_integer_2);
        EXPECT_FALSE(optional_integer_1 == optional_integer_2);
    }

    // compare two non-empty optionals with same values
    {
        Optional<int> optional_integer_1(1);
        Optional<int> optional_integer_2(1);
        EXPECT_TRUE(optional_integer_1 == optional_integer_2);
        EXPECT_FALSE(optional_integer_1 != optional_integer_2);
    }
}