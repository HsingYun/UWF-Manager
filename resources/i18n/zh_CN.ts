<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS[]>
<TS version="2.1" language="zh_CN" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>预览</translation></message>

    <!-- Common verbs / states -->
    <message><source>enable</source><translation>启用</translation></message>
    <message><source>disable</source><translation>停用</translation></message>
    <message><source>Enabled</source><translation>已启用</translation></message>
    <message><source>Disabled</source><translation>已停用</translation></message>
    <message><source>OK</source><translation>确定</translation></message>
    <message><source>Cancel</source><translation>取消</translation></message>
    <message><source>Close</source><translation>关闭</translation></message>
    <message><source>Refresh</source><translation>刷新</translation></message>
    <message><source>Add</source><translation>添加</translation></message>
    <message><source>Remove</source><translation>移除</translation></message>
    <message><source>Yes</source><translation>是</translation></message>
    <message><source>No</source><translation>否</translation></message>
    <message><source>Excluded</source><translation>排除</translation></message>
    <message><source>Not excluded</source><translation>不排除</translation></message>
    <message><source>File…</source><translation>文件…</translation></message>
    <message><source>Folder…</source><translation>文件夹…</translation></message>
    <message><source>volume ID</source><translation>卷 ID</translation></message>
    <message><source>drive letter</source><translation>盘符</translation></message>
    <message><source>Drive letter</source><translation>盘符</translation></message>
    <message><source>Volume ID</source><translation>卷 ID</translation></message>

    <!-- main.cpp: 兼容模式提示 -->

    <!-- StatusPanel -->
    <message>
        <source>Protection state of this volume in the current session (read-only).</source>
        <translation>本卷在当前会话中的保护状态（只读）。</translation>
    </message>
    <message>
        <source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source>
        <translation>下次会话保护本卷。本卷的所有写入都会被重定向到覆盖层，重启后丢弃。</translation>
    </message>
    <message><source>Current session</source><translation>当前会话</translation></message>
    <message><source>Next session</source><translation>下次启动</translation></message>
    <message>
        <source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source>
        <translation>UWF 识别本卷的方式。盘符直观，但增减其他磁盘等情况下盘符可能被重新分配，绑定会失效；卷 ID 不随盘符变化，更稳定。</translation>
    </message>
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
    <message>
        <source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source>
        <translation>覆盖层的类型 / 最大大小只能在筛选器停用时修改：
1. 用上方的「筛选状态」开关停用筛选器
2. 重启电脑（重启后筛选器为停用状态）
3. 重新尝试修改此项</translation>
    </message>
    <message>
        <source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source>
        <translation>覆盖层的存放位置。RAM 速度快但占用内存；Disk 存放在系统盘，可用容量更大。重启后两者都会丢弃。</translation>
    </message>
    <message><source>Type</source><translation>类型</translation></message>
    <message>
        <source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source>
        <translation>覆盖层的最大容量。RAM 模式下受系统总内存限制。Disk 模式要求至少 1024 MB，且系统盘可用空间需大于该值。</translation>
    </message>
    <message><source>Maximum size</source><translation>最大大小</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>最大大小 · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>警告阈值</translation></message>
    <message>
        <source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source>
        <translation>覆盖层占用达到此值时向事件日志写入警告级别通知。两个事件均启用时，此值必须低于严重阈值。设为 0 只关闭警告事件。</translation>
    </message>
    <message><source>Critical threshold</source><translation>严重阈值</translation></message>
    <message>
        <source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source>
        <translation>覆盖层占用达到此值时向事件日志写入严重级别通知。两个事件均启用时，此值必须高于警告阈值。设为 0 只关闭严重事件。</translation>
    </message>
    <message><source>Overlay</source><translation>覆盖层</translation></message>

    <!-- DiskTab -->
    <message>
        <source>Unsupported drive type (only fixed local disks are supported).</source>
        <translation>不支持的驱动器类型（仅支持本地固定磁盘）。</translation>
    </message>
    <message>
        <source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source>
        <translation>卷容量超过 UWF 单个受保护卷 16 TiB 的上限。</translation>
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
    <message><source>Delete and commit file…</source><translation>删除并提交文件…</translation></message>
    <message>
        <source>Pick a file to delete, and commit the deletion to disk.</source>
        <translation>选择一个要删除的文件，把这次删除提交到磁盘。</translation>
    </message>
    <message><source>Delete and commit folder…</source><translation>删除并提交文件夹…</translation></message>
    <message>
        <source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source>
        <translation>选择一个要删除的文件夹（连同其内部所有内容一并递归删除），把这些删除提交到磁盘。</translation>
    </message>
    <message><source>Commit registry changes…</source><translation>提交注册表修改…</translation></message>
    <message>
        <source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source>
        <translation>输入注册表键（可选值名），把修改提交到注册表。值名留空则递归提交该键及其所有子键中的每一个值。</translation>
    </message>
    <message><source>Delete and commit registry…</source><translation>删除并提交注册表…</translation></message>
    <message>
        <source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source>
        <translation>输入要删除的注册表键（可选值名），把这次删除提交到注册表。值名留空则递归删除整棵子树（该键、它的所有值、以及它所有的子键）。</translation>
    </message>
    <message><source>File exclusions</source><translation>文件排除</translation></message>
    <message>
        <source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source>
        <translation>本卷不受 UWF 保护的文件 / 目录列表。双击条目可复制路径。</translation>
    </message>
    <message><source>Registry exclusions</source><translation>注册表排除</translation></message>
    <message>
        <source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source>
        <translation>注册表排除是全局的：由全局 UWF 筛选器开关控制，与本卷的保护状态无关。该列表跨所有卷共享，并且只会在磁盘标签页中显示一次。双击条目可复制路径。</translation>
    </message>
    <message><source>File staging</source><translation>文件暂存</translation></message>
    <message>
        <source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source>
        <translation>列表中的文件和文件夹会在安全关机或重启前自动提交，文件夹将递归处理。列表中存在数据时，UWF Manager 会排除 HKLM\SOFTWARE\HsingYun\UWF Manager，以确保该列表在受保护会话之间保持有效。</translation>
    </message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>将文件或文件夹加入自动提交列表。</translation></message>
    <message>
        <source>Select one or more files to commit automatically before safe shutdown or restart.</source>
        <translation>选择一个或多个文件，在安全关机或重启前自动提交。</translation>
    </message>
    <message>
        <source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source>
        <translation>选择一个文件夹，在安全关机或重启前递归提交其中的文件。</translation>
    </message>
    <message><source>Select files for automatic commit</source><translation>选择需要自动提交的文件</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>选择需要自动递归提交的文件夹</translation></message>
    <message><source>Cannot stage this path</source><translation>无法暂存此路径</translation></message>
    <message>
        <source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source>
        <translation>该路径与 UWF 文件排除项存在重叠。请先移除以下排除项，再添加暂存路径：
%1</translation>
    </message>
    <message>
        <source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source>
        <translation>该路径与文件暂存项存在重叠。请先移除以下暂存路径，再添加排除项：
%1</translation>
    </message>
    <message><source>The path overlaps an existing file staging entry</source><translation>路径与现有文件暂存项存在重叠</translation></message>
    <message>
        <source>The registry exclusion is required while File staging contains saved entries</source>
        <translation>文件暂存中存在已保存的条目时，必须保留该注册表排除项</translation>
    </message>
    <message>
        <source>The selected path does not exist or is not the expected file type:
%1</source>
        <translation>所选路径不存在，或其类型与选择操作不符：
%1</translation>
    </message>
    <message>
        <source>The selected path could not be inspected:
%1</source>
        <translation>无法检查所选路径：
%1</translation>
    </message>
    <message>
        <source>Reparse points cannot be added to automatic file staging:
%1</source>
        <translation>不能将重解析点加入文件暂存：
%1</translation>
    </message>
    <message>
        <source>The path has no drive letter and cannot be committed by UWF:
%1</source>
        <translation>此路径没有盘符，UWF 无法提交：
%1</translation>
    </message>
    <message>
        <source>The selected path is on volume %1. Add it from that volume's File staging tab.</source>
        <translation>所选路径位于 %1 卷，请在该卷的“文件暂存”页中添加。</translation>
    </message>
    <message>
        <source>A volume root cannot be staged because it would recursively commit the entire volume.</source>
        <translation>不能暂存卷根目录，否则会递归提交整个卷。</translation>
    </message>
    <message><source>File staging could not be saved</source><translation>无法保存文件暂存列表</translation></message>
    <message>
        <source>The file staging state was reloaded after the update could not be completed:
%1</source>
        <translation>更新未能完成，已重新加载文件暂存的实际状态：
