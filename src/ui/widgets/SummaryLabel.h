// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QStringList>

#include "TextLabel.h"

namespace uwf::ui {
// Parts are ordered by priority. Narrow layouts drop complete trailing parts;
// activation and accessibility always retain the complete summary.
class SummaryLabel final : public TextLabel {
  Q_OBJECT
 public:
  explicit SummaryLabel(const QStringList& parts, QWidget* parent = nullptr);
  void setParts(const QStringList& parts);
  [[nodiscard]] const QString& fullText() const { return m_fullText; }
  [[nodiscard]] QSize sizeHint() const override;
 signals:
  void activated(const QString& text);

 protected:
  void resizeEvent(QResizeEvent* event) override;
  void changeEvent(QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  void updateText();
  QStringList m_parts;
  QString m_fullText;
  bool m_pressed = false;
};
}  // namespace uwf::ui
