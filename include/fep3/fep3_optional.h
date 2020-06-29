/**
 * @file 
 * @copyright AUDI AG
 *            All right reserved.
 * 
 * This Source Code Form is subject to the terms of the 
 * Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * 
 */
 
#pragma once

#include <exception>
#include <utility>
#include <type_traits>

namespace fep3
{
namespace arya
{

/**
 * Exception class used by \ref Optional
 */
class BadOptionalAccess : public std::exception
{};

/**
 * Optional class
 * @tparam type The type of the (optional) value
 * @remark The interface is partly compatible with std::optional as indroduced in C++17.
 *         The functionality significantly differs from std::optional, e. g. an object of type \p type
 *         exists as long as the instance of this class exists, even if the instance of this class
 *         has no value
 */
template<typename type> class Optional
{
public:
    /**
     * Alias for the type of the contained value
     */
    using value_type = type;
    /**
     * CTOR constructing an optional that has no value
     */
    constexpr Optional() noexcept = default;
    /**
     * CTOR constructing an optional by copying from \p other optional
     * @param other The other value to take the value from
     */
    constexpr Optional(const Optional& other)
    {
        if(other.has_value())
        {
            _has_value = true;
            _value = other.value();
        }
    }
    /**
     * CTOR constructing an optional by copying from \p other optional
     * @tparam other_type The type of the other optional's value
     * @param other The other value to take the value from
     */
    template<typename other_type>
    constexpr Optional(const Optional<other_type>& other)
    {
        if(other.has_value())
        {
            _has_value = true;
            _value = other.value();
        }
    }
    /**
     * CTOR constructing an optional by moving from \p other optional
     * @param other The other value to take the value from
     */
    constexpr Optional(Optional&& other)
    {
        if(other.has_value())
        {
            _has_value = true;
            _value = std::move(other.value());
        }
        other.reset();
    }
    /**
     * CTOR constructing an optional by moving from \p other optional
     * @tparam other_type The type of the other optional's value
     * @param other The other value to take the value from
     */
    template<typename other_type>
    constexpr Optional(Optional<other_type>&& other)
    {
        if(other.has_value())
        {
            _has_value = true;
            _value = std::move(other.value());
        }
        other.reset();
    }
    /**
     * CTOR constructing an optional that has \p other_value as value
     * @tparam other_value_type The type of the other value
     * @param other_value The other value to be used as value
     */
    template
        <typename other_value_type = value_type
        , std::enable_if_t<std::is_assignable<type&, other_value_type>::value, int> = 0
        >
    constexpr Optional(other_value_type&& other_value)
        : _has_value(true)
        , _value(std::forward<other_value_type>(other_value))
    {}
    /**
     * DTOR
     */
    ~Optional() = default;
    /**
     * Assigns \p other to this
     * @param other Other optional to assign from
     * @return Reference to this
     */
    Optional& operator=(const Optional& other)
    {
        if(other.has_value())
        {
            _value = other.value();
        }
        _has_value = other.has_value();
        return *this;
    }
    /**
     * Assigns \p other to this
     * @tparam other_type The type of \p other
     * @param other Other optional to assign from
     * @return Reference to this
     */
    template<typename other_type>
    Optional& operator=(const Optional<other_type>& other)
    {
        if(other.has_value())
        {
            _value = other.value();
        }
        _has_value = other.has_value();
        return *this;
    }
    /**
     * Assigns \p other to this
     * @param other Other optional to move assign from
     * @return Reference to this
     */
    Optional& operator=(Optional&& other)
    {
        if(other.has_value())
        {
            _value = std::move(other.value());
        }
        _has_value = other.has_value();
        other.reset();
        return *this;
    }
    /**
     * Assigns \p other to this
     * @tparam other_type The type of \p other
     * @param other Other optional to move assign from
     * @return Reference to this
     */
    template<typename other_type>
    Optional& operator=(Optional<other_type>&& other)
    {
        if(other.has_value())
        {
            _value = std::move(other.value());
        }
        _has_value = other.has_value();
        other.reset();
        return *this;
    }
    /**
     * Assigns \p other_value the value of to this
     * @tparam other_value_type The type of the other value
     * @param other_value The other value to assign to the value of this
     * @return Reference to this
     */
    template
        <typename other_value_type = value_type
        , std::enable_if_t<std::is_assignable<type&, other_value_type>::value, int> = 0
        >
    Optional& operator=(other_value_type&& other_value)
    {
        _has_value = true;
        _value = std::forward<other_value_type>(other_value);
        return *this;
    }
    /**
     * Dereferences the value
     * @return Const reference to the value
     */
    constexpr const type& operator*() const&
    {
        return _value;
    }
    /**
     * Dereferences the value
     * @return Reference to the value
     */
    type& operator*() &
    {
        return _value;
    }
    /**
     * Dereferences the value
     * @return Const rvalue reference to the value
     */
    constexpr const type&& operator*() const&&
    {
        return std::move(_value);
    }
    /**
     * Dereferences the value
     * @return Rvalue reference to the value
     */
    type&& operator*() &&
    {
        return std::move(_value);
    }
    /**
     * Checks if this has a value
     * @return True if this has a value, false otherwise
     */
    constexpr explicit operator bool() const noexcept
    {
        return _has_value;
    }
    /**
     * Checks if this has a value
     * @return True if this has a value, false otherwise
     */
    constexpr bool has_value() const noexcept
    {
        return _has_value;
    }
    /**
     * Returns a reference to the value
     * @throw BadOptionalAccess if this has no value
     * @return Reference to the value
     */
    type& value() &
    {
        if(!_has_value)
        {
            throw BadOptionalAccess{};
        }
        return _value;
    }
    /**
     * Returns a const reference to the value
     * @throw BadOptionalAccess if this has no value
     * @return Const reference to the value
     */
    constexpr const type& value() const&
    {
        if(!_has_value)
        {
            throw BadOptionalAccess{};
        }
        return _value;
    }
    /**
     * Returns an rvalue reference to the value
     * @throw BadOptionalAccess if this has no value
     * @return Rvalue reference to the value
     */
    type&& value() &&
    {
        if(!_has_value)
        {
            throw BadOptionalAccess{};
        }
        return std::move(_value);
    }
    /**
     * Returns a const rvalue reference to the value
     * @throw BadOptionalAccess if this has no value
     * @return Const rvalue reference to the value
     */
    constexpr const type&& value() const&&
    {
        if(!_has_value)
        {
            throw BadOptionalAccess{};
        }
        return std::move(_value);
    }
    /**
     * Returns the value or the passed \p default_value
     * @tparam default_value_type The type of the \p default_value
     * @param default_value The default value to be returned if this has no value
     * @return The value if this has a value, \p default_value otherwise
     */
    template<typename default_value_type>
    constexpr type value_or(default_value_type&& default_value) const&
    {
        return _has_value ? _value : type{std::move(default_value)};
    }
    /**
     * Returns the value or the passed \p default_value
     * @tparam default_value_type The type of the \p default_value
     * @param default_value The default value to be returned if this has no value
     * @return The value if this has a value, \p default_value otherwise
     */
    template<typename default_value_type>
    type value_or(default_value_type&& default_value) &&
    {
        return _has_value ? _value : type{std::move(default_value)};
    }
    /**
     * Swaps the content with those of @p other
     * @param other The other optional to swap content with
     */
    void swap(Optional& other) noexcept
    {
        if(!_has_value && !other.has_value())
        {}
        else if(!_has_value)
        {
            _has_value = true;
            _value = std::move(other.value());
            other.reset();
        }
        else if(!other.has_value())
        {
            _has_value = false;
            other = std::move(_value);
        }
        else
        {
            std::swap(*this, other);
        }
    }
    /**
     * Resets the optional
     * @post This has no value set
     */
    void reset() noexcept
    {
        _has_value = false;
    }
    /**
     * Emplaces the value by constructing it using \p arguments as parameters to the constructor
     * @tparam argument_types Parameter pack of the arguments to be passed to the constructor
     * @param arguments The arguments to be passed to the constructor
     * @return Reference to this
     */
    template<typename... argument_types>
    type& emplace(argument_types&&... arguments)
    {
        _has_value = true;
        _value = type{std::forward<argument_types>(arguments)...};
        return _value;
    }

private:
    bool _has_value{false};
    type _value{};
};

