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
#ifndef _FEP3_COMPONENTS_H_
#define _FEP3_COMPONENTS_H_

/**
 * @brief Macro defines a static internal function.
 * @param iid the interface identifier as string which is use for the getComponentIID functionality of the @ref fep3::arya::IComponents registry
 * 
 */
#define FEP_COMPONENT_IID(iid) \
static constexpr const char* const FEP3_COMP_IID = iid; \
static const char* getComponentIID() \
{                        \
    return iid;          \
}

/**
* @brief Macro defines a static internal function.
* @param iid the interface identifier as string which is use for the getComponentIID functionality of the @ref fep3::arya::IComponents registry
*
*/
#define FEP3_COMPONENT_IID(iid) FEP_COMPONENT_IID(iid)

#endif // _FEP3_COMPONENTS_H_

