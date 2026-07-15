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
#include "UwfFilter.h"

#include <format>

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf::api {

namespace {

std::optional<api::FilterRow> decodeFilter(const WmiRow& source, std::string* error) {
  const auto path = rowutil::requireString(source, "__PATH", error, false);
  const auto currentEnabled = rowutil::requireBool(source, "CurrentEnabled", error);
  const auto nextEnabled = rowutil::requireBool(source, "NextEnabled", error);
  if (!path || !currentEnabled || !nextEnabled) return std::nullopt;
  return api::FilterRow{*path, *currentEnabled, *nextEnabled};
}

std::optional<api::FilterRow> rereadFilter(WmiSession& session, const api::FilterRow& target, std::string* error) {
  const auto source = session.getObject(target.path, error);
  if (!source) return std::nullopt;
  return decodeFilter(*source, error);
}

}  // namespace

std::optional<api::FilterRow> UwfFilter::read(std::string* error) const {
  if (error) error->clear();
  std::string queryError;
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Filter", &queryError);
  if (!queryError.empty()) {
    if (error) *error = std::move(queryError);
    return std::nullopt;
  }
  if (rows.size() != 1) {
    if (error) *error = std::format("UWF_Filter expected one instance, received {}", rows.size());
    UWF_LOG_W("UWF_Filter") << "read: singleton cardinality mismatch; rows=" << rows.size();
    return std::nullopt;
  }

  const auto& f = rows.front();
  rowutil::dumpRow("UWF_Filter", f);
  auto r = decodeFilter(f, error);
  if (!r) return std::nullopt;
  UWF_LOG_D("UWF_Filter") << std::format("read ok: rows={} currentEnabled={} nextEnabled={}", rows.size(), r->currentEnabled, r->nextEnabled);
  return r;
}

namespace {

WmiResult invokeCommand(WmiSession& session, const api::FilterRow& row, const char* methodName) {
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, methodName));
  if (out.ok) UWF_LOG_I("UWF_Filter") << methodName << " ok";
  return out;
}

WmiResult setEnabled(WmiSession& session, const api::FilterRow& row, const bool enabled) {
  const char* method = enabled ? "Enable" : "Disable";
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, method));

  std::string readError;
  const auto observed = rereadFilter(session, row, &readError);
  auto verification = verifyObservedState(observed, [enabled](const api::FilterRow& state) { return state.nextEnabled == enabled; }, std::move(readError));
  out = confirmWriteState(std::move(out), std::move(verification), std::format("UWF_Filter::{}", method));
  if (out.ok) UWF_LOG_I("UWF_Filter") << method << " confirmed";
  return out;
}

}  // namespace

WmiResult UwfFilter::enable(const api::FilterRow& row) const { return setEnabled(m_session, row, true); }
WmiResult UwfFilter::disable(const api::FilterRow& row) const { return setEnabled(m_session, row, false); }
WmiResult UwfFilter::resetSettings(const api::FilterRow& row) const { return invokeCommand(m_session, row, "ResetSettings"); }
WmiResult UwfFilter::shutdownSystem(const api::FilterRow& row) const { return invokeCommand(m_session, row, "ShutdownSystem"); }
WmiResult UwfFilter::restartSystem(const api::FilterRow& row) const { return invokeCommand(m_session, row, "RestartSystem"); }

}  // namespace uwf::api
