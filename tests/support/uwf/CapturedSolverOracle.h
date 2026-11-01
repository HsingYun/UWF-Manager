// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "domain/uwf/contracts/UwfStateFacts.h"
namespace uwf::test::solver_oracle {
using namespace compat;
inline constexpr auto volume = "11111111-2222-3333-4444-555555555555";
inline constexpr auto otherVolume = "66666666-7777-8888-9999-aaaaaaaaaaaa";
inline constexpr auto key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\SolverFixture";
struct Slot {
  const char* id;
  Request request;
  bool rejectWithNoFacts;
};
// Independent literal request inventory. Writes to the three read-only properties reject without state facts.
inline std::vector<Slot> capabilitySlots() {
  return {{"GetBoolean.FilterEnabled", GetBoolean{BooleanProperty::FilterEnabled}, false},
          {"GetBoolean.ShutdownPending", GetBoolean{BooleanProperty::ShutdownPending}, false},
          {"GetBoolean.HormEnabled", GetBoolean{BooleanProperty::HormEnabled}, false},
          {"GetBoolean.ServicingEnabled", GetBoolean{BooleanProperty::ServicingEnabled}, false},
          {"GetBoolean.PersistDomainSecretKey", GetBoolean{BooleanProperty::PersistDomainSecretKey}, false},
          {"GetBoolean.PersistTscal", GetBoolean{BooleanProperty::PersistTscal}, false},
          {"GetBoolean.OverlayCommit", GetBoolean{BooleanProperty::OverlayCommit}, false},
          {"GetBoolean.RegistryChangesAlwaysPersisted", GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted}, false},
          {"SetBoolean.FilterEnabled", SetBoolean{BooleanProperty::FilterEnabled, true}, false},
          {"SetBoolean.ShutdownPending", SetBoolean{BooleanProperty::ShutdownPending, true}, true},
          {"SetBoolean.HormEnabled", SetBoolean{BooleanProperty::HormEnabled, true}, false},
          {"SetBoolean.ServicingEnabled", SetBoolean{BooleanProperty::ServicingEnabled, true}, false},
          {"SetBoolean.PersistDomainSecretKey", SetBoolean{BooleanProperty::PersistDomainSecretKey, true}, false},
          {"SetBoolean.PersistTscal", SetBoolean{BooleanProperty::PersistTscal, true}, false},
          {"SetBoolean.OverlayCommit", SetBoolean{BooleanProperty::OverlayCommit, true}, false},
          {"SetBoolean.RegistryChangesAlwaysPersisted", SetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, true}, true},
          {"GetNumber.OverlayType", GetNumber{NumberProperty::OverlayType}, false},
          {"GetNumber.MaximumOverlaySize", GetNumber{NumberProperty::MaximumOverlaySize}, false},
          {"GetNumber.WarningThreshold", GetNumber{NumberProperty::WarningThreshold}, false},
          {"GetNumber.CriticalThreshold", GetNumber{NumberProperty::CriticalThreshold}, false},
          {"GetNumber.OverlayFlags", GetNumber{NumberProperty::OverlayFlags}, false},
          {"GetNumber.ResetPersistentState", GetNumber{NumberProperty::ResetPersistentState}, false},
          {"GetNumber.ResetPersistentStateSavedMode", GetNumber{NumberProperty::ResetPersistentStateSavedMode}, false},
          {"SetNumber.OverlayType", SetNumber{NumberProperty::OverlayType, 0}, false},
          {"SetNumber.MaximumOverlaySize", SetNumber{NumberProperty::MaximumOverlaySize, 1024}, false},
          {"SetNumber.WarningThreshold", SetNumber{NumberProperty::WarningThreshold, 0}, false},
          {"SetNumber.CriticalThreshold", SetNumber{NumberProperty::CriticalThreshold, 0}, false},
          {"SetNumber.OverlayFlags", SetNumber{NumberProperty::OverlayFlags, 0}, true},
          {"SetNumber.ResetPersistentState", SetNumber{NumberProperty::ResetPersistentState, 1}, false},
          {"SetNumber.ResetPersistentStateSavedMode", SetNumber{NumberProperty::ResetPersistentStateSavedMode, 1}, false},
          {"SetOverlayOption.Passthrough", SetOverlayOption{OverlayOption::Passthrough, true}, false},
          {"SetOverlayOption.Persistent", SetOverlayOption{OverlayOption::Persistent, true}, false},
          {"SetOverlayOption.ReadOnlyMedia", SetOverlayOption{OverlayOption::ReadOnlyMedia, true}, false},
          {"GetOverlayUsage", GetOverlayUsage{}, false},
          {"GetVolumes", GetVolumes{}, false},
          {"AddVolume", AddVolume{volume, {}}, false},
          {"RemoveVolume", RemoveVolume{volume}, false},
          {"SetVolume.Protected", SetVolume{volume, VolumeProperty::Protected, true}, false},
          {"SetVolume.BindByDriveLetter", SetVolume{volume, VolumeProperty::BindByDriveLetter, true}, false},
          {"GetVolumeProperty.Protected", GetVolumeProperty{volume, VolumeProperty::Protected}, false},
          {"GetVolumeProperty.BindByDriveLetter", GetVolumeProperty{volume, VolumeProperty::BindByDriveLetter}, false},
          {"GetFileExclusions", GetFileExclusions{volume}, false},
          {"EditFileExclusion.Add", EditFileExclusion{volume, "\\data", ExclusionAction::Add}, false},
          {"EditFileExclusion.Remove", EditFileExclusion{volume, "\\data", ExclusionAction::Remove}, false},
          {"EditFileExclusion.Clear", EditFileExclusion{volume, "\\data", ExclusionAction::Clear}, false},
          {"FindFileExclusion", FindFileExclusion{volume, "\\data"}, false},
          {"GetRegistryExclusions", GetRegistryExclusions{}, false},
          {"EditRegistryExclusion.Add", EditRegistryExclusion{key, ExclusionAction::Add}, false},
          {"EditRegistryExclusion.Remove", EditRegistryExclusion{key, ExclusionAction::Remove}, false},
          {"EditRegistryExclusion.Clear", EditRegistryExclusion{key, ExclusionAction::Clear}, false},
          {"FindRegistryExclusion", FindRegistryExclusion{key}, false},
          {"CommitFile", CommitFile{volume, "\\data"}, false},
          {"CommitFileDeletion", CommitFileDeletion{volume, "\\data"}, false},
          {"CommitRegistry", CommitRegistry{key, std::string{"value"}}, false},
          {"CommitRegistryDeletion", CommitRegistryDeletion{key, std::string{"value"}}, false},
          {"GetOverlayFiles", GetOverlayFiles{volume}, false},
          {"GetSwapfileSize", GetSwapfileSize{volume}, false},
          {"CreateSwapfile", CreateSwapfile{volume}, false},
          {"ResetSettings", ResetSettings{}, false},
          {"UpdateWindows", UpdateWindows{}, false},
          {"Power.Shutdown", Power{PowerAction::Shutdown}, false},
          {"Power.Restart", Power{PowerAction::Restart}, false}};
}
// Two independent opaque-bit witnesses distinguish one known-zero bit from an entirely zero opaque region.
inline constexpr std::array<std::uint32_t, 32> flags{0,   1,   2,   3,   4,   5,   6,   7,   256, 257, 258, 259, 260, 261, 262, 263,
                                                     512, 513, 514, 515, 516, 517, 518, 519, 768, 769, 770, 771, 772, 773, 774, 775};
