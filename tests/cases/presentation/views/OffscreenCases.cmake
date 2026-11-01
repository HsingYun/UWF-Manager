# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

# Mixed suites retain their complete Windows acceptance entry. These explicit
# selections exercise Qt rendering and behavior without native input, taskbar
# attachment, compositor receipts or Windows clipboard ownership confirmation.

# Match the QtTest data tags: translated catalogs plus the English source text.
set(uwf_locale_theme_rows)
set(uwf_configuration_icon_rows)
foreach(locale IN ITEMS en ${UWF_I18N_LOCALES})
    foreach(theme IN ITEMS light dark)
        list(APPEND uwf_locale_theme_rows "${locale}-${theme}")
        foreach(width IN ITEMS 720 1096)
            list(APPEND uwf_configuration_icon_rows "${locale}-${theme}-${width}")
        endforeach()
    endforeach()
endforeach()

uwf_register_test(UwfWidgetConfigurationOffscreenTests
    TARGET UwfWidgetConfigurationTests LAYER views FEATURE widgets-configuration RUNTIME offscreen
    FUNCTIONS
        configurationAvailabilitySurvivesLanguageTransitions
        exclusionActionsFollowSelectionAndLanguage
        stagingActionsFollowStorageSelectionAndLanguage
        stagingDependencyReviewKeepsTheBuiltinBadgeAndCompleteDescription
        currentSessionShowsAllThreeStateIcons
        globalStatusEditsPublishOnlyConstrainedValues
        globalStatusTypeChangesAndImportsPreserveValidTargets
        globalStatusThresholdEventsCanBeDisabledIndependently
        globalStatusShowsObservedValuesWithoutEditing
        globalUsageChartUsesObservedCapacity
        globalStatusEditsCompleteUint32Thresholds
        exclusionListButtonsFilterAndPersistenceRowsUseDisplayedSelection
        exclusionFileActionsUseThePlatformBoundaryAndPreserveValidation
        importDialogFiltersCommentsAndAppendsBatchReports
        importFileActionLoadsOnlyCommandsFromEverySelectedFile
        recommendedConfigurationSelectionAppendsOnlyChosenGroups
)

uwf_register_offscreen_matrix(UwfWidgetConfigurationVisualTests.inline
    TARGET UwfWidgetConfigurationTests LAYER views FEATURE widgets-configuration
    LABELS visual-only
    FUNCTION configurationInlineIconsUseTheirRenderedTextCenter
    DATA_TAGS ${uwf_configuration_icon_rows}
)

uwf_register_offscreen_matrix(UwfWidgetConfigurationVisualTests.global
    TARGET UwfWidgetConfigurationTests LAYER views FEATURE widgets-configuration
    LABELS visual-only
    FUNCTION configurationGlobalInlineIconsUseTheirRenderedTextCenter
    DATA_TAGS ${uwf_locale_theme_rows}
)

uwf_register_test(UwfWidgetAppearanceOffscreenTests
    TARGET UwfWidgetAppearanceTests LAYER views FEATURE widgets-appearance RUNTIME offscreen
    FUNCTIONS
        systemBackdropStartsSolidUntilConfirmed
        systemBackdropRefreshDoesNotOutliveWindow
        appearanceRequestsCoalesceAndPreserveEditing
        appearanceChangesRespectOwnerLifetimes
        themeIconsReuseParsedAssets
        themeRoundTripRefreshesLiveSurfacesEditorsAndLinks
        productionStylesKeepFullDialogScrollbarsAndTheirCorners
)

uwf_register_test(UwfWidgetControlsOffscreenTests
    TARGET UwfWidgetControlsTests LAYER views FEATURE widgets-controls RUNTIME offscreen
    FUNCTIONS
        switchButtonRespondsToMouseKeyboardAndDisabledState
        arabicNumberFieldsKeepUnitsOutsideTheEditor
        collapsingDisclosureRestoresDialogHeight
        standardContextMenusPreserveCopyAndLocale
        longCommandMenusKeepFullScrollbarsAndKeyboardActions
        scrollableMenuCommandsRunAfterInputAndBeforeExecReturns
        scrollableMenusCancelStaleSelections
        codeEditorLinePitchSurvivesEmptyAndTypedParagraphs
        codeEditorGutterStaysOnTheTextBaselineAfterChineseInputAndScrolling
        groupedLayoutReflowsWithoutClippingOrRecreatingControls
        tableCopyReflectsTheActualSelectionAndMissingCells
)

