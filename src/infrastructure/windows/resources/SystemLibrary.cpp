// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "infrastructure/windows/resources/SystemLibrary.h"

#include <cstddef>
#include <cstring>
#include <span>

namespace uwf::windows {
namespace {
template <typename T>
const T* imageObject(const std::span<const std::byte> image, const std::size_t offset) noexcept {
  if (offset > image.size() || sizeof(T) > image.size() - offset) return nullptr;
  return reinterpret_cast<const T*>(image.data() + offset);
}
std::string_view imageString(const std::span<const std::byte> image, const std::size_t offset) noexcept {
  if (offset >= image.size()) return {};
  const auto* begin = reinterpret_cast<const char*>(image.data() + offset);
  const auto* end = static_cast<const char*>(std::memchr(begin, '\0', image.size() - offset));
  return end ? std::string_view(begin, static_cast<std::size_t>(end - begin)) : std::string_view{};
}
std::span<const std::byte> moduleImage(const HMODULE module) noexcept {
  if (!module) return {};
  const auto* base = reinterpret_cast<const std::byte*>(module);
  const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
  if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < static_cast<LONG>(sizeof(IMAGE_DOS_HEADER))) return {};
  const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
  if (nt->Signature != IMAGE_NT_SIGNATURE || nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) return {};
  return {base, static_cast<std::size_t>(nt->OptionalHeader.SizeOfImage)};
}
const IMAGE_NT_HEADERS* imageHeaders(const std::span<const std::byte> image) noexcept {
  const auto* dos = imageObject<IMAGE_DOS_HEADER>(image, 0);
  return dos ? imageObject<IMAGE_NT_HEADERS>(image, static_cast<std::size_t>(dos->e_lfanew)) : nullptr;
}
}  // namespace

SystemLibrary::SystemLibrary(const wchar_t* name) noexcept : m_module(LoadLibraryExW(name, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)) {
  if (!m_module) m_error = GetLastError();
}
FARPROC SystemLibrary::resolve(const char* name) const noexcept { return m_module ? GetProcAddress(m_module.get(), name) : nullptr; }
FARPROC SystemLibrary::importedSymbol(const std::string_view name) const noexcept {
  // LoadLibrary validates the initial PE headers; bound subsequent offsets by SizeOfImage.
  const auto image = moduleImage(m_module.get());
  const auto* nt = imageHeaders(image);
  if (!nt) return nullptr;
  if (nt->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_IMPORT) return nullptr;
  const auto imports = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (imports.VirtualAddress == 0 || imports.Size < sizeof(IMAGE_IMPORT_DESCRIPTOR)) return nullptr;
  const auto descriptorCount = static_cast<std::size_t>(imports.Size) / sizeof(IMAGE_IMPORT_DESCRIPTOR);
  FARPROC found = nullptr;
  for (std::size_t index = 0; index < descriptorCount; ++index) {
    const auto* descriptor =
        imageObject<IMAGE_IMPORT_DESCRIPTOR>(image, static_cast<std::size_t>(imports.VirtualAddress) + index * sizeof(IMAGE_IMPORT_DESCRIPTOR));
    if (!descriptor || descriptor->Name == 0) break;
    if (descriptor->OriginalFirstThunk == 0 || descriptor->FirstThunk == 0) continue;
    for (std::size_t item = 0; item < image.size() / sizeof(IMAGE_THUNK_DATA); ++item) {
      const auto offset = item * sizeof(IMAGE_THUNK_DATA);
      const auto* lookup = imageObject<IMAGE_THUNK_DATA>(image, static_cast<std::size_t>(descriptor->OriginalFirstThunk) + offset);
      const auto* bound = imageObject<IMAGE_THUNK_DATA>(image, static_cast<std::size_t>(descriptor->FirstThunk) + offset);
      if (!lookup || !bound || lookup->u1.AddressOfData == 0) break;
      if (IMAGE_SNAP_BY_ORDINAL(lookup->u1.Ordinal)) continue;
      const auto symbol = static_cast<std::size_t>(lookup->u1.AddressOfData);
      if (imageObject<IMAGE_IMPORT_BY_NAME>(image, symbol) && imageString(image, symbol + offsetof(IMAGE_IMPORT_BY_NAME, Name)) == name) {
        const auto function = reinterpret_cast<FARPROC>(static_cast<ULONG_PTR>(bound->u1.Function));
        if (found && found != function) return nullptr;
        found = function;
      }
    }
  }
  return found;
}
bool SystemLibrary::matchesCodeView(const GUID& guid, const DWORD age) const noexcept {
  const auto image = moduleImage(m_module.get());
  const auto* nt = imageHeaders(image);
  if (!nt || nt->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_DEBUG) return false;
  const auto debug = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
  const auto count = static_cast<std::size_t>(debug.Size) / sizeof(IMAGE_DEBUG_DIRECTORY);
  struct CodeView {
    DWORD signature;
    GUID guid;
    DWORD age;
  };
  constexpr DWORD rsdsSignature = 0x53445352;
  for (std::size_t index = 0; index < count; ++index) {
    const auto* record = imageObject<IMAGE_DEBUG_DIRECTORY>(image, static_cast<std::size_t>(debug.VirtualAddress) + index * sizeof(IMAGE_DEBUG_DIRECTORY));
    if (!record || record->Type != IMAGE_DEBUG_TYPE_CODEVIEW || record->SizeOfData < sizeof(CodeView)) continue;
    const auto* identity = imageObject<CodeView>(image, record->AddressOfRawData);
    if (identity && identity->signature == rsdsSignature && identity->age == age && IsEqualGUID(identity->guid, guid)) return true;
  }
  return false;
}
}  // namespace uwf::windows
