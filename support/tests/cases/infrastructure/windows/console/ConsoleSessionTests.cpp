// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "domain/uwf/contracts/Backend.h"
#include "infrastructure/windows/console/ConsoleSession.h"
#include "infrastructure/windows/console/WindowsConsoleEnvironment.h"
#include "infrastructure/windows/resources/UniqueHandle.h"
#include "infrastructure/windows/text/UnicodeConversion.h"

namespace {
constexpr auto identity = "12345678-1234-1234-1234-123456789abc";
class ConsoleSessionTests final : public QObject {
  Q_OBJECT
 private slots:
  void redirectedOutputDetectionUsesOnlyOutputDestinations_data() {
    QTest::addColumn<quint32>("stream");
    QTest::addColumn<QString>("destination");
    QTest::addColumn<bool>("expected");
    for (const auto& [name, stream] : {std::pair{"stdin", STD_INPUT_HANDLE}, std::pair{"stdout", STD_OUTPUT_HANDLE}, std::pair{"stderr", STD_ERROR_HANDLE}})
      for (const auto* destination : {"pipe", "file", "NUL", "null-handle", "invalid-handle"}) {
        const bool valid = std::string_view(destination) != "null-handle" && std::string_view(destination) != "invalid-handle";
        const auto row = QString::fromLatin1(name) + '/' + QString::fromLatin1(destination);
        QTest::newRow(qPrintable(row)) << static_cast<quint32>(stream) << QString::fromLatin1(destination) << (stream != STD_INPUT_HANDLE && valid);
      }
  }
  void redirectedOutputDetectionUsesOnlyOutputDestinations() {
    using namespace uwf::windows;
    QFETCH(quint32, stream);
    QFETCH(QString, destination);
    QFETCH(bool, expected);
    const QTemporaryDir directory;
    QVERIFY(directory.isValid());
    UniqueHandle handle;
    UniqueHandle pipeReader;
    HANDLE target = nullptr;
    if (destination == QStringLiteral("pipe")) {
      HANDLE reader = nullptr;
      HANDLE writer = nullptr;
      QVERIFY(CreatePipe(&reader, &writer, nullptr, 0));
      pipeReader.reset(reader);
      handle.reset(writer);
      target = stream == STD_INPUT_HANDLE ? pipeReader.get() : handle.get();
    } else if (destination == QStringLiteral("file")) {
      const auto path = directory.filePath(QStringLiteral("stream.txt")).toStdWString();
      handle.reset(CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr));
      QVERIFY(handle.valid());
      target = handle.get();
    } else if (destination == QStringLiteral("NUL")) {
      handle.reset(
          CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
      QVERIFY(handle.valid());
      target = handle.get();
    } else if (destination == QStringLiteral("invalid-handle")) {
      target = INVALID_HANDLE_VALUE;
    }

    const std::array<DWORD, 3> streams{STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
    const std::array<HANDLE, 3> saved{GetStdHandle(streams[0]), GetStdHandle(streams[1]), GetStdHandle(streams[2])};
    const auto restore = qScopeGuard([&] {
      for (std::size_t index = 0; index < streams.size(); ++index) SetStdHandle(streams[index], saved[index]);
    });
    for (const auto item : streams) QVERIFY(SetStdHandle(item, item == stream ? target : nullptr));
    QCOMPARE(hasRedirectedConsoleOutput(), expected);
    QCOMPARE(hasRedirectedConsoleOutput(), expected);
    // Detection borrows the table: it does not replace handles or attach a
    // console to supply missing output streams, including stdin-only capture.
    for (const auto item : streams) QCOMPARE(GetStdHandle(item), item == stream ? target : nullptr);
  }
  void redirectedBytesAreUtf8AndHandlesAreBorrowed() {
    using namespace uwf::windows;
    const QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("output.txt")).toStdWString();
    const UniqueHandle handle(CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, 0, nullptr));
    QVERIFY(handle.valid());
    const std::string text = "English \xE4\xB8\xAD\xE6\x96\x87 \xF0\x9F\x92\xBE\n";
    writeConsoleText(handle.get(), text);
    writeConsoleText(handle.get(), text);
    QVERIFY(SetFilePointerEx(handle.get(), {}, nullptr, FILE_BEGIN));
    std::array<char, 128> bytes{};
    DWORD read = 0;
    QVERIFY(ReadFile(handle.get(), bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr));
    QCOMPARE(std::string(bytes.data(), read), text + text);
    QVERIFY_THROWS_EXCEPTION(std::exception, writeConsoleText(handle.get(), "\xC3"));
    QVERIFY_THROWS_EXCEPTION(std::exception, writeConsoleText(INVALID_HANDLE_VALUE, "text"));
  }
  void pipesStaySeparate() {
    using namespace uwf::windows;
    std::array<UniqueHandle, 4> handles;
    for (std::size_t index = 0; index < handles.size(); index += 2) {
      HANDLE reader = nullptr;
      HANDLE writer = nullptr;
      QVERIFY(CreatePipe(&reader, &writer, nullptr, 0));
      handles[index].reset(reader);
      handles[index + 1].reset(writer);
    }
    writeConsoleText(handles[1].get(), "stdout");
    writeConsoleText(handles[3].get(), "stderr");
    for (std::size_t index = 0; index < handles.size(); index += 2) {
      std::array<char, 16> bytes{};
      DWORD read = 0;
      QVERIFY(ReadFile(handles[index].get(), bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr));
      QCOMPARE(std::string(bytes.data(), read), index == 0 ? std::string("stdout") : std::string("stderr"));
    }
  }
  void inputOnlyRedirectionSelectsConsoleWithoutReadingInput() {
    using namespace uwf::windows;
    HANDLE reader = nullptr;
    HANDLE writer = nullptr;
    QVERIFY(CreatePipe(&reader, &writer, nullptr, 0));
    const UniqueHandle input(reader);
    const UniqueHandle producer(writer);
    const std::array<DWORD, 3> streams{STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
    const std::array<HANDLE, 3> saved{GetStdHandle(streams[0]), GetStdHandle(streams[1]), GetStdHandle(streams[2])};
    const auto restore = qScopeGuard([&] {
      for (std::size_t index = 0; index < streams.size(); ++index) SetStdHandle(streams[index], saved[index]);
    });
    const bool prepared = SetStdHandle(streams[0], input.get()) && SetStdHandle(streams[1], nullptr) && SetStdHandle(streams[2], nullptr);
    const bool detected = prepared && hasConsoleContext();
    QVERIFY(prepared);
    QVERIFY(detected);
    QVERIFY(!hasRedirectedConsoleOutput());
  }
  void explicitVolumeTargetsPreserveIdentityAndOriginalBody() {
    const uwf::windows::WindowsConsoleEnvironment environment;
    const auto root = uwf::compat::volumePath(identity);
    const auto volume = environment.resolveVolume(root);
    QCOMPARE(volume.id, std::string(identity));
    QVERIFY(!volume.bindByDriveLetter);
    const auto file = environment.resolveFile(root + "Data\\..\\\xE4\xB8\xAD\xE6\x96\x87\\");
    QCOMPARE(file.volume, std::string(identity));
    QCOMPARE(file.path, std::string("\\Data\\..\\\xE4\xB8\xAD\xE6\x96\x87\\"));
    QCOMPARE(environment.resolveFile(root).path, std::string("\\"));
    QVERIFY(environment.resolveFile(root.substr(0, root.size() - 1)).path.empty());
  }
  void drivePrefixesRetainTheExplicitRootAndLiteralBody_data() {
    QTest::addColumn<QString>("prefix");
    QTest::addColumn<QString>("body");
    const std::array prefixes{std::pair{"ordinary", QString{}}, std::pair{"extended", QStringLiteral("\\\\?\\")},
                              std::pair{"device", QStringLiteral("\\\\.\\")}};
    const std::array bodies{std::pair{"root-no-separator", QString{}}, std::pair{"root-separator", QStringLiteral("\\")},
                            std::pair{"mount-like", QStringLiteral("\\UwfSyntheticMount\\Folder\\file.bin")},
                            std::pair{"dot-unicode", QStringLiteral("\\UwfSyntheticMount\\..\\资料\\file.bin")},
                            std::pair{"trailing", QStringLiteral("\\UwfSyntheticMount\\资料\\")}};
    for (const auto& [prefixName, prefix] : prefixes)
      for (const auto& [bodyName, body] : bodies) {
        const auto name = QByteArray(prefixName) + '/' + bodyName;
        QTest::newRow(name.constData()) << prefix << body;
      }
  }
  void drivePrefixesRetainTheExplicitRootAndLiteralBody() {
    QFETCH(QString, prefix);
    QFETCH(QString, body);
    std::array<wchar_t, MAX_PATH + 1> windowsDirectory{};
    const auto length = GetWindowsDirectoryW(windowsDirectory.data(), static_cast<UINT>(windowsDirectory.size()));
    QVERIFY(length >= 3 && length < windowsDirectory.size());
    QVERIFY(windowsDirectory[1] == L':' && windowsDirectory[2] == L'\\');
    const auto drive = uwf::windows::wideToUtf8(std::wstring_view(windowsDirectory.data(), 2));
    const uwf::windows::WindowsConsoleEnvironment environment;
    const auto expected = environment.resolveVolume(drive);
    const auto literal = body.toUtf8().toStdString();
    const auto target = environment.resolveFile(prefix.toUtf8().toStdString() + drive + literal);
    QCOMPARE(target.volume, expected.id);
    QCOMPARE(target.path, literal);
  }
  void invalidPathsNeverAddressAnotherObject_data() {
    QTest::addColumn<QByteArray>("path");
    for (const auto* input : {"relative.txt", "C:relative.txt", "\\\\server\\share\\file", "", "\\\\?\\Volume{bad}\\file"})
      QTest::newRow(input) << QByteArray(input);
    QTest::newRow("invalid-utf8") << QByteArray("\xC3");
    QTest::newRow("embedded-nul") << QByteArray("C:\\a\0b", 7);
  }
  void invalidPathsNeverAddressAnotherObject() {
    QFETCH(QByteArray, path);
    const uwf::windows::WindowsConsoleEnvironment environment;
    try {
      static_cast<void>(environment.resolveFile(path.toStdString()));
      QFAIL("Invalid file operand was accepted");
    } catch (const uwf::compat::Error& error) {
      QCOMPARE(error.failure().kind, uwf::compat::ErrorKind::InvalidArgument);
      QCOMPARE(error.failure().effect, uwf::compat::Effect::None);
    }
  }
};
}  // namespace
QTEST_APPLESS_MAIN(ConsoleSessionTests)
#include "ConsoleSessionTests.moc"
