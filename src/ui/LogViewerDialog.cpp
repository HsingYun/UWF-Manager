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
#include "LogViewerDialog.h"

#include <QAction>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QShortcut>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../util/Log.h"
#include "../util/PostGate.h"
#include "I18n.h"
#include "Pager.h"
#include "PathElideDelegate.h"
#include "TableText.h"

namespace uwf::ui {

LogViewerDialog::LogViewerDialog(QWidget* parent) : QDialog(parent) {
  using LogRow = std::array<QString, 4>;
  constexpr int kRowHeight = 22;

  setWindowTitle(I18n::tr("Log"));
  resize(1100, 560);
  auto* layout = new QVBoxLayout(this);

  // 本地 QTableWidget 子类：暴露 resize 钩子，让外层在 viewport 高度变化时
  // 重算单页能装多少行。无 Q_OBJECT，纯虚函数 override。
  class PagedTable : public QTableWidget {
   public:
    using QTableWidget::QTableWidget;
    std::function<void()> onViewportResized;

   protected:
    void resizeEvent(QResizeEvent* e) override {
      QTableWidget::resizeEvent(e);
      if (onViewportResized) onViewportResized();
    }
  };

  auto* table = new PagedTable(0, 4, this);
  table->setHorizontalHeaderLabels({I18n::tr("Time"), I18n::tr("Level"), I18n::tr("Tag"), I18n::tr("Message")});
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  table->setWordWrap(false);
  table->setTextElideMode(Qt::ElideRight);
  // 永远不出垂直滚动条——pageSize 跟 viewport 高度走，单页内容必然刚好填满。
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto* hh = table->horizontalHeader();
  hh->setSectionResizeMode(0, QHeaderView::Interactive);
  hh->setSectionResizeMode(1, QHeaderView::Interactive);
  // Tag 列按内容定宽——最长 tag 是 "UWF_RegistryFilter"，让 Qt 自己量；
  // 不再像以前那样手动加裕度（之前是为了绕 paint 时 elide 宽度被算小的 bug，
  // 现在套了 PathElideDelegate 解决了根因，不需要 padding 兜底）。
  hh->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  hh->setSectionResizeMode(3, QHeaderView::Stretch);
  table->setColumnWidth(0, 110);
  table->setColumnWidth(1, 50);
  // Tag / Message 套 PathElideDelegate——绕开 Qt 默认 paint 路径在非 100% DPI 下
  // 把 elide 宽度算小的 bug（详见 PathElideDelegate.h）。
  table->setItemDelegateForColumn(2, new PathElideDelegate(table));
  table->setItemDelegateForColumn(3, new PathElideDelegate(table));
  table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  table->verticalHeader()->setDefaultSectionSize(kRowHeight);
  table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  layout->addWidget(table, 1);

  auto* statusRow = new QHBoxLayout();
  auto* statusLabel = new QLabel(this);
  statusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  statusRow->addWidget(statusLabel, 1);
  auto* progressBar = new QProgressBar(this);
  progressBar->setMinimumWidth(220);
  progressBar->setMaximumHeight(14);
  progressBar->setRange(0, 0);  // 不定长，worker 加载期间打转
  progressBar->setTextVisible(false);
  progressBar->hide();
  statusRow->addWidget(progressBar);
  layout->addLayout(statusRow);

  // 分页导航
  auto* pageRow = new QHBoxLayout();
  auto* firstBtn = new QPushButton(QStringLiteral("«"), this);
  auto* prevBtn = new QPushButton(QStringLiteral("‹"), this);
  auto* pageInfo = new QLabel(this);
  pageInfo->setAlignment(Qt::AlignCenter);
  pageInfo->setMinimumWidth(220);
  auto* nextBtn = new QPushButton(QStringLiteral("›"), this);
  auto* lastBtn = new QPushButton(QStringLiteral("»"), this);
  for (auto* b : {firstBtn, prevBtn, nextBtn, lastBtn}) b->setMaximumWidth(36);
  pageRow->addStretch(1);
  pageRow->addWidget(firstBtn);
  pageRow->addWidget(prevBtn);
  pageRow->addWidget(pageInfo);
  pageRow->addWidget(nextBtn);
  pageRow->addWidget(lastBtn);
  pageRow->addStretch(1);
  layout->addLayout(pageRow);

  auto parseLine = [](const std::string& line) -> LogRow {
    if (line.size() > 2 && line.front() == '[') {
      const auto rb = line.find(']');
      if (rb != std::string::npos) {
        const std::string head = line.substr(1, rb - 1);
        const std::string msg = line.substr(rb + 1);
        const auto sp1 = head.find(' ');
        const auto sp2 = sp1 == std::string::npos ? sp1 : head.find(' ', sp1 + 1);
        if (sp1 != std::string::npos && sp2 != std::string::npos) {
          return {QString::fromStdString(head.substr(0, sp1)), QString::fromStdString(head.substr(sp1 + 1, sp2 - sp1 - 1)),
                  QString::fromStdString(head.substr(sp2 + 1)), QString::fromStdString(msg).trimmed()};
        }
      }
    }
    return {QString(), QString(), QString(), QString::fromStdString(line)};
  };

  // 分页状态。entries 在 worker 解析完后通过主线程 invokeMethod 一次性写入；
  // 之后 nav（currentPage / pageSize）由 nav 按钮 / resize 改、renderPage 读。所有
  // 写都在主线程，无锁。generation 用 atomic 是因为 worker 也读它（判断结果还要不要
  // 回投）。pageSize 初值 1 占位——首个 resize event（dialog show 时必发）按 viewport
  // 高度修正。分页算术统一走 ui::Pager（与 OverlayFiles / CommitReport 同一套）。
  struct LogState {
    std::vector<LogRow> entries;
    Pager nav;
    std::atomic<int> generation{0};
  };
  auto pager = std::make_shared<LogState>();

  QPointer<QTableWidget> tablePtr(table);
  QPointer<QLabel> statusPtr(statusLabel);
  QPointer<QProgressBar> barPtr(progressBar);
  QPointer<QLabel> pageInfoPtr(pageInfo);
  QPointer<QPushButton> firstPtr(firstBtn);
  QPointer<QPushButton> prevPtr(prevBtn);
  QPointer<QPushButton> nextPtr(nextBtn);
  QPointer<QPushButton> lastPtr(lastBtn);

  auto totalPages = [pager](int n) { return pager->nav.pageCount(n); };

  auto renderPage = [pager, tablePtr, pageInfoPtr, firstPtr, prevPtr, nextPtr, lastPtr, totalPages]() {
    if (!tablePtr) return;
    const int total = static_cast<int>(pager->entries.size());
    pager->nav.clamp(total);
    const int pages = totalPages(total);

    const int start = pager->nav.pageStart();
    const int end = pager->nav.pageEnd(total);
    const int rows = end - start;

    tablePtr->setSortingEnabled(false);
    tablePtr->clearContents();
    tablePtr->setRowCount(rows);
    tablePtr->setUpdatesEnabled(false);
    for (int i = 0; i < rows; ++i) {
      const auto& f = pager->entries[static_cast<size_t>(start + i)];
      for (int c = 0; c < 4; ++c) {
        auto* it = new QTableWidgetItem(f[static_cast<size_t>(c)]);
        if (c == 3) it->setToolTip(f[3]);
        tablePtr->setItem(i, c, it);
      }
    }
    tablePtr->setUpdatesEnabled(true);

    if (pageInfoPtr) {
      pageInfoPtr->setText(pages == 0 ? I18n::tr("No log entries")
                                      : I18n::tr("Page %1 / %2 · %3 lines total").arg(pager->nav.currentPage + 1).arg(pages).arg(total));
    }
    if (firstPtr) firstPtr->setEnabled(pager->nav.currentPage > 0);
    if (prevPtr) prevPtr->setEnabled(pager->nav.currentPage > 0);
    if (nextPtr) nextPtr->setEnabled(pager->nav.currentPage < pages - 1);
    if (lastPtr) lastPtr->setEnabled(pager->nav.currentPage < pages - 1);
  };

  // viewport 高度变了就按 "viewport_height / row_height" 算新 pageSize；尽量
  // 保住"当前页第一条"原本指向的那条 entry，重新定位到对应页面。
  table->onViewportResized = [pager, table, renderPage]() {
    const int rowH = std::max(1, table->verticalHeader()->defaultSectionSize());
    const int viewH = std::max(0, table->viewport()->height());
    // setPageSize 内部 max(1,·) 并尽量保住"当前页第一条"；pageSize 真的变了才重渲染。
    if (pager->nav.setPageSize(viewH / rowH)) renderPage();
  };

  auto reload = [pager, statusPtr, barPtr, parseLine, renderPage, totalPages]() {
    const int gen = ++pager->generation;
    if (statusPtr) statusPtr->setText(I18n::tr("Loading log entries…"));
    if (barPtr) barPtr->show();

    std::thread([pager, gen, parseLine, statusPtr, barPtr, renderPage, totalPages]() {
      auto raw = uwf::recentLogLines();
      std::vector<LogRow> entries;
      entries.reserve(raw.size());
      for (auto& line : raw) entries.push_back(parseLine(line));

      // 经"可投递"门闸投回 UI 线程：app 已关停（main 调过 postgate::close）时
      // runIfOpen 直接跳过，不会向已销毁的 qApp 投递。详见 src/util/PostGate.h。
      uwf::postgate::runIfOpen([&]() {
        QMetaObject::invokeMethod(
            qApp,
            [pager, gen, entries = std::move(entries), statusPtr, barPtr, renderPage, totalPages]() mutable {
              if (pager->generation.load() != gen) return;
              pager->entries = std::move(entries);
              const int total = static_cast<int>(pager->entries.size());
              const int pages = totalPages(total);
              // 默认跳到最后一页（最新日志）。
              pager->nav.currentPage = pages > 0 ? pages - 1 : 0;
              renderPage();
              if (statusPtr) {
                statusPtr->setText(total == 0 ? I18n::tr("0 lines") : I18n::tr("%1 lines").arg(total));
              }
              if (barPtr) barPtr->hide();
            },
            Qt::QueuedConnection);
      });
    }).detach();
  };

  // 分页按钮 → 改 currentPage 后重渲染当前页（不重新解析日志）。
  connect(firstBtn, &QPushButton::clicked, this, [pager, renderPage]() {
    pager->nav.currentPage = 0;
    renderPage();
  });
  connect(prevBtn, &QPushButton::clicked, this, [pager, renderPage]() {
    if (pager->nav.currentPage > 0) --pager->nav.currentPage;
    renderPage();
  });
  connect(nextBtn, &QPushButton::clicked, this, [pager, renderPage, totalPages]() {
    const int pages = totalPages(static_cast<int>(pager->entries.size()));
    if (pager->nav.currentPage < pages - 1) ++pager->nav.currentPage;
    renderPage();
  });
  connect(lastBtn, &QPushButton::clicked, this, [pager, renderPage, totalPages]() {
    const int pages = totalPages(static_cast<int>(pager->entries.size()));
    pager->nav.currentPage = pages > 0 ? pages - 1 : 0;
    renderPage();
  });

  reload();

  auto* copyShortcut = new QShortcut(QKeySequence::Copy, table);
  copyShortcut->setContext(Qt::WidgetShortcut);
  connect(copyShortcut, &QShortcut::activated, table, [table]() {
    const auto txt = tableSelectionToText(table);
    if (!txt.isEmpty()) QGuiApplication::clipboard()->setText(txt);
  });

  table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(table, &QWidget::customContextMenuRequested, table, [table](const QPoint& pos) {
    QMenu menu;
    auto* copySel = menu.addAction(I18n::tr("Copy selected rows"));
    auto* copyAll = menu.addAction(I18n::tr("Copy current page"));
    copySel->setEnabled(!table->selectedRanges().isEmpty());
    // 用 &menu 当 context——actions 跟 menu 同生命周期，menu 析构时连接自动断；
    // 不加 context 的话 clazy 会报 connect-3arg-lambda（lambda 可能在 actions 死后被调）。
    QObject::connect(copySel, &QAction::triggered, &menu, [table]() { QGuiApplication::clipboard()->setText(tableSelectionToText(table)); });
    QObject::connect(copyAll, &QAction::triggered, &menu, [table]() { QGuiApplication::clipboard()->setText(tableAllToText(table)); });
    menu.exec(table->viewport()->mapToGlobal(pos));
  });

  // "Copy all" 走 pager 里的全量解析数据，避开"按当前页 table 内容拼"——
  // 那样只复制可见 500 行，跟用户期望的"全部"不符。
  auto copyAllFromPager = [pager]() {
    QString out;
    out.reserve(static_cast<qsizetype>(pager->entries.size()) * 64);
    out += QStringLiteral("Time\tLevel\tTag\tMessage\n");
    for (const auto& f : pager->entries) {
      out += f[0];
      out += QChar('\t');
      out += f[1];
      out += QChar('\t');
      out += f[2];
      out += QChar('\t');
      out += f[3];
      out += QChar('\n');
    }
    QGuiApplication::clipboard()->setText(out);
  };

  auto* btns = new QDialogButtonBox(this);
  auto* refreshBtn = btns->addButton(I18n::tr("Refresh"), QDialogButtonBox::ActionRole);
  auto* copyAllBtn = btns->addButton(I18n::tr("Copy all"), QDialogButtonBox::ActionRole);
  auto* clearBtn = btns->addButton(I18n::tr("Clear"), QDialogButtonBox::DestructiveRole);
  btns->addButton(I18n::tr("Close"), QDialogButtonBox::AcceptRole);
  layout->addWidget(btns);

  connect(refreshBtn, &QPushButton::clicked, this, reload);
  connect(copyAllBtn, &QPushButton::clicked, this, copyAllFromPager);
  connect(clearBtn, &QPushButton::clicked, this, [reload]() {
    uwf::clearLogLines();
    UWF_LOG_I("ui") << "log buffer cleared by user";
    reload();
  });
  connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

}  // namespace uwf::ui
