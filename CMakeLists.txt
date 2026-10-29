# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

cmake_minimum_required(VERSION 3.16)
project(UWF LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(UWF_SANITIZE "Build with AddressSanitizer + UBSan (Clang/GCC only)" ON)
option(UWF_BUILD_TESTS "Build UWF test suites" ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(UwfDependencies)
include(UwfComponents)
include(UwfExecutable)
include(UwfTesting)
