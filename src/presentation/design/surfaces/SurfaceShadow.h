// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <QMargins>

class QPainter;
class QRectF;
class QWidget;
namespace uwf::ui {
enum class ShadowKind { Window, Popup, Hud };
[[nodiscard]] QMargins surfaceShadowMargins(ShadowKind kind);
// The source is an alpha-only rounded mask. No widget/effect snapshot is used.
void paintSurfaceShadow(QPainter& painter, const QRectF& surface, ShadowKind kind);
void installSurfaceShadow(QWidget* owner, ShadowKind kind);
}  // namespace uwf::ui
