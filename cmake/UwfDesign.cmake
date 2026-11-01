# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)
include_guard(GLOBAL)

add_executable(UwfDesignCompiler tools/design/DesignCompiler.cpp)
target_link_libraries(UwfDesignCompiler PRIVATE Qt6::Core)
uwf_configure_cpp_target(UwfDesignCompiler)

set(uwf_design_header "${PROJECT_BINARY_DIR}/generated/design/DesignTokens.h")
set(uwf_design_browser "${PROJECT_SOURCE_DIR}/build/knowledge/ui/design-tokens.js")
add_custom_command(
    OUTPUT "${uwf_design_header}" "${uwf_design_browser}"
    COMMAND UwfDesignCompiler "${PROJECT_SOURCE_DIR}/resources/styles/design.json"
        "${uwf_design_header}" "${uwf_design_browser}"
    DEPENDS UwfDesignCompiler "${PROJECT_SOURCE_DIR}/resources/styles/design.json"
        "${PROJECT_SOURCE_DIR}/resources/icons/scroll_up.svg"
        "${PROJECT_SOURCE_DIR}/resources/icons/scroll_down.svg"
        "${PROJECT_SOURCE_DIR}/resources/icons/scroll_left.svg"
        "${PROJECT_SOURCE_DIR}/resources/icons/scroll_right.svg"
    VERBATIM
)
add_custom_target(UwfDesignTokens DEPENDS "${uwf_design_header}" "${uwf_design_browser}")
add_library(UwfDesignSystem STATIC
    src/presentation/controls/clipboard/ClipboardText.cpp
    src/presentation/controls/clipboard/ClipboardText.h
    src/presentation/controls/feedback/BusyIndicator.cpp
    src/presentation/controls/feedback/BusyIndicator.h
    src/presentation/controls/feedback/ContentHintBox.cpp
    src/presentation/controls/feedback/ContentHintBox.h
    src/presentation/controls/feedback/ContentState.cpp
    src/presentation/controls/feedback/ContentState.h
    src/presentation/controls/feedback/HoverToolTip.h
    src/presentation/controls/feedback/Notice.cpp
    src/presentation/controls/feedback/Notice.h
    src/presentation/controls/feedback/StatusBadge.cpp
    src/presentation/controls/feedback/StatusBadge.h
    src/presentation/controls/feedback/TransientLabel.cpp
    src/presentation/controls/feedback/TransientLabel.h
    src/presentation/controls/inputs/CodeEditor.cpp
    src/presentation/controls/inputs/CodeEditor.h
    src/presentation/controls/inputs/ComboBoxSelection.cpp
    src/presentation/controls/inputs/ComboBoxSelection.h
    src/presentation/controls/inputs/NumberBox.cpp
    src/presentation/controls/inputs/NumberBox.h
    src/presentation/controls/inputs/SearchField.cpp
    src/presentation/controls/inputs/SearchField.h
    src/presentation/controls/inputs/SelectBox.cpp
    src/presentation/controls/inputs/SelectBox.h
    src/presentation/controls/inputs/SwitchButton.cpp
    src/presentation/controls/inputs/SwitchButton.h
    src/presentation/controls/layout/FlexibleColumnLayout.cpp
    src/presentation/controls/layout/FlexibleColumnLayout.h
    src/presentation/controls/layout/AlignedGraphic.cpp
    src/presentation/controls/layout/AlignedGraphic.h
    src/presentation/controls/layout/FlowLayout.cpp
    src/presentation/controls/layout/FlowLayout.h
    src/presentation/controls/layout/RoundedCornerOverlay.cpp
    src/presentation/controls/layout/RoundedCornerOverlay.h
    src/presentation/controls/layout/Surface.cpp
    src/presentation/controls/layout/Surface.h
    src/presentation/controls/layout/VerticalScrollArea.cpp
    src/presentation/controls/layout/VerticalScrollArea.h
    src/presentation/controls/navigation/CommandBar.cpp
    src/presentation/controls/navigation/CommandBar.h
    src/presentation/controls/navigation/CommandMenu.cpp
    src/presentation/controls/navigation/CommandMenu.h
    src/presentation/controls/navigation/ContextMenus.cpp
    src/presentation/controls/navigation/ContextMenus.h
    src/presentation/controls/navigation/Disclosure.cpp
    src/presentation/controls/navigation/Disclosure.h
    src/presentation/controls/navigation/TabView.cpp
    src/presentation/controls/navigation/TabView.h
    src/presentation/controls/pagination/PagedWindowLayout.cpp
    src/presentation/controls/pagination/PagedWindowLayout.h
    src/presentation/controls/pagination/PaginationBar.cpp
    src/presentation/controls/pagination/PaginationBar.h
    src/presentation/controls/tables/ItemDelegate.cpp
    src/presentation/controls/tables/ItemDelegate.h
    src/presentation/controls/tables/PagedTableView.cpp
    src/presentation/controls/tables/PagedTableView.h
    src/presentation/controls/tables/PagedViewController.cpp
    src/presentation/controls/tables/PagedViewController.h
    src/presentation/controls/tables/PathEntryControls.cpp
    src/presentation/controls/tables/PathEntryControls.h
    src/presentation/controls/tables/PathEntryListView.cpp
    src/presentation/controls/tables/PathEntryListView.h
    src/presentation/controls/tables/RecordInspector.cpp
    src/presentation/controls/tables/RecordInspector.h
    src/presentation/controls/tables/RecyclingListView.cpp
    src/presentation/controls/tables/RecyclingListView.h
    src/presentation/controls/tables/ResponsiveTableView.cpp
    src/presentation/controls/tables/ResponsiveTableView.h
    src/presentation/controls/tables/SelectionListView.cpp
    src/presentation/controls/tables/SelectionListView.h
    src/presentation/controls/tables/TablePresentation.cpp
    src/presentation/controls/tables/TablePresentation.h
    src/presentation/controls/tables/TableText.cpp
    src/presentation/controls/tables/TableText.h
    src/presentation/controls/text/IconView.cpp
    src/presentation/controls/text/IconView.h
    src/presentation/controls/text/SummaryLabel.cpp
    src/presentation/controls/text/SummaryLabel.h
    src/presentation/controls/text/TextLabel.cpp
    src/presentation/controls/text/TextLabel.h
    src/presentation/design/controls/ControlAnimation.cpp
    src/presentation/design/controls/ControlAnimation.h
    src/presentation/design/controls/ControlShape.cpp
    src/presentation/design/controls/ControlShape.h
    src/presentation/design/controls/FocusIndicator.cpp
    src/presentation/design/controls/FocusIndicator.h
    src/presentation/design/controls/ItemViewGeometry.h
    src/presentation/design/painting/BadgePainting.cpp
    src/presentation/design/painting/BadgePainting.h
    src/presentation/design/painting/IconPainting.cpp
    src/presentation/design/painting/IconPainting.h
    src/presentation/design/painting/RasterInkBounds.cpp
    src/presentation/design/painting/RasterInkBounds.h
    src/presentation/design/painting/ItemHighlight.cpp
    src/presentation/design/painting/ItemHighlight.h
    src/presentation/design/painting/TextPainting.cpp
    src/presentation/design/painting/TextPainting.h
    src/presentation/design/surfaces/PopupAppearance.cpp
    src/presentation/design/surfaces/PopupAppearance.h
    src/presentation/design/surfaces/PopupPlacement.cpp
    src/presentation/design/surfaces/PopupPlacement.h
    src/presentation/design/surfaces/SurfaceClip.cpp
    src/presentation/design/surfaces/SurfaceClip.h
    src/presentation/design/surfaces/SurfaceShadow.cpp
    src/presentation/design/surfaces/SurfaceShadow.h
    src/presentation/design/theme/DesignStyle.cpp
    src/presentation/design/theme/DesignStyle.h
    src/presentation/design/theme/DesignSystem.cpp
    src/presentation/design/theme/DesignSystem.h
    "${uwf_design_header}"
)
add_dependencies(UwfDesignSystem UwfDesignTokens)
target_include_directories(UwfDesignSystem PUBLIC "${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/generated")
target_link_libraries(UwfDesignSystem PUBLIC UwfPresentationModels Qt6::Widgets Qt6::Svg PRIVATE UwfFoundation)
uwf_configure_production_target(UwfDesignSystem)

option(UWF_BUILD_DESIGN_TOOLS "Build offline UI design preview tools" OFF)
