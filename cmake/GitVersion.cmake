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

    string(TOLOWER "${ARG_PREFIX}_version" _tgt)
    set(_header "${ARG_OUTPUT_DIR}/${_tgt}.h")

    # ── 把 a.b.c.d 拆成四个数字 ──────────────────────────────────────
    string(REPLACE "." ";" _ver_list "${ARG_VERSION}")
    list(LENGTH _ver_list _ver_len)
    if (NOT _ver_len EQUAL 4)
        message(FATAL_ERROR "cc_git_rev: VERSION 需形如 a.b.c.d，收到 '${ARG_VERSION}'")
    endif ()
    list(GET _ver_list 0 _v_major)
    list(GET _ver_list 1 _v_minor)
    list(GET _ver_list 2 _v_patch)
    list(GET _ver_list 3 _v_build)

    # ── 读 .git 解析 HEAD 短哈希 ─────────────────────────────────────
    # .git 通常是目录；git worktree / submodule 下是写着 "gitdir: <路径>" 的文件。
    set(_git_dir "")
    if (IS_DIRECTORY "${ARG_SOURCE_DIR}/.git")
        set(_git_dir "${ARG_SOURCE_DIR}/.git")
    elseif (EXISTS "${ARG_SOURCE_DIR}/.git")
        file(READ "${ARG_SOURCE_DIR}/.git" _dotgit)
        string(STRIP "${_dotgit}" _dotgit)
        if (_dotgit MATCHES "^gitdir: (.+)$")
            get_filename_component(_git_dir "${CMAKE_MATCH_1}" ABSOLUTE
                    BASE_DIR "${ARG_SOURCE_DIR}")
        endif ()
    endif ()

    set(_sha "")
    if (_git_dir AND EXISTS "${_git_dir}/HEAD")
        file(READ "${_git_dir}/HEAD" _head)
        string(STRIP "${_head}" _head)
        if (_head MATCHES "^ref: (.+)$")
            # HEAD 指向某个分支：先读松散 ref 文件，没有再查 packed-refs。
            set(_ref "${CMAKE_MATCH_1}")
            if (EXISTS "${_git_dir}/${_ref}")
                file(READ "${_git_dir}/${_ref}" _sha)
            elseif (EXISTS "${_git_dir}/packed-refs")
                # packed-refs 每行形如 "<sha> <refname>"；取匹配当前 ref 的那行。
                file(STRINGS "${_git_dir}/packed-refs" _line REGEX " ${_ref}$")
                if (_line)
                    list(GET _line 0 _line)
                    string(REGEX MATCH "^[0-9a-fA-F]+" _sha "${_line}")
                endif ()
            endif ()
        else ()
            # 分离 HEAD：文件内容本身就是完整 SHA。
            set(_sha "${_head}")
        endif ()
    endif ()

    # 取前 7 位作为短哈希；解析不出合法 SHA 时回退为纯数字版本。
    string(STRIP "${_sha}" _sha)
    string(LENGTH "${_sha}" _sha_len)
    set(_ver_string "${ARG_VERSION}")
    if (_sha MATCHES "^[0-9a-fA-F]+$" AND _sha_len GREATER_EQUAL 7)
        string(SUBSTRING "${_sha}" 0 7 _short)
        set(_ver_string "${ARG_VERSION}+g${_short}")
    endif ()

    # ── 动态拼出头文件内容并写盘 ─────────────────────────────────────
    # 仅在内容真正变化时重写，哈希不变时不触发无谓的重新编译。
    set(_content
            "#pragma once

// 本文件由 cmake/GitVersion.cmake 的 cc_git_rev() 在配置期生成 —— 请勿手工编辑。

#define ${ARG_PREFIX}_VER_MAJOR    ${_v_major}
#define ${ARG_PREFIX}_VER_MINOR    ${_v_minor}
#define ${ARG_PREFIX}_VER_PATCH    ${_v_patch}
#define ${ARG_PREFIX}_VER_BUILD    ${_v_build}
#define ${ARG_PREFIX}_VER_STRING   \"${_ver_string}\"
")
    set(_old "")
    if (EXISTS "${_header}")
        file(READ "${_header}" _old)
    endif ()
    if (NOT _old STREQUAL _content)
        file(WRITE "${_header}" "${_content}")
    endif ()

    if (NOT ARG_QUIET)
        message(STATUS "cc_git_rev: ${ARG_PREFIX}_VER_STRING = ${_ver_string}")
    endif ()

    # ── 自定义 target + 依赖装配 ─────────────────────────────────────
    if (NOT TARGET ${_tgt})
        add_custom_target(${_tgt} SOURCES "${_header}")
    endif ()
    add_dependencies(${target} ${_tgt})
    target_include_directories(${target} PRIVATE "${ARG_OUTPUT_DIR}")
endfunction()
