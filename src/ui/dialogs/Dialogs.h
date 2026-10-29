// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 共享消息、确认与文件选择边界。消息对话框继承应用字体渲染参数，确认操作默认聚焦取消。

#include <QDir>
#include <QString>
#include <QStringList>
#include <memory>

#include "DialogAppearance.h"
#include "app/ports/FileBrowser.h"

class QWidget;

namespace uwf::ui::dialogs {

struct FileDialogRequest {
  QString title;
  QString initialPath;
  QString filter;
  QStringList allowedRoots{};
};

// 文件选择属于平台 UI 边界。业务控件只依赖这组用户意图，不依赖 QFileDialog
// 的静态全局入口，因此离线数据源、自动化 UI 和其它宿主都能复用同一控件。
// 生产实现使用应用自绘的文件选择器；空返回值统一表示取消。
class FileDialogProvider {
 public:
  virtual ~FileDialogProvider() = default;
  [[nodiscard]] virtual QStringList openFiles(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString openFile(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString selectDirectory(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString saveFile(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QVector<app::FileBrowserEntry> openItems(QWidget* parent, const FileDialogRequest& request) {
    QVector<app::FileBrowserEntry> result;
    const auto paths = openFiles(parent, request);
    for (const auto& path : paths) result.append({path, {}, false});
    return result;
  }
};

// The composition root supplies the browser; the view layer owns only the UI.
std::unique_ptr<FileDialogProvider> createFileDialogs(std::shared_ptr<const app::FileBrowser> browser);

// 文件 / 文件夹选择对话框的默认 base 路径：盘符已知就用其根目录，否则用用户
// home。VolumeTab 的 commit 入口、ExclusionListWidget 的添加按钮共用。
[[nodiscard]] inline QString dialogBasePath(const QString& driveLetter) { return driveLetter.isEmpty() ? QDir::homePath() : driveLetter + "\\"; }

// Messages are literal unless the caller explicitly requests rich text.
void warning(QWidget* parent, const QString& title, const QString& text, Qt::TextFormat format = Qt::PlainText);
void information(QWidget* parent, const QString& title, const QString& text, Qt::TextFormat format = Qt::PlainText);

// 返回是否确认；取消或关闭均返回 false。
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
