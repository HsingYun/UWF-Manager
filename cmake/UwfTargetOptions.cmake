# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

function(uwf_configure_cpp_target target)
    target_compile_definitions(${target} PRIVATE $<$<CONFIG:Debug>:UWF_DEBUG_LOGGING>)
    uwf_configure_platform_cpp_target(${target})
endfunction()

function(uwf_configure_production_target target)
    uwf_configure_cpp_target(${target})
    uwf_configure_platform_production_target(${target})
endfunction()

function(uwf_configure_application_target target)
    uwf_configure_production_target(${target})
    uwf_configure_platform_application_target(${target})
endfunction()
