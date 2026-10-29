# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 HsingYun (iakext@gmail.com)

include_guard(GLOBAL)

set(UWF_APP_RESOURCE_FILES
    # Themes
    resources/styles/surfaces.qss
    resources/styles/dialogs.qss

    # Application icon and common actions
    resources/icons/app.svg
    resources/icons/refresh.svg
    resources/icons/close.svg
    resources/icons/export.svg
    resources/icons/arrow.svg
    resources/icons/chevron.svg
    resources/icons/columns.svg
    resources/icons/minimize.svg
    resources/icons/maximize.svg
    resources/icons/restore.svg
    resources/icons/move.svg
    resources/icons/resize.svg
    resources/icons/check.svg
    resources/icons/import.svg
    resources/icons/search.svg
    resources/icons/copy.svg
    resources/icons/github.svg
    resources/icons/mail.svg
    resources/icons/external.svg
    resources/icons/service.svg
    resources/icons/code.svg
    resources/icons/spark.svg
    resources/icons/add.svg
    resources/icons/remove.svg
    resources/icons/trash.svg
    resources/icons/apply.svg
    resources/icons/info.svg
    resources/icons/more.svg
    resources/icons/lock.svg
    resources/icons/page_first.svg
    resources/icons/page_previous.svg
    resources/icons/page_next.svg
    resources/icons/page_last.svg
    resources/icons/scroll_up.svg
    resources/icons/scroll_down.svg
    resources/icons/scroll_left.svg
    resources/icons/scroll_right.svg

    # Volumes
    resources/icons/disk.svg
    resources/icons/disk_off.svg
    resources/icons/disk_system.svg

    # Documents and registry
    resources/icons/file.svg
    resources/icons/folder.svg
    resources/icons/registry.svg
    resources/icons/key.svg
    resources/icons/license.svg

    # Navigation and state indicators
    resources/icons/arrow_up.svg

    # Power and enhanced mode
    resources/icons/shutdown.svg
    resources/icons/restart.svg
    resources/icons/enhanced.svg

    # File staging and operation results
    resources/icons/file_staging.svg
    resources/icons/status_warning.svg
    resources/icons/status_error.svg
    resources/icons/status_success.svg
    resources/icons/commit.svg

    # Toolbar controls
    resources/icons/theme_sun.svg
    resources/icons/theme_moon.svg
    resources/icons/log.svg
    resources/icons/language.svg
    resources/icons/overlay_float_on.svg
    resources/icons/overlay_float_off.svg
)

# Application and tests embed identical assets under stable resource aliases.
function(uwf_add_resources target resource_name)
    set(resource_files)
    foreach(resource_file IN LISTS UWF_APP_RESOURCE_FILES)
        list(APPEND resource_files "${PROJECT_SOURCE_DIR}/${resource_file}")
    endforeach()
    qt_add_resources(${target} "${resource_name}"
        PREFIX "/"
        BASE "${PROJECT_SOURCE_DIR}/resources"
        FILES ${resource_files}
    )
endfunction()
