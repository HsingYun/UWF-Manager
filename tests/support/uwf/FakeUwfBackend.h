// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "domain/uwf/contracts/Backend.h"

namespace uwf::test {

// Captured UWF semantics for compatibility/application tests, without host filesystem or registry I/O.
// Data objects use Windows ordinal Unicode identity; management exclusion lists use captured CRT C-locale ASCII comparison.
class FakeUwfBackend final : public compat::Backend, public std::enable_shared_from_this<FakeUwfBackend> {
 public:
  enum class Storage { Durable, Overlay };
  // Checkpoints follow recovered worker order: After* records completed work; Before*/During* may reject the next step.
  enum class FaultTiming {
    Before,
    AfterDestinationEof,
    AfterFileDataChunk,
    AfterFileData,
    AfterFileEa,
    AfterFileAds,
    AfterFileTime,
    AfterFileSecurity,
    AfterFileAccessAdjusted,
    AfterVisibleDeletion,
    BeforePersistentDeletion,
    AfterPersistentDeletion,
    DuringDeleteCompensation,
    DuringDeletePermissionsRestore,
    AfterUwfNotification,
    BeforeOsPowerRequest,
    AfterEffect
  };
  enum class PersistentView { Unknown, NotReady, Ready };
  enum class RegistryHive { Unknown, Uncached, Ready };
  enum class SafetyObservation { Unknown, Rejected, Satisfied };
  struct FullCommitFilterOutcome {
    bool copyFails = false;
    // Explicit witnesses of completed copies on a failing filter. Other
    // addresses retain their old base, unless marked indeterminate below.
    std::set<std::string> completedFilePaths;
    std::set<std::string> indeterminateFilePaths;
    bool registryCopyCompleted = false;
    bool operator==(const FullCommitFilterOutcome&) const = default;
  };
  enum class SavedModeProfile { CapturedDll, Unsupported };
  enum class ServicingStatus : std::uint32_t { NotStarted = 0, Started = 1, Completed = 2, Failed = 3 };
  struct Profile {
    // Captured runtime reset accepts 0/1 on an available control device.
    // The absent-device branch ignores the resolved value and performs Next-type-dependent static setup.
    // The SavedMode wrapper accepts 1/2 while its static writer accepts 0/1; preserve both validation boundaries.
    SavedModeProfile savedMode = SavedModeProfile::CapturedDll;
  };
  struct VolumeConfiguration {
    compat::Volume volume;
    // Original management spelling; do not normalize trailing separators.
    std::vector<std::string> fileExclusions;
    std::uint32_t swapfileSize = 0;  // Captured raw host marker, not a physical file length.
    bool operator==(const VolumeConfiguration&) const = default;
  };
  struct Configuration {
    bool filterEnabled = false;
    bool servicingEnabled = false;
    bool persistDomainSecretKey = false;
    bool persistTscal = false;
    std::uint32_t overlayType = static_cast<std::uint32_t>(compat::OverlayType::Ram);
    std::uint32_t maximumOverlaySize = 1024;
    std::uint32_t warningThreshold = 512;
    std::uint32_t criticalThreshold = 768;
    std::uint32_t overlayFlags = 0;
    std::uint32_t resetPersistentStateSavedMode = 0;
    std::map<std::string, VolumeConfiguration> volumes;
    // Original management spelling; hive aliases identify separate entries.
    std::vector<std::string> registryExclusions;
    bool operator==(const Configuration&) const = default;
  };
  struct Runtime {
    bool shutdownPending = false;
    bool hormEnabled = false;
    std::uint32_t romDisposition = 0;
    // Public control-device cache and filter/header rollback state are distinct.
    std::uint32_t resetPersistentState = 0;
    std::uint32_t persistentRollbackState = 0;
    // A filter can fail its header write after changing its in-memory intent.
    // The global control handler swallows that status and retains its old cache.
    bool persistentResetHeaderWriteFails = false;
    // Per-filter observations override this default to model mixed or partial outcomes.
    bool fullCommitCopyFails = false;
    std::map<std::string, FullCommitFilterOutcome> fullCommitFilters;
    // Readiness is independent of mode flags; defaults model an available simulated environment.
    PersistentView defaultFileCommitView = PersistentView::Ready;
    std::map<std::string, PersistentView> fileCommitViews;
    RegistryHive registryCommitHive = RegistryHive::Uncached;
    SafetyObservation hormFixedVolumeSafety = SafetyObservation::Satisfied;
    SafetyObservation hormFinalSafety = SafetyObservation::Satisfied;
    // Captured IsPageFile checks aggregate PagingFiles configuration, not the target path.
    // Rejected/Unknown do not imply a pagefile-name blacklist.
    SafetyObservation pagingFilesValidation = SafetyObservation::Satisfied;
    // Unlisted volumes use an eligible fixed-media default; clearing it makes their eligibility unknown.
    std::optional<compat::VolumeAdmission> defaultVolumeEligibility = compat::VolumeAdmission{true, true};
    std::map<std::string, compat::VolumeAdmission> physicalVolumes;
    std::map<std::string, std::string> fileMaintenancePrefixes;
    // Override device presence independently; otherwise it follows the simulated filter state.
    std::optional<bool> controlDevicePresent;
    bool controlDeviceObservationAvailable = true;
    bool hiberfileExists = true;
    bool administrator = true;
    // Power privilege is independent of administrator membership; nullopt means unreadable.
    std::optional<bool> shutdownPrivilegeAvailable = true;
    // The servicing transition uses its own runtime status and saved Next
    // filter state; neither is implied by the static ServicingEnabled flag.
    ServicingStatus servicingStatus = ServicingStatus::NotStarted;
    std::optional<bool> servicingSavedFilterEnabled;
    bool servicingObservationAvailable = true;
    // Explicit environment facts; never inferred from the host or a C: label.
    std::uint32_t currentControlSet = 1;
    std::optional<std::string> systemVolumeId;
    std::optional<std::string> windowsDirectory;
    std::optional<compat::OverlayUsage> usageOverride;
    // Extent reports are independent of content/dirtiness; an empty vector is an explicit empty report.
    std::map<std::string, std::vector<compat::OverlayFile>> diagnosticCandidateReports;
    // Unseeded reports use cluster coverage, not content length; nullopt/0 supplies no usable cluster size.
    std::optional<std::uint32_t> diagnosticClusterSizeBytes = 4096;
    bool operator==(const Runtime&) const = default;
  };
  struct FileAddress {
    std::string volume;
    std::string path;
    bool operator==(const FileAddress&) const = default;
  };
  struct RegistryAddress {
    std::string key;
    std::string valueName;
    bool operator==(const RegistryAddress&) const = default;
  };
  struct FileAddressLess {
    bool operator()(const FileAddress& left, const FileAddress& right) const;
  };
  struct RegistryAddressLess {
    bool operator()(const RegistryAddress& left, const RegistryAddress& right) const;
  };
  struct RegistryKeyLess {
    bool operator()(const std::string& left, const std::string& right) const;
  };
  struct Data {
    std::map<FileAddress, std::string, FileAddressLess> files;
    std::map<RegistryAddress, std::string, RegistryAddressLess> registry;
    std::set<FileAddress, FileAddressLess> directories;
    std::set<std::string, RegistryKeyLess> registryKeys;
    bool operator==(const Data&) const = default;
  };
  struct OverlayData {
    // nullopt is a deletion, rather than an absent overlay entry.
    std::map<FileAddress, std::optional<std::string>, FileAddressLess> files;
    std::map<RegistryAddress, std::optional<std::string>, RegistryAddressLess> registry;
    std::map<FileAddress, bool, FileAddressLess> directories;
    std::map<std::string, bool, RegistryKeyLess> registryKeys;
    bool operator==(const OverlayData&) const = default;
  };
  struct State {
    Configuration current;
    Configuration next;
    std::optional<Configuration> capturedSettings = Configuration{};
    Runtime runtime;
    Data durable;
    OverlayData overlay;
    struct PartialFile {
      // nullopt means a cluster-copy failure does not establish even EOF.
      std::optional<std::size_t> length;
      std::string completedPrefix;
      bool operator==(const PartialFile&) const = default;
    };
    // readFile rejects exact-content reads while a persistent suffix remains indeterminate.
    std::map<FileAddress, PartialFile, FileAddressLess> partialPersistentFiles;
    std::map<FileAddress, std::vector<FaultTiming>, FileAddressLess> fileCommitStages;
    struct DeletionProgress {
      bool visibleDeleted = false;
      bool persistentDeleted = false;
      bool compensationAttempted = false;
      bool compensationSucceeded = false;
      bool permissionsRestoreAttempted = false;
      bool permissionsRestoreSucceeded = false;
      bool operator==(const DeletionProgress&) const = default;
    };
    std::map<FileAddress, DeletionProgress, FileAddressLess> fileDeletions;
    std::vector<compat::PowerAction> powerRequests;
    std::uint32_t windowsUpdates = 0;
    compat::WindowsUpdateOutcome windowsUpdateOutcome = compat::WindowsUpdateOutcome::Completed;
    std::uint32_t restarts = 0;
    bool operator==(const State&) const = default;
  };
  struct Fault {
    std::optional<compat::Request> request;
    std::string operation;
    FaultTiming timing = FaultTiming::Before;
    compat::ErrorKind kind = compat::ErrorKind::Failed;
    std::string message = "Injected UWF failure";
    std::int64_t code = 0;
  };
  struct TraceEntry {
    compat::Request request;
    compat::Response response;
    bool capturedPrimitive = false;
  };
  struct StageEntry {
    compat::Request request;
    FaultTiming stage;
  };

