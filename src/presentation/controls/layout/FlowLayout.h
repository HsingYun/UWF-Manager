// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QLayout>
#include <memory>
#include <vector>

namespace uwf::ui {

// Wraps intact control groups and reports their height through heightForWidth().
class FlowLayout final : public QLayout {
 public:
  explicit FlowLayout(QWidget* parent = nullptr);
  ~FlowLayout() override;

  void addItem(QLayoutItem* item) override;
  [[nodiscard]] int count() const override;
  [[nodiscard]] QLayoutItem* itemAt(int index) const override;
  QLayoutItem* takeAt(int index) override;
  [[nodiscard]] Qt::Orientations expandingDirections() const override;
  [[nodiscard]] bool hasHeightForWidth() const override;
  [[nodiscard]] int heightForWidth(int width) const override;
  [[nodiscard]] QSize minimumSize() const override;
  [[nodiscard]] QSize sizeHint() const override;
  void setGeometry(const QRect& rect) override;

 private:
  struct ItemPlacement {
    QRect geometry;
    qreal verticalOffset = 0;
  };
  struct Placement {
    std::vector<ItemPlacement> items;
    int height = 0;
  };
  [[nodiscard]] Placement arrange(const QRect& rect) const;
  std::vector<std::unique_ptr<QLayoutItem>> m_items;
};

}  // namespace uwf::ui
