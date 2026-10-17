<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>啟用</translation></message>
    <message><source>Disable</source><translation>停用</translation></message>
    <message><source>enable</source><translation>啟用</translation></message>
    <message><source>disable</source><translation>停用</translation></message>
    <message><source>Enabled</source><translation>啟用</translation></message>
    <message><source>Disabled</source><translation>停用</translation></message>
    <message><source>OK</source><translation>確定</translation></message>
    <message><source>Cancel</source><translation>取消</translation></message>
    <message><source>Close</source><translation>關閉</translation></message>
    <message><source>Refresh</source><translation>重新整理</translation></message>
    <message><source>Apply</source><translation>應用</translation></message>
    <message><source>Add</source><translation>新增</translation></message>
    <message><source>Remove</source><translation>移除</translation></message>
    <message><source>Yes</source><translation>是</translation></message>
    <message><source>No</source><translation>否</translation></message>
    <message><source>Excluded</source><translation>排除</translation></message>
    <message><source>Not excluded</source><translation>不排除</translation></message>
    <message><source>File…</source><translation>檔案…</translation></message>
    <message><source>Folder…</source><translation>資料夾…</translation></message>
    <message><source>volume ID</source><translation>磁碟區 ID</translation></message>
    <message><source>drive letter</source><translation>磁碟機代號</translation></message>
    <message><source>Drive letter</source><translation>按磁碟機代號</translation></message>
    <message><source>Volume ID</source><translation>按磁碟區 ID</translation></message>
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>當前系統「%1」（%2）不是已知的受支援 Windows 10/11 版本，UWF 管理器正以相容模式執行，部分功能可能不可用。</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>此磁碟區在當前工作階段中的保護狀態（只讀）。</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>下次工作階段保護此磁碟區。此磁碟區的所有寫入都會被重定向到覆寫層，重啟後丟棄。</translation></message>
    <message><source>Protection:</source><translation>保護狀態：</translation></message>
    <message><source>Current session</source><translation>目前工作階段</translation></message>
    <message><source>Next session</source><translation>下次工作階段</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>當前生效工作階段（只讀），使用者所作的修改均不會在本次工作階段生效。</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>重啟後生效的工作階段，使用者所作的修改將會在系統重啟後生效。</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>UWF 識別此磁碟區的方式。磁碟機代號直觀，但增減其他磁碟等情況下磁碟機代號可能被重新分配，繫結會失效；磁碟區 ID 不隨磁碟機代號變化，更穩定。</translation></message>
    <message><source>Bind by:</source><translation>繫結方式：</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>此磁碟區不支援 UWF：</translation></message>
    <message><source>Global settings</source><translation>全域設定</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>UWF 篩選器在當前工作階段中的啟用狀態（只讀）。</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>下次工作階段啟用 UWF 篩選器。對受保護卷的所有寫入都會被重定向到覆寫層，重啟後丟棄。</translation></message>
    <message><source>Filter</source><translation>篩選器</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>覆寫層的型別 / 最大大小只能在篩選器停用時修改：