  /**
  * Comparison for equality of \ref fep3::arya::Optional
  * @tparam lhs_type The type of the left hand side optional
  * @tparam rhs_type The type of the right hand side optional
  * @param lhs The left hand side optional
  * @param rhs The right hand side optional
  * @return True if left and right hand side optionals are equal, false otherwise
  */
template<class lhs_type, class rhs_type>
constexpr bool operator==(const fep3::arya::Optional<lhs_type>& lhs, const fep3::arya::Optional<rhs_type>& rhs)
{
    return (!lhs.has_value() && !rhs.has_value())
        || ((lhs.has_value() && rhs.has_value())
            && (lhs.value() == rhs.value()));
}
/**
* Comparison for inequality of \ref fep3::arya::Optional
* @tparam lhs_type The type of the left hand side optional
* @tparam rhs_type The type of the right hand side optional
* @param lhs The left hand side optional
* @param rhs The right hand side optional
* @return True if left and right hand side optionals are not equal, false otherwise
*/
template<class lhs_type, class rhs_type>
constexpr bool operator!=(const fep3::arya::Optional<lhs_type>& lhs, const fep3::arya::Optional<rhs_type>& rhs)
{
    return !(lhs == rhs);
}

} // namespace arya
using arya::BadOptionalAccess;
using arya::Optional;
} // namespace fep3

