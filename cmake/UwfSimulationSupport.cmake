# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(GLOBAL)

add_library(UwfSimulationBackend STATIC
    "${PROJECT_SOURCE_DIR}/support/simulation/uwf/FakeUwfBackend.cpp"
    "${PROJECT_SOURCE_DIR}/support/simulation/uwf/FakeUwfBackend.h"
)
set_target_properties(UwfSimulationBackend PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfSimulationBackend PUBLIC "${PROJECT_SOURCE_DIR}/support")
target_link_libraries(UwfSimulationBackend PUBLIC UwfCompatibilityRuntime UwfObjectNameComparison UwfOperationExecution)
uwf_configure_cpp_target(UwfSimulationBackend)
uwf_assert_backend_independent(UwfSimulationBackend)

add_library(UwfSimulationApplication STATIC
    "${PROJECT_SOURCE_DIR}/support/simulation/app/ConfigurationState.cpp"
    "${PROJECT_SOURCE_DIR}/support/simulation/app/ConfigurationState.h"
    "${PROJECT_SOURCE_DIR}/support/simulation/app/EnhancedMode.cpp"
    "${PROJECT_SOURCE_DIR}/support/simulation/app/EnhancedMode.h"
    "${PROJECT_SOURCE_DIR}/support/simulation/app/MemoryFileStagingStore.h"
    "${PROJECT_SOURCE_DIR}/support/simulation/app/OverlayPolicy.h"
)
set_target_properties(UwfSimulationApplication PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfSimulationApplication PUBLIC "${PROJECT_SOURCE_DIR}/support")
target_link_libraries(UwfSimulationApplication PUBLIC UwfApplication)
uwf_configure_cpp_target(UwfSimulationApplication)

add_library(UwfSimulationUi INTERFACE)
target_sources(UwfSimulationUi INTERFACE "${PROJECT_SOURCE_DIR}/support/simulation/ui/FileDialogStub.h")
target_include_directories(UwfSimulationUi INTERFACE "${PROJECT_SOURCE_DIR}/support")
target_link_libraries(UwfSimulationUi INTERFACE UwfViews)
