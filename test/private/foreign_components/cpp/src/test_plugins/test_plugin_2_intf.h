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

#include <fep3/components/base/component_iid.h>

class ITestPlugin2
{
protected:
    //we dont want not be deleted thru the interface!!
    virtual ~ITestPlugin2() = default;
public:
    FEP_COMPONENT_IID("ITestPlugin2");

public:
    virtual int32_t get2() = 0;
    virtual void set2(int32_t value) = 0;
};




