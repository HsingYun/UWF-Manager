// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <windows.h>

#include <QElapsedTimer>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <array>
#include <cstddef>

#include "application/configuration/commands/UwfmgrCommandParser.h"

namespace {
QString productExecutablePath() {
  const auto overridePath = qEnvironmentVariable("UWF_TEST_EXECUTABLE_PATH");
  return overridePath.isEmpty() ? QString::fromUtf8(UWF_EXECUTABLE_PATH) : overridePath;
}

struct WindowSearch {
  DWORD process = 0;
  HWND dialog = nullptr;
  QString title;
  int visibleWindows = 0;
};

BOOL CALLBACK findOutputDialog(const HWND window, const LPARAM data) {
  auto& search = *reinterpret_cast<WindowSearch*>(data);
  DWORD process = 0;
  GetWindowThreadProcessId(window, &process);
  if (process != search.process || !IsWindowVisible(window)) return TRUE;
  ++search.visibleWindows;
  constexpr std::size_t maximumWindowTitle = 512;
  std::array<wchar_t, maximumWindowTitle> title{};
  const auto length = GetWindowTextW(window, title.data(), static_cast<int>(title.size()));
  const auto text = QString::fromWCharArray(title.data(), length);
  if (text == QStringLiteral("UWF Manager - Help") || text == QStringLiteral("UWF Manager - Command result")) {
    search.dialog = window;
    search.title = text;
  }
  return TRUE;
}

struct ProcessResult {
  bool dialogFound = false;
  bool waitedForClose = false;
  int visibleWindows = 0;
  QString title;
  int code = -1;
  QProcess::ExitStatus status = QProcess::CrashExit;
  QByteArray output;
  QByteArray error;
};

enum class OutputHandles { Pipes, None, OnlyOutput, OnlyError };

ProcessResult execute(const QStringList& arguments, const OutputHandles handles = OutputHandles::Pipes, const QString& outputFile = {},
                      const QString& errorFile = {}) {
  QProcess child;
  child.setProgram(productExecutablePath());
  child.setArguments(arguments);
  if (handles != OutputHandles::None) {
    auto environment = QProcessEnvironment::systemEnvironment();
    // Captured calls must work without loading a GUI platform plugin.
    environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("uwf-test-no-gui-platform"));
    child.setProcessEnvironment(environment);
  }
  if (!outputFile.isEmpty()) child.setStandardOutputFile(outputFile);
  if (!errorFile.isEmpty()) child.setStandardErrorFile(errorFile);
  if (handles != OutputHandles::Pipes) {
    child.setCreateProcessArgumentsModifier([handles](QProcess::CreateProcessArguments* launch) {
      launch->startupInfo->dwFlags |= STARTF_USESTDHANDLES;
      if (handles != OutputHandles::OnlyOutput) launch->startupInfo->hStdOutput = nullptr;
      if (handles != OutputHandles::OnlyError) launch->startupInfo->hStdError = nullptr;
      // stdin stays a pipe: it still selects the command entry with no operands,
      // but must never make graphical output look like captured output.
    });
  }
  child.start();
  ProcessResult result;
  if (!child.waitForStarted(10000)) {
    result.error = child.errorString().toUtf8();
    return result;
  }
  const auto cleanup = qScopeGuard([&] {
    if (child.state() != QProcess::NotRunning) {
      child.kill();
      child.waitForFinished(5000);
    }
  });
  if (handles == OutputHandles::None) {
    QElapsedTimer timer;
    timer.start();
    WindowSearch search;
    search.process = static_cast<DWORD>(child.processId());
    while (timer.elapsed() < 10000 && child.state() != QProcess::NotRunning) {
      search.dialog = nullptr;
      search.title.clear();
      search.visibleWindows = 0;
      EnumWindows(findOutputDialog, reinterpret_cast<LPARAM>(&search));
      if (search.dialog) break;
      QTest::qWait(10);
    }
    if (!search.dialog) {
      result.error = "The command output dialog did not appear";
      return result;
    }
    result.dialogFound = true;
    result.title = search.title;
    result.visibleWindows = search.visibleWindows;
    result.waitedForClose = child.state() == QProcess::Running;
    if (!PostMessageW(search.dialog, WM_CLOSE, 0, 0) || !child.waitForFinished(10000)) {
      result.error = "The command process did not exit after its dialog closed";
      return result;
    }
  } else if (!child.waitForFinished(10000)) {
    result.error = "The captured command did not exit without interactive input";
    return result;
  }
  result.code = child.exitCode();
  result.status = child.exitStatus();
  if (outputFile.isEmpty()) result.output = child.readAllStandardOutput();
  if (errorFile.isEmpty()) result.error = child.readAllStandardError();
  return result;
}

