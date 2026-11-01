// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
// clang-format off
#include <Windows.h>
// clang-format on

#include <QTest>
#include <array>
#include <cstdint>
#include <optional>

#include "infrastructure/uwf/errors/WindowsUwfError.h"

namespace {
using uwf::compat::ErrorKind;
using uwf::windows::classifyWindowsUwfError;
using uwf::windows::WindowsUwfErrorContext;

// Independent wire literals are the oracle. SDK assertions guard their spelling;
// the production classifier has no Windows SDK/API dependency.
static_assert(ERROR_INVALID_FUNCTION == 1 && ERROR_FILE_NOT_FOUND == 2 && ERROR_PATH_NOT_FOUND == 3 && ERROR_ACCESS_DENIED == 5);
static_assert(ERROR_NOT_ENOUGH_MEMORY == 8 && ERROR_INVALID_DATA == 13 && ERROR_OUTOFMEMORY == 14);
static_assert(ERROR_NOT_READY == 21 && ERROR_BAD_COMMAND == 22 && ERROR_SHARING_VIOLATION == 32 && ERROR_LOCK_VIOLATION == 33);
static_assert(ERROR_HANDLE_DISK_FULL == 39 && ERROR_NOT_SUPPORTED == 50 && ERROR_FILE_EXISTS == 80 && ERROR_INVALID_PARAMETER == 87);
static_assert(ERROR_DISK_FULL == 112 && ERROR_CALL_NOT_IMPLEMENTED == 120 && ERROR_MOD_NOT_FOUND == 126 && ERROR_PROC_NOT_FOUND == 127);
static_assert(ERROR_BUSY == 170 && ERROR_ALREADY_EXISTS == 183 && ERROR_OPERATION_ABORTED == 995);
static_assert(ERROR_SERVICE_NOT_ACTIVE == 1062 && ERROR_NO_UNICODE_TRANSLATION == 1113 && ERROR_DEVICE_NOT_CONNECTED == 1167);
static_assert(ERROR_NOT_FOUND == 1168 && ERROR_CANCELLED == 1223 && ERROR_PRIVILEGE_NOT_HELD == 1314);
static_assert(ERROR_DATATYPE_MISMATCH == 1629 && RPC_S_SERVER_UNAVAILABLE == 1722 && RPC_S_CALL_FAILED == 1726 && RPC_S_CALL_FAILED_DNE == 1727);
static_assert(ERROR_INVALID_ENVIRONMENT == 1805 && ERROR_NOT_ENOUGH_QUOTA == 1816);
static_assert(static_cast<std::uint32_t>(E_NOTIMPL) == 0x80004001u && static_cast<std::uint32_t>(E_NOINTERFACE) == 0x80004002u);
static_assert(static_cast<std::uint32_t>(E_ABORT) == 0x80004004u && static_cast<std::uint32_t>(E_UNEXPECTED) == 0x8000ffffu);
static_assert(static_cast<std::uint32_t>(RPC_E_DISCONNECTED) == 0x80010108u && static_cast<std::uint32_t>(RPC_E_SERVER_DIED) == 0x80010007u);
static_assert(static_cast<std::uint32_t>(RPC_E_SERVER_DIED_DNE) == 0x80010012u);
static_assert(static_cast<std::uint32_t>(CO_E_OBJNOTCONNECTED) == 0x800401fdu && static_cast<std::uint32_t>(CO_E_SERVER_EXEC_FAILURE) == 0x80080005u);
static_assert(static_cast<std::uint32_t>(REGDB_E_CLASSNOTREG) == 0x80040154u);
static_assert(noexcept(classifyWindowsUwfError(0u, WindowsUwfErrorContext::Operation)));

struct Win32Status {
  const char* name;
  std::uint32_t code;
  std::optional<ErrorKind> operation;
  std::optional<ErrorKind> observation;
};
constexpr std::array win32Statuses{Win32Status{"invalid-function", 1u, ErrorKind::Unsupported, ErrorKind::Unsupported},
                                   Win32Status{"not-supported", 50u, ErrorKind::Unsupported, ErrorKind::Unsupported},
                                   Win32Status{"not-implemented", 120u, ErrorKind::Unsupported, ErrorKind::Unsupported},
                                   Win32Status{"procedure-missing", 127u, ErrorKind::Unsupported, ErrorKind::Unsupported},
                                   Win32Status{"access-denied", 5u, ErrorKind::AccessDenied, ErrorKind::AccessDenied},
                                   Win32Status{"privilege-missing", 1314u, ErrorKind::AccessDenied, ErrorKind::AccessDenied},
                                   Win32Status{"file-missing", 2u, ErrorKind::NotFound, ErrorKind::NotFound},
                                   Win32Status{"path-missing", 3u, ErrorKind::NotFound, ErrorKind::NotFound},
                                   Win32Status{"object-missing", 1168u, ErrorKind::NotFound, ErrorKind::NotFound},
                                   Win32Status{"invalid-parameter", 87u, ErrorKind::InvalidArgument, ErrorKind::InvalidArgument},
                                   Win32Status{"canceled", 1223u, ErrorKind::Canceled, ErrorKind::Canceled},
                                   Win32Status{"aborted", 995u, ErrorKind::Canceled, ErrorKind::Canceled},
                                   Win32Status{"memory", 8u, ErrorKind::ResourceExhausted, ErrorKind::ResourceExhausted},
                                   Win32Status{"allocation", 14u, ErrorKind::ResourceExhausted, ErrorKind::ResourceExhausted},
                                   Win32Status{"disk-full", 112u, ErrorKind::ResourceExhausted, ErrorKind::ResourceExhausted},
                                   Win32Status{"handle-disk-full", 39u, ErrorKind::ResourceExhausted, ErrorKind::ResourceExhausted},
                                   Win32Status{"quota", 1816u, ErrorKind::ResourceExhausted, ErrorKind::ResourceExhausted},
                                   Win32Status{"busy", 170u, ErrorKind::Conflict, ErrorKind::Conflict},
                                   Win32Status{"already-exists", 183u, ErrorKind::Conflict, ErrorKind::Conflict},
                                   Win32Status{"file-exists", 80u, ErrorKind::Conflict, ErrorKind::Conflict},
                                   Win32Status{"sharing", 32u, ErrorKind::Conflict, ErrorKind::Conflict},
                                   Win32Status{"locked", 33u, ErrorKind::Conflict, ErrorKind::Conflict},
                                   Win32Status{"invalid-environment", 1805u, ErrorKind::Conflict, ErrorKind::Conflict},
                                   Win32Status{"invalid-device-state-dos-mapping", 22u, ErrorKind::Conflict, ErrorKind::Conflict},
                                   Win32Status{"not-ready", 21u, ErrorKind::Unavailable, ErrorKind::Unavailable},
                                   Win32Status{"module-missing", 126u, ErrorKind::Unavailable, ErrorKind::Unavailable},
                                   Win32Status{"device-disconnected", 1167u, ErrorKind::Unavailable, ErrorKind::Unavailable},
                                   Win32Status{"service-inactive", 1062u, ErrorKind::Unavailable, ErrorKind::Unavailable},
                                   Win32Status{"rpc-server", 1722u, ErrorKind::Unavailable, ErrorKind::Unavailable},
                                   Win32Status{"rpc-failed", 1726u, ErrorKind::Unavailable, ErrorKind::Unavailable},
                                   Win32Status{"rpc-not-executed", 1727u, ErrorKind::Unavailable, ErrorKind::Unavailable},
                                   Win32Status{"invalid-data", 13u, {}, ErrorKind::Protocol},
                                   Win32Status{"wrong-type", 1629u, {}, ErrorKind::Protocol},
                                   Win32Status{"invalid-unicode", 1113u, {}, ErrorKind::Protocol}};
static_assert(win32Statuses.size() == 34);

struct FullStatus {
  const char* name;
  std::uint32_t code;
  std::optional<ErrorKind> kind;
};
constexpr std::array fullStatuses{FullStatus{"not-implemented", 0x80004001u, ErrorKind::Unsupported},
                                  FullStatus{"interface-missing", 0x80004002u, ErrorKind::Unsupported},
                                  FullStatus{"abort", 0x80004004u, ErrorKind::Canceled},
                                  FullStatus{"rpc-disconnected", 0x80010108u, ErrorKind::Unavailable},
                                  FullStatus{"rpc-server-died", 0x80010007u, ErrorKind::Unavailable},
                                  FullStatus{"rpc-server-died-not-executed", 0x80010012u, ErrorKind::Unavailable},
                                  FullStatus{"object-disconnected", 0x800401fdu, ErrorKind::Unavailable},
                                  FullStatus{"server-launch-failed", 0x80080005u, ErrorKind::Unavailable},
                                  FullStatus{"class-missing", 0x80040154u, ErrorKind::Unavailable},
                                  FullStatus{"nt-device-not-ready", 0xc00000a3u, ErrorKind::Unavailable},
                                  FullStatus{"hresult-nt-device-not-ready", 0xd00000a3u, ErrorKind::Unavailable},
                                  FullStatus{"nt-invalid-device-state", 0xc0000184u, ErrorKind::Conflict},
                                  FullStatus{"hresult-nt-invalid-device-state", 0xd0000184u, ErrorKind::Conflict},
                                  FullStatus{"success-is-not-error-class", 0u, {}},
                                  FullStatus{"unknown-win32", 1603u, {}},
                                  FullStatus{"unknown-win32-hresult", 0x80070643u, {}},
                                  FullStatus{"unexpected-keeps-provider-meaning", 0x8000ffffu, {}},
                                  FullStatus{"generic-failure-keeps-provider-meaning", 0x80004005u, {}},
                                  FullStatus{"changed-apartment-keeps-provider-meaning", 0x80010106u, {}},
                                  FullStatus{"foreign-facility-access-denied-lowword", 0x80040005u, {}},
                                  FullStatus{"foreign-facility-missing-lowword", 0x80040002u, {}},
                                  FullStatus{"foreign-facility-invalid-data-lowword", 0x8004000du, {}},
                                  FullStatus{"nonwin32-not-supported-lowword", 0x80000032u, {}},
                                  FullStatus{"success-facility-access-denied-lowword", 0x00070005u, {}},
                                  FullStatus{"customer-facility-access-denied-lowword", 0xa0070005u, {}},
                                  FullStatus{"nt-facility-access-denied-lowword", 0xc0070005u, {}},
                                  FullStatus{"unknown-nt-status", 0xc0000001u, {}},
                                  FullStatus{"unknown-hresult-nt-status", 0xd0000001u, {}},
                                  FullStatus{"unlisted-dos-gen-failure", 31u, {}},
                                  FullStatus{"unlisted-dos-gen-failure-hresult", 0x8007001fu, {}}};
static_assert(fullStatuses.size() == 30);
}  // namespace

