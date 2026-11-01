// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/management/csp/DmCspTransport.h"

// OleAuto.h requires the Windows types first.
// clang-format off
#include <Windows.h>
#include <OleAuto.h>
#include <combaseapi.h>
// clang-format on

#include <QScopeGuard>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "foundation/logging/Log.h"
#include "infrastructure/windows/errors/WindowsErrorText.h"
#include "infrastructure/windows/execution/MtaExecutor.h"
#include "infrastructure/windows/resources/ComPtr.h"
#include "infrastructure/windows/resources/SystemLibrary.h"

namespace uwf::windows {
namespace {
using ProcessConfigXml = HRESULT(WINAPI*)(PCWSTR, PCWSTR*, DWORD, BSTR*);
constexpr wchar_t kLibraryName[] = L"dmprocessxmlfiltered.dll";
constexpr char kProcessConfigXmlExport[] = "DMProcessConfigXMLFiltered";

HRESULT mtaApartmentStatus() noexcept {
  APTTYPE apartment;
  APTTYPEQUALIFIER qualifier;
  const auto status = CoGetApartmentType(&apartment, &qualifier);
  if (FAILED(status)) return status;
  return apartment == APTTYPE_MTA ? S_OK : RPC_E_CHANGED_MODE;
}

QString takeBstr(BSTR value) {
  const auto cleanup = qScopeGuard([value] { SysFreeString(value); });
  return value ? QString::fromWCharArray(value, static_cast<qsizetype>(SysStringLen(value))) : QString{};
}

class NativeDmCspTransport final : public DmCspTransport {
 public:
  NativeDmCspTransport() : m_library(kLibraryName) {
    if (!m_library.loaded()) {
      m_unavailableCode = static_cast<std::uint32_t>(HRESULT_FROM_WIN32(m_library.error()));
      UWF_LOG_I("dm-csp") << "transport unavailable: load library=" << QString::fromWCharArray(kLibraryName).toStdString() << " code=" << m_unavailableCode
                          << " detail=" << windowsErrorText(m_unavailableCode).toStdString();
      return;
    }
    m_processXml = reinterpret_cast<ProcessConfigXml>(m_library.resolve(kProcessConfigXmlExport));
    if (!m_processXml) {
      m_unavailableCode = static_cast<std::uint32_t>(HRESULT_FROM_WIN32(GetLastError()));
      UWF_LOG_I("dm-csp") << "transport unavailable: resolve export=" << kProcessConfigXmlExport << " code=" << m_unavailableCode
                          << " detail=" << windowsErrorText(m_unavailableCode).toStdString();
    }
  }
  bool available() override { return SUCCEEDED(mtaApartmentStatus()) && m_processXml != nullptr; }
  DmCspReply process(const QString& xml, const QStringList& allowedCspNames) override {
    const auto apartment = mtaApartmentStatus();
    if (FAILED(apartment)) return {static_cast<std::uint32_t>(apartment), {}, windowsErrorText(static_cast<std::uint32_t>(apartment)), false};
    if (static_cast<quint64>(allowedCspNames.size()) > std::numeric_limits<DWORD>::max())
      return {static_cast<std::uint32_t>(E_INVALIDARG), {}, QStringLiteral("Too many CSP names"), false};
    if (!m_processXml) return {m_unavailableCode, {}, windowsErrorText(m_unavailableCode), false};
    std::vector<PCWSTR> allowed;
    allowed.reserve(static_cast<std::size_t>(allowedCspNames.size()));
    for (const auto& name : allowedCspNames) allowed.push_back(reinterpret_cast<PCWSTR>(name.utf16()));
    BSTR output = nullptr;
    const auto releaseOutput = qScopeGuard([&output] { SysFreeString(output); });
    // Never confuse an earlier COM error with this provider's extended information.
    SetErrorInfo(0, nullptr);
    const auto hr = m_processXml(reinterpret_cast<PCWSTR>(xml.utf16()), allowed.data(), static_cast<DWORD>(allowed.size()), &output);
    ComPtr<IErrorInfo> error;
    GetErrorInfo(0, error.put());  // Capture before any further COM operations.
    QString extended;
    if (error) {
      BSTR source = nullptr;
      BSTR description = nullptr;
      error->GetSource(&source);
      error->GetDescription(&description);
      const auto sourceText = takeBstr(source);
      const auto descriptionText = takeBstr(description);
      extended = sourceText.isEmpty() ? descriptionText : descriptionText.isEmpty() ? sourceText : sourceText + u": " + descriptionText;
    }
    if (FAILED(hr)) {
      const auto detail = windowsErrorText(static_cast<std::uint32_t>(hr));
      if (!detail.isEmpty()) extended = extended.isEmpty() ? detail : detail + u": " + extended;
    }
    return {static_cast<std::uint32_t>(hr), output ? QString::fromWCharArray(output, static_cast<qsizetype>(SysStringLen(output))) : QString{}, extended};
  }

 private:
  SystemLibrary m_library;
  ProcessConfigXml m_processXml = nullptr;
  std::uint32_t m_unavailableCode = 0;
};

// The facade is apartment-neutral. Only its native implementation enters COM,
// and it is constructed, called, and released within the shared MTA pool.
class MtaDmCspTransport final : public DmCspTransport {
 public:
  MtaDmCspTransport(std::function<std::shared_ptr<DmCspTransport>()> factory, std::shared_ptr<MtaExecutor> executor)
      : m_executor(std::move(executor)), m_native(m_executor->invoke(std::move(factory))) {
    if (!m_native) throw std::invalid_argument("The MTA CSP transport factory returned no transport");
  }
  ~MtaDmCspTransport() override {
    m_executor->finalize([this]() noexcept { m_native.reset(); });
  }
  bool available() override {
    return m_executor->invoke([native = m_native] { return native->available(); });
  }
  DmCspReply process(const QString& xml, const QStringList& allowedCspNames) override {
    return m_executor->invoke([native = m_native, xml, allowedCspNames] { return native->process(xml, allowedCspNames); });
  }

 private:
  // The executor outlives final release of the native transport.
  std::shared_ptr<MtaExecutor> m_executor;
  std::shared_ptr<DmCspTransport> m_native;
};
}  // namespace

std::shared_ptr<DmCspTransport> createMtaDmCspTransport(std::function<std::shared_ptr<DmCspTransport>()> factory, std::shared_ptr<MtaExecutor> executor) {
  if (!factory || !executor) throw std::invalid_argument("An MTA CSP transport requires a factory and executor");
  return std::make_shared<MtaDmCspTransport>(std::move(factory), std::move(executor));
}
std::shared_ptr<DmCspTransport> createSystemDmCspTransport() {
  return createMtaDmCspTransport([] { return std::make_shared<NativeDmCspTransport>(); }, systemMtaExecutor());
}
}  // namespace uwf::windows
