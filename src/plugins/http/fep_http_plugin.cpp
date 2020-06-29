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

#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>
#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>
#include <fep3/components/base/component_base.h>
#include <fep3/native_components/service_bus/service_bus.h>
#include <functional>


void fep3_plugin_getPluginVersion(void(*callback)(void*, const char*), void* destination)
{
    callback(destination, FEP3_PARTICIPANT_LIBRARY_VERSION_STR);
}

fep3::ICPPPluginComponentFactory* fep3_plugin_cpp_arya_getFactory()
{
    return new fep3::arya::CPPPluginComponentFactory<fep3::native::ServiceBus>();
}
