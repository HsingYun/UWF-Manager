// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// Operation feedback takes precedence over hover text, then baseline text.
// The borrowed target must expose a setText slot.

#include <QObject>
#include <QPointer>
#include <QString>

class QWidget;
class QTimer;

namespace uwf::ui {

class TransientLabel : public QObject {
  Q_OBJECT
 public:
  explicit TransientLabel(QWidget* label, QObject* parent = nullptr);

  // Updates the baseline without interrupting transient or hover text.
  void setBaseline(const QString& text);
  [[nodiscard]] const QString& baseline() const { return m_baseline; }

  // Displays feedback until restoreAfter() schedules or performs restoration.
  void show(const QString& text);

  // Replaces the restoration timer. Nonpositive delays also clear hover text.
  void restoreAfter(int delayMs);

  void flash(const QString& text, int msec);

  // Retains hover text without interrupting operation feedback or its timer.
  void showHover(const QString& text);

  [[nodiscard]] bool isShowing() const { return m_showing || !m_hover.isEmpty(); }

 private:
  void applyText(const QString& text);
  void render();

  QPointer<QWidget> m_label;
  QString m_baseline;
  QString m_transient;
  QString m_hover;
  QString m_current;  // Avoids repeated layout and document measurement for unchanged text.
  QTimer* m_timer;
  bool m_showing = false;
};

}  // namespace uwf::ui
