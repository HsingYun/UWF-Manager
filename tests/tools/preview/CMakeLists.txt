# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)
qt_add_executable(UwfDesignPreview DesignPreview.cpp)
target_link_libraries(UwfDesignPreview PRIVATE UwfUiDesign)
uwf_configure_cpp_target(UwfDesignPreview)

qt_add_executable(UwfApplicationPreview
    ApplicationPreview.cpp
    ../../support/app/ConfigurationFixture.cpp
    ../../support/app/EnhancedModeFixture.cpp
    ../../support/app/FileStagingFixture.cpp
)
target_include_directories(UwfApplicationPreview PRIVATE "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfApplicationPreview PRIVATE UwfBootstrap)
uwf_configure_cpp_target(UwfApplicationPreview)
uwf_add_resources(UwfApplicationPreview UwfApplicationPreview_assets)
uwf_add_translations(UwfApplicationPreview)
