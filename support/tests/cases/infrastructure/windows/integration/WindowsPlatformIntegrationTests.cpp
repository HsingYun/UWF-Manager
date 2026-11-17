// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <windows.h>
#include <winioctl.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
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
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <filesystem>
#include <future>
#include <limits>
#include <memory>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include "application/commit/CommitPlanner.h"
#include "application/configuration/commands/UwfmgrCommandCodec.h"
#include "application/configuration/planning/ConfigurationScript.h"
#include "application/operations/contracts/OperationError.h"
#include "application/paths/PathPolicy.h"
#include "application/staging/editing/FileStagingEntries.h"
#include "application/uwf/adapters/UwfReadState.h"
#include "composition/uwf/SystemUwf.h"
#include "foundation/paths/DriveLetter.h"
#include "infrastructure/windows/commit/WindowsCommitTraversalSource.h"
#include "infrastructure/windows/commit/WindowsFileTraversalSource.h"
#include "infrastructure/windows/compatibility/SystemCheck.h"
#include "infrastructure/windows/compatibility/WindowsVersion.h"
#include "infrastructure/windows/diagnostics/SystemHardwareInfo.h"
#include "infrastructure/windows/errors/WindowsSystemError.h"
#include "infrastructure/windows/filesystem/browsing/LocalFileBrowser.h"
#include "infrastructure/windows/filesystem/paths/VolumePaths.h"
#include "infrastructure/windows/filesystem/text/AtomicTextFileWriter.h"
#include "infrastructure/windows/filesystem/text/BoundedTextFileReader.h"
#include "infrastructure/windows/filesystem/text/CreateOnlyTextFileWriter.h"
#include "infrastructure/windows/ipc/NamedPipeIo.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/management/wmi/WmiOperations.h"
#include "infrastructure/windows/management/wmi/WmiRuntime.h"
#include "infrastructure/windows/process/SecureSingleInstance.h"
#include "infrastructure/windows/registry/RegistryCommitPlan.h"
#include "infrastructure/windows/registry/RegistryKey.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/security/ExecutableIdentity.h"
#include "infrastructure/windows/service/WindowsEnhancedModeServiceControl.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace uwf {

namespace {
struct RegistryBoundaryOperation {
  const char* name;
  bool hasValueName;
  bool hasPayload;
  void (*invoke)(const std::string&, const std::string&, const std::vector<std::string>&);
};
constexpr RegistryBoundaryOperation kRegistryBoundaryOperations[] = {
    {"key-exists", false, false, [](const auto& key, const auto&, const auto&) { static_cast<void>(regkey::keyExists(key)); }},
    {"value-exists", true, false, [](const auto& key, const auto& name, const auto&) { static_cast<void>(regkey::valueExists(key, name)); }},
    {"read-string", true, false, [](const auto& key, const auto& name, const auto&) { static_cast<void>(regkey::readString(key, name)); }},
    {"read-dword", true, false, [](const auto& key, const auto& name, const auto&) { static_cast<void>(regkey::readDword(key, name)); }},
    {"read-list", true, false, [](const auto& key, const auto& name, const auto&) { static_cast<void>(regkey::readMultiString(key, name)); }},
    {"write-list", true, true, [](const auto& key, const auto& name, const auto& values) { regkey::writeMultiString(key, name, values); }},
    {"delete-tree", false, false, [](const auto& key, const auto&, const auto&) { regkey::deleteTree(key); }},
    {"subkey-names", false, false, [](const auto& key, const auto&, const auto&) { static_cast<void>(regkey::subkeyNames(key)); }},
    {"visit-subkeys", false, false, [](const auto& key, const auto&, const auto&) { regkey::visitSubkeyNames(key, [](const auto&) {}); }},
    {"has-subkeys", false, false, [](const auto& key, const auto&, const auto&) { static_cast<void>(regkey::hasSubkeys(key)); }},
    {"value-names", false, false, [](const auto& key, const auto&, const auto&) { static_cast<void>(regkey::valueNames(key)); }},
    {"visit-values", false, false, [](const auto& key, const auto&, const auto&) { regkey::visitValueNames(key, [](const auto&) {}); }},
    {"values", false, false, [](const auto& key, const auto&, const auto&) { static_cast<void>(regkey::values(key)); }},
    {"collect-tree", false, false, [](const auto& key, const auto&, const auto&) { static_cast<void>(regkey::collectKeyTree(key)); }},
};

struct NamedPipePair {
  windows::UniqueHandle server, client, stop;
  NamedPipePair() {
    const auto name = (QStringLiteral("\\\\.\\pipe\\UwfNamedPipeIoTests.") + QUuid::createUuid().toString(QUuid::WithoutBraces)).toStdWString();
    server.reset(
        CreateNamedPipeW(name.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 4096, 4096, 0, nullptr));
    if (!server.valid()) throw windows::WindowsSystemError(GetLastError(), "create test named pipe");
    client.reset(CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr));
    if (!client.valid()) throw windows::WindowsSystemError(GetLastError(), "connect test named pipe");
    stop.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    if (!stop.valid()) throw windows::WindowsSystemError(GetLastError(), "create test named pipe stop event");
    if (windows::connectNamedPipe(server.get(), {stop.get(), std::chrono::steady_clock::now() + std::chrono::seconds{1}}) !=
        windows::NamedPipeIoResult::Completed)
      throw std::runtime_error("test named pipe did not connect");
  }
};

// Directory junction with both ends under one QTemporaryDir.
// Layout: https://learn.microsoft.com/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_reparse_data_buffer
// Operation: https://learn.microsoft.com/windows/win32/api/winioctl/ni-winioctl-fsctl_set_reparse_point
DWORD createScopedDirectoryJunction(const std::wstring& junction, const std::wstring& target) {
  struct MountPointHeader {
    DWORD tag;
    WORD dataLength;
    WORD reserved;
    WORD substituteOffset;
    WORD substituteLength;
    WORD printOffset;
    WORD printLength;
  };
  static_assert(sizeof(MountPointHeader) == 16);
  if (target.starts_with(L"\\\\")) return ERROR_NOT_SUPPORTED;
  const auto substitute = std::wstring(L"\\??\\") + target;
  const auto substituteBytes = substitute.size() * sizeof(wchar_t);
  const auto printBytes = target.size() * sizeof(wchar_t);
  const auto pathBytes = substituteBytes + sizeof(wchar_t) + printBytes + sizeof(wchar_t);
  if (pathBytes + 8 > (std::numeric_limits<WORD>::max)()) return ERROR_FILENAME_EXCED_RANGE;
  const MountPointHeader header{IO_REPARSE_TAG_MOUNT_POINT,
                                static_cast<WORD>(8 + pathBytes),
                                0,
                                0,
                                static_cast<WORD>(substituteBytes),
                                static_cast<WORD>(substituteBytes + sizeof(wchar_t)),
                                static_cast<WORD>(printBytes)};
  std::vector<std::byte> buffer(sizeof(header) + pathBytes);
  std::memcpy(buffer.data(), &header, sizeof(header));
  std::memcpy(buffer.data() + sizeof(header), substitute.c_str(), substituteBytes + sizeof(wchar_t));
  std::memcpy(buffer.data() + sizeof(header) + substituteBytes + sizeof(wchar_t), target.c_str(), printBytes + sizeof(wchar_t));
  const HANDLE directory = CreateFileW(junction.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                                       FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (directory == INVALID_HANDLE_VALUE) return GetLastError();
  const auto close = qScopeGuard([&] { CloseHandle(directory); });
  DWORD returned = 0;
  return DeviceIoControl(directory, FSCTL_SET_REPARSE_POINT, buffer.data(), static_cast<DWORD>(buffer.size()), nullptr, 0, &returned, nullptr) ? ERROR_SUCCESS
                                                                                                                                               : GetLastError();
}
}  // namespace

