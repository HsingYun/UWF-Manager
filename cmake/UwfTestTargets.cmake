# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

option(UWF_ENABLE_DESKTOP_TESTS "Enable tests that use the interactive Windows desktop" OFF)
# Locale/theme/size image matrices need a suite budget separate from the
# single-function watchdog. Ordinary behavior tests retain their shorter limits.
set(uwf_visual_matrix_timeout 600)
# Validate the single XML outside the QtTest execution budget.
set(uwf_result_check_timeout 10)
set(UWF_OFFSCREEN_PROFILE "hidpi" CACHE STRING "Virtual display for offscreen tests: compact, fractional or hidpi")
set_property(CACHE UWF_OFFSCREEN_PROFILE PROPERTY STRINGS compact fractional hidpi)
if(UWF_OFFSCREEN_PROFILE STREQUAL "compact")
    set(uwf_offscreen_width 1280)
    set(uwf_offscreen_height 800)
    set(uwf_offscreen_scale 1)
elseif(UWF_OFFSCREEN_PROFILE STREQUAL "fractional")
    set(uwf_offscreen_width 1920)
    set(uwf_offscreen_height 1080)
    set(uwf_offscreen_scale 1.25)
elseif(UWF_OFFSCREEN_PROFILE STREQUAL "hidpi")
    set(uwf_offscreen_width 3840)
    set(uwf_offscreen_height 2160)
    set(uwf_offscreen_scale 2)
else()
    message(FATAL_ERROR "Unknown UWF_OFFSCREEN_PROFILE: ${UWF_OFFSCREEN_PROFILE}")
endif()
configure_file("${PROJECT_SOURCE_DIR}/tests/support/ui/offscreen.json.in"
    "${PROJECT_BINARY_DIR}/uwf-offscreen.json" @ONLY)

# Register either a whole executable or selected QtTest functions from it.
function(uwf_register_test name)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "RUN_SERIAL;VISUAL_MATRIX"
        "TARGET;LAYER;FEATURE;RUNTIME;TIMEOUT"
        "FUNCTIONS;ENVIRONMENT;LABELS"
    )
    if(arg_UNPARSED_ARGUMENTS OR arg_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "uwf_register_test(${name}): invalid arguments")
    endif()
    if(NOT arg_TARGET OR NOT arg_LAYER OR NOT arg_FEATURE OR NOT arg_RUNTIME)
        message(FATAL_ERROR "uwf_register_test(${name}): TARGET, LAYER, FEATURE and RUNTIME are required")
    endif()
    if(NOT TARGET ${arg_TARGET})
        message(FATAL_ERROR "uwf_register_test(${name}): target ${arg_TARGET} does not exist")
    endif()
    set(runtimes appless eventloop offscreen desktop)
    if(NOT arg_RUNTIME IN_LIST runtimes)
        message(FATAL_ERROR "uwf_register_test(${name}): unknown runtime ${arg_RUNTIME}")
    endif()
    if(arg_VISUAL_MATRIX)
        if(NOT arg_RUNTIME STREQUAL "desktop" AND NOT arg_RUNTIME STREQUAL "offscreen")
            message(FATAL_ERROR "uwf_register_test(${name}): VISUAL_MATRIX requires a GUI runtime")
        endif()
        list(APPEND arg_LABELS visual-matrix)
    endif()
    if(NOT arg_TIMEOUT)
        if(arg_VISUAL_MATRIX)
            set(arg_TIMEOUT ${uwf_visual_matrix_timeout})
        elseif(NOT arg_RUNTIME STREQUAL "appless")
            set(arg_TIMEOUT 180)
        else()
            set(arg_TIMEOUT 60)
        endif()
    endif()
    math(EXPR ctest_timeout "${arg_TIMEOUT} + ${uwf_result_check_timeout}")
    string(TOLOWER "${arg_LAYER}" arg_LAYER)
    string(TOLOWER "${arg_FEATURE}" arg_FEATURE)
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/test-results")
    add_dependencies(${arg_TARGET} UwfTestResultCheck)
    add_test(NAME ${name} COMMAND "${CMAKE_COMMAND}"
        "-DTEST_EXECUTABLE=$<TARGET_FILE:${arg_TARGET}>"
        "-DTEST_FUNCTIONS=${arg_FUNCTIONS}"
        "-DTEST_TIMEOUT=${arg_TIMEOUT}"
        "-DRESULT_PREFIX=${PROJECT_BINARY_DIR}/test-results/${name}"
        "-DRESULT_CHECKER=$<TARGET_FILE:UwfTestResultCheck>"
        "-DRESULT_CHECK_TIMEOUT=${uwf_result_check_timeout}"
        -P "${PROJECT_SOURCE_DIR}/tests/tools/results/RunQtTest.cmake"
    )
    # Worker-backed suites also cover native I/O outside QTRY's event polling.
    # Keep a bounded function watchdog independently of the whole-suite budget.
    if(arg_RUNTIME STREQUAL "eventloop")
        set(function_timeout_ms 120000)
    else()
        set(function_timeout_ms 60000)
    endif()
    list(APPEND arg_ENVIRONMENT "QTEST_FUNCTION_TIMEOUT=${function_timeout_ms}" "UWF_TEST_ALLOW_DESKTOP=0")
    if(arg_RUNTIME STREQUAL "desktop")
        list(REMOVE_ITEM arg_ENVIRONMENT "UWF_TEST_ALLOW_DESKTOP=0")
        list(APPEND arg_ENVIRONMENT "QT_QPA_PLATFORM=windows" "UWF_TEST_ALLOW_DESKTOP=1")
        set_tests_properties(${name} PROPERTIES RESOURCE_LOCK uwf-desktop)
        if(NOT UWF_ENABLE_DESKTOP_TESTS)
            set_tests_properties(${name} PROPERTIES DISABLED TRUE)
        endif()
    elseif(arg_RUNTIME STREQUAL "offscreen")
        # Qt separates platform arguments at colons, including Windows drive letters.
        set_tests_properties(${name} PROPERTIES WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
        list(APPEND arg_ENVIRONMENT
            "QT_QPA_PLATFORM=offscreen:configfile=uwf-offscreen.json"
            # Scale the whole profile: a screen-only DPR would revert to 1 on show.
            "QT_SCALE_FACTOR=${uwf_offscreen_scale}" "QT_SCREEN_SCALE_FACTORS="
            "QT_FONT_DPI=96" "QT_USE_PHYSICAL_DPI=0"
        )
    endif()
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "${arg_ENVIRONMENT}"
        LABELS "kind-case;layer-${arg_LAYER};feature-${arg_FEATURE};runtime-${arg_RUNTIME};${arg_LABELS}"
    )
    set_tests_properties(${name} PROPERTIES TIMEOUT "${ctest_timeout}")
    if(arg_RUN_SERIAL)
        set_tests_properties(${name} PROPERTIES RUN_SERIAL TRUE)
    endif()
