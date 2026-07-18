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

#include <QSignalSpy>
#include <QtTest>
#include <exception>
#include <filesystem>

#include "app/SecureSingleInstance.h"
#include "ui/SystemInfoProvider.h"
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
  void embeddedCapabilityAndSnapshotUseTheProductionTransport();
  void singleInstanceAcquisitionIsIdempotent();
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

void WindowsPlatformIntegrationTests::singleInstanceAcquisitionIsIdempotent() {
  const QString discriminator = QStringLiteral("UwfWindowsPlatformIntegrationTests.%1").arg(QCoreApplication::applicationPid());
  app::SecureSingleInstance instance(app::SecureSingleInstance::Scope{discriminator});
  QSignalSpy activation(&instance, &app::SecureSingleInstance::activationRequested);
  const auto first = instance.acquire();
  QCOMPARE(instance.acquire(), first);
  QCOMPARE(first, app::SecureSingleInstance::AcquireResult::Primary);
  QVERIFY(instance.errorString().isEmpty());
  instance.enableActivationNotifications();
  QCOMPARE(activation.count(), 0);
}

}  // namespace uwf

QTEST_MAIN(uwf::WindowsPlatformIntegrationTests)

#include "WindowsPlatformIntegrationTests.moc"
