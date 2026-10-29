# SPDX-License-Identifier: GPL-3.0-or-later

# Value and domain contracts need neither widgets nor Windows adapters.
uwf_add_test(UwfSharedValueTests
    LAYER shared FEATURE values RUNTIME appless
    SOURCES shared/SharedValueTests.cpp
    LIBRARIES UwfShared Qt6::Test
)
uwf_add_test(UwfDomainPolicyTests
    LAYER core FEATURE configuration RUNTIME appless
    SOURCES core/DomainPolicyTests.cpp
    LIBRARIES UwfCore Qt6::Test
)

# Application cases link Windows adapters only when they use those ports.
function(uwf_add_application_case suite feature)
    uwf_add_test(Uwf${suite}
        LAYER app FEATURE ${feature} RUNTIME appless
        SOURCES "app/${suite}.cpp"
        LIBRARIES UwfApplication Qt6::Test ${ARGN}
    )
endfunction()
uwf_add_application_case(ConfigurationDraftTests configuration)
uwf_add_application_case(ConfigurationExecutionTests configuration UwfTestApplicationSupport)
uwf_add_test(UwfConfigurationLifecycleTests
    LAYER presentation FEATURE configuration-lifecycle RUNTIME eventloop
    SOURCES presentation/ConfigurationLifecycleTests.cpp
    LIBRARIES UwfPresentation UwfTestApplicationSupport UwfWindows Qt6::Test
)
uwf_add_application_case(StartupConfigurationTests startup)
uwf_add_application_case(CommandProtocolTests protocols)
uwf_add_application_case(RegistryMetadataTests registry UwfWindows)
uwf_add_application_case(FileStagingPolicyTests staging UwfWindows)
uwf_add_application_case(FileStagingDraftTests staging UwfTestApplicationSupport UwfWindows)
uwf_add_application_case(FileStagingDurabilityTests staging UwfTestApplicationSupport)
uwf_add_application_case(PathPolicyTests paths)
uwf_add_application_case(OverlayCatalogTests overlay)
uwf_add_application_case(CommandRenderingTests commands)
uwf_add_application_case(OverlayPlanningTests configuration)
uwf_add_application_case(CommitBatchExecutionTests commit)
uwf_add_application_case(CommitPreparationTests commit)
uwf_add_application_case(ExclusionEditingTests exclusions UwfWindows)
uwf_add_application_case(ConfigurationEditSessionTests configuration UwfWindows)
uwf_add_application_case(RegistryTreeCommitterTests registry UwfTestApplicationSupport UwfWindows)
uwf_add_application_case(FileStagingCommitterTests staging UwfTestApplicationSupport UwfWindows)

uwf_add_test(UwfFileStagingCoordinatorTests
    LAYER app FEATURE staging RUNTIME eventloop
    SOURCES app/FileStagingCoordinatorTests.cpp
    LIBRARIES UwfApplication Qt6::Test
)
uwf_add_test(UwfEnhancedModeSessionTests
    LAYER app FEATURE enhanced-mode RUNTIME eventloop
    SOURCES app/EnhancedModeSessionTests.cpp
    LIBRARIES UwfApplication UwfTestApplicationSupport Qt6::Test
)
uwf_add_test(UwfEnhancedModeManagerTests
    LAYER app FEATURE enhanced-mode RUNTIME eventloop
    SOURCES app/EnhancedModeManagerTests.cpp
    LIBRARIES UwfApplication UwfTestApplicationSupport UwfWindows Qt6::Test
)
uwf_add_test(UwfFileStagingPreparationTests
    LAYER app FEATURE staging RUNTIME eventloop
    SOURCES app/FileStagingPreparationTests.cpp
    LIBRARIES UwfApplication UwfWindows Qt6::Test
)
uwf_add_test(UwfPowerCoordinatorTests
    LAYER app FEATURE power RUNTIME eventloop
    SOURCES app/PowerCoordinatorTests.cpp
    LIBRARIES UwfApplication UwfWindows Qt6::Test
)