class WindowsPlatformIntegrationTests final : public QObject {
  Q_OBJECT

 private slots:
  void windowsAndHardwareMetadataRemainInternallyConsistent();
  void executableIdentityDistinguishesAliasesFromCopies();
  void serviceExecutableCommandsUseHostObjectIdentity_data();
  void serviceExecutableCommandsUseHostObjectIdentity();
  void cimv2TransportDistinguishesPresentAndMissingClasses();
  void failedWmiReadsLeaveSubsequentReadsUsable_data();
  void failedWmiReadsLeaveSubsequentReadsUsable();
  void volumeMountQueryIncludesTheSystemDrive();
  void unicodeConversionPreservesValidDataAndRejectsMalformedInput();
  void windowsErrorsPreserveUtf8TextAndSystemCodes_data();
  void windowsErrorsPreserveUtf8TextAndSystemCodes();
  void registryReadFailuresExposeUtf8Text();
  void registryObjectPathsPreserveTheirLiteralSuffix_data();
  void registryObjectPathsPreserveTheirLiteralSuffix();
  void registryRejectsMalformedInputBeforeObservationOrMutation_data();
  void registryRejectsMalformedInputBeforeObservationOrMutation();
  void nativeCommitTraversalContracts_data();
  void nativeCommitTraversalContracts();
  void selectedDirectoryReparseLeafStaysOutsideTraversal_data();
  void selectedDirectoryReparseLeafStaysOutsideTraversal();
  void boundFileObservationUsesItsIdentityAndExecutionRechecksDisplayPath();
  void driveAndGuidAliasesShareNativeIdentityAndStagingCoverage();
  void driveResolutionHonorsLiteralPathBoundaries_data();
  void driveResolutionHonorsLiteralPathBoundaries();
  void embeddedInventoryCapabilityDoesNotInventCompleteness();
  void explicitWmiShutdownRetiresTheWorkerContext();
  void namedPipeReadIsCancelledByStopEvent();
  void namedPipeDeadlinesCancelIncompleteOperations_data();
  void namedPipeDeadlinesCancelIncompleteOperations();
  void namedPipeFrameSharesItsDeadlineWithTheResponse();
  void namedPipeIdleWaitDoesNotConsumeTheFrameDeadline();
  void singleInstanceForwardsTypedCommandsAcrossTrustedProcesses();
  void busySingleInstanceCannotBlockAnotherLaunch();
  void commandArgumentsSurviveTheWindowsShell_data();
  void commandArgumentsSurviveTheWindowsShell();
  void commandDependenciesSurviveTheWindowsShell();
  void omittedCommandsKeepTheWindowsShellSafe_data();
  void omittedCommandsKeepTheWindowsShellSafe();
  void textExportsAreAtomicAndPreserveUtf8();
  void textReadsRespectByteLimits_data();
  void textReadsRespectByteLimits();
  void fileBrowserRenamesRespectExistingTargets_data();
  void fileBrowserRenamesRespectExistingTargets();
  void fileObjectNamesPreserveTheirLiteralSuffix_data();
  void fileObjectNamesPreserveTheirLiteralSuffix();
};

void WindowsPlatformIntegrationTests::serviceExecutableCommandsUseHostObjectIdentity_data() {
  QTest::addColumn<QString>("configuredPath");
  QTest::addColumn<QString>("executablePath");
  QTest::addColumn<QString>("arguments");
  QTest::addColumn<bool>("matches");
  struct Identity {
    const char* label;
    const char16_t* configured;
    const char16_t* expected;
    bool same;
  };
  constexpr std::array identities{Identity{"ascii-alias", u"C:/Apps/UWF.exe", u"c:/apps/uwf.EXE", true},
                                  Identity{"unicode-alias", u"C:/Ä/UWF.exe", u"C:/ä/UWF.exe", true},
                                  Identity{"unicode-micro-distinct", u"C:/µ/UWF.exe", u"C:/μ/UWF.exe", false},
                                  Identity{"unicode-sharp-s-distinct", u"C:/ß/UWF.exe", u"C:/ẞ/UWF.exe", false},
                                  Identity{"normalized-path", u"C:/Apps/Temp/../UWF.exe", u"C:/Apps/UWF.exe", true},
                                  Identity{"different-file", u"C:/Apps/Old.exe", u"C:/Apps/UWF.exe", false}};
  const QStringList arguments{QStringLiteral("--service"), QStringLiteral("--SERVICE"), QStringLiteral("--service extra"), QString{}};
  for (const auto& identity : identities)
    for (qsizetype index = 0; index < arguments.size(); ++index)
      QTest::newRow(qUtf8Printable(QStringLiteral("%1-arguments-%2").arg(QString::fromLatin1(identity.label)).arg(index)))
          << QString::fromUtf16(identity.configured) << QString::fromUtf16(identity.expected) << arguments[index] << (identity.same && index == 0);
}

void WindowsPlatformIntegrationTests::serviceExecutableCommandsUseHostObjectIdentity() {
  QFETCH(QString, configuredPath);
  QFETCH(QString, executablePath);
  QFETCH(QString, arguments);
  QFETCH(bool, matches);
  const auto command = QStringLiteral("\"%1\" %2").arg(configuredPath, arguments).toStdWString();
  QCOMPARE(windows::serviceCommandMatchesExecutable(command.c_str(), executablePath.toStdWString()), matches);
  QVERIFY(!windows::serviceCommandMatchesExecutable(nullptr, executablePath.toStdWString()));
}

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

void WindowsPlatformIntegrationTests::fileObjectNamesPreserveTheirLiteralSuffix_data() {
  QTest::addColumn<QString>("parentName");
  QTest::newRow("ordinary-parent") << QString{};
  QTest::newRow("literal-parent-plain") << QStringLiteral("Folder");
  QTest::newRow("literal-parent-space") << QStringLiteral("Folder ");
  QTest::newRow("literal-parent-dot") << QStringLiteral("Folder.");
}

