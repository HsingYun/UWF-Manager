# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

include(UwfSources)
include(UwfTargetOptions)
include(UwfVersion)
uwf_select_platform_adapters()
include(UwfDesign)

# Include the design library and every component registered below in extraction.
set(UWF_COMPONENT_TARGETS UwfDesignSystem)

# QObject implementations include their own moc output so static-library
# consumers do not pull unrelated QObject implementations into the link.
function(uwf_add_component target source_group)
    add_library(${target} STATIC ${${source_group}})
    target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/src")
    uwf_configure_production_target(${target})
    set(UWF_COMPONENT_TARGETS ${UWF_COMPONENT_TARGETS} ${target} PARENT_SCOPE)
endfunction()

uwf_add_version(UwfVersion)

set(uwf_foundation ${UWF_FOUNDATION_SOURCES} ${uwf_local_time})
uwf_add_component(UwfFoundation uwf_foundation)
set_target_properties(UwfFoundation PROPERTIES AUTOMOC OFF AUTOUIC OFF)
uwf_add_component(UwfObjectNameComparison uwf_object_names)
set_target_properties(UwfObjectNameComparison PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfObjectNameComparison PRIVATE ${uwf_object_name_libraries})
uwf_add_component(UwfPathOperations uwf_paths)
set_target_properties(UwfPathOperations PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfPathOperations PUBLIC Qt6::Core)

uwf_add_component(UwfDomain UWF_DOMAIN_SOURCES)
set_target_properties(UwfDomain PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfDomain PUBLIC UwfFoundation PRIVATE UwfObjectNameComparison)

# Backends do not link these optional inference components. Their direct tests
# exercise the complete implementation without a solver in the link closure.
uwf_add_component(UwfContractSolvers UWF_CONTRACT_SOLVER_SOURCES)
set_target_properties(UwfContractSolvers PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfContractSolvers PUBLIC UwfDomain)
uwf_add_component(UwfAllVolumeProtectionSolvers UWF_ALL_VOLUME_PROTECTION_SOLVER_SOURCES)
set_target_properties(UwfAllVolumeProtectionSolvers PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfAllVolumeProtectionSolvers PUBLIC UwfDomain)

uwf_add_component(UwfRequestPrecheck UWF_REQUEST_PRECHECK_SOURCES)
set_target_properties(UwfRequestPrecheck PROPERTIES AUTOMOC OFF AUTOUIC OFF)
uwf_add_component(UwfOperationExecution UWF_OPERATION_EXECUTION_SOURCES)
set_target_properties(UwfOperationExecution PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfOperationExecution PUBLIC UwfDomain PRIVATE UwfObjectNameComparison)
target_link_libraries(UwfRequestPrecheck PUBLIC UwfDomain PRIVATE UwfContractSolvers UwfOperationExecution)

uwf_add_component(UwfCompatibilityRuntime UWF_COMPATIBILITY_RUNTIME_SOURCES)
set_target_properties(UwfCompatibilityRuntime PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfCompatibilityRuntime PUBLIC UwfDomain)

uwf_add_component(UwfApplicationContracts UWF_APPLICATION_CONTRACTS_SOURCES)
target_link_libraries(UwfApplicationContracts PUBLIC UwfDomain Qt6::Core)

uwf_add_component(UwfApplication UWF_APPLICATION_SOURCES)
target_link_libraries(UwfApplication PUBLIC UwfApplicationContracts PRIVATE UwfVersion UwfObjectNameComparison UwfPathOperations)
uwf_add_component(UwfConfigurationComposition UWF_CONFIGURATION_COMPOSITION_SOURCES)
set_target_properties(UwfConfigurationComposition PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfConfigurationComposition PUBLIC UwfApplication)
uwf_add_component(UwfAllVolumeProtection UWF_ALL_VOLUME_PROTECTION_SOURCES)
set_target_properties(UwfAllVolumeProtection PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfAllVolumeProtection PUBLIC UwfApplication PRIVATE UwfAllVolumeProtectionSolvers)

uwf_add_component(UwfPresentationModels UWF_PRESENTATION_MODELS_SOURCES)

target_link_libraries(UwfPresentationModels PUBLIC Qt6::Core)

