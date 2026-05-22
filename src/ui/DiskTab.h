#pragma once

#include <QString>
#include <QWidget>
#include <optional>
#include <string>
#include <utility>

#include "../core/UwfConfig.h"
// 完整 include（不是前向声明）：本文件 public API 暴露
// ExclusionListWidget::ImportOutcome 这个嵌套枚举类型，前向声明无法满足。
#include "ExclusionListWidget.h"

class QAction;
class QLabel;
class QPushButton;
class QTabWidget;

namespace uwf::ui {

class StatusPanel;

// 把 DiskSupport 枚举翻译成中文原因（用于 tooltip / banner）。
// 放在这里而不是 core/，因为只有 UI 才需要这段文字。
std::string diskSupportText(core::DiskSupport s, const std::string& fileSystem);

class DiskTab : public QWidget {
  Q_OBJECT
 public:
  explicit DiskTab(const core::DiskInfo& disk, QWidget* parent = nullptr);

  // UWF 不可读或进程未提权时，保护开关 / 绑定方式 / 排除列表的增删 / 提交
  // 按钮一律置灰；列表内容仍可查看、可滚动、可切换 TAB。
  void applySnapshot(const core::UwfSnapshot& snap);
  void markUnsupported() const;
  // 文件系统受限（exFAT 等）的 UI 处理：保护开关 / 绑定方式保持可用，
  // 但文件排除列表 readOnly + commit 文件类菜单灰掉。
  void markLimitedFileSystem() const;

  [[nodiscard]] QString driveLetter() const { return QString::fromStdString(m_disk.driveLetter); }
  // 卷是否可被 UWF 保护——含 NTFS/FAT 完全支持 + exFAT/ReFS 等 limited
  // 支持。Limited 卷允许 protect 开关与绑定方式，但禁用文件排除 / commit。
  [[nodiscard]] bool supported() const { return m_disk.support == core::DiskSupport::Supported || m_disk.support == core::DiskSupport::FileSystemLimited; }
  // 卷的文件系统是否支持文件排除列表 / 单文件提交。仅 NTFS / FAT(32) 是 true。
  [[nodiscard]] bool canManageExclusions() const { return m_disk.support == core::DiskSupport::Supported; }

  [[nodiscard]] QStringList pendingFileAdded() const;
  [[nodiscard]] QStringList pendingFileRemoved() const;
  [[nodiscard]] QStringList pendingRegAdded() const;
  [[nodiscard]] QStringList pendingRegRemoved() const;

  [[nodiscard]] std::optional<bool> pendingVolumeProtected() const;
  // bBindByVolumeName：true=按卷 ID 绑定；nullopt=未改动。
  [[nodiscard]] std::optional<bool> pendingBindByVolumeName() const;

  // 给"导入 uwfmgr 命令"用：把命令翻译成对应的 UI 操作。
  // - importProtect 返回 false：卷不受支持 / 已是目标值。
  // - importAddFileExclusion / importRemoveFileExclusion / importAddRegistryExclusion /
  //   importRemoveRegistryExclusion 沿用 ExclusionListWidget::ImportOutcome
  //   把"成功 / 重复 / 不在本卷 / 黑名单拒绝"的语义透传上去。
  bool importProtect(bool v);
  ExclusionListWidget::ImportOutcome importAddFileExclusion(const QString& path);
  ExclusionListWidget::ImportOutcome importRemoveFileExclusion(const QString& path);
  // 注册表排除只挂在系统盘 TAB 上，其他 TAB 调用会返回 RejectedNotOnVolume
  // （表示"这个 TAB 没有注册表排除入口"）。
  ExclusionListWidget::ImportOutcome importAddRegistryExclusion(const QString& key);
  ExclusionListWidget::ImportOutcome importRemoveRegistryExclusion(const QString& key);

 signals:
  void pendingChanged();
  void statusHint(const QString& text, int msec);
  void commitFileRequested(const QString& path);
  void commitFileDeletionRequested(const QString& path);
  void commitRegistryRequested(const QString& key, const QString& valueName);
  void commitRegistryDeletionRequested(const QString& key, const QString& valueName);

 private slots:
  void onCommitFile();
  void onCommitDir();
  void onCommitFileDelete();
  void onCommitRegistry();
  void onCommitRegistryDelete();

 private:
  // 按当前快照里的两个"当前会话"状态计算"持久化"按钮的可用性：
  //   - globalFilterOn：UWF_Filter.CurrentEnabled。false 则一切持久化都不可能成功。
  //   - thisVolumeProtected：本卷在当前会话是否受保护。false 则文件/目录持久化不可用。
  // 此外还叠加 m_editable（UWF 可读 + 已提权）：为 false 则一切持久化按钮直接
  // 置灰。注册表持久化只看 globalFilterOn，和具体卷无关。
  void updateCommitEnablement(bool globalFilterOn, bool thisVolumeProtected) const;
  // 主题切换时刷新菜单 icon + 重新生成 heading 的 RichText（里面带 inline 色）。
  void refreshThemedIcons();
  // 弹出"注册表键 + 可选值名"两行输入框，提交注册表修改 / 删除共用。标题、值名
  // 占位符、warn 横幅文案由调用方按用途定制。用户确认且键非空时返回
  // {已 trim 的键, 原样值名}；取消或键为空返回 nullopt。
  std::optional<std::pair<QString, QString>> promptRegistryTarget(const QString& title, const QString& valuePlaceholder, const QString& hintText);

  core::DiskInfo m_disk;
  QLabel* m_headingLabel = nullptr;     // 顶部盘符 + 磁盘信息
  QPushButton* m_overlayBtn = nullptr;  // "查看覆盖层文件" 按钮（仅 NTFS）
  QPushButton* m_commitBtn = nullptr;   // "持久化" 菜单按钮
  QAction* m_commitFileAct = nullptr;
  QAction* m_commitDirAct = nullptr;
  QAction* m_commitFileDeleteAct = nullptr;
  QAction* m_commitRegAct = nullptr;        // 仅系统盘 TAB 上创建
  QAction* m_commitRegDeleteAct = nullptr;  // 仅系统盘 TAB 上创建
  StatusPanel* m_status = nullptr;
  ExclusionListWidget* m_files = nullptr;
  ExclusionListWidget* m_regs = nullptr;
  QTabWidget* m_infoTabs = nullptr;  // 内层 TAB（文件排除 / 注册表排除）
  bool m_showRegistry = false;
  // 最近一次 applySnapshot 算出的"可写"（UWF 可读 + 已提权），供
  // updateCommitEnablement 读取。构造期默认 false：彼时还没有快照，首帧持久化
  // 按钮一律按"不可写"出。
  bool m_editable = false;
};

}  // namespace uwf::ui
