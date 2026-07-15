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

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf::api {

namespace {

std::optional<api::OverlayConfigRow> decodeOverlayConfig(const WmiRow& source, std::string* error) {
  const auto path = rowutil::requireString(source, "__PATH", error, false);
  const auto currentSession = rowutil::requireBool(source, "CurrentSession", error);
  const auto type = rowutil::requireUInt(source, "Type", error);
  const auto maximumSize = rowutil::requireUInt(source, "MaximumSize", error);
  if (!path || !currentSession || !type || !maximumSize) return std::nullopt;
  if (*type > static_cast<uint32_t>(api::OverlayType::Disk)) {
    if (error) *error = std::format("UWF_OverlayConfig.Type has unsupported value {}", *type);
    return std::nullopt;
  }
  return api::OverlayConfigRow{*path, *currentSession, static_cast<api::OverlayType>(*type), *maximumSize};
}

std::optional<api::OverlayConfigRow> rereadOverlayConfig(WmiSession& session, const api::OverlayConfigRow& target, std::string* error) {
  const auto source = session.getObject(target.path, error);
  if (!source) return std::nullopt;
  auto observed = decodeOverlayConfig(*source, error);
  if (observed && observed->currentSession != target.currentSession) {
    if (error) *error = "UWF_OverlayConfig reread returned a different session instance";
    return std::nullopt;
  }
  return observed;
}

}  // namespace

std::vector<api::OverlayConfigRow> UwfOverlayConfig::readAll(std::string* error) const {
  if (error) error->clear();
  std::vector<api::OverlayConfigRow> out;
  std::string queryError;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_OverlayConfig", &queryError);
  if (!queryError.empty()) {
    if (error) *error = std::move(queryError);
    return out;
  }
  bool seenCurrent = false;
  bool seenNext = false;

  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_OverlayConfig", o);
    auto r = decodeOverlayConfig(o, error);
    if (!r) return {};
    bool& seen = r->currentSession ? seenCurrent : seenNext;
    if (seen) {
      if (error) *error = std::format("UWF_OverlayConfig returned duplicate {} session instances", r->currentSession ? "current" : "next");
      return {};
    }
    seen = true;
    out.push_back(std::move(*r));
  }
  UWF_LOG_D("UWF_OverlayConfig") << std::format("readAll ok: rows={}", rows.size());
  return out;
}

std::optional<api::OverlayConfigRow> UwfOverlayConfig::read(bool currentSession, std::string* error) const {
  for (auto& r : readAll(error)) {
    if (r.currentSession == currentSession) return r;
  }
  if (error && error->empty()) *error = std::format("UWF_OverlayConfig {} session instance is missing", currentSession ? "current" : "next");
  return std::nullopt;
}

WmiResult UwfOverlayConfig::setType(const api::OverlayConfigRow& row, api::OverlayType type) const {
  if (row.path.empty()) return WmiResult::failed("UWF_OverlayConfig row has empty __PATH; call read() first");
  WmiRow inputs;
  inputs.emplace("type", WmiValue::fromUInt(static_cast<uint32_t>(type)));
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, "SetType", inputs));
  std::string readError;
  const auto observed = rereadOverlayConfig(m_session, row, &readError);
  auto verification = verifyObservedState(observed, [type](const api::OverlayConfigRow& state) { return state.type == type; }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), "UWF_OverlayConfig::SetType");
  if (out.ok) {
    UWF_LOG_I("UWF_OverlayConfig") << std::format("SetType confirmed: type={} ({})", static_cast<uint32_t>(type),
                                                  type == api::OverlayType::Disk ? "Disk" : "RAM");
  }
  return out;
}

WmiResult UwfOverlayConfig::setMaximumSize(const api::OverlayConfigRow& row, uint32_t sizeMb) const {
  if (row.path.empty()) return WmiResult::failed("UWF_OverlayConfig row has empty __PATH; call read() first");
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  auto out = WmiResult::fromMethodResult(m_session.callMethod(row.path, "SetMaximumSize", inputs));
  std::string readError;
  const auto observed = rereadOverlayConfig(m_session, row, &readError);
  auto verification =
      verifyObservedState(observed, [sizeMb](const api::OverlayConfigRow& state) { return state.maximumSize == sizeMb; }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), "UWF_OverlayConfig::SetMaximumSize");
  if (out.ok) UWF_LOG_I("UWF_OverlayConfig") << "SetMaximumSize confirmed: size=" << sizeMb << "MB";
  return out;
}

}  // namespace uwf::api
