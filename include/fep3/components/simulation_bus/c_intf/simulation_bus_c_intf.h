/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file simulation_bus_intf.h
 *
 */

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/c/c_intf/shared_binary_c_intf.h>
#include <fep3/plugin/c/c_intf/destruction_manager_c_intf.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
// C interface dependencies
#include <fep3/base/sample/c_intf/data_sample_c_intf.h>
#include <fep3/base/streamtype/c_intf/stream_type_c_intf.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::ISimulationBus
typedef struct fep3_arya_OISimulationBus* fep3_arya_HISimulationBus;
/// Handle to @ref fep3::arya::ISimulationBus::IDataReader
typedef struct fep3_arya_ISimulationBus_OIDataReader* fep3_arya_ISimulationBus_HIDataReader;
/// Handle to @ref fep3::arya::ISimulationBus::IDataReceiver
typedef struct fep3_arya_ISimulationBus_OIDataReceiver* fep3_arya_ISimulationBus_HIDataReceiver;
/// Handle to @ref fep3::arya::ISimulationBus::IDataWriter
typedef struct fep3_arya_ISimulationBus_OIDataWriter* fep3_arya_ISimulationBus_HIDataWriter;

/// Access structure for @ref fep3::arya::ISimulationBus::IDataReceiver
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_ISimulationBus_HIDataReceiver _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *callByStreamType)(fep3_arya_ISimulationBus_HIDataReceiver, fep3_plugin_c_arya_SDestructionManager, fep3_arya_SIStreamType);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *callByDataSample)(fep3_arya_ISimulationBus_HIDataReceiver, fep3_plugin_c_arya_SDestructionManager, fep3_arya_SIDataSample);
    /// @endcond no_documentation
} fep3_arya_ISimulationBus_SIDataReceiver;
/// Access structure for @ref fep3::arya::ISimulationBus::IDataReader
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_ISimulationBus_HIDataReader _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *size)(fep3_arya_ISimulationBus_HIDataReader, size_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *capacity)(fep3_arya_ISimulationBus_HIDataReader, size_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *pop)(fep3_arya_ISimulationBus_HIDataReader, bool*, fep3_arya_ISimulationBus_SIDataReceiver);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *receive)(fep3_arya_ISimulationBus_HIDataReader, fep3_arya_ISimulationBus_SIDataReceiver);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *stop)(fep3_arya_ISimulationBus_HIDataReader);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getFrontTime)(fep3_arya_ISimulationBus_HIDataReader, int64_t*);
    /// @endcond no_documentation
} fep3_arya_ISimulationBus_SIDataReader;
/// Access structure for @ref fep3::arya::ISimulationBus::IDataWriter
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_ISimulationBus_HIDataWriter _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *writeDataSample)(fep3_arya_ISimulationBus_HIDataWriter, int32_t*, fep3_arya_SIDataSample);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *writeStreamType)(fep3_arya_ISimulationBus_HIDataWriter, int32_t*, fep3_arya_SIStreamType);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *transmit)(fep3_arya_ISimulationBus_HIDataWriter, int32_t*);
    /// @endcond no_documentation
} fep3_arya_ISimulationBus_SIDataWriter;
/// Access structure for @ref fep3::arya::ISimulationBus
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HISimulationBus _handle;
    /// Base class @ref fep3::arya::IComponent
    fep3_arya_SIComponent _component;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *isSupported)(fep3_arya_HISimulationBus, bool*, fep3_arya_SIStreamType);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getReaderByNameAndStreamType)
        (fep3_arya_HISimulationBus
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_ISimulationBus_SIDataReader*
        , const char*
        , fep3_arya_SIStreamType
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getReaderByNameAndStreamTypeAndQueueCapacity)
        (fep3_arya_HISimulationBus
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_ISimulationBus_SIDataReader*
        , const char*
        , fep3_arya_SIStreamType
        , size_t
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getReaderByName)
        (fep3_arya_HISimulationBus
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_ISimulationBus_SIDataReader*
        , const char*
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getReaderByNameAndQueueCapacity)
        (fep3_arya_HISimulationBus
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_ISimulationBus_SIDataReader*
        , const char*
        , size_t
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getWriterByNameAndStreamType)
        (fep3_arya_HISimulationBus
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_ISimulationBus_SIDataWriter*
        , const char*
        , fep3_arya_SIStreamType
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getWriterByNameAndStreamTypeAndQueueCapacity)
        (fep3_arya_HISimulationBus
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_ISimulationBus_SIDataWriter*
        , const char*
        , fep3_arya_SIStreamType
        , size_t
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getWriterByName)
        (fep3_arya_HISimulationBus
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_ISimulationBus_SIDataWriter*
        , const char*
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getWriterByNameAndQueueCapacity)
        (fep3_arya_HISimulationBus
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_ISimulationBus_SIDataWriter*
        , const char*
        , size_t
        );
    /// @endcond no_documentation
} fep3_arya_SISimulationBus;

/** @brief Gets a simulation bus that implements the interface identified by @p iid and provides access to it via @p access_result
 *
 * @param[in,out] access_result Pointer to an access structure providing access to the component to get;
 *                              if null, no object will be get and the parameter remains unchanged
 * @param iid IID of the component to be created
 * @param handle_to_component Handle to the interface of the component to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The @p handle is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The @p access_result is null
 */
fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getSimulationBus
    (fep3_arya_SISimulationBus* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    );

/// defines the symbol name of the function that creates a simulation bus
#define SYMBOL_fep3_plugin_c_arya_createSimulationBus fep3_plugin_c_arya_createSimulationBus

/** @brief Creates a simulation bus that implements the interface identified by \p iid and provides access to it via \p access
 *
 * @param[in,out] access Pointer to an access structure providing access to the created component;
 *                       if null, no object will be created and the parameter remains unchanged
 * @param shared_binary_access Access structure to the shared binary the component will reside in
 * @param iid IID of the component to be created
 * @return error code (if any)
 */
FEP3_PLUGIN_EXPORT fep3_plugin_c_InterfaceError FEP3_PLUGIN_CALL fep3_plugin_c_arya_createSimulationBus
    (fep3_arya_SISimulationBus* access
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    );

#ifdef __cplusplus
}
#endif
