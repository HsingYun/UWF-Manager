/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// 目标 widget + 内嵌单次定时器：一个"基线文本"和若干次"临时覆盖"——临时覆盖到点
// 后自动回基线，新的覆盖到来时取消未到期的恢复。
//
// MainWindow 的状态栏（updatePendingSummary / showTransientHint）与右下角悬停
// 提示框（eventFilter 跟踪 hover）行为同构，过去用 5 个成员变量 + 散乱的
// QTimer::isActive() 检查拼出来，调用方得自己关心"是否处于 transient"。这里
// 把它收成单个对象，调用方只看 setBaseline / show / restoreAfter / flash 四个动作。
//
// 目标 widget 由外部持有（通常已塞进 layout 或 statusBar），可以是 QLabel 或
// QTextBrowser——本类只通过 setText 槽驱动它（QMetaObject::invokeMethod 调用，不
// 绑死具体类型）。QObject 父子关系按调用方习惯走——传 parent 进构造即可。

#include <QObject>
#include <QString>

class QWidget;
class QTimer;

namespace uwf::ui {

class TransientLabel : public QObject {
  Q_OBJECT
 public:
  explicit TransientLabel(QWidget* label, QObject* parent = nullptr);

  // 设新基线。当前正在 transient 时只更基线值，不立刻刷 label——避免覆盖用户
  // 还在看的临时提示；处于基线状态时同步刷 label。
  void setBaseline(const QString& text);
  [[nodiscard]] const QString& baseline() const { return m_baseline; }

  // 立刻显示这段文字，取消任何排队的恢复——不会自动回基线，要回得自己调
  // restoreAfter()。悬停"按住"语义用它（hover enter 时 show、hover leave 时
  // restoreAfter）。
  void show(const QString& text);

  // 启动定时器：到点后把 label 恢复成 baseline。多次调用以最后一次为准。
  // delayMs <= 0 立刻恢复。
  void restoreAfter(int delayMs);

  // 便捷：show(text) + restoreAfter(msec)。状态栏的"临时一闪"用它。
  void flash(const QString& text, int msec);

  // 是否处于 transient：show 过、且 restoreAfter 还没到点。调用方在某些场景
  // （比如收到新基线但想保留 transient）需要据此决定 setBaseline 后是否手动刷。
  // 一般用不上——setBaseline 已经按这个状态自动决定要不要刷 label。
  [[nodiscard]] bool isShowing() const { return m_showing; }

 private:
  // m_label 可能是 QLabel 或 QTextBrowser；两者都有 setText 槽，统一用 invokeMethod
  // 驱动，避免把本类绑死到某个具体控件类型。
  void applyText(const QString& text);

  QWidget* m_label;
  QString m_baseline;
  QString m_current;  // 上一次真正推给 label 的文本，用于 applyText 去重
  QTimer* m_timer;
  bool m_showing = false;
};

}  // namespace uwf::ui
