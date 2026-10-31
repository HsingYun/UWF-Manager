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
add_library(UwfUiDesign STATIC
    src/ui/design/BadgePainting.cpp
    src/ui/design/BadgePainting.h
    src/ui/design/ControlAnimation.cpp
    src/ui/design/ControlAnimation.h
    src/ui/design/ControlShape.cpp
    src/ui/design/ControlShape.h
    src/ui/design/ContextMenus.cpp
    src/ui/design/ContextMenus.h
    src/ui/design/DesignStyle.cpp
    src/ui/design/DesignStyle.h
    src/ui/design/DesignSystem.cpp
    src/ui/design/DesignSystem.h
    src/ui/design/FocusIndicator.cpp
    src/ui/design/FocusIndicator.h
    src/ui/design/IconPainting.cpp
    src/ui/design/IconPainting.h
    src/ui/design/ItemHighlight.cpp
    src/ui/design/ItemHighlight.h
    src/ui/design/ItemViewGeometry.h
    src/ui/design/PopupAppearance.cpp
    src/ui/design/PopupAppearance.h
    src/ui/design/PopupPlacement.cpp
    src/ui/design/PopupPlacement.h
    src/ui/design/SurfaceClip.cpp
    src/ui/design/SurfaceClip.h
    src/ui/design/SurfaceShadow.cpp
    src/ui/design/SurfaceShadow.h
    src/ui/design/TextPainting.cpp
    src/ui/design/TextPainting.h
    src/ui/widgets/BusyIndicator.cpp
    src/ui/widgets/BusyIndicator.h
    src/ui/widgets/CodeEditor.cpp
    src/ui/widgets/CodeEditor.h
    src/ui/widgets/CommandBar.cpp
    src/ui/widgets/CommandBar.h
    src/ui/widgets/CommandMenu.cpp
    src/ui/widgets/CommandMenu.h
    src/ui/widgets/ContentState.cpp
    src/ui/widgets/ContentState.h
    src/ui/widgets/Disclosure.cpp
    src/ui/widgets/Disclosure.h
    src/ui/widgets/FlexibleColumnLayout.h
    src/ui/widgets/FlexibleColumnLayout.cpp
    src/ui/widgets/IconView.cpp
    src/ui/widgets/IconView.h
    src/ui/widgets/Notice.cpp
    src/ui/widgets/Notice.h
    src/ui/widgets/NumberBox.cpp
    src/ui/widgets/NumberBox.h
    src/ui/widgets/SearchField.cpp
    src/ui/widgets/SearchField.h
    src/ui/widgets/SelectBox.cpp
    src/ui/widgets/SelectBox.h
    src/ui/widgets/SelectionListView.cpp
    src/ui/widgets/SelectionListView.h
    src/ui/widgets/RecyclingListView.cpp
    src/ui/widgets/RecyclingListView.h
    src/ui/widgets/StatusBadge.cpp
    src/ui/widgets/StatusBadge.h
    src/ui/widgets/Surface.cpp
    src/ui/widgets/Surface.h
    src/ui/widgets/HoverToolTip.h
    src/ui/widgets/SwitchButton.cpp
    src/ui/widgets/SwitchButton.h
    src/ui/widgets/TabView.cpp
    src/ui/widgets/TabView.h
    src/ui/widgets/TextLabel.cpp
    src/ui/widgets/TextLabel.h
    src/ui/widgets/SummaryLabel.cpp
    src/ui/widgets/SummaryLabel.h
    src/ui/widgets/VerticalScrollArea.cpp
    src/ui/widgets/VerticalScrollArea.h
    "${uwf_design_header}"
)
add_dependencies(UwfUiDesign UwfDesignTokens)
target_include_directories(UwfUiDesign PUBLIC "${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/generated")
target_link_libraries(UwfUiDesign PUBLIC Qt6::Widgets Qt6::Svg)
uwf_configure_production_target(UwfUiDesign)

option(UWF_BUILD_DESIGN_TOOLS "Build offline UI design preview tools" OFF)
