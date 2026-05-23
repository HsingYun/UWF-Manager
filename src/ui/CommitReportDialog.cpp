#include "CommitReportDialog.h"

#include <QAction>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QShortcut>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <algorithm>

#include "../uwf/wmi/WmiError.h"
#include "I18n.h"
#include "TableText.h"

namespace uwf::ui {

// 把 HRESULT / UWF returnValue 翻译成普通用户看得懂的一句话。
// 不在这里暴露 "WBEM_E_*" / "ExecMethod" 这些实现术语——那些进日志。
//   * NotFound 对 Commit：overlay 里没有该目标的待提交改动。
//   * NotFound 对 Deletion：目标不在物理盘 / 持久化 hive 上——只活在 overlay 里，
//     重启就没；本来就不需要也无法用提交删除。
QString explainCommitFailure(int32_t hresult, uint32_t returnValue, bool isDeletion) {
  if (hresult != 0) {
    switch (uwf::WmiError(hresult).code()) {
      // WBEM_E_FAILED（0x80041001）是一般性失败码，没有确定原因。实测最常见
      // 的一种诱因：目标被其他进程占着 handle（例如资源管理器正在浏览该目录、
      // 文本编辑器打开了文件、regedit 打开了键、AV 在扫描），UWF 看到非
      // 0 引用计数就拒。措辞要带"可能"避免误导——其它原因（权限、UWF 内部
      // 状态等）也会落到同一个 HRESULT，无法仅凭代码区分。
      case uwf::WmiErrorCode::Failed:
        return I18n::tr(
            "The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any "
            "program holding it and try again.");
      case uwf::WmiErrorCode::NotFound:
        if (isDeletion) {
          return I18n::tr(
              "Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It "
              "will disappear on reboot; commit-delete is neither needed nor possible.");
        }
        return I18n::tr("The target was not found; there is nothing in the overlay to commit.");
      case uwf::WmiErrorCode::InvalidParameter:
        return I18n::tr("A parameter was rejected by the system (invalid path or argument).");
      default:
        return I18n::tr("The operation failed (see log for details).");
    }
  }
  if (returnValue != 0) return I18n::tr("Operation rejected (code %1).").arg(returnValue);
  return I18n::tr("Unknown cause.");
}

QString formatErrorCode(int32_t hresult, uint32_t returnValue) {
  // %08X：大写 8 位定宽十六进制；显示 "0x" 小写前缀更符合 Win32 文档惯例。
  if (hresult != 0) return QString::asprintf("0x%08X", static_cast<uint32_t>(hresult));
  if (returnValue != 0) return QString("rv=%1").arg(returnValue);
  return "-";
}

// 列：类别 / 路径 / [删除操作额外的"执行前存在""执行后存在"] / 错误码 / 原因。
// 大批量（十万条级）分页展示，每页 kReportPageSize 行。用普通 QDialog 而非
// QMessageBox，避免 Windows 提示音。
void showCommitReport(QWidget* parent, const QList<CommitReportRow>& rows, int canceledRemaining) {
  constexpr int kReportPageSize = 200;

  // 删除操作才填了"执行前 / 后是否存在"；提交操作为 nullopt，那两列不出现。
  // 提前算：决定要不要多两列、影响默认窗宽。下方表头处复用同一份判定。
  const bool showExistence = !rows.isEmpty() && rows.front().existedBefore.has_value();

  QDialog dlg(parent);
  dlg.setWindowTitle(canceledRemaining > 0 ? I18n::tr("Commit canceled") : I18n::tr("Commit result"));
  dlg.resize(showExistence ? 1300 : 1180, 520);
  auto* lay = new QVBoxLayout(&dlg);

  const QString okLabel = I18n::tr("Succeeded");
  const QString skipLabel = I18n::tr("Skipped");
  int okN = 0, skipN = 0, failN = 0;
  for (const auto& r : rows) {
    if (r.category == okLabel)
      ++okN;
    else if (r.category == skipLabel)
      ++skipN;
    else
      ++failN;
  }
  QString summaryText = I18n::tr("%1 succeeded; %2 skipped; %3 failed.").arg(okN).arg(skipN).arg(failN);
  if (canceledRemaining > 0) summaryText += I18n::tr("\nCanceled by user; %1 entries not processed.").arg(canceledRemaining);
  auto* summary = new QLabel(summaryText);
  summary->setTextInteractionFlags(Qt::TextSelectableByMouse);
  lay->addWidget(summary);

  QStringList headers{I18n::tr("Category"), I18n::tr("Path")};
  if (showExistence) headers << I18n::tr("Existed before") << I18n::tr("Exists after");
  headers << I18n::tr("Error code") << I18n::tr("Reason");
  const int colCount = static_cast<int>(headers.size());

  auto* table = new QTableWidget(0, colCount, &dlg);
  table->setHorizontalHeaderLabels(headers);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  table->setTextElideMode(Qt::ElideMiddle);
  table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  auto* hh = table->horizontalHeader();
  // 全部 Interactive：QHeaderView 文档明确 Stretch / ResizeToContents 都禁止用户拖动
  // 列宽；只有 Interactive / Fixed 让用户拖。给每列一个合理初值（按内容典型宽度），
  // 路径列默认给得最宽。Reason 不能再用 ResizeToContents——它有时是一长串中文
  // （"目标可能正被其他程序占用…"）会把整张表撑爆，挤掉路径列变成 "C..."。
  // 横向滚动模式已开（ScrollPerPixel），列宽总和 > 表宽时用户可以横向滚动看全。
  hh->setSectionResizeMode(QHeaderView::Interactive);
  hh->setStretchLastSection(false);
  int initialCol = 0;
  hh->resizeSection(initialCol++, 70);   // Category：「成功」/「跳过」/「失败」
  hh->resizeSection(initialCol++, 520);  // Path：大头——典型 Windows 路径
  if (showExistence) {
    hh->resizeSection(initialCol++, 100);  // Existed before：「是」/「否」+ 表头宽度
    hh->resizeSection(initialCol++, 100);  // Exists after
  }
  hh->resizeSection(initialCol++, 120);  // Error code：「0x80041001」
  hh->resizeSection(initialCol++, 360);  // Reason：可能很长，给 360 起；用户可拖宽
  lay->addWidget(table, 1);

  const QString yes = I18n::tr("Yes");
  const QString no = I18n::tr("No");
  const int pageCount = rows.isEmpty() ? 1 : (static_cast<int>(rows.size()) + kReportPageSize - 1) / kReportPageSize;

  auto fillPage = [&](int page) {
    const int start = page * kReportPageSize;
    const int end = std::min<int>(start + kReportPageSize, static_cast<int>(rows.size()));
    table->setRowCount(end - start);
    for (int i = start; i < end; ++i) {
      const auto& r = rows[i];
      const int vr = i - start;
      int c = 0;
      table->setItem(vr, c++, new QTableWidgetItem(r.category));
      auto* pathItem = new QTableWidgetItem(r.path);
      pathItem->setToolTip(r.path);
      table->setItem(vr, c++, pathItem);
      if (showExistence) {
        table->setItem(vr, c++, new QTableWidgetItem(r.existedBefore.value_or(false) ? yes : no));
        table->setItem(vr, c++, new QTableWidgetItem(r.existsAfter.value_or(false) ? yes : no));
      }
      table->setItem(vr, c++, new QTableWidgetItem(r.errorCode));
      auto* reasonItem = new QTableWidgetItem(r.reason);
      reasonItem->setToolTip(r.reason);
      table->setItem(vr, c++, reasonItem);
    }
  };

  // 分页栏常驻显示——即使只有一页也保留，让"分页 + 总条数"这两条信息一直可见，
  // 同时和 LogViewerDialog 的体感一致。按钮按状态置灰，标签固定写"第 X / Y 页 · 共 N 条"。
  int currentPage = 0;
  auto* pageBar = new QHBoxLayout();
  auto* prevBtn = new QPushButton(I18n::tr("Previous page"), &dlg);
  auto* nextBtn = new QPushButton(I18n::tr("Next page"), &dlg);
  auto* pageLabel = new QLabel(&dlg);
  pageBar->addStretch();
  pageBar->addWidget(prevBtn);
  pageBar->addWidget(pageLabel);
  pageBar->addWidget(nextBtn);
  pageBar->addStretch();
  lay->addLayout(pageBar);

  const int totalRows = static_cast<int>(rows.size());
  auto refreshPage = [&]() {
    fillPage(currentPage);
    pageLabel->setText(I18n::tr("Page %1 / %2 · %3 entries total").arg(currentPage + 1).arg(pageCount).arg(totalRows));
    prevBtn->setEnabled(currentPage > 0);
    nextBtn->setEnabled(currentPage + 1 < pageCount);
  };
  QObject::connect(prevBtn, &QPushButton::clicked, &dlg, [&] {
    if (currentPage > 0) {
      --currentPage;
      refreshPage();
    }
  });
  QObject::connect(nextBtn, &QPushButton::clicked, &dlg, [&] {
    if (currentPage + 1 < pageCount) {
      ++currentPage;
      refreshPage();
    }
  });
  refreshPage();

  // "复制全部"复制所有行（不止当前页），直接从 rows 拼 TSV。
  auto allRowsToText = [&] {
    QString out = headers.join('\t');
    for (const auto& r : rows) {
      out += '\n' + r.category + '\t' + r.path;
      if (showExistence) {
        out += '\t';
        out += r.existedBefore.value_or(false) ? yes : no;
        out += '\t';
        out += r.existsAfter.value_or(false) ? yes : no;
      }
      out += '\t' + r.errorCode + '\t' + r.reason;
    }
    return out;
  };

  auto* copyShortcut = new QShortcut(QKeySequence::Copy, table);
  copyShortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(copyShortcut, &QShortcut::activated, table, [table] {
    const auto txt = tableSelectionToText(table);
    if (!txt.isEmpty()) QGuiApplication::clipboard()->setText(txt);
  });
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(table, &QWidget::customContextMenuRequested, table, [table, allRowsToText](const QPoint& pos) {
    QMenu menu;
    auto* copySel = menu.addAction(I18n::tr("Copy selected rows"));
    auto* copyAll = menu.addAction(I18n::tr("Copy all"));
    copySel->setEnabled(!table->selectedRanges().isEmpty());
    QObject::connect(copySel, &QAction::triggered, [table] { QGuiApplication::clipboard()->setText(tableSelectionToText(table)); });
    QObject::connect(copyAll, &QAction::triggered, [allRowsToText] { QGuiApplication::clipboard()->setText(allRowsToText()); });
    menu.exec(table->viewport()->mapToGlobal(pos));
  });

  auto* btns = new QDialogButtonBox;
  const auto* copyAllBtn = btns->addButton(I18n::tr("Copy all"), QDialogButtonBox::ActionRole);
  QObject::connect(copyAllBtn, &QPushButton::clicked, &dlg, [allRowsToText] { QGuiApplication::clipboard()->setText(allRowsToText()); });
  const auto* closeBtn = btns->addButton(I18n::tr("Close"), QDialogButtonBox::AcceptRole);
  QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  lay->addWidget(btns);

  dlg.exec();
}

}  // namespace uwf::ui
