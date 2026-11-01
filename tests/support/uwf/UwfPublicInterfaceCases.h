// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

#include "domain/uwf/contracts/Requests.h"

namespace uwf::test {
// Keep request spellings and write expectations independent of production catalogs so closure tests detect shared omissions.
struct UwfPublicInterfaceCase {
  std::string_view slot;
  std::string_view session;
  compat::Request request;
  bool write;
};

inline constexpr std::size_t publicInterfaceSlotCount = 62;
inline constexpr std::size_t publicInterfaceSignatureCount = 85;
inline constexpr std::size_t publicReadSignatureCount = 48;
inline constexpr std::size_t publicWriteSignatureCount = 37;
inline constexpr auto publicInterfaceVolume = "11111111-2222-3333-4444-555555555555";
inline constexpr auto publicInterfaceRegistryKey = "HKEY_LOCAL_MACHINE\\Software\\InterfaceWitness";

static_assert(std::variant_size_v<compat::Request> == 27);
static_assert(static_cast<int>(compat::BooleanProperty::Count) == 8);
static_assert(static_cast<int>(compat::NumberProperty::Count) == 7);
static_assert(static_cast<int>(compat::VolumeProperty::Count) == 2);
static_assert(static_cast<int>(compat::OverlayOption::Count) == 3);
static_assert(static_cast<int>(compat::ExclusionAction::Count) == 3);
static_assert(static_cast<int>(compat::PowerAction::Count) == 2);
static_assert(static_cast<int>(compat::Session::Current) == 0 && static_cast<int>(compat::Session::Next) == 1);

inline const auto uwfPublicInterfaceCases = [] {
  using namespace compat;
  return std::to_array<UwfPublicInterfaceCase>({
      {"GetBoolean.FilterEnabled", "Current", GetBoolean{BooleanProperty::FilterEnabled, Session::Current}, false},
      {"GetBoolean.FilterEnabled", "Next", GetBoolean{BooleanProperty::FilterEnabled, Session::Next}, false},
      {"GetBoolean.ShutdownPending", "Current", GetBoolean{BooleanProperty::ShutdownPending, Session::Current}, false},
      {"GetBoolean.ShutdownPending", "Next", GetBoolean{BooleanProperty::ShutdownPending, Session::Next}, false},
      {"GetBoolean.HormEnabled", "Current", GetBoolean{BooleanProperty::HormEnabled, Session::Current}, false},
      {"GetBoolean.HormEnabled", "Next", GetBoolean{BooleanProperty::HormEnabled, Session::Next}, false},
      {"GetBoolean.ServicingEnabled", "Current", GetBoolean{BooleanProperty::ServicingEnabled, Session::Current}, false},
      {"GetBoolean.ServicingEnabled", "Next", GetBoolean{BooleanProperty::ServicingEnabled, Session::Next}, false},
      {"GetBoolean.PersistDomainSecretKey", "Current", GetBoolean{BooleanProperty::PersistDomainSecretKey, Session::Current}, false},
      {"GetBoolean.PersistDomainSecretKey", "Next", GetBoolean{BooleanProperty::PersistDomainSecretKey, Session::Next}, false},
      {"GetBoolean.PersistTscal", "Current", GetBoolean{BooleanProperty::PersistTscal, Session::Current}, false},
      {"GetBoolean.PersistTscal", "Next", GetBoolean{BooleanProperty::PersistTscal, Session::Next}, false},
      {"GetBoolean.OverlayCommit", "Current", GetBoolean{BooleanProperty::OverlayCommit, Session::Current}, false},
      {"GetBoolean.OverlayCommit", "Next", GetBoolean{BooleanProperty::OverlayCommit, Session::Next}, false},
      {"GetBoolean.RegistryChangesAlwaysPersisted", "Current", GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, Session::Current}, false},
      {"GetBoolean.RegistryChangesAlwaysPersisted", "Next", GetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, Session::Next}, false},
      {"SetBoolean.FilterEnabled", "implicit", SetBoolean{BooleanProperty::FilterEnabled, true}, true},
      {"SetBoolean.ShutdownPending", "implicit", SetBoolean{BooleanProperty::ShutdownPending, true}, true},
      {"SetBoolean.HormEnabled", "implicit", SetBoolean{BooleanProperty::HormEnabled, true}, true},
      {"SetBoolean.ServicingEnabled", "implicit", SetBoolean{BooleanProperty::ServicingEnabled, true}, true},
      {"SetBoolean.PersistDomainSecretKey", "implicit", SetBoolean{BooleanProperty::PersistDomainSecretKey, true}, true},
      {"SetBoolean.PersistTscal", "implicit", SetBoolean{BooleanProperty::PersistTscal, true}, true},
      {"SetBoolean.OverlayCommit", "implicit", SetBoolean{BooleanProperty::OverlayCommit, true}, true},
      {"SetBoolean.RegistryChangesAlwaysPersisted", "implicit", SetBoolean{BooleanProperty::RegistryChangesAlwaysPersisted, true}, true},
      {"GetNumber.OverlayType", "Current", GetNumber{NumberProperty::OverlayType, Session::Current}, false},
      {"GetNumber.OverlayType", "Next", GetNumber{NumberProperty::OverlayType, Session::Next}, false},
      {"GetNumber.MaximumOverlaySize", "Current", GetNumber{NumberProperty::MaximumOverlaySize, Session::Current}, false},
      {"GetNumber.MaximumOverlaySize", "Next", GetNumber{NumberProperty::MaximumOverlaySize, Session::Next}, false},
      {"GetNumber.WarningThreshold", "Current", GetNumber{NumberProperty::WarningThreshold, Session::Current}, false},
      {"GetNumber.WarningThreshold", "Next", GetNumber{NumberProperty::WarningThreshold, Session::Next}, false},
      {"GetNumber.CriticalThreshold", "Current", GetNumber{NumberProperty::CriticalThreshold, Session::Current}, false},
      {"GetNumber.CriticalThreshold", "Next", GetNumber{NumberProperty::CriticalThreshold, Session::Next}, false},
      {"GetNumber.OverlayFlags", "Current", GetNumber{NumberProperty::OverlayFlags, Session::Current}, false},
      {"GetNumber.OverlayFlags", "Next", GetNumber{NumberProperty::OverlayFlags, Session::Next}, false},
      {"GetNumber.ResetPersistentState", "Current", GetNumber{NumberProperty::ResetPersistentState, Session::Current}, false},
      {"GetNumber.ResetPersistentState", "Next", GetNumber{NumberProperty::ResetPersistentState, Session::Next}, false},
      {"GetNumber.ResetPersistentStateSavedMode", "Current", GetNumber{NumberProperty::ResetPersistentStateSavedMode, Session::Current}, false},
      {"GetNumber.ResetPersistentStateSavedMode", "Next", GetNumber{NumberProperty::ResetPersistentStateSavedMode, Session::Next}, false},
      {"SetNumber.OverlayType", "implicit", SetNumber{NumberProperty::OverlayType, 1}, true},
      {"SetNumber.MaximumOverlaySize", "implicit", SetNumber{NumberProperty::MaximumOverlaySize, 2048}, true},
      {"SetNumber.WarningThreshold", "implicit", SetNumber{NumberProperty::WarningThreshold, 256}, true},
      {"SetNumber.CriticalThreshold", "implicit", SetNumber{NumberProperty::CriticalThreshold, 512}, true},
      {"SetNumber.OverlayFlags", "implicit", SetNumber{NumberProperty::OverlayFlags, 0}, true},
      {"SetNumber.ResetPersistentState", "implicit", SetNumber{NumberProperty::ResetPersistentState, 1}, true},
      {"SetNumber.ResetPersistentStateSavedMode", "implicit", SetNumber{NumberProperty::ResetPersistentStateSavedMode, 1}, true},
      {"SetOverlayOption.Passthrough", "implicit", SetOverlayOption{OverlayOption::Passthrough, true}, true},
      {"SetOverlayOption.Persistent", "implicit", SetOverlayOption{OverlayOption::Persistent, true}, true},
      {"SetOverlayOption.ReadOnlyMedia", "implicit", SetOverlayOption{OverlayOption::ReadOnlyMedia, true}, true},
      {"GetOverlayUsage", "implicit", GetOverlayUsage{}, false},
      {"GetVolumes", "Current", GetVolumes{Session::Current}, false},
      {"GetVolumes", "Next", GetVolumes{Session::Next}, false},
      {"AddVolume", "implicit", AddVolume{publicInterfaceVolume, "C:"}, true},
      {"RemoveVolume", "implicit", RemoveVolume{publicInterfaceVolume}, true},
      {"SetVolume.Protected", "implicit", SetVolume{publicInterfaceVolume, VolumeProperty::Protected, true}, true},
      {"SetVolume.BindByDriveLetter", "implicit", SetVolume{publicInterfaceVolume, VolumeProperty::BindByDriveLetter, true}, true},
      {"GetVolumeProperty.Protected", "Current", GetVolumeProperty{publicInterfaceVolume, VolumeProperty::Protected, Session::Current}, false},
      {"GetVolumeProperty.Protected", "Next", GetVolumeProperty{publicInterfaceVolume, VolumeProperty::Protected, Session::Next}, false},
      {"GetVolumeProperty.BindByDriveLetter", "Current", GetVolumeProperty{publicInterfaceVolume, VolumeProperty::BindByDriveLetter, Session::Current}, false},
      {"GetVolumeProperty.BindByDriveLetter", "Next", GetVolumeProperty{publicInterfaceVolume, VolumeProperty::BindByDriveLetter, Session::Next}, false},
      {"GetFileExclusions", "Current", GetFileExclusions{publicInterfaceVolume, Session::Current}, false},
      {"GetFileExclusions", "Next", GetFileExclusions{publicInterfaceVolume, Session::Next}, false},
      {"EditFileExclusion.Add", "implicit", EditFileExclusion{publicInterfaceVolume, "\\InterfaceWitness", ExclusionAction::Add}, true},
      {"EditFileExclusion.Remove", "implicit", EditFileExclusion{publicInterfaceVolume, "\\InterfaceWitness", ExclusionAction::Remove}, true},
      {"EditFileExclusion.Clear", "implicit", EditFileExclusion{publicInterfaceVolume, {}, ExclusionAction::Clear}, true},
      {"FindFileExclusion", "Current", FindFileExclusion{publicInterfaceVolume, "\\InterfaceWitness", Session::Current}, false},
      {"FindFileExclusion", "Next", FindFileExclusion{publicInterfaceVolume, "\\InterfaceWitness", Session::Next}, false},
      {"GetRegistryExclusions", "Current", GetRegistryExclusions{Session::Current}, false},
      {"GetRegistryExclusions", "Next", GetRegistryExclusions{Session::Next}, false},
      {"EditRegistryExclusion.Add", "implicit", EditRegistryExclusion{publicInterfaceRegistryKey, ExclusionAction::Add}, true},
      {"EditRegistryExclusion.Remove", "implicit", EditRegistryExclusion{publicInterfaceRegistryKey, ExclusionAction::Remove}, true},
      {"EditRegistryExclusion.Clear", "implicit", EditRegistryExclusion{{}, ExclusionAction::Clear}, true},
      {"FindRegistryExclusion", "Current", FindRegistryExclusion{publicInterfaceRegistryKey, Session::Current}, false},
      {"FindRegistryExclusion", "Next", FindRegistryExclusion{publicInterfaceRegistryKey, Session::Next}, false},
      {"CommitFile", "implicit", CommitFile{publicInterfaceVolume, "\\InterfaceWitness"}, true},
      {"CommitFileDeletion", "implicit", CommitFileDeletion{publicInterfaceVolume, "\\InterfaceWitness"}, true},
      {"CommitRegistry", "implicit", CommitRegistry{publicInterfaceRegistryKey, std::string("Named")}, true},
      {"CommitRegistryDeletion", "implicit", CommitRegistryDeletion{publicInterfaceRegistryKey, std::nullopt}, true},
      {"GetOverlayFiles", "implicit", GetOverlayFiles{publicInterfaceVolume}, false},
      {"GetSwapfileSize", "Current", GetSwapfileSize{publicInterfaceVolume, Session::Current}, false},
      {"GetSwapfileSize", "Next", GetSwapfileSize{publicInterfaceVolume, Session::Next}, false},
      {"CreateSwapfile", "implicit", CreateSwapfile{publicInterfaceVolume}, true},
      {"ResetSettings", "implicit", ResetSettings{}, true},
      {"UpdateWindows", "implicit", UpdateWindows{}, true},
      {"Power.Shutdown", "implicit", Power{PowerAction::Shutdown}, true},
      {"Power.Restart", "implicit", Power{PowerAction::Restart}, true},
  });
}();

static_assert(std::tuple_size_v<std::remove_cv_t<decltype(uwfPublicInterfaceCases)>> == publicInterfaceSignatureCount);
}  // namespace uwf::test
