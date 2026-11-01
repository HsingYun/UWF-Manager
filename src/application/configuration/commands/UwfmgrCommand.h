// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace uwf::app {
enum class UwfmgrKind : uint8_t {
  Unknown,
  Help,
  GetConfig,

  FilterEnable,
  FilterDisable,
  FilterEnableHorm,
  FilterDisableHorm,
  FilterResetSettings,

  VolumeProtect,
  VolumeUnprotect,
  VolumeProtectAll,  // The "volume protect all" form; not another capability.
  VolumeCreateSwapfile,
  VolumeGetConfig,

  FileAddExclusion,
  FileRemoveExclusion,
  FileGetExclusions,
  FileCommit,
  FileCommitDelete,

  OverlaySetType,
  OverlaySetSize,
  OverlaySetWarningThreshold,
  OverlaySetCriticalThreshold,
  OverlaySetPassthrough,
  OverlaySetPersistent,
  OverlaySetReadOnlyMedia,
  OverlayResetPersistentState,
  OverlayGetConfig,
  OverlayGetAvailableSpace,
  OverlayGetConsumption,
  OverlayCommit,

  RegistryAddExclusion,
  RegistryRemoveExclusion,
  RegistryGetExclusions,
  RegistryCommit,
  RegistryCommitDelete,

  ServicingEnable,
  ServicingDisable,
  ServicingUpdateWindows,
  ServicingGetConfig,
};

// Parse categories are language-neutral; consumers localize their presentation.
enum class ParseError : uint8_t {
  None,
  Comment,  // Blank/comment input; callers omit it from execution.
  Incomplete,
  MissingSizeArg,
  InvalidSize,  // Not a uint32 decimal, or zero maximum capacity.
  MissingTypeArg,
  UnknownType,
  MissingVolumeArg,
  InvalidVolume,
  MissingPathArg,
  MissingRegistryKeyArg,
  MalformedQuoting,
  UnexpectedArgument,
  Unsupported,
};

// Rendering uses kind/args; parsing also retains source location and diagnostic context.
struct UwfmgrCommand {
  UwfmgrKind kind = UwfmgrKind::Unknown;
  std::vector<std::string> args;
  bool requiresPrevious = false;

  int sourceLineNo = 0;  // 1-based
  std::string rawLine;   // Original input, including surrounding whitespace.
  ParseError parseError = ParseError::None;
  std::string parseErrorContext;  // Original argument used in the diagnostic.
};

}  // namespace uwf::app
