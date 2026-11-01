// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace uwf::compat {
// Current is loaded/running configuration; Next is saved startup configuration.
// Global properties ignore this selector. Unsupported Current access never
// permits substituting a Next value for the requested observation.
enum class Session { Current, Next };
enum class BooleanProperty {
  FilterEnabled,           // Current/Next filter configuration; setter targets Next.
  ShutdownPending,         // Global runtime notification state, read-only.
  HormEnabled,             // Global HORM runtime/boot state, not the static ROM flag.
  ServicingEnabled,        // Current/Next flag; setter includes the runtime transition.
  PersistDomainSecretKey,  // Current/Next built-in machine-account persistence, not user exclusions.
  PersistTscal,            // Current/Next built-in Terminal Services license persistence.
  OverlayCommit,           // Global FullCommit disposition bit; not a data-durability audit.
  // Global Current fact: true when the filter is disabled or the actual system
  // volume is not protected. Another protected volume does not protect registry
  // changes. This observation does not inspect per-key exclusions or FullCommit.
  RegistryChangesAlwaysPersisted,
  Count  // Catalogue sentinel; never a valid public property.
};
enum class NumberProperty {
  OverlayType,                    // Current/Next; OverlayType encoding, setter targets Next.
  MaximumOverlaySize,             // Current/Next; UWF MB, setter targets Next.
  WarningThreshold,               // Global live/saved UWF MB threshold; no artificial session split.
  CriticalThreshold,              // Global live/saved UWF MB threshold; zero remains a reported value.
  OverlayFlags,                   // Current/Next raw bits; public writes use SetOverlayOption.
  ResetPersistentState,           // Global control-device state, not a saved configuration DWORD.
  ResetPersistentStateSavedMode,  // Current/Next saved default; setter targets Next, not runtime Reset.
  Count
};
// Setters target Next. Binding=true selects letter binding; false selects volume
// identity. Neither field changes currently loaded protection or commits data.
enum class VolumeProperty { Protected, BindByDriveLetter, Count };
// Single-field Next intents over OverlayFlags, not claims a mode is active.
// Intrinsic activation constraints and stricter editor policy are distinct.
enum class OverlayOption { Passthrough, Persistent, ReadOnlyMedia, Count };
// Exact user entries. Clear snapshots that list; it is not physical tree deletion
// or a purge of the separate built-in persistence settings.
enum class ExclusionAction { Add, Remove, Clear, Count };
enum class PowerAction { Shutdown, Restart, Count };
// Workflow result, separate from invocation/transport failure. Failed is a
// reported outcome, not a successful claim that updates were installed.
enum class WindowsUpdateOutcome : std::uint32_t { Completed = 0, RebootRequired = 1, Failed = 2 };
enum class OverlayType : std::uint32_t { Ram = 0, Disk = 1 };
// Clear schedules one cleanup at startup. Saved resolves Current SavedMode at
// execution; readback reports actual runtime state, not selector 255. Next
// SavedMode affects later boots. Commit is readable but not portably writable.
enum class ResetMode : std::uint32_t { Preserve = 0, Clear = 1, Commit = 2, Saved = 255 };
namespace flags {
// Preserve opaque bits during single-option edits. These configuration masks
// do not encode runtime HORM/FullCommit disposition.
inline constexpr std::uint32_t passthrough = 1, persistent = 2, readOnlyMedia = 4;
}  // namespace flags
namespace disposition {
// Runtime control state, independent of the identically numbered configuration
// flags above. Preserve opaque bits when interpreting captured dispositions.
inline constexpr std::uint32_t filterChange = 1, horm = 2, fullCommit = 4;
}  // namespace disposition
namespace limits {
inline constexpr std::size_t fileMaintenanceBufferUnits = 260;
inline constexpr std::size_t registryKeyBufferUnits = 1024;
inline constexpr std::size_t registryValueBufferUnits = 256;
}  // namespace limits

