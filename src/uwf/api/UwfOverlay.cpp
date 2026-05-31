#include "UwfOverlay.h"

#include <format>
#include <regex>

#include "../../util/Log.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf {

namespace {

// 从 UWF_OverlayFile 的 MOF 文本中读取 FileSize（无引号数字）。
uint64_t parseFileSizeFromMof(const std::string& mof) {
  const std::regex re(R"(FileSize\s*=\s*(\d+))");
  std::smatch m;
  if (!std::regex_search(mof, m, re)) return 0;
  try {
    return std::stoull(m[1].str());
  } catch (...) {
    return 0;
  }
}

}  // namespace

std::optional<api::OverlayRow> UwfOverlay::read(std::string* error) const {
  const auto rows = m_session.query("SELECT * FROM UWF_Overlay", error);
  if (rows.empty()) {
    UWF_LOG_W("UWF_Overlay") << "read: no rows returned";
    return std::nullopt;
  }

  const auto& o = rows.front();
  rowutil::dumpRow("UWF_Overlay", o);

  api::OverlayRow r;
  r.path = rowutil::getString(o, "__PATH");
  r.overlayConsumption = rowutil::getUInt(o, "OverlayConsumption");
  r.availableSpace = rowutil::getUInt(o, "AvailableSpace");
  r.criticalOverlayThreshold = rowutil::getUInt(o, "CriticalOverlayThreshold");
  r.warningOverlayThreshold = rowutil::getUInt(o, "WarningOverlayThreshold");
  UWF_LOG_I("UWF_Overlay") << std::format("read ok: consumption={}MB available={}MB warning={}MB critical={}MB", r.overlayConsumption, r.availableSpace,
                                          r.warningOverlayThreshold, r.criticalOverlayThreshold);
  return r;
}

std::vector<api::OverlayFileRow> UwfOverlay::getOverlayFiles(const api::OverlayRow& row, const std::string& volume, std::string* error,
                                                              int32_t* hresult) const {
  std::vector<api::OverlayFileRow> out;
  if (hresult) *hresult = 0;
  if (row.path.empty()) {
    if (error) *error = "UWF_Overlay row has empty __PATH; call read() first";
    UWF_LOG_E("UWF_Overlay") << "GetOverlayFiles rejected: empty __PATH; volume=" << volume;
    return out;
  }

  WmiRow inputs;
  inputs.emplace("Volume", WmiValue::fromString(volume));

  const auto r = m_session.callMethod(row.path, "GetOverlayFiles", inputs);
  if (!r.ok()) {
    if (error) *error = methodErrorDetail(r, "UWF_Overlay::GetOverlayFiles");
    if (hresult) *hresult = r.hresult;
    return out;
  }

  const auto it = r.outArrays.find("OverlayFiles");
  if (it == r.outArrays.end()) {
    UWF_LOG_W("UWF_Overlay") << "GetOverlayFiles: no OverlayFiles array in out params; volume=" << volume;
    return out;
  }

  out.reserve(it->second.size());
  uint64_t totalBytes = 0;
  for (const auto& item : it->second) {
    api::OverlayFileRow info;
    info.fileName = rowutil::readExcludedKey(item, "FileName");
    if (const auto direct = rowutil::getUInt64(item, "FileSize"); direct != 0) {
      info.fileSize = direct;
    } else {
      info.fileSize = parseFileSizeFromMof(item.value("__MOF").toString());
    }
    if (info.fileName.empty()) continue;
    totalBytes += info.fileSize;
    out.push_back(std::move(info));
  }
  UWF_LOG_I("UWF_Overlay") << std::format("GetOverlayFiles ok: volume={} files={} totalBytes={}", volume, out.size(), totalBytes);
  return out;
}

namespace {

// SetWarningThreshold / SetCriticalThreshold 共享前置校验 + WMI 调用骨架。
WmiResult invokeSetThreshold(const WmiSession& session, const api::OverlayRow& row, const char* method, uint32_t sizeMb) {
  if (row.path.empty()) return WmiResult::failed("UWF_Overlay row has empty __PATH; call read() first");
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  auto out = WmiResult::fromMethodResult(session.callMethod(row.path, method, inputs));
  if (out.ok) UWF_LOG_I("UWF_Overlay") << method << " ok: size=" << sizeMb << "MB";
  return out;
}

}  // namespace

WmiResult UwfOverlay::setWarningThreshold(const api::OverlayRow& row, const uint32_t sizeMb) const {
  return invokeSetThreshold(m_session, row, "SetWarningThreshold", sizeMb);
}

WmiResult UwfOverlay::setCriticalThreshold(const api::OverlayRow& row, const uint32_t sizeMb) const {
  return invokeSetThreshold(m_session, row, "SetCriticalThreshold", sizeMb);
}

}  // namespace uwf
