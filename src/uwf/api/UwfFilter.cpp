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

namespace {

// 5 个写方法都是无参 ExecMethod、行为一致——只是方法名与日志面包屑不同。
bool invoke(const WmiSession& session, const api::FilterRow& row, const char* methodName, std::string* error) {
  const auto r = session.callMethod(row.path, methodName);
  if (!r.ok()) {
    if (error) *error = r.error;
    return false;
  }
  UWF_LOG_I("UWF_Filter") << methodName << " ok";
  return true;
}

}  // namespace

bool UwfFilter::enable(const api::FilterRow& row, std::string* error) const { return invoke(m_session, row, "Enable", error); }
bool UwfFilter::disable(const api::FilterRow& row, std::string* error) const { return invoke(m_session, row, "Disable", error); }
bool UwfFilter::resetSettings(const api::FilterRow& row, std::string* error) const { return invoke(m_session, row, "ResetSettings", error); }
bool UwfFilter::shutdownSystem(const api::FilterRow& row, std::string* error) const { return invoke(m_session, row, "ShutdownSystem", error); }
bool UwfFilter::restartSystem(const api::FilterRow& row, std::string* error) const { return invoke(m_session, row, "RestartSystem", error); }

}  // namespace uwf
