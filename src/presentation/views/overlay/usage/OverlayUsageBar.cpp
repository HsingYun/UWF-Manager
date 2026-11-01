// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/views/overlay/usage/OverlayUsageBar.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include "domain/uwf/model/UwfModel.h"
#include "presentation/appearance/ThemeManager.h"

namespace uwf::ui {

namespace {
constexpr int kBarH = 9;
}  // namespace

OverlayUsageBar::OverlayUsageBar(QWidget* parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](app::Theme) { update(); });
}

void OverlayUsageBar::setData(const uint32_t currentMb, const uint32_t warningMb, const uint32_t criticalMb, const uint64_t totalMb) {
  m_current = currentMb;
  m_warning = warningMb;
  m_critical = criticalMb;
  m_totalMb = totalMb;
  update();
}

QSize OverlayUsageBar::sizeHint() const { return {240, kBarH}; }
QSize OverlayUsageBar::minimumSizeHint() const { return {160, kBarH}; }

void OverlayUsageBar::paintEvent(QPaintEvent*) {
  QPainter p(this);
  // Floating-point geometry preserves subpixel antialiasing for the fill and stripes.
  p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  const auto& tm = ThemeManager::instance();
  const auto severity = core::overlayUsageSeverity(m_current, m_warning, m_critical);
  const QColor cUsed = tm.color(severity == core::OverlayUsageSeverity::Critical  ? Sem::Danger
                                : severity == core::OverlayUsageSeverity::Warning ? Sem::Warn
                                                                                  : Sem::Accent);
  const QColor cWarn = tm.color(Sem::Warn);
  const QColor cCrit = tm.color(Sem::Danger);
  const QColor cTotal = tm.color(Sem::BarBg);

  const QRectF bar(0, 2, width(), 5);
  QPainterPath path;
  path.addRoundedRect(bar, 2.5, 2.5);
  p.fillPath(path, cTotal);
  const auto total = std::max(m_totalMb, uint64_t{1});
  const auto position = [&](const uint32_t value) { return static_cast<qreal>(std::min<uint64_t>(value, total)) / static_cast<qreal>(total) * bar.width(); };
  p.save();
  p.setClipPath(path);
  if (m_current > 0) p.fillRect(QRectF(bar.left(), bar.top(), std::max(1.0, position(m_current)), bar.height()), cUsed);
  p.restore();
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(Qt::NoPen);
  for (const auto& [value, ink] : {std::pair{m_warning, cWarn}, std::pair{m_critical, cCrit}}) {
    if (value == 0 || value > m_totalMb) continue;
    p.setBrush(ink);
    p.drawRoundedRect(QRectF(std::clamp(position(value), 1.0, std::max(1.0, bar.width() - 1)), 0, 2, 9), 1, 1);
  }
}

}  // namespace uwf::ui

#include "moc_OverlayUsageBar.cpp"