1. 用上方的「篩選狀態」開關停用篩選器
2. 重啟電腦（重啟後篩選器為停用狀態）
3. 重新嘗試修改此項</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>覆寫層的存放位置。RAM 速度快但佔用記憶體；Disk 存放在系統盤，可用容量更大。重啟後兩者都會丟棄。</translation></message>
    <message><source>Type</source><translation>型別</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>覆寫層的最大容量。RAM 模式下受系統總記憶體限制。Disk 模式要求至少 1024 MB，且系統盤可用空間需大於該值。</translation></message>
    <message><source>Maximum size</source><translation>最大大小</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>最大大小 · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>警告閾值</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>覆寫層佔用達到此值時向事件日誌寫入警告級別通知。必須低於嚴重閾值。設為 0 可關閉該事件。</translation></message>
    <message><source>Critical threshold</source><translation>嚴重閾值</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>覆寫層佔用達到此值時向事件日誌寫入嚴重級別通知。必須高於警告閾值。設為 0 可關閉該事件。</translation></message>
    <message><source>Used / total</source><translation>已用 / 總計</translation></message>
    <message><source>Overlay</source><translation>覆寫層</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; 已佔用 &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; 警告 &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; 嚴重</translation></message>
    <message><source>UWF status unavailable: </source><translation>UWF 狀態不可用：</translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>需要管理員許可權才能修改 UWF 設定。請右鍵選擇「以管理員身份執行」重新啟動本程式。</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>不支援的驅動器型別（僅支援本地固定磁碟）。</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>卷容量超過 UWF 單個受保護卷 16 TiB 的上限。</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1 檔案系統：可以保護此磁碟區，但不支援檔案排除和單檔案提交。</translation></message>
    <message><source>Failed to read volume information.</source><translation>讀取磁碟區資訊失敗。</translation></message>
    <message><source>%1 free / %2</source><translation>%1 可用 / %2</translation></message>
    <message><source>Commit</source><translation>提交</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>把覆寫層中的修改提交到磁碟 / 登錄檔。此操作不可撤銷。</translation></message>
    <message><source>Commit file changes…</source><translation>提交檔案修改…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>選擇一個檔案，把它在覆寫層中的修改提交到磁碟。</translation></message>
    <message><source>Commit folder changes…</source><translation>提交資料夾修改…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>選擇一個目錄，把裡面所有檔案在覆寫層中的修改提交到磁碟。</translation></message>
    <message><source>Delete and commit file…</source><translation>刪除並提交檔案…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>選擇一個要刪除的檔案，把這次刪除提交到磁碟。</translation></message>
    <message><source>Delete and commit folder…</source><translation>刪除並提交資料夾…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>選擇一個要刪除的資料夾（連同其內部所有內容一併遞迴刪除），把這些刪除提交到磁碟。</translation></message>
    <message><source>Commit registry changes…</source><translation>提交登錄檔修改…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>輸入登錄檔鍵（可選值名），把修改提交到登錄檔。值名留空則遞迴提交該鍵及其所有子鍵中的每一個值。</translation></message>
    <message><source>Delete and commit registry…</source><translation>刪除並提交登錄檔…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>輸入要刪除的登錄檔鍵（可選值名），把這次刪除提交到登錄檔。值名留空則遞迴刪除整棵子樹（該鍵、它的所有值、以及它所有的子鍵）。</translation></message>
    <message><source>File exclusions</source><translation>檔案排除項目</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>此磁碟區不受 UWF 保護的檔案 / 目錄列表。雙擊條目可複製路徑。</translation></message>
    <message><source>Registry exclusions</source><translation>登錄排除項目</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>登錄檔排除是全域性的：由全域性 UWF 篩選器開關控制，與此磁碟區的保護狀態無關。該列表跨所有卷共享，並且只會在磁碟標籤頁中顯示一次。雙擊條目可複製路徑。</translation></message>
    <message><source>File staging</source><translation>檔案暫存</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>列表中的檔案和資料夾會在安全關機或重啟前自動提交，資料夾將遞迴處理。列表中存在資料時，UWF Manager 會排除 HKLM\SOFTWARE\HsingYun\UWF Manager，以確保該列表在受保護工作階段之間保持有效。</translation></message>
    <message><source>Search staged paths…</source><translation>搜尋暫存路徑…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>將檔案或資料夾加入自動提交列表。</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>選擇一個或多個檔案，在安全關機或重啟前自動提交。</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>選擇一個資料夾，在安全關機或重啟前遞迴提交其中的檔案。</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>立即從自動提交列表中移除所選路徑。</translation></message>
    <message><source>Select files for automatic commit</source><translation>選擇需要自動提交的檔案</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>選擇需要自動遞迴提交的資料夾</translation></message>
    <message><source>Cannot stage this path</source><translation>無法暫存此路徑</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>該路徑與 UWF 檔案排除項存在重疊。請先移除以下排除項，再新增暫存路徑：
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>該路徑與檔案暫存項存在重疊。請先移除以下暫存路徑，再新增排除項：
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>路徑與現有檔案暫存項存在重疊</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>檔案暫存中存在已儲存的條目時，必須保留該登錄檔排除項</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>所選路徑不存在，或其型別與選擇操作不符：
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>無法檢查所選路徑：
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>不能將重解析點加入檔案暫存：
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>此路徑沒有磁碟機代號，UWF 無法提交：
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>所選路徑位於 %1 卷，請在該磁碟區的“檔案暫存”頁中新增。</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>不能暫存卷根目錄，否則會遞迴提交整個卷。</translation></message>
    <message><source>File staging could not be saved</source><translation>無法儲存檔案暫存列表</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>更新未能完成，已重新載入檔案暫存的實際狀態：
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>檔案暫存不可用：%1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>將在安全關機或重啟前自動提交。</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>%1 個暫存項 · %2 個檔案 · %3 個資料夾</translation></message>
    <message><source>Required by UWF for File staging.</source><translation>UWF 檔案暫存功能需要此項。</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>UWF 篩選器當前已停用，覆寫層中不會累積修改，沒有可提交的內容。</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>%1 檔案系統不支援單檔案提交。</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>此磁碟區不受 UWF 支援。</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>此磁碟區不支援檔案提交。只能提交登錄檔修改（登錄檔排除是全域性的）。</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>%1 檔案系統不支援單檔案提交，只能提交登錄檔修改（登錄檔是全域性的）。</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>此磁碟區當前未受 UWF 保護，沒有可提交的檔案修改。</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>此磁碟區當前未受 UWF 保護，只能提交登錄檔修改（登錄檔是全域性的）。</translation></message>
    <message><source>Select a file to commit to disk</source><translation>選擇要提交到磁碟的檔案</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>選擇要提交到磁碟的目錄</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>選擇要提交刪除的檔案</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>選擇要提交刪除的資料夾</translation></message>
    <message><source>Commit registry changes</source><translation>提交登錄檔修改</translation></message>
    <message><source>Commit registry deletion</source><translation>提交登錄檔刪除</translation></message>
    <message><source>Key path:</source><translation>鍵路徑：</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>HKLM\Software\… —— 可輸入或貼上，按回車跳轉</translation></message>
    <message><source>Name</source><translation>名稱</translation></message>
    <message><source>Data</source><translation>資料</translation></message>
    <message><source>Key: %1</source><translation>鍵：%1</translation></message>
    <message><source>Value: %1</source><translation>值：%1</translation></message>
    <message><source>(none)</source><translation>（無）</translation></message>
    <message><source>(Default)</source><translation>（預設）</translation></message>
    <message><source>(value not set)</source><translation>（數值未設定）</translation></message>
    <message><source>(empty)</source><translation>（空）</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>無法預覽（值過大）</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>（整鍵遞迴，遞迴包含子鍵）</translation></message>
    <message><source>Overlay files - %1</source><translation>覆寫層檔案 - %1</translation></message>
    <message><source>View overlay files</source><translation>檢視覆寫層檔案</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>開啟診斷檢視，檢視當前在此磁碟區覆寫層中快取的檔案。</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>僅 NTFS 卷支援檢視覆寫層檔案列表。</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF 篩選器當前處於禁用狀態，沒有任何卷存在可檢視的覆寫層。</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>此磁碟區當前未被 UWF 保護，沒有可檢視的覆寫層。</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;僅作診斷快照。&lt;/b&gt;請勿根據此列表決定要提交的內容。</translation></message>
    <message><source>NTFS volumes only.</source><translation>僅支援 NTFS 卷。</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>覆寫層較大時可能非常慢，甚至失敗——記憶體佔用和耗時都隨覆寫層大小增長。</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>列表並不精確：小於磁碟簇大小（通常 4 KB）的檔案可能缺失；此前的提交、排除路徑下的檔案，以及看似不相關操作影響到的檔案都可能出現在列表裡。</translation></message>
    <message><source>Loading…</source><translation>載入中…</translation></message>
    <message><source>Failed: %1</source><translation>失敗：%1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>覆寫層中共 %1 個檔案</translation></message>
    <message><source>No files</source><translation>無檔案</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>第 %1 / %2 頁 · 共 %3 個檔案</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>WMI 提供程式在列舉覆寫層檔案時崩潰。這是 UWF_Overlay.GetOverlayFiles 在覆寫層較大或磁碟 I/O 繁忙時的已知不穩定行為。</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>記憶體不足或提供程式不支援此操作。覆寫層檔案列舉僅在 NTFS 捲上可用，且需要一定的可用記憶體；請減小覆寫層容量或關閉佔用記憶體較多的程式後再試。</translation></message>
    <message><source>Raw error: %1</source><translation>原始錯誤：%1</translation></message>
    <message><source>Export to file…</source><translation>匯出到檔案…</translation></message>
    <message><source>Export overlay file list</source><translation>匯出覆寫層檔案列表</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>文字檔案 (*.txt);;所有檔案 (*)</translation></message>
    <message><source>Export failed</source><translation>匯出失敗</translation></message>
    <message><source>Export finished</source><translation>匯出完成</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>無法開啟檔案寫入：%1</translation></message>
    <message><source>Could not write file: %1</source><translation>無法寫入檔案：%1</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>已儲存 %1 條記錄到：
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>不允許把整個卷 %1 加入排除列表。</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>分頁檔案 / 交換檔案 / 休眠檔案不能加入排除（這些是 UWF 本身依賴的系統檔案）。</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>不允許把整個 \Windows 目錄加入排除。</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>不允許把整個 \Windows\System32 目錄加入排除。</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>不允許把整個 \Windows\System32\Drivers 目錄加入排除。</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>不允許把關鍵系統檔案加入排除：%1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>不允許把使用者登錄檔檔案 NTUSER.DAT 加入排除。</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>登錄檔鍵不能為空。</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>登錄檔鍵不能以反斜槓開頭。</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>路徑含有連續反斜槓，無效。</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>登錄檔路徑使用反斜槓 `\` 作為分隔符，不要用正斜槓 `/`。</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>路徑含有不可見字元，無效。</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>不允許把 HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC 加入排除（域機器賬戶金鑰；UWF 文件明確禁止排除此鍵）。</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>UWF 只允許排除以下頂層登錄檔鍵的子項，請在這些鍵下面選擇具體位置：
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Search file paths…</source><translation>搜尋檔案路徑…</translation></message>
    <message><source>Search registry keys…</source><translation>搜尋登錄檔項…</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>把檔案或資料夾加入排除列表。排除的物件不受 UWF 保護。</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>選擇一個檔案加入排除列表。</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>選擇一個資料夾（包含其中所有內容）加入排除列表。</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>手動輸入一個登錄檔鍵路徑加入排除列表。</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>把一個登錄檔鍵加入排除列表，或開啟一個持久化開關。</translation></message>
    <message><source>Registry key…</source><translation>登錄檔鍵…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>域機密金鑰 (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>終端服務客戶端訪問許可證 (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>跨 UWF 工作階段持久化域機密金鑰（計算機賬戶密碼）。</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>跨 UWF 工作階段持久化終端服務客戶端訪問許可證。</translation></message>
    <message><source>· %1 persistence %2</source><translation>· %1 持久化 %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ %1 持久化：%2</translation></message>
    <message><source>Remove selected</source><translation>移除所選</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>從排除列表中移除選中項（應用後生效）。</translation></message>
    <message><source>Open containing folder</source><translation>開啟所在資料夾</translation></message>
    <message><source>Copy file path</source><translation>複製檔案路徑</translation></message>
    <message><source>Copy registry path</source><translation>複製登錄檔路徑</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>提交資料夾改動到磁碟…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>提交檔案改動到磁碟…</translation></message>
    <message><source>Copied to clipboard: </source><translation>已複製到剪貼簿：</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>選擇要加入排除的檔案（可多選）</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>選擇要加入排除的資料夾</translation></message>
    <message><source>Add registry exclusion</source><translation>新增登錄檔排除</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>所選路徑 %1 不在 %2 捲上，無法作為此磁碟區的排除項。</translation></message>
    <message><source>Path is not on this volume</source><translation>路徑不在當前卷</translation></message>
    <message><source>Cannot add this exclusion</source><translation>不能新增此排除項</translation></message>
    <message><source>File: %1</source><translation>檔案：%1</translation></message>
    <message><source>Folder: %1</source><translation>資料夾：%1</translation></message>
    <message><source>Registry: %1</source><translation>登錄檔：%1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>預設排除，可隨時更改。</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>當前工作階段：%1
下次工作階段：%2</translation></message>
    <message><source>Pending change: %1</source><translation>本次待應用：%1</translation></message>
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>共 %1 項 · 下次工作階段將新增 %2 · 移除 %3 · 待應用 %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>統一寫入篩選器 (UWF) 管理員</translation></message>
    <message><source>Main toolbar</source><translation>主工具欄</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>重新讀取 UWF 的當前工作階段狀態和下次工作階段配置。</translation></message>
    <message><source>Review and apply</source><translation>預覽並應用</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>預覽本次所有待應用的變更，確認後一次性應用（多數變更在下次重啟後生效）。</translation></message>
    <message><source>Safe shutdown</source><translation>安全關機</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>安全關機：即使 UWF 覆寫層已滿也能正常關機。</translation></message>
    <message><source>Safe restart</source><translation>安全重啟</translation></message>
    <message><source>Direct restart</source><translation>直接重啟</translation></message>
    <message><source>Direct shutdown</source><translation>直接關機</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>安全重啟：即使 UWF 覆寫層已滿也能正常重啟。</translation></message>
    <message><source>Log</source><translation>日誌</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>檢視本次工作階段累積的內部日誌（用於排查問題）。</translation></message>
    <message><source>About</source><translation>關於</translation></message>
    <message><source>About this program.</source><translation>關於本程式。</translation></message>
    <message><source>Switch display language</source><translation>切換顯示語言</translation></message>
    <message><source>Language</source><translation>語言</translation></message>
    <message><source>Theme</source><translation>主題</translation></message>
    <message><source>Toggle light / dark theme</source><translation>切換淺色 / 深色主題</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;UWF 的圖形化管理介面：篩選器狀態、覆寫層、檔案與登錄檔排除。多數變更在下次重啟後生效。&lt;/p&gt;&lt;p&gt;原始碼倉庫：&lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;本程式採用 &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt; 協議釋出；完整協議文字見隨程式分發的 LICENSE 檔案。&lt;/p&gt;&lt;p&gt;本程式為自由軟體，您可以在 GPL v3 條款下重新分發或修改本程式。本程式按「原樣」提供，不附帶任何形式的擔保。&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>關於 UWF 管理器</translation></message>
    <message><source>System information</source><translation>系統資訊</translation></message>
    <message><source>Diagnostic snapshot</source><translation>診斷快照</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>系統、安全、UWF、增強模式及近期應用日誌。</translation></message>
    <message><source>Read only</source><translation>只讀</translation></message>
    <message><source>Diagnostic report</source><translation>診斷報告</translation></message>
    <message><source>Copied</source><translation>已複製</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>檢視系統、安全、UWF、增強模式和日誌診斷資訊。</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>此報告可能包含賬戶名、裝置標識、檔案路徑和近期應用日誌。分享前請先檢查內容。</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;本程式依賴 Windows 統一寫入篩選器（UWF）。&lt;/b&gt;UWF Manager 自身不執行寫入過濾；實際的寫入保護由 Windows 內建的 UWF 功能提供。本程式僅用於配置和管理 UWF，需系統已安裝並啟用 UWF 才能使用。&lt;/p&gt;&lt;p&gt;首次在裝置上啟用 UWF 時，為提升 UWF 效能，UWF 會對系統做以下更改：&lt;/p&gt;&lt;ul&gt;&lt;li&gt;禁用頁面檔案（paging files）。&lt;/li&gt;&lt;li&gt;禁用系統還原（System Restore）。&lt;/li&gt;&lt;li&gt;禁用 SuperFetch。&lt;/li&gt;&lt;li&gt;關閉檔案索引服務（File indexing service）。&lt;/li&gt;&lt;li&gt;關閉碎片整理服務（Defragmentation service）。&lt;/li&gt;&lt;li&gt;禁用快速啟動（Fast boot）。&lt;/li&gt;&lt;li&gt;將 BCD 設定 bootstatuspolicy 設為 ignoreallfailures。&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;啟用 UWF 後，可以按需更改這些設定。例如可把頁面檔案移到未保護捲上並重新啟用頁面檔案。&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>未找到目標，覆寫層中沒有可提交的內容。</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>目標不在物理卷 / 持久化登錄檔上——它只存在於覆寫層中（在 UWF 保護下新建，從未提交過）。重啟後會自動消失，無需也無法用「提交刪除」處理。</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>系統拒絕了傳入的引數（路徑或引數非法）。</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>操作失敗。目標可能正被其他程式佔用（如資源管理器正在瀏覽該目錄、檔案被某個程式開啟）。請關閉佔用它的程式後重試。</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>操作失敗（詳情見日誌）。</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>操作被拒絕（程式碼 %1）。</translation></message>
    <message><source>Unknown cause.</source><translation>未知原因。</translation></message>
    <message><source>Commit canceled</source><translation>提交已取消</translation></message>
    <message><source>Commit result</source><translation>提交結果</translation></message>
    <message><source>Succeeded</source><translation>成功</translation></message>
    <message><source>Skipped</source><translation>跳過</translation></message>
    <message><source>Failed</source><translation>失敗</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>成功 %1 個；跳過 %2 個；失敗 %3 個。</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
使用者取消，剩餘 %1 個未處理。</translation></message>
    <message><source>Category</source><translation>類別</translation></message>
    <message><source>Path</source><translation>路徑</translation></message>
    <message><source>Existed before</source><translation>執行前存在</translation></message>
    <message><source>Exists after</source><translation>執行後存在</translation></message>
    <message><source>Error code</source><translation>錯誤碼</translation></message>
    <message><source>Reason</source><translation>原因</translation></message>
    <message><source>Previous page</source><translation>上一頁</translation></message>
    <message><source>First page</source><translation>首頁</translation></message>
    <message><source>Last page</source><translation>末頁</translation></message>
    <message><source>Next page</source><translation>下一頁</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>第 %1 / %2 頁 · 共 %3 條</translation></message>
    <message><source>Copy selected rows</source><translation>複製選中行</translation></message>
    <message><source>Copy all</source><translation>複製全部</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>檔案暫存中存在已儲存的條目時必須保留此登錄檔排除項。請先清空檔案暫存列表，再移除此項。</translation></message>
    <message><source>0 lines</source><translation>0 行</translation></message>
    <message><source>%1 lines</source><translation>共 %1 行</translation></message>
    <message><source>Loading log entries…</source><translation>正在載入日誌…</translation></message>
    <message><source>No log entries</source><translation>無日誌條目</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>第 %1 / %2 頁 · 共 %3 行</translation></message>
    <message><source>Copy current page</source><translation>複製當前頁</translation></message>
    <message><source>Clear</source><translation>清空</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>有 %1 項待應用變更（尚未寫入系統）</translation></message>
    <message><source>No pending changes</source><translation>無待應用變更</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation>（同時管理全域性登錄檔排除）</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>切換到卷 %1 的保護設定、檔案排除與檔案暫存。%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF 名稱空間不可用</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>已重新整理 · 共 %1 個卷</translation></message>
    <message><source>· Filter (global) %1</source><translation>· 篩選器（全域性） %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· 覆寫層 型別 → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· 覆寫層 最大大小 → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· 覆寫層 警告閾值 → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· 覆寫層 嚴重閾值 → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ 啟用篩選器期間無法修改型別 / 最大大小，需先停用篩選器並重啟。</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· 卷 %1 保護 %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· 卷 %1 繫結方式 → %2（命令列不支援，僅本程式內可改）</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ 檔案排除  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− 檔案排除  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ 登錄檔排除  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− 登錄檔排除  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>篩選器（全域性） %1</translation></message>
    <message><source>Overlay type → %1</source><translation>覆寫層 型別 → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>覆寫層 最大大小 → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>覆寫層 警告閾值 → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>覆寫層 嚴重閾值 → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>卷 %1 保護 %2</translation></message>
    <message><source>File exclusion %1</source><translation>檔案排除 %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>登錄檔排除 %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>待應用的變更 (%1)</translation></message>
    <message><source>Current session configuration</source><translation>當前工作階段配置</translation></message>
    <message><source>Review and apply changes</source><translation>變更預覽 · 應用</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>以下以 uwfmgr 命令形式列出當前所有配置；若有&lt;span style='color:%1'&gt;待應用的變更&lt;/span&gt;會先單獨成段。確認後點選 &lt;span style='color:%2'&gt;應用&lt;/span&gt;，本程式會真實寫入系統（多數在下次重啟後生效）。</translation></message>
    <message><source>Confirm apply</source><translation>確認應用</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>即將&lt;span style='color:%1'&gt;真實寫入系統&lt;/span&gt;，多數變更在下次重啟後才生效。&lt;br&gt;&lt;br&gt;確定要繼續嗎？</translation></message>
    <message><source>Applied changes</source><translation>已應用的變更</translation></message>
    <message><source>Result</source><translation>應用結果</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ 篩選器：%1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ 覆寫層 警告閾值設為 %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ 覆寫層 嚴重閾值設為 %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ 型別 / 最大大小未應用：篩選器當前已啟用，需要先停用篩選器並重啟後再修改。</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ 覆寫層 型別設為 %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ 覆寫層 最大大小設為 %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ 最大大小未應用：基於磁碟的覆寫層要求至少 %1 MB。</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ 卷 %1 保護：%2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ 卷 %1 繫結方式：%2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ 卷 %1 新增檔案排除：%2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ 卷 %1 移除檔案排除：%2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ 新增登錄檔排除：%1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ 移除登錄檔排除：%1</translation></message>
    <message><source>Time</source><translation>時間</translation></message>
    <message><source>Level</source><translation>級別</translation></message>
    <message><source>Tag</source><translation>TAG</translation></message>
    <message><source>Message</source><translation>內容</translation></message>
    <message><source>Confirm safe shutdown?</source><translation>確定要安全關機嗎？</translation></message>
    <message><source>Confirm safe restart?</source><translation>確定要安全重啟嗎？</translation></message>
    <message><source>Shut down</source><translation>關機</translation></message>
    <message><source>Restart</source><translation>重啟</translation></message>
    <message><source>Continue shutdown</source><translation>仍然關機</translation></message>
    <message><source>Continue restart</source><translation>仍然重啟</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>系統將透過 UWF 安全關機。</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>系統將透過 UWF 安全重啟。</translation></message>
    <message><source>UWF protection</source><translation>UWF 保護</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>即使 UWF 覆寫層已滿，仍可安全執行此操作。</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>正在計算需要自動提交的檔案…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>執行此操作前將自動提交 %1 個檔案；當前未受 UWF 保護的卷將被跳過。</translation></message>
    <message><source>Committing staged files</source><translation>正在提交暫存檔案</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>檔案暫存完成後，將繼續執行電源操作。</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>已處理 %1 / %2 個檔案。</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>檔案暫存未能全部提交</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>請檢視操作詳情，再決定是否仍要繼續。</translation></message>
    <message><source>Operation details</source><translation>操作詳情</translation></message>
    <message><source>No additional error details were provided.</source><translation>未提供更多錯誤詳情。</translation></message>
    <message><source>Some changes may not be preserved</source><translation>部分更改可能無法保留</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>如果仍然繼續，系統將關機，未能提交的檔案更改不會被保留。</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>如果仍然繼續，系統將重啟，未能提交的檔案更改不會被保留。</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>檔案暫存意外中止：
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>檔案暫存因未知錯誤中止。</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>尚未提交的更改將會丟失</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>繼續前，請儲存當前工作，並透過 UWF 提交需要永久保留的更改。</translation></message>
    <message><source>Confirm direct restart?</source><translation>確認直接重啟？</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>確認直接關機？</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>系統將跳過檔案暫存區的提交併直接重啟。</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>系統將跳過檔案暫存區的提交併直接關機。</translation></message>
    <message><source>File staging will be skipped</source><translation>將跳過檔案暫存區</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>重啟前不會提交檔案暫存區中的檔案。</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>關機前不會提交檔案暫存區中的檔案。</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>直接重啟會跳過檔案暫存區提交。</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>直接關機會跳過檔案暫存區提交。</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>已選擇直接操作，因此跳過檔案暫存區提交。</translation></message>
    <message><source>Safe shutdown failed</source><translation>安全關機失敗</translation></message>
    <message><source>Shutdown failed: %1</source><translation>關機失敗：%1</translation></message>
    <message><source>Safe restart failed</source><translation>安全重啟失敗</translation></message>
    <message><source>Direct restart failed</source><translation>直接重啟失敗</translation></message>
    <message><source>Direct shutdown failed</source><translation>直接關機失敗</translation></message>
    <message><source>Restart failed: %1</source><translation>重啟失敗：%1</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>無法讀取檔案暫存列表：
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>讀取檔案暫存列表時發生未知錯誤。</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>發現檔案：%1 · 已提交：%2 · 已跳過檔案：%3 · 已跳過條目：%4 · 失敗：%5</translation></message>
    <message><source>File staging preparation</source><translation>檔案暫存準備階段</translation></message>
    <message><source>Reparse points are not supported.</source><translation>不支援重解析點。</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>僅可暫存本地捲上的絕對路徑。</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>僅可將本地捲上的絕對路徑加入自動檔案暫存：
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>無法檢查暫存路徑。</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>暫存檔案當前已變為其他路徑型別。</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>暫存資料夾當前已變為其他路徑型別。</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>無法完整列舉暫存資料夾。</translation></message>
    <message><source>The path has no drive letter.</source><translation>路徑沒有磁碟機代號。</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>不能遞迴處理卷根目錄。</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>UWF 提供程式拒絕了提交請求。</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>提交因未知錯誤失敗。</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>另有 %1 項失敗已寫入日誌。</translation></message>
    <message><source>Commit failed</source><translation>提交失敗</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>路徑缺少磁碟機代號，無法確定目標卷。</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>讀取磁碟區資訊失敗：%1</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>找不到卷 %1 的當前工作階段記錄。</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>此路徑在檔案排除列表中。
排除項：%1</translation></message>
    <message><source>No files were found under %1.</source><translation>目錄 %1 下沒有任何檔案。</translation></message>
    <message><source>Commit to disk</source><translation>提交到磁碟</translation></message>
    <message><source>Committing…</source><translation>正在提交…</translation></message>
    <message><source>This action cannot be undone.</source><translation>此操作不可撤銷。</translation></message>
    <message><source>Continue</source><translation>繼續</translation></message>
    <message><source>Delete and commit</source><translation>刪除並提交</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>把這個檔案在覆寫層中的修改提交到磁碟</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>把這個資料夾在覆寫層中的修改提交到磁碟</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>將提交該資料夾及其所有子資料夾中的 %1 個檔案。</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>刪除這個檔案，並把刪除提交到磁碟</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>刪除這個資料夾及其全部內容，並把刪除提交到磁碟</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>將刪除 %1 個檔案和 %2 個子資料夾。</translation></message>
    <message><source>Commit this registry value to disk</source><translation>把這個登錄檔值提交到磁碟</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>把這個登錄檔鍵及其整棵子樹提交到磁碟</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>將提交該鍵及其所有子鍵中的 %1 個值。</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>刪除這個登錄檔值，並把刪除提交到磁碟</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>刪除這個登錄檔鍵及其整棵子樹，並把刪除提交到磁碟</translation></message>
    <message><source>Scanning registry keys…</source><translation>正在掃描登錄檔鍵…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>正在掃描登錄檔鍵…
已找到 %1 個鍵</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>將刪除的登錄檔鍵：</translation></message>
    <message><source>Copy current entry</source><translation>複製當前條目</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>將遞迴刪除 %1 個子鍵以及其包含的全部值。</translation></message>
    <message><source>Commit file deletion failed</source><translation>提交檔案刪除失敗</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>此路徑不存在，沒有可刪除的內容。</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>此鍵在登錄檔排除列表中。
排除項：%1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>讀取登錄檔篩選器失敗：%1</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>找不到當前工作階段的登錄檔篩選記錄。</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>此登錄檔值不存在，沒有可提交的內容。</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>此登錄檔鍵不存在，沒有可提交的內容。</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>此登錄檔值不存在，沒有可刪除的內容。</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>此登錄檔鍵不存在，沒有可刪除的內容。</translation></message>
    <message><source>Export commands…</source><translation>匯出命令…</translation></message>
    <message><source>Export commands to file</source><translation>匯出命令到檔案</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>已匯出 %1 條命令到：
%2</translation></message>
    <message><source>Import</source><translation>匯入</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>貼上、鍵入或從檔案載入一段 uwfmgr 命令指令碼，把每一行翻譯成 UI 上的待應用變更。點選「預覽並應用」之前不會真的寫入系統。</translation></message>
    <message><source>Import uwfmgr commands</source><translation>匯入 uwfmgr 命令</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;在下方貼上或鍵入 &lt;b&gt;uwfmgr&lt;/b&gt; 命令，每行一條。支援的類別：&lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;。&lt;/p&gt;&lt;p&gt;點選 &lt;b&gt;從檔案載入…&lt;/b&gt; 選擇任意文字類檔案（日誌、指令碼、.txt / .bat / .ps1），其中包含 &lt;code&gt;uwfmgr&lt;/code&gt; 的行會被追加到下面的輸入框。&lt;/p&gt;&lt;p&gt;點選 &lt;b&gt;匯入&lt;/b&gt; 會把每條命令轉成 UI 上的待應用變更——&lt;b&gt;此時還不會寫入系統&lt;/b&gt;。需要真正生效請用工具欄上的 &lt;b&gt;預覽並應用&lt;/b&gt;。&lt;/p&gt;</translation></message>
    <message><source>Load from file…</source><translation>從檔案載入…</translation></message>
    <message><source>Load recommended configuration</source><translation>載入推薦配置</translation></message>
    <message><source>Choose recommended configuration</source><translation>選擇推薦配置</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>選擇要追加的推薦配置分組。匯入前仍可檢查或刪除單條命令。</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>來自 Microsoft 官方文件。</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Microsoft 推薦的 UWF 排除規則</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>匯入前請先檢查這些推薦項；資料夾需要已存在，UWF 才會接受檔案排除。</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>來源：Microsoft 官方 UWF 文件，包括常見寫入篩選器排除項以及反惡意軟體支援。</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>客戶體驗改善計劃 (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP：持久化策略參與狀態</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP：持久化本機參與狀態</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP：持久化上傳禁用標誌</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>後臺智慧傳輸服務 (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS：持久化下載佇列檔案</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS：持久化傳輸狀態索引</translation></message>
    <message><source>Network profiles and policies</source><translation>網路配置檔案和策略</translation></message>
    <message><source>Wireless network GPO policy</source><translation>無線網路 GPO 策略</translation></message>
    <message><source>Wired network GPO policy</source><translation>有線網路 GPO 策略</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>無線網路 GPO 策略檔案</translation></message>
    <message><source>Wired network GPO policy files</source><translation>有線網路 GPO 策略檔案</translation></message>
    <message><source>Wireless network interface profiles</source><translation>無線網路介面配置</translation></message>
    <message><source>Wired network interface profiles</source><translation>有線網路介面配置</translation></message>
    <message><source>Wireless service configuration</source><translation>無線服務配置</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>移動寬頻服務配置</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>有線自動配置服務配置</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>網路配置 XML 檔案因裝置而異；如有需要，請手動新增具體的 Interfaces\{GUID}\{GUID}.xml 路徑。</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>夏令時 (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST：持久化時區定義</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST：持久化當前時區資訊</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender：持久化程式檔案和更新</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender：持久化 ProgramData 中的簽名和狀態</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender：持久化 Windows Update 日誌</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender：持久化 MpCmdRun 日誌</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender：持久化產品登錄檔狀態</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender：持久化 WdBoot 服務狀態</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender：持久化 WdFilter 服務狀態</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender：持久化 WdNisSvc 服務狀態</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender：持久化 WdNisDrv 服務狀態</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender：持久化 WinDefend 服務狀態</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP：持久化客戶端程式檔案</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP：持久化 Windows Update 日誌</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP：持久化 MpCmdRun 日誌</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP：持久化反惡意軟體簽名和狀態</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP：持久化反惡意軟體登錄檔狀態</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>選擇包含 uwfmgr 命令的檔案</translation></message>
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>所有檔案 (*);;文字檔案 (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
    <message><source>uwfmgr filter enable
uwfmgr overlay set-type RAM
uwfmgr volume protect C:
uwfmgr file add-exclusion "C:\Users\foo\bar.txt"
uwfmgr registry add-exclusion HKLM\Software\MyApp</source><translation>uwfmgr filter enable
uwfmgr overlay set-type RAM
uwfmgr volume protect C:
uwfmgr file add-exclusion "C:\Users\foo\bar.txt"
uwfmgr registry add-exclusion HKLM\Software\MyApp</translation></message>
    <message><source>#</source><translation>#</translation></message>
    <message><source>Status</source><translation>狀態</translation></message>
    <message><source>Command</source><translation>命令</translation></message>
    <message><source>Detail</source><translation>詳情</translation></message>
    <message><source>Import failed</source><translation>匯入失敗</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>內部錯誤：未註冊命令應用器。</translation></message>
    <message><source>Nothing to import</source><translation>沒有可匯入的內容</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>輸入中未找到任何 uwfmgr 命令。</translation></message>
    <message><source>Applied: %1</source><translation>已應用：%1</translation></message>
    <message><source>Duplicates: %1</source><translation>重複：%1</translation></message>
    <message><source>Unsupported: %1</source><translation>不支援：%1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>累計 %1 批後：</translation></message>
    <message><source>── Batch %1 ──</source><translation>── 第 %1 批 ──</translation></message>
    <message><source>Applied</source><translation>已應用</translation></message>
    <message><source>Duplicate</source><translation>重複</translation></message>
    <message><source>Unsupported</source><translation>不支援</translation></message>
    <message><source>File too large</source><translation>檔案過大</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>檔案 %1 超過 5 MB，未被解析；請先手動篩選後再載入。</translation></message>
    <message><source>Cannot read file</source><translation>無法讀取檔案</translation></message>
    <message><source>Could not open file %1: %2</source><translation>無法開啟檔案 %1：%2</translation></message>
    <message><source>Discard pending changes?</source><translation>放棄待應用變更？</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>還有 %1 個待應用變更尚未應用。

繼續並放棄這些變更？</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>uwfmgr 命令不完整</translation></message>
    <message><source>Missing size argument (MB)</source><translation>缺少大小引數（MB）</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>大小必須是非負的 MB 整數</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>缺少覆寫層型別引數（RAM 或 Disk）</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>未知的覆寫層型別 %1（應為 RAM 或 Disk）</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>缺少卷引數（如 C:）</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>卷必須是磁碟機代號形式（如 C:）</translation></message>
    <message><source>Missing path argument</source><translation>缺少路徑引數</translation></message>
    <message><source>Missing registry key argument</source><translation>缺少登錄檔鍵引數</translation></message>
    <message><source>Unclosed double quote in command</source><translation>命令中存在未閉合的雙引號</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>存在多餘引數：%1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>不支援的 uwfmgr 命令（無法翻譯成 UI 操作）</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>已加入待應用的%1變更</translation></message>
    <message><source>Already in the target state — no-op</source><translation>已經處於目標狀態——無須變更</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>路徑不在此磁碟區，或此磁碟區不支援檔案排除（如 exFAT / ReFS）</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>被 UWF 黑名單拒絕（系統檔案 / Windows / 分頁檔案等）</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>本次匯入中此前已出現過相同命令</translation></message>
    <message><source>Pending filter %1</source><translation>待應用：%1 篩選器</translation></message>
    <message><source>Filter is already in the target state</source><translation>篩選器已經處於目標狀態</translation></message>
    <message><source>Pending overlay type → %1</source><translation>待應用：覆寫層型別 → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>覆寫層型別已經是 %1</translation></message>
    <message><source>Invalid size value: %1</source><translation>大小數值非法：%1</translation></message>
    <message><source>maximum size</source><translation>最大大小</translation></message>
    <message><source>warning threshold</source><translation>警告閾值</translation></message>
    <message><source>critical threshold</source><translation>嚴重閾值</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>待應用：覆寫層 %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>覆寫層 %1 已經是 %2 MB</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>未知的卷 %1（沒有該磁碟機代號對應的 UWF 可保護磁碟）</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>待應用：卷 %1 %2 保護</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>卷 %1 的保護狀態已是目標值</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>路徑 %1 沒有磁碟機代號，無法定位到卷 TAB</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>沒有磁碟機代號 %1 對應的 UWF 可保護磁碟</translation></message>
    <message><source>file exclusion</source><translation>檔案排除</translation></message>
    <message><source>registry exclusion</source><translation>登錄檔排除</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>登錄檔排除只能在登錄檔標籤頁中配置，但當前沒有該標籤頁</translation></message>
    <message><source>Unsupported command</source><translation>不支援的命令</translation></message>
    <message><source>Exit</source><translation>退出</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>顯示或隱藏覆寫層 Hub。</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF：已啟用</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF：已禁用</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF 狀態不可用</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>已用 %1 MB / 總計 %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>增強模式</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>將檔案暫存自動提交與 Windows 關機、重啟流程聯動。</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>增強模式會安裝一項以 LocalSystem 身份執行的自動啟動服務。系統啟動後，服務會啟動 UWF 管理器；關機或重啟前，會嘗試提交使用者預先配置的暫存檔案的變更，以改善部分與 UWF 存在相容性衝突的軟體執行體驗。</translation></message>
    <message><source>System changes</source><translation>系統變更</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>建立自動啟動服務 UWFManagerEnhanced，並將其繫結到當前可執行檔案路徑</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>在登錄檔 HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced 下寫入服務配置，並提交</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>下次 Windows 啟動時自動執行 UWF 管理器；移動當前可執行檔案可能導致自啟動失效</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>在 Windows 正常關機或重啟過程中，提交符合條件的暫存檔案</translation></message>
    <message><source>Operational and security considerations</source><translation>執行與安全注意事項</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>啟用增強模式後會在系統登錄檔註冊服務，打破純綠色執行的約束</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>暫存區包含大量檔案時，關機或重啟耗時可能顯著增加；UWF Manager 需要充足時間完成使用者變更的持久化</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>增強模式會以 SYSTEM 身份執行本程式。如果程式被非管理員使用者篡改或者被替換為惡意二進位制檔案，可能導致惡意程式許可權提升至 SYSTEM。請僅在受管理員許可權保護的位置啟用增強模式（建議複製本程式至系統保護的目錄：%ProgramFiles%\UWF Manager\Service\UWF.exe 執行）</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>無法讀取增強模式狀態</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>讀取增強模式狀態時發生未知錯誤。</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>無法讀取服務的當前狀態：
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>讀取服務當前狀態時發生未知錯誤。</translation></message>
    <message><source>Status: Disabled</source><translation>狀態：未啟用</translation></message>
    <message><source>Status: Enabled</source><translation>狀態：已啟用</translation></message>
    <message><source>Status: Service stopped</source><translation>狀態：服務未執行</translation></message>
    <message><source>Status: Repair required</source><translation>狀態：需要修復</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>服務未配置為獨立程序。</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>服務未配置為自動啟動。</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>服務未配置為以 LocalSystem 身份執行。</translation></message>
    <message><source>The service is not running.</source><translation>服務當前未執行。</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>服務配置的可執行檔案路徑或啟動引數與當前程式不一致。</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>服務的預關機超時配置不正確。</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>服務未宣告啟動 UI 代理所需的許可權。</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>服務尚未接受 Windows 預關機通知。</translation></message>
    <message><source>Enable enhanced mode</source><translation>啟用增強模式</translation></message>
    <message><source>Disable enhanced mode</source><translation>關閉增強模式</translation></message>
    <message><source>Start service</source><translation>啟動服務</translation></message>
    <message><source>Repair enhanced mode</source><translation>修復增強模式</translation></message>
    <message><source>Delete service</source><translation>刪除服務</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>停止並刪除增強模式服務，同時清理殘留的服務登錄檔資料。</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>服務已不存在，但仍有服務登錄檔資料殘留。</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>正在等待 UI 代理完成身份認證。</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>服務正在執行，但當前沒有透過身份認證的 UI 代理連線。</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>UWF Manager 增強模式輔助服務</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>增強模式配置已應用。</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>服務配置已應用，但 UWF 持久化操作報告了以下問題：
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>增強模式配置失敗：
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>增強模式配置因未知錯誤而失敗。</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>增強模式服務及其殘留配置已刪除。</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>已請求刪除服務，但殘留清理報告了以下問題：
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>無法刪除增強模式服務：
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>刪除增強模式服務時發生未知錯誤。</translation></message>
    <message><source>File staging completed</source><translation>檔案暫存提交完成</translation></message>
    <message><source>File staging failed</source><translation>檔案暫存提交失敗</translation></message>
    <message><source>All staged targets were processed.</source><translation>所有暫存目標均已處理。</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>所有暫存目標均已處理，但部分操作失敗。</translation></message>
    <message><source>Show main window</source><translation>顯示主介面</translation></message>
    <message><source>Restore default position</source><translation>恢復預設位置</translation></message>
    <message><source>Hide overlay hub</source><translation>隱藏覆寫層 Hub</translation></message>
    <message><source>Exit application</source><translation>退出應用</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ 連線系統失敗</translation></message>
    <message><source>Registry access failed</source><translation>登錄檔訪問失敗</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>讀取登錄檔值失敗：%1</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>列舉覆寫層檔案時發生意外錯誤。</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ %1 篩選器失敗</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ 讀取篩選器狀態失敗</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ 設定警告閾值失敗</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ 設定嚴重閾值失敗</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ 讀取覆寫層狀態失敗</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ 設定覆寫層型別失敗</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ 設定最大大小失敗</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ 讀取覆寫層配置失敗</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ 卷 %1：註冊到 UWF 失敗</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ 卷 %2 %1 保護失敗</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ 設定卷 %1 的繫結方式失敗</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ 卷 %1 新增檔案排除項 %2 失敗</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ 卷 %1 已不存在檔案排除項：%2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ 卷 %1 移除檔案排除項 %2 失敗</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ 讀取卷配置失敗</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ 新增登錄檔排除項 %1 失敗</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ 移除登錄檔排除項 %1 失敗</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ 更新登錄檔持久化開關失敗</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ 讀取登錄檔篩選器失敗</translation></message>
    <message><source>UWF is not available</source><translation>UWF 不可用</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>讀取目標狀態時發生未知錯誤。</translation></message>
    <message><source>The target state could not be read before the operation: %1</source><translation>操作前無法讀取目標狀態：%1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>目標已不存在，無需刪除。</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>操作因未知錯誤而失敗。</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>重新讀取目標狀態失敗，無法確認操作結果：%1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>提供程式已接受刪除請求，但重新讀取權威狀態後目標仍然存在。</translation></message>
    <message><source>Unknown</source><translation>未知</translation></message>
    <message><source>Failed to load logs: %1</source><translation>載入日誌失敗：%1</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>載入日誌時發生意外錯誤。</translation></message>
    <message><source>: unknown error</source><translation>：未知錯誤</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>讀取目標狀態失敗：%1</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>讀取目標狀態時發生未知錯誤。</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>解析目標卷失敗：%1</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>列舉登錄檔值失敗：%1</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>列舉登錄檔鍵失敗：%1</translation></message>
    <message><source>Delete and commit failed</source><translation>刪除並提交失敗</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>無法解析路徑 %1 所在的卷：%2</translation></message>
    <message><source>%1/%2 slots</source><translation>%1/%2 插槽</translation></message>
</context>
</TS>
