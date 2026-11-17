<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Simulation mode · UWF changes are kept in memory only</source><translation>模擬模式 · UWF 變更僅保存在記憶體中</translation></message>
    <message><source>Access denied</source><translation>拒絕存取</translation></message>
    <message><source>Text and scripts (*.txt *.bat *.ps1 *.cmd);;Text files (*.txt);;All files (*)</source><translation>文字與指令碼 (*.txt *.bat *.ps1 *.cmd);;文字檔案 (*.txt);;所有檔案 (*)</translation></message>
    <message><source>Unable to complete the operation</source><translation>無法完成操作</translation></message>
    <message><source>Select files</source><translation>選取檔案</translation></message>
    <message><source>Open configuration file</source><translation>開啟設定檔</translation></message>
    <message><source>Committed to disk</source><translation>已提交至磁碟</translation></message>
    <message><source>There are pending changes that have not been applied. Continue and discard them?</source><translation>還有尚未套用的變更。要繼續並捨棄這些變更嗎？</translation></message>
    <message><source>Access denied. Check permissions and try again.</source><translation>拒絕存取。請檢查權限後再試一次。</translation></message>
    <message><source>Save file</source><translation>儲存檔案</translation></message>
    <message><source>Preview</source><translation>檢閱</translation></message>
    <message><source>enable</source><translation>啟用</translation></message>
    <message><source>disable</source><translation>停用</translation></message>
    <message><source>Enabled</source><translation>已啟用</translation></message>
    <message><source>Disabled</source><translation>已停用</translation></message>
    <message><source>Disabled</source><comment>service startup type</comment><translation>已停用</translation></message>
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
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>此磁碟區在目前工作階段中的保護狀態（唯讀）。</translation></message>
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>下次工作階段保護此磁碟區。對受保護路徑的寫入會重新導向至覆蓋層；重新啟動後是否保留取決於覆蓋層設定。</translation></message>
    <message><source>Current session</source><translation>目前工作階段</translation></message>
    <message><source>Next session</source><translation>下次工作階段</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>UWF 識別此磁碟區的方式。磁碟機代號直觀，但增減其他磁碟等情況下磁碟機代號可能被重新分配，繫結會失效；磁碟區 ID 不隨磁碟機代號變化，更穩定。</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>此磁碟區不支援 UWF：</translation></message>
    <message><source>Global settings</source><translation>全域設定</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>UWF 篩選器在目前工作階段中的啟用狀態（唯讀）。</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>下次工作階段啟用 UWF。對受保護路徑的寫入會重新導向至覆蓋層；重新啟動後是否保留取決於覆蓋層設定。</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>若要變更覆蓋層類型或最大容量：