void WindowsPlatformIntegrationTests::fileObjectNamesPreserveTheirLiteralSuffix() {
  QFETCH(QString, parentName);
  QTemporaryDir root;
  QVERIFY(root.isValid());
  struct Seed {
    const char16_t* name;
    DWORD bytes;
  };
  struct OwnedFile {
    QString path;
    std::wstring exact;
    QByteArray payload;
  };
  constexpr std::array seeds{Seed{u"Entry", 3}, Seed{u"Entry ", 7}, Seed{u"Entry.", 11}};
  const auto baseName = QStringLiteral("Entry");
  const auto exactPath = [](const QString& path) {
    const auto native = QDir::toNativeSeparators(path);
    return (native.startsWith(QStringLiteral("\\\\")) ? QStringLiteral("\\\\?\\UNC\\") + native.mid(2) : QStringLiteral("\\\\?\\") + native).toStdWString();
  };
  std::vector<std::wstring> exactFiles;
  std::vector<std::wstring> exactDirectories;
  std::vector<OwnedFile> files;
  const auto cleanup = qScopeGuard([&] {
    for (const auto& path : exactFiles) DeleteFileW(path.c_str());
    for (auto iterator = exactDirectories.rbegin(); iterator != exactDirectories.rend(); ++iterator) RemoveDirectoryW(iterator->c_str());
  });
  const QStringList parents =
      parentName.isEmpty() ? QStringList{QString{}} : QStringList{QStringLiteral("Folder"), QStringLiteral("Folder "), QStringLiteral("Folder.")};
  std::array<std::size_t, 3> selected{};
  // Seed and read back through Win32 so Qt path coercion cannot normalize both the fixture and the result.
  // Distinct parent and child payloads reveal coercion of either component.
  for (qsizetype parentIndex = 0; parentIndex < parents.size(); ++parentIndex) {
    const auto directory = parents[parentIndex].isEmpty() ? root.path() : root.filePath(parents[parentIndex]);
    if (!parents[parentIndex].isEmpty()) {
      const auto exact = exactPath(directory);
      QVERIFY2(CreateDirectoryW(exact.c_str(), nullptr), qPrintable(QStringLiteral("CreateDirectoryW failed: %1").arg(GetLastError())));
      exactDirectories.push_back(exact);
    }
    for (std::size_t index = 0; index < seeds.size(); ++index) {
      const auto path = QDir(directory).filePath(QString::fromUtf16(seeds[index].name));
      const auto exact = exactPath(path);
      const HANDLE file = CreateFileW(exact.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, CREATE_NEW,
                                      FILE_ATTRIBUTE_NORMAL, nullptr);
      QVERIFY2(file != INVALID_HANDLE_VALUE, qPrintable(QStringLiteral("CreateFileW failed: %1").arg(GetLastError())));
      exactFiles.push_back(exact);
      const auto close = qScopeGuard([file] { CloseHandle(file); });
      const QByteArray payload(static_cast<qsizetype>(seeds[index].bytes) + parentIndex * 13, static_cast<char>('A' + parentIndex));
      DWORD written = 0;
      QVERIFY(WriteFile(file, payload.constData(), static_cast<DWORD>(payload.size()), &written, nullptr));
      QCOMPARE(written, static_cast<DWORD>(payload.size()));
      if (parents[parentIndex] == parentName) selected[index] = files.size();
      files.push_back({path, exact, payload});
    }
  }
  const auto verifyAll = [&]() -> QString {
    for (const auto& expected : files) {
      const HANDLE file = CreateFileW(expected.exact.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL, nullptr);
      if (file == INVALID_HANDLE_VALUE) return QStringLiteral("Opening exact [%1] failed: %2").arg(expected.path).arg(GetLastError());
      const auto close = qScopeGuard([file] { CloseHandle(file); });
      std::array<char, 256> bytes{};
      DWORD read = 0;
      if (!ReadFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr))
        return QStringLiteral("Reading exact [%1] failed: %2").arg(expected.path).arg(GetLastError());
      if (QByteArray(bytes.data(), static_cast<qsizetype>(read)) != expected.payload)
        return QStringLiteral("Wrong object or changed sibling: [%1]").arg(expected.path);
    }
    return {};
  };
  const auto directory = parentName.isEmpty() ? root.path() : root.filePath(parentName);
  const auto browser = windows::systemFileBrowser();
  const auto entries = browser->list(directory, {});
  QCOMPARE(entries.size(), qsizetype{3});
  const auto traversal = windows::systemCommitTraversalSource();
  QList<app::CommitNode> children;
  QVERIFY(traversal->enumerateChildren({app::CommitTargetKind::Folder, directory, {}, {}}, {}, [&](const app::CommitNode& child) { children.append(child); })
              .isEmpty());
  QCOMPARE(children.size(), qsizetype{3});
  for (std::size_t index = 0; index < seeds.size(); ++index) {
    const auto& expected = files[selected[index]];
    const auto name = QString::fromUtf16(seeds[index].name);
    const auto listed = std::ranges::find_if(entries, [&](const auto& value) { return value.name == name; });
    QVERIFY2(listed != entries.end(), qPrintable(QStringLiteral("Missing literal browser name: [%1]").arg(name)));
    QCOMPARE(QDir::fromNativeSeparators(listed->path), QDir::fromNativeSeparators(expected.path));
    QCOMPARE(listed->size, static_cast<qint64>(expected.payload.size()));
    const auto inspected = browser->inspect(expected.path);
    QVERIFY(inspected.exists);
    QVERIFY(!inspected.directory);
    QCOMPARE(inspected.name, name);
    QCOMPARE(inspected.size, static_cast<qint64>(expected.payload.size()));
    QCOMPARE(QDir::fromNativeSeparators(inspected.path), QDir::fromNativeSeparators(expected.path));
    const auto child =
        std::ranges::find_if(children, [&](const auto& value) { return QDir::fromNativeSeparators(value.path) == QDir::fromNativeSeparators(expected.path); });
    QVERIFY2(child != children.end(), qPrintable(QStringLiteral("Missing literal traversal path: [%1]").arg(name)));
    const auto node = traversal->inspect({app::CommitTargetKind::File, expected.path, {}, {}});
    QVERIFY(node);
    QCOMPARE(node->kind, app::CommitTargetKind::File);
    QCOMPARE(node->path, expected.path);
    QVERIFY(node->skippedReason.isEmpty());
    QVERIFY(traversal->exists(*node));
    const auto read = windows::readBoundedTextFile(expected.path, 4096);
    QCOMPARE(read.failure, app::TextFileReadResult::Failure::None);
    QCOMPARE(read.bytes, expected.payload);
    QCOMPARE(windows::readBoundedTextFile(expected.path, 1).failure, app::TextFileReadResult::Failure::TooLarge);
    for (const auto format : {windows::FileIdentityFormat::FileIndex64, windows::FileIdentityFormat::FileId128}) {
      QVERIFY(windows::sameFile(expected.path.toStdWString(), expected.path.toStdWString(), format));
      for (std::size_t sibling = 0; sibling < seeds.size(); ++sibling)
        if (sibling != index) QVERIFY(!windows::sameFile(expected.path.toStdWString(), files[selected[sibling]].path.toStdWString(), format));
    }
  }
  QString oracleError;
  for (std::size_t index = 0; index < seeds.size(); ++index) {
    auto& expected = files[selected[index]];
    const auto replacement = QStringLiteral("replacement-%1").arg(index);
    QCOMPARE(windows::writeUtf8TextFile(expected.path, replacement).failure, app::TextFileWriteResult::Failure::None);
    expected.payload = replacement.toUtf8();
    oracleError = verifyAll();
    QVERIFY2(oracleError.isEmpty(), qPrintable(oracleError));
    QCOMPARE(windows::createUtf8TextFile(expected.path, QStringLiteral("must not replace")).outcome, app::TextFileCreationResult::Outcome::AlreadyExists);
    oracleError = verifyAll();
    QVERIFY2(oracleError.isEmpty(), qPrintable(oracleError));
  }
  const auto originalCount = files.size();
  for (std::size_t index = 0; index < seeds.size(); ++index) {
    const auto suffix = QString::fromUtf16(seeds[index].name).sliced(baseName.size());
    const auto created = QDir(directory).filePath(QStringLiteral("Created") + suffix);
    const auto payload = QStringLiteral("新建-%1").arg(index);
    exactFiles.push_back(exactPath(created));
    QCOMPARE(windows::createUtf8TextFile(created, payload).outcome, app::TextFileCreationResult::Outcome::Created);
    files.push_back({created, exactFiles.back(), payload.toUtf8()});
    oracleError = verifyAll();
    QVERIFY2(oracleError.isEmpty(), qPrintable(oracleError));
    const auto source = files[selected[index]].path;
    try {
      browser->renameEntry(source, created);
      QFAIL("Renaming must not replace the literal target");
    } catch (const app::FileBrowserError& error) {
      QCOMPARE(error.code(), app::FileBrowserErrorCode::AlreadyExists);
    }
    oracleError = verifyAll();
    QVERIFY2(oracleError.isEmpty(), qPrintable(oracleError));
    const auto renamed = QDir(directory).filePath(QStringLiteral("Renamed") + suffix);
    const auto oldExact = files[selected[index]].exact;
    exactFiles.push_back(exactPath(renamed));
    browser->renameEntry(source, renamed);
    files[selected[index]].path = renamed;
    files[selected[index]].exact = exactFiles.back();
    QCOMPARE(GetFileAttributesW(oldExact.c_str()), INVALID_FILE_ATTRIBUTES);
    QVERIFY(!browser->inspect(source).exists);
    QCOMPARE(browser->inspect(renamed).name, QStringLiteral("Renamed") + suffix);
    oracleError = verifyAll();
    QVERIFY2(oracleError.isEmpty(), qPrintable(oracleError));
  }
  QCOMPARE(files.size(), originalCount + seeds.size());
  for (const auto& seed : seeds) {
    const auto suffix = QString::fromUtf16(seed.name).sliced(baseName.size());
    const auto created = QDir(directory).filePath(QStringLiteral("NewFolder") + suffix);
    exactDirectories.push_back(exactPath(created));
    browser->createDirectory(created);
    const auto attributes = GetFileAttributesW(exactDirectories.back().c_str());
    QVERIFY(attributes != INVALID_FILE_ATTRIBUTES);
    QVERIFY((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    QVERIFY(browser->inspect(created).directory);
    QVERIFY(browser->list(created, {}).isEmpty());
    try {
      browser->createDirectory(created);
      QFAIL("Creating an existing literal directory must be rejected");
    } catch (const app::FileBrowserError& error) {
      QCOMPARE(error.code(), app::FileBrowserErrorCode::AlreadyExists);
    }
    const auto renamed = QDir(directory).filePath(QStringLiteral("MovedFolder") + suffix);
    const auto oldExact = exactDirectories.back();
    exactDirectories.push_back(exactPath(renamed));
    browser->renameEntry(created, renamed);
    QCOMPARE(GetFileAttributesW(oldExact.c_str()), INVALID_FILE_ATTRIBUTES);
    QVERIFY(browser->inspect(renamed).directory);
    QVERIFY(browser->list(renamed, {}).isEmpty());
  }
  QCOMPARE(browser->list(directory, {}).size(), qsizetype{9});
  oracleError = verifyAll();
  QVERIFY2(oracleError.isEmpty(), qPrintable(oracleError));
}

void WindowsPlatformIntegrationTests::registryObjectPathsPreserveTheirLiteralSuffix_data() {
  QTest::addColumn<QString>("leaf");
  QTest::addColumn<QString>("sibling");
  QTest::addColumn<bool>("alias");
  for (const bool alias : {false, true}) {
    QTest::newRow(alias ? "space/alias" : "space/long-hive") << QStringLiteral("Vendor ") << QStringLiteral("Vendor") << alias;
    QTest::newRow(alias ? "dot/alias" : "dot/long-hive") << QStringLiteral("Vendor.") << QStringLiteral("Vendor") << alias;
    QTest::newRow(alias ? "dot-segment/alias" : "dot-segment/long-hive") << QStringLiteral("..") << QStringLiteral(".") << alias;
    QTest::newRow(alias ? "ordinary/alias" : "ordinary/long-hive") << QStringLiteral("Vendor") << QStringLiteral("Sibling") << alias;
    QTest::newRow(alias ? "slash/alias" : "slash/long-hive") << QStringLiteral("Slash/Name") << QStringLiteral("SlashName") << alias;
    QTest::newRow(alias ? "control/alias" : "control/long-hive")
        << (QStringLiteral("Control") + QChar(1) + QStringLiteral("Name")) << QStringLiteral("ControlName") << alias;
  }
}
void WindowsPlatformIntegrationTests::registryObjectPathsPreserveTheirLiteralSuffix() {
  QFETCH(QString, leaf);
  QFETCH(QString, sibling);
  QFETCH(bool, alias);
  const auto owned = QStringLiteral("Software\\UwfTests\\%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  const auto root = QStringLiteral("HKEY_CURRENT_USER\\") + owned;
  const auto cleanup = qScopeGuard([&] { regkey::deleteTree(root.toStdString()); });
  // Seed through Win32 to keep the fixture independent of production path normalization.
  const auto seed = [&](const QString& relative, const std::wstring& marker) {
    HKEY handle = nullptr;
    const auto subkey = (owned + u'\\' + relative).toStdWString();
    const auto created = RegCreateKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, nullptr, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr, &handle, nullptr);
    if (created != ERROR_SUCCESS) return created;
    const auto close = qScopeGuard([handle] { RegCloseKey(handle); });
    return RegSetValueExW(handle, L"Marker ", 0, REG_SZ, reinterpret_cast<const BYTE*>(marker.c_str()),
                          static_cast<DWORD>((marker.size() + 1) * sizeof(wchar_t)));
  };
  QCOMPARE(seed(leaf, L"selected"), ERROR_SUCCESS);
  QCOMPARE(seed(leaf + QStringLiteral("\\Child"), L"child"), ERROR_SUCCESS);
  QCOMPARE(seed(sibling, L"sibling"), ERROR_SUCCESS);
  const auto selected = root + u'\\' + leaf;
  const auto counterpart = root + u'\\' + sibling;
  const auto queried = (alias ? QStringLiteral("hKcU\\") + owned : root) + u'\\' + leaf;
  QVERIFY(regkey::keyExists(queried.toStdString()));
  QVERIFY(regkey::valueExists(queried.toStdString(), "Marker "));
  QVERIFY(!regkey::valueExists(queried.toStdString(), "Marker"));
  QCOMPARE(regkey::readString(queried.toStdString(), "Marker "), std::optional<std::string>{"selected"});
  QCOMPARE(regkey::readString(counterpart.toStdString(), "Marker "), std::optional<std::string>{"sibling"});
  const auto names = regkey::subkeyNames(root.toStdString());
  QVERIFY(std::ranges::find(names, leaf.toStdString()) != names.end());
  QVERIFY(std::ranges::find(names, sibling.toStdString()) != names.end());
  QCOMPARE(regkey::subkeyNames(queried.toStdString()), std::vector<std::string>{"Child"});
  QVERIFY(regkey::hasSubkeys(queried.toStdString()));
  QCOMPARE(regkey::valueNames(queried.toStdString()), std::vector<std::string>{"Marker "});
  const auto values = regkey::values(queried.toStdString());
  QCOMPARE(values.size(), std::size_t{1});
  QCOMPARE(values.front().name, std::string("Marker "));
  regkey::writeMultiString(queried.toStdString(), "Updated", {"exact-object"});
  QCOMPARE(regkey::readMultiString(queried.toStdString(), "Updated"), std::optional<std::vector<std::string>>{{"exact-object"}});
  QVERIFY(!regkey::valueExists(counterpart.toStdString(), "Updated"));
  const auto child = selected + QStringLiteral("\\Child");
  QCOMPARE(regkey::collectKeyTree(queried.toStdString()), (std::vector<std::string>{child.toStdString(), selected.toStdString()}));
  const auto deletion = windows::planRegistryTreeDeletion(queried.toStdString());
  QCOMPARE(deletion.size(), std::size_t{2});
  QCOMPARE(deletion.front().key, child.toStdString());
  QCOMPARE(deletion.back().key, selected.toStdString());
  const auto commits = windows::planRegistryTreeCommit(queried.toStdString());
  QCOMPARE(commits.size(), std::size_t{3});
  for (const auto& target : commits) QVERIFY(target.key == selected.toStdString() || target.key == child.toStdString());
  const auto source = windows::systemCommitTraversalSource();
  const app::CommitRequest request{app::CommitOperation::Persist, {app::CommitTargetKind::RegistryKey, queried, {}, {}}};
  const auto plan = app::prepareCommit(request, *source, {}, {}, {});
  QCOMPARE(plan.progress().failed, 0);
  QCOMPARE(plan.progress().items, 3);
  QCOMPARE(plan.progress().containers, 2);
  regkey::deleteTree(queried.toStdString());
  QVERIFY(!regkey::keyExists(selected.toStdString()));
  QVERIFY(regkey::keyExists(counterpart.toStdString()));
  QCOMPARE(regkey::readString(counterpart.toStdString(), "Marker "), std::optional<std::string>{"sibling"});
}

void WindowsPlatformIntegrationTests::registryRejectsMalformedInputBeforeObservationOrMutation_data() {
  QTest::addColumn<qsizetype>("operationIndex");
  QTest::addColumn<QString>("field");
  QTest::addColumn<QString>("corruption");
  QTest::addColumn<bool>("existing");
  for (qsizetype index = 0; index < static_cast<qsizetype>(std::size(kRegistryBoundaryOperations)); ++index) {
    const auto& operation = kRegistryBoundaryOperations[index];
    for (const auto* field : {"key", "value-name", "payload"}) {
      if ((std::string_view(field) == "value-name" && !operation.hasValueName) || (std::string_view(field) == "payload" && !operation.hasPayload)) continue;
      for (const auto* corruption : {"nul", "utf8", "empty"}) {
        if (std::string_view(corruption) == "empty" && std::string_view(field) != "payload") continue;
        for (const bool existing : {false, true}) {
          const auto name = QStringLiteral("%1/%2/%3/%4")
                                .arg(QString::fromLatin1(operation.name), QString::fromLatin1(field), QString::fromLatin1(corruption),
                                     existing ? QStringLiteral("existing") : QStringLiteral("missing"));
          QTest::newRow(qUtf8Printable(name)) << index << QString::fromLatin1(field) << QString::fromLatin1(corruption) << existing;
        }
      }
    }
  }
}

void WindowsPlatformIntegrationTests::registryRejectsMalformedInputBeforeObservationOrMutation() {
  QFETCH(qsizetype, operationIndex);
  QFETCH(QString, field);
  QFETCH(QString, corruption);
  QFETCH(bool, existing);
  const auto& operation = kRegistryBoundaryOperations[operationIndex];
  const auto owned = QStringLiteral("Software\\UwfTests\\%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  const auto root = QStringLiteral("HKEY_CURRENT_USER\\") + owned;
  const auto target = root + QStringLiteral("\\Target");
  const auto sibling = root + QStringLiteral("\\Sibling");
  const auto cleanup = qScopeGuard([&] { regkey::deleteTree(root.toStdString()); });
  regkey::writeMultiString(sibling.toStdString(), "List", {"untouched"});
  if (existing) {
    // Seed through Win32 so production name normalization cannot also alter the expected fixture.
    HKEY handle = nullptr;
    const auto subkey = (owned + QStringLiteral("\\Target")).toStdWString();
    QCOMPARE(RegCreateKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, nullptr, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr, &handle, nullptr), ERROR_SUCCESS);
    const auto close = qScopeGuard([handle] { RegCloseKey(handle); });
    constexpr wchar_t text[] = L"original";
    constexpr wchar_t list[] = L"original\0";
    constexpr DWORD number = 7;
    QCOMPARE(RegSetValueExW(handle, L"String", 0, REG_SZ, reinterpret_cast<const BYTE*>(text), sizeof(text)), ERROR_SUCCESS);
    QCOMPARE(RegSetValueExW(handle, L"Dword", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&number), sizeof(number)), ERROR_SUCCESS);
    QCOMPARE(RegSetValueExW(handle, L"List", 0, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(list), sizeof(list)), ERROR_SUCCESS);
  }
  std::string key = target.toStdString();
  std::string value = std::string_view(operation.name) == "read-string" ? "String" : std::string_view(operation.name) == "read-dword" ? "Dword" : "List";
  std::vector<std::string> payload{"replacement"};
  const auto corrupt = [&](std::string& text) {
    if (corruption == QStringLiteral("nul"))
      text += std::string("\0suffix", 7);
    else if (corruption == QStringLiteral("utf8"))
      text += std::string("\xC3\x28", 2);
    else
      text.clear();
  };
  if (field == QStringLiteral("key"))
    corrupt(key);
  else if (field == QStringLiteral("value-name"))
    corrupt(value);
  else
    corrupt(payload.front());
  QVERIFY_THROWS_EXCEPTION(std::exception, operation.invoke(key, value, payload));
  QCOMPARE(regkey::keyExists(target.toStdString()), existing);
  QCOMPARE(regkey::readMultiString(sibling.toStdString(), "List"), std::optional<std::vector<std::string>>{{"untouched"}});
  if (existing) {
    QCOMPARE(regkey::readString(target.toStdString(), "String"), std::optional<std::string>{"original"});
    QCOMPARE(regkey::readDword(target.toStdString(), "Dword"), std::optional<std::uint32_t>{7});
    QCOMPARE(regkey::readMultiString(target.toStdString(), "List"), std::optional<std::vector<std::string>>{{"original"}});
    QCOMPARE(regkey::valueNames(target.toStdString()).size(), std::size_t{3});
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
  const auto expectedRoot = app::bindFileCommitTarget(*source, request.target);
  const auto plan = app::prepareCommit(request, *source, {}, {}, {});
  QCOMPARE(plan.request.target.fileTarget, expectedRoot.fileTarget);
  QCOMPARE(plan.progress().items, 3);
  QCOMPARE(plan.progress().containers, 2);
  QCOMPARE(plan.progress().failed, 0);
  const auto targets = plan.targets();
  QCOMPARE(targets.size(), deleting ? registry ? 2 : 5 : 3);
  if (deleting) QCOMPARE(targets.constLast().identity(), expectedRoot.identity());
  std::stop_source stop;
  const auto canceled = app::prepareCommit(request, *source, {}, stop.get_token(), [&](const auto& progress) {
    if (progress.items) stop.request_stop();
  });
  QVERIFY(canceled.canceled);
  QVERIFY(canceled.targets().isEmpty());
  QCOMPARE(canceled.progress().items, 1);
}

void WindowsPlatformIntegrationTests::boundFileObservationUsesItsIdentityAndExecutionRechecksDisplayPath() {
  QTemporaryDir temporary;
  QVERIFY(temporary.isValid());
  const auto actual = QDir::toNativeSeparators(temporary.filePath(QStringLiteral("actual.bin")));
  QFile file(actual);
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write("exact target"), qint64{12});
  file.close();
  const auto display = QDir::toNativeSeparators(temporary.filePath(QStringLiteral("display")));
  QVERIFY(QDir().mkpath(display));
  windows::WindowsFileTraversalSource source;
  const auto identity = source.resolveFileTarget(actual);
  const app::CommitNode bound{app::CommitTargetKind::File, display, {}, {}, identity};
  const auto observed = source.inspect(bound);
  QVERIFY(observed);
  QCOMPARE(observed->kind, app::CommitTargetKind::File);
  QCOMPARE(observed->fileTarget, std::optional{identity});
  QVERIFY(observed->skippedReason.isEmpty());
  // Scan-owned identities are retained. The execution boundary must still
  // reject a display path that no longer names that same exact target.
  QVERIFY_THROWS_EXCEPTION(app::OperationError, (void)app::bindFileCommitTarget(source, *observed));
}

