// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "ContentHintBox.h"

#include <QAbstractTextDocumentLayout>
#include <QMouseEvent>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <memory>

#include "ui/design/DesignSystem.h"

namespace uwf::ui {
namespace {
int documentHeightForWidth(const QTextDocument& document, const int width, const QTextOption::WrapMode wrapMode) {
  const std::unique_ptr<QTextDocument> measured(document.clone());
  measured->documentLayout()->setPaintDevice(document.documentLayout()->paintDevice());
  auto option = measured->defaultTextOption();
  option.setWrapMode(wrapMode);
  measured->setDefaultTextOption(option);
  measured->setTextWidth(std::max(1, width));
  return static_cast<int>(std::ceil(measured->size().height()));
}
}  // namespace

ContentHintBox::ContentHintBox(QWidget* parent) : QTextBrowser(parent) {
  setReadOnly(true);
  setCursorWidth(0);
  setMouseTracking(true);
  viewport()->setMouseTracking(true);
  viewport()->setCursor(Qt::ArrowCursor);
  setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  setFrameShape(QFrame::NoFrame);
  document()->setDocumentMargin(0);
  viewport()->setAutoFillBackground(false);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto policy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  policy.setHeightForWidth(true);
  setSizePolicy(policy);
  connect(this, &QTextEdit::textChanged, this, &ContentHintBox::scheduleGeometryUpdate);
  connect(this, &QTextEdit::textChanged, this, &ContentHintBox::applyLineHeight);
  connect(this, &QTextEdit::textChanged, this, &ContentHintBox::applyLinkStyle);
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, &ContentHintBox::applyLinkStyle);
  connect(document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, &ContentHintBox::scheduleGeometryUpdate);
}
void ContentHintBox::mouseMoveEvent(QMouseEvent* event) {
  QTextBrowser::mouseMoveEvent(event);
  const auto anchor = anchorAt(event->position().toPoint());
  QList<QTextEdit::ExtraSelection> selections;
  if (!anchor.isEmpty()) {
    auto cursor = cursorForPosition(event->position().toPoint());
    const auto block = cursor.block();
    for (auto it = block.begin(); !it.atEnd(); ++it) {
      const auto fragment = it.fragment();
      if (fragment.isValid() && fragment.charFormat().anchorHref() == anchor && cursor.position() >= fragment.position() &&
          cursor.position() <= fragment.position() + fragment.length()) {
        cursor.setPosition(fragment.position());
        cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
        QTextEdit::ExtraSelection selected;
        selected.cursor = cursor;
        selected.format.setFontUnderline(true);
        selections.append(selected);
        break;
      }
    }
  }
  setExtraSelections(selections);
}
void ContentHintBox::leaveEvent(QEvent* event) {
  setExtraSelections({});
  QTextBrowser::leaveEvent(event);
}
void ContentHintBox::applyLinkStyle() {
  if (m_formatting) return;
  const QScopedValueRollback formatting(m_formatting, true);
  const auto accent = DesignSystem::instance().color(design::Color::Accent);
  document()->setDefaultStyleSheet(QStringLiteral("a { color: %1; text-decoration: none; }").arg(accent.name()));
  for (auto block = document()->firstBlock(); block.isValid(); block = block.next()) {
    for (auto fragment = block.begin(); !fragment.atEnd(); ++fragment) {
      const auto text = fragment.fragment();
      if (!text.isValid() || !text.charFormat().isAnchor()) continue;
      auto format = text.charFormat();
      if (format.foreground().color() == accent && !format.fontUnderline()) continue;
      format.setForeground(accent);
      format.setFontUnderline(false);
      QTextCursor cursor(document());
      cursor.setPosition(text.position());
      cursor.setPosition(text.position() + text.length(), QTextCursor::KeepAnchor);
      cursor.setCharFormat(format);
    }
  }
  viewport()->update();
}
void ContentHintBox::setLineHeight(const qreal pixels) {
  m_lineHeight = pixels;
  applyLineHeight();
  scheduleGeometryUpdate();
}
void ContentHintBox::setMaximumContentHeight(const int height) {
  m_maximumContentHeight = std::max(0, height);
  updateHeightConstraints();
}
void ContentHintBox::applyLineHeight() {
  if (m_formatting || m_lineHeight <= 0) return;
  const QScopedValueRollback<bool> formatting(m_formatting, true);
  for (auto block = document()->firstBlock(); block.isValid(); block = block.next()) {
    auto format = block.blockFormat();
    if (format.lineHeightType() == QTextBlockFormat::FixedHeight && qFuzzyCompare(format.lineHeight(), m_lineHeight)) continue;
    format.setLineHeight(m_lineHeight, QTextBlockFormat::FixedHeight);
    QTextCursor(block).setBlockFormat(format);
  }
}

void ContentHintBox::scheduleGeometryUpdate() {
  if (m_geometryUpdateQueued) return;
  m_geometryUpdateQueued = true;
  QTimer::singleShot(0, this, [this] {
    m_geometryUpdateQueued = false;
    updateGeometry();
  });
}

void ContentHintBox::updateHeightConstraints() {
  setMaximumHeight(m_maximumContentHeight ? m_maximumContentHeight : QWIDGETSIZE_MAX);
  setVerticalScrollBarPolicy(m_maximumContentHeight ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
  scheduleGeometryUpdate();
}

int ContentHintBox::heightForWidth(const int width) const {
  const auto margins = contentsMargins();
  const int availableWidth = width - margins.left() - margins.right();
  int desiredHeight = documentHeightForWidth(*document(), availableWidth, wordWrapMode()) + margins.top() + margins.bottom();
  if (m_maximumContentHeight && desiredHeight > m_maximumContentHeight) desiredHeight = m_maximumContentHeight;
  return desiredHeight;
}

QSize ContentHintBox::sizeHint() const {
  const int width = QTextBrowser::sizeHint().width();
  return QSize(width, heightForWidth(width));
}

QSize ContentHintBox::minimumSizeHint() const { return {0, 0}; }

}  // namespace uwf::ui

#include "moc_ContentHintBox.cpp"
