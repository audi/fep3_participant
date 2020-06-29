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
//Guideline - FEP System Library API Exception
#ifndef _FEP3_COMP_PROPERTY_TYPE_CONVERSION_H_
#define _FEP3_COMP_PROPERTY_TYPE_CONVERSION_H_

#include <string>
#include <vector>
#include <a_util/strings.h>

#include "property_type.h"

namespace fep3
{
namespace arya
{
/**
 * @brief spezialized DefaultPropertyTypeConversion for type bool. Can be used for PropertyValue.
 *
 */
template<>
struct DefaultPropertyTypeConversion<bool>
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation bool
     *
     * @param from the value as string
     * @return bool the value as bool
     */
    inline static bool fromString(const std::string& from)
    {
        return a_util::strings::toBool(from);
    }
    /**
     * @brief static function to serialize the value of type bool to a string (utf-8)
     *
     * @param value the value as bool
     * @return std::string the value as string
     */
    inline static std::string toString(const bool& value)
    {
        return a_util::strings::toString(value);
    }
};

/**
 * @brief spezialized DefaultPropertyTypeConversion for type int32_t. Can be used for PropertyValue.
 *
 */
template<>
struct DefaultPropertyTypeConversion<int32_t>
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation int32_t
     *
     * @param from the value as string
     * @return int32_t the value as int32_t
     */
    static int32_t fromString(const std::string& from)
    {
        return a_util::strings::toInt32(from);
    }
    /**
     * @brief static function to serialize the value of type int32_t to a string (utf-8)
     *
     * @param value the value as int32_t
     * @return std::string the value as string
     */
    static std::string toString(const int32_t& value)
    {
        return a_util::strings::toString(value);
    }
};
/**
 * @brief spezialized DefaultPropertyTypeConversion for type double. Can be used for PropertyValue.
 *
 */
template<>
struct DefaultPropertyTypeConversion<double>
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation double
     *
     * @param from the value as string
     * @return double the value as double
     */
    inline static double fromString(const std::string& from)
    {
        return a_util::strings::toDouble(from);
    }
    /**
     * @brief static function to serialize the value of type double to a string (utf-8)
     *
     * @param value the value as double
     * @return std::string the value as string
     */
    inline static std::string toString(const double& value)
    {
        return a_util::strings::toString(value);
    }
};

/**
 * @brief spezialized DefaultPropertyTypeConversion for type std::string. Can be used for @ref PropertyValue.
 *
 */
template<>
struct DefaultPropertyTypeConversion<std::string>
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation std::string
     *
     * @param from the value as string
     * @return std::string the value as std::string
     */
    inline static std::string fromString(const std::string& from)
    {
        return from;
    }
    /**
     * @brief static function to serialize the value of type std::string to a string (utf-8)
     *
     * @param value the value as std::string
     * @return std::string the value as string
     */
    inline static std::string toString(const std::string& value)
    {
        return value;
    }
};

/**
 * @brief spezialized DefaultPropertyTypeConversion for type std::vector<bool>. Can be used for @ref PropertyValue.
 *
 */
template<>
struct DefaultPropertyTypeConversion<std::vector<bool>>
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation std::vector<bool>
     *
     * @param from the value as string
     * @return std::vector<bool> the value as std::vector<bool>
     */
    inline static std::vector<bool> fromString(const std::string& from)
    {
        std::vector<std::string> split_vector_value = a_util::strings::split(from, ";");
        std::vector<bool> return_value;
        return_value.reserve(split_vector_value.size());
        for (const auto& current_split_value : split_vector_value)
        {
            return_value.push_back(a_util::strings::toBool(current_split_value));
        }
        return return_value;
    }
    /**
     * @brief static function to serialize the value of type std::vector<bool> to a string (utf-8)
     *
     * @param value the value as std::vector<bool>
     * @return std::string the value as string
     */
    inline static std::string toString(const std::vector<bool>& value)
    {
        bool first = true;
        std::string result_string;
        for (const auto& current_value : value)
        {
            if (!first)
            {
                result_string += ";";
            }
            result_string += a_util::strings::toString(current_value);
            first = false;
        }
        return result_string;
    }
};

