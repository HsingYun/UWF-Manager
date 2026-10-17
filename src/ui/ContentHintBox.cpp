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
#include "ContentHintBox.h"

#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <memory>

namespace uwf::ui {

ContentHintBox::ContentHintBox(QWidget* parent) : QTextBrowser(parent) {
  setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  setFrameShape(QFrame::NoFrame);
  document()->setDocumentMargin(0);
  viewport()->setAutoFillBackground(false);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  connect(this, &QTextEdit::textChanged, this, &ContentHintBox::scheduleFit);
  connect(document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, &ContentHintBox::scheduleFit);
}

void ContentHintBox::resizeEvent(QResizeEvent* event) {
  QTextBrowser::resizeEvent(event);
  scheduleFit();
}

void ContentHintBox::scheduleFit() {
  if (m_fitQueued) return;
  m_fitQueued = true;
  QTimer::singleShot(0, this, &ContentHintBox::fitDocument);
}

void ContentHintBox::fitDocument() {
  m_fitQueued = false;
  if (viewport()->width() <= 0) return;
  const int desiredHeight = documentHeight(viewport()->width()) + height() - viewport()->height();
  if (minimumHeight() != desiredHeight || maximumHeight() != desiredHeight) setFixedHeight(desiredHeight);
}

QSize ContentHintBox::sizeHint() const {
  const int width = QTextBrowser::sizeHint().width();
  return QSize(width, documentHeight(width - (this->width() - viewport()->width())) + height() - viewport()->height());
}

int ContentHintBox::documentHeight(const int width) const {
  // 用独立排版测量，不能依赖当前视口高度：QTextEdit 在零高度视口中可能
  // 尚未布局文档，否则会把“未排版的零高度”反复写回，令文本区永远展不开。
  const std::unique_ptr<QTextDocument> measured(document()->clone());
  measured->documentLayout()->setPaintDevice(document()->documentLayout()->paintDevice());
  measured->setTextWidth(std::max(1, width));
  return static_cast<int>(std::ceil(measured->size().height()));
}

}  // namespace uwf::ui
