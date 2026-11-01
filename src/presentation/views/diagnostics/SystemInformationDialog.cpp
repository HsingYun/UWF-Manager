// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/diagnostics/SystemInformationDialog.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextDocument>
#include <QVBoxLayout>
#include <utility>

#include "presentation/appearance/I18n.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/controls/clipboard/ClipboardText.h"
#include "presentation/controls/feedback/ContentHintBox.h"
#include "presentation/controls/feedback/Notice.h"
#include "presentation/controls/inputs/CodeEditor.h"
#include "presentation/controls/layout/AlignedGraphic.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/controls/navigation/TabView.h"
#include "presentation/controls/text/IconView.h"
#include "presentation/controls/text/TextLabel.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogLayout.h"

namespace uwf::ui {

SystemInformationDialog::SystemInformationDialog(presentation::DiagnosticReportViewModel model, QWidget* parent) : InformationDialog(parent) {
  setObjectName(QStringLiteral("systemInformationDialog"));
  setWindowTitle(I18n::tr("System information"));
  setDescription(I18n::tr("A diagnostic snapshot of the system, privileges, and UWF."));
  auto* frame = new DialogLayout(this, DialogContent::Scrollable, QSize(1096, 754), QSize(720, 500));
  auto* tabs = new TabView(this);
  tabs->setMeasureCurrentPage(true);
  tabs->setObjectName(QStringLiteral("systemInformationTabs"));
  const auto privacyNote = [](QWidget* parent) {
    auto* host = new QWidget(parent);
    auto* row = new QHBoxLayout(host);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(9);
    auto* icon = new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/info.svg")), 17, design::Color::Muted, host);
    auto* text = new TextLabel(I18n::tr("This report may contain account names, device identifiers, and file paths. Review it before sharing."),
                               TextStyle::Caption, host);
    text->setWordWrap(true);
    text->setColorRole(design::Color::Muted);
    row->addWidget(new AlignedGraphic(icon, text, host));
    row->addWidget(text, 1);
    return host;
  };
  auto* overview = new QWidget(tabs);
  auto* summary = new QVBoxLayout(overview);
  summary->setContentsMargins(0, 22, 0, 0);
  summary->setSpacing(16);
  summary->addWidget(privacyNote(overview));
  if (model.partial()) {
    auto* warning = new Notice(overview);
    warning->setTone(Notice::Tone::Warning);
    warning->setText(I18n::tr("Some hardware information could not be read. Other diagnostic information remains available to view and copy."));
    summary->addWidget(warning);
  }
  auto* cards = new QGridLayout();
  cards->setContentsMargins(0, 0, 0, 0);
  cards->setSpacing(16);
  int index = 0;
  for (const auto& section : model.sections()) {
    auto* card = new Surface(design::Color::Surface, overview);
    card->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
    auto* content = new QVBoxLayout(card);
    content->setContentsMargins(19, 17, 19, 17);
    content->setSpacing(14);
    auto* heading = new QHBoxLayout();
    heading->setSpacing(9);
    heading->addWidget(new IconView(ThemeManager::instance().icon(section.icon), 17, design::Color::Muted, card));
    heading->addWidget(new TextLabel(section.title, TextStyle::Section, card), 1);
    content->addLayout(heading);
    auto* fields = new QGridLayout();
    fields->setHorizontalSpacing(12);
    fields->setVerticalSpacing(10);
    int row = 0;
    for (const auto& field : section.fields) {
      auto* label = new TextLabel(field.label, TextStyle::Caption, card);
      label->setColorRole(design::Color::Muted);
      label->setWordWrap(true);
      label->setFixedWidth(88);
      auto* value = new ContentHintBox(card);
      auto font = value->font();
      font.setPixelSize(12);
      value->setFont(font);
      value->setLineHeight(19.2);
      value->setTextInteractionFlags(Qt::TextSelectableByMouse);
      value->setFocusPolicy(Qt::ClickFocus);
      value->setPlainText(field.value);
      fields->addWidget(label, row, 0, Qt::AlignTop);
      fields->addWidget(value, row, 1, Qt::AlignTop);
      ++row;
    }
    fields->setColumnStretch(1, 1);
    content->addLayout(fields);
    cards->addWidget(card, index / 2, index % 2, Qt::AlignTop);
    ++index;
  }
  cards->setColumnStretch(0, 1);
  cards->setColumnStretch(1, 1);
  summary->addLayout(cards);
  summary->addStretch();
  tabs->addTab(overview, I18n::tr("Overview"));
  auto* raw = new QWidget(tabs);
  auto* rawLayout = new QVBoxLayout(raw);
  rawLayout->setContentsMargins(0, 22, 0, 0);
  rawLayout->setSpacing(16);
  rawLayout->addWidget(privacyNote(raw));
  auto* surface = new Surface(design::Color::Surface, raw);
  surface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  surface->setClipContent(true);
  auto* document = new QVBoxLayout(surface);
  document->setContentsMargins(1, 1, 1, 1);
  auto* editor = new CodeEditor(surface);
  editor->setLayoutDirection(Qt::LeftToRight);
  editor->setObjectName(QStringLiteral("systemInformationReport"));
  editor->setProperty("accessibleName", I18n::tr("Raw report"));
  editor->setReadOnly(true);
  editor->setLineNumbersVisible(false);
  editor->setLineHeight(23.4);
  editor->setContentMargins(QMargins(18, 14, 18, 14));
  auto reportFont = font();
  reportFont.setFamilies(DesignSystem::monospaceFamilies());
  reportFont.setPixelSize(12);
  editor->setFont(reportFont);
  editor->setPlainText(model.report());
  editor->moveCursor(QTextCursor::Start);
  document->addWidget(editor);
  rawLayout->addWidget(surface, 1);
  tabs->addTab(raw, I18n::tr("Raw report"));
  if (model.sections().isEmpty()) {
    tabs->setTabEnabled(0, false);
    tabs->setCurrentIndex(1);
  }
  frame->body()->addWidget(tabs, 1);
  auto* leading = new QDialogButtonBox(this);
  auto* copy = leading->addButton(I18n::tr("Copy full report"), QDialogButtonBox::ActionRole);
  copy->setIcon(ThemeManager::instance().icon(QStringLiteral(":/icons/copy.svg")));
  copy->setObjectName(QStringLiteral("systemInformationCopyButton"));
  connect(copy, &QPushButton::clicked, this, [text = model.report()] { static_cast<void>(writeClipboardText(text)); });
  auto* readOnly = new QWidget(this);
  auto* readOnlyLayout = new QHBoxLayout(readOnly);
  readOnlyLayout->setContentsMargins(0, 0, 0, 0);
  readOnlyLayout->setSpacing(8);
  readOnlyLayout->addWidget(new IconView(ThemeManager::instance().icon(QStringLiteral(":/icons/lock.svg")), 16, design::Color::Muted, readOnly));
  auto* readOnlyText = new TextLabel(I18n::tr("Read only"), TextStyle::Caption, readOnly);
  readOnlyText->setColorRole(design::Color::Muted);
  readOnlyLayout->addWidget(readOnlyText);
  frame->setActions(nullptr, leading, readOnly);
}
}  // namespace uwf::ui
#include "moc_SystemInformationDialog.cpp"