QByteArray productResource(const WORD type) {
  const auto path = productExecutablePath().toStdWString();
  const auto module = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
  if (!module) return {};
  const auto release = qScopeGuard([&] { FreeLibrary(module); });
  const auto resource = FindResourceW(module, MAKEINTRESOURCEW(1), MAKEINTRESOURCEW(type));
  if (!resource) return {};
  const auto loaded = LoadResource(module, resource);
  if (!loaded) return {};
  const auto bytes = static_cast<const char*>(LockResource(loaded));
  if (!bytes) return {};
  return QByteArray(bytes, static_cast<qsizetype>(SizeofResource(module, resource)));
}

class ConsoleApplicationTests final : public QObject {
  Q_OBJECT
 private slots:
  void embeddedManifestUsesInvokerPrivileges() {
    const auto manifest = productResource(24);
    QVERIFY(!manifest.isEmpty());
    QVERIFY(manifest.contains("asInvoker"));
    QVERIFY(!manifest.contains("requireAdministrator"));
  }

  void executableRemainsAGuiApplication() {
    QFile image(productExecutablePath());
    QVERIFY(image.open(QIODevice::ReadOnly));
    IMAGE_DOS_HEADER dos{};
    QCOMPARE(image.read(reinterpret_cast<char*>(&dos), static_cast<qint64>(sizeof(dos))), static_cast<qint64>(sizeof(dos)));
    QCOMPARE(dos.e_magic, static_cast<WORD>(IMAGE_DOS_SIGNATURE));
    QVERIFY(dos.e_lfanew > 0);
    QVERIFY(image.seek(dos.e_lfanew));
    DWORD signature = 0;
    QCOMPARE(image.read(reinterpret_cast<char*>(&signature), static_cast<qint64>(sizeof(signature))), static_cast<qint64>(sizeof(signature)));
    QCOMPARE(signature, static_cast<DWORD>(IMAGE_NT_SIGNATURE));
    static_assert(offsetof(IMAGE_OPTIONAL_HEADER32, Subsystem) == offsetof(IMAGE_OPTIONAL_HEADER64, Subsystem));
    const auto subsystemPosition = static_cast<qint64>(dos.e_lfanew) + static_cast<qint64>(sizeof(signature) + sizeof(IMAGE_FILE_HEADER)) +
                                   static_cast<qint64>(offsetof(IMAGE_OPTIONAL_HEADER64, Subsystem));
    QVERIFY(image.seek(subsystemPosition));
    WORD subsystem = 0;
    QCOMPARE(image.read(reinterpret_cast<char*>(&subsystem), static_cast<qint64>(sizeof(subsystem))), static_cast<qint64>(sizeof(subsystem)));
    QCOMPARE(subsystem, static_cast<WORD>(IMAGE_SUBSYSTEM_WINDOWS_GUI));
  }

  void embeddedVersionResourceKeepsProductIdentity() {
    const auto metadata = productResource(16);
    QVERIFY(!metadata.isEmpty());
    const auto text = QString::fromUtf16(reinterpret_cast<const char16_t*>(metadata.constData()), metadata.size() / static_cast<qsizetype>(sizeof(char16_t)));
    QVERIFY(text.contains(QStringLiteral("FileVersion")));
    QVERIFY(text.contains(QStringLiteral("ProductVersion")));
    QVERIFY(text.contains(QStringLiteral("UWF Manager")));
    const auto help = QString::fromStdString(uwf::app::uwfmgrHelpText());
    const auto banner = help.first(help.indexOf(u'\n')).trimmed();
    static const QRegularExpression version(QStringLiteral(R"(^UWF Manager version ([0-9]+\.[0-9]+\.[0-9]+[^\s]*)$)"));
    const auto match = version.match(banner);
    QVERIFY2(match.hasMatch(), qPrintable(banner));
    QVERIFY(text.contains(match.captured(1)));
  }

