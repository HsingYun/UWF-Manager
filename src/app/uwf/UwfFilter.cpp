// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfFilter.h"

#include <format>

#include "WriteVerification.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "shared/Log.h"

namespace uwf::app::provider {

namespace {

provider::FilterRow decodeFilter(const wmi::WmiRow& source) {
  return {wmi::row::requireString(source, "__PATH", wmi::row::EmptyString::Reject), wmi::row::requireBoolOrFalseIfNull(source, "CurrentEnabled"),
          wmi::row::requireBoolOrFalseIfNull(source, "NextEnabled")};
}

provider::FilterRow rereadFilter(wmi::WmiOperations& session, const provider::FilterRow& target) { return decodeFilter(session.getObject(target.path)); }

}  // namespace

provider::FilterRow UwfFilter::read() const {
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Filter");
  if (rows.size() != 1) {
    throw wmi::WmiProtocolError("read UWF_Filter", std::format("expected one instance, received {}", rows.size()));
  }

  const auto& f = rows.front();
  wmi::row::dumpRow("UWF_Filter", f);
  auto r = decodeFilter(f);
  UWF_LOG_D("uwf") << "filter read completed: currentEnabled=" << r.currentEnabled << " nextEnabled=" << r.nextEnabled;
  return r;
}

namespace {

void requirePath(const provider::FilterRow& row, const char* operation) {
  if (row.path.empty()) throw wmi::WmiProtocolError(operation, "UWF_Filter row has no object path");
}

}  // namespace

void UwfFilter::enable(const provider::FilterRow& row) const {
  requirePath(row, "enable UWF");
  invokeAndConfirm("enable UWF", [&] { m_session.invokeMethod(row.path, "Enable"); }, [&] { return rereadFilter(m_session, row).nextEnabled; });
}

void UwfFilter::disable(const provider::FilterRow& row) const {
  requirePath(row, "disable UWF");
  invokeAndConfirm("disable UWF", [&] { m_session.invokeMethod(row.path, "Disable"); }, [&] { return !rereadFilter(m_session, row).nextEnabled; });
}

void UwfFilter::shutdownSystem(const provider::FilterRow& row) const {
  requirePath(row, "shut down UWF-protected system");
  m_session.invokeMethod(row.path, "ShutdownSystem");
}

void UwfFilter::restartSystem(const provider::FilterRow& row) const {
  requirePath(row, "restart UWF-protected system");
  m_session.invokeMethod(row.path, "RestartSystem");
}

}  // namespace uwf::app::provider
