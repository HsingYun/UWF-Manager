#include "UwfVolume.h"

#include <format>
#include <optional>

#include "../../util/DriveLetter.h"
#include "../../util/Log.h"
#include "../wmi/WmiError.h"
#include "../wmi/WmiRowUtil.h"

namespace uwf {

namespace {

bool invokeSimple(const WmiSession& session, const std::string& path, const char* method, const WmiRow& inputs, std::string* error) {
  if (path.empty()) {
    if (error) *error = "UWF_Volume row has empty __PATH; call readAll() first";
    UWF_LOG_E("UWF_Volume") << "invoke rejected: empty __PATH; method=" << method;
    return false;
  }
  const auto r = session.callMethod(path, method, inputs);
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_Volume::{} returned {}", method, r.returnValue) : r.error;
    UWF_LOG_E("UWF_Volume") << std::format("{} failed: invoked={} rv={} err={}", method, r.invoked, r.returnValue, r.error);
    return false;
  }
  return true;
}

// UWF_Volume 的 AddExclusion / RemoveExclusion / FindExclusion / CommitFile /
// CommitFileDeletion 的路径参数均不包含驱动器号或卷名（见
// knowledge/reference/11-uwf-api.html 各方法的参数说明）。若用户传入
// "C:\foo"，这里在确认与目标卷匹配后剥掉开头的 "<letter>:"。盘符不匹配直接
// 返回空 optional 并写 *error。
std::optional<std::string> stripVolumeDriveLetter(const std::string& path, const std::string& volumeDriveLetter, std::string* error) {
  // UWF 的 AddExclusion / CommitFile 等方法收的是"卷内相对路径"（不含盘符）。
  // 这里把用户给的完整路径按盘符拆开：拆分逻辑统一交给 drive::split。
  const auto s = drive::split(path);
  if (s.letter.empty()) return path;  // 无盘符前缀——已是卷内相对路径，原样返回
  if (s.letter != volumeDriveLetter) {
    if (error) *error = std::format("path \"{}\" drive letter does not match target volume {}", path, volumeDriveLetter);
    return std::nullopt;
  }
  return s.rest;
}

// 把字符串转义后嵌入 WMI 对象路径的引号键值（DriveLetter="..." / VolumeName="..."）。
// WMI 对象路径按 C/C++ 规则解析引号转义（见 MS 文档 "WMI Object Path
// Requirements"："embedded quotation marks ... must delimit the quotation mark
// with escape characters, as in a C or C++ application"），故 `"` 要写成 `\"`、
// `\` 要写成 `\\`。
//
// 实测：UWF_Volume.VolumeName 是 "Volume{GUID}" 形式（不带 \\?\ 前缀、不带结尾
// 反斜杠——详见 knowledge/reference/11-uwf-api.html 中 UWF_Volume 的备注），
// DriveLetter 是 "C:" 形式；两者都不含 `\` 或 `"`，所以此处转义当前其实是空操作。
// 保留它作为防御——若将来某个 key 值含特殊字符，合成出的 __PATH 仍 correct-
// by-construction。
// （注：readAll() 里 row.path 取自 __PATH，是 WMI 自己产出、已正确转义的路径，
// 不经过这里；只有本文件 ensureNextSessionEntry 手工拼 __PATH 才用到本函数。）
std::string escapeWmiPathValue(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (const char c : s) {
    if (c == '\\' || c == '"') out += '\\';
    out += c;
  }
  return out;
}

// CommitFile / CommitFileDeletion 的失败分类：
//   * WBEM_E_FAILED    ≈ 文件被其它进程占用
//   * WBEM_E_NOT_FOUND ≈ overlay 里没相关条目（可能已和磁盘一致）
// 这两种归 Skipped——UI 只提示不报错；其它 hresult 算 Failed。
CommitOutcome classifyCommitFailure(const WmiMethodResult& r) {
  if (r.invoked) return CommitOutcome::Failed;
  const WmiError err(r.hresult);
  return (err == WmiErrorCode::Failed || err == WmiErrorCode::NotFound) ? CommitOutcome::Skipped : CommitOutcome::Failed;
}

}  // namespace

std::vector<api::VolumeRow> UwfVolume::readAll(std::string* error) const {
  std::vector<api::VolumeRow> out;
  const auto rows = m_session.query("SELECT * FROM UWF_Volume", error);

  size_t curCount = 0;
  size_t nextCount = 0;
  out.reserve(rows.size());
  for (const auto& o : rows) {
    rowutil::dumpRow("UWF_Volume", o);
    api::VolumeRow r;
    r.path = rowutil::getString(o, "__PATH");
    r.currentSession = rowutil::getBool(o, "CurrentSession");
    r.driveLetter = drive::normalize(rowutil::getString(o, "DriveLetter"));
    r.volumeName = rowutil::getString(o, "VolumeName");
    r.bindByDriveLetter = rowutil::getBool(o, "BindByDriveLetter", true);
    r.commitPending = rowutil::getBool(o, "CommitPending");
    r.isProtected = rowutil::getBool(o, "Protected");
    if (r.currentSession)
      ++curCount;
    else
      ++nextCount;
    out.push_back(std::move(r));
  }
  UWF_LOG_I("UWF_Volume") << std::format("readAll ok: rows={} (current={}, next={})", rows.size(), curCount, nextCount);
  return out;
}

