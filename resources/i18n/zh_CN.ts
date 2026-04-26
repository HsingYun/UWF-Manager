<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN" sourcelanguage="en">
<context>
    <name>uwf</name>

    <!-- Common verbs / states -->
    <message><source>Enable</source><translation>启用</translation></message>
    <message><source>Disable</source><translation>停用</translation></message>
    <message><source>enable</source><translation>启用</translation></message>
    <message><source>disable</source><translation>停用</translation></message>
    <message><source>Enabled</source><translation>启用</translation></message>
    <message><source>Disabled</source><translation>停用</translation></message>
    <message><source>OK</source><translation>确定</translation></message>
    <message><source>Cancel</source><translation>取消</translation></message>
    <message><source>Close</source><translation>关闭</translation></message>
    <message><source>Refresh</source><translation>刷新</translation></message>
    <message><source>Apply</source><translation>应用</translation></message>
    <message><source>Add</source><translation>添加</translation></message>
    <message><source>Remove</source><translation>移除</translation></message>
    <message><source>Keep</source><translation>保留</translation></message>
    <message><source>Already removed</source><translation>已移除</translation></message>
    <message><source>Yes</source><translation>是</translation></message>
    <message><source>No</source><translation>否</translation></message>
    <message><source>File…</source><translation>文件…</translation></message>
    <message><source>Folder…</source><translation>文件夹…</translation></message>
    <message><source>volume ID</source><translation>卷 ID</translation></message>
    <message><source>drive letter</source><translation>盘符</translation></message>
    <message><source>By drive letter</source><translation>按盘符</translation></message>
    <message><source>By volume ID</source><translation>按卷 ID</translation></message>

    <!-- main.cpp: SystemCheck 弹窗 -->
    <message>
        <source>Unified Write Filter (UWF) is only available on Windows.</source>
        <translation>统一写入筛选器 (UWF) 仅在 Windows 上可用。</translation>
    </message>
    <message>
        <source>UWF is only supported on Windows Enterprise, Education or IoT Enterprise editions.

Current system: %1 (%2)</source>
        <translation>统一写入筛选器仅支持 Windows 企业版、教育版或 IoT 企业版。

当前系统版本：%1（%2）</translation>
    </message>
    <message>
        <source>Unified Write Filter (UWF) was not detected. Open &quot;Control Panel → Programs → Turn Windows features on or off&quot;, enable &quot;Device Lockdown → Unified Write Filter&quot;, reboot, and try again.</source>
        <translation>未检测到统一写入筛选器 (UWF)。请到「控制面板 → 程序 → 启用或关闭 Windows 功能」中勾选「设备锁定 → 统一写入筛选器」，重启后再运行本程序。</translation>
    </message>
    <message><source>System version not supported</source><translation>系统版本不支持</translation></message>
    <message><source>UWF feature not enabled</source><translation>未启用 UWF 功能</translation></message>
    <message><source>Administrator privileges required</source><translation>需要管理员权限</translation></message>
    <message>
        <source>This program is not running as administrator.

Right-click and choose &quot;Run as administrator&quot; to restart it; otherwise UWF settings cannot be read or modified.</source>
        <translation>本程序当前未以管理员身份运行。

请右键选择「以管理员身份运行」重新启动本程序，否则无法读取或修改 UWF 设置。</translation>
    </message>

    <!-- StatusPanel -->
    <message>
        <source>Protection state of this volume in the current session (read-only).</source>
        <translation>本卷在当前会话中的保护状态（只读）。</translation>
    </message>
    <message>
        <source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source>
        <translation>下次会话保护本卷。本卷的所有写入都会被重定向到覆盖层，重启后丢弃。</translation>
    </message>
    <message><source>Protection:</source><translation>保护状态：</translation></message>
    <message>
        <source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source>
        <translation>UWF 识别本卷的方式。盘符直观，但增减其它磁盘等情况下盘符可能被重新分配，绑定会失效；卷 ID 不随盘符变化，更稳定。</translation>
    </message>
    <message><source>Bind by:</source><translation>绑定方式：</translation></message>
    <message>
        <source>This volume is not supported by UWF: </source>
        <translation>此卷不支持 UWF：</translation>
    </message>

    <!-- GlobalStatusPanel -->
    <message><source>Global settings</source><translation>全局设置</translation></message>
    <message>
        <source>UWF filter state in the current session (read-only).</source>
        <translation>UWF 筛选器在当前会话中的启用状态（只读）。</translation>
    </message>
    <message>
        <source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source>
        <translation>下次会话启用 UWF 筛选器。对受保护卷的所有写入都会被重定向到覆盖层，重启后丢弃。</translation>
    </message>
    <message><source>Filter state:</source><translation>筛选状态：</translation></message>
    <message><source>Filter</source><translation>筛选器</translation></message>
    <message>
        <source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source>
        <translation>覆盖层的类型 / 最大大小只能在筛选器停用时修改：
