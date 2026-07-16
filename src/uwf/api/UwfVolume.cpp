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
#include "UwfVolume.h"

#include <exception>
#include <format>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>

#include "../../util/DriveLetter.h"
#include "../../util/Log.h"
#include "../wmi/WmiError.h"
#include "../wmi/WmiException.h"
#include "../wmi/WmiRowUtil.h"
#include "WriteVerification.h"

namespace uwf::api {

namespace {

std::string stripVolumeDriveLetter(const std::string& path, const std::string& volumeDriveLetter) {
  const auto split = drive::split(path);
  if (split.letter.empty()) return path;
  if (split.letter != volumeDriveLetter) {
    throw std::invalid_argument(std::format("path '{}' belongs to {}, not target volume {}", path, split.letter, volumeDriveLetter));
  }
  return split.rest;
}

std::string escapeWmiPathValue(const std::string& value) {
  std::string escaped;
  escaped.reserve(value.size() + 8);
  for (const char character : value) {
    if (character == '\\' || character == '"') escaped += '\\';
    escaped += character;
  }
  return escaped;
}

std::optional<api::VolumeRow> decodeManagedVolume(const WmiRow& source) {
  const std::string path = rowutil::requireString(source, "__PATH", rowutil::EmptyString::Reject);
  const bool currentSession = rowutil::requireBool(source, "CurrentSession");
  const std::string volumeName = rowutil::requireString(source, "VolumeName", rowutil::EmptyString::Reject);

  const auto drive = source.find("DriveLetter");
  if (drive == source.end()) throw WmiDecodeError("decode UWF_Volume", "field 'DriveLetter' is missing");
  if (!drive->second.isValid()) return std::nullopt;
  const std::string driveLetter = rowutil::requireString(source, "DriveLetter");
  if (driveLetter.empty()) return std::nullopt;
  const std::string normalizedDrive = drive::normalize(driveLetter);
  if (normalizedDrive.empty()) throw WmiDecodeError("decode UWF_Volume", std::format("invalid DriveLetter: {}", driveLetter));

  return api::VolumeRow{path,
                        currentSession,
                        normalizedDrive,
                        volumeName,
                        rowutil::requireBool(source, "BindByDriveLetter"),
                        rowutil::requireBool(source, "CommitPending"),
                        rowutil::requireBool(source, "Protected")};
}

api::VolumeRow rereadVolume(WmiSession& session, const api::VolumeRow& target) {
  auto observed = decodeManagedVolume(session.getObject(target.path));
  if (!observed || observed->currentSession != target.currentSession || observed->driveLetter != target.driveLetter ||
      observed->volumeName != target.volumeName) {
    throw WmiProtocolError("reread UWF_Volume", "provider returned a different volume instance");
  }
  return *observed;
}

void requirePath(const api::VolumeRow& row, const char* operation) {
  if (row.path.empty()) throw WmiProtocolError(operation, "UWF_Volume row has no object path");
}

std::string normalizeVolumePath(const api::VolumeRow& row, const std::string& fileName) {
  return stripVolumeDriveLetter(fileName, row.driveLetter);
}

WmiRow fileInput(const std::string& normalizedFileName) {
  WmiRow inputs;
  inputs.emplace("FileName", WmiValue::fromString(normalizedFileName));
  return inputs;
}

bool isAlreadyExists(const WmiInfrastructureError& error) {
  return WmiError(static_cast<int32_t>(error.code().value())) == WmiErrorCode::AlreadyExists;
}

}  // namespace

std::vector<api::VolumeRow> UwfVolume::readAll() const {
  const auto rows = m_session.queryInstances("SELECT * FROM UWF_Volume");
  std::vector<api::VolumeRow> volumes;
  std::set<std::pair<bool, std::string>> managedIdentities;
  volumes.reserve(rows.size());
  for (const auto& source : rows) {
    rowutil::dumpRow("UWF_Volume", source);
    auto decoded = decodeManagedVolume(source);
    if (!decoded) continue;
    if (!managedIdentities.emplace(decoded->currentSession, decoded->driveLetter).second) {
      throw WmiProtocolError("read UWF_Volume",
                             std::format("duplicate {} session instance for {}", decoded->currentSession ? "current" : "next", decoded->driveLetter));
    }
    volumes.push_back(std::move(*decoded));
  }
  UWF_LOG_D("uwf") << "volume read completed: providerRows=" << rows.size() << " managedRows=" << volumes.size();
  return volumes;
}

void UwfVolume::protectVolume(const api::VolumeRow& row) const {
  requirePath(row, "protect UWF volume");
  invokeAndConfirm("protect UWF volume", [&] { m_session.invokeMethod(row.path, "Protect"); }, [&] { return rereadVolume(m_session, row).isProtected; });
}

void UwfVolume::unprotect(const api::VolumeRow& row) const {
  requirePath(row, "unprotect UWF volume");
  invokeAndConfirm("unprotect UWF volume", [&] { m_session.invokeMethod(row.path, "Unprotect"); }, [&] { return !rereadVolume(m_session, row).isProtected; });
}

void UwfVolume::commitFile(const api::VolumeRow& row, const std::string& fileFullPath) const {
  requirePath(row, "commit UWF file");
  const WmiRow inputs = fileInput(normalizeVolumePath(row, fileFullPath));
  m_session.invokeMethod(row.path, "CommitFile", inputs);
}

void UwfVolume::commitFileDeletion(const api::VolumeRow& row, const std::string& fileName) const {
  requirePath(row, "commit UWF file deletion");
  const WmiRow inputs = fileInput(normalizeVolumePath(row, fileName));
  m_session.invokeMethod(row.path, "CommitFileDeletion", inputs);
}

void UwfVolume::setBinding(const api::VolumeRow& row, const api::VolumeBinding binding) const {
  requirePath(row, "set UWF volume binding");
  const bool bindByDriveLetter = binding == api::VolumeBinding::DriveLetter;
  WmiRow inputs;
  inputs.emplace("bBindByDriveLetter", WmiValue::fromBool(bindByDriveLetter));
  invokeAndConfirm("set UWF volume binding", [&] { m_session.invokeMethod(row.path, "SetBindByDriveLetter", inputs); },
                   [&] { return rereadVolume(m_session, row).bindByDriveLetter == bindByDriveLetter; });
}

void UwfVolume::addExclusion(const api::VolumeRow& row, const std::string& fileName) const {
  requirePath(row, "add UWF file exclusion");
  const std::string normalized = normalizeVolumePath(row, fileName);
  const WmiRow inputs = fileInput(normalized);
  invokeAndConfirm("add UWF file exclusion", [&] { m_session.invokeMethod(row.path, "AddExclusion", inputs); },
                   [&] { return findExclusion(row, normalized); });
}

void UwfVolume::removeExclusion(const api::VolumeRow& row, const std::string& fileName) const {
  requirePath(row, "remove UWF file exclusion");
  const std::string normalized = normalizeVolumePath(row, fileName);
  const WmiRow inputs = fileInput(normalized);
  invokeAndConfirm("remove UWF file exclusion", [&] { m_session.invokeMethod(row.path, "RemoveExclusion", inputs); },
                   [&] { return !findExclusion(row, normalized); });
}

void UwfVolume::removeAllExclusions(const api::VolumeRow& row) const {
  requirePath(row, "remove all UWF file exclusions");
  invokeAndConfirm("remove all UWF file exclusions", [&] { m_session.invokeMethod(row.path, "RemoveAllExclusions"); },
                   [&] { return getExclusions(row).empty(); });
}

bool UwfVolume::findExclusion(const api::VolumeRow& row, const std::string& fileName) const {
  requirePath(row, "find UWF file exclusion");
  const WmiRow inputs = fileInput(normalizeVolumePath(row, fileName));
  const auto output = m_session.callMethodRead(row.path, "FindExclusion", inputs);
  return rowutil::requireBool(output.values, "bFound");
}

std::vector<api::ExcludedFile> UwfVolume::getExclusions(const api::VolumeRow& row) const {
  requirePath(row, "read UWF file exclusions");
  const auto output = m_session.callMethodRead(row.path, "GetExclusions");
  return rowutil::readArrayOutput<api::ExcludedFile>(output, "ExcludedFiles", [](const WmiRow& item) {
    return api::ExcludedFile{rowutil::requireEmbeddedString(item, "FileName")};
  });
}

VolumeRegistration UwfVolume::ensureNextSessionEntry(const std::string& driveLetter) const {
  const std::string normalizedDrive = drive::normalize(driveLetter);
  if (normalizedDrive.empty()) throw std::invalid_argument(std::format("invalid drive letter: {}", driveLetter));

  const auto volumes = readAll();
  if (const auto* existing = api::findBySession(volumes, api::Session::Next,
                                                [&](const api::VolumeRow& volume) { return volume.driveLetter == normalizedDrive; })) {
    return {*existing, VolumeRegistrationDisposition::AlreadyPresent};
  }
  const auto* current = api::findBySession(volumes, api::Session::Current,
                                           [&](const api::VolumeRow& volume) { return volume.driveLetter == normalizedDrive; });
  if (!current) throw WmiProtocolError("register UWF volume", std::format("no current-session instance exists for {}", normalizedDrive));

  WmiRow properties;
  properties.emplace("CurrentSession", WmiValue::fromBool(false));
  properties.emplace("DriveLetter", WmiValue::fromString(normalizedDrive));
  properties.emplace("VolumeName", WmiValue::fromString(current->volumeName));

  VolumeRegistrationDisposition outcome = VolumeRegistrationDisposition::Created;
  std::string uncertainFailure;
  try {
    m_session.putInstance("UWF_Volume", properties, WmiPutMode::CreateOnly);
  } catch (const WmiInfrastructureError& error) {
    if (isAlreadyExists(error)) {
      outcome = VolumeRegistrationDisposition::ConcurrentlyCreated;
    } else {
      throw;
    }
  } catch (const WmiInvocationUncertain& error) {
    uncertainFailure = error.what();
    outcome = VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite;
  }

  const std::string relativePath = std::format(R"(UWF_Volume.CurrentSession=FALSE,DriveLetter="{}",VolumeName="{}")",
                                               escapeWmiPathValue(normalizedDrive), escapeWmiPathValue(current->volumeName));
  std::optional<api::VolumeRow> observed;
  try {
    observed = decodeManagedVolume(m_session.getObject(relativePath));
  } catch (const std::exception& confirmationError) {
    std::string detail;
    if (outcome == VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite) {
      detail = "write outcome was uncertain and the created instance could not be confirmed; original failure: " + uncertainFailure;
    } else if (outcome == VolumeRegistrationDisposition::ConcurrentlyCreated) {
      detail = "the instance was concurrently created, but its state could not be confirmed";
    } else {
      detail = "provider accepted the registration, but the created instance could not be confirmed";
    }
    detail += "; reread failure: ";
    detail += confirmationError.what();
    std::throw_with_nested(WmiStateVerificationError("register UWF volume", detail));
  } catch (...) {
    const std::string detail = outcome == VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite
                                   ? "write outcome was uncertain and state confirmation failed with a non-standard exception; original failure: " +
                                         uncertainFailure
                                   : "registration state confirmation failed with a non-standard exception";
    std::throw_with_nested(WmiStateVerificationError("register UWF volume", detail));
  }

  if (!observed || observed->currentSession || observed->driveLetter != normalizedDrive || observed->volumeName != current->volumeName) {
    std::string detail;
    if (outcome == VolumeRegistrationDisposition::ConfirmedAfterUncertainWrite) {
      detail = "write outcome was uncertain and the observed instance does not match the requested next-session identity; original failure: " +
               uncertainFailure;
    } else if (outcome == VolumeRegistrationDisposition::ConcurrentlyCreated) {
      detail = "the concurrently created instance does not match the requested next-session identity";
    } else {
      detail = "provider accepted the registration, but the observed instance does not match the requested next-session identity";
    }
    throw WmiStateVerificationError("register UWF volume", detail);
  }
  return {std::move(*observed), outcome};
}

}  // namespace uwf::api
