// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QVBoxLayout>
#include <QPointer>
#include <QWidget>
#include <algorithm>

class QAbstractItemView;

namespace uwf::ui {

// A flex column uses spare height for expanding children (tables, editors),
// while retaining the complete wrapped height of its text. QScrollArea uses
// heightForWidth as a lower bound; returning QBoxLayout's preferred height
// there would reserve a table's sizeHint even when its rows can paginate.
class FlexibleColumnLayout final : public QVBoxLayout {
 public:
  explicit FlexibleColumnLayout(QWidget* parent) : QVBoxLayout(parent) {}
  // The records surface wraps a single item view; its spare row remainder
  // belongs to the adjacent details surface.
  void setPagedDetails(QWidget* records, QAbstractItemView* view, QWidget* details, int rowHeight);
  void setGeometry(const QRect& rect) override;
  [[nodiscard]] QSize minimumSize() const override;
  [[nodiscard]] int heightForWidth(const int width) const override {
    const auto margins = contentsMargins();
    const int available = std::max(0, width - margins.left() - margins.right());
    int height = margins.top() + margins.bottom();
    bool preceding = false;
    for (int index = 0; index < count(); ++index) {
      const auto* item = itemAt(index);
      if (item->widget() && item->isEmpty()) continue;
      if (!item->isEmpty()) {
        if (preceding) height += std::max(0, spacing());
        preceding = true;
      }
      const bool flexible = stretch(index) > 0 || item->expandingDirections().testFlag(Qt::Vertical);
      // A centered QWidgetItem can report unlimited width even when its widget has a fixed or maximum width.
      const int maximumWidth = item->widget() ? std::min(item->maximumSize().width(), item->widget()->maximumWidth()) : item->maximumSize().width();
      const int itemWidth = std::clamp(available, std::min(item->minimumSize().width(), maximumWidth), maximumWidth);
      const int preferred = item->hasHeightForWidth() ? (flexible ? item->minimumHeightForWidth(itemWidth) : item->heightForWidth(itemWidth))
                                                      : (flexible ? item->minimumSize().height() : item->sizeHint().height());
      const int minimum = item->widget() == m_records ? pagedRecordsMinimumHeight() : item->minimumSize().height();
      height += std::max(preferred, minimum);
    }
    return height;
  }
  [[nodiscard]] int minimumHeightForWidth(const int width) const override { return heightForWidth(width); }

 protected:
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  [[nodiscard]] int pagedRecordsMinimumHeight() const;
  QPointer<QWidget> m_records;
  QPointer<QWidget> m_details;
  QPointer<QAbstractItemView> m_view;
  int m_rowHeight = 0;
  bool m_distributingHeight = false;
};

}  // namespace uwf::ui
