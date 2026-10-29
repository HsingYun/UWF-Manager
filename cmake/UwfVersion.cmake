# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

set(_UWF_VERSION_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(uwf_add_version target)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "QUIET"
        "PREFIX;VERSION;OUTPUT_DIR;SOURCE_DIR"
        ""
    )
    if(arg_UNPARSED_ARGUMENTS OR arg_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "uwf_add_version(${target}): invalid arguments")
    endif()

    if(NOT arg_PREFIX)
        set(arg_PREFIX "UWF")
    endif()
    if(NOT arg_VERSION)
        set(arg_VERSION "1.0.0.0")
    endif()
    if(NOT arg_OUTPUT_DIR)
        set(arg_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated")
    endif()
    if(NOT arg_SOURCE_DIR)
        set(arg_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
    endif()

    string(TOLOWER "${arg_PREFIX}_version" version_target)
    set(header "${arg_OUTPUT_DIR}/${version_target}.h")
    set(generator "${_UWF_VERSION_MODULE_DIR}/scripts/GenerateVersionHeader.cmake")
    find_package(Git QUIET)
    set(generator_args
        "-DPREFIX=${arg_PREFIX}"
        "-DFALLBACK_VERSION=${arg_VERSION}"
        "-DOUTPUT_FILE=${header}"
        "-DSOURCE_DIR=${arg_SOURCE_DIR}"
        "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}"
    )

    # Generate at configure time for clean builds; the build target refreshes
    # Git metadata without rewriting an unchanged header.
    set(configure_args ${generator_args})
    if(arg_QUIET)
        list(APPEND configure_args "-DQUIET=TRUE")
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" ${configure_args} -P "${generator}"
        RESULT_VARIABLE generator_result
    )
    if(NOT generator_result EQUAL 0)
        message(FATAL_ERROR "uwf_add_version: version generator failed with exit code ${generator_result}")
    endif()

    add_custom_target(${version_target}
        COMMAND "${CMAKE_COMMAND}" ${generator_args} -DQUIET=TRUE -P "${generator}"
        BYPRODUCTS "${header}"
        COMMENT "Updating Git version"
        VERBATIM
    )
    add_library(${target} INTERFACE)
    add_dependencies(${target} ${version_target})
    target_sources(${target} INTERFACE "${header}")
    target_include_directories(${target} INTERFACE "${arg_OUTPUT_DIR}")
    set_target_properties(${target} PROPERTIES
        INTERFACE_UWF_VERSION_HEADER "${header}"
        INTERFACE_UWF_VERSION_PREFIX "${arg_PREFIX}"
    )
endfunction()