endfunction()

# Give CTest one process per data row so long visual matrices can use its
# parallel scheduler without sharing QApplication, theme or language state.
function(uwf_register_offscreen_matrix name)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        ""
        "TARGET;LAYER;FEATURE;FUNCTION"
        "DATA_TAGS;LABELS"
    )
    if(arg_UNPARSED_ARGUMENTS OR arg_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "uwf_register_offscreen_matrix(${name}): invalid arguments")
    endif()
    foreach(required TARGET LAYER FEATURE FUNCTION DATA_TAGS)
        if(NOT arg_${required})
            message(FATAL_ERROR "uwf_register_offscreen_matrix(${name}): ${required} is required")
        endif()
    endforeach()
    set(registration_options)
    if(arg_LABELS)
        list(APPEND registration_options LABELS ${arg_LABELS})
    endif()
    foreach(tag IN LISTS arg_DATA_TAGS)
        uwf_register_test("${name}.${tag}"
            TARGET ${arg_TARGET} LAYER ${arg_LAYER} FEATURE ${arg_FEATURE}
            RUNTIME offscreen VISUAL_MATRIX ${registration_options}
            FUNCTIONS "${arg_FUNCTION}:${tag}"
        )
    endforeach()
endfunction()

# Register executable cases; artifact-checking tools have separate CTest entries.
function(uwf_add_test target)
    cmake_parse_arguments(PARSE_ARGV 1 arg
        "RESOURCES;TRANSLATIONS;RUN_SERIAL;VISUAL_MATRIX"
        "LAYER;FEATURE;RUNTIME;TIMEOUT"
        "SOURCES;LIBRARIES;INCLUDE_DIRECTORIES;ENVIRONMENT;LABELS"
    )
    if(arg_UNPARSED_ARGUMENTS OR arg_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "uwf_add_test(${target}): invalid arguments")
    endif()

    if(NOT arg_LAYER OR NOT arg_FEATURE OR NOT arg_RUNTIME)
        message(FATAL_ERROR "uwf_add_test(${target}): LAYER, FEATURE and RUNTIME are required")
    endif()
    qt_add_executable(${target} ${arg_SOURCES})
    target_include_directories(${target} PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/tests"
        ${arg_INCLUDE_DIRECTORIES}
    )
    target_link_libraries(${target} PRIVATE ${arg_LIBRARIES})
    uwf_configure_cpp_target(${target})

    if(arg_RESOURCES)
        uwf_add_resources(${target})
    endif()
    if(arg_TRANSLATIONS)
        uwf_add_translations(${target})
    endif()

    set(registration_options)
    if(arg_VISUAL_MATRIX)
        list(APPEND registration_options VISUAL_MATRIX)
    endif()
    foreach(keyword TIMEOUT ENVIRONMENT LABELS)
        if(arg_${keyword})
            list(APPEND registration_options ${keyword} ${arg_${keyword}})
        endif()
    endforeach()
    uwf_register_test(${target}
        TARGET ${target} LAYER ${arg_LAYER} FEATURE ${arg_FEATURE} RUNTIME ${arg_RUNTIME}
        ${registration_options}
    )
    if(arg_RUN_SERIAL)
        set_tests_properties(${target} PROPERTIES RUN_SERIAL TRUE)
    endif()
endfunction()