%1</translation>
    </message>
    <message><source>File staging is unavailable: %1</source><translation>文件暂存不可用：%1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>将在安全关机或重启前自动提交。</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>%1 个暂存项 · %2 个文件 · %3 个文件夹</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>文件暂存依赖此注册表排除项，以便在重启后保留暂存列表。</translation></message>
    <message><source>Built-in</source><translation>内建</translation></message>
    <message>
        <source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source>
        <translation>UWF 筛选器当前已停用，覆盖层中不会累积修改，没有可提交的内容。</translation>
    </message>
    <message>
        <source>Per-file commit is not supported on the %1 file system.</source>
        <translation>%1 文件系统不支持单文件提交。</translation>
    </message>
    <message>
        <source>This volume is not supported by UWF.</source>
        <translation>此卷不受 UWF 支持。</translation>
    </message>
    <message>
        <source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source>
        <translation>此卷不支持文件提交。只能提交注册表修改（注册表排除是全局的）。</translation>
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
        <source>Select a file whose deletion you want to commit</source>
        <translation>选择要提交删除的文件</translation>
    </message>
    <message>
        <source>Select a folder whose deletion you want to commit</source>
        <translation>选择要提交删除的文件夹</translation>
    </message>

    <!-- RegistryPickerDialog -->
    <message><source>Name</source><translation>名称</translation></message>
    <message><source>Data</source><translation>数据</translation></message>
    <message><source>(Default)</source><translation>（默认）</translation></message>
    <message><source>(value not set)</source><translation>（数值未设置）</translation></message>
    <message><source>(empty)</source><translation>（空）</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>无法预览（值过大）</translation></message>

    <!-- OverlayFilesDialog -->
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
    <message><source>Failed: %1</source><translation>失败 %1</translation></message>
    <message>
        <source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source>
        <translation>WMI 提供程序在枚举覆盖层文件时崩溃。这是 UWF_Overlay.GetOverlayFiles 在覆盖层较大或磁盘 I/O 繁忙时的已知不稳定行为。</translation>
    </message>
    <message>
        <source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source>
        <translation>内存不足或提供程序不支持此操作。覆盖层文件枚举仅在 NTFS 卷上可用，且需要一定的可用内存；请减小覆盖层容量或关闭占用内存较多的程序后再试。</translation>
    </message>
    <message><source>Raw error: %1</source><translation>原始错误：%1</translation></message>
    <message><source>Export overlay file list</source><translation>导出覆盖层文件列表</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>文本文件 (*.txt);;所有文件 (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>命令文件 (*.cmd);;所有文件 (*)</translation></message>
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
    <message>
        <source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source>
        <translation>把文件或文件夹加入排除列表。排除的对象不受 UWF 保护。</translation>
    </message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>选择一个文件加入排除列表。</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>选择一个文件夹（包含其中所有内容）加入排除列表。</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>手动输入一个注册表键路径加入排除列表。</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>把一个注册表键加入排除列表，或开启一个持久化开关。</translation></message>
    <message><source>Registry key…</source><translation>注册表键…</translation></message>
    <!-- 持久化开关名：source 直接写「英文全称 (英文短键)」，中文环境翻成
         「中文术语 (英文短键)」。列表伪条目 / 添加菜单项 / 应用计划全程共用同一个
         I18n key，口径一致。菜单项不再带「启用 / Enable」前缀——按钮本身已叫
         「添加 / Add」，前缀语义重复。 -->
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>域密钥 (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>终端服务客户端访问许可证 (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>跨 UWF 会话持久化域机密密钥（计算机账户密码）。</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>跨 UWF 会话持久化终端服务客户端访问许可证。</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ %1 持久化：%2</translation></message>
    <message><source>Remove selected</source><translation>移除所选项</translation></message>
    <message>
        <source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source>
        <translation>从排除列表中移除选中项（应用后生效）。</translation>
    </message>
    <message><source>Open containing folder</source><translation>打开所在文件夹</translation></message>
    <message><source>Copy file path</source><translation>复制文件路径</translation></message>
    <message><source>Copy registry path</source><translation>复制注册表路径</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>提交文件夹改动到磁盘…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>提交文件改动到磁盘…</translation></message>
    <message><source>Copied to clipboard</source><translation>已复制到剪贴板</translation></message>
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
        <source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source>
        <translation>所选路径 %1 不在 %2 卷上，无法作为此卷的排除项。</translation>
    </message>
    <message><source>Path is not on this volume</source><translation>路径不在当前卷</translation></message>
    <message><source>Cannot add this exclusion</source><translation>不能添加此排除项</translation></message>
    <message><source>File: %1</source><translation>文件：%1</translation></message>
    <message><source>Folder: %1</source><translation>文件夹：%1</translation></message>
    <message><source>Registry: %1</source><translation>注册表：%1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>默认排除，可随时更改。</translation></message>
    <message>
        <source>Current session: %1
Next session: %2</source>
        <translation>当前会话：%1
下次会话：%2</translation>
    </message>
    <message><source>Pending change: %1</source><translation>本次待应用：%1</translation></message>

    <!-- MainWindow: toolbar / about / theme -->
    <message><source>Unified Write Filter (UWF) Manager</source><translation>统一写入筛选器 (UWF) 管理器</translation></message>
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
    <message><source>Direct restart</source><translation>直接重启</translation></message>
    <message><source>Direct shutdown</source><translation>直接关机</translation></message>
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
    <message><source>Switch display language</source><translation>切换显示语言</translation></message>
    <message><source>Language</source><translation>语言</translation></message>
    <message><source>Theme</source><translation>主题</translation></message>
    <message>
        <source>Toggle light / dark theme</source>
        <translation>切换浅色 / 深色主题</translation>
    </message>
    <message><source>System information</source><translation>系统信息</translation></message>
    <message><source>Diagnostic snapshot</source><translation>诊断快照</translation></message>
    <message><source>Read only</source><translation>只读</translation></message>

    <!-- Commit failure explanations -->
    <message>
        <source>The target was not found; there is nothing in the overlay to commit.</source>
        <translation>未找到目标，覆盖层中没有可提交的内容。</translation>
    </message>
    <message>
        <source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source>
        <translation>目标不在物理卷 / 持久化注册表上——它只存在于覆盖层中（在 UWF 保护下新建，从未提交过）。重启后会自动消失，无需也无法用「提交删除」处理。</translation>
    </message>
    <message>
        <source>A parameter was rejected by the system (invalid path or argument).</source>
        <translation>系统拒绝了传入的参数（路径或参数非法）。</translation>
    </message>
    <message>
        <source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source>
        <translation>操作失败。目标可能正被其他程序占用（如资源管理器正在浏览该目录、文件被某个程序打开）。请关闭占用它的程序后重试。</translation>
    </message>
    <message>
        <source>The operation failed (see log for details).</source>
        <translation>操作失败（详情见日志）。</translation>
    </message>
    <message><source>Operation rejected (code %1).</source><translation>操作被拒绝（代码 %1）。</translation></message>
    <message><source>Unknown cause.</source><translation>未知原因。</translation></message>

    <!-- Commit report dialog -->
    <message><source>Commit canceled</source><translation>提交已取消</translation></message>

    <message><source>Succeeded</source><translation>成功</translation></message>
    <message><source>Skipped</source><translation>跳过</translation></message>
    <message><source>Failed</source><translation>失败</translation></message>
    <message>
        <source>%1 succeeded; %2 skipped; %3 failed.</source>
        <translation>成功 %1 个；跳过 %2 个；失败 %3 个。</translation>
    </message>

    <message><source>Existed before</source><translation>删除前</translation></message>
    <message><source>Exists after</source><translation>删除后</translation></message>
    <message><source>Error code</source><translation>错误码</translation></message>
    <message><source>Reason</source><translation>原因</translation></message>
    <message><source>Previous page</source><translation>上一页</translation></message>
    <message><source>First page</source><translation>首页</translation></message>
    <message><source>Last page</source><translation>末页</translation></message>
    <message><source>Next page</source><translation>下一页</translation></message>
    <message><source>Copy selected rows</source><translation>复制选中行</translation></message>
    <message><source>Copy all</source><translation>复制全部</translation></message>
    <message>
        <source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source>
        <translation>文件暂存中存在已保存的条目时必须保留此注册表排除项。请先清空文件暂存列表，再移除此项。</translation>
    </message>
    <message><source>Copy current page</source><translation>复制当前页</translation></message>

    <!-- MainWindow: status bar / tabs / refresh -->
    <message><source>No pending changes</source><translation>无待应用变更</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation>（同时管理全局注册表排除）</translation></message>
    <message>
        <source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source>
        <translation>切换到卷 %1 的保护设置、文件排除与文件暂存。%2</translation>
    </message>
    <message><source>UWF namespace is not available</source><translation>UWF 命名空间不可用</translation></message>

    <!-- showPlan: changeCmds / snapshotCmds -->
    <message><source>Review and apply changes</source><translation>预览并应用</translation></message>
    <message><source>Confirm apply</source><translation>确认应用</translation></message>

    <!-- showPlan commit lambda result lines -->
    <message><source>Result</source><translation>结果</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ 筛选器：%1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ 覆盖层 警告阈值设为 %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ 覆盖层 严重阈值设为 %1 MB</translation></message>
    <message>
        <source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source>
        <translation>✘ 类型 / 最大大小未应用：筛选器当前已启用，需要先停用筛选器并重启后再修改。</translation>
    </message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ 覆盖层 类型设为 %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ 覆盖层 最大大小设为 %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ 最大大小未应用：基于磁盘的覆盖层要求至少 %1 MB。</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ 卷 %1 保护：%2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ 卷 %1 绑定方式：%2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ 卷 %1 新增文件排除：%2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ 卷 %1 移除文件排除：%2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ 新增注册表排除：%1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ 移除注册表排除：%1</translation></message>

    <!-- Log dialog -->
    <message><source>Time</source><translation>时间</translation></message>
    <message><source>Level</source><translation>等级</translation></message>
    <message><source>Tag</source><translation>TAG</translation></message>
    <message><source>Message</source><translation>消息</translation></message>

    <!-- Safe shutdown / restart -->
    <message><source>Calculating files for automatic commit…</source><translation>正在计算需要自动提交的文件…</translation></message>
    <message><source>Committing staged files</source><translation>正在提交暂存文件</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>已处理 %1 / %2 个文件。</translation></message>
    <message><source>No additional error details were provided.</source><translation>未提供更多错误详情。</translation></message>
    <message>
        <source>Automatic file staging stopped unexpectedly:
%1</source>
        <translation>文件暂存意外中止：
%1</translation>
    </message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>文件暂存因未知错误中止。</translation></message>
    <message><source>File staging will be skipped</source><translation>将跳过文件暂存区</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>直接重启会跳过文件暂存区提交。</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>直接关机会跳过文件暂存区提交。</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>已选择直接操作，因此跳过文件暂存区提交。</translation></message>
    <message><source>Safe shutdown failed</source><translation>安全关机失败</translation></message>
    <message><source>Shutdown failed: %1</source><translation>关机失败：%1</translation></message>
    <message><source>Safe restart failed</source><translation>安全重启失败</translation></message>
    <message><source>Direct restart failed</source><translation>直接重启失败</translation></message>
    <message><source>Direct shutdown failed</source><translation>直接关机失败</translation></message>
    <message><source>Restart failed: %1</source><translation>重启失败：%1</translation></message>
    <message>
        <source>The file staging list could not be read:
%1</source>
        <translation>无法读取文件暂存列表：
%1</translation>
    </message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>读取文件暂存列表时发生未知错误。</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>发现文件：%1 · 已提交：%2 · 已跳过文件：%3 · 已跳过条目：%4 · 失败：%5</translation></message>
    <message><source>File staging preparation</source><translation>文件暂存准备阶段</translation></message>
    <message><source>Reparse points are not supported.</source><translation>不支持重解析点。</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>仅可暂存本地卷上的绝对路径。</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>仅可将本地卷上的绝对路径加入自动文件暂存：
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>无法检查暂存路径。</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>暂存文件当前已变为其他路径类型。</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>暂存文件夹当前已变为其他路径类型。</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>无法完整枚举暂存文件夹。</translation></message>
    <message><source>The path has no drive letter.</source><translation>路径没有盘符。</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>不能递归处理卷根目录。</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>UWF 提供程序拒绝了提交请求。</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>提交因未知错误失败。</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>另有 %1 项失败已写入日志。</translation></message>

    <!-- commitFilePath -->

    <message>
        <source>The path has no drive letter; cannot identify the target volume.</source>
        <translation>路径缺少盘符，无法确定目标卷。</translation>
    </message>

    <message>
        <source>No current-session record found for volume %1.</source>
        <translation>找不到卷 %1 的当前会话记录。</translation>
    </message>
    <message>
        <source>This path is in the file exclusion list.
Exclusion: %1</source>
        <translation>此路径在文件排除列表中。
排除项：%1</translation>
    </message>

    <!-- confirmCommit dialog -->

    <message><source>Continue</source><translation>继续</translation></message>
    <message><source>Delete and commit</source><translation>删除并提交</translation></message>

    <!-- commitFileDeletionPath -->

    <!-- commitRegistryKey -->
    <message>
        <source>This key is in the registry exclusion list.
Exclusion: %1</source>
        <translation>此键在注册表排除列表中。
排除项：%1</translation>
    </message>

    <message>
        <source>No current-session registry filter record found.</source>
        <translation>找不到当前会话的注册表筛选记录。</translation>
    </message>

    <!-- commitRegistryDeletionKey -->

    <!-- showPlan: Export commands button -->
    <message><source>Export commands to file</source><translation>导出命令到文件</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>已导出 %1 条命令到：
%2</translation></message>

    <!-- toolbar: Import button -->
    <message><source>Import</source><translation>导入</translation></message>
    <message>
        <source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source>
        <translation>粘贴、键入或从文件加载一段 uwfmgr 命令脚本，把每一行翻译成 UI 上的待应用变更。点击「预览并应用」之前不会真的写入系统。</translation>
    </message>

    <!-- ImportDialog -->
    <message><source>Load from file…</source><translation>从文件加载</translation></message>
    <message><source>Load recommended configuration</source><translation>推荐配置</translation></message>
    <message><source>Choose recommended configuration</source><translation>选择推荐配置</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>选择要追加的推荐配置分组。导入前仍可检查或删除单条命令。</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>来自 Microsoft 官方文档。</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Microsoft 推荐的 UWF 排除规则</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>导入前请先检查这些推荐项；文件夹需要已存在，UWF 才会接受文件排除。</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>来源：Microsoft 官方 UWF 文档，包括常见写入筛选器排除项以及反恶意软件支持。</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>客户体验改善计划（CEIP）</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP：持久化策略参与状态</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP：持久化本机参与状态</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP：持久化上传禁用标志</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>后台智能传输服务（BITS）</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS：持久化下载队列文件</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS：持久化传输状态索引</translation></message>
    <message><source>Network profiles and policies</source><translation>网络配置与策略</translation></message>
    <message><source>Wireless network GPO policy</source><translation>无线网络 GPO 策略</translation></message>
    <message><source>Wired network GPO policy</source><translation>有线网络 GPO 策略</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>无线网络 GPO 策略文件</translation></message>
    <message><source>Wired network GPO policy files</source><translation>有线网络 GPO 策略文件</translation></message>
    <message><source>Wireless network interface profiles</source><translation>无线网络接口配置</translation></message>
    <message><source>Wired network interface profiles</source><translation>有线网络接口配置</translation></message>
    <message><source>Wireless service configuration</source><translation>无线服务配置</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>移动宽带服务配置</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>有线自动配置服务配置</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>网络配置 XML 文件因设备而异；如有需要，请手动添加具体的 Interfaces\{GUID}\{GUID}.xml 路径。</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>夏令时（DST）</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST：持久化时区定义</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST：持久化当前时区信息</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender：持久化程序文件和更新</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender：持久化 ProgramData 中的签名和状态</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender：持久化 Windows Update 日志</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender：持久化 MpCmdRun 日志</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender：持久化产品注册表状态</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender：持久化 WdBoot 服务状态</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender：持久化 WdFilter 服务状态</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender：持久化 WdNisSvc 服务状态</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender：持久化 WdNisDrv 服务状态</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender：持久化 WinDefend 服务状态</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP：持久化客户端程序文件</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP：持久化 Windows Update 日志</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP：持久化 MpCmdRun 日志</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP：持久化反恶意软件签名和状态</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP：持久化反恶意软件注册表状态</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>选择包含 uwfmgr 命令的文件</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>文本文件 (*.txt *.cmd);;所有文件 (*)</translation></message>
    <message><source>Status</source><translation>状态</translation></message>
    <message><source>Import failed</source><translation>导入失败</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>内部错误：未注册命令应用器。</translation></message>
    <message><source>Nothing to import</source><translation>没有可导入的内容</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>输入中未找到任何 uwfmgr 命令。</translation></message>
    <message><source>Duplicates: %1</source><translation>重复 %1</translation></message>
    <message><source>Unsupported: %1</source><translation>不支持 %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── 第 %1 批 ──</translation></message>
    <message><source>Duplicate</source><translation>重复</translation></message>
    <message><source>Unsupported</source><translation>不支持</translation></message>
    <message><source>File too large</source><translation>文件过大</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>文件 %1 超过 5 MB，未被解析；请先手动筛选后再加载。</translation></message>
    <message><source>Cannot read file</source><translation>无法读取文件</translation></message>
    <message><source>Could not open file %1: %2</source><translation>无法打开文件 %1：%2</translation></message>

    <!-- pending change guard -->
    <message><source>Discard pending changes?</source><translation>放弃待应用变更？</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>还有 %1 个待应用变更尚未应用。

继续并放弃这些变更？</translation></message>

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
    <message><source>Unclosed double quote in command</source><translation>命令中存在未闭合的双引号</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>存在多余参数：%1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>不支持的 uwfmgr 命令（无法翻译成 UI 操作）</translation></message>

    <!-- showImport applier: per-command results -->
    <message><source>Already in the target state — no-op</source><translation>已经处于目标状态——无须变更</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>路径不在本卷，或本卷不支持文件排除（如 exFAT / ReFS）</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>被 UWF 黑名单拒绝（系统文件 / Windows / 分页文件等）</translation></message>
    <message><source>Invalid size value: %1</source><translation>大小数值非法：%1</translation></message>
    <message><source>maximum size</source><translation>最大大小</translation></message>
    <message><source>warning threshold</source><translation>警告阈值</translation></message>
    <message><source>critical threshold</source><translation>严重阈值</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>全局配置当前不可用，未将此命令加入待应用更改。</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>未知的卷 %1（没有该盘符对应的 UWF 可保护磁盘）</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>卷 %1 的保护状态已是目标值</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>路径 %1 没有盘符，无法定位到卷 TAB</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>没有盘符 %1 对应的 UWF 可保护磁盘</translation></message>
    <message><source>file exclusion</source><translation>文件排除</translation></message>
    <message><source>registry exclusion</source><translation>注册表排除</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>注册表排除只能在注册表标签页中配置，但当前没有该标签页</translation></message>
    <message><source>Unsupported command</source><translation>不支持的命令</translation></message>

    <!-- System tray -->
    <message><source>Show or hide the overlay hub.</source><translation>显示或隐藏覆盖层 Hub。</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF：已启用</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF：已禁用</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF 状态不可用</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>已用 %1 MB / 总计 %2 MB</translation></message>

    <!-- Enhanced mode -->
    <message><source>Enhanced mode</source><translation>增强模式</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>将文件暂存自动提交与 Windows 关机、重启流程联动。</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>读取增强模式状态时发生未知错误。</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>无法读取服务的当前状态：
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>读取服务当前状态时发生未知错误。</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>服务未配置为独立进程。</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>服务未配置为自动启动。</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>服务未配置为以 LocalSystem 身份运行。</translation></message>
    <message><source>The service is not running.</source><translation>服务当前未运行。</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>服务配置的可执行文件路径或启动参数与当前程序不一致。</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>服务的预关机超时配置不正确。</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>服务未声明启动 UI 代理所需的权限。</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>服务尚未接受 Windows 预关机通知。</translation></message>
    <message><source>Enable enhanced mode</source><translation>启用增强模式</translation></message>
    <message><source>Disable enhanced mode</source><translation>停用增强模式</translation></message>
    <message><source>Start service</source><translation>启动服务</translation></message>
    <message><source>Repair enhanced mode</source><translation>修复增强模式</translation></message>
    <message><source>Delete service</source><translation>删除服务</translation></message>
    <message>
        <source>Stop and delete the enhanced mode service, including any remaining service registry data.</source>
        <translation>停止并删除增强模式服务，同时清理残留的服务注册表数据。</translation>
    </message>
    <message>
        <source>The service is absent, but its service registry data remains.</source>
        <translation>服务已不存在，但仍有服务注册表数据残留。</translation>
    </message>
    <message>
        <source>Waiting for the UI agent to complete authentication.</source>
        <translation>正在等待 UI 代理完成身份认证。</translation>
    </message>
    <message>
        <source>The service is running, but no authenticated UI agent is connected.</source>
        <translation>服务正在运行，但当前没有通过身份认证的 UI 代理连接。</translation>
    </message>
    <message>
        <source>UWF Manager enhanced mode helper service</source>
        <translation>UWF Manager 增强模式辅助服务</translation>
    </message>
    <message><source>The enhanced mode configuration was applied.</source><translation>增强模式配置已应用。</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>服务配置已应用，但 UWF 持久化操作报告了以下问题：
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>增强模式配置失败：
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>增强模式配置因未知错误而失败。</translation></message>
    <message>
        <source>The enhanced mode service and its remaining configuration were deleted.</source>
        <translation>增强模式服务及其残留配置已删除。</translation>
    </message>
    <message>
        <source>The service deletion was requested, but residual cleanup reported:
%1</source>
        <translation>已请求删除服务，但残留清理报告了以下问题：
%1</translation>
    </message>
    <message>
        <source>The enhanced mode service could not be deleted:
%1</source>
        <translation>无法删除增强模式服务：
%1</translation>
    </message>
    <message>
        <source>The enhanced mode service could not be deleted because of an unknown error.</source>
        <translation>删除增强模式服务时发生未知错误。</translation>
    </message>
    <message><source>File staging completed</source><translation>文件暂存提交完成</translation></message>
    <message><source>File staging failed</source><translation>文件暂存提交失败</translation></message>
    <message><source>All staged targets were processed.</source><translation>所有暂存目标均已处理。</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>所有暂存目标均已处理，但部分操作失败。</translation></message>

    <!-- Overlay floating window -->
    <message><source>Show main window</source><translation>显示主界面</translation></message>
    <message><source>Restore default position</source><translation>恢复默认位置</translation></message>
    <message><source>Show overlay hub</source><translation>显示覆盖层 HUB</translation></message>
    <message><source>Hide overlay hub</source><translation>隐藏覆盖层 HUB</translation></message>
    <message><source>Exit application</source><translation>退出应用</translation></message>
    <!-- 由 lupdate 校验出的当前源码文案。 -->
    <message><source>✘ Failed to connect to the system</source><translation>✘ 连接系统失败</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>读取注册表值失败：%1</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>枚举覆盖层文件时发生意外错误。</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ %1 筛选器失败</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ 读取筛选器状态失败</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ 设置警告阈值失败</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ 设置严重阈值失败</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ 读取覆盖层状态失败</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ 设置覆盖层类型失败</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ 设置最大大小失败</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ 读取覆盖层配置失败</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ 卷 %1：注册到 UWF 失败</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ 卷 %2 %1 保护失败</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ 设置卷 %1 的绑定方式失败</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ 卷 %1 添加文件排除项 %2 失败</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ 卷 %1 已不存在文件排除项：%2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ 卷 %1 移除文件排除项 %2 失败</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ 读取卷配置失败</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ 添加注册表排除项 %1 失败</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ 移除注册表排除项 %1 失败</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ 更新注册表持久化开关失败</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ 读取注册表筛选器失败</translation></message>
    <message><source>UWF is not available</source><translation>UWF 不可用</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>读取目标状态时发生未知错误。</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>目标已不存在，无需删除。</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>操作因未知错误而失败。</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>重新读取目标状态失败，无法确认操作结果：%1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>提供程序已接受删除请求，但重新读取权威状态后目标仍然存在。</translation></message>
    <message><source>Unknown</source><translation>未知</translation></message>
    <message><source>Failed to load logs: %1</source><translation>加载日志失败：%1</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>加载日志时发生意外错误。</translation></message>
    <message><source>: unknown error</source><translation>：未知错误</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>解析目标卷失败：%1</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>枚举注册表键失败：%1</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>无法解析路径 %1 所在的卷：%2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>前置操作失败或无法确认，已跳过此操作。</translation></message>
    <message><source>Ready</source><translation>就绪</translation></message>
    <message><source>Copy</source><translation>复制</translation></message>
    <message><source>Discard changes</source><translation>放弃变更</translation></message>
    <message><source>More</source><translation>更多</translation></message>
    <message><source>Minimize</source><translation>最小化</translation></message>
    <message><source>Maximize</source><translation>最大化</translation></message>
    <message><source>Protected</source><translation>已保护</translation></message>
    <message><source>Unprotected</source><translation>未保护</translation></message>
    <message><source>Current usage</source><translation>当前使用量</translation></message>
    <message><source>Configured</source><translation>已配置</translation></message>
    <message><source>On</source><translation>开启</translation></message>
    <message><source>Off</source><translation>关闭</translation></message>
    <message><source>Folder</source><translation>文件夹</translation></message>
    <message><source>File</source><translation>文件</translation></message>
    <message><source>File or folder</source><translation>文件或文件夹</translation></message>
    <message><source>%1 items</source><translation>%1 项</translation></message>
    <message><source>%1 selected</source><translation>已选择 %1 项</translation></message>
    <message><source>Pending removal</source><translation>待移除</translation></message>
    <message><source>Added</source><translation>新增</translation></message>
    <message><source>Warning %1 MB</source><translation>警告 %1 MB</translation></message>
    <message><source>Critical %1 MB</source><translation>严重 %1 MB</translation></message>
    <message><source>Administrator</source><translation>管理员</translation></message>
    <message><source>Read-only</source><translation>只读</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 项待应用</translation></message>
    <message><source>Import configuration</source><translation>导入配置</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>从脚本或文本中添加 UWF 配置。</translation></message>
    <message><source>One uwfmgr command per line</source><translation>每行一条 uwfmgr 命令</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr 命令</translation></message>
    <message><source>Editable</source><translation>可编辑</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>粘贴命令，或从文件加载…</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>导入会更新配置草稿。点击“预览并应用”可将更改写入系统。</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>报告列出了各条命令对配置草稿的处理结果。选择一行可查看完整命令和处理结果。</translation></message>
    <message><source>Edit commands</source><translation>编辑命令</translation></message>
    <message><source>Import results</source><translation>导入结果</translation></message>
    <message><source>Line</source><translation>行</translation></message>
    <message><source>Original command</source><translation>原始命令</translation></message>
    <message><source>Processing result</source><translation>处理结果</translation></message>
    <message><source>Imported</source><translation>已导入</translation></message>
    <message><source>Imported: %1</source><translation>已导入 %1</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>增强模式不可用；请使用安全关机或安全重启保存暂存文件。</translation></message>
    <message><source>Not written to the system</source><translation>尚未写入系统</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 行</translation></message>
    <message><source>Continue editing</source><translation>继续编辑</translation></message>
    <message><source>UWF filter</source><translation>UWF 筛选器</translation></message>
    <message><source>View overlay</source><translation>查看覆盖层</translation></message>
    <message><source>Binding</source><translation>绑定</translation></message>
    <message><source>Search paths</source><translation>搜索路径</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>不会保存暂存文件。</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>先将暂存文件提交到磁盘，再关机。</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>先将暂存文件提交到磁盘，再重启。</translation></message>
    <message><source>staged files</source><translation>个暂存文件</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>仅暂存文件会自动提交。其他受保护路径中的未提交更改将在关机后丢失。</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>仅暂存文件会自动提交。其他受保护路径中的未提交更改将在重启后丢失。</translation></message>
    <message><source>Ready to save</source><translation>即将保存</translation></message>
    <message><source>No files to commit</source><translation>没有待提交文件</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>文件较多时可能需要更长时间。</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>将直接执行 UWF 安全关机。</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>将直接执行 UWF 安全重启。</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 个文件</translation></message>
    <message><source>Saving staged files</source><translation>正在保存暂存文件</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>保存完成后将继续关机。请勿关闭程序。</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>保存完成后将继续重启。请勿关闭程序。</translation></message>
    <message><source>Saving to disk</source><translation>正在提交到磁盘</translation></message>
    <message><source>Saving…</source><translation>正在保存…</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>文件提交已开始，暂时无法取消。</translation></message>
    <message><source>Some files could not be saved</source><translation>部分文件未能保存</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>请查看失败详情，再决定是否继续关机。</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>请查看失败详情，再决定是否继续重启。</translation></message>
    <message><source>File saving incomplete</source><translation>文件提交未完成</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>继续关机将丢失未能保存的文件更改。</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>继续重启将丢失未能保存的文件更改。</translation></message>
    <message><source>Shut down anyway</source><translation>仍然关机</translation></message>
    <message><source>Restart anyway</source><translation>仍然重启</translation></message>
    <message><source>Counting staged files…</source><translation>正在统计暂存文件…</translation></message>
    <message><source>Scanning staged paths</source><translation>正在扫描暂存路径</translation></message>
    <message><source>View failure details</source><translation>查看失败详情</translation></message>
    <message><source>Version %1</source><translation>版本 %1</translation></message>
    <message><source>Source code</source><translation>源代码</translation></message>
    <message><source>Contact author</source><translation>联系作者</translation></message>
    <message><source>Copy version</source><translation>复制版本</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Windows 统一写入筛选器的图形管理工具。
管理卷保护、覆盖层、排除项与文件暂存。</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>写入保护由 Windows 内置 UWF 提供，本程序负责配置与管理。</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>首次启用 UWF 对系统的影响</translation></message>
    <message><source>License and copyright</source><translation>许可与版权</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows 会停用分页文件、系统还原、SuperFetch、文件索引、磁盘碎片整理和快速启动，并将 BCD 的 bootstatuspolicy 设为 ignoreallfailures。&lt;/p&gt;&lt;p&gt;之后可按需调整。例如，将分页文件移至未保护卷后重新启用分页。&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;采用 &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt; 发布，完整条款见程序随附的 LICENSE 文件。&lt;/p&gt;&lt;p&gt;可按 GPL v3 条款再分发或修改。软件按原样提供，不附带任何保证。&lt;/p&gt;</translation></message>
    <message><source>Cancel reading</source><translation>取消读取</translation></message>
    <message><source>Error details</source><translation>错误详情</translation></message>
    <message><source>Information</source><translation>信息</translation></message>
    <message><source>Warning</source><translation>警告</translation></message>
    <message><source>Error</source><translation>错误</translation></message>
    <message><source>Debug</source><translation>调试</translation></message>
    <message><source>Module</source><translation>模块</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>查看运行记录与诊断信息。</translation></message>
    <message><source>Log level</source><translation>日志等级</translation></message>
    <message><source>All levels</source><translation>全部等级</translation></message>
    <message><source>Current run</source><translation>本次运行</translation></message>
    <message><source>Search messages or modules</source><translation>搜索消息或模块</translation></message>
    <message><source>Search logs</source><translation>搜索日志</translation></message>
    <message><source>Message details</source><translation>消息详情</translation></message>
    <message><source>Clear…</source><translation>清空…</translation></message>
    <message><source>Clear logs?</source><translation>清空日志？</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>当前日志记录将被清空，无法恢复。</translation></message>
    <message><source>Clear logs</source><translation>清空日志</translation></message>
    <message><source>Reading logs</source><translation>正在读取日志</translation></message>
    <message><source>Unable to read logs</source><translation>无法读取日志</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>读取完成后将在此显示本次运行记录。</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>日志读取失败。请重试，或检查日志文件的访问权限。</translation></message>
    <message><source>Read again</source><translation>重新读取</translation></message>
    <message><source>No matching logs</source><translation>没有匹配的日志</translation></message>
    <message><source>No logs yet</source><translation>暂无日志</translation></message>
    <message><source>Try another keyword or log level.</source><translation>尝试调整关键词或日志等级。</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>新的运行记录将在刷新后显示。</translation></message>
    <message><source>Clear filters</source><translation>清除筛选</translation></message>
    <message><source>%1 records</source><translation>%1 条记录</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 条记录 · 共 %2 条</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>清空日志失败：%1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>清空日志失败：发生意外错误。</translation></message>
    <message><source>%1 items · %2</source><translation>%1 项 · %2</translation></message>
    <message><source>About this snapshot</source><translation>了解范围</translation></message>
    <message><source>Account</source><translation>运行账户</translation></message>
    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>该诊断快照未返回文件，不代表覆盖层中没有更改。</translation></message>
    <message><source>Application path</source><translation>程序路径</translation></message>
    <message><source>Automatic</source><translation>自动</translation></message>
    <message><source>Automatic file staging</source><translation>自动保存暂存文件</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>正常关机或重启时，自动保存符合条件的暂存文件。</translation></message>
    <message><source>Background service</source><translation>后台服务</translation></message>
    <message><source>Commit changes…</source><translation>提交更改…</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>接管正常关机与重启前的文件提交</translation></message>
    <message><source>Configuration error</source><translation>配置异常</translation></message>
    <message><source>Connected and authenticated</source><translation>已连接 · 身份已验证</translation></message>
    <message><source>Connecting…</source><translation>正在连接…</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>与 Windows 关机流程协同</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>增强模式会添加一个随 Windows 启动的后台服务。移动或删除程序前，请先移除该服务。</translation></message>
    <message><source>Error details and suggestions</source><translation>错误详情与建议</translation></message>
    <message><source>Export list</source><translation>导出列表</translation></message>
    <message><source>Local disk (%1)</source><translation>本地磁盘 (%1)</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>如果程序所在磁盘受 UWF 保护，启用前请先确保程序文件在重启后仍会保留。暂存文件较多时，关机或重启可能需要更长时间。</translation></message>
    <message><source>Name and location</source><translation>名称与位置</translation></message>
    <message><source>No overlay files found</source><translation>未发现覆盖层文件</translation></message>
    <message><source>Not configured</source><translation>尚未配置</translation></message>
    <message><source>Not connected</source><translation>未连接</translation></message>
    <message><source>Not installed</source><translation>尚未安装</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>请将程序放在普通用户无法修改的固定文件夹中。建议位置：</translation></message>
    <message><source>Operation and security</source><translation>运行与安全说明</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>重新读取以获取诊断快照。</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>大覆盖层的读取可能较慢，可随时取消。</translation></message>
    <message><source>Reading canceled</source><translation>已取消读取</translation></message>
    <message><source>Reading overlay</source><translation>正在读取覆盖层</translation></message>
    <message><source>Repair required</source><translation>需要修复</translation></message>
    <message><source>Retry</source><translation>重试</translation></message>
    <message><source>Running</source><translation>正在运行</translation></message>
    <message><source>Selected file</source><translation>所选文件</translation></message>
    <message><source>Selected folder</source><translation>所选文件夹</translation></message>
    <message><source>Service configuration needs repair</source><translation>服务配置需要修复</translation></message>
    <message><source>Service details</source><translation>服务详情</translation></message>
    <message><source>Service name</source><translation>服务名称</translation></message>
    <message><source>Service stopped</source><translation>服务已停止</translation></message>
    <message><source>Size</source><translation>大小</translation></message>
    <message><source>Starts automatically with Windows</source><translation>随 Windows 自动启动</translation></message>
    <message><source>Startup type</source><translation>启动类型</translation></message>
    <message><source>Status unavailable</source><translation>状态不可用</translation></message>
    <message><source>Stopped</source><translation>已停止</translation></message>
    <message><source>System metadata</source><translation>系统元数据</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>系统元数据仅供诊断，不提供提交操作。</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>UWF 提供程序未能完成枚举。覆盖层较大或系统资源不足时可能发生此问题。</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>列表可能遗漏或包含已提交文件，请勿将其作为提交清单。</translation></message>
    <message><source>UI agent</source><translation>界面代理</translation></message>
    <message><source>Unable to read service status</source><translation>服务状态读取失败</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>暂时无法读取覆盖层</translation></message>
    <message><source>Unavailable</source><translation>不可用</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>查看 %1 的文件诊断快照。</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>未启用时，仍可使用程序内的“安全关机”和“安全重启”保存暂存文件。</translation></message>
    <message>
        <source>A diagnostic snapshot of the system, privileges, and UWF.</source>
        <translation>系统、权限与 UWF 的诊断快照。</translation>
    </message>
    <message>
        <source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source>
        <translation>报告可能包含账户名、设备标识和文件路径，分享前请先检查。</translation>
    </message>
    <message>
        <source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source>
        <translation>部分硬件信息未能读取，其余诊断信息仍可查看和复制。</translation>
    </message>
    <message>
        <source>Overview</source>
        <translation>概览</translation>
    </message>
    <message>
        <source>Raw report</source>
        <translation>原始报告</translation>
    </message>
    <message>
        <source>Copy full report</source>
        <translation>复制完整报告</translation>
    </message>
    <message>
        <source>System</source>
        <translation>系统</translation>
    </message>
    <message>
        <source>Operating system</source>
        <translation>操作系统</translation>
    </message>
    <message>
        <source>Architecture</source>
        <translation>体系结构</translation>
    </message>
    <message>
        <source>System memory</source>
        <translation>系统内存</translation>
    </message>
    <message>
        <source>Permissions and protection</source>
        <translation>权限与保护</translation>
    </message>
    <message>
        <source>Privileges</source>
        <translation>运行权限</translation>
    </message>
    <message>
        <source>UWF feature</source>
        <translation>UWF 功能</translation>
    </message>
    <message>
        <source>Available</source>
        <translation>可用</translation>
    </message>
    <message>
        <source>Current filter</source>
        <translation>当前筛选器</translation>
    </message>
    <message>
        <source>Protected volumes</source>
        <translation>已保护卷</translation>
    </message>
    <message>
        <source>Service status</source>
        <translation>服务状态</translation>
    </message>
    <message>
        <source>Service account</source>
        <translation>服务账户</translation>
    </message>
    <message>
        <source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source>
        <translation>停用筛选器并重启后，可调整类型与大小。</translation>
    </message>
    <message>
        <source>Disk</source>
        <translation>磁盘</translation>
    </message>
    <message>
        <source>Protection and overlay</source>
        <translation>保护与覆盖层</translation>
    </message>
    <message>
        <source>Write protection</source>
        <translation>写入保护</translation>
    </message>
    <message>
        <source>Overlay type</source>
        <translation>覆盖层类型</translation>
    </message>
    <message>
        <source>Storage</source>
        <translation>存储介质</translation>
    </message>
    <message>
        <source>Overlay maximum size</source>
        <translation>覆盖层最大大小</translation>
    </message>
    <message>
        <source>Volume %1 protection</source>
        <translation>卷 %1 保护</translation>
    </message>
    <message>
        <source>By volume ID</source>
        <translation>按卷 ID</translation>
    </message>
    <message>
        <source>By drive letter</source>
        <translation>按盘符</translation>
    </message>
    <message>
        <source>Volume %1 binding</source>
        <translation>卷 %1 绑定</translation>
    </message>
    <message>
        <source>Volume identity</source>
        <translation>卷标识</translation>
    </message>
    <message>
        <source>Removed</source>
        <translation>移除</translation>
    </message>
    <message>
        <source>Future writes to this path will not be protected by UWF.</source>
        <translation>此路径的后续写入不受 UWF 保护。</translation>
    </message>
    <message>
        <source>Remove this path from the next-session exclusions.</source>
        <translation>从下次启动的排除项中移除此路径。</translation>
    </message>
    <message>
        <source>Future writes to this key will not be protected by UWF.</source>
        <translation>此键的后续写入不受 UWF 保护。</translation>
    </message>
    <message>
        <source>Remove this key from the next-session exclusions.</source>
        <translation>从下次启动的排除项中移除此键。</translation>
    </message>
    <message>
        <source>Domain Secret Key persistence</source>
        <translation>域密钥持久化</translation>
    </message>
    <message>
        <source>Terminal Services license persistence</source>
        <translation>终端服务许可证持久化</translation>
    </message>
    <message>
        <source>Review these changes before writing them to the system.</source>
        <translation>核对本次变更，确认后写入系统。</translation>
    </message>
    <message>
        <source>%1 changes</source>
        <translation>%1 项变更</translation>
    </message>
    <message>
        <source>Takes effect after restart</source>
        <translation>下次启动生效</translation>
    </message>
    <message>
        <source>View commands for these changes</source>
        <translation>查看本次变更的命令</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export commands in Full configuration.</source>
        <translation>配置与下一会话目标一致。可在“完整配置”中查看或导出命令。</translation>
    </message>
    <message>
        <source>Pending changes</source>
        <translation>待应用变更</translation>
    </message>
    <message>
        <source>Next-session configuration</source>
        <translation>下一会话配置</translation>
    </message>
    <message>
        <source>%1 commands</source>
        <translation>%1 条命令</translation>
    </message>
    <message>
        <source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source>
        <translation>将此 UTF-8 CMD 脚本保存为 .cmd 文件，并在命令提示符中运行。导出使用同一脚本。</translation>
    </message>
    <message>
        <source>Full configuration</source>
        <translation>完整配置</translation>
    </message>
    <message>
        <source>Apply changes</source>
        <translation>应用更改</translation>
    </message>
    <message>
        <source>Export commands</source>
        <translation>导出命令</translation>
    </message>
    <message>
        <source>Direct restart…</source>
        <translation>直接重启…</translation>
    </message>
    <message>
        <source>Some changes could not be applied</source>
        <translation>部分变更未能应用</translation>
    </message>
    <message>
        <source>Configuration applied</source>
        <translation>配置已应用</translation>
    </message>
    <message>
        <source>Review the operation details. Read the current system state before retrying changes that may have been written.</source>
        <translation>请查看操作详情；重新读取系统状态后，再修正未完成项。</translation>
    </message>
    <message>
        <source>The configuration has been written to the system. Most changes take effect after restart.</source>
        <translation>配置已写入系统，大部分变更在下次启动时生效。</translation>
    </message>
    <message>
        <source>Application result</source>
        <translation>应用结果</translation>
    </message>
    <message>
        <source>Write these configuration changes to the system? Most changes take effect after restart.</source>
        <translation>将这些配置变更写入系统？大部分配置将在下次启动时生效。</translation>
    </message>
    <message>
        <source>Overlay usage</source>
        <translation>覆盖层使用量</translation>
    </message>
    <message>
        <source>Standard user</source>
        <translation>标准用户</translation>
    </message>
    <message>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <source>Commit complete</source>
        <translation>提交完成</translation>
    </message>
    <message>
        <source>Review the result for each target.</source>
        <translation>查看各目标的操作结果。</translation>
    </message>

    <message>
        <source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source>
        <translation>失败项的更改尚未保存。选择一行查看完整路径和原因。</translation>
    </message>
    <message>
        <source>All changes have been saved to disk and will persist without a restart.</source>
        <translation>所有更改已提交到磁盘，无需重启即可持久保存。</translation>
    </message>
    <message>
        <source>Item details</source>
        <translation>项目详情</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>完成</translation>
    </message>
    <message>
        <source>Select a key to operate on its subtree, or a named value to operate on that value only.</source>
        <translation>选择键可操作整个子树，选择命名值仅操作此值。</translation>
    </message>
    <message>
        <source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source>
        <translation>选择要从 UWF 写入保护中排除的注册表键（包括子键和值）。</translation>
    </message>
    <message>
        <source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source>
        <translation>输入 HKLM\SOFTWARE\…，按 Enter 跳转</translation>
    </message>
    <message>
        <source>Navigate to registry path</source>
        <translation>转到注册表路径</translation>
    </message>
    <message>
        <source>Unable to access this registry key</source>
        <translation>无法访问此注册表键</translation>
    </message>
    <message>
        <source>Choose another key, or check access permissions and try again.</source>
        <translation>请选择其他键，或检查访问权限后重试。</translation>
    </message>
    <message>
        <source>Copy path</source>
        <translation>复制路径</translation>
    </message>
    <message>
        <source>Add exclusion</source>
        <translation>添加排除</translation>
    </message>
    <message>
        <source>No registry key selected</source>
        <translation>尚未选择注册表键</translation>
    </message>
    <message>
        <source>Select an available subkey</source>
        <translation>请选择可用子键</translation>
    </message>
    <message>
        <source>Global registry exclusion · includes subkeys</source>
        <translation>全局注册表排除项 · 包含子键</translation>
    </message>
    <message>
        <source>Named value: %1 · this value only</source>
        <translation>命名值：%1 · 仅操作此值</translation>
    </message>
    <message>
        <source>Whole key and all subkeys · recursive</source>
        <translation>整键及全部子键 · 递归操作</translation>
    </message>

    <message><source>Back</source><translation>返回上一个文件夹</translation></message>
    <message><source>Up one folder</source><translation>上一级文件夹</translation></message>
    <message><source>Refresh folder</source><translation>刷新文件夹</translation></message>
    <message><source>Folder address</source><translation>文件夹地址</translation></message>
    <message><source>Search this folder</source><translation>搜索此文件夹</translation></message>
    <message><source>Quick access</source><translation>快速访问</translation></message>
    <message><source>This PC</source><translation>此电脑</translation></message>
    <message><source>Folders</source><translation>文件夹</translation></message>
    <message><source>Files and folders</source><translation>文件与文件夹</translation></message>
    <message><source>New folder</source><translation>新建文件夹</translation></message>
    <message><source>File name</source><translation>文件名</translation></message>
    <message><source>File type</source><translation>文件类型</translation></message>
    <message><source>Selection</source><translation>所选项</translation></message>
    <message><source>Select the current folder</source><translation>选择当前文件夹</translation></message>
    <message><source>Select files or folders</source><translation>选择文件或文件夹</translation></message>
    <message><source>Enter a file name</source><translation>输入文件名</translation></message>
    <message><source>Select a file or enter its name</source><translation>选择文件或输入名称</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>可同时选择文件与文件夹</translation></message>
    <message><source>Select folders only.</source><translation>仅选择文件夹</translation></message>
    <message><source>Choose a location to save the file.</source><translation>选择文件保存位置</translation></message>
    <message><source>Multiple files can be selected.</source><translation>支持多选</translation></message>
    <message><source>Select a file to open.</source><translation>选择要加载的配置文件</translation></message>
    <message><source>Save</source><translation>保存</translation></message>
    <message><source>Open</source><translation>打开</translation></message>
    <message><source>Add selected items</source><translation>添加所选项</translation></message>
    <message><source>Select folder</source><translation>选择文件夹</translation></message>
    <message><source>Reading folder</source><translation>正在读取文件夹</translation></message>
    <message><source>Unable to access this folder</source><translation>无法访问此文件夹</translation></message>
    <message><source>This folder is empty</source><translation>此文件夹为空</translation></message>
    <message><source>No matching items</source><translation>没有匹配的项目</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>正在读取此位置的文件与子文件夹…</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>可以返回上一级，或选择其他位置。</translation></message>
    <message><source>Try another search term.</source><translation>尝试其他关键词。</translation></message>
    <message><source> · %1 selected</source><translation> · 已选择 %1 项</translation></message>
    <message><source>Unable to select this item</source><translation>无法选择此项目</translation></message>
    <message><source>Replace the existing file?</source><translation>替换现有文件？</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>此位置已存在文件：
%1

保存将覆盖原文件。</translation></message>
    <message><source>Replace file</source><translation>替换文件</translation></message>
    <message><source>Folder name</source><translation>文件夹名称</translation></message>
    <message><source>Create</source><translation>创建</translation></message>
    <message><source>Date modified</source><translation>修改日期</translation></message>
    <message><source>Add files or folders</source><translation>添加文件或文件夹</translation></message>
    <message><source>Unable to read this folder.</source><translation>无法读取此文件夹。</translation></message>
    <message><source>The destination folder does not exist.</source><translation>目标文件夹不存在。</translation></message>
    <message><source>Enter a valid file name.</source><translation>请输入有效的文件名。</translation></message>
    <message><source>The selected item no longer exists: %1</source><translation>所选项目已不存在：%1</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>请选择文件，或打开文件夹浏览其内容。</translation></message>
    <message><source>Select a folder.</source><translation>请选择文件夹。</translation></message>
    <message><source>Enter a valid folder name.</source><translation>请输入有效的文件夹名称。</translation></message>    <message><source>Current selection</source><translation>当前选择</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>改选整个键（含全部子键）</translation></message>
    <message><source>Select this registry key</source><translation>选择此注册表键</translation></message>
    <message><source>Select this registry value</source><translation>选择此注册表值</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>请选择限定磁盘或文件夹内的位置。</translation></message>
    <message><source>Registry key or persistence setting</source><translation>注册表项或持久化设置</translation></message>
    <message><source>Staged path</source><translation>暂存路径</translation></message>
    <message><source>Required configuration</source><translation>必要配置</translation></message>
    <message><source>Recursive staging</source><translation>递归暂存</translation></message>
    <message><source>Staged file</source><translation>暂存文件</translation></message>
    <message><source>Home</source><translation>主目录</translation></message>
    <message><source>Compatibility mode</source><translation>兼容模式</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>覆盖层已达到警告阈值。请保存更改，再安全重启。</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>覆盖层已达到严重阈值。请保存更改，并尽快安全重启。</translation></message>
    <message><source>Rename</source><translation>重命名</translation></message>
    <message><source>Rename…</source><translation>重命名…</translation></message>
    <message><source>Open folder</source><translation>打开文件夹</translation></message>
    <message><source>Select this file</source><translation>选择此文件</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>请输入有效的文件或文件夹名称。</translation></message>
    <message><source>Copy all logs</source><translation>复制全部日志</translation></message>
    <message><source>Copy name</source><translation>复制名称</translation></message>
    <message><source>Fit columns to contents</source><translation>根据内容调整列宽</translation></message>
    <message><source>Restore</source><translation>还原</translation></message>
    <message><source>Move</source><translation>移动</translation></message>
    <message><source>Expand</source><translation>展开</translation></message>
    <message><source>Collapse</source><translation>折叠</translation></message>
    <message><source>Undo removal</source><translation>撤销移除</translation></message>

    <message><source>32-bit</source><translation>32 位</translation></message>
    <message><source>64-bit</source><translation>64 位</translation></message>
    <message><source>All files (*)</source><translation>所有文件 (*)</translation></message>

    <message><source>Changes already committed are retained.</source><translation>已提交的更改会保留。</translation></message>
    <message><source>Commit changes</source><translation>提交更改</translation></message>
    <message><source>Commit file changes?</source><translation>提交文件更改？</translation></message>
    <message><source>Commit folder changes?</source><translation>提交文件夹更改？</translation></message>
    <message><source>Commit registry key changes?</source><translation>提交注册表键更改？</translation></message>
    <message><source>Commit registry value changes?</source><translation>提交注册表值更改？</translation></message>
    <message><source>Copy selection</source><translation>复制所选内容</translation></message>
    <message><source>Delete and commit this file?</source><translation>删除并提交文件？</translation></message>
    <message><source>Delete and commit this folder?</source><translation>删除并提交文件夹？</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>删除并提交注册表键？</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>删除并提交注册表值？</translation></message>

    <message><source>Direct restart?</source><translation>直接重启？</translation></message>
    <message><source>Direct shutdown?</source><translation>直接关机？</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>将跳过文件暂存。受保护路径中尚未提交的更改会在重启后丢失。</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>将跳过文件暂存。受保护路径中尚未提交的更改会在关机后丢失。</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>已处理 %1 / %2 项</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>当前目标处理完成后，可取消剩余任务。</translation></message>
    <message><source>Target</source><translation>目标</translation></message>

    <message><source>Select registry key or value</source><translation>选择注册表键或值</translation></message>
    <message><source>Select registry key or value to delete</source><translation>选择要删除的注册表键或值</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>服务已安装但未运行。启动服务以恢复自动保存。</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>无法读取增强模式服务状态。请检查服务权限后重试。</translation></message>
    <message><source>Committing</source><translation>提交中</translation></message>
    <message><source>Confirm before continuing</source><translation>请确认是否继续</translation></message>
    <message><source>Automatic saving incomplete</source><translation>自动保存未完成</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>请选择其他文件夹，或检查访问权限后重试。</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF 当前不可用</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>未能读取 UWF 配置。确认 Windows 已安装统一写入筛选器后重新读取。</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>启用并重启后显示实时用量</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>读取配置后显示实时用量</translation></message>
    <message><source>Filter is disabled</source><translation>筛选器未启用</translation></message>
    <message><source>Usage unavailable</source><translation>用量不可用</translation></message>
    <message><source>Select All</source><translation>全选</translation></message>
    <message><source>Display language</source><translation>显示语言</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>将所选路径标记为待移除，应用后生效。</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>应用后从自动暂存列表中移除此路径。</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>应用后将此路径添加到自动暂存列表。</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>应用此计划将清空已保存的暂存列表，并在不再需要时移除关联的注册表排除项。</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>暂存列表的更改须在 UWF 管理器中应用，无法导出为 uwfmgr 命令。</translation></message>
    <message><source>File staging is saved after Apply</source><translation>暂存列表在应用后保存</translation></message>
    <message><source>Overlay snapshot scope</source><translation>覆盖层快照的范围</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>此列表仅供诊断，不能用来判断哪些文件必须提交。</translation></message>
    <message><source>NTFS volumes only</source><translation>仅支持 NTFS 卷</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>大覆盖层可能读取缓慢或因资源不足而失败。</translation></message>
    <message><source>Small files may be missing</source><translation>可能遗漏小文件</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>小于磁盘簇大小（通常为 4 KB）的文件可能不在列表中。</translation></message>
    <message><source>Does not indicate pending commits</source><translation>不代表待提交状态</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>已提交文件、排除路径及受其他操作影响的项目也可能出现。</translation></message>

    <message><source>Command processing result</source><translation>命令处理结果</translation></message>
    <message><source>Copy command</source><translation>复制命令</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 个分支读取失败</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 个分支无法读取。可继续处理已发现内容，或仅重试失败项。</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 个文件 · %2 个目录</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 个值 · %2 个键</translation></message>
    <message><source>Attempts</source><translation>尝试次数</translation></message>
    <message><source>Cancel remaining tasks</source><translation>取消剩余任务</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>仅提交已成功遍历的内容；读取失败的分支保持原样。</translation></message>
    <message><source>Committing changes</source><translation>正在提交更改</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>已完成的删除及提交无法撤销。</translation></message>
    <message><source>Continue…</source><translation>继续…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>仅删除已成功遍历的目标并保存删除结果；读取失败的分支及其上级目录或键会保留。</translation></message>
    <message><source>Deleting and committing</source><translation>正在删除并提交</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>删除及提交无法撤销。仅处理所选范围，不会应用待应用的 UWF 配置。</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>正在确定操作范围。读取失败的分支会记录下来，并继续遍历其余内容。</translation></message>
    <message><source>Export traversal records</source><translation>导出遍历记录</translation></message>
    <message><source>No actionable content</source><translation>没有可操作的内容</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>所选范围中没有找到可操作的目标，没有执行删除或提交。</translation></message>
    <message><source>No changes have been committed.</source><translation>尚未提交任何更改</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>尚未删除或提交任何内容</translation></message>
    <message><source>Path</source><translation>路径</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>将永久删除所选范围，并将删除结果保存到受保护卷。</translation></message>
    <message><source>Preparation complete</source><translation>准备完成</translation></message>
    <message><source>Preparing commit</source><translation>准备提交</translation></message>
    <message><source>Preparing deletion and commit</source><translation>准备删除并提交</translation></message>
    <message><source>Process only successfully traversed content</source><translation>仅处理已成功遍历的内容</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>跳过重解析点，以确保遍历不超出所选范围。</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>提交后无法通过重启撤销。仅处理所选范围，不会应用待应用的 UWF 配置。</translation></message>
    <message><source>Retry failed items</source><translation>重试失败项</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>将所选范围的更改保存到受保护卷。</translation></message>
    <message><source>Stopping the operation</source><translation>正在停止操作</translation></message>
    <message><source>Successful and failed records are retained</source><translation>已保留成功与失败记录</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>文件夹不为空，未处理的内容已保留。</translation></message>
    <message><source>The operation could not be completed</source><translation>无法完成操作</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>注册表键中出现了新值，未处理的内容已保留。</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>注册表键中仍有子键，未处理的内容已保留。</translation></message>
    <message><source>The target no longer exists.</source><translation>目标已不存在。</translation></message>
    <message><source>Traversal complete</source><translation>遍历完成</translation></message>
    <message><source>Traversal completed with unread content</source><translation>遍历完成，部分内容未能读取</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>遍历记录已保留，可导出查看。</translation></message>
    <message><source>Traverse again</source><translation>重新遍历</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>可重新遍历，或关闭此窗口。</translation></message>
    <message><source>Traversing the folder</source><translation>正在遍历文件夹</translation></message>
    <message><source>Traversing the registry</source><translation>正在遍历注册表</translation></message>
    <message><source>Unable to prepare the operation</source><translation>无法完成操作准备</translation></message>
    <message><source>View results</source><translation>查看结果</translation></message>

    <message><source>All changes committed</source><translation>全部提交成功</translation></message>
    <message><source>Some changes committed</source><translation>部分提交成功</translation></message>
    <message><source>No changes could be committed</source><translation>提交失败，未能提交任何更改</translation></message>
    <message><source>No changes to commit</source><translation>没有需要提交的更改</translation></message>
    <message><source>Commit interrupted</source><translation>提交已中断</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 项未处理。</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 项注册表操作失败。</translation></message>
    <message><source>The folder does not exist.</source><translation>文件夹不存在。</translation></message>
    <message><source>Access denied.</source><translation>拒绝访问。</translation></message>
    <message><source>An item with this name already exists.</source><translation>已存在同名项目。</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>无法创建文件夹。请检查名称和权限。</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>无法重命名此项目。请检查名称和权限。</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>此浏览器不支持重命名。</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>打开此计划后，文件暂存草稿发生了变化。请重新查看计划。</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>打开此计划后，已保存的文件暂存列表发生了变化。请刷新并查看更改。</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>已保存的文件暂存列表与确认的计划不一致。</translation></message>
    <message><source>Export configuration</source><translation>导出配置</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>将当前主题、语言和 HUB 显示状态保存到程序所在目录的 UWF.ini。</translation></message>
    <message><source>Configuration saved</source><translation>配置已保存</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>已将当前界面设置保存到：
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini 已存在，未修改现有文件：
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>无法保存 UWF.ini：
%1
%2</translation></message>
    <message><source>Desktop</source><translation>桌面</translation></message>
    <message><source>Downloads</source><translation>下载</translation></message>
    <message><source>Documents</source><translation>文档</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>文件暂存已中止。请先解决错误，再重试。</translation></message>
    <message><source>No net change for this setting</source><translation>本次导入后，此设置的最终值未变</translation></message>
    <message><source>Final draft %1 → %2</source><translation>草稿最终值：%1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>草稿最终值：%1 → %2（请求值为 %3）</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>请求值：%1 → %2；已被后续命令替代。草稿最终值 → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>已更新%1草稿</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>已更新卷 %1 的保护设置草稿 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>请选择要移除的条目。</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>当前无法恢复所选条目。</translation></message>
</context>
</TS>
