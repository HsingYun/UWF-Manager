<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ja" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>有効にする</translation></message>
    <message><source>Disable</source><translation>無効にする</translation></message>
    <message><source>enable</source><translation>有効にする</translation></message>
    <message><source>disable</source><translation>無効にする</translation></message>
    <message><source>Enabled</source><translation>有効</translation></message>
    <message><source>Disabled</source><translation>無効</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>キャンセル</translation></message>
    <message><source>Close</source><translation>閉じる</translation></message>
    <message><source>Refresh</source><translation>更新</translation></message>
    <message><source>Apply</source><translation>適用</translation></message>
    <message><source>Add</source><translation>追加</translation></message>
    <message><source>Remove</source><translation>削除</translation></message>
    <message><source>Yes</source><translation>はい</translation></message>
    <message><source>No</source><translation>いいえ</translation></message>
    <message><source>Excluded</source><translation>除外済み</translation></message>
    <message><source>Not excluded</source><translation>除外なし</translation></message>
    <message><source>File…</source><translation>ファイル…</translation></message>
    <message><source>Folder…</source><translation>フォルダー…</translation></message>
    <message><source>volume ID</source><translation>ボリューム ID</translation></message>
    <message><source>drive letter</source><translation>ドライブ文字</translation></message>
    <message><source>Drive letter</source><translation>ドライブ文字</translation></message>
    <message><source>Volume ID</source><translation>ボリューム ID</translation></message>
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>現在のシステム「%1」（%2）は、対応する Windows 10/11 エディションとして認識されません。互換モードで動作するため、一部の機能を利用できない場合があります。</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>現在のセッションでの保護状態（読み取り専用）。</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>次のセッションでこのボリュームを保護します。書き込みはオーバーレイに転送され、再起動時に破棄されます。</translation></message>
    <message><source>Protection:</source><translation>保護：</translation></message>
    <message><source>Current session</source><translation>現在のセッション</translation></message>
    <message><source>Next session</source><translation>次のセッション</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>現在有効なセッション（読み取り専用）。設定変更はこのセッションには反映されません。</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>再起動後に有効になるセッションです。設定変更はシステムの再起動後に反映されます。</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>ボリュームの識別方法です。ドライブ文字は簡単ですが、ディスクの増設・取り外しなどで文字が変わると関連付けが無効になります。ボリューム ID は文字が変わっても維持されます。</translation></message>
    <message><source>Bind by:</source><translation>識別方法：</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF 非対応のボリューム： </translation></message>
    <message><source>Global settings</source><translation>全体設定</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>現在のセッションの UWF フィルター状態（読み取り専用）。</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>次のセッションで UWF を有効にします。保護されたボリュームへの書き込みはオーバーレイに転送され、再起動時に破棄されます。</translation></message>
    <message><source>Filter</source><translation>フィルター</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>種類と最大サイズを変更するには：
