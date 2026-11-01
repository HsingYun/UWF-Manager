# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

foreach(required TEST_EXECUTABLE TEST_TIMEOUT RESULT_PREFIX RESULT_CHECKER RESULT_CHECK_TIMEOUT)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "${required} is required")
    endif()
endforeach()

# Optional captures and audits belong to the CTest entry, just like its results.
# Keep shared fixture inputs unchanged and preserve unset/empty output variables.
get_filename_component(test_entry "${RESULT_PREFIX}" NAME)
foreach(output_environment
    UWF_TEST_SCREENSHOT_DIR
    UWF_ALIGNMENT_AUDIT_DIR
    UWF_INLINE_AUDIT_DIR
    UWF_UI_MOTION_ROOT
    UWF_UI_SCREENSHOT_ROOT
    UWF_SHADOW_CAPTURE_ROOT
)
    if(DEFINED ENV{${output_environment}} AND NOT "$ENV{${output_environment}}" STREQUAL "")
        set(output_directory "$ENV{${output_environment}}/${test_entry}")
        file(MAKE_DIRECTORY "${output_directory}")
        set(ENV{${output_environment}} "${output_directory}")
    endif()
endforeach()

# Each CTest entry owns its files, including when only a subset is selected.
# A successful process must never reuse an XML/report left by an earlier run.
set(result_xml "${RESULT_PREFIX}.xml")
set(result_report "${RESULT_PREFIX}.json")
file(REMOVE "${result_xml}" "${result_report}")

execute_process(
    COMMAND "${TEST_EXECUTABLE}" ${TEST_FUNCTIONS} -o "${result_xml},xml" -o "-,txt"
    TIMEOUT "${TEST_TIMEOUT}"
    RESULT_VARIABLE test_result
)
if(NOT test_result STREQUAL "0")
    message(FATAL_ERROR "QtTest failed (${test_result}): ${TEST_EXECUTABLE}")
endif()

execute_process(
    COMMAND "${RESULT_CHECKER}" --report "${result_report}" "${result_xml}"
    TIMEOUT "${RESULT_CHECK_TIMEOUT}"
    RESULT_VARIABLE check_result
)
if(NOT check_result STREQUAL "0")
    message(FATAL_ERROR "QtTest result validation failed (${check_result}): ${result_xml}")
endif()