void WindowsPlatformIntegrationTests::driveAndGuidAliasesShareNativeIdentityAndStagingCoverage() {
  QTemporaryDir temporary;
  QVERIFY(temporary.isValid());
  const auto driveDirectory = QDir::toNativeSeparators(temporary.filePath(QStringLiteral("Data")));
  QVERIFY(QDir().mkpath(driveDirectory));
  const auto driveFile = QDir::toNativeSeparators(QDir(driveDirectory).filePath(QStringLiteral("identity.bin")));
  QFile file(driveFile);
  QVERIFY(file.open(QIODevice::WriteOnly));
  QCOMPARE(file.write("temporary identity"), qint64{18});
  file.close();

  // Only inspect objects owned by this temporary fixture. No UWF repository,
  // commit dispatch, registry update or volume-setting operation is involved.
  const auto source = windows::systemCommitTraversalSource();
  const auto fileTarget = source->resolveFileTarget(driveFile);
  const auto directoryTarget = source->resolveFileTarget(driveDirectory);
  // Display text need not resolve again after the physical volume was observed.
  QCOMPARE(source->driveLetterForPath(QStringLiteral("unresolved display path"), fileTarget), source->driveLetterForPath(driveFile));
  const auto guidFile = app::paths::volumeFilePath({fileTarget.volumeId, fileTarget.relativePath});
  const auto guidDirectory = app::paths::volumeFilePath({directoryTarget.volumeId, directoryTarget.relativePath});
  QCOMPARE(source->resolveFileTarget(guidFile), fileTarget);
  QCOMPARE(source->resolveFileTarget(guidDirectory), directoryTarget);
  const auto direct = app::bindFileCommitTarget(*source, {app::CommitTargetKind::File, driveFile, {}, {}});
  const auto alias = app::bindFileCommitTarget(*source, {app::CommitTargetKind::File, guidFile, {}, {}});
  QCOMPARE(direct.identity(), alias.identity());
  QVERIFY(source->exists(alias));
  const auto observed = source->inspect(alias);
  QVERIFY(observed);
  QCOMPARE(observed->kind, app::CommitTargetKind::File);
  QCOMPARE(observed->fileTarget, std::optional{fileTarget});

  using Kind = app::FileStagingKind;
  const QList<app::FileStagingEntry> first{{Kind::File, driveFile}, {Kind::File, guidFile}};
  const QList<app::FileStagingEntry> second{{Kind::File, guidFile}, {Kind::File, driveFile}};
  const auto normalized = app::normalizeFileStagingEntries(first, source.get());
  const auto reversed = app::normalizeFileStagingEntries(second, source.get());
  QCOMPARE(normalized.size(), qsizetype{1});
  QCOMPARE(normalized, reversed);
  QCOMPARE(normalized.front().fileTarget, std::optional{fileTarget});
  for (const auto& selections : {QList<app::FileStagingEntry>{{Kind::Directory, guidDirectory}, {Kind::File, driveFile}},
                                 QList<app::FileStagingEntry>{{Kind::File, guidFile}, {Kind::Directory, driveDirectory}}}) {
    const auto covered = app::normalizeFileStagingEntries(selections, source.get());
    QCOMPARE(covered.size(), qsizetype{1});
    QCOMPARE(covered.front().kind, Kind::Directory);
    QCOMPARE(covered.front().fileTarget, std::optional{directoryTarget});
  }
  const auto plan = app::prepareCommit({app::CommitOperation::Persist, {app::CommitTargetKind::Folder, guidDirectory, {}, {}}}, *source, {}, {}, {});
  QCOMPARE(plan.progress().failed, 0);
  QCOMPARE(plan.targets().size(), qsizetype{1});
  QCOMPARE(plan.targets().constFirst().identity(), direct.identity());
  for (const auto& record : plan.records) {
    QVERIFY(record.node.path.startsWith(QStringLiteral(R"(\\?\)")));
    QVERIFY(record.node.fileTarget);
    const auto rebound = app::bindFileCommitTarget(*source, record.node);
    QCOMPARE(rebound.fileTarget, record.node.fileTarget);
  }
}

