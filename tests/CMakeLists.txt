function(uwf_add_test_resources target)
    set(resource_files)
    foreach (resource_file IN LISTS UWF_APP_RESOURCE_FILES)
        list(APPEND resource_files "${PROJECT_SOURCE_DIR}/${resource_file}")
    endforeach ()
    qt_add_resources(${target} "${target}_resources"
            PREFIX "/"
            BASE "${PROJECT_SOURCE_DIR}/resources"
            FILES ${resource_files}
    )
endfunction()

qt_add_executable(UwfHubLifecycleTests
        HubLifecycleTests.cpp
)

target_include_directories(UwfHubLifecycleTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(UwfHubLifecycleTests PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Test user32 shcore dwmapi)
uwf_configure_cpp_target(UwfHubLifecycleTests)
uwf_add_test_resources(UwfHubLifecycleTests)

add_test(NAME UwfHubLifecycleTests COMMAND UwfHubLifecycleTests)
if (WIN32)
    set_tests_properties(UwfHubLifecycleTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")
else ()
    set_tests_properties(UwfHubLifecycleTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif ()

# 与窗口生命周期测试互补：这里覆盖不依赖真实 WMI provider 的领域规则与
# 序列化边界。测试直接编译生产实现，不复制算法，也不通过测试专用入口绕过
# 公共 API。
qt_add_executable(UwfCoreBehaviorTests
        CoreBehaviorTests.cpp
        ../src/util/ByteFormat.cpp
        ../src/util/DriveLetter.cpp
        ../src/util/PathMatch.cpp
        ../src/core/UwfModel.cpp
        ../src/core/RegistryExclusionPolicy.cpp
        ../src/uwf/api/UwfmgrCli.cpp
        ../src/uwf/wmi/WmiError.cpp
        ../src/uwf/wmi/WmiException.cpp
)
target_include_directories(UwfCoreBehaviorTests PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(UwfCoreBehaviorTests PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Test advapi32 user32)
uwf_configure_cpp_target(UwfCoreBehaviorTests)

add_test(NAME UwfCoreBehaviorTests COMMAND UwfCoreBehaviorTests)
set_tests_properties(UwfCoreBehaviorTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# WMI 数据协议与 UWF API 的领域级契约测试。内存 WmiOperations 实现只替换
# transport；生产解码、参数构造、写后确认和错误分类全部原样执行。
qt_add_executable(UwfWmiApiBehaviorTests
        WmiApiBehaviorTests.cpp
        ../src/core/UwfModel.cpp
        ../src/util/DriveLetter.cpp
        ../src/uwf/SystemCheck.cpp
        ../src/uwf/UwfSnapshot.cpp
        ../src/uwf/api/UwfFilter.cpp
        ../src/uwf/api/UwfOverlay.cpp
        ../src/uwf/api/UwfOverlayConfig.cpp
        ../src/uwf/api/UwfRegistryFilter.cpp
        ../src/uwf/api/UwfVolume.cpp
        ../src/uwf/wmi/WmiClient.cpp
        ../src/uwf/wmi/WmiError.cpp
        ../src/uwf/wmi/WmiException.cpp
        ../src/uwf/wmi/WmiRowUtil.cpp
)
target_include_directories(UwfWmiApiBehaviorTests PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(UwfWmiApiBehaviorTests PRIVATE UwfOverlayRuntime Qt6::Test advapi32 ole32 user32 wbemuuid)
uwf_configure_cpp_target(UwfWmiApiBehaviorTests)

add_test(NAME UwfWmiApiBehaviorTests COMMAND UwfWmiApiBehaviorTests)

# 真实 QWidget 黑盒交互：鼠标、键盘、hover、modal dialog、表格选区和排除列表
# 全部走生产控件及公开行为。电源测试仅操作确认框，不链接 PowerController，因而
# 不可能调用关机/重启；排除测试也只改控件内 pending，不写 WMI 或注册表。
qt_add_executable(UwfUiBehaviorTests
        UiBehaviorTests.cpp
        ../src/util/ByteFormat.cpp
        ../src/util/DriveLetter.cpp
        ../src/core/RegistryExclusionPolicy.cpp
        ../src/ui/Dialogs.cpp
        ../src/ui/ExclusionListWidget.cpp
        ../src/ui/GlobalStatusPanel.cpp
        ../src/ui/HoverHintController.cpp
        ../src/ui/ImportDialog.cpp
        ../src/ui/OverlayUsageBar.cpp
        ../src/ui/PowerConfirmDialog.cpp
        ../src/ui/RegistryPickerDialog.cpp
        ../src/ui/RoundedCornerOverlay.cpp
        ../src/ui/StatusBanner.cpp
        ../src/ui/StatusPanel.cpp
        ../src/ui/SwitchButton.cpp
        ../src/ui/TableText.cpp
        ../src/ui/TransientLabel.cpp
        ../src/ui/UiUtil.cpp
        ../src/uwf/api/UwfmgrCli.cpp
)
target_include_directories(UwfUiBehaviorTests PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(UwfUiBehaviorTests PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Svg Qt6::Test advapi32 shell32 user32)
uwf_configure_cpp_target(UwfUiBehaviorTests)
uwf_add_test_resources(UwfUiBehaviorTests)

add_test(NAME UwfUiBehaviorTests COMMAND UwfUiBehaviorTests)
if (WIN32)
    set_tests_properties(UwfUiBehaviorTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows" LABELS "ui")
else ()
    set_tests_properties(UwfUiBehaviorTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" LABELS "ui")
endif ()

# 应用层真实 QWidget 黑盒测试：语言/主题、通用对话框、日志查看器、磁盘能力
# 边界、导入路由与 pending 汇总均通过生产公开行为验证。系统写操作全部由
# 内存 WMI transport 截断；仅用临时文件和已存在注册表值完成只读前置观察，
# 不执行真实 commit、关机、重启或 Shell 写操作。
qt_add_executable(UwfApplicationUiBehaviorTests
        ApplicationUiBehaviorTests.cpp
        ../src/util/ByteFormat.cpp
        ../src/util/DriveLetter.cpp
        ../src/util/PathMatch.cpp
        ../src/util/SystemHardwareInfo.cpp
        ../src/core/RegistryExclusionPolicy.cpp
        ../src/core/UwfModel.cpp
        ../src/ui/AboutDialog.cpp
        ../src/ui/ApplyPlanDialog.cpp
        ../src/ui/CommitReportDialog.cpp
        ../src/ui/Dialogs.cpp
        ../src/ui/DiskTab.cpp
        ../src/ui/ExclusionListWidget.cpp
        ../src/ui/GlobalStatusPanel.cpp
        ../src/ui/ImportApplier.cpp
        ../src/ui/ImportDialog.cpp
        ../src/ui/LogViewerDialog.cpp
        ../src/ui/MainWindow.cpp
        ../src/ui/MarqueeHintBox.cpp
        ../src/ui/OverlayFilesDialog.cpp
        ../src/ui/OverlayUsageBar.cpp
        ../src/ui/PendingCollect.cpp
        ../src/ui/PowerConfirmDialog.cpp
        ../src/ui/PowerController.cpp
        ../src/ui/RegistryPickerDialog.cpp
        ../src/ui/RoundedCornerOverlay.cpp
        ../src/ui/StatusBanner.cpp
        ../src/ui/StatusPanel.cpp
        ../src/ui/SwitchButton.cpp
        ../src/ui/TableText.cpp
        ../src/ui/UiUtil.cpp
        ../src/ui/CenteredTextStyle.cpp
        ../src/ui/CommitDispatcher.cpp
        ../src/ui/HoverHintController.cpp
        ../src/ui/OverlayHubFactory.cpp
        ../src/ui/OverlayPresentationController.cpp
        ../src/ui/SystemInfoProvider.cpp
        ../src/ui/TrayController.cpp
        ../src/ui/TransientLabel.cpp
        ../src/ui/WindowChromeController.cpp
        ../src/uwf/SystemCheck.cpp
        ../src/uwf/UwfSnapshot.cpp
        ../src/uwf/api/UwfOverlay.cpp
        ../src/uwf/api/UwfFilter.cpp
        ../src/uwf/api/UwfOverlayConfig.cpp
        ../src/uwf/api/UwfRegistryFilter.cpp
        ../src/uwf/api/UwfVolume.cpp
        ../src/uwf/api/UwfmgrCli.cpp
        ../src/uwf/wmi/WmiClient.cpp
        ../src/uwf/wmi/WmiError.cpp
        ../src/uwf/wmi/WmiException.cpp
        ../src/uwf/wmi/WmiRowUtil.cpp
)
target_include_directories(UwfApplicationUiBehaviorTests PRIVATE "${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/generated")
target_link_libraries(UwfApplicationUiBehaviorTests PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Svg Qt6::Test advapi32 dwmapi ole32 shell32 shcore user32 wbemuuid)
uwf_configure_cpp_target(UwfApplicationUiBehaviorTests)
uwf_add_test_resources(UwfApplicationUiBehaviorTests)
qt_add_translations(UwfApplicationUiBehaviorTests
        TS_FILES
        ../resources/i18n/zh_CN.ts
        ../resources/i18n/qtbase_zh_CN.ts
)

add_test(NAME UwfApplicationUiBehaviorTests COMMAND UwfApplicationUiBehaviorTests)
if (WIN32)
    set_tests_properties(UwfApplicationUiBehaviorTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows" LABELS "ui")
else ()
    set_tests_properties(UwfApplicationUiBehaviorTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" LABELS "ui")
endif ()

# 数据密集型对话框的真实 QWidget 行为。覆盖层与注册表选择器通过生产公开的
# 只读数据源边界接收内存快照；路径处理、异步投递、分页、菜单、选择和 modal
# 结果全部仍走生产代码。测试不访问真实 WMI/注册表，也不执行任何写操作。
qt_add_executable(UwfDialogBehaviorTests
        DialogBehaviorTests.cpp
        ../src/util/ByteFormat.cpp
        ../src/util/DriveLetter.cpp
        ../src/core/RegistryExclusionPolicy.cpp
        ../src/ui/CommitReportDialog.cpp
        ../src/ui/Dialogs.cpp
        ../src/ui/OverlayFilesDialog.cpp
        ../src/ui/RegistryPickerDialog.cpp
        ../src/ui/StatusBanner.cpp
        ../src/ui/TableText.cpp
        ../src/ui/UiUtil.cpp
        ../src/uwf/api/UwfOverlay.cpp
        ../src/uwf/wmi/WmiClient.cpp
        ../src/uwf/wmi/WmiError.cpp
        ../src/uwf/wmi/WmiException.cpp
        ../src/uwf/wmi/WmiRowUtil.cpp
)
target_include_directories(UwfDialogBehaviorTests PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(UwfDialogBehaviorTests PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Svg Qt6::Test advapi32 ole32 shell32 user32 wbemuuid)
uwf_configure_cpp_target(UwfDialogBehaviorTests)
uwf_add_test_resources(UwfDialogBehaviorTests)

add_test(NAME UwfDialogBehaviorTests COMMAND UwfDialogBehaviorTests)
if (WIN32)
    set_tests_properties(UwfDialogBehaviorTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows" LABELS "ui")
else ()
    set_tests_properties(UwfDialogBehaviorTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" LABELS "ui")
endif ()

if (WIN32)
    # 真实但只读的 Windows 平台契约：COM/WMI、系统版本/硬件、Embedded 能力
    # 探测和单实例命名管道。该目标不执行 UWF 写入、注册表写入或电源操作。
    qt_add_executable(UwfWindowsPlatformIntegrationTests
            WindowsPlatformIntegrationTests.cpp
            ../src/app/SecureSingleInstance.cpp
            ../src/core/UwfModel.cpp
            ../src/ui/SystemInfoProvider.cpp
            ../src/util/DriveLetter.cpp
            ../src/util/SystemHardwareInfo.cpp
            ../src/uwf/SystemCheck.cpp
            ../src/uwf/UwfSnapshot.cpp
            ../src/uwf/api/UwfFilter.cpp
            ../src/uwf/api/UwfOverlay.cpp
            ../src/uwf/api/UwfOverlayConfig.cpp
            ../src/uwf/api/UwfRegistryFilter.cpp
            ../src/uwf/api/UwfVolume.cpp
            ../src/uwf/wmi/WmiClient.cpp
            ../src/uwf/wmi/WmiError.cpp
            ../src/uwf/wmi/WmiException.cpp
            ../src/uwf/wmi/WmiRowUtil.cpp
    )
    target_include_directories(UwfWindowsPlatformIntegrationTests PRIVATE "${PROJECT_SOURCE_DIR}/src")
    target_link_libraries(UwfWindowsPlatformIntegrationTests PRIVATE UwfOverlayRuntime Qt6::Network Qt6::Widgets Qt6::Test advapi32 dxgi ole32 user32 wbemuuid)
    uwf_configure_cpp_target(UwfWindowsPlatformIntegrationTests)

    add_test(NAME UwfWindowsPlatformIntegrationTests COMMAND UwfWindowsPlatformIntegrationTests)
    set_tests_properties(UwfWindowsPlatformIntegrationTests PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=windows"
            LABELS "windows-integration;read-only"
            RUN_SERIAL TRUE
    )

    qt_add_executable(UwfWindowsTaskbarIntegrationTests
            WindowsTaskbarIntegrationTests.cpp
            Win11TaskbarLayoutStrategyTestAccess.cpp
    )
    target_include_directories(UwfWindowsTaskbarIntegrationTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")
    target_link_libraries(UwfWindowsTaskbarIntegrationTests PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Svg Qt6::Test user32 shcore dwmapi ole32)
    # Win10/11 compatibility + PerMonitorV2 DPI（与主程序清单同类）。
    # 完整性级别刻意保持 asInvoker：本套件验证与中完整性 Explorer 的跨进程
    # parent，不等于 requireAdministrator 的生产 UAC 上下文。
    if (MSVC)
        target_sources(UwfWindowsTaskbarIntegrationTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/windows_integration.manifest")
    else ()
        target_sources(UwfWindowsTaskbarIntegrationTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/windows_integration_manifest.rc")
    endif ()
    uwf_configure_cpp_target(UwfWindowsTaskbarIntegrationTests)
    uwf_add_test_resources(UwfWindowsTaskbarIntegrationTests)

    add_test(NAME UwfWindowsTaskbarIntegrationTests COMMAND UwfWindowsTaskbarIntegrationTests)
    set_tests_properties(UwfWindowsTaskbarIntegrationTests PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=windows"
            LABELS "windows-integration"
            RUN_SERIAL TRUE
    )

    qt_add_executable(UwfWindowsTaskbarIncompatibilityTests
            WindowsTaskbarIncompatibilityTests.cpp
    )
    target_include_directories(UwfWindowsTaskbarIncompatibilityTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")
    target_link_libraries(UwfWindowsTaskbarIncompatibilityTests PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Test user32 shcore dwmapi)
    # 故意不声明 Windows 8+ supportedOS：这是 layered child window 不可用的
    # 真实进程能力边界，不通过生产故障注入伪造。
    if (MSVC)
        target_sources(UwfWindowsTaskbarIncompatibilityTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/windows_taskbar_incompatible.manifest")
    else ()
        target_sources(UwfWindowsTaskbarIncompatibilityTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/windows_taskbar_incompatible_manifest.rc")
    endif ()
    uwf_configure_cpp_target(UwfWindowsTaskbarIncompatibilityTests)
    uwf_add_test_resources(UwfWindowsTaskbarIncompatibilityTests)

    add_test(NAME UwfWindowsTaskbarIncompatibilityTests COMMAND UwfWindowsTaskbarIncompatibilityTests)
    set_tests_properties(UwfWindowsTaskbarIncompatibilityTests PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=windows"
            LABELS "windows-integration"
            RUN_SERIAL TRUE
    )
endif ()
