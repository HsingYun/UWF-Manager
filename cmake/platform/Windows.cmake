# SPDX-License-Identifier: GPL-3.0-or-later
# Windows implementation of the platform hooks used by the shared build.

# Adapter selection sets the same source and link variables on both platforms.
macro(uwf_select_platform_adapters)
    set(uwf_local_time ${UWF_WINDOWS_LOCAL_TIME_SOURCES})
    set(uwf_object_names ${UWF_OBJECT_NAME_COMPARISON_SOURCES})
    set(uwf_object_name_libraries kernel32)
    set(uwf_paths ${UWF_WINDOWS_PATH_OPERATIONS_SOURCES})
    set(uwf_platform_views ${UWF_WINDOWS_VIEW_SOURCES})
    set(uwf_design_font_sources ${UWF_WINDOWS_DESIGN_FONT_SOURCES})
    set(uwf_platform_desktop_sources ${UWF_WINDOWS_DESKTOP_COMPOSITION_SOURCES})
    set(uwf_platform_backends UwfWindowsInfrastructure)
    set(uwf_inactive_sources
        ${UWF_POSIX_LOCAL_TIME_SOURCES} ${UWF_QT_DESIGN_FONT_SOURCES}
        ${UWF_SIMULATED_OBJECT_NAME_COMPARISON_SOURCES} ${UWF_SIMULATED_PATH_OPERATIONS_SOURCES} ${UWF_QT_VIEW_SOURCES}
    )
endmacro()

# Component registration must update UWF_COMPONENT_TARGETS in the caller's scope.
macro(uwf_add_platform_components)
    uwf_add_component(UwfWindowsInfrastructure UWF_WINDOWS_INFRASTRUCTURE_SOURCES)
    target_link_libraries(UwfWindowsInfrastructure
        PUBLIC UwfApplication
        PRIVATE UwfCompatibilityRuntime UwfObjectNameComparison UwfVersion Qt6::Network advapi32 shell32 userenv wtsapi32 wbemuuid ole32 oleaut32 dxgi powrprof
    )
    if(MSVC OR NOT UWF_SANITIZE)
        target_link_libraries(UwfWindowsInfrastructure PRIVATE dbghelp)
    endif()

    uwf_add_component(UwfSystemComposition UWF_SYSTEM_COMPOSITION_SOURCES)
    target_link_libraries(UwfSystemComposition PUBLIC UwfWindowsInfrastructure UwfConfigurationComposition PRIVATE UwfCompatibilityRuntime UwfRequestPrecheck)
    if(UWF_ENABLE_FAILURE_SOLVER)
        target_compile_definitions(UwfSystemComposition PRIVATE UWF_ENABLE_FAILURE_SOLVER)
    endif()

    uwf_add_component(UwfServiceHost UWF_SERVICE_HOST_SOURCES)
    target_link_libraries(UwfServiceHost PUBLIC UwfSystemComposition)

    target_link_libraries(UwfViews PRIVATE UwfWindowsInfrastructure advapi32 shell32 user32 shcore dwmapi)
    target_link_libraries(UwfDesktopComposition PUBLIC UwfSystemComposition PRIVATE UwfServiceHost)

    uwf_add_component(UwfConsoleComposition UWF_CONSOLE_COMPOSITION_SOURCES)
    set_target_properties(UwfConsoleComposition PROPERTIES AUTOMOC OFF AUTOUIC OFF)
    target_link_libraries(UwfConsoleComposition PUBLIC UwfSystemComposition PRIVATE UwfConsolePresentation UwfAllVolumeProtection UwfViews)

    uwf_add_component(UwfEntryComposition UWF_ENTRY_COMPOSITION_SOURCES)
    set_target_properties(UwfEntryComposition PROPERTIES AUTOMOC OFF AUTOUIC OFF)
    target_link_libraries(UwfEntryComposition PRIVATE UwfDesktopComposition UwfConsoleComposition UwfServiceHost)
endmacro()

if(MSVC)
    include(UwfMsvcIncludes)
endif()

function(uwf_configure_platform_cpp_target target)

    set(_sanitize_enabled OFF)
    if(UWF_SANITIZE AND NOT MSVC)
        set(_sanitize_enabled ON)
    endif()

    target_compile_definitions(${target} PRIVATE
        UNICODE _UNICODE _WIN32_DCOM NOMINMAX
    )
    if(_sanitize_enabled)
        target_compile_definitions(${target} PRIVATE UWF_SANITIZE)
    endif()

    if(MSVC)
        set_target_properties(${target} PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
        target_compile_options(${target} PRIVATE
            /utf-8 /Zc:__cplusplus
            /W4 /WX /permissive-
            /GS
            $<$<CONFIG:Release>:/Os>
            $<$<CONFIG:Release>:/Gy>
            $<$<CONFIG:Release>:/Gw>
            $<$<CONFIG:Release>:/guard:cf>
            $<$<CONFIG:Release>:/sdl>
        )
        return()
    endif()

    target_compile_options(${target} PRIVATE
        -Wall -Wextra -Wpedantic
        -Wconversion -Wsign-conversion
        -Werror
        -Werror=sign-compare
        -Werror=conversion
        -Werror=sign-conversion
        -Werror=narrowing
        -fstack-protector-strong
    )
    target_link_options(${target} PRIVATE -fstack-protector-strong)
    if(_sanitize_enabled)
        target_compile_options(${target} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -fno-sanitize-recover=all
        )
        target_link_options(${target} PRIVATE
            -fsanitize=address,undefined
            -fuse-ld=lld
        )
    else()
        target_link_options(${target} PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
            -fuse-ld=lld
            -Wl,-Bstatic
        )
    endif()
endfunction()

function(uwf_configure_platform_production_target target)
    if(MSVC)
        set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
        target_compile_options(${target} PRIVATE $<$<CONFIG:Release>:/Zi>)
    endif()
endfunction()

function(uwf_configure_platform_application_target target)
    if(MSVC)
        set_target_properties(${target} PROPERTIES
            PDB_NAME "${target}"
            PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
        target_link_options(${target} PRIVATE
            "/MANIFEST:NO"
            "/MANIFESTUAC:NO"
            /DEBUG:FULL
            $<$<CONFIG:Release>:/DEPENDENTLOADFLAG:0x800>
            $<$<CONFIG:Release>:/DYNAMICBASE>
            $<$<CONFIG:Release>:/HIGHENTROPYVA>
            $<$<CONFIG:Release>:/NXCOMPAT>
            $<$<CONFIG:Release>:/GUARD:CF>
            $<$<CONFIG:Release>:/CETCOMPAT>
            $<$<CONFIG:Release>:/OPT:REF>
            $<$<CONFIG:Release>:/OPT:ICF>
        )
    endif()
endfunction()

function(uwf_configure_platform_preview_target target)
    target_sources(${target} PRIVATE windows/PreviewHost.cpp)
    if(MSVC)
        target_sources(${target} PRIVATE ApplicationPreview.manifest)
    else()
        target_sources(${target} PRIVATE ApplicationPreview_manifest.rc)
        target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endfunction()

macro(uwf_add_platform_application)
    include(UwfExecutable)
endmacro()

macro(uwf_add_platform_test_support)
    include("${PROJECT_SOURCE_DIR}/support/tests/support/WindowsSupport.cmake")
endmacro()

macro(uwf_add_platform_test_cases)
    include("${PROJECT_SOURCE_DIR}/support/tests/cases/WindowsCases.cmake")
endmacro()