  class System;
  using SystemHandle = std::shared_ptr<System>;
  [[nodiscard]] static SystemHandle createSystem();
  [[nodiscard]] static SystemHandle createSystem(State state);

  explicit FakeUwfBackend(std::string name = "FakeUWF");
  explicit FakeUwfBackend(State state, std::string name = "FakeUWF");
  FakeUwfBackend(State state, std::string name, Profile profile);
  FakeUwfBackend(SystemHandle system, std::string name);
  FakeUwfBackend(SystemHandle system, std::string name, Profile profile);
  [[nodiscard]] std::string name() const override;
  [[nodiscard]] compat::Capability probe(const compat::Request& request) const override;
  [[nodiscard]] compat::Response execute(const compat::Request& request, const compat::Context& context = {}) const override;
  // A shared Fake can expose its literal provider boundary to a separately
  // composed UWF abstraction; the view retains this fixture and its trace.
  [[nodiscard]] std::shared_ptr<compat::Backend> rawBackend() const;
  // Explicit raw-profile seam: bypasses the public command/admission layer.
  // Only recovered primitive/no-op/readiness tests should use this method.
  [[nodiscard]] compat::Response executeCapturedPrimitiveForTest(const compat::Request& request, const compat::Context& context = {}) const;

  void seed(State state);
  [[nodiscard]] State snapshot() const;
  [[nodiscard]] compat::UwfStateSnapshot stateSnapshot() const override;
  void updateState(compat::UwfStateFacts facts) const override;
  void simulateRestart();

