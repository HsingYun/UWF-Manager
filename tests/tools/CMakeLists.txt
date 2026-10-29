# SPDX-License-Identifier: GPL-3.0-or-later

function(uwf_add_test_tool target library module implementation)
    add_library(${library} STATIC "${module}/${implementation}.cpp" "${module}/${implementation}.h")
    set_target_properties(${library} PROPERTIES AUTOMOC OFF AUTOUIC OFF)
    target_include_directories(${library} PUBLIC "${PROJECT_SOURCE_DIR}/tests")
    target_link_libraries(${library} PUBLIC Qt6::Core ${ARGN})
    uwf_configure_cpp_target(${library})

    qt_add_executable(${target} "${module}/main.cpp")
    set_target_properties(${target} PROPERTIES AUTOMOC OFF AUTOUIC OFF)
    target_link_libraries(${target} PRIVATE ${library})
    uwf_configure_cpp_target(${target})
endfunction()

uwf_add_test_tool(UwfTranslationCatalogCheck UwfCatalogCheckSupport catalogs CatalogCheck)
target_link_libraries(UwfTranslationCatalogCheck PRIVATE UwfApplication)
uwf_add_test_tool(UwfImageCompare UwfImageCompareSupport images ImageCompare Qt6::Gui)
uwf_add_test_tool(UwfTestResultCheck UwfTestResultsSupport results TestResults)