void WindowsPlatformIntegrationTests::selectedDirectoryReparseLeafStaysOutsideTraversal_data() {
  QTest::addColumn<bool>("trailingSlash");
  QTest::addColumn<bool>("boundBeforeReparse");
  QTest::addColumn<bool>("deleting");
  for (const bool trailing : {false, true})
    for (const bool bound : {false, true})
      for (const bool deleting : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("trailing-%1/bound-%2/deleting-%3").arg(trailing).arg(bound).arg(deleting)))
            << trailing << bound << deleting;
}

void WindowsPlatformIntegrationTests::selectedDirectoryReparseLeafStaysOutsideTraversal() {
  QFETCH(bool, trailingSlash);
  QFETCH(bool, boundBeforeReparse);
  QFETCH(bool, deleting);
  QTemporaryDir temporary;
  QVERIFY(temporary.isValid());
  const auto selected = QDir::toNativeSeparators(temporary.filePath(QStringLiteral("selected")));
  const auto target = QDir::toNativeSeparators(temporary.filePath(QStringLiteral("target")));
  QVERIFY(QDir().mkpath(selected));
  QVERIFY(QDir().mkpath(target + QStringLiteral("\\Nested")));
  const auto payload = target + QStringLiteral("\\outside-selection.txt");
  for (const auto& path : {payload, target + QStringLiteral("\\Nested\\also-outside.txt")}) {
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("keep outside selected scope"), qint64{27});
  }
  windows::WindowsFileTraversalSource source;
  app::CommitNode leaf{app::CommitTargetKind::Folder, selected + (trailingSlash ? QStringLiteral("\\") : QString{}), {}, {}};
  if (boundBeforeReparse) leaf.fileTarget = source.resolveFileTarget(leaf.path);
  const auto nativeSelected = selected.toStdWString();
  const auto removeJunction = qScopeGuard([&] { RemoveDirectoryW(nativeSelected.c_str()); });
  const auto created = createScopedDirectoryJunction(nativeSelected, target.toStdWString());
  if (created == ERROR_INVALID_FUNCTION || created == ERROR_NOT_SUPPORTED || created == ERROR_PRIVILEGE_NOT_HELD || created == ERROR_ACCESS_DENIED)
    QSKIP(qUtf8Printable(
        QStringLiteral("Temporary junction fixture unavailable without elevation (Windows error %1); reparse scope was not tested.").arg(created)));
  QVERIFY2(created == ERROR_SUCCESS, qUtf8Printable(QStringLiteral("Creating temporary junction failed: Windows error %1").arg(created)));
  const auto attributes = GetFileAttributesW(nativeSelected.c_str());
  QVERIFY(attributes != INVALID_FILE_ATTRIBUTES);
  QVERIFY((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
  QVERIFY((attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0);

  const auto inspected = source.inspect(leaf);
  QVERIFY(inspected);
  QVERIFY(!inspected->skippedReason.isEmpty());
  int children = 0;
  const auto readFailure = source.enumerateChildren(leaf, {}, [&](const app::CommitNode&) { ++children; });
  QCOMPARE(readFailure, inspected->skippedReason);
  QCOMPARE(children, 0);
  const auto plan = app::prepareCommit({deleting ? app::CommitOperation::DeleteAndPersist : app::CommitOperation::Persist, leaf}, source, {}, {}, {});
  QCOMPARE(plan.records.size(), qsizetype{1});
  QCOMPARE(plan.records.front().status, app::TraversalStatus::Skipped);
  QCOMPARE(plan.progress().failed, qsizetype{0});
  QVERIFY(plan.targets().isEmpty());
  QVERIFY(QFileInfo::exists(payload));
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
    QCOMPARE(session.classStatus("Win32_OperatingSystem"), uwf::windows::wmi::WmiClassStatus::Present);
    QCOMPARE(session.classStatus("UwfManager_Class_That_Does_Not_Exist"), uwf::windows::wmi::WmiClassStatus::Missing);
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
      static_cast<void>(session.callMethodReadOnce("Win32_OperatingSystem", "UwfManager_Method_That_Does_Not_Exist", {}, stop.get_token()));
    }
  };
  QVERIFY_THROWS_EXCEPTION(windows::wmi::WmiException, failRead());
  QCOMPARE(session.classStatus("Win32_OperatingSystem"), windows::wmi::WmiClassStatus::Present);
  QVERIFY(!session.getObject("Win32_OperatingSystem").empty());
  const auto rows = session.query("SELECT Caption FROM Win32_OperatingSystem");
  QCOMPARE(rows.size(), std::size_t{1});
  QVERIFY(!rows.front().at("Caption").toString().empty());
}

