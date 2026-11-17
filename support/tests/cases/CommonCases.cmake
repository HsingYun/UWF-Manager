# SPDX-License-Identifier: GPL-3.0-or-later
# Shared suites use common components and fixtures without native Windows ports.

function(uwf_add_application_case suite feature source)
    uwf_add_test(Uwf${suite}
            LAYER application FEATURE ${feature} RUNTIME appless
            SOURCES "${source}"
            LIBRARIES UwfApplication Qt6::Test ${ARGN}
    )
endfunction()

function(uwf_add_viewmodel_case suite feature source)
    uwf_add_test(Uwf${suite}
            LAYER presentation FEATURE ${feature} RUNTIME eventloop
            SOURCES ${source}
            LIBRARIES UwfViewModels UwfTestApplicationSupport Qt6::Test ${ARGN}
    )
endfunction()

uwf_add_test(UwfSharedValueTests
        LAYER foundation FEATURE values RUNTIME appless
        SOURCES foundation/SharedValueTests.cpp
        LIBRARIES UwfFoundation Qt6::Test
)

uwf_add_test(UwfDomainPolicyTests
        LAYER domain FEATURE configuration RUNTIME appless
        SOURCES domain/uwf/DomainPolicyTests.cpp
        LIBRARIES UwfDomain UwfObjectNameComparison Qt6::Test
)

uwf_add_test(UwfCollectionContractTests
        LAYER domain FEATURE collection-contract RUNTIME appless
        SOURCES domain/uwf/UwfCollectionContractTests.cpp
        LIBRARIES UwfDomain Qt6::Test
)

uwf_add_test(UwfFailureSolverTests
        LAYER domain FEATURE uwf-admission RUNTIME appless
        SOURCES domain/uwf/UwfFailureSolverTests.cpp
        ../support/uwf/UwfOperationCatalog.cpp
        ../support/uwf/UwfOperationCatalog.h
        LIBRARIES UwfContractSolvers Qt6::Test
)

uwf_add_test(UwfAllVolumeProtectionSolverTests
        LAYER domain FEATURE all-volume-protection RUNTIME appless
        SOURCES domain/uwf/AllVolumeProtectionSolverTests.cpp
        LIBRARIES UwfAllVolumeProtectionSolvers Qt6::Test
)

uwf_add_test(UwfCompositeAllVolumeProtectionTests
        LAYER application FEATURE all-volume-protection RUNTIME appless
        SOURCES application/uwf/CompositeAllVolumeProtectionTests.cpp
        LIBRARIES UwfAllVolumeProtection Qt6::Test
)

