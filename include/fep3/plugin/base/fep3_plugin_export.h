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

/**
 * @def FEP3_PLUGIN_EXPORT 
 * Defintion for exporting in linux or windows 
 */
#ifdef WIN32
    #define FEP3_PLUGIN_EXPORT __declspec(dllexport)
#else
    #define FEP3_PLUGIN_EXPORT __attribute__ ((visibility ("default")))
#endif
