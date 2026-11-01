// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Confirmation dialogs focus Cancel by default.

#include <QDir>
#include <QString>
#include <QStringList>
#include <utility>

#include "application/files/contracts/FileBrowser.h"
#include "presentation/dialogs/DialogAppearance.h"

class QWidget;

namespace uwf::ui::dialogs {

struct FileDialogRequest {
  QString title;
  QString initialPath;
  QString filter;
  QStringList allowedRoots{};
};

// Injected file selection keeps widgets independent of the concrete picker.
// Empty results indicate cancellation.
class FileDialogProvider {
 public:
  virtual ~FileDialogProvider() = default;
  [[nodiscard]] virtual QStringList openFiles(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString openFile(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString selectDirectory(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString saveFile(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QVector<app::FileBrowserEntry> openItems(QWidget* parent, const FileDialogRequest& request) {
    QVector<app::FileBrowserEntry> result;
    for (auto paths = openFiles(parent, request); const auto& path : std::as_const(paths)) result.append({path, {}, false});
    return result;
  }
};

// Uses the drive root when known, otherwise the user home directory.
[[nodiscard]] inline QString dialogBasePath(const QString& driveLetter) { return driveLetter.isEmpty() ? QDir::homePath() : driveLetter + "\\"; }

// Messages are literal unless the caller explicitly requests rich text.
void warning(QWidget* parent, const QString& title, const QString& text, Qt::TextFormat format = Qt::PlainText);
void information(QWidget* parent, const QString& title, const QString& text, Qt::TextFormat format = Qt::PlainText);

// Cancellation and window closure return false.
bool confirm(QWidget* parent, const QString& title, const QString& text, Qt::TextFormat format = Qt::PlainText);
struct Confirmation {
  QString title;
  QString text;
  QString acceptText;
  DialogAction action = DialogAction::Destructive;
  Qt::TextFormat format = Qt::PlainText;
};
bool confirm(QWidget* parent, const Confirmation& request);

}  // namespace uwf::ui::dialogs
