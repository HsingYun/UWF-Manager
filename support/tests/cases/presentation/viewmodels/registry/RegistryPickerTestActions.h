// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QSignalSpy>
#include <QTest>

#include "presentation/viewmodels/registry/RegistryPickerViewModel.h"

namespace uwf::test {
inline bool awaitRegistryRoots(presentation::RegistryPickerViewModel& model) {
  if (QTest::qWaitFor([&] { return model.rootsReady(); })) return true;
  QTest::qFail("Registry roots did not finish", __FILE__, __LINE__);
  return false;
}
inline QModelIndex navigateRegistry(presentation::RegistryPickerViewModel& model, const QString& path) {
  QSignalSpy finished(&model, &presentation::RegistryPickerViewModel::navigationFinished);
  model.navigateTo(path);
  if (!QTest::qWaitFor([&] { return !finished.isEmpty(); })) {
    QTest::qFail("Registry navigation did not finish", __FILE__, __LINE__);
    return {};
  }
  return finished.constLast().constFirst().value<QModelIndex>();
}
inline bool loadRegistryChildren(presentation::RegistryPickerViewModel& model, const QModelIndex& parent) {
  QSignalSpy failed(&model, &presentation::RegistryPickerViewModel::childrenLoadFailed);
  model.loadChildren(parent);
  if (!QTest::qWaitFor([&] { return !model.childrenLoading(parent); })) {
    QTest::qFail("Registry children did not finish", __FILE__, __LINE__);
    return false;
  }
  return failed.isEmpty();
}
inline void loadRegistryValues(presentation::RegistryPickerViewModel& model, const QString& key) {
  model.loadValues(key);
  if (!QTest::qWaitFor([&] { return !model.valuesLoading(); })) QTest::qFail("Registry values did not finish", __FILE__, __LINE__);
}
}  // namespace uwf::test
