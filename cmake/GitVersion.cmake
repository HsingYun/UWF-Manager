set(_CC_GIT_VERSION_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(cc_git_rev target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG
            "QUIET"
            "PREFIX;VERSION;OUTPUT_DIR;SOURCE_DIR"
            "")

    if (NOT ARG_PREFIX)
        set(ARG_PREFIX "UWF")
    endif ()
    if (NOT ARG_VERSION)
        set(ARG_VERSION "1.0.0.0")
    endif ()
    if (NOT ARG_OUTPUT_DIR)
        set(ARG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated")
    endif ()
    if (NOT ARG_SOURCE_DIR)
        set(ARG_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
    endif ()

    string(TOLOWER "${ARG_PREFIX}_version" _version_target)
    set(_header "${ARG_OUTPUT_DIR}/${_version_target}.h")
    set(_generator "${_CC_GIT_VERSION_MODULE_DIR}/GenerateGitVersion.cmake")

    find_package(Git QUIET)
    set(_generator_args
            "-DPREFIX=${ARG_PREFIX}"
            "-DFALLBACK_VERSION=${ARG_VERSION}"
            "-DOUTPUT_FILE=${_header}"
            "-DSOURCE_DIR=${ARG_SOURCE_DIR}"
            "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}"
    )

    # 配置时先生成一次，保证全新构建目录中的 C++/RC 编译器能立即找到头文件。
    # 后面的自定义 target 每次构建都会再检查 Git；生成脚本仅在内容变化时写盘，
    # 因而普通增量构建不会因为时间戳变化而重新编译。
    set(_configure_args ${_generator_args})
    if (ARG_QUIET)
        list(APPEND _configure_args "-DQUIET=TRUE")
    endif ()
    execute_process(
            COMMAND "${CMAKE_COMMAND}" ${_configure_args} -P "${_generator}"
            RESULT_VARIABLE _generator_result
    )
    if (NOT _generator_result EQUAL 0)
        message(FATAL_ERROR "cc_git_rev: version generator failed with exit code ${_generator_result}")
    endif ()

    if (NOT TARGET ${_version_target})
        add_custom_target(${_version_target}
                COMMAND "${CMAKE_COMMAND}" ${_generator_args} -DQUIET=TRUE -P "${_generator}"
                BYPRODUCTS "${_header}"
                COMMENT "Updating Git version"
                VERBATIM
        )
    endif ()

    add_dependencies(${target} ${_version_target})
    target_sources(${target} PRIVATE "${_header}")
    target_include_directories(${target} PRIVATE "${ARG_OUTPUT_DIR}")
endfunction()