1. 用上方的「筛选状态」开关停用筛选器
2. 重启电脑（重启后筛选器为停用状态）
3. 再来修改此项</translation>
    </message>
    <message>
        <source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source>
        <translation>覆盖层的存放位置。RAM 速度快但占用内存；Disk 存放在系统盘，可用容量更大。重启后两者都会丢弃。</translation>
    </message>
    <message><source>Type</source><translation>类型</translation></message>
    <message>
        <source>Maximum overlay capacity. In RAM mode, capped by total system memory.</source>
        <translation>覆盖层的最大容量。RAM 模式下受系统总内存限制。</translation>
    </message>
    <message><source>Maximum size</source><translation>最大大小</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>最大大小 · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>警告阈值</translation></message>
    <message>
        <source>Triggers a warning when overlay usage reaches this value. Recommended: 50–70% of maximum size.</source>
        <translation>覆盖层占用达到此值时弹出警告。建议设为最大大小的 50%–70%。</translation>
    </message>
    <message><source>Critical threshold</source><translation>严重阈值</translation></message>
    <message>
        <source>Triggers a critical warning when overlay usage reaches this value; the system may force a flush or reboot.</source>
        <translation>覆盖层占用达到此值时弹出严重警告，系统可能强制刷盘或重启。</translation>
    </message>
    <message><source>Used / total</source><translation>已用 / 占用</translation></message>
    <message><source>Overlay</source><translation>覆盖层</translation></message>
    <message>
        <source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source>
        <translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; 已占用 &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; 警告 &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; 严重</translation>
    </message>
    <message><source>UWF status unavailable: </source><translation>UWF 状态不可用：</translation></message>

    <!-- DiskTab -->
    <message>
        <source>Unsupported drive type (only fixed local disks are supported).</source>
        <translation>不支持的驱动器类型（仅支持本地固定磁盘）。</translation>
    </message>
    <message>
        <source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source>
        <translation>%1 文件系统：可以保护此卷，但不支持文件排除和单文件提交。</translation>
    </message>
    <message><source>Failed to read volume information.</source><translation>读取卷信息失败。</translation></message>
    <message><source>%1 free / %2</source><translation>%1 可用 / %2</translation></message>
    <message><source>Commit</source><translation>提交</translation></message>
    <message>
        <source>Commit overlay changes to disk / registry. This action cannot be undone.</source>
        <translation>把覆盖层中的修改提交到磁盘 / 注册表。此操作不可撤销。</translation>
    </message>
    <message><source>Commit file changes…</source><translation>提交文件修改…</translation></message>
    <message>
        <source>Pick a file and commit its overlay changes to disk.</source>
        <translation>选择一个文件，把它在覆盖层中的修改提交到磁盘。</translation>
    </message>
    <message><source>Commit folder changes…</source><translation>提交文件夹修改…</translation></message>
    <message>
        <source>Pick a folder and commit overlay changes for every file inside it to disk.</source>
        <translation>选择一个目录，把里面所有文件在覆盖层中的修改提交到磁盘。</translation>
    </message>
    <message><source>Commit file deletion…</source><translation>提交文件删除…</translation></message>
    <message>
        <source>Enter the path of a file that has already been deleted in the current session, and commit the deletion to disk.</source>
        <translation>手动输入一个在当前会话已被删除的文件路径，把这次「删除」操作提交到磁盘。</translation>
    </message>
    <message><source>Commit registry changes…</source><translation>提交注册表修改…</translation></message>
    <message>
        <source>Enter a registry key (and optional value name) and commit changes to the registry.</source>
        <translation>输入注册表键（可选值名），把修改提交到注册表。</translation>
    </message>
    <message><source>File exclusions</source><translation>文件排除</translation></message>
    <message>
        <source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source>
        <translation>本卷不受 UWF 保护的文件 / 目录列表。双击条目可复制路径。</translation>
    </message>
    <message><source>Registry exclusions</source><translation>注册表排除</translation></message>
    <message>
        <source>Global registry exclusion list (shared across volumes; shown only on the system drive). Double-click an entry to copy its path.</source>
        <translation>UWF 全局的注册表排除列表（跨卷共享，仅在系统盘上展示）。双击条目可复制路径。</translation>
    </message>
    <message>
        <source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source>
        <translation>UWF 筛选器当前已停用，覆盖层中不会累积修改，没有可提交的内容。</translation>
    </message>
    <message>
        <source>Per-file commit is not supported on the %1 file system.</source>
        <translation>%1 文件系统不支持单文件提交。</translation>
    </message>
    <message>
        <source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source>
        <translation>%1 文件系统不支持单文件提交，只能提交注册表修改（注册表是全局的）。</translation>
    </message>
    <message>
        <source>This volume is not currently protected by UWF; there are no file changes to commit.</source>
        <translation>本卷当前未受 UWF 保护，没有可提交的文件修改。</translation>
    </message>
    <message>
        <source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source>
        <translation>本卷当前未受 UWF 保护，只能提交注册表修改（注册表是全局的）。</translation>
    </message>
    <message>
        <source>Select a file to commit to disk</source>
        <translation>选择要提交到磁盘的文件</translation>
    </message>
    <message>
        <source>Select a folder to commit to disk</source>
        <translation>选择要提交到磁盘的目录</translation>
    </message>
    <message>
        <source>Enter the full path of the file whose deletion you want to commit (e.g. %1\Users\xxx\foo.txt).

The path must no longer exist in the current session — meaning it has already been deleted, leaving only a deletion marker in the overlay waiting to be written to disk.</source>
        <translation>输入要提交「删除」操作的完整文件路径（例如 %1\Users\xxx\foo.txt）。

