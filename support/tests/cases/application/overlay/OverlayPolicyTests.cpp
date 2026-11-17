// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QStringList>
#include <QTest>
#include <functional>
#include <map>
#include <memory>
#include <type_traits>
#include <utility>

#include "application/configuration/execution/ConfigurationExecutor.h"
#include "application/overlay/policy/OverlayPolicyRepository.h"
#include "application/overlay/policy/OverlayPolicyService.h"
#include "application/state/ApplicationStateSession.h"
#include "application/uwf/adapters/UwfOverlayOperations.h"
#include "foundation/logging/Log.h"

namespace {
using namespace uwf::app;

class MemoryOverlayOperations final : public OverlayPolicyOperations {
 public:
  bool loaded = true;
  std::map<OverlayProperty, std::int64_t> values{{OverlayProperty::CurrentFilterEnabled, 0}, {OverlayProperty::CurrentOverlayType, 1},
                                                 {OverlayProperty::CurrentOverlayFlags, 0},  {OverlayProperty::NextFilterEnabled, 1},
                                                 {OverlayProperty::NextOverlayType, 1},      {OverlayProperty::NextOverlayFlags, 0},
                                                 {OverlayProperty::ResetPersistentState, 0}};
  std::map<OverlayProperty, OverlayPolicyFailure> observationFailures;
  std::vector<std::pair<OverlayProperty, std::int64_t>> writes;
  std::vector<std::pair<OverlayOption, bool>> optionWrites;
  std::function<void(OverlayProperty, std::int64_t)> onWrite;
  std::optional<WriteEffect> returnedEffect;
  OverlayPolicySnapshot snapshot() const override {
    OverlayPolicySnapshot result;
    result.transportAvailable = loaded;
    if (!loaded) return result;
    const auto field = [&]<class T>(const OverlayProperty node) -> PolicyObservation<T> {
      if (const auto failed = observationFailures.find(node); failed != observationFailures.end())
        return {.status = failed->second.kind == OperationFailureKind::Unsupported ? PolicyObservationStatus::Unsupported : PolicyObservationStatus::Failed,
                .failure = failed->second};
      const auto value = values.at(node);
      if constexpr (std::is_same_v<T, OverlayFlags>)
        return {.status = PolicyObservationStatus::Available, .value = OverlayFlags{static_cast<std::uint32_t>(value)}};
      else
        return {.status = PolicyObservationStatus::Available, .value = static_cast<T>(value)};
    };
    result.current.filterEnabled = field.template operator()<bool>(OverlayProperty::CurrentFilterEnabled);
    result.next.filterEnabled = field.template operator()<bool>(OverlayProperty::NextFilterEnabled);
    result.current.overlayType = field.template operator()<std::int64_t>(OverlayProperty::CurrentOverlayType);
    result.next.overlayType = field.template operator()<std::int64_t>(OverlayProperty::NextOverlayType);
    result.current.flags = field.template operator()<OverlayFlags>(OverlayProperty::CurrentOverlayFlags);
    result.next.flags = field.template operator()<OverlayFlags>(OverlayProperty::NextOverlayFlags);
    result.resetPersistentState = field.template operator()<std::int64_t>(OverlayProperty::ResetPersistentState);
    return result;
  }
  WriteEffect setOption(const OverlayOption option, const bool enabled) override {
    optionWrites.emplace_back(option, enabled);
    const auto mask = option == OverlayOption::Passthrough  ? OverlayFlags::Passthrough
                      : option == OverlayOption::Persistent ? OverlayFlags::Persistent
                                                            : OverlayFlags::ReadOnlyMedia;
    const auto prior = static_cast<std::uint32_t>(values.at(OverlayProperty::NextOverlayFlags));
    return replace(OverlayProperty::NextOverlayFlags, enabled ? prior | mask : prior & ~mask);
  }
  WriteEffect replace(OverlayProperty node, std::int64_t value) override {
    const auto previous = values.at(node);
    writes.emplace_back(node, value);
    if (onWrite)
      onWrite(node, value);
    else
      values.at(node) = value;
    return returnedEffect.value_or(previous == value ? WriteEffect::None : WriteEffect::Confirmed);
  }
};

class EffectUwf final : public uwf::compat::Uwf {
 public:
  uwf::compat::Effect effect = uwf::compat::Effect::None;
  mutable std::vector<uwf::compat::Request> requests;
  uwf::compat::Capability probe(const uwf::compat::Request&) const override { return {uwf::compat::Support::Supported, {}}; }
  uwf::compat::Response execute(const uwf::compat::Request& request, const uwf::compat::Context&) const override {
    requests.push_back(request);
    return uwf::compat::Response::success(std::monostate{}, effect);
  }
};

QStringList logRecords(const char level) {
  QStringList result;
  const auto marker = std::string(" ") + level + ' ';
  for (const auto& line : uwf::recentLogLines())
    if (line.find(marker) < line.find(']')) result.append(QString::fromStdString(line));
  return result;
}

OverlayPolicyFailure diagnosticFailure(const OverlayProperty node) {
  return {.origin = OverlayPolicyFailureOrigin::Provider,
          .kind = OperationFailureKind::AccessDenied,
          .node = node,
          .operation = "Replace",
          .message = "provider rejected the change\n节点错误详情",
          .diagnostics = "backend=CSP code=0x80070005\nCOM diagnostic 完整 UTF-8 原始详情\n<wap-provisioningdoc>拒绝写入</wap-provisioningdoc>",
          .effect = WriteEffect::None};
}

class OverlayPolicyTests final : public QObject {
  Q_OBJECT
 private slots:
  void compatibilityEffectsCrossTheOverlayPortUnchanged_data() {
    QTest::addColumn<int>("effect");
    QTest::addColumn<bool>("option");
    for (const auto effect : {uwf::compat::Effect::None, uwf::compat::Effect::Confirmed, uwf::compat::Effect::Uncertain})
      for (const bool option : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("effect-%1-option-%2").arg(static_cast<int>(effect)).arg(option))) << static_cast<int>(effect) << option;
  }
  void compatibilityEffectsCrossTheOverlayPortUnchanged() {
    QFETCH(int, effect);
    QFETCH(bool, option);
    const auto system = std::make_shared<EffectUwf>();
    system->effect = static_cast<uwf::compat::Effect>(effect);
    UwfOverlayOperations operations(system);
    const auto invoke = [&] {
      return option ? operations.setOption(OverlayOption::Passthrough, true) : operations.replace(OverlayProperty::ResetPersistentState, 1);
    };
    if (system->effect == uwf::compat::Effect::Uncertain) {
      try {
        (void)invoke();
        QFAIL("An uncertain compatibility outcome must fail");
      } catch (const OverlayPolicyError& error) {
        QCOMPARE(error.writeEffect(), WriteEffect::Uncertain);
        QCOMPARE(error.failure().kind, OperationFailureKind::Failed);
      }
    } else {
      QCOMPARE(invoke(), system->effect == uwf::compat::Effect::None ? WriteEffect::None : WriteEffect::Confirmed);
    }
    QCOMPARE(system->requests.size(), std::size_t{1});
    QVERIFY(uwf::compat::isWrite(system->requests.front()));
  }

