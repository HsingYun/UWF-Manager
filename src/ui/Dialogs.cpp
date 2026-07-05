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
#include "Dialogs.h"

#include <QAbstractItemView>
#include <QAction>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QVBoxLayout>
#include <algorithm>

#include "I18n.h"
#include "Pager.h"
#include "StatusBanner.h"
#include "ThemeManager.h"

namespace uwf::ui::dialogs {

namespace {

// 统一构造 dialog 主体：标题、可选中正文 label、底部按钮区。caller 把按钮
// 自己 add 到返回的 QDialogButtonBox 上，再 connect 进 dlg.accept/reject。
struct DialogParts {
  QDialog* dlg;
  QLabel* body;
  QDialogButtonBox* btns;
};

DialogParts build(QWidget* parent, const QString& title, const QString& text) {
  auto* dlg = new QDialog(parent);
  dlg->setWindowTitle(title);

  auto* layout = new QVBoxLayout(dlg);
  layout->setContentsMargins(20, 16, 20, 12);
  layout->setSpacing(12);

  auto* body = new QLabel(text, dlg);
  body->setWordWrap(true);
  // 排错场景常需要复制文字进日志，所有 dialog 默认开启选中。
  body->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  layout->addWidget(body, 1);

  // 对话框宽度跟随内容：按最宽一行（整行不折所需的宽度）定下限，夹在
  // [360, 760]。够宽时 wordWrap 自然不折——长注册表路径之类的单行内容因而
  // 完整显示；只有极长文本才会折到 760 上限，避免对话框宽到溢出屏幕。
  const QFontMetrics fm(body->fontMetrics());
  int widest = 0;
  // 绑到 const 变量再 range-loop——直接 for (... : text.split('\n')) 会让
  // range-loop 隐式调非 const begin() 触发 Qt 容器隐式共享的 detach 检查。
  const QStringList lines = text.split('\n');
  for (const auto& line : lines) widest = std::max(widest, fm.horizontalAdvance(line));
  const auto margins = layout->contentsMargins();
  dlg->setMinimumWidth(std::clamp(widest + margins.left() + margins.right(), 360, 760));

  auto* btns = new QDialogButtonBox(dlg);
  layout->addWidget(btns);
  return {dlg, body, btns};
}

}  // namespace

void warning(QWidget* parent, const QString& title, const QString& text) {
  auto [dlg, body, btns] = build(parent, title, text);
  auto* ok = btns->addButton(I18n::tr("OK"), QDialogButtonBox::AcceptRole);
  QObject::connect(ok, &QPushButton::clicked, dlg, &QDialog::accept);
  dlg->exec();
  delete dlg;
}

void information(QWidget* parent, const QString& title, const QString& text) {
  // 当前实现 warning / information 完全等价：都是单 OK 按钮 + 可选中正文。
  // 留两个名字是为了 caller 表达意图（语义性 > DRY）。
  warning(parent, title, text);
}

bool confirm(QWidget* parent, const QString& title, const QString& text) {
  auto* dlg = new QDialog(parent);
  dlg->setWindowTitle(title);

  auto* layout = new QVBoxLayout(dlg);
  layout->setContentsMargins(22, 18, 22, 14);
  layout->setSpacing(14);

  auto* bodyRow = new QHBoxLayout();
  bodyRow->setContentsMargins(0, 0, 0, 0);
  bodyRow->setSpacing(12);

  auto* badge = new QLabel("!", dlg);
  badge->setAlignment(Qt::AlignCenter);
  badge->setFixedSize(28, 28);
  badge->setStyleSheet(QString("QLabel { background: %1; color: #FFFFFF; border-radius: 14px; font-weight: bold; }")
                           .arg(ThemeManager::instance().color(Sem::Danger).name()));
  bodyRow->addWidget(badge, 0, Qt::AlignVCenter);

  auto* bodyFrame = new QFrame(dlg);
  bodyFrame->setObjectName("confirmBody");
  bodyFrame->setStyleSheet(QString("QFrame#confirmBody { border: 1px solid %1; border-radius: 8px; background: %2; }")
                               .arg(ThemeManager::instance().color(Sem::Border).name(), ThemeManager::instance().color(Sem::Surface).name()));
  auto* bodyLayout = new QVBoxLayout(bodyFrame);
  bodyLayout->setContentsMargins(12, 10, 12, 10);
  bodyLayout->setSpacing(0);

  auto* body = new QLabel(text, bodyFrame);
  body->setWordWrap(true);
  body->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  bodyLayout->addWidget(body);
  bodyRow->addWidget(bodyFrame, 1);
  layout->addLayout(bodyRow, 1);

  auto* btns = new QDialogButtonBox(dlg);
  auto* okBtn = btns->addButton(I18n::tr("Continue"), QDialogButtonBox::AcceptRole);
  okBtn->setObjectName("dangerBtn");
  auto* cancelBtn = btns->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  QObject::connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
  QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
  // 默认焦点在 Cancel 上：误按 Enter 不会触发危险动作。
  cancelBtn->setDefault(true);
  cancelBtn->setFocus();
  layout->addWidget(btns);

  const QFontMetrics titleFm(dlg->fontMetrics());
  const QFontMetrics bodyFm(body->fontMetrics());
  const QString measuredText = Qt::mightBeRichText(text) ? QTextDocumentFragment::fromHtml(text).toPlainText() : text;
  int widest = titleFm.horizontalAdvance(title);
  const QStringList lines = measuredText.split('\n');
  for (const auto& line : lines) widest = std::max(widest, bodyFm.horizontalAdvance(line));
  const int bodyWidth = std::clamp(widest, 320, 440);
  body->setFixedWidth(bodyWidth);
  dlg->setMinimumWidth(bodyWidth + 96);

  const bool accepted = dlg->exec() == QDialog::Accepted;
  delete dlg;
  return accepted;
}

bool confirmCommit(QWidget* parent, const QString& title, const QString& heading, const QString& target, const QString& detail, bool allowContinue) {
  return confirmCommit(parent, title, heading, target, detail, {}, allowContinue);
}

bool confirmCommit(QWidget* parent, const QString& title, const QString& heading, const QString& target, const QString& detail,
                   const QStringList& previewItems, bool allowContinue) {
  constexpr int kPreviewPageSize = 10;

  auto* dlg = new QDialog(parent);
  dlg->setWindowTitle(title);

  auto* layout = new QVBoxLayout(dlg);
  layout->setContentsMargins(20, 18, 20, 14);
  layout->setSpacing(12);

  // 操作标题：加粗，作为视觉重心——一句话说清"即将做什么"。
  auto* head = new QLabel(heading, dlg);
  head->setWordWrap(true);
  QFont headFont = head->font();
  headFont.setBold(true);
  head->setFont(headFont);
  layout->addWidget(head);

  // 目标：等宽字体 + 描边框，单独成块——一眼看清"动的是哪一个"。
  auto* targetLabel = new QLabel(target, dlg);
  targetLabel->setWordWrap(true);
  targetLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  QFont monoFont("Consolas");
  monoFont.setStyleHint(QFont::Monospace);
  targetLabel->setFont(monoFont);
  const QString mutedHex = ThemeManager::instance().color(Sem::FgMuted).name();
  targetLabel->setStyleSheet(QString("QLabel { border: 1px solid %1; border-radius: 4px; padding: 8px 10px; }").arg(mutedHex));
  layout->addWidget(targetLabel);

  // 范围说明：次要色弱化。allowContinue=false 时 detail 是"无法继续的原因"，
  // 改放进下方警示横幅醒目展示，这里不再重复渲染。
  if (!detail.isEmpty() && allowContinue) {
    auto* detailLabel = new QLabel(detail, dlg);
    detailLabel->setWordWrap(true);
    detailLabel->setStyleSheet(QString("color: %1;").arg(mutedHex));
    layout->addWidget(detailLabel);
  }

  if (allowContinue && !previewItems.isEmpty()) {
    auto* previewTitle = new QLabel(I18n::tr("Registry keys that will be deleted:"), dlg);
    previewTitle->setStyleSheet(QString("color: %1;").arg(mutedHex));
    layout->addWidget(previewTitle);

    auto* list = new QListWidget(dlg);
    list->setObjectName("commitPreviewList");
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QFont previewFont("Consolas");
    previewFont.setStyleHint(QFont::Monospace);
    list->setFont(previewFont);
    list->setStyleSheet(QString("QListWidget#commitPreviewList { border: 1px solid %1; border-radius: 4px; }"
                                "QListWidget#commitPreviewList::item { min-height: 20px; padding: 2px 8px; }")
                            .arg(mutedHex));
    layout->addWidget(list);
    QObject::connect(list, &QListWidget::customContextMenuRequested, dlg, [list](const QPoint& pos) {
      QListWidgetItem* item = list->itemAt(pos);
      if (!item) return;
      list->setCurrentItem(item);
      QMenu menu(list);
      QAction* copyAct = menu.addAction(I18n::tr("Copy current entry"));
      QObject::connect(copyAct, &QAction::triggered, list, [item] { QGuiApplication::clipboard()->setText(item->text()); });
      menu.exec(list->viewport()->mapToGlobal(pos));
    });

    auto* pageRow = new QHBoxLayout();
    auto* firstBtn = new QPushButton(QStringLiteral("«"), dlg);
    auto* prevBtn = new QPushButton(QStringLiteral("‹"), dlg);
    auto* pageLabel = new QLabel(dlg);
    pageLabel->setAlignment(Qt::AlignCenter);
    pageLabel->setMinimumWidth(220);
    auto* nextBtn = new QPushButton(QStringLiteral("›"), dlg);
    auto* lastBtn = new QPushButton(QStringLiteral("»"), dlg);
    for (auto* b : {firstBtn, prevBtn, nextBtn, lastBtn}) b->setMaximumWidth(36);
    pageRow->addStretch(1);
    pageRow->addWidget(firstBtn);
    pageRow->addWidget(prevBtn);
    pageRow->addWidget(pageLabel);
    pageRow->addWidget(nextBtn);
    pageRow->addWidget(lastBtn);
    pageRow->addStretch(1);
    layout->addLayout(pageRow);

    Pager pager;
    pager.pageSize = kPreviewPageSize;
    const int total = static_cast<int>(previewItems.size());
    const int displayPages = std::max(1, pager.pageCount(total));

    auto renderPreview = [list, pageLabel, firstBtn, prevBtn, nextBtn, lastBtn, &pager, &previewItems, total, displayPages]() {
      pager.clamp(total);
      list->clear();
      for (int i = pager.pageStart(); i < pager.pageEnd(total); ++i) {
        auto* item = new QListWidgetItem(previewItems.at(i));
        item->setToolTip(previewItems.at(i));
        list->addItem(item);
      }
      const int rowHeight = list->count() > 0 ? list->sizeHintForRow(0) : std::max(26, list->fontMetrics().height() + 8);
      const int visibleRows = std::min(pager.pageSize, total);
      list->setFixedHeight(rowHeight * visibleRows + list->frameWidth() * 2 + 2);
      pageLabel->setText(I18n::tr("Page %1 / %2 · %3 entries total").arg(pager.currentPage + 1).arg(displayPages).arg(total));
      firstBtn->setEnabled(pager.hasPrev());
      prevBtn->setEnabled(pager.hasPrev());
      nextBtn->setEnabled(pager.hasNext(total));
      lastBtn->setEnabled(pager.hasNext(total));
    };
    QObject::connect(firstBtn, &QPushButton::clicked, dlg, [&pager, renderPreview]() {
      pager.goFirst();
      renderPreview();
    });
    QObject::connect(prevBtn, &QPushButton::clicked, dlg, [&pager, renderPreview]() {
      pager.goPrev();
      renderPreview();
    });
    QObject::connect(nextBtn, &QPushButton::clicked, dlg, [&pager, total, renderPreview]() {
      pager.goNext(total);
      renderPreview();
    });
    QObject::connect(lastBtn, &QPushButton::clicked, dlg, [&pager, total, renderPreview]() {
      pager.goLast(total);
      renderPreview();
    });
    renderPreview();
  }

  // 警示横幅——复用全局状态横幅的 warn 样式（橙色）。可继续时提示"不可撤销"；
  // "继续"被置灰时改为展示原因（detail），让用户一眼看到为何不能继续。
  auto* warnBanner = new StatusBanner(dlg);
  warnBanner->setText(allowContinue ? I18n::tr("This action cannot be undone.") : detail);
  warnBanner->setObjectName("statusBanner");
  warnBanner->setProperty("level", "warn");
  warnBanner->setWordWrap(true);
  layout->addWidget(warnBanner);

  auto* btns = new QDialogButtonBox(dlg);
  auto* okBtn = btns->addButton(I18n::tr("Continue"), QDialogButtonBox::AcceptRole);
  // allowContinue 为 false：没有可执行的动作，"继续"置灰，用户只能取消。
  if (!allowContinue) okBtn->setEnabled(false);
  auto* cancelBtn = btns->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  QObject::connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
  QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
  // 默认焦点在 Cancel 上：误按 Enter 不会触发破坏性动作。
  cancelBtn->setDefault(true);
  cancelBtn->setFocus();
  layout->addWidget(btns);

  // 宽度跟随内容：取标题 / 目标 / 说明里最宽的一行，夹在 [420, 760]。
  const QFontMetrics headFm(headFont);
  const QFontMetrics monoFm(monoFont);
  int widest = std::max(headFm.horizontalAdvance(heading), monoFm.horizontalAdvance(target));
  if (!detail.isEmpty()) widest = std::max(widest, headFm.horizontalAdvance(detail));
  if (!previewItems.isEmpty()) widest = std::max(widest, 620);
  dlg->setMinimumWidth(std::clamp(widest + 64, 420, 760));

  const bool accepted = dlg->exec() == QDialog::Accepted;
  delete dlg;
  return accepted;
}

}  // namespace uwf::ui::dialogs
