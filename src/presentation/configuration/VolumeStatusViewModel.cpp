// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "VolumeStatusViewModel.h"

#include "presentation/common/PresentationText.h"

namespace uwf::presentation {
VolumeStatusViewModel::VolumeStatusViewModel(QObject* parent) : VolumeStatusViewModel(std::make_shared<app::VolumeStatusEditor>(), parent) {}
VolumeStatusViewModel::VolumeStatusViewModel(std::shared_ptr<app::VolumeStatusEditor> editor, QObject* parent) : QObject(parent), m_editor(std::move(editor)) {
  connect(m_editor.get(), &app::VolumeStatusEditor::availabilityChanged, this, &VolumeStatusViewModel::availabilityChanged);
  connect(m_editor.get(), &app::VolumeStatusEditor::stateChanged, this, &VolumeStatusViewModel::stateChanged);
  connect(m_editor.get(), &app::VolumeStatusEditor::pendingChanged, this, &VolumeStatusViewModel::pendingChanged);
}
QString VolumeStatusViewModel::currentProtectionText() const {
  const auto current = m_editor->currentProtection();
  return current ? (*current ? PresentationText::tr("Protected") : PresentationText::tr("Unprotected")) : QStringLiteral("—");
}
}  // namespace uwf::presentation

#include "moc_VolumeStatusViewModel.cpp"