  void unsupportedObservationsUseSemanticKindsOnly() {
    for (const bool unsupported : {false, true}) {
      const auto port = std::make_shared<MemoryOverlayOperations>();
      port->observationFailures[OverlayProperty::ResetPersistentState] = {
          .origin = OverlayPolicyFailureOrigin::Provider,
          .kind = unsupported ? OperationFailureKind::Unsupported : OperationFailureKind::AccessDenied,
          .message = "Original backend message",
          // Contradict the semantic kind to verify diagnostics cannot reclassify it.
          .diagnostics = unsupported ? "opaque code=0x80070005" : "opaque code=0x80004001"};
      const auto state = OverlayPolicyService(port).snapshot();
      QCOMPARE(state.resetPersistentState.status, unsupported ? PolicyObservationStatus::Unsupported : PolicyObservationStatus::Failed);
      QCOMPARE(state.resetPersistentState.failure->diagnostics, port->observationFailures.begin()->second.diagnostics);
    }
  }

  void commandReceiptDoesNotRequireASecondObservation_data() {
    QTest::addColumn<int>("effect");
    QTest::addColumn<bool>("matches");
    for (const auto effect : {WriteEffect::None, WriteEffect::Confirmed, WriteEffect::Uncertain})
      for (const bool matches : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("effect-%1-matches-%2").arg(static_cast<int>(effect)).arg(matches))) << static_cast<int>(effect) << matches;
  }
  void commandReceiptDoesNotRequireASecondObservation() {
    QFETCH(int, effect);
    QFETCH(bool, matches);
    const auto confirmed = static_cast<WriteEffect>(effect);
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::NextOverlayFlags] = matches && confirmed == WriteEffect::None ? OverlayFlags::Passthrough : 0;
    port->returnedEffect = confirmed;
    port->onWrite = [&](const auto node, const auto value) {
      if (matches) port->values[node] = value;
    };
    OverlayPolicyService service(port);
    const auto result = service.apply({.changes = {.passthrough = true}});
    const bool success = confirmed != WriteEffect::Uncertain;
    QCOMPARE(result.succeeded(), success);
    QCOMPARE(result.effect, confirmed);
    QCOMPARE(result.steps.front().outcome, confirmed == WriteEffect::None        ? PolicyStepOutcome::Unchanged
                                           : confirmed == WriteEffect::Confirmed ? PolicyStepOutcome::Confirmed
                                                                                 : PolicyStepOutcome::Uncertain);
    QCOMPARE(port->optionWrites.size(), std::size_t{1});

    if (success) QVERIFY(result.snapshot.next.flags.value->passthrough());
    if (!success) QCOMPARE(result.failure->effect, result.effect);
  }

  void failedPolicyOperationsLogOneCompleteError_data() {
    QTest::addColumn<int>("kind");
    QTest::newRow("provider-rejected") << 0;
    QTest::newRow("transport-uncertain") << 1;
    QTest::newRow("matching-readback-after-api-error") << 2;
    QTest::newRow("partially-applied") << 3;
    QTest::newRow("required-read-failed") << 4;
  }
  void failedPolicyOperationsLogOneCompleteError() {
    QFETCH(int, kind);
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::CurrentOverlayFlags] = port->values[OverlayProperty::NextOverlayFlags] = OverlayFlags::Persistent;
    auto detail = diagnosticFailure(kind == 3 ? OverlayProperty::ResetPersistentState : OverlayProperty::NextOverlayFlags);
    if (kind == 1 || kind == 2) {
      detail.origin = OverlayPolicyFailureOrigin::Transport;
      detail.kind = OperationFailureKind::Unavailable;
      detail.diagnostics = "backend=CSP code=0x80004005\nlost transport reply";
      detail.effect = WriteEffect::Uncertain;
    }
    if (kind == 4) {
      detail.operation = "Get";
      port->observationFailures[detail.node] = detail;
    } else {
      port->onWrite = [&](const OverlayProperty node, const std::int64_t value) {
        if (node != detail.node || kind == 2) port->values[node] = value;
        if (node == detail.node) throw OverlayPolicyError(detail);
      };
    }
    OverlayPolicyUpdate update;
    update.changes.passthrough = true;
    if (kind == 3) update.changes.reset = PersistentReset::Clear;
    uwf::app::OverlayPolicyService policy(port);
    uwf::clearLogLines();
    const auto result = policy.apply(update);
    QVERIFY(!result.succeeded());
    const auto errors = logRecords('E');
    QCOMPARE(errors.size(), 1);
    const auto& record = errors.front();
    QVERIFY(record.contains(QString::fromStdString(detail.message)));
    QVERIFY(record.contains(QString::fromStdString(detail.diagnostics)));
    QVERIFY(record.contains(kind == 1 || kind == 2 ? QStringLiteral("code=0x80004005") : QStringLiteral("code=0x80070005")));
    QVERIFY(record.contains(QString::fromStdString(std::string(overlayPropertyName(detail.node)))));
    if (kind == 4) {
      QVERIFY(port->writes.empty());
      QVERIFY(record.contains(QStringLiteral("writeAttempted=false")));
    } else {
      QVERIFY(record.contains(QStringLiteral("writeAttempted=true")));
      QVERIFY(record.contains(QStringLiteral("before=0 requested=1")));
    }
    const auto effect = kind == 1 || kind == 2 ? WriteEffect::Uncertain : kind == 3 ? WriteEffect::Confirmed : WriteEffect::None;
    QCOMPARE(result.effect, effect);
    QVERIFY(record.contains(effect == WriteEffect::Uncertain   ? QStringLiteral("effect=uncertain")
                            : effect == WriteEffect::Confirmed ? QStringLiteral("effect=confirmed")
                                                               : QStringLiteral("effect=none")));
    QCOMPARE(port->writes.size(), kind == 4 ? std::size_t{0} : kind == 3 ? std::size_t{2} : std::size_t{1});
  }
  void policyAndBaselineBlocksAreWarningsWithoutWrites() {
    for (const bool stale : {false, true}) {
      const auto port = std::make_shared<MemoryOverlayOperations>();
      port->values[OverlayProperty::CurrentFilterEnabled] = stale ? 0 : 1;
      OverlayPolicyUpdate update;
      update.changes.passthrough = true;
      if (stale) update.expectedFlags = 0x80;
      uwf::app::OverlayPolicyService policy(port);
      uwf::clearLogLines();
      const auto result = policy.apply(update);
      QVERIFY(!result.succeeded());
      QVERIFY(port->writes.empty());
      QVERIFY(logRecords('E').isEmpty());
      QCOMPARE(logRecords('W').size(), 1);
      QVERIFY(logRecords('W').constFirst().contains(QStringLiteral("apply blocked:")));
    }
  }
  void configurationExecutionDoesNotRepeatThePolicyError() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    const auto detail = diagnosticFailure(OverlayProperty::NextOverlayFlags);
    port->onWrite = [detail](OverlayProperty, std::int64_t) { throw OverlayPolicyError(detail); };
    const auto policy = std::make_shared<uwf::app::OverlayPolicyService>(port);
    uwf::core::UwfSnapshot settings;
    settings.uwfAvailable = settings.elevated = true;
    settings.next.overlay = {.type = uwf::core::OverlayType::Disk, .maximumSizeMb = 2048, .warningThresholdMb = 512, .criticalThresholdMb = 1024};
    ConfigurationChanges changes;
    changes.overlayPolicy.passthrough = true;
    const auto plan = planApplicationConfiguration(changes, settings, policy->snapshot());
    uwf::clearLogLines();
    const auto result = ConfigurationExecutor(overlayPolicyRepository(policy)).apply(plan);
    QVERIFY(result.failed());
    QCOMPARE(logRecords('E').size(), 1);
    QCOMPARE(std::ranges::count_if(result.steps, [](const auto& step) { return step.policyFailure.has_value(); }), std::ptrdiff_t{1});
    QCOMPARE(result.steps.front().policyFailure->diagnostics, detail.diagnostics);
    QCOMPARE(result.steps.front().policyFailure->kind, detail.kind);
    for (const auto& step : result.steps) {
      QVERIFY(step.description.find("<wap-provisioningdoc>") == std::string::npos);
      QVERIFY(step.description.find("backend=CSP") == std::string::npos);
    }
  }
  void providerNormalizationOfUneditedFlagsIsAnObservation() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::NextOverlayFlags] = 0x80;
    port->onWrite = [&](OverlayProperty node, std::int64_t value) { port->values[node] = value | 0x101; };
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update;
    update.changes.persistent = true;
    update.expectedFlags = 0x80;
    const auto result = service.apply(update);
    QVERIFY(result.succeeded());
    QCOMPARE(result.snapshot.next.flags.value->raw, 0x82u);

    QCOMPARE(service.snapshot().next.flags.value->raw, 0x183u);
    QCOMPARE(result.steps.front().requested, 1);
    QCOMPARE(result.steps.front().observed, std::optional<std::int64_t>{1});
    QCOMPARE(port->writes.size(), std::size_t{1});
  }
  void configurationReceiptsUseOnlyTheirWriteOutcomes_data() {
    QTest::addColumn<int>("boundary");
    QTest::newRow("first-step-rejected") << 0;
    QTest::newRow("later-step-uncertain") << 1;
    QTest::newRow("final-readback-failed") << 2;
  }
  void configurationReceiptsUseOnlyTheirWriteOutcomes() {
    QFETCH(int, boundary);
    const auto port = std::make_shared<MemoryOverlayOperations>();
    const auto policy = std::make_shared<OverlayPolicyService>(port);
    const auto baseline = policy->snapshot();
    int writes = 0;
    port->onWrite = [&, boundary](const OverlayProperty node, const std::int64_t value) {
      ++writes;
      if (boundary == 0 || (boundary == 1 && writes == 2)) {
        auto failure = diagnosticFailure(node);
        failure.effect = boundary == 0 ? WriteEffect::None : WriteEffect::Uncertain;
        throw OverlayPolicyError(failure);
      }
      port->values[node] = value;
      if (boundary == 2 && writes == 2) port->observationFailures[node] = diagnosticFailure(node);
    };
    const OverlayPolicyDelta changes{.passthrough = true, .persistent = true};
    const auto repository = overlayPolicyRepository(policy);
    const auto result = repository->prepareWrite(ConfigurationOverlayPolicyPhase{changes}, {})(baseline);
    QCOMPARE(result.steps.size(), std::size_t{2});
    QCOMPARE(result.succeededOperations(), boundary == 0 ? 0 : boundary == 1 ? 1 : 2);
    QCOMPARE(result.failedOperations(), boundary == 2 ? 0 : 1);
    QCOMPARE(result.notAttemptedOperations(), boundary == 0 ? 1 : 0);
    QCOMPARE(result.dependencySkippedOperations(), 0);
    QCOMPARE(result.batchFailures(), 0);
    QCOMPARE(result.failed(), boundary != 2);
    QCOMPARE(result.anyWriteConfirmed(), boundary != 0);
    QCOMPARE(result.reconciliationRequired(), boundary != 0);
    QCOMPARE(result.writeEffect(), boundary == 0 ? WriteEffect::None : boundary == 1 ? WriteEffect::Uncertain : WriteEffect::Confirmed);
    if (boundary == 2) {
      QCOMPARE(result.steps[0].outcome, ConfigurationStepOutcome::Succeeded);
      QCOMPARE(result.steps[1].outcome, ConfigurationStepOutcome::Succeeded);
    }
  }
  void successfulReceiptNeedsNoFinalFlagObservation() {
    uwf::clearLogLines();
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->onWrite = [&](OverlayProperty node, std::int64_t) { port->values[node] = 0x80; };
    uwf::app::OverlayPolicyService service(port);
    const auto result = service.apply({.changes = {.persistent = true}});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, WriteEffect::Confirmed);
    QCOMPARE(result.steps.front().observed, std::optional<std::int64_t>{1});
    QCOMPARE(result.steps.front().outcome, PolicyStepOutcome::Confirmed);
    QCOMPARE(result.snapshot.next.flags.value->raw, OverlayFlags::Persistent);
    QCOMPARE(port->writes.size(), std::size_t{1});

    QVERIFY(logRecords('E').isEmpty());
    QCOMPARE(service.snapshot().next.flags.value->raw, std::uint32_t{0x80});
  }
  void malformedLaterObservationCannotEraseASuccessfulReceipt() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->onWrite = [&](OverlayProperty node, std::int64_t) {
      port->observationFailures[node] = {.origin = OverlayPolicyFailureOrigin::Protocol,
                                         .kind = OperationFailureKind::Protocol,
                                         .node = node,
                                         .message = "Published observation is malformed"};
    };
    uwf::app::OverlayPolicyService service(port);
    const auto result = service.apply({.changes = {.passthrough = true}});
    QVERIFY(result.succeeded());
    QCOMPARE(result.steps.front().outcome, PolicyStepOutcome::Confirmed);
    QCOMPARE(result.effect, WriteEffect::Confirmed);
    QCOMPARE(result.snapshot.next.flags.value->raw, OverlayFlags::Passthrough);

    QCOMPARE(service.snapshot().next.flags.status, PolicyObservationStatus::Failed);
  }
  void unavailableTransportIsDistinctFromUnreadableNodes() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->loaded = false;
    uwf::app::OverlayPolicyService service(port);
    const auto missing = service.snapshot();
    QVERIFY(!missing.transportAvailable);

    QCOMPARE(missing.next.flags.status, PolicyObservationStatus::Unobserved);
    port->loaded = true;
    port->observationFailures[OverlayProperty::NextOverlayFlags] = {
        .origin = OverlayPolicyFailureOrigin::Provider, .kind = OperationFailureKind::AccessDenied, .message = "Denied"};
    const auto unreadable = service.snapshot();
    QVERIFY(unreadable.transportAvailable);
    QCOMPARE(unreadable.next.flags.status, PolicyObservationStatus::Failed);
    QVERIFY(unreadable.current.flags.value.has_value());
  }
  void independentObservationsAndCoreComposition() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::NextOverlayFlags] = 0x80000001;
    port->observationFailures[OverlayProperty::ResetPersistentState] = {
        .origin = OverlayPolicyFailureOrigin::Provider, .kind = OperationFailureKind::AccessDenied, .message = "Access denied"};
    uwf::app::OverlayPolicyService service(port);
    const auto state = service.snapshot();
    QCOMPARE(state.next.flags.value->raw, std::uint32_t{0x80000001});
    QCOMPARE(state.resetPersistentState.status, PolicyObservationStatus::Failed);
    QVERIFY(!state.resetPersistentState.value);
    QVERIFY(port->writes.empty());
    ApplicationState application;
    application.snapshot.uwfAvailable = true;
    application.snapshot.elevated = true;
    application.overlayPolicy = state;
    ApplicationStateSession session;
    session.commit(application);
    QVERIFY(session.configurationWritesAllowed());
    QCOMPARE(session.overlayPolicy().resetPersistentState.status, PolicyObservationStatus::Failed);
    port->observationFailures[OverlayProperty::ResetPersistentState].kind = OperationFailureKind::Unsupported;
    QCOMPARE(service.snapshot().resetPersistentState.status, PolicyObservationStatus::Unsupported);
    port->observationFailures[OverlayProperty::ResetPersistentState].origin = OverlayPolicyFailureOrigin::Transport;
    QCOMPARE(service.snapshot().resetPersistentState.status, PolicyObservationStatus::Unsupported);
    port->observationFailures[OverlayProperty::ResetPersistentState].kind = OperationFailureKind::Unavailable;
    QCOMPARE(service.snapshot().resetPersistentState.status, PolicyObservationStatus::Failed);
  }

  void preservesAllUneditedFlagBits() {
    for (const std::uint32_t initial : {0u, 1u, 2u, 3u, 0x40u, 0x80000000u, 0xFFFFFFFBu}) {
      for (const bool enable : {false, true}) {
        const auto port = std::make_shared<MemoryOverlayOperations>();
        port->values[OverlayProperty::NextOverlayFlags] = initial;
        uwf::app::OverlayPolicyService service(port);
        OverlayPolicyUpdate update;
        update.changes.passthrough = enable;
        update.expectedFlags = initial;
        const auto result = service.apply(update);
        QVERIFY(result.succeeded());
        const auto actual = result.snapshot.next.flags.value->raw;
        QCOMPARE(actual & ~OverlayFlags::Passthrough, initial & ~OverlayFlags::Passthrough);
        QCOMPARE((actual & OverlayFlags::Passthrough) != 0, enable);
        QCOMPARE(port->writes.size(), std::size_t{1});
        QCOMPARE(result.effect, initial == actual ? WriteEffect::None : WriteEffect::Confirmed);
      }
    }
  }

  void optionsUseSeparateDomainCommandsAndObserveOpaqueProviderChanges() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::NextOverlayType] = 0;
    port->values[OverlayProperty::NextOverlayFlags] = 0x82;
    port->onWrite = [&](const OverlayProperty node, const std::int64_t value) { port->values[node] = value | 0x100; };
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update{.changes = {.passthrough = true, .persistent = false, .readOnlyMedia = true}, .expectedFlags = 0x82};
    const auto result = service.apply(update);
    QVERIFY(result.succeeded());
    QCOMPARE(port->optionWrites, (std::vector<std::pair<OverlayOption, bool>>{
                                     {OverlayOption::Passthrough, true}, {OverlayOption::Persistent, false}, {OverlayOption::ReadOnlyMedia, true}}));
    QCOMPARE(result.snapshot.next.flags.value->raw, std::uint32_t{0x85});

    QCOMPARE(service.snapshot().next.flags.value->raw, std::uint32_t{0x185});
    QCOMPARE(result.steps.size(), std::size_t{3});
    QCOMPARE(result.steps[0].option, std::optional{OverlayOption::Passthrough});
    QCOMPARE(result.steps[1].option, std::optional{OverlayOption::Persistent});
    QCOMPARE(result.steps[2].option, std::optional{OverlayOption::ReadOnlyMedia});
  }

  void ramPassthroughAndDiskPersistentAreDifferent() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::NextOverlayType] = 0;
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update;
    update.changes.passthrough = true;
    QVERIFY(service.apply(update).succeeded());
    update.changes.persistent = true;
    const auto rejected = service.apply(update);
    QVERIFY(rejected.failure);
    QCOMPARE(rejected.failure->origin, OverlayPolicyFailureOrigin::Policy);
    QCOMPARE(port->writes.size(), std::size_t{1});
    port->values[OverlayProperty::NextOverlayType] = 1;
    QVERIFY(service.apply(update).succeeded());
    QCOMPARE(port->values[OverlayProperty::NextOverlayFlags], 3);
  }

  void flagChangesUseCurrentSessionPreconditions_data() {
    QTest::addColumn<bool>("currentEnabled");
    QTest::addColumn<bool>("nextEnabled");
    QTest::addColumn<int>("nextType");
    QTest::addColumn<std::uint32_t>("initialFlags");
    QTest::addColumn<bool>("persistent");
    QTest::addColumn<bool>("allowed");
    QTest::newRow("on-disk-passthrough") << true << true << 1 << std::uint32_t{1} << false << false;
    QTest::newRow("on-disk-persistent") << true << true << 1 << std::uint32_t{1} << true << false;
    QTest::newRow("on-next-off-passthrough") << true << false << 1 << std::uint32_t{1} << false << false;
    QTest::newRow("on-next-off-persistent") << true << false << 1 << std::uint32_t{1} << true << false;
    QTest::newRow("off-next-off-passthrough") << false << false << 1 << std::uint32_t{1} << false << true;
    QTest::newRow("off-next-off-persistent") << false << false << 1 << std::uint32_t{1} << true << true;
    QTest::newRow("off-next-on-disk-passthrough") << false << true << 1 << std::uint32_t{1} << false << true;
    QTest::newRow("off-next-on-disk-persistent") << false << true << 1 << std::uint32_t{1} << true << true;
    QTest::newRow("off-next-on-ram-passthrough") << false << true << 0 << std::uint32_t{0} << false << true;
    QTest::newRow("off-next-on-ram-persistent") << false << true << 0 << std::uint32_t{0} << true << false;
  }
  void flagChangesUseCurrentSessionPreconditions() {
    QFETCH(bool, currentEnabled);
    QFETCH(bool, nextEnabled);
    QFETCH(int, nextType);
    QFETCH(std::uint32_t, initialFlags);
    QFETCH(bool, persistent);
    QFETCH(bool, allowed);
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::CurrentFilterEnabled] = currentEnabled;
    port->values[OverlayProperty::NextFilterEnabled] = nextEnabled;
    port->values[OverlayProperty::NextOverlayType] = nextType;
    port->values[OverlayProperty::NextOverlayFlags] = initialFlags;
    uwf::app::OverlayPolicyService service(port);
    const auto mask = persistent ? OverlayFlags::Persistent : OverlayFlags::Passthrough;
    const bool original = (initialFlags & mask) != 0;
    OverlayPolicyUpdate update;
    if (persistent)
      update.changes.persistent = !original;
    else
      update.changes.passthrough = !original;
    update.expectedFlags = initialFlags;
    const auto result = service.apply(update);
    QCOMPARE(result.succeeded(), allowed);
    if (!allowed) {
      QVERIFY(result.failure);
      QCOMPARE(result.failure->origin, OverlayPolicyFailureOrigin::Policy);
      QCOMPARE(result.effect, WriteEffect::None);
      QVERIFY(result.steps.empty());
      QVERIFY(port->writes.empty());
      return;
    }
    QCOMPARE(port->writes, (std::vector<std::pair<OverlayProperty, std::int64_t>>{{OverlayProperty::NextOverlayFlags, initialFlags ^ mask}}));
    if (persistent)
      update.changes.persistent = original;
    else
      update.changes.passthrough = original;
    update.expectedFlags = initialFlags ^ mask;
    QVERIFY(service.apply(update).succeeded());
    QCOMPARE(port->values[OverlayProperty::NextOverlayFlags], std::int64_t{initialFlags});
    QCOMPARE(port->writes.size(), std::size_t{2});
  }

  void unreadableCurrentSessionPreventsFlagWrites() {
    for (const auto mask : {OverlayFlags::Passthrough, OverlayFlags::Persistent, OverlayFlags::ReadOnlyMedia}) {
      const auto port = std::make_shared<MemoryOverlayOperations>();
      port->observationFailures[OverlayProperty::CurrentFilterEnabled] = {.origin = OverlayPolicyFailureOrigin::Transport,
                                                                          .kind = OperationFailureKind::Unavailable,
                                                                          .node = OverlayProperty::CurrentFilterEnabled,
                                                                          .message = "Current session unavailable"};
      uwf::app::OverlayPolicyService service(port);
      OverlayPolicyUpdate update;
      if (mask == OverlayFlags::Persistent)
        update.changes.persistent = true;
      else if (mask == OverlayFlags::Passthrough)
        update.changes.passthrough = true;
      else {
        port->values[OverlayProperty::NextOverlayFlags] = OverlayFlags::ReadOnlyMedia;
        update.changes.readOnlyMedia = false;
      }
      const auto result = service.apply(update);
      QVERIFY(!result.succeeded());
      QCOMPARE(result.failure->node, OverlayProperty::CurrentFilterEnabled);
      QCOMPARE(result.failure->kind, OperationFailureKind::Unavailable);
      QCOMPARE(result.effect, WriteEffect::None);
      QVERIFY(port->writes.empty());
    }
  }

  void explicitUnchangedOptionsStillRequireAStoppedCurrentSession_data() {
    QTest::addColumn<bool>("persistent");
    QTest::addColumn<bool>("current");
    for (const bool persistent : {false, true})
      for (const bool current : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("persistent-%1-current-%2").arg(persistent).arg(current))) << persistent << current;
  }
  void explicitUnchangedOptionsStillRequireAStoppedCurrentSession() {
    QFETCH(bool, persistent);
    QFETCH(bool, current);
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::CurrentFilterEnabled] = current;
    port->values[OverlayProperty::NextOverlayFlags] = OverlayFlags::Passthrough | OverlayFlags::Persistent;
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update;
    if (persistent)
      update.changes.persistent = true;
    else
      update.changes.passthrough = true;
    const auto result = service.apply(update);
    QCOMPARE(result.succeeded(), !current);
    QCOMPARE(result.effect, WriteEffect::None);
    QCOMPARE(port->writes.size(), current ? std::size_t{0} : std::size_t{1});
    if (!current) QCOMPARE(result.steps.front().outcome, PolicyStepOutcome::Unchanged);
  }

  void romChangesInEitherDirectionRequireCurrentFilterOff() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update;
    update.changes.readOnlyMedia = true;
    QVERIFY(!service.apply(update).succeeded());  // Disk cannot enable ROM.
    port->values[OverlayProperty::NextOverlayType] = 0;
    port->values[OverlayProperty::CurrentFilterEnabled] = 1;
    QVERIFY(!service.apply(update).succeeded());
    port->values[OverlayProperty::CurrentFilterEnabled] = 0;
    QVERIFY(service.apply(update).succeeded());
    port->values[OverlayProperty::CurrentFilterEnabled] = 1;
    port->values[OverlayProperty::CurrentOverlayFlags] = 4;
    QVERIFY(service.snapshot().current.flags.value->readOnlyMedia());
    update.changes.readOnlyMedia = false;
    QVERIFY(!service.apply(update).succeeded());
    QCOMPARE(port->writes.size(), std::size_t{1});
    port->values[OverlayProperty::CurrentFilterEnabled] = 0;
    QVERIFY(service.apply(update).succeeded());
    QCOMPARE(port->values[OverlayProperty::NextOverlayFlags], 0);
    QCOMPARE(port->writes.size(), std::size_t{2});
    QVERIFY(service.snapshot().current.flags.value->readOnlyMedia());  // Next changes do not alter loaded Current flags.
  }

  void resetSelectionDoesNotInventOneShotSemanticsOrDiagnosticGuards() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->values[OverlayProperty::CurrentFilterEnabled] = 1;
    port->values[OverlayProperty::CurrentOverlayType] = 0;
    port->values[OverlayProperty::NextOverlayType] = 0;
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update;
    update.changes.reset = PersistentReset::UseSavedMode;
    QVERIFY(!service.apply(update).succeeded());
    QVERIFY(port->writes.empty());
    port->values[OverlayProperty::ResetPersistentState] = 255;
    QCOMPARE(service.snapshot().resetMode(), PersistentReset::UseSavedMode);
    QCOMPARE(service.snapshot().currentPersistentDisk(), false);
    update.changes.reset = PersistentReset::Clear;
    QVERIFY(service.apply(update).succeeded());
    QCOMPARE(service.snapshot().resetMode(), PersistentReset::Clear);
    QCOMPARE(port->values[OverlayProperty::ResetPersistentState], 1);
    port->values[OverlayProperty::ResetPersistentState] = 17;
    QVERIFY(!service.snapshot().resetMode());
  }

  void conflictsAndAllValidationPrecedeWrites() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update;
    update.changes.passthrough = true;
    update.expectedFlags = 1;
    QCOMPARE(service.apply(update).failure->origin, OverlayPolicyFailureOrigin::Conflict);
    QVERIFY(port->writes.empty());
    update.expectedFlags = 0;
    update.changes.reset = static_cast<PersistentReset>(2);
    QCOMPARE(service.apply(update).failure->origin, OverlayPolicyFailureOrigin::Policy);
    QVERIFY(port->writes.empty());
    update.changes.reset = PersistentReset::Clear;
    port->observationFailures[OverlayProperty::ResetPersistentState] = {.origin = OverlayPolicyFailureOrigin::Provider,
                                                                        .kind = OperationFailureKind::AccessDenied};
    QVERIFY(!service.apply(update).succeeded());
    QVERIFY(port->writes.empty());
  }

  void partialFailureRetainsEarlierEffectsAndNeverReplays() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->onWrite = [&](OverlayProperty node, std::int64_t value) {
      if (node == OverlayProperty::ResetPersistentState)
        throw OverlayPolicyError(
            {.origin = OverlayPolicyFailureOrigin::Provider, .kind = OperationFailureKind::AccessDenied, .node = node, .message = "denied"});
      port->values.at(node) = value;
    };
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update;
    update.changes.passthrough = true;
    update.changes.reset = PersistentReset::Clear;
    const auto result = service.apply(update);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.effect, WriteEffect::Confirmed);
    QVERIFY(result.reconciliationRequired());
    QCOMPARE(result.steps[0].outcome, PolicyStepOutcome::Confirmed);
    QCOMPARE(result.steps[1].outcome, PolicyStepOutcome::Failed);
    QCOMPARE(port->writes.size(), std::size_t{2});
    QCOMPARE(port->values[OverlayProperty::NextOverlayFlags], 1);  // No rollback.
    QCOMPARE(port->values[OverlayProperty::ResetPersistentState], 0);
  }

  void ambiguousWriteIsNeitherReadBackNorRetried() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    uwf::app::OverlayPolicyService service(port);
    port->onWrite = [&](OverlayProperty node, std::int64_t value) {
      port->values.at(node) = value;
      throw OverlayPolicyError({.origin = OverlayPolicyFailureOrigin::Transport,
                                .kind = OperationFailureKind::Unavailable,
                                .node = node,
                                .message = "lost reply",
                                .effect = WriteEffect::Uncertain});
    };
    const auto failed = service.apply({.changes = {.passthrough = true}});
    QVERIFY(!failed.succeeded());
    QCOMPARE(failed.effect, WriteEffect::Uncertain);
    QCOMPARE(failed.steps[0].outcome, PolicyStepOutcome::Uncertain);
    QVERIFY(!failed.steps[0].observed);
    QVERIFY(!failed.snapshot.next.flags.value);
    QCOMPARE(port->writes.size(), std::size_t{1});
  }
  void failedSetterDoesNotReadOrUpgradeWriteEvidence_data() {
    QTest::addColumn<int>("writeEffect");
    QTest::addColumn<bool>("matches");
    for (const auto effect : {WriteEffect::None, WriteEffect::Confirmed, WriteEffect::Uncertain})
      for (const bool matches : {false, true})
        QTest::newRow(qUtf8Printable(QStringLiteral("effect-%1-match-%2").arg(static_cast<int>(effect)).arg(matches))) << static_cast<int>(effect) << matches;
  }
  void failedSetterDoesNotReadOrUpgradeWriteEvidence() {
    QFETCH(int, writeEffect);
    QFETCH(bool, matches);
    const auto effect = static_cast<WriteEffect>(writeEffect);
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->onWrite = [&](const OverlayProperty node, const std::int64_t value) {
      if (matches) port->values[node] = value;
      throw OverlayPolicyError({.origin = OverlayPolicyFailureOrigin::Provider, .node = node, .message = "incomplete side effect", .effect = effect});
    };
    uwf::app::OverlayPolicyService service(port);
    OverlayPolicyUpdate update;
    update.changes.passthrough = true;
    update.changes.reset = PersistentReset::Clear;
    const auto result = service.apply(update);
    QVERIFY(!result.succeeded());
    QCOMPARE(result.effect, effect == WriteEffect::None ? WriteEffect::None : WriteEffect::Uncertain);
    QCOMPARE(result.steps.front().outcome, effect == WriteEffect::None ? PolicyStepOutcome::Failed : PolicyStepOutcome::Uncertain);
    QVERIFY(!result.steps.front().observed);

    QCOMPARE(result.steps.back().outcome, PolicyStepOutcome::NotAttempted);
    QCOMPARE(result.failure->message, std::string("incomplete side effect"));
    QCOMPARE(port->writes.size(), std::size_t{1});
  }

  void batchUsesOnlyPublishedStateAndWriteReceipts() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    uwf::app::OverlayPolicyService service(port);
    const auto result = service.apply({.changes = {.passthrough = true, .reset = PersistentReset::Clear}});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, WriteEffect::Confirmed);
    QCOMPARE(result.steps[1].outcome, PolicyStepOutcome::Confirmed);
    QCOMPARE(result.snapshot.next.flags.value->raw, OverlayFlags::Passthrough);
    QCOMPARE(result.snapshot.resetPersistentState.value, std::optional<std::int64_t>{1});
    QCOMPARE(port->writes.size(), std::size_t{2});
  }
  void laterReadFailureDoesNotTurnAConfirmedCommandIntoFailure() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->onWrite = [&](OverlayProperty node, std::int64_t value) {
      port->values.at(node) = value;
      port->observationFailures[node] = {.origin = OverlayPolicyFailureOrigin::Provider, .kind = OperationFailureKind::AccessDenied, .node = node};
    };
    uwf::app::OverlayPolicyService service(port);
    const auto result = service.apply({.changes = {.passthrough = true}});
    QVERIFY(result.succeeded());
    QCOMPARE(result.effect, WriteEffect::Confirmed);
    QVERIFY(!result.reconciliationRequired());
    QCOMPARE(result.steps[0].observed, 1);
    QVERIFY(result.snapshot.next.flags.value->passthrough());
    QCOMPARE(port->writes.size(), std::size_t{1});

    QCOMPARE(service.snapshot().next.flags.status, PolicyObservationStatus::Failed);
  }
  void receiptSnapshotRetainsEarlierAcknowledgedValues() {
    uwf::clearLogLines();
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->onWrite = [&](OverlayProperty node, std::int64_t value) {
      port->values.at(node) = value;
      if (node == OverlayProperty::ResetPersistentState) port->values[OverlayProperty::NextOverlayFlags] = 0;
    };
    uwf::app::OverlayPolicyService service(port);
    const auto result = service.apply({.changes = {.passthrough = true, .reset = PersistentReset::Clear}});
    QVERIFY(result.succeeded());
    QVERIFY(!result.reconciliationRequired());
    QCOMPARE(result.effect, WriteEffect::Confirmed);
    QCOMPARE(result.steps[0].outcome, PolicyStepOutcome::Confirmed);
    QCOMPARE(result.steps[0].observed, 1);
    QCOMPARE(result.snapshot.next.flags.value->raw, OverlayFlags::Passthrough);
    QCOMPARE(port->writes.size(), std::size_t{2});

    QVERIFY(logRecords('E').isEmpty());
    QCOMPARE(service.snapshot().next.flags.value->raw, std::uint32_t{0});
  }
  void batchCompletesWithoutAProviderObservationAfterItsWrites() {
    const auto port = std::make_shared<MemoryOverlayOperations>();
    port->onWrite = [&](OverlayProperty node, std::int64_t value) {
      port->values.at(node) = value;
      if (node == OverlayProperty::ResetPersistentState)
        port->observationFailures[OverlayProperty::NextOverlayFlags] = {
            .origin = OverlayPolicyFailureOrigin::Provider, .kind = OperationFailureKind::AccessDenied, .node = OverlayProperty::NextOverlayFlags};
    };
    uwf::app::OverlayPolicyService service(port);
    const auto result = service.apply({.changes = {.passthrough = true, .reset = PersistentReset::Clear}});
    QVERIFY(result.succeeded());
    QVERIFY(!result.reconciliationRequired());
    QCOMPARE(result.effect, WriteEffect::Confirmed);
    QCOMPARE(result.steps[0].outcome, PolicyStepOutcome::Confirmed);
    QCOMPARE(result.steps[1].outcome, PolicyStepOutcome::Confirmed);
    QVERIFY(result.snapshot.next.flags.value->passthrough());
    QCOMPARE(port->writes.size(), std::size_t{2});

    QCOMPARE(service.snapshot().next.flags.status, PolicyObservationStatus::Failed);
  }
};
}  // namespace

QTEST_APPLESS_MAIN(OverlayPolicyTests)
#include "OverlayPolicyTests.moc"
