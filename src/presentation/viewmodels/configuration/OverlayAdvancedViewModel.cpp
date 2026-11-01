// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "presentation/viewmodels/configuration/OverlayAdvancedViewModel.h"

#include <utility>

#include "application/localization/OverlayTypeText.h"
#include "presentation/viewmodels/common/PresentationText.h"

namespace uwf::presentation {
namespace {
OverlayAdvancedViewModel::ValueState state(const std::optional<bool> value) {
  return !value ? OverlayAdvancedViewModel::ValueState::Unknown : *value ? OverlayAdvancedViewModel::ValueState::On : OverlayAdvancedViewModel::ValueState::Off;
}
}  // namespace
OverlayAdvancedViewModel::OverlayAdvancedViewModel(std::shared_ptr<app::OverlayPolicyEditor> editor, QObject* parent)
    : QObject(parent), m_editor(std::move(editor)) {
  connect(m_editor.get(), &app::OverlayPolicyEditor::stateChanged, this, &OverlayAdvancedViewModel::stateChanged);
  connect(m_editor.get(), &app::OverlayPolicyEditor::pendingChanged, this, &OverlayAdvancedViewModel::pendingChanged);
}
QString OverlayAdvancedViewModel::fieldName(const Field field) {
  switch (field) {
    case Field::Passthrough:
      return PresentationText::tr("Freespace Passthrough");
    case Field::Persistent:
      return PresentationText::tr("Persistent Overlay");
    case Field::Reset:
      return PresentationText::tr("Clear persistent data on next startup");
    case Field::ReadOnlyMedia:
      return PresentationText::tr("Read-Only Media mode");
  }
  return {};
}
QString OverlayAdvancedViewModel::officialName(const Field field) {
  switch (field) {
    case Field::Passthrough:
      return QStringLiteral("Freespace Passthrough");
    case Field::Persistent:
      return QStringLiteral("Persistent Overlay");
    case Field::Reset:
      return QStringLiteral("ResetPersistentState");
    case Field::ReadOnlyMedia:
      return QStringLiteral("Read-Only Media mode");
  }
  return {};
}
QString OverlayAdvancedViewModel::description(const Field field) {
  switch (field) {
    case Field::Passthrough:
      return PresentationText::tr("Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).");
    case Field::Persistent:
      return PresentationText::tr("Preserves writes in the Disk overlay across restarts; not available for RAM overlays.");
    case Field::Reset:
      return PresentationText::tr("Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled");
    case Field::ReadOnlyMedia:
      return PresentationText::tr(
          "Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; "
          "Full Commit is a separate global operation.");
  }
  return {};
}
OverlayAdvancedViewModel::FeatureDetails OverlayAdvancedViewModel::featureDetails(const Field field) {
  FeatureDetails details{officialName(field), description(field), {}};
  switch (field) {
    case Field::Passthrough:
      details.conditions = {
          {PresentationText::tr("Default behavior"),
           PresentationText::tr("In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.")},
          {PresentationText::tr("Change requirements"),
           PresentationText::tr(
               "UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.")},
          {PresentationText::tr("Takes effect"), PresentationText::tr("Will take effect at the next startup after applying.")}};
      break;
    case Field::Persistent:
      details.conditions = {
          {PresentationText::tr("Enable requirements"),
           PresentationText::tr("UWF must be disabled in the current session, and the next-session overlay must use Disk.")},
          {PresentationText::tr("Disable requirements"),
           PresentationText::tr(
               "UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.")}};
      break;
    case Field::Reset:
      details.conditions = {
          {PresentationText::tr("Usage requirements"), PresentationText::tr("UWF and Disk Persistent Overlay must be active in the current session. Clearing "
                                                                            "data also requires retaining Disk Persistent Overlay for the next session.")}};
      break;
    case Field::ReadOnlyMedia:
      details.conditions = {
          {PresentationText::tr("Enable requirements"),
           PresentationText::tr("UWF must be disabled in the current session, and the next-session overlay must use RAM.")},
          {PresentationText::tr("Disable requirements"),
           PresentationText::tr(
               "UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.")}};
      break;
  }
  return details;
}
bool OverlayAdvancedViewModel::changed(const Edit& edit, const Field field) const { return m_editor->changed(edit, field); }
OverlayAdvancedViewModel::ValueState OverlayAdvancedViewModel::currentState(const Field field) const { return state(m_editor->current(field)); }
OverlayAdvancedViewModel::ValueState OverlayAdvancedViewModel::currentFilterState() const {
  const auto& observed = m_editor->baseline().current.filterEnabled;
  return observed.status == app::PolicyObservationStatus::Available ? state(observed.value) : ValueState::Unknown;
}
QString OverlayAdvancedViewModel::currentText(const Field field) const {
  const auto current = currentState(field);
  return current == ValueState::Unknown ? PresentationText::tr("Unknown")
         : current == ValueState::On    ? PresentationText::tr("On")
                                        : PresentationText::tr("Off");
}
QString OverlayAdvancedViewModel::currentFilterText() const {
  const auto current = currentFilterState();
  return current == ValueState::Unknown ? PresentationText::tr("Unknown")
         : current == ValueState::On    ? PresentationText::tr("Enabled")
                                        : PresentationText::tr("Disabled");
}
QString OverlayAdvancedViewModel::currentTypeText() const {
  const auto& observed = m_editor->baseline().current.overlayType;
  return app::overlayTypeText(observed.status == app::PolicyObservationStatus::Available ? core::overlayTypeFromValue(observed.value)
                                                                                         : core::OverlayType::Unknown);
}
QString OverlayAdvancedViewModel::targetTypeText() const { return app::overlayTypeText(m_editor->targetType()); }
QString OverlayAdvancedViewModel::targetTypeText(const Edit& edit) const { return app::overlayTypeText(m_editor->targetType(edit)); }
QString OverlayAdvancedViewModel::issueText(const app::OverlayPolicyIssue issue) {
  using Issue = app::OverlayPolicyIssue;
  switch (issue) {
    case Issue::None:
      return {};
    case Issue::AdministratorRequired:
      return PresentationText::tr("Administrator privileges are required; the overlay configuration is read-only.");
    case Issue::HormActive:
      return PresentationText::tr("UWF settings cannot be changed while HORM is active.");
    case Issue::WritesBlocked:
      return PresentationText::tr("Configuration editing is temporarily unavailable. Wait for the current operation to finish.");
    case Issue::CurrentFilterMustBeOff:
      return PresentationText::tr("UWF is enabled in the current session. Disable UWF and restart before changing this setting.");
    case Issue::FlagsUnavailable:
      return PresentationText::tr("The overlay configuration could not be read. Refresh before editing.");
    case Issue::ResetUnsupported:
      return PresentationText::tr("This system does not support clearing persistent data at the next startup.");
    case Issue::ResetUnknown:
      return PresentationText::tr("The current clear request is unknown. Refresh the configuration before editing.");
    case Issue::CurrentPersistentRequired:
      return PresentationText::tr("Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.");
    case Issue::NextPersistentRequired:
      return PresentationText::tr(
          "Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.");
    case Issue::PersistentNeedsDisk:
      return PresentationText::tr("Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.");
    case Issue::MediaNeedsRam:
      return PresentationText::tr("Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.");
    case Issue::StaleEdit:
      return PresentationText::tr("The configuration has changed. Close and reopen this dialog before editing.");
  }
  return {};
}
QString OverlayAdvancedViewModel::disabledReason(const Edit& edit, const Field field) const {
  const auto issue = m_editor->disabledReason(edit, field);
  using Issue = app::OverlayPolicyIssue;
  if (issue == Issue::NextPersistentRequired)
    return field == Field::Persistent
               ? PresentationText::tr("A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.")
               : PresentationText::tr("The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.");
  if (issue == Issue::PersistentNeedsDisk) return PresentationText::tr("Persistent Overlay is only available for Disk; the next overlay is RAM.");
  if (issue == Issue::MediaNeedsRam) return PresentationText::tr("Enabling Read-Only Media mode requires RAM; the next overlay is Disk.");
  if (issue == Issue::FlagsUnavailable && field == Field::Reset)
    return PresentationText::tr("The current Persistent Overlay state could not be confirmed. Read the configuration again.");
  return issueText(issue);
}
QStringList OverlayAdvancedViewModel::conflicts(const Edit& edit) const {
  QStringList messages;
  for (const auto issue : m_editor->conflicts(edit)) messages.append(conflictText(issue));
  return messages;
}
QString OverlayAdvancedViewModel::conflictText(const app::OverlayPolicyIssue issue) {
  if (issue == app::OverlayPolicyIssue::CurrentFilterMustBeOff)
    return PresentationText::tr(
        "UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.");
  return issueText(issue);
}
}  // namespace uwf::presentation
#include "moc_OverlayAdvancedViewModel.cpp"
