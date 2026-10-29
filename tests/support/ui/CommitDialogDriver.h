// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QDialog>
#include <QPushButton>
#include <QTimer>
namespace uwf::test {
// Integration tests inspect the real report once, then close the owning flow.
inline bool driveCommitOperation(QDialog* dialog, bool reportInspected, bool& confirmationAccepted) {
  if (dialog->objectName() != QLatin1String("commitOperationDialog")) return false;
  auto* primary = dialog->findChild<QPushButton*>(QStringLiteral("commitOperationPrimary"));
  auto* report = dialog->findChild<QPushButton*>(QStringLiteral("commitOperationReport"));
  if (primary && primary->isVisible() && primary->isEnabled()) {
    confirmationAccepted = true;
    primary->click();
  } else if (report && report->isVisible()) {
    if (reportInspected)
      dialog->findChild<QPushButton*>(QStringLiteral("commitOperationCancel"))->click();
    else
      QTimer::singleShot(0, report, &QPushButton::click);
  }
  return true;
}
}  // namespace uwf::test
