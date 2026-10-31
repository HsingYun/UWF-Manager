<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>預覽</translation></message>
    <message><source>enable</source><translation>啟用</translation></message>
    <message><source>disable</source><translation>停用</translation></message>
    <message><source>Enabled</source><translation>啟用</translation></message>
    <message><source>Disabled</source><translation>停用</translation></message>
    <message><source>OK</source><translation>確定</translation></message>
    <message><source>Cancel</source><translation>取消</translation></message>
    <message><source>Close</source><translation>關閉</translation></message>
    <message><source>Refresh</source><translation>重新整理</translation></message>
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
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>此磁碟區在當前工作階段中的保護狀態（只讀）。</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>下次工作階段保護此磁碟區。此磁碟區的所有寫入都會被重定向到覆寫層，重啟後丟棄。</translation></message>
    <message><source>Current session</source><translation>目前工作階段</translation></message>
    <message><source>Next session</source><translation>下次工作階段</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>UWF 識別此磁碟區的方式。磁碟機代號直觀，但增減其他磁碟等情況下磁碟機代號可能被重新分配，繫結會失效；磁碟區 ID 不隨磁碟機代號變化，更穩定。</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>此磁碟區不支援 UWF：</translation></message>
    <message><source>Global settings</source><translation>全域設定</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>UWF 篩選器在當前工作階段中的啟用狀態（只讀）。</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>下次工作階段啟用 UWF 篩選器。對受保護卷的所有寫入都會被重定向到覆寫層，重啟後丟棄。</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>覆寫層使用量達到此值時，會將警告層級通知寫入事件記錄。兩個事件皆啟用時，此值必須低於嚴重閾值。設為 0 只會停用警告事件。</translation></message>
    <message><source>Critical threshold</source><translation>嚴重閾值</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>覆寫層使用量達到此值時，會將嚴重層級通知寫入事件記錄。兩個事件皆啟用時，此值必須高於警告閾值。設為 0 只會停用嚴重事件。</translation></message>
    <message><source>Overlay</source><translation>覆寫層</translation></message>
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
    <message><source>Add files or a folder to the automatic commit list.</source><translation>將檔案或資料夾加入自動提交列表。</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>選擇一個或多個檔案，在安全關機或重啟前自動提交。</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>選擇一個資料夾，在安全關機或重啟前遞迴提交其中的檔案。</translation></message>
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
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>檔案暫存依賴此登錄排除項，以便在重新啟動後保留暫存清單。</translation></message>
    <message><source>Built-in</source><translation>內建</translation></message>
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
    <message><source>Name</source><translation>名稱</translation></message>
    <message><source>Data</source><translation>資料</translation></message>
    <message><source>(Default)</source><translation>（預設）</translation></message>
    <message><source>(value not set)</source><translation>（數值未設定）</translation></message>
    <message><source>(empty)</source><translation>（空）</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>無法預覽（值過大）</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>開啟診斷檢視，檢視當前在此磁碟區覆寫層中快取的檔案。</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>僅 NTFS 卷支援檢視覆寫層檔案列表。</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF 篩選器當前處於禁用狀態，沒有任何卷存在可檢視的覆寫層。</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>此磁碟區當前未被 UWF 保護，沒有可檢視的覆寫層。</translation></message>
    <message><source>Failed: %1</source><translation>失敗：%1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>WMI 提供程式在列舉覆寫層檔案時崩潰。這是 UWF_Overlay.GetOverlayFiles 在覆寫層較大或磁碟 I/O 繁忙時的已知不穩定行為。</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>記憶體不足或提供程式不支援此操作。覆寫層檔案列舉僅在 NTFS 捲上可用，且需要一定的可用記憶體；請減小覆寫層容量或關閉佔用記憶體較多的程式後再試。</translation></message>
    <message><source>Raw error: %1</source><translation>原始錯誤：%1</translation></message>
    <message><source>Export overlay file list</source><translation>匯出覆寫層檔案列表</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>文字檔案 (*.txt);;所有檔案 (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>命令檔案 (*.cmd);;所有檔案 (*)</translation></message>
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
    <message><source>✓ %1 persistence: %2</source><translation>✓ %1 持久化：%2</translation></message>
    <message><source>Remove selected</source><translation>移除所選</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>從排除列表中移除選中項（應用後生效）。</translation></message>
    <message><source>Open containing folder</source><translation>開啟所在資料夾</translation></message>
    <message><source>Copy file path</source><translation>複製檔案路徑</translation></message>
    <message><source>Copy registry path</source><translation>複製登錄檔路徑</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>提交資料夾改動到磁碟…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>提交檔案改動到磁碟…</translation></message>
    <message><source>Copied to clipboard</source><translation>已複製到剪貼簿</translation></message>
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
    <message><source>Unified Write Filter (UWF) Manager</source><translation>統一寫入篩選器 (UWF) 管理員</translation></message>
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
    <message><source>Switch display language</source><translation>切換顯示語言</translation></message>
    <message><source>Language</source><translation>語言</translation></message>
    <message><source>Theme</source><translation>主題</translation></message>
    <message><source>Toggle light / dark theme</source><translation>切換淺色 / 深色主題</translation></message>
    <message><source>System information</source><translation>系統資訊</translation></message>
    <message><source>Diagnostic snapshot</source><translation>診斷快照</translation></message>
    <message><source>Read only</source><translation>只讀</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>未找到目標，覆寫層中沒有可提交的內容。</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>目標不在物理卷 / 持久化登錄檔上——它只存在於覆寫層中（在 UWF 保護下新建，從未提交過）。重啟後會自動消失，無需也無法用「提交刪除」處理。</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>系統拒絕了傳入的引數（路徑或引數非法）。</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>操作失敗。目標可能正被其他程式佔用（如資源管理器正在瀏覽該目錄、檔案被某個程式開啟）。請關閉佔用它的程式後重試。</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>操作失敗（詳情見日誌）。</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>操作被拒絕（程式碼 %1）。</translation></message>
    <message><source>Unknown cause.</source><translation>未知原因。</translation></message>
    <message><source>Commit canceled</source><translation>提交已取消</translation></message>

    <message><source>Succeeded</source><translation>成功</translation></message>
    <message><source>Skipped</source><translation>跳過</translation></message>
    <message><source>Failed</source><translation>失敗</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>成功 %1 個；跳過 %2 個；失敗 %3 個。</translation></message>

    <message><source>Existed before</source><translation>執行前存在</translation></message>
    <message><source>Exists after</source><translation>執行後存在</translation></message>
    <message><source>Error code</source><translation>錯誤碼</translation></message>
    <message><source>Reason</source><translation>原因</translation></message>
    <message><source>Previous page</source><translation>上一頁</translation></message>
    <message><source>First page</source><translation>首頁</translation></message>
    <message><source>Last page</source><translation>末頁</translation></message>
    <message><source>Next page</source><translation>下一頁</translation></message>
    <message><source>Copy selected rows</source><translation>複製選中行</translation></message>
    <message><source>Copy all</source><translation>複製全部</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>檔案暫存中存在已儲存的條目時必須保留此登錄檔排除項。請先清空檔案暫存列表，再移除此項。</translation></message>
    <message><source>Copy current page</source><translation>複製當前頁</translation></message>
    <message><source>No pending changes</source><translation>無待應用變更</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation>（同時管理全域性登錄檔排除）</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>切換到卷 %1 的保護設定、檔案排除與檔案暫存。%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF 名稱空間不可用</translation></message>
    <message><source>Review and apply changes</source><translation>變更預覽 · 應用</translation></message>
    <message><source>Confirm apply</source><translation>確認應用</translation></message>
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
    <message><source>Calculating files for automatic commit…</source><translation>正在計算需要自動提交的檔案…</translation></message>
    <message><source>Committing staged files</source><translation>正在提交暫存檔案</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>已處理 %1 / %2 個檔案。</translation></message>
    <message><source>No additional error details were provided.</source><translation>未提供更多錯誤詳情。</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>檔案暫存意外中止：
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>檔案暫存因未知錯誤中止。</translation></message>
    <message><source>File staging will be skipped</source><translation>將跳過檔案暫存區</translation></message>
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

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>路徑缺少磁碟機代號，無法確定目標卷。</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>找不到卷 %1 的當前工作階段記錄。</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>此路徑在檔案排除列表中。
排除項：%1</translation></message>

    <message><source>Continue</source><translation>繼續</translation></message>
    <message><source>Delete and commit</source><translation>刪除並提交</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>此鍵在登錄檔排除列表中。
排除項：%1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>找不到當前工作階段的登錄檔篩選記錄。</translation></message>

    <message><source>Export commands to file</source><translation>匯出命令到檔案</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>已匯出 %1 條命令到：
%2</translation></message>
    <message><source>Import</source><translation>匯入</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>貼上、鍵入或從檔案載入一段 uwfmgr 命令指令碼，把每一行翻譯成 UI 上的待應用變更。點選「預覽並應用」之前不會真的寫入系統。</translation></message>
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
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>文字檔案 (*.txt *.cmd);;所有檔案 (*)</translation></message>
    <message><source>Status</source><translation>狀態</translation></message>
    <message><source>Import failed</source><translation>匯入失敗</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>內部錯誤：未註冊命令應用器。</translation></message>
    <message><source>Nothing to import</source><translation>沒有可匯入的內容</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>輸入中未找到任何 uwfmgr 命令。</translation></message>
    <message><source>Duplicates: %1</source><translation>重複：%1</translation></message>
    <message><source>Unsupported: %1</source><translation>不支援：%1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── 第 %1 批 ──</translation></message>
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
    <message><source>Already in the target state — no-op</source><translation>已經處於目標狀態——無須變更</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>路徑不在此磁碟區，或此磁碟區不支援檔案排除（如 exFAT / ReFS）</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>被 UWF 黑名單拒絕（系統檔案 / Windows / 分頁檔案等）</translation></message>
    <message><source>Invalid size value: %1</source><translation>大小數值非法：%1</translation></message>
    <message><source>maximum size</source><translation>最大大小</translation></message>
    <message><source>warning threshold</source><translation>警告閾值</translation></message>
    <message><source>critical threshold</source><translation>嚴重閾值</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>全域設定目前無法使用，未將此命令加入待套用變更。</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>未知的卷 %1（沒有該磁碟機代號對應的 UWF 可保護磁碟）</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>卷 %1 的保護狀態已是目標值</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>路徑 %1 沒有磁碟機代號，無法定位到卷 TAB</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>沒有磁碟機代號 %1 對應的 UWF 可保護磁碟</translation></message>
    <message><source>file exclusion</source><translation>檔案排除</translation></message>
    <message><source>registry exclusion</source><translation>登錄檔排除</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>登錄檔排除只能在登錄檔標籤頁中配置，但當前沒有該標籤頁</translation></message>
    <message><source>Unsupported command</source><translation>不支援的命令</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>顯示或隱藏覆寫層 Hub。</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF：已啟用</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF：已禁用</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF 狀態不可用</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>已用 %1 MB / 總計 %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>增強模式</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>將檔案暫存自動提交與 Windows 關機、重啟流程聯動。</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>讀取增強模式狀態時發生未知錯誤。</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>無法讀取服務的當前狀態：
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>讀取服務當前狀態時發生未知錯誤。</translation></message>
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
    <message><source>Show overlay hub</source><translation>顯示覆寫層 Hub</translation></message>
    <message><source>Hide overlay hub</source><translation>隱藏覆寫層 Hub</translation></message>
    <message><source>Exit application</source><translation>退出應用</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ 連線系統失敗</translation></message>
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
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>目標已不存在，無需刪除。</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>操作因未知錯誤而失敗。</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>重新讀取目標狀態失敗，無法確認操作結果：%1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>提供程式已接受刪除請求，但重新讀取權威狀態後目標仍然存在。</translation></message>
    <message><source>Unknown</source><translation>未知</translation></message>
    <message><source>Failed to load logs: %1</source><translation>載入日誌失敗：%1</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>載入日誌時發生意外錯誤。</translation></message>
    <message><source>: unknown error</source><translation>：未知錯誤</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>解析目標卷失敗：%1</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>列舉登錄檔鍵失敗：%1</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>無法解析路徑 %1 所在的卷：%2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>前置操作失敗或無法確認，已略過此操作。</translation></message>
    <message><source>Copy</source><translation>複製</translation></message>
    <message><source>Select All</source><translation>全部選擇</translation></message>
    <message><source>Display language</source><translation>顯示語言</translation></message>
    <message><source>Folder</source><translation>資料夾</translation></message>
    <message><source>File</source><translation>檔案</translation></message>
    <message><source>Export commands</source><translation>導出命令</translation></message>
    <message><source>Direct restart…</source><translation>直接重啟…</translation></message>
    <message><source>Clear…</source><translation>清空…</translation></message>
    <message><source>Committing</source><translation>提交中</translation></message>
    <message><source>Target</source><translation>目標</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>停用篩選器并重啟後，可調整類型與大小。</translation></message>
    <message><source>Disk</source><translation>磁碟</translation></message>
    <message><source>Protection and overlay</source><translation>保護與覆寫層</translation></message>
    <message><source>Write protection</source><translation>寫入保護</translation></message>
    <message><source>Overlay type</source><translation>覆寫層類型</translation></message>
    <message><source>Storage</source><translation>存儲介質</translation></message>
    <message><source>Overlay maximum size</source><translation>覆寫層最大大小</translation></message>
    <message><source>Volume %1 protection</source><translation>磁碟區 %1 保護</translation></message>
    <message><source>By volume ID</source><translation>按磁碟區 ID</translation></message>
    <message><source>By drive letter</source><translation>按磁碟機代號</translation></message>
    <message><source>Volume %1 binding</source><translation>磁碟區 %1 繫結</translation></message>
    <message><source>Volume identity</source><translation>磁碟區標識</translation></message>
    <message><source>Removed</source><translation>移除</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>此路徑的后續寫入不受 UWF 保護。</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>從下次啟動的排除項中移除此路徑。</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>此鍵的后續寫入不受 UWF 保護。</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>從下次啟動的排除項中移除此鍵。</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>域密鑰持久化</translation></message>
    <message><source>Terminal Services license persistence</source><translation>終端服務許可證持久化</translation></message>
    <message><source>Line</source><translation>行</translation></message>
    <message><source>Original command</source><translation>原始命令</translation></message>
    <message><source>Processing result</source><translation>處理結果</translation></message>
    <message><source>Imported</source><translation>已導入</translation></message>
    <message><source>Error</source><translation>錯誤</translation></message>
    <message><source>Warning</source><translation>警告</translation></message>
    <message><source>Information</source><translation>信息</translation></message>
    <message><source>Debug</source><translation>調試</translation></message>
    <message><source>Module</source><translation>模塊</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>清空記錄失敗：%1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>清空記錄失敗：發生意外錯誤。</translation></message>
    <message><source>Name and location</source><translation>名稱與位置</translation></message>
    <message><source>Size</source><translation>大小</translation></message>
    <message><source>System metadata</source><translation>系統元數據</translation></message>
    <message><source>Date modified</source><translation>修改日期</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>請選擇限定磁碟或資料夾內的位置。</translation></message>
    <message><source>Unable to read this folder.</source><translation>無法讀取此資料夾。</translation></message>
    <message><source>All files (*)</source><translation>所有檔案 (*)</translation></message>
    <message><source>The destination folder does not exist.</source><translation>目標資料夾不存在。</translation></message>
    <message><source>Enter a valid file name.</source><translation>請輸入有效的檔案名。</translation></message>
    <message><source>The selected item no longer exists: %1</source><translation>所選項目已不存在：%1</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>請選擇檔案，或打開資料夾瀏覽其內容。</translation></message>
    <message><source>Select a folder.</source><translation>請選擇資料夾。</translation></message>
    <message><source>Enter a valid folder name.</source><translation>請輸入有效的資料夾名稱。</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>請輸入有效的檔案或資料夾名稱。</translation></message>
    <message><source>Commit complete</source><translation>提交完成</translation></message>
    <message><source>Review the result for each target.</source><translation>查看各目標的操作結果。</translation></message>

    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>失敗項的更改尚未儲存。選擇一行查看完整路徑和原因。</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>所有更改已提交到磁碟，無需重啟即可持久儲存。</translation></message>
    <message><source>Item details</source><translation>項目詳情</translation></message>
    <message><source>Done</source><translation>完成</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>核對本次變更，確認后寫入系統。</translation></message>
    <message><source>%1 changes</source><translation>%1 項變更</translation></message>
    <message><source>Takes effect after restart</source><translation>下次啟動生效</translation></message>
    <message><source>View commands for these changes</source><translation>查看本次變更的命令</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>設定與下一會話目標一致。可在“完整設定”中查看或導出命令。</translation></message>
    <message><source>Pending changes</source><translation>待應用變更</translation></message>
    <message><source>Next-session configuration</source><translation>下一會話設定</translation></message>
    <message><source>%1 commands</source><translation>%1 條命令</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>將此 UTF-8 CMD 指令碼儲存為 .cmd 檔案，並在命令提示字元中執行。匯出使用相同的指令碼。</translation></message>
    <message><source>Full configuration</source><translation>完整設定</translation></message>
    <message><source>Apply changes</source><translation>應用更改</translation></message>
    <message><source>Some changes could not be applied</source><translation>部分變更未能應用</translation></message>
    <message><source>Configuration applied</source><translation>設定已應用</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>請查看操作詳情；重新讀取系統狀態后，再修正未完成項。</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>設定已寫入系統，大部分變更在下次啟動時生效。</translation></message>
    <message><source>Application result</source><translation>應用結果</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>將這些設定變更寫入系統？大部分設定將在下次啟動時生效。</translation></message>
    <message><source>Search paths</source><translation>搜索路徑</translation></message>
    <message><source>Add files or folders</source><translation>添加檔案或資料夾</translation></message>
    <message><source>File or folder</source><translation>檔案或資料夾</translation></message>
    <message><source>Registry key or persistence setting</source><translation>登錄檔項或持久化設置</translation></message>
    <message><source>%1 selected</source><translation>已選擇 %1 項</translation></message>
    <message><source>Undo removal</source><translation>撤銷移除</translation></message>
    <message><source>%1 items</source><translation>%1 項</translation></message>
    <message><source>Filter is disabled</source><translation>篩選器未啟用</translation></message>
    <message><source>Usage unavailable</source><translation>用量不可用</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>覆寫層已達到嚴重臨界值。請儲存更改，并盡快安全重啟。</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>覆寫層已達到警告臨界值。請儲存更改，再安全重啟。</translation></message>
    <message><source>UWF filter</source><translation>UWF 篩選器</translation></message>
    <message><source>Overlay usage</source><translation>覆寫層使用量</translation></message>
    <message><source>Current usage</source><translation>目前使用量</translation></message>
    <message><source>Warning %1 MB</source><translation>警告 %1 MB</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>啟用并重啟後顯示實時用量</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>讀取設定后顯示實時用量</translation></message>
    <message><source>Critical %1 MB</source><translation>嚴重 %1 MB</translation></message>
    <message><source>On</source><translation>開啟</translation></message>
    <message><source>Off</source><translation>關閉</translation></message>
    <message><source>Import configuration</source><translation>導入設定</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>從腳本或文本中添加 UWF 設定。</translation></message>
    <message><source>One uwfmgr command per line</source><translation>每行一條 uwfmgr 命令</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr 命令</translation></message>
    <message><source>Editable</source><translation>可編輯</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>粘貼命令，或從檔案載入…</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>匯入會更新設定草稿。點擊「預覽並應用」可將變更寫入系統。</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>報告列出了各條命令對設定草稿的處理結果。選擇一行可檢視完整命令和處理結果。</translation></message>
    <message><source>Edit commands</source><translation>編輯命令</translation></message>
    <message><source>Import results</source><translation>導入結果</translation></message>
    <message><source>Not written to the system</source><translation>尚未寫入系統</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 行</translation></message>
    <message><source>Continue editing</source><translation>繼續編輯</translation></message>
    <message><source>Imported: %1</source><translation>已導入 %1</translation></message>
    <message><source>Binding</source><translation>繫結</translation></message>
    <message><source>Protected</source><translation>已保護</translation></message>
    <message><source>Unprotected</source><translation>未保護</translation></message>
    <message><source>View overlay</source><translation>查看覆寫層</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF 目前不可用</translation></message>
    <message><source>Select registry key or value</source><translation>選擇登錄檔鍵或值</translation></message>
    <message><source>Select registry key or value to delete</source><translation>選擇要刪除的登錄檔鍵或值</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>未能讀取 UWF 設定。確認 Windows 已安裝統一寫入篩選器后重新讀取。</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>增強模式不可用；請使用安全關機或安全重啟儲存暫存檔案。</translation></message>
    <message><source>Version %1</source><translation>版本 %1</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Windows 統一寫入篩選器的圖形管理工具。
管理磁碟區保護、覆寫層、排除項與檔案暫存。</translation></message>
    <message><source>Source code</source><translation>源代碼</translation></message>
    <message><source>Contact author</source><translation>聯系作者</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>寫入保護由 Windows 內置 UWF 提供，本程式負責設定與管理。</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>首次啟用 UWF 對系統的影響</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows 會停用分頁檔案、系統還原、SuperFetch、檔案索引、磁碟碎片整理和快速啟動，并將 BCD 的 bootstatuspolicy 設為 ignoreallfailures。&lt;/p&gt;&lt;p&gt;之後可按需調整。例如，將分頁檔案移至未保護磁碟區后重新啟用分頁。&lt;/p&gt;</translation></message>
    <message><source>License and copyright</source><translation>許可與版權</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;采用 &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt; 發布，完整條款見程式隨附的 LICENSE 檔案。&lt;/p&gt;&lt;p&gt;可按 GPL v3 條款再分發或修改。軟件按原樣提供，不附帶任何保證。&lt;/p&gt;</translation></message>
    <message><source>Copy version</source><translation>復制版本</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>查看運行記錄與診斷信息。</translation></message>
    <message><source>Log level</source><translation>記錄等級</translation></message>
    <message><source>All levels</source><translation>全部等級</translation></message>
    <message><source>Current run</source><translation>本次運行</translation></message>
    <message><source>Search messages or modules</source><translation>搜索消息或模塊</translation></message>
    <message><source>Search logs</source><translation>搜索記錄</translation></message>
    <message><source>Message details</source><translation>消息詳情</translation></message>
    <message><source>Error details</source><translation>錯誤詳情</translation></message>
    <message><source>Clear logs?</source><translation>清空記錄？</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>目前記錄記錄將被清空，無法恢復。</translation></message>
    <message><source>Clear logs</source><translation>清空記錄</translation></message>
    <message><source>Copy all logs</source><translation>復制全部記錄</translation></message>
    <message><source>Reading logs</source><translation>正在讀取記錄</translation></message>
    <message><source>Unable to read logs</source><translation>無法讀取記錄</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>讀取完成後將在此顯示本次運行記錄。</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>記錄讀取失敗。請重試，或檢查記錄檔案的訪問權限。</translation></message>
    <message><source>Cancel reading</source><translation>取消讀取</translation></message>
    <message><source>Read again</source><translation>重新讀取</translation></message>
    <message><source>No matching logs</source><translation>沒有匹配的記錄</translation></message>
    <message><source>No logs yet</source><translation>暫無記錄</translation></message>
    <message><source>Try another keyword or log level.</source><translation>嘗試調整關鍵詞或記錄等級。</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>新的運行記錄將在刷新后顯示。</translation></message>
    <message><source>Clear filters</source><translation>清除篩選</translation></message>
    <message><source>%1 records</source><translation>%1 條記錄</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 條記錄 · 共 %2 條</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>系統、權限與 UWF 的診斷快照。</translation></message>
    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>報告可能包含賬戶名、裝置標識和檔案路徑，分享前請先檢查。</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>部分硬件信息未能讀取，其余診斷信息仍可查看和復制。</translation></message>
    <message><source>Overview</source><translation>概覽</translation></message>
    <message><source>Raw report</source><translation>原始報告</translation></message>
    <message><source>Copy full report</source><translation>復制完整報告</translation></message>
    <message><source>Delete and commit this file?</source><translation>刪除并提交檔案？</translation></message>
    <message><source>Commit file changes?</source><translation>提交檔案更改？</translation></message>
    <message><source>Delete and commit this folder?</source><translation>刪除并提交資料夾？</translation></message>
    <message><source>Commit folder changes?</source><translation>提交資料夾更改？</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>刪除并提交登錄檔鍵？</translation></message>
    <message><source>Commit registry key changes?</source><translation>提交登錄檔鍵更改？</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>刪除并提交登錄檔值？</translation></message>
    <message><source>Commit registry value changes?</source><translation>提交登錄檔值更改？</translation></message>

    <message><source>Commit changes</source><translation>提交更改</translation></message>
    <message><source>Service stopped</source><translation>服務已停止</translation></message>
    <message><source>Repair required</source><translation>需要修復</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>服務已安裝但未運行。啟動服務以恢復自動儲存。</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>無法讀取增強模式服務狀態。請檢查服務權限后重試。</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>與 Windows 關機流程協同</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>正常關機或重啟時，自動儲存符合條件的暫存檔案。</translation></message>
    <message><source>Background service</source><translation>后臺服務</translation></message>
    <message><source>Starts automatically with Windows</source><translation>隨 Windows 自動啟動</translation></message>
    <message><source>Automatic file staging</source><translation>自動儲存暫存檔案</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>接管正常關機與重啟前的檔案提交</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>未啟用時，仍可使用程式內的“安全關機”和“安全重啟”儲存暫存檔案。</translation></message>
    <message><source>Operation and security</source><translation>運行與安全說明</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>增強模式會新增一個隨 Windows 啟動的背景服務。移動或刪除程式前，請先移除該服務。</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>請將程式放在一般使用者無法修改的固定資料夾中。建議位置：</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>如果程式所在磁碟受 UWF 保護，啟用前請先確保程式檔案在重新啟動後仍會保留。暫存檔案較多時，關機或重新啟動可能需要更長時間。</translation></message>
    <message><source>Service details</source><translation>服務詳情</translation></message>
    <message><source>Configuration error</source><translation>設定異常</translation></message>
    <message><source>Running</source><translation>正在運行</translation></message>
    <message><source>Service status</source><translation>服務狀態</translation></message>
    <message><source>Stopped</source><translation>已停止</translation></message>
    <message><source>Not installed</source><translation>尚未安裝</translation></message>
    <message><source>Ready</source><translation>就緒</translation></message>
    <message><source>Unavailable</source><translation>不可用</translation></message>
    <message><source>Service name</source><translation>服務名稱</translation></message>
    <message><source>Account</source><translation>運行賬戶</translation></message>
    <message><source>Startup type</source><translation>啟動類型</translation></message>
    <message><source>Automatic</source><translation>自動</translation></message>
    <message><source>Not configured</source><translation>尚未設定</translation></message>
    <message><source>Application path</source><translation>程式路徑</translation></message>
    <message><source>UI agent</source><translation>界面代理</translation></message>
    <message><source>Connected and authenticated</source><translation>已連接 · 身份已驗證</translation></message>
    <message><source>64-bit</source><translation>64 位</translation></message>
    <message><source>32-bit</source><translation>32 位</translation></message>
    <message><source>System</source><translation>系統</translation></message>
    <message><source>Operating system</source><translation>操作系統</translation></message>
    <message><source>Architecture</source><translation>體系結構</translation></message>
    <message><source>System memory</source><translation>系統記憶體</translation></message>
    <message><source>Permissions and protection</source><translation>權限與保護</translation></message>
    <message><source>Privileges</source><translation>運行權限</translation></message>
    <message><source>Standard user</source><translation>標準使用者</translation></message>
    <message><source>UWF feature</source><translation>UWF 功能</translation></message>
    <message><source>Available</source><translation>可用</translation></message>
    <message><source>Current filter</source><translation>目前篩選器</translation></message>
    <message><source>Protected volumes</source><translation>已保護磁碟區</translation></message>
    <message><source>None</source><translation>無</translation></message>
    <message><source>Connecting…</source><translation>正在連接…</translation></message>
    <message><source>Not connected</source><translation>未連接</translation></message>
    <message><source>Service account</source><translation>服務賬戶</translation></message>
    <message><source>Service configuration needs repair</source><translation>服務設定需要修復</translation></message>
    <message><source>Retry</source><translation>重試</translation></message>
    <message><source>Status unavailable</source><translation>狀態不可用</translation></message>
    <message><source>Unable to read service status</source><translation>服務狀態讀取失敗</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>查看 %1 的檔案診斷快照。</translation></message>
    <message><source>Local disk (%1)</source><translation>本地磁碟 (%1)</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>列表可能遺漏或包含已提交檔案，請勿將其作為提交清單。</translation></message>
    <message><source>About this snapshot</source><translation>了解範圍</translation></message>
    <message><source>Commit changes…</source><translation>提交更改…</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>系統元數據僅供診斷，不提供提交操作。</translation></message>
    <message><source>Error details and suggestions</source><translation>錯誤詳情與建議</translation></message>
    <message><source>Export list</source><translation>導出列表</translation></message>
    <message><source>Reading overlay</source><translation>正在讀取覆寫層</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>暫時無法讀取覆寫層</translation></message>
    <message><source>Reading canceled</source><translation>已取消讀取</translation></message>
    <message><source>No overlay files found</source><translation>未發現覆寫層檔案</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>大覆寫層的讀取可能較慢，可隨時取消。</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>UWF 提供程式未能完成枚舉。覆寫層較大或系統資源不足時可能發生此問題。</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>重新讀取以獲取診斷快照。</translation></message>
    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>該診斷快照未返回檔案，不代表覆寫層中沒有更改。</translation></message>
    <message><source>%1 items · %2</source><translation>%1 項 · %2</translation></message>
    <message><source>Selected folder</source><translation>所選資料夾</translation></message>
    <message><source>Selected file</source><translation>所選檔案</translation></message>
    <message><source>Direct shutdown?</source><translation>直接關機？</translation></message>
    <message><source>Direct restart?</source><translation>直接重啟？</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>將跳過檔案暫存。受保護路徑中尚未提交的更改會在關機後丟失。</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>將跳過檔案暫存。受保護路徑中尚未提交的更改會在重啟後丟失。</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>不會儲存暫存檔案。</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>先將暫存檔案提交到磁碟，再關機。</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>先將暫存檔案提交到磁碟，再重啟。</translation></message>
    <message><source>staged files</source><translation>個暫存檔案</translation></message>
    <message><source>View failure details</source><translation>查看失敗詳情</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>僅暫存檔案會自動提交。其他受保護路徑中的未提交更改將在關機後丟失。</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>僅暫存檔案會自動提交。其他受保護路徑中的未提交更改將在重啟後丟失。</translation></message>
    <message><source>Ready to save</source><translation>即將儲存</translation></message>
    <message><source>No files to commit</source><translation>沒有待提交檔案</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>檔案較多時可能需要更長時間。</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>將直接執行 UWF 安全關機。</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>將直接執行 UWF 安全重啟。</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 個檔案</translation></message>
    <message><source>Saving staged files</source><translation>正在儲存暫存檔案</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>儲存完成後將繼續關機。請勿關閉程式。</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>儲存完成後將繼續重啟。請勿關閉程式。</translation></message>
    <message><source>Saving to disk</source><translation>正在提交到磁碟</translation></message>
    <message><source>Saving…</source><translation>正在儲存…</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>檔案提交已開始，暫時無法取消。</translation></message>
    <message><source>Confirm before continuing</source><translation>請確認是否繼續</translation></message>
    <message><source>Some files could not be saved</source><translation>部分檔案未能儲存</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>請查看失敗詳情，再決定是否繼續關機。</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>請查看失敗詳情，再決定是否繼續重啟。</translation></message>
    <message><source>File saving incomplete</source><translation>檔案提交未完成</translation></message>
    <message><source>Automatic saving incomplete</source><translation>自動儲存未完成</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>繼續關機將丟失未能儲存的檔案更改。</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>繼續重啟將丟失未能儲存的檔案更改。</translation></message>
    <message><source>Shut down anyway</source><translation>仍然關機</translation></message>
    <message><source>Restart anyway</source><translation>仍然重啟</translation></message>
    <message><source>Counting staged files…</source><translation>正在統計暫存檔案…</translation></message>
    <message><source>Scanning staged paths</source><translation>正在掃描暫存路徑</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>選擇鍵可操作整個子樹，選擇命名值僅操作此值。</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>選擇要從 UWF 寫入保護中排除的登錄機碼（包含子機碼與值）。</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>輸入 HKLM\SOFTWARE\…，按 Enter 跳轉</translation></message>
    <message><source>Navigate to registry path</source><translation>轉到登錄檔路徑</translation></message>
    <message><source>Collapse</source><translation>折疊</translation></message>
    <message><source>Expand</source><translation>展開</translation></message>
    <message><source>Copy name</source><translation>復制名稱</translation></message>
    <message><source>Unable to access this registry key</source><translation>無法訪問此登錄檔鍵</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>請選擇其他鍵，或檢查訪問權限后重試。</translation></message>
    <message><source>Current selection</source><translation>目前選擇</translation></message>
    <message><source>Copy selection</source><translation>復制所選內容</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>改選整個鍵（含全部子鍵）</translation></message>
    <message><source>Select this registry value</source><translation>選擇此登錄檔值</translation></message>
    <message><source>Home</source><translation>主目錄</translation></message>
    <message><source>Back</source><translation>返回上一個資料夾</translation></message>
    <message><source>Up one folder</source><translation>上一級資料夾</translation></message>
    <message><source>Refresh folder</source><translation>刷新資料夾</translation></message>
    <message><source>Folder address</source><translation>資料夾地址</translation></message>
    <message><source>Search this folder</source><translation>搜索此資料夾</translation></message>
    <message><source>Quick access</source><translation>快速訪問</translation></message>
    <message><source>This PC</source><translation>此電腦</translation></message>
    <message><source>Folders</source><translation>資料夾</translation></message>
    <message><source>Files and folders</source><translation>檔案與資料夾</translation></message>
    <message><source>New folder</source><translation>新建資料夾</translation></message>
    <message><source>Open folder</source><translation>打開資料夾</translation></message>
    <message><source>Select this file</source><translation>選擇此檔案</translation></message>
    <message><source>Copy path</source><translation>復制路徑</translation></message>
    <message><source>Rename…</source><translation>重命名…</translation></message>
    <message><source>Selection</source><translation>所選項</translation></message>
    <message><source>File name</source><translation>檔案名</translation></message>
    <message><source>Select the current folder</source><translation>選擇目前資料夾</translation></message>
    <message><source>Select files or folders</source><translation>選擇檔案或資料夾</translation></message>
    <message><source>Enter a file name</source><translation>輸入檔案名</translation></message>
    <message><source>Select a file or enter its name</source><translation>選擇檔案或輸入名稱</translation></message>
    <message><source>File type</source><translation>檔案類型</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>可同時選擇檔案與資料夾</translation></message>
    <message><source>Select folders only.</source><translation>僅選擇資料夾</translation></message>
    <message><source>Choose a location to save the file.</source><translation>選擇檔案儲存位置</translation></message>
    <message><source>Multiple files can be selected.</source><translation>支持多選</translation></message>
    <message><source>Select a file to open.</source><translation>選擇要載入的設定檔案</translation></message>
    <message><source>Save</source><translation>儲存</translation></message>
    <message><source>Add selected items</source><translation>添加所選項</translation></message>
    <message><source>Select folder</source><translation>選擇資料夾</translation></message>
    <message><source>Open</source><translation>打開</translation></message>
    <message><source>Unable to select this item</source><translation>無法選擇此項目</translation></message>
    <message><source>Reading folder</source><translation>正在讀取資料夾</translation></message>
    <message><source>Unable to access this folder</source><translation>無法訪問此資料夾</translation></message>
    <message><source>This folder is empty</source><translation>此資料夾為空</translation></message>
    <message><source>No matching items</source><translation>沒有匹配的項目</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>正在讀取此位置的檔案與子資料夾…</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>請選擇其他資料夾，或檢查訪問權限后重試。</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>可以返回上一級，或選擇其他位置。</translation></message>
    <message><source>Try another search term.</source><translation>嘗試其他關鍵詞。</translation></message>
    <message><source> · %1 selected</source><translation> · 已選擇 %1 項</translation></message>
    <message><source>Replace the existing file?</source><translation>替換現有檔案？</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>此位置已存在檔案：
%1

儲存將覆蓋原檔案。</translation></message>
    <message><source>Replace file</source><translation>替換檔案</translation></message>
    <message><source>Folder name</source><translation>資料夾名稱</translation></message>
    <message><source>Create</source><translation>創建</translation></message>
    <message><source>Rename</source><translation>重命名</translation></message>
    <message><source>Add exclusion</source><translation>添加排除</translation></message>
    <message><source>Select this registry key</source><translation>選擇此登錄檔鍵</translation></message>
    <message><source>No registry key selected</source><translation>尚未選擇登錄檔鍵</translation></message>
    <message><source>Select an available subkey</source><translation>請選擇可用子鍵</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>全局登錄檔排除項 · 包含子鍵</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>命名值：%1 · 僅操作此值</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>整鍵及全部子鍵 · 遞歸操作</translation></message>
    <message><source>More</source><translation>更多</translation></message>
    <message><source>Compatibility mode</source><translation>相容模式</translation></message>
    <message><source>Discard changes</source><translation>放棄變更</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 項待應用</translation></message>
    <message><source>Administrator</source><translation>管理員</translation></message>
    <message><source>Read-only</source><translation>唯讀</translation></message>
    <message><source>Restore</source><translation>還原</translation></message>
    <message><source>Move</source><translation>移動</translation></message>
    <message><source>Minimize</source><translation>最小化</translation></message>
    <message><source>Maximize</source><translation>最大化</translation></message>
    <message><source>Staged path</source><translation>暫存路徑</translation></message>
    <message><source>Required configuration</source><translation>必要設定</translation></message>
    <message><source>Pending removal</source><translation>待移除</translation></message>
    <message><source>Recursive staging</source><translation>遞歸暫存</translation></message>
    <message><source>Staged file</source><translation>暫存檔案</translation></message>
    <message><source>Added</source><translation>新增</translation></message>
    <message><source>Configured</source><translation>已設定</translation></message>

    <message><source>Changes already committed are retained.</source><translation>已提交的更改會保留。</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>已處理 %1 / %2 項</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>目前目標處理完成後，可取消剩餘任務。</translation></message>

    <message><source>Fit columns to contents</source><translation>根據內容調整列寬</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>將所選路徑標記為待移除，套用後生效。</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>套用後從自動暫存清單中移除此路徑。</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>套用後將此路徑新增至自動暫存清單。</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>套用此計畫將清空已儲存的暫存清單，並在不再需要時移除相關的登錄排除項目。</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>暫存清單的變更須在 UWF 管理員中套用，無法匯出為 uwfmgr 命令。</translation></message>
    <message><source>File staging is saved after Apply</source><translation>暫存清單在套用後儲存</translation></message>
    <message><source>Overlay snapshot scope</source><translation>覆寫層快照的範圍</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>此清單僅供診斷，不能用來判斷哪些檔案必須提交。</translation></message>
    <message><source>NTFS volumes only</source><translation>僅支援 NTFS 磁碟區</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>大型覆寫層可能讀取緩慢或因資源不足而失敗。</translation></message>
    <message><source>Small files may be missing</source><translation>可能遺漏小型檔案</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>小於磁碟叢集大小（通常為 4 KB）的檔案可能不在清單中。</translation></message>
    <message><source>Does not indicate pending commits</source><translation>不代表待提交狀態</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>已提交的檔案、排除路徑及受其他操作影響的項目也可能出現。</translation></message>

    <message><source>Command processing result</source><translation>命令處理結果</translation></message>
    <message><source>Copy command</source><translation>複製命令</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 個分支讀取失敗</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 個分支無法讀取。可繼續處理已發現內容，或僅重試失敗項目。</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 個檔案 · %2 個目錄</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 個值 · %2 個機碼</translation></message>
    <message><source>Attempts</source><translation>嘗試次數</translation></message>
    <message><source>Cancel remaining tasks</source><translation>取消剩餘工作</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>僅提交已成功走訪的內容；讀取失敗的分支維持原樣。</translation></message>
    <message><source>Committing changes</source><translation>正在提交變更</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>已完成的刪除及提交無法復原。</translation></message>
    <message><source>Continue…</source><translation>繼續…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>僅刪除已成功走訪的目標並儲存刪除結果；讀取失敗的分支及其上層目錄或機碼會保留。</translation></message>
    <message><source>Deleting and committing</source><translation>正在刪除並提交</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>刪除及提交無法復原。僅處理所選範圍，不會套用待套用的 UWF 設定。</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>正在確定操作範圍。讀取失敗的分支會記錄下來，並繼續走訪其餘內容。</translation></message>
    <message><source>Export traversal records</source><translation>匯出走訪記錄</translation></message>
    <message><source>No actionable content</source><translation>沒有可操作的內容</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>所選範圍中沒有找到可操作的目標，未執行刪除或提交。</translation></message>
    <message><source>No changes have been committed.</source><translation>尚未提交任何變更</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>尚未刪除或提交任何內容</translation></message>
    <message><source>Path</source><translation>路徑</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>將永久刪除所選範圍，並將刪除結果儲存至受保護磁碟區。</translation></message>
    <message><source>Preparation complete</source><translation>準備完成</translation></message>
    <message><source>Preparing commit</source><translation>準備提交</translation></message>
    <message><source>Preparing deletion and commit</source><translation>準備刪除並提交</translation></message>
    <message><source>Process only successfully traversed content</source><translation>僅處理已成功走訪的內容</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>略過重新剖析點，確保走訪不超出所選範圍。</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>提交後無法透過重新啟動復原。僅處理所選範圍，不會套用待套用的 UWF 設定。</translation></message>
    <message><source>Retry failed items</source><translation>重試失敗項目</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>將所選範圍的變更儲存至受保護磁碟區。</translation></message>
    <message><source>Stopping the operation</source><translation>正在停止操作</translation></message>
    <message><source>Successful and failed records are retained</source><translation>已保留成功與失敗記錄</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>資料夾不是空的，未處理的內容已保留。</translation></message>
    <message><source>The operation could not be completed</source><translation>無法完成操作</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>登錄機碼中出現了新值，未處理的內容已保留。</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>登錄機碼中仍有子機碼，未處理的內容已保留。</translation></message>
    <message><source>The target no longer exists.</source><translation>目標已不存在。</translation></message>
    <message><source>Traversal complete</source><translation>走訪完成</translation></message>
    <message><source>Traversal completed with unread content</source><translation>走訪完成，部分內容無法讀取</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>走訪記錄已保留，可匯出檢視。</translation></message>
    <message><source>Traverse again</source><translation>重新走訪</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>可重新走訪，或關閉此視窗。</translation></message>
    <message><source>Traversing the folder</source><translation>正在走訪資料夾</translation></message>
    <message><source>Traversing the registry</source><translation>正在走訪登錄</translation></message>
    <message><source>Unable to prepare the operation</source><translation>無法完成操作準備</translation></message>
    <message><source>View results</source><translation>檢視結果</translation></message>

    <message><source>All changes committed</source><translation>全部提交成功</translation></message>
    <message><source>Some changes committed</source><translation>部分提交成功</translation></message>
    <message><source>No changes could be committed</source><translation>提交失敗，未能提交任何變更</translation></message>
    <message><source>No changes to commit</source><translation>沒有需要提交的變更</translation></message>
    <message><source>Commit interrupted</source><translation>提交已中斷</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 項未處理。</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 項登錄操作失敗。</translation></message>
    <message><source>The folder does not exist.</source><translation>資料夾不存在。</translation></message>
    <message><source>Access denied.</source><translation>拒絕存取。</translation></message>
    <message><source>An item with this name already exists.</source><translation>已存在同名項目。</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>無法建立資料夾。請檢查名稱和權限。</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>無法重新命名此項目。請檢查名稱和權限。</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>此瀏覽器不支援重新命名。</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>開啟此計畫後，檔案暫存草稿已變更。請重新檢視計畫。</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>開啟此計畫後，已儲存的檔案暫存清單已變更。請重新整理並檢視變更。</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>已儲存的檔案暫存清單與確認的計畫不一致。</translation></message>
    <message><source>Export configuration</source><translation>匯出設定</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>將目前的主題、語言和 HUB 顯示狀態儲存至程式所在目錄的 UWF.ini。</translation></message>
    <message><source>Configuration saved</source><translation>配置已保存</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>已將目前的介面設定儲存至：
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini 已存在，未變更現有檔案：
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>無法儲存 UWF.ini：
%1
%2</translation></message>
    <message><source>Desktop</source><translation>桌面</translation></message>
    <message><source>Downloads</source><translation>下載</translation></message>
    <message><source>Documents</source><translation>文件</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>檔案暫存已中止。請先解決錯誤，再重試。</translation></message>
    <message><source>No net change for this setting</source><translation>本次匯入後，此設定的最終值未變</translation></message>
    <message><source>Final draft %1 → %2</source><translation>草稿最終值：%1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>草稿最終值：%1 → %2（要求值為 %3）</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>要求值：%1 → %2；已由後續命令取代。草稿最終值 → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>已更新%1草稿</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>已更新磁碟區 %1 的保護設定草稿 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>請選取要移除的項目。</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>目前無法還原所選項目。</translation></message>
</context>
</TS>
