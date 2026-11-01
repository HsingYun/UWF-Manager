// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/commands/CommandOutputDialog.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QFrame>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include "presentation/appearance/I18n.h"
#include "presentation/controls/layout/Surface.h"
#include "presentation/design/theme/DesignSystem.h"
#include "presentation/dialogs/DialogLayout.h"

namespace uwf::ui {

CommandOutputDialog::CommandOutputDialog(const QString& title, const QString& text, QWidget* parent) : PanelDialog(parent) {
  setObjectName(QStringLiteral("commandOutputDialog"));
  setWindowTitle(title);
  constexpr QSize initialSize{960, 680};
  constexpr QSize minimumSize{480, 320};
  auto* frame = new DialogLayout(this, DialogContent::Form, initialSize, minimumSize);
  auto* surface = new Surface(design::Color::Surface, this);
  surface->setBorder(Qt::LeftEdge | Qt::TopEdge | Qt::RightEdge | Qt::BottomEdge, 6);
  surface->setClipContent(true);
  auto* content = new QVBoxLayout(surface);
  content->setContentsMargins(1, 1, 1, 1);

  auto* output = new QPlainTextEdit(surface);
  output->setObjectName(QStringLiteral("commandOutputText"));
  output->setProperty("accessibleName", title);
  output->setFrameShape(QFrame::NoFrame);
  output->setReadOnly(true);
  output->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  output->setLineWrapMode(QPlainTextEdit::NoWrap);
  output->setLayoutDirection(Qt::LeftToRight);
  output->setMaximumBlockCount(0);
  auto type = output->font();
  type.setStyleHint(QFont::Monospace);
  type.setFamilies(DesignSystem::monospaceFamilies());
  output->setFont(type);
  output->setPlainText(text);
  content->addWidget(output);
  frame->body()->addWidget(surface, 1);

  auto* actions = new QDialogButtonBox(this);
  actions->addButton(I18n::tr("Close"), QDialogButtonBox::RejectRole);
  connect(actions, &QDialogButtonBox::rejected, this, &QDialog::reject);
  frame->setActions(actions);
  output->setFocus();
}

}  // namespace uwf::ui
#include "moc_CommandOutputDialog.cpp"