uwf_register_test(UwfWidgetShellOffscreenTests
    TARGET UwfWidgetShellTests LAYER views FEATURE widgets-shell RUNTIME offscreen
    FUNCTIONS
        transientLabelPreservesNewBaselineUntilRestore
        hoverHintDefersEverySourceAndCancelsStaleCandidates
        summariesKeepCompletePartsAtMeasuredWidthBoundaries
        statusSurfaceKeepsOneDynamicBaselineAcrossLanguages
        statusSurfaceAlignsSelectionDetail
        titleBarAlignsApplicationNameWithIcon
        startupGeometryFitsTheWorkArea
)

uwf_register_test(UwfDialogLayoutOffscreenTests
    TARGET UwfDialogLayoutTests LAYER views FEATURE dialogs RUNTIME offscreen VISUAL_MATRIX
    FUNCTIONS
        headerMetricsRemainAuthoritativeAfterFontChanges
        dialogActionsDetermineTheMinimumWidth
        dialogGraphicsShareTheirVisibleTextCenter
        windowMenuIconsShareTheirVisibleTextCenter
        scrollContentKeepsItsLayoutSynchronized
        readOnlyDetailsKeepLineSpacingAndExposeLongContent
        messageFormattingIsExplicit
        panelEditorsKeepEnterLocalAndPreserveRejectionGuards
        panelContentReceivesInitialKeyboardFocus
        informationActionsKeepKeyboardActivation
        dialogButtonsHaveOneKeyboardStop
        aboutKeepsCompleteInformationWhenResized
        aboutExpandedDisclosuresKeepTheirGeometryWhilePainting
        dialogsCenterOnTheirOwnerAfterMeasuringContent
        contentDialogsMeasureVisibleTextWithoutReservingBlankSpace
        powerFailureDetailsExpandUntilTheScreenLimit
        readOnlyReportsUseTheScreenBeforeScrolling
)

uwf_register_test(UwfDesignControlOffscreenTests
    TARGET UwfDesignControlTests LAYER views FEATURE design-controls RUNTIME offscreen
    FUNCTIONS
        rebuiltTabsReleaseHoverWhenThePointerMovesOrLeaves
        disabledSwitchesPreserveKnownStateAndSeparateUnknown
        scrollBarRolesPreserveGeometryAndPaintingAcrossRenaming
        contextMenusFollowWidgetCapabilitiesAndQtTranslations
        qtClipboardSubmissionReportsItsSingleReceipt
        clipboardRequestsOutsideTheGuiThreadAreRejected
        contextMenuRejectsAnEmptyClipboardWriter
        contextMenusHaveOneOwnerWhenTheirTargetRetires
        itemHighlightsReleaseFinishedTransitionsAndCanRestart
        semanticTextColorsFollowTheThemeWithoutStylesheetRepolishing
        contentStatesWrapTheirCompleteTextAtTheAvailableWidth
        styleSheetsPreserveDesignedHeadersAndProgressMotion
        determinateProgressInterpolatesWithoutDelayingTheReportedValue
        keyboardTraversalVisitsEachControlOnce
        dangerButtonFeedback
        animationClockPreservesElapsedTimeAndRestoresNormalTiming
        selectPreservesValueUntilAccepted
        commandAndSelectionPopupsShareTransparentCorners
        numberUnitDoesNotChangeNumericEditing
        anchoredPopupsUseOnePlacementRule
        codeEditingPreservesPlainTextLineSpacingAndUndo
        dialogScrollbarsPreserveSurfaceAndCleanIntersections
        flowCanAlignTechnicalNotesToTheBottomWithoutChangingOtherGroups
        realCopyButtonGlyphsShareTheVisibleCenter
        graphicalEntryPointsShareTheirVisibleCenter
        iconOnlyEntryPointsKeepTheirVisibleCenter
        sourceSvgAssetsKeepTheirVisibleCenter
        differentSizedGlyphBoundsShareTheBottomEdge
        textRasterMeasurementPreservesShapedGlyphsWithoutDecorations
        defaultLabelTextSharesItsVisibleCenterAcrossScripts
        noticeIconFollowsTheFirstVisibleTextLine
        changingLabelTextKeepsTheSharedGlyphOnOneBaseline
        partiallySpecifiedListFontsUseTheApplicationTypography
        smallListTextKeepsItsRasterAcrossRows
        disclosuresPreserveKeyboardAndContentVisibility
        disclosureContentReplacementRetainsOneQtOwner
        disclosureContentCanRetireBeforeTheContainer
        disclosureContentAdoptionKeepsTheNewestReentrantBody
        vectorIconsKeepVisibleCenterAtFractionalScale
        roundedSurfacesClipChildPaintWithoutAFringe
)

