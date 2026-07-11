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
endif ()
