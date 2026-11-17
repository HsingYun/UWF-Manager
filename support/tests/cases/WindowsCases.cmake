# SPDX-License-Identifier: GPL-3.0-or-later

uwf_add_test(UwfWindowsStatusTests
        LAYER infrastructure FEATURE error-contract RUNTIME appless
        SOURCES infrastructure/uwf/execution/UwfWindowsStatusTests.cpp
        LIBRARIES UwfCompatibilityRuntime Qt6::Test
)

uwf_add_test(UwfHibernationCapabilitiesTests
        LAYER infrastructure FEATURE hibernation RUNTIME appless
        SOURCES infrastructure/windows/power/HibernationCapabilitiesTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfBackendEquivalenceTests
        LAYER infrastructure FEATURE backend-equivalence RUNTIME eventloop
        SOURCES infrastructure/uwf/conformance/UwfBackendEquivalenceTests.cpp
        ../support/uwf/UwfPublicInterfaceCases.h
        LIBRARIES UwfWindowsInfrastructure UwfTestCompatibilitySupport Qt6::Test
)
uwf_assert_backend_independent(UwfBackendEquivalenceTests)
uwf_assert_backend_independent(UwfWindowsStatusTests)

uwf_add_test(UwfCommandProtocolTests
        LAYER infrastructure FEATURE protocols RUNTIME appless
        SOURCES infrastructure/windows/ipc/CommandProtocolTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfRegistryFileStagingCodecTests
        LAYER infrastructure FEATURE staging RUNTIME appless
        SOURCES infrastructure/windows/staging/RegistryFileStagingCodecTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfIniUserConfigTests
        LAYER infrastructure FEATURE settings RUNTIME appless
        SOURCES infrastructure/windows/settings/IniUserConfigTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_application_case(RegistryMetadataTests registry application/registry/RegistryMetadataTests.cpp UwfWindowsInfrastructure)

uwf_add_application_case(FileStagingDraftTests staging application/staging/FileStagingDraftTests.cpp UwfTestApplicationSupport UwfTestCompatibilitySupport UwfWindowsInfrastructure)

uwf_add_test(UwfConsoleSessionTests
        LAYER infrastructure FEATURE console RUNTIME appless
        SOURCES infrastructure/windows/console/ConsoleSessionTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
qt_add_executable(UwfConsoleApplicationTests composition/ConsoleApplicationTests.cpp)
target_include_directories(UwfConsoleApplicationTests PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/support/tests"
)
target_link_libraries(UwfConsoleApplicationTests PRIVATE UwfApplication Qt6::Test)
uwf_configure_cpp_target(UwfConsoleApplicationTests)
target_compile_definitions(UwfConsoleApplicationTests PRIVATE UWF_EXECUTABLE_PATH="$<TARGET_FILE:UWF>")
add_dependencies(UwfConsoleApplicationTests UWF)
uwf_register_test(UwfConsoleApplicationTests
        TARGET UwfConsoleApplicationTests LAYER composition FEATURE console RUNTIME eventloop
        FUNCTIONS embeddedManifestUsesInvokerPrivileges executableRemainsAGuiApplication embeddedVersionResourceKeepsProductIdentity
        everyHelpEntryIsCapturedWithoutAGui invalidCommandsAreCapturedWithTheirOriginalExitCode
        redirectedFilesCaptureTheOriginalOutput oneRedirectedOutputIsEnough
)
uwf_register_test(UwfConsoleApplicationGuiTests
        TARGET UwfConsoleApplicationTests LAYER composition FEATURE console RUNTIME desktop
        FUNCTIONS everyHelpEntryUsesAGraphicalDialog invalidCommandsUseAGraphicalDialogWithoutCapture
)

uwf_add_application_case(FileStagingCommitterTests staging application/staging/FileStagingCommitterTests.cpp UwfTestApplicationSupport UwfTestCompatibilitySupport UwfWindowsInfrastructure)

uwf_add_test(UwfAgentSupervisionTests
        LAYER infrastructure FEATURE enhanced-mode RUNTIME appless
        SOURCES infrastructure/windows/service/AgentSupervisionTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfEnhancedModeServiceRuntimeTests
        LAYER infrastructure FEATURE enhanced-mode RUNTIME appless
        SOURCES infrastructure/windows/service/EnhancedModeServiceRuntimeTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfPreshutdownCommitGateTests
        LAYER infrastructure FEATURE enhanced-mode RUNTIME appless
        SOURCES infrastructure/windows/service/PreshutdownCommitGateTests.cpp
        LIBRARIES Qt6::Test
)

uwf_add_test(UwfFileStagingPreparationTests
        LAYER application FEATURE staging RUNTIME eventloop
        SOURCES application/staging/FileStagingPreparationTests.cpp
        LIBRARIES UwfApplication UwfWindowsInfrastructure UwfRequestPrecheck UwfTestCompatibilitySupport Qt6::Test
)
uwf_add_test(UwfPowerCoordinatorTests
        LAYER application FEATURE power RUNTIME eventloop
        SOURCES application/power/PowerCoordinatorTests.cpp
        LIBRARIES UwfApplication UwfWindowsInfrastructure UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfWmiApiBehaviorTests
        LAYER infrastructure FEATURE wmi RUNTIME appless
        SOURCES infrastructure/windows/management/wmi/WmiApiBehaviorTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test advapi32 ole32 user32 wbemuuid
)
uwf_add_test(UwfMtaExecutionTests
        LAYER infrastructure FEATURE mta-execution RUNTIME appless TIMEOUT 120
        SOURCES infrastructure/windows/execution/MtaExecutionTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test ole32
)

# Exercise the application reader and injected WMI adapter without COM.
uwf_add_test(UwfOverlayFilesReaderTests
        LAYER integration FEATURE overlay RUNTIME eventloop
        SOURCES infrastructure/uwf/integration/OverlayFilesReaderTests.cpp
        LIBRARIES UwfApplication UwfWindowsInfrastructure UwfRequestPrecheck Qt6::Test
)

uwf_add_test(UwfFilePickerViewModelTests
        LAYER presentation FEATURE file-picker RUNTIME eventloop
        SOURCES presentation/viewmodels/files/FilePickerViewModelTests.cpp
        LIBRARIES UwfViewModels UwfWindowsInfrastructure UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfDesignControlTests
        LAYER views FEATURE design-controls RUNTIME desktop
        RESOURCES
        SOURCES presentation/views/DesignControlTests.cpp
        LIBRARIES UwfDesignSystem UwfTestWidgetInput Qt6::Test
)

uwf_add_test(UwfOverlayAdvancedTests
        LAYER views FEATURE overlay-policy RUNTIME offscreen
        RESOURCES TRANSLATIONS VISUAL_MATRIX
        SOURCES presentation/views/OverlayAdvancedTests.cpp
        LIBRARIES UwfViews UwfTestApplicationSupport UwfTestUiSupport UwfTestVisualSupport Qt6::Test
)
uwf_add_test(UwfOverlayMaintenanceDialogTests
        LAYER views FEATURE overlay-maintenance RUNTIME offscreen
        RESOURCES TRANSLATIONS VISUAL_MATRIX
        SOURCES presentation/views/OverlayMaintenanceDialogTests.cpp
        LIBRARIES UwfViews UwfTestApplicationSupport UwfTestUiSupport UwfTestVisualSupport Qt6::Test
)

uwf_add_test(UwfComboBoxTests
        LAYER views FEATURE interaction RUNTIME offscreen
        RESOURCES
        SOURCES presentation/views/ComboBoxTests.cpp
        LIBRARIES UwfViews UwfWindowsInfrastructure Qt6::Test
)

uwf_add_test(UwfPickerKeyboardTests
        LAYER views FEATURE interaction RUNTIME offscreen
        RESOURCES TRANSLATIONS
        SOURCES presentation/views/PickerKeyboardTests.cpp
        LIBRARIES UwfViews UwfWindowsInfrastructure Qt6::Test
)

uwf_add_test(UwfPowerAvailabilityTests
        LAYER views FEATURE power RUNTIME offscreen
        RESOURCES TRANSLATIONS
        SOURCES presentation/views/PowerAvailabilityTests.cpp
        LIBRARIES UwfSimulationUi UwfWindowsInfrastructure UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfHubLifecycleTests
        LAYER views FEATURE overlay RUNTIME desktop
        RESOURCES
        SOURCES presentation/views/HubLifecycleTests.cpp
        LIBRARIES UwfViews UwfWindowsInfrastructure Qt6::Widgets Qt6::Test user32 shcore dwmapi
)
add_library(UwfWidgetUiSupport STATIC presentation/views/widgets/WidgetUiFixture.cpp)
target_include_directories(UwfWidgetUiSupport PRIVATE "${PROJECT_SOURCE_DIR}/support/tests" "${PROJECT_BINARY_DIR}/generated")
target_link_libraries(UwfWidgetUiSupport PUBLIC UwfViews UwfWindowsInfrastructure UwfTestUiSupport UwfTestVisualSupport Qt6::Svg Qt6::Test advapi32 shell32 user32)
uwf_configure_cpp_target(UwfWidgetUiSupport)
foreach (feature Appearance Configuration Controls Pagination Power Shell)
    string(TOLOWER "${feature}" label)
    if (feature STREQUAL "Pagination" OR feature STREQUAL "Power")
        set(runtime offscreen)
    else ()
        set(runtime desktop)
    endif ()
    set(visual_matrix)
    if (feature STREQUAL "Configuration")
        set(visual_matrix VISUAL_MATRIX)
    endif ()
    uwf_add_test(UwfWidget${feature}Tests
            LAYER views FEATURE "widgets-${label}" RUNTIME ${runtime}
            RESOURCES TRANSLATIONS ${visual_matrix}
            SOURCES "presentation/views/widgets/${feature}Cases.cpp"
            LIBRARIES UwfWidgetUiSupport
    )
endforeach ()

target_link_libraries(UwfWidgetConfigurationTests PRIVATE UwfTestApplicationSupport)
uwf_add_test(UwfDialogBehaviorTests
        LAYER views FEATURE data-dialogs RUNTIME offscreen
        RESOURCES TRANSLATIONS
        SOURCES presentation/views/DialogBehaviorTests.cpp
        LIBRARIES UwfViews UwfWindowsInfrastructure UwfTestUiSupport UwfTestVisualSupport Qt6::Svg Qt6::Test advapi32 ole32 shell32 user32 wbemuuid
)

uwf_add_test(UwfCommitOperationDialogTests
        LAYER views FEATURE commit RUNTIME offscreen
        RESOURCES TRANSLATIONS VISUAL_MATRIX
        SOURCES presentation/views/CommitOperationDialogTests.cpp
        LIBRARIES UwfViews UwfWindowsInfrastructure UwfTestVisualSupport UwfTestUiSupport Qt6::Test
)
uwf_add_test(UwfDialogLayoutTests
        LAYER views FEATURE dialogs RUNTIME desktop
        RESOURCES TRANSLATIONS VISUAL_MATRIX
        SOURCES presentation/views/DialogLayoutTests.cpp
        LIBRARIES UwfViews UwfWindowsInfrastructure UwfTestUiSupport UwfTestVisualSupport Qt6::Svg Qt6::Test advapi32 ole32 shell32 user32 wbemuuid
)

# Feature suites share fixtures, not their execution state or entry point.
add_library(UwfApplicationUiSupport STATIC
        presentation/views/application/ApplicationUiFixture.cpp
        presentation/views/application/ApplicationUiTest.cpp
)
set_target_properties(UwfApplicationUiSupport PROPERTIES AUTOUIC OFF)
target_include_directories(UwfApplicationUiSupport PRIVATE
        "${PROJECT_SOURCE_DIR}/support/tests" "${PROJECT_BINARY_DIR}/generated"
)
target_link_libraries(UwfApplicationUiSupport PUBLIC
        UwfDesktopComposition UwfWindowsInfrastructure UwfTestApplicationSupport UwfTestUiSupport UwfTestVisualSupport Qt6::Svg Qt6::Test
        advapi32 dwmapi ole32 shell32 shcore user32 userenv wbemuuid wtsapi32
)
uwf_configure_cpp_target(UwfApplicationUiSupport)

function(uwf_add_application_ui_case feature label)
    if (feature MATCHES "^(Configuration|EnhancedMode|FileStaging)$")
        set(runtime offscreen)
    else ()
        set(runtime desktop)
    endif ()
    uwf_add_test(UwfApplication${feature}Tests
            LAYER views FEATURE ${label} RUNTIME ${runtime}
            RESOURCES TRANSLATIONS ${ARGN}
            SOURCES presentation/views/application/${feature}Cases.cpp
            LIBRARIES UwfApplicationUiSupport
    )
endfunction()
uwf_add_application_ui_case(Appearance appearance VISUAL_MATRIX)
uwf_add_application_ui_case(CommitPower commit-power)
target_link_libraries(UwfApplicationCommitPowerTests PRIVATE UwfTestCompatibilitySupport)
uwf_add_application_ui_case(Configuration configuration VISUAL_MATRIX)
target_link_libraries(UwfApplicationConfigurationTests PRIVATE UwfTestCompatibilitySupport)
uwf_add_application_ui_case(Diagnostics diagnostics)
uwf_add_application_ui_case(EnhancedMode enhanced-mode VISUAL_MATRIX)
uwf_add_application_ui_case(FileStaging staging VISUAL_MATRIX)
target_link_libraries(UwfApplicationFileStagingTests PRIVATE UwfTestCompatibilitySupport)
uwf_add_application_ui_case(Shell shell VISUAL_MATRIX)
uwf_add_test(UwfConfigurationLifecycleUiTests
        LAYER views FEATURE configuration-lifecycle RUNTIME offscreen
        RESOURCES TRANSLATIONS
        SOURCES presentation/views/ConfigurationLifecycleUiTests.cpp
        LIBRARIES UwfApplicationUiSupport UwfTestCompatibilitySupport
)
uwf_add_test(UwfFileStagingViewTests
        LAYER views FEATURE staging RUNTIME offscreen
        RESOURCES
        SOURCES presentation/views/FileStagingViewTests.cpp
        LIBRARIES UwfViews UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfFilePickerDialogTests
        LAYER views FEATURE file-picker RUNTIME desktop
        RESOURCES TRANSLATIONS
        SOURCES presentation/views/FilePickerDialogTests.cpp
        LIBRARIES UwfViews UwfWindowsInfrastructure UwfTestVisualSupport UwfTestWidgetInput Qt6::Test
)
uwf_add_test(UwfSurfaceShadowTests
        LAYER views FEATURE shadows RUNTIME desktop
        SOURCES presentation/views/SurfaceShadowTests.cpp
        LIBRARIES UwfDesignSystem Qt6::Test
)

uwf_add_test(UwfDesktopCompositionTests
        LAYER composition FEATURE desktop-ports RUNTIME offscreen
        SOURCES composition/DesktopCompositionTests.cpp
        LIBRARIES UwfDesktopComposition UwfSimulationUi UwfTestCompatibilitySupport Qt6::Test
)

# Link and exercise public component usage with exactly the required components.

uwf_add_test(UwfNativeUwfCompositionTests
        LAYER composition FEATURE provider RUNTIME appless
        SOURCES composition/NativeUwfCompositionTests.cpp
        LIBRARIES UwfSystemComposition UwfAllVolumeProtection UwfTestCompatibilitySupport Qt6::Test
)
target_compile_definitions(UwfNativeUwfCompositionTests PRIVATE
        $<$<BOOL:${UWF_ENABLE_FAILURE_SOLVER}>:UWF_ENABLE_FAILURE_SOLVER>
)

uwf_add_test(UwfComPtrTests
        LAYER infrastructure FEATURE resources RUNTIME appless
        SOURCES infrastructure/windows/resources/ComPtrTests.cpp
        LIBRARIES Qt6::Test
)
uwf_add_test(UwfWmiMethodCompletionTests
        LAYER infrastructure FEATURE wmi RUNTIME appless
        SOURCES infrastructure/windows/management/wmi/WmiMethodCompletionTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfCspUwfBackendTests
        LAYER infrastructure FEATURE csp RUNTIME appless
        SOURCES infrastructure/uwf/windows/csp/CspUwfBackendTests.cpp
        LIBRARIES UwfWindowsInfrastructure UwfTestCompatibilitySupport Qt6::Test
)
uwf_add_test(UwfWmiUwfBackendTests
        LAYER infrastructure FEATURE wmi RUNTIME appless
        SOURCES infrastructure/uwf/windows/wmi/WmiUwfBackendTests.cpp
        LIBRARIES UwfWindowsInfrastructure UwfSimulationBackend Qt6::Test
)
uwf_add_test(UwfNativeUwfBackendTests
        LAYER infrastructure FEATURE uwf-native RUNTIME appless
        SOURCES infrastructure/uwf/windows/native/NativeUwfBackendTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfWindowsVolumeRepositoryTests
        LAYER infrastructure FEATURE volumes RUNTIME appless
        SOURCES infrastructure/windows/storage/WindowsVolumeRepositoryTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
uwf_add_test(UwfWindowsTextFileCreationTests
        LAYER infrastructure FEATURE text-files RUNTIME appless
        SOURCES infrastructure/windows/filesystem/WindowsTextFileCreationTests.cpp
        LIBRARIES UwfWindowsInfrastructure Qt6::Test
)
set(uwf_windows_test_support "${PROJECT_SOURCE_DIR}/support/tests/support/windows")
uwf_add_test(UwfWindowsPlatformIntegrationTests
        LAYER infrastructure FEATURE platform RUNTIME offscreen
        RUN_SERIAL
        SOURCES infrastructure/windows/integration/WindowsPlatformIntegrationTests.cpp
        LIBRARIES UwfViews UwfSystemComposition Qt6::Network Qt6::Test advapi32 dxgi ole32 user32 wbemuuid
        LABELS native isolated-resources
)
uwf_add_test(UwfWindowsTaskbarIntegrationTests
        LAYER infrastructure FEATURE taskbar RUNTIME desktop
        RESOURCES RUN_SERIAL
        SOURCES infrastructure/windows/integration/WindowsTaskbarIntegrationTests.cpp
        "${uwf_windows_test_support}/Win11TaskbarLayoutStrategyTestAccess.cpp"
        "${uwf_windows_test_support}/windows_integration_manifest.rc"
        LIBRARIES UwfViews UwfWindowsInfrastructure Qt6::Test user32 shcore dwmapi ole32
        LABELS native isolated-resources
)
uwf_add_test(UwfWindowsTaskbarIncompatibilityTests
        LAYER infrastructure FEATURE taskbar RUNTIME desktop
        RESOURCES RUN_SERIAL
        SOURCES infrastructure/windows/integration/WindowsTaskbarIncompatibilityTests.cpp
        "${uwf_windows_test_support}/windows_taskbar_incompatible_manifest.rc"
        LIBRARIES UwfViews UwfWindowsInfrastructure Qt6::Test user32 shcore dwmapi
        LABELS native isolated-resources
)

uwf_add_test(UwfImageCompareTests
        LAYER tools FEATURE images RUNTIME appless
        SOURCES tools/ImageCompareTests.cpp
        LIBRARIES UwfImageCompareSupport Qt6::Test
)

uwf_add_test(UwfTextLayoutChecksTests
        LAYER tools FEATURE layout RUNTIME offscreen
        RESOURCES
        SOURCES tools/TextLayoutChecksTests.cpp
        LIBRARIES UwfTestUiSupport UwfWindowsInfrastructure Qt6::Test
)

include(presentation/views/OffscreenCases.cmake)
