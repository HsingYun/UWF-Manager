# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

include(UwfSources)
include(UwfTargetOptions)
include(UwfVersion)
include(UwfDesign)

# Include the design library and every component registered below in extraction.
set(UWF_COMPONENT_TARGETS UwfUiDesign)

# QObject implementations include their own moc output so static-library
# consumers do not pull unrelated QObject implementations into the link.
function(uwf_add_component target source_group)
    add_library(${target} STATIC ${${source_group}})
    target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/src")
    uwf_configure_production_target(${target})
    set(UWF_COMPONENT_TARGETS ${UWF_COMPONENT_TARGETS} ${target} PARENT_SCOPE)
endfunction()

uwf_add_version(UwfVersion)

uwf_add_component(UwfShared UWF_SHARED_SOURCES)
set_target_properties(UwfShared PROPERTIES AUTOMOC OFF AUTOUIC OFF)

uwf_add_component(UwfCore UWF_CORE_SOURCES)
set_target_properties(UwfCore PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfCore PUBLIC UwfShared)

uwf_add_component(UwfContracts UWF_CONTRACTS_SOURCES)
target_link_libraries(UwfContracts PUBLIC UwfCore Qt6::Core)

uwf_add_component(UwfApplication UWF_APPLICATION_SOURCES)
target_link_libraries(UwfApplication PUBLIC UwfContracts PRIVATE UwfVersion)

uwf_add_component(UwfPresentation UWF_PRESENTATION_SOURCES)
target_link_libraries(UwfPresentation PUBLIC UwfApplication Qt6::Core)

uwf_add_component(UwfWindows UWF_WINDOWS_SOURCES)
target_link_libraries(UwfWindows
    PUBLIC UwfApplication
    PRIVATE UwfVersion Qt6::Network advapi32 shell32 userenv wtsapi32 wbemuuid ole32 oleaut32 dxgi
)
if(MSVC OR NOT UWF_SANITIZE)
    target_link_libraries(UwfWindows PRIVATE dbghelp)
endif()

uwf_add_component(UwfUi UWF_UI_SOURCES)
target_link_libraries(UwfUi
    PUBLIC UwfPresentation UwfUiDesign Qt6::Widgets Qt6::Svg
    PRIVATE UwfWindows UwfVersion advapi32 shell32 user32 shcore dwmapi
)

uwf_add_component(UwfService UWF_SERVICE_SOURCES)
target_link_libraries(UwfService PUBLIC UwfWindows)

uwf_add_component(UwfBootstrap UWF_BOOTSTRAP_SOURCES)
target_link_libraries(UwfBootstrap
    PUBLIC UwfUi UwfWindows
    PRIVATE UwfService
)

if(UWF_BUILD_DESIGN_TOOLS)
    include(UwfResources)
    include(UwfTranslations)
    add_subdirectory(tests/tools/preview "${PROJECT_BINARY_DIR}/preview")
endif()
