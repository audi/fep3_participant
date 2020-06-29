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

#include <fep3/components/base/component_iid.h>

class ITestPlugin1
{
protected:
    //we dont want not be deleted thru the interface!!
    virtual ~ITestPlugin1() = default;
public:
    FEP_COMPONENT_IID("ITestPlugin1");

public:
    virtual int32_t get1() = 0;
    virtual void set1(int32_t value) = 0;
};




