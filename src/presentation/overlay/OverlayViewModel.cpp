// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "OverlayViewModel.h"

#include <QLocale>
#include <QPointer>
#include <exception>
#include <utility>

#include "presentation/common/PresentationText.h"
#include "presentation/common/PresentationTiming.h"
#include "shared/Log.h"
namespace uwf::presentation {
using namespace app;
QString overlayUsageToolTip(const app::OverlayUsageState& state) {
  if (const auto* enabled = std::get_if<OverlayUsageEnabled>(&state)) {
    const QLocale numbers(QLocale::English, QLocale::UnitedStates);
    const auto used = enabled->runtime.currentConsumptionMb;
    const auto total = static_cast<qulonglong>(core::overlayTotalMb(enabled->runtime));
    const auto percentage = total == 0 ? 0.0 : static_cast<double>(used) * 100.0 / static_cast<double>(total);
    return QStringLiteral("UWF Manager\n%1% (%2/%3 MB)").arg(QString::number(percentage, 'f', 0), numbers.toString(used), numbers.toString(total));
  }
  return QStringLiteral("UWF Manager\n") +
         (std::holds_alternative<OverlayUsageDisabled>(state) ? PresentationText::tr("UWF: Disabled") : PresentationText::tr("UWF status unavailable"));
}
OverlayViewModel::OverlayViewModel(std::shared_ptr<OverlayUsageReader> reader) : m_reader(std::move(reader)) {
  m_usageTimer.setInterval(timing::kOverlayUsageRefreshInterval);
  connect(&m_usageTimer, &QTimer::timeout, this, &OverlayViewModel::refreshUsage);
}
void OverlayViewModel::applySnapshot(const core::UwfSnapshot& snapshot) {
  m_read.cancel();
  const QPointer guard(this);
  m_usageRefreshFailed = false;
  if (snapshot.uwfAvailable) {
    m_hasCommittedSnapshot = true;
    m_usageTimer.start();
    if (snapshot.current.filter.enabled)
      publishUsageState(OverlayUsageEnabled{snapshot.runtime, snapshot.current.overlay});
    else
      publishUsageState(OverlayUsageDisabled{});
  } else {
    m_hasCommittedSnapshot = false;
    m_usageTimer.stop();
    publishUsageState(OverlayUsageUnavailable{});
  }
  if (guard) emit availabilityChanged();
}

void OverlayViewModel::refreshUsage() {
  // 增量刷新必须建立在一份完整快照上。构造期或首次读取失败时，禁止只凭
  // Filter/Overlay 三个运行时字段提前改变 Hub/托盘，避免不同 UI 消费者对
  // “是否已有可信状态”得出相反结论。
  if (!m_hasCommittedSnapshot || refreshing()) return;
  m_read.start([reader = m_reader](std::stop_token) { return reader->read(); },
               [this](AsyncResult<OverlayUsageState> result) {
                 if (const auto* failure = std::get_if<std::exception_ptr>(&result)) {
                   std::string detail;
                   try {
                     std::rethrow_exception(*failure);
                   } catch (const std::exception& error) {
                     detail = error.what();
                   } catch (...) {
                     detail = "unknown overlay read failure";
                   }
                   // 周期刷新失败可能持续多个 tick；同一故障周期只发一条 Warning，后续重复
                   // 仅保留在 Debug，避免 Release 日志每五秒刷屏。
                   if (!m_usageRefreshFailed)
                     UWF_LOG_W("hub") << "usage refresh failed: committedState=retained error=" << detail;
                   else
                     UWF_LOG_D("hub") << "usage refresh still failing: committedState=retained error=" << detail;
                   m_usageRefreshFailed = true;
                   return;
                 }

                 if (m_usageRefreshFailed) UWF_LOG_I("hub") << "usage refresh recovered";
                 m_usageRefreshFailed = false;

                 // 候选读取全部成功后一次性提交到所有消费者，Hub、主面板与托盘不会看见
                 // 来自不同轮次的混合状态。
                 const QPointer guard(this);
                 publishUsageState(std::get<OverlayUsageState>(std::move(result)));
                 if (guard) emit availabilityChanged();
               });
}
void OverlayViewModel::publishUsageState(const OverlayUsageState& state) {
  m_usage = state;
  emit usageChanged(m_usage);
}
}  // namespace uwf::presentation

#include "moc_OverlayViewModel.cpp"
