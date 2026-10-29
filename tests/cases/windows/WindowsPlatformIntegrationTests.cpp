// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QStringDecoder>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <future>
#include <memory>
#include <string>
#include <system_error>
#include <thread>

#include "app/commit/CommitPreparation.h"
#include "app/configuration/UwfmgrCommands.h"
#include "app/ports/RegistryKey.h"
#include "app/ports/TextEncoding.h"
#include "app/ports/VolumePaths.h"
#include "app/ports/WmiOperations.h"
#include "app/uwf/UwfStateReader.h"
#include "app/wmi/WmiError.h"
#include "app/wmi/WmiException.h"
#include "shared/StringUtil.h"
#include "windows/commit/WindowsCommitTraversalSource.h"
#include "windows/filesystem/AtomicTextFileWriter.h"
#include "windows/filesystem/BoundedTextFileReader.h"
#include "windows/filesystem/LocalFileBrowser.h"
#include "windows/ipc/EnhancedModePipeIo.h"
#include "windows/process/SecureSingleInstance.h"
#include "windows/security/ExecutableIdentity.h"
#include "windows/system/SystemCheck.h"
#include "windows/system/SystemHardwareInfo.h"
#include "windows/system/UnicodeConversion.h"
#include "windows/system/WindowsSystemError.h"
#include "windows/system/WindowsSystemInformationSource.h"
#include "windows/system/WindowsVersion.h"
#include "windows/wmi/WmiSession.h"

namespace uwf {

class WindowsPlatformIntegrationTests final : public QObject {
  Q_OBJECT

 private slots:
  void windowsAndHardwareMetadataRemainInternallyConsistent();
  void executableIdentityDistinguishesAliasesFromCopies();
  void cimv2TransportDistinguishesPresentAndMissingClasses();
  void failedWmiReadsLeaveSubsequentReadsUsable_data();
  void failedWmiReadsLeaveSubsequentReadsUsable();
  void volumeMountQueryIncludesTheSystemDrive();
  void unicodeConversionPreservesValidDataAndRejectsMalformedInput();
  void windowsErrorsPreserveUtf8TextAndSystemCodes_data();
  void windowsErrorsPreserveUtf8TextAndSystemCodes();
  void registryReadFailuresExposeUtf8Text();
  void nativeCommitTraversalContracts_data();
  void nativeCommitTraversalContracts();
  void driveResolutionHonorsLiteralPathBoundaries_data();
  void driveResolutionHonorsLiteralPathBoundaries();
  void embeddedCapabilityAndSnapshotUseTheProductionTransport();
  void explicitWmiShutdownRetiresTheWorkerContext();
  void enhancedModePipeReadIsCancelledByStopEvent();
  void singleInstanceForwardsTypedCommandsAcrossTrustedProcesses();
  void commandArgumentsSurviveTheWindowsShell_data();
  void commandArgumentsSurviveTheWindowsShell();
  void commandDependenciesSurviveTheWindowsShell();
  void textExportsAreAtomicAndPreserveUtf8();
  void textReadsRespectByteLimits_data();
  void textReadsRespectByteLimits();
  void fileBrowserRenamesRespectExistingTargets_data();
  void fileBrowserRenamesRespectExistingTargets();
};

void WindowsPlatformIntegrationTests::fileBrowserRenamesRespectExistingTargets_data() {
  QTest::addColumn<bool>("directory");
  QTest::addColumn<bool>("collision");
  QTest::newRow("file-case-change") << false << false;
  QTest::newRow("directory-case-change") << true << false;
  QTest::newRow("file-existing-target") << false << true;
  QTest::newRow("directory-existing-target") << true << true;
}

void WindowsPlatformIntegrationTests::fileBrowserRenamesRespectExistingTargets() {
  QFETCH(bool, directory);
  QFETCH(bool, collision);
  QTemporaryDir root;
  QVERIFY(root.isValid());
  const QString originalName = directory ? QStringLiteral("Folder") : QStringLiteral("report.txt");
  const QString targetName = collision ? (directory ? QStringLiteral("Existing") : QStringLiteral("existing.txt")) : originalName.toUpper();
  const auto source = root.filePath(originalName);
  const auto target = root.filePath(targetName);
  const auto contentPath = [directory](const QString& path) { return directory ? QDir(path).filePath(QStringLiteral("payload.txt")) : path; };
  const auto create = [&](const QString& path, const QByteArray& content) {
    if (directory && !QDir().mkdir(path)) return false;
    QFile file(contentPath(path));
    return file.open(QIODevice::WriteOnly) && file.write(content) == content.size();
  };
  const auto read = [&](const QString& path) {
    QFile file(contentPath(path));
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray{};
  };
  QVERIFY(create(source, "source payload"));
  if (collision) QVERIFY(create(target, "existing payload"));
  const auto browser = windows::systemFileBrowser();
  if (collision) {
    try {
      browser->renameEntry(source, target);
      QFAIL("Renaming must not replace an existing target");
    } catch (const app::FileBrowserError& error) {
      QCOMPARE(error.code(), app::FileBrowserErrorCode::AlreadyExists);
    }
    QCOMPARE(read(source), QByteArray("source payload"));
    QCOMPARE(read(target), QByteArray("existing payload"));
  } else {
    browser->renameEntry(source, target);
    const auto names = QDir(root.path()).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QVERIFY(names.contains(targetName, Qt::CaseSensitive));
    QVERIFY(!names.contains(originalName, Qt::CaseSensitive));
    QCOMPARE(read(target), QByteArray("source payload"));
  }
}

void WindowsPlatformIntegrationTests::nativeCommitTraversalContracts_data() {
  QTest::addColumn<bool>("registry");
  QTest::addColumn<bool>("deleting");
  for (const bool registry : {false, true})
    for (const bool deleting : {false, true}) QTest::newRow(qUtf8Printable(QStringLiteral("%1-%2").arg(registry).arg(deleting))) << registry << deleting;
}

void WindowsPlatformIntegrationTests::nativeCommitTraversalContracts() {
  QFETCH(bool, registry);
  QFETCH(bool, deleting);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const auto key = QStringLiteral("HKEY_CURRENT_USER\\Software\\UwfTests\\%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  const auto cleanup = qScopeGuard([&] {
    if (registry) regkey::deleteTree(key.toStdString());
  });
  if (registry) {
    for (const auto& name : {QString(), QStringLiteral("One")}) regkey::writeMultiString(key.toStdString(), name.toStdString(), {"payload"});
    regkey::writeMultiString((key + QStringLiteral("\\Branch")).toStdString(), "Two", {"payload"});
  } else {
    QVERIFY(QDir(directory.path()).mkpath(QStringLiteral("Branch")));
    for (const auto& name : {QStringLiteral("default.txt"), QStringLiteral("one.txt"), QStringLiteral("Branch/two.txt")}) {
      QFile file(directory.filePath(name));
      QVERIFY(file.open(QIODevice::WriteOnly));
      QCOMPARE(file.write("payload"), 7);
    }
  }
  const auto source = windows::systemCommitTraversalSource();
  const app::CommitRequest request{
      deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist,
      {registry ? app::CommitTargetKind::RegistryKey : app::CommitTargetKind::Folder, registry ? key : QDir::toNativeSeparators(directory.path()), {}, {}}};
  const auto plan = app::prepareCommit(request, *source, {}, {}, {});
  QCOMPARE(plan.progress().items, 3);
  QCOMPARE(plan.progress().containers, 2);
  QCOMPARE(plan.progress().failed, 0);
  const auto targets = plan.targets();
  QCOMPARE(targets.size(), deleting ? registry ? 2 : 5 : 3);
  if (deleting) QCOMPARE(targets.back().identity(), request.target.identity());
  std::stop_source stop;
  const auto canceled = app::prepareCommit(request, *source, {}, stop.get_token(), [&](const auto& progress) {
    if (progress.items) stop.request_stop();
  });
  QVERIFY(canceled.canceled);
  QVERIFY(canceled.targets().isEmpty());
  QCOMPARE(canceled.progress().items, 1);
}

