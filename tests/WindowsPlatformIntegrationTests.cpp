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

#include <QApplication>
#include <QProcess>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTest>
#include <QUuid>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <future>
#include <memory>
#include <string>
#include <thread>

#include "app/SecureSingleInstance.h"
#include "service/EnhancedModePipeIo.h"
#include "ui/SystemInfoProvider.h"
#include "util/DriveLetter.h"
#include "util/StringUtil.h"
#include "util/SystemHardwareInfo.h"
#include "util/WindowsVersion.h"
#include "uwf/SystemCheck.h"
#include "uwf/UwfSnapshot.h"
#include "uwf/wmi/WmiClient.h"
#include "uwf/wmi/WmiError.h"
#include "uwf/wmi/WmiException.h"

namespace uwf {

class WindowsPlatformIntegrationTests final : public QObject {
  Q_OBJECT

 private slots:
  void windowsAndHardwareMetadataRemainInternallyConsistent();
  void cimv2TransportDistinguishesPresentAndMissingClasses();
  void volumeMountQueryIncludesTheSystemDrive();
  void embeddedCapabilityAndSnapshotUseTheProductionTransport();
  void explicitWmiShutdownRetiresTheWorkerContext();
  void enhancedModePipeReadIsCancelledByStopEvent();
  void singleInstanceForwardsTypedCommandsAcrossTrustedProcesses();
};

void WindowsPlatformIntegrationTests::windowsAndHardwareMetadataRemainInternallyConsistent() {
  const auto& version = windowsVersionInfo();
  QVERIFY(version.major != 0);
  QVERIFY(version.build != 0);
  QVERIFY(!version.productName.empty());

  const auto& hardware = systemHardwareInfo();
  QVERIFY(hardware.totalMemoryBytes != 0);
  std::uint64_t installedBytes = 0;
  for (const auto& module : hardware.memoryModules) {
    QVERIFY(module.capacityBytes != 0);
    QVERIFY(module.capacityBytes <= hardware.totalMemoryBytes);
    installedBytes += module.capacityBytes;
  }
  if (!hardware.memoryModules.empty()) QCOMPARE(installedBytes, hardware.totalMemoryBytes);

  const auto check = runSystemChecks();
  QCOMPARE(check.productName, version.productName);
  QCOMPARE(check.editionId, version.editionId);
  QVERIFY(check.status == CheckStatus::Ok || check.status == CheckStatus::UnsupportedSystem);
  const std::string manager = uwfmgrPath();
  if (!manager.empty()) QVERIFY(std::filesystem::is_regular_file(manager));
  QVERIFY(!ui::SystemInfoProvider::summaryHtml().isEmpty());
}

void WindowsPlatformIntegrationTests::cimv2TransportDistinguishesPresentAndMissingClasses() {
  try {
    auto& session = cimv2WmiSession();
    session.ensureConnected();
    QCOMPARE(session.classStatus("Win32_OperatingSystem"), WmiClassStatus::Present);
    QCOMPARE(session.classStatus("UwfManager_Class_That_Does_Not_Exist"), WmiClassStatus::Missing);
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

void WindowsPlatformIntegrationTests::volumeMountQueryIncludesTheSystemDrive() {
  try {
    const auto systemDrive = drive::systemLetter();
    QVERIFY(!systemDrive.empty());
    const auto root = utf8ToWide(systemDrive + '\\');
    std::array<wchar_t, 50> volumeName{};
    QVERIFY(GetVolumeNameForVolumeMountPointW(root.c_str(), volumeName.data(), static_cast<DWORD>(volumeName.size())));
    const auto letters = drive::mountedLetters(wideToUtf8(volumeName.data()));
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
    auto& session = embeddedWmiSession();
    const UwfCapability capability = probeUwfCapability(session);
    QVERIFY(capability == UwfCapability::Available || capability == UwfCapability::Unavailable);
    try {
      const auto snapshot = readSnapshot(session, capability, isElevated());
      QCOMPARE(snapshot.uwfAvailable, capability == UwfCapability::Available);
      if (snapshot.uwfAvailable) {
        QVERIFY(snapshot.unavailableReason.empty());
        QCOMPARE(snapshot.elevated, isElevated());
      } else {
        QVERIFY(!snapshot.unavailableReason.empty());
      }
    } catch (const WmiInfrastructureError& error) {
      const auto wmiError = WmiError(static_cast<std::int32_t>(error.code().value()));
      if (isElevated() || wmiError.code() != WmiErrorCode::AccessDenied) throw;
      QVERIFY(capability == UwfCapability::Available);
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
        initializeWmiRuntime();
        const auto shutdownWmi = qScopeGuard([] { shutdownWmiRuntime(); });
        auto* const firstSession = &cimv2WmiSession();
        firstSession->ensureConnected();
        initializeWmiRuntime();
        if (&cimv2WmiSession() != firstSession) {
          promise.set_value(QStringLiteral("active WMI context was replaced"));
          return;
        }
      }

      try {
        static_cast<void>(cimv2WmiSession());
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
  QVERIFY2(error.isEmpty(), qPrintable(error));
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
  QCOMPARE(service::connectEnhancedPipe(server, stopEvent), service::EnhancedPipeIoResult::Completed);

  std::promise<service::EnhancedPipeIoResult> resultPromise;
  auto result = resultPromise.get_future();
  std::jthread reader([server, stopEvent, promise = std::move(resultPromise)]() mutable {
    try {
      std::array<std::byte, 1> buffer{};
      promise.set_value(service::readEnhancedPipe(server, buffer, stopEvent));
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
  });

  QTest::qWait(10);
  QVERIFY(SetEvent(stopEvent));
  QCOMPARE(result.wait_for(std::chrono::seconds{1}), std::future_status::ready);
  QCOMPARE(result.get(), service::EnhancedPipeIoResult::Stopped);
}

void WindowsPlatformIntegrationTests::singleInstanceForwardsTypedCommandsAcrossTrustedProcesses() {
  const QString discriminator = QStringLiteral("UwfWindowsPlatformIntegrationTests.%1").arg(QCoreApplication::applicationPid());
  app::SecureSingleInstance instance(app::SecureSingleInstance::Scope{discriminator});
  QSignalSpy activation(&instance, &app::SecureSingleInstance::activationRequested);
  QSignalSpy commit(&instance, &app::SecureSingleInstance::commitStageRequested);
  const auto first = instance.acquire();
  QCOMPARE(instance.acquire(), first);
  QCOMPARE(first, app::SecureSingleInstance::AcquireResult::Primary);
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
  QVERIFY2(activateClient->waitForStarted(), qPrintable(activateClient->errorString()));
  QTRY_COMPARE_WITH_TIMEOUT(activation.count(), 1, 3000);
  QTRY_COMPARE_WITH_TIMEOUT(activateClient->state(), QProcess::NotRunning, 3000);
  QCOMPARE(activateClient->exitStatus(), QProcess::NormalExit);
  QCOMPARE(activateClient->exitCode(), 0);

  auto quietClient = launchClient(QStringLiteral("ensure-running"));
  QVERIFY2(quietClient->waitForStarted(), qPrintable(quietClient->errorString()));
  QTRY_COMPARE_WITH_TIMEOUT(quietClient->state(), QProcess::NotRunning, 3000);
  QCOMPARE(quietClient->exitStatus(), QProcess::NormalExit);
  QCOMPARE(quietClient->exitCode(), 0);
  QCOMPARE(activation.count(), 1);
  QCOMPARE(commit.count(), 0);

  const app::ApplicationCommandResult expected{app::ApplicationCommandOutcome::CompletedWithFailures, 3, 1, 1, 2, 1, QStringLiteral("one staged file failed")};
  const QMetaObject::Connection completion = connect(&instance, &app::SecureSingleInstance::commitStageRequested, &instance,
                                                     [&](const std::uint64_t requestToken) { instance.completeCommand(requestToken, expected); });
  auto commitClient = launchClient(QStringLiteral("commit-stage"));
  QVERIFY2(commitClient->waitForStarted(), qPrintable(commitClient->errorString()));
  QTRY_COMPARE_WITH_TIMEOUT(commit.count(), 1, 3000);
  QTRY_COMPARE_WITH_TIMEOUT(commitClient->state(), QProcess::NotRunning, 3000);
  disconnect(completion);
  QCOMPARE(commitClient->exitStatus(), QProcess::NormalExit);
  QCOMPARE(commitClient->exitCode(), 0);
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

  uwf::app::SecureSingleInstance instance(uwf::app::SecureSingleInstance::Scope{arguments[2]});
  uwf::app::ApplicationCommandResult result;
  if (instance.acquire(command, &result) != uwf::app::SecureSingleInstance::AcquireResult::ForwardedExisting) return 12;
  if (command != uwf::app::ApplicationCommandKind::CommitStage) return equalResult(result, {}) ? 0 : 13;

  const uwf::app::ApplicationCommandResult expected{
      uwf::app::ApplicationCommandOutcome::CompletedWithFailures, 3, 1, 1, 2, 1, QStringLiteral("one staged file failed")};
  return equalResult(result, expected) ? 0 : 14;
}

}  // namespace

int main(int argc, char** argv) {
  QApplication application(argc, argv);
  const QStringList arguments = application.arguments();
  if (arguments.size() >= 2 && arguments[1] == QStringLiteral("--single-instance-test-client")) {
    return runSingleInstanceClient(arguments);
  }
  uwf::initializeWmiRuntime();
  const auto shutdownWmi = qScopeGuard([] { uwf::shutdownWmiRuntime(); });
  uwf::WindowsPlatformIntegrationTests tests;
  return QTest::qExec(&tests, argc, argv);
}

#include "WindowsPlatformIntegrationTests.moc"
