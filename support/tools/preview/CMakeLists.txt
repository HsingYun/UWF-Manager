# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)
include(UwfSimulationSupport)

qt_add_executable(UwfDesignPreview DesignPreview.cpp)
target_link_libraries(UwfDesignPreview PRIVATE UwfDesignSystem)
uwf_configure_cpp_target(UwfDesignPreview)

qt_add_executable(UwfApplicationPreview
        ApplicationPreview.cpp
        PreviewFixtures.h
        PreviewHost.h
)
uwf_configure_platform_preview_target(UwfApplicationPreview)

target_link_libraries(UwfApplicationPreview PRIVATE UwfDesktopComposition UwfSimulationApplication UwfSimulationBackend UwfSimulationUi)
uwf_configure_cpp_target(UwfApplicationPreview)
uwf_add_resources(UwfApplicationPreview)
uwf_add_translations(UwfApplicationPreview)
