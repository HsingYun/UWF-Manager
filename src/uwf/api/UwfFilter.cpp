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
#include "../wmi/WmiException.h"
#include "../wmi/WmiRowUtil.h"
#include "WriteVerification.h"

namespace uwf::api {

namespace {

api::FilterRow decodeFilter(const WmiRow& source) {
  return {rowutil::requireString(source, "__PATH", rowutil::EmptyString::Reject), rowutil::requireBool(source, "CurrentEnabled"),
          rowutil::requireBool(source, "NextEnabled")};
}

api::FilterRow rereadFilter(WmiSession& session, const api::FilterRow& target) {
  return decodeFilter(session.getObject(target.path));
}

}  // namespace

api::FilterRow UwfFilter::read() const {
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Filter");
  if (rows.size() != 1) {
    throw WmiProtocolError("read UWF_Filter", std::format("expected one instance, received {}", rows.size()));
  }

  const auto& f = rows.front();
  rowutil::dumpRow("UWF_Filter", f);
  auto r = decodeFilter(f);
  UWF_LOG_D("uwf") << "filter read completed: currentEnabled=" << r.currentEnabled << " nextEnabled=" << r.nextEnabled;
  return r;
}

namespace {

void requirePath(const api::FilterRow& row, const char* operation) {
  if (row.path.empty()) throw WmiProtocolError(operation, "UWF_Filter row has no object path");
}

}  // namespace

void UwfFilter::enable(const api::FilterRow& row) const {
  requirePath(row, "enable UWF");
  invokeAndConfirm("enable UWF", [&] { m_session.invokeMethod(row.path, "Enable"); }, [&] { return rereadFilter(m_session, row).nextEnabled; });
}

void UwfFilter::disable(const api::FilterRow& row) const {
  requirePath(row, "disable UWF");
  invokeAndConfirm("disable UWF", [&] { m_session.invokeMethod(row.path, "Disable"); }, [&] { return !rereadFilter(m_session, row).nextEnabled; });
}

void UwfFilter::shutdownSystem(const api::FilterRow& row) const {
  requirePath(row, "shut down UWF-protected system");
  m_session.invokeMethod(row.path, "ShutdownSystem");
}

void UwfFilter::restartSystem(const api::FilterRow& row) const {
  requirePath(row, "restart UWF-protected system");
  m_session.invokeMethod(row.path, "RestartSystem");
}

}  // namespace uwf::api
