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
#pragma once

#include <string>
#include "raw_memory_intf.h"

namespace fep3
{
namespace arya
{
/**
 * @brief Helper class to wrap up a const void* pointer as IRawMemory interface
 *
 */
struct RawMemoryRef : public IRawMemory
{
    /**
     * @brief Construct a new Raw Memory Ref object
     *
     * @param data the pointer to wrap up
     * @param data_size size in bytes stored in \p data
     */
    explicit RawMemoryRef(const void* data, size_t data_size) : _data(data), _data_size(data_size)
    {
    }
    size_t capacity() const override
    {
        return _data_size;
    }
    const void* cdata() const override
    {
        return _data;
    }
    size_t size() const override
    {
        return _data_size;
    }
    size_t set(const void* /*data*/, size_t /*data_size*/) override
    {
        return 0;
    }
    size_t resize(size_t /*data_size*/) override
    {
        return size();
    }
private:
    ///the data pointer
    const void* _data;
    ///the size in bytes stored in _data
    size_t _data_size;
};

/**
 * @brief Helper class to wrap up a standard layout type as IRawMemory interface
 *
 * @tparam T
 * @tparam Enable check for constness
 */
template<class T, typename Enable = void>
struct RawMemoryStandardType : public IRawMemory
{
    ///the standard layout type value
    T& _value;
    /**
     * @brief Construct a new Raw Memory Standard Type wrap up
     *
     * @param value the value to wrap up
     */
    RawMemoryStandardType(T& value) : _value(value)
    {
    }
    ///@copydoc fep3::arya::IRawMemory::capacity
    size_t capacity() const override
    {
        return sizeof(T);
    }
    ///@copydoc fep3::arya::IRawMemory::cdata
    const void* cdata() const override
    {
        return &_value;
    }
    ///@copydoc fep3::arya::IRawMemory::size
    size_t size() const override
    {
        return sizeof(T);
    }
    ///@copydoc fep3::arya::IRawMemory::set
    size_t set(const void* data, size_t data_size) override
    {
        if (data_size != size())
        {
            //usually throw
            return 0;
        }
        else
        {
            return a_util::memory::copy(&_value, data, data_size) ? data_size : 0;
        }
    }
    ///@copydoc fep3::arya::IRawMemory::resize
    size_t resize(size_t data_size) override
    {
        if (data_size != size())
        {
            //usually throw 
            return size();
        }
        else
        {
            return size();
        }
    }
};

/**
 * @brief Helper class to wrap up a standard layout type as IRawMemory interface
 *
 * @tparam T
 */
template<class T>
struct RawMemoryStandardType<T,
    typename std::enable_if<std::is_const<T>::value>::type>
    : public IRawMemory
{
    ///the value reference
    T& _value;
    /**
    * CTOR
    * @param value reference to the representing value
    */
    RawMemoryStandardType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return sizeof(T);
    }
    const void* cdata() const override
    {
        return &_value;
    }
    size_t size() const override
    {
        return sizeof(T);
    }

    size_t set(const void* /*data*/, size_t /*data_size*/) override
    {
        //usually throw
        return 0;
    }
    size_t resize(size_t /*data_size*/) override
    {
        //usually throw
        return 0;
    }
};

/**
 * @brief Helper class to wrap up a class type as IRawMemory interface
 * the given class type must provide an interface to access
 * \p capacity(), \p size(), \p cdata(), \p resize()
 *
 * @tparam T
 * @tparam Enable check for constness
 */
template<class T, typename Enable = void>
struct RawMemoryClassType : public IRawMemory
{
    /// reference to the representing value
    T& _value;
    /**
     * CTOR
     * @param value reference to the representing value
     */
    RawMemoryClassType(T& value) : _value(value)
    {
    }
    /// @copydoc fep3::arya::IRawMemory::capacity
    size_t capacity() const override
    {
        return _value.capacity();
    }
    /// @copydoc fep3::arya::IRawMemory::cdata
    const void* cdata() const override
    {
        return _value.cdata();
    }
    /// @copydoc fep3::arya::IRawMemory::size
    size_t size() const override
    {
        return _value.size();
    }
    /// @cond no_documentation
    size_t set(const void* data, size_t /*data_size*/) override
    {
        _value = static_cast<const typename T::value_type*>(data);
        return size();
    }
    /// @endcond no_documentation
    /// @copydoc fep3::arya::IRawMemory::resize
    size_t resize(size_t data_size) override
    {
        _value.resize(data_size);
        return capacity();
    }
};

/**
 * @brief Helper class to wrap up a class type as IRawMemory interface
 * the given class type must provide an interface to access
 * \p capacity(), \p size(), \p cdata(), \p resize()
 *
 * @tparam T
 */
template<typename T>
struct RawMemoryClassType<T,
    typename std::enable_if<std::is_const<T>::value>::type >
    : public IRawMemory
{
    /// reference to the representing value
    T& _value;
    /**
     * CTOR
     * @param value reference to the representing value
     */
    RawMemoryClassType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.cdata();
    }
    size_t size() const override
    {
        return _value.size();
    }
    size_t set(const void* /*data*/, size_t /*data_size*/) override
    {
        //usually throw
        return 0;
    }
    size_t resize(size_t /*data_size*/) override
    {
        //usually throw
        return 0;
    }
};
/**
 * specialized raw memory type for the non standard layout type std::string
 */