bool UwfVolume::protectVolume(const api::VolumeRow& row, std::string* error) const {
  const bool ok = invokeSimple(m_session, row.path, "Protect", {}, error);
  if (ok) UWF_LOG_I("UWF_Volume") << "Protect ok: dl=" << row.driveLetter;
  return ok;
}

bool UwfVolume::unprotect(const api::VolumeRow& row, std::string* error) const {
  const bool ok = invokeSimple(m_session, row.path, "Unprotect", {}, error);
  if (ok) UWF_LOG_I("UWF_Volume") << "Unprotect ok: dl=" << row.driveLetter;
  return ok;
}

CommitFileResult UwfVolume::commitFile(const api::VolumeRow& row, const std::string& fileFullPath) const {
  CommitFileResult out;
  std::string stripErr;
  const auto normalized = stripVolumeDriveLetter(fileFullPath, row.driveLetter, &stripErr);
  if (!normalized) {
    out.outcome = CommitOutcome::Failed;
    out.detail = stripErr;
    UWF_LOG_E("UWF_Volume") << "CommitFile rejected: " << stripErr;
    return out;
  }
  if (row.path.empty()) {
    out.outcome = CommitOutcome::Failed;
    out.detail = "UWF_Volume row has empty __PATH; call readAll() first";
    UWF_LOG_E("UWF_Volume") << "CommitFile rejected: empty __PATH; file=" << fileFullPath;
    return out;
  }

  WmiRow in;
  // 实机 WMI schema 的参数名是 FileName（见
  // knowledge/reference/11-uwf-api.html 附录 B.1）。
  in.emplace("FileName", WmiValue::fromString(*normalized));
  const auto r = m_session.callMethod(row.path, "CommitFile", in);
  out.hresult = r.hresult;
  out.returnValue = r.returnValue;
  if (r.ok()) {
    out.outcome = CommitOutcome::Ok;
    UWF_LOG_I("UWF_Volume") << std::format("CommitFile ok: dl={} file={}", row.driveLetter, *normalized);
    return out;
  }

  out.detail = r.invoked ? std::format("UWF_Volume::CommitFile returned {}", r.returnValue) : r.error;
  out.outcome = classifyCommitFailure(r);
  return out;
}

CommitFileResult UwfVolume::commitFileDeletion(const api::VolumeRow& row, const std::string& fileName) const {
  CommitFileResult out;
  std::string stripErr;
  const auto normalized = stripVolumeDriveLetter(fileName, row.driveLetter, &stripErr);
  if (!normalized) {
    out.outcome = CommitOutcome::Failed;
    out.detail = stripErr;
    UWF_LOG_E("UWF_Volume") << "CommitFileDeletion rejected: " << stripErr;
    return out;
  }
  if (row.path.empty()) {
    out.outcome = CommitOutcome::Failed;
    out.detail = "UWF_Volume row has empty __PATH; call readAll() first";
    UWF_LOG_E("UWF_Volume") << "CommitFileDeletion rejected: empty __PATH; file=" << fileName;
    return out;
  }

  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  const auto r = m_session.callMethod(row.path, "CommitFileDeletion", in);
  out.hresult = r.hresult;
  out.returnValue = r.returnValue;
  if (r.ok()) {
    out.outcome = CommitOutcome::Ok;
    UWF_LOG_I("UWF_Volume") << std::format("CommitFileDeletion ok: dl={} file={}", row.driveLetter, *normalized);
    return out;
  }

  out.detail = r.invoked ? std::format("UWF_Volume::CommitFileDeletion returned {}", r.returnValue) : r.error;
  out.outcome = classifyCommitFailure(r);
  return out;
}

bool UwfVolume::setBindByDriveLetter(const api::VolumeRow& row, bool bBindByDriveLetter, std::string* error) const {
  WmiRow in;
  in.emplace("bBindByDriveLetter", WmiValue::fromBool(bBindByDriveLetter));
  const bool ok = invokeSimple(m_session, row.path, "SetBindByDriveLetter", in, error);
  if (ok) UWF_LOG_I("UWF_Volume") << std::format("SetBindByDriveLetter ok: dl={} bindByDriveLetter={}", row.driveLetter, bBindByDriveLetter);
  return ok;
}

bool UwfVolume::addExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error) const {
  const auto normalized = stripVolumeDriveLetter(fileName, row.driveLetter, error);
  if (!normalized) return false;
  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  const bool ok = invokeSimple(m_session, row.path, "AddExclusion", in, error);
  if (ok) UWF_LOG_I("UWF_Volume") << std::format("AddExclusion ok: dl={} file={}", row.driveLetter, *normalized);
  return ok;
}

bool UwfVolume::removeExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error) const {
  const auto normalized = stripVolumeDriveLetter(fileName, row.driveLetter, error);
  if (!normalized) return false;
  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  const bool ok = invokeSimple(m_session, row.path, "RemoveExclusion", in, error);
  if (ok) UWF_LOG_I("UWF_Volume") << std::format("RemoveExclusion ok: dl={} file={}", row.driveLetter, *normalized);
  return ok;
}

