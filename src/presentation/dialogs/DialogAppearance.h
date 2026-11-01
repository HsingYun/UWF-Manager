// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

class QDialog;
class QIcon;
class QPushButton;
class QString;
class QWidget;

namespace uwf::ui {
class Surface;
class IconView;
class TextLabel;

struct DialogHero {
  QWidget* widget;
  Surface* icon;
  TextLabel* heading;
  TextLabel* technical;
  IconView* glyph;
};

enum class Sem;
enum class DialogAction { Secondary, Primary, Destructive, DestructiveSecondary, Close };

void configureDialogWindow(QDialog* dialog);
void setDialogAction(QPushButton* button, DialogAction action);
void styleDialogIcon(Surface* surface, IconView* glyph, bool warning = false);
[[nodiscard]] DialogHero createDialogHero(QWidget* parent, const QIcon& icon, bool warning = false);
void updateDialogHeroText(const DialogHero& hero, const QString& heading, const QString& technical);

}  // namespace uwf::ui
