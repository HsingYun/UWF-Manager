// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

// 按操作反馈、悬停提示、基线文本的优先级统一管理显示与恢复。
// 目标须提供 setText 槽，所有权由调用方或 QObject 父子关系管理。

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

  // 临时提示期间仅更新基线；基线状态下同步更新目标文本。
  void setBaseline(const QString& text);
  [[nodiscard]] const QString& baseline() const { return m_baseline; }

  // 立即显示并取消恢复计时；须调用 restoreAfter 才会恢复基线。
  void show(const QString& text);

  // 定时结束操作反馈，恢复悬停提示或基线。多次调用以最后一次为准。
  // delayMs <= 0 清除全部临时文本并立即恢复基线。
  void restoreAfter(int delayMs);

  // 显示文本并安排定时恢复。
  void flash(const QString& text, int msec);

  // 悬停提示独立保存，操作反馈显示期间不会抢占反馈或缩短其时长。
  void showHover(const QString& text);

  // 当前是否仍显示临时文本。
  [[nodiscard]] bool isShowing() const { return m_showing || !m_hover.isEmpty(); }

 private:
  // 目标通过 setText 槽更新。
  void applyText(const QString& text);
  void render();

  QPointer<QWidget> m_label;
  QString m_baseline;
  QString m_transient;
  QString m_hover;
  QString m_current;  // 已显示文本，用于避免重复设置
  QTimer* m_timer;
  bool m_showing = false;
};

}  // namespace uwf::ui
