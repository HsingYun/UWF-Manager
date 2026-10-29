# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

# Register executable cases; artifact-checking tools have separate CTest entries.
function(uwf_add_test target)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "RESOURCES;TRANSLATIONS;RUN_SERIAL"
        "LAYER;FEATURE;RUNTIME;TIMEOUT"
        "SOURCES;LIBRARIES;INCLUDE_DIRECTORIES;ENVIRONMENT;LABELS"
    )
    if(arg_UNPARSED_ARGUMENTS OR arg_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "uwf_add_test(${target}): invalid arguments")
    endif()

    if(NOT arg_LAYER OR NOT arg_FEATURE OR NOT arg_RUNTIME)
        message(FATAL_ERROR "uwf_add_test(${target}): LAYER, FEATURE and RUNTIME are required")
    endif()
    set(runtimes appless eventloop desktop)
    if(NOT arg_RUNTIME IN_LIST runtimes)
        message(FATAL_ERROR "uwf_add_test(${target}): unknown runtime ${arg_RUNTIME}")
    endif()
    string(TOLOWER "${arg_LAYER}" arg_LAYER)
    string(TOLOWER "${arg_FEATURE}" arg_FEATURE)
    qt_add_executable(${target} ${arg_SOURCES})
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/tests"
        ${arg_INCLUDE_DIRECTORIES}
    )
    target_link_libraries(${target} PRIVATE ${arg_LIBRARIES})
    uwf_configure_cpp_target(${target})

    if(arg_RESOURCES)
        uwf_add_resources(${target} "${target}_resources")
    endif()
    if(arg_TRANSLATIONS)
        uwf_add_translations(${target})
    endif()

    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/test-results")
    add_test(NAME ${target} COMMAND ${target}
        -o "${PROJECT_BINARY_DIR}/test-results/${target}.xml,xml" -o "-,txt"
    )
    if(arg_RUNTIME STREQUAL "desktop")
        list(APPEND arg_ENVIRONMENT "QT_QPA_PLATFORM=windows")
        set_tests_properties(${target} PROPERTIES RESOURCE_LOCK uwf-desktop)
    endif()
    if(arg_ENVIRONMENT)
        set_tests_properties(${target} PROPERTIES ENVIRONMENT "${arg_ENVIRONMENT}")
    endif()
    set_tests_properties(${target} PROPERTIES LABELS
        "kind-case;layer-${arg_LAYER};feature-${arg_FEATURE};runtime-${arg_RUNTIME};${arg_LABELS}"
    )
    if(NOT arg_TIMEOUT)
        if(arg_RUNTIME STREQUAL "desktop")
            set(arg_TIMEOUT 180)
        else()
            set(arg_TIMEOUT 60)
        endif()
    endif()
    set_tests_properties(${target} PROPERTIES TIMEOUT "${arg_TIMEOUT}")
    if(arg_RUN_SERIAL)
        set_tests_properties(${target} PROPERTIES RUN_SERIAL TRUE)
    endif()
endfunction()