void WindowsPlatformIntegrationTests::textExportsAreAtomicAndPreserveUtf8() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const auto path = directory.filePath(QStringLiteral("导出.txt"));
  using Failure = app::TextFileWriteResult::Failure;
  const QString original = QStringLiteral("Unicode 内容\nsecond line\n");
  QCOMPARE(windows::writeUtf8TextFile(path, original).failure, Failure::None);
  const auto nativePath = path.toStdWString();
  const HANDLE locked = CreateFileW(nativePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  QVERIFY(locked != INVALID_HANDLE_VALUE);
  const auto unlock = qScopeGuard([&] { CloseHandle(locked); });
  const auto failed = windows::writeUtf8TextFile(path, QStringLiteral("replacement"));
  QCOMPARE(failed.failure, Failure::Write);
  QVERIFY(!failed.error.isEmpty());
  QFile file(path);
  QVERIFY(file.open(QIODevice::ReadOnly));
  QCOMPARE(file.readAll(), original.toUtf8());
  file.close();
  QCOMPARE(windows::writeUtf8TextFile(directory.path(), original).failure, Failure::Open);
}

void WindowsPlatformIntegrationTests::textReadsRespectByteLimits_data() {
  QTest::addColumn<QByteArray>("bytes");
  QTest::newRow("empty") << QByteArray{};
  QTest::newRow("plain") << QByteArray("plain text\n");
  QTest::newRow("utf8") << QStringLiteral("中文\n").toUtf8();
  QTest::newRow("windows-lines") << QByteArray("first\r\nsecond\r\n");
}

void WindowsPlatformIntegrationTests::textReadsRespectByteLimits() {
  QFETCH(QByteArray, bytes);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const auto path = directory.filePath(QStringLiteral("input.txt"));
  QFile file(path);
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write(bytes), bytes.size());
  file.close();
  using Failure = app::TextFileReadResult::Failure;
  for (qint64 limit = 0; limit <= bytes.size() + 1; ++limit) {
    const auto result = windows::readBoundedTextFile(path, limit);
    QCOMPARE(result.failure, bytes.size() <= limit ? Failure::None : Failure::TooLarge);
    QCOMPARE(result.bytes, bytes.size() <= limit ? bytes : QByteArray{});
  }
  QCOMPARE(windows::readBoundedTextFile(directory.filePath(QStringLiteral("missing.txt")), 10).failure, Failure::Open);
}

void WindowsPlatformIntegrationTests::windowsErrorsPreserveUtf8TextAndSystemCodes_data() {
  QTest::addColumn<quint32>("code");
  QTest::newRow("access-denied") << quint32{ERROR_ACCESS_DENIED};
  QTest::newRow("invalid-parameter") << quint32{ERROR_INVALID_PARAMETER};
  QTest::newRow("invalid-unicode") << quint32{ERROR_NO_UNICODE_TRANSLATION};
  QTest::newRow("unknown-code") << std::numeric_limits<quint32>::max();
}