// A UWF volume identity is a GUID, never a WMI object path. Backends accept
// bare GUIDs and the conventional Volume{GUID}/\\?\Volume{GUID}\ spelling.
struct Volume {
  std::string id;
  std::string driveLetter;  // Optional metadata; empty can mean no letter or unknown.
  bool protectedVolume = false;
  bool bindByDriveLetter = false;
  std::optional<bool> commitPending;  // nullopt is not supplied, not "no changes".
  bool operator==(const Volume&) const = default;
};
struct OverlayUsage {
  // UWF MB of the running overlay, not filesystem byte capacity. No promise that
  // their sum equals a maximum independently read from another session.
  std::uint32_t consumptionMb = 0;
  std::uint32_t availableMb = 0;
  bool operator==(const OverlayUsage&) const = default;
};
struct OverlayFile {
  std::string path;         // Volume-relative diagnostic path for the requested identity.
  std::uint64_t bytes = 0;  // Bytes, not UWF MB; zero does not prove absent or committed.
  bool operator==(const OverlayFile&) const = default;
};
// Per request/session/intent. Supported means an expressive entry exists, not
// that permissions/runtime conditions permit execution. Unknown needs execution
// to discover support. Unavailable is a transport/backend problem; Unsupported
// means that backend cannot safely express the requested contract.
enum class Support { Supported, Unsupported, Unavailable, Unknown };
// Effects describe submitted writes, not changed values. None means none
// submitted; Confirmed acknowledges writes, including earlier steps on failure.
// Uncertain is a failed, potentially submitted operation; matching state does not erase it.
enum class Effect { None, Confirmed, Uncertain };
// A failed read can leave expensive provider work running after its client
// stops waiting. Only NotDispatched permits another automatic dispatch.
enum class ReadCompletion { NotDispatched, ProviderCompleted, Unknown };
// NotFound is known target/configuration absence, not a fallback value for
// Unsupported, Unavailable, access/decoding failure, or a different session.
// Protocol denotes an invalid provider representation/response shape. Canceled
// alone does not prove no effect after dispatch; consult Failure.effect.
enum class ErrorKind { Unsupported, Unavailable, AccessDenied, NotFound, InvalidArgument, Canceled, ResourceExhausted, Conflict, Protocol, Failed };
struct Diagnostic {
  // Backend evidence for diagnostics. Branch on typed ErrorKind/Effect rather
  // than provider names, native status text, or arbitrary detail strings.
  std::string backend;
  std::string operation;
  std::int64_t code = 0;
  std::string detail;
};
struct Failure {
  ErrorKind kind = ErrorKind::Failed;
  Effect effect = Effect::None;
  std::string message;
  std::vector<Diagnostic> diagnostics;
  ReadCompletion readCompletion = ReadCompletion::NotDispatched;
};
[[nodiscard]] constexpr Effect combineEffects(const Effect first, const Effect second) {
  if (first == Effect::Uncertain || second == Effect::Uncertain) return Effect::Uncertain;
  return first == Effect::Confirmed || second == Effect::Confirmed ? Effect::Confirmed : Effect::None;
}
// An invalid observation cannot erase evidence that work was dispatched. Keep
// that evidence and its native diagnostics even when rejecting the protocol.
[[nodiscard]] inline Failure readOnlyEffectViolation(Failure original, const Effect observed, std::string message) {
  const auto effect = combineEffects(original.effect, observed);
  if (!original.message.empty() && original.message != message) original.diagnostics.push_back({{}, "ReadOnlyObservation", 0, original.message});
  original.kind = ErrorKind::Protocol;
  original.effect = effect;
  original.message = std::move(message);
  return original;
}
struct Capability {
  Support support = Support::Unknown;
  std::string reason;
};
// Strip conventional wrappers/trailing separators and lowercase text; this does not validate GUID syntax.
[[nodiscard]] std::string normalizeVolumeId(std::string value);
// Accept a bare GUID, {GUID}, Volume{GUID}, or device-prefixed volume path;
// return a lowercase bare GUID. Reject whitespace, NUL, malformed text, and
// extra separators/suffixes. This validates syntax, not volume existence.
[[nodiscard]] std::optional<std::string> parseVolumeId(std::string_view value);
// Conventional volume-path spelling; does not infer a drive letter.
[[nodiscard]] std::string volumePath(const std::string& id);
}  // namespace uwf::compat
