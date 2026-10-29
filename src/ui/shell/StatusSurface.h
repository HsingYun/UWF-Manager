// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include "ui/widgets/Surface.h"
class QLabel;
class QPushButton;
namespace uwf::ui {
class TextLabel;
class IconView;
class StatusSurface final : public Surface {
  Q_OBJECT
 public:
  explicit StatusSurface(QWidget* parent = nullptr);
  void setPending(std::size_t count);
  void setAccess(bool writable);
  void setCompatibility(bool compatible);
  void setDetail(const QString& detail);
  [[nodiscard]] const QString& detail() const { return m_detail; }
 public slots:
  void setText(const QString& text);
 signals:
  void copiedToClipboard();
  void discardRequested();

 private:
  IconView* m_icon;
  TextLabel* m_text;
  TextLabel* m_pending;
  TextLabel* m_access;
  TextLabel* m_compatibility;
  QPushButton* m_copy;
  QPushButton* m_discard;
  QWidget* m_pendingGroup;
  QString m_detail;
  std::size_t m_pendingCount = 0;
  bool m_writable = true;
  QString m_displayText;
  bool m_ready = true;
};
}  // namespace uwf::ui
