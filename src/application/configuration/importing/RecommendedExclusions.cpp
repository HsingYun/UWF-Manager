// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "application/configuration/importing/RecommendedExclusions.h"

#include <QCoreApplication>
#include <string>
#include <utility>

namespace uwf::app {
std::vector<RecommendedExclusionGroup> recommendedExclusions(const std::string_view systemDrive) {
  const std::string system(systemDrive.empty() ? "C:" : systemDrive);
  const auto win = [&](const std::string& path) { return system + "\\Windows" + path; };
  const auto programData = [&](const std::string& path) { return system + "\\ProgramData" + path; };
  const auto programFiles = [&](const std::string& path) { return system + "\\Program Files" + path; };
  std::vector<RecommendedExclusionGroup> groups;
  const auto addGroup = [&](const RecommendedExclusionSection section, const char* title) { groups.push_back({section, title, {}}); };
  const auto addCommand = [&](const char* description, const UwfmgrKind kind, std::string path) {
    UwfmgrCommand command;
    command.kind = kind;
    command.args.push_back(std::move(path));
    groups.back().entries.push_back({description, std::move(command)});
  };
  const auto addFile = [&](const char* description, std::string path) { addCommand(description, UwfmgrKind::FileAddExclusion, std::move(path)); };
  const auto addReg = [&](const char* description, std::string path) { addCommand(description, UwfmgrKind::RegistryAddExclusion, std::move(path)); };

  {
    addGroup(RecommendedExclusionSection::Ceip, QT_TRANSLATE_NOOP("uwf", "Customer Experience Improvement Program (CEIP)"));
    addReg(QT_TRANSLATE_NOOP("uwf", "CEIP: persist policy opt-in state"), "HKEY_LOCAL_MACHINE\\Software\\Policies\\Microsoft\\SQMClient\\Windows");
    addReg(QT_TRANSLATE_NOOP("uwf", "CEIP: persist local opt-in state"), "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\SQMClient\\Windows");
    addReg(QT_TRANSLATE_NOOP("uwf", "CEIP: persist upload-disable flag"), "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\SQMClient\\UploadDisableFlag");
  }

  {
    addGroup(RecommendedExclusionSection::Bits, QT_TRANSLATE_NOOP("uwf", "Background Intelligent Transfer Service (BITS)"));
    addFile(QT_TRANSLATE_NOOP("uwf", "BITS: persist downloader queue files"), programData("\\Microsoft\\Network\\Downloader"));
    addReg(QT_TRANSLATE_NOOP("uwf", "BITS: persist transfer state index"),
           "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\BITS\\StateIndex");
  }

  {
    addGroup(RecommendedExclusionSection::Network, QT_TRANSLATE_NOOP("uwf", "Network profiles and policies"));
    addReg(QT_TRANSLATE_NOOP("uwf", "Wireless network GPO policy"), "HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\Wireless\\GPTWirelessPolicy");
    addReg(QT_TRANSLATE_NOOP("uwf", "Wired network GPO policy"), "HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\WiredL2\\GP_Policy");
    addFile(QT_TRANSLATE_NOOP("uwf", "Wireless network GPO policy files"), win("\\wlansvc\\Policies"));
    addFile(QT_TRANSLATE_NOOP("uwf", "Wired network GPO policy files"), win("\\dot2svc\\Policies"));
    addReg(QT_TRANSLATE_NOOP("uwf", "Wireless network interface profiles"), "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\wlansvc");
    addReg(QT_TRANSLATE_NOOP("uwf", "Wired network interface profiles"), "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\dot3svc");
    addReg(QT_TRANSLATE_NOOP("uwf", "Wireless service configuration"), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\Wlansvc");
    addReg(QT_TRANSLATE_NOOP("uwf", "Mobile broadband service configuration"), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\WwanSvc");
    addReg(QT_TRANSLATE_NOOP("uwf", "Wired AutoConfig service configuration"), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\dot3svc");
    groups.back().note =
        QT_TRANSLATE_NOOP("uwf", "Network profile XML files are per-device; add the concrete Interfaces\\{GUID}\\{GUID}.xml paths manually if needed.");
  }

  {
    addGroup(RecommendedExclusionSection::Dst, QT_TRANSLATE_NOOP("uwf", "Daylight saving time (DST)"));
    addReg(QT_TRANSLATE_NOOP("uwf", "DST: persist time zone definitions"), "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones");
    addReg(QT_TRANSLATE_NOOP("uwf", "DST: persist selected time zone information"),
           "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation");
  }

  {
    addGroup(RecommendedExclusionSection::Defender, QT_TRANSLATE_NOOP("uwf", "Microsoft Defender"));
    addFile(QT_TRANSLATE_NOOP("uwf", "Defender: persist product files and updates"), programFiles("\\Windows Defender"));
    addFile(QT_TRANSLATE_NOOP("uwf", "Defender: persist ProgramData signatures and state"), programData("\\Microsoft\\Windows Defender"));
    addFile(QT_TRANSLATE_NOOP("uwf", "Defender: persist Windows Update log"), win("\\WindowsUpdate.log"));
    addFile(QT_TRANSLATE_NOOP("uwf", "Defender: persist MpCmdRun log"), win("\\Temp\\MpCmdRun.log"));
    addReg(QT_TRANSLATE_NOOP("uwf", "Defender: persist product registry state"), "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Defender");
    addReg(QT_TRANSLATE_NOOP("uwf", "Defender: persist WdBoot service state"), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdBoot");
    addReg(QT_TRANSLATE_NOOP("uwf", "Defender: persist WdFilter service state"), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdFilter");
    addReg(QT_TRANSLATE_NOOP("uwf", "Defender: persist WdNisSvc service state"), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisSvc");
    addReg(QT_TRANSLATE_NOOP("uwf", "Defender: persist WdNisDrv service state"), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisDrv");
    addReg(QT_TRANSLATE_NOOP("uwf", "Defender: persist WinDefend service state"), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WinDefend");
  }

  {
    addGroup(RecommendedExclusionSection::Scep, QT_TRANSLATE_NOOP("uwf", "System Center Endpoint Protection"));
    addFile(QT_TRANSLATE_NOOP("uwf", "SCEP: persist client program files"), programFiles("\\Microsoft Security Client"));
    addFile(QT_TRANSLATE_NOOP("uwf", "SCEP: persist Windows Update log"), win("\\Windowsupdate.log"));
    addFile(QT_TRANSLATE_NOOP("uwf", "SCEP: persist MpCmdRun log"), win("\\Temp\\Mpcmdrun.log"));
    addFile(QT_TRANSLATE_NOOP("uwf", "SCEP: persist antimalware signatures and state"), programData("\\Microsoft\\Microsoft Antimalware"));
    addReg(QT_TRANSLATE_NOOP("uwf", "SCEP: persist antimalware registry state"), "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft Antimalware");
  }

  return groups;
}
}  // namespace uwf::app
