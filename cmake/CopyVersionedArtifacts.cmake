if (NOT SOURCE_FILE OR NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "CopyVersionedArtifacts: SOURCE_FILE does not exist: ${SOURCE_FILE}")
endif ()
if (NOT VERSION_HEADER OR NOT EXISTS "${VERSION_HEADER}")
    message(FATAL_ERROR "CopyVersionedArtifacts: VERSION_HEADER does not exist: ${VERSION_HEADER}")
endif ()
if (NOT PREFIX)
    set(PREFIX "UWF")
endif ()

file(READ "${VERSION_HEADER}" _version_header)
string(REGEX MATCH "#define[ \t]+${PREFIX}_VER_STRING[ \t]+\"([^\"]+)\"" _version_match "${_version_header}")
if (NOT _version_match)
    message(FATAL_ERROR "CopyVersionedArtifacts: ${PREFIX}_VER_STRING is missing from ${VERSION_HEADER}")
endif ()
set(_version "${CMAKE_MATCH_1}")

function(copy_versioned_artifact source_file output_dir)
    get_filename_component(_extension "${source_file}" EXT)
    set(_destination "${output_dir}/${PREFIX}.${_version}${_extension}")
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${source_file}" "${_destination}"
            RESULT_VARIABLE _copy_result
    )
    if (NOT _copy_result EQUAL 0)
        message(FATAL_ERROR "CopyVersionedArtifacts: failed to copy ${source_file}")
    endif ()
    message(STATUS "Versioned artifact: ${_destination}")
endfunction()

get_filename_component(_output_dir "${SOURCE_FILE}" DIRECTORY)
copy_versioned_artifact("${SOURCE_FILE}" "${_output_dir}")

if (SYMBOL_FILE)
    if (NOT EXISTS "${SYMBOL_FILE}")
        message(FATAL_ERROR "CopyVersionedArtifacts: SYMBOL_FILE does not exist: ${SYMBOL_FILE}")
    endif ()
    copy_versioned_artifact("${SYMBOL_FILE}" "${_output_dir}")
endif ()
