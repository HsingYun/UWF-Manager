// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>

class QAbstractButton;
class QVBoxLayout;

namespace uwf::ui {
class Disclosure final : public QWidget {
  Q_OBJECT
 public:
  explicit Disclosure(const QString& title, QWidget* parent = nullptr);
  void setContent(QWidget* content);
  void setOpen(bool open);
  void setCount(int count);
  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] QAbstractButton* toggleButton() const { return m_toggle; }

 signals:
  void openChanged(bool open);

 private:
  QAbstractButton* m_toggle;
  QWidget* m_content = nullptr;
  QVBoxLayout* m_layout;
};
}  // namespace uwf::ui
