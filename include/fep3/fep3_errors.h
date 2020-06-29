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
#ifndef _FEP3_ERRORS_H
#define _FEP3_ERRORS_H

#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>

#include "fep3_result_decl.h"

namespace fep3
{
    /// \copydoc a_util::result::is_ok
    using a_util::result::isOk;
    /// \copydoc a_util::result::is_failed
    using a_util::result::isFailed;

    // TODO has to be removed once ODAUTIL-168 is fixed
    #ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #endif

    /// \cond nodoc
    _MAKE_RESULT(0, ERR_NOERROR);
    _MAKE_RESULT(-2, ERR_UNKNOWN);
    _MAKE_RESULT(-3, ERR_UNEXPECTED);
    _MAKE_RESULT(-4, ERR_POINTER);
    _MAKE_RESULT(-5, ERR_INVALID_ARG);
    _MAKE_RESULT(-6, ERR_INVALID_FUNCTION);
    _MAKE_RESULT(-7, ERR_INVALID_ADDRESS);
    _MAKE_RESULT(-8, ERR_INVALID_HANDLE);
    _MAKE_RESULT(-9, ERR_INVALID_FLAGS);
    _MAKE_RESULT(-10, ERR_INVALID_INDEX);
    _MAKE_RESULT(-11, ERR_INVALID_FILE);
    _MAKE_RESULT(-12, ERR_MEMORY);
    _MAKE_RESULT(-13, ERR_TIMEOUT);
    _MAKE_RESULT(-14, ERR_OUT_OF_SYNC);
    _MAKE_RESULT(-15, ERR_RESOURCE_IN_USE);
    _MAKE_RESULT(-16, ERR_NOT_IMPL);
    _MAKE_RESULT(-17, ERR_NO_INTERFACE);
    _MAKE_RESULT(-18, ERR_NO_CLASS);
    _MAKE_RESULT(-19, ERR_NOT_SUPPORTED);
    _MAKE_RESULT(-20, ERR_NOT_FOUND);
    _MAKE_RESULT(-21, ERR_CANCELLED);
    _MAKE_RESULT(-22, ERR_RETRY);
    _MAKE_RESULT(-23, ERR_FILE_NOT_FOUND);
    _MAKE_RESULT(-24, ERR_PATH_NOT_FOUND);
    _MAKE_RESULT(-25, ERR_ACCESS_DENIED);
    _MAKE_RESULT(-26, ERR_NOT_READY);
    _MAKE_RESULT(-27, ERR_OPEN_FAILED);
    _MAKE_RESULT(-28, ERR_IO_INCOMPLETE);
    _MAKE_RESULT(-29, ERR_IO_PENDING);
    _MAKE_RESULT(-30, ERR_NOACCESS);
    _MAKE_RESULT(-31, ERR_BAD_DEVICE);
    _MAKE_RESULT(-32, ERR_DEVICE_IO);
    _MAKE_RESULT(-33, ERR_DEVICE_NOT_READY);
    _MAKE_RESULT(-34, ERR_DEVICE_IN_USE);
    _MAKE_RESULT(-35, ERR_NOT_CONNECTED);
    _MAKE_RESULT(-36, ERR_UNKNOWN_FORMAT);
    _MAKE_RESULT(-37, ERR_NOT_INITIALISED);
    _MAKE_RESULT(-38, ERR_FAILED);
    _MAKE_RESULT(-39, ERR_END_OF_FILE);
    _MAKE_RESULT(-40, ERR_INVALID_STATE);
    _MAKE_RESULT(-41, ERR_EXCEPTION_RAISED);
    _MAKE_RESULT(-42, ERR_INVALID_TYPE);
    _MAKE_RESULT(-43, ERR_EMPTY);
    _MAKE_RESULT(-44, ERR_INVALID_VERSION);
    _MAKE_RESULT(-45, ERR_INVALID_LICENSE);
    _MAKE_RESULT(-46, ERR_SERVICE_NOT_FOUND);
    _MAKE_RESULT(-47, ERR_DAU);
    _MAKE_RESULT(-48, ERR_IDLE_NOWAIT);
    _MAKE_RESULT(-49, ERR_OUT_OF_RANGE);
    _MAKE_RESULT(-50, ERR_KNOWN_PROBLEM);
    /// \endcond

    #ifdef __GNUC__
    #pragma GCC diagnostic pop
    #endif
}

/**
 * @brief returns @param lhs if it is an error otherwise @param rhs
 * @return fep3::Result the merged result code
 */
inline fep3::Result operator|(const fep3::Result& lhs, const fep3::Result& rhs)
{
    if (a_util::result::isFailed(lhs))
    {
        return lhs;
    }
    return rhs;
}

/**
 * @brief returns @param lhs if it is an error otherwise @param rhs
 * @return fep3::Result the merged result code
 */
inline fep3::Result& operator|=(fep3::Result& lhs, const fep3::Result& rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

/// Provide legacy RETURN_IF_FAILED implementation
#ifndef FEP3_RETURN_IF_FAILED
#define FEP3_RETURN_IF_FAILED(s)                                                  \
{                                                                                 \
    fep3::Result _errcode(s);                                                      \
    if (fep3::isFailed(_errcode)) { return (_errcode); }                           \
}
#endif

/// using a printf like parameter list for detailed error description
#ifndef CREATE_ERROR_DESCRIPTION
#define CREATE_ERROR_DESCRIPTION(_errcode, ...)                                                    \
    a_util::result::Result(_errcode,                                                               \
                                  a_util::strings::format(__VA_ARGS__).c_str(),                    \
                                  __LINE__,                                                        \
                                  __FILE__,                                                        \
                                  A_UTIL_CURRENT_FUNCTION)
#endif

#endif // _FEP3_ERRORS_H