该路径在当前会话中应该已经不存在——即在本次会话里已经把它删掉，覆盖层里只剩一个删除标记，等待写入磁盘。</translation>
    </message>
    <message><source>Commit file deletion</source><translation>提交文件删除</translation></message>
    <message><source>Commit registry changes</source><translation>提交注册表修改</translation></message>
    <message><source>Leave empty to commit the whole key</source><translation>留空则提交整个键</translation></message>
    <message><source>Registry key:</source><translation>注册表键：</translation></message>
    <message><source>Value name (optional):</source><translation>值名（可选）：</translation></message>

    <!-- OverlayFilesDialog -->
    <message><source>Overlay files - %1</source><translation>覆盖层文件 - %1</translation></message>
    <message><source>View overlay files</source><translation>查看覆盖层文件</translation></message>
    <message>
        <source>Open a diagnostic view of files currently cached in the overlay for this volume.</source>
        <translation>打开诊断视图，查看当前在本卷覆盖层中缓存的文件。</translation>
    </message>
    <message>
        <source>Overlay file listing is only supported on NTFS volumes.</source>
        <translation>仅 NTFS 卷支持查看覆盖层文件列表。</translation>
    </message>
    <message>
        <source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source>
        <translation>UWF 筛选器当前处于禁用状态，没有任何卷存在可查看的覆盖层。</translation>
    </message>
    <message>
        <source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source>
        <translation>本卷当前未被 UWF 保护，没有可查看的覆盖层。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source>
        <translation>&lt;b&gt;仅作诊断快照。&lt;/b&gt;请勿根据此列表决定要提交的内容。</translation>
    </message>
    <message><source>NTFS volumes only.</source><translation>仅支持 NTFS 卷。</translation></message>
    <message>
        <source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source>
        <translation>覆盖层较大时可能非常慢，甚至失败——内存占用和耗时都随覆盖层大小增长。</translation>
    </message>
    <message>
        <source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source>
        <translation>列表并不精确：小于磁盘簇大小（通常 4 KB）的文件可能缺失；此前的提交、排除路径下的文件，以及看似不相关操作影响到的文件都可能出现在列表里。</translation>
    </message>
    <message><source>Loading…</source><translation>加载中…</translation></message>
    <message><source>Failed: %1</source><translation>失败：%1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>覆盖层中共 %1 个文件</translation></message>
    <message>
        <source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure. Wait a few seconds and click &quot;View overlay files&quot; again, often it succeeds on retry.</source>
        <translation>WMI 提供程序在枚举覆盖层文件时崩溃。这是 UWF_Overlay.GetOverlayFiles 在覆盖层较大或磁盘 I/O 繁忙时的已知不稳定行为。等几秒后再点一次"查看覆盖层文件"，多数情况下重试就能成功。</translation>
    </message>
    <message>
        <source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source>
        <translation>内存不足或提供程序不支持此操作。覆盖层文件枚举仅在 NTFS 卷上可用，且需要一定的可用内存；请减小覆盖层容量或关闭占用内存较多的程序后再试。</translation>
    </message>
    <message><source>Raw error: %1</source><translation>原始错误：%1</translation></message>
    <message><source>Export to file…</source><translation>导出到文件…</translation></message>
    <message><source>Export overlay file list</source><translation>导出覆盖层文件列表</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>文本文件 (*.txt);;所有文件 (*)</translation></message>
    <message><source>Export failed</source><translation>导出失败</translation></message>
    <message><source>Export finished</source><translation>导出完成</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>无法打开文件写入：%1</translation></message>
    <message><source>Could not write file: %1</source><translation>无法写入文件：%1</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>已保存 %1 条记录到：
