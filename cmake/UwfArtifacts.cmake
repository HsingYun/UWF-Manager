# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

set(_UWF_ARTIFACT_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(uwf_add_versioned_artifacts target version_target)
    get_target_property(version_header ${version_target} INTERFACE_UWF_VERSION_HEADER)
    get_target_property(version_prefix ${version_target} INTERFACE_UWF_VERSION_PREFIX)
    if(NOT version_header OR NOT version_prefix)
        message(FATAL_ERROR "uwf_add_versioned_artifacts: ${version_target} does not provide version metadata")
    endif()

    set(copy_args
        "-DSOURCE_FILE=$<TARGET_FILE:${target}>"
        "-DVERSION_HEADER=${version_header}"
        "-DPREFIX=${version_prefix}"
        "-DBUILD_CONFIG=$<CONFIG>"
    )
    if(MSVC)
        list(APPEND copy_args "-DSYMBOL_FILE=$<TARGET_PDB_FILE:${target}>")
    endif()
    # Capture the local preset environment for builds started from other shells.
    if(NOT "$ENV{UPX_EXECUTABLE}" STREQUAL "")
        list(APPEND copy_args "-DUPX_EXECUTABLE=$ENV{UPX_EXECUTABLE}")
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" ${copy_args} -P "${_UWF_ARTIFACT_MODULE_DIR}/scripts/CreateVersionedArtifacts.cmake"
        COMMENT "Creating versioned artifacts"
        VERBATIM
    )
endfunction()
