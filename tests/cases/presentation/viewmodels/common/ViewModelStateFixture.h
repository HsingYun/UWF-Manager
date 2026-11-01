// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once
#include <stdexcept>

#include "application/state/contracts/ApplicationStateSource.h"
#include "support/app/ConfigurationFixture.h"

namespace uwf::test::viewmodels {
inline core::UwfSnapshot snapshot() {
  core::UwfSnapshot result;
  result.uwfAvailable = true;
  result.elevated = true;
  result.next.filter.enabled = true;
  result.next.overlay = {core::OverlayType::RAM, 8192, 4096, 6144};
  result.next.volumes = {{uwf::test::application::fixtureVolumeCName, "C:", true, uwf::core::VolumeBinding::DriveLetter, std::string("C:")},
                         {uwf::test::application::fixtureVolumeDName, "D:", false, uwf::core::VolumeBinding::DriveLetter, std::string("D:")}};
  result.current = result.next;
  result.runtime.romDisposition = 0;
  result.runtime.fullCommit = false;
  result.runtime.hormEnabled = false;
  result.runtime.currentOverlayFlags = 0;
  result.runtime.systemVolumeProtected = true;
  return result;
}

inline core::VolumeInfo disk(const std::string& letter = "C:") {
  return {letter,
          letter == "C:" ? uwf::test::application::fixtureVolumeCName : uwf::test::application::fixtureVolumeDName,
          "NTFS",
          "Data",
          512ULL << 30,
          256ULL << 30,
          core::VolumeSupport::Supported,
          {true, true}};
}

class FakeStateSource final : public app::ApplicationStateSource {
 public:
  app::ApplicationState next;
  bool fail = false;
  ReadOperation readOperation(uwf::app::UwfCapability) override {
    return [candidate = next, failed = fail](std::stop_token) {
      if (failed) throw std::runtime_error("snapshot failed");
      return candidate;
    };
  }
};
}  // namespace uwf::test::viewmodels