inline constexpr std::array<std::uint32_t, 16> dispositions{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
// Captured truth table, uwfcfgmgmt.dll:0x1b830 -> uwfvol.sys:0x3110.
// With loaded ROM, disposition 0 starts; mask 0x4 acknowledges a no-op; other nonzero dispositions reject.
inline constexpr std::array<char, 16> fullCommitRom{'A', 'R', 'R', 'R', 'A', 'A', 'A', 'A', 'R', 'R', 'R', 'R', 'A', 'A', 'A', 'A'};
// uwfcfgmgmt.dll:0x1b410, loaded-ROM disposition-1 request: masks 0x1/0x4 admit;
// mask 0x2 alone and opaque-only masks reject. Matching Next state bypasses the raw export.
inline constexpr std::array<char, 16> filterRom{'A', 'A', 'R', 'A', 'A', 'A', 'A', 'A', 'R', 'A', 'R', 'A', 'A', 'A', 'A', 'A'};
// HORM enable requires flags exactly 4; any additional opaque bit rejects.
inline constexpr std::array<char, 32> hormFlags{'R', 'R', 'R', 'R', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
                                                'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R'};
inline constexpr std::array<char, 32> loadedRom{'N', 'N', 'N', 'N', 'Y', 'Y', 'Y', 'Y', 'N', 'N', 'N', 'N', 'Y', 'Y', 'Y', 'Y',
                                                'N', 'N', 'N', 'N', 'Y', 'Y', 'Y', 'Y', 'N', 'N', 'N', 'N', 'Y', 'Y', 'Y', 'Y'};
inline std::vector<bool> completions(const Fact<bool>& fact) { return fact.value ? std::vector<bool>{*fact.value} : std::vector<bool>{false, true}; }
// Match partial bit facts without applying UWF policy.
inline bool compatible(const BitFacts& fact, std::uint32_t full) {
  if ((fact.valueBits & ~fact.knownMask) != 0) return true;
  return ((full ^ fact.valueBits) & fact.knownMask) == 0;
}
inline std::vector<BitFacts> partialFlags() {
  std::vector<BitFacts> result;
  constexpr std::array<std::uint32_t, 5> bits{1, 2, 4, 256, 512};
  for (unsigned code = 0; code < 243; ++code)
    for (bool otherKnown : {false, true}) {
      auto digits = code;
      BitFacts fact;
      if (otherKnown) fact.knownMask = ~std::uint32_t{775};
      for (const auto bit : bits) {
        const auto state = digits % 3;
        digits /= 3;
        if (state) {
          fact.knownMask |= bit;
          if (state == 2) fact.valueBits |= bit;
        }
      }
      result.push_back(fact);
    }
  return result;
}
inline std::vector<BitFacts> partialDispositions() {
  std::vector<BitFacts> result;
  constexpr std::array<std::uint32_t, 4> bits{1, 2, 4, 8};
  for (unsigned code = 0; code < 81; ++code)
    for (bool otherKnown : {false, true}) {
      auto digits = code;
      BitFacts fact;
      if (otherKnown) fact.knownMask = ~std::uint32_t{15};
      for (const auto bit : bits) {
        const auto state = digits % 3;
        digits /= 3;
        if (state) {
          fact.knownMask |= bit;
          if (state == 2) fact.valueBits |= bit;
        }
      }
      result.push_back(fact);
    }
  return result;
}

struct BitInformationClass {
  BitFacts representative;
  std::uint64_t weight;
  const char* category;
};
// 27 low-bit patterns x three opaque summaries partition valid 32-bit partial facts.
// Each of the 29 opaque positions is Unknown/Known0/Known1; literal weights account for these combinations.
// Invalid value/mask pairs form a separate Unknown class.
inline std::vector<BitInformationClass> bitInformationClasses(std::uint32_t opaqueWitness) {
  std::vector<BitInformationClass> result;
  constexpr std::array<std::uint32_t, 3> bits{1, 2, 4};
  for (unsigned code = 0; code < 27; ++code)
    for (unsigned category = 0; category < 3; ++category) {
      auto digits = code;
      BitFacts fact;
      if (category == 0) {
        fact.knownMask = opaqueWitness;
        fact.valueBits = opaqueWitness;
      }
      if (category == 1) fact.knownMask = ~std::uint32_t{7};
      for (const auto bit : bits) {
        const auto state = digits % 3;
        digits /= 3;
        if (state) {
          fact.knownMask |= bit;
          if (state == 2) fact.valueBits |= bit;
        }
      }
      constexpr std::array<std::uint64_t, 3> weights{68629840493971ULL, 1ULL, 536870911ULL};
      constexpr std::array<const char*, 3> labels{"opaque-known-one", "opaque-all-known-zero", "opaque-unknown-no-known-one"};
      result.push_back({fact, weights[category], labels[category]});
    }
  result.push_back({BitFacts{0, opaqueWitness}, 18444891053520699775ULL, "invalid-raw-mask-pair"});
  return result;
}
}  // namespace uwf::test::solver_oracle
