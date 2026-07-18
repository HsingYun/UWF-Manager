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
#pragma once

#include <QDialog>
#include <QString>
#include <QVector>
#include <exception>
#include <functional>
#include <thread>
#include <variant>

#include "Pager.h"

class QLabel;
class QListWidget;
class QProgressBar;
class QPushButton;

namespace uwf::ui {

namespace dialogs {
class FileDialogProvider;
}

// 单条覆盖层文件条目。
//   rawName          —— WMI 直接返回的相对路径，可能带 ":$DATA" / ":$INDEX_ALLOCATION"
//                       等 NTFS 流后缀；只用于诊断回查（去重后会丢失，仅保留一条）。
//   absolutePath     —— 拼好盘符且去掉 NTFS 流后缀的路径，如 "C:\Users\foo"。
//                       UI 列表、右键菜单、commit、导出统一用这个值。
//   isDirectory      —— rawName 后缀为 ":$INDEX_ALLOCATION" 时为 true（NTFS 目录索引流）。
//                       去重时只要任一条原始条目是目录索引，合并后即视为目录。
//   isSystemMetadata —— absolutePath 的任一卷内路径段以 '$' 开头（NTFS 元数据
//                       文件 / $RECYCLE.BIN 等）。这类条目右键菜单不显示 commit——
//                       UWF_Volume.CommitFile 对元数据走不通，强行调用只会报
//                       NOT_FOUND，没有意义。
//   fileSize         —— 去重后是 raw 条目里所有同 absolutePath 的 size 饱和求和
//                       （同一文件的多条 NTFS 流加起来才是它在 overlay 占的总
//                       字节；畸形 provider 数据也不会发生无符号回绕）。
struct OverlayFileEntry {
  QString rawName;
  QString absolutePath;
  bool isDirectory = false;
  bool isSystemMetadata = false;
  qulonglong fileSize = 0;
};

using OverlayFileLoader = std::function<QVector<OverlayFileEntry>(const QString& driveLetter, std::stop_token stopToken)>;

struct OverlayFilesServices {
  OverlayFileLoader loader;
  dialogs::FileDialogProvider& fileDialogs;
};

// 异步展示某个 NTFS 卷当前 overlay 中缓存的文件列表。底层走
// UWF_Overlay.GetOverlayFiles——这调用很慢、按 overlay 用量近似指数级增长，
// 所以在 worker 线程上跑（使用该线程自己的 thread_local WMI session），结果
// 投回 UI 线程渲染。加载期间显示忙碌进度条；用户可以提前关闭对话框，
// worker 线程靠 QPointer 检查 dialog 是否还活着，活着再投递结果。
//
// 覆盖层文件可能成千上万，列表与日志查看器一样按页展示：条目全量留在
// m_entries，m_list 每次只渲染当前页，单页行数随窗口高度自适应、不出滚动条。
class OverlayFilesDialog : public QDialog {
  Q_OBJECT
 public:
  // 覆盖层枚举是本对话框唯一的外部数据依赖。调用方返回 WMI 的原始条目，
  // 路径规范化、去重、排序、分页和交互仍由对话框负责。生产构造函数使用
  // UWF_Overlay；此重载也允许其它只读数据源复用同一套 UI。loader 会被复制
  // 到工作线程并调用一次，必须可并发安全且在阻塞等待时响应 stopToken；对话框
  // 析构会请求停止并等待该调用退出。
  explicit OverlayFilesDialog(const QString& driveLetter, QWidget* parent = nullptr);
  // 自定义文件选择器但仍使用生产 WMI 数据源，供嵌入式宿主复用。
  OverlayFilesDialog(const QString& driveLetter, dialogs::FileDialogProvider& fileDialogs, QWidget* parent = nullptr);
  // 服务引用不转移所有权，生命周期必须覆盖本对话框；loader 按上方线程契约复制。
  OverlayFilesDialog(const QString& driveLetter, OverlayFilesServices services, QWidget* parent = nullptr);
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
  using LoadResult = std::variant<QVector<OverlayFileEntry>, std::exception_ptr>;

  void startLoading();
  // 线程边界用 exception_ptr 保留原始异常类型，UI 线程在这里显式重抛并展示。
  // 成功数据与失败互斥，不再用空字符串/HRESULT 哨兵值组合控制流。
  void onLoadFinished(LoadResult result);
  // 把 m_pager.currentPage 指向的那一页条目渲染进 m_list，并刷新页码标签 / 导航按钮。
  void renderPage();
  // viewport 高度变化时按实测行高重算每页行数；行数变了才重渲染。
  void recomputePageSize();
  // 实测一行（含 QSS padding / border）的像素高度，量到一次后缓存复用。
  int rowHeight();
  // absolutePath 已是规范化绝对路径，不依赖任何实例状态，故为 static。
  static void openContainingFolder(const QString& absolutePath);

  QString m_driveLetter;  // 形如 "C:"
  dialogs::FileDialogProvider& m_fileDialogs;
  OverlayFileLoader m_loader;
  QListWidget* m_list = nullptr;
  QProgressBar* m_progress = nullptr;
  QLabel* m_loadingLabel = nullptr;
  QLabel* m_summary = nullptr;
  QPushButton* m_exportBtn = nullptr;

  // 分页导航控件与状态。与日志查看器一致：条目全量留在 m_entries，
  // m_list 每次只渲染一页，单页行数随 viewport 高度走、不出垂直滚动条。
  QLabel* m_pageInfo = nullptr;
  QPushButton* m_firstBtn = nullptr;
  QPushButton* m_prevBtn = nullptr;
  QPushButton* m_nextBtn = nullptr;
  QPushButton* m_lastBtn = nullptr;
  Pager m_pager;          // pageSize/currentPage + 分页算术（pageSize 占位 1，首个 resize 按 viewport 修正）
  int m_rowHeight = 0;    // 实测行高缓存，0 = 尚未测量
  bool m_loaded = false;  // 加载成功后才在页码标签里显示统计

  // 加载完成后的条目副本（已规范化 + 排序），导出与分页渲染直接读这个，
  // 避免再走一遍 list widget 还原。
  QVector<OverlayFileEntry> m_entries;

  // std::jthread 的 stop_token 直接传入 WMI 原生异步调用；析构请求 stop 后，
  // worker 在发起调用的同一线程执行 CancelAsyncCall，再由 jthread 收束生命周期。
  std::jthread m_worker;
};

}  // namespace uwf::ui
