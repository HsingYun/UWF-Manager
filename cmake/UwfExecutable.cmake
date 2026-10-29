# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

include(UwfArtifacts)
include(UwfResources)
include(UwfTranslations)

qt_add_executable(UWF WIN32 main.cpp)
target_link_libraries(UWF PRIVATE UwfBootstrap UwfVersion Qt6::Widgets)
target_sources(UWF PRIVATE "${PROJECT_SOURCE_DIR}/resources/app.rc")
if(MSVC)
    target_sources(UWF PRIVATE "${PROJECT_SOURCE_DIR}/resources/app.manifest")
else()
    target_sources(UWF PRIVATE "${PROJECT_SOURCE_DIR}/resources/app_manifest.rc")
endif()
uwf_configure_application_target(UWF)
uwf_add_resources(UWF app_resources)
uwf_add_translations(UWF)

qt_add_lupdate(
    SOURCE_TARGETS UWF ${UWF_COMPONENT_TARGETS}
    TS_FILES ${UWF_APP_I18N_TS_FILES}
    OPTIONS -no-obsolete -locations none
)

# Keep the production Qt plugin set explicit for static builds.
qt_import_plugins(UWF NO_DEFAULT
    INCLUDE_BY_TYPE platforms Qt6::QWindowsIntegrationPlugin
    INCLUDE_BY_TYPE styles Qt6::QModernWindowsStylePlugin
    INCLUDE_BY_TYPE iconengines Qt6::QSvgIconPlugin
    INCLUDE_BY_TYPE imageformats Qt6::QSvgPlugin
)
uwf_add_versioned_artifacts(UWF UwfVersion)