1. 上のスイッチでフィルターを無効にします。
2. 再起動し、無効な状態にします。
3. 設定を変更します。</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>オーバーレイの保存先。RAM は高速ですがメモリを使います。Disk はシステムドライブを使い、容量を大きくできます。どちらも再起動時に破棄されます。</translation></message>
    <message><source>Type</source><translation>種類</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>オーバーレイの最大容量。RAM はシステムの総メモリ量が上限です。Disk は 1024 MB 以上で、システムボリュームに十分な空き容量が必要です。</translation></message>
    <message><source>Maximum size</source><translation>最大サイズ</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>最大サイズ · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>警告しきい値</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>使用量がこの値に達すると、イベントログに警告を記録します。重大しきい値より小さくしてください。0 で通知を無効にします。</translation></message>
    <message><source>Critical threshold</source><translation>重大しきい値</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>使用量がこの値に達すると、イベントログに重大イベントを記録します。警告しきい値より大きくしてください。0 で通知を無効にします。</translation></message>
    <message><source>Used / total</source><translation>使用量 / 合計</translation></message>
    <message><source>Overlay</source><translation>オーバーレイ</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; 使用中 &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; 警告 &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; 重大</translation></message>
    <message><source>UWF status unavailable: </source><translation>UWF の状態を取得できません： </translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>UWF の設定変更には管理者権限が必要です。右クリック →「管理者として実行」で起動し直してください。</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>非対応のドライブ種類です（ローカル固定ディスクのみ対応）。</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>容量が UWF の保護ボリュームあたりの上限 16 TiB を超えています。</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1：ボリューム保護は可能ですが、ファイル除外とファイル単位のコミットには非対応です。</translation></message>
    <message><source>Failed to read volume information.</source><translation>失敗: ボリューム情報の読み取り.</translation></message>
    <message><source>%1 free / %2</source><translation>空き %1 / %2</translation></message>
    <message><source>Commit</source><translation>コミット</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>ファイルの変更をオーバーレイからディスクにコミット / レジストリの変更をコミット. この操作は元に戻せません。</translation></message>
    <message><source>Commit file changes…</source><translation>ファイルの変更をコミット…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>ディスクにコミットするファイルを選択. ファイルの変更をオーバーレイからディスクにコミット.</translation></message>
    <message><source>Commit folder changes…</source><translation>フォルダーの変更をコミット…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>ディスクにコミットするフォルダーを選択. フォルダーの変更をオーバーレイからディスクにコミット. 全サブフォルダーを含みます。</translation></message>
    <message><source>Delete and commit file…</source><translation>ファイルを削除し、削除をディスクにコミット…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>削除をコミットするファイルを選択. ファイルを削除し、削除をディスクにコミット.</translation></message>
    <message><source>Delete and commit folder…</source><translation>フォルダーと全内容を削除し、削除をディスクにコミット…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>削除をコミットするフォルダーを選択. フォルダーと全内容を削除し、削除をディスクにコミット. 全サブフォルダーを含みます。</translation></message>
    <message><source>Commit registry changes…</source><translation>レジストリの変更をコミット…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>キーのパスと値名（任意）を入力し、変更をコミットします。値名を空にすると、キーと全サブキーのすべての値をコミットします。</translation></message>
    <message><source>Delete and commit registry…</source><translation>レジストリの削除をコミット…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>削除するキーのパスと値名（任意）を入力し、削除をコミットします。値名を空にすると、キー・全値・全サブキーを再帰的に削除します。</translation></message>
    <message><source>File exclusions</source><translation>ファイルの除外</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>UWF の保護から除外するファイルとフォルダーです。項目をダブルクリックするとパスをコピーします。</translation></message>
    <message><source>Registry exclusions</source><translation>レジストリの除外</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>レジストリ除外はボリューム保護とは独立した全体設定で、UWF フィルタースイッチに従います。一覧は全ボリューム共通で、タブには一度だけ表示されます。ダブルクリックでパスをコピーします。</translation></message>
    <message><source>File staging</source><translation>ファイルの自動コミット</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>ファイル / フォルダー: 安全なシャットダウンまたは再起動の前に、自動でコミットします。 全サブフォルダーを含みます。 一覧に項目がある間は、次のレジストリ除外により、保護されたセッション間でも一覧を保持します： HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Search staged paths…</source><translation>自動コミット対象を検索…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>ファイルまたはフォルダーを自動コミット一覧に追加します。</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>ディスクにコミットするファイルを選択. 安全なシャットダウンまたは再起動の前に、自動でコミットします。</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>ディスクにコミットするフォルダーを選択. 安全なシャットダウンまたは再起動の前に、自動でコミットします。 全サブフォルダーを含みます。</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>選択したパスを自動コミット一覧から直ちに削除します。</translation></message>
    <message><source>Select files for automatic commit</source><translation>ディスクにコミットするファイルを選択 · ファイルの自動コミット</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>ディスクにコミットするフォルダーを選択 · ファイルの自動コミット · 全サブフォルダーを含みます。</translation></message>
    <message><source>Cannot stage this path</source><translation>このパスを自動コミット対象に追加できません</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>UWF のファイル除外と重複しています。次の除外を削除してから追加してください：
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>自動コミット対象と重複しています。次の対象を削除してから除外を追加してください：
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>既存の自動コミット対象とパスが重複しています</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>自動コミット一覧に保存済み項目がある間、このレジストリ除外は必要です</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>パスが存在しないか、想定したファイル種類ではありません：
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>パスを検査できません：
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>再解析ポイントは自動コミット対象に追加できません：
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>ドライブ文字がないため、UWF でコミットできません：
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>ボリューム %1 のパスです。そのボリュームの自動コミットタブから追加してください。</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>ボリュームのルートは追加できません。ボリューム全体が再帰的にコミットされてしまいます。</translation></message>
    <message><source>File staging could not be saved</source><translation>失敗: ファイル自動コミット一覧の保存</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>更新を完了できなかったため、自動コミット一覧を再読み込みしました：
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>ファイルの自動コミット: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>安全なシャットダウンまたは再起動の前に、自動でコミットします。</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>ファイルの自動コミット: %1 · ファイル: %2 · フォルダー: %3</translation></message>
    <message><source>Required by UWF for File staging.</source><translation>UWF のファイル自動コミットに必要です。</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>UWF フィルターが無効のため、オーバーレイに変更は蓄積されておらず、コミットする内容はありません。</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>%1 ではファイル単位のコミットに対応していません。</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>このボリュームは UWF 非対応です。</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>このボリュームでファイルのコミットは利用できません。 レジストリのコミットのみ可能です（レジストリ除外は全体設定）。</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>%1 ではファイル単位のコミットに対応していません。 レジストリのコミットのみ可能です（レジストリ除外は全体設定）。</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>このボリュームは現在保護されていないため、コミットするファイル変更はありません。</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>このボリュームは現在保護されていないため、コミットするファイル変更はありません。 レジストリのコミットのみ可能です（レジストリ除外は全体設定）。</translation></message>
    <message><source>Select a file to commit to disk</source><translation>ディスクにコミットするファイルを選択</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>ディスクにコミットするフォルダーを選択</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>削除をコミットするファイルを選択</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>削除をコミットするフォルダーを選択</translation></message>
    <message><source>Commit registry changes</source><translation>レジストリの変更をコミット</translation></message>
    <message><source>Commit registry deletion</source><translation>レジストリの削除をコミット</translation></message>
    <message><source>Key path:</source><translation>キー · パス:</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>HKLM\Software\... を入力または貼り付け、Enter で移動</translation></message>
    <message><source>Name</source><translation>名前</translation></message>
    <message><source>Data</source><translation>データ</translation></message>
    <message><source>Key: %1</source><translation>キー: %1</translation></message>
    <message><source>Value: %1</source><translation>値: %1</translation></message>
    <message><source>(none)</source><translation>（なし）</translation></message>
    <message><source>(Default)</source><translation>（既定）</translation></message>
    <message><source>(value not set)</source><translation>（値の設定なし）</translation></message>
    <message><source>(empty)</source><translation>（空）</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>不明 (値: ファイルが大きすぎます)</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>（キー全体と全サブキー）</translation></message>
    <message><source>Overlay files - %1</source><translation>オーバーレイ · ファイル - %1</translation></message>
    <message><source>View overlay files</source><translation>オーバーレイのファイルを表示</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>オーバーレイのファイルを表示. &lt;b&gt;診断用のスナップショットです。&lt;/b&gt;コミット対象の判断には使用しないでください。</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>オーバーレイ一覧は NTFS ボリュームのみ対応しています。</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF が無効のため、検査できるオーバーレイはありません。</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>このボリュームは UWF で保護されていないため、検査できるオーバーレイはありません。</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;診断用のスナップショットです。&lt;/b&gt;コミット対象の判断には使用しないでください。</translation></message>
    <message><source>NTFS volumes only.</source><translation>オーバーレイ一覧は NTFS ボリュームのみ対応しています。</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>オーバーレイが大きいほどメモリ使用量と所要時間が増え、遅延や失敗が生じる場合があります。</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>一覧は正確ではありません。クラスターサイズ（通常 4 KB）未満のファイルが欠ける一方、以前コミットしたファイル、除外パス内のファイル、無関係な操作の影響を受けたファイルが表示される場合があります。</translation></message>
    <message><source>Loading…</source><translation>読み込み中…</translation></message>
    <message><source>Failed: %1</source><translation>失敗: %1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>オーバーレイ · ファイル: %1</translation></message>
    <message><source>No files</source><translation>ファイルなし</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>ページ %1 / %2 · ファイル: %3</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>列挙中に WMI プロバイダーがクラッシュしました。オーバーレイが大きい場合や I/O 負荷が高い場合に発生する UWF_Overlay.GetOverlayFiles の既知の不具合です。</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>メモリ不足、またはプロバイダーが操作に対応していません。NTFS と十分な空きメモリが必要です。オーバーレイを小さくするか、メモリを多く使うアプリを閉じて再試行してください。</translation></message>
    <message><source>Raw error: %1</source><translation>エラーコード: %1</translation></message>
    <message><source>Export to file…</source><translation>ファイルにエクスポート…</translation></message>
    <message><source>Export overlay file list</source><translation>ファイルにエクスポート… · オーバーレイ · ファイル</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>テキストファイル (*.txt);;すべてのファイル (*)</translation></message>
    <message><source>Export failed</source><translation>エクスポート失敗</translation></message>
    <message><source>Export finished</source><translation>エクスポート完了</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>失敗: ファイルを開く (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>失敗: ファイルの書き込み (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>設定: ファイルの書き込み · 項目: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>ボリューム全体（%1）は除外できません。</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>ページファイル・スワップファイル・休止状態ファイルは、UWF 自体が依存するため除外できません。</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>ディレクトリ全体は除外できません： \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>ディレクトリ全体は除外できません： \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>ディレクトリ全体は除外できません： \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>この重要なシステムファイルは除外できません：%1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>ユーザー別のレジストリファイル NTUSER.DAT は除外できません。</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>レジストリキーを入力してください。</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>レジストリキーの先頭にバックスラッシュは使えません。</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>連続したバックスラッシュを含むパスは無効です。</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>レジストリパスの区切りは `\` です。`/` は使用しないでください。</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>見えない制御文字を含むパスは無効です。</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>ドメインのコンピューターアカウントの秘密情報は、UWF の仕様で除外が禁止されています：HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>次の最上位キー内のサブキーのみ除外できます。具体的なサブキーを選択してください：
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Search file paths…</source><translation>ファイルパスを検索…</translation></message>
    <message><source>Search registry keys…</source><translation>レジストリキーを検索…</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>ファイルやフォルダーを除外一覧に追加します。除外した対象は UWF で保護されません。</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>追加するファイルを選択 · ファイルの除外.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>追加するフォルダーを選択 · ファイルの除外. 全サブフォルダーを含みます。</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>除外するレジストリキーのパスを入力してください。</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>除外するレジストリキーのパスを入力してください。 / 永続化.</translation></message>
    <message><source>Registry key…</source><translation>レジストリ · キー…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>ドメインの秘密キー (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>ターミナルサービスのクライアントアクセスライセンス (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>ドメインの秘密キー（コンピューターアカウントのパスワード）を UWF のセッション間で保持します。</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>ターミナルサービスのクライアントアクセスライセンスを UWF のセッション間で保持します。</translation></message>
    <message><source>· %1 persistence %2</source><translation>· 永続化 %1: %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ 永続化 %1: %2</translation></message>
    <message><source>Remove selected</source><translation>選択項目を削除</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>選択した除外項目を削除します。「適用」後に反映されます。</translation></message>
    <message><source>Open containing folder</source><translation>保存先フォルダーを開く</translation></message>
    <message><source>Copy file path</source><translation>ファイルパスをコピー</translation></message>
    <message><source>Copy registry path</source><translation>レジストリパスをコピー</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>フォルダーの変更をオーバーレイからディスクにコミット…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>ファイルの変更をオーバーレイからディスクにコミット…</translation></message>
    <message><source>Copied to clipboard: </source><translation>クリップボードにコピーしました： </translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>追加するファイルを選択 · ファイルの除外. ファイルを複数選択できます。</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>追加するフォルダーを選択 · ファイルの除外</translation></message>
    <message><source>Add registry exclusion</source><translation>レジストリ除外の追加</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>パス %1 はボリューム %2 上にないため、このボリュームの除外項目に追加できません。</translation></message>
    <message><source>Path is not on this volume</source><translation>このボリューム上のパスではありません</translation></message>
    <message><source>Cannot add this exclusion</source><translation>この除外項目を追加できません</translation></message>
    <message><source>File: %1</source><translation>ファイル… %1</translation></message>
    <message><source>Folder: %1</source><translation>フォルダー… %1</translation></message>
    <message><source>Registry: %1</source><translation>レジストリ: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>既定で除外されています。いつでも変更できます。</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>現在のセッション: %1
次のセッション: %2</translation></message>
    <message><source>Pending change: %1</source><translation>保留中の変更: %1</translation></message>
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>項目: %1 · 追加予定: %2 · 次のセッションで削除: %3 · 保留中の変更: %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>統合書き込みフィルター (UWF) マネージャー</translation></message>
    <message><source>Main toolbar</source><translation>メインツールバー</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>UWF の現在のセッション状態と次回の設定を読み直します。</translation></message>
    <message><source>Review and apply</source><translation>確認して適用</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>保留中の変更をすべて確認し、まとめて適用します。 多くの変更は次回の再起動後に反映されます。</translation></message>
    <message><source>Safe shutdown</source><translation>安全なシャットダウン</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>UWF を使用して安全にシャットダウンします。 UWF のオーバーレイが満杯でも実行できます。</translation></message>
    <message><source>Safe restart</source><translation>安全な再起動</translation></message>
    <message><source>Direct restart</source><translation>直接再起動</translation></message>
    <message><source>Direct shutdown</source><translation>直接シャットダウン</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>UWF を使用して安全に再起動します。 UWF のオーバーレイが満杯でも実行できます。</translation></message>
    <message><source>Log</source><translation>ログ</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>このセッションで蓄積された内部ログを表示し、問題の診断に利用します。</translation></message>
    <message><source>About</source><translation>このソフトについて</translation></message>
    <message><source>About this program.</source><translation>このソフトについて UWF Manager.</translation></message>
    <message><source>Switch display language</source><translation>表示言語を切り替えます</translation></message>
    <message><source>Language</source><translation>言語</translation></message>
    <message><source>Theme</source><translation>テーマ</translation></message>
    <message><source>Toggle light / dark theme</source><translation>ライト／ダークテーマを切り替えます</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;UWF のフィルター状態、オーバーレイ、ファイルとレジストリの除外項目を管理する GUI です。 多くの変更は次回の再起動後に反映されます。&lt;/p&gt;&lt;p&gt;ソースコード: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;ライセンス: &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; ライセンス全文は、プログラムに同梱の LICENSE ファイルに記載されています。&lt;/p&gt;&lt;p&gt;本プログラムは自由ソフトウェアです。GPL v3 の条件に従って再配布・変更できます。「現状のまま」提供され、一切の保証はありません。&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>このソフトについて UWF Manager</translation></message>
    <message><source>System information</source><translation>システム情報</translation></message>
    <message><source>Diagnostic snapshot</source><translation>診断スナップショット</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>システム、セキュリティ、UWF、拡張モード、最近のアプリログの診断情報。</translation></message>
    <message><source>Read only</source><translation>読み取り専用</translation></message>
    <message><source>Diagnostic report</source><translation>診断レポート</translation></message>
    <message><source>Copied</source><translation>コピーしました</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>システム、セキュリティ、UWF、拡張モード、最近のアプリログの診断情報。</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>アカウント名、デバイス ID、ファイルパス、最近のログを含む場合があります。共有前に内容を確認してください。</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;このプログラムは Windows の統合書き込みフィルター (UWF) を利用します。&lt;/b&gt; 書き込み保護は Windows 内蔵の UWF が行います。UWF Manager 自体はフィルタリングせず、設定と管理のみを行うため、システムに UWF をインストールして有効にする必要があります。&lt;/p&gt;&lt;p&gt;UWF を初めて有効にすると、性能向上のため次のシステム設定が変更されます。&lt;/p&gt;&lt;ul&gt;&lt;li&gt;ページファイルを無効化します。&lt;/li&gt;&lt;li&gt;システムの復元を無効化します。&lt;/li&gt;&lt;li&gt;SuperFetch を無効化します。&lt;/li&gt;&lt;li&gt;ファイルのインデックスサービスを停止します。&lt;/li&gt;&lt;li&gt;デフラグサービスを停止します。&lt;/li&gt;&lt;li&gt;高速スタートアップを無効化します。&lt;/li&gt;&lt;li&gt;BCD の bootstatuspolicy を ignoreallfailures に設定します。&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;有効化後、これらの設定は必要に応じて変更できます。例えば、ページファイルを保護されていないボリュームに移し、ページングを再び有効にできます。&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>対象が見つかりません。オーバーレイにコミットする内容はありません。</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>対象は物理ディスクや永続レジストリにはなく、保護中に作成されたオーバーレイ内にのみ存在します。再起動で消えるため、削除のコミットは不要で、実行もできません。</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>システムが引数を拒否しました（パスまたは引数が無効です）。</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>操作に失敗しました。ファイルが開かれている、フォルダーをエクスプローラーで表示中など、別のプロセスが使用している可能性があります。使用中のプログラムを閉じて再試行してください。</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>操作に失敗しました。詳細はログを確認してください。</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>操作が拒否されました（コード %1）。</translation></message>
    <message><source>Unknown cause.</source><translation>不明.</translation></message>
    <message><source>Commit canceled</source><translation>コミットをキャンセルしました</translation></message>
    <message><source>Commit result</source><translation>コミット結果</translation></message>
    <message><source>Succeeded</source><translation>成功</translation></message>
    <message><source>Skipped</source><translation>スキップ</translation></message>
    <message><source>Failed</source><translation>失敗</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>成功: %1; スキップ: %2; 失敗: %3.</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
ユーザーがキャンセルしました。%1 件は未処理です。</translation></message>
    <message><source>Category</source><translation>分類</translation></message>
    <message><source>Path</source><translation>パス</translation></message>
    <message><source>Existed before</source><translation>操作前に存在</translation></message>
    <message><source>Exists after</source><translation>操作後に存在</translation></message>
    <message><source>Error code</source><translation>エラーコード</translation></message>
    <message><source>Reason</source><translation>理由</translation></message>
    <message><source>Previous page</source><translation>前のページ</translation></message>
    <message><source>First page</source><translation>最初のページ</translation></message>
    <message><source>Last page</source><translation>最後のページ</translation></message>
    <message><source>Next page</source><translation>次のページ</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>ページ %1 / %2 · 項目: %3</translation></message>
    <message><source>Copy selected rows</source><translation>選択行をコピー</translation></message>
    <message><source>Copy all</source><translation>すべてコピー</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>自動コミット一覧に保存済み項目がある間、このレジストリ除外は必要です. ファイルの自動コミット: クリア → レジストリ除外の削除.</translation></message>
    <message><source>0 lines</source><translation>0 行</translation></message>
    <message><source>%1 lines</source><translation>%1 行</translation></message>
    <message><source>Loading log entries…</source><translation>ログを読み込み中…</translation></message>
    <message><source>No log entries</source><translation>ログなし</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>ページ %1 / %2 · 行: %3</translation></message>
    <message><source>Copy current page</source><translation>このページをコピー</translation></message>
    <message><source>Clear</source><translation>クリア</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>保留中の変更: %1</translation></message>
    <message><source>No pending changes</source><translation>保留中の変更なし</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (レジストリの除外 · 全体設定.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>ボリューム %1: UWF 保護 / ファイルの除外 / ファイルの自動コミット.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: 未対応</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>更新 · ボリューム: %1</translation></message>
    <message><source>· Filter (global) %1</source><translation>· フィルター (UWF): %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· オーバーレイ · 種類 → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· オーバーレイ · 最大サイズ → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· オーバーレイ · 警告しきい値 → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· オーバーレイ · 重大しきい値 → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ 種類と最大サイズを変更するには：
1. 上のスイッチでフィルターを無効にします。
2. 再起動し、無効な状態にします。
3. 設定を変更します。</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· ボリューム %1 · UWF 保護: %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· ボリューム %1 · 識別方法： %2 （同等の CLI コマンドなし・本プログラム専用）</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ ファイルの除外  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− ファイルの除外  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ レジストリの除外  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− レジストリの除外  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>フィルター (UWF): %1</translation></message>
    <message><source>Overlay type → %1</source><translation>オーバーレイ · 種類 → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>オーバーレイ · 最大サイズ → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>オーバーレイ · 警告しきい値 → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>オーバーレイ · 重大しきい値 → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>ボリューム %1 · UWF 保護: %2</translation></message>
    <message><source>File exclusion %1</source><translation>ファイルの除外: %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>レジストリの除外: %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>保留中の変更 (%1)</translation></message>
    <message><source>Current session configuration</source><translation>現在のセッション設定</translation></message>
    <message><source>Review and apply changes</source><translation>変更を確認して適用</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>以下は uwfmgr コマンド形式の全設定です。保留中の変更は先頭の専用欄に表示します。 &lt;span style='color:%1'&gt;保留中の変更&lt;/span&gt;. &lt;span style='color:%2'&gt;適用&lt;/span&gt;: クリックするとシステムに変更を書き込みます。 多くの変更は次回の再起動後に反映されます。</translation></message>
    <message><source>Confirm apply</source><translation>適用の確認</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>&lt;span style='color:%1'&gt;これらの変更はシステムに書き込まれます&lt;/span&gt;. 多くの変更は次回の再起動後に反映されます。&lt;br&gt;&lt;br&gt;続行?</translation></message>
    <message><source>Applied changes</source><translation>適用した変更</translation></message>
    <message><source>Result</source><translation>結果</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ フィルター: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ 警告しきい値の設定: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ 重大しきい値の設定: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ オーバーレイ種類の設定 / 最大サイズの設定: 種類と最大サイズを変更するには：
1. 上のスイッチでフィルターを無効にします。
2. 再起動し、無効な状態にします。
3. 設定を変更します。</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ オーバーレイ種類の設定: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ 最大サイズの設定: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ 最大サイズの設定: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ ボリューム %1 · UWF 保護: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ ボリューム %1 · 識別方法： %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ ボリューム %1 · ファイル除外の追加: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ ボリューム %1 · ファイル除外の削除: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ レジストリ除外の追加: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ レジストリ除外の削除: %1</translation></message>
    <message><source>Time</source><translation>時刻</translation></message>
    <message><source>Level</source><translation>レベル</translation></message>
    <message><source>Tag</source><translation>タグ</translation></message>
    <message><source>Message</source><translation>メッセージ</translation></message>
    <message><source>Confirm safe shutdown?</source><translation>安全なシャットダウン?</translation></message>
    <message><source>Confirm safe restart?</source><translation>安全な再起動?</translation></message>
    <message><source>Shut down</source><translation>シャットダウン</translation></message>
    <message><source>Restart</source><translation>再起動</translation></message>
    <message><source>Continue shutdown</source><translation>シャットダウンを続行</translation></message>
    <message><source>Continue restart</source><translation>再起動を続行</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>UWF を使用して安全にシャットダウンします。</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>UWF を使用して安全に再起動します。</translation></message>
    <message><source>UWF protection</source><translation>UWF 保護</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>UWF のオーバーレイが満杯でも実行できます。</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>自動コミット対象のファイルを集計中…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>操作前に %1 個のファイルを自動コミットします。現在保護されていないボリュームのファイルはスキップします。</translation></message>
    <message><source>Committing staged files</source><translation>自動コミット中</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>自動コミットの完了後、電源操作を続行します。</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%2 個中 %1 個のファイルを処理しました。</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>ファイルの自動コミットが完了しませんでした</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>続行するか判断する前に、操作の詳細を確認してください。</translation></message>
    <message><source>Operation details</source><translation>操作の詳細</translation></message>
    <message><source>No additional error details were provided.</source><translation>追加のエラー情報はありません。</translation></message>
    <message><source>Some changes may not be preserved</source><translation>一部の変更を保持できない可能性があります</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>続行すると、コミットに失敗したファイルの変更を保持せずにシャットダウンします。</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>続行すると、コミットに失敗したファイルの変更を保持せずに再起動します。</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>ファイルの自動コミットが予期せず停止しました：
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>ファイルの自動コミットが完了しませんでした: 不明なエラー.</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>コミットしていない変更は失われます</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>続行する前に作業内容を保存し、保持したい変更をすべてコミットしてください。</translation></message>
    <message><source>Confirm direct restart?</source><translation>直接再起動?</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>直接シャットダウン?</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>再起動. 再起動前に、自動コミット一覧のファイルはコミットされません。</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>シャットダウン. シャットダウン前に、自動コミット一覧のファイルはコミットされません。</translation></message>
    <message><source>File staging will be skipped</source><translation>ファイルの自動コミットをスキップします</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>再起動前に、自動コミット一覧のファイルはコミットされません。</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>シャットダウン前に、自動コミット一覧のファイルはコミットされません。</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>直接再起動: ファイルの自動コミットをスキップします.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>直接シャットダウン: ファイルの自動コミットをスキップします.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>直接の電源操作を選択したため、自動コミットをスキップしました。</translation></message>
    <message><source>Safe shutdown failed</source><translation>安全なシャットダウン: 失敗</translation></message>
    <message><source>Shutdown failed: %1</source><translation>シャットダウン: 失敗 (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>安全な再起動: 失敗</translation></message>
    <message><source>Direct restart failed</source><translation>直接再起動: 失敗</translation></message>
    <message><source>Direct shutdown failed</source><translation>直接シャットダウン: 失敗</translation></message>
    <message><source>Restart failed: %1</source><translation>再起動: 失敗 (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>自動コミット一覧の読み取りに失敗しました:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>自動コミット一覧の読み取りに失敗しました: 不明なエラー.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>ファイル: %1 · 成功: %2 · スキップ (ファイル): %3 · スキップ (項目): %4 · 失敗: %5</translation></message>
    <message><source>File staging preparation</source><translation>ファイルの自動コミット: 操作の詳細</translation></message>
    <message><source>Reparse points are not supported.</source><translation>再解析ポイントには対応していません。</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>ローカルボリューム上の絶対パスのみ追加できます。</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>ローカルボリューム上の絶対パスのみ追加できます。
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>自動コミット対象のパスを検査できませんでした。</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>登録したファイルのパス種類が変わっています。</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>登録したフォルダーのパス種類が変わっています。</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>登録したフォルダーの全内容を列挙できませんでした。</translation></message>
    <message><source>The path has no drive letter.</source><translation>パスにドライブ文字がありません。</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>ボリュームのルートは再帰処理できません。</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>UWF プロバイダーがコミットを拒否しました。</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>コミット失敗: 不明なエラー.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>追加の %1 件の失敗はログに記録しました。</translation></message>
    <message><source>Commit failed</source><translation>コミット失敗</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>パスにドライブ文字がありません。 対象ボリュームの特定: 失敗.</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>失敗: ボリューム情報の読み取り (%1)</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>ボリューム %1 の現在のセッション情報がありません。</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>このパスはファイル除外一覧に含まれています。
除外：%1</translation></message>
    <message><source>No files were found under %1.</source><translation>%1 にファイルが見つかりませんでした。</translation></message>
    <message><source>Commit to disk</source><translation>ディスクにコミット</translation></message>
    <message><source>Committing…</source><translation>コミット中…</translation></message>
    <message><source>This action cannot be undone.</source><translation>この操作は元に戻せません。</translation></message>
    <message><source>Continue</source><translation>続行</translation></message>
    <message><source>Delete and commit</source><translation>削除してコミット</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>ファイルの変更をオーバーレイからディスクにコミット</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>フォルダーの変更をオーバーレイからディスクにコミット</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>ファイル: %1. 全サブフォルダーを含みます。</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>ファイルを削除し、削除をディスクにコミット</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>フォルダーと全内容を削除し、削除をディスクにコミット</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>ファイル: %1; フォルダー: %2. 削除してコミット.</translation></message>
    <message><source>Commit this registry value to disk</source><translation>レジストリ値をディスクにコミット</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>レジストリキーと全サブキーをディスクにコミット</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>値: %1. （キー全体と全サブキー）</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>レジストリ値を削除し、削除をディスクにコミット</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>レジストリキーと全サブキーを削除し、削除をディスクにコミット</translation></message>
    <message><source>Scanning registry keys…</source><translation>レジストリキーの列挙…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>レジストリキーの列挙…
キー: %1</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>レジストリ · キー · 削除してコミット:</translation></message>
    <message><source>Copy current entry</source><translation>現在の項目をコピー</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>%1 個のキーとそのすべての値を再帰的に削除します。</translation></message>
    <message><source>Commit file deletion failed</source><translation>失敗: ファイル削除のコミット</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>パスが存在しないため、削除するものはありません。</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>このキーはレジストリ除外一覧に含まれています。
除外：%1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>失敗: レジストリフィルターの読み取り (%1)</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>現在のセッションのレジストリフィルター情報がありません。</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>この値は存在しないため、コミットするものはありません。</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>このキーは存在しないため、コミットするものはありません。</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>この値は存在しないため、削除するものはありません。</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>このキーは存在しないため、削除するものはありません。</translation></message>
    <message><source>Export commands…</source><translation>コマンドをエクスポート…</translation></message>
    <message><source>Export commands to file</source><translation>コマンドをファイルにエクスポート</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>ファイルにエクスポート… · コマンド: %1
%2</translation></message>
    <message><source>Import</source><translation>インポート</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: コマンドを一行ずつ入力または貼り付けてください。 ログやスクリプトなどのテキスト形式（.txt、.bat、.ps1）から、このコマンドを含む行を追加します。 インポートは変更を保留状態にします。システムに書き込むには、ツールバーの「確認して適用」を選んでください。</translation></message>
    <message><source>Import uwfmgr commands</source><translation>uwfmgr · インポート</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;uwfmgr&lt;/b&gt;: コマンドを一行ずつ入力または貼り付けてください。 &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;ファイルから読み込む…&lt;/b&gt;: ログやスクリプトなどのテキスト形式（.txt、.bat、.ps1）から、このコマンドを含む行を追加します。 &lt;code&gt;uwfmgr&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;インポート&lt;/b&gt;: &lt;b&gt;インポートは変更を保留状態にします。システムに書き込むには、ツールバーの「確認して適用」を選んでください。&lt;/b&gt; &lt;b&gt;確認して適用&lt;/b&gt;.&lt;/p&gt;</translation></message>
    <message><source>Load from file…</source><translation>ファイルから読み込む…</translation></message>
    <message><source>Load recommended configuration</source><translation>推奨設定を読み込む</translation></message>
    <message><source>Choose recommended configuration</source><translation>推奨設定を選択</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>追加する推奨設定のグループを選んでください。インポート前に個々のコマンドを確認・削除できます。</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Microsoft 公式ドキュメントより。</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Microsoft 推奨の UWF 除外項目</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>インポート前に確認してください。ファイル除外を UWF に追加する前に、対象フォルダーが存在する必要があります。</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>出典：Microsoft 公式 UWF ドキュメント（一般的な除外項目およびマルウェア対策のサポート）。</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>カスタマー エクスペリエンス向上プログラム (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: 永続保存: ポリシーでの参加状態</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: 永続保存: ローカルの参加状態</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: 永続保存: アップロード無効化フラグ</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>バックグラウンド インテリジェント転送サービス (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: 永続保存: ダウンロードキューファイル</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: 永続保存: 転送状態インデックス</translation></message>
    <message><source>Network profiles and policies</source><translation>ネットワークプロファイルとポリシー</translation></message>
    <message><source>Wireless network GPO policy</source><translation>無線ネットワーク GPO ポリシー</translation></message>
    <message><source>Wired network GPO policy</source><translation>有線ネットワーク GPO ポリシー</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>無線ネットワーク GPO ポリシー · ファイル</translation></message>
    <message><source>Wired network GPO policy files</source><translation>有線ネットワーク GPO ポリシー · ファイル</translation></message>
    <message><source>Wireless network interface profiles</source><translation>無線インターフェイスプロファイル</translation></message>
    <message><source>Wired network interface profiles</source><translation>有線インターフェイスプロファイル</translation></message>
    <message><source>Wireless service configuration</source><translation>無線サービス設定</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>モバイルブロードバンドサービス設定</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>有線自動構成サービス設定</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>ネットワークプロファイル XML はデバイスごとに異なります。必要に応じて Interfaces\{GUID}\{GUID}.xml の具体的なパスを手動で追加してください。</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>夏時間 (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: 永続保存: タイムゾーン定義</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: 永続保存: 選択したタイムゾーン</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: 永続保存: 製品ファイルと更新</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: 永続保存: ProgramData 内の定義ファイルと状態</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: 永続保存: Windows Update · ログ</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: 永続保存: MpCmdRun · ログ</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: 永続保存: 製品のレジストリ状態</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: 永続保存: WdBoot · サービス状態</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: 永続保存: WdFilter · サービス状態</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: 永続保存: WdNisSvc · サービス状態</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: 永続保存: WdNisDrv · サービス状態</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: 永続保存: WinDefend · サービス状態</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: 永続保存: クライアントプログラムファイル</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: 永続保存: Windows Update · ログ</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: 永続保存: MpCmdRun · ログ</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: 永続保存: マルウェア対策の定義ファイルと状態</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: 永続保存: マルウェア対策のレジストリ状態</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>uwfmgr コマンドを含むファイルを選択</translation></message>
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>すべてのファイル (*);;テキストファイル (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
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
    <message><source>Status</source><translation>状態</translation></message>
    <message><source>Command</source><translation>コマンド</translation></message>
    <message><source>Detail</source><translation>詳細</translation></message>
    <message><source>Import failed</source><translation>インポート失敗</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>内部エラー：適用処理が登録されていません。</translation></message>
    <message><source>Nothing to import</source><translation>インポートするコマンドがありません</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>入力に uwfmgr コマンドが見つかりませんでした。</translation></message>
    <message><source>Applied: %1</source><translation>適用済み: %1</translation></message>
    <message><source>Duplicates: %1</source><translation>重複: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>未対応: %1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>%1 回のインポート後の累計：</translation></message>
    <message><source>── Batch %1 ──</source><translation>── インポート %1 ──</translation></message>
    <message><source>Applied</source><translation>適用済み</translation></message>
    <message><source>Duplicate</source><translation>重複</translation></message>
    <message><source>Unsupported</source><translation>未対応</translation></message>
    <message><source>File too large</source><translation>ファイルが大きすぎます</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>%1 は 5 MB を超えるため解析しませんでした。必要な内容を手動で絞り込んでください。</translation></message>
    <message><source>Cannot read file</source><translation>ファイルを読み取れません</translation></message>
    <message><source>Could not open file %1: %2</source><translation>失敗: ファイルを開く (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>保留中の変更を破棄しますか？</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>未適用の変更が %1 件あります。

破棄して続行しますか？</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>uwfmgr コマンドが不完全です</translation></message>
    <message><source>Missing size argument (MB)</source><translation>引数がありません： サイズ (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>サイズは MB 単位の 0 以上の整数にしてください</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>引数がありません： オーバーレイ · 種類 (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>無効な値： オーバーレイ · 種類: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>引数がありません： ボリューム (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>ボリュームは C: のようなドライブ文字で指定してください</translation></message>
    <message><source>Missing path argument</source><translation>引数がありません： パス</translation></message>
    <message><source>Missing registry key argument</source><translation>引数がありません： レジストリ · キー</translation></message>
    <message><source>Unclosed double quote in command</source><translation>コマンド内の二重引用符が閉じられていません</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>余分な引数があります：%1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>UI の操作に対応付けられない uwfmgr コマンドです</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>保留中の変更: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>既に指定した状態のため、操作は不要です</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>このボリューム外のパスか、ファイル除外に非対応のボリュームです（exFAT / ReFS など）</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>UWF の禁止リストにより拒否されました（システムファイル、Windows、ページファイルなど）</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>このバッチで既に同じコマンドを処理しています</translation></message>
    <message><source>Pending filter %1</source><translation>保留中の変更: フィルター %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>フィルター: 既に指定した状態のため、操作は不要です</translation></message>
    <message><source>Pending overlay type → %1</source><translation>保留中の変更: オーバーレイ · 種類 → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>オーバーレイ · 種類: %1 (既に指定した状態のため、操作は不要です)</translation></message>
    <message><source>Invalid size value: %1</source><translation>無効な値： サイズ: %1</translation></message>
    <message><source>maximum size</source><translation>最大サイズ</translation></message>
    <message><source>warning threshold</source><translation>警告しきい値</translation></message>
    <message><source>critical threshold</source><translation>重大しきい値</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>保留中の変更: オーバーレイ %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>オーバーレイ %1: %2 MB (既に指定した状態のため、操作は不要です)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>不明なボリューム %1（このドライブ文字に UWF 対応ディスクがありません）</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>保留中の変更: ボリューム %1 · UWF 保護: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>ボリューム %1: 既に指定した状態のため、操作は不要です</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: パスにドライブ文字がありません。 対象ボリュームの特定: 失敗.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>ボリューム %1: UWF 未対応</translation></message>
    <message><source>file exclusion</source><translation>ファイルの除外</translation></message>
    <message><source>registry exclusion</source><translation>レジストリの除外</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>レジストリ除外を扱うタブがありません</translation></message>
    <message><source>Unsupported command</source><translation>コマンド: 未対応</translation></message>
    <message><source>Exit</source><translation>終了</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: 表示切り替え.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: 有効</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: 無効</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: 不明</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>使用中 %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>拡張モード</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Windows の終了・再起動時に、ファイルの自動コミットを連携させます。</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>自動起動の LocalSystem サービスをインストールします。Windows 起動後に UWF Manager を起動し、終了・再起動前に登録済みファイルの変更をコミットして、UWF と競合するアプリの互換性改善を図ります。</translation></message>
    <message><source>System changes</source><translation>システムへの変更</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>自動起動サービス UWFManagerEnhanced を作成し、現在の実行ファイルに関連付けます</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>設定: レジストリ → コミット (HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>次回の Windows 起動後に自動起動します。実行ファイルを移動すると自動起動できなくなる場合があります</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>Windows の通常の終了・再起動時に、条件を満たすファイルをコミットします</translation></message>
    <message><source>Operational and security considerations</source><translation>運用とセキュリティの留意事項</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>サービスをシステムのレジストリに登録するため、完全なポータブル動作ではなくなります</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>対象ファイルが多いと終了・再起動が大幅に遅くなります。変更の保持に十分な時間を確保してください</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>拡張モードは UWF Manager を SYSTEM 権限で実行します。管理者以外が実行ファイルを改ざん・置換できると、SYSTEM 権限を奪取される恐れがあります。管理者のみ変更できる場所から有効にしてください（推奨：%ProgramFiles%\UWF Manager\Service\UWF.exe）。</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>失敗: 拡張モード状態の読み取り</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>失敗: 拡張モード状態の読み取り (不明なエラー).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>失敗: サービス状態の読み取り:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>失敗: サービス状態の読み取り (不明なエラー).</translation></message>
    <message><source>Status: Disabled</source><translation>状態: 無効</translation></message>
    <message><source>Status: Enabled</source><translation>状態: 有効</translation></message>
    <message><source>Status: Service stopped</source><translation>状態：サービス停止</translation></message>
    <message><source>Status: Repair required</source><translation>状態：修復が必要</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>サービスが独立したプロセスとして構成されていません。</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>サービスが自動起動に設定されていません。</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>サービスが LocalSystem で実行される設定ではありません。</translation></message>
    <message><source>The service is not running.</source><translation>サービスは実行されていません。</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>サービスの実行パスまたは起動引数が、現在のプログラムと一致しません。</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>サービスのシャットダウン事前通知の待機時間が正しく設定されていません。</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>UI エージェントの起動に必要な権限が、サービスで宣言されていません。</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>サービスが Windows のシャットダウン事前通知を受け付けていません。</translation></message>
    <message><source>Enable enhanced mode</source><translation>拡張モードを有効化</translation></message>
    <message><source>Disable enhanced mode</source><translation>拡張モードを無効化</translation></message>
    <message><source>Start service</source><translation>サービスを開始</translation></message>
    <message><source>Repair enhanced mode</source><translation>拡張モードを修復</translation></message>
    <message><source>Delete service</source><translation>サービスを削除</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>拡張モードのサービスを停止して削除し、残存するサービスのレジストリデータも削除します。</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>サービスはありませんが、レジストリに設定が残っています。</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>UI エージェントの認証完了を待っています。</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>サービスは動作中ですが、認証済みの UI エージェントが接続していません。</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>UWF Manager 拡張モード補助サービス</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>拡張モード · 設定: 適用済み.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>設定: 適用済み. UWF の永続化からの報告:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>拡張モード · 設定: 失敗:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>拡張モード · 設定: 失敗 (不明なエラー).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>拡張モードのサービスと残存設定を削除しました。</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>サービスの削除を要求しましたが、残存データの削除で次の報告がありました：
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>拡張モード: サービスを削除: 失敗:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>拡張モード: サービスを削除: 失敗 (不明なエラー).</translation></message>
    <message><source>File staging completed</source><translation>ファイルの自動コミット: 成功</translation></message>
    <message><source>File staging failed</source><translation>ファイルの自動コミット: 失敗</translation></message>
    <message><source>All staged targets were processed.</source><translation>すべての対象を処理しました。</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>すべての対象を処理しましたが、一部の操作に失敗しました。</translation></message>
    <message><source>Show main window</source><translation>メインウィンドウを表示</translation></message>
    <message><source>Restore default position</source><translation>既定の位置に戻す</translation></message>
    <message><source>Hide overlay hub</source><translation>オーバーレイ HUB を隠す</translation></message>
    <message><source>Exit application</source><translation>アプリケーションを終了</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ 失敗: システムへの接続</translation></message>
    <message><source>Registry access failed</source><translation>失敗: レジストリへのアクセス</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>失敗: レジストリ値の読み取り (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>オーバーレイファイルの列挙: 不明なエラー.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ 失敗: フィルターの%1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ 失敗: フィルター状態の読み取り</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ 失敗: 警告しきい値の設定</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ 失敗: 重大しきい値の設定</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ 失敗: オーバーレイ状態の読み取り</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ 失敗: オーバーレイ種類の設定</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ 失敗: 最大サイズの設定</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ 失敗: オーバーレイ設定の読み取り</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ 失敗: ボリューム %1 の UWF 登録</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ 失敗: ボリューム %2 の保護を%1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ 失敗: ボリューム %1 の識別方法の設定</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ ボリューム %1 · 失敗: ファイル除外の追加 %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ ボリューム %1 のファイル除外は既にありません：%2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ ボリューム %1 · 失敗: ファイル除外の削除 %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ 失敗: ボリューム設定の読み取り</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ 失敗: レジストリ除外の追加 %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ 失敗: レジストリ除外の削除 %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ 失敗: レジストリ永続化スイッチの更新</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ 失敗: レジストリフィルターの読み取り</translation></message>
    <message><source>UWF is not available</source><translation>UWF: 未対応</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>対象の状態の読み取り: 不明なエラー.</translation></message>
    <message><source>The target state could not be read before the operation: %1</source><translation>操作前に対象の状態を読み取れませんでした：%1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>パスが存在しないため、削除するものはありません。</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>不明なエラー.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>操作後の状態を再読み取りできず、結果を確認できませんでした：%1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>プロバイダーは削除を受け付けましたが、確実な情報源から再確認したところ、対象はまだ存在します。</translation></message>
    <message><source>Unknown</source><translation>不明</translation></message>
    <message><source>Failed to load logs: %1</source><translation>失敗: ログの読み込み (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>失敗: ログの読み込み (不明なエラー).</translation></message>
    <message><source>: unknown error</source><translation>: 不明なエラー</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>失敗: 対象の状態の読み取り (%1)</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>失敗: 対象の状態の読み取り (不明なエラー).</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>失敗: 対象ボリュームの特定 (%1)</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>失敗: レジストリ値の列挙 (%1)</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>失敗: レジストリキーの列挙 (%1)</translation></message>
    <message><source>Delete and commit failed</source><translation>削除してコミット: 失敗</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>失敗: 対象ボリュームの特定 (%1): %2</translation></message>
    <message><source>%1/%2 slots</source><translation>%1/%2 スロット</translation></message>
</context>
</TS>