  void seedFile(std::string volume, std::string path, std::string contents, Storage storage = Storage::Durable);
  void seedDirectory(std::string volume, std::string path, Storage storage = Storage::Durable);
  void seedRegistryKey(std::string key, Storage storage = Storage::Durable);
  void seedRegistryValue(std::string key, std::string valueName, std::string contents, Storage storage = Storage::Durable);
  void writeFile(std::string volume, std::string path, std::string contents);
  void deleteFile(std::string volume, std::string path);
  // Ordinary host key creation, including recreation after a visible deletion.
  // Unlike seedRegistryKey, this follows the current protection/exclusion law.
  void createRegistryKey(std::string key);
  void writeRegistryValue(std::string key, std::string valueName, std::string contents);
  void deleteRegistryValue(std::string key, std::string valueName);
  void deleteRegistryKey(std::string key);
  [[nodiscard]] std::optional<std::string> readFile(std::string volume, std::string path, Storage storage = Storage::Durable) const;
  [[nodiscard]] std::optional<std::string> readRegistryValue(std::string key, std::string valueName, Storage storage = Storage::Durable) const;
  [[nodiscard]] std::optional<std::string> visibleFile(std::string volume, std::string path) const;
  [[nodiscard]] bool visibleDirectory(std::string volume, std::string path) const;
  [[nodiscard]] bool visibleRegistryKey(std::string key) const;
  [[nodiscard]] std::optional<std::string> visibleRegistryValue(std::string key, std::string valueName) const;

  // Exact-request overrides precede operation-name overrides; neither bypasses shared command constraints.
  void setCapability(compat::Request request, compat::Capability capability);
  void setOperationCapability(std::string operation, compat::Capability capability);
  void clearCapabilityOverrides();
  // Faults are consumed once, at the requested phase of a matching call.
  void inject(Fault fault);
  void clearFaults();
  [[nodiscard]] std::vector<TraceEntry> trace() const;
  void clearTrace();
  // Record raw provider requests to distinguish pre-dispatch rejection from dispatched no-ops.
  [[nodiscard]] std::vector<compat::Request> primitiveRequests() const;
  void clearPrimitiveRequests();
  [[nodiscard]] std::vector<StageEntry> stages() const;
  void clearStages();

 private:
  enum class ReceiptSurface { Portable, Captured };
  class RawUwf;
  [[nodiscard]] compat::Response executeImpl(const compat::Request& request, const compat::Context& context, bool compose) const;
  [[nodiscard]] compat::UwfStateFacts requestFacts(const compat::Request& request) const;
  [[nodiscard]] compat::Capability probeRaw(const compat::Request& request) const;
  [[nodiscard]] compat::Response executeRaw(const compat::Request& request, const compat::Context& context,
                                            ReceiptSurface surface = ReceiptSurface::Portable) const;
  [[nodiscard]] compat::Response writeSwapfile(const std::string& volume, std::uint32_t value, const compat::Context& context,
                                               ReceiptSurface surface = ReceiptSurface::Portable) const;
  [[nodiscard]] compat::Capability capability(const compat::Request& request) const;
  [[nodiscard]] std::optional<Fault> takeFault(const compat::Request& request, FaultTiming timing) const;
  std::string m_name;
  Profile m_profile;
  SystemHandle m_system;
  std::vector<std::pair<compat::Request, compat::Capability>> m_capabilities;
  std::map<std::string, compat::Capability> m_operationCapabilities;
  mutable std::vector<Fault> m_faults;
  mutable std::vector<TraceEntry> m_trace;
  mutable std::vector<compat::Request> m_primitiveRequests;
  mutable std::vector<StageEntry> m_stages;
};
}  // namespace uwf::test