uwf_add_component(UwfViewModels UWF_VIEWMODEL_SOURCES)
target_link_libraries(UwfViewModels PUBLIC UwfApplication UwfPresentationModels Qt6::Core)

set(uwf_views ${UWF_VIEW_SOURCES} ${uwf_platform_views})
uwf_add_component(UwfViews uwf_views)
target_link_libraries(UwfViews
    PUBLIC UwfViewModels UwfDesignSystem Qt6::Widgets Qt6::Svg
    PRIVATE UwfVersion
)

set(uwf_desktop ${UWF_DESKTOP_COMPOSITION_SOURCES} ${uwf_platform_desktop_sources})
uwf_add_component(UwfDesktopComposition uwf_desktop)
target_link_libraries(UwfDesktopComposition
    PUBLIC UwfViews
    PRIVATE UwfAllVolumeProtection UwfConfigurationComposition
)

uwf_add_component(UwfConsolePresentation UWF_CONSOLE_PRESENTATION_SOURCES)
set_target_properties(UwfConsolePresentation PROPERTIES AUTOMOC OFF AUTOUIC OFF)
target_link_libraries(UwfConsolePresentation PUBLIC UwfApplication)

uwf_add_platform_components()

# Explicit ownership prevents a moved file from silently becoming dead source,
# or being compiled by two layers. Generated headers are outside src.
file(GLOB_RECURSE uwf_production_files CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.cpp"
)
set(uwf_registered_files)
foreach(target IN LISTS UWF_COMPONENT_TARGETS)
    get_target_property(sources ${target} SOURCES)
    foreach(source IN LISTS sources)
        get_filename_component(absolute "${source}" ABSOLUTE BASE_DIR "${PROJECT_SOURCE_DIR}")
        if(absolute MATCHES "^${PROJECT_SOURCE_DIR}/src/.*\\.(cpp|h)$")
            if(absolute IN_LIST uwf_registered_files)
                message(FATAL_ERROR "Production source belongs to multiple components: ${absolute}")
            endif()
            list(APPEND uwf_registered_files "${absolute}")
        endif()
    endforeach()
endforeach()
# Inactive platform implementations retain explicit ownership without being built.
foreach(source IN LISTS uwf_inactive_sources)
    get_filename_component(absolute "${source}" ABSOLUTE BASE_DIR "${PROJECT_SOURCE_DIR}")
    if(absolute IN_LIST uwf_registered_files)
        message(FATAL_ERROR "Production source belongs to multiple components: ${absolute}")
    endif()
    list(APPEND uwf_registered_files "${absolute}")
endforeach()
foreach(source IN LISTS uwf_production_files)
    if(NOT source IN_LIST uwf_registered_files)
        message(FATAL_ERROR "Production source has no component: ${source}")
    endif()
endforeach()

# Verify transitive link independence, including LINK_ONLY generator edges.
# Merely banning a direct solver include would miss an accidental library edge.
function(uwf_assert_backend_independent target)
    set(pending "${target}")
    set(visited)
    while(pending)
        list(POP_FRONT pending current)
        if(current IN_LIST visited OR NOT TARGET "${current}")
            continue()
        endif()
        if(current STREQUAL "UwfContractSolvers" OR current STREQUAL "UwfRequestPrecheck"
            OR current STREQUAL "UwfAllVolumeProtectionSolvers" OR current STREQUAL "UwfAllVolumeProtection")
            message(FATAL_ERROR "Backend ${target} depends on optional inference: ${current}")
        endif()
        list(APPEND visited "${current}")
        foreach(property LINK_LIBRARIES INTERFACE_LINK_LIBRARIES)
            get_target_property(links "${current}" "${property}")
            string(REGEX MATCHALL "Uwf[A-Za-z0-9_]+" edges "${links}")
            list(APPEND pending ${edges})
        endforeach()
    endwhile()
    message(STATUS "Backend link independence checked: ${target}")
endfunction()
uwf_assert_backend_independent(UwfCompatibilityRuntime)
foreach(backend IN LISTS uwf_platform_backends)
    uwf_assert_backend_independent(${backend})
endforeach()

if(UWF_BUILD_DESIGN_TOOLS)
    include(UwfResources)
    include(UwfTranslations)
    add_subdirectory(support/tools/preview "${PROJECT_BINARY_DIR}/preview")
endif()
