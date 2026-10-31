// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfOverlay.h"

#include <format>

#include "WriteVerification.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "shared/Log.h"

namespace uwf::app::provider {

namespace {

provider::OverlayRow decodeOverlay(const wmi::WmiRow& source) {
  return {wmi::row::requireString(source, "__PATH", wmi::row::EmptyString::Reject), wmi::row::requireUInt(source, "OverlayConsumption"),
          wmi::row::requireUInt(source, "AvailableSpace"), wmi::row::requireUInt(source, "CriticalOverlayThreshold"),
          wmi::row::requireUInt(source, "WarningOverlayThreshold")};
}

provider::OverlayRow rereadOverlay(wmi::WmiOperations& session, const provider::OverlayRow& target) { return decodeOverlay(session.getObject(target.path)); }

void requirePath(const provider::OverlayRow& row, const char* operation) {
  if (row.path.empty()) throw wmi::WmiProtocolError(operation, "UWF_Overlay row has no object path");
}

}  // namespace

provider::OverlayRow UwfOverlay::read() const {
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Overlay");
  if (rows.size() != 1) {
    throw wmi::WmiProtocolError("read UWF_Overlay", std::format("expected one instance, received {}", rows.size()));
  }

  const auto& o = rows.front();
  wmi::row::dumpRow("UWF_Overlay", o);
  auto r = decodeOverlay(o);
  UWF_LOG_D("uwf") << "overlay read completed: consumptionMb=" << r.overlayConsumption << " availableMb=" << r.availableSpace
                   << " warningMb=" << r.warningOverlayThreshold << " criticalMb=" << r.criticalOverlayThreshold;
  return r;
}

std::vector<provider::OverlayFileRow> UwfOverlay::getOverlayFiles(const provider::OverlayRow& row, const std::string& volume,
                                                                  const std::stop_token stopToken) const {
  requirePath(row, "read UWF overlay files");

  wmi::WmiRow inputs;
  inputs.emplace("Volume", wmi::WmiValue::fromString(volume));

  const auto output = stopToken.stop_possible() ? m_session.callMethodReadCancelable(row.path, "GetOverlayFiles", inputs, stopToken)
                                                : m_session.callMethodRead(row.path, "GetOverlayFiles", inputs);

  auto out = wmi::row::readArrayOutput<provider::OverlayFileRow>(output, "OverlayFiles", [](const wmi::WmiRow& item) {
    return provider::OverlayFileRow{wmi::row::requireEmbeddedString(item, "FileName"), wmi::row::requireEmbeddedUInt64(item, "FileSize")};
  });
  UWF_LOG_D("uwf") << "overlay files read completed: volume=" << volume << " files=" << out.size();
  return out;
}

void UwfOverlay::setWarningThreshold(const provider::OverlayRow& row, const uint32_t sizeMb) const {
  requirePath(row, "set UWF warning threshold");
  wmi::WmiRow inputs;
  inputs.emplace("size", wmi::WmiValue::fromUInt(sizeMb));
  invokeAndConfirm(
      "set UWF warning threshold", [&] { m_session.invokeMethod(row.path, "SetWarningThreshold", inputs); },
      [&] { return rereadOverlay(m_session, row).warningOverlayThreshold == sizeMb; });
}

void UwfOverlay::setCriticalThreshold(const provider::OverlayRow& row, const uint32_t sizeMb) const {
  requirePath(row, "set UWF critical threshold");
  wmi::WmiRow inputs;
  inputs.emplace("size", wmi::WmiValue::fromUInt(sizeMb));
  invokeAndConfirm(
      "set UWF critical threshold", [&] { m_session.invokeMethod(row.path, "SetCriticalThreshold", inputs); },
      [&] { return rereadOverlay(m_session, row).criticalOverlayThreshold == sizeMb; });
}

}  // namespace uwf::app::provider