  void everyHelpEntryIsCapturedWithoutAGui_data() {
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<QString>("family");
    QTest::newRow("default-redirected") << QStringList{} << QString{};
    QTest::newRow("root-help") << QStringList{QStringLiteral("help")} << QString{};
    QTest::newRow("root-question") << QStringList{QStringLiteral("?")} << QString{};
    for (const auto* family : {"filter", "overlay", "volume", "file", "registry", "servicing"}) {
      const auto name = QString::fromLatin1(family);
      QTest::newRow(qUtf8Printable(name + QStringLiteral("-bare"))) << QStringList{name} << name;
      QTest::newRow(qUtf8Printable(name + QStringLiteral("-help"))) << QStringList{name, QStringLiteral("help")} << name;
      QTest::newRow(qUtf8Printable(name + QStringLiteral("-question"))) << QStringList{name, QStringLiteral("?")} << name;
    }
  }

  void everyHelpEntryIsCapturedWithoutAGui() {
    QFETCH(QStringList, arguments);
    QFETCH(QString, family);
    const auto result = execute(arguments);
    QVERIFY(!result.dialogFound);
    QCOMPARE(result.status, QProcess::NormalExit);
    QCOMPARE(result.code, 0);
    QCOMPARE(result.output, QByteArray::fromStdString(uwf::app::uwfmgrHelpText(family.toStdString())));
    QVERIFY(result.output.startsWith("UWF Manager version "));
    QVERIFY(result.output.contains("Usage:"));
    QVERIFY(result.error.isEmpty());
  }

  void everyHelpEntryUsesAGraphicalDialog_data() { everyHelpEntryIsCapturedWithoutAGui_data(); }

  void everyHelpEntryUsesAGraphicalDialog() {
    if (qEnvironmentVariable("UWF_TEST_ALLOW_DESKTOP") != QStringLiteral("1")) QSKIP("Command dialog checks require UWF_TEST_ALLOW_DESKTOP=1.");
    QFETCH(QStringList, arguments);
    const auto result = execute(arguments, OutputHandles::None);
    QVERIFY2(result.dialogFound, result.error.constData());
    QVERIFY(result.waitedForClose);
    QCOMPARE(result.visibleWindows, 1);
    QCOMPARE(result.title, QStringLiteral("UWF Manager - Help"));
    QCOMPARE(result.status, QProcess::NormalExit);
    QCOMPARE(result.code, 0);
    QVERIFY(result.output.isEmpty());
    QVERIFY(result.error.isEmpty());
  }

  void invalidCommandsAreCapturedWithTheirOriginalExitCode_data() {
    QTest::addColumn<QStringList>("arguments");
    for (const auto* flag : {"--commit-stage", "--install", "--uninstall", "--help", "--version", "--other"})
      QTest::newRow(flag) << QStringList{QString::fromLatin1(flag)};
    QTest::newRow("hidden-power") << QStringList{QStringLiteral("filter"), QStringLiteral("restart")};
    QTest::newRow("hidden-filter-report") << QStringList{QStringLiteral("filter"), QStringLiteral("get-config")};
    QTest::newRow("hidden-overlay-files") << QStringList{QStringLiteral("overlay"), QStringLiteral("get-files")};
    QTest::newRow("unknown-bare-family") << QStringList{QStringLiteral("unknown")};
    QTest::newRow("unknown-family-help") << QStringList{QStringLiteral("unknown"), QStringLiteral("help")};
    QTest::newRow("bad-arity") << QStringList{QStringLiteral("filter"), QStringLiteral("enable"), QStringLiteral("extra")};
    QTest::newRow("bad-size") << QStringList{QStringLiteral("overlay"), QStringLiteral("set-size"), QStringLiteral("4294967296")};
    QTest::newRow("unicode-unknown-command") << QStringList{QString::fromUtf8("\xE4\xB8\xAD\xE6\x96\x87\xF0\x9F\x92\xBE%&")};
  }

  void invalidCommandsAreCapturedWithTheirOriginalExitCode() {
    QFETCH(QStringList, arguments);
    const auto result = execute(arguments);
    QVERIFY(!result.dialogFound);
    QCOMPARE(result.status, QProcess::NormalExit);
    QCOMPARE(result.code, 87);
    QVERIFY(result.output.isEmpty());
    QVERIFY(result.error.startsWith("Error:"));
    QVERIFY(result.error.contains("Effect: None"));
    if (arguments.size() == 1 && arguments.front().startsWith(QString::fromUtf8("\xE4\xB8\xAD"))) QVERIFY(result.error.contains(arguments.front().toUtf8()));
  }

