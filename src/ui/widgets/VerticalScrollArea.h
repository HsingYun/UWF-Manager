// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QScrollArea>

namespace uwf::ui {

// 纵向滚动容器保留内容的横向最小尺寸，不把完整内容高度传播为窗口下限。
// 字体、译文或横幅改变后重新报告尺寸，不缓存首次 polish 前的内容宽度。
class VerticalScrollArea final : public QScrollArea {
 public:
  explicit VerticalScrollArea(QWidget* parent = nullptr);
  void setWidget(QWidget* content);
  [[nodiscard]] QSize minimumSizeHint() const override;
  [[nodiscard]] QSize sizeHint() const override;

 protected:
  bool event(QEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  void scheduleScrollUpdate();
  void updateScrollPolicy();
  bool m_updatingScrollPolicy = false;
};

}  // namespace uwf::ui
