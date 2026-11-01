// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/controls/tables/PathEntryControls.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>

#include "design/DesignTokens.h"
#include "presentation/controls/inputs/SearchField.h"
#include "presentation/controls/pagination/PaginationBar.h"
#include "presentation/controls/tables/PathEntryListView.h"

namespace uwf::ui {
PathEntryControls createPathEntryControls(QWidget& host, QAbstractItemModel& model, const QString& heading, const QString& statusHeading) {
  PathEntryControls controls;
  auto* layout = new QVBoxLayout(&host);
  layout->setContentsMargins(0, 14, 0, 0);
  layout->setSpacing(14);

  controls.commands = new QWidget(&host);
  auto* toolbar = new QHBoxLayout(controls.commands);
  toolbar->setContentsMargins(0, 0, 0, 0);
  toolbar->setSpacing(3);
  controls.filter = new SearchField(controls.commands);
  controls.filter->setObjectName(QStringLiteral("filterInput"));
  controls.filter->setClearButtonEnabled(true);
  controls.filter->setMinimumWidth(80);
  controls.filter->setFixedHeight(30);
  toolbar->addWidget(controls.filter, 1);
  toolbar->addSpacing(9);

  controls.add = new QPushButton(QIcon(QStringLiteral(":/icons/add.svg")), {}, controls.commands);
  controls.add->setObjectName(QStringLiteral("primaryBtn"));
  controls.add->setProperty("designRole", "secondary");
  controls.remove = new QPushButton(controls.commands);
  controls.remove->setObjectName(QStringLiteral("dangerBtn"));
  controls.remove->setFixedWidth(32);
  for (auto* button : {controls.add, controls.remove}) {
    button->setFixedHeight(30);
    auto type = button->font();
    type.setPixelSize(12);
    button->setFont(type);
    toolbar->addWidget(button);
  }
  layout->addWidget(controls.commands);

  controls.list = new PathEntryListView(heading, statusHeading, &host);
  controls.list->setAlternatingRowColors(false);
  controls.list->setIconSize({16, 16});
  controls.list->setModel(&model);
  controls.list->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
  controls.list->setMinimumHeight(80);
  controls.list->setMinimumWidth(0);
  auto* records = new QVBoxLayout();
  records->setSpacing(design::listPaginationGap);
  records->addWidget(controls.list, 1);
  controls.pagination = new PaginationBar(controls.list->pages(), &host);
  controls.pagination->setObjectName(QStringLiteral("entryPagination"));
  records->addWidget(controls.pagination);
  layout->addLayout(records, 1);
  return controls;
}

void PathEntryControls::renderRemoval(const bool restoring, const QString& label) const {
  remove->setProperty("accessibleName", label);
  remove->setIcon(QIcon(restoring ? QStringLiteral(":/icons/restart.svg") : QStringLiteral(":/icons/remove.svg")));
  remove->setProperty("designRole", restoring ? "quiet" : "dangerQuiet");
  remove->update();
}
}  // namespace uwf::ui
