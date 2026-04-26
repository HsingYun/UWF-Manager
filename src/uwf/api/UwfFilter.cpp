#include "UwfFilter.h"

#include <format>

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf {

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

bool UwfFilter::invokeNoArgs(const api::FilterRow& row, const char* methodName, std::string* error) const {
  if (row.path.empty()) {
    if (error) *error = "UWF_Filter row has empty __PATH; call read() first";
    UWF_LOG_E("UWF_Filter") << "invoke rejected: empty __PATH; method=" << methodName;
    return false;
  }
  const auto r = m_session.callMethod(row.path, methodName);
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_Filter::{} returned {}", methodName, r.returnValue) : r.error;
    UWF_LOG_E("UWF_Filter") << std::format("{} failed: invoked={} rv={} err={}", methodName, r.invoked, r.returnValue, r.error);
    return false;
  }
  UWF_LOG_I("UWF_Filter") << methodName << " ok";
  return true;
}

bool UwfFilter::enable(const api::FilterRow& row, std::string* error) const { return invokeNoArgs(row, "Enable", error); }
bool UwfFilter::disable(const api::FilterRow& row, std::string* error) const { return invokeNoArgs(row, "Disable", error); }
bool UwfFilter::resetSettings(const api::FilterRow& row, std::string* error) const { return invokeNoArgs(row, "ResetSettings", error); }
bool UwfFilter::shutdownSystem(const api::FilterRow& row, std::string* error) const { return invokeNoArgs(row, "ShutdownSystem", error); }
bool UwfFilter::restartSystem(const api::FilterRow& row, std::string* error) const { return invokeNoArgs(row, "RestartSystem", error); }

}  // namespace uwf
