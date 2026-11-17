// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QWidget>

namespace uwf::ui {
class IconView;
class TextLabel;
class AlignedGraphic;
class Notice final : public QWidget {
  Q_OBJECT
 public:
  enum class Tone { Neutral, Warning, Error, Success, Accent };
  explicit Notice(QWidget* parent = nullptr);
  void setText(const QString& text);
  [[nodiscard]] QString text() const;
  void setHeading(const QString& text);
  void setTone(Tone tone);
  void setFlat(bool flat);
  void setIcon(const QIcon& icon);
  [[nodiscard]] Tone tone() const { return m_tone; }

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  Tone m_tone = Tone::Neutral;
  bool m_flat = false;
  IconView* m_icon;
  AlignedGraphic* m_iconHost;
  TextLabel* m_heading;
  TextLabel* m_text;
};
}  // namespace uwf::ui
