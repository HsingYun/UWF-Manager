// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfOverlayConfig.h"

#include <format>
#include <utility>

#include "WriteVerification.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "shared/Log.h"

namespace uwf::app::provider {

namespace {

provider::OverlayConfigRow decodeOverlayConfig(const wmi::WmiRow& source) {
  const uint32_t type = wmi::row::requireUInt(source, "Type");
  if (type > static_cast<uint32_t>(provider::OverlayType::Disk)) {
    throw wmi::WmiDecodeError("decode UWF_OverlayConfig", std::format("Type has unsupported value {}", type));
  }
  return {wmi::row::requireString(source, "__PATH", wmi::row::EmptyString::Reject), wmi::row::requireBool(source, "CurrentSession"),
          static_cast<provider::OverlayType>(type), wmi::row::requireUInt(source, "MaximumSize")};
}

provider::OverlayConfigRow rereadOverlayConfig(wmi::WmiOperations& session, const provider::OverlayConfigRow& target) {
  auto observed = decodeOverlayConfig(session.getObject(target.path));
  if (observed.currentSession != target.currentSession)
    throw wmi::WmiProtocolError("reread UWF_OverlayConfig", "provider returned a different session instance");
  return observed;
}

void requirePath(const provider::OverlayConfigRow& row, const char* operation) {
  if (row.path.empty()) throw wmi::WmiProtocolError(operation, "UWF_OverlayConfig row has no object path");
}

}  // namespace

std::vector<provider::OverlayConfigRow> UwfOverlayConfig::readAll() const {
  std::vector<provider::OverlayConfigRow> out;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_OverlayConfig");
  bool seenCurrent = false;
  bool seenNext = false;

  out.reserve(rows.size());
  for (const auto& o : rows) {
    wmi::row::dumpRow("UWF_OverlayConfig", o);
    auto r = decodeOverlayConfig(o);
    bool& seen = r.currentSession ? seenCurrent : seenNext;
    if (seen) {
      throw wmi::WmiProtocolError("read UWF_OverlayConfig", std::format("duplicate {} session instances", r.currentSession ? "current" : "next"));
    }
    seen = true;
    out.push_back(std::move(r));
  }
  UWF_LOG_D("uwf") << "overlay configuration read completed: rows=" << rows.size();
  return out;
}

provider::OverlayConfigRow UwfOverlayConfig::read(const provider::Session session) const {
  const bool current = session == provider::Session::Current;
  for (auto& r : readAll()) {
    if (r.currentSession == current) return r;
  }
  throw wmi::WmiProtocolError("read UWF_OverlayConfig", std::format("{} session instance is missing", current ? "current" : "next"));
}

void UwfOverlayConfig::setType(const provider::OverlayConfigRow& row, const provider::OverlayType type) const {
  requirePath(row, "set UWF overlay type");
  wmi::WmiRow inputs;
  inputs.emplace("type", wmi::WmiValue::fromUInt(static_cast<uint32_t>(type)));
  invokeAndConfirm(
      "set UWF overlay type", [&] { m_session.invokeMethod(row.path, "SetType", inputs); }, [&] { return rereadOverlayConfig(m_session, row).type == type; });
}

void UwfOverlayConfig::setMaximumSize(const provider::OverlayConfigRow& row, const uint32_t sizeMb) const {
  requirePath(row, "set UWF overlay maximum size");
  wmi::WmiRow inputs;
  inputs.emplace("size", wmi::WmiValue::fromUInt(sizeMb));
  invokeAndConfirm(
      "set UWF overlay maximum size", [&] { m_session.invokeMethod(row.path, "SetMaximumSize", inputs); },
      [&] { return rereadOverlayConfig(m_session, row).maximumSize == sizeMb; });
}

}  // namespace uwf::app::provider