bool UwfVolume::removeAllExclusions(const api::VolumeRow& row, std::string* error) const {
  const bool ok = invokeSimple(m_session, row.path, "RemoveAllExclusions", {}, error);
  if (ok) UWF_LOG_I("UWF_Volume") << "RemoveAllExclusions ok: dl=" << row.driveLetter;
  return ok;
}

std::optional<bool> UwfVolume::findExclusion(const api::VolumeRow& row, const std::string& fileName, std::string* error) const {
  if (row.path.empty()) {
    if (error) *error = "UWF_Volume row has empty __PATH; call readAll() first";
    return std::nullopt;
  }
  const auto normalized = stripVolumeDriveLetter(fileName, row.driveLetter, error);
  if (!normalized) return std::nullopt;
  WmiRow in;
  in.emplace("FileName", WmiValue::fromString(*normalized));
  const auto r = m_session.callMethod(row.path, "FindExclusion", in);
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_Volume::FindExclusion returned {}", r.returnValue) : r.error;
    return std::nullopt;
  }
  return rowutil::getBool(r.outParams, "bFound");
}

std::vector<api::ExcludedFile> UwfVolume::getExclusions(const api::VolumeRow& row, std::string* error) const {
  std::vector<api::ExcludedFile> out;
  if (row.path.empty()) {
    if (error) *error = "UWF_Volume row has empty __PATH; call readAll() first";
    return out;
  }
  const auto r = m_session.callMethod(row.path, "GetExclusions");
  if (!r.ok()) {
    if (error) *error = r.invoked ? std::format("UWF_Volume::GetExclusions returned {}", r.returnValue) : r.error;
    return out;
  }
  const auto it = r.outArrays.find("ExcludedFiles");
  if (it == r.outArrays.end()) return out;

  out.reserve(it->second.size());
  for (const auto& item : it->second) {
    api::ExcludedFile e;
    e.fileName = rowutil::readExcludedKey(item, "FileName");
    if (e.fileName.empty()) continue;
    out.push_back(std::move(e));
  }
  UWF_LOG_D("UWF_Volume") << std::format("GetExclusions ok: dl={} session={} count={}", row.driveLetter, row.currentSession ? "current" : "next", out.size());
  return out;
}

std::optional<api::VolumeRow> UwfVolume::ensureNextSessionEntry(const std::string& driveLetter, std::string* error) const {
  if (driveLetter.empty()) {
    if (error) *error = "ensureNextSessionEntry requires a non-empty driveLetter";
    return std::nullopt;
  }

  const auto rows = readAll();

  // 1) 已有 next session 实例 → 直接复用，避免重复 PutInstance。
  for (const auto& v : rows) {
    if (!v.currentSession && v.driveLetter == driveLetter) return v;
  }

  // 2) 没有 next session 实例：从同卷的 current session 实例上拿 VolumeName。
  //    UWF 自己写出的 current 实例 VolumeName 格式必然规范，避免再走
  //    Win32_Volume 然后归一化的路。
  std::string volumeName;
  for (const auto& v : rows) {
    if (v.currentSession && v.driveLetter == driveLetter) {
      volumeName = v.volumeName;
      break;
    }
  }
  if (volumeName.empty()) {
    if (error) *error = std::format("no current-session UWF_Volume row found for {}; cannot register", driveLetter);
    return std::nullopt;
  }

  // 3) PutInstance 创建 next session 实例（key = CurrentSession=false +
  //    DriveLetter + VolumeName）。
  WmiRow props;
  props.emplace("CurrentSession", WmiValue::fromBool(false));
  props.emplace("DriveLetter", WmiValue::fromString(driveLetter));
  props.emplace("VolumeName", WmiValue::fromString(volumeName));

  std::string putErr;
  if (!m_session.putInstance("UWF_Volume", props, &putErr)) {
    if (error) *error = putErr;
    return std::nullopt;
  }
  UWF_LOG_I("UWF_Volume") << std::format("ensureNextSessionEntry created: dl={} volumeName={}", driveLetter, volumeName);

  // 4) PutInstance 是同步的，但 UWF provider 对 ExecQuery 的可见性略微滞后；
  //    立即 readAll 经常拿不到，要等下次 query 才出现。所以不依赖 readAll，
  //    按 key 三元组直接构造 relative __PATH 给 caller —— WMI 的 ExecMethod
  //    接受 relative path（不含 hostname / namespace），后续 Protect 等
  //    instance 方法能正确解析。
  api::VolumeRow row;
  row.currentSession = false;
  row.driveLetter = driveLetter;
  row.volumeName = volumeName;
  row.isProtected = false;
  row.bindByDriveLetter = true;
  row.path =
      std::format(R"(UWF_Volume.CurrentSession=FALSE,DriveLetter="{}",VolumeName="{}")", escapeWmiPathValue(driveLetter), escapeWmiPathValue(volumeName));
  return row;
}

}  // namespace uwf
