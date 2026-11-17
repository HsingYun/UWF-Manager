// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "support/uwf/UwfOperationCatalog.h"

#include <algorithm>
#include <array>
#include <type_traits>

namespace uwf::test::solver_catalog {
using namespace compat;
namespace {
constexpr std::array<OperationDescriptor, 62> operations{
    {{OperationSlot::GetBooleanFilterEnabled, RequestFamily::GetBoolean, "GetBoolean.FilterEnabled", BooleanProperty::FilterEnabled,
      OperationScope::SelectedSession, ParameterAxis::Session},
     {OperationSlot::GetBooleanShutdownPending, RequestFamily::GetBoolean, "GetBoolean.ShutdownPending", BooleanProperty::ShutdownPending,
      OperationScope::RuntimeGlobal, ParameterAxis::Session},
     {OperationSlot::GetBooleanHormEnabled, RequestFamily::GetBoolean, "GetBoolean.HormEnabled", BooleanProperty::HormEnabled, OperationScope::RuntimeGlobal,
      ParameterAxis::Session},
     {OperationSlot::GetBooleanServicingEnabled, RequestFamily::GetBoolean, "GetBoolean.ServicingEnabled", BooleanProperty::ServicingEnabled,
      OperationScope::SelectedSession, ParameterAxis::Session},
     {OperationSlot::GetBooleanPersistDomainSecretKey, RequestFamily::GetBoolean, "GetBoolean.PersistDomainSecretKey", BooleanProperty::PersistDomainSecretKey,
      OperationScope::SelectedSession, ParameterAxis::Session},
     {OperationSlot::GetBooleanPersistTscal, RequestFamily::GetBoolean, "GetBoolean.PersistTscal", BooleanProperty::PersistTscal,
      OperationScope::SelectedSession, ParameterAxis::Session},
     {OperationSlot::GetBooleanOverlayCommit, RequestFamily::GetBoolean, "GetBoolean.OverlayCommit", BooleanProperty::OverlayCommit,
      OperationScope::RuntimeGlobal, ParameterAxis::Session},
     {OperationSlot::GetBooleanRegistryChangesAlwaysPersisted, RequestFamily::GetBoolean, "GetBoolean.RegistryChangesAlwaysPersisted",
      BooleanProperty::RegistryChangesAlwaysPersisted, OperationScope::RuntimeGlobal, ParameterAxis::Session},
     {OperationSlot::SetBooleanFilterEnabled, RequestFamily::SetBoolean, "SetBoolean.FilterEnabled", BooleanProperty::FilterEnabled,
      OperationScope::NextConfiguration, ParameterAxis::Boolean},
     {OperationSlot::SetBooleanShutdownPending, RequestFamily::SetBoolean, "SetBoolean.ShutdownPending", BooleanProperty::ShutdownPending,
      OperationScope::RuntimeGlobal, ParameterAxis::Boolean},
     {OperationSlot::SetBooleanHormEnabled, RequestFamily::SetBoolean, "SetBoolean.HormEnabled", BooleanProperty::HormEnabled, OperationScope::RuntimeGlobal,
      ParameterAxis::Boolean},
     {OperationSlot::SetBooleanServicingEnabled, RequestFamily::SetBoolean, "SetBoolean.ServicingEnabled", BooleanProperty::ServicingEnabled,
      OperationScope::NextConfiguration, ParameterAxis::Boolean},
     {OperationSlot::SetBooleanPersistDomainSecretKey, RequestFamily::SetBoolean, "SetBoolean.PersistDomainSecretKey", BooleanProperty::PersistDomainSecretKey,
      OperationScope::NextConfiguration, ParameterAxis::Boolean},
     {OperationSlot::SetBooleanPersistTscal, RequestFamily::SetBoolean, "SetBoolean.PersistTscal", BooleanProperty::PersistTscal,
      OperationScope::NextConfiguration, ParameterAxis::Boolean},
     {OperationSlot::SetBooleanOverlayCommit, RequestFamily::SetBoolean, "SetBoolean.OverlayCommit", BooleanProperty::OverlayCommit,
      OperationScope::RuntimeGlobal, ParameterAxis::Boolean},
     {OperationSlot::SetBooleanRegistryChangesAlwaysPersisted, RequestFamily::SetBoolean, "SetBoolean.RegistryChangesAlwaysPersisted",
      BooleanProperty::RegistryChangesAlwaysPersisted, OperationScope::RuntimeGlobal, ParameterAxis::Boolean},
     {OperationSlot::GetNumberOverlayType, RequestFamily::GetNumber, "GetNumber.OverlayType", NumberProperty::OverlayType, OperationScope::SelectedSession,
      ParameterAxis::Session},
     {OperationSlot::GetNumberMaximumOverlaySize, RequestFamily::GetNumber, "GetNumber.MaximumOverlaySize", NumberProperty::MaximumOverlaySize,
      OperationScope::SelectedSession, ParameterAxis::Session},
     {OperationSlot::GetNumberWarningThreshold, RequestFamily::GetNumber, "GetNumber.WarningThreshold", NumberProperty::WarningThreshold,
      OperationScope::RuntimeGlobal, ParameterAxis::Session},
     {OperationSlot::GetNumberCriticalThreshold, RequestFamily::GetNumber, "GetNumber.CriticalThreshold", NumberProperty::CriticalThreshold,
      OperationScope::RuntimeGlobal, ParameterAxis::Session},
     {OperationSlot::GetNumberOverlayFlags, RequestFamily::GetNumber, "GetNumber.OverlayFlags", NumberProperty::OverlayFlags, OperationScope::SelectedSession,
      ParameterAxis::Session},
     {OperationSlot::GetNumberResetPersistentState, RequestFamily::GetNumber, "GetNumber.ResetPersistentState", NumberProperty::ResetPersistentState,
      OperationScope::RuntimeGlobal, ParameterAxis::Session},
     {OperationSlot::GetNumberResetPersistentStateSavedMode, RequestFamily::GetNumber, "GetNumber.ResetPersistentStateSavedMode",
      NumberProperty::ResetPersistentStateSavedMode, OperationScope::SelectedSession, ParameterAxis::Session},
     {OperationSlot::SetNumberOverlayType, RequestFamily::SetNumber, "SetNumber.OverlayType", NumberProperty::OverlayType, OperationScope::NextConfiguration,
      ParameterAxis::Number},
     {OperationSlot::SetNumberMaximumOverlaySize, RequestFamily::SetNumber, "SetNumber.MaximumOverlaySize", NumberProperty::MaximumOverlaySize,
      OperationScope::NextConfiguration, ParameterAxis::Number},
     {OperationSlot::SetNumberWarningThreshold, RequestFamily::SetNumber, "SetNumber.WarningThreshold", NumberProperty::WarningThreshold,
      OperationScope::RuntimeGlobal, ParameterAxis::Number},
     {OperationSlot::SetNumberCriticalThreshold, RequestFamily::SetNumber, "SetNumber.CriticalThreshold", NumberProperty::CriticalThreshold,
      OperationScope::RuntimeGlobal, ParameterAxis::Number},
     {OperationSlot::SetNumberOverlayFlags, RequestFamily::SetNumber, "SetNumber.OverlayFlags", NumberProperty::OverlayFlags, OperationScope::NextConfiguration,
      ParameterAxis::Number},
     {OperationSlot::SetNumberResetPersistentState, RequestFamily::SetNumber, "SetNumber.ResetPersistentState", NumberProperty::ResetPersistentState,
      OperationScope::RuntimeGlobal, ParameterAxis::Number},
     {OperationSlot::SetNumberResetPersistentStateSavedMode, RequestFamily::SetNumber, "SetNumber.ResetPersistentStateSavedMode",
      NumberProperty::ResetPersistentStateSavedMode, OperationScope::NextConfiguration, ParameterAxis::Number},
     {OperationSlot::SetOverlayOptionPassthrough, RequestFamily::SetOverlayOption, "SetOverlayOption.Passthrough", OverlayOption::Passthrough,
      OperationScope::NextConfiguration, ParameterAxis::Boolean},
     {OperationSlot::SetOverlayOptionPersistent, RequestFamily::SetOverlayOption, "SetOverlayOption.Persistent", OverlayOption::Persistent,
      OperationScope::NextConfiguration, ParameterAxis::Boolean},
     {OperationSlot::SetOverlayOptionReadOnlyMedia, RequestFamily::SetOverlayOption, "SetOverlayOption.ReadOnlyMedia", OverlayOption::ReadOnlyMedia,
      OperationScope::NextConfiguration, ParameterAxis::Boolean},
     {OperationSlot::GetOverlayUsage, RequestFamily::GetOverlayUsage, "GetOverlayUsage", std::monostate{}, OperationScope::CurrentOperation,
      ParameterAxis::None},
     {OperationSlot::GetVolumes, RequestFamily::GetVolumes, "GetVolumes", std::monostate{}, OperationScope::SelectedSession, ParameterAxis::Session},
     {OperationSlot::AddVolume, RequestFamily::AddVolume, "AddVolume", std::monostate{}, OperationScope::NextConfiguration,
      ParameterAxis::VolumeIdentity | ParameterAxis::DriveContext},
     {OperationSlot::RemoveVolume, RequestFamily::RemoveVolume, "RemoveVolume", std::monostate{}, OperationScope::NextConfiguration,
      ParameterAxis::VolumeIdentity},
     {OperationSlot::SetVolumeProtected, RequestFamily::SetVolume, "SetVolume.Protected", VolumeProperty::Protected, OperationScope::NextConfiguration,
      ParameterAxis::Boolean | ParameterAxis::VolumeIdentity | ParameterAxis::DriveContext | ParameterAxis::CreationBinding},
     {OperationSlot::SetVolumeBindByDriveLetter, RequestFamily::SetVolume, "SetVolume.BindByDriveLetter", VolumeProperty::BindByDriveLetter,
      OperationScope::NextConfiguration, ParameterAxis::Boolean | ParameterAxis::VolumeIdentity | ParameterAxis::DriveContext | ParameterAxis::CreationBinding},
     {OperationSlot::GetVolumePropertyProtected, RequestFamily::GetVolumeProperty, "GetVolumeProperty.Protected", VolumeProperty::Protected,
      OperationScope::SelectedSession, ParameterAxis::Session | ParameterAxis::VolumeIdentity},
     {OperationSlot::GetVolumePropertyBindByDriveLetter, RequestFamily::GetVolumeProperty, "GetVolumeProperty.BindByDriveLetter",
      VolumeProperty::BindByDriveLetter, OperationScope::SelectedSession, ParameterAxis::Session | ParameterAxis::VolumeIdentity},
     {OperationSlot::GetFileExclusions, RequestFamily::GetFileExclusions, "GetFileExclusions", std::monostate{}, OperationScope::SelectedSession,
      ParameterAxis::Session | ParameterAxis::VolumeIdentity},
     {OperationSlot::EditFileExclusionAdd, RequestFamily::EditFileExclusion, "EditFileExclusion.Add", ExclusionAction::Add, OperationScope::NextConfiguration,
      ParameterAxis::VolumeIdentity | ParameterAxis::FilePath | ParameterAxis::ExclusionSchema},
     {OperationSlot::EditFileExclusionRemove, RequestFamily::EditFileExclusion, "EditFileExclusion.Remove", ExclusionAction::Remove,
      OperationScope::NextConfiguration, ParameterAxis::VolumeIdentity | ParameterAxis::FilePath},
     {OperationSlot::EditFileExclusionClear, RequestFamily::EditFileExclusion, "EditFileExclusion.Clear", ExclusionAction::Clear,
      OperationScope::NextConfiguration, ParameterAxis::VolumeIdentity},
     {OperationSlot::FindFileExclusion, RequestFamily::FindFileExclusion, "FindFileExclusion", std::monostate{}, OperationScope::SelectedSession,
      ParameterAxis::Session | ParameterAxis::VolumeIdentity | ParameterAxis::FilePath},
     {OperationSlot::GetRegistryExclusions, RequestFamily::GetRegistryExclusions, "GetRegistryExclusions", std::monostate{}, OperationScope::SelectedSession,
      ParameterAxis::Session},
     {OperationSlot::EditRegistryExclusionAdd, RequestFamily::EditRegistryExclusion, "EditRegistryExclusion.Add", ExclusionAction::Add,
      OperationScope::NextConfiguration, ParameterAxis::RegistryKey | ParameterAxis::ExclusionSchema},
     {OperationSlot::EditRegistryExclusionRemove, RequestFamily::EditRegistryExclusion, "EditRegistryExclusion.Remove", ExclusionAction::Remove,
      OperationScope::NextConfiguration, ParameterAxis::RegistryKey},
     {OperationSlot::EditRegistryExclusionClear, RequestFamily::EditRegistryExclusion, "EditRegistryExclusion.Clear", ExclusionAction::Clear,
      OperationScope::NextConfiguration, ParameterAxis::None},
     {OperationSlot::FindRegistryExclusion, RequestFamily::FindRegistryExclusion, "FindRegistryExclusion", std::monostate{}, OperationScope::SelectedSession,
      ParameterAxis::Session | ParameterAxis::RegistryKey},
     {OperationSlot::CommitFile, RequestFamily::CommitFile, "CommitFile", std::monostate{}, OperationScope::CurrentOperation,
      ParameterAxis::VolumeIdentity | ParameterAxis::FilePath},
     {OperationSlot::CommitFileDeletion, RequestFamily::CommitFileDeletion, "CommitFileDeletion", std::monostate{}, OperationScope::CurrentOperation,
      ParameterAxis::VolumeIdentity | ParameterAxis::FilePath},
     {OperationSlot::CommitRegistry, RequestFamily::CommitRegistry, "CommitRegistry", std::monostate{}, OperationScope::CurrentOperation,
      ParameterAxis::RegistryKey | ParameterAxis::ValueName},
     {OperationSlot::CommitRegistryDeletion, RequestFamily::CommitRegistryDeletion, "CommitRegistryDeletion", std::monostate{},
      OperationScope::CurrentOperation, ParameterAxis::RegistryKey | ParameterAxis::ValueName},
     {OperationSlot::GetOverlayFiles, RequestFamily::GetOverlayFiles, "GetOverlayFiles", std::monostate{}, OperationScope::CurrentOperation,
      ParameterAxis::VolumeIdentity},
     {OperationSlot::GetSwapfileSize, RequestFamily::GetSwapfileSize, "GetSwapfileSize", std::monostate{}, OperationScope::SelectedSession,
      ParameterAxis::Session | ParameterAxis::VolumeIdentity},
     {OperationSlot::CreateSwapfile, RequestFamily::CreateSwapfile, "CreateSwapfile", std::monostate{}, OperationScope::NextConfiguration,
      ParameterAxis::VolumeIdentity | ParameterAxis::DriveContext | ParameterAxis::CreationBinding},
     {OperationSlot::ResetSettings, RequestFamily::ResetSettings, "ResetSettings", std::monostate{}, OperationScope::NextConfiguration, ParameterAxis::None},
     {OperationSlot::UpdateWindows, RequestFamily::UpdateWindows, "UpdateWindows", std::monostate{}, OperationScope::ImmediateWorkflow, ParameterAxis::None},
     {OperationSlot::PowerShutdown, RequestFamily::Power, "Power.Shutdown", PowerAction::Shutdown, OperationScope::ImmediateWorkflow, ParameterAxis::None},
     {OperationSlot::PowerRestart, RequestFamily::Power, "Power.Restart", PowerAction::Restart, OperationScope::ImmediateWorkflow, ParameterAxis::None}}};
constexpr std::array<StateAxisDescriptor, 89> stateAxes{
    {{StateAxis::CurrentFilter, "currentFilter", AxisNature::ObservedFact, "current.filterEnabled", "Exact session observation; no Current/Next substitution."},
     {StateAxis::NextFilter, "nextFilter", AxisNature::ObservedFact, "next.filterEnabled", "Exact session observation; no Current/Next substitution."},
     {StateAxis::Horm, "horm", AxisNature::ObservedFact, "runtime.horm", "Trusted applicable current fact only; missing or stale is Unknown."},
     {StateAxis::CurrentServicing, "currentServicing", AxisNature::ObservedFact, "current.servicingEnabled",
      "Exact session observation; no Current/Next substitution."},
     {StateAxis::NextServicing, "nextServicing", AxisNature::ObservedFact, "next.servicingEnabled", "Exact session observation; no Current/Next substitution."},
     {StateAxis::CurrentPersistDomain, "currentPersistDomain", AxisNature::ObservedFact, "current.persistDomainSecretKey",
      "Exact session observation; no Current/Next substitution."},
     {StateAxis::NextPersistDomain, "nextPersistDomain", AxisNature::ObservedFact, "next.persistDomainSecretKey",
      "Exact session observation; no Current/Next substitution."},
     {StateAxis::CurrentPersistTscal, "currentPersistTscal", AxisNature::ObservedFact, "current.persistTscal",
      "Exact session observation; no Current/Next substitution."},
     {StateAxis::NextPersistTscal, "nextPersistTscal", AxisNature::ObservedFact, "next.persistTscal",
      "Exact session observation; no Current/Next substitution."},
     {StateAxis::CurrentVolumeProtection, "currentVolumeProtection", AxisNature::ObservedFact, "current.volumes[id].protectedVolume",
      "Canonical target GUID; membership, property and physical presence remain distinct."},
     {StateAxis::NextVolumeProtection, "nextVolumeProtection", AxisNature::ObservedFact, "next.volumes[id].protectedVolume",
      "Canonical target GUID; membership, property and physical presence remain distinct."},
     {StateAxis::Administrator, "administrator", AxisNature::ObservedFact, "administrator",
      "Trusted applicable current fact only; missing or stale is Unknown."},
     {StateAxis::ControlDevicePresent, "controlDevicePresent", AxisNature::ObservedFact, "runtime.controlDevicePresent",
      "Trusted applicable current fact only; missing or stale is Unknown."},
     {StateAxis::Hiberfile, "hiberfile", AxisNature::ObservedFact, "runtime.hiberFile", "Trusted applicable current fact only; missing or stale is Unknown."},
     {StateAxis::HormVolumesProtected, "hormVolumesProtected", AxisNature::ObservedFact, "runtime.hormVolumesProtected",
      "Trusted applicable current fact only; missing or stale is Unknown."},
     {StateAxis::CapturedSettingsAvailable, "capturedSettingsAvailable", AxisNature::ObservedFact, "capturedSettingsAvailable",
      "Trusted applicable current fact only; missing or stale is Unknown."},
     {StateAxis::CurrentOverlayType, "currentOverlayType", AxisNature::ObservedFact, "current.overlayType",
      "Preserve unrecognized numeric observations and partial known masks."},
     {StateAxis::NextOverlayType, "nextOverlayType", AxisNature::ObservedFact, "next.overlayType",
      "Preserve unrecognized numeric observations and partial known masks."},
     {StateAxis::CurrentOverlayFlags, "currentOverlayFlags", AxisNature::ObservedFact, "current.overlayFlags",
      "Preserve unrecognized numeric observations and partial known masks."},
     {StateAxis::NextOverlayFlags, "nextOverlayFlags", AxisNature::ObservedFact, "next.overlayFlags",
      "Preserve unrecognized numeric observations and partial known masks."},
     {StateAxis::RomDisposition, "romDisposition", AxisNature::ObservedFact, "runtime.romDisposition",
      "Distinct runtime observations; a future write result is not a fact."},
     {StateAxis::RuntimeReset, "runtimeReset", AxisNature::ObservedFact, "runtime.resetState",
      "Distinct runtime observations; a future write result is not a fact."},
     {StateAxis::PersistentRollbackState, "persistentRollbackState", AxisNature::ObservedFact, "runtime.persistentRollbackState",
      "Distinct runtime observations; a future write result is not a fact."},
     {StateAxis::CurrentSavedMode, "currentSavedMode", AxisNature::ObservedFact, "current.savedMode",
      "Preserve unrecognized numeric observations and partial known masks."},
     {StateAxis::NextSavedMode, "nextSavedMode", AxisNature::ObservedFact, "next.savedMode",
      "Preserve unrecognized numeric observations and partial known masks."},
     {StateAxis::CurrentVolumePresent, "currentVolumePresent", AxisNature::ObservedFact, "current.volumes[id].present",
      "Canonical target GUID; membership, property and physical presence remain distinct."},
     {StateAxis::NextVolumePresent, "nextVolumePresent", AxisNature::ObservedFact, "next.volumes[id].present",
      "Canonical target GUID; membership, property and physical presence remain distinct."},
     {StateAxis::CurrentVolumeBinding, "currentVolumeBinding", AxisNature::ObservedFact, "current.volumes[id].bindByDriveLetter",
      "Canonical target GUID; membership, property and physical presence remain distinct."},
     {StateAxis::NextVolumeBinding, "nextVolumeBinding", AxisNature::ObservedFact, "next.volumes[id].bindByDriveLetter",
      "Canonical target GUID; membership, property and physical presence remain distinct."},
     {StateAxis::DriveMapping, "driveMapping", AxisNature::BackendCondition, "current/next.volumes[id].driveLetter",
      "Configuration metadata alone is not proof of a current OS mapping. Backend resolves targets independently."},
     {StateAxis::CurrentFileExclusion, "currentFileExclusion", AxisNature::DerivedView, "current inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::CurrentRegistryExclusion, "currentRegistryExclusion", AxisNature::DerivedView, "current inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::NextFileExclusion, "nextFileExclusion", AxisNature::DerivedView, "next inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::NextRegistryExclusion, "nextRegistryExclusion", AxisNature::DerivedView, "next inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::CurrentSwapHost, "currentSwapHost", AxisNature::ObservedFact, "current.volumes[id].swapfileMarker",
      "Canonical target GUID; membership, property and physical presence remain distinct."},
     {StateAxis::NextSwapHost, "nextSwapHost", AxisNature::ObservedFact, "next.volumes[id].swapfileMarker",
      "Canonical target GUID; membership, property and physical presence remain distinct."},
     {StateAxis::CurrentMaximumSize, "currentMaximumSize", AxisNature::ObservedFact, "current.maximumOverlaySize",
      "Preserve unrecognized numeric observations and partial known masks."},
     {StateAxis::NextMaximumOverlaySize, "nextMaximumOverlaySize", AxisNature::ObservedFact, "next.maximumOverlaySize",
      "Preserve unrecognized numeric observations and partial known masks."},
     {StateAxis::WarningThreshold, "warningThreshold", AxisNature::ObservedFact, "runtime.warningThreshold",
      "Distinct runtime observations; a future write result is not a fact."},
     {StateAxis::CriticalThreshold, "criticalThreshold", AxisNature::ObservedFact, "runtime.criticalThreshold",
      "Distinct runtime observations; a future write result is not a fact."},
     {StateAxis::HormFinalSafety, "hormFinalSafety", AxisNature::BackendCondition, "runtime.hormFinalSafety",
      "Provider/local branch restrictions are not universal solver rules. An injected eventual check result is not a precondition observation."},
     {StateAxis::VolumeEligibility, "volumeEligibility", AxisNature::ObservedFact, "targets[id].volumeEligibility",
      "Only a proved selected branch may use the fact. Readiness alone must not reject a possible no-copy path."},
     {StateAxis::PersistentFileView, "persistentFileView", AxisNature::ObservedFact, "targets[id].persistentFileViewReady",
      "Only a proved selected branch may use the fact. Readiness alone must not reject a possible no-copy path."},
     {StateAxis::PersistentHiveView, "persistentHiveView", AxisNature::BackendCondition, "backend-local observation",
      "Provider/local branch restrictions are not universal solver rules. An injected eventual check result is not a precondition observation."},
     {StateAxis::TargetKind, "targetKind", AxisNature::DerivedView, "targets[id].fileTarget + requestTarget",
      "Exact request-bound target observation; unknown is not missing. Length and reserved rules differ from Add exclusion context."},
     {StateAxis::SystemVolumeIdentity, "systemVolumeIdentity", AxisNature::ObservedFact, "systemVolumeId",
      "Actual system volume identity, never arbitrary protected volume or C: inference."},
     {StateAxis::ServicingStatus, "servicingStatus", AxisNature::ObservedFact, "runtime.servicingStatus",
      "Distinct runtime observations; a future write result is not a fact."},
     {StateAxis::SavedServicingFilter, "savedServicingFilter", AxisNature::ObservedFact, "runtime.savedServicingFilter",
      "Distinct runtime observations; a future write result is not a fact."},
     {StateAxis::FeatureAvailability, "featureAvailability", AxisNature::BackendCondition, "backend-local observation",
      "Provider/local branch restrictions are not universal solver rules. An injected eventual check result is not a precondition observation."},
     {StateAxis::ProviderAbi, "providerAbi", AxisNature::BackendCondition, "backend-local observation",
      "Provider/local branch restrictions are not universal solver rules. An injected eventual check result is not a precondition observation."},
     {StateAxis::ProviderTransport, "providerTransport", AxisNature::ObservedOutput, "backend response/diagnostic observation",
      "Model output validation and diagnostic units in backend tests; never predict this request result in the solver."},
     {StateAxis::ResponseShape, "responseShape", AxisNature::ObservedOutput, "backend response/diagnostic observation",
      "Model output validation and diagnostic units in backend tests; never predict this request result in the solver."},
     {StateAxis::Cancellation, "cancellation", AxisNature::ExecutionScenario, "controlled execution scenario",
      "Future schedule, fault stage, receipt or copy outcome is not production solver input. Content is unbounded outside the finite scenario profile."},
     {StateAxis::Concurrency, "concurrency", AxisNature::ExecutionScenario, "controlled execution scenario",
      "Future schedule, fault stage, receipt or copy outcome is not production solver input. Content is unbounded outside the finite scenario profile."},
     {StateAxis::RequestSession, "requestSession", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestSelectorValidity, "requestSelectorValidity", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestBoolean, "requestBoolean", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestNumber, "requestNumber", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestVolumeEncoding, "requestVolumeEncoding", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestExactText, "requestExactText", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestAddSchema, "requestAddSchema", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestRegistryValue, "requestRegistryValue", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::ShutdownPrivilege, "shutdownPrivilege", AxisNature::BackendCondition, "backend-local observation",
      "Provider/local branch restrictions are not universal solver rules. An injected eventual check result is not a precondition observation."},
     {StateAxis::ProviderPrerequisites, "providerPrerequisites", AxisNature::BackendCondition, "backend-local observation",
      "Provider/local branch restrictions are not universal solver rules. An injected eventual check result is not a precondition observation."},
     {StateAxis::MutationStage, "mutationStage", AxisNature::ExecutionScenario, "controlled execution scenario",
      "Future schedule, fault stage, receipt or copy outcome is not production solver input. Content is unbounded outside the finite scenario profile."},
     {StateAxis::ProviderWriteEffect, "providerWriteEffect", AxisNature::ExecutionScenario, "controlled execution scenario",
      "Future schedule, fault stage, receipt or copy outcome is not production solver input. Content is unbounded outside the finite scenario profile."},
     {StateAxis::UpdateOutcome, "updateOutcome", AxisNature::ExecutionScenario, "controlled execution scenario",
      "Future schedule, fault stage, receipt or copy outcome is not production solver input. Content is unbounded outside the finite scenario profile."},
     {StateAxis::TargetFileLength, "targetFileLength", AxisNature::ExecutionScenario, "controlled execution scenario",
      "Future schedule, fault stage, receipt or copy outcome is not production solver input. Content is unbounded outside the finite scenario profile."},
     {StateAxis::RequestTargetValidation, "requestTargetValidation", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestCreationBinding, "requestCreationBinding", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::RequestDriveContext, "requestDriveContext", AxisNature::RequestParameter, "ordinary Request",
      "Typed original arguments; schema, encoding and exact identity are assessed independently from system facts."},
     {StateAxis::PagingFilesValidation, "pagingFilesValidation", AxisNature::ObservedFact, "targets[id].pagingFilesValidation",
      "Only a proved selected branch may use the fact. Readiness alone must not reject a possible no-copy path."},
     {StateAxis::FileTargetObservation, "fileTargetObservation", AxisNature::DerivedView, "targets[id].fileTarget + requestTarget",
      "Exact request-bound target observation; unknown is not missing. Length and reserved rules differ from Add exclusion context."},
     {StateAxis::FileFullPathUnits, "fileFullPathUnits", AxisNature::DerivedView, "targets[id].fileTarget + requestTarget",
      "Exact request-bound target observation; unknown is not missing. Length and reserved rules differ from Add exclusion context."},
     {StateAxis::FileReserved, "fileReserved", AxisNature::DerivedView, "targets[id].fileTarget + requestTarget",
      "Exact request-bound target observation; unknown is not missing. Length and reserved rules differ from Add exclusion context."},
     {StateAxis::CurrentControlSet, "currentControlSet", AxisNature::BackendCondition, "backend-local observation",
      "Provider/local branch restrictions are not universal solver rules. An injected eventual check result is not a precondition observation."},
     {StateAxis::ConfiguredInventoryShape, "configuredInventoryShape", AxisNature::DerivedView, "current inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::CurrentFileListShape, "currentFileListShape", AxisNature::DerivedView, "current inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::NextFileListShape, "nextFileListShape", AxisNature::DerivedView, "next inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::CurrentRegistryListShape, "currentRegistryListShape", AxisNature::DerivedView, "current inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::NextRegistryListShape, "nextRegistryListShape", AxisNature::DerivedView, "next inventory/exclusions",
      "Derived from authoritative inventory/list facts plus exact target; do not multiply this view independently from its source."},
     {StateAxis::FullCommitFilterOutcome, "fullCommitFilterOutcome", AxisNature::ExecutionScenario, "controlled execution scenario",
      "Future schedule, fault stage, receipt or copy outcome is not production solver input. Content is unbounded outside the finite scenario profile."},
     {StateAxis::DiagnosticClusterSize, "diagnosticClusterSize", AxisNature::ObservedOutput, "backend response/diagnostic observation",
      "Model output validation and diagnostic units in backend tests; never predict this request result in the solver."},
     {StateAxis::DiagnosticCandidates, "diagnosticCandidates", AxisNature::ObservedOutput, "backend response/diagnostic observation",
      "Model output validation and diagnostic units in backend tests; never predict this request result in the solver."},
     {StateAxis::DiagnosticReportProfile, "diagnosticReportProfile", AxisNature::FixtureProfile, "Fake Runtime diagnosticCandidateReports",
      "Explicit observation profile independent of simulated file Data; never an OS-completeness claim."},
     {StateAxis::ShutdownPending, "shutdownPending", AxisNature::ObservedFact, "runtime.shutdownPending",
      "Existing runtime observation; no universal failure predicate is established from its value."},
     {StateAxis::FileSystem, "fileSystem", AxisNature::ObservedFact, "targets[id].fileSystem",
      "Identity-bound existing observation; no filesystem-name whitelist is inferred for solver rejection."},
     {StateAxis::VolumeBytes, "volumeBytes", AxisNature::ObservedFact, "targets[id].volumeBytes",
      "Identity-bound byte observation; this is not a reservation or a prediction of a later allocation result."},
     {StateAxis::AvailableBytes, "availableBytes", AxisNature::ObservedFact, "targets[id].availableBytes",
      "Identity-bound available-byte observation; no unproved allocation failure threshold is imposed by the solver."}}};
static_assert(operations.size() == static_cast<std::size_t>(OperationSlot::Count));
static_assert(static_cast<std::size_t>(BooleanProperty::Count) == 8);
static_assert(static_cast<std::size_t>(NumberProperty::Count) == 7);
static_assert(static_cast<std::size_t>(VolumeProperty::Count) == 2);
static_assert(static_cast<std::size_t>(OverlayOption::Count) == 3);
static_assert(static_cast<std::size_t>(ExclusionAction::Count) == 3);
static_assert(static_cast<std::size_t>(PowerAction::Count) == 2);
static_assert(operations.size() == std::variant_size_v<Request> + 2 * (static_cast<std::size_t>(BooleanProperty::Count) - 1) +
                                       2 * (static_cast<std::size_t>(NumberProperty::Count) - 1) + 2 * (static_cast<std::size_t>(VolumeProperty::Count) - 1) +
                                       (static_cast<std::size_t>(OverlayOption::Count) - 1) + 2 * (static_cast<std::size_t>(ExclusionAction::Count) - 1) +
                                       (static_cast<std::size_t>(PowerAction::Count) - 1));
static_assert(stateAxes.size() == static_cast<std::size_t>(StateAxis::Count));
static_assert(std::variant_size_v<Request> == static_cast<std::size_t>(RequestFamily::Count));
template <class>
inline constexpr bool unregisteredRequest = false;
std::optional<OperationSlot> find(RequestFamily family, OperationSelector selector) {
  auto found = std::ranges::find_if(operations, [&](const auto& entry) { return entry.family == family && entry.selector == selector; });
  return found == operations.end() ? std::nullopt : std::optional{found->slot};
}
}  // namespace
std::span<const OperationDescriptor> operationCatalog() { return operations; }
std::span<const StateAxisDescriptor> stateAxisCatalog() { return stateAxes; }
std::optional<OperationSlot> operationSlot(const Request& request) {
  return std::visit(
      [](const auto& operation) -> std::optional<OperationSlot> {
        using R = std::decay_t<decltype(operation)>;
        if constexpr (std::is_same_v<R, GetBoolean>)
          return find(RequestFamily::GetBoolean, operation.property);
        else if constexpr (std::is_same_v<R, SetBoolean>)
          return find(RequestFamily::SetBoolean, operation.property);
        else if constexpr (std::is_same_v<R, GetNumber>)
          return find(RequestFamily::GetNumber, operation.property);
        else if constexpr (std::is_same_v<R, SetNumber>)
          return find(RequestFamily::SetNumber, operation.property);
        else if constexpr (std::is_same_v<R, SetOverlayOption>)
          return find(RequestFamily::SetOverlayOption, operation.option);
        else if constexpr (std::is_same_v<R, GetOverlayUsage>)
          return find(RequestFamily::GetOverlayUsage, std::monostate{});
        else if constexpr (std::is_same_v<R, GetVolumes>)
          return find(RequestFamily::GetVolumes, std::monostate{});
        else if constexpr (std::is_same_v<R, AddVolume>)
          return find(RequestFamily::AddVolume, std::monostate{});
        else if constexpr (std::is_same_v<R, RemoveVolume>)
          return find(RequestFamily::RemoveVolume, std::monostate{});
        else if constexpr (std::is_same_v<R, SetVolume>)
          return find(RequestFamily::SetVolume, operation.property);
        else if constexpr (std::is_same_v<R, GetVolumeProperty>)
          return find(RequestFamily::GetVolumeProperty, operation.property);
        else if constexpr (std::is_same_v<R, GetFileExclusions>)
          return find(RequestFamily::GetFileExclusions, std::monostate{});
        else if constexpr (std::is_same_v<R, EditFileExclusion>)
          return find(RequestFamily::EditFileExclusion, operation.action);
        else if constexpr (std::is_same_v<R, FindFileExclusion>)
          return find(RequestFamily::FindFileExclusion, std::monostate{});
        else if constexpr (std::is_same_v<R, GetRegistryExclusions>)
          return find(RequestFamily::GetRegistryExclusions, std::monostate{});
        else if constexpr (std::is_same_v<R, EditRegistryExclusion>)
          return find(RequestFamily::EditRegistryExclusion, operation.action);
        else if constexpr (std::is_same_v<R, FindRegistryExclusion>)
          return find(RequestFamily::FindRegistryExclusion, std::monostate{});
        else if constexpr (std::is_same_v<R, CommitFile>)
          return find(RequestFamily::CommitFile, std::monostate{});
        else if constexpr (std::is_same_v<R, CommitFileDeletion>)
          return find(RequestFamily::CommitFileDeletion, std::monostate{});
        else if constexpr (std::is_same_v<R, CommitRegistry>)
          return find(RequestFamily::CommitRegistry, std::monostate{});
        else if constexpr (std::is_same_v<R, CommitRegistryDeletion>)
          return find(RequestFamily::CommitRegistryDeletion, std::monostate{});
        else if constexpr (std::is_same_v<R, GetOverlayFiles>)
          return find(RequestFamily::GetOverlayFiles, std::monostate{});
        else if constexpr (std::is_same_v<R, GetSwapfileSize>)
          return find(RequestFamily::GetSwapfileSize, std::monostate{});
        else if constexpr (std::is_same_v<R, CreateSwapfile>)
          return find(RequestFamily::CreateSwapfile, std::monostate{});
        else if constexpr (std::is_same_v<R, ResetSettings>)
          return find(RequestFamily::ResetSettings, std::monostate{});
        else if constexpr (std::is_same_v<R, UpdateWindows>)
          return find(RequestFamily::UpdateWindows, std::monostate{});
        else if constexpr (std::is_same_v<R, Power>)
          return find(RequestFamily::Power, operation.action);
        else
          static_assert(unregisteredRequest<R>, "Every request variant needs a catalog entry");
      },
      request);
}
}  // namespace uwf::test::solver_catalog
