// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>

#include "app/ports/TextFileCreator.h"
#include "app/settings/UserConfig.h"

namespace uwf::presentation {

class UserConfigExportViewModel final {
 public:
  struct Result {
    app::TextFileCreationResult::Outcome outcome;
    QString path;
    QString error;
  };

  UserConfigExportViewModel(QString applicationDirectory, app::TextFileCreator creator);
  [[nodiscard]] Result exportConfig(const app::UserConfig& config) const;

 private:
  QString m_path;
  app::TextFileCreator m_creator;
};

}  // namespace uwf::presentation
