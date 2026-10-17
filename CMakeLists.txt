#
# Copyright (c) 2026 HsingYun (iakext@gmail.com)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.16)
project(UWF LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ASan+UBSan
option(UWF_SANITIZE "Build with AddressSanitizer + UBSan (Clang/GCC only)" ON)
option(UWF_BUILD_TESTS "Build UWF test suites" ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(tools)

find_package(Qt6 COMPONENTS Widgets Svg Network LinguistTools REQUIRED)
qt_standard_project_setup()

set(UWF_OVERLAY_RUNTIME_SOURCES
        src/ui/CenteredTextStyle.h
        src/ui/CenteredTextStyle.cpp
        src/ui/FlowLayout.h
        src/ui/FlowLayout.cpp
        src/ui/VerticalScrollArea.h
        src/ui/VerticalScrollArea.cpp
        src/ui/ContentHintBox.h
        src/ui/ContentHintBox.cpp
        src/ui/OverlayFloatingWidget.h
        src/ui/OverlayFloatingWidget.cpp
        src/ui/OverlayHubView.h
        src/ui/OverlayHubView.cpp
        src/ui/OverlayHub.h
        src/ui/OverlayHub.cpp
        src/ui/OverlayUsageState.h
        src/ui/OverlayTaskbarWidget.h
        src/ui/OverlayTaskbarWidget.cpp
        src/ui/TaskbarLayoutStrategy.h
        src/ui/TaskbarLayoutCoordinator.h
        src/ui/TaskbarLayoutCoordinator.cpp
        src/ui/TaskbarLayoutCoordinatorFactory.cpp
        src/ui/Win11TaskbarEnvironment.h
        src/ui/Win11TaskbarEnvironment.cpp
        src/ui/Win11TaskbarLayoutStrategy.h
        src/ui/Win11TaskbarLayoutStrategy.cpp
        src/ui/OverlayHudPalette.h
        src/ui/OverlayHudRenderer.h
        src/ui/OverlayHudRenderer.cpp
        src/ui/ThemeManager.h
        src/ui/ThemeManager.cpp
        src/ui/I18n.h
        src/ui/I18n.cpp
        src/util/Log.h
        src/util/Log.cpp
        src/util/RegistryKey.h
        src/util/RegistryKey.cpp
        src/util/StringUtil.h
        src/util/StringUtil.cpp
        src/util/WindowsFileSystem.h
        src/util/WindowsFileSystem.cpp
        src/util/WindowsVersion.h
        src/util/WindowsVersion.cpp
)

add_library(UwfOverlayRuntime STATIC ${UWF_OVERLAY_RUNTIME_SOURCES})
target_include_directories(UwfOverlayRuntime PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(UwfOverlayRuntime PUBLIC Qt6::Widgets Qt6::Svg PRIVATE advapi32 user32 shcore dwmapi)
uwf_configure_cpp_target(UwfOverlayRuntime)

set(UWF_SOURCES
        main.cpp
        src/app/CrashHandler.h
        src/app/CrashHandler.cpp
        src/app/ApplicationCommand.h
        src/app/ApplicationCommand.cpp
        src/app/SecureSingleInstance.h
        src/app/SecureSingleInstance.cpp
        src/app/StartupOptions.h
        src/app/StartupOptions.cpp
        src/app/UserConfig.h
        src/app/UserConfig.cpp
        src/app/FileStagingStore.h
        src/app/FileStagingStore.cpp
        src/app/FileStagingCodec.h
        src/app/FileStagingCodec.cpp
        src/app/FileStagingConflictPolicy.h
        src/app/FileStagingConflictPolicy.cpp
        src/service/EnhancedModeAgent.h
        src/service/EnhancedModeAgent.cpp
        src/service/EnhancedModePipeIo.h
        src/service/EnhancedModePipeIo.cpp
        src/service/EnhancedModeService.h
        src/service/EnhancedModeServiceControl.cpp
        src/service/EnhancedModeServiceRuntime.cpp
        src/util/DriveLetter.h
        src/util/DriveLetter.cpp
        src/util/ByteFormat.h
        src/util/ByteFormat.cpp
        src/util/PathMatch.h
        src/util/PathMatch.cpp
        src/util/SystemHardwareInfo.h
        src/util/SystemHardwareInfo.cpp
        src/core/Config.h
        src/core/UwfModel.h
        src/core/UwfModel.cpp
        src/core/RegistryExclusionPolicy.h
        src/core/RegistryExclusionPolicy.cpp
        src/uwf/SystemCheck.h
        src/uwf/SystemCheck.cpp
        src/uwf/wmi/WmiClient.h
        src/uwf/wmi/WmiClient.cpp
        src/uwf/wmi/WmiError.h
        src/uwf/wmi/WmiError.cpp
        src/uwf/wmi/WmiException.h
        src/uwf/wmi/WmiException.cpp
        src/uwf/wmi/WmiRowUtil.h
        src/uwf/wmi/WmiRowUtil.cpp
        src/uwf/api/Types.h
        src/uwf/api/WriteVerification.h
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
        src/uwf/FileStagingCommitter.h
        src/uwf/FileStagingCommitter.cpp
        src/uwf/FileStagingTask.h
        src/uwf/FileStagingTask.cpp
        src/uwf/RegistryTreeCommitter.h
        src/uwf/RegistryTreeCommitter.cpp
        src/ui/MainWindow.h
        src/ui/MainWindow.cpp
        src/ui/AboutDialog.h
        src/ui/AboutDialog.cpp
        src/ui/SystemInformationDialog.h
        src/ui/SystemInformationDialog.cpp
        src/ui/CommitDispatcher.h
        src/ui/CommitDispatcher.cpp
        src/ui/EnhancedModeDialog.h
        src/ui/EnhancedModeDialog.cpp
        src/ui/FileStagingCoordinator.h
        src/ui/FileStagingCoordinator.cpp
        src/ui/FileStagingPresentation.h
        src/ui/FileStagingPresentation.cpp
        src/ui/TrayController.h
        src/ui/TrayController.cpp
        src/ui/ApplyPlanDialog.h
        src/ui/ApplyPlanDialog.cpp
        src/ui/PendingCollect.h
        src/ui/PendingCollect.cpp
        src/ui/CommitBatch.h
        src/ui/CommitReportDialog.h
        src/ui/CommitReportDialog.cpp
        src/ui/RegistryPickerDialog.h
        src/ui/RegistryPickerDialog.cpp
        src/ui/TransientLabel.h
        src/ui/TransientLabel.cpp
        src/ui/SystemInfoProvider.h
        src/ui/SystemInfoProvider.cpp
        src/ui/DiagnosticReportProvider.h
        src/ui/DiagnosticReportProvider.cpp
        src/ui/ProcessTokenDiagnostics.h
        src/ui/ProcessTokenDiagnostics.cpp
        src/ui/HoverHintController.h
        src/ui/HoverHintController.cpp
        src/ui/WindowChromeController.h
        src/ui/WindowChromeController.cpp
        src/ui/OverlayPresentationController.h
        src/ui/OverlayPresentationController.cpp
        src/ui/PowerConfirmDialog.h
        src/ui/PowerConfirmDialog.cpp
        src/ui/UiTiming.h
        src/ui/PowerController.h
        src/ui/PowerController.cpp
        src/ui/LogViewerDialog.h
        src/ui/LogViewerDialog.cpp
        src/ui/TableText.h
        src/ui/TableText.cpp
        src/ui/PathElideDelegate.h
        src/ui/DiskTab.h
        src/ui/DiskTab.cpp
        src/ui/StatusPanel.h
        src/ui/StatusPanel.cpp
        src/ui/StatusBanner.h
        src/ui/StatusBanner.cpp
        src/ui/RoundedCornerOverlay.h
        src/ui/RoundedCornerOverlay.cpp
        src/ui/GlobalStatusPanel.h
        src/ui/GlobalStatusPanel.cpp
        src/ui/ExclusionListWidget.h
        src/ui/ExclusionListWidget.cpp
        src/ui/FileStagingWidget.h
        src/ui/FileStagingWidget.cpp
        src/ui/OverlayFilesDialog.h
        src/ui/OverlayFilesDialog.cpp
        src/ui/ImportDialog.h
        src/ui/ImportDialog.cpp
        src/ui/ImportApplier.h
        src/ui/ImportApplier.cpp
        src/ui/SwitchButton.h
        src/ui/SwitchButton.cpp
        src/ui/OverlayHubFactory.h
        src/ui/OverlayHubFactory.cpp
        src/ui/OverlayUsageBar.h
        src/ui/OverlayUsageBar.cpp
        src/ui/UsageBarGeometry.h
        src/ui/UiUtil.h
        src/ui/UiUtil.cpp
        src/ui/Pager.h
        src/ui/Dialogs.h
        src/ui/Dialogs.cpp
)

qt_add_executable(UWF WIN32 ${UWF_SOURCES})
cc_git_rev(UWF)

set(UWF_APP_RESOURCE_FILES
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
        resources/icons/key.svg
        resources/icons/license.svg
        resources/icons/arrow_up_dark.svg
        resources/icons/arrow_down_dark.svg
        resources/icons/arrow_up_light.svg
        resources/icons/arrow_down_light.svg
        resources/icons/arrow_right.svg
        resources/icons/check_dark.svg
        resources/icons/check_light.svg
        resources/icons/shutdown.svg
        resources/icons/restart.svg
        resources/icons/enhanced.svg
        resources/icons/commit.svg
        resources/icons/theme_sun.svg
        resources/icons/theme_moon.svg
        resources/icons/log.svg
        resources/icons/language.svg
        resources/icons/overlay_float_on.svg
        resources/icons/overlay_float_off.svg
)

qt_add_resources(UWF "app_resources"
        PREFIX "/"
        BASE "${CMAKE_SOURCE_DIR}/resources"
        FILES ${UWF_APP_RESOURCE_FILES}
)

qt_add_translations(UWF
        TS_FILES
        resources/i18n/zh_CN.ts
        resources/i18n/qtbase_zh_CN.ts
)

target_link_libraries(UWF PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Svg Qt6::Network advapi32 shell32 userenv wtsapi32)
uwf_configure_target(UWF)

qt_import_plugins(UWF NO_DEFAULT
        INCLUDE_BY_TYPE platforms Qt6::QWindowsIntegrationPlugin
        INCLUDE_BY_TYPE styles Qt6::QModernWindowsStylePlugin
        INCLUDE_BY_TYPE iconengines Qt6::QSvgIconPlugin
        INCLUDE_BY_TYPE imageformats Qt6::QSvgPlugin
)

if (UWF_BUILD_TESTS AND NOT MSVC)
    enable_testing()
    find_package(Qt6 COMPONENTS Test REQUIRED)
    add_subdirectory(tests)
endif ()