uwf_add_test(UwfWriteVerificationTests
    LAYER contracts FEATURE uwf RUNTIME appless
    SOURCES contracts/UwfWriteVerificationTests.cpp
    LIBRARIES UwfApplication UwfWindows Qt6::Test advapi32 user32
)
uwf_add_test(UwfWmiApiBehaviorTests
    LAYER contracts FEATURE wmi RUNTIME appless
    SOURCES contracts/WmiApiBehaviorTests.cpp
    LIBRARIES UwfApplication UwfWindows Qt6::Test advapi32 ole32 user32 wbemuuid
)
# Compile the production reader against a controlled runtime, without COM.
uwf_add_test(UwfOverlayFilesReaderTests
    LAYER contracts FEATURE overlay RUNTIME appless
    SOURCES contracts/OverlayFilesReaderTests.cpp
        "${PROJECT_SOURCE_DIR}/src/windows/uwf/OverlayFilesReader.cpp"
    LIBRARIES UwfApplication Qt6::Test
)

uwf_add_test(UwfPagerTests
    LAYER presentation FEATURE pagination RUNTIME appless
    SOURCES presentation/PagerTests.cpp
    LIBRARIES UwfPresentation Qt6::Test
)
uwf_add_test(UwfTextFilePresentationTests
    LAYER presentation FEATURE text-files RUNTIME appless
    SOURCES presentation/TextFilePresentationTests.cpp
    LIBRARIES UwfPresentation Qt6::Test
)
uwf_add_test(UwfUserConfigExportViewModelTests
    LAYER presentation FEATURE startup RUNTIME appless
    SOURCES presentation/UserConfigExportViewModelTests.cpp
    LIBRARIES UwfPresentation Qt6::Test
)
uwf_add_test(UwfViewModelBehaviorTests
    LAYER presentation FEATURE models RUNTIME eventloop
    SOURCES presentation/ViewModelBehaviorTests.cpp
    LIBRARIES UwfPresentation UwfWindows Qt6::Test advapi32 user32
)
uwf_add_test(UwfConfigurationAsyncTests
    LAYER presentation FEATURE configuration RUNTIME eventloop
    SOURCES presentation/ConfigurationAsyncTests.cpp
    LIBRARIES UwfPresentation UwfWindows UwfTestApplicationSupport Qt6::Test
)
uwf_add_test(UwfAsyncModelTests
    LAYER presentation FEATURE loading RUNTIME eventloop
    SOURCES presentation/AsyncModelTests.cpp
    LIBRARIES UwfPresentation Qt6::Test
)
uwf_add_test(UwfCommitOperationViewModelTests
    LAYER presentation FEATURE commit RUNTIME eventloop
    SOURCES presentation/CommitOperationViewModelTests.cpp
    LIBRARIES UwfPresentation Qt6::Test
)
uwf_add_test(UwfFilePickerViewModelTests
    LAYER presentation FEATURE file-picker RUNTIME eventloop
    SOURCES presentation/FilePickerViewModelTests.cpp
    LIBRARIES UwfPresentation UwfWindows Qt6::Test
)
uwf_add_test(UwfFileStagingFailureLoggingTests
    LAYER presentation FEATURE staging RUNTIME eventloop TIMEOUT 180
    SOURCES presentation/FileStagingFailureLoggingTests.cpp
    LIBRARIES UwfPresentation Qt6::Test
)
uwf_add_test(UwfSystemInformationViewModelTests
    LAYER presentation FEATURE diagnostics RUNTIME eventloop
    SOURCES presentation/SystemInformationViewModelTests.cpp
    LIBRARIES UwfPresentation Qt6::Test
)
uwf_add_test(UwfTaskPresentationTests
    LAYER presentation FEATURE task-lifetime RUNTIME eventloop
    SOURCES presentation/TaskPresentationTests.cpp
    LIBRARIES UwfPresentation UwfWindows Qt6::Test
)
uwf_add_test(UwfUsageBarGeometryTests
    LAYER ui FEATURE overlay RUNTIME appless
    SOURCES ui/UsageBarGeometryTests.cpp
    LIBRARIES Qt6::Gui Qt6::Test
)
uwf_add_test(UwfDesignControlTests
    LAYER ui FEATURE design-controls RUNTIME desktop
    RESOURCES
    SOURCES ui/DesignControlTests.cpp
    LIBRARIES UwfUiDesign UwfTestWidgetInput Qt6::Test
)

uwf_add_test(UwfComboBoxTests
    LAYER ui FEATURE interaction RUNTIME desktop
    RESOURCES
    SOURCES ui/ComboBoxTests.cpp
    LIBRARIES UwfUi UwfWindows Qt6::Test
)

