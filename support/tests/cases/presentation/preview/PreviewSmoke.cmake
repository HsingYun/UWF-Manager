# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.16)

set(ENV{QT_QPA_PLATFORM} "offscreen:configfile=uwf-offscreen.json")
set(ENV{QT_SCALE_FACTOR} "1")
set(ENV{QT_SCREEN_SCALE_FACTORS} "")
set(ENV{QT_FONT_DPI} "96")
set(ENV{QT_USE_PHYSICAL_DPI} "0")
file(REMOVE "${OUTPUT}")

set(arguments --english --page "${PAGE}" --scenario "${SCENARIO}" ${EXTRA_ARGUMENTS})
if (MODE STREQUAL "interactive")
    list(APPEND arguments --smoke-test --screenshot "${OUTPUT}")
else ()
    list(APPEND arguments --output "${OUTPUT}")
endif ()
execute_process(
        COMMAND "${PREVIEW}" ${arguments}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr
        TIMEOUT 20
)
if (NOT result STREQUAL "0")
    message(FATAL_ERROR "Preview ${PAGE}/${MODE} failed (${result}):\n${stdout}\n${stderr}")
endif ()
if (NOT EXISTS "${OUTPUT}")
    message(FATAL_ERROR "Preview ${PAGE}/${MODE} did not save its window")
endif ()
file(READ "${OUTPUT}" signature OFFSET 0 LIMIT 8 HEX)
if (NOT signature STREQUAL "89504e470d0a1a0a")
    message(FATAL_ERROR "Preview capture is not a PNG")
endif ()

if (MODE STREQUAL "interactive")
    # Observe the actual widget: a hidden main window must not stand in for the
    # requested modal page, and the saved image must belong to that same widget.
    string(REGEX MATCH "Preview smoke evidence: ([{][^\r\n]+[}])" evidence "${stdout}\n${stderr}")
    if (evidence STREQUAL "")
        message(FATAL_ERROR "Preview ${PAGE} exited without verifying a visible target window")
    endif ()
    foreach (field "\"visible\":true" "\"modal\":${MODAL}" "\"title\":\"${TITLE}\"" ${EXPECT})
        string(FIND "${evidence}" "${field}" found)
        if (found EQUAL -1)
            message(FATAL_ERROR "Unexpected preview window: ${evidence}; expected ${field}")
        endif ()
    endforeach ()
    foreach (dimension width height)
        string(REGEX MATCH "\"${dimension}\":([0-9]+)" matched "${evidence}")
        set(${dimension} "${CMAKE_MATCH_1}")
    endforeach ()
    file(READ "${OUTPUT}" png_dimensions OFFSET 16 LIMIT 8 HEX)
    string(SUBSTRING "${png_dimensions}" 0 8 png_width)
    string(SUBSTRING "${png_dimensions}" 8 8 png_height)
    math(EXPR png_width "0x${png_width}")
    math(EXPR png_height "0x${png_height}")
    if (NOT png_width EQUAL width OR NOT png_height EQUAL height)
        message(FATAL_ERROR "Saved image ${png_width}x${png_height} differs from visible window ${width}x${height}")
    endif ()
endif ()
