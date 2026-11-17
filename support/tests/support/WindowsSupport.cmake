# SPDX-License-Identifier: GPL-3.0-or-later

add_library(UwfTestWidgetInput STATIC ui/WidgetInput.cpp ui/PointerPressHistory.h)
set_target_properties(UwfTestWidgetInput PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestWidgetInput PUBLIC "${PROJECT_SOURCE_DIR}/support/tests")
target_link_libraries(UwfTestWidgetInput PUBLIC Qt6::Widgets Qt6::Test)
target_link_libraries(UwfTestWidgetInput PRIVATE user32)
uwf_configure_cpp_target(UwfTestWidgetInput)

add_library(UwfTestUiSupport STATIC
        ui/DialogControls.h
        ui/TextLayoutChecks.cpp
        ui/WindowActivation.h
)
set_target_properties(UwfTestUiSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestUiSupport PUBLIC "${PROJECT_SOURCE_DIR}/support/tests")
target_link_libraries(UwfTestUiSupport PUBLIC UwfSimulationUi UwfTestWidgetInput Qt6::Test)
uwf_configure_cpp_target(UwfTestUiSupport)

add_library(UwfTestVisualSupport STATIC visual/WidgetCapture.cpp visual/AnimationClock.h visual/InkBounds.h visual/VisualGeometry.h)
set_target_properties(UwfTestVisualSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestVisualSupport PUBLIC "${PROJECT_SOURCE_DIR}/support/tests")
target_link_libraries(UwfTestVisualSupport PUBLIC UwfViewModels Qt6::Widgets Qt6::Test)
uwf_configure_cpp_target(UwfTestVisualSupport)
