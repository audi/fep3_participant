##################################################################
# @file
# @copyright AUDI AG
#            All right reserved.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
##################################################################

add_subdirectory(base)
add_subdirectory(scenarios)
add_subdirectory(native_components/base/src)
#add_subdirectory(foreign_components/c/src)

add_subdirectory(participant/cpp)
add_subdirectory(participant/core)
