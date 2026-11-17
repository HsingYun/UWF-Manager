# SPDX-License-Identifier: GPL-3.0-or-later
# Simulated implementation of the platform hooks used by the shared build.

if(NOT UWF_BUILD_DESIGN_TOOLS)
    message(FATAL_ERROR "The system application requires Windows. Use -DUWF_BUILD_DESIGN_TOOLS=ON to build the preview tools.")
endif()

# Adapter selection sets the same source and link variables on both platforms.
macro(uwf_select_platform_adapters)
    set(uwf_local_time ${UWF_POSIX_LOCAL_TIME_SOURCES})
    set(uwf_object_names ${UWF_SIMULATED_OBJECT_NAME_COMPARISON_SOURCES})
    set(uwf_object_name_libraries Qt6::Core)
    set(uwf_paths ${UWF_SIMULATED_PATH_OPERATIONS_SOURCES})
    set(uwf_platform_views ${UWF_QT_VIEW_SOURCES})
    set(uwf_design_font_sources ${UWF_QT_DESIGN_FONT_SOURCES})
    set(uwf_platform_desktop_sources "")
    set(uwf_platform_backends "")
    set(uwf_inactive_sources
        ${UWF_WINDOWS_LOCAL_TIME_SOURCES} ${UWF_WINDOWS_DESIGN_FONT_SOURCES}
        ${UWF_OBJECT_NAME_COMPARISON_SOURCES} ${UWF_WINDOWS_PATH_OPERATIONS_SOURCES}
        ${UWF_WINDOWS_VIEW_SOURCES} ${UWF_WINDOWS_INFRASTRUCTURE_SOURCES} ${UWF_SYSTEM_COMPOSITION_SOURCES}
        ${UWF_SERVICE_HOST_SOURCES} ${UWF_WINDOWS_DESKTOP_COMPOSITION_SOURCES}
        ${UWF_CONSOLE_COMPOSITION_SOURCES} ${UWF_ENTRY_COMPOSITION_SOURCES}
    )
endmacro()

# Simulation uses the shared components without native services or entry points.
macro(uwf_add_platform_components)
endmacro()

function(uwf_configure_platform_cpp_target target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    if(UWF_SANITIZE)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

function(uwf_configure_platform_production_target target)
endfunction()

function(uwf_configure_platform_application_target target)
endfunction()

function(uwf_configure_platform_preview_target target)
    target_sources(${target} PRIVATE qt/PreviewHost.cpp)
    set_target_properties(${target} PROPERTIES
        MACOSX_BUNDLE TRUE
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
        MACOSX_BUNDLE_BUNDLE_NAME "UWF Manager Preview"
        MACOSX_BUNDLE_GUI_IDENTIFIER "dev.uwf-manager.preview")
endfunction()

macro(uwf_add_platform_application)
endmacro()

macro(uwf_add_platform_test_support)
endmacro()

macro(uwf_add_platform_test_cases)
endmacro()