void WindowsPlatformIntegrationTests::windowsErrorsPreserveUtf8TextAndSystemCodes() {
  QFETCH(quint32, code);
  const auto operation = QStringLiteral("读取注册表键");
  try {
    throw windows::WindowsSystemError(code, operation.toUtf8().toStdString());
  } catch (const std::system_error& error) {
    QCOMPARE(error.code(), std::error_code(static_cast<int>(code), std::system_category()));
    if (code == ERROR_ACCESS_DENIED) QVERIFY(error.code() == std::errc::permission_denied);
    QStringDecoder decode(QStringDecoder::Utf8);
    const QString text = decode(QByteArray(error.what()));
    QVERIFY(!decode.hasError());
    QVERIFY(text.startsWith(operation + QStringLiteral(": ")));
    std::array<wchar_t, 4096> native{};
    const auto count =
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, 0, native.data(), static_cast<DWORD>(native.size()), nullptr);
    const auto expected = count ? QString::fromWCharArray(native.data(), count).trimmed() : QStringLiteral("Windows error %1").arg(code);
    QCOMPARE(text, operation + QStringLiteral(": ") + expected);
  }
}

void WindowsPlatformIntegrationTests::registryReadFailuresExposeUtf8Text() {
  try {
    static_cast<void>(regkey::subkeyNames("INVALID_HIVE\\Software"));
    QFAIL("An invalid hive must fail instead of returning an empty key list");
  } catch (const std::system_error& error) {
    QCOMPARE(error.code(), std::error_code(ERROR_INVALID_PARAMETER, std::system_category()));
    QStringDecoder decode(QStringDecoder::Utf8);
    const QString text = decode(QByteArray(error.what()));
    QVERIFY(!decode.hasError());
    QVERIFY(text.startsWith(QStringLiteral("open registry key for reading: ")));
  }
}

void WindowsPlatformIntegrationTests::unicodeConversionPreservesValidDataAndRejectsMalformedInput() {
  const std::string utf8 = "UWF \xE8\xA6\x86\xE7\x9B\x96\xE5\xB1\x82";
  QCOMPARE(QString::fromUtf8(uwf::windows::wideToUtf8(uwf::windows::utf8ToWide(utf8))), QString::fromUtf8(utf8));
  QVERIFY_THROWS_EXCEPTION(std::system_error, (void)uwf::windows::utf8ToWide(std::string_view("\xC3\x28", 2)));
  QVERIFY(uwf::windows::wideToUtf8(nullptr).empty());
  QVERIFY_THROWS_EXCEPTION(std::system_error, (void)uwf::windows::wideToUtf8(std::wstring(1, static_cast<wchar_t>(0xD800))));
  QCOMPARE(app::decodeUtf8Text(utf8), QString::fromUtf8(utf8));
  QVERIFY_THROWS_EXCEPTION(std::system_error, (void)app::decodeUtf8Text(std::string_view("\xC3\x28", 2)));
}

void WindowsPlatformIntegrationTests::driveResolutionHonorsLiteralPathBoundaries_data() {
  QTest::addColumn<QString>("path");
  QTest::addColumn<QString>("letter");
  QTest::addColumn<bool>("malformedVolume");
  const auto add = [](const char* name, const QString& path, const QString& letter = {}, const bool malformed = false) {
    QTest::newRow(name) << path << letter << malformed;
  };
  add("dos-path", QStringLiteral("c:/Data"), QStringLiteral("C:"));
  add("extended-path", QStringLiteral(R"(\\?\d:\Data)"), QStringLiteral("D:"));
  add("device-prefix", QStringLiteral(R"(\\.\z:\Data)"), QStringLiteral("Z:"));
  add("drive-name", QStringLiteral("c:"), QStringLiteral("C:"));
  add("drive-relative", QStringLiteral("c:Data"), QStringLiteral("C:"));
  add("leading-space", QStringLiteral(R"( C:\Data)"));
  add("trailing-space", QStringLiteral("x:/Data "), QStringLiteral("X:"));
  add("unc", QStringLiteral(R"(\\server\share\file)"));
  add("extended-unc", QStringLiteral(R"(\\?\UNC\server\share\file)"));
  add("relative", QStringLiteral("relative/file"));
  add("single-letter", QStringLiteral("C"));
  add("empty", {});
  add("numeric-drive", QStringLiteral("1:/Data"));
  add("unicode-drive", QStringLiteral("\u00c5:/Data"));
  add("missing-brace", QStringLiteral(R"(\\?\Volume{missing-brace\path)"), {}, true);
  add("missing-separator", QStringLiteral(R"(\\?\Volume{abc})"), {}, true);
  add("invalid-separator", QStringLiteral(R"(\\?\Volume{abc}x\file)"), {}, true);
  add("malformed-device-volume", QStringLiteral(R"(\\.\Volume{abc})"), {}, true);
}

void WindowsPlatformIntegrationTests::driveResolutionHonorsLiteralPathBoundaries() {
  QFETCH(QString, path);
  QFETCH(QString, letter);
  QFETCH(bool, malformedVolume);
  if (malformedVolume) {
    QVERIFY_THROWS_EXCEPTION(drive::DriveLetterResolutionError, (void)drive::fromPath(path.toStdString()));
  } else {
    QCOMPARE(QString::fromStdString(drive::fromPath(path.toStdString())), letter);
  }
}

