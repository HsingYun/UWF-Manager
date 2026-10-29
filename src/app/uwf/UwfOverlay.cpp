// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UwfOverlay.h"

#include <format>
#include <optional>
#include <regex>
#include <stdexcept>

#include "WriteVerification.h"
#include "app/wmi/WmiException.h"
#include "app/wmi/WmiRowUtil.h"
#include "shared/Log.h"

namespace uwf::app::provider {

namespace {

// 从 UWF_OverlayFile 的 MOF 文本中读取 FileSize。不同 WMI 文本化路径可能把
// UInt64 写成裸数字或带引号的十进制字符串；两种都是同一字段语义。0 是合法
// 的空文件大小，因此用 optional 区分“值为 0”和“字段无法解析”。
std::optional<uint64_t> parseFileSizeFromMof(const std::string& mof) {
  const std::regex re(R"mof(FileSize\s*=\s*(?:"(\d+)"|(\d+))\s*;)mof");
  std::smatch m;
  if (!std::regex_search(mof, m, re)) return std::nullopt;
  try {
    return std::stoull(m[1].matched ? m[1].str() : m[2].str());
  } catch (const std::invalid_argument&) {
    return std::nullopt;
  } catch (const std::out_of_range&) {
    return std::nullopt;
  }
}

uint64_t requireOverlayFileSize(const wmi::WmiRow& row) {
  if (const auto direct = row.find("FileSize"); direct != row.end() && direct->second.isValid()) {
    return wmi::row::requireUInt64(row, "FileSize");
  }
  const auto mof = wmi::row::requireString(row, "__MOF", wmi::row::EmptyString::Reject);
  const auto size = parseFileSizeFromMof(mof);
  if (!size) throw wmi::WmiDecodeError("decode UWF_OverlayFile", "FileSize is missing or invalid in embedded MOF");
  return *size;
}

provider::OverlayRow decodeOverlay(const wmi::WmiRow& source) {
  return {wmi::row::requireString(source, "__PATH", wmi::row::EmptyString::Reject), wmi::row::requireUInt(source, "OverlayConsumption"),
          wmi::row::requireUInt(source, "AvailableSpace"), wmi::row::requireUInt(source, "CriticalOverlayThreshold"),
          wmi::row::requireUInt(source, "WarningOverlayThreshold")};
}

provider::OverlayRow rereadOverlay(wmi::WmiOperations& session, const provider::OverlayRow& target) { return decodeOverlay(session.getObject(target.path)); }

void requirePath(const provider::OverlayRow& row, const char* operation) {
  if (row.path.empty()) throw wmi::WmiProtocolError(operation, "UWF_Overlay row has no object path");
}

}  // namespace

provider::OverlayRow UwfOverlay::read() const {
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Overlay");
  if (rows.size() != 1) {
    throw wmi::WmiProtocolError("read UWF_Overlay", std::format("expected one instance, received {}", rows.size()));
  }

  const auto& o = rows.front();
  wmi::row::dumpRow("UWF_Overlay", o);
  auto r = decodeOverlay(o);
  UWF_LOG_D("uwf") << "overlay read completed: consumptionMb=" << r.overlayConsumption << " availableMb=" << r.availableSpace
                   << " warningMb=" << r.warningOverlayThreshold << " criticalMb=" << r.criticalOverlayThreshold;
  return r;
}

std::vector<provider::OverlayFileRow> UwfOverlay::getOverlayFiles(const provider::OverlayRow& row, const std::string& volume,
                                                                  const std::stop_token stopToken) const {
  requirePath(row, "read UWF overlay files");

  wmi::WmiRow inputs;
  inputs.emplace("Volume", wmi::WmiValue::fromString(volume));

  const auto output = stopToken.stop_possible() ? m_session.callMethodReadCancelable(row.path, "GetOverlayFiles", inputs, stopToken)
                                                : m_session.callMethodRead(row.path, "GetOverlayFiles", inputs);

  auto out = wmi::row::readArrayOutput<provider::OverlayFileRow>(output, "OverlayFiles", [](const wmi::WmiRow& item) {
    return provider::OverlayFileRow{wmi::row::requireEmbeddedString(item, "FileName"), requireOverlayFileSize(item)};
  });
  UWF_LOG_D("uwf") << "overlay files read completed: volume=" << volume << " files=" << out.size();
  return out;
}

void UwfOverlay::setWarningThreshold(const provider::OverlayRow& row, const uint32_t sizeMb) const {
  requirePath(row, "set UWF warning threshold");
  wmi::WmiRow inputs;
  inputs.emplace("size", wmi::WmiValue::fromUInt(sizeMb));
  invokeAndConfirm(
      "set UWF warning threshold", [&] { m_session.invokeMethod(row.path, "SetWarningThreshold", inputs); },
      [&] { return rereadOverlay(m_session, row).warningOverlayThreshold == sizeMb; });
}

void UwfOverlay::setCriticalThreshold(const provider::OverlayRow& row, const uint32_t sizeMb) const {
  requirePath(row, "set UWF critical threshold");
  wmi::WmiRow inputs;
  inputs.emplace("size", wmi::WmiValue::fromUInt(sizeMb));
  invokeAndConfirm(
      "set UWF critical threshold", [&] { m_session.invokeMethod(row.path, "SetCriticalThreshold", inputs); },
      [&] { return rereadOverlay(m_session, row).criticalOverlayThreshold == sizeMb; });
}

}  // namespace uwf::app::provider
