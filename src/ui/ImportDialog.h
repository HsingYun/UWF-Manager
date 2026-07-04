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

// "导入 uwfmgr 命令" 对话框。
//
// 工作流：
//   - 用户在多行文本框里键入 / 粘贴 uwfmgr 命令，或者点 "Load from file…"
//     按钮选一个文本类文件，dialog 会用 grep 把文件里含 "uwfmgr" 的行追加
//     到文本框；也可以点 "Load default rules" 选择一组 Microsoft 推荐的默认
//     排除规则模板追加到文本框；
//   - 点击 Import → dialog 调 uwf::api::parseUwfmgrText 把每一行解析成
//     api::UwfmgrCommand；
//   - 调用通过 setApplier 注入的回调把这些命令"应用"为 UI 上的 pending 变更
//     （由 MainWindow 实现：filter / overlay 走 GlobalStatusPanel；按盘符
//     走对应 DiskTab）；
//   - 把每行结果（成功 / 重复 / 失败 / 不支持）渲染到结果表格里。
//
// 不支持把文件拖到对话框里——本程序需要管理员身份运行，Windows UAC/UIPI
// 会阻断从普通用户身份的资源管理器到管理员进程的 OLE 拖放（哪怕做了
// ChangeWindowMessageFilter 在新版 Windows 上也仍然失败）。所以走 QFileDialog
// 选文件是唯一可靠路径。
//
// uwfmgr CLI ↔ UWF API state 的语法/结构互转都在 src/uwf/api/UwfmgrCli。
// dialog 只负责 UI：拼按钮、累计 summary、把 ParseError 翻成中文提示等等。

#include <QDialog>
#include <QList>
#include <QString>
#include <functional>

#include "../uwf/api/UwfmgrCli.h"

class QPushButton;
class QPlainTextEdit;
class QTableWidget;
class QLabel;

namespace uwf::ui {

// 单行结果——UI 把它渲染到结果表格里。
struct ImportReportRow {
  enum class Status { Success, Duplicate, Failed, Unsupported };
  Status status = Status::Failed;
  int lineNo = 0;
  QString lineText;
  QString detail;
};

// 把 api::ParseError 翻成给用户看的中文提示。MainWindow 的 applier 在解析阶段
// 失败时调它来填 ImportReportRow::detail。errorContext 是 parser 在某些错误
// 下额外塞进来的"用户实际写的值"（UnknownType / InvalidVolume），用于拼到
// 错误模板里。
QString parseErrorMessage(api::ParseError e, const QString& errorContext);

class ImportDialog : public QDialog {
  Q_OBJECT
 public:
  explicit ImportDialog(QWidget* parent = nullptr);

  // 由 MainWindow 注入：拿到一组解析出来的命令，按顺序应用到 UI（不写 WMI），
  // 返回每条命令的执行结果。dialog 把结果展示到表格里。
  using Applier = std::function<QList<ImportReportRow>(const QList<api::UwfmgrCommand>&)>;
  void setApplier(Applier a) { m_applier = std::move(a); }

 private slots:
  void onImportClicked();

 private:
  // 把 m_applier 返回的本批结果**追加**到结果表底部（不清掉之前的）；
  // 同时更新累计 summary。从第二批开始会先插一条 "── Batch N ──" 分隔行
  // 让用户能区分批次边界。
  void appendReport(const QList<ImportReportRow>& rows);

  Applier m_applier;
  QPlainTextEdit* m_text = nullptr;
  QTableWidget* m_report = nullptr;
  QLabel* m_summary = nullptr;
  QPushButton* m_importBtn = nullptr;
  // 累计统计：dialog 一次会话里多次点 Import 累计——summary / report 都在
  // 这一份对象生命周期内持续叠加。每次 showImport() 会新建 dialog 实例，
  // 这些字段自然回到 0。
  int m_totalSuccess = 0;
  int m_totalDuplicate = 0;
  int m_totalFailed = 0;
  int m_totalUnsupported = 0;
  int m_batchNo = 0;
};

}  // namespace uwf::ui