void WindowsPlatformIntegrationTests::executableIdentityDistinguishesAliasesFromCopies() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const QString first = directory.filePath(QStringLiteral("first.exe"));
  const QString copy = directory.filePath(QStringLiteral("copy.exe"));
  const QString alias = directory.filePath(QStringLiteral("alias.exe"));
  for (const auto& path : {first, copy}) {
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("identity"), qint64{8});
  }
  QVERIFY(CreateHardLinkW(alias.toStdWString().c_str(), first.toStdWString().c_str(), nullptr));
  const auto executable = windows::processImagePath(GetCurrentProcess());
  const auto current = windows::currentExecutablePath();
  QVERIFY(!current.empty());
  QVERIFY(!executable.empty());
  QVERIFY(windows::processImagePath(nullptr).empty());
  for (const auto format : {windows::FileIdentityFormat::FileIndex64, windows::FileIdentityFormat::FileId128}) {
    QVERIFY(windows::sameFile(first.toStdWString(), alias.toStdWString(), format));
    QVERIFY(!windows::sameFile(first.toStdWString(), copy.toStdWString(), format));
    QVERIFY(!windows::sameFile(first.toStdWString(), directory.filePath(QStringLiteral("missing.exe")).toStdWString(), format));
    QVERIFY(windows::sameFile(executable, QCoreApplication::applicationFilePath().toStdWString(), format));
    QVERIFY(windows::sameFile(executable, current, format));
  }
}

void WindowsPlatformIntegrationTests::windowsAndHardwareMetadataRemainInternallyConsistent() {
  const auto& version = uwf::windows::windowsVersionInfo();
  QVERIFY(version.major != 0);
  QVERIFY(version.build != 0);
  QVERIFY(!version.productName.empty());

  const auto& hardware = uwf::windows::systemHardwareInfo();
  QVERIFY(hardware.totalMemoryBytes != 0);
  std::uint64_t installedBytes = 0;
  for (const auto& module : hardware.memoryModules) {
    QVERIFY(module.capacityBytes != 0);
    QVERIFY(module.capacityBytes <= hardware.totalMemoryBytes);
    installedBytes += module.capacityBytes;
  }
  if (!hardware.memoryModules.empty()) QCOMPARE(installedBytes, hardware.totalMemoryBytes);

  const auto check = uwf::windows::runSystemChecks();
  QCOMPARE(check.productName, version.productName);
  QCOMPARE(check.editionId, version.editionId);
  QVERIFY(check.status == uwf::windows::CheckStatus::Ok || check.status == uwf::windows::CheckStatus::UnsupportedSystem);
  const std::string manager = uwf::windows::uwfmgrPath();
  if (!manager.empty()) QVERIFY(std::filesystem::is_regular_file(manager));
}

void WindowsPlatformIntegrationTests::cimv2TransportDistinguishesPresentAndMissingClasses() {
  try {
    auto& session = uwf::windows::cimv2WmiSession();
    session.ensureConnected();
    QCOMPARE(session.classStatus("Win32_OperatingSystem"), uwf::app::wmi::WmiClassStatus::Present);
    QCOMPARE(session.classStatus("UwfManager_Class_That_Does_Not_Exist"), uwf::app::wmi::WmiClassStatus::Missing);
    const auto rows = session.queryInstances("SELECT Caption, ProductType FROM Win32_OperatingSystem");
    QCOMPARE(rows.size(), std::size_t{1});
    QVERIFY(rows.front().contains("__PATH"));
    QVERIFY(!rows.front().at("Caption").toString().empty());
    QVERIFY(rows.front().at("ProductType").toUInt() >= 1);
  } catch (const std::exception& error) {
    QFAIL(error.what());
  } catch (...) {
    QFAIL("CIMv2 transport raised a non-standard exception");
  }
}

void WindowsPlatformIntegrationTests::failedWmiReadsLeaveSubsequentReadsUsable_data() {
  QTest::addColumn<QString>("operation");
  for (const auto* operation : {"invalid-query", "missing-object", "missing-method", "canceled-read"})
    QTest::newRow(operation) << QString::fromLatin1(operation);
}

void WindowsPlatformIntegrationTests::failedWmiReadsLeaveSubsequentReadsUsable() {
  QFETCH(QString, operation);
  auto& session = windows::cimv2WmiSession();
  const auto failRead = [&] {
    if (operation == QStringLiteral("invalid-query")) {
      static_cast<void>(session.query("SELECT FROM"));
    } else if (operation == QStringLiteral("missing-object")) {
      static_cast<void>(session.getObject("UwfManager_Class_That_Does_Not_Exist"));
    } else if (operation == QStringLiteral("missing-method")) {
      static_cast<void>(session.callMethodRead("Win32_OperatingSystem", "UwfManager_Method_That_Does_Not_Exist"));
    } else {
      std::stop_source stop;
      stop.request_stop();
      static_cast<void>(session.callMethodReadCancelable("Win32_OperatingSystem", "UwfManager_Method_That_Does_Not_Exist", {}, stop.get_token()));
    }
  };
  QVERIFY_THROWS_EXCEPTION(app::wmi::WmiException, failRead());
  QCOMPARE(session.classStatus("Win32_OperatingSystem"), app::wmi::WmiClassStatus::Present);
  QVERIFY(!session.getObject("Win32_OperatingSystem").empty());
  const auto rows = session.query("SELECT Caption FROM Win32_OperatingSystem");
  QCOMPARE(rows.size(), std::size_t{1});
  QVERIFY(!rows.front().at("Caption").toString().empty());
  static_cast<void>(app::probeUwfCapability(windows::embeddedWmiSession()));
}