class UwfWindowsStatusTests final : public QObject {
  Q_OBJECT
 private slots:
  void literalWireStatusesRespectRepresentationAndObservationContext_data() {
    QTest::addColumn<quint32>("code");
    QTest::addColumn<bool>("observation");
    QTest::addColumn<int>("expectedKind");
    for (const auto& entry : win32Statuses)
      for (const bool hresult : {false, true})
        for (const bool observation : {false, true}) {
          const auto expected = observation ? entry.observation : entry.operation;
          const auto name = QStringLiteral("win32/%1/%2/%3")
                                .arg(QString::fromLatin1(entry.name), hresult ? QStringLiteral("hresult") : QStringLiteral("raw"),
                                     observation ? QStringLiteral("observation") : QStringLiteral("operation"));
          QTest::newRow(qUtf8Printable(name)) << quint32(hresult ? 0x80070000u | entry.code : entry.code) << observation
                                              << (expected ? static_cast<int>(*expected) : -1);
        }
    for (const auto& entry : fullStatuses)
      for (const bool observation : {false, true}) {
        const auto name =
            QStringLiteral("full/%1/%2").arg(QString::fromLatin1(entry.name), observation ? QStringLiteral("observation") : QStringLiteral("operation"));
        QTest::newRow(qUtf8Printable(name)) << quint32(entry.code) << observation << (entry.kind ? static_cast<int>(*entry.kind) : -1);
      }
  }
  void literalWireStatusesRespectRepresentationAndObservationContext() {
    QFETCH(quint32, code);
    QFETCH(bool, observation);
    QFETCH(int, expectedKind);
    const auto actual = classifyWindowsUwfError(code, observation ? WindowsUwfErrorContext::Observation : WindowsUwfErrorContext::Operation);
    QCOMPARE(actual.has_value(), expectedKind >= 0);
    if (actual) QCOMPARE(static_cast<int>(*actual), expectedKind);
  }
};

QTEST_APPLESS_MAIN(UwfWindowsStatusTests)
#include "UwfWindowsStatusTests.moc"
