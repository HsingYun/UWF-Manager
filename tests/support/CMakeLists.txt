# SPDX-License-Identifier: GPL-3.0-or-later

add_library(UwfTestCompatibilitySupport STATIC uwf/FakeUwfBackend.cpp uwf/FakeUwfBackend.h uwf/FakeUwfTrace.h)
set_target_properties(UwfTestCompatibilitySupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestCompatibilitySupport PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestCompatibilitySupport PUBLIC UwfCompatibilityRuntime UwfObjectNameComparison UwfOperationExecution)
uwf_configure_cpp_target(UwfTestCompatibilitySupport)
uwf_assert_backend_independent(UwfTestCompatibilitySupport)

add_library(UwfTestApplicationSupport STATIC
    app/ConfigurationFixture.cpp
    app/EnhancedModeFixture.cpp
    app/FileStagingFixture.cpp
)
set_target_properties(UwfTestApplicationSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestApplicationSupport PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestApplicationSupport PUBLIC UwfApplication UwfTestCompatibilitySupport)
uwf_configure_cpp_target(UwfTestApplicationSupport)

add_library(UwfTestWidgetInput STATIC ui/WidgetInput.cpp ui/PointerPressHistory.h)
set_target_properties(UwfTestWidgetInput PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestWidgetInput PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestWidgetInput PUBLIC Qt6::Widgets Qt6::Test)
target_link_libraries(UwfTestWidgetInput PRIVATE user32)
uwf_configure_cpp_target(UwfTestWidgetInput)

add_library(UwfTestUiSupport STATIC
    ui/DialogControls.h
    ui/TextLayoutChecks.cpp
    ui/WindowActivation.h
)
set_target_properties(UwfTestUiSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestUiSupport PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestUiSupport PUBLIC UwfViews UwfTestWidgetInput Qt6::Test)
uwf_configure_cpp_target(UwfTestUiSupport)

add_library(UwfTestVisualSupport STATIC visual/WidgetCapture.cpp visual/AnimationClock.h visual/InkBounds.h visual/VisualGeometry.h)
set_target_properties(UwfTestVisualSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestVisualSupport PUBLIC "${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(UwfTestVisualSupport PUBLIC UwfViewModels Qt6::Widgets Qt6::Test)
uwf_configure_cpp_target(UwfTestVisualSupport)
