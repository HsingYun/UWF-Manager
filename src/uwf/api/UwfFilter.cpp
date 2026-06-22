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

std::optional<api::FilterRow> UwfFilter::read(std::string* error) const {
  const auto rows = m_session.query("SELECT * FROM UWF_Filter", error);
  if (rows.empty()) {
    UWF_LOG_W("UWF_Filter") << "read: no rows returned";
    return std::nullopt;
  }

  const auto& f = rows.front();
  rowutil::dumpRow("UWF_Filter", f);

  api::FilterRow r;
  r.path = rowutil::getString(f, "__PATH");
  r.currentEnabled = rowutil::getBool(f, "CurrentEnabled");
  r.nextEnabled = rowutil::getBool(f, "NextEnabled");
  UWF_LOG_I("UWF_Filter") << std::format("read ok: rows={} currentEnabled={} nextEnabled={}", rows.size(), r.currentEnabled, r.nextEnabled);
  return r;
}

namespace {

// 5 个写方法都是无参 ExecMethod、行为一致——只是方法名与日志面包屑不同。
WmiResult invoke(const WmiSession& session, const api::FilterRow& row, const char* methodName) {
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, methodName));
  if (out.ok) UWF_LOG_I("UWF_Filter") << methodName << " ok";
  return out;
}

}  // namespace

WmiResult UwfFilter::enable(const api::FilterRow& row) const { return invoke(m_session, row, "Enable"); }
WmiResult UwfFilter::disable(const api::FilterRow& row) const { return invoke(m_session, row, "Disable"); }
WmiResult UwfFilter::resetSettings(const api::FilterRow& row) const { return invoke(m_session, row, "ResetSettings"); }
WmiResult UwfFilter::shutdownSystem(const api::FilterRow& row) const { return invoke(m_session, row, "ShutdownSystem"); }
WmiResult UwfFilter::restartSystem(const api::FilterRow& row) const { return invoke(m_session, row, "RestartSystem"); }

}  // namespace uwf::api