void WindowsPlatformIntegrationTests::volumeMountQueryIncludesTheSystemDrive() {
  try {
    const auto systemDrive = drive::systemLetter();
    QVERIFY(!systemDrive.empty());
    const auto root = uwf::windows::utf8ToWide(systemDrive + '\\');
    std::array<wchar_t, 50> volumeName{};
    QVERIFY(GetVolumeNameForVolumeMountPointW(root.c_str(), volumeName.data(), static_cast<DWORD>(volumeName.size())));
    const auto letters = drive::mountedLetters(uwf::windows::wideToUtf8(volumeName.data()));
    QVERIFY(std::ranges::find(letters, systemDrive) != letters.end());
    for (const auto& letter : letters) {
      QCOMPARE(letter, drive::normalize(letter));
      QCOMPARE(letter.size(), std::size_t{2});
    }
  } catch (const std::exception& error) {
    QFAIL(error.what());
  }
}

void WindowsPlatformIntegrationTests::embeddedCapabilityAndSnapshotUseTheProductionTransport() {
  try {
    auto& session = uwf::windows::embeddedWmiSession();
    const uwf::app::UwfCapability capability = uwf::app::probeUwfCapability(session);
    QVERIFY(capability == uwf::app::UwfCapability::Available || capability == uwf::app::UwfCapability::Unavailable);
    try {
      const auto snapshot = uwf::app::readSnapshot(session, capability, uwf::windows::isElevated(), drive::mountedLetters);
      QCOMPARE(snapshot.uwfAvailable, capability == uwf::app::UwfCapability::Available);
      if (snapshot.uwfAvailable) {
        QVERIFY(snapshot.unavailableReason.empty());
        QCOMPARE(snapshot.elevated, uwf::windows::isElevated());
      } else {
        QVERIFY(!snapshot.unavailableReason.empty());
      }
    } catch (const uwf::app::wmi::WmiInfrastructureError& error) {
      const auto wmiError = uwf::app::wmi::WmiError(static_cast<std::int32_t>(error.code().value()));
      if (uwf::windows::isElevated() || wmiError.code() != uwf::app::wmi::WmiErrorCode::AccessDenied) throw;
      QVERIFY(capability == uwf::app::UwfCapability::Available);
    }
  } catch (const std::exception& error) {
    QFAIL(error.what());
  } catch (...) {
    QFAIL("Embedded WMI transport raised a non-standard exception");
  }
}

void WindowsPlatformIntegrationTests::explicitWmiShutdownRetiresTheWorkerContext() {
  std::promise<QString> outcomePromise;
  auto outcome = outcomePromise.get_future();
  std::jthread worker([promise = std::move(outcomePromise)]() mutable {
    try {
      {
        uwf::windows::initializeWmiRuntime();
        const auto shutdownWmi = qScopeGuard([] { uwf::windows::shutdownWmiRuntime(); });
        auto* const firstSession = &uwf::windows::cimv2WmiSession();
        firstSession->ensureConnected();
        uwf::windows::initializeWmiRuntime();
        if (&uwf::windows::cimv2WmiSession() != firstSession) {
          promise.set_value(QStringLiteral("active WMI context was replaced"));
          return;
        }
      }

      try {
        static_cast<void>(uwf::windows::cimv2WmiSession());
        promise.set_value(QStringLiteral("retired WMI context was recreated"));
      } catch (const std::logic_error&) {
        promise.set_value({});
      }
    } catch (const std::exception& error) {
      promise.set_value(QString::fromUtf8(error.what()));
    } catch (...) {
      promise.set_value(QStringLiteral("worker raised a non-standard exception"));
    }
  });

  worker.join();
  const QString error = outcome.get();
  QVERIFY2(error.isEmpty(), qUtf8Printable(error));
}