uwf_add_test(UwfPrecheckDifferentialTests
        LAYER infrastructure FEATURE uwf-compatibility RUNTIME appless
        SOURCES infrastructure/uwf/simulation/UwfPrecheckDifferentialTests.cpp
        LIBRARIES UwfRequestPrecheck UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_application_case(ConfigurationDraftTests configuration application/configuration/ConfigurationDraftTests.cpp)

uwf_add_test(UwfStateExecutionTests
        LAYER application FEATURE uwf-state RUNTIME appless
        SOURCES application/uwf/UwfStateExecutionTests.cpp
        LIBRARIES UwfRequestPrecheck UwfCompatibilityRuntime Qt6::Test
)

uwf_add_test(UwfStateConsistencyTests
        LAYER application FEATURE uwf-state RUNTIME appless
        SOURCES application/uwf/UwfStateConsistencyTests.cpp
        LIBRARIES UwfApplication UwfRequestPrecheck UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfStateMutationMatrixTests
        LAYER application FEATURE uwf-state RUNTIME appless
        SOURCES application/uwf/UwfStateMutationMatrixTests.cpp
        LIBRARIES UwfApplication UwfRequestPrecheck UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfApplicationStateSourceTests
        LAYER application FEATURE uwf-state RUNTIME appless
        SOURCES application/uwf/UwfApplicationStateSourceTests.cpp
        LIBRARIES UwfApplication UwfRequestPrecheck UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfReadStateTests
        LAYER application FEATURE uwf-state RUNTIME appless
        SOURCES application/uwf/UwfReadStateTests.cpp
        LIBRARIES UwfApplication UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfFallbackTests
        LAYER infrastructure FEATURE uwf-compatibility RUNTIME appless
        SOURCES infrastructure/uwf/routing/UwfFallbackTests.cpp
        LIBRARIES UwfApplication UwfRequestPrecheck UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfFakeIntegrationTests
        LAYER infrastructure FEATURE uwf-compatibility RUNTIME appless
        SOURCES infrastructure/uwf/simulation/UwfFakeIntegrationTests.cpp
        LIBRARIES UwfApplication UwfRequestPrecheck UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfFakeStageContractTests
        LAYER infrastructure FEATURE uwf-compatibility RUNTIME appless
        SOURCES infrastructure/uwf/simulation/UwfFakeStageContractTests.cpp
        LIBRARIES UwfApplication UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_application_case(ConfigurationImportTests configuration application/configuration/ConfigurationImportTests.cpp)

uwf_add_application_case(ConfigurationExecutionTests configuration application/configuration/ConfigurationExecutionTests.cpp UwfTestApplicationSupport UwfTestCompatibilitySupport UwfAllVolumeProtection)

uwf_add_test(UwfConfigurationLifecycleTests
        LAYER presentation FEATURE configuration-lifecycle RUNTIME eventloop
        SOURCES presentation/viewmodels/configuration/ConfigurationLifecycleTests.cpp
        LIBRARIES UwfViewModels UwfTestApplicationSupport Qt6::Test
)

uwf_add_application_case(StartupConfigurationTests startup application/startup/StartupConfigurationTests.cpp)

uwf_add_application_case(FileStagingPolicyTests staging application/staging/FileStagingPolicyTests.cpp)

uwf_add_application_case(AutomaticCommitPolicyTests staging application/staging/AutomaticCommitPolicyTests.cpp)

uwf_add_test(UwfFileStagingEditorTests
        LAYER application FEATURE staging RUNTIME eventloop
        SOURCES application/staging/FileStagingEditorTests.cpp
        LIBRARIES UwfSimulationApplication Qt6::Test
)

uwf_add_application_case(FileStagingDurabilityTests staging application/staging/FileStagingDurabilityTests.cpp UwfTestApplicationSupport UwfTestCompatibilitySupport UwfRequestPrecheck)

uwf_add_application_case(PathPolicyTests paths application/paths/PathPolicyTests.cpp)

uwf_add_application_case(OverlayCatalogTests overlay application/overlay/OverlayCatalogTests.cpp)

uwf_add_application_case(UwfmgrCommandCodecTests commands application/configuration/commands/UwfmgrCommandCodecTests.cpp)

uwf_add_application_case(ConsoleCommandExecutorTests console application/console/ConsoleCommandExecutorTests.cpp UwfTestCompatibilitySupport UwfAllVolumeProtection)

uwf_add_application_case(ConfigurationScriptTests configuration application/configuration/planning/ConfigurationScriptTests.cpp UwfTestCompatibilitySupport UwfAllVolumeProtection)

uwf_add_application_case(OverlayPlanningTests configuration application/configuration/OverlayPlanningTests.cpp)

uwf_add_application_case(OverlayStoragePlanningTests configuration application/configuration/OverlayStoragePlanningTests.cpp UwfTestCompatibilitySupport UwfRequestPrecheck)

uwf_add_test(UwfMainWindowViewModelTests
        LAYER presentation FEATURE shell RUNTIME eventloop
        SOURCES presentation/viewmodels/shell/MainWindowViewModelTests.cpp
        LIBRARIES UwfViewModels UwfTestApplicationSupport Qt6::Test
)

uwf_add_viewmodel_case(ConfigurationWorkflowViewModelTests configuration-integration presentation/viewmodels/integration/ConfigurationWorkflowViewModelTests.cpp)

uwf_add_viewmodel_case(OverlayFilesViewModelTests overlay-files presentation/viewmodels/overlay/OverlayFilesViewModelTests.cpp)

uwf_add_test(UwfConsoleRendererTests
        LAYER presentation FEATURE console RUNTIME appless
        SOURCES presentation/console/ConsoleRendererTests.cpp
        LIBRARIES UwfConsolePresentation Qt6::Test
)

uwf_add_test(UwfConsoleDiagnosticsTests
        LAYER presentation FEATURE console RUNTIME appless
        SOURCES presentation/console/ConsoleDiagnosticsTests.cpp
        LIBRARIES UwfConsolePresentation Qt6::Test
)

uwf_add_test(UwfOverlayFilesQueryManagerTests
        LAYER application FEATURE overlay-files RUNTIME eventloop
        SOURCES application/overlay/OverlayFilesQueryManagerTests.cpp
        LIBRARIES UwfApplication Qt6::Test
)

uwf_add_application_case(OverlayPolicyTests overlay-policy application/overlay/OverlayPolicyTests.cpp)

uwf_add_application_case(OverlayMaintenanceTests overlay-maintenance application/overlay/OverlayMaintenanceTests.cpp UwfTestCompatibilitySupport)

uwf_add_application_case(OverlayMaintenanceExecutionTests configuration application/configuration/OverlayMaintenanceExecutionTests.cpp)

uwf_add_application_case(OverlayPolicyEditorTests overlay-policy application/configuration/OverlayPolicyEditorTests.cpp UwfSimulationApplication)
target_include_directories(UwfOverlayPolicyEditorTests PRIVATE "${PROJECT_SOURCE_DIR}/support/tests")

uwf_add_application_case(OverlaySettingsEditSessionTests overlay-maintenance application/configuration/OverlaySettingsEditSessionTests.cpp UwfSimulationApplication)
target_include_directories(UwfOverlaySettingsEditSessionTests PRIVATE "${PROJECT_SOURCE_DIR}/support/tests")

uwf_add_application_case(MixedConfigurationTests configuration application/configuration/MixedConfigurationTests.cpp UwfTestApplicationSupport)

uwf_add_application_case(ConfigurationRepositoryTests configuration application/configuration/ConfigurationRepositoryTests.cpp)

uwf_add_application_case(CommitBatchExecutionTests commit application/commit/CommitBatchExecutionTests.cpp)

uwf_add_application_case(CommitPreparationTests commit application/commit/CommitPreparationTests.cpp)

uwf_add_application_case(ExclusionEditingTests exclusions application/configuration/ExclusionEditingTests.cpp)

uwf_add_application_case(ConfigurationEditSessionTests configuration application/configuration/ConfigurationEditSessionTests.cpp UwfTestApplicationSupport)

uwf_add_application_case(EditorPublicationTests configuration application/configuration/EditorPublicationTests.cpp)

uwf_add_application_case(RegistryBatchCommitterTests registry application/registry/RegistryBatchCommitterTests.cpp UwfTestApplicationSupport UwfTestCompatibilitySupport UwfRequestPrecheck)

uwf_add_application_case(MaintenanceRepositoryTests uwf-maintenance application/uwf/MaintenanceRepositoryTests.cpp UwfTestCompatibilitySupport UwfRequestPrecheck)

uwf_add_test(UwfEnhancedModeSessionTests
        LAYER application FEATURE enhanced-mode RUNTIME eventloop
        SOURCES application/enhanced/EnhancedModeSessionTests.cpp
        LIBRARIES UwfApplication UwfTestApplicationSupport Qt6::Test
)

uwf_add_test(UwfEnhancedModeManagerTests
        LAYER application FEATURE enhanced-mode RUNTIME eventloop
        SOURCES application/enhanced/EnhancedModeManagerTests.cpp
        LIBRARIES UwfApplication UwfTestApplicationSupport UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfEnhancedModeOperationsTests
        LAYER application FEATURE enhanced-mode RUNTIME eventloop
        SOURCES application/enhanced/EnhancedModeOperationsTests.cpp
        LIBRARIES UwfApplication UwfTestApplicationSupport UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfPagerTests
        LAYER presentation FEATURE pagination RUNTIME appless
        SOURCES presentation/models/PagerTests.cpp
        LIBRARIES UwfPresentationModels Qt6::Test
)

uwf_add_test(UwfTextFilePresentationTests
        LAYER presentation FEATURE text-files RUNTIME appless
        SOURCES presentation/viewmodels/common/TextFilePresentationTests.cpp
        LIBRARIES UwfViewModels Qt6::Test
)

uwf_add_test(UwfOverlayTextTests
        LAYER presentation FEATURE localization RUNTIME eventloop
        TRANSLATIONS
        SOURCES presentation/viewmodels/integration/OverlayTextTests.cpp
        LIBRARIES UwfViewModels UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfUserConfigExportViewModelTests
        LAYER presentation FEATURE startup RUNTIME appless
        SOURCES presentation/viewmodels/settings/UserConfigExportViewModelTests.cpp
        LIBRARIES UwfViewModels Qt6::Test
)

uwf_add_test(UwfPageProxyModelTests
        LAYER presentation FEATURE pagination RUNTIME eventloop
        SOURCES presentation/models/PageProxyModelTests.cpp
        LIBRARIES UwfPresentationModels Qt6::Test
)

uwf_add_viewmodel_case(ConfigurationEditingViewModelTests configuration presentation/viewmodels/configuration/ConfigurationEditingViewModelTests.cpp)

uwf_add_viewmodel_case(OverlayMaintenanceViewModelTests overlay-maintenance presentation/viewmodels/configuration/OverlayMaintenanceViewModelTests.cpp)

uwf_add_viewmodel_case(OverlaySettingsEditViewModelTests overlay-maintenance presentation/viewmodels/configuration/OverlaySettingsEditViewModelTests.cpp)

uwf_add_viewmodel_case(RuntimeModePresentationTests overlay-maintenance presentation/viewmodels/configuration/RuntimeModePresentationTests.cpp)

uwf_add_viewmodel_case(VolumeConfigurationViewModelTests configuration presentation/viewmodels/configuration/VolumeConfigurationViewModelTests.cpp)

uwf_add_viewmodel_case(ExclusionListViewModelTests exclusions presentation/viewmodels/configuration/ExclusionListViewModelTests.cpp)

uwf_add_viewmodel_case(ConfigurationReviewViewModelTests configuration presentation/viewmodels/configuration/ConfigurationReviewViewModelTests.cpp)

uwf_add_viewmodel_case(FileStagingViewModelTests staging presentation/viewmodels/staging/FileStagingViewModelTests.cpp)

uwf_add_viewmodel_case(EnhancedModeViewModelTests enhanced-mode presentation/viewmodels/enhanced/EnhancedModeViewModelTests.cpp)

uwf_add_viewmodel_case(OverlayViewModelTests overlay presentation/viewmodels/overlay/OverlayViewModelTests.cpp)

uwf_add_viewmodel_case(PowerDialogViewModelTests power presentation/viewmodels/power/PowerDialogViewModelTests.cpp)

uwf_add_viewmodel_case(RegistryPickerViewModelTests registry presentation/viewmodels/registry/RegistryPickerViewModelTests.cpp)
target_sources(UwfRegistryPickerViewModelTests PRIVATE
        presentation/viewmodels/registry/RegistryPickerTestActions.h
)

uwf_add_viewmodel_case(CommitPresentationTests commit presentation/viewmodels/commit/CommitPresentationTests.cpp)

uwf_add_viewmodel_case(ItemModelContractTests models presentation/viewmodels/integration/ItemModelContractTests.cpp)

uwf_add_test(UwfConfigurationAsyncTests
        LAYER presentation FEATURE configuration RUNTIME eventloop
        SOURCES presentation/viewmodels/configuration/ConfigurationAsyncTests.cpp
        LIBRARIES UwfViewModels UwfTestApplicationSupport Qt6::Test
)

uwf_add_test(UwfAsyncModelTests
        LAYER presentation FEATURE loading RUNTIME eventloop
        SOURCES presentation/viewmodels/common/AsyncModelTests.cpp
        LIBRARIES UwfViewModels Qt6::Test
)

uwf_add_test(UwfCommitOperationViewModelTests
        LAYER presentation FEATURE commit RUNTIME eventloop
        SOURCES presentation/viewmodels/commit/CommitOperationViewModelTests.cpp
        LIBRARIES UwfViewModels Qt6::Test
)

uwf_add_test(UwfFileStagingFailureLoggingTests
        LAYER presentation FEATURE staging RUNTIME eventloop
        SOURCES presentation/viewmodels/staging/FileStagingFailureLoggingTests.cpp
        LIBRARIES UwfViewModels UwfTestCompatibilitySupport Qt6::Test
)

uwf_add_test(UwfSystemInformationViewModelTests
        LAYER presentation FEATURE diagnostics RUNTIME eventloop
        SOURCES presentation/viewmodels/diagnostics/SystemInformationViewModelTests.cpp
        LIBRARIES UwfViewModels Qt6::Test
)

uwf_add_test(UwfUsageBarGeometryTests
        LAYER views FEATURE overlay RUNTIME appless
        SOURCES presentation/views/UsageBarGeometryTests.cpp
        LIBRARIES Qt6::Gui Qt6::Test
)

uwf_add_test(UwfPathEntryListRenderingTests
        LAYER views FEATURE path-entry-rendering RUNTIME offscreen
        RESOURCES
        SOURCES presentation/views/PathEntryListRenderingTests.cpp
        LIBRARIES UwfViews Qt6::Test
)

uwf_add_test(UwfExclusionRemovalHintTests
        LAYER views FEATURE exclusions RUNTIME offscreen
        RESOURCES TRANSLATIONS
        SOURCES presentation/views/ExclusionRemovalHintTests.cpp
        LIBRARIES UwfSimulationUi Qt6::Test
)

uwf_add_test(UwfCommandOutputDialogTests
        LAYER views FEATURE command-output RUNTIME offscreen
        RESOURCES TRANSLATIONS
        SOURCES presentation/views/commands/CommandOutputDialogTests.cpp
        LIBRARIES UwfViews Qt6::Test
)

uwf_add_test(UwfPagedDetailsLayoutTests
        LAYER views FEATURE pagination RUNTIME offscreen
        RESOURCES
        SOURCES presentation/views/PagedDetailsLayoutTests.cpp
        LIBRARIES UwfViews Qt6::Test
)

uwf_add_test(UwfApplicationComponentLinkTests
        LAYER composition FEATURE application RUNTIME appless
        SOURCES composition/ApplicationComponentLinkTests.cpp
        LIBRARIES UwfApplication Qt6::Test
)

uwf_add_test(UwfUiComponentLinkTests
        LAYER composition FEATURE ui RUNTIME offscreen
        RESOURCES
        SOURCES composition/UiComponentLinkTests.cpp
        LIBRARIES UwfSimulationUi Qt6::Test
)

uwf_add_test(UwfPointerPressHistoryTests
        LAYER tools FEATURE native-input RUNTIME appless
        SOURCES tools/PointerPressHistoryTests.cpp
        LIBRARIES Qt6::Test
)

uwf_add_test(UwfCatalogCheckTests
        LAYER tools FEATURE localization RUNTIME appless
        SOURCES tools/CatalogCheckTests.cpp
        LIBRARIES UwfCatalogCheckSupport Qt6::Test
)

uwf_add_test(UwfTestResultsTests
        LAYER tools FEATURE results RUNTIME appless
        SOURCES tools/TestResultsTests.cpp
        LIBRARIES UwfTestResultsSupport Qt6::Test
)

uwf_add_test(UwfVisualGeometryTests
        LAYER tools FEATURE visual RUNTIME appless
        SOURCES tools/VisualGeometryTests.cpp
        LIBRARIES Qt6::Gui Qt6::Test
)

if (TARGET UwfApplicationPreview)
    function(uwf_add_preview_smoke name mode page scenario modal title)
        cmake_parse_arguments(PARSE_ARGV 6 arg "" "" "ARGS;EXPECT")
        add_test(NAME "${name}"
                COMMAND "${CMAKE_COMMAND}"
                "-DPREVIEW=$<TARGET_FILE:UwfApplicationPreview>"
                "-DOUTPUT=${PROJECT_BINARY_DIR}/preview-smoke/${name}.png"
                "-DMODE=${mode}" "-DPAGE=${page}" "-DSCENARIO=${scenario}"
                "-DMODAL=${modal}" "-DTITLE=${title}"
                "-DEXTRA_ARGUMENTS=${arg_ARGS}" "-DEXPECT=${arg_EXPECT}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/presentation/preview/PreviewSmoke.cmake")
        set_tests_properties("${name}" PROPERTIES
                WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
                LABELS "kind-case;layer-presentation;feature-preview;runtime-offscreen"
                TIMEOUT 25)
    endfunction()
    uwf_add_preview_smoke(UwfPreviewDialogsSmoke interactive dialogs confirm true "Discard pending changes?")
    uwf_add_preview_smoke(UwfPreviewCommitSmoke interactive commit confirm true "Commit folder changes?")
    uwf_add_preview_smoke(UwfPreviewRestartSmoke interactive restart empty true "Safe restart")
    uwf_add_preview_smoke(UwfPreviewShutdownSmoke interactive shutdown empty true "Safe shutdown")
    uwf_add_preview_smoke(UwfPreviewCaptureSmoke capture restart empty true "Safe restart")
    uwf_add_preview_smoke(UwfPreviewSystemRawSmoke interactive system raw false "System information"
            EXPECT "\"systemInformationTabs\":1")
    uwf_add_preview_smoke(UwfPreviewReviewSceneSmoke interactive review normal false "Review and apply changes"
            ARGS --command-preview --expand all
            EXPECT "\"reviewTabs\":1" "\"changeCommandsDisclosure\":true")
endif ()