template<>
struct RawMemoryClassType<std::string, void> : public IRawMemory
{
    /// type to use fro string
    typedef std::string T;
    /// reference to the representing value
    T& _value;
    /**
     * CTOR
     * @param value reference to the representing value
     */
    RawMemoryClassType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.c_str();
    }
    size_t size() const override
    {
        //we ALWAYS use it with trailing \0
        return _value.size() + 1;
    }
    size_t set(const void* data, size_t /*data_size*/) override
    {
        _value = static_cast<const typename T::value_type*>(data);
        return size();
    }
    size_t resize(size_t data_size) override
    {
        _value.resize(data_size);
        return capacity();
    }
};

/**
 * @brief specialized raw memory class type to wrap up a std::string as IRawMemory
 */
template<>
struct RawMemoryClassType<const std::string, const std::string> : public IRawMemory
{
    /// definition for type string as value type
    typedef const std::string T;
    ///value reference
    T& _value;
    /**
     * CTOR
     * @param value Reference to the value
     */
    RawMemoryClassType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.c_str();
    }
    size_t size() const override
    {
        //we ALWAYS use it with trailing \0
        return _value.size() + 1;
    }
    size_t set(const void* /*data*/, size_t /*data_size*/) override
    {
        //usually throw
        return 0;
    }
    size_t resize(size_t /*data_size*/) override
    {
        //usually throw
        return 0;
    }
};


/**
 * specialized raw memory type for the non standard layout type std::string
 */
template<>
struct RawMemoryStandardType<std::string, void> : public IRawMemory
{
    ///the value type
    typedef std::string T;
    ///the value reference
    T& _value;
    /**
     * CTOR
     * @param value reference to the value representing as IRawMemory
     */
    RawMemoryStandardType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.c_str();
    }
    size_t size() const override
    {
        //we ALWAYS use it with trailing \0
        return _value.size() + 1;
    }
    size_t set(const void* data, size_t /*data_size*/) override
    {
        _value = static_cast<const typename T::value_type*>(data);
        return size();
    }
    size_t resize(size_t data_size) override
    {
        _value.resize(data_size);
        return capacity();
    }
};

/**
 * specialized raw memory type for the non standard layout type const std::string
 */
template<>
struct RawMemoryStandardType<const std::string, const std::string> : public IRawMemory
{
    /// type to the string as value type of RawMemoryStandardType
    typedef const std::string T;
    /// reference to the value
    T& _value;
    /**
     * CTOR
     * @param value reference to the value representing as IRawMemory
     */
    RawMemoryStandardType(T& value) : _value(value)
    {
    }
    size_t capacity() const override
    {
        return _value.capacity();
    }
    const void* cdata() const override
    {
        return _value.c_str();
    }
    size_t size() const override
    {
        //we ALWAYS use it with trailing \0
        return _value.size() + 1;
    }
    size_t set(const void* /*data*/, size_t /*data_size*/) override
    {
        //usually throw
        return 0;
    }
    size_t resize(size_t /*data_size*/) override
    {
        //usually throw
        return 0;
    }
};

/**
 * Specialized raw memory type for the non standard layout type std::vector
 */
template<typename T>
struct RawMemoryClassType<std::vector<T>, void> : public IRawMemory
{
    /// reference to the representing value
    std::vector<T>& _value;
    /**
     * CTOR
     * @param value reference to the std vector which will be wrapped
     */
    RawMemoryClassType(std::vector<T>& value) : _value(value)
    {
    }

    /**
     * @brief Returns the preallocated memory of the std vector in bytes
     * 
     * @return size_t In bytes
     */
    size_t capacity() const override
    {
        return _value.capacity() * sizeof(T);
    }

    /**
     * @brief Gets the raw pointer to the memory of the std vector
     *
     * @return const void* The pointer
     */
    const void* cdata() const override
    {
        return _value.data();
    }

    /**
     * @brief Gets the size in memory of all elements 
     *
     * @return size_t The size in bytes
     */
    size_t size() const override
    {
        return _value.size() * sizeof(T);
    }

    /**
     * @brief Copies the memory of the given std vector
     *
     * @param [in] data Pointer to c-array of type T
     * @param [in] data_size Size in bytes. Equals to element count multiplied by element size
     * @return size_t The size in bytes that were copied (if not equal to @p data_size, something went wrong)
     */
    size_t set(const void* data, size_t data_size) override
    {
        size_t element_amount = data_size / sizeof(T);
        _value.reserve(element_amount);

        const T* array = static_cast<const T*>(data);
        _value.assign(array, array + element_amount);

        return size();
    }

    /**
     * @brief Resizes the memory
     *
     * @param [in] data_size The size in bytes to resize
     * @return size_t The new size in bytes (if not equal to @p data_size, something went wrong)
     */
    size_t resize(size_t data_size) override
    {
        _value.resize(data_size / sizeof(T));
        return size();
    }
};

}
using arya::RawMemoryRef;
using arya::RawMemoryClassType;
using arya::RawMemoryStandardType;
}

