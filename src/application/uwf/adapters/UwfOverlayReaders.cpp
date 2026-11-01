// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/uwf/adapters/UwfOverlayReaders.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include "application/overlay/files/OverlayFilesFailure.h"
#include "application/uwf/adapters/UwfCall.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "domain/uwf/contracts/UwfStateFacts.h"
#include "domain/uwf/policies/VolumeIdentityPolicy.h"

namespace uwf::app {
namespace {
template <class T>
const T& requireObserved(const compat::Fact<T>& fact, const char* message) {
  if (!fact.value) throw OperationError(std::make_error_code(std::errc::state_not_recoverable), message);
  return *fact.value;
}
class UwfOverlayUsageReader final : public OverlayUsageReader {
 public:
  explicit UwfOverlayUsageReader(std::shared_ptr<compat::Uwf> system) : m_system(std::move(system)) {
    if (!m_system) throw std::invalid_argument("UWF compatibility service is required");
  }
  OverlayUsageState read() const override {
    const auto state = m_system->state();
    if (!requireObserved(state.current.filterEnabled, "The global Current filter state is unavailable")) return OverlayUsageDisabled{};
    const auto usage = callUwf(*m_system, compat::GetOverlayUsage{});
    const auto type = requireObserved(state.current.overlayType, "The global Current overlay type is unavailable");
    const core::OverlayRuntime runtime{.availableSpaceMb = usage.availableMb, .currentConsumptionMb = usage.consumptionMb};
    const core::OverlayConfig config{.type = overlayTypeFromUwf(type),
                                     .maximumSizeMb = requireObserved(state.current.maximumOverlaySize, "The global Current overlay capacity is unavailable"),
                                     .warningThresholdMb = requireObserved(state.runtime.warningThreshold, "The global warning threshold is unavailable"),
                                     .criticalThresholdMb = requireObserved(state.runtime.criticalThreshold, "The global critical threshold is unavailable")};
    return OverlayUsageEnabled{runtime, config};
  }

 private:
  std::shared_ptr<compat::Uwf> m_system;
};
}  // namespace

std::shared_ptr<OverlayUsageReader> uwfOverlayUsageReader(std::shared_ptr<compat::Uwf> system) {
  return std::make_shared<UwfOverlayUsageReader>(std::move(system));
}

QVector<OverlayFileEntry> loadUwfOverlayFiles(const compat::Uwf& system, const QString& volumeGuid, const std::stop_token stopToken) {
  if (stopToken.stop_requested()) return {};
  const auto volumeId = compat::parseVolumeId(volumeGuid.toStdString());
  if (!volumeId) throw std::invalid_argument("Overlay enumeration requires a complete volume GUID");
  const auto state = system.state();
  if (!state.current.volumesObserved)
    throw OperationError(std::make_error_code(std::errc::state_not_recoverable), "The global Current volume inventory is unavailable");
  bool present = false;
  for (const auto& [id, volume] : state.current.volumes) {
    if (volume.present.value == true && compat::parseVolumeId(id) == volumeId) present = true;
  }
  if (!present) throw std::invalid_argument("The requested volume is not present in the Current inventory");
  std::vector<compat::OverlayFile> files;
  try {
    files = compat::call(system, compat::GetOverlayFiles{*volumeId}, {stopToken});
  } catch (const compat::Error& error) {
    if (error.failure().readCompletion == compat::ReadCompletion::Unknown) throw OverlayFilesQueryUncertain(uwfDiagnostics(error));
    throwUwfOperationError(error);
  } catch (const std::exception& error) {
    throw OverlayFilesQueryUncertain(error.what());
  } catch (...) {
    throw OverlayFilesQueryUncertain("The enumeration client failed without completion evidence");
  }
  if (stopToken.stop_requested()) return {};
  if (files.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error("overlay file count exceeds the UI model capacity");
  QVector<OverlayFileEntry> result;
  result.reserve(static_cast<int>(files.size()));
  for (const auto& file : files) result.append({QString::fromStdString(file.path), {}, false, false, static_cast<qulonglong>(file.bytes)});
  return result;
}
}  // namespace uwf::app
