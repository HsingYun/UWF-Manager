# SPDX-License-Identifier: GPL-3.0-or-later

add_library(UwfTestApplicationSupport STATIC
    app/ConfigurationFixture.cpp
    app/EnhancedModeFixture.cpp
    app/FileStagingFixture.cpp
)
set_target_properties(UwfTestApplicationSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestApplicationSupport PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestApplicationSupport PUBLIC UwfApplication)
uwf_configure_cpp_target(UwfTestApplicationSupport)

add_library(UwfTestWidgetInput STATIC ui/WidgetInput.cpp)
set_target_properties(UwfTestWidgetInput PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestWidgetInput PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestWidgetInput PUBLIC Qt6::Widgets Qt6::Test)
uwf_configure_cpp_target(UwfTestWidgetInput)

add_library(UwfTestUiSupport STATIC
    ui/DialogControls.h
    ui/TextLayoutChecks.cpp
)
set_target_properties(UwfTestUiSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestUiSupport PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestUiSupport PUBLIC UwfUi UwfTestWidgetInput Qt6::Test)
uwf_configure_cpp_target(UwfTestUiSupport)

add_library(UwfTestVisualSupport STATIC visual/WidgetCapture.cpp visual/AnimationClock.h visual/InkBounds.h)
set_target_properties(UwfTestVisualSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestVisualSupport PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestVisualSupport PUBLIC UwfPresentation Qt6::Widgets Qt6::Test)
uwf_configure_cpp_target(UwfTestVisualSupport)