void WindowsPlatformIntegrationTests::enhancedModePipeReadIsCancelledByStopEvent() {
  const std::wstring pipeName = QStringLiteral("\\\\.\\pipe\\UWFManager.Tests.%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)).toStdWString();
  const HANDLE server = CreateNamedPipeW(pipeName.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                         PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, 1, 1024, 1024, 0, nullptr);
  QVERIFY(server != INVALID_HANDLE_VALUE);
  const auto closeServer = qScopeGuard([server] { CloseHandle(server); });

  const HANDLE client = CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
  QVERIFY(client != INVALID_HANDLE_VALUE);
  const auto closeClient = qScopeGuard([client] { CloseHandle(client); });

  const HANDLE stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  QVERIFY(stopEvent);
  const auto closeStopEvent = qScopeGuard([stopEvent] { CloseHandle(stopEvent); });
  QCOMPARE(windows::connectEnhancedPipe(server, stopEvent), windows::EnhancedPipeIoResult::Completed);

  std::promise<windows::EnhancedPipeIoResult> resultPromise;
  auto result = resultPromise.get_future();
  std::jthread reader([server, stopEvent, promise = std::move(resultPromise)]() mutable {
    try {
      std::array<std::byte, 1> buffer{};
      promise.set_value(windows::readEnhancedPipe(server, buffer, stopEvent));
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
  });

  QTest::qWait(10);
  QVERIFY(SetEvent(stopEvent));
  QCOMPARE(result.wait_for(std::chrono::seconds{1}), std::future_status::ready);
  QCOMPARE(result.get(), windows::EnhancedPipeIoResult::Stopped);
}

void WindowsPlatformIntegrationTests::singleInstanceForwardsTypedCommandsAcrossTrustedProcesses() {
  const QString discriminator = QStringLiteral("UwfWindowsPlatformIntegrationTests.%1").arg(QCoreApplication::applicationPid());
  uwf::windows::SecureSingleInstance instance(uwf::windows::SecureSingleInstance::Scope{discriminator});
  QSignalSpy activation(&instance, &uwf::windows::SecureSingleInstance::activationRequested);
  QSignalSpy commit(&instance, &uwf::windows::SecureSingleInstance::commitStageRequested);
  const auto first = instance.acquire();
  QCOMPARE(instance.acquire(), first);
  QCOMPARE(first, uwf::windows::SecureSingleInstance::AcquireResult::Primary);
  QVERIFY(instance.errorString().isEmpty());
  instance.enableCommandNotifications();
  QCOMPARE(activation.count(), 0);

  const auto launchClient = [&](const QString& command) {
    auto process = std::make_unique<QProcess>();
    process->setProgram(QCoreApplication::applicationFilePath());
    process->setArguments({QStringLiteral("--single-instance-test-client"), discriminator, command});
    process->start();
    return process;
  };

  auto activateClient = launchClient(QStringLiteral("activate"));
  QVERIFY2(activateClient->waitForStarted(), qUtf8Printable(activateClient->errorString()));
  QTRY_COMPARE_WITH_TIMEOUT(activation.count(), 1, 3000);
  QTRY_COMPARE_WITH_TIMEOUT(activateClient->state(), QProcess::NotRunning, 3000);
  QCOMPARE(activateClient->exitStatus(), QProcess::NormalExit);
  QCOMPARE(activateClient->exitCode(), 0);

  auto quietClient = launchClient(QStringLiteral("ensure-running"));
  QVERIFY2(quietClient->waitForStarted(), qUtf8Printable(quietClient->errorString()));
  QTRY_COMPARE_WITH_TIMEOUT(quietClient->state(), QProcess::NotRunning, 3000);
  QCOMPARE(quietClient->exitStatus(), QProcess::NormalExit);
  QCOMPARE(quietClient->exitCode(), 0);
  QCOMPARE(activation.count(), 1);
  QCOMPARE(commit.count(), 0);

  const app::ApplicationCommandResult expected{app::ApplicationCommandOutcome::CompletedWithFailures, 3, 1, 1, 2, 1, QStringLiteral("one staged file failed")};
  const QMetaObject::Connection completion = connect(&instance, &uwf::windows::SecureSingleInstance::commitStageRequested, &instance,
                                                     [&](const std::uint64_t requestToken) { instance.completeCommand(requestToken, expected); });
  auto commitClient = launchClient(QStringLiteral("commit-stage"));
  QVERIFY2(commitClient->waitForStarted(), qUtf8Printable(commitClient->errorString()));
  QTRY_COMPARE_WITH_TIMEOUT(commit.count(), 1, 3000);
  QTRY_COMPARE_WITH_TIMEOUT(commitClient->state(), QProcess::NotRunning, 3000);
  disconnect(completion);
  QCOMPARE(commitClient->exitStatus(), QProcess::NormalExit);
  QCOMPARE(commitClient->exitCode(), 0);
}

void WindowsPlatformIntegrationTests::commandArgumentsSurviveTheWindowsShell_data() {
  QTest::addColumn<QString>("argument");
  QTest::addColumn<bool>("batch");
  QTest::addColumn<bool>("delayedExpansion");
  const QStringList arguments = {QStringLiteral("Z:\\R&ver"),
                                 QStringLiteral("Z:\\A (B)\\"),
                                 QStringLiteral("Z:\\A^B"),
                                 QStringLiteral("HKLM\\Software\\A\"&ver&\"B"),
                                 QStringLiteral("HKLM\\Software\\A\\\"&ver&\"B\\"),
                                 QStringLiteral("HKLM\\Software\\A\"&ver"),
                                 QStringLiteral("HKLM\\Software\\A\"^&|<>() B\\"),
                                 QStringLiteral("HKLM\\Software\\A\"\"B"),
                                 QStringLiteral("Z:\\资料&ver"),
                                 QStringLiteral("Z:\\%UWF_TEST_LITERAL%\\Cache"),
                                 QStringLiteral("Z:\\%UWF_TEST_UNDEFINED%\\Cache"),
                                 QStringLiteral("Z:\\%1\\%%C\\100%"),
                                 QStringLiteral("Z:\\!UWF_TEST_LITERAL!\\Cache!"),
                                 QStringLiteral("HKLM\\Software\\A\"%UWF_TEST_LITERAL%&!UWF_TEST_LITERAL!\"B\\"),
                                 QStringLiteral("Z:\\资料 %UWF_TEST_LITERAL% !UWF_TEST_LITERAL!\\")};
  for (qsizetype index = 0; index < arguments.size(); ++index)
    for (const bool batch : {false, true})
      for (const bool delayedExpansion : {false, true}) {
        // Raw text remains an internal literal representation. Its existing
        // non-variable shell cases are retained only as regression coverage.
        if (!batch && index >= 9) continue;
        QTest::newRow(qUtf8Printable(QStringLiteral("argument-%1-%2-delayed-%3").arg(index).arg(batch ? "batch" : "raw").arg(delayedExpansion)))
            << arguments[index] << batch << delayedExpansion;
      }
}

void WindowsPlatformIntegrationTests::commandArgumentsSurviveTheWindowsShell() {
  QFETCH(QString, argument);
  QFETCH(bool, batch);
  QFETCH(bool, delayedExpansion);
  wchar_t systemDirectory[MAX_PATH]{};
  const UINT length = GetSystemDirectoryW(systemDirectory, MAX_PATH);
  QVERIFY(length > 0 && length < MAX_PATH);
  const QString cmd = QDir(QString::fromWCharArray(systemDirectory)).filePath(QStringLiteral("cmd.exe"));
  const QString client = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
  app::UwfmgrCommand command;
  command.kind = app::UwfmgrKind::FileAddExclusion;
  command.args = {argument.toStdString()};
  const auto prefix = QStringLiteral("uwfmgr.exe file add-exclusion ");
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const auto scriptPath = directory.filePath(QStringLiteral("arguments.cmd"));
  QString invocation;
  if (batch) {
    auto script = QString::fromStdString(app::renderCommandScript({command}));
    script.replace(prefix, QStringLiteral("\"%1\" --command-argument-test-client ").arg(client));
    QVERIFY(!script.contains(QStringLiteral("uwfmgr.exe")));
    QFile file(scriptPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    const auto bytes = script.toUtf8();
    QCOMPARE(file.write(bytes), bytes.size());
    file.close();
    // Set and observe the code page in the same child console that runs the
    // script; QProcess children need not inherit the parent's console state.
    invocation = QStringLiteral("chcp 437 >nul & chcp & call \"%1\" & chcp").arg(QDir::toNativeSeparators(scriptPath));
  } else {
    const auto value = QString::fromStdString(app::renderCommand(command)).mid(prefix.size());
    invocation = QStringLiteral("\"%1\" --command-argument-test-client %2").arg(client, value);
  }
  QProcess process;
  auto environment = QProcessEnvironment::systemEnvironment();
  environment.insert(QStringLiteral("UWF_TEST_LITERAL"), QStringLiteral("must-not-expand"));
  environment.remove(QStringLiteral("UWF_TEST_UNDEFINED"));
  process.setProcessEnvironment(environment);
  process.setProgram(cmd);
  process.setNativeArguments(QStringLiteral("/d /v:%1 /s /c \"%2\"").arg(delayedExpansion ? "on" : "off", invocation));
  process.start();
  QVERIFY2(process.waitForStarted(), qUtf8Printable(process.errorString()));
  QVERIFY2(process.waitForFinished(5000), qUtf8Printable(argument));
  QCOMPARE(process.exitStatus(), QProcess::NormalExit);
  QCOMPARE(process.exitCode(), 0);
  const auto output = process.readAllStandardOutput();
  if (batch) {
    const auto firstLineEnd = output.indexOf('\n');
    QVERIFY(firstLineEnd >= 0);
    const auto baselineCodepage = output.first(firstLineEnd + 1);
    QVERIFY(baselineCodepage.contains("437"));
    QCOMPARE(output, baselineCodepage + argument.toUtf8() + baselineCodepage);
  } else {
    QCOMPARE(output, argument.toUtf8());
  }
  QVERIFY(process.readAllStandardError().isEmpty());
}

void WindowsPlatformIntegrationTests::commandDependenciesSurviveTheWindowsShell() {
  using enum app::UwfmgrKind;
  core::PendingChanges changes;
  changes.setOverlay.type = core::OverlayType::Disk;
  changes.setOverlay.maximumSizeMb = 2048;
  changes.setOverlay.warningThresholdMb = 1024;
  changes.setOverlay.criticalThresholdMb = 1536;
  changes.volumeProtect["C:"] = true;
  const core::OverlayConfig baseline{core::OverlayType::RAM, 512, 256, 384};
  const auto commands = app::renderPendingChanges(changes, baseline);
  const auto lines = app::renderCommandLines(commands);
  wchar_t systemDirectory[MAX_PATH]{};
  const UINT length = GetSystemDirectoryW(systemDirectory, MAX_PATH);
  QVERIFY(length > 0 && length < MAX_PATH);
  const QString cmd = QDir(QString::fromWCharArray(systemDirectory)).filePath(QStringLiteral("cmd.exe"));
  const QString client = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  for (const auto failed : {Unknown, OverlaySetSize, OverlaySetCriticalThreshold, VolumeProtect}) {
    QByteArray expected;
    bool previousSucceeded = true;
    for (const auto& command : commands) {
      if (command.requiresPrevious && !previousSucceeded) continue;
      expected += QByteArray::number(static_cast<int>(command.kind)) + '\n';
      previousSucceeded = command.kind != failed;
    }
    QByteArray observed;
    for (const auto& line : lines) {
      QString substituted = QString::fromStdString(line);
      for (const auto& command : commands) {
        const auto replacement =
            QStringLiteral("\"%1\" --command-step-test-client %2 %3").arg(client).arg(static_cast<int>(command.kind)).arg(command.kind == failed ? 7 : 0);
        substituted.replace(QString::fromStdString(app::renderCommand(command)), replacement);
      }
      QVERIFY(!substituted.contains(QStringLiteral("uwfmgr.exe")));
      QProcess process;
      process.setProgram(cmd);
      process.setNativeArguments(QStringLiteral("/d /v:off /s /c \"%1\"").arg(substituted));
      process.start();
      QVERIFY2(process.waitForStarted(), qUtf8Printable(process.errorString()));
      QVERIFY(process.waitForFinished(5000));
      QCOMPARE(process.exitStatus(), QProcess::NormalExit);
      QVERIFY(process.readAllStandardError().isEmpty());
      observed += process.readAllStandardOutput();
    }
    QCOMPARE(observed.replace("\r\n", "\n"), expected);

    auto script = QString::fromStdString(app::renderCommandScript(commands));
    for (const auto& command : commands) {
      const auto replacement =
          QStringLiteral("\"%1\" --command-step-test-client %2 %3").arg(client).arg(static_cast<int>(command.kind)).arg(command.kind == failed ? 7 : 0);
      script.replace(QString::fromStdString(app::renderCommand(command)), replacement);
    }
    QVERIFY(!script.contains(QStringLiteral("uwfmgr.exe")));
    const auto scriptPath = directory.filePath(QStringLiteral("dependencies.cmd"));
    QFile file(scriptPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    const auto bytes = script.toUtf8();
    QCOMPARE(file.write(bytes), bytes.size());
    file.close();
    for (const bool delayed : {false, true}) {
      QProcess process;
      process.setProgram(cmd);
      process.setNativeArguments(QStringLiteral("/d /v:%1 /s /c \"\"%2\"\"").arg(delayed ? "on" : "off", QDir::toNativeSeparators(scriptPath)));
      process.start();
      QVERIFY2(process.waitForStarted(), qUtf8Printable(process.errorString()));
      QVERIFY(process.waitForFinished(5000));
      QCOMPARE(process.exitStatus(), QProcess::NormalExit);
      QCOMPARE(process.exitCode(), failed == VolumeProtect ? 7 : 0);
      QCOMPARE(process.readAllStandardOutput().replace("\r\n", "\n"), expected);
      QVERIFY(process.readAllStandardError().isEmpty());
    }
  }
}

}  // namespace uwf

namespace {

bool equalResult(const uwf::app::ApplicationCommandResult& lhs, const uwf::app::ApplicationCommandResult& rhs) {
  return lhs.outcome == rhs.outcome && lhs.discoveredFiles == rhs.discoveredFiles && lhs.committedFiles == rhs.committedFiles &&
         lhs.skippedFiles == rhs.skippedFiles && lhs.skippedEntries == rhs.skippedEntries && lhs.failedFiles == rhs.failedFiles && lhs.detail == rhs.detail;
}

int runSingleInstanceClient(const QStringList& arguments) {
  if (arguments.size() != 4) return 10;

  uwf::app::ApplicationCommandKind command;
  if (arguments[3] == QStringLiteral("activate")) {
    command = uwf::app::ApplicationCommandKind::Activate;
  } else if (arguments[3] == QStringLiteral("ensure-running")) {
    command = uwf::app::ApplicationCommandKind::EnsureRunning;
  } else if (arguments[3] == QStringLiteral("commit-stage")) {
    command = uwf::app::ApplicationCommandKind::CommitStage;
  } else {
    return 11;
  }

  uwf::windows::SecureSingleInstance instance(uwf::windows::SecureSingleInstance::Scope{arguments[2]});
  uwf::app::ApplicationCommandResult result;
  if (instance.acquire(command, &result) != uwf::windows::SecureSingleInstance::AcquireResult::ForwardedExisting) return 12;
  if (command != uwf::app::ApplicationCommandKind::CommitStage) return equalResult(result, {}) ? 0 : 13;

  const uwf::app::ApplicationCommandResult expected{
      uwf::app::ApplicationCommandOutcome::CompletedWithFailures, 3, 1, 1, 2, 1, QStringLiteral("one staged file failed")};
  return equalResult(result, expected) ? 0 : 14;
}

}  // namespace

int main(int argc, char** argv) {
  QApplication application(argc, argv);
  const QStringList arguments = application.arguments();
  if (arguments.size() >= 2 && arguments[1] == QStringLiteral("--command-step-test-client")) {
    if (arguments.size() != 4) return 30;
    QFile output;
    if (!output.open(stdout, QIODevice::WriteOnly)) return 31;
    const auto bytes = arguments[2].toUtf8() + '\n';
    if (output.write(bytes) != bytes.size() || !output.flush()) return 32;
    return arguments[3].toInt();
  }
  if (arguments.size() >= 2 && arguments[1] == QStringLiteral("--command-argument-test-client")) {
    if (arguments.size() != 3) return 20;
    QFile output;
    if (!output.open(stdout, QIODevice::WriteOnly)) return 21;
    const QByteArray bytes = arguments[2].toUtf8();
    return output.write(bytes) == bytes.size() && output.flush() ? 0 : 22;
  }
  if (arguments.size() >= 2 && arguments[1] == QStringLiteral("--single-instance-test-client")) {
    return runSingleInstanceClient(arguments);
  }
  uwf::windows::initializeWmiRuntime();
  const auto shutdownWmi = qScopeGuard([] { uwf::windows::shutdownWmiRuntime(); });
  uwf::WindowsPlatformIntegrationTests tests;
  return QTest::qExec(&tests, argc, argv);
}

#include "WindowsPlatformIntegrationTests.moc"
