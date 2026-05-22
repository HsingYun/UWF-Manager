#include "StringUtil.h"

#include <windows.h>

#include <cctype>

namespace uwf {

std::string toLowerAscii(std::string s) {
  for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}

std::string toUpperAscii(std::string s) {
  for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  return s;
}

std::string trim(std::string s) {
  size_t begin = 0;
  size_t end = s.size();
  while (begin < end && std::isspace(static_cast<unsigned char>(s[begin]))) ++begin;
  while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
  return s.substr(begin, end - begin);
}

std::wstring utf8ToWide(std::string_view utf8) {
  if (utf8.empty()) return {};
  const int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
  if (wideLen <= 0) return {};
  std::wstring out(static_cast<size_t>(wideLen), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), out.data(), wideLen);
  return out;
}

std::string wideToUtf8(std::wstring_view wide) {
  if (wide.empty()) return {};
  const int byteLen = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
  if (byteLen <= 0) return {};
  std::string out(static_cast<size_t>(byteLen), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), out.data(), byteLen, nullptr, nullptr);
  return out;
}

std::string wideToUtf8(const wchar_t* wide) {
  if (!wide) return {};
  return wideToUtf8(std::wstring_view(wide));
}

}  // namespace uwf
