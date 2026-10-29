// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CodeEditor.h"

#include <QAbstractTextDocumentLayout>
#include <QEvent>
#include <QMimeData>
#include <QPaintEvent>
#include <QPainter>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextLayout>

#include "ui/design/DesignSystem.h"
#include "ui/design/TextPainting.h"
namespace uwf::ui {
namespace {
class LineNumbers final : public QWidget {
 public:
  explicit LineNumbers(CodeEditor* editor) : QWidget(editor), m_editor(editor) {
    setObjectName(QStringLiteral("codeLineNumbers"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
  }

 protected:
  void paintEvent(QPaintEvent* event) override { m_editor->paintLineNumbers(event); }

 private:
  CodeEditor* m_editor;
};
}  // namespace
CodeEditor::CodeEditor(QWidget* parent) : QTextEdit(parent), m_numbers(new LineNumbers(this)) {
  setObjectName(QStringLiteral("codeEditor"));
  setProperty("designCodeEditor", true);
  setFrameShape(QFrame::NoFrame);
  // The outer Surface owns the rounded frame. A local rule also invalidates
  // QTextEdit's pre-construction stylesheet cache and removes native corners.
  setStyleSheet(QStringLiteral("QTextEdit { border: none; border-radius: 0; background: transparent; }"));
  setLineWrapMode(QTextEdit::NoWrap);
  setAcceptRichText(false);
  setLayoutDirection(Qt::LeftToRight);
  QFont type = font();
  type.setFamilies(DesignSystem::monospaceFamilies());
  type.setPixelSize(13);
  setFont(type);
  document()->setDefaultFont(type);
  setTabStopDistance(QFontMetricsF(type).horizontalAdvance(u' ') * 4);
  document()->setDocumentMargin(0);
  resetLineFormat();
  connect(document(), &QTextDocument::contentsChange, this, [this](const int position, int, const int added) { normalizeLineFormats(position, added); });
  connect(document(), &QTextDocument::blockCountChanged, this, [this] { updateGutter(); });
  connect(this, &QTextEdit::textChanged, m_numbers, qOverload<>(&QWidget::update));
  connect(verticalScrollBar(), &QScrollBar::valueChanged, m_numbers, qOverload<>(&QWidget::update));
  connect(&DesignSystem::instance(), &DesignSystem::changed, this, [this] { m_numbers->update(); });
  updateGutter();
}
int CodeEditor::blockCount() const { return document()->blockCount(); }
void CodeEditor::setLineNumbersVisible(const bool visible) {
  m_lineNumbersVisible = visible;
  m_numbers->setVisible(visible);
  updateGutter();
}
void CodeEditor::setLineHeight(const qreal pixels) {
  m_lineHeight = std::max(qreal{1}, pixels);
  resetLineFormat();
}
void CodeEditor::setContentMargins(const QMargins& margins) {
  m_contentMargins = margins;
  updateGutter();
}
void CodeEditor::setPlaceholderText(const QString& text) {
  if (m_placeholder == text) return;
  m_placeholder = text;
  QTextEdit::setPlaceholderText({});
  viewport()->update();
  emit placeholderTextChanged(text);
}
void CodeEditor::paintEvent(QPaintEvent* event) {
  QTextEdit::paintEvent(event);
  if (!document()->isEmpty() || m_placeholder.isEmpty()) return;
  const auto* layout = document()->firstBlock().layout();
  if (!layout || layout->lineCount() == 0 || !layout->preeditAreaText().isEmpty()) return;
  const auto line = layout->lineAt(0);
  QPainter painter(viewport());
  painter.setFont(font());
  painter.setPen(DesignSystem::instance().color(design::Color::Faint));
  painter.drawText(QPointF(0, layout->position().y() + line.y() + line.ascent() - verticalScrollBar()->value()), m_placeholder);
}
void CodeEditor::resetLineFormat() {
  const QScopedValueRollback formatting(m_formatting, true);
  QTextCursor cursor(document());
  cursor.select(QTextCursor::Document);
  QTextBlockFormat format;
  format.setLineHeight(m_lineHeight, QTextBlockFormat::FixedHeight);
  // Qt's rich-text control resets a blank paragraph on Return and consumes
  // that key when its format differs from the default. Code blocks explicitly
  // opt out of that word-processor behavior, preserving empty code lines.
  format.setProperty(QTextFormat::BlockCodeLanguage, QStringLiteral("plain"));
  cursor.setBlockFormat(format);
}
void CodeEditor::normalizeLineFormats(const int position, const int added) {
  if (m_formatting) return;
  const QScopedValueRollback formatting(m_formatting, true);
  auto block = document()->findBlock(std::max(0, position - 1));
  const int end = position + added + 1;
  QTextCursor edit(document());
  edit.joinPreviousEditBlock();
  while (block.isValid() && block.position() <= end) {
    auto format = block.blockFormat();
    if (format.lineHeightType() != QTextBlockFormat::FixedHeight || !qFuzzyCompare(format.lineHeight(), m_lineHeight) ||
        !format.hasProperty(QTextFormat::BlockCodeLanguage)) {
      format.setLineHeight(m_lineHeight, QTextBlockFormat::FixedHeight);
      format.setProperty(QTextFormat::BlockCodeLanguage, QStringLiteral("plain"));
      edit.setPosition(block.position());
      edit.setBlockFormat(format);
    }
    block = block.next();
  }
  edit.endEditBlock();
  m_numbers->update();
}
void CodeEditor::setPlainText(const QString& text) {
  QTextEdit::setPlainText(text);
  resetLineFormat();
  document()->clearUndoRedoStacks();
}
void CodeEditor::clear() {
  QTextEdit::clear();
  resetLineFormat();
  document()->clearUndoRedoStacks();
}
void CodeEditor::insertFromMimeData(const QMimeData* source) {
  if (source->hasText()) insertPlainText(source->text());
}
void CodeEditor::updateGutter() {
  const int width =
      m_contentMargins.left() + (m_lineNumbersVisible ? 15 + QFontMetrics(font()).horizontalAdvance(QString::number(std::max(1, blockCount()))) : 0);
  setViewportMargins(width, m_contentMargins.top(), m_contentMargins.right(), m_contentMargins.bottom());
  m_numbers->setGeometry(0, m_contentMargins.top(), width, std::max(0, height() - m_contentMargins.top() - m_contentMargins.bottom()));
  m_numbers->update();
}
void CodeEditor::changeEvent(QEvent* event) {
  QTextEdit::changeEvent(event);
  if (event->type() == QEvent::FontChange) updateGutter();
}
void CodeEditor::resizeEvent(QResizeEvent* event) {
  QTextEdit::resizeEvent(event);
  updateGutter();
}
void CodeEditor::paintLineNumbers(QPaintEvent* event) {
  QPainter painter(m_numbers);
  const auto& system = DesignSystem::instance();
  painter.fillRect(event->rect(), system.color(design::Color::Surface));
  auto type = font();
  painter.setFont(type);
  painter.setPen(system.color(design::Color::Faint));
  auto block = cursorForPosition(QPoint(0, 0)).block();
  int number = block.blockNumber();
  while (block.isValid()) {
    const auto blockBounds = document()->documentLayout()->blockBoundingRect(block);
    const qreal top = blockBounds.top() - verticalScrollBar()->value();
    if (top > event->rect().bottom()) break;
    const auto height = blockBounds.height();
    if (block.isVisible() && top + height >= event->rect().top() && block.layout()->lineCount() > 0) {
      const auto line = block.layout()->lineAt(0);
      const QString label = QString::number(number + 1);
      // blockBoundingRect already contains the line's vertical adjustment.
      // QTextLayout's origin is the coordinate space used to draw the text.
      const qreal baseline = block.layout()->position().y() + line.y() + line.ascent() - verticalScrollBar()->value() + viewport()->y() - m_numbers->y();
      painter.drawText(QPointF(m_numbers->width() - 15 - QFontMetricsF(type).horizontalAdvance(label), baseline), label);
    }
    block = block.next();
    ++number;
  }
}
}  // namespace uwf::ui
#include "moc_CodeEditor.cpp"