1. 停用 UWF。
2. 重新啟動裝置。
3. 變更此設定。</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM 使用系統記憶體。Disk 使用系統磁碟機，可提供更大的覆蓋層容量。持久覆蓋層可在重新啟動後保留 Disk 覆蓋層資料。</translation></message>
    <message><source>Type</source><translation>類型</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>RAM 覆蓋層容量受系統記憶體限制。Disk 覆蓋層至少需要 1024 MB，且系統磁碟機須有足夠的可用空間。</translation></message>
    <message><source>Maximum capacity</source><translation>最大容量</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>最大容量 · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>警告臨界值</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>覆蓋層使用量達到此值時，會將警告層級通知寫入事件記錄。兩個事件皆啟用時，此值必須低於嚴重臨界值。設為 0 只會停用警告事件。</translation></message>
    <message><source>Critical threshold</source><translation>嚴重臨界值</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>覆蓋層使用量達到此值時，會將嚴重層級通知寫入事件記錄。兩個事件皆啟用時，此值必須高於警告臨界值。設為 0 只會停用嚴重事件。</translation></message>
    <message><source>Overlay</source><translation>覆蓋層</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>不支援的磁碟機類型（僅支援本機固定磁碟）。</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>磁碟區容量超過 UWF 單個受保護磁碟區 16 TiB 的上限。</translation></message>
    <message><source>%1 free / %2</source><translation>%1 可用 / %2</translation></message>
    <message><source>Commit</source><translation>提交</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>將覆蓋層中的變更提交至磁碟或登錄。此操作無法復原。</translation></message>
    <message><source>Commit file changes…</source><translation>提交檔案修改…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>選取檔案，將其覆蓋層變更提交至磁碟。</translation></message>
    <message><source>Commit folder changes…</source><translation>提交資料夾修改…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>選取資料夾，將其中所有檔案的覆蓋層變更提交至磁碟。</translation></message>
    <message><source>Delete and commit file…</source><translation>刪除並提交檔案…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>選取要刪除的檔案，並將刪除提交至磁碟。</translation></message>
    <message><source>Delete and commit folder…</source><translation>刪除並提交資料夾…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>選取資料夾，遞迴刪除該資料夾及其全部內容，並將刪除結果提交至磁碟。</translation></message>
    <message><source>Commit registry changes…</source><translation>提交登錄檔修改…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>輸入登錄機碼及選填的值名稱，並將變更提交至登錄。將值名稱留空可遞迴提交該機碼及其所有子機碼中的每個值。</translation></message>
    <message><source>Delete and commit registry…</source><translation>刪除並提交登錄檔…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>輸入登錄機碼及選填的值名稱。將值名稱留空可刪除該機碼及其所有子機碼和值，然後提交刪除。</translation></message>
    <message><source>File exclusions</source><translation>檔案排除項目</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>此磁碟區中不受 UWF 保護的檔案與資料夾。按兩下項目可複製其路徑。</translation></message>
    <message><source>Registry exclusions</source><translation>登錄排除項目</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>登錄排除項目為全域設定，僅在 Windows 所在的磁碟區顯示。</translation></message>
    <message><source>File staging</source><translation>檔案暫存</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>將檔案或資料夾加入自動提交清單。</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>選取一個或多個要在安全關機或重新啟動前自動提交的檔案。</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>選取要在安全關機或重新啟動前遞迴提交檔案的資料夾。</translation></message>
    <message><source>Select files for automatic commit</source><translation>選取要自動提交的檔案</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>選取要遞迴自動提交的資料夾</translation></message>
    <message><source>Cannot stage this path</source><translation>無法暫存此路徑</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>此路徑與 UWF 檔案排除項目重疊。加入暫存路徑前，請先移除以下排除項目：
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>此路徑與檔案暫存項目重疊。加入排除項目前，請先移除以下暫存路徑：
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>此路徑與現有的檔案暫存項目重疊</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>檔案暫存清單中有已儲存項目時，必須保留此登錄排除項目</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>所選路徑不存在，或其類型與選取操作不符：
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>無法檢查所選路徑：
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>不能將重新剖析點加入檔案暫存：
%1</translation></message>
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>路徑沒有磁碟機代號，無法加入檔案暫存：
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>所選路徑位於磁碟區 %1。請從該磁碟區的「檔案暫存」分頁加入。</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>不能暫存磁碟區根目錄，否則會遞迴提交整個磁碟區。</translation></message>
    <message><source>File staging could not be saved</source><translation>無法儲存檔案暫存清單</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>更新未能完成，已重新載入檔案暫存的實際狀態：
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>無法使用檔案暫存：%1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>暫存項目 %1 個 · 檔案 %2 個 · 資料夾 %3 個</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>檔案暫存依賴此登錄排除項，以便在重新啟動後保留暫存清單。</translation></message>
    <message><source>Built-in</source><translation>內建</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>UWF 篩選器目前已停用，尚未累積覆蓋層變更，因此沒有可提交的內容。</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>%1 檔案系統不支援單檔案提交。</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>此磁碟區不受 UWF 支援。</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>%1 檔案系統不支援單一檔案提交。僅可提交登錄變更（登錄排除項目屬於全域設定）。</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>此磁碟區目前未受 UWF 保護，沒有可提交的檔案修改。</translation></message>
    <message><source>Select a file to commit to disk</source><translation>選取要提交到磁碟的檔案</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>選取要提交至磁碟的資料夾</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>選取要提交刪除的檔案</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>選取要提交刪除的資料夾</translation></message>
    <message><source>Name</source><translation>名稱</translation></message>
    <message><source>Data</source><translation>資料</translation></message>
    <message><source>(Default)</source><translation>（預設）</translation></message>
    <message><source>(value not set)</source><translation>（數值未設定）</translation></message>
    <message><source>(empty)</source><translation>（空）</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>無法檢閱（值過大）</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>開啟診斷檢視，查看此磁碟區目前快取於覆蓋層中的檔案。</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>僅 NTFS 磁碟區支援檢視覆蓋層檔案清單。</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>UWF 篩選器目前已停用，所有磁碟區均無可檢視的覆蓋層。</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>此磁碟區目前未被 UWF 保護，沒有可檢視的覆蓋層。</translation></message>
    <message><source>Failed: %1</source><translation>失敗：%1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>上一次列舉可能仍在執行，目前無法開始新的讀取。</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>列舉覆蓋層檔案時，UWF 服務無法使用。</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>記憶體不足，或系統不支援此操作。此功能僅支援 NTFS 磁碟區；請先關閉佔用記憶體較多的程式後重試。</translation></message>
    <message><source>Raw error: %1</source><translation>原始錯誤：%1</translation></message>
    <message><source>Export overlay file list</source><translation>匯出覆蓋層檔案清單</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>文字檔案 (*.txt);;所有檔案 (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>命令檔案 (*.cmd);;所有檔案 (*)</translation></message>
    <message><source>Export failed</source><translation>匯出失敗</translation></message>
    <message><source>Export finished</source><translation>匯出完成</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>無法開啟檔案寫入：%1</translation></message>
    <message><source>Could not write file: %1</source><translation>無法寫入檔案：%1</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>已將 %1 個項目儲存至：
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>不允許把整個磁碟區 %1 加入排除清單。</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>分頁檔案、交換檔案及休眠檔案不能加入排除項目。</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>不允許把整個 \Windows 資料夾加入排除。</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>不允許把整個 \Windows\System32 資料夾加入排除。</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>不允許把整個 \Windows\System32\Drivers 資料夾加入排除。</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>不允許把關鍵系統檔案加入排除：%1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>不允許把使用者登錄檔檔案 NTUSER.DAT 加入排除。</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>登錄機碼不能為空。</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>登錄機碼不能以反斜線開頭。</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>登錄路徑不能包含空字元。</translation></message>
    <message><source>A registry key component is too long.</source><translation>登錄機碼名稱過長。</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>登錄路徑超過 UWF 的巢狀層數限制。</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>將檔案或資料夾加入排除清單。</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>選取一個檔案加入排除清單。</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>選取一個資料夾（包含其中所有內容）加入排除清單。</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>手動輸入一個登錄機碼路徑加入排除清單。</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>把一個登錄機碼加入排除清單，或開啟一個持久化開關。</translation></message>
    <message><source>Registry key…</source><translation>登錄機碼…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>網域機密金鑰 (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>終端機服務用戶端存取授權 (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>跨 UWF 工作階段持久化網域機密金鑰（電腦帳戶密碼）。</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>跨 UWF 工作階段持久化終端機服務用戶端存取授權。</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ %1 持久化：%2</translation></message>
    <message><source>Remove selected</source><translation>移除所選</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>從排除清單中移除選取項（套用後生效）。</translation></message>
    <message><source>Open containing folder</source><translation>開啟所在資料夾</translation></message>
    <message><source>Copy file path</source><translation>複製檔案路徑</translation></message>
    <message><source>Copy registry path</source><translation>複製登錄檔路徑</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>提交資料夾變更到磁碟…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>提交檔案變更到磁碟…</translation></message>
    <message><source>Copied to clipboard</source><translation>已複製到剪貼簿</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>選取要加入排除的檔案（可多選）</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>選取要加入排除的資料夾</translation></message>
    <message><source>Add registry exclusion</source><translation>新增登錄檔排除</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>所選路徑 %1 不在 %2 磁碟區上，無法作為此磁碟區的排除項。</translation></message>
    <message><source>Path is not on this volume</source><translation>路徑不在目前磁碟區</translation></message>
    <message><source>Cannot add this exclusion</source><translation>不能新增此排除項</translation></message>
    <message><source>File: %1</source><translation>檔案：%1</translation></message>
    <message><source>Folder: %1</source><translation>資料夾：%1</translation></message>
    <message><source>Registry: %1</source><translation>登錄檔：%1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>預設排除，可隨時變更。</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>目前工作階段：%1
下次工作階段：%2</translation></message>
    <message><source>Pending change: %1</source><translation>本次待套用：%1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>統一寫入篩選器 (UWF) 管理工具</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>重新讀取 UWF 的目前工作階段狀態和下次工作階段設定。</translation></message>
    <message><source>Review and apply</source><translation>檢閱並套用</translation></message>
    <message><source>Review all pending changes and apply them in one batch.</source><translation>檢閱本次所有待套用的變更，確認後一次性套用。</translation></message>
    <message><source>Safe shutdown</source><translation>安全關機</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>安全關機：即使 UWF 覆蓋層已滿也能正常關機。</translation></message>
    <message><source>Safe restart</source><translation>安全重新啟動</translation></message>
    <message><source>Direct restart</source><translation>直接重新啟動</translation></message>
    <message><source>Direct shutdown</source><translation>直接關機</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>安全重新啟動：即使 UWF 覆蓋層已滿也能正常重新啟動。</translation></message>
    <message><source>Log</source><translation>記錄</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>檢視本次工作階段累積的內部記錄（用於排查問題）。</translation></message>
    <message><source>About</source><translation>關於</translation></message>
    <message><source>Switch display language</source><translation>切換顯示語言</translation></message>
    <message><source>Language</source><translation>語言</translation></message>
    <message><source>Theme</source><translation>佈景主題</translation></message>
    <message><source>Toggle light / dark theme</source><translation>切換淺色／深色佈景主題</translation></message>
    <message><source>System information</source><translation>系統資訊</translation></message>
    <message><source>Diagnostic snapshot</source><translation>診斷快照</translation></message>
    <message><source>Read only</source><translation>唯讀</translation></message>
    <message><source>The system did not find the target.</source><translation>系統未找到目標。</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>系統拒絕了傳入的引數（路徑或引數非法）。</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>操作失敗。目標可能正被其他程式佔用（如檔案總管正在瀏覽該資料夾、檔案被某個程式開啟）。請關閉佔用它的程式後重試。</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>操作失敗（詳細資訊見記錄）。</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>操作遭拒絕（錯誤碼 %1）。</translation></message>
    <message><source>Unknown cause.</source><translation>原因不明。</translation></message>
    <message><source>Commit canceled</source><translation>提交已取消</translation></message>

    <message><source>Succeeded</source><translation>成功</translation></message>
    <message><source>Skipped</source><translation>略過</translation></message>
    <message><source>Failed</source><translation>失敗</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>%1 項成功；%2 項失敗。</translation></message>

    <message><source>Existed before</source><translation>刪除前存在</translation></message>
    <message><source>Exists after</source><translation>刪除後存在</translation></message>
    <message><source>Error code</source><translation>錯誤碼</translation></message>
    <message><source>Reason</source><translation>原因</translation></message>
    <message><source>Previous page</source><translation>上一頁</translation></message>
    <message><source>First page</source><translation>首頁</translation></message>
    <message><source>Last page</source><translation>末頁</translation></message>
    <message><source>Next page</source><translation>下一頁</translation></message>
    <message><source>Copy selected rows</source><translation>複製選取的列</translation></message>
    <message><source>Copy all</source><translation>複製全部</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>檔案暫存清單中有已儲存項目時，必須保留此登錄排除項目。請先清空檔案暫存清單，再移除此排除項目。</translation></message>
    <message><source>Copy current page</source><translation>複製目前頁</translation></message>
    <message><source>No pending changes</source><translation>無待套用變更</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation>（同時管理全域登錄檔排除）</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>切換到磁碟區 %1 的保護設定、檔案排除與檔案暫存。%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>無法使用 UWF 命名空間</translation></message>
    <message><source>Review and apply changes</source><translation>檢閱並套用變更</translation></message>
    <message><source>Confirm apply</source><translation>確認套用</translation></message>
    <message><source>Result</source><translation>套用結果</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ 篩選器：%1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ 覆蓋層 警告臨界值設為 %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ 覆蓋層 嚴重臨界值設為 %1 MB</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ 未套用類型／最大容量：UWF 已啟用。請先停用 UWF 並重新啟動。</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ 覆蓋層 類型設為 %1</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ 覆蓋層最大容量已設為 %1 MB</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ 磁碟區 %1 保護：%2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ 磁碟區 %1 繫結方式：%2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ 磁碟區 %1 新增檔案排除：%2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ 磁碟區 %1 移除檔案排除：%2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ 新增登錄檔排除：%1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ 移除登錄檔排除：%1</translation></message>
    <message><source>Time</source><translation>時間</translation></message>
    <message><source>Level</source><translation>級別</translation></message>

    <message><source>Message</source><translation>內容</translation></message>
    <message><source>Committing staged files</source><translation>正在提交暫存檔案</translation></message>
    <message><source>No additional error details were provided.</source><translation>未提供更多錯誤詳細資訊。</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>自動檔案暫存意外停止：
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>自動檔案暫存因不明錯誤而停止。</translation></message>
    <message><source>Safe shutdown failed</source><translation>安全關機失敗</translation></message>
    <message><source>Shutdown failed: %1</source><translation>關機失敗：%1</translation></message>
    <message><source>Safe restart failed</source><translation>安全重新啟動失敗</translation></message>
    <message><source>Direct restart failed</source><translation>直接重新啟動失敗</translation></message>
    <message><source>Direct shutdown failed</source><translation>直接關機失敗</translation></message>
    <message><source>Restart failed: %1</source><translation>重新啟動失敗：%1</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>無法讀取檔案暫存清單：
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>因不明錯誤而無法讀取檔案暫存清單。</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>找到的檔案：%1 · 已提交：%2 · 略過檔案：%3 · 略過項目：%4 · 失敗：%5</translation></message>
    <message><source>File staging preparation</source><translation>檔案暫存準備</translation></message>
    <message><source>Reparse points are not supported.</source><translation>不支援重新剖析點。</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>僅可暫存本機磁碟區上的絕對路徑。</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>僅可將本機磁碟區上的絕對路徑加入自動檔案暫存：
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>無法檢查暫存路徑。</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>暫存檔案目前已變為其他路徑類型。</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>暫存資料夾目前已變為其他路徑類型。</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>無法完整列舉暫存資料夾。</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>不能遞迴處理磁碟區根目錄。</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>UWF 提供程式拒絕了提交請求。</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>因不明錯誤而提交失敗。</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>另有 %1 項失敗已寫入記錄。</translation></message>



    <message><source>No current-session record found for volume %1.</source><translation>找不到磁碟區 %1 的目前工作階段記錄。</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>此路徑在檔案排除清單中。
排除項：%1</translation></message>

    <message><source>Continue</source><translation>繼續</translation></message>
    <message><source>Delete and commit</source><translation>刪除並提交</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>此機碼在登錄排除清單中。
排除項目：%1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>找不到目前工作階段的登錄檔篩選記錄。</translation></message>

    <message><source>Export commands to file</source><translation>匯出命令到檔案</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>已將 %1 個命令匯出至：
%2</translation></message>
    <message><source>Import</source><translation>匯入</translation></message>
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>將 uwfmgr 命令匯入草稿。只有在「檢閱並套用」中確認後，才會寫入變更。</translation></message>
    <message><source>Load from file…</source><translation>從檔案載入…</translation></message>
    <message><source>Load recommended configuration</source><translation>載入建議設定</translation></message>
    <message><source>Choose recommended configuration</source><translation>選取建議設定</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>選取要追加的建議設定分組。匯入前仍可檢查或刪除單條命令。</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>來自 Microsoft 官方文件。</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Microsoft 建議的 UWF 排除規則</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>匯入前請先檢查這些建議項目。</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>來源：Microsoft 官方 UWF 文件，包括常見寫入篩選器排除項以及反惡意軟體支援。</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>客戶體驗改善計劃 (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP：持久化原則參與狀態</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP：持久化本機參與狀態</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP：持久化上傳停用標誌</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>背景智慧傳輸服務 (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS：持久化下載佇列檔案</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS：持久化傳輸狀態索引</translation></message>
    <message><source>Network profiles and policies</source><translation>網路設定檔和原則</translation></message>
    <message><source>Wireless network GPO policy</source><translation>無線網路 GPO 原則</translation></message>
    <message><source>Wired network GPO policy</source><translation>有線網路 GPO 原則</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>無線網路 GPO 原則檔案</translation></message>
    <message><source>Wired network GPO policy files</source><translation>有線網路 GPO 原則檔案</translation></message>
    <message><source>Wireless network interface profiles</source><translation>無線網路介面設定</translation></message>
    <message><source>Wired network interface profiles</source><translation>有線網路介面設定</translation></message>
    <message><source>Wireless service configuration</source><translation>無線服務設定</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>行動寬頻服務設定</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>有線自動設定服務設定</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>網路設定 XML 檔案因裝置而異；如有需要，請手動新增具體的 Interfaces\{GUID}\{GUID}.xml 路徑。</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>日光節約時間 (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST：持久化時區定義</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST：持久化目前時區資訊</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender：持久化程式檔案和更新</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender：持久化 ProgramData 中的簽名和狀態</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender：持久化 Windows Update 記錄</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender：持久化 MpCmdRun 記錄</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender：持久化產品登錄檔狀態</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender：持久化 WdBoot 服務狀態</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender：持久化 WdFilter 服務狀態</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender：持久化 WdNisSvc 服務狀態</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender：持久化 WdNisDrv 服務狀態</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender：持久化 WinDefend 服務狀態</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP：持久化用戶端程式檔案</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP：持久化 Windows Update 記錄</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP：持久化 MpCmdRun 記錄</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP：持久化反惡意軟體簽名和狀態</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP：持久化反惡意軟體登錄檔狀態</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>選取包含 uwfmgr 命令的檔案</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>文字檔案 (*.txt *.cmd);;所有檔案 (*)</translation></message>
    <message><source>Status</source><translation>狀態</translation></message>
    <message><source>Import failed</source><translation>匯入失敗</translation></message>
    <message><source>Command import is not ready.</source><translation>命令匯入尚未就緒。</translation></message>
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
    <message><source>Discard pending changes?</source><translation>放棄待套用變更？</translation></message>
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>有 %1 項變更尚未套用。

要捨棄這些變更並繼續嗎？</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>uwfmgr 命令不完整</translation></message>
    <message><source>Missing size argument (MB)</source><translation>缺少大小引數（MB）</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>容量必須為非負整數（MB）</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>缺少覆蓋層類型引數（RAM 或 Disk）</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>未知的覆蓋層類型 %1（應為 RAM 或 Disk）</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>缺少磁碟區引數（如 C:）</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>磁碟區必須是磁碟機代號形式（如 C:）</translation></message>
    <message><source>Missing path argument</source><translation>缺少路徑引數</translation></message>
    <message><source>Missing registry key argument</source><translation>缺少登錄機碼引數</translation></message>
    <message><source>Unclosed double quote in command</source><translation>命令中存在未閉合的雙引號</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>存在多餘引數：%1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>無法匯入此 uwfmgr 命令。</translation></message>
    <message><source>Already in the target state — no-op</source><translation>已經處於目標狀態——無須變更</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>路徑不在此磁碟區，或此磁碟區不支援檔案排除（如 exFAT / ReFS）</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>無法將此路徑新增為排除項目。</translation></message>
    <message><source>Invalid size value: %1</source><translation>大小數值非法：%1</translation></message>
    <message><source>maximum capacity</source><translation>最大容量</translation></message>
    <message><source>warning threshold</source><translation>警告臨界值</translation></message>
    <message><source>critical threshold</source><translation>嚴重臨界值</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>全域設定目前無法使用，未將此命令加入待套用變更。</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>未知的磁碟區 %1（沒有相符的磁碟區組態）</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>磁碟區 %1 的保護狀態已是目標值</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>路徑 %1 沒有磁碟機代號，無法識別所屬的磁碟區分頁。</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>沒有磁碟機代號 %1 對應的 UWF 可保護磁碟</translation></message>
    <message><source>file exclusion</source><translation>檔案排除</translation></message>
    <message><source>registry exclusion</source><translation>登錄檔排除</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>登錄檔排除只能在登錄檔分頁中設定，但目前沒有該分頁</translation></message>
    <message><source>Unsupported command</source><translation>不支援的命令</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>顯示或隱藏覆蓋層小工具。</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF：已啟用</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF：已停用</translation></message>
    <message><source>UWF status unavailable</source><translation>無法取得 UWF 狀態</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>已用 %1 MB / 總計 %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>增強模式</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>將檔案暫存自動提交與 Windows 關機、重新啟動流程聯動。</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>讀取增強模式狀態時發生未知錯誤。</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>無法讀取服務的目前狀態：
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>讀取服務目前狀態時發生未知錯誤。</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>服務未設定為獨立程序。</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>服務未設定為自動啟動。</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>服務未設定為以 LocalSystem 身分執行。</translation></message>
    <message><source>The service is not running.</source><translation>服務目前未執行。</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>服務設定的執行檔路徑或啟動引數與目前程式不一致。</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>服務的預關機逾時設定不正確。</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>服務未宣告啟動 UI 代理所需的權限。</translation></message>
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>目前的服務無法接收 Windows 預關機通知。</translation></message>
    <message><source>Enable enhanced mode</source><translation>啟用增強模式</translation></message>
    <message><source>Disable enhanced mode</source><translation>關閉增強模式</translation></message>
    <message><source>Start service</source><translation>啟動服務</translation></message>
    <message><source>Repair enhanced mode</source><translation>修復增強模式</translation></message>
    <message><source>Delete service</source><translation>刪除服務</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>停止並刪除增強模式服務，同時清理殘留的服務登錄檔資料。</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>服務已不存在，但仍有服務登錄檔資料殘留。</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>正在等待 UI 代理完成身分認證。</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>服務正在執行，但目前沒有透過身分認證的 UI 代理連線。</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>UWF Manager 增強模式輔助服務</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>增強模式設定已套用。</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>服務設定已套用，但 UWF 持久化操作報告了以下問題：
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>增強模式設定失敗：
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>增強模式設定因未知錯誤而失敗。</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>增強模式服務及其殘留設定已刪除。</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>已請求刪除服務，但殘留清理報告了以下問題：
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>無法刪除增強模式服務：
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>刪除增強模式服務時發生未知錯誤。</translation></message>
    <message><source>Show main window</source><translation>顯示主視窗</translation></message>
    <message><source>Restore default position</source><translation>還原預設位置</translation></message>
    <message><source>Show overlay widget</source><translation>顯示覆蓋層小工具</translation></message>
    <message><source>Hide overlay widget</source><translation>隱藏覆蓋層小工具</translation></message>
    <message><source>Exit application</source><translation>結束程式</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>讀取登錄值失敗：%1</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>列舉覆蓋層檔案時發生意外錯誤。</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ %1 篩選器失敗</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ 讀取篩選器狀態失敗</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ 設定警告臨界值失敗</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ 設定嚴重臨界值失敗</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ 讀取覆蓋層狀態失敗</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ 設定覆蓋層類型失敗</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ 無法設定最大容量</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ 讀取覆蓋層設定失敗</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ 磁碟區 %2 %1 保護失敗</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ 設定磁碟區 %1 的繫結方式失敗</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ 磁碟區 %1 新增檔案排除項 %2 失敗</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ 磁碟區 %1 移除檔案排除項 %2 失敗</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ 讀取磁碟區設定失敗</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ 新增登錄檔排除項 %1 失敗</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ 移除登錄檔排除項 %1 失敗</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ 更新登錄檔持久化開關失敗</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ 讀取登錄檔篩選器失敗</translation></message>
    <message><source>UWF is not available</source><translation>無法使用 UWF</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>讀取目標狀態時發生未知錯誤。</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>操作因未知錯誤而失敗。</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>無法驗證目標狀態，因此操作結果不明：%1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>系統已接受刪除要求，但驗證後目標仍存在。</translation></message>
    <message><source>Unknown</source><translation>未知</translation></message>
    <message><source>Failed to load logs: %1</source><translation>載入記錄失敗：%1</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>載入記錄時發生意外錯誤。</translation></message>
    <message><source>: unknown error</source><translation>：未知錯誤</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>解析目標磁碟區失敗：%1</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>列舉登錄機碼失敗：%1</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>無法解析路徑 %1 所在的磁碟區：%2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>前置操作失敗或無法確認，已略過此操作。</translation></message>
    <message><source>Copy</source><translation>複製</translation></message>
    <message><source>Select All</source><translation>全部選取</translation></message>
    <message><source>Display language</source><translation>顯示語言</translation></message>
    <message><source>Folder</source><translation>資料夾</translation></message>
    <message><source>File</source><translation>檔案</translation></message>
    <message><source>Export commands</source><translation>匯出命令</translation></message>
    <message><source>Direct restart…</source><translation>直接重新啟動…</translation></message>
    <message><source>Clear…</source><translation>清空…</translation></message>
    <message><source>Committing</source><translation>提交中</translation></message>
    <message><source>Target</source><translation>目標</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>變更覆蓋層類型或最大容量前，請先停用 UWF 並重新啟動。</translation></message>
    <message><source>Disk</source><translation>磁碟</translation></message>
    <message><source>Protection and overlay</source><translation>保護與覆蓋層</translation></message>
    <message><source>Write protection</source><translation>寫入保護</translation></message>
    <message><source>Overlay type</source><translation>覆蓋層類型</translation></message>
    <message><source>Storage</source><translation>儲存介質</translation></message>
    <message><source>Maximum overlay capacity</source><translation>覆蓋層最大容量</translation></message>
    <message><source>Volume %1 protection</source><translation>磁碟區 %1 保護</translation></message>
    <message><source>By volume ID</source><translation>按磁碟區 ID</translation></message>
    <message><source>By drive letter</source><translation>按磁碟機代號</translation></message>
    <message><source>Volume %1 binding</source><translation>磁碟區 %1 繫結</translation></message>
    <message><source>Volume identity</source><translation>磁碟區識別</translation></message>
    <message><source>Removed</source><translation>移除</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>將此路徑加入下次工作階段的排除清單。</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>從下次工作階段的排除項目中移除此路徑。</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>將此機碼加入下次工作階段的排除清單。</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>從下次工作階段的排除項目中移除此機碼。</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>網域機密金鑰持久化</translation></message>
    <message><source>Terminal Services license persistence</source><translation>終端機服務授權持久化</translation></message>
    <message><source>Line</source><translation>行</translation></message>
    <message><source>Original command</source><translation>原始命令</translation></message>
    <message><source>Processing result</source><translation>處理結果</translation></message>
    <message><source>Imported</source><translation>已匯入</translation></message>
    <message><source>Error</source><translation>錯誤</translation></message>
    <message><source>Warning</source><translation>警告</translation></message>
    <message><source>Information</source><translation>資訊</translation></message>
    <message><source>Debug</source><translation>偵錯</translation></message>
    <message><source>Module</source><translation>模組</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>清空記錄失敗：%1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>清空記錄失敗：發生意外錯誤。</translation></message>
    <message><source>Name and location</source><translation>名稱與位置</translation></message>
    <message><source>Size</source><translation>大小</translation></message>
    <message><source>System metadata</source><translation>系統中繼資料</translation></message>
    <message><source>Date modified</source><translation>修改日期</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>請選取限定磁碟或資料夾內的位置。</translation></message>
    <message><source>Unable to read this folder.</source><translation>無法讀取此資料夾。</translation></message>
    <message><source>All files (*)</source><translation>所有檔案 (*)</translation></message>
    <message><source>The destination folder does not exist.</source><translation>目標資料夾不存在。</translation></message>
    <message><source>Enter a valid file name.</source><translation>請輸入有效的檔案名稱。</translation></message>
    <message><source>The selected item no longer exists: %1</source><translation>所選項目已不存在：%1</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>請選取檔案，或開啟資料夾瀏覽其內容。</translation></message>
    <message><source>Select a folder.</source><translation>請選取資料夾。</translation></message>
    <message><source>Enter a valid folder name.</source><translation>請輸入有效的資料夾名稱。</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>請輸入有效的檔案或資料夾名稱。</translation></message>
    <message><source>Review the result for each target.</source><translation>查看各目標的操作結果。</translation></message>

    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>部分項目未完成或無法驗證。選取項目可查看完整路徑及詳細資訊。</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>所選操作已完成。UWF 可能略過排除項目。</translation></message>
    <message><source>Item details</source><translation>項目詳細資訊</translation></message>
    <message><source>Done</source><translation>完成</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>核對本次變更，確認後寫入系統。</translation></message>
    <message><source>%1 changes</source><translation>%1 項變更</translation></message>
    <message><source>Takes effect after restart</source><translation>下次啟動生效</translation></message>
    <message><source>Takes effect when applied</source><translation>套用時生效</translation></message>
    <message><source>Used when UWF starts</source><translation>UWF 啟動時使用</translation></message>
    <message><source>Mixed activation times</source><translation>生效時間不同</translation></message>
    <message><source>View commands for these changes</source><translation>查看本次變更的命令</translation></message>
    <message><source>Pending changes</source><translation>待套用變更</translation></message>
    <message><source>Next-session configuration</source><translation>下次工作階段設定</translation></message>
    <message><source>%1 commands</source><translation>%1 條命令</translation></message>

    <message><source>Apply changes</source><translation>套用變更</translation></message>
    <message><source>Some changes could not be applied</source><translation>部分變更未能套用</translation></message>
    <message><source>Configuration applied</source><translation>設定已套用</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>部分變更可能已寫入。重試前請重新整理系統狀態。</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>設定已寫入系統。</translation></message>
    <message><source>Application result</source><translation>套用結果</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>將這些設定變更寫入系統？</translation></message>
    <message><source>Search paths</source><translation>搜尋路徑</translation></message>
    <message><source>Add files or folders</source><translation>新增檔案或資料夾</translation></message>
    <message><source>File or folder</source><translation>檔案或資料夾</translation></message>
    <message><source>Registry key or persistence setting</source><translation>登錄機碼或保留設定</translation></message>
    <message><source>%1 selected</source><translation>已選取 %1 項</translation></message>
    <message><source>Undo removal</source><translation>復原移除</translation></message>
    <message><source>%1 items</source><translation>%1 項</translation></message>
    <message><source>Filter is disabled</source><translation>篩選器未啟用</translation></message>
    <message><source>Usage unavailable</source><translation>無法取得使用量</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>覆蓋層已達到嚴重臨界值。請儲存變更，並盡快安全重新啟動。</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>覆蓋層已達到警告臨界值。請儲存變更，再安全重新啟動。</translation></message>
    <message><source>UWF filter</source><translation>UWF 篩選器</translation></message>
    <message><source>Current usage</source><translation>目前使用量</translation></message>
    <message><source>Warning %1 MB</source><translation>警告 %1 MB</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>啟用並重新啟動後顯示實時用量</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>讀取設定後顯示實時用量</translation></message>
    <message><source>Critical %1 MB</source><translation>嚴重 %1 MB</translation></message>
    <message><source>On</source><translation>開啟</translation></message>
    <message><source>Off</source><translation>關閉</translation></message>
    <message><source>Import configuration</source><translation>匯入設定</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>從腳本或文本中新增 UWF 設定。</translation></message>
    <message><source>One uwfmgr command per line</source><translation>每行一條 uwfmgr 命令</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr 命令</translation></message>
    <message><source>Editable</source><translation>可編輯</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>貼上命令，或從檔案載入…</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>匯入會更新草稿。請在「檢閱並套用」中確認變更，才會寫入系統。</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>清單顯示命令對設定草稿的處理結果；選取一列可檢視完整命令與詳細資訊。</translation></message>
    <message><source>Edit commands</source><translation>編輯命令</translation></message>
    <message><source>Import results</source><translation>匯入結果</translation></message>
    <message><source>Not written to the system</source><translation>尚未寫入系統</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 行</translation></message>
    <message><source>Continue editing</source><translation>繼續編輯</translation></message>
    <message><source>Imported: %1</source><translation>已匯入 %1</translation></message>
    <message><source>Binding</source><translation>繫結</translation></message>
    <message><source>Protected</source><translation>已保護</translation></message>
    <message><source>Unprotected</source><translation>未保護</translation></message>
    <message><source>View overlay</source><translation>查看覆蓋層</translation></message>
    <message><source>UWF is currently unavailable</source><translation>目前無法使用 UWF</translation></message>
    <message><source>Select registry key or value</source><translation>選取登錄機碼或值</translation></message>
    <message><source>Select registry key or value to delete</source><translation>選取要刪除的登錄機碼或值</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>未能讀取 UWF 設定。確認 Windows 已安裝統一寫入篩選器後重新讀取。</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>無法使用增強模式。請使用「安全關機」或「安全重新啟動」提交暫存檔案。</translation></message>
    <message><source>Version %1</source><translation>版本 %1</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Windows 統一寫入篩選器的圖形管理工具。
管理磁碟區保護、覆蓋層、排除項與檔案暫存。</translation></message>
    <message><source>Source code</source><translation>原始碼</translation></message>
    <message><source>Contact author</source><translation>聯絡作者</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>寫入保護由 Windows 內置 UWF 提供，本程式負責設定與管理。</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>首次啟用 UWF 對系統的影響</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows 會停用分頁檔案、系統還原、SuperFetch、檔案索引、磁碟重組和快速啟動，並將 BCD 的 bootstatuspolicy 設為 ignoreallfailures。&lt;/p&gt;&lt;p&gt;之後可按需調整。例如，將分頁檔案移至未保護磁碟區後重新啟用分頁。&lt;/p&gt;</translation></message>
    <message><source>License and copyright</source><translation>授權與著作權</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;采用 &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt; 發布，完整條款見程式隨附的 LICENSE 檔案。&lt;/p&gt;&lt;p&gt;可按 GPL v3 條款再分發或修改。軟體按原樣提供，不附帶任何保證。&lt;/p&gt;</translation></message>
    <message><source>Copy version</source><translation>複製版本</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>查看執行記錄與診斷資訊。</translation></message>
    <message><source>Log level</source><translation>記錄等級</translation></message>
    <message><source>All levels</source><translation>全部等級</translation></message>
    <message><source>Current run</source><translation>本次執行</translation></message>
    <message><source>Search messages or modules</source><translation>搜尋訊息或模組</translation></message>
    <message><source>Search logs</source><translation>搜尋記錄</translation></message>
    <message><source>Message details</source><translation>訊息詳細資訊</translation></message>
    <message><source>Error details</source><translation>錯誤詳細資訊</translation></message>
    <message><source>Clear logs?</source><translation>清空記錄？</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>目前的記錄將被清空，且無法復原。</translation></message>
    <message><source>Clear logs</source><translation>清空記錄</translation></message>
    <message><source>Copy all logs</source><translation>複製全部記錄</translation></message>
    <message><source>Reading logs</source><translation>正在讀取記錄</translation></message>
    <message><source>Unable to read logs</source><translation>無法讀取記錄</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>讀取完成後將在此顯示本次執行記錄。</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>記錄讀取失敗。請重試，或檢查記錄檔案的存取權限。</translation></message>
    <message><source>Cancel reading</source><translation>取消讀取</translation></message>
    <message><source>Read again</source><translation>重新讀取</translation></message>
    <message><source>No matching logs</source><translation>沒有符合的記錄</translation></message>
    <message><source>No logs yet</source><translation>暫無記錄</translation></message>
    <message><source>Try another keyword or log level.</source><translation>嘗試調整關鍵字或記錄等級。</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>新的執行記錄將在重新整理後顯示。</translation></message>
    <message><source>Clear filters</source><translation>清除篩選</translation></message>
    <message><source>%1 records</source><translation>%1 條記錄</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 條記錄 · 共 %2 條</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>系統、權限與 UWF 的診斷快照。</translation></message>
    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>報告可能包含帳戶名、裝置標識和檔案路徑，分享前請先檢查。</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>部分硬體資訊未能讀取，其餘診斷資訊仍可查看和複製。</translation></message>
    <message><source>Overview</source><translation>概覽</translation></message>
    <message><source>Raw report</source><translation>原始報告</translation></message>
    <message><source>Copy full report</source><translation>複製完整報告</translation></message>
    <message><source>Delete and commit this file?</source><translation>刪除並提交檔案？</translation></message>
    <message><source>Commit file changes?</source><translation>提交檔案變更？</translation></message>
    <message><source>Delete and commit this folder?</source><translation>刪除並提交資料夾？</translation></message>
    <message><source>Commit folder changes?</source><translation>提交資料夾變更？</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>刪除並提交登錄機碼？</translation></message>
    <message><source>Commit registry key changes?</source><translation>提交登錄機碼變更？</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>刪除並提交登錄值？</translation></message>
    <message><source>Commit registry value changes?</source><translation>提交登錄值變更？</translation></message>

    <message><source>Commit changes</source><translation>提交變更</translation></message>
    <message><source>Service stopped</source><translation>服務已停止</translation></message>
    <message><source>Repair required</source><translation>需要修復</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>服務已安裝但已停止。請啟動服務以恢復自動檔案提交。</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>無法讀取增強模式服務狀態。請檢查服務權限後重試。</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>與 Windows 關機流程協同</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>在 Windows 關機及重新啟動時，自動提交符合條件的暫存檔案。</translation></message>
    <message><source>Background service</source><translation>背景服務</translation></message>
    <message><source>Starts automatically with Windows</source><translation>隨 Windows 自動啟動</translation></message>
    <message><source>Automatic staged file commits</source><translation>自動提交暫存檔案</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>未使用增強模式時，請使用本程式的「安全關機」或「安全重新啟動」提交暫存檔案。</translation></message>
    <message><source>Operation and security</source><translation>執行與安全說明</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>增強模式會新增一個隨 Windows 啟動的背景服務。移動或刪除程式前，請先移除該服務。</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>請將程式放在一般使用者無法修改的固定資料夾中。建議位置：</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>若程式所在的磁碟機受 UWF 保護，請先確認重新啟動後程式檔案仍會保留。提交大量暫存檔案可能使關機或重新啟動花費較長時間。</translation></message>
    <message><source>Service details</source><translation>服務詳細資訊</translation></message>
    <message><source>Configuration error</source><translation>設定異常</translation></message>
    <message><source>Running</source><translation>正在執行</translation></message>
    <message><source>Service status</source><translation>服務狀態</translation></message>
    <message><source>Stopped</source><translation>已停止</translation></message>
    <message><source>Not installed</source><translation>尚未安裝</translation></message>
    <message><source>Ready</source><translation>就緒</translation></message>
    <message><source>Unavailable</source><translation>無法使用</translation></message>
    <message><source>Service name</source><translation>服務名稱</translation></message>
    <message><source>Account</source><translation>帳戶</translation></message>
    <message><source>Startup type</source><translation>啟動類型</translation></message>
    <message><source>Automatic</source><translation>自動</translation></message>
    <message><source>Not configured</source><translation>尚未設定</translation></message>
    <message><source>Application path</source><translation>程式路徑</translation></message>
    <message><source>UI agent</source><translation>介面代理</translation></message>
    <message><source>Connected and authenticated</source><translation>已連接 · 身分已驗證</translation></message>
    <message><source>64-bit</source><translation>64 位元</translation></message>
    <message><source>32-bit</source><translation>32 位元</translation></message>
    <message><source>System</source><translation>系統</translation></message>
    <message><source>Operating system</source><translation>作業系統</translation></message>
    <message><source>Architecture</source><translation>架構</translation></message>
    <message><source>System memory</source><translation>系統記憶體</translation></message>
    <message><source>Permissions and protection</source><translation>權限與保護</translation></message>
    <message><source>Privileges</source><translation>權限</translation></message>
    <message><source>Standard user</source><translation>標準使用者</translation></message>
    <message><source>UWF feature</source><translation>UWF 功能</translation></message>
    <message><source>Available</source><translation>可用</translation></message>
    <message><source>Current filter</source><translation>目前篩選器</translation></message>
    <message><source>Protected volumes</source><translation>已保護磁碟區</translation></message>
    <message><source>None</source><translation>無</translation></message>
    <message><source>Connecting…</source><translation>正在連接…</translation></message>
    <message><source>Not connected</source><translation>未連接</translation></message>
    <message><source>Service account</source><translation>服務帳戶</translation></message>
    <message><source>Service configuration needs repair</source><translation>服務設定需要修復</translation></message>
    <message><source>Retry</source><translation>重試</translation></message>
    <message><source>Status unavailable</source><translation>無法取得狀態</translation></message>
    <message><source>Unable to read service status</source><translation>服務狀態讀取失敗</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>查看 %1 的檔案診斷快照。</translation></message>
    <message><source>Local disk (%1)</source><translation>本機磁碟 (%1)</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>清單可能遺漏檔案，也可能包含已提交的檔案；請勿將其作為提交清單。</translation></message>
    <message><source>Snapshot details</source><translation>快照詳細資訊</translation></message>
    <message><source>Commit changes…</source><translation>提交變更…</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>系統中繼資料僅供診斷，不提供提交操作。</translation></message>
    <message><source>Error details and suggestions</source><translation>錯誤詳細資訊與建議</translation></message>
    <message><source>Export list</source><translation>匯出清單</translation></message>
    <message><source>Reading overlay</source><translation>正在讀取覆蓋層</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>暫時無法讀取覆蓋層</translation></message>
    <message><source>Reading canceled</source><translation>已取消讀取</translation></message>
    <message><source>No overlay files found</source><translation>未發現覆蓋層檔案</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>大覆蓋層的讀取可能較慢，可隨時取消。</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>UWF 提供程式未能完成列舉。覆蓋層較大或系統資源不足時可能發生此問題。</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>重新讀取以獲取診斷快照。</translation></message>
    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>該診斷快照未返回檔案，不代表覆蓋層中沒有變更。</translation></message>
    <message><source>%1 items · %2</source><translation>%1 項 · %2</translation></message>
    <message><source>Selected folder</source><translation>所選資料夾</translation></message>
    <message><source>Selected file</source><translation>所選檔案</translation></message>
    <message><source>Direct shutdown?</source><translation>直接關機？</translation></message>
    <message><source>Direct restart?</source><translation>直接重新啟動？</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>將暫存檔案提交至磁碟後關機。</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>將暫存檔案提交至磁碟後重新啟動。</translation></message>
    <message><source>staged files</source><translation>個暫存檔案</translation></message>
    <message><source>View failure details</source><translation>查看失敗詳細資訊</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>僅會自動提交暫存檔案。其他未提交的變更可能在關機後遺失。</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>僅會自動提交暫存檔案。其他未提交的變更可能在重新啟動後遺失。</translation></message>
    <message><source>Files to commit</source><translation>待提交檔案</translation></message>
    <message><source>No files to commit</source><translation>沒有待提交檔案</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>提交大量暫存檔案可能需要較長時間。</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>將直接執行 UWF 安全關機。</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>將直接執行 UWF 安全重新啟動。</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 個檔案</translation></message>

    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>提交後將繼續關機。請保持程式開啟。</translation></message>
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>提交後將繼續重新啟動。請保持程式開啟。</translation></message>
    <message><source>Committing to disk</source><translation>正在提交至磁碟</translation></message>
    <message><source>Committing…</source><translation>正在提交…</translation></message>
    <message><source>File commits have started and cannot be canceled.</source><translation>檔案提交已開始，無法取消。</translation></message>
    <message><source>Confirm before continuing</source><translation>請確認是否繼續</translation></message>
    <message><source>Some files could not be committed</source><translation>部分檔案無法提交</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>請查看失敗詳細資訊，再決定是否繼續關機。</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>請查看失敗詳細資訊，再決定是否繼續重新啟動。</translation></message>
    <message><source>File commits incomplete</source><translation>檔案提交未完成</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>自動檔案提交未完成</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>若繼續關機，未提交的檔案變更可能遺失。</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>若繼續重新啟動，未提交的檔案變更可能遺失。</translation></message>
    <message><source>Shut down anyway</source><translation>仍然關機</translation></message>
    <message><source>Restart anyway</source><translation>仍然重新啟動</translation></message>
    <message><source>Counting staged files…</source><translation>正在統計暫存檔案…</translation></message>
    <message><source>Scanning staged paths</source><translation>正在掃描暫存路徑</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>選取機碼可處理其整個子樹；選取命名值則僅處理該值。</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>選取要從 UWF 寫入保護中排除的登錄機碼（包含子機碼與值）。</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>輸入 HKLM\SOFTWARE\…，按 Enter 跳轉</translation></message>
    <message><source>Navigate to registry path</source><translation>轉到登錄檔路徑</translation></message>
    <message><source>Collapse</source><translation>折疊</translation></message>
    <message><source>Expand</source><translation>展開</translation></message>
    <message><source>Copy name</source><translation>複製名稱</translation></message>
    <message><source>Unable to access this registry key</source><translation>無法存取此登錄機碼</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>請選取其他機碼，或檢查存取權限後重試。</translation></message>
    <message><source>Current selection</source><translation>目前選取</translation></message>
    <message><source>Copy selection</source><translation>複製所選內容</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>使用整個機碼，包含所有子機碼</translation></message>
    <message><source>Select this registry value</source><translation>選取此登錄值</translation></message>
    <message><source>Home</source><translation>主目錄</translation></message>
    <message><source>Back</source><translation>返回上一個資料夾</translation></message>
    <message><source>Up one folder</source><translation>上一級資料夾</translation></message>
    <message><source>Refresh folder</source><translation>重新整理資料夾</translation></message>
    <message><source>Folder address</source><translation>資料夾位址</translation></message>
    <message><source>Search this folder</source><translation>搜尋此資料夾</translation></message>
    <message><source>Quick access</source><translation>快速存取</translation></message>
    <message><source>This PC</source><translation>此電腦</translation></message>
    <message><source>Folders</source><translation>資料夾</translation></message>
    <message><source>Files and folders</source><translation>檔案與資料夾</translation></message>
    <message><source>New folder</source><translation>新建資料夾</translation></message>
    <message><source>Open folder</source><translation>開啟資料夾</translation></message>
    <message><source>Select this file</source><translation>選取此檔案</translation></message>
    <message><source>Copy path</source><translation>複製路徑</translation></message>
    <message><source>Rename…</source><translation>重新命名…</translation></message>
    <message><source>Selection</source><translation>所選項</translation></message>
    <message><source>File name</source><translation>檔案名稱</translation></message>
    <message><source>Select the current folder</source><translation>選取目前資料夾</translation></message>
    <message><source>Select files or folders</source><translation>選取檔案或資料夾</translation></message>
    <message><source>Enter a file name</source><translation>輸入檔案名稱</translation></message>
    <message><source>Select a file or enter its name</source><translation>選取檔案或輸入名稱</translation></message>
    <message><source>File type</source><translation>檔案類型</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>可同時選取檔案與資料夾</translation></message>
    <message><source>Select folders only.</source><translation>僅選取資料夾</translation></message>
    <message><source>Choose a location to save the file.</source><translation>選取檔案儲存位置</translation></message>
    <message><source>Multiple files can be selected.</source><translation>支援多選</translation></message>
    <message><source>Select a file to open.</source><translation>選取要開啟的檔案。</translation></message>
    <message><source>Save</source><translation>儲存</translation></message>
    <message><source>Add selected items</source><translation>新增所選項</translation></message>
    <message><source>Select folder</source><translation>選取資料夾</translation></message>
    <message><source>Open</source><translation>開啟</translation></message>
    <message><source>Unable to select this item</source><translation>無法選取此項目</translation></message>
    <message><source>Reading folder</source><translation>正在讀取資料夾</translation></message>
    <message><source>Unable to access this folder</source><translation>無法存取此資料夾</translation></message>
    <message><source>This folder is empty</source><translation>此資料夾為空</translation></message>
    <message><source>No matching items</source><translation>沒有符合的項目</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>正在讀取此位置的檔案與子資料夾…</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>請選取其他資料夾，或檢查存取權限後重試。</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>可以返回上一級，或選取其他位置。</translation></message>
    <message><source>Try another search term.</source><translation>嘗試其他關鍵字。</translation></message>
    <message><source> · %1 selected</source><translation> · 已選取 %1 項</translation></message>
    <message><source>Replace the existing file?</source><translation>替換現有檔案？</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>此位置已存在檔案：
%1

儲存將覆蓋原檔案。</translation></message>
    <message><source>Replace file</source><translation>替換檔案</translation></message>
    <message><source>Folder name</source><translation>資料夾名稱</translation></message>
    <message><source>Create</source><translation>建立</translation></message>
    <message><source>Rename</source><translation>重新命名</translation></message>
    <message><source>Add exclusion</source><translation>新增排除</translation></message>
    <message><source>Select this registry key</source><translation>選取此登錄機碼</translation></message>
    <message><source>No registry key selected</source><translation>尚未選取登錄機碼</translation></message>
    <message><source>Select an available subkey</source><translation>選取可用的子機碼</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>全域登錄排除項目 · 包含子機碼</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>命名值：%1 · 僅操作此值</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>整個機碼及所有子機碼 · 遞迴</translation></message>
    <message><source>More</source><translation>更多</translation></message>
    <message><source>Compatibility mode</source><translation>相容模式</translation></message>
    <message><source>Discard changes</source><translation>放棄變更</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 項待套用</translation></message>
    <message><source>Administrator</source><translation>系統管理員</translation></message>
    <message><source>Read-only</source><translation>唯讀</translation></message>
    <message><source>Restore</source><translation>還原</translation></message>
    <message><source>Move</source><translation>移動</translation></message>
    <message><source>Minimize</source><translation>最小化</translation></message>
    <message><source>Maximize</source><translation>最大化</translation></message>
    <message><source>Staged path</source><translation>暫存路徑</translation></message>
    <message><source>Required configuration</source><translation>必要設定</translation></message>
    <message><source>Pending removal</source><translation>待移除</translation></message>


    <message><source>Added</source><translation>新增</translation></message>
    <message><source>Configured</source><translation>已設定</translation></message>

    <message><source>Completed operations cannot be undone.</source><translation>已完成的操作無法復原。</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>已處理 %1 / %2 項</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>取消後，會在目前操作結束時停止後續工作。</translation></message>

    <message><source>Fit columns to contents</source><translation>依內容調整欄寬</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>將所選路徑標記為待移除，套用後生效。</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>套用後從自動暫存清單中移除此路徑。</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>套用後將此路徑新增至自動暫存清單。</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>套用此計畫將清空已儲存的暫存清單，並在不再需要時移除相關的登錄排除項目。</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>檔案暫存變更必須在 UWF Manager 中套用，無法匯出為 uwfmgr 命令。</translation></message>
    <message><source>File staging is saved after Apply</source><translation>暫存清單在套用後儲存</translation></message>
    <message><source>Overlay snapshot scope</source><translation>覆蓋層快照的範圍</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>此清單僅供診斷，不能用來判斷哪些檔案必須提交。</translation></message>
    <message><source>NTFS volumes only</source><translation>僅支援 NTFS 磁碟區</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>大型覆蓋層可能讀取緩慢或因資源不足而失敗。</translation></message>
    <message><source>Small files may be missing</source><translation>可能遺漏小型檔案</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>小於磁碟叢集大小（通常為 4 KB）的檔案可能不在清單中。</translation></message>
    <message><source>Does not indicate pending commits</source><translation>不代表待提交狀態</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>已提交的檔案、排除路徑及受其他操作影響的項目也可能出現。</translation></message>

    <message><source>Command processing result</source><translation>命令處理結果</translation></message>
    <message><source>Copy command</source><translation>複製命令</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 個分支讀取失敗</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 個分支無法讀取。可繼續處理已發現內容，或僅重試失敗項目。</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 個檔案 · %2 個資料夾</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 個值 · %2 個機碼</translation></message>
    <message><source>Attempts</source><translation>嘗試次數</translation></message>
    <message><source>Cancel remaining tasks</source><translation>取消剩餘工作</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>僅提交已成功走訪的內容；讀取失敗的分支維持原樣。</translation></message>
    <message><source>Committing changes</source><translation>正在提交變更</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>已完成的刪除及提交無法復原。</translation></message>
    <message><source>Continue…</source><translation>繼續…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>僅刪除並提交成功走訪的目標；無法讀取的分支及其上層資料夾或機碼會保留。</translation></message>
    <message><source>Deleting and committing</source><translation>正在刪除並提交</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>刪除及提交無法復原。僅處理所選範圍，不會套用待套用的 UWF 設定。</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>正在確定操作範圍。讀取失敗的分支會記錄下來，並繼續走訪其餘內容。</translation></message>
    <message><source>Export traversal records</source><translation>匯出走訪記錄</translation></message>
    <message><source>No actionable content</source><translation>沒有可操作的內容</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>所選範圍中沒有找到可操作的目標，未執行刪除或提交。</translation></message>
    <message><source>No changes have been committed.</source><translation>尚未提交任何變更</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>尚未刪除或提交任何內容</translation></message>
    <message><source>Path</source><translation>路徑</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>永久刪除所選內容，並提交刪除結果。</translation></message>
    <message><source>Preparation complete</source><translation>準備完成</translation></message>
    <message><source>Preparing commit</source><translation>正在準備提交</translation></message>
    <message><source>Preparing deletion and commit</source><translation>準備刪除並提交</translation></message>
    <message><source>Process only successfully traversed content</source><translation>僅處理已成功走訪的內容</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>略過重新剖析點，確保走訪不超出所選範圍。</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>提交後無法透過重新啟動復原。僅處理所選範圍，不會套用待套用的 UWF 設定。</translation></message>
    <message><source>Retry failed items</source><translation>重試失敗項目</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>提交所選範圍內的變更。</translation></message>
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

    <message><source>All operations succeeded</source><translation>全部操作成功</translation></message>
    <message><source>Some operations succeeded</source><translation>部分操作成功</translation></message>
    <message><source>No operations succeeded</source><translation>沒有操作成功</translation></message>
    <message><source>No targets to process</source><translation>沒有需要處理的目標</translation></message>
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
    <message><source>Export interface settings</source><translation>匯出介面設定</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>將目前的佈景主題、語言及覆蓋層小工具顯示狀態儲存至本程式旁的 UWF.ini。</translation></message>
    <message><source>Interface settings saved</source><translation>介面設定已儲存</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>檔案暫存已停止。請先解決錯誤，再重試。</translation></message>
    <message><source>No net change for this setting</source><translation>本次匯入後，此設定的最終值未變</translation></message>
    <message><source>Final draft %1 → %2</source><translation>草稿最終值：%1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>草稿最終值：%1 → %2（要求值為 %3）</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>要求值：%1 → %2；已由後續命令取代。草稿最終值 → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>已更新%1草稿</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>已更新磁碟區 %1 的保護設定草稿 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>請選取要移除的項目。</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>目前無法還原所選項目。</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>可用空間直通</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>持久覆蓋層</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>下次啟動清除持久資料</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>唯讀媒體模式</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>利用磁碟可用空間儲存寫入，減少保留覆蓋層的用量；支援 RAM（不建議）和 Disk（建議）。</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>讓 Disk 覆蓋層中的寫入在重新啟動後保留；不適用於 RAM 覆蓋層。</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>重設持久覆蓋層：下次啟動時清除其中的寫入，還原至啟用覆蓋層時的狀態</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>需要系統管理員權限；目前只能檢視覆蓋層設定。</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>暫時無法編輯設定，請等待目前操作完成。</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>無法讀取覆蓋層設定；請重新整理後再編輯。</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>目前系統不支援在下次啟動時清除持久資料。</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>無法識別目前的清除要求；請重新整理設定後再編輯。</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>變更此要求需要目前工作階段已啟用 UWF 與 Disk 持續覆寫層。</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>下次啟動清除持久資料需要保留 Disk 持久覆蓋層，請取消清除或保留持久覆蓋層。</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>持久覆蓋層僅支援 Disk，請關閉持久覆蓋層或選取 Disk。</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>唯讀媒體模式僅支援 RAM，請關閉唯讀媒體模式或選取 RAM。</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>目前工作階段的 UWF 已啟用。請停用 UWF 並重新啟動後，再變更此設定。</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>設定已更新，請關閉並重新開啟此對話方塊後再編輯。</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>覆蓋層進階設定</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>目前覆蓋層</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>下次覆蓋層</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>草稿中的覆蓋層設定有衝突或無法使用，請返回編輯並調整覆蓋層設定。</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>已排定下次啟動時清除。停用持久覆蓋層前，請先取消清除要求。</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>下次工作階段未保留 Disk 持久覆蓋層。要求清除前，請先設為保留。</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>持久覆蓋層僅適用於 Disk；下次工作階段的覆蓋層為 RAM。</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>啟用唯讀媒體模式需要 RAM；下次工作階段的覆蓋層為 Disk。</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>無法確認目前持久覆蓋層的狀態。請重新讀取設定。</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>套用後將於下次啟動時生效。</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>已排定清除</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>未排定清除</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>僅在下次啟動清除一次，之後繼續保留資料。目前工作階段不會立即清除。</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>取消已排定的下次啟動清除要求，保留持久覆蓋層資料。</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>重設持久覆蓋層 · 單次要求</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>正在處理…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>請等待操作完成。</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · 下次工作階段</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>返回設定</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>變更條件</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>已變更</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>檢查設定衝突</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>設定有衝突</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>停用條件</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>目前工作階段須已啟用 UWF 與 Disk 持續覆寫層；清除資料還需要下次工作階段保留 Disk 持續覆寫層。</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>啟用條件</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>功能</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>功能說明</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>瞭解%1</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>請先處理以下設定衝突，再保留草稿。</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>還原</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>生效時間</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>目前無法變更此設定</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>目前工作階段的 UWF 必須已停用，且下次工作階段的覆蓋層須使用 Disk。</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>目前工作階段的 UWF 必須已停用，且下次工作階段的覆蓋層須使用 RAM。</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>目前工作階段的 UWF 必須已停用；若已排定清除持久資料，請先取消清除要求，再停用持久覆蓋層。</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>目前工作階段的 UWF 必須已停用；若僅將下次工作階段設為停用，仍須重新啟動後才能變更。</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>使用條件</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>查看詳細資料</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>僅供檢視</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>查看無法變更%1的原因</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>目前工作階段的 UWF 已啟用，無法變更這些覆疊設定。請還原變更，或停用 UWF 並重新啟動。</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>查看變更條件</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>未寫入任何設定變更，可以重試。</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>無需變更設定。</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>請透過「網域機密金鑰持久化」設定管理網域電腦帳戶機密金鑰（$MACHINE.ACC）。</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>此機碼本身無法加入排除，請選擇其子機碼。</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>登錄排除項目必須指定 HKEY_LOCAL_MACHINE（HKLM）下的非空子機碼。</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>此指令碼僅包含可由 uwfmgr 命令表達的設定，不包含：</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>磁碟區繫結方式</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>無法透過磁碟機代號可靠定位的設定</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>命令指令碼</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>設定與下次工作階段目標一致。可在「命令指令碼」中檢視或匯出支援的設定。</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>路徑或名稱含控制字元的設定</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>相依於已省略設定的命令</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>UWF 排除設定使此目標無法可靠提交。請移除衝突的排除項目並重新啟動後再提交。</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>UWF 排除設定使檔案暫存清單無法可靠提交。請移除衝突的排除項目並重新啟動後再套用此變更。</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF 無法保證操作指向所選登錄機碼。</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>移除登錄排除項目前，確認已儲存的檔案暫存清單為空。</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>待確認</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>空</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>系統磁碟區目前未受 UWF 保護，沒有可提交的登錄變更。</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>登錄變更已直接持久化，無需 UWF 提交。</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>登錄變更已直接持久化，UWF 無法執行此刪除。</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>UWF 排除路徑必須使用反斜線。</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>檔案路徑不能包含空字元。</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>請先在草稿中啟用磁碟區保護，再新增檔案排除項目。</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>排除路徑包含 UWF 不支援的字元。</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>排除路徑超過 UWF 的長度限制。</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>HORM 生效時，無法提交檔案和登錄變更。</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>HORM 生效時，無法寫入登錄。</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>唯讀媒體模式生效時，無法永久儲存登錄寫入。</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>唯讀媒體模式生效時，無法提交檔案和登錄變更。</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>攔截實體媒體寫入，包括中繼資料。排除項不會繞過保護。無法逐項提交檔案和登錄變更；Full Commit 是獨立的全域操作。</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>HORM 生效時，無法變更 UWF 設定。</translation></message>
    <message><source>Default value · this value only</source><translation>預設值 · 僅此值</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>檔案變更已持久化，無需執行 UWF 提交。</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>上次成功觀測覆蓋層用量：%1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>觀測到的覆蓋層儲存設定超出可寫入的大小或類型範圍</translation></message>
    <message><source>Overlay (last observed)</source><translation>覆蓋層（上次觀測）</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>覆蓋層用量重新整理失敗。
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>覆蓋層用量重新整理失敗，顯示 %1 的上次成功觀測。
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>無法將此路徑解析至本機磁碟區。</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF：已啟用（上次觀測）</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>檔案變更已持久化，UWF 無法執行此刪除操作。</translation></message>

    <!-- Overlay maintenance -->
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>%1 GB 可用</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>需要管理員權限。</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>正在處理，請稍候。</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>會嘗試將本次啟動以來，所有受保護卷上的新增、修改和刪除保存到磁碟。范圍包括檔案、系統設置等全部更改，不能只選其中一部分。</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>當前模式下無法自動保存</translation></message>
    <message><source>Cannot be undone once started</source><translation>開始后不能撤銷</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>為磁碟覆蓋層選擇存儲位置</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>選擇磁碟覆蓋層的存放位置。</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>暫時無法修改配置。</translation></message>
    <message><source>Configured, inactive this session</source><translation>已配置 · 本次不生效</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>已配置，重新啟動后生效</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>Swap 存放位置已配置，重新啟動后生效。當前啟動的覆蓋層不會遷移到新位置。</translation></message>
    <message><source>Configure…</source><translation>配置…</translation></message>
    <message><source>Current UWF</source><translation>當前 UWF</translation></message>
    <message><source>Currently configured storage volume</source><translation>系統已設定的存放位置</translation></message>
    <message><source>Default behavior</source><translation>預設行為</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>請先停用 HORM 并重新啟動電腦，再進行完整卷提交。</translation></message>
    <message><source>Disabled · restart required</source><translation>已停用 · 請重新啟動</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>停用 HORM 不會自動關閉 UWF 或唯讀媒體模式，也不會自動保存受保護磁碟上的改動。請重新啟動后再查看可用操作。</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>請先啟用 UWF 并重新啟動電腦，再啟用 HORM。</translation></message>

    <message><source>Full volume commit</source><translation>完整卷提交</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>完整磁碟區提交模式（已啟用，持續至重新啟動）</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>完整磁碟區提交模式已啟用。再次使用前請重新啟動。</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>本次已開始完整卷提交，請重新啟動后再啟用 HORM。</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>僅在當前已啟用唯讀媒體模式時使用。</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>完整磁碟區提交模式會持續啟用至重新啟動。新變更會直接儲存，逐項提交仍無法使用。</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>當前系統不支持完整卷提交。</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM 已啟用。UWF 設置和逐項提交暫不可用；需要修改時，請先停用 HORM 并重新啟動電腦。</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM 已停用。請重新啟動電腦后，再調整 UWF 設置。</translation></message>
    <message><source>HORM is not available on this system.</source><translation>當前系統不支持 HORM。</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM 需要當前使用 RAM 唯讀媒體模式。</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM 需要當前使用 RAM 覆蓋層。</translation></message>
    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>在磁碟模式下，系統實現可能會預設打開本開關；請以界面顯示的當前狀態為準。</translation></message>
    <message><source>Inactive this session</source><translation>本次不生效</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>重新啟動前，后續更改也會保存</translation></message>

    <message><source>Maintenance and recovery</source><translation>維護與恢復</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>請先確認當前狀態就是你希望長期保留的狀態。</translation></message>

    <message><source>Next startup: %1</source><translation>下次啟動：%1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>沒有可用的存放卷，請重新讀取磁碟列表。</translation></message>
    <message><source>Not enabled</source><translation>未啟用</translation></message>
    <message><source>Not selected</source><translation>未選擇</translation></message>
    <message><source>Not supported</source><translation>不支持</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>開始后，新產生的更改會直接寫入磁碟，直到你重新啟動電腦。</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>存放位置僅支援單一磁碟區設定。變更後將取代現有位置，並將下次覆蓋層類型設為磁碟。</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>覆蓋層容量（沿用當前設置）</translation></message>
    <message><source>Overlay configuration</source><translation>覆蓋層配置</translation></message>

    <message><source>Processing…</source><translation>正在處理…</translation></message>
    <message><source>Read state again</source><translation>重新讀取狀態</translation></message>
    <message><source>Read the current UWF state first.</source><translation>請先讀取當前 UWF 狀態。</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>唯讀媒體模式不支持逐項提交，檔案暫存也無法自動保存。</translation></message>
    <message><source>Reading state…</source><translation>正在讀取狀態…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>請重新啟動電腦并重新讀取狀態后，再啟用 HORM。</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>每次啟動，恢復同一份桌面和程式狀態。</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>查看可用的提交操作及使用條件。</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>保存所有受保護卷上的更改</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>保存所有受保護卷的更改，請先查看影響說明。</translation></message>


    <message><source>State unknown</source><translation>狀態未知</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>成功保存的更改無法再通過重新啟動還原。</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>成功保存的更改會保留</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>無法確認 HORM 狀態，請先重新讀取。</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>無法確認 HORM 狀態，請先重新讀取。</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>無法確認提交或 HORM 狀態，請先重新讀取。</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>無法讀取當前 UWF 狀態，請重新讀取后再繼續。</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>當前使用 RAM 唯讀媒體模式</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>無法讀取當前保護模式，請重新讀取狀態。</translation></message>
    <message><source>The result could not be confirmed</source><translation>未能確認操作結果</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>此排除項已配置，但在本次啟動期間不會繞過保護。</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>此操作可能同時變更可用空間直通和持續覆蓋層設定。</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>當前已開啟 UWF 保護</translation></message>

    <message><source>%1 (will be replaced)</source><translation>%1（將替換）</translation></message>

    <!-- Unified maintenance drafts -->
    <message><source>Add full volume commit to the draft?</source><translation>將完整卷提交加入草稿？</translation></message>
    <message><source>Add to draft</source><translation>加入草稿</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>影響所有受保護卷，僅在應用此草稿時開始。</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>應用此草稿會開始完整卷提交。成功保存的更改無法通過重新啟動還原，重新啟動前的新更改也會直接保存。</translation></message>
    <message><source>Configure storage location</source><translation>配置存放位置</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>當前系統不支持配置存放位置。</translation></message>
    <message><source>Current: %1</source><translation>當前：%1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>配置存放位置前，請先停用 HORM 并重新啟動。</translation></message>
    <message><source>Disable saved-state startup?</source><translation>停用休眠一次/恢復許多？</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>草稿：%1 · 待應用</translation></message>
    <message><source>Enable saved-state startup?</source><translation>啟用休眠一次/恢復許多？</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>完整卷提交（待應用）</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>完整卷提交已加入草稿，僅在“預覽并應用”中確認后開始。</translation></message>
    <message><source>Hibernate once / resume many</source><translation>休眠一次/恢復許多</translation></message>
    <message><source>Not started</source><translation>尚未發起</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>從待應用更改中移除完整卷提交。</translation></message>
    <message><source>Save draft</source><translation>保存草稿</translation></message>

    <message><source>Will start when applied</source><translation>應用時開始</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>已掛載的固定磁碟未通過保護檢查，請先查看列出的磁碟再啟用 HORM。</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>檢查仍掛載且 UWF 可保護的固定磁碟；已卸載的分割區允許略過。</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>無法確認 HORM 啟用條件，請重新讀取狀態後再繼續。</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>本機磁碟保護檢查</translation></message>
    <message><source>Not using a Disk overlay</source><translation>未使用磁碟覆蓋層</translation></message>
    <message><source>Selected storage volume</source><translation>本次選擇的存放磁碟區</translation></message>
    <message><source>System drive default (%1)</source><translation>系統磁碟預設位置（%1）</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>本次啟動期間保護狀態已變更，請重新啟動後再啟用 HORM。</translation></message>
    <message><source>Unable to read</source><translation>無法讀取</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>選擇用於存放暫時變更的磁碟。設定在下次啟動時生效。</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>設定存放位置前，須停用 UWF 並重新啟動。</translation></message>
    <message><source>Met</source><translation>已符合</translation></message>
    <message><source>Not met</source><translation>尚未符合</translation></message>
    <message><source>Partition</source><translation>分割區</translation></message>


    <message><source>Protect all</source><translation>保護全部</translation></message>
    <message><source>Applied</source><translation>已套用</translation></message>
    <message><source>Not applied</source><translation>未套用</translation></message>
    <message><source>Could not confirm</source><translation>無法確認</translation></message>
    <message><source>Hibernation is not supported</source><translation>不支援休眠</translation></message>
    <message><source>Hibernation is not enabled</source><translation>休眠未啟用</translation></message>
    <message><source>Fast startup only</source><translation>僅支援快速啟動</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>此系統不支援批次磁碟區保護。</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>變更磁碟區保護前，須停用 HORM 並重新啟動。</translation></message>
    <message><source>Setup and operation</source><translation>設定與操作流程</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. 設定下次啟動</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>目前 UWF 已停用時，設定下次啟動使用 RAM 覆蓋層、唯讀媒體模式和磁碟區保護，並啟用 UWF，統一套用上述設定。同時啟用系統 S4 休眠並設定完整休眠檔案。</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>重新啟動後，確認目前工作階段已啟用 UWF、使用 RAM 覆蓋層和唯讀媒體模式，且 UWF 可保護、已掛接的固定磁碟區均已受保護。確認系統休眠可用。</translation></message>
    <message><source>2. Restart and verify</source><translation>2. 重新啟動並驗證</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. 啟用 HORM 並儲存休眠狀態</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>將「啟用 HORM」加入草稿並統一套用。此操作等同於：</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>啟用成功後，設定桌面與應用程式的執行狀態，執行一次休眠以儲存基準狀態：</translation></message>
    <message><source>HORM usage</source><translation>HORM 使用說明</translation></message>
    <message><source>Volume protection</source><translation>磁碟區保護</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>統一設定所有磁碟區的下次保護狀態。</translation></message>
    <message><source>Unprotect all</source><translation>取消全部保護</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>下次啟動：全部受保護</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>下次啟動：全部未保護</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>下次啟動：依磁碟區設定</translation></message>
    <message><source>System hibernation is available</source><translation>系統休眠可用</translation></message>
    <message><source>Usage guide</source><translation>使用說明</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>全部保護待套用</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>磁碟區保護在重新啟動後生效</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>取消全部保護與啟用 HORM 無法同時套用。</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>不支援批次磁碟區保護。</translation></message>



    <message><source>Storage volume:</source><translation>存放卷：</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>排除項目在本次啟動期間不會略過保護，逐項提交與檔案暫存自動儲存無法使用。完整磁碟區提交會影響所有受保護磁碟區，請先查看說明。</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>重複恢復同一休眠狀態</translation></message>
    <message><source>Remove full volume commit draft</source><translation>移除完整磁碟區提交草稿</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>唯讀媒體模式已啟用</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>無法讀取已設定的儲存標記。</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>檔案暫存與檔案排除項目衝突：
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>此電腦無法使用完整休眠</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>多個儲存磁碟區的標記為非零值。</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>無法確定覆蓋層是否為磁碟類型。</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>無法讀取已設定的儲存位置。</translation></message>
    <message><source>The system drive could not be identified.</source><translation>無法識別系統磁碟機。</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>直接重新啟動會略過暫存檔案的提交。</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>直接關機會略過暫存檔案的提交。</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>將不提交暫存檔案。未提交的變更可能在重新啟動後遺失。</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>將不提交暫存檔案。未提交的變更可能在關機後遺失。</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>在正常關機或重新啟動前提交暫存檔案。</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>完整磁碟區提交與啟用 HORM 無法同時套用。</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>設定儲存位置與啟用 HORM 無法同時套用。</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>設定儲存位置與完整磁碟區提交對目前 UWF 模式的要求不同。</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>套用前請選擇儲存磁碟區。</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>設定磁碟儲存位置與要求的 RAM 覆蓋層衝突。</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>要求完整磁碟區提交前，請先套用篩選器變更並重新啟動。</translation></message>
    <message><source>Canceled</source><translation>已取消</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>已檢閱的儲存磁碟區已無法使用。請返回編輯並選擇其他磁碟區。</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>此磁碟區的儲存匯流排不受支援，無法向 UWF 註冊。</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>無法確認實體磁碟區是否受支援。請重新整理後再啟用保護。</translation></message>
    <message><source>Paused</source><translation>暫停</translation></message>
    <message><source>Partially paused</source><translation>部分暫停</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>目前使用持久化覆蓋層，下次啟動保留資料，已暫停自動提交。</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>暫停的項目：%1 · 暫停的檔案：%2</translation></message>
    <message><source>Read-only ROM</source><translation>唯讀 ROM</translation></message>

    <message><source>%1 changes · %2 commands</source><translation>%1 項變更 · %2 條命令</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>%1 檔案系統不支援檔案排除與單一檔案提交。</translation></message>
    <message><source>Applying changes</source><translation>正在套用變更</translation></message>
    <message><source>Automatic commits paused</source><translation>自動提交已暫停</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>HORM 模式已變更，重新啟動前暫停自動提交。</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>整卷提交後不再執行單一檔案自動提交。</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>HORM 生效期間不支援自動提交。</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>本次重新啟動暫停自動提交暫存檔案。</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>本次關機暫停自動提交暫存檔案。</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>無法確認設定能否持續儲存。請重新整理系統狀態後再修改。</translation></message>
    <message><source>Elapsed: %1 s</source><translation>已耗時 %1 秒</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>僅提交符合條件的暫存檔案。</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>唯讀 ROM 無法在受保護的系統磁碟區上持續儲存一般 UWF 設定。請停用 UWF 並重新啟動後修改。</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>本次操作不提交已略過的暫存檔案。</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>暫存檔案位於未受保護的儲存空間，無需自動提交。</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>閾值修改需要提交登錄，唯讀 ROM 下無法使用。</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>正在等候 Windows 完成操作，請稍候。</translation></message>

    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM 已停用，重新啟動前暫停自動提交暫存檔案。手動操作依目前 UWF 能力開放。</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>此設定快照包含要求目前工作階段停用 UWF 的設定。請先停用 UWF 並重新啟動，再執行此快照。</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>目前工作階段操作（HORM、FullCommit 和持久化覆蓋層重設；請從「待套用變更」匯出）</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>批次取消保護需要確認完整的下次工作階段磁碟區範圍。</translation></message>
</context>
</TS>
