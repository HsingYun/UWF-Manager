// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QStringList>
#include <stdexcept>

#include "app/ports/WmiOperations.h"
#include "app/state/ApplicationState.h"
#include "app/wmi/WmiException.h"
namespace uwf::test::application {
core::UwfSnapshot editableSnapshot();

class OverlayContractWmiOperations final : public uwf::app::wmi::WmiOperations {
 public:
  mutable core::OverlayConfig state;
  int failureMode = 0;
  mutable QStringList invocations;
  mutable bool invalidIntermediate = false;

  void ensureConnected() const override {}
  std::vector<uwf::app::wmi::WmiRow> query(const std::string& wql) const override {
    if (wql.find("UWF_OverlayConfig") != std::string::npos) return {getObject("config")};
    return {getObject("overlay")};
  }
  std::vector<uwf::app::wmi::WmiRow> queryInstances(const std::string& wql) const override { return query(wql); }
  uwf::app::wmi::WmiClassStatus classStatus(const std::string&) const override { return uwf::app::wmi::WmiClassStatus::Present; }
  uwf::app::wmi::WmiRow getObject(const std::string& path) const override {
    if (path == "config") {
      return {{"__PATH", uwf::app::wmi::WmiValue::fromString(path)},
              {"CurrentSession", uwf::app::wmi::WmiValue::fromBool(false)},
              {"Type", uwf::app::wmi::WmiValue::fromUInt(static_cast<uint32_t>(state.type))},
              {"MaximumSize", uwf::app::wmi::WmiValue::fromUInt(state.maximumSizeMb)}};
    }
    return {{"__PATH", uwf::app::wmi::WmiValue::fromString(path)},
            {"OverlayConsumption", uwf::app::wmi::WmiValue::fromUInt(0)},
            {"AvailableSpace", uwf::app::wmi::WmiValue::fromUInt(state.maximumSizeMb)},
            {"WarningOverlayThreshold", uwf::app::wmi::WmiValue::fromUInt(state.warningThresholdMb)},
            {"CriticalOverlayThreshold", uwf::app::wmi::WmiValue::fromUInt(state.criticalThresholdMb)}};
  }
  void invokeMethod(const std::string&, const std::string& method, const uwf::app::wmi::WmiRow& inputs) const override {
    invocations.append(QString::fromStdString(method));
    if (invocations.size() == 1) {
      if (failureMode == 1) throw uwf::app::wmi::WmiProviderError(5, method, "injected rejection");
      if (failureMode == 2) throw uwf::app::wmi::WmiInvocationUncertain(method, "injected uncertain result");
      if (failureMode == 4) return;
    }
    auto candidate = state;
    if (method == "SetWarningThreshold")
      candidate.warningThresholdMb = inputs.at("size").toUInt();
    else if (method == "SetCriticalThreshold")
      candidate.criticalThresholdMb = inputs.at("size").toUInt();
    else if (method == "SetMaximumSize")
      candidate.maximumSizeMb = inputs.at("size").toUInt();
    else if (method == "SetType")
      candidate.type = static_cast<core::OverlayType>(inputs.at("type").toUInt());
    else
      throw std::runtime_error("unexpected overlay method");
    const bool invalidThresholds =
        candidate.warningThresholdMb != 0 && candidate.criticalThresholdMb != 0 && candidate.warningThresholdMb >= candidate.criticalThresholdMb;
    const bool invalidStorage = candidate.type == core::OverlayType::Disk && (candidate.maximumSizeMb < 1024 || candidate.maximumSizeMb >= 3000);
    if (invalidThresholds || invalidStorage) {
      invalidIntermediate = true;
      throw uwf::app::wmi::WmiProviderError(0x80041008u, method, "intermediate state violates provider constraints");
    }
    state = candidate;
    if (invocations.size() == 1 && failureMode == 3) throw uwf::app::wmi::WmiInvocationUncertain(method, "injected lost acknowledgement");
  }
  uwf::app::wmi::WmiMethodOutput callMethodRead(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&) const override {
    throw std::runtime_error("unexpected overlay read method");
  }
  uwf::app::wmi::WmiMethodOutput callMethodReadCancelable(const std::string&, const std::string&, const uwf::app::wmi::WmiRow&,
                                                          std::stop_token) const override {
    throw std::runtime_error("unexpected cancelable overlay read");
  }
  void putInstance(const std::string&, const uwf::app::wmi::WmiRow&, uwf::app::wmi::WmiPutMode) const override {
    throw std::runtime_error("unexpected overlay putInstance");
  }
};

class MutableApplicationStateSource final : public app::ApplicationStateSource {
 public:
  std::vector<core::VolumeInfo> volumes;
  core::UwfSnapshot snapshot;
  QString failure;
  int reads = 0;

  ReadOperation readOperation(uwf::app::UwfCapability) override {
    ++reads;
    return [candidate = app::ApplicationState{volumes, snapshot}, error = failure](std::stop_token) {
      if (!error.isEmpty()) throw std::runtime_error(error.toStdString());
      return candidate;
    };
  }
};

}  // namespace uwf::test::application
