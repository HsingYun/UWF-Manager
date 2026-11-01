# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

# Application and standard Qt widget catalogs cover the same supported locales.
set(UWF_I18N_LOCALES zh_CN zh_TW ja ko de fr es it pt_BR pt_PT ru uk pl nl tr id vi ar)
set(UWF_APP_I18N_TS_FILES)
set(UWF_QT_I18N_TS_FILES)
foreach(locale IN LISTS UWF_I18N_LOCALES)
    list(APPEND UWF_APP_I18N_TS_FILES "${PROJECT_SOURCE_DIR}/resources/i18n/${locale}.ts")
    list(APPEND UWF_QT_I18N_TS_FILES "${PROJECT_SOURCE_DIR}/resources/i18n/qt/qtbase_${locale}.ts")
endforeach()

function(uwf_add_translations target)
    if(NOT TARGET UwfTranslationResources)
        # All consumers use identical catalogs. Compile and embed them once;
        # direct object-library linkage retains Qt's resource initializer.
        add_library(UwfTranslationResources OBJECT)
        set_target_properties(UwfTranslationResources PROPERTIES AUTOMOC OFF AUTOUIC OFF)
        target_link_libraries(UwfTranslationResources PRIVATE Qt6::Core)
        uwf_configure_cpp_target(UwfTranslationResources)

        set(qm_directory "${PROJECT_BINARY_DIR}/translations")
        qt_add_lrelease(UwfTranslationResources
            TS_FILES ${UWF_APP_I18N_TS_FILES} ${UWF_QT_I18N_TS_FILES}
            QM_OUTPUT_DIRECTORY "${qm_directory}"
            QM_FILES_OUTPUT_VARIABLE qm_files
        )
        qt_add_resources(UwfTranslationResources UWF_translations
            PREFIX "/i18n"
            BASE "${qm_directory}"
            FILES ${qm_files}
        )
    endif()
    target_link_libraries(${target} PRIVATE UwfTranslationResources)
endfunction()