  void invalidCommandsUseAGraphicalDialogWithoutCapture_data() { invalidCommandsAreCapturedWithTheirOriginalExitCode_data(); }

  void invalidCommandsUseAGraphicalDialogWithoutCapture() {
    if (qEnvironmentVariable("UWF_TEST_ALLOW_DESKTOP") != QStringLiteral("1")) QSKIP("Command dialog checks require UWF_TEST_ALLOW_DESKTOP=1.");
    QFETCH(QStringList, arguments);
    const auto result = execute(arguments, OutputHandles::None);
    QVERIFY2(result.dialogFound, result.error.constData());
    QVERIFY(result.waitedForClose);
    QCOMPARE(result.visibleWindows, 1);
    QCOMPARE(result.title, QStringLiteral("UWF Manager - Command result"));
    QCOMPARE(result.status, QProcess::NormalExit);
    QCOMPARE(result.code, 87);
    QVERIFY(result.output.isEmpty());
    QVERIFY(result.error.isEmpty());
  }

  void redirectedFilesCaptureTheOriginalOutput_data() {
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<int>("exitCode");
    QTest::newRow("help") << QStringList{QStringLiteral("help")} << 0;
    QTest::newRow("error") << QStringList{QStringLiteral("--other")} << 87;
  }

  void redirectedFilesCaptureTheOriginalOutput() {
    QFETCH(QStringList, arguments);
    QFETCH(int, exitCode);
    const QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto outputPath = directory.filePath(QStringLiteral("stdout.txt"));
    const auto errorPath = directory.filePath(QStringLiteral("stderr.txt"));
    const auto result = execute(arguments, OutputHandles::Pipes, outputPath, errorPath);
    QVERIFY(!result.dialogFound);
    QCOMPARE(result.status, QProcess::NormalExit);
    QCOMPARE(result.code, exitCode);
    QFile output(outputPath);
    QFile error(errorPath);
    QVERIFY(output.open(QIODevice::ReadOnly));
    QVERIFY(error.open(QIODevice::ReadOnly));
    if (exitCode == 0) {
      QCOMPARE(output.readAll(), QByteArray::fromStdString(uwf::app::uwfmgrHelpText()));
      QVERIFY(error.readAll().isEmpty());
    } else {
      QVERIFY(output.readAll().isEmpty());
      const auto diagnostic = error.readAll();
      QVERIFY(diagnostic.startsWith("Error:"));
      QVERIFY(diagnostic.contains("Effect: None"));
    }
  }

  void oneRedirectedOutputIsEnough_data() {
    QTest::addColumn<bool>("help");
    QTest::addColumn<bool>("nul");
    QTest::newRow("stdout-pipe") << true << false;
    QTest::newRow("stderr-pipe") << false << false;
    QTest::newRow("stdout-nul") << true << true;
    QTest::newRow("stderr-nul") << false << true;
  }

  void oneRedirectedOutputIsEnough() {
    QFETCH(bool, help);
    QFETCH(bool, nul);
    const auto handles = help ? OutputHandles::OnlyOutput : OutputHandles::OnlyError;
    const auto destination = nul ? QProcess::nullDevice() : QString{};
    const auto result =
        execute({help ? QStringLiteral("help") : QStringLiteral("--other")}, handles, help ? destination : QString{}, help ? QString{} : destination);
    QVERIFY(!result.dialogFound);
    QVERIFY2(result.status == QProcess::NormalExit, result.error.constData());
    QCOMPARE(result.code, help ? 0 : 87);
    if (nul) {
      QVERIFY(result.output.isEmpty());
      QVERIFY(result.error.isEmpty());
    } else if (help) {
      QCOMPARE(result.output, QByteArray::fromStdString(uwf::app::uwfmgrHelpText()));
      QVERIFY(result.error.isEmpty());
    } else {
      QVERIFY(result.output.isEmpty());
      QVERIFY(result.error.startsWith("Error:"));
      QVERIFY(result.error.contains("Effect: None"));
    }
  }
};
}  // namespace
QTEST_GUILESS_MAIN(ConsoleApplicationTests)
#include "ConsoleApplicationTests.moc"
