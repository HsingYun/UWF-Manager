# SPDX-License-Identifier: GPL-3.0-or-later

include(UwfSimulationSupport)

add_library(UwfTestCompatibilitySupport INTERFACE)
target_sources(UwfTestCompatibilitySupport INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/uwf/FakeUwfTrace.h")
target_include_directories(UwfTestCompatibilitySupport INTERFACE "${PROJECT_SOURCE_DIR}/support/tests")
target_link_libraries(UwfTestCompatibilitySupport INTERFACE UwfSimulationBackend)
uwf_assert_backend_independent(UwfTestCompatibilitySupport)

add_library(UwfTestApplicationSupport STATIC app/FileStagingFixture.cpp)
set_target_properties(UwfTestApplicationSupport PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_include_directories(UwfTestApplicationSupport PUBLIC "${PROJECT_SOURCE_DIR}/support/tests")
target_link_libraries(UwfTestApplicationSupport PUBLIC UwfSimulationApplication UwfTestCompatibilitySupport)
uwf_configure_cpp_target(UwfTestApplicationSupport)

uwf_add_platform_test_support()
