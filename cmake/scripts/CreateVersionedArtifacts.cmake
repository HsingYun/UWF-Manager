# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

if(NOT SOURCE_FILE OR NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "CreateVersionedArtifacts: SOURCE_FILE does not exist: ${SOURCE_FILE}")
endif()
if(NOT VERSION_HEADER OR NOT EXISTS "${VERSION_HEADER}")
    message(FATAL_ERROR "CreateVersionedArtifacts: VERSION_HEADER does not exist: ${VERSION_HEADER}")
endif()
if(NOT PREFIX)
    set(PREFIX "UWF")
endif()

file(READ "${VERSION_HEADER}" _version_header)
string(REGEX MATCH "#define[ \t]+${PREFIX}_VER_STRING[ \t]+\"([^\"]+)\"" _version_match "${_version_header}")
if(NOT _version_match)
    message(FATAL_ERROR "CreateVersionedArtifacts: ${PREFIX}_VER_STRING is missing from ${VERSION_HEADER}")
endif()
set(_version "${CMAKE_MATCH_1}")

function(uwf_copy_versioned_artifact source_file output_dir)
    get_filename_component(_extension "${source_file}" EXT)
    set(_destination "${output_dir}/${PREFIX}.${_version}${_extension}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${source_file}" "${_destination}"
        RESULT_VARIABLE _copy_result
    )
    if(NOT _copy_result EQUAL 0)
        message(FATAL_ERROR "CreateVersionedArtifacts: failed to copy ${source_file}")
    endif()
    message(STATUS "Versioned artifact: ${_destination}")
endfunction()

get_filename_component(_output_dir "${SOURCE_FILE}" DIRECTORY)
uwf_copy_versioned_artifact("${SOURCE_FILE}" "${_output_dir}")

if(SYMBOL_FILE)
    if(NOT EXISTS "${SYMBOL_FILE}")
        message(FATAL_ERROR "CreateVersionedArtifacts: SYMBOL_FILE does not exist: ${SYMBOL_FILE}")
    endif()
    uwf_copy_versioned_artifact("${SYMBOL_FILE}" "${_output_dir}")
endif()

if(BUILD_CONFIG STREQUAL "Release" AND DEFINED UPX_EXECUTABLE AND NOT UPX_EXECUTABLE STREQUAL "")
    if(NOT EXISTS "${UPX_EXECUTABLE}" OR IS_DIRECTORY "${UPX_EXECUTABLE}")
        message(FATAL_ERROR "CreateVersionedArtifacts: UPX executable does not exist: ${UPX_EXECUTABLE}")
    endif()

    get_filename_component(_extension "${SOURCE_FILE}" EXT)
    set(_upx_destination "${_output_dir}/${PREFIX}.${_version}.upx${_extension}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy "${SOURCE_FILE}" "${_upx_destination}"
        RESULT_VARIABLE _upx_copy_result
    )
    if(NOT _upx_copy_result EQUAL 0)
        message(FATAL_ERROR "CreateVersionedArtifacts: failed to copy ${SOURCE_FILE} for UPX")
    endif()

    execute_process(
        COMMAND "${UPX_EXECUTABLE}" --best --lzma --force "${_upx_destination}"
        RESULT_VARIABLE _upx_result
    )
    if(NOT _upx_result EQUAL 0)
        file(REMOVE "${_upx_destination}")
        message(FATAL_ERROR "CreateVersionedArtifacts: UPX failed with exit code ${_upx_result}")
    endif()
    message(STATUS "UPX artifact: ${_upx_destination}")
endif()
