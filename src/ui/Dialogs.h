/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// uwf::ui::dialogs —— 全工程共用的对话框辅助。两类：
//   1) QMessageBox 替代品（warning / information / confirm / confirmCommit）。
//      原生 QMessageBox 的内部 label 走单独的字体路径，全局 app.setFont() 上设的
//      hinting / styleStrategy 不会传播过去，中文在 9-10pt 下渲染明显糊。这里用
//      QDialog + QLabel 自己拼，文字直接继承 app font。warning/information 默认让
//      文本可选中复制（排查 WMI 错误时方便贴日志），confirm 默认按钮是 Cancel。
//   2) FileDialogProvider / dialogBasePath —— 原生文件选择边界及其默认起始目录。

#include <QDir>
#include <QString>
#include <QStringList>

class QWidget;

namespace uwf::ui::dialogs {

struct FileDialogRequest {
  QString title;
  QString initialPath;
  QString filter;
};

// 文件选择属于平台 UI 边界。业务控件只依赖这组用户意图，不依赖 QFileDialog
// 的静态全局入口，因此离线数据源、自动化 UI 和其它宿主都能复用同一控件。
// 生产默认实现仍使用系统原生对话框；空返回值统一表示取消。
class FileDialogProvider {
 public:
  virtual ~FileDialogProvider() = default;
  [[nodiscard]] virtual QStringList openFiles(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString openFile(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString selectDirectory(QWidget* parent, const FileDialogRequest& request) = 0;
  [[nodiscard]] virtual QString saveFile(QWidget* parent, const FileDialogRequest& request) = 0;
};

// 进程生命周期内唯一的无状态系统实现。
FileDialogProvider& systemFileDialogs();

// 文件 / 文件夹选择对话框的默认 base 路径：盘符已知就用其根目录，否则用用户
// home。DiskTab 的 commit 入口、ExclusionListWidget 的添加按钮共用。
[[nodiscard]] inline QString dialogBasePath(const QString& driveLetter) { return driveLetter.isEmpty() ? QDir::homePath() : driveLetter + "\\"; }

void warning(QWidget* parent, const QString& title, const QString& text);
void information(QWidget* parent, const QString& title, const QString& text);

// 二选一确认。OK / Cancel 文案走 I18n::tr 拿翻译，避免依赖 Qt 内置翻译包。
// 返回 true = 用户选了 OK；false = 用户选了 Cancel 或关掉对话框。
bool confirm(QWidget* parent, const QString& title, const QString& text);

// 提交 / 删除到磁盘前的二次确认对话框，带视觉层次：操作标题（加粗）、目标
// （等宽字体 + 描边框，单独成块）、范围说明（次要色，detail 为空则不显示）、
// 固定的"不可撤销"警示横幅。比纯文本 confirm 更适合「即将改动磁盘 / 注册表」
// 这类破坏性操作。用户点"继续"返回 true。
bool confirmCommit(QWidget* parent, const QString& title, const QString& heading, const QString& target, const QString& detail);
bool confirmCommit(QWidget* parent, const QString& title, const QString& heading, const QString& target, const QString& detail,
                   const QStringList& previewItems);

// 用与提交确认一致的视觉层次说明操作为何不能执行。该对话框没有危险操作按钮，
// 只负责告知结果，避免调用方通过布尔参数把提示流程伪装成确认流程。
void showCommitBlocker(QWidget* parent, const QString& title, const QString& heading, const QString& target, const QString& reason);

}  // namespace uwf::ui::dialogs
