cmake_minimum_required(VERSION 3.16)
project(UWF LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(tools)

find_package(Qt6 COMPONENTS Widgets Svg Network LinguistTools REQUIRED)
qt_standard_project_setup()

set(UWF_SOURCES
        main.cpp
        src/util/Log.h
        src/util/Log.cpp
        src/util/DriveLetter.h
        src/util/DriveLetter.cpp
        src/core/UwfConfig.h
        src/core/UwfConfig.cpp
        src/uwf/SystemCheck.h
        src/uwf/SystemCheck.cpp
        src/uwf/wmi/WmiClient.h
        src/uwf/wmi/WmiClient.cpp
        src/uwf/wmi/WmiError.h
        src/uwf/wmi/WmiError.cpp
        src/uwf/wmi/WmiRowUtil.h
        src/uwf/wmi/WmiRowUtil.cpp
        src/uwf/api/Types.h
        src/uwf/api/UwfFilter.h
        src/uwf/api/UwfFilter.cpp
        src/uwf/api/UwfVolume.h
        src/uwf/api/UwfVolume.cpp
        src/uwf/api/UwfOverlay.h
        src/uwf/api/UwfOverlay.cpp
        src/uwf/api/UwfOverlayConfig.h
        src/uwf/api/UwfOverlayConfig.cpp
        src/uwf/api/UwfRegistryFilter.h
        src/uwf/api/UwfRegistryFilter.cpp
        src/uwf/api/UwfmgrCli.h
        src/uwf/api/UwfmgrCli.cpp
        src/uwf/UwfSnapshot.h
        src/uwf/UwfSnapshot.cpp
        src/ui/MainWindow.h
        src/ui/MainWindow.cpp
        src/ui/TrayController.h
        src/ui/TrayController.cpp
        src/ui/ApplyPlanDialog.h
        src/ui/ApplyPlanDialog.cpp
        src/ui/LogViewerDialog.h
        src/ui/LogViewerDialog.cpp
        src/ui/TableText.h
        src/ui/TableText.cpp
        src/ui/DiskTab.h
        src/ui/DiskTab.cpp
        src/ui/StatusPanel.h
        src/ui/StatusPanel.cpp
        src/ui/GlobalStatusPanel.h
        src/ui/GlobalStatusPanel.cpp
        src/ui/ExclusionListWidget.h
        src/ui/ExclusionListWidget.cpp
        src/ui/OverlayFilesDialog.h
        src/ui/OverlayFilesDialog.cpp
        src/ui/ImportDialog.h
        src/ui/ImportDialog.cpp
        src/ui/SwitchButton.h
        src/ui/SwitchButton.cpp
        src/ui/OverlayUsageBar.h
        src/ui/OverlayUsageBar.cpp
        src/ui/CenteredTextStyle.h
        src/ui/CenteredTextStyle.cpp
        src/ui/ThemeManager.h
        src/ui/ThemeManager.cpp
        src/ui/I18n.h
        src/ui/I18n.cpp
        src/ui/MessageDialog.h
        src/ui/MessageDialog.cpp
)

qt_add_executable(UWF WIN32 ${UWF_SOURCES})
cc_git_rev(UWF)

qt_add_resources(UWF "app_resources"
        PREFIX "/"
        BASE "${CMAKE_SOURCE_DIR}/resources"
        FILES
        resources/style_dark.qss
        resources/style_light.qss
        resources/icons/app.svg
        resources/icons/refresh.svg
        resources/icons/add.svg
        resources/icons/remove.svg
        resources/icons/apply.svg
        resources/icons/info.svg
        resources/icons/disk.svg
        resources/icons/disk_off.svg
        resources/icons/disk_system.svg
        resources/icons/file.svg
        resources/icons/folder.svg
        resources/icons/registry.svg
        resources/icons/arrow_up_dark.svg
        resources/icons/arrow_down_dark.svg
        resources/icons/arrow_up_light.svg
        resources/icons/arrow_down_light.svg
        resources/icons/arrow_right.svg
        resources/icons/check_dark.svg
        resources/icons/check_light.svg
        resources/icons/shutdown.svg
        resources/icons/restart.svg
        resources/icons/commit.svg
        resources/icons/theme_sun.svg
        resources/icons/theme_moon.svg
        resources/icons/log.svg
        resources/icons/language.svg
)

qt_add_translations(UWF
        TS_FILES
        resources/i18n/zh_CN.ts
)

target_link_libraries(UWF PRIVATE Qt6::Widgets Qt6::Svg Qt6::Network)

target_sources(UWF PRIVATE resources/app.rc)

if (MSVC)
    target_sources(UWF PRIVATE resources/app.manifest)
else ()
    target_sources(UWF PRIVATE resources/app_manifest.rc)
endif ()

target_compile_definitions(UWF PRIVATE UNICODE _UNICODE _WIN32_DCOM NOMINMAX)

target_link_libraries(UWF PRIVATE wbemuuid ole32 oleaut32 shell32)

if (MSVC)
    set_target_properties(UWF PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
            INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
    )
    target_compile_options(UWF PRIVATE
            /W4 /WX /permissive-
            $<$<CONFIG:Release>:/Os>
    )
    target_link_options(UWF PRIVATE "/MANIFEST:NO" "/MANIFESTUAC:NO")
else ()
    target_compile_options(UWF PRIVATE
            -Wall -Wextra -Wpedantic
            -Wconversion -Wsign-conversion
            -Werror
            -Werror=sign-compare
            -Werror=conversion
            -Werror=sign-conversion
            -Werror=narrowing
    )
    target_link_options(UWF PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
            -fuse-ld=lld
            -Wl,-Bstatic
    )
endif ()