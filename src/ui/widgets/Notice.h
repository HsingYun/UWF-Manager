// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QWidget>
namespace uwf::ui {
class IconView;
class TextLabel;
class Notice final : public QWidget {
  Q_OBJECT
 public:
  enum class Tone { Neutral, Warning, Error, Success };
  explicit Notice(QWidget* parent = nullptr);
  void setText(const QString& text);
  [[nodiscard]] QString text() const;
  void setHeading(const QString& text);
  void setTone(Tone tone);
  [[nodiscard]] Tone tone() const { return m_tone; }

 protected:
  void paintEvent(QPaintEvent* event) override;
  bool event(QEvent* event) override;
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  Tone m_tone = Tone::Neutral;
  IconView* m_icon;
  QWidget* m_iconHost;
  void alignIcon();
  TextLabel* m_heading;
  TextLabel* m_text;
};
}  // namespace uwf::ui
