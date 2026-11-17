# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

if(WIN32)
    include("${CMAKE_CURRENT_LIST_DIR}/platform/Windows.cmake")
else()
    include("${CMAKE_CURRENT_LIST_DIR}/platform/Simulated.cmake")
endif()

include(CheckLayerBoundaries)
include(UwfDependencies)
include(UwfComponents)
uwf_add_platform_application()
include(UwfTesting)
