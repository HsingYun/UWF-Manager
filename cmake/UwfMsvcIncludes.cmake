# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

# Ninja parses /showIncludes as a byte protocol. CMake versions that decode a
# UTF-8 compiler response with the Windows ANSI code page can save a different
# prefix, silently losing every header dependency. Probe the actual bytes with
# the same flags and language as production, including installations without
# the English language pack. Preserve any caller-supplied compiler launcher.
set(uwf_msvc_probe_dir "${CMAKE_BINARY_DIR}/CMakeFiles/uwf-msvc-includes")
set(uwf_msvc_probe_header "${uwf_msvc_probe_dir}/uwf_showincludes_probe.h")
set(uwf_msvc_probe_source "${uwf_msvc_probe_dir}/probe.cpp")
file(MAKE_DIRECTORY "${uwf_msvc_probe_dir}")
file(WRITE "${uwf_msvc_probe_header}" "#pragma once\n")
file(WRITE "${uwf_msvc_probe_source}" "#include \"uwf_showincludes_probe.h\"\n")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "VSLANG=1033"
        "${CMAKE_CXX_COMPILER}" /nologo /utf-8 /showIncludes /EP "${uwf_msvc_probe_source}"
    RESULT_VARIABLE uwf_msvc_probe_result
    OUTPUT_VARIABLE uwf_msvc_probe_output
    ERROR_VARIABLE uwf_msvc_probe_error
    ENCODING NONE
)
if(NOT uwf_msvc_probe_result EQUAL 0)
    message(FATAL_ERROR "MSVC header dependency probe failed: ${uwf_msvc_probe_output}${uwf_msvc_probe_error}")
endif()
string(REGEX MATCH "[^\r\n]*uwf_showincludes_probe[.]h"
    uwf_msvc_include_line "${uwf_msvc_probe_output}\n${uwf_msvc_probe_error}")
file(TO_NATIVE_PATH "${uwf_msvc_probe_header}" uwf_msvc_native_header)
string(FIND "${uwf_msvc_include_line}" "${uwf_msvc_native_header}" uwf_msvc_path_offset)
if(uwf_msvc_path_offset LESS 0)
    string(FIND "${uwf_msvc_include_line}" "${uwf_msvc_probe_header}" uwf_msvc_path_offset)
endif()
if(uwf_msvc_path_offset LESS 1)
    message(FATAL_ERROR "MSVC did not report the expected header dependency: ${uwf_msvc_include_line}")
endif()
string(SUBSTRING "${uwf_msvc_include_line}" 0 ${uwf_msvc_path_offset} CMAKE_CL_SHOWINCLUDES_PREFIX)
set(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX "${CMAKE_CL_SHOWINCLUDES_PREFIX}")
list(PREPEND CMAKE_CXX_COMPILER_LAUNCHER "${CMAKE_COMMAND}" -E env "VSLANG=1033")
message(STATUS "MSVC header dependency protocol verified")
