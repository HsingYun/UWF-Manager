// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "UserConfigExportViewModel.h"

#include <stdexcept>
#include <utility>

namespace uwf::presentation {

UserConfigExportViewModel::UserConfigExportViewModel(QString applicationDirectory, app::TextFileCreator creator)
    : m_path(app::userConfigPath(applicationDirectory)), m_creator(std::move(creator)) {
  if (!m_creator) throw std::invalid_argument("UserConfigExportViewModel requires a file creator");
}

UserConfigExportViewModel::Result UserConfigExportViewModel::exportConfig(const app::UserConfig& config) const {
  try {
    const auto result = m_creator(m_path, app::serializeUserConfig(config));
    return {result.outcome, m_path, result.error};
  } catch (const std::exception& error) {
    return {app::TextFileCreationResult::Outcome::Failed, m_path, QString::fromUtf8(error.what())};
  } catch (...) {
    return {app::TextFileCreationResult::Outcome::Failed, m_path, {}};
  }
}

}  // namespace uwf::presentation
