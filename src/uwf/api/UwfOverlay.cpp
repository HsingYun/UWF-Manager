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

std::vector<api::OverlayFileInfo> UwfOverlay::getOverlayFiles(const api::OverlayRow& row, const std::string& volume, std::string* error,
                                                              int32_t* hresult) const {
  std::vector<api::OverlayFileInfo> out;
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
    if (error) *error = r.invoked ? std::format("UWF_Overlay::GetOverlayFiles returned {}", r.returnValue) : r.error;
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
    api::OverlayFileInfo info;
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

bool UwfOverlay::setWarningThreshold(const api::OverlayRow& row, const uint32_t sizeMb, std::string* error) const {
  if (row.path.empty()) {
    if (error) *error = "UWF_Overlay row has empty __PATH; call read() first";
    return false;
  }
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  const auto r = m_session.callMethod(row.path, "SetWarningThreshold", inputs);
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_Overlay::SetWarningThreshold returned {}", r.returnValue) : r.error;
    return false;
  }
  UWF_LOG_I("UWF_Overlay") << "SetWarningThreshold ok: size=" << sizeMb << "MB";
  return true;
}

bool UwfOverlay::setCriticalThreshold(const api::OverlayRow& row, const uint32_t sizeMb, std::string* error) const {
  if (row.path.empty()) {
    if (error) *error = "UWF_Overlay row has empty __PATH; call read() first";
    return false;
  }
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  const auto r = m_session.callMethod(row.path, "SetCriticalThreshold", inputs);
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_Overlay::SetCriticalThreshold returned {}", r.returnValue) : r.error;
    return false;
  }
  UWF_LOG_I("UWF_Overlay") << "SetCriticalThreshold ok: size=" << sizeMb << "MB";
  return true;
}

}  // namespace uwf
