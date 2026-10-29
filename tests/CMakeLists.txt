# SPDX-License-Identifier: GPL-3.0-or-later

include(UwfTestTargets)

add_subdirectory(tools)
add_subdirectory(support)
add_subdirectory(cases)

# This gate validates translation artifacts; it is not a QtTest behavior suite.
string(JOIN "," uwf_test_locales ${UWF_I18N_LOCALES})
add_test(NAME UwfTranslationCatalogCheck
    COMMAND UwfTranslationCatalogCheck
        --root "${PROJECT_SOURCE_DIR}"
        --lupdate "$<TARGET_FILE:Qt6::lupdate>"
        --locales "${uwf_test_locales}"
)
set_tests_properties(UwfTranslationCatalogCheck PROPERTIES
    LABELS "kind-check;feature-localization;runtime-appless"
    TIMEOUT 180
)
