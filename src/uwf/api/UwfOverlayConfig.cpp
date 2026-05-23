#include "UwfOverlayConfig.h"

#include <format>

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf {

std::vector<api::OverlayConfigRow> UwfOverlayConfig::readAll(std::string* error) const {
  std::vector<api::OverlayConfigRow> out;
  const auto rows = m_session.query("SELECT * FROM UWF_OverlayConfig", error);

  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_OverlayConfig", o);
    api::OverlayConfigRow r;
    r.path = rowutil::getString(o, "__PATH");
    r.currentSession = rowutil::getBool(o, "CurrentSession");
    r.type = static_cast<api::OverlayType>(rowutil::getUInt(o, "Type", 0));
    r.maximumSize = rowutil::getInt(o, "MaximumSize");
    out.push_back(std::move(r));
  }
  UWF_LOG_I("UWF_OverlayConfig") << std::format("readAll ok: rows={}", rows.size());
  return out;
}

std::optional<api::OverlayConfigRow> UwfOverlayConfig::read(bool currentSession, std::string* error) const {
  for (auto& r : readAll(error)) {
    if (r.currentSession == currentSession) return r;
  }
  return std::nullopt;
}

WmiResult UwfOverlayConfig::setType(const api::OverlayConfigRow& row, api::OverlayType type) const {
  if (row.path.empty()) return WmiResult::failed("UWF_OverlayConfig row has empty __PATH; call read() first");
  WmiRow inputs;
  inputs.emplace("type", WmiValue::fromUInt(static_cast<uint32_t>(type)));
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, "SetType", inputs));
  if (out.ok) {
    UWF_LOG_I("UWF_OverlayConfig") << std::format("SetType ok: type={} ({})", static_cast<uint32_t>(type), type == api::OverlayType::Disk ? "Disk" : "RAM");
  }
  return out;
}

WmiResult UwfOverlayConfig::setMaximumSize(const api::OverlayConfigRow& row, uint32_t sizeMb) const {
  if (row.path.empty()) return WmiResult::failed("UWF_OverlayConfig row has empty __PATH; call read() first");
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, "SetMaximumSize", inputs));
  if (out.ok) UWF_LOG_I("UWF_OverlayConfig") << "SetMaximumSize ok: size=" << sizeMb << "MB";
  return out;
}

}  // namespace uwf
