#include "UwfConfig.h"

#include <algorithm>
#include <ranges>

namespace uwf::core {

namespace {
std::string toLowerAscii(std::string s) {
  std::ranges::transform(s, s.begin(), [](const unsigned char c) { return std::tolower(c); });
  return s;
}
}  // namespace

void sortExclusions(std::vector<std::string>& items) {
  std::ranges::sort(items, [](const std::string& a, const std::string& b) { return toLowerAscii(a) < toLowerAscii(b); });
  // 必须传与排序一致的大小写不敏感谓词，否则 "C:\Foo" 与 "c:\foo"
  // 会排到一起但 std::ranges::unique 默认 operator== 判不等，留两份。
  const auto dup = std::ranges::unique(items, [](const std::string& a, const std::string& b) { return toLowerAscii(a) == toLowerAscii(b); });
  items.erase(dup.begin(), dup.end());
}

void sortSnapshot(UwfSnapshot& snapshot) {
  for (auto& v : snapshot.current.fileExclusions | std::views::values) sortExclusions(v);
  for (auto& v : snapshot.next.fileExclusions | std::views::values) sortExclusions(v);
  sortExclusions(snapshot.current.registryExclusions);
  sortExclusions(snapshot.next.registryExclusions);
}

bool PendingChanges::empty() const {
  return !setFilterEnabled && setOverlay.empty() && volumeProtect.empty() && volumeBindByVolumeName.empty() && addFileExclusions.empty() &&
         removeFileExclusions.empty() && addRegistryExclusions.empty() && removeRegistryExclusions.empty();
}

void PendingChanges::clear() {
  setFilterEnabled.reset();
  setOverlay = {};
  volumeProtect.clear();
  volumeBindByVolumeName.clear();
  addFileExclusions.clear();
  removeFileExclusions.clear();
  addRegistryExclusions.clear();
  removeRegistryExclusions.clear();
}

}  // namespace uwf::core
