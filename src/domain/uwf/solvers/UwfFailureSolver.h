// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <string>
#include <variant>
#include <vector>

#include "domain/uwf/contracts/UwfStateFacts.h"

namespace uwf::compat {
enum class RuleId {
  InvalidRequest,
  AdministratorRequired,
  HormBlocksConfiguration,
  CurrentFilterMustBeOff,
  FullCommitNeedsCurrentFilter,
  FullCommitDispositionConflict,
  HormEnableNeedsCurrentFilter,
  HormEnableNeedsRam,
  HormEnableNeedsRomFlags,
  HormEnableNeedsHiberFile,
  HormEnableNeedsProtectedVolumes,
  HormDispositionConflict,
  ControlDeviceRequired,
  FilterDispositionConflict,
  InvalidNextOverlayType,
  IncompatibleOverlayConfiguration,
  ResetNeedsCurrentFilter,
  ResetNeedsDisk,
  ResetNeedsPersistence,
  InvalidSavedResetMode,
  CapturedDefaultsRequired,
  ExistingNextVolume,
  MissingNextVolume,
  MissingCurrentVolume,
  IneligibleVolume,
  FixedMediaRequired,
  MaintenanceNeedsProtection,
  HormBlocksMaintenance,
  RomBlocksMaintenance,
  PersistentFileViewUnavailable,
  OverlayEnumerationNeedsCurrentFilter,
  OverlayEnumerationNeedsProtection,
  InvalidServicingStatus,
  MissingServicingRestoreState,
  ServicingFilterConflict,
  FileTargetTooLong,
  ReservedFileTarget,
  MissingFileTarget,
  InvalidFileTargetKind,
  NonemptyDirectory,
  AllPathsRejected,
  RomConfigurationNotPersistent
};
struct Continue {};
struct Reject {
  RuleId rule;
  ErrorKind kind;
  std::string reason;
};
using SolverDecision = std::variant<Continue, Reject>;
struct ParameterAssessment {
  std::string operation;
  bool staticallyValid = true;
};
struct SolverEvaluation {
  Request arguments;
  SolverDecision decision;
  ParameterAssessment parameters;
};
// Pure partial-information inference. Continue never supplies execution facts
// or grants permission; every backend remains an independent implementation.
[[nodiscard]] SolverEvaluation solveUwfRequest(const Request& request, const UwfStateFacts& facts);
[[nodiscard]] std::string ruleName(RuleId rule);
}  // namespace uwf::compat
