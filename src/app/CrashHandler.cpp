/*
 * Copyright (c) 2026 HsingYun (iakext@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "CrashHandler.h"

#if !defined(UWF_SANITIZE)
// MinGW 的 dbghelp.h 不自包含，必须先由 windows.h 提供 Win32 类型。
// clang-format off
#include <windows.h>
#include <dbghelp.h>
#include <strsafe.h>
// clang-format on

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iterator>

#include "uwf_version.h"
#endif

namespace uwf::app {

#if defined(UWF_SANITIZE)

void CrashHandler::install() {}

#else

namespace {

constexpr DWORD kTerminateException = 0xE0000001UL;
constexpr std::size_t kPathCapacity = 32768;
constexpr std::size_t kMaximumFrames = 128;

wchar_t g_executableDirectory[kPathCapacity]{};
wchar_t g_fallbackDirectory[kPathCapacity]{};
wchar_t g_textPath[kPathCapacity]{};
wchar_t g_dumpPath[kPathCapacity]{};
wchar_t g_modulePath[kPathCapacity]{};
volatile LONG g_crashStarted = 0;

void writeBytes(const HANDLE file, const char* const data, const DWORD size) {
  if (file == INVALID_HANDLE_VALUE || !data || size == 0) return;
  DWORD offset = 0;
  while (offset < size) {
    DWORD written = 0;
    if (!WriteFile(file, data + offset, size - offset, &written, nullptr) || written == 0) return;
    offset += written;
  }
}

void writeText(const HANDLE file, const char* const text) {
  if (!text) return;
  const std::size_t length = strnlen(text, 16384);
  writeBytes(file, text, static_cast<DWORD>(length));
}

void writeWin32Error(const HANDLE file, const char* const operation, const DWORD error) {
  char line[256]{};
  (void)StringCchPrintfA(line, std::size(line), "%s failed; win32=%lu\r\n", operation, static_cast<unsigned long>(error));
  writeText(file, line);
}

DWORD64 moduleName(const DWORD64 address, char* const output, const int outputSize) {
  if (!output || outputSize <= 0) return 0;
  output[0] = '\0';
  HMODULE module = nullptr;
  if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCWSTR>(static_cast<std::uintptr_t>(address)), &module))
    return 0;

  g_modulePath[0] = L'\0';
  if (GetModuleFileNameW(module, g_modulePath, static_cast<DWORD>(std::size(g_modulePath))) == 0) return reinterpret_cast<DWORD64>(module);
  const wchar_t* name = g_modulePath;
  for (const wchar_t* cursor = g_modulePath; *cursor; ++cursor) {
    if (*cursor == L'\\' || *cursor == L'/') name = cursor + 1;
  }
  (void)WideCharToMultiByte(CP_UTF8, 0, name, -1, output, outputSize, nullptr, nullptr);
  return reinterpret_cast<DWORD64>(module);
}

struct ImageIdentity {
  DWORD timestamp = 0;
  DWORD size = 0;
};

ImageIdentity executableIdentity() {
  const auto* const base = reinterpret_cast<const std::byte*>(GetModuleHandleW(nullptr));
  if (!base) return {};
  const auto* const dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
  if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0) return {};
  const auto* const nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
  if (nt->Signature != IMAGE_NT_SIGNATURE) return {};
  return {nt->FileHeader.TimeDateStamp, nt->OptionalHeader.SizeOfImage};
}

bool formatCrashArtifactPath(wchar_t* const output, const wchar_t* const directory, const SYSTEMTIME& time, const wchar_t* const suffix) {
  if (!output || !directory || !suffix) return false;
  return SUCCEEDED(StringCchPrintfW(output, kPathCapacity, L"%sUWF.%hs.crash.%04u%02u%02u-%02u%02u%02u-%03u%s", directory, UWF_VER_STRING,
                                    static_cast<unsigned int>(time.wYear), static_cast<unsigned int>(time.wMonth), static_cast<unsigned int>(time.wDay),
                                    static_cast<unsigned int>(time.wHour), static_cast<unsigned int>(time.wMinute), static_cast<unsigned int>(time.wSecond),
                                    static_cast<unsigned int>(time.wMilliseconds), suffix));
}

bool initializeExecutableDirectory() {
  wchar_t executablePath[kPathCapacity]{};
  const DWORD length = GetModuleFileNameW(nullptr, executablePath, static_cast<DWORD>(std::size(executablePath)));
  if (length == 0 || length >= std::size(executablePath) || FAILED(StringCchCopyW(g_executableDirectory, std::size(g_executableDirectory), executablePath)))
    return false;

  for (wchar_t* cursor = g_executableDirectory + length; cursor != g_executableDirectory; --cursor) {
    if (cursor[-1] == L'\\' || cursor[-1] == L'/') {
      *cursor = L'\0';
      return true;
    }
  }
  return false;
}

bool initializeFallbackDirectory() {
  const DWORD length = GetTempPathW(static_cast<DWORD>(std::size(g_fallbackDirectory)), g_fallbackDirectory);
  if (length == 0 || length >= std::size(g_fallbackDirectory)) return false;
  if (FAILED(StringCchCatW(g_fallbackDirectory, std::size(g_fallbackDirectory), L"UWF-CrashDumps\\"))) return false;
  if (CreateDirectoryW(g_fallbackDirectory, nullptr) || GetLastError() == ERROR_ALREADY_EXISTS) return true;
  g_fallbackDirectory[0] = L'\0';
  return false;
}

struct CrashFiles {
  HANDLE text = INVALID_HANDLE_VALUE;
  HANDLE dump = INVALID_HANDLE_VALUE;
};

CrashFiles createCrashFiles(const wchar_t* const directory, const SYSTEMTIME& time) {
  if (!directory || directory[0] == L'\0' || !formatCrashArtifactPath(g_textPath, directory, time, L".txt") ||
      !formatCrashArtifactPath(g_dumpPath, directory, time, L".dmp"))
    return {};
  return {CreateFileW(g_textPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr),
          CreateFileW(g_dumpPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr)};
}

void discardPartialCrashFiles(const CrashFiles files) {
  if (files.text != INVALID_HANDLE_VALUE) {
    (void)CloseHandle(files.text);
    (void)DeleteFileW(g_textPath);
  }
  if (files.dump != INVALID_HANDLE_VALUE) {
    (void)CloseHandle(files.dump);
    (void)DeleteFileW(g_dumpPath);
  }
}

void writeStackTrace(const HANDLE file, const EXCEPTION_POINTERS* const exceptionPointers) {
  if (!exceptionPointers || !exceptionPointers->ContextRecord) {
    writeText(file, "stack unavailable: no exception context\r\n");
    return;
  }
  CONTEXT context = *exceptionPointers->ContextRecord;
  writeText(file, "\r\nStack:\r\n");

  const auto writeFrame = [file](const std::size_t index, const DWORD64 address) {
    char module[1024]{};
    const DWORD64 moduleBase = moduleName(address, module, static_cast<int>(std::size(module)));
    char line[2048]{};
    (void)StringCchPrintfA(line, std::size(line), "#%03zu 0x%016llX %s+0x%llX\r\n", index, static_cast<unsigned long long>(address),
                           module[0] ? module : "<unknown>", static_cast<unsigned long long>(moduleBase && address >= moduleBase ? address - moduleBase : 0));
    writeText(file, line);
  };

#if defined(_WIN64)
  for (std::size_t index = 0; index < kMaximumFrames; ++index) {
    const DWORD64 address = context.Rip;
    if (address == 0) break;
    writeFrame(index, address);

    const DWORD64 previousStack = context.Rsp;
    DWORD64 imageBase = 0;
    if (const PRUNTIME_FUNCTION function = RtlLookupFunctionEntry(address, &imageBase, nullptr)) {
      PVOID handlerData = nullptr;
      DWORD64 establisherFrame = 0;
      (void)RtlVirtualUnwind(UNW_FLAG_NHANDLER, imageBase, address, function, &context, &handlerData, &establisherFrame, nullptr);
    } else {
      DWORD64 returnAddress = 0;
      SIZE_T bytesRead = 0;
      if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(static_cast<std::uintptr_t>(context.Rsp)), &returnAddress,
                             sizeof(returnAddress), &bytesRead) ||
          bytesRead != sizeof(returnAddress))
        break;
      context.Rsp += sizeof(returnAddress);
      context.Rip = returnAddress;
    }
    if (context.Rsp <= previousStack) break;
  }
#else
  writeText(file, "stack unavailable: unsupported architecture\r\n");
#endif
}

void writeCrashArtifacts(EXCEPTION_POINTERS* const exceptionPointers) {
  if (InterlockedCompareExchange(&g_crashStarted, 1, 0) != 0) return;

  SYSTEMTIME time{};
  GetLocalTime(&time);
  const DWORD processId = GetCurrentProcessId();
  const DWORD threadId = GetCurrentThreadId();
  CrashFiles files = createCrashFiles(g_executableDirectory, time);
  if ((files.text == INVALID_HANDLE_VALUE || files.dump == INVALID_HANDLE_VALUE) && g_fallbackDirectory[0] != L'\0') {
    discardPartialCrashFiles(files);
    files = createCrashFiles(g_fallbackDirectory, time);
  }
  const HANDLE textFile = files.text;
  const HANDLE dumpFile = files.dump;
  const DWORD dumpCreateError = dumpFile == INVALID_HANDLE_VALUE ? GetLastError() : ERROR_SUCCESS;

  if (textFile != INVALID_HANDLE_VALUE) {
    const DWORD exceptionCode = exceptionPointers && exceptionPointers->ExceptionRecord ? exceptionPointers->ExceptionRecord->ExceptionCode : 0;
    const void* const exceptionAddress =
        exceptionPointers && exceptionPointers->ExceptionRecord ? exceptionPointers->ExceptionRecord->ExceptionAddress : nullptr;
    const ImageIdentity identity = executableIdentity();
    char header[512]{};
    (void)StringCchPrintfA(header, std::size(header),
                           "UWF crash report\r\nversion=%s image_timestamp=0x%08lX image_size=0x%08lX\r\n"
                           "exception=0x%08lX address=%p\r\nprocess=%lu thread=%lu\r\n",
                           UWF_VER_STRING, static_cast<unsigned long>(identity.timestamp), static_cast<unsigned long>(identity.size),
                           static_cast<unsigned long>(exceptionCode), exceptionAddress, static_cast<unsigned long>(processId),
                           static_cast<unsigned long>(threadId));
    writeText(textFile, header);
    writeStackTrace(textFile, exceptionPointers);
    if (dumpFile == INVALID_HANDLE_VALUE) writeWin32Error(textFile, "CreateFileW(dump)", dumpCreateError);
    (void)FlushFileBuffers(textFile);
  }

  if (dumpFile != INVALID_HANDLE_VALUE) {
    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo{threadId, exceptionPointers, FALSE};
    constexpr MINIDUMP_TYPE dumpType =
        static_cast<MINIDUMP_TYPE>(static_cast<unsigned int>(MiniDumpWithThreadInfo) | static_cast<unsigned int>(MiniDumpWithIndirectlyReferencedMemory) |
                                   static_cast<unsigned int>(MiniDumpScanMemory) | static_cast<unsigned int>(MiniDumpWithUnloadedModules));
    if (!MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFile, dumpType, exceptionPointers ? &exceptionInfo : nullptr, nullptr, nullptr)) {
      const DWORD dumpError = GetLastError();
      if (textFile != INVALID_HANDLE_VALUE) writeWin32Error(textFile, "MiniDumpWriteDump", dumpError);
    }
    (void)CloseHandle(dumpFile);
  }

  if (textFile != INVALID_HANDLE_VALUE) {
    (void)FlushFileBuffers(textFile);
    (void)CloseHandle(textFile);
  }
}

LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS* const exceptionPointers) {
  writeCrashArtifacts(exceptionPointers);
  return EXCEPTION_EXECUTE_HANDLER;
}

}  // namespace

void CrashHandler::install() {
  if (!initializeExecutableDirectory()) g_executableDirectory[0] = L'\0';
  (void)initializeFallbackDirectory();

  ULONG stackGuarantee = 64 * 1024;
  (void)SetThreadStackGuarantee(&stackGuarantee);

  (void)SetUnhandledExceptionFilter(unhandledExceptionFilter);
  std::set_terminate([]() {
    RaiseException(kTerminateException, EXCEPTION_NONCONTINUABLE, 0, nullptr);
    TerminateProcess(GetCurrentProcess(), kTerminateException);
  });
}

#endif

}  // namespace uwf::app
