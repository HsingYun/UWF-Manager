// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#pragma once

#include <QString>
#include <cstdint>
#include <exception>
#include <optional>
#include <type_traits>
#include <utility>

#include "app/api/CommitResult.h"
#include "app/common/ApplicationText.h"
#include "app/wmi/WmiException.h"
#include "shared/Log.h"

namespace uwf::app {

namespace detail {

struct ExistenceObservation {
  std::optional<bool> value;
  QString failure;
};

template <typename ExistsFn, typename Target>
[[nodiscard]] ExistenceObservation observeExistence(ExistsFn& existsFn, const Target& target) {
  try {
    return {.value = existsFn(target), .failure = {}};
  } catch (const std::exception& error) {
    return {.value = std::nullopt, .failure = QString::fromUtf8(error.what())};
  } catch (...) {
    return {.value = std::nullopt, .failure = ApplicationText::tr("Unknown error while reading the target state.")};
  }
}

}  // namespace detail

template <typename Target, typename CommitFn, typename ExistsFn = decltype(nullptr)>
[[nodiscard]] CommitTargetResult executeCommitTarget(const Target& target, const QString& display, CommitFn& commitOne, ExistsFn& existsFn) {
  constexpr bool kHasExists = !std::is_same_v<ExistsFn, decltype(nullptr)>;
  std::optional<bool> existedBefore;
  std::optional<bool> existsAfter;
  if constexpr (kHasExists) {
    const auto observation = detail::observeExistence(existsFn, target);
    existedBefore = observation.value;
    if (!observation.value) {
      UWF_LOG_W("commit") << "target state unavailable before operation; continuing with provider call: target=" << display.toStdString()
                          << " error=" << observation.failure.toStdString();
    } else if (!*observation.value) {
      return {.existedBefore = false, .existsAfter = false, .targetAlreadyAbsent = true};
    }
  }
  int32_t hresult = 0;
  uint32_t returnValue = 0;
  QString failureDetail = {};
  QString authoritativeFailure = {};
  bool succeeded = false;
  bool mayHaveTakenEffect = false;
  try {
    commitOne(target);
    succeeded = true;
  } catch (const wmi::WmiProviderError& error) {
    returnValue = error.returnValue();
    failureDetail = QString::fromUtf8(error.what());
  } catch (const wmi::WmiWriteOutcomeError& error) {
    if (error.code().category() == wmi::wmiErrorCategory()) hresult = static_cast<int32_t>(error.code().value());
    mayHaveTakenEffect = true;
    failureDetail = QString::fromUtf8(error.what());
  } catch (const wmi::WmiException& error) {
    if (error.code().category() == wmi::wmiErrorCategory()) hresult = static_cast<int32_t>(error.code().value());
    failureDetail = QString::fromUtf8(error.what());
  } catch (const std::exception& error) {
    failureDetail = QString::fromUtf8(error.what());
  } catch (...) {
    failureDetail = ApplicationText::tr("The operation failed with an unknown error.");
  }
  if constexpr (kHasExists) {
    const auto observation = detail::observeExistence(existsFn, target);
    existsAfter = observation.value;
    if (!observation.value) {
      // 写操作的成功契约包括权威终态重读；provider 接受请求不等于
      // 删除已被确认。重读失败时不重放写入，但必须如实报告“未能确认”；
      // 这与调用过程本身不确定的处理使用同一个收敛规则。
      if (succeeded || mayHaveTakenEffect) {
        succeeded = false;
        authoritativeFailure =
            ApplicationText::tr("The operation result could not be confirmed because the target state reread failed: %1").arg(observation.failure);
      }
    } else if (mayHaveTakenEffect && !*observation.value) {
      // 连接在调用过程中断开时，调用是否落地不可知；目标已不存在是删除
      // 业务真正关心的权威终态，因此它可以确认“不确定”，但绝不覆盖
      // provider 明确拒绝的结果。
      succeeded = true;
    } else if (succeeded && *observation.value) {
      succeeded = false;
      authoritativeFailure = ApplicationText::tr("The provider accepted the deletion, but the target still exists after the authoritative reread.");
    }
  }

  return {existedBefore, existsAfter, false, hresult, returnValue, std::move(failureDetail), std::move(authoritativeFailure), succeeded};
}

}  // namespace uwf::app
