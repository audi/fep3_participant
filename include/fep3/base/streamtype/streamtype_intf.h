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
#ifndef _FEP3_BASE_STREAMTYPE_INTF_H_
#define _FEP3_BASE_STREAMTYPE_INTF_H_

#include <string>
#include "../properties/properties_intf.h"

namespace fep3
{
namespace arya
{
/**
 * Definition of a streamtype interface.
 * The streamtype is a composition of properties
 * (name value pairs) to describe a stream or data sample content.
 *
 * @see page_stream_type
 */
class IStreamType
    : public IProperties

{
protected:
    /**
     * @brief DTOR
     * @note This DTOR is explicitly protected to prevent destruction via this interface.
     */
    virtual ~IStreamType() = default;

public:
    /**
     * @brief Returns the streammetatypes name (@see fep3::StreamMetaType)
     * @return std::string The streammetatype name
     */
    virtual std::string getMetaTypeName() const = 0;
};

}
using arya::IStreamType;
}
/**
 * @brief Bool operator to compare streamtypes.
 * Streamtypes are equal if the name of the stream metatypes and the set properties are equal.
 * @param [in] left Left streamtype
 * @param [in] right Right streamtype
 * @return Bool value if equal or not
 */
inline bool operator==(const fep3::arya::IStreamType& left, const fep3::arya::IStreamType& right)
{
    if (std::string(left.getMetaTypeName()) != right.getMetaTypeName())
    {
        return false;
    }
    else
    {
        return left.isEqual(right);
    }
}

#endif //_FEP3_BASE_STREAMTYPE_INTF_H_



