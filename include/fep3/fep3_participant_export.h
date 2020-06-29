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
 
#ifndef _FEP3_PARTICIPANT_EXPORT_H_INCLUDED_
#define _FEP3_PARTICIPANT_EXPORT_H_INCLUDED_

#ifdef _FEP3_PARTICIPANT_INCLUDED_STATIC
    /// Macro switching between export / import of the fep sdk shared object
    #define FEP3_PARTICIPANT_EXPORT /**/

#else //_FEP3_PARTICIPANT_INCLUDED_STATIC

    #ifdef WIN32
        #ifdef _FEP3_PARTICIPANT_DO_EXPORT
            /// Macro switching between export / import of the fep sdk shared object
            #define FEP3_PARTICIPANT_EXPORT __declspec( dllexport )
        #else   // _FEP3_PARTICIPANT_DO_EXPORT
            /// Macro switching between export / import of the fep sdk shared object
            #define FEP3_PARTICIPANT_EXPORT __declspec( dllimport )
        #endif
    #else   // WIN32
        #ifdef _FEP3_PARTICIPANT_DO_EXPORT
            /// Macro switching between export / import of the fep sdk shared object
            #define FEP3_PARTICIPANT_EXPORT __attribute__ ((visibility("default")))
        #else   // _FEP3_PARTICIPANT_DO_EXPORT
            /// Macro switching between export / import of the fep sdk shared object
            #define FEP3_PARTICIPANT_EXPORT
        #endif
    #endif
#endif //_FEP3_PARTICIPANT_INCLUDED_STATIC

#endif // _FEP3_PARTICIPANT_EXPORT_H_INCLUDED_
