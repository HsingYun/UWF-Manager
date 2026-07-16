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
#include "UwfOverlay.h"

#include <format>
#include <optional>
#include <regex>
#include <stdexcept>

#include "../../util/Log.h"
#include "../wmi/WmiException.h"
#include "../wmi/WmiRowUtil.h"
#include "WriteVerification.h"

namespace uwf::api {

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

uint64_t requireOverlayFileSize(const WmiRow& row) {
  if (const auto direct = row.find("FileSize"); direct != row.end() && direct->second.isValid()) {
    return rowutil::requireUInt64(row, "FileSize");
  }
  const auto mof = rowutil::requireString(row, "__MOF", rowutil::EmptyString::Reject);
  const auto size = parseFileSizeFromMof(mof);
  if (!size) throw WmiDecodeError("decode UWF_OverlayFile", "FileSize is missing or invalid in embedded MOF");
  return *size;
}

api::OverlayRow decodeOverlay(const WmiRow& source) {
  return {rowutil::requireString(source, "__PATH", rowutil::EmptyString::Reject), rowutil::requireUInt(source, "OverlayConsumption"),
          rowutil::requireUInt(source, "AvailableSpace"), rowutil::requireUInt(source, "CriticalOverlayThreshold"),
          rowutil::requireUInt(source, "WarningOverlayThreshold")};
}

api::OverlayRow rereadOverlay(WmiSession& session, const api::OverlayRow& target) {
  return decodeOverlay(session.getObject(target.path));
}

void requirePath(const api::OverlayRow& row, const char* operation) {
  if (row.path.empty()) throw WmiProtocolError(operation, "UWF_Overlay row has no object path");
}

}  // namespace

api::OverlayRow UwfOverlay::read() const {
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Overlay");
  if (rows.size() != 1) {
    throw WmiProtocolError("read UWF_Overlay", std::format("expected one instance, received {}", rows.size()));
  }

  const auto& o = rows.front();
  rowutil::dumpRow("UWF_Overlay", o);
  auto r = decodeOverlay(o);
  UWF_LOG_D("uwf") << "overlay read completed: consumptionMb=" << r.overlayConsumption << " availableMb=" << r.availableSpace
                    << " warningMb=" << r.warningOverlayThreshold << " criticalMb=" << r.criticalOverlayThreshold;
  return r;
}

std::vector<api::OverlayFileRow> UwfOverlay::getOverlayFiles(const api::OverlayRow& row, const std::string& volume,
                                                             const std::stop_token stopToken) const {
  requirePath(row, "read UWF overlay files");

  WmiRow inputs;
  inputs.emplace("Volume", WmiValue::fromString(volume));

  const auto output = stopToken.stop_possible() ? m_session.callMethodReadCancelable(row.path, "GetOverlayFiles", inputs, stopToken)
                                                : m_session.callMethodRead(row.path, "GetOverlayFiles", inputs);

  auto out = rowutil::readArrayOutput<api::OverlayFileRow>(output, "OverlayFiles", [](const WmiRow& item) {
    return api::OverlayFileRow{rowutil::requireEmbeddedString(item, "FileName"), requireOverlayFileSize(item)};
  });
  UWF_LOG_D("uwf") << "overlay files read completed: volume=" << volume << " files=" << out.size();
  return out;
}

void UwfOverlay::setWarningThreshold(const api::OverlayRow& row, const uint32_t sizeMb) const {
  requirePath(row, "set UWF warning threshold");
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  invokeAndConfirm("set UWF warning threshold", [&] { m_session.invokeMethod(row.path, "SetWarningThreshold", inputs); },
                   [&] { return rereadOverlay(m_session, row).warningOverlayThreshold == sizeMb; });
}

void UwfOverlay::setCriticalThreshold(const api::OverlayRow& row, const uint32_t sizeMb) const {
  requirePath(row, "set UWF critical threshold");
  WmiRow inputs;
  inputs.emplace("size", WmiValue::fromUInt(sizeMb));
  invokeAndConfirm("set UWF critical threshold", [&] { m_session.invokeMethod(row.path, "SetCriticalThreshold", inputs); },
                   [&] { return rereadOverlay(m_session, row).criticalOverlayThreshold == sizeMb; });
}

}  // namespace uwf::api