%2</translation></message>

    <!-- ExclusionListWidget -->
    <message>
        <source>Cannot add an entire volume (%1) to the exclusion list.</source>
        <translation>不允许把整个卷 %1 加入排除列表。</translation>
    </message>
    <message>
        <source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source>
        <translation>分页文件 / 交换文件 / 休眠文件不能加入排除（这些是 UWF 本身依赖的系统文件）。</translation>
    </message>
    <message>
        <source>The entire \Windows directory cannot be excluded.</source>
        <translation>不允许把整个 \Windows 目录加入排除。</translation>
    </message>
    <message>
        <source>The entire \Windows\System32 directory cannot be excluded.</source>
        <translation>不允许把整个 \Windows\System32 目录加入排除。</translation>
    </message>
    <message>
        <source>The entire \Windows\System32\Drivers directory cannot be excluded.</source>
        <translation>不允许把整个 \Windows\System32\Drivers 目录加入排除。</translation>
    </message>
    <message>
        <source>This critical system file cannot be excluded: %1</source>
        <translation>不允许把关键系统文件加入排除：%1</translation>
    </message>
    <message>
        <source>The per-user registry file NTUSER.DAT cannot be excluded.</source>
        <translation>不允许把用户注册表文件 NTUSER.DAT 加入排除。</translation>
    </message>
    <message><source>Registry key cannot be empty.</source><translation>注册表键不能为空。</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>注册表键不能以反斜杠开头。</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>路径含有连续反斜杠，无效。</translation></message>
    <message>
        <source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source>
        <translation>注册表路径使用反斜杠 `\` 作为分隔符，不要用正斜杠 `/`。</translation>
    </message>
    <message>
        <source>The path contains invisible control characters; this is not valid.</source>
        <translation>路径含有不可见字符，无效。</translation>
    </message>
    <message>
        <source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source>
        <translation>不允许把 HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC 加入排除（域机器账户密钥；UWF 文档明确禁止排除此键）。</translation>
    </message>
    <message>
        <source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source>
        <translation>UWF 只允许排除以下顶层注册表键的子项，请在这些键下面选择具体位置：
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation>
    </message>
    <message><source>Search file paths…</source><translation>搜索文件路径…</translation></message>
    <message><source>Search registry keys…</source><translation>搜索注册表项…</translation></message>
    <message>
        <source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source>
        <translation>把文件或文件夹加入排除列表。排除的对象不受 UWF 保护。</translation>
    </message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>选择一个文件加入排除列表。</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>选择一个文件夹（包含其中所有内容）加入排除列表。</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>手动输入一个注册表键路径加入排除列表。</translation></message>
    <message><source>Remove selected</source><translation>删除所选</translation></message>
    <message>
        <source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source>
        <translation>从排除列表中移除选中项（应用后生效）。</translation>
    </message>
    <message><source>Open containing folder</source><translation>打开所在文件夹</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>提交文件夹改动到磁盘…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>提交文件改动到磁盘…</translation></message>
    <message><source>Copied to clipboard: </source><translation>已复制到剪贴板：</translation></message>
    <message>
        <source>Select files to add to the exclusion list (multiple selection allowed)</source>
        <translation>选择要加入排除的文件（可多选）</translation>
    </message>
    <message>
        <source>Select a folder to add to the exclusion list</source>
        <translation>选择要加入排除的文件夹</translation>
    </message>
    <message><source>Add registry exclusion</source><translation>添加注册表排除</translation></message>
    <message>
        <source>Full registry key (e.g. HKLM\Software\MyApp):</source>
        <translation>完整注册表键（例如 HKLM\Software\MyApp）：</translation>
    </message>
    <message>
        <source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source>
        <translation>所选路径 %1 不在 %2 卷上，无法作为此卷的排除项。</translation>
    </message>
    <message><source>Path is not on this volume</source><translation>路径不在当前卷</translation></message>
    <message><source>Cannot add this exclusion</source><translation>不能添加此排除项</translation></message>
    <message>
        <source>Current session: %1
Next session (saved on disk): %2
Pending change: %3</source>
        <translation>当前会话：%1
下次会话（磁盘已保存）：%2
本次待应用：%3</translation>
    </message>
    <message>
        <source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source>
        <translation>共 %1 项 · 下次会话将新增 %2 · 移除 %3 · 待应用 %4</translation>
    </message>

    <!-- MainWindow: toolbar / about / theme -->
    <message><source>Unified Write Filter (UWF) Manager</source><translation>统一写入筛选器 (UWF) 管理器</translation></message>
    <message><source>Main toolbar</source><translation>主工具栏</translation></message>
    <message>
        <source>Re-read the current session state and next-session configuration of UWF.</source>
        <translation>重新读取 UWF 的当前会话状态和下次会话配置。</translation>
    </message>
    <message><source>Review and apply</source><translation>预览并应用</translation></message>
    <message>
        <source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source>
        <translation>预览本次所有待应用的变更，确认后一次性应用（多数变更在下次重启后生效）。</translation>
    </message>
    <message><source>Safe shutdown</source><translation>安全关机</translation></message>
    <message>
        <source>Shut down safely, even when the UWF overlay is full.</source>
        <translation>安全关机：即使 UWF 覆盖层已满也能正常关机。</translation>
    </message>
    <message><source>Safe restart</source><translation>安全重启</translation></message>
    <message>
        <source>Restart safely, even when the UWF overlay is full.</source>
        <translation>安全重启：即使 UWF 覆盖层已满也能正常重启。</translation>
    </message>
    <message><source>Log</source><translation>日志</translation></message>
    <message>
        <source>View the internal log accumulated during this session, for troubleshooting.</source>
        <translation>查看本次会话累积的内部日志（用于排查问题）。</translation>
    </message>
    <message><source>About</source><translation>关于</translation></message>
    <message><source>About this program.</source><translation>关于本程序。</translation></message>
    <message><source>Switch display language</source><translation>切换显示语言</translation></message>
    <message>
        <source>Toggle light / dark theme. Follows the system setting on startup.</source>
        <translation>切换浅色 / 深色主题。启动时跟随系统设置。</translation>
    </message>
    <message>
        <source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href=&quot;%3&quot;&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href=&quot;mailto:%1&quot;&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href=&quot;%2&quot;&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided &quot;as is&quot;, without any warranty.&lt;/p&gt;</source>
        <translation>&lt;p&gt;UWF 的图形化管理界面：筛选器状态、覆盖层、文件与注册表排除。多数变更在下次重启后生效。&lt;/p&gt;&lt;p&gt;源代码仓库：&lt;a href=&quot;%3&quot;&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href=&quot;mailto:%1&quot;&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;本程序采用 &lt;a href=&quot;%2&quot;&gt;GNU General Public License v3.0&lt;/a&gt; 协议发布；完整协议文本见随程序分发的 LICENSE 文件。&lt;/p&gt;&lt;p&gt;本程序为自由软件，您可以在 GPL v3 条款下重新分发或修改本程序。本程序按"原样"提供，不附带任何形式的担保。&lt;/p&gt;</translation>
    </message>
    <message><source>About UWF Manager</source><translation>关于 UWF 管理器</translation></message>

    <!-- Commit failure explanations -->
    <message>
        <source>The file is in use by another process and cannot be saved right now.</source>
        <translation>文件正被其他程序占用，暂时无法保存。</translation>
    </message>
    <message>
        <source>The file has no pending changes; nothing to save.</source>
        <translation>该文件没有改动，无需保存。</translation>
    </message>
    <message>
        <source>The path is invalid or improperly formatted.</source>
        <translation>路径无效或格式不正确。</translation>
    </message>
    <message>
        <source>System call failed (see log for details).</source>
        <translation>系统调用失败（详情见日志）。</translation>
    </message>
    <message><source>Operation rejected (code %1).</source><translation>操作被拒绝（代码 %1）。</translation></message>
    <message><source>Unknown cause.</source><translation>未知原因。</translation></message>

    <!-- Commit report dialog -->
    <message><source>Commit canceled</source><translation>提交已取消</translation></message>
    <message><source>Commit result</source><translation>提交结果</translation></message>
    <message><source>Skipped</source><translation>跳过</translation></message>
    <message><source>Failed</source><translation>失败</translation></message>
    <message>
        <source>%1 succeeded; %2 skipped; %3 failed.</source>
        <translation>成功 %1 个；跳过 %2 个；失败 %3 个。</translation>
    </message>
    <message><source>%1 files committed successfully.</source><translation>已成功提交 %1 个文件。</translation></message>
    <message>
        <source>
Canceled by user; %1 entries not processed.</source>
        <translation>
用户取消，剩余 %1 个未处理。</translation>
    </message>
    <message><source>Category</source><translation>类别</translation></message>
    <message><source>Path</source><translation>路径</translation></message>
    <message><source>Error code</source><translation>错误码</translation></message>
    <message><source>Reason</source><translation>原因</translation></message>
    <message><source>Copy selected rows</source><translation>复制选中行</translation></message>
    <message><source>Copy all</source><translation>复制全部</translation></message>
    <message><source>0 lines</source><translation>0 行</translation></message>
    <message><source>%1 lines</source><translation>共 %1 行</translation></message>
    <message><source>Loading log entries…</source><translation>正在加载日志…</translation></message>
    <message><source>No log entries</source><translation>无日志条目</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>第 %1 / %2 页 · 共 %3 行</translation></message>
    <message><source>Copy current page</source><translation>复制当前页</translation></message>
    <message><source>Clear</source><translation>清空</translation></message>

    <!-- MainWindow: status bar / tabs / refresh -->
    <message>
        <source>%1 pending change(s) (not yet written to the system)</source>
        <translation>有 %1 项待应用变更（尚未写入系统）</translation>
    </message>
    <message><source>No pending changes</source><translation>无待应用变更</translation></message>
    <message>
        <source> (System drive: also manages the global registry exclusion list here.)</source>
        <translation>（系统盘，也在这里管理全局注册表排除）</translation>
    </message>
    <message>
        <source>Switch to protection settings and file exclusions for volume %1.%2</source>
        <translation>切换到卷 %1 的保护设置与文件排除列表。%2</translation>
    </message>
    <message><source>Failed to read volume information</source><translation>读取卷信息失败</translation></message>
    <message>
        <source>Error: %1

Please verify that the UWF feature is enabled and that this program is running as administrator.</source>
        <translation>错误：%1

请确认已启用 UWF 功能，并以管理员身份运行。</translation>
    </message>
    <message><source>Failed to read UWF state</source><translation>读取 UWF 状态失败</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF 命名空间不可用</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>已刷新 · 共 %1 个卷</translation></message>

    <!-- showPlan: changeCmds / snapshotCmds -->
    <message><source>· Filter (global) %1</source><translation>· 筛选器（全局） %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· 覆盖层 类型 → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· 覆盖层 最大大小 → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· 覆盖层 警告阈值 → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· 覆盖层 严重阈值 → %1 MB</translation></message>
    <message>
        <source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source>
        <translation>⚠ 修改类型 / 最大大小前需先停用筛选器并重启，否则系统会拒绝。</translation>
    </message>
    <message><source>· Volume %1 protection %2</source><translation>· 卷 %1 保护 %2</translation></message>
    <message>
        <source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source>
        <translation>· 卷 %1 绑定方式 → %2（命令行不支持，仅本程序内可改）</translation>
    </message>
    <message><source>+ File exclusion  %1  %2</source><translation>+ 文件排除  %1  %2</translation></message>
    <message><source>− File exclusion  %1  %2</source><translation>− 文件排除  %1  %2</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ 注册表排除  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− 注册表排除  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>筛选器（全局） %1</translation></message>
    <message><source>Overlay type → %1</source><translation>覆盖层 类型 → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>覆盖层 最大大小 → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>覆盖层 警告阈值 → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>覆盖层 严重阈值 → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>卷 %1 保护 %2</translation></message>
    <message><source>File exclusion %1 %2</source><translation>文件排除 %1 %2</translation></message>
    <message><source>Registry exclusion %1</source><translation>注册表排除 %1</translation></message>
    <message><source>Pending changes</source><translation>待应用的变更</translation></message>
    <message><source>Current session configuration</source><translation>当前会话配置</translation></message>
    <message><source>Review and apply changes</source><translation>变更预览 · 应用</translation></message>
    <message>
        <source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source>
        <translation>以下以 uwfmgr 命令形式列出当前所有配置；若有&lt;span style='color:%1'&gt;待应用的变更&lt;/span&gt;会先单独成段。确认后点 &lt;span style='color:%2'&gt;应用&lt;/span&gt;，本程序会真实写入系统（多数在下次重启后生效）。</translation>
    </message>
    <message><source>Confirm apply</source><translation>确认应用</translation></message>
    <message>
        <source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source>
        <translation>即将&lt;span style='color:%1'&gt;真实写入系统&lt;/span&gt;，多数变更在下次重启后才生效。&lt;br&gt;&lt;br&gt;确定要继续吗？</translation>
    </message>

    <!-- showPlan commit lambda result lines -->
    <message><source>✘ Failed to connect to the system: %1</source><translation>✘ 连接系统失败：%1</translation></message>
    <message><source>Applied changes</source><translation>已应用的变更</translation></message>
    <message><source>Result</source><translation>应用结果</translation></message>
    <message><source>✘ Failed to read filter state: %1</source><translation>✘ 读取筛选器状态失败：%1</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ 筛选器：%1</translation></message>
    <message><source>✘ Failed to %1 filter: %2</source><translation>✘ %1筛选器失败：%2</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ 覆盖层 警告阈值设为 %1 MB</translation></message>
    <message><source>✘ Failed to set warning threshold: %1</source><translation>✘ 设置警告阈值失败：%1</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ 覆盖层 严重阈值设为 %1 MB</translation></message>
    <message><source>✘ Failed to set critical threshold: %1</source><translation>✘ 设置严重阈值失败：%1</translation></message>
    <message><source>✘ Failed to read overlay state: %1</source><translation>✘ 读取覆盖层信息失败：%1</translation></message>
    <message>
        <source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source>
        <translation>✘ 类型 / 最大大小未应用：筛选器当前已启用，需要先停用筛选器并重启后再修改。</translation>
    </message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ 覆盖层 类型设为 %1</translation></message>
    <message><source>✘ Failed to set overlay type: %1</source><translation>✘ 设置覆盖层类型失败：%1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ 覆盖层 最大大小设为 %1 MB</translation></message>
    <message><source>✘ Failed to set maximum size: %1</source><translation>✘ 设置最大大小失败：%1</translation></message>
    <message><source>✘ Failed to read overlay configuration: %1</source><translation>✘ 读取覆盖层配置失败：%1</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF: %2</source><translation>✘ 卷 %1：注册到 UWF 失败：%2</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ 卷 %1 保护：%2</translation></message>
    <message>
        <source>✘ Failed to %1 protection on volume %2: %3</source>
        <translation>✘ %1卷 %2 保护失败：%3</translation>
    </message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ 卷 %1 绑定方式：%2</translation></message>
    <message><source>✘ Failed to set binding for volume %1: %2</source><translation>✘ 卷 %1 绑定方式设置失败：%2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ 卷 %1 新增文件排除：%2</translation></message>
    <message>
        <source>✘ Volume %1 failed to add file exclusion %2: %3</source>
        <translation>✘ 卷 %1 新增文件排除 %2 失败:%3</translation>
    </message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ 卷 %1 移除文件排除：%2</translation></message>
    <message>
        <source>✘ Volume %1 failed to remove file exclusion %2: %3</source>
        <translation>✘ 卷 %1 移除文件排除 %2 失败：%3</translation>
    </message>
    <message><source>✘ Failed to read registry filter: %1</source><translation>✘ 读取注册表筛选器失败：%1</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ 新增注册表排除：%1</translation></message>
    <message><source>✘ Failed to add registry exclusion %1: %2</source><translation>✘ 新增注册表排除 %1 失败：%2</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ 移除注册表排除：%1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1: %2</source><translation>✘ 移除注册表排除 %1 失败：%2</translation></message>

    <!-- Log dialog -->
    <message><source>Time</source><translation>时间</translation></message>
    <message><source>Level</source><translation>级别</translation></message>
    <message><source>Tag</source><translation>TAG</translation></message>
    <message><source>Message</source><translation>内容</translation></message>

    <!-- Safe shutdown / restart -->
    <message>
        <source>The system will shut down safely.
Uncommitted changes in this session will be lost.

Continue?</source>
        <translation>即将安全关机。
未提交的会话内修改将丢失。

确定要继续吗？</translation>
    </message>
    <message><source>Safe shutdown failed</source><translation>安全关机失败</translation></message>
    <message><source>Failed to read filter state: %1</source><translation>读取筛选器状态失败：%1</translation></message>
    <message><source>Shutdown failed: %1</source><translation>关机失败：%1</translation></message>
    <message>
        <source>The system will restart safely.
Uncommitted changes in this session will be lost.

Continue?</source>
        <translation>即将安全重启。
未提交的会话内修改将丢失。

确定要继续吗？</translation>
    </message>
    <message><source>Safe restart failed</source><translation>安全重启失败</translation></message>
    <message><source>Restart failed: %1</source><translation>重启失败：%1</translation></message>

    <!-- commitFilePath -->
    <message><source>Commit failed</source><translation>提交失败</translation></message>
    <message>
        <source>The path has no drive letter; cannot identify the target volume.</source>
        <translation>路径缺少盘符，无法确定目标卷。</translation>
    </message>
    <message>
        <source>Failed to read volume information: %1</source>
        <translation>读取卷信息失败：%1</translation>
    </message>
    <message>
        <source>No current-session record found for volume %1.</source>
        <translation>找不到卷 %1 的当前会话记录。</translation>
    </message>
    <message><source>Commit rejected</source><translation>提交被拒绝</translation></message>
    <message>
        <source>This path is in the file exclusion list. UWF does not write it to the overlay, so committing it to disk is neither needed nor possible.

Target: %1
Exclusion: %2</source>
        <translation>该路径已在文件排除列表中，UWF 不会把它写入覆盖层，因此也无需（也无法）提交到磁盘：

目标：%1
排除项：%2</translation>
    </message>
    <message><source>Nothing to commit</source><translation>无文件可提交</translation></message>
    <message><source>No files were found under %1.</source><translation>目录 %1 下递归遍历到 0 个文件。</translation></message>
    <message><source>Commit to disk</source><translation>提交到磁盘</translation></message>
    <message>
        <source>Recursively walk the folder and commit %1 files to disk one by one. This action cannot be undone.

%2

Continue?</source>
        <translation>将递归遍历目录，对其中 %1 个文件逐个提交到磁盘。此操作不可撤销。

%2

确定要继续吗？</translation>
    </message>
    <message>
        <source>Commit the following path from the overlay to disk. This action cannot be undone.

%1

Continue?</source>
        <translation>将把下面路径从覆盖层提交到磁盘。此操作不可撤销。

%1

确定要继续吗？</translation>
    </message>
    <message><source>Committing…</source><translation>正在提交…</translation></message>

    <!-- commitFileDeletionPath -->
    <message><source>Commit file deletion failed</source><translation>提交文件删除失败</translation></message>
    <message><source>Path still exists</source><translation>该路径仍然存在</translation></message>
    <message>
        <source>Commit file deletion requires the path to no longer exist in the current session — that is, the file has already been deleted in this session, leaving only a deletion marker in the overlay waiting to be written to disk.

However, the following path is still visible:

%1

If you want to delete a currently visible file and commit the deletion, delete it in File Explorer first, then return here to commit the deletion.</source>
        <translation>「提交文件删除」要求该路径在当前会话中已经不存在（即在本次会话里已经把它删掉，覆盖层里只剩一个删除标记，等着写入磁盘）。

但下面这个路径现在还能看到：

%1

如果你是想把已经看得到的文件删掉并提交，请先在资源管理器里把它删掉，再来这里提交删除。</translation>
    </message>
    <message>
        <source>This path is in the file exclusion list. UWF does not track its deletion in the overlay, so committing the deletion is meaningless.

Target: %1
Exclusion: %2</source>
        <translation>该路径已在文件排除列表中，UWF 不会为它维护覆盖层中的删除标记，所以「提交删除」没有意义：

目标：%1
排除项：%2</translation>
    </message>
    <message>
        <source>Commit the deletion of the following file to disk. This action cannot be undone.

%1

Continue?</source>
        <translation>将把下面文件的「删除」操作提交到磁盘，此操作不可撤销：

%1

确定要继续吗？</translation>
    </message>

    <!-- commitRegistryKey -->
    <message>
        <source>This key is in the registry exclusion list. UWF does not write it to the overlay, so committing it to disk is neither needed nor possible.

Target: %1
Exclusion: %2</source>
        <translation>该键已在注册表排除列表中，UWF 不会把它写入覆盖层，因此也无需（也无法）提交到磁盘：

目标：%1
排除项：%2</translation>
    </message>
    <message>
        <source>Key: %1
(empty value name → commit the entire key)</source>
        <translation>键：%1
（值名为空 → 提交整项）</translation>
    </message>
    <message>
        <source>Key: %1
Value: %2</source>
        <translation>键：%1
值：%2</translation>
    </message>
    <message>
        <source>Commit the following registry entry to disk. This action cannot be undone.

%1

Continue?</source>
        <translation>将把下面的注册表项提交到磁盘，此操作不可撤销：

%1

确定要继续吗？</translation>
    </message>
    <message>
        <source>Failed to read registry filter: %1</source>
        <translation>读取注册表筛选器失败：%1</translation>
    </message>
    <message>
        <source>No current-session registry filter record found.</source>
        <translation>找不到当前会话的注册表筛选记录。</translation>
    </message>
    <message><source>Failed to write registry: %1</source><translation>写入注册表失败：%1</translation></message>
    <message><source>Committed: %1</source><translation>已提交：%1</translation></message>

    <!-- showPlan: Export commands button -->
    <message><source>Export commands…</source><translation>导出命令…</translation></message>
    <message><source>Export commands to file</source><translation>导出命令到文件</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>已导出 %1 条命令到：
%2</translation></message>

    <!-- toolbar: Import button -->
    <message><source>Import</source><translation>导入</translation></message>
    <message>
        <source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click &quot;Review and apply&quot;.</source>
        <translation>粘贴、键入或从文件加载一段 uwfmgr 命令脚本，把每一行翻译成 UI 上的待应用变更。点击「预览并应用」之前不会真的写入系统。</translation>
    </message>

    <!-- ImportDialog -->
    <message><source>Import uwfmgr commands</source><translation>导入 uwfmgr 命令</translation></message>
    <message>
        <source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source>
        <translation>&lt;p&gt;在下方粘贴或键入 &lt;b&gt;uwfmgr&lt;/b&gt; 命令，每行一条。支持的类别：&lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;。&lt;/p&gt;&lt;p&gt;点击 &lt;b&gt;从文件加载…&lt;/b&gt; 选择任意文本类文件（日志、脚本、.txt / .bat / .ps1），其中包含 &lt;code&gt;uwfmgr&lt;/code&gt; 的行会被追加到下面的输入框。&lt;/p&gt;&lt;p&gt;点击 &lt;b&gt;导入&lt;/b&gt; 会把每条命令转成 UI 上的待应用变更——&lt;b&gt;此时还不会写入系统&lt;/b&gt;。需要真正生效请用工具栏上的 &lt;b&gt;预览并应用&lt;/b&gt;。&lt;/p&gt;</translation>
    </message>
    <message><source>Load from file…</source><translation>从文件加载…</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>选择包含 uwfmgr 命令的文件</translation></message>
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>所有文件 (*);;文本文件 (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
    <message><source>uwfmgr filter enable
uwfmgr overlay set-type RAM
uwfmgr volume protect C:
uwfmgr file add-exclusion &quot;C:\Users\foo\bar.txt&quot;
uwfmgr registry add-exclusion HKLM\Software\MyApp</source><translation>uwfmgr filter enable
uwfmgr overlay set-type RAM
uwfmgr volume protect C:
uwfmgr file add-exclusion &quot;C:\Users\foo\bar.txt&quot;
uwfmgr registry add-exclusion HKLM\Software\MyApp</translation></message>
    <message><source>#</source><translation>#</translation></message>
    <message><source>Status</source><translation>状态</translation></message>
    <message><source>Command</source><translation>命令</translation></message>
    <message><source>Detail</source><translation>详情</translation></message>
    <message><source>Import failed</source><translation>导入失败</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>内部错误：未注册命令应用器。</translation></message>
    <message><source>Nothing to import</source><translation>没有可导入的内容</translation></message>
    <message><source>The text area is empty.</source><translation>文本框为空。</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>输入中未找到任何 uwfmgr 命令。</translation></message>
    <message><source>Applied: %1</source><translation>已应用：%1</translation></message>
    <message><source>Duplicates: %1</source><translation>重复：%1</translation></message>
    <message><source>Failed: %1</source><translation>失败：%1</translation></message>
    <message><source>Unsupported: %1</source><translation>不支持：%1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>累计 %1 批后：</translation></message>
    <message><source>── Batch %1 ──</source><translation>── 第 %1 批 ──</translation></message>
    <message><source>Applied</source><translation>已应用</translation></message>
    <message><source>Duplicate</source><translation>重复</translation></message>
    <message><source>Failed</source><translation>失败</translation></message>
    <message><source>Unsupported</source><translation>不支持</translation></message>
    <message><source>File too large</source><translation>文件过大</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>文件 %1 超过 5 MB，未被解析；请先手动筛选后再加载。</translation></message>
    <message><source>Cannot read file</source><translation>无法读取文件</translation></message>
    <message><source>Could not open file %1: %2</source><translation>无法打开文件 %1：%2</translation></message>

    <!-- ImportDialog: parse errors -->
    <message><source>Incomplete uwfmgr command</source><translation>uwfmgr 命令不完整</translation></message>
    <message><source>Missing size argument (MB)</source><translation>缺少大小参数（MB）</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>大小必须是非负的 MB 整数</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>缺少覆盖层类型参数（RAM 或 Disk）</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>未知的覆盖层类型 %1（应为 RAM 或 Disk）</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>缺少卷参数（如 C:）</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>卷必须是盘符形式（如 C:）</translation></message>
    <message><source>Missing path argument</source><translation>缺少路径参数</translation></message>
    <message><source>Missing registry key argument</source><translation>缺少注册表键参数</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>不支持的 uwfmgr 命令（无法翻译成 UI 操作）</translation></message>

    <!-- showImport applier: per-command results -->
    <message><source>Queued as a pending %1 change</source><translation>已加入待应用的%1变更</translation></message>
    <message><source>Already in the target state — no-op</source><translation>已经处于目标状态——无须变更</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>路径不在本卷，或本卷不支持文件排除（如 exFAT / ReFS）</translation></message>
    <message><source>Rejected by UWF&apos;s blacklist (system file / Windows / pagefile / etc.)</source><translation>被 UWF 黑名单拒绝（系统文件 / Windows / 分页文件等）</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>本次导入中此前已出现过相同命令</translation></message>
    <message><source>Pending filter %1</source><translation>待应用：筛选器%1</translation></message>
    <message><source>Filter is already in the target state</source><translation>筛选器已经处于目标状态</translation></message>
    <message><source>Pending overlay type → %1</source><translation>待应用：覆盖层类型 → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>覆盖层类型已经是 %1</translation></message>
    <message><source>Invalid size value: %1</source><translation>大小数值非法：%1</translation></message>
    <message><source>maximum size</source><translation>最大大小</translation></message>
    <message><source>warning threshold</source><translation>警告阈值</translation></message>
    <message><source>critical threshold</source><translation>严重阈值</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>待应用：覆盖层 %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>覆盖层 %1 已经是 %2 MB</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>未知的卷 %1（没有该盘符对应的 UWF 可保护磁盘）</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>待应用：卷 %1 保护%2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>卷 %1 的保护状态已是目标值</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>路径 %1 没有盘符，无法定位到卷 TAB</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>没有盘符 %1 对应的 UWF 可保护磁盘</translation></message>
    <message><source>file exclusion</source><translation>文件排除</translation></message>
    <message><source>registry exclusion</source><translation>注册表排除</translation></message>
    <message><source>Registry exclusions are only available on the system drive tab, which is not present</source><translation>注册表排除只挂在系统盘 TAB 上，但该 TAB 不存在</translation></message>
    <message><source>Unsupported command</source><translation>不支持的命令</translation></message>
</context>
</TS>
