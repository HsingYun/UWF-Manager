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
#include "PowerConfirmDialog.h"

#include <QColor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "I18n.h"
#include "ThemeManager.h"

namespace uwf::ui {

bool confirmPowerAction(QWidget* parent, const PowerAction action) {
  const bool shutdown = action == PowerAction::Shutdown;
  const QString title = shutdown ? I18n::tr("Safe shutdown") : I18n::tr("Safe restart");
  const QString heading = shutdown ? I18n::tr("Confirm safe shutdown?") : I18n::tr("Confirm safe restart?");
  const QString actionText = shutdown ? I18n::tr("Shut down") : I18n::tr("Restart");
  const QString summaryText =
      shutdown ? I18n::tr("The system will shut down safely through UWF.") : I18n::tr("The system will restart safely through UWF.");
  const QString iconPath = shutdown ? QStringLiteral(":/icons/shutdown.svg") : QStringLiteral(":/icons/restart.svg");

  auto* dlg = new QDialog(parent);
  dlg->setObjectName(QStringLiteral("powerConfirmDialog"));
  dlg->setWindowTitle(title);
  dlg->setWindowIcon(ThemeManager::instance().icon(iconPath));
  dlg->setMinimumWidth(500);

  auto* layout = new QVBoxLayout(dlg);
  layout->setContentsMargins(24, 22, 24, 16);
  layout->setSpacing(16);

  auto* header = new QHBoxLayout();
  header->setContentsMargins(0, 0, 0, 0);
  header->setSpacing(16);

  const auto& theme = ThemeManager::instance();
  const QColor actionColor = theme.color(shutdown ? Sem::Danger : Sem::Warn);
  auto* icon = new QLabel(dlg);
  icon->setObjectName(QStringLiteral("powerActionIcon"));
  icon->setAlignment(Qt::AlignCenter);
  icon->setFixedSize(52, 52);
  icon->setPixmap(ThemeManager::iconWithColor(iconPath, QColor(Qt::white)).pixmap(26, 26));
  icon->setStyleSheet(QStringLiteral("QLabel#powerActionIcon { background: %1; border-radius: 26px; }").arg(actionColor.name()));
  header->addWidget(icon, 0, Qt::AlignTop);

  auto* titles = new QVBoxLayout();
  titles->setContentsMargins(0, 1, 0, 0);
  titles->setSpacing(5);
  auto* headingLabel = new QLabel(heading, dlg);
  headingLabel->setObjectName(QStringLiteral("powerActionHeading"));
  QFont headingFont = headingLabel->font();
  headingFont.setPointSizeF(headingFont.pointSizeF() + 2.0);
  headingFont.setBold(true);
  headingLabel->setFont(headingFont);
  titles->addWidget(headingLabel);

  auto* summary = new QLabel(summaryText, dlg);
  summary->setWordWrap(true);
  summary->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
  titles->addWidget(summary);
  header->addLayout(titles, 1);
  layout->addLayout(header);

  auto* safetyCard = new QFrame(dlg);
  safetyCard->setObjectName(QStringLiteral("powerSafetyCard"));
  safetyCard->setStyleSheet(QStringLiteral("QFrame#powerSafetyCard { background: %1; border: 1px solid %2; border-radius: 9px; }")
                                .arg(theme.color(Sem::Surface).name(), theme.color(Sem::Border).name()));
  auto* safetyLayout = new QVBoxLayout(safetyCard);
  safetyLayout->setContentsMargins(14, 12, 14, 12);
  safetyLayout->setSpacing(4);
  auto* safetyHeading = new QLabel(I18n::tr("UWF protection"), safetyCard);
  QFont safetyFont = safetyHeading->font();
  safetyFont.setBold(true);
  safetyHeading->setFont(safetyFont);
  safetyLayout->addWidget(safetyHeading);
  auto* safetyDetail = new QLabel(I18n::tr("This operation remains available even if the UWF overlay is full."), safetyCard);
  safetyDetail->setWordWrap(true);
  safetyDetail->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::FgMuted).name()));
  safetyLayout->addWidget(safetyDetail);
  layout->addWidget(safetyCard);

  auto* warningCard = new QFrame(dlg);
  warningCard->setObjectName(QStringLiteral("powerWarningCard"));
  QColor warningBackground = theme.color(Sem::Danger);
  warningBackground.setAlpha(theme.isLight() ? 16 : 28);
  warningCard->setStyleSheet(
      QStringLiteral("QFrame#powerWarningCard { background: rgba(%1, %2, %3, %4); border: 1px solid %5; border-radius: 9px; }")
          .arg(warningBackground.red())
          .arg(warningBackground.green())
          .arg(warningBackground.blue())
          .arg(warningBackground.alpha())
          .arg(theme.color(Sem::Danger).name()));
  auto* warningLayout = new QVBoxLayout(warningCard);
  warningLayout->setContentsMargins(14, 11, 14, 11);
  warningLayout->setSpacing(4);
  auto* warningHeading = new QLabel(I18n::tr("Uncommitted changes will be lost"), warningCard);
  QFont warningFont = warningHeading->font();
  warningFont.setBold(true);
  warningHeading->setFont(warningFont);
  warningHeading->setStyleSheet(QStringLiteral("color: %1;").arg(theme.color(Sem::Danger).name()));
  warningLayout->addWidget(warningHeading);
  auto* warningDetail = new QLabel(I18n::tr("Before continuing, save your work and commit any changes that you want to keep permanently."), warningCard);
  warningDetail->setWordWrap(true);
  warningLayout->addWidget(warningDetail);
  layout->addWidget(warningCard);

  auto* buttons = new QDialogButtonBox(dlg);
  auto* actionButton = buttons->addButton(actionText, QDialogButtonBox::AcceptRole);
  actionButton->setObjectName(shutdown ? QStringLiteral("dangerBtn") : QStringLiteral("restartBtn"));
  auto* cancelButton = buttons->addButton(I18n::tr("Cancel"), QDialogButtonBox::RejectRole);
  QObject::connect(actionButton, &QPushButton::clicked, dlg, &QDialog::accept);
  QObject::connect(cancelButton, &QPushButton::clicked, dlg, &QDialog::reject);
  cancelButton->setDefault(true);
  cancelButton->setFocus();
  layout->addWidget(buttons);

  const bool accepted = dlg->exec() == QDialog::Accepted;
  delete dlg;
  return accepted;
}

}  // namespace uwf::ui
