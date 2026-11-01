// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <variant>

#include "domain/uwf/contracts/Requests.h"
namespace uwf::test::solver_catalog {
using compat::BooleanProperty;
using compat::ExclusionAction;
using compat::NumberProperty;
using compat::OverlayOption;
using compat::PowerAction;
using compat::Request;
using compat::VolumeProperty;
enum class RequestFamily {
  GetBoolean,
  SetBoolean,
  GetNumber,
  SetNumber,
  SetOverlayOption,
  GetOverlayUsage,
  GetVolumes,
  AddVolume,
  RemoveVolume,
  SetVolume,
  GetVolumeProperty,
  GetFileExclusions,
  EditFileExclusion,
  FindFileExclusion,
  GetRegistryExclusions,
  EditRegistryExclusion,
  FindRegistryExclusion,
  CommitFile,
  CommitFileDeletion,
  CommitRegistry,
  CommitRegistryDeletion,
  GetOverlayFiles,
  GetSwapfileSize,
  CreateSwapfile,
  ResetSettings,
  UpdateWindows,
  Power,
  Count
};
enum class OperationSlot {
  GetBooleanFilterEnabled,
  GetBooleanShutdownPending,
  GetBooleanHormEnabled,
  GetBooleanServicingEnabled,
  GetBooleanPersistDomainSecretKey,
  GetBooleanPersistTscal,
  GetBooleanOverlayCommit,
  GetBooleanRegistryChangesAlwaysPersisted,
  SetBooleanFilterEnabled,
  SetBooleanShutdownPending,
  SetBooleanHormEnabled,
  SetBooleanServicingEnabled,
  SetBooleanPersistDomainSecretKey,
  SetBooleanPersistTscal,
  SetBooleanOverlayCommit,
  SetBooleanRegistryChangesAlwaysPersisted,
  GetNumberOverlayType,
  GetNumberMaximumOverlaySize,
  GetNumberWarningThreshold,
  GetNumberCriticalThreshold,
  GetNumberOverlayFlags,
  GetNumberResetPersistentState,
  GetNumberResetPersistentStateSavedMode,
  SetNumberOverlayType,
  SetNumberMaximumOverlaySize,
  SetNumberWarningThreshold,
  SetNumberCriticalThreshold,
  SetNumberOverlayFlags,
  SetNumberResetPersistentState,
  SetNumberResetPersistentStateSavedMode,
  SetOverlayOptionPassthrough,
  SetOverlayOptionPersistent,
  SetOverlayOptionReadOnlyMedia,
  GetOverlayUsage,
  GetVolumes,
  AddVolume,
  RemoveVolume,
  SetVolumeProtected,
  SetVolumeBindByDriveLetter,
  GetVolumePropertyProtected,
  GetVolumePropertyBindByDriveLetter,
  GetFileExclusions,
  EditFileExclusionAdd,
  EditFileExclusionRemove,
  EditFileExclusionClear,
  FindFileExclusion,
  GetRegistryExclusions,
  EditRegistryExclusionAdd,
  EditRegistryExclusionRemove,
  EditRegistryExclusionClear,
  FindRegistryExclusion,
  CommitFile,
  CommitFileDeletion,
  CommitRegistry,
  CommitRegistryDeletion,
  GetOverlayFiles,
  GetSwapfileSize,
  CreateSwapfile,
  ResetSettings,
  UpdateWindows,
  PowerShutdown,
  PowerRestart,
  Count
};
enum class OperationScope { SelectedSession, RuntimeGlobal, NextConfiguration, CurrentOperation, ImmediateWorkflow };
enum class ParameterAxis : std::uint16_t {
  None = 0,
  Session = 1,
  Boolean = 2,
  Number = 4,
  VolumeIdentity = 8,
  DriveContext = 16,
  CreationBinding = 32,
  FilePath = 64,
  RegistryKey = 128,
  ValueName = 256,
  ExclusionSchema = 512
};
[[nodiscard]] constexpr ParameterAxis operator|(ParameterAxis a, ParameterAxis b) {
  return static_cast<ParameterAxis>(static_cast<std::uint16_t>(a) | static_cast<std::uint16_t>(b));
}
using OperationSelector = std::variant<std::monostate, BooleanProperty, NumberProperty, OverlayOption, VolumeProperty, ExclusionAction, PowerAction>;
struct OperationDescriptor {
  OperationSlot slot;
  RequestFamily family;
  std::string_view name;
  OperationSelector selector;
  OperationScope scope;
  ParameterAxis parameters;
};
[[nodiscard]] std::span<const OperationDescriptor> operationCatalog();
[[nodiscard]] std::optional<OperationSlot> operationSlot(const Request& request);
enum class StateAxis {
  CurrentFilter,
  NextFilter,
  Horm,
  CurrentServicing,
  NextServicing,
  CurrentPersistDomain,
  NextPersistDomain,
  CurrentPersistTscal,
  NextPersistTscal,
  CurrentVolumeProtection,
  NextVolumeProtection,
  Administrator,
  ControlDevicePresent,
  Hiberfile,
  HormVolumesProtected,
  CapturedSettingsAvailable,
  CurrentOverlayType,
  NextOverlayType,
  CurrentOverlayFlags,
  NextOverlayFlags,
  RomDisposition,
  RuntimeReset,
  PersistentRollbackState,
  CurrentSavedMode,
  NextSavedMode,
  CurrentVolumePresent,
  NextVolumePresent,
  CurrentVolumeBinding,
  NextVolumeBinding,
  DriveMapping,
  CurrentFileExclusion,
  CurrentRegistryExclusion,
  NextFileExclusion,
  NextRegistryExclusion,
  CurrentSwapHost,
  NextSwapHost,
  CurrentMaximumSize,
  NextMaximumOverlaySize,
  WarningThreshold,
  CriticalThreshold,
  HormFinalSafety,
  VolumeEligibility,
  PersistentFileView,
  PersistentHiveView,
  TargetKind,
  SystemVolumeIdentity,
  ServicingStatus,
  SavedServicingFilter,
  FeatureAvailability,
  ProviderAbi,
  ProviderTransport,
  ResponseShape,
  Cancellation,
  Concurrency,
  RequestSession,
  RequestSelectorValidity,
  RequestBoolean,
  RequestNumber,
  RequestVolumeEncoding,
  RequestExactText,
  RequestAddSchema,
  RequestRegistryValue,
  ShutdownPrivilege,
  ProviderPrerequisites,
  MutationStage,
  ProviderWriteEffect,
  UpdateOutcome,
  TargetFileLength,
  RequestTargetValidation,
  RequestCreationBinding,
  RequestDriveContext,
  PagingFilesValidation,
  FileTargetObservation,
  FileFullPathUnits,
  FileReserved,
  CurrentControlSet,
  ConfiguredInventoryShape,
  CurrentFileListShape,
  NextFileListShape,
  CurrentRegistryListShape,
  NextRegistryListShape,
  FullCommitFilterOutcome,
  DiagnosticClusterSize,
  DiagnosticCandidates,
  DiagnosticReportProfile,
  ShutdownPending,
  FileSystem,
  VolumeBytes,
  AvailableBytes,
  Count
};
// Only ObservedFact and trusted DerivedView supply solver facts; other categories describe test/adapter dimensions.
enum class AxisNature { ObservedFact, DerivedView, RequestParameter, BackendCondition, ObservedOutput, ExecutionScenario, FixtureProfile };
struct StateAxisDescriptor {
  StateAxis axis;
  std::string_view name;
  AxisNature nature;
  std::string_view representedBy;
  std::string_view boundary;
};
[[nodiscard]] std::span<const StateAxisDescriptor> stateAxisCatalog();
}  // namespace uwf::test::solver_catalog