uwf_register_test(UwfApplicationAppearanceOffscreenTests
    TARGET UwfApplicationAppearanceTests LAYER views FEATURE appearance RUNTIME offscreen
    FUNCTIONS
        languageAndThemeChangesReachProductionWidgets
        qtStandardWidgetsUseSupplementalTranslations
        commonDialogsPreserveSafeDefaults
        hintAndUsageWidgetsHandleEmptyOverflowAndThresholdEdges
        comboSelectionUsesStableItemData
        longMessageDialogsKeepCompleteTextAndReachableActions
        themeRoundTripPreservesMainWindowDraftAndWidgets
        textRenderingProducesValidGlyphsAndSmoothEdges
)

uwf_register_offscreen_matrix(UwfApplicationAppearanceVisualTests
    TARGET UwfApplicationAppearanceTests LAYER views FEATURE appearance
    LABELS visual-only
    FUNCTION dialogLayoutsKeepActionsVisible
    DATA_TAGS ${uwf_locale_theme_rows}
)

uwf_register_test(UwfApplicationCommitPowerOffscreenTests
    TARGET UwfApplicationCommitPowerTests LAYER views FEATURE commit-power RUNTIME offscreen
    FUNCTIONS
        diskCommitActionsRouteSelectedTargetsAndHonorCancellation
        commitDispatcherRejectsUnaddressablePathsAndReleasesObservationPause
        commitDispatcherConfirmsAndReportsARealFileThroughTheRepository
        commitDispatcherRoutesAnExistingRegistryValueThroughTheRepository
        safePowerActionsRequireConfirmationAndUseTheInjectedRepository
        commitBatchKeepsApiFailuresAlongsideExistenceObservations
        powerFlowVisualStates
)

uwf_register_test(UwfApplicationDiagnosticsOffscreenTests
    TARGET UwfApplicationDiagnosticsTests LAYER views FEATURE diagnostics RUNTIME offscreen
    FUNCTIONS
        systemInformationFieldsPreserveLiteralSelectableText
        mainWindowMoreMenuOpensSystemInformation
        reportDetailsPreserveCompleteText
        aboutDialogExposesVersionLicenseAndSafeClose
        diagnosticReportDistinguishesMissingStateAndBoundsLogs
        logBufferAndViewerPreserveMalformedAndStructuredLines
        logViewerFilteringSelectionAndClearConfirmation
)

uwf_register_test(UwfApplicationShellOffscreenTests
    TARGET UwfApplicationShellTests LAYER views FEATURE shell RUNTIME offscreen VISUAL_MATRIX
    FUNCTIONS
        volumeTabIconsKeepTheirAdmissionStateAcrossAppearanceChanges
        everyLanguageSwitchUpdatesTheExistingShell
        protectedUnletteredTabsKeepIdentityAndShowReadOnlySessionStates
        commitMenuKeepsGroupsAndSeparatesFullCommitDraftRemoval
        startupHiddenHubRemainsHiddenAcrossRefreshAndUiRebuild
        mainWindowQuietStartupInitializesWhileRemainingHidden
        mainWindowDistinguishesInitialFailureFromCommittedUnavailableState
        importedUint32ThresholdsUpdateTheSettingsWidth
        mainWindowCompactLayoutKeepsControlsReachable
        mainWindowCompactToolbarKeepsLanguageMenuUsable
)

