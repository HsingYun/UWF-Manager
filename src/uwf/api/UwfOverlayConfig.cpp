/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "UwfOverlayConfig.h"

#include <format>
#include <utility>

#include "../../util/Log.h"
#include "../wmi/WmiException.h"
#include "../wmi/WmiRowUtil.h"
#include "WriteVerification.h"

namespace uwf::api {

namespace {

api::OverlayConfigRow decodeOverlayConfig(const WmiRow& source) {
  const uint32_t type = rowutil::requireUInt(source, "Type");
  if (type > static_cast<uint32_t>(api::OverlayType::Disk)) {
    throw WmiDecodeError("decode UWF_OverlayConfig", std::format("Type has unsupported value {}", type));
  }
  return {rowutil::requireString(source, "__PATH", rowutil::EmptyString::Reject), rowutil::requireBool(source, "CurrentSession"),
          static_cast<api::OverlayType>(type), rowutil::requireUInt(source, "MaximumSize")};
}

api::OverlayConfigRow rereadOverlayConfig(WmiSession& session, const api::OverlayConfigRow& target) {
  auto observed = decodeOverlayConfig(session.getObject(target.path));
  if (observed.currentSession != target.currentSession) throw WmiProtocolError("reread UWF_OverlayConfig", "provider returned a different session instance");
  return observed;
}

void requirePath(const api::OverlayConfigRow& row, const char* operation) {
  if (row.path.empty()) throw WmiProtocolError(operation, "UWF_OverlayConfig row has no object path");
}

}  // namespace

std::vector<api::OverlayConfigRow> UwfOverlayConfig::readAll() const {
  std::vector<api::OverlayConfigRow> out;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_OverlayConfig");
  bool seenCurrent = false;
  bool seenNext = false;

  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_OverlayConfig", o);
    auto r = decodeOverlayConfig(o);
    bool& seen = r.currentSession ? seenCurrent : seenNext;
    if (seen) {
      throw WmiProtocolError("read UWF_OverlayConfig", std::format("duplicate {} session instances", r.currentSession ? "current" : "next"));
    }
    seen = true;
    out.push_back(std::move(r));
  }
  UWF_LOG_D("uwf") << "overlay configuration read completed: rows=" << rows.size();
  return out;
}

api::OverlayConfigRow UwfOverlayConfig::read(const api::Session session) const {
  const bool current = session == api::Session::Current;
  for (auto& r : readAll()) {
    if (r.currentSession == current) return r;
  }
  throw WmiProtocolError("read UWF_OverlayConfig", std::format("{} session instance is missing", current ? "current" : "next"));
}

void UwfOverlayConfig::setType(const api::OverlayConfigRow& row, const api::OverlayType type) const {
  requirePath(row, "set UWF overlay type");
  WmiRow inputs;
  inputs.emplace("type", WmiValue::fromUInt(static_cast<uint32_t>(type)));
  invokeAndConfirm("set UWF overlay type", [&] { m_session.invokeMethod(row.path, "SetType", inputs); },
                   [&] { return rereadOverlayConfig(m_session, row).type == type; });
}

void UwfOverlayConfig::setMaximumSize(const api::OverlayConfigRow& row, const uint32_t sizeMb) const {
  requirePath(row, "set UWF overlay maximum size");
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  invokeAndConfirm("set UWF overlay maximum size", [&] { m_session.invokeMethod(row.path, "SetMaximumSize", inputs); },
                   [&] { return rereadOverlayConfig(m_session, row).maximumSize == sizeMb; });
}

}  // namespace uwf::api
