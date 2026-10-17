<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ko" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>사용</translation></message>
    <message><source>Disable</source><translation>사용 안 함</translation></message>
    <message><source>enable</source><translation>사용</translation></message>
    <message><source>disable</source><translation>사용 안 함</translation></message>
    <message><source>Enabled</source><translation>사용 중</translation></message>
    <message><source>Disabled</source><translation>사용 안 함</translation></message>
    <message><source>OK</source><translation>확인</translation></message>
    <message><source>Cancel</source><translation>취소</translation></message>
    <message><source>Close</source><translation>닫기</translation></message>
    <message><source>Refresh</source><translation>새로 고침</translation></message>
    <message><source>Apply</source><translation>적용</translation></message>
    <message><source>Add</source><translation>추가</translation></message>
    <message><source>Remove</source><translation>제거</translation></message>
    <message><source>Yes</source><translation>예</translation></message>
    <message><source>No</source><translation>아니요</translation></message>
    <message><source>Excluded</source><translation>제외됨</translation></message>
    <message><source>Not excluded</source><translation>제외 안 됨</translation></message>
    <message><source>File…</source><translation>파일…</translation></message>
    <message><source>Folder…</source><translation>폴더…</translation></message>
    <message><source>volume ID</source><translation>볼륨 ID</translation></message>
    <message><source>drive letter</source><translation>드라이브 문자</translation></message>
    <message><source>Drive letter</source><translation>드라이브 문자</translation></message>
    <message><source>Volume ID</source><translation>볼륨 ID</translation></message>
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>현재 시스템 "%1"(%2)은 지원되는 Windows 10/11 버전으로 인식되지 않습니다. 호환 모드로 실행되며 일부 기능을 사용할 수 없습니다.</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>현재 세션의 볼륨 보호 상태입니다(읽기 전용).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>다음 세션에서 이 볼륨을 보호합니다. 쓰기는 오버레이로 리디렉션되며 다시 시작하면 삭제됩니다.</translation></message>
    <message><source>Protection:</source><translation>보호:</translation></message>
    <message><source>Current session</source><translation>현재 세션</translation></message>
    <message><source>Next session</source><translation>다음 세션</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>현재 적용된 세션입니다(읽기 전용). 변경 사항은 이 세션에 적용되지 않습니다.</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>시스템을 다시 시작한 후 적용되는 세션입니다. 변경 사항은 다시 시작 후 적용됩니다.</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>볼륨 식별 방식입니다. 드라이브 문자는 간단하지만 디스크 추가·제거로 문자가 바뀌면 연결이 끊어집니다. 볼륨 ID는 문자가 바뀌어도 유지됩니다.</translation></message>
    <message><source>Bind by:</source><translation>연결 기준:</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF에서 지원하지 않는 볼륨: </translation></message>
    <message><source>Global settings</source><translation>전역 설정</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>현재 세션의 UWF 필터 상태입니다(읽기 전용).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>다음 세션에서 UWF 필터를 사용합니다. 보호된 볼륨의 쓰기는 오버레이로 리디렉션되며 다시 시작 시 삭제됩니다.</translation></message>
    <message><source>Filter</source><translation>필터</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>유형과 최대 크기를 바꾸려면:
1. 위 스위치로 필터를 끕니다.
2. 다시 시작하여 필터를 해제합니다.
3. 설정을 변경합니다.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>오버레이 저장 위치입니다. RAM은 빠르지만 메모리를 사용하고 Disk는 시스템 드라이브를 사용해 용량이 더 큽니다. 둘 다 다시 시작 시 삭제됩니다.</translation></message>
    <message><source>Type</source><translation>유형</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>오버레이 최대 용량입니다. RAM은 시스템 전체 메모리로 제한됩니다. Disk는 최소 1024 MB와 시스템 볼륨의 충분한 여유 공간이 필요합니다.</translation></message>
    <message><source>Maximum size</source><translation>최대 크기</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>최대 크기 · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>경고 임계값</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>사용량이 이 값에 도달하면 이벤트 로그에 경고를 기록합니다. 위험 임계값보다 낮아야 합니다. 0은 이 이벤트를 끕니다.</translation></message>
    <message><source>Critical threshold</source><translation>위험 임계값</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>사용량이 이 값에 도달하면 이벤트 로그에 위험 이벤트를 기록합니다. 경고 임계값보다 높아야 합니다. 0은 이 이벤트를 끕니다.</translation></message>
    <message><source>Used / total</source><translation>사용량 / 전체</translation></message>
    <message><source>Overlay</source><translation>오버레이</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; 사용 중 &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; 경고 &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; 위험</translation></message>
    <message><source>UWF status unavailable: </source><translation>UWF 상태를 확인할 수 없음: </translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>UWF 설정을 변경하려면 관리자 권한이 필요합니다. 마우스 오른쪽 버튼 → "관리자 권한으로 실행"으로 다시 실행하세요.</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>지원하지 않는 드라이브 유형입니다(로컬 고정 디스크만 지원).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>볼륨 용량이 UWF의 보호 볼륨당 한도인 16 TiB를 초과합니다.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: 볼륨 보호는 가능하지만 파일 제외와 파일별 커밋은 지원하지 않습니다.</translation></message>
    <message><source>Failed to read volume information.</source><translation>실패: 볼륨 정보 읽기.</translation></message>
    <message><source>%1 free / %2</source><translation>여유 %1 / %2</translation></message>
    <message><source>Commit</source><translation>커밋</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>파일의 오버레이 변경을 디스크에 커밋 / 레지스트리 변경 커밋. 이 작업은 되돌릴 수 없습니다.</translation></message>
    <message><source>Commit file changes…</source><translation>파일 변경 커밋…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>디스크에 커밋할 파일 선택. 파일의 오버레이 변경을 디스크에 커밋.</translation></message>
    <message><source>Commit folder changes…</source><translation>폴더 변경 커밋…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>디스크에 커밋할 폴더 선택. 폴더의 오버레이 변경을 디스크에 커밋. 모든 하위 폴더를 포함합니다.</translation></message>
    <message><source>Delete and commit file…</source><translation>파일을 삭제하고 삭제를 디스크에 커밋…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>삭제를 커밋할 파일 선택. 파일을 삭제하고 삭제를 디스크에 커밋.</translation></message>
    <message><source>Delete and commit folder…</source><translation>폴더와 모든 내용을 삭제하고 삭제를 디스크에 커밋…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>삭제를 커밋할 폴더 선택. 폴더와 모든 내용을 삭제하고 삭제를 디스크에 커밋. 모든 하위 폴더를 포함합니다.</translation></message>
    <message><source>Commit registry changes…</source><translation>레지스트리 변경 커밋…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>키 경로와 값 이름(선택)을 입력해 변경을 커밋합니다. 값 이름을 비우면 키와 모든 하위 키의 모든 값을 커밋합니다.</translation></message>
    <message><source>Delete and commit registry…</source><translation>레지스트리 삭제 커밋…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>삭제할 키 경로와 값 이름(선택)을 입력하고 삭제를 커밋합니다. 값 이름을 비우면 키와 모든 값·하위 키를 재귀적으로 삭제합니다.</translation></message>
    <message><source>File exclusions</source><translation>파일 제외</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>UWF 보호에서 제외되는 파일과 폴더입니다. 항목을 두 번 클릭하면 경로를 복사합니다.</translation></message>
    <message><source>Registry exclusions</source><translation>레지스트리 제외</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>레지스트리 제외는 볼륨 보호 상태와 별개인 전역 설정으로 UWF 필터 스위치를 따릅니다. 모든 볼륨이 목록을 공유하며 디스크 탭에 한 번만 표시됩니다. 두 번 클릭하면 경로를 복사합니다.</translation></message>
    <message><source>File staging</source><translation>파일 자동 커밋</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>파일 / 폴더: 안전 종료 또는 다시 시작 전에 자동으로 커밋됩니다. 모든 하위 폴더를 포함합니다. 목록에 항목이 있으면 다음 레지스트리 제외로 보호된 세션 간에도 목록을 유지합니다: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Search staged paths…</source><translation>자동 커밋 경로 검색…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>파일 또는 폴더를 자동 커밋 목록에 추가합니다.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>디스크에 커밋할 파일 선택. 안전 종료 또는 다시 시작 전에 자동으로 커밋됩니다.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>디스크에 커밋할 폴더 선택. 안전 종료 또는 다시 시작 전에 자동으로 커밋됩니다. 모든 하위 폴더를 포함합니다.</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>선택한 경로를 자동 커밋 목록에서 즉시 제거합니다.</translation></message>
    <message><source>Select files for automatic commit</source><translation>디스크에 커밋할 파일 선택 · 파일 자동 커밋</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>디스크에 커밋할 폴더 선택 · 파일 자동 커밋 · 모든 하위 폴더를 포함합니다.</translation></message>
    <message><source>Cannot stage this path</source><translation>이 경로를 자동 커밋에 추가할 수 없음</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>UWF 파일 제외와 겹칩니다. 다음 제외 항목을 먼저 제거하세요:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>파일 자동 커밋 항목과 겹칩니다. 다음 경로를 먼저 제거한 후 제외를 추가하세요:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>기존 파일 자동 커밋 항목과 경로가 겹칩니다</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>파일 자동 커밋 목록에 저장된 항목이 있으면 이 레지스트리 제외가 필요합니다</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>경로가 없거나 예상한 파일 유형이 아닙니다:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>경로를 검사할 수 없습니다:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>재분석 지점은 자동 커밋에 추가할 수 없습니다:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>드라이브 문자가 없어 UWF로 커밋할 수 없습니다:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>볼륨 %1의 경로입니다. 해당 볼륨의 파일 자동 커밋 탭에서 추가하세요.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>볼륨 루트는 추가할 수 없습니다. 전체 볼륨이 재귀적으로 커밋되기 때문입니다.</translation></message>
    <message><source>File staging could not be saved</source><translation>실패: 파일 자동 커밋 목록 저장</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>갱신을 완료할 수 없어 파일 자동 커밋 목록을 다시 불러왔습니다:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>파일 자동 커밋: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>안전 종료 또는 다시 시작 전에 자동으로 커밋됩니다.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>파일 자동 커밋: %1 · 파일: %2 · 폴더: %3</translation></message>
    <message><source>Required by UWF for File staging.</source><translation>UWF 파일 자동 커밋에 필요합니다.</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>UWF 필터가 꺼져 오버레이 변경이 누적되지 않았으므로 커밋할 내용이 없습니다.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>%1 파일 시스템은 파일별 커밋을 지원하지 않습니다.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>이 볼륨은 UWF에서 지원하지 않습니다.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>이 볼륨에서는 파일 커밋을 사용할 수 없습니다. 레지스트리 커밋만 가능합니다(레지스트리 제외는 전역 설정).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>%1 파일 시스템은 파일별 커밋을 지원하지 않습니다. 레지스트리 커밋만 가능합니다(레지스트리 제외는 전역 설정).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>현재 UWF로 보호되지 않는 볼륨이므로 커밋할 파일 변경이 없습니다.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>현재 UWF로 보호되지 않는 볼륨이므로 커밋할 파일 변경이 없습니다. 레지스트리 커밋만 가능합니다(레지스트리 제외는 전역 설정).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>디스크에 커밋할 파일 선택</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>디스크에 커밋할 폴더 선택</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>삭제를 커밋할 파일 선택</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>삭제를 커밋할 폴더 선택</translation></message>
    <message><source>Commit registry changes</source><translation>레지스트리 변경 커밋</translation></message>
    <message><source>Commit registry deletion</source><translation>레지스트리 삭제 커밋</translation></message>
    <message><source>Key path:</source><translation>키 · 경로:</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>HKLM\Software\... 입력 또는 붙여넣기 후 Enter로 이동</translation></message>
    <message><source>Name</source><translation>이름</translation></message>
    <message><source>Data</source><translation>데이터</translation></message>
    <message><source>Key: %1</source><translation>키: %1</translation></message>
    <message><source>Value: %1</source><translation>값: %1</translation></message>
    <message><source>(none)</source><translation>(없음)</translation></message>
    <message><source>(Default)</source><translation>(기본값)</translation></message>
    <message><source>(value not set)</source><translation>(값 설정 안 됨)</translation></message>
    <message><source>(empty)</source><translation>(비어 있음)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>알 수 없음 (값: 파일이 너무 큼)</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>하위 키를 포함한 전체 키</translation></message>
    <message><source>Overlay files - %1</source><translation>오버레이 · 파일 - %1</translation></message>
    <message><source>View overlay files</source><translation>오버레이 파일 보기</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>오버레이 파일 보기. &lt;b&gt;진단용 스냅샷입니다.&lt;/b&gt; 커밋 대상을 결정하는 데 사용하지 마세요.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>오버레이 파일 목록은 NTFS 볼륨에서만 지원합니다.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF 필터가 꺼져 검사할 오버레이가 없습니다.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>UWF로 보호되지 않는 볼륨이므로 검사할 오버레이가 없습니다.</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;진단용 스냅샷입니다.&lt;/b&gt; 커밋 대상을 결정하는 데 사용하지 마세요.</translation></message>
    <message><source>NTFS volumes only.</source><translation>오버레이 파일 목록은 NTFS 볼륨에서만 지원합니다.</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>오버레이가 클수록 메모리와 시간이 증가해 매우 느리거나 실패할 수 있습니다.</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>목록은 정확하지 않습니다. 클러스터 크기(보통 4 KB)보다 작은 파일은 빠질 수 있고 이전 커밋, 제외 경로, 관련 없는 작업의 영향으로 파일이 표시될 수 있습니다.</translation></message>
    <message><source>Loading…</source><translation>불러오는 중…</translation></message>
    <message><source>Failed: %1</source><translation>실패: %1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>오버레이 · 파일: %1</translation></message>
    <message><source>No files</source><translation>파일 없음</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>페이지 %1 / %2 · 파일: %3</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>파일 열거 중 WMI 공급자가 충돌했습니다. 오버레이가 크거나 I/O 부하가 높을 때 발생하는 UWF_Overlay.GetOverlayFiles의 알려진 불안정성입니다.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>메모리가 부족하거나 공급자가 작업을 지원하지 않습니다. NTFS와 충분한 여유 메모리가 필요합니다. 오버레이를 줄이거나 메모리 사용이 큰 앱을 닫고 다시 시도하세요.</translation></message>
    <message><source>Raw error: %1</source><translation>오류 코드: %1</translation></message>
    <message><source>Export to file…</source><translation>파일로 내보내기…</translation></message>
    <message><source>Export overlay file list</source><translation>파일로 내보내기… · 오버레이 · 파일</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>텍스트 파일 (*.txt);;모든 파일 (*)</translation></message>
    <message><source>Export failed</source><translation>내보내기 실패</translation></message>
    <message><source>Export finished</source><translation>내보내기 완료</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>실패: 파일 열기 (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>실패: 파일 쓰기 (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>구성: 파일 쓰기 · 항목: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>볼륨 전체(%1)는 제외할 수 없습니다.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>페이지·스왑·최대 절전 모드 파일은 UWF 자체가 의존하므로 제외할 수 없습니다.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>전체 디렉터리는 제외할 수 없습니다: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>전체 디렉터리는 제외할 수 없습니다: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>전체 디렉터리는 제외할 수 없습니다: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>중요 시스템 파일은 제외할 수 없습니다: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>사용자별 레지스트리 파일 NTUSER.DAT는 제외할 수 없습니다.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>레지스트리 키는 비워 둘 수 없습니다.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>레지스트리 키는 역슬래시로 시작할 수 없습니다.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>연속된 역슬래시가 있어 유효하지 않은 경로입니다.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>레지스트리 경로 구분자는 `\`입니다. `/`를 사용하지 마세요.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>보이지 않는 제어 문자가 있어 유효하지 않은 경로입니다.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>도메인 컴퓨터 계정 비밀 정보는 UWF 문서에서 제외를 명시적으로 금지합니다: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>다음 최상위 키 아래의 구체적인 하위 키만 제외할 수 있습니다:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Search file paths…</source><translation>파일 경로 검색…</translation></message>
    <message><source>Search registry keys…</source><translation>레지스트리 키 검색…</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>파일 또는 폴더를 제외 목록에 추가합니다. 제외 항목은 UWF로 보호되지 않습니다.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>추가할 파일 선택 · 파일 제외.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>추가할 폴더 선택 · 파일 제외. 모든 하위 폴더를 포함합니다.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>제외할 레지스트리 키 경로를 입력하세요.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>제외할 레지스트리 키 경로를 입력하세요. / 영구 저장.</translation></message>
    <message><source>Registry key…</source><translation>레지스트리 · 키…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>도메인 비밀 키(DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>터미널 서비스 클라이언트 액세스 라이선스(TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>도메인 비밀 키(컴퓨터 계정 암호)를 UWF 세션 간에 유지합니다.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>터미널 서비스 클라이언트 액세스 라이선스를 UWF 세션 간에 유지합니다.</translation></message>
    <message><source>· %1 persistence %2</source><translation>· 영구 저장 %1: %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ 영구 저장 %1: %2</translation></message>
    <message><source>Remove selected</source><translation>선택 항목 제거</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>선택한 제외 항목을 제거합니다. 적용 후 반영됩니다.</translation></message>
    <message><source>Open containing folder</source><translation>포함 폴더 열기</translation></message>
    <message><source>Copy file path</source><translation>파일 경로 복사</translation></message>
    <message><source>Copy registry path</source><translation>레지스트리 경로 복사</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>폴더의 오버레이 변경을 디스크에 커밋…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>파일의 오버레이 변경을 디스크에 커밋…</translation></message>
    <message><source>Copied to clipboard: </source><translation>클립보드에 복사됨: </translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>추가할 파일 선택 · 파일 제외. 여러 파일을 선택할 수 있습니다.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>추가할 폴더 선택 · 파일 제외</translation></message>
    <message><source>Add registry exclusion</source><translation>레지스트리 제외 추가</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>경로 %1이 볼륨 %2에 없어 이 볼륨의 제외 항목으로 추가할 수 없습니다.</translation></message>
    <message><source>Path is not on this volume</source><translation>이 볼륨의 경로가 아님</translation></message>
    <message><source>Cannot add this exclusion</source><translation>제외 항목을 추가할 수 없음</translation></message>
    <message><source>File: %1</source><translation>파일… %1</translation></message>
    <message><source>Folder: %1</source><translation>폴더… %1</translation></message>
    <message><source>Registry: %1</source><translation>레지스트리: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>기본적으로 제외됩니다. 언제든 변경할 수 있습니다.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>현재 세션: %1
다음 세션: %2</translation></message>
    <message><source>Pending change: %1</source><translation>대기 중인 변경: %1</translation></message>
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>항목: %1 · 추가 예정: %2 · 다음 세션에서 제거: %3 · 대기 중인 변경: %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>통합 쓰기 필터(UWF) 관리자</translation></message>
    <message><source>Main toolbar</source><translation>기본 도구 모음</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>UWF 현재 세션 상태와 다음 세션 구성을 다시 읽습니다.</translation></message>
    <message><source>Review and apply</source><translation>검토 및 적용</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>대기 중인 변경을 모두 검토하고 한 번에 적용합니다. 대부분의 변경은 다음 다시 시작 후 적용됩니다.</translation></message>
    <message><source>Safe shutdown</source><translation>안전 종료</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>UWF를 통해 안전하게 종료합니다. UWF 오버레이가 가득 차도 사용할 수 있습니다.</translation></message>
    <message><source>Safe restart</source><translation>안전 다시 시작</translation></message>
    <message><source>Direct restart</source><translation>바로 다시 시작</translation></message>
    <message><source>Direct shutdown</source><translation>바로 종료</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>UWF를 통해 안전하게 다시 시작합니다. UWF 오버레이가 가득 차도 사용할 수 있습니다.</translation></message>
    <message><source>Log</source><translation>로그</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>문제 해결을 위해 현재 세션의 내부 로그를 봅니다.</translation></message>
    <message><source>About</source><translation>정보</translation></message>
    <message><source>About this program.</source><translation>정보 UWF Manager.</translation></message>
    <message><source>Switch display language</source><translation>표시 언어 전환</translation></message>
    <message><source>Language</source><translation>언어</translation></message>
    <message><source>Theme</source><translation>테마</translation></message>
    <message><source>Toggle light / dark theme</source><translation>밝은/어두운 테마 전환</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;UWF 필터 상태, 오버레이, 파일 및 레지스트리 제외를 관리하는 GUI입니다. 대부분의 변경은 다음 다시 시작 후 적용됩니다.&lt;/p&gt;&lt;p&gt;소스 코드: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;라이선스: &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; 전체 라이선스는 프로그램과 함께 제공된 LICENSE 파일에 있습니다.&lt;/p&gt;&lt;p&gt;이 프로그램은 자유 소프트웨어로 GPL v3에 따라 재배포·수정할 수 있습니다. 어떠한 보증 없이 "있는 그대로" 제공됩니다.&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>정보 UWF Manager</translation></message>
    <message><source>System information</source><translation>시스템 정보</translation></message>
    <message><source>Diagnostic snapshot</source><translation>진단 스냅샷</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>시스템, 보안, UWF, 고급 모드와 최근 앱 로그의 진단 정보입니다.</translation></message>
    <message><source>Read only</source><translation>읽기 전용</translation></message>
    <message><source>Diagnostic report</source><translation>진단 보고서</translation></message>
    <message><source>Copied</source><translation>복사됨</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>시스템, 보안, UWF, 고급 모드와 최근 앱 로그의 진단 정보입니다.</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>계정 이름, 장치 ID, 파일 경로, 최근 로그가 포함될 수 있습니다. 공유 전에 확인하세요.</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;이 프로그램은 Windows 통합 쓰기 필터(UWF)에 의존합니다.&lt;/b&gt; 쓰기 보호는 Windows 내장 UWF가 수행합니다. UWF Manager는 직접 필터링하지 않고 구성·관리만 하므로 시스템에 UWF를 설치하고 활성화해야 합니다.&lt;/p&gt;&lt;p&gt;UWF를 처음 활성화하면 성능 향상을 위해 다음 시스템 설정이 바뀝니다.&lt;/p&gt;&lt;ul&gt;&lt;li&gt;페이징 파일이 비활성화됩니다.&lt;/li&gt;&lt;li&gt;시스템 복원이 비활성화됩니다.&lt;/li&gt;&lt;li&gt;SuperFetch가 비활성화됩니다.&lt;/li&gt;&lt;li&gt;파일 인덱싱 서비스가 꺼집니다.&lt;/li&gt;&lt;li&gt;조각 모음 서비스가 꺼집니다.&lt;/li&gt;&lt;li&gt;빠른 시작이 비활성화됩니다.&lt;/li&gt;&lt;li&gt;BCD의 bootstatuspolicy가 ignoreallfailures로 설정됩니다.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;활성화 후 필요에 따라 설정을 바꿀 수 있습니다. 예를 들어 페이징 파일을 보호되지 않는 볼륨으로 옮겨 페이징을 다시 켤 수 있습니다.&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>대상을 찾지 못했습니다. 오버레이에 커밋할 내용이 없습니다.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>대상이 실제 디스크/영구 레지스트리에 없고 보호 중 생성된 오버레이에만 있습니다. 다시 시작하면 사라지므로 삭제 커밋은 불필요하며 불가능합니다.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>시스템이 매개 변수를 거부했습니다(잘못된 경로 또는 인수).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>작업에 실패했습니다. 파일이 열려 있거나 탐색기에서 폴더를 보고 있는 등 다른 프로세스가 대상을 사용할 수 있습니다. 사용하는 프로그램을 닫고 다시 시도하세요.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>작업에 실패했습니다. 자세한 내용은 로그를 확인하세요.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>작업이 거부되었습니다(코드 %1).</translation></message>
    <message><source>Unknown cause.</source><translation>알 수 없음.</translation></message>
    <message><source>Commit canceled</source><translation>커밋 취소됨</translation></message>
    <message><source>Commit result</source><translation>커밋 결과</translation></message>
    <message><source>Succeeded</source><translation>성공</translation></message>
    <message><source>Skipped</source><translation>건너뜀</translation></message>
    <message><source>Failed</source><translation>실패</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>성공: %1; 건너뜀: %2; 실패: %3.</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
사용자가 취소했습니다. %1개 항목을 처리하지 않았습니다.</translation></message>
    <message><source>Category</source><translation>분류</translation></message>
    <message><source>Path</source><translation>경로</translation></message>
    <message><source>Existed before</source><translation>작업 전 존재</translation></message>
    <message><source>Exists after</source><translation>작업 후 존재</translation></message>
    <message><source>Error code</source><translation>오류 코드</translation></message>
    <message><source>Reason</source><translation>이유</translation></message>
    <message><source>Previous page</source><translation>이전 페이지</translation></message>
    <message><source>First page</source><translation>첫 페이지</translation></message>
    <message><source>Last page</source><translation>마지막 페이지</translation></message>
    <message><source>Next page</source><translation>다음 페이지</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>페이지 %1 / %2 · 항목: %3</translation></message>
    <message><source>Copy selected rows</source><translation>선택 행 복사</translation></message>
    <message><source>Copy all</source><translation>모두 복사</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>파일 자동 커밋 목록에 저장된 항목이 있으면 이 레지스트리 제외가 필요합니다. 파일 자동 커밋: 지우기 → 레지스트리 제외 제거.</translation></message>
    <message><source>0 lines</source><translation>0 줄</translation></message>
    <message><source>%1 lines</source><translation>%1 줄</translation></message>
    <message><source>Loading log entries…</source><translation>로그 불러오는 중…</translation></message>
    <message><source>No log entries</source><translation>로그 없음</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>페이지 %1 / %2 · 줄: %3</translation></message>
    <message><source>Copy current page</source><translation>현재 페이지 복사</translation></message>
    <message><source>Clear</source><translation>지우기</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>대기 중인 변경: %1</translation></message>
    <message><source>No pending changes</source><translation>대기 중인 변경 없음</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (레지스트리 제외 · 전역 설정.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>볼륨 %1: UWF 보호 / 파일 제외 / 파일 자동 커밋.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: 지원 안 됨</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>새로 고침 · 볼륨: %1</translation></message>
    <message><source>· Filter (global) %1</source><translation>· 필터 (UWF): %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· 오버레이 · 유형 → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· 오버레이 · 최대 크기 → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· 오버레이 · 경고 임계값 → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· 오버레이 · 위험 임계값 → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ 유형과 최대 크기를 바꾸려면:
1. 위 스위치로 필터를 끕니다.
2. 다시 시작하여 필터를 해제합니다.
3. 설정을 변경합니다.</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· 볼륨 %1 · UWF 보호: %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· 볼륨 %1 · 연결 기준: %2 동일한 CLI 명령 없음; 이 프로그램 전용</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ 파일 제외  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− 파일 제외  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ 레지스트리 제외  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− 레지스트리 제외  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>필터 (UWF): %1</translation></message>
    <message><source>Overlay type → %1</source><translation>오버레이 · 유형 → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>오버레이 · 최대 크기 → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>오버레이 · 경고 임계값 → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>오버레이 · 위험 임계값 → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>볼륨 %1 · UWF 보호: %2</translation></message>
    <message><source>File exclusion %1</source><translation>파일 제외: %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>레지스트리 제외: %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>대기 중인 변경 (%1)</translation></message>
    <message><source>Current session configuration</source><translation>현재 세션 구성</translation></message>
    <message><source>Review and apply changes</source><translation>변경 검토 및 적용</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>아래는 uwfmgr 명령 형식의 전체 구성입니다. 대기 중인 변경은 별도 영역에 먼저 표시됩니다. &lt;span style='color:%1'&gt;대기 중인 변경&lt;/span&gt;. &lt;span style='color:%2'&gt;적용&lt;/span&gt;: 클릭하면 변경을 시스템에 씁니다. 대부분의 변경은 다음 다시 시작 후 적용됩니다.</translation></message>
    <message><source>Confirm apply</source><translation>적용 확인</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>&lt;span style='color:%1'&gt;이 변경 사항을 시스템에 씁니다&lt;/span&gt;. 대부분의 변경은 다음 다시 시작 후 적용됩니다.&lt;br&gt;&lt;br&gt;계속?</translation></message>
    <message><source>Applied changes</source><translation>적용된 변경</translation></message>
    <message><source>Result</source><translation>결과</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ 필터: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ 경고 임계값 설정: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ 위험 임계값 설정: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ 오버레이 유형 설정 / 최대 크기 설정: 유형과 최대 크기를 바꾸려면:
1. 위 스위치로 필터를 끕니다.
2. 다시 시작하여 필터를 해제합니다.
3. 설정을 변경합니다.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ 오버레이 유형 설정: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ 최대 크기 설정: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ 최대 크기 설정: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ 볼륨 %1 · UWF 보호: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ 볼륨 %1 · 연결 기준: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ 볼륨 %1 · 파일 제외 추가: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ 볼륨 %1 · 파일 제외 제거: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ 레지스트리 제외 추가: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ 레지스트리 제외 제거: %1</translation></message>
    <message><source>Time</source><translation>시간</translation></message>
    <message><source>Level</source><translation>수준</translation></message>
    <message><source>Tag</source><translation>태그</translation></message>
    <message><source>Message</source><translation>메시지</translation></message>
    <message><source>Confirm safe shutdown?</source><translation>안전 종료?</translation></message>
    <message><source>Confirm safe restart?</source><translation>안전 다시 시작?</translation></message>
    <message><source>Shut down</source><translation>시스템 종료</translation></message>
    <message><source>Restart</source><translation>다시 시작</translation></message>
    <message><source>Continue shutdown</source><translation>종료 계속</translation></message>
    <message><source>Continue restart</source><translation>다시 시작 계속</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>UWF를 통해 안전하게 종료합니다.</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>UWF를 통해 안전하게 다시 시작합니다.</translation></message>
    <message><source>UWF protection</source><translation>UWF 보호</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>UWF 오버레이가 가득 차도 사용할 수 있습니다.</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>자동 커밋 파일 계산 중…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>작업 전에 파일 %1개를 자동 커밋합니다. 현재 보호되지 않는 볼륨의 파일은 건너뜁니다.</translation></message>
    <message><source>Committing staged files</source><translation>준비된 파일 커밋 중</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>파일 자동 커밋 완료 후 전원 작업을 계속합니다.</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>파일 %2개 중 %1개 처리했습니다.</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>파일 자동 커밋이 완료되지 않음</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>계속할지 결정하기 전에 작업 세부 정보를 확인하세요.</translation></message>
    <message><source>Operation details</source><translation>작업 세부 정보</translation></message>
    <message><source>No additional error details were provided.</source><translation>추가 오류 정보가 없습니다.</translation></message>
    <message><source>Some changes may not be preserved</source><translation>일부 변경 사항이 보존되지 않을 수 있음</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>계속하면 커밋하지 못한 파일 변경을 보존하지 않고 종료합니다.</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>계속하면 커밋하지 못한 파일 변경을 보존하지 않고 다시 시작합니다.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>파일 자동 커밋이 예기치 않게 중단되었습니다:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>파일 자동 커밋이 완료되지 않음: 알 수 없는 오류.</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>커밋하지 않은 변경은 손실됩니다</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>계속하기 전에 작업을 저장하고 보존할 변경을 모두 커밋하세요.</translation></message>
    <message><source>Confirm direct restart?</source><translation>바로 다시 시작?</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>바로 종료?</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>다시 시작. 다시 시작 전 자동 커밋 목록의 파일을 커밋하지 않습니다.</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>시스템 종료. 종료 전 자동 커밋 목록의 파일을 커밋하지 않습니다.</translation></message>
    <message><source>File staging will be skipped</source><translation>파일 자동 커밋을 건너뜁니다</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>다시 시작 전 자동 커밋 목록의 파일을 커밋하지 않습니다.</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>종료 전 자동 커밋 목록의 파일을 커밋하지 않습니다.</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>바로 다시 시작: 파일 자동 커밋을 건너뜁니다.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>바로 종료: 파일 자동 커밋을 건너뜁니다.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>직접 전원 작업을 선택하여 파일 자동 커밋을 건너뛰었습니다.</translation></message>
    <message><source>Safe shutdown failed</source><translation>안전 종료: 실패</translation></message>
    <message><source>Shutdown failed: %1</source><translation>시스템 종료: 실패 (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>안전 다시 시작: 실패</translation></message>
    <message><source>Direct restart failed</source><translation>바로 다시 시작: 실패</translation></message>
    <message><source>Direct shutdown failed</source><translation>바로 종료: 실패</translation></message>
    <message><source>Restart failed: %1</source><translation>다시 시작: 실패 (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>파일 자동 커밋 목록을 읽지 못했습니다:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>파일 자동 커밋 목록을 읽지 못했습니다: 알 수 없는 오류.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>파일: %1 · 성공: %2 · 건너뜀 (파일): %3 · 건너뜀 (항목): %4 · 실패: %5</translation></message>
    <message><source>File staging preparation</source><translation>파일 자동 커밋: 작업 세부 정보</translation></message>
    <message><source>Reparse points are not supported.</source><translation>재분석 지점을 지원하지 않습니다.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>로컬 볼륨의 절대 경로만 추가할 수 있습니다.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>로컬 볼륨의 절대 경로만 추가할 수 있습니다.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>자동 커밋 경로를 검사할 수 없습니다.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>등록한 파일의 경로 유형이 변경되었습니다.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>등록한 폴더의 경로 유형이 변경되었습니다.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>등록한 폴더의 내용을 완전히 열거하지 못했습니다.</translation></message>
    <message><source>The path has no drive letter.</source><translation>경로에 드라이브 문자가 없습니다.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>볼륨 루트를 재귀적으로 처리할 수 없습니다.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>UWF 공급자가 커밋을 거부했습니다.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>커밋 실패: 알 수 없는 오류.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>추가 실패 %1건을 로그에 기록했습니다.</translation></message>
    <message><source>Commit failed</source><translation>커밋 실패</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>경로에 드라이브 문자가 없습니다. 대상 볼륨 확인: 실패.</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>실패: 볼륨 정보 읽기 (%1)</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>볼륨 %1의 현재 세션 레코드가 없습니다.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>파일 제외 목록에 포함된 경로입니다.
제외: %1</translation></message>
    <message><source>No files were found under %1.</source><translation>%1 아래에 파일이 없습니다.</translation></message>
    <message><source>Commit to disk</source><translation>디스크에 커밋</translation></message>
    <message><source>Committing…</source><translation>커밋 중…</translation></message>
    <message><source>This action cannot be undone.</source><translation>이 작업은 되돌릴 수 없습니다.</translation></message>
    <message><source>Continue</source><translation>계속</translation></message>
    <message><source>Delete and commit</source><translation>삭제 및 커밋</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>파일의 오버레이 변경을 디스크에 커밋</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>폴더의 오버레이 변경을 디스크에 커밋</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>파일: %1. 모든 하위 폴더를 포함합니다.</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>파일을 삭제하고 삭제를 디스크에 커밋</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>폴더와 모든 내용을 삭제하고 삭제를 디스크에 커밋</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>파일: %1; 폴더: %2. 삭제 및 커밋.</translation></message>
    <message><source>Commit this registry value to disk</source><translation>레지스트리 값을 디스크에 커밋</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>레지스트리 키와 전체 하위 트리를 디스크에 커밋</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>값: %1. 하위 키를 포함한 전체 키</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>레지스트리 값을 삭제하고 삭제를 디스크에 커밋</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>레지스트리 키와 전체 하위 트리를 삭제하고 삭제를 디스크에 커밋</translation></message>
    <message><source>Scanning registry keys…</source><translation>레지스트리 키 열거…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>레지스트리 키 열거…
키: %1</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>레지스트리 · 키 · 삭제 및 커밋:</translation></message>
    <message><source>Copy current entry</source><translation>현재 항목 복사</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>키 %1개와 포함된 모든 값을 재귀적으로 삭제합니다.</translation></message>
    <message><source>Commit file deletion failed</source><translation>실패: 파일 삭제 커밋</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>경로가 없어 삭제할 항목이 없습니다.</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>레지스트리 제외 목록에 포함된 키입니다.
제외: %1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>실패: 레지스트리 필터 읽기 (%1)</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>현재 세션의 레지스트리 필터 레코드가 없습니다.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>값이 없어 커밋할 항목이 없습니다.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>키가 없어 커밋할 항목이 없습니다.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>값이 없어 삭제할 항목이 없습니다.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>키가 없어 삭제할 항목이 없습니다.</translation></message>
    <message><source>Export commands…</source><translation>명령 내보내기…</translation></message>
    <message><source>Export commands to file</source><translation>명령을 파일로 내보내기</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>파일로 내보내기… · 명령: %1
%2</translation></message>
    <message><source>Import</source><translation>가져오기</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: 명령을 한 줄에 하나씩 입력하거나 붙여 넣으세요. 로그·스크립트 등 텍스트 형식(.txt, .bat, .ps1)에서 해당 명령이 포함된 줄을 추가합니다. 가져오기는 변경을 대기 상태로 만듭니다. 시스템에 쓰려면 도구 모음의 "검토 및 적용"을 사용하세요.</translation></message>
    <message><source>Import uwfmgr commands</source><translation>uwfmgr · 가져오기</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;uwfmgr&lt;/b&gt;: 명령을 한 줄에 하나씩 입력하거나 붙여 넣으세요. &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;파일에서 불러오기…&lt;/b&gt;: 로그·스크립트 등 텍스트 형식(.txt, .bat, .ps1)에서 해당 명령이 포함된 줄을 추가합니다. &lt;code&gt;uwfmgr&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;가져오기&lt;/b&gt;: &lt;b&gt;가져오기는 변경을 대기 상태로 만듭니다. 시스템에 쓰려면 도구 모음의 "검토 및 적용"을 사용하세요.&lt;/b&gt; &lt;b&gt;검토 및 적용&lt;/b&gt;.&lt;/p&gt;</translation></message>
    <message><source>Load from file…</source><translation>파일에서 불러오기…</translation></message>
    <message><source>Load recommended configuration</source><translation>권장 구성 불러오기</translation></message>
    <message><source>Choose recommended configuration</source><translation>권장 구성 선택</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>추가할 권장 구성 그룹을 선택하세요. 가져오기 전에 개별 명령을 검토하거나 삭제할 수 있습니다.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Microsoft 공식 문서에서 가져왔습니다.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Microsoft 권장 UWF 제외 항목</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>가져오기 전에 검토하세요. UWF가 파일 제외를 수락하려면 대상 폴더가 먼저 존재해야 합니다.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>출처: Microsoft 공식 UWF 문서(일반 쓰기 필터 제외와 맬웨어 방지 지원 포함).</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>사용자 환경 개선 프로그램(CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: 영구 저장: 정책 참여 동의 상태</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: 영구 저장: 로컬 참여 동의 상태</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: 영구 저장: 업로드 비활성화 플래그</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Background Intelligent Transfer Service(BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: 영구 저장: 다운로드 대기열 파일</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: 영구 저장: 전송 상태 인덱스</translation></message>
    <message><source>Network profiles and policies</source><translation>네트워크 프로필 및 정책</translation></message>
    <message><source>Wireless network GPO policy</source><translation>무선 네트워크 GPO 정책</translation></message>
    <message><source>Wired network GPO policy</source><translation>유선 네트워크 GPO 정책</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>무선 네트워크 GPO 정책 · 파일</translation></message>
    <message><source>Wired network GPO policy files</source><translation>유선 네트워크 GPO 정책 · 파일</translation></message>
    <message><source>Wireless network interface profiles</source><translation>무선 인터페이스 프로필</translation></message>
    <message><source>Wired network interface profiles</source><translation>유선 인터페이스 프로필</translation></message>
    <message><source>Wireless service configuration</source><translation>무선 서비스 구성</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>모바일 광대역 서비스 구성</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>유선 자동 구성 서비스 설정</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>네트워크 프로필 XML은 장치마다 다릅니다. 필요하면 실제 Interfaces\{GUID}\{GUID}.xml 경로를 직접 추가하세요.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>일광 절약 시간(DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: 영구 저장: 표준 시간대 정의</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: 영구 저장: 선택된 표준 시간대 정보</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: 영구 저장: 제품 파일 및 업데이트</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: 영구 저장: ProgramData 서명 및 상태</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: 영구 저장: Windows Update · 로그</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: 영구 저장: MpCmdRun · 로그</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: 영구 저장: 제품 레지스트리 상태</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: 영구 저장: WdBoot · 서비스 상태</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: 영구 저장: WdFilter · 서비스 상태</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: 영구 저장: WdNisSvc · 서비스 상태</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: 영구 저장: WdNisDrv · 서비스 상태</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: 영구 저장: WinDefend · 서비스 상태</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: 영구 저장: 클라이언트 프로그램 파일</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: 영구 저장: Windows Update · 로그</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: 영구 저장: MpCmdRun · 로그</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: 영구 저장: 맬웨어 방지 서명 및 상태</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: 영구 저장: 맬웨어 방지 레지스트리 상태</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>uwfmgr 명령이 포함된 파일 선택</translation></message>
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>모든 파일 (*);;텍스트 파일 (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
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
    <message><source>Status</source><translation>상태</translation></message>
    <message><source>Command</source><translation>명령</translation></message>
    <message><source>Detail</source><translation>세부 정보</translation></message>
    <message><source>Import failed</source><translation>가져오기 실패</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>내부 오류: 적용 처리기가 등록되지 않았습니다.</translation></message>
    <message><source>Nothing to import</source><translation>가져올 명령 없음</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>입력에서 uwfmgr 명령을 찾지 못했습니다.</translation></message>
    <message><source>Applied: %1</source><translation>적용됨: %1</translation></message>
    <message><source>Duplicates: %1</source><translation>중복: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>지원 안 됨: %1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>%1회 일괄 처리 후 누계:</translation></message>
    <message><source>── Batch %1 ──</source><translation>── 일괄 처리 %1 ──</translation></message>
    <message><source>Applied</source><translation>적용됨</translation></message>
    <message><source>Duplicate</source><translation>중복</translation></message>
    <message><source>Unsupported</source><translation>지원 안 됨</translation></message>
    <message><source>File too large</source><translation>파일이 너무 큼</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>파일 %1이 5 MB를 넘어 분석하지 않았습니다. 먼저 직접 내용을 추려 주세요.</translation></message>
    <message><source>Cannot read file</source><translation>파일을 읽을 수 없음</translation></message>
    <message><source>Could not open file %1: %2</source><translation>실패: 파일 열기 (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>대기 중인 변경을 버릴까요?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>적용하지 않은 변경이 %1개 있습니다.

버리고 계속할까요?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>불완전한 uwfmgr 명령</translation></message>
    <message><source>Missing size argument (MB)</source><translation>누락된 인수: 크기 (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>크기는 MB 단위의 0 이상 정수여야 합니다</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>누락된 인수: 오버레이 · 유형 (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>잘못된 값: 오버레이 · 유형: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>누락된 인수: 볼륨 (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>볼륨은 C: 같은 드라이브 문자여야 합니다</translation></message>
    <message><source>Missing path argument</source><translation>누락된 인수: 경로</translation></message>
    <message><source>Missing registry key argument</source><translation>누락된 인수: 레지스트리 · 키</translation></message>
    <message><source>Unclosed double quote in command</source><translation>명령의 큰따옴표가 닫히지 않았습니다</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>예상하지 못한 추가 인수: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>UI 작업에 연결할 수 없는 uwfmgr 명령입니다</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>대기 중인 변경: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>이미 목표 상태이므로 작업이 필요 없습니다</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>경로가 이 볼륨에 없거나 파일 제외를 지원하지 않는 볼륨입니다(예: exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>UWF 차단 목록에서 거부했습니다(시스템 파일 / Windows / 페이지 파일 등)</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>이 일괄 처리에서 이미 같은 명령이 실행되었습니다</translation></message>
    <message><source>Pending filter %1</source><translation>대기 중인 변경: 필터 %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>필터: 이미 목표 상태이므로 작업이 필요 없습니다</translation></message>
    <message><source>Pending overlay type → %1</source><translation>대기 중인 변경: 오버레이 · 유형 → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>오버레이 · 유형: %1 (이미 목표 상태이므로 작업이 필요 없습니다)</translation></message>
    <message><source>Invalid size value: %1</source><translation>잘못된 값: 크기: %1</translation></message>
    <message><source>maximum size</source><translation>최대 크기</translation></message>
    <message><source>warning threshold</source><translation>경고 임계값</translation></message>
    <message><source>critical threshold</source><translation>위험 임계값</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>대기 중인 변경: 오버레이 %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>오버레이 %1: %2 MB (이미 목표 상태이므로 작업이 필요 없습니다)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>알 수 없는 볼륨 %1(이 드라이브 문자에 UWF 지원 디스크가 없음)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>대기 중인 변경: 볼륨 %1 · UWF 보호: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>볼륨 %1: 이미 목표 상태이므로 작업이 필요 없습니다</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: 경로에 드라이브 문자가 없습니다. 대상 볼륨 확인: 실패.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>볼륨 %1: UWF 지원 안 됨</translation></message>
    <message><source>file exclusion</source><translation>파일 제외</translation></message>
    <message><source>registry exclusion</source><translation>레지스트리 제외</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>레지스트리 제외를 사용할 수 있는 레지스트리 탭이 없습니다</translation></message>
    <message><source>Unsupported command</source><translation>명령: 지원 안 됨</translation></message>
    <message><source>Exit</source><translation>종료</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: 표시 전환.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: 사용 중</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: 사용 안 함</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: 알 수 없음</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>사용 중 %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>고급 모드</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Windows 종료 및 다시 시작과 파일 자동 커밋을 연동합니다.</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>자동 시작 LocalSystem 서비스를 설치합니다. Windows 시작 후 UWF Manager를 실행하고 종료·다시 시작 전에 등록된 파일 변경을 커밋하여 UWF와 충돌하는 앱의 호환성 개선을 시도합니다.</translation></message>
    <message><source>System changes</source><translation>시스템 변경 사항</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>자동 시작 서비스 UWFManagerEnhanced를 만들고 현재 실행 파일에 연결합니다</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>구성: 레지스트리 → 커밋 (HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>다음 Windows 시작 후 자동 실행됩니다. 실행 파일을 옮기면 자동 시작이 실패할 수 있습니다</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>정상적인 Windows 종료·다시 시작 시 조건에 맞는 파일을 커밋합니다</translation></message>
    <message><source>Operational and security considerations</source><translation>운영 및 보안 고려 사항</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>시스템 레지스트리에 서비스를 등록하므로 완전한 포터블 상태가 아니게 됩니다</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>대상 파일이 많으면 종료·다시 시작이 크게 지연될 수 있습니다. 변경을 보존할 충분한 시간을 주세요</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>고급 모드는 UWF Manager를 SYSTEM으로 실행합니다. 비관리자가 프로그램을 변조하거나 악성 실행 파일로 바꾸면 SYSTEM 권한을 얻을 수 있습니다. 관리자만 수정할 수 있는 위치에서만 사용하세요(권장: %ProgramFiles%\UWF Manager\Service\UWF.exe).</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>실패: 고급 모드 상태 읽기</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>실패: 고급 모드 상태 읽기 (알 수 없는 오류).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>실패: 서비스 상태 읽기:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>실패: 서비스 상태 읽기 (알 수 없는 오류).</translation></message>
    <message><source>Status: Disabled</source><translation>상태: 사용 안 함</translation></message>
    <message><source>Status: Enabled</source><translation>상태: 사용 중</translation></message>
    <message><source>Status: Service stopped</source><translation>상태: 서비스 중지됨</translation></message>
    <message><source>Status: Repair required</source><translation>상태: 복구 필요</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>서비스가 독립 프로세스로 구성되지 않았습니다.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>서비스가 자동 시작으로 구성되지 않았습니다.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>서비스가 LocalSystem으로 실행되도록 구성되지 않았습니다.</translation></message>
    <message><source>The service is not running.</source><translation>서비스가 실행 중이 아닙니다.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>서비스 실행 경로나 시작 인수가 현재 실행 파일과 다릅니다.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>서비스의 종료 사전 알림 제한 시간이 올바르지 않습니다.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>서비스에 UI 에이전트 시작에 필요한 권한이 선언되지 않았습니다.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>서비스가 Windows 종료 사전 알림을 수락하지 않았습니다.</translation></message>
    <message><source>Enable enhanced mode</source><translation>고급 모드 사용</translation></message>
    <message><source>Disable enhanced mode</source><translation>고급 모드 사용 안 함</translation></message>
    <message><source>Start service</source><translation>서비스 시작</translation></message>
    <message><source>Repair enhanced mode</source><translation>고급 모드 복구</translation></message>
    <message><source>Delete service</source><translation>서비스 삭제</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>고급 모드 서비스를 중지·삭제하고 남은 서비스 레지스트리 데이터도 삭제합니다.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>서비스는 없지만 레지스트리 데이터가 남아 있습니다.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>UI 에이전트 인증 완료를 기다리는 중입니다.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>서비스는 실행 중이지만 인증된 UI 에이전트가 연결되지 않았습니다.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>UWF Manager 고급 모드 도우미 서비스</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>고급 모드 · 구성: 적용됨.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>구성: 적용됨. UWF 영구 저장 보고:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>고급 모드 · 구성: 실패:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>고급 모드 · 구성: 실패 (알 수 없는 오류).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>고급 모드 서비스와 남은 구성을 삭제했습니다.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>서비스 삭제를 요청했지만 잔여 데이터 정리 중 다음 문제가 보고되었습니다:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>고급 모드: 서비스 삭제: 실패:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>고급 모드: 서비스 삭제: 실패 (알 수 없는 오류).</translation></message>
    <message><source>File staging completed</source><translation>파일 자동 커밋: 성공</translation></message>
    <message><source>File staging failed</source><translation>파일 자동 커밋: 실패</translation></message>
    <message><source>All staged targets were processed.</source><translation>모든 대상을 처리했습니다.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>모든 대상을 처리했지만 일부 작업이 실패했습니다.</translation></message>
    <message><source>Show main window</source><translation>메인 창 표시</translation></message>
    <message><source>Restore default position</source><translation>기본 위치로 복원</translation></message>
    <message><source>Hide overlay hub</source><translation>오버레이 HUB 숨기기</translation></message>
    <message><source>Exit application</source><translation>앱 종료</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ 실패: 시스템 연결</translation></message>
    <message><source>Registry access failed</source><translation>실패: 레지스트리 접근</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>실패: 레지스트리 값 읽기 (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>오버레이 파일 열거: 알 수 없는 오류.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ 실패: 필터 %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ 실패: 필터 상태 읽기</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ 실패: 경고 임계값 설정</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ 실패: 위험 임계값 설정</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ 실패: 오버레이 상태 읽기</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ 실패: 오버레이 유형 설정</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ 실패: 최대 크기 설정</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ 실패: 오버레이 구성 읽기</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ 실패: 볼륨 %1 UWF 등록</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ 실패: 볼륨 %2 보호 %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ 실패: 볼륨 %1 연결 설정</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ 볼륨 %1 · 실패: 파일 제외 추가 %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ 볼륨 %1의 파일 제외가 이미 없음: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ 볼륨 %1 · 실패: 파일 제외 제거 %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ 실패: 볼륨 구성 읽기</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ 실패: 레지스트리 제외 추가 %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ 실패: 레지스트리 제외 제거 %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ 실패: 레지스트리 영구 저장 스위치 갱신</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ 실패: 레지스트리 필터 읽기</translation></message>
    <message><source>UWF is not available</source><translation>UWF: 지원 안 됨</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>대상 상태 읽기: 알 수 없는 오류.</translation></message>
    <message><source>The target state could not be read before the operation: %1</source><translation>작업 전에 대상 상태를 읽지 못했습니다: %1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>경로가 없어 삭제할 항목이 없습니다.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>알 수 없는 오류.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>대상 상태 재읽기에 실패하여 작업 결과를 확인할 수 없습니다: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>공급자가 삭제를 수락했지만 신뢰할 수 있는 상태 재확인에서 대상이 여전히 존재합니다.</translation></message>
    <message><source>Unknown</source><translation>알 수 없음</translation></message>
    <message><source>Failed to load logs: %1</source><translation>실패: 로그 불러오기 (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>실패: 로그 불러오기 (알 수 없는 오류).</translation></message>
    <message><source>: unknown error</source><translation>: 알 수 없는 오류</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>실패: 대상 상태 읽기 (%1)</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>실패: 대상 상태 읽기 (알 수 없는 오류).</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>실패: 대상 볼륨 확인 (%1)</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>실패: 레지스트리 값 열거 (%1)</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>실패: 레지스트리 키 열거 (%1)</translation></message>
    <message><source>Delete and commit failed</source><translation>삭제 및 커밋: 실패</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>실패: 대상 볼륨 확인 (%1): %2</translation></message>
    <message><source>%1/%2 slots</source><translation>%1/%2 슬롯</translation></message>
</context>
</TS>