void WindowsPlatformIntegrationTests::volumeMountQueryIncludesTheSystemDrive() {
  try {
    const auto systemDrive = drive::systemLetter();
    QVERIFY(!systemDrive.empty());
    const auto root = uwf::windows::utf8ToWide(systemDrive + '\\');
    std::array<wchar_t, 50> volumeName{};
    QVERIFY(GetVolumeNameForVolumeMountPointW(root.c_str(), volumeName.data(), static_cast<DWORD>(volumeName.size())));
    const auto volume = uwf::windows::wideToUtf8(volumeName.data());
    const auto paths = drive::mountedPaths(volume);
    QVERIFY(std::ranges::find(paths, systemDrive + '\\') != paths.end());
    const auto letters = drive::mountedLetters(volume);
    QVERIFY(std::ranges::find(letters, systemDrive) != letters.end());
    for (const auto& letter : letters) {
      QCOMPARE(letter, drive::normalize(letter));
      QCOMPARE(letter.size(), std::size_t{2});
      QVERIFY(std::ranges::find(paths, letter + '\\') != paths.end());
    }
  } catch (const std::exception& error) {
    QFAIL(error.what());
  }
}

void WindowsPlatformIntegrationTests::embeddedInventoryCapabilityDoesNotInventCompleteness() {
  try {
    auto& session = uwf::windows::embeddedWmiSession();
    const auto system = uwf::composition::wmiUwf(session);
    // Unsupported adapter capabilities must reject before querying the installed UWF feature.
    for (const auto scope : {compat::Session::Current, compat::Session::Next}) {
      const compat::Request request = compat::GetVolumes{scope};
      QCOMPARE(system->probe(request).support, compat::Support::Unsupported);
      const auto result = system->execute(request);
      QVERIFY(result.failure);
      QCOMPARE(result.failure->kind, compat::ErrorKind::Unsupported);
      QCOMPARE(result.effect, compat::Effect::None);
    }
    const auto snapshot = app::publishedUwfSnapshot(system->state(), app::UwfCapability::Unavailable, windows::isElevated(), {});
    QVERIFY(!snapshot.uwfAvailable);
    QVERIFY(!snapshot.unavailableReason.empty());
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

void WindowsPlatformIntegrationTests::namedPipeReadIsCancelledByStopEvent() {
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
  QCOMPARE(windows::connectNamedPipe(server, {stopEvent}), windows::NamedPipeIoResult::Completed);

  std::promise<windows::NamedPipeIoResult> resultPromise;
  auto result = resultPromise.get_future();
  std::jthread reader([server, stopEvent, promise = std::move(resultPromise)]() mutable {
    try {
      std::array<std::byte, 1> buffer{};
      promise.set_value(windows::readNamedPipe(server, buffer, {stopEvent}));
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
  });

  QTest::qWait(10);
  QVERIFY(SetEvent(stopEvent));
  QCOMPARE(result.wait_for(std::chrono::seconds{1}), std::future_status::ready);
  QCOMPARE(result.get(), windows::NamedPipeIoResult::Stopped);
}

void WindowsPlatformIntegrationTests::namedPipeDeadlinesCancelIncompleteOperations_data() {
  QTest::addColumn<bool>("write");
  QTest::addColumn<bool>("withStopEvent");
  for (const bool write : {false, true})
    for (const bool stop : {false, true})
      QTest::newRow(write ? (stop ? "write/deadline-and-stop" : "write/deadline-only") : (stop ? "read/deadline-and-stop" : "read/deadline-only"))
          << write << stop;
}
void WindowsPlatformIntegrationTests::namedPipeDeadlinesCancelIncompleteOperations() {
  QFETCH(bool, write);
  QFETCH(bool, withStopEvent);
  NamedPipePair pipe;
  // A read has no input; a write exceeds the unread peer's native buffer.
  std::vector<std::byte> bytes(write ? 1024 * 1024 : 1);
  const windows::NamedPipeIoContext context{withStopEvent ? pipe.stop.get() : nullptr, std::chrono::steady_clock::now() + std::chrono::milliseconds{100}};
  const auto result = write ? windows::writeNamedPipe(pipe.server.get(), bytes, context) : windows::readNamedPipe(pipe.server.get(), bytes, context);
  QCOMPARE(result, windows::NamedPipeIoResult::TimedOut);
  if (!write) {
    // Cancellation must complete before the channel and caller-owned buffer are reused.
    const windows::NamedPipeIoContext next{pipe.stop.get(), std::chrono::steady_clock::now() + std::chrono::seconds{1}};
    const std::array expected{std::byte{0x42}};
    QCOMPARE(windows::writeNamedPipe(pipe.client.get(), expected, next), windows::NamedPipeIoResult::Completed);
    QCOMPARE(windows::readNamedPipe(pipe.server.get(), bytes, next), windows::NamedPipeIoResult::Completed);
    QCOMPARE(bytes.front(), expected.front());
  }
}

void WindowsPlatformIntegrationTests::namedPipeFrameSharesItsDeadlineWithTheResponse() {
  NamedPipePair pipe;
  const windows::NamedPipeIoContext initial{pipe.stop.get(), std::chrono::steady_clock::now() + std::chrono::seconds{1}};
  const std::array first{std::byte{0x42}};
  QCOMPARE(windows::writeNamedPipe(pipe.client.get(), first, initial), windows::NamedPipeIoResult::Completed);
  std::array<std::byte, 2> frame{};
  const auto received = windows::readNamedPipeFrame(pipe.server.get(), frame, pipe.stop.get(), std::chrono::milliseconds{100});
  QCOMPARE(received.result, windows::NamedPipeIoResult::TimedOut);
  QVERIFY(received.exchange.deadline.has_value());
  QCOMPARE(windows::writeNamedPipe(pipe.server.get(), first, received.exchange), windows::NamedPipeIoResult::TimedOut);
}

void WindowsPlatformIntegrationTests::namedPipeIdleWaitDoesNotConsumeTheFrameDeadline() {
  NamedPipePair pipe;
  std::promise<windows::NamedPipeIoResult> sentPromise;
  auto sent = sentPromise.get_future();
  std::jthread sender([&pipe, promise = std::move(sentPromise)]() mutable {
    try {
      std::this_thread::sleep_for(std::chrono::milliseconds{100});
      const std::array bytes{std::byte{0x41}, std::byte{0x42}};
      const auto result = windows::writeNamedPipe(pipe.client.get(), bytes, {pipe.stop.get(), std::chrono::steady_clock::now() + std::chrono::seconds{1}});
      promise.set_value(result);
      if (result != windows::NamedPipeIoResult::Completed) SetEvent(pipe.stop.get());
    } catch (...) {
      promise.set_exception(std::current_exception());
      SetEvent(pipe.stop.get());
    }
  });
  std::array<std::byte, 2> received{};
  const auto result = windows::readNamedPipeFrame(pipe.server.get(), received, pipe.stop.get(), std::chrono::milliseconds{50});
  QCOMPARE(result.result, windows::NamedPipeIoResult::Completed);
  QCOMPARE(sent.get(), windows::NamedPipeIoResult::Completed);
  QCOMPARE(received, (std::array{std::byte{0x41}, std::byte{0x42}}));
}

void WindowsPlatformIntegrationTests::singleInstanceForwardsTypedCommandsAcrossTrustedProcesses() {
  const QString discriminator = QStringLiteral("UwfWindowsPlatformIntegrationTests.%1").arg(QCoreApplication::applicationPid());
  uwf::windows::SecureSingleInstance instance(uwf::windows::SecureSingleInstance::Scope{discriminator});
  QSignalSpy activation(&instance, &uwf::windows::SecureSingleInstance::activationRequested);
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
  QVERIFY_THROWS_EXCEPTION(std::invalid_argument, static_cast<void>(instance.acquire(static_cast<app::ApplicationCommandKind>(3))));
  QCOMPARE(activation.count(), 1);
}

void WindowsPlatformIntegrationTests::busySingleInstanceCannotBlockAnotherLaunch() {
  const QString discriminator = QStringLiteral("UwfWindowsPlatformIntegrationTests.busy.%1").arg(QCoreApplication::applicationPid());
  windows::SecureSingleInstance instance(windows::SecureSingleInstance::Scope{discriminator});
  QCOMPARE(instance.acquire(), windows::SecureSingleInstance::AcquireResult::Primary);
  QProcess client;
  client.setProgram(QCoreApplication::applicationFilePath());
  client.setArguments({QStringLiteral("--single-instance-test-client"), discriminator, QStringLiteral("activate")});
  client.start();
  QVERIFY2(client.waitForStarted(), qUtf8Printable(client.errorString()));
  const auto cleanup = qScopeGuard([&] {
    if (client.state() == QProcess::NotRunning) return;
    client.kill();
    client.waitForFinished();
  });
  const windows::UniqueHandle process(OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(client.processId())));
  QVERIFY(process.valid());
  // Do not dispatch this primary's Qt events. The child authenticates the
  // correct executable, then must finish even while its peer cannot answer.
  QCOMPARE(WaitForSingleObject(process.get(), 10'000), DWORD{WAIT_OBJECT_0});
  QVERIFY(client.waitForFinished());
  QCOMPARE(client.exitStatus(), QProcess::NormalExit);
  QCOMPARE(client.exitCode(), 12);  // No acknowledgement means no forwarding receipt.
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
        // Raw command text covers only non-variable shell arguments; batch export covers the full matrix.
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
    auto script = QString::fromStdString(app::renderUwfmgrCommandScript({command}));
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
    const auto value = QString::fromStdString(app::renderUwfmgrCommand(command)).mid(prefix.size());
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

void WindowsPlatformIntegrationTests::omittedCommandsKeepTheWindowsShellSafe_data() {
  QTest::addColumn<bool>("target");
  QTest::addColumn<bool>("chained");
  for (const bool target : {false, true})
    for (const bool chained : {false, true}) QTest::newRow(qPrintable(QStringLiteral("target-%1-chained-%2").arg(target).arg(chained))) << target << chained;
}
void WindowsPlatformIntegrationTests::omittedCommandsKeepTheWindowsShellSafe() {
  QFETCH(bool, target);
  QFETCH(bool, chained);
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  const auto sentinel = QDir::toNativeSeparators(directory.filePath(QStringLiteral("injected.txt")));
  const auto unsafe = QStringLiteral("HKLM\\SOFTWARE\\Original\r\necho UWF_CMD_INJECTION>\"%1\"\r\n").arg(sentinel).toStdString();
  const std::string safe = "HKLM\\SOFTWARE\\SafeTarget";
  app::ConfigurationPlan plan;
  plan.hasPolicyContext = chained;
  plan.snapshot.next.overlay = {core::OverlayType::RAM, 1024, 512, 768};
  if (target)
    plan.snapshot.next.registryExclusions = {unsafe, safe};
  else
    plan.changes.addRegistryExclusions = {unsafe, safe};
  const auto product = target ? app::configurationTargetScript(plan) : app::configurationChangeScript(plan);
  auto script = QString::fromStdString(app::renderConfigurationScript(product));
  QVERIFY(!script.contains(QStringLiteral("UWF_CMD_INJECTION")));
  const auto client = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
  QByteArray expected;
  for (const auto& command : product.commands) {
    expected += QByteArray::number(static_cast<int>(command.kind)) + '\n';
    const auto replacement = QStringLiteral("\"%1\" --command-step-test-client %2 0").arg(client).arg(static_cast<int>(command.kind));
    script.replace(QString::fromStdString(app::renderUwfmgrCommand(command)), replacement);
  }
  // Execute only the recorder helper, including all retained scalar commands.
  QVERIFY(!script.contains(QStringLiteral("uwfmgr.exe")));
  QCOMPARE(std::ranges::count_if(product.commands, [](const auto& command) { return command.kind == app::UwfmgrKind::RegistryAddExclusion; }), chained ? 0 : 1);
  const auto scriptPath = directory.filePath(QStringLiteral("omitted.cmd"));
  QFile file(scriptPath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  const auto bytes = script.toUtf8();
  QCOMPARE(file.write(bytes), bytes.size());
  file.close();
  wchar_t systemDirectory[MAX_PATH]{};
  const auto length = GetSystemDirectoryW(systemDirectory, MAX_PATH);
  QVERIFY(length > 0 && length < MAX_PATH);
  const auto cmd = QDir(QString::fromWCharArray(systemDirectory)).filePath(QStringLiteral("cmd.exe"));
  for (const bool delayed : {false, true}) {
    QProcess process;
    process.setProgram(cmd);
    process.setNativeArguments(QStringLiteral("/d /v:%1 /s /c \"\"%2\"\"").arg(delayed ? "on" : "off", QDir::toNativeSeparators(scriptPath)));
    process.start();
    QVERIFY2(process.waitForStarted(), qUtf8Printable(process.errorString()));
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QCOMPARE(process.exitCode(), 0);
    QCOMPARE(process.readAllStandardOutput().replace("\r\n", "\n"), expected);
    QVERIFY(process.readAllStandardError().isEmpty());
    QVERIFY(!QFile::exists(sentinel));
  }
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
  app::ConfigurationPlan plan;
  plan.changes = changes;
  plan.snapshot.next.overlay = baseline;
  const auto product = app::configurationChangeScript(plan);
  const auto& commands = product.commands;
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
    auto script = QString::fromStdString(app::renderConfigurationScript(product));
    for (const auto& command : commands) {
      const auto replacement =
          QStringLiteral("\"%1\" --command-step-test-client %2 %3").arg(client).arg(static_cast<int>(command.kind)).arg(command.kind == failed ? 7 : 0);
      script.replace(QString::fromStdString(app::renderUwfmgrCommand(command)), replacement);
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

int runSingleInstanceClient(const QStringList& arguments) {
  if (arguments.size() != 4) return 10;

  uwf::app::ApplicationCommandKind command;
  if (arguments[3] == QStringLiteral("activate")) {
    command = uwf::app::ApplicationCommandKind::Activate;
  } else if (arguments[3] == QStringLiteral("ensure-running")) {
    command = uwf::app::ApplicationCommandKind::EnsureRunning;
  } else {
    return 11;
  }

  uwf::windows::SecureSingleInstance instance(uwf::windows::SecureSingleInstance::Scope{arguments[2]});
  return instance.acquire(command) == uwf::windows::SecureSingleInstance::AcquireResult::ForwardedExisting ? 0 : 12;
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
