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
#include "StatusBanner.h"

#include <QPainter>
#include <QPen>

#include "ThemeManager.h"

namespace uwf::ui {

namespace {

struct BannerColors {
  QColor bg;
  QColor border;
};

// 与原 QSS 同值（alpha 用 0..255）：
//   warn：底 7% 琥珀，边 亮 40% / 暗 35% 琥珀；
//   error：底 亮 10% / 暗 15% 红，边 不透明红（#D93025 / #DC3545）。
// 半透明保留下来——抗锯齿绘制不会像 QSS 那样在锯齿角上叠出杂色，且底色能透出身后
// 任意背景，沿用原设计的复用性。
BannerColors colorsFor(const bool warn, const bool light) {
  if (warn) return light ? BannerColors{QColor(242, 153, 0, 18), QColor(242, 153, 0, 102)} : BannerColors{QColor(255, 180, 90, 18), QColor(255, 180, 90, 89)};
  return light ? BannerColors{QColor(217, 48, 37, 26), QColor(217, 48, 37)} : BannerColors{QColor(220, 53, 69, 38), QColor(220, 53, 69)};
}

}  // namespace

StatusBanner::StatusBanner(QWidget* parent) : QLabel(parent) {
  // 配色随主题变；切主题时重绘（GlobalStatusPanel 会整体重建，但 StatusPanel 等常驻
  // 实例不会，故这里自挂一条）。
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](Theme) { update(); });
}

void StatusBanner::paintEvent(QPaintEvent* ev) {
  const bool warn = property("level").toString() == QLatin1String("warn");
  const BannerColors c = colorsFor(warn, ThemeManager::instance().isLight());
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);
  // 内缩 0.5px 让 1px 边框完全落在控件内、不被裁掉半像素；圆角 6 对齐原 QSS。
  const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
  p.setPen(QPen(c.border, 1));
  p.setBrush(c.bg);
  p.drawRoundedRect(r, 6, 6);
  // 文字由基类按 QSS 的 color / padding 绘制，叠在底之上。
  QLabel::paintEvent(ev);
}

}  // namespace uwf::ui
