qt_add_executable(UwfHubLifecycleTests
        HubLifecycleTests.cpp
)

target_include_directories(UwfHubLifecycleTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(UwfHubLifecycleTests PRIVATE UwfOverlayRuntime Qt6::Widgets Qt6::Test user32 shcore dwmapi)
uwf_configure_cpp_target(UwfHubLifecycleTests)

add_test(NAME UwfHubLifecycleTests COMMAND UwfHubLifecycleTests)
if (WIN32)
    set_tests_properties(UwfHubLifecycleTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=windows")
else ()
    set_tests_properties(UwfHubLifecycleTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif ()

if (WIN32)
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

    add_test(NAME UwfWindowsTaskbarIncompatibilityTests COMMAND UwfWindowsTaskbarIncompatibilityTests)
    set_tests_properties(UwfWindowsTaskbarIncompatibilityTests PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=windows"
            LABELS "windows-integration"
            RUN_SERIAL TRUE
    )
endif ()
