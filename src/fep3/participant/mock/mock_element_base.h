/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/fep3_result_decl.h>
#include <fep3/core/element_base.h>

namespace fep3
{
namespace mock
{

class MockElementBase
    : public fep3::core::ElementBase
{
public:
    MockElementBase()
        : ElementBase("test_element", "0.0.1")
    {}
    MOCK_METHOD0(die, void());
    virtual ~MockElementBase()
    {
        die();
    }

    // mocked non-final methods of ElementBase
    MOCK_METHOD0(initialize, Result());
    MOCK_METHOD0(deinitialize, void());
    MOCK_METHOD0(run, Result());
    MOCK_METHOD0(stop, void());
};

} // namespace mock
} // namespace fep3