uwf_add_test(UwfHubLifecycleTests
    LAYER ui FEATURE overlay RUNTIME desktop
    RESOURCES
    SOURCES ui/HubLifecycleTests.cpp
    LIBRARIES UwfUi UwfWindows Qt6::Widgets Qt6::Test user32 shcore dwmapi
)
add_library(UwfWidgetUiSupport STATIC ui/widgets/WidgetUiFixture.cpp)
target_include_directories(UwfWidgetUiSupport PRIVATE "${PROJECT_SOURCE_DIR}/tests" "${PROJECT_BINARY_DIR}/generated")
target_link_libraries(UwfWidgetUiSupport PUBLIC UwfUi UwfWindows UwfTestUiSupport UwfTestVisualSupport Qt6::Svg Qt6::Test advapi32 shell32 user32)
uwf_configure_cpp_target(UwfWidgetUiSupport)
foreach(feature Appearance Configuration Controls Pagination Power Shell)
    string(TOLOWER "${feature}" label)
    uwf_add_test(UwfWidget${feature}Tests
        LAYER ui FEATURE "widgets-${label}" RUNTIME desktop
        RESOURCES TRANSLATIONS
        SOURCES "ui/widgets/${feature}Cases.cpp"
        LIBRARIES UwfWidgetUiSupport
    )
endforeach()
uwf_add_test(UwfDialogBehaviorTests
    LAYER ui FEATURE data-dialogs RUNTIME desktop
    RESOURCES TRANSLATIONS
    SOURCES ui/DialogBehaviorTests.cpp
    LIBRARIES UwfUi UwfWindows UwfTestUiSupport UwfTestVisualSupport Qt6::Svg Qt6::Test advapi32 ole32 shell32 user32 wbemuuid
)
uwf_add_test(UwfCommitOperationDialogTests
    LAYER ui FEATURE commit RUNTIME desktop
    RESOURCES TRANSLATIONS TIMEOUT 240
    SOURCES ui/CommitOperationDialogTests.cpp
    LIBRARIES UwfUi UwfWindows UwfTestVisualSupport UwfTestUiSupport Qt6::Test
)
uwf_add_test(UwfDialogLayoutTests
    LAYER ui FEATURE dialogs RUNTIME desktop
    RESOURCES TRANSLATIONS
    SOURCES ui/DialogLayoutTests.cpp
    LIBRARIES UwfUi UwfWindows UwfTestUiSupport UwfTestVisualSupport Qt6::Svg Qt6::Test advapi32 ole32 shell32 user32 wbemuuid
)

# Feature suites share fixtures, not their execution state or entry point.
add_library(UwfApplicationUiSupport STATIC
    ui/application/ApplicationUiFixture.cpp
    ui/application/ApplicationUiTest.cpp
)
set_target_properties(UwfApplicationUiSupport PROPERTIES AUTOUIC OFF)
target_include_directories(UwfApplicationUiSupport PRIVATE
    "${PROJECT_SOURCE_DIR}/tests" "${PROJECT_BINARY_DIR}/generated"
)
target_link_libraries(UwfApplicationUiSupport PUBLIC
    UwfBootstrap UwfTestApplicationSupport UwfTestUiSupport UwfTestVisualSupport Qt6::Svg Qt6::Test
    advapi32 dwmapi ole32 shell32 shcore user32 userenv wbemuuid wtsapi32
)
uwf_configure_cpp_target(UwfApplicationUiSupport)

function(uwf_add_application_ui_case feature label)
    uwf_add_test(UwfApplication${feature}Tests
        LAYER ui FEATURE ${label} RUNTIME desktop TIMEOUT 900
        RESOURCES TRANSLATIONS
        SOURCES ui/application/${feature}Cases.cpp
        LIBRARIES UwfApplicationUiSupport
    )
