// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <memory>
#include <stdexcept>

#include "domain/uwf/policies/UwfConstraints.h"
#include "infrastructure/windows/management/wmi/WmiException.h"
#include "infrastructure/windows/storage/WindowsVolumeRepository.h"
#include "support/windows/RecordingWmiOperations.h"

namespace {
using namespace uwf;
using Value = windows::wmi::WmiValue;
constexpr int fixedLocalDriveType = 3;  // Win32_LogicalDisk.DriveType fixture encoding.
class Inventory final : public app::VolumeRepository {
 public:
  mutable int reads = 0;
  bool fail = false;
  std::vector<core::VolumeInfo> read() const override {
    ++reads;
    if (fail) throw std::runtime_error("Independent inventory failed");
    return {{"X:", "Volume{independent}", "NTFS", "Recovered", 4096, 2048, core::VolumeSupport::Supported}};
  }
  std::string driveForPath(const std::string&) const override { throw std::logic_error("Unexpected path query"); }
  std::vector<std::string> mountedLetters(const std::string&) const override { throw std::logic_error("Unexpected mount query"); }
  std::vector<std::string> mountedPaths(const std::string&) const override { throw std::logic_error("Unexpected mount query"); }
  std::string systemDrive() const override { throw std::logic_error("Unexpected system-drive query"); }
};

windows::wmi::WmiRow logicalDisk(const char* drive, const char* fileSystem, const std::uint64_t bytes, const int type) {
  return {{"DeviceID", Value::fromString(drive)}, {"FileSystem", Value::fromString(fileSystem)}, {"VolumeName", Value::fromString("Fixture")},
          {"Size", Value::fromUInt(bytes)},       {"FreeSpace", Value::fromUInt(bytes / 2)},     {"DriveType", Value::fromInt(type)}};
}

class WindowsVolumeRepositoryTests final : public QObject {
  Q_OBJECT
 private slots:
  void physicalAdmissionRequiresCompleteAffirmativeFacts() {
    const std::optional<bool> states[] = {std::nullopt, false, true};
    using Admission = core::PhysicalVolumeAdmission;
    constexpr Admission matrix[3][3] = {{Admission::Unknown, Admission::Unknown, Admission::Unknown},
                                        {Admission::NotFixedMedia, Admission::NotFixedMedia, Admission::NotFixedMedia},
                                        {Admission::Unknown, Admission::UnsupportedBus, Admission::Eligible}};
    for (std::size_t fixed = 0; fixed < std::size(states); ++fixed)
      for (std::size_t bus = 0; bus < std::size(states); ++bus) {
        const core::PhysicalVolumeFacts facts{states[fixed], states[bus]};
        const auto expected = matrix[fixed][bus];
        QCOMPARE(core::physicalVolumeAdmission(facts), expected);
        core::VolumeInfo volume{"C:", "Volume{fixture}", "NTFS", {}, 1000, 500, core::VolumeSupport::Supported, facts};
        QCOMPARE(core::canRegisterVolume(volume), expected == core::PhysicalVolumeAdmission::Eligible);
        volume.support = core::VolumeSupport::FileSystemLimited;
        QCOMPARE(core::canRegisterVolume(volume), expected == core::PhysicalVolumeAdmission::Eligible);
        volume.volumeName.clear();
        QVERIFY(!core::canRegisterVolume(volume));
      }
  }
  void oneFailedPhysicalObservationDoesNotHideOtherVolumes() {
    test::native::RecordingWmiOperations transport;
    std::vector<windows::wmi::WmiRow> disks, mappings;
    for (const auto* letter : {"C:", "D:", "E:", "F:", "G:", "H:"}) {
      disks.push_back(logicalDisk(letter, "NTFS", 1000, fixedLocalDriveType));
      mappings.push_back({{"DeviceID", Value::fromString(std::string("Volume{") + letter[0] + '}')}, {"DriveLetter", Value::fromString(letter)}});
    }
    transport.queryResults.push_back(disks);
    transport.queryResults.push_back(mappings);
    auto fallback = std::make_shared<Inventory>();
    QStringList observed;
    const auto repository = windows::windowsVolumeRepository(transport, fallback, [&](const std::string& id) -> core::PhysicalVolumeFacts {
      observed.append(QString::fromStdString(id));
      if (id == "Volume{D}") return {true, false};  // USB / Storage Spaces admission.
      if (id == "Volume{E}") return {false, {}};
      if (id == "Volume{F}") return {true, {}};  // Unknown storage bus.
      if (id == "Volume{G}") throw std::runtime_error("Device observation denied");
      if (id == "Volume{H}") throw 42;
      return {true, true};
    });
    const auto volumes = repository->read();
    QCOMPARE(volumes.size(), std::size_t{6});
    QCOMPARE(observed.size(), 6);
    QCOMPARE(fallback->reads, 0);
    const auto expected = {core::PhysicalVolumeAdmission::Eligible, core::PhysicalVolumeAdmission::UnsupportedBus, core::PhysicalVolumeAdmission::NotFixedMedia,
                           core::PhysicalVolumeAdmission::Unknown,  core::PhysicalVolumeAdmission::Unknown,        core::PhysicalVolumeAdmission::Unknown};
    std::size_t index = 0;
    for (const auto admission : expected) {
      QCOMPARE(core::physicalVolumeAdmission(volumes[index].physical), admission);
      QCOMPARE(volumes[index].support, core::VolumeSupport::Supported);
      ++index;
    }
  }
  void independentInventoryIsUsedOnlyAfterThePrimaryFails_data() {
    QTest::addColumn<int>("failure");
    QTest::newRow("primary-success") << 0;
    QTest::newRow("primary-failure") << 1;
    QTest::newRow("primary-unknown-exception") << 2;
    QTest::newRow("both-fail") << 3;
  }
  void independentInventoryIsUsedOnlyAfterThePrimaryFails() {
    QFETCH(int, failure);
    test::native::RecordingWmiOperations transport;
    transport.queryResults.push_back({});
    transport.queryResults.push_back({});
    if (failure) transport.queryFailure = failure == 2 ? std::make_exception_ptr(42) : std::make_exception_ptr(std::runtime_error("WMI offline"));
    auto fallback = std::make_shared<Inventory>();
    fallback->fail = failure == 3;
    const auto repository = windows::windowsVolumeRepository(transport, fallback);
    if (failure == 3) {
      try {
        (void)repository->read();
        QFAIL("Both inventory failures must be reported");
      } catch (const std::runtime_error& error) {
        QVERIFY(std::string(error.what()).find("WMI offline") != std::string::npos);
        QVERIFY(std::string(error.what()).find("Independent inventory failed") != std::string::npos);
      }
    } else {
      const auto observed = repository->read();
      QCOMPARE(observed.size(), failure == 0 ? std::size_t{0} : std::size_t{1});
      if (failure) QCOMPARE(observed.front().volumeName, std::string("Volume{independent}"));
    }
    QCOMPARE(fallback->reads, failure ? 1 : 0);
    QVERIFY(transport.invocations.empty());
  }
  void inventoryPreservesPhysicalIdentityAndClassifiesSupport() {
    test::native::RecordingWmiOperations transport;
    constexpr auto capacity = core::kMaxProtectedVolumeBytes;
    transport.queryResults.push_back({logicalDisk("C:", "ntfs", capacity, fixedLocalDriveType), logicalDisk("D:", "exFAT", capacity, fixedLocalDriveType),
                                      logicalDisk("E:", "NTFS", capacity + 1, fixedLocalDriveType), logicalDisk("F:", "FAT32", capacity, 2)});
    transport.queryResults.push_back({{{"DeviceID", Value::fromString(R"(\\?\Volume{c}\)")}, {"DriveLetter", Value::fromString("C:")}},
                                      {{"DeviceID", Value::fromString(R"(\\?\Volume{d}\)")}, {"DriveLetter", Value::fromString("D:")}},
                                      {{"DeviceID", Value::fromString(R"(\\?\Volume{hidden}\)")}, {"DriveLetter", Value{}}}});
    const auto inventory = windows::windowsVolumeRepository(transport)->read();
    QCOMPARE(inventory.size(), std::size_t{4});
    QCOMPARE(inventory[0].support, core::VolumeSupport::Supported);
    QCOMPARE(inventory[1].support, core::VolumeSupport::FileSystemLimited);
    QCOMPARE(inventory[2].support, core::VolumeSupport::ExceedsMaxSize);
    QCOMPARE(inventory[3].support, core::VolumeSupport::NotFixedLocalDisk);
    QCOMPARE(inventory[0].volumeName, std::string(R"(\\?\Volume{c}\)"));
    QCOMPARE(inventory[0].totalBytes, capacity);
    QCOMPARE(inventory[0].freeBytes, capacity / 2);
    QVERIFY(inventory[2].volumeName.empty());
    QVERIFY(transport.invocations.empty());
  }
  void ambiguousInventoryIsAReadFailure_data() {
    QTest::addColumn<bool>("duplicateLogicalDisk");
    QTest::newRow("duplicate-logical-drive") << true;
    QTest::newRow("duplicate-volume-mapping") << false;
  }
  void ambiguousInventoryIsAReadFailure() {
    QFETCH(bool, duplicateLogicalDisk);
    test::native::RecordingWmiOperations transport;
    const auto disk = logicalDisk("C:", "NTFS", 1024, fixedLocalDriveType);
    transport.queryResults.push_back(duplicateLogicalDisk ? std::vector{disk, disk} : std::vector{disk});
    windows::wmi::WmiRow mapping{{"DeviceID", Value::fromString(R"(\\?\Volume{c}\)")}, {"DriveLetter", Value::fromString("C:")}};
    transport.queryResults.push_back(duplicateLogicalDisk ? std::vector<windows::wmi::WmiRow>{} : std::vector{mapping, mapping});
    QVERIFY_THROWS_EXCEPTION(windows::wmi::WmiProtocolError, (void)windows::windowsVolumeRepository(transport)->read());
    QVERIFY(transport.invocations.empty());
  }
};
}  // namespace
QTEST_APPLESS_MAIN(WindowsVolumeRepositoryTests)
#include "WindowsVolumeRepositoryTests.moc"
