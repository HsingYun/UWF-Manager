// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "domain/uwf/contracts/Backend.h"

namespace uwf::test::captured {
// Captured observations with literal expectations; each adapter supplies independent wire fixtures.
struct ExpectedReceipt {
  std::optional<compat::ErrorKind> failure;
  compat::Effect effect = compat::Effect::None;
  std::optional<compat::Value> value;
};
struct RequestVector {
  const char* id;
  compat::Request request;
  ExpectedReceipt expected;
  const char* evidence;
};

// Fixture facts: Current filter=false, Next filter=true. The two observations
// must remain distinct even when an adapter uses one session-bearing getter.
inline std::vector<RequestVector> filterSessionReads() {
  using namespace compat;
  return {{"filter-current-off",
           GetBoolean{BooleanProperty::FilterEnabled, Session::Current},
           {{}, Effect::None, Value{false}},
           "uwfcfgmgmt.dll:GetFilterEnabled; uwfmgr.exe current/next handler census"},
          {"filter-next-on",
           GetBoolean{BooleanProperty::FilterEnabled, Session::Next},
           {{}, Effect::None, Value{true}},
           "uwfcfgmgmt.dll:GetFilterEnabled; uwfmgr.exe current/next handler census"}};
}

inline std::vector<RequestVector> exactTargetRejections() {
  using namespace compat;
  constexpr auto key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\CapturedContract";
  const ExpectedReceipt unsupported{ErrorKind::Unsupported, Effect::None, {}};
  const ExpectedReceipt invalid{ErrorKind::InvalidArgument, Effect::None, {}};
  return {{"whole-key-ordinary-commit", CommitRegistry{key, std::nullopt}, unsupported, "uwfreg.sys exact-value commit protocol; Requests CommitRegistry"},
          {"default-value-deletion", CommitRegistryDeletion{key, std::string{}}, unsupported,
           "uwfcfgmgmt.dll deletion packing loses present-empty identity; Requests CommitRegistryDeletion"},
          {"embedded-nul-value", CommitRegistry{key, std::string("Value\0Other", 11)}, unsupported,
           "captured NUL-terminated value ABI cannot preserve exact identity"},
          {"current-control-set-prefix-collision", CommitRegistry{"HKLM\\SYSTEM\\CurrentControlSetExtra\\Fixture", std::string("Value")}, unsupported,
           "uwfrtl.sys CurrentControlSet prefix translation; exact-target safety contract"},
          {"incomplete-volume-identity", GetVolumeProperty{"not-a-guid", VolumeProperty::Protected, Session::Current}, invalid,
           "captured volume ABI requires complete GUID; shared exact-input contract"},
          {"malformed-utf8-path", CommitFile{"11111111-2222-3333-4444-555555555555", std::string("\\\xC0", 2)}, invalid,
           "UTF-8/UTF-16 exact target boundary rejects malformed sequence before probe"}};
}

struct HormTransitionVector {
  const char* id;
  bool beforeEnabled;
  bool requestedEnabled;
  bool controlPresent;
  std::uint32_t beforeDisposition;
  bool afterEnabled;
  std::uint32_t afterDisposition;
  ExpectedReceipt expected;
};
inline std::vector<HormTransitionVector> hormTransitions() {
  using namespace compat;
  // uwfvol.sys:0x1b60 and uwfrtl.sys:0xd830: disable ORs mask 1 without clearing mask 2.
  return {{"enable", false, true, true, 0, true, 2, {{}, Effect::Confirmed, {}}},
          {"disable-or-one", true, false, true, 2, false, 3, {{}, Effect::Confirmed, {}}},
          {"enable-no-device", false, true, false, 0, false, 0, {ErrorKind::Conflict, Effect::Uncertain, {}}},
          {"disable-no-device", true, false, false, 2, true, 2, {ErrorKind::Conflict, Effect::Uncertain, {}}},
          {"already-disabled-no-device", false, false, false, 0, false, 0, {{}, Effect::None, {}}},
          {"already-enabled-no-device", true, true, false, 2, true, 2, {{}, Effect::None, {}}}};
}

enum class CopyCheckpoint { Eof, FirstDataChunk, Data, Ea, Ads, Time, Security };
struct CopyFailureVector {
  const char* id;
  CopyCheckpoint checkpoint;
  bool exactDataKnown;
  std::size_t completedPrefixBytes;
  std::size_t completedMetadataHelpers;
};
inline std::vector<CopyFailureVector> copyFailures() {
  // Fixture input has 65,537 bytes. uwfs.sys:0xb054 sets EOF then copies
  // 65,536-byte chunks; 0xba88 metadata order is EA, ADS, time, security.
  return {{"eof", CopyCheckpoint::Eof, false, 0, 0},
          {"first-data-chunk", CopyCheckpoint::FirstDataChunk, false, 65536, 0},
          {"data", CopyCheckpoint::Data, true, 65537, 0},
          {"ea", CopyCheckpoint::Ea, true, 65537, 1},
          {"ads", CopyCheckpoint::Ads, true, 65537, 2},
          {"time", CopyCheckpoint::Time, true, 65537, 3},
          {"security", CopyCheckpoint::Security, true, 65537, 4}};
}
}  // namespace uwf::test::captured
