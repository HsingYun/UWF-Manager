<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="ja" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>プレビュー</translation></message>
    <message><source>enable</source><translation>有効にする</translation></message>
    <message><source>disable</source><translation>無効にする</translation></message>
    <message><source>Enabled</source><translation>有効</translation></message>
    <message><source>Disabled</source><translation>無効</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>キャンセル</translation></message>
    <message><source>Close</source><translation>閉じる</translation></message>
    <message><source>Refresh</source><translation>更新</translation></message>
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
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>現在のセッションでの保護状態（読み取り専用）。</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>次のセッションでこのボリュームを保護します。書き込みはオーバーレイに転送され、再起動時に破棄されます。</translation></message>
    <message><source>Current session</source><translation>現在のセッション</translation></message>
    <message><source>Next session</source><translation>次のセッション</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>ボリュームの識別方法です。ドライブ文字は簡単ですが、ディスクの増設・取り外しなどで文字が変わると関連付けが無効になります。ボリューム ID は文字が変わっても維持されます。</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF 非対応のボリューム： </translation></message>
    <message><source>Global settings</source><translation>全体設定</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>現在のセッションの UWF フィルター状態（読み取り専用）。</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>次のセッションで UWF を有効にします。保護されたボリュームへの書き込みはオーバーレイに転送され、再起動時に破棄されます。</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>オーバーレイの使用量がこの値に達すると、警告レベルの通知をイベントログに記録します。両方のイベントが有効な場合、この値は重大しきい値より小さくする必要があります。0 に設定すると、警告イベントのみを無効にします。</translation></message>
    <message><source>Critical threshold</source><translation>重大しきい値</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>オーバーレイの使用量がこの値に達すると、重大レベルの通知をイベントログに記録します。両方のイベントが有効な場合、この値は警告しきい値より大きくする必要があります。0 に設定すると、重大イベントのみを無効にします。</translation></message>
    <message><source>Overlay</source><translation>オーバーレイ</translation></message>
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
    <message><source>File staging</source><translation>ファイルの保存待ち</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>ファイル / フォルダー: 安全なシャットダウンまたは再起動の前に、自動でコミットします。 全サブフォルダーを含みます。 一覧に項目がある間は、次のレジストリ除外により、保護されたセッション間でも一覧を保持します： HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>ファイルまたはフォルダーを自動コミット一覧に追加します。</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>ディスクにコミットするファイルを選択. 安全なシャットダウンまたは再起動の前に、自動でコミットします。</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>ディスクにコミットするフォルダーを選択. 安全なシャットダウンまたは再起動の前に、自動でコミットします。 全サブフォルダーを含みます。</translation></message>
    <message><source>Select files for automatic commit</source><translation>ディスクにコミットするファイルを選択 · ファイルの保存待ち</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>ディスクにコミットするフォルダーを選択 · ファイルの保存待ち · 全サブフォルダーを含みます。</translation></message>
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
    <message><source>File staging is unavailable: %1</source><translation>ファイルの保存待ち: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>安全なシャットダウンまたは再起動の前に、自動でコミットします。</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>ファイルの保存待ち: %1 · ファイル: %2 · フォルダー: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>ファイルの保存待ち一覧を再起動後も保持するため、このレジストリ除外が必要です。</translation></message>
    <message><source>Built-in</source><translation>組み込み</translation></message>
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
    <message><source>Name</source><translation>名前</translation></message>
    <message><source>Data</source><translation>データ</translation></message>
    <message><source>(Default)</source><translation>（既定）</translation></message>
    <message><source>(value not set)</source><translation>（値の設定なし）</translation></message>
    <message><source>(empty)</source><translation>（空）</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>不明 (値: ファイルが大きすぎます)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>オーバーレイのファイルを表示. &lt;b&gt;診断用のスナップショットです。&lt;/b&gt;コミット対象の判断には使用しないでください。</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>オーバーレイ一覧は NTFS ボリュームのみ対応しています。</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF が無効のため、検査できるオーバーレイはありません。</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>このボリュームは UWF で保護されていないため、検査できるオーバーレイはありません。</translation></message>
    <message><source>Failed: %1</source><translation>失敗: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>列挙中に WMI プロバイダーがクラッシュしました。オーバーレイが大きい場合や I/O 負荷が高い場合に発生する UWF_Overlay.GetOverlayFiles の既知の不具合です。</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>メモリ不足、またはプロバイダーが操作に対応していません。NTFS と十分な空きメモリが必要です。オーバーレイを小さくするか、メモリを多く使うアプリを閉じて再試行してください。</translation></message>
    <message><source>Raw error: %1</source><translation>エラーコード: %1</translation></message>
    <message><source>Export overlay file list</source><translation>ファイルにエクスポート… · オーバーレイ · ファイル</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>テキストファイル (*.txt);;すべてのファイル (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>コマンドファイル (*.cmd);;すべてのファイル (*)</translation></message>
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
    <message><source>✓ %1 persistence: %2</source><translation>✓ 永続化 %1: %2</translation></message>
    <message><source>Remove selected</source><translation>選択項目を削除</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>選択した除外項目を削除します。「適用」後に反映されます。</translation></message>
    <message><source>Open containing folder</source><translation>保存先フォルダーを開く</translation></message>
    <message><source>Copy file path</source><translation>ファイルパスをコピー</translation></message>
    <message><source>Copy registry path</source><translation>レジストリパスをコピー</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>フォルダーの変更をオーバーレイからディスクにコミット…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>ファイルの変更をオーバーレイからディスクにコミット…</translation></message>
    <message><source>Copied to clipboard</source><translation>クリップボードにコピーしました</translation></message>
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
    <message><source>Unified Write Filter (UWF) Manager</source><translation>統合書き込みフィルター (UWF) マネージャー</translation></message>
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
    <message><source>Switch display language</source><translation>表示言語を切り替えます</translation></message>
    <message><source>Language</source><translation>言語</translation></message>
    <message><source>Theme</source><translation>テーマ</translation></message>
    <message><source>Toggle light / dark theme</source><translation>ライト／ダークテーマを切り替えます</translation></message>
    <message><source>System information</source><translation>システム情報</translation></message>
    <message><source>Diagnostic snapshot</source><translation>診断スナップショット</translation></message>
    <message><source>Read only</source><translation>読み取り専用</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>対象が見つかりません。オーバーレイにコミットする内容はありません。</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>対象は物理ディスクや永続レジストリにはなく、保護中に作成されたオーバーレイ内にのみ存在します。再起動で消えるため、削除のコミットは不要で、実行もできません。</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>システムが引数を拒否しました（パスまたは引数が無効です）。</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>操作に失敗しました。ファイルが開かれている、フォルダーをエクスプローラーで表示中など、別のプロセスが使用している可能性があります。使用中のプログラムを閉じて再試行してください。</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>操作に失敗しました。詳細はログを確認してください。</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>操作が拒否されました（コード %1）。</translation></message>
    <message><source>Unknown cause.</source><translation>不明.</translation></message>
    <message><source>Commit canceled</source><translation>コミットをキャンセルしました</translation></message>

    <message><source>Succeeded</source><translation>成功</translation></message>
    <message><source>Skipped</source><translation>スキップ</translation></message>
    <message><source>Failed</source><translation>失敗</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>成功: %1; スキップ: %2; 失敗: %3.</translation></message>

    <message><source>Existed before</source><translation>操作前に存在</translation></message>
    <message><source>Exists after</source><translation>操作後に存在</translation></message>
    <message><source>Error code</source><translation>エラーコード</translation></message>
    <message><source>Reason</source><translation>理由</translation></message>
    <message><source>Previous page</source><translation>前のページ</translation></message>
    <message><source>First page</source><translation>最初のページ</translation></message>
    <message><source>Last page</source><translation>最後のページ</translation></message>
    <message><source>Next page</source><translation>次のページ</translation></message>
    <message><source>Copy selected rows</source><translation>選択行をコピー</translation></message>
    <message><source>Copy all</source><translation>すべてコピー</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>自動コミット一覧に保存済み項目がある間、このレジストリ除外は必要です. ファイルの保存待ち: クリア → レジストリ除外の削除.</translation></message>
    <message><source>Copy current page</source><translation>このページをコピー</translation></message>
    <message><source>No pending changes</source><translation>保留中の変更なし</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (レジストリの除外 · 全体設定.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>ボリューム %1: UWF 保護 / ファイルの除外 / ファイルの保存待ち.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: 未対応</translation></message>
    <message><source>Review and apply changes</source><translation>変更を確認して適用</translation></message>
    <message><source>Confirm apply</source><translation>適用の確認</translation></message>
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
    <message><source>Calculating files for automatic commit…</source><translation>自動コミット対象のファイルを集計中…</translation></message>
    <message><source>Committing staged files</source><translation>自動コミット中</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%2 個中 %1 個のファイルを処理しました。</translation></message>
    <message><source>No additional error details were provided.</source><translation>追加のエラー情報はありません。</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>ファイルの保存待ちが予期せず停止しました：
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>ファイルの保存待ちが完了しませんでした: 不明なエラー.</translation></message>
    <message><source>File staging will be skipped</source><translation>ファイルの保存待ちをスキップします</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>直接再起動: ファイルの保存待ちをスキップします.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>直接シャットダウン: ファイルの保存待ちをスキップします.</translation></message>
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
    <message><source>File staging preparation</source><translation>ファイルの保存待ち: 操作の詳細</translation></message>
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

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>パスにドライブ文字がありません。 対象ボリュームの特定: 失敗.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>ボリューム %1 の現在のセッション情報がありません。</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>このパスはファイル除外一覧に含まれています。
除外：%1</translation></message>

    <message><source>Continue</source><translation>続行</translation></message>
    <message><source>Delete and commit</source><translation>削除してコミット</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>このキーはレジストリ除外一覧に含まれています。
除外：%1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>現在のセッションのレジストリフィルター情報がありません。</translation></message>

    <message><source>Export commands to file</source><translation>コマンドをファイルにエクスポート</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>ファイルにエクスポート… · コマンド: %1
%2</translation></message>
    <message><source>Import</source><translation>インポート</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: コマンドを一行ずつ入力または貼り付けてください。 ログやスクリプトなどのテキスト形式（.txt、.bat、.ps1）から、このコマンドを含む行を追加します。 インポートは変更を保留状態にします。システムに書き込むには、ツールバーの「確認して適用」を選んでください。</translation></message>
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
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>テキストファイル (*.txt *.cmd);;すべてのファイル (*)</translation></message>
    <message><source>Status</source><translation>状態</translation></message>
    <message><source>Import failed</source><translation>インポート失敗</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>内部エラー：適用処理が登録されていません。</translation></message>
    <message><source>Nothing to import</source><translation>インポートするコマンドがありません</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>入力に uwfmgr コマンドが見つかりませんでした。</translation></message>
    <message><source>Duplicates: %1</source><translation>重複: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>未対応: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── インポート %1 ──</translation></message>
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
    <message><source>Already in the target state — no-op</source><translation>既に指定した状態のため、操作は不要です</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>このボリューム外のパスか、ファイル除外に非対応のボリュームです（exFAT / ReFS など）</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>UWF の禁止リストにより拒否されました（システムファイル、Windows、ページファイルなど）</translation></message>
    <message><source>Invalid size value: %1</source><translation>無効な値： サイズ: %1</translation></message>
    <message><source>maximum size</source><translation>最大サイズ</translation></message>
    <message><source>warning threshold</source><translation>警告しきい値</translation></message>
    <message><source>critical threshold</source><translation>重大しきい値</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>全体設定を利用できないため、このコマンドは適用待ちに追加されませんでした。</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>不明なボリューム %1（このドライブ文字に UWF 対応ディスクがありません）</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>ボリューム %1: 既に指定した状態のため、操作は不要です</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: パスにドライブ文字がありません。 対象ボリュームの特定: 失敗.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>ボリューム %1: UWF 未対応</translation></message>
    <message><source>file exclusion</source><translation>ファイルの除外</translation></message>
    <message><source>registry exclusion</source><translation>レジストリの除外</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>レジストリ除外を扱うタブがありません</translation></message>
    <message><source>Unsupported command</source><translation>コマンド: 未対応</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: 表示切り替え.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: 有効</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: 無効</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: 不明</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>使用中 %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>拡張モード</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Windows の終了・再起動時に、ファイルの保存待ちを連携させます。</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>失敗: 拡張モード状態の読み取り (不明なエラー).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>失敗: サービス状態の読み取り:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>失敗: サービス状態の読み取り (不明なエラー).</translation></message>
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
    <message><source>File staging completed</source><translation>ファイルの保存待ち: 成功</translation></message>
    <message><source>File staging failed</source><translation>ファイルの保存待ち: 失敗</translation></message>
    <message><source>All staged targets were processed.</source><translation>すべての対象を処理しました。</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>すべての対象を処理しましたが、一部の操作に失敗しました。</translation></message>
    <message><source>Show main window</source><translation>メインウィンドウを表示</translation></message>
    <message><source>Restore default position</source><translation>既定の位置に戻す</translation></message>
    <message><source>Show overlay hub</source><translation>オーバーレイ HUB を表示</translation></message>
    <message><source>Hide overlay hub</source><translation>オーバーレイ HUB を隠す</translation></message>
    <message><source>Exit application</source><translation>アプリケーションを終了</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ 失敗: システムへの接続</translation></message>
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
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>パスが存在しないため、削除するものはありません。</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>不明なエラー.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>操作後の状態を再読み取りできず、結果を確認できませんでした：%1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>プロバイダーは削除を受け付けましたが、確実な情報源から再確認したところ、対象はまだ存在します。</translation></message>
    <message><source>Unknown</source><translation>不明</translation></message>
    <message><source>Failed to load logs: %1</source><translation>失敗: ログの読み込み (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>失敗: ログの読み込み (不明なエラー).</translation></message>
    <message><source>: unknown error</source><translation>: 不明なエラー</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>失敗: 対象ボリュームの特定 (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>失敗: レジストリキーの列挙 (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>失敗: 対象ボリュームの特定 (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>前提となる操作が失敗したか、結果を確認できなかったため、この操作をスキップしました。</translation></message>
    <message><source>Copy</source><translation>コピー</translation></message>
    <message><source>Select All</source><translation>すべてを選択</translation></message>
    <message><source>Display language</source><translation>表示言語</translation></message>
    <message><source>Folder</source><translation>フォルダー</translation></message>
    <message><source>File</source><translation>ファイル</translation></message>
    <message><source>Export commands</source><translation>コマンドをエクスポート</translation></message>
    <message><source>Direct restart…</source><translation>直接再起動…</translation></message>
    <message><source>Clear…</source><translation>クリア…</translation></message>
    <message><source>Committing</source><translation>コミット中</translation></message>
    <message><source> · %1 selected</source><translation> · %1 件選択</translation></message>
    <message><source>%1 changes</source><translation>%1 件の変更</translation></message>
    <message><source>%1 commands</source><translation>%1 個のコマンド</translation></message>
    <message><source>%1 items</source><translation>%1 項目</translation></message>
    <message><source>%1 items · %2</source><translation>%1 項目 · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 件の保留中の変更</translation></message>
    <message><source>%1 records</source><translation>%1 件のレコード</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 件のレコード · 合計 %2 件</translation></message>


    <message><source>%1 selected</source><translation>%1 件選択</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 ファイル</translation></message>
    <message><source>32-bit</source><translation>32 ビット</translation></message>
    <message><source>64-bit</source><translation>64 ビット</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt; に基づいて公開されています。全文はアプリに付属の LICENSE ファイルをご覧ください。&lt;/p&gt;&lt;p&gt;GPL v3 の条件に従って再配布・改変できます。本ソフトウェアは現状のまま提供され、いかなる保証もありません。&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows はページファイル、システムの復元、SuperFetch、ファイルのインデックス作成、ディスクの最適化、高速スタートアップを無効にします。また、BCD の bootstatuspolicy を ignoreallfailures に設定します。&lt;/p&gt;&lt;p&gt;これらは後から変更できます。たとえば、ページファイルを保護されていないボリュームへ移してから、ページングを再び有効にできます。&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>システム、権限、UWF の診断スナップショットです。</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>この場所には既にファイルがあります：
%1

保存すると内容が置き換わります。</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Windows Unified Write Filter のグラフィカル管理ツール。
ボリューム保護、オーバーレイ、除外、保存待ちファイルを管理します。</translation></message>
    <message><source>About this snapshot</source><translation>このスナップショットについて</translation></message>
    <message><source>Account</source><translation>アカウント</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>スクリプトやテキストから UWF 設定を追加します。</translation></message>
    <message><source>Add exclusion</source><translation>除外を追加</translation></message>
    <message><source>Add files or folders</source><translation>ファイルやフォルダーを追加</translation></message>
    <message><source>Add selected items</source><translation>選択した項目を追加</translation></message>
    <message><source>Added</source><translation>追加済み</translation></message>
    <message><source>Administrator</source><translation>管理者</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>すべての変更をディスクに保存しました。再起動せずに保持されます。</translation></message>
    <message><source>All files (*)</source><translation>すべてのファイル (*)</translation></message>
    <message><source>All levels</source><translation>すべてのレベル</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>診断スナップショットが空でも、オーバーレイに変更がないとは限りません。</translation></message>
    <message><source>Application path</source><translation>アプリのパス</translation></message>
    <message><source>Application result</source><translation>適用結果</translation></message>
    <message><source>Apply changes</source><translation>変更を適用</translation></message>
    <message><source>Architecture</source><translation>アーキテクチャ</translation></message>
    <message><source>Automatic</source><translation>自動</translation></message>
    <message><source>Automatic file staging</source><translation>保存待ちファイルの自動保存</translation></message>
    <message><source>Automatic saving incomplete</source><translation>自動保存が未完了</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>通常のシャットダウンや再起動時に、対象の保存待ちファイルを自動的に保存します。</translation></message>
    <message><source>Available</source><translation>利用可能</translation></message>
    <message><source>Back</source><translation>戻る</translation></message>
    <message><source>Background service</source><translation>バックグラウンドサービス</translation></message>
    <message><source>Binding</source><translation>関連付け</translation></message>
    <message><source>By drive letter</source><translation>ドライブ文字で識別</translation></message>
    <message><source>By volume ID</source><translation>ボリューム ID で識別</translation></message>
    <message><source>Cancel reading</source><translation>読み取りをキャンセル</translation></message>
    <message><source>Changes already committed are retained.</source><translation>確定済みの変更は保持されます。</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>UWF の保護から除外するレジストリ キーを選択します。サブキーと値も含まれます。</translation></message>
    <message><source>Choose a location to save the file.</source><translation>ファイルの保存先を選択してください。</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>許可されたドライブまたはフォルダー内の場所を選択してください。</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>別のフォルダーを選ぶか、アクセス権を確認して再試行してください。</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>別のキーを選ぶか、アクセス権を確認して再試行してください。</translation></message>
    <message><source>Clear filters</source><translation>絞り込みを解除</translation></message>
    <message><source>Clear logs</source><translation>ログを消去</translation></message>
    <message><source>Clear logs?</source><translation>ログを消去しますか？</translation></message>
    <message><source>Collapse</source><translation>折りたたむ</translation></message>
    <message><source>Commit changes</source><translation>変更を確定</translation></message>
    <message><source>Commit changes…</source><translation>変更を確定…</translation></message>
    <message><source>Commit complete</source><translation>確定完了</translation></message>
    <message><source>Commit file changes?</source><translation>ファイルの変更を確定しますか？</translation></message>
    <message><source>Commit folder changes?</source><translation>フォルダーの変更を確定しますか？</translation></message>
    <message><source>Commit registry key changes?</source><translation>レジストリキーの変更を確定しますか？</translation></message>
    <message><source>Commit registry value changes?</source><translation>レジストリ値の変更を確定しますか？</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>通常のシャットダウンや再起動の前に保存待ちファイルを確定</translation></message>
    <message><source>Compatibility mode</source><translation>互換モード</translation></message>
    <message><source>Configuration applied</source><translation>設定を適用しました</translation></message>
    <message><source>Configuration error</source><translation>設定エラー</translation></message>
    <message><source>Configured</source><translation>設定済み</translation></message>
    <message><source>Confirm before continuing</source><translation>続行する前に確認</translation></message>
    <message><source>Connected and authenticated</source><translation>接続・認証済み</translation></message>
    <message><source>Connecting…</source><translation>接続中…</translation></message>
    <message><source>Contact author</source><translation>作者に連絡</translation></message>
    <message><source>Continue editing</source><translation>編集を続ける</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>再起動を続けると、保存できなかったファイルの変更が失われます。</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>シャットダウンを続けると、保存できなかったファイルの変更が失われます。</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Windows のシャットダウンと連携</translation></message>
    <message><source>Copy all logs</source><translation>すべてのログをコピー</translation></message>
    <message><source>Copy full report</source><translation>レポート全体をコピー</translation></message>
    <message><source>Copy name</source><translation>名前をコピー</translation></message>
    <message><source>Copy path</source><translation>パスをコピー</translation></message>
    <message><source>Copy selection</source><translation>選択範囲をコピー</translation></message>
    <message><source>Copy version</source><translation>バージョンをコピー</translation></message>
    <message><source>Counting staged files…</source><translation>保存待ちファイルを数えています…</translation></message>
    <message><source>Create</source><translation>作成</translation></message>
    <message><source>Critical %1 MB</source><translation>危険 %1 MB</translation></message>
    <message><source>Current filter</source><translation>現在のフィルター</translation></message>
    <message><source>Current run</source><translation>今回の実行</translation></message>
    <message><source>Current selection</source><translation>現在の選択</translation></message>
    <message><source>Current usage</source><translation>現在の使用量</translation></message>
    <message><source>Date modified</source><translation>更新日時</translation></message>
    <message><source>Debug</source><translation>デバッグ</translation></message>
    <message><source>Delete and commit this file?</source><translation>このファイルを削除して確定しますか？</translation></message>
    <message><source>Delete and commit this folder?</source><translation>このフォルダーを削除して確定しますか？</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>このレジストリキーを削除して確定しますか？</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>このレジストリ値を削除して確定しますか？</translation></message>

    <message><source>Direct restart?</source><translation>直接再起動しますか？</translation></message>
    <message><source>Direct shutdown?</source><translation>直接シャットダウンしますか？</translation></message>
    <message><source>Discard changes</source><translation>変更を破棄</translation></message>
    <message><source>Disk</source><translation>ディスク</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>ドメインシークレットキーの永続化</translation></message>
    <message><source>Done</source><translation>完了</translation></message>
    <message><source>Edit commands</source><translation>コマンドを編集</translation></message>
    <message><source>Editable</source><translation>編集可能</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>フィルターを有効にして再起動すると現在の使用量を表示できます</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>拡張モードでは、Windows とともに起動するバックグラウンドサービスが追加されます。アプリを移動または削除する前に、このサービスを削除してください。</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>拡張モードは利用できません。保存待ちファイルを保存するには、安全なシャットダウンまたは安全な再起動を使用してください。</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>HKLM\SOFTWARE\… を入力し、Enter キーで移動</translation></message>
    <message><source>Enter a file name</source><translation>ファイル名を入力</translation></message>
    <message><source>Enter a valid file name.</source><translation>有効なファイル名を入力してください。</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>有効なファイル名またはフォルダー名を入力してください。</translation></message>
    <message><source>Enter a valid folder name.</source><translation>有効なフォルダー名を入力してください。</translation></message>
    <message><source>Error</source><translation>エラー</translation></message>
    <message><source>Error details</source><translation>エラーの詳細</translation></message>
    <message><source>Error details and suggestions</source><translation>エラーの詳細と対処方法</translation></message>
    <message><source>Expand</source><translation>展開</translation></message>
    <message><source>Export list</source><translation>一覧をエクスポート</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>ログを消去できませんでした：%1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>ログを消去できませんでした：予期しないエラー。</translation></message>
    <message><source>File name</source><translation>ファイル名</translation></message>
    <message><source>File or folder</source><translation>ファイルまたはフォルダー</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>ファイルの保存が始まったため、キャンセルできません。</translation></message>
    <message><source>File saving incomplete</source><translation>ファイルの保存が未完了</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>保存待ちファイルの処理を省略します。保護されたパスの未確定の変更は、再起動後に失われます。</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>保存待ちファイルの処理を省略します。保護されたパスの未確定の変更は、シャットダウン後に失われます。</translation></message>
    <message><source>File type</source><translation>ファイルの種類</translation></message>
    <message><source>Files and folders</source><translation>ファイルとフォルダー</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>ファイルとフォルダーを一緒に選択できます。</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>保存待ちのファイルは保存されません。</translation></message>
    <message><source>Filter is disabled</source><translation>フィルターは無効です</translation></message>
    <message><source>Fit columns to contents</source><translation>列幅を内容に合わせる</translation></message>
    <message><source>Folder address</source><translation>フォルダーのアドレス</translation></message>
    <message><source>Folder name</source><translation>フォルダー名</translation></message>
    <message><source>Folders</source><translation>フォルダー</translation></message>
    <message><source>Full configuration</source><translation>設定全体</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>今後このキーへの書き込みは UWF で保護されません。</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>今後このパスへの書き込みは UWF で保護されません。</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>グローバルなレジストリ除外 · サブキーを含む</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>親フォルダーへ移動するか、別の場所を選択してください。</translation></message>
    <message><source>Home</source><translation>ホーム</translation></message>
    <message><source>Import configuration</source><translation>設定をインポート</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>インポートは設定の下書きを更新します。変更をシステムに書き込むには「確認して適用」を使用してください。</translation></message>
    <message><source>Import results</source><translation>インポート結果</translation></message>
    <message><source>Imported</source><translation>インポート済み</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>レポートには、設定の下書きに対する各コマンドの処理結果が表示されます。行を選択すると、コマンドと結果の全文を確認できます。</translation></message>
    <message><source>Imported: %1</source><translation>インポート済み：%1</translation></message>
    <message><source>Information</source><translation>情報</translation></message>
    <message><source>Item details</source><translation>項目の詳細</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>保存待ち一覧が大きい場合、保存に時間がかかることがあります。</translation></message>
    <message><source>License and copyright</source><translation>ライセンスと著作権</translation></message>
    <message><source>Line</source><translation>行</translation></message>
    <message><source>Local disk (%1)</source><translation>ローカルディスク (%1)</translation></message>
    <message><source>Log level</source><translation>ログレベル</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>ログの読み取りに失敗しました。再試行するか、ログファイルへのアクセスを確認してください。</translation></message>
    <message><source>Maximize</source><translation>最大化</translation></message>
    <message><source>Message details</source><translation>メッセージの詳細</translation></message>
    <message><source>Minimize</source><translation>最小化</translation></message>
    <message><source>Module</source><translation>モジュール</translation></message>
    <message><source>More</source><translation>その他</translation></message>
    <message><source>Move</source><translation>移動</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>アプリがあるドライブを UWF で保護している場合は、まず再起動後もアプリのファイルが残ることを確認してください。保存待ちファイルを大量に保存すると、シャットダウンや再起動に時間がかかる場合があります。</translation></message>
    <message><source>Multiple files can be selected.</source><translation>複数のファイルを選択できます。</translation></message>
    <message><source>Name and location</source><translation>名前と場所</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>名前付きの値：%1 · この値のみ</translation></message>
    <message><source>Navigate to registry path</source><translation>レジストリのパスに移動</translation></message>
    <message><source>New folder</source><translation>新しいフォルダー</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>更新すると新しいレコードが表示されます。</translation></message>
    <message><source>Next-session configuration</source><translation>次回セッションの設定</translation></message>
    <message><source>No files to commit</source><translation>確定するファイルはありません</translation></message>
    <message><source>No logs yet</source><translation>ログはまだありません</translation></message>
    <message><source>No matching items</source><translation>一致する項目はありません</translation></message>
    <message><source>No matching logs</source><translation>一致するログはありません</translation></message>
    <message><source>No overlay files found</source><translation>オーバーレイファイルが見つかりません</translation></message>
    <message><source>No registry key selected</source><translation>レジストリキーが選択されていません</translation></message>
    <message><source>None</source><translation>なし</translation></message>
    <message><source>Not configured</source><translation>未設定</translation></message>
    <message><source>Not connected</source><translation>未接続</translation></message>
    <message><source>Not installed</source><translation>未インストール</translation></message>
    <message><source>Not written to the system</source><translation>システムに未反映</translation></message>
    <message><source>Off</source><translation>オフ</translation></message>
    <message><source>On</source><translation>オン</translation></message>
    <message><source>One uwfmgr command per line</source><translation>1 行に 1 つの uwfmgr コマンド</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>アプリは、標準ユーザーが変更できない固定のフォルダーに保存してください。推奨の場所：</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>保存待ちファイルのみ自動的に保存されます。保護されたパスのその他の未確定の変更は、再起動後に失われます。</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>保存待ちファイルのみ自動的に保存されます。保護されたパスのその他の未確定の変更は、シャットダウン後に失われます。</translation></message>
    <message><source>Open</source><translation>開く</translation></message>
    <message><source>Open folder</source><translation>フォルダーを開く</translation></message>
    <message><source>Operating system</source><translation>オペレーティングシステム</translation></message>
    <message><source>Operation and security</source><translation>動作とセキュリティ</translation></message>
    <message><source>Original command</source><translation>元のコマンド</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>オーバーレイが危険しきい値に達しました。変更を保存し、安全に再起動してください。</translation></message>
    <message><source>Overlay maximum size</source><translation>オーバーレイの最大サイズ</translation></message>
    <message><source>Overlay type</source><translation>オーバーレイの種類</translation></message>
    <message><source>Overlay usage</source><translation>オーバーレイの使用量</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>オーバーレイが警告しきい値に達しました。変更を保存し、安全に再起動してください。</translation></message>
    <message><source>Overview</source><translation>概要</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>コマンドを貼り付けるか、ファイルから読み込みます…</translation></message>
    <message><source>Pending changes</source><translation>保留中の変更</translation></message>
    <message><source>Pending removal</source><translation>削除予定</translation></message>
    <message><source>Permissions and protection</source><translation>権限と保護</translation></message>
    <message><source>Privileges</source><translation>特権</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2 項目を処理済み</translation></message>
    <message><source>Processing result</source><translation>処理結果</translation></message>
    <message><source>Protected</source><translation>保護中</translation></message>
    <message><source>Protected volumes</source><translation>保護されたボリューム</translation></message>
    <message><source>Protection and overlay</source><translation>保護とオーバーレイ</translation></message>
    <message><source>Quick access</source><translation>クイックアクセス</translation></message>
    <message><source>Raw report</source><translation>生のレポート</translation></message>
    <message><source>Read again</source><translation>再読み取り</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>再読み取りで診断スナップショットを取得してください。</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>設定を読み取ると現在の使用量を表示できます</translation></message>
    <message><source>Read-only</source><translation>読み取り専用</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>大きなオーバーレイの読み取りには時間がかかる場合があります。いつでもキャンセルできます。</translation></message>
    <message><source>Reading canceled</source><translation>読み取りをキャンセルしました</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>この場所のファイルとサブフォルダーを読み取っています…</translation></message>
    <message><source>Reading folder</source><translation>フォルダーを読み取り中</translation></message>
    <message><source>Reading logs</source><translation>ログを読み取り中</translation></message>
    <message><source>Reading overlay</source><translation>オーバーレイを読み取り中</translation></message>
    <message><source>Ready</source><translation>準備完了</translation></message>
    <message><source>Ready to save</source><translation>保存の準備完了</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>読み取りが完了すると、今回の実行のレコードが表示されます。</translation></message>
    <message><source>Recursive staging</source><translation>再帰的な保存待ち</translation></message>
    <message><source>Refresh folder</source><translation>フォルダーを更新</translation></message>
    <message><source>Registry key or persistence setting</source><translation>レジストリキーまたは永続化設定</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>現在の対象の処理が終わると、残りのタスクをキャンセルできます。</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>このキーを次回セッションの除外から削除します。</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>このパスを次回セッションの除外から削除します。</translation></message>
    <message><source>Removed</source><translation>削除済み</translation></message>
    <message><source>Rename</source><translation>名前の変更</translation></message>
    <message><source>Rename…</source><translation>名前の変更…</translation></message>
    <message><source>Repair required</source><translation>修復が必要</translation></message>
    <message><source>Replace file</source><translation>ファイルを置き換え</translation></message>
    <message><source>Replace the existing file?</source><translation>既存のファイルを置き換えますか？</translation></message>
    <message><source>Required configuration</source><translation>必須の設定</translation></message>
    <message><source>Restart anyway</source><translation>そのまま再起動</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>保存後に再起動を続けます。アプリを閉じないでください。</translation></message>
    <message><source>Restore</source><translation>元に戻す</translation></message>
    <message><source>Retry</source><translation>再試行</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>シャットダウンを続ける前に、失敗の詳細を確認してください。</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>再起動する前に、失敗の詳細を確認してください。</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>操作の詳細を確認してください。既に書き込まれた可能性がある変更を再試行する前に、現在のシステム状態を読み取ってください。</translation></message>
    <message><source>Review the result for each target.</source><translation>各対象の結果を確認してください。</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>システムに書き込む前に、これらの変更を確認してください。</translation></message>
    <message><source>Running</source><translation>実行中</translation></message>
    <message><source>Save</source><translation>保存</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>保存待ちファイルをディスクに保存してから再起動します。</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>保存待ちファイルをディスクに保存してからシャットダウンします。</translation></message>
    <message><source>Saving staged files</source><translation>保存待ちファイルを保存中</translation></message>
    <message><source>Saving to disk</source><translation>ディスクに保存中</translation></message>
    <message><source>Saving…</source><translation>保存中…</translation></message>
    <message><source>Scanning staged paths</source><translation>保存待ちパスをスキャン中</translation></message>
    <message><source>Search logs</source><translation>ログを検索</translation></message>
    <message><source>Search messages or modules</source><translation>メッセージまたはモジュールを検索</translation></message>
    <message><source>Search paths</source><translation>パスを検索</translation></message>
    <message><source>Search this folder</source><translation>このフォルダーを検索</translation></message>
    <message><source>Select a file or enter its name</source><translation>ファイルを選択するか名前を入力</translation></message>
    <message><source>Select a file to open.</source><translation>開くファイルを選択してください。</translation></message>
    <message><source>Select a folder.</source><translation>フォルダーを選択してください。</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>キーを選ぶとサブツリー全体を操作し、名前付きの値を選ぶとその値のみを操作します。</translation></message>
    <message><source>Select an available subkey</source><translation>利用可能なサブキーを選択</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>ファイルのみを選択するか、フォルダーを開いて内容を参照してください。</translation></message>
    <message><source>Select files or folders</source><translation>ファイルまたはフォルダーを選択</translation></message>
    <message><source>Select folder</source><translation>フォルダーを選択</translation></message>
    <message><source>Select folders only.</source><translation>フォルダーのみ選択してください。</translation></message>
    <message><source>Select registry key or value</source><translation>レジストリキーまたは値を選択</translation></message>
    <message><source>Select registry key or value to delete</source><translation>削除するレジストリキーまたは値を選択</translation></message>
    <message><source>Select the current folder</source><translation>現在のフォルダーを選択</translation></message>
    <message><source>Select this file</source><translation>このファイルを選択</translation></message>
    <message><source>Select this registry key</source><translation>このレジストリキーを選択</translation></message>
    <message><source>Select this registry value</source><translation>このレジストリ値を選択</translation></message>
    <message><source>Selected file</source><translation>選択したファイル</translation></message>
    <message><source>Selected folder</source><translation>選択したフォルダー</translation></message>
    <message><source>Selection</source><translation>選択範囲</translation></message>
    <message><source>Service account</source><translation>サービスアカウント</translation></message>
    <message><source>Service configuration needs repair</source><translation>サービス設定の修復が必要です</translation></message>
    <message><source>Service details</source><translation>サービスの詳細</translation></message>
    <message><source>Service name</source><translation>サービス名</translation></message>
    <message><source>Service status</source><translation>サービスの状態</translation></message>
    <message><source>Service stopped</source><translation>サービス停止中</translation></message>
    <message><source>Shut down anyway</source><translation>そのままシャットダウン</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>保存後にシャットダウンを続けます。アプリを閉じないでください。</translation></message>
    <message><source>Size</source><translation>サイズ</translation></message>
    <message><source>Some changes could not be applied</source><translation>一部の変更を適用できませんでした</translation></message>
    <message><source>Some files could not be saved</source><translation>一部のファイルを保存できませんでした</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>一部のハードウェア情報を読み取れませんでした。他の診断情報は引き続き表示・コピーできます。</translation></message>
    <message><source>Source code</source><translation>ソースコード</translation></message>
    <message><source>Staged file</source><translation>保存待ちファイル</translation></message>
    <message><source>Staged path</source><translation>保存待ちパス</translation></message>
    <message><source>Standard user</source><translation>標準ユーザー</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Windows と同時に自動起動</translation></message>
    <message><source>Startup type</source><translation>スタートアップの種類</translation></message>
    <message><source>Status unavailable</source><translation>状態を取得できません</translation></message>
    <message><source>Stopped</source><translation>停止中</translation></message>
    <message><source>Storage</source><translation>保存先</translation></message>
    <message><source>System</source><translation>システム</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>UWF を初めて有効にしたときのシステム変更</translation></message>
    <message><source>System memory</source><translation>システムメモリ</translation></message>
    <message><source>System metadata</source><translation>システムメタデータ</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>システムメタデータは診断専用で、確定できません。</translation></message>
    <message><source>Takes effect after restart</source><translation>再起動後に有効</translation></message>
    <message><source>Target</source><translation>対象</translation></message>
    <message><source>Terminal Services license persistence</source><translation>ターミナルサービスライセンスの永続化</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>UWF プロバイダーが列挙を完了できませんでした。オーバーレイが大きい場合やシステムリソースが不足している場合に発生することがあります。</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>設定をシステムに書き込みました。ほとんどの変更は再起動後に有効になります。</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>設定は次回セッションの目標と一致しています。「設定全体」でコマンドを表示・エクスポートできます。</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>現在のログレコードを消去します。元には戻せません。</translation></message>
    <message><source>The destination folder does not exist.</source><translation>保存先フォルダーが存在しません。</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>拡張モードのサービス状態を読み取れませんでした。サービスの権限を確認して再試行してください。</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>一覧には欠落や確定済みのファイルが含まれる場合があります。確定対象の一覧として使用しないでください。</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>選択した項目は既に存在しません：%1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>サービスはインストール済みですが停止しています。自動保存を復旧するには起動してください。</translation></message>
    <message><source>This PC</source><translation>PC</translation></message>
    <message><source>This folder is empty</source><translation>このフォルダーは空です</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>このレポートにはアカウント名、デバイス識別子、ファイルパスが含まれる場合があります。共有する前に確認してください。</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>この UTF-8 CMD スクリプトを .cmd ファイルとして保存し、コマンド プロンプトで実行してください。エクスポートでも同じスクリプトを使用します。</translation></message>
    <message><source>Try another keyword or log level.</source><translation>別のキーワードまたはログレベルを試してください。</translation></message>
    <message><source>Try another search term.</source><translation>別の検索語を試してください。</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>フィルターが有効な間は種類と最大サイズを変更できません。フィルターを無効にし、先に再起動してください。</translation></message>
    <message><source>UI agent</source><translation>UI エージェント</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 行</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>UWF 設定を読み取れませんでした。Windows に Unified Write Filter がインストールされていることを確認し、再読み取りしてください。</translation></message>
    <message><source>UWF feature</source><translation>UWF 機能</translation></message>
    <message><source>UWF filter</source><translation>UWF フィルター</translation></message>
    <message><source>UWF is currently unavailable</source><translation>現在 UWF は利用できません</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>UWF の安全な再起動を直接実行します。</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>UWF の安全なシャットダウンを直接実行します。</translation></message>
    <message><source>Unable to access this folder</source><translation>このフォルダーにアクセスできません</translation></message>
    <message><source>Unable to access this registry key</source><translation>このレジストリキーにアクセスできません</translation></message>
    <message><source>Unable to read logs</source><translation>ログを読み取れません</translation></message>
    <message><source>Unable to read service status</source><translation>サービスの状態を読み取れません</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>現在オーバーレイを読み取れません</translation></message>
    <message><source>Unable to read this folder.</source><translation>このフォルダーを読み取れません。</translation></message>
    <message><source>Unable to select this item</source><translation>この項目は選択できません</translation></message>
    <message><source>Unavailable</source><translation>利用不可</translation></message>
    <message><source>Undo removal</source><translation>削除を取り消す</translation></message>
    <message><source>Unprotected</source><translation>未保護</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>失敗した項目は保存されていません。行を選択すると完全なパスと理由を確認できます。</translation></message>
    <message><source>Up one folder</source><translation>親フォルダーへ</translation></message>
    <message><source>Usage unavailable</source><translation>使用量を取得できません</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>すべてのサブキーを含むキー全体を使用</translation></message>
    <message><source>Version %1</source><translation>バージョン %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>アプリの記録と診断情報を表示します。</translation></message>
    <message><source>View commands for these changes</source><translation>これらの変更のコマンドを表示</translation></message>
    <message><source>View failure details</source><translation>失敗の詳細を表示</translation></message>
    <message><source>View overlay</source><translation>オーバーレイを表示</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>%1 のファイル診断スナップショットを表示します。</translation></message>
    <message><source>Volume %1 binding</source><translation>ボリューム %1 の関連付け</translation></message>
    <message><source>Volume %1 protection</source><translation>ボリューム %1 の保護</translation></message>
    <message><source>Volume identity</source><translation>ボリュームの識別情報</translation></message>
    <message><source>Warning</source><translation>警告</translation></message>
    <message><source>Warning %1 MB</source><translation>警告 %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>キー全体とすべてのサブキー · 再帰的</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>書き込み保護は Windows の UWF が提供し、このアプリで設定・管理します。</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>拡張モードを使わない場合は、このアプリの安全なシャットダウンまたは安全な再起動で保存待ちファイルを保存してください。</translation></message>
    <message><source>Write protection</source><translation>書き込み保護</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>これらの設定変更をシステムに書き込みますか？ほとんどの変更は再起動後に有効になります。</translation></message>
    <message><source>staged files</source><translation>保存待ちファイル</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr コマンド</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>選択したパスを削除予定としてマークします。適用後に反映されます。</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>適用後に自動ステージングの一覧から削除します。</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>適用後に自動ステージングの一覧へ追加します。</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>この計画を適用すると、保存済みのステージング一覧が空になり、不要になったレジストリ除外も削除されます。</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>ファイルステージングの変更は UWF Manager で適用する必要があり、uwfmgr コマンドとしてエクスポートできません。</translation></message>
    <message><source>File staging is saved after Apply</source><translation>ファイルステージングは適用後に保存されます</translation></message>
    <message><source>Overlay snapshot scope</source><translation>オーバーレイスナップショットの範囲</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>この一覧は診断専用です。コミットが必要なファイルの判断には使えません。</translation></message>
    <message><source>NTFS volumes only</source><translation>NTFS ボリュームのみ対応</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>オーバーレイが大きい場合、読み取りに時間がかかったり、リソース不足で失敗したりすることがあります。</translation></message>
    <message><source>Small files may be missing</source><translation>小さいファイルが欠落する場合があります</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>ディスクのクラスターサイズ（通常 4 KB）未満のファイルは一覧に表示されない場合があります。</translation></message>
    <message><source>Does not indicate pending commits</source><translation>未コミットの状態を示すものではありません</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>コミット済みのファイル、除外パス、他の操作の影響を受けた項目も表示されることがあります。</translation></message>

    <message><source>Command processing result</source><translation>コマンド処理結果</translation></message>
    <message><source>Copy command</source><translation>コマンドをコピー</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 個の分岐を読み取れませんでした</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 個の分岐を読み取れませんでした。検出済みの内容で続行するか、失敗した項目のみ再試行してください。</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 個のファイル · %2 個のフォルダー</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 個の値 · %2 個のキー</translation></message>
    <message><source>Attempts</source><translation>試行回数</translation></message>
    <message><source>Cancel remaining tasks</source><translation>残りのタスクをキャンセル</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>走査できた内容のみコミットします。読み取れなかった分岐は変更しません。</translation></message>
    <message><source>Committing changes</source><translation>変更をコミット中</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>完了した削除とコミットは元に戻せません。</translation></message>
    <message><source>Continue…</source><translation>続行…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>検出済みの対象のみ削除してコミットします。読み取れなかった分岐とその親フォルダーやキーは保持します。</translation></message>
    <message><source>Deleting and committing</source><translation>削除してコミット中</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>削除とコミットは元に戻せません。選択範囲のみ処理し、保留中の UWF 設定は適用しません。</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>操作範囲を確認中です。読み取れない分岐を記録し、他の内容の走査を続けます。</translation></message>
    <message><source>Export traversal records</source><translation>走査記録をエクスポート</translation></message>
    <message><source>No actionable content</source><translation>操作できる内容がありません</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>操作できる対象は見つかりませんでした。削除やコミットは実行していません。</translation></message>
    <message><source>No changes have been committed.</source><translation>変更はまだコミットされていません</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>削除やコミットはまだ実行していません</translation></message>
    <message><source>Path</source><translation>パス</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>選択範囲を完全に削除し、削除結果を保護ボリュームに保存します。</translation></message>
    <message><source>Preparation complete</source><translation>準備完了</translation></message>
    <message><source>Preparing commit</source><translation>コミットの準備中</translation></message>
    <message><source>Preparing deletion and commit</source><translation>削除とコミットの準備中</translation></message>
    <message><source>Process only successfully traversed content</source><translation>走査できた内容のみ処理</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>選択範囲を超えて走査しないよう、再解析ポイントをスキップします。</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>コミットは再起動しても元に戻せません。選択範囲のみ処理し、保留中の UWF 設定は適用しません。</translation></message>
    <message><source>Retry failed items</source><translation>失敗した項目を再試行</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>選択範囲の変更を保護ボリュームに保存します。</translation></message>
    <message><source>Stopping the operation</source><translation>操作を停止中</translation></message>
    <message><source>Successful and failed records are retained</source><translation>成功と失敗の記録を保持しています</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>フォルダーが空ではありません。未処理の内容は保持されました。</translation></message>
    <message><source>The operation could not be completed</source><translation>操作を完了できませんでした</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>レジストリキーに新しい値があります。未処理の内容は保持されました。</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>レジストリキーにサブキーが残っています。未処理の内容は保持されました。</translation></message>
    <message><source>The target no longer exists.</source><translation>対象は既に存在しません。</translation></message>
    <message><source>Traversal complete</source><translation>走査完了</translation></message>
    <message><source>Traversal completed with unread content</source><translation>走査完了、一部の内容を読み取れませんでした</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>走査記録は保持されており、エクスポートできます。</translation></message>
    <message><source>Traverse again</source><translation>再走査</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>再走査するか、このウィンドウを閉じてください。</translation></message>
    <message><source>Traversing the folder</source><translation>フォルダーを走査中</translation></message>
    <message><source>Traversing the registry</source><translation>レジストリを走査中</translation></message>
    <message><source>Unable to prepare the operation</source><translation>操作を準備できません</translation></message>
    <message><source>View results</source><translation>結果を表示</translation></message>

    <message><source>All changes committed</source><translation>すべての変更をコミットしました</translation></message>
    <message><source>Some changes committed</source><translation>一部の変更をコミットしました</translation></message>
    <message><source>No changes could be committed</source><translation>変更をコミットできませんでした</translation></message>
    <message><source>No changes to commit</source><translation>コミットする変更はありません</translation></message>
    <message><source>Commit interrupted</source><translation>コミットが中断されました</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 件は処理されていません。</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 件のレジストリ操作が失敗しました。</translation></message>
    <message><source>The folder does not exist.</source><translation>フォルダーが存在しません。</translation></message>
    <message><source>Access denied.</source><translation>アクセスが拒否されました。</translation></message>
    <message><source>An item with this name already exists.</source><translation>同じ名前の項目が既に存在します。</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>フォルダーを作成できませんでした。名前とアクセス権を確認してください。</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>項目の名前を変更できませんでした。名前とアクセス権を確認してください。</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>このブラウザーでは名前の変更に対応していません。</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>このプランを開いた後にファイルステージングの下書きが変更されました。新しいプランを確認してください。</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>このプランを開いた後に保存済みのファイルステージング一覧が変更されました。更新して変更内容を確認してください。</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>保存済みのファイルステージング一覧が承認したプランと一致しません。</translation></message>
    <message><source>Export configuration</source><translation>設定をエクスポート</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>現在のテーマ、言語、HUB の表示状態を、このアプリケーションと同じフォルダーの UWF.ini に保存します。</translation></message>
    <message><source>Configuration saved</source><translation>設定を保存しました</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>現在のインターフェイス設定を次の場所に保存しました:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini は既に存在します。既存のファイルは変更されていません:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>UWF.ini を保存できませんでした:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>デスクトップ</translation></message>
    <message><source>Downloads</source><translation>ダウンロード</translation></message>
    <message><source>Documents</source><translation>ドキュメント</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>ファイルの保存待ち処理が中止されました。エラーを解決してから再試行してください。</translation></message>
    <message><source>No net change for this setting</source><translation>この設定の最終値はインポート前と変わりません</translation></message>
    <message><source>Final draft %1 → %2</source><translation>下書きの最終値: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>下書きの最終値: %1 → %2（要求値: %3）</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>要求値: %1 → %2。後のコマンドで置き換えられました。下書きの最終値 → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>%1 の下書きを更新しました</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>ボリューム %1 の保護設定の下書きを更新しました → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>削除する項目を選択してください。</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>選択した項目は現在復元できません。</translation></message>
</context>
</TS>