endfunction()
uwf_add_application_ui_case(Appearance appearance)
uwf_add_application_ui_case(CommitPower commit-power)
uwf_add_application_ui_case(Configuration configuration)
uwf_add_application_ui_case(Diagnostics diagnostics)
uwf_add_application_ui_case(EnhancedMode enhanced-mode)
uwf_add_application_ui_case(FileStaging staging)
uwf_add_application_ui_case(Shell shell)
uwf_add_test(UwfConfigurationLifecycleUiTests
    LAYER ui FEATURE configuration-lifecycle RUNTIME desktop
    RESOURCES TRANSLATIONS
    SOURCES ui/ConfigurationLifecycleUiTests.cpp
    LIBRARIES UwfApplicationUiSupport
)
uwf_add_test(UwfFileStagingViewTests
    LAYER ui FEATURE staging RUNTIME desktop
    RESOURCES
    SOURCES ui/FileStagingViewTests.cpp
    LIBRARIES UwfUi UwfWindows Qt6::Test
)
uwf_add_test(UwfFilePickerDialogTests
    LAYER ui FEATURE file-picker RUNTIME desktop
    RESOURCES TRANSLATIONS
    SOURCES ui/FilePickerDialogTests.cpp
    LIBRARIES UwfUi UwfWindows UwfTestVisualSupport Qt6::Test
)
uwf_add_test(UwfSurfaceShadowTests
    LAYER ui FEATURE shadows RUNTIME desktop
    SOURCES ui/SurfaceShadowTests.cpp
    LIBRARIES UwfUiDesign Qt6::Test
)

# Link and exercise public component usage with exactly the required components.
uwf_add_test(UwfApplicationComponentLinkTests
    LAYER composition FEATURE provider RUNTIME appless
    SOURCES composition/ApplicationComponentLinkTests.cpp
    LIBRARIES UwfApplication Qt6::Test
)
uwf_add_test(UwfUiComponentLinkTests
    LAYER composition FEATURE ui RUNTIME desktop
    RESOURCES
    SOURCES composition/UiComponentLinkTests.cpp
    LIBRARIES UwfUi Qt6::Test
)

uwf_add_test(UwfComPtrTests
    LAYER windows FEATURE resources RUNTIME appless
    SOURCES windows/ComPtrTests.cpp
    LIBRARIES Qt6::Test
)
uwf_add_test(UwfWindowsTextFileCreationTests
    LAYER windows FEATURE text-files RUNTIME appless
    SOURCES windows/WindowsTextFileCreationTests.cpp
    LIBRARIES UwfWindows Qt6::Test
)
set(uwf_windows_test_support "${PROJECT_SOURCE_DIR}/tests/support/windows")
uwf_add_test(UwfWindowsPlatformIntegrationTests
    LAYER windows FEATURE platform RUNTIME desktop
    RUN_SERIAL
    SOURCES windows/WindowsPlatformIntegrationTests.cpp
    LIBRARIES UwfUi UwfWindows Qt6::Network Qt6::Test advapi32 dxgi ole32 user32 wbemuuid
    LABELS native isolated-resources
)
uwf_add_test(UwfWindowsTaskbarIntegrationTests
    LAYER windows FEATURE taskbar RUNTIME desktop
    RESOURCES RUN_SERIAL
    SOURCES windows/WindowsTaskbarIntegrationTests.cpp
        "${uwf_windows_test_support}/Win11TaskbarLayoutStrategyTestAccess.cpp"
        "${uwf_windows_test_support}/windows_integration_manifest.rc"
    LIBRARIES UwfUi UwfWindows Qt6::Test user32 shcore dwmapi ole32
    LABELS native isolated-resources
)
uwf_add_test(UwfWindowsTaskbarIncompatibilityTests
    LAYER windows FEATURE taskbar RUNTIME desktop
    RESOURCES RUN_SERIAL
    SOURCES windows/WindowsTaskbarIncompatibilityTests.cpp
        "${uwf_windows_test_support}/windows_taskbar_incompatible_manifest.rc"
    LIBRARIES UwfUi UwfWindows Qt6::Test user32 shcore dwmapi
    LABELS native isolated-resources
)

uwf_add_test(UwfCatalogCheckTests
    LAYER tools FEATURE localization RUNTIME appless
    SOURCES tools/CatalogCheckTests.cpp
    LIBRARIES UwfCatalogCheckSupport Qt6::Test
)
uwf_add_test(UwfImageCompareTests
    LAYER tools FEATURE images RUNTIME appless
    SOURCES tools/ImageCompareTests.cpp
    LIBRARIES UwfImageCompareSupport Qt6::Test
)
uwf_add_test(UwfTestResultsTests
    LAYER tools FEATURE results RUNTIME appless
    SOURCES tools/TestResultsTests.cpp
    LIBRARIES UwfTestResultsSupport Qt6::Test
)
uwf_add_test(UwfTextLayoutChecksTests
    LAYER tools FEATURE layout RUNTIME desktop
    RESOURCES
    SOURCES tools/TextLayoutChecksTests.cpp
    LIBRARIES UwfTestUiSupport UwfWindows Qt6::Test
)
