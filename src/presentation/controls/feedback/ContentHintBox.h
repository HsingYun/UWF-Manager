// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Read-only text sized to its full document; its host owns scrolling.

#include <QTextBrowser>

namespace uwf::ui {

class ContentHintBox final : public QTextBrowser {
  Q_OBJECT
 public:
  explicit ContentHintBox(QWidget* parent = nullptr);
  [[nodiscard]] QSize sizeHint() const override;
  [[nodiscard]] QSize minimumSizeHint() const override;
  [[nodiscard]] int heightForWidth(int width) const override;
  void setLineHeight(qreal pixels);

 protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;

 private:
  void scheduleGeometryUpdate();
  void applyLineHeight();
  void applyLinkStyle();
  bool m_geometryUpdateQueued = false;
  bool m_formatting = false;
  qreal m_lineHeight = 0;
};

}  // namespace uwf::ui
