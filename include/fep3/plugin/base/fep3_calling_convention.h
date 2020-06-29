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

/*
 * @brief Calling convention macro
 */
#ifdef WIN32
    #define FEP3_PLUGIN_CALL __stdcall
#else
    #define FEP3_PLUGIN_CALL
#endif