/**
 * @brief spezialized DefaultPropertyTypeConversion for type std::vector<int32_t>. Can be used for @ref PropertyValue.
 *
 */
template<>
struct DefaultPropertyTypeConversion<std::vector<int32_t>>
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation std::vector<int32_t>
     *
     * @param from the value as string
     * @return std::vector<int32_t> the value as std::vector<int32_t>
     */
    inline static std::vector<int32_t> fromString(const std::string& from)
    {
        std::vector<std::string> split_vector_value = a_util::strings::split(from, ";");
        std::vector<int32_t> return_value;
        return_value.reserve(split_vector_value.size());
        for (const auto& current_split_value : split_vector_value)
        {
            return_value.push_back(a_util::strings::toInt32(current_split_value));
        }
        return return_value;
    }
    /**
     * @brief static function to serialize the value of type std::vector<int32_t> to a string (utf-8)
     *
     * @param value the value as std::vector<int32_t>
     * @return std::string the value as string
     */
    inline static std::string toString(const std::vector<int32_t>& value)
    {
        bool first = true;
        std::string result_string;
        for (const auto& current_value : value)
        {
            if (!first)
            {
                result_string += ";";
            }
            result_string += a_util::strings::toString(current_value);
            first = false;
        }
        return result_string;
    }
};

/**
 * @brief spezialized DefaultPropertyTypeConversion for type std::vector<double>. Can be used for @ref PropertyValue.
 *
 */
template<>
struct DefaultPropertyTypeConversion<std::vector<double>>
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation std::vector<double>
     *
     * @param from the value as string
     * @return std::vector<double> the value as std::vector<double>
     */
    inline static std::vector<double> fromString(const std::string& from)
    {
        std::vector<std::string> split_vector_value = a_util::strings::split(from, ";");
        std::vector<double> return_value;
        return_value.reserve(split_vector_value.size());
        for (const auto& current_split_value : split_vector_value)
        {
            return_value.push_back(a_util::strings::toDouble(current_split_value));
        }
        return return_value;
    }
    /**
     * @brief static function to serialize the value of type std::vector<double> to a string (utf-8)
     *
     * @param value the value as std::vector<double>
     * @return std::string the value as string
     */
    inline static std::string toString(const std::vector<double>& value)
    {
        bool first = true;
        std::string result_string;
        for (const auto& current_value : value)
        {
            if (!first)
            {
                result_string += ";";
            }
            result_string += a_util::strings::toString(current_value);
            first = false;
        }
        return result_string;
    }
};

/**
 * @brief spezialized DefaultPropertyTypeConversion for type std::vector<std::string>. Can be used for @ref PropertyValue.
 *
 */
template<>
struct DefaultPropertyTypeConversion<std::vector<std::string>>
{
    /**
     * @brief static function to deserialize the value from string (utf-8) to the typed representation std::vector<std::string>
     *
     * @param from the value as string
     * @return std::vector<std::string> the value as std::vector<std::string>
     */
    inline static std::vector<std::string> fromString(const std::string& from)
    {
        std::vector<std::string> split_vector_value = a_util::strings::split(from, ";");
        return split_vector_value;
    }
    /**
     * @brief static function to serialize the value of type std::vector<std::string> to a string (utf-8)
     *
     * @param value the value as std::vector<std::string>
     * @return std::string the value as string
     */
    inline static std::string toString(const std::vector<std::string>& value)
    {
        return a_util::strings::join(value, std::string(";"));
    }
};
}
using arya::DefaultPropertyTypeConversion;
} //end of fep namespace

#endif //_FEP3_COMP_PROPERTY_TYPE_CONVERSION_H_