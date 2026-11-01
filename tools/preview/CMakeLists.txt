# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)
qt_add_executable(UwfDesignPreview DesignPreview.cpp)
target_link_libraries(UwfDesignPreview PRIVATE UwfDesignSystem)
uwf_configure_cpp_target(UwfDesignPreview)

qt_add_executable(UwfApplicationPreview
    ApplicationPreview.cpp
    ../../tests/support/app/ConfigurationFixture.cpp
    ../../tests/support/app/EnhancedModeFixture.cpp
    ../../tests/support/uwf/FakeUwfBackend.cpp
)
target_include_directories(UwfApplicationPreview PRIVATE "${PROJECT_SOURCE_DIR}/tests")
if(MSVC)
    target_sources(UwfApplicationPreview PRIVATE ApplicationPreview.manifest)
else()
    target_sources(UwfApplicationPreview PRIVATE ApplicationPreview_manifest.rc)
    target_include_directories(UwfApplicationPreview PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
target_link_libraries(UwfApplicationPreview PRIVATE UwfDesktopComposition UwfCompatibilityRuntime)
uwf_configure_cpp_target(UwfApplicationPreview)
uwf_add_resources(UwfApplicationPreview)
uwf_add_translations(UwfApplicationPreview)
