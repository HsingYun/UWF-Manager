include_guard(GLOBAL)

function(uwf_configure_cpp_target target)
    set(_sanitize_enabled OFF)
    if (UWF_SANITIZE AND NOT MSVC)
        set(_sanitize_enabled ON)
    endif ()

    target_compile_definitions(${target} PRIVATE UNICODE _UNICODE _WIN32_DCOM NOMINMAX)
    if (_sanitize_enabled)
        target_compile_definitions(${target} PRIVATE UWF_SANITIZE)
    endif ()

    if (MSVC)
        set_target_properties(${target} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
        target_compile_options(${target} PRIVATE
                /W4 /WX /permissive-
                /GS
                $<$<CONFIG:Release>:/Os>
                $<$<CONFIG:Release>:/Gy>
                $<$<CONFIG:Release>:/Gw>
                $<$<CONFIG:Release>:/guard:cf>
                $<$<CONFIG:Release>:/sdl>
        )
        return()
    endif ()

    target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic
            -Wconversion -Wsign-conversion
            -Werror
            -Werror=sign-compare
            -Werror=conversion
            -Werror=sign-conversion
            -Werror=narrowing
            -fstack-protector-strong
    )
    target_link_options(${target} PRIVATE -fstack-protector-strong)
    if (_sanitize_enabled)
        target_compile_options(${target} PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
                -fno-sanitize-recover=all
        )
        target_link_options(${target} PRIVATE
                -fsanitize=address,undefined
                -fuse-ld=lld
        )
    else ()
        target_link_options(${target} PRIVATE
                -static
                -static-libgcc
                -static-libstdc++
                -fuse-ld=lld
                -Wl,-Bstatic
        )
    endif ()
endfunction()

function(uwf_configure_target target)
    uwf_configure_cpp_target(${target})

    target_sources(${target} PRIVATE "${PROJECT_SOURCE_DIR}/resources/app.rc")
    if (MSVC)
        target_sources(${target} PRIVATE "${PROJECT_SOURCE_DIR}/resources/app.manifest")
    else ()
        target_sources(${target} PRIVATE "${PROJECT_SOURCE_DIR}/resources/app_manifest.rc")
    endif ()

    target_link_libraries(${target} PRIVATE wbemuuid ole32 oleaut32 shell32 dwmapi dxgi)
    if (NOT (UWF_SANITIZE AND NOT MSVC))
        target_link_libraries(${target} PRIVATE dbghelp)
    endif ()

    if (MSVC)
        set_target_properties(${target} PROPERTIES
                INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
                PDB_NAME "UWF"
                PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
        target_compile_options(${target} PRIVATE $<$<CONFIG:Release>:/Zi>)
        target_link_options(${target} PRIVATE
                "/MANIFEST:NO"
                "/MANIFESTUAC:NO"
                /DEBUG:FULL
                $<$<CONFIG:Release>:/DYNAMICBASE>
                $<$<CONFIG:Release>:/HIGHENTROPYVA>
                $<$<CONFIG:Release>:/NXCOMPAT>
                $<$<CONFIG:Release>:/GUARD:CF>
                $<$<CONFIG:Release>:/CETCOMPAT>
                $<$<CONFIG:Release>:/OPT:REF>
                $<$<CONFIG:Release>:/OPT:ICF>
        )
    endif ()
endfunction()
