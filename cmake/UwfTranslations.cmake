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
    set(qm_directory "${CMAKE_CURRENT_BINARY_DIR}/${target}_translations")
    qt_add_lrelease(${target}
        TS_FILES ${UWF_APP_I18N_TS_FILES} ${UWF_QT_I18N_TS_FILES}
        QM_OUTPUT_DIRECTORY "${qm_directory}"
        QM_FILES_OUTPUT_VARIABLE qm_files
    )
    qt_add_resources(${target} "${target}_translations"
        PREFIX "/i18n"
        BASE "${qm_directory}"
        FILES ${qm_files}
    )
endfunction()
