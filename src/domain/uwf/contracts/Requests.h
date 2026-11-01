// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <variant>

#include "domain/uwf/contracts/Types.h"

namespace uwf::compat {
// Execution contract:
// - Backends execute their own native entry points and return their results.
//   Prechecks use shared in-memory state. Cross-capability operations are composed
//   by the UWF abstraction; a backend must not issue extra state queries.
// - Configuration writes target Next unless explicitly global/runtime. Current
//   remains loaded until restart. Explicit refreshes and successful writes update
//   shared state. Batches are not transactions. No request implicitly reboots.
// - Write receipts come from the invoked entry point. No setter implicitly calls
//   a getter for confirmation. A matching getter cannot repair a failed step.
// - Effect::None means no mutation was submitted. Confirmed records acknowledged
//   writes, including earlier steps of a failed operation. Failed opaque writes
//   and lost native completion receipts are Uncertain; do not replay them
//   through another provider. Internal phase knowledge does not weaken this rule.
// - Unsupported means the provider cannot express the contract. Unavailable,
//   missing targets, and failed prerequisites remain distinct. A read must not
//   substitute another session or convert unknown state to false/zero.
// - Strings are UTF-8. Validate volume identities with parseVolumeId before
//   provider probing; malformed identities are InvalidArgument/None. File targets
//   paired with a volume are rooted volume-relative paths. Registry value names
//   are exact names; absent, default, and named values remain distinct.
// - Collections sort deterministically: volumes by canonical GUID; exclusions
//   and diagnostic paths by ASCII-folded bytes, then original bytes. Diagnostic
//   path ties sort by size. Preserve spelling, duplicates, and optional metadata.
// - Stricter editor rules are application policy, not additional backend gates.

// Honor BooleanProperty scope; unsupported sessions and failed reads are errors.
// OverlayCommit is false without a raw disposition read when Current UWF is off.
// RegistryChangesAlwaysPersisted derives Current filter and actual system-volume
// protection from shared state; only known absence means unprotected.
struct GetBoolean {
  using Result = bool;
  BooleanProperty property;
  Session session = Session::Current;
  bool operator==(const GetBoolean&) const = default;
};
struct SetBoolean {
  // FilterEnabled: Next; administrator and HORM off. Matching Next is a no-op
  // after admission. Changes include reset/boot configuration and, when Current
  // is on, runtime ROM disposition before writing Next. Later failure may be partial.
  // HormEnabled: administrator; matching state is a no-op. Enable requires Current
  // UWF on, RAM, flags exactly ROM, hibernation, protected mounted fixed volumes,
  // and compatible runtime disposition. Disable also needs a readable control
  // device and does not clear configured ROM or guarantee FullCommit admission.
  // ServicingEnabled: Next flag plus complete servicing transition; administrator
  // and HORM off. Matching flags do not skip the workflow. Entry saves and disables
  // Next Filter; exit restores it only if previously enabled. Runtime status is
  // separate, and its receipt does not independently audit all restoration steps.
  // PersistDomainSecretKey/PersistTscal update Next built-in persistence data,
  // not user exclusions. ShutdownPending/RegistryChangesAlwaysPersisted are read-only.
  // OverlayCommit: true-only global FullCommit request. Published Current UWF off
  // rejects Conflict/None before dispatch; an existing ROM request is a no-op.
  // Without loaded ROM, the driver may accept a no-op and still report false.
  // ROM disposition 0 permits the request; incompatible nonzero state rejects it.
  // Acceptance is not a cluster-durability audit. FullCommit lasts until restart,
  // makes later writes pass through, and does not enable individual commits.
  using Result = std::monostate;
  BooleanProperty property;
  bool value;
  bool operator==(const SetBoolean&) const = default;
};
// Honor NumberProperty scope and units. OverlayFlags retains opaque bits.
// Reset reads runtime control state, not SavedMode or selector 255. Confirmed
// control-device absence has reset default 0; other read failures remain errors.
struct GetNumber {
  using Result = std::uint32_t;
  NumberProperty property;
  Session session = Session::Current;
  bool operator==(const GetNumber&) const = default;
};
struct SetNumber {
  // OverlayType: Next RAM/Disk; administrator, HORM off, Current UWF off, and
  // valid Next capacity. A transition to Disk creates backing storage and sets
  // Passthrough; RAM removes storage and clears Passthrough/Persistent. Other bits
  // survive. Same-type commands still require admission but do not normalize flags.
  // MaximumOverlaySize: positive Next MB; Disk requires at least 1024 MB.
  // Administrator, HORM off, and Current UWF off are required, including no-ops.
  // Disk storage is resized; threshold inconsistencies warn without rewriting thresholds.
  // WarningThreshold/CriticalThreshold: global live and saved MB settings;
  // administrator and HORM off. Zero or inconsistent values may succeed with
  // warnings. Failure after saving but before the live update can be partial.
  // OverlayFlags is the raw write primitive used by abstract-layer option
  // composition. Public callers use SetOverlayOption instead of writing all flags.
  // ResetPersistentState: runtime intent; administrator, HORM off, active Current
  // persistent Disk, and a readable control device, including Preserve requests.
  // Clear schedules one cleanup at startup. Saved resolves Current SavedMode
  // internally to 0/1; no separately exposed getter or literal-255 readback is required.
  // Only Preserve=0, Clear=1, and Saved=255 are writable; other inputs reject before
  // provider discovery. Missing devices reject before any native fallback mutation.
  // ResetPersistentStateSavedMode: Next default; only Clear=1 is portably writable.
  // Other raw defaults remain readable. Administrator and HORM off are required;
  // this changes neither the runtime reset request nor current overlay contents.
  using Result = std::monostate;
  NumberProperty property;
  std::uint32_t value;
  bool operator==(const SetNumber&) const = default;
};
struct SetOverlayOption {
  // Next single-bit intent composed from complete published Next flags into one
  // raw flags write. Preserve unrelated input bits; the invoked native entry
  // point retains its intrinsic linked effects and boot-persistence steps.
  // No getter is called to fill missing flags or confirm the write.
  // All options require Current UWF off.
  // Passthrough and Persistent also require administrator and HORM off, including same-value commands.
  // No implicit type change or Current toggle. Apply explicit type changes first.
  using Result = std::monostate;
  OverlayOption option;
  bool value;
  bool operator==(const SetOverlayOption&) const = default;
};
struct GetOverlayUsage {
  // Current overlay usage in UWF MB, independent of filesystem free space and
  // Next capacity. Current UWF off returns 0/0 without an active-provider read.
  using Result = OverlayUsage;
  bool operator==(const GetOverlayUsage&) const = default;
};
struct GetVolumes {
  // Complete configured inventory for the requested session, not physical disks.
  // Empty is valid; incomplete inventory is Unsupported. Optional drive-letter
  // and commitPending metadata may be absent.
  using Result = std::vector<Volume>;
  Session session = Session::Current;
  bool operator==(const GetVolumes&) const = default;
};
struct AddVolume {
  // Create an unprotected, identity-bound Next registration. Shared admission
  // uses published membership and physical eligibility; known existing membership
  // rejects Conflict/None. The raw API supplies its own result without existence
  // probes or readback. Optional driveLetter is creation context, not proof of
  // volume existence or protection.
  using Result = std::monostate;
  std::string volume;
  std::string driveLetter;
  bool operator==(const AddVolume&) const = default;
};
struct RemoveVolume {
  // Remove the Next registration and its exclusions/metadata, leaving Current
  // and the physical disk unchanged. Published missing membership rejects
  // NotFound/None; otherwise invoke the removal capability without pre/post getters.
  using Result = std::monostate;
  std::string volume;
  bool operator==(const RemoveVolume&) const = default;
};
struct SetVolume {
  // Next protection requires administrator and HORM off. true creates a missing
  // registration with bindByDriveLetterOnCreate; false on missing is NotFound.
  // Matching existing protection is a no-op after admission and preserves binding.
  // Binding edits require an existing registration and appropriate physical metadata.
  // driveLetter is creation context only. No Current protection or data commit occurs.
  using Result = std::monostate;
  std::string volume;
  VolumeProperty property;
  bool value;
  std::string driveLetter{};
  bool bindByDriveLetterOnCreate = false;
  bool operator==(const SetVolume&) const = default;
};
struct GetVolumeProperty {
  // Query the exact registration/session. Preserve native lookup errors;
  // malformed returned fields are Protocol. Neither is a false property value.
  using Result = bool;
  std::string volume;
  VolumeProperty property;
  Session session = Session::Current;
  bool operator==(const GetVolumeProperty&) const = default;
};
// Embedded NUL in file/registry targets is Unsupported/None before provider
// probing because native C-string APIs would address a truncated name.
// Clear ignores its unused target. Empty registry value names remain distinct
// from absent names as specified by each operation.
struct GetFileExclusions {
  // Original user-exclusion spellings for one registration/session. Preserve
  // volume-relative paths; do not merge implicit exclusions or infer ROM/HORM coverage.
  using Result = std::vector<std::string>;
  std::string volume;
  Session session = Session::Current;
  bool operator==(const GetFileExclusions&) const = default;
};
struct EditFileExclusion {
  // Next user list; administrator and HORM off. Add requires a registered Next
  // volume, which need not be protected. Add-existing/Remove-absent are exact-entry
  // no-ops, using original spelling rather than descendant coverage.
  // Before dispatch or cached duplicate lookup, Add strictly decodes UTF-8 and applies
  // fileExclusionError to the original pattern. Invalid syntax is InvalidArgument/None.
  // Only the exact device prefix changes the validation length limit; preserve
  // that prefix, separators, dot segments, whitespace, and U+001F in saved data.
  // Find/Remove may address existing raw entries that new Add would reject.
  // Provider reserved-target checks still apply; object existence is not required.
  // Clear snapshots the published Next list and removes exact entries through
  // the abstraction layer; empty Clear has no effect. Later
  // failures retain earlier effects. No physical file or Current list is changed.
  using Result = std::monostate;
  std::string volume;
  std::string path;
  ExclusionAction action;
  bool operator==(const EditFileExclusion&) const = default;
};
struct FindFileExclusion {
  // Exact management-list identity: ASCII case-insensitive, preserving non-ASCII
  // bytes and trailing separators. This does not test driver coverage. false is
  // confirmed entry absence; an unregistered volume may instead report NotFound.
  using Result = bool;
  std::string volume;
  std::string path;
  Session session = Session::Current;
  bool operator==(const FindFileExclusion&) const = default;
};
struct GetRegistryExclusions {
  // Session-scoped user exclusions, global across volumes. Preserve hive spelling
  // and keep built-in persistence entries separate. Membership does not prove runtime coverage.
  using Result = std::vector<std::string>;
  Session session = Session::Current;
  bool operator==(const GetRegistryExclusions&) const = default;
};
struct EditRegistryExclusion {
  // Next user list; administrator and HORM off. Before dispatch or cached duplicate lookup,
  // Add strictly decodes UTF-8 and applies registryExclusionError to the original
  // HKLM subkey. Invalid schema is InvalidArgument/None, including existing invalid entries.
  // Reserved keys are exact targets, not subtrees. Machine-account routing is
  // editor policy. Preserve hive spelling, repeated/trailing separators, and controls.
  // Add-existing/Remove-absent are exact-entry no-ops. Clear shares the snapshot
  // and partial-effect rules of EditFileExclusion; built-in persistence and
  // Current exclusions remain unchanged. No physical keys or values are deleted.
  using Result = std::monostate;
  std::string key;
  ExclusionAction action;
  bool operator==(const EditRegistryExclusion&) const = default;
};
struct FindRegistryExclusion {
  // Exact management-list identity/session, not coverage. Fold ASCII case only;
  // hive aliases and non-ASCII spellings remain distinct. false is confirmed absence.
  using Result = bool;
  std::string key;
  Session session = Session::Current;
  bool operator==(const FindRegistryExclusion&) const = default;
};
// Individual maintenance admission uses published Current filter and exact target
// protection. Registry protection belongs to the actual system volume. Active
// protected targets reject known HORM and loaded Current ROM before mutation;
// Next-only ROM and FullCommit do not change this rule. Shared observations may
// come from different capabilities or providers. Missing facts are never filled
// by implicit getters; each selected raw API retains its own native validation.
// Already-persistent commits retain their no-op rules.
struct CommitFile {
  // Commit one visible Current file, not a directory or tree. A Current registration
  // is required even for already-persistent no-ops. Before no-op checks, validate
  // published full-target prefix/path facts against the 260-unit UTF-16 buffer
  // including NUL. The entry point retains its intrinsic native buffer checks.
  // Reserved files may reject; missing files are NotFound. Persistent or management-
  // excluded files may acknowledge a no-op. Success is a commit receipt, not an
  // independent disk audit or guarantee about subsequent writes.
  using Result = std::monostate;
  std::string volume;
  std::string path;
  bool operator==(const CommitFile&) const = default;
};
struct CommitFileDeletion {
  // Delete and persist one Current file or empty directory on an active protected
  // volume outside management exclusions. Nonrecursive; process children first.
  // The same resolved-prefix limits as CommitFile apply. A driver-only exclusion
  // may make deleting the source itself persistent, but does not waive the required
  // backing view. Dispatch despite preliminary absence; providers may return NotFound.
  // Visible deletion can precede persistent deletion, so later failure is Uncertain
  // and visible absence cannot repair it.
  using Result = std::monostate;
  std::string volume;
  std::string path;
  bool operator==(const CommitFileDeletion&) const = default;
};
struct CommitRegistry {
  // Commit one exact Current HKLM value. Empty valueName selects the default value;
  // nullopt whole-key commits are Unsupported. Native validation/packing limits apply.
  // Persistent or management-excluded targets may acknowledge a no-op; missing
  // values are NotFound. Protection belongs to the system volume, not another disk.
  // The receipt is not an independent hive/disk audit.
  // Reject HKLM\SYSTEM\CurrentControlSetExtra and similar same-component suffixes
  // as Unsupported/None before probing: the native prefix rewrite would address
  // ControlSetNNNExtra instead. Exact CurrentControlSet components remain valid.
  // Exclusion patterns follow their separate configuration contract.
  using Result = std::monostate;
  std::string key;
  std::optional<std::string> valueName;
  bool operator==(const CommitRegistry&) const = default;
};
struct CommitRegistryDeletion {
  // Delete and persist one Current target. nullopt selects a nonrecursive key;
  // a nonempty name selects a value. Empty value names are Unsupported because
  // the native deletion protocol cannot distinguish default-value deletion safely.
  // Values require HKLM. Keys use the distinct RegistryCommitPolicy blacklist,
  // including its requested-length prefix comparison, not exclusion Add rules.
  // Require active registry protection outside management exclusions and a loaded
  // persistent hive. Already-persistent targets are refused; this is not host deletion.
  // Driver-only exclusions may make source deletion durable but are not management skips.
  // Dispatch despite preliminary absence. Visible deletion can precede base-hive
  // deletion; later absence cannot repair failure or justify replay after Uncertain.
  // CommitRegistry's CurrentControlSet exact-target restriction also applies here.
  using Result = std::monostate;
  std::string key;
  std::optional<std::string> valueName;
  bool operator==(const CommitRegistryDeletion&) const = default;
};
struct GetOverlayFiles {
  // Diagnostic pending files for one active protected Current registration, not
  // an exhaustive commit checklist. Sizes are bytes. Empty is valid; unsupported
  // enumeration must remain an error.
  using Result = std::vector<OverlayFile>;
  std::string volume;
  bool operator==(const GetOverlayFiles&) const = default;
};
struct GetSwapfileSize {
  // Configured host marker, not file length or overlay capacity. CreateSwapfile
  // writes marker 1. A missing field may default to 0 only on a known registration;
  // a missing registration is NotFound.
  using Result = std::uint32_t;
  std::string volume;
  Session session = Session::Current;
  bool operator==(const GetSwapfileSize&) const = default;
};
struct CreateSwapfile {
  // Select one Next swapfile host. The abstraction layer creates an unprotected
  // registration with the requested binding if needed, then invokes the raw
  // marker-1 capability. Its native sequence clears other host markers and
  // leaves Next Disk; no extra getter confirms these acknowledged effects.
  // The native RAM-to-Disk sequence inherits type-transition flag effects;
  // reselecting the same host is not a no-op and may clear Persistent.
  // Require Current UWF off. The primitive uses the existing Next capacity;
  // this marker operation does not impose the CLI type/size editing limits.
  // Registration/binding may precede a later failure. No rollback, data
  // migration, or physical-file length verification is promised.
  using Result = std::monostate;
  std::string volume;
  std::string driveLetter{};
  bool bindByDriveLetterOnCreate = false;
  bool operator==(const CreateSwapfile&) const = default;
};
struct ResetSettings {
  // Restore platform defaults; administrator, HORM off, Current UWF off, and
  // verified captured defaults are required before dispatch. This is not an OS
  // reset or data rollback. Failure may retain partial configuration effects.
  using Result = std::monostate;
  bool operator==(const ResetSettings&) const = default;
};
struct UpdateWindows {
  // The workflow value may be Failed or RebootRequired even after successful
  // invocation; neither is transport failure or an instruction to reboot.
  // Missing update support is Unsupported, not Completed.
  using Result = WindowsUpdateOutcome;
  bool operator==(const UpdateWindows&) const = default;
};
struct Power {
  // Acknowledge a UWF-aware power request, not completed shutdown. Staging commits
  // belong to the application. Current UWF on requires a successful control-channel
  // notification before OS power; off requires no control device and leaves its
  // notification state unchanged. Error/cancellation after dispatch does not prove cancellation.
  using Result = std::monostate;
  PowerAction action;
  bool operator==(const Power&) const = default;
};

using Request = std::variant<GetBoolean, SetBoolean, GetNumber, SetNumber, SetOverlayOption, GetOverlayUsage, GetVolumes, AddVolume, RemoveVolume, SetVolume,
                             GetVolumeProperty, GetFileExclusions, EditFileExclusion, FindFileExclusion, GetRegistryExclusions, EditRegistryExclusion,
                             FindRegistryExclusion, CommitFile, CommitFileDeletion, CommitRegistry, CommitRegistryDeletion, GetOverlayFiles, GetSwapfileSize,
                             CreateSwapfile, ResetSettings, UpdateWindows, Power>;
using Value = std::variant<std::monostate, bool, std::uint32_t, OverlayUsage, std::vector<Volume>, std::vector<std::string>, std::vector<OverlayFile>,
                           WindowsUpdateOutcome>;
[[nodiscard]] std::string operationName(const Request& request);
[[nodiscard]] bool isWrite(const Request& request);
[[nodiscard]] bool validValue(const Request& request, const Value& value);
// Validate exact output strings and inventory identities before a value crosses
// the abstraction boundary. Successful volume IDs have canonical GUID spelling.
[[nodiscard]] std::optional<Failure> normalizeValue(const Request& request, Value& value);
// Provider-independent admission before capability probing, including routing.
// This checks exact input identity, not provider availability or Current state.
[[nodiscard]] std::optional<Failure> requestInputFailure(const Request& request);
}  // namespace uwf::compat
