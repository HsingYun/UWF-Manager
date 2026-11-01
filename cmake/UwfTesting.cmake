# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

if(UWF_BUILD_TESTS AND MSVC)
    message(WARNING
        "UWF_BUILD_TESTS is enabled, but UWF test suites are not supported with MSVC "
        "or clang-cl. No tests will be registered; application configuration will continue. "
        "Use an MSYS2 clang64 or MinGW toolchain to build and run the tests."
    )
elseif(UWF_BUILD_TESTS)
    enable_testing()
    find_package(Qt6 COMPONENTS Test REQUIRED)
    add_subdirectory(tests)
endif()
