#pragma once

#include <QDialog>
#include <QString>
#include <QVector>
#include <cstdint>
#include <mutex>
#include <thread>

class QLabel;
class QListWidget;
class QProgressBar;
class QPushButton;

namespace uwf::ui {

// 单条覆盖层文件条目。
//   rawName          —— WMI 直接返回的相对路径，可能带 ":$DATA" / ":$INDEX_ALLOCATION"
//                       等 NTFS 流后缀；只用于诊断回查（去重后会丢失，仅保留一条）。
//   absolutePath     —— 拼好盘符且去掉 NTFS 流后缀的路径，如 "C:\Users\foo"。
//                       UI 列表、右键菜单、commit、导出统一用这个值。
//   isDirectory      —— rawName 后缀为 ":$INDEX_ALLOCATION" 时为 true（NTFS 目录索引流）。
//                       去重时只要任一条原始条目是目录索引，合并后即视为目录。
//   isSystemMetadata —— absolutePath 的 basename 以 '$' 开头（NTFS 元数据文件 /
//                       $RECYCLE.BIN 等）。这类条目右键菜单不显示 commit——
//                       UWF_Volume.CommitFile 对元数据走不通，强行调用只会报
//                       NOT_FOUND，没有意义。
//   fileSize         —— 去重后是 raw 条目里所有同 absolutePath 的 size 求和（同一
//                       文件的多条 NTFS 流加起来才是它在 overlay 占的总字节）。
struct OverlayFileEntry {
  QString rawName;
  QString absolutePath;
  bool isDirectory = false;
  bool isSystemMetadata = false;
  qulonglong fileSize = 0;
};

// 异步展示某个 NTFS 卷当前 overlay 中缓存的文件列表。底层走
// UWF_Overlay.GetOverlayFiles——这调用很慢、按 overlay 用量近似指数级增长，
// 所以在 worker 线程上跑（自己 CoInitializeEx + 独立 WmiSession），结果
// 投回 UI 线程渲染。加载期间显示忙碌进度条；用户可以提前关闭对话框，
// worker 线程靠 QPointer 检查 dialog 是否还活着，活着再投递结果。
class OverlayFilesDialog : public QDialog {
  Q_OBJECT
 public:
  explicit OverlayFilesDialog(const QString& driveLetter, QWidget* parent = nullptr);
  ~OverlayFilesDialog() override;

 signals:
  // 用户在右键菜单点击"提交文件/文件夹改动到磁盘…"时发出，绝对路径（带盘符）。
  // DiskTab 转发给 MainWindow 走 commitFilePath——后者按 QFileInfo::isDir 自动
  // 走单文件 commit 还是 QDirIterator 递归遍历，文件 / 目录共用同一条信号即可。
  // （NTFS 元数据条目在 isSystemMetadata 过滤里直接不出 commit 菜单，不会走到这。）
  void commitFileRequested(const QString& absolutePath);

 private slots:
  void onContextMenu(const QPoint& pos);
  void onExportClicked();

 private:
  void startLoading();
  // hresult 0 表示成功；非 0 时按命名常量分支处理（RPC_E_SERVERFAULT
  // / WBEM_E_OUT_OF_MEMORY / WBEM_E_NOT_SUPPORTED 等）。
  void onLoadFinished(QVector<OverlayFileEntry> entries, const QString& error, int32_t hresult);
  // absolutePath 已是规范化绝对路径，不依赖任何实例状态，故为 static。
  static void openContainingFolder(const QString& absolutePath);

  QString m_driveLetter;  // 形如 "C:"
  QListWidget* m_list = nullptr;
  QProgressBar* m_progress = nullptr;
  QLabel* m_loadingLabel = nullptr;
  QLabel* m_summary = nullptr;
  QPushButton* m_exportBtn = nullptr;
  // 加载完成后的条目副本（已规范化 + 排序），导出按钮直接读这个，避免再
  // 走一遍 list widget 还原。
  QVector<OverlayFileEntry> m_entries;

  // 后台加载 worker。GetOverlayFiles 可能跑到小时级——worker 启用 COM 调用
  // 取消，析构时对 m_workerThreadId 调 CoCancelCall 解阻塞，再 join（瞬间完成）。
  std::thread m_worker;
  std::mutex m_cancelMutex;            // 保护 m_workerThreadId
  unsigned long m_workerThreadId = 0;  // worker 线程 ID（DWORD）；0 = 未运行 / 已结束
};

}  // namespace uwf::ui
