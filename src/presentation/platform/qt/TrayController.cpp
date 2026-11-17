// SPDX-License-Identifier: GPL-3.0-or-later
#include "presentation/views/shell/TrayController.h"

namespace uwf::ui {
TrayController::TrayController(QWidget*, QObject* parent) : QObject(parent) {}
TrayController::~TrayController() = default;
void TrayController::applyUsageState(app::OverlayUsageState state) { m_usageState = std::move(state); }
void TrayController::setHubState(bool, bool) {}
void TrayController::setOperationAvailability(bool, bool) {}
bool TrayController::eventFilter(QObject*, QEvent*) { return false; }
bool TrayController::nativeEventFilter(const QByteArray&, void*, qintptr*) { return false; }
}  // namespace uwf::ui