uwf_register_test(UwfFilePickerDialogOffscreenTests
    TARGET UwfFilePickerDialogTests LAYER views FEATURE file-picker RUNTIME offscreen
    # Native save/replace focus checks remain in the complete desktop suite.
    FUNCTIONS
        pastingIntoPickerFieldsDoesNotEnableTheNavigationFocusRing
        fileTypePopupContainsOnlyRealSelectableFilters
        failureDetailsWrapAndCopyTheirCompleteText
        manyPlacesUseTheViewModelAndNavigateWithoutRowWidgets
        multiSelectionTogglesAndSurvivesSorting
        keyboardNavigationAndTypedNamesReachTheSameResult
        busyOperationsDisableEditingAndKeepCancelAvailable
        cancelDuringLoadingCannotAcceptOrReopen
        emptyPlaceholderDoesNotScrollOrMoveThePickerFields
        editedFilenameReplacesOneSelectionIntent
        productionProviderUsesTheCustomDialog
)

uwf_register_test(UwfHubLifecycleOffscreenTests
    TARGET UwfHubLifecycleTests LAYER views FEATURE overlay RUNTIME offscreen
    FUNCTIONS
        staticCompatibilityFiltering
        retainedAttachmentIsDistinctFromReleasedTemporaryFailure
        attachDetachLifecycle
        activatePreparedReportsStatusForEveryCoordinatorState
        ignoredAcquireDoesNotReplacePreparedRequest
        malformedAcquirePayloadConvergesToDetached
        destroyedPreparedWindowReleasesTransaction
        detachDuringAttachIsSerialized
        rollbackFailureEntersDetaching
        hostInvalidationIsSerialized
        hostInvalidationDominatesUserDetach
        hostInvalidationRestartsBlockedReleaseContract
        detachRetryAndNativeWindowReset
        attachNativeWindowResetRecovery
        rapidToggleConverges
        coordinatorDestructorCompletesPendingCleanup
        environmentClassification
        coordinatorTransitionTable
        fsmTransitionTable
        fsmTotalityAndPayloadTransitions
        coordinatorPayloadTransitions
        fsmPublishesReleaseBeforeFallbackAttach
        failedProbeKeepsConfirmedFallbackStable
        incompatibleProbeKeepsConfirmedFallbackStableWithoutRetry
        activationWaitsForFallbackRelease
        hubInitiatedSelectionAuthorizesActivationAfterRelease
        hubLifecycleDiagnostics
        retainedAttachDiagnosticMatchesStateSemantics
        coordinatorDiagnostics
        failureCounterResetsOnlyAfterConfirmation
        confirmationTimeoutRepairsRefreshablePresentation
        releaseAnnouncementPrecedesSyncCompletion
        disableDuringFailedCleanupConvergesToWithdrawn
        persistentPlacementMismatchDoesNotSpin
        fallbackRemainsStableAfterTaskbarRelease
        explicitReenableResumesWhenCleanupCompletes
        blockedReleaseHandsOwnershipToFallback
        hubUsesCommittedStateSnapshot
        requestedVisibilityRetainsUserIntentWithoutPresentation
        higherPriorityViewAutomaticallyReplacesFallback
        attachingViewRepairsChangedPlacement
        toolbarToggleDuringTransientShellPresentation
        interleavedToolbarAndHostTransitionsConverge
        refreshOwnershipControlsFallbackEligibility
        startMenuJitterThenToolbarToggleKeepsTaskbarExclusive
        healthDueRepairsConfirmedPresentationInPlace
        failingHostInvalidatedCompletionDoesNotDeadlock
        recoveringSoftFailureExhaustionYieldsFallback
        healthDueInvalidYieldsToFallback
        confirmationTimeoutExhaustionYieldsFallback
        externalRefreshDuringAttachingRepairsWithoutFallback
)

uwf_register_test(UwfSurfaceShadowOffscreenTests
    TARGET UwfSurfaceShadowTests LAYER views FEATURE shadows RUNTIME offscreen
    FUNCTIONS
        transparentInteriorAndSmoothColoredFalloff
        companionFollowsOwnerAndCannotTakeFocus
)
