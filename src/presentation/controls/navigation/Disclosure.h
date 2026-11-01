// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QPointer>
#include <QWidget>

class QAbstractButton;
class QVBoxLayout;

namespace uwf::ui {
class Disclosure final : public QWidget {
  Q_OBJECT
 public:
  explicit Disclosure(const QString& title, QWidget* parent = nullptr);
  // Takes Qt parent ownership. Replacing content retires the previous child.
  void setContent(QWidget* content);
  void setOpen(bool open);
  void setCount(int count);
  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] QAbstractButton* toggleButton() const { return m_toggle; }

 signals:
  void openChanged(bool open);

 private:
  void syncContentLayout();
  QAbstractButton* m_toggle;
  QPointer<QWidget> m_content;
  QVBoxLayout* m_layout;
};
}  // namespace uwf::ui
