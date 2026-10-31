<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="ko" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>미리 보기</translation></message>
    <message><source>enable</source><translation>사용</translation></message>
    <message><source>disable</source><translation>사용 안 함</translation></message>
    <message><source>Enabled</source><translation>사용 중</translation></message>
    <message><source>Disabled</source><translation>사용 안 함</translation></message>
    <message><source>OK</source><translation>확인</translation></message>
    <message><source>Cancel</source><translation>취소</translation></message>
    <message><source>Close</source><translation>닫기</translation></message>
    <message><source>Refresh</source><translation>새로 고침</translation></message>
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
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>현재 세션의 볼륨 보호 상태입니다(읽기 전용).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>다음 세션에서 이 볼륨을 보호합니다. 쓰기는 오버레이로 리디렉션되며 다시 시작하면 삭제됩니다.</translation></message>
    <message><source>Current session</source><translation>현재 세션</translation></message>
    <message><source>Next session</source><translation>다음 세션</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>볼륨 식별 방식입니다. 드라이브 문자는 간단하지만 디스크 추가·제거로 문자가 바뀌면 연결이 끊어집니다. 볼륨 ID는 문자가 바뀌어도 유지됩니다.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF에서 지원하지 않는 볼륨: </translation></message>
    <message><source>Global settings</source><translation>전역 설정</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>현재 세션의 UWF 필터 상태입니다(읽기 전용).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>다음 세션에서 UWF 필터를 사용합니다. 보호된 볼륨의 쓰기는 오버레이로 리디렉션되며 다시 시작 시 삭제됩니다.</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>오버레이 사용량이 이 값에 도달하면 이벤트 로그에 경고 수준 알림을 기록합니다. 두 이벤트가 모두 활성화된 경우 이 값은 심각 임계값보다 작아야 합니다. 0으로 설정하면 경고 이벤트만 비활성화됩니다.</translation></message>
    <message><source>Critical threshold</source><translation>위험 임계값</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>오버레이 사용량이 이 값에 도달하면 이벤트 로그에 심각 수준 알림을 기록합니다. 두 이벤트가 모두 활성화된 경우 이 값은 경고 임계값보다 커야 합니다. 0으로 설정하면 심각 이벤트만 비활성화됩니다.</translation></message>
    <message><source>Overlay</source><translation>오버레이</translation></message>
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
    <message><source>File staging</source><translation>파일 저장 대기</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>파일 / 폴더: 안전 종료 또는 다시 시작 전에 자동으로 커밋됩니다. 모든 하위 폴더를 포함합니다. 목록에 항목이 있으면 다음 레지스트리 제외로 보호된 세션 간에도 목록을 유지합니다: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>파일 또는 폴더를 자동 커밋 목록에 추가합니다.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>디스크에 커밋할 파일 선택. 안전 종료 또는 다시 시작 전에 자동으로 커밋됩니다.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>디스크에 커밋할 폴더 선택. 안전 종료 또는 다시 시작 전에 자동으로 커밋됩니다. 모든 하위 폴더를 포함합니다.</translation></message>
    <message><source>Select files for automatic commit</source><translation>디스크에 커밋할 파일 선택 · 파일 저장 대기</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>디스크에 커밋할 폴더 선택 · 파일 저장 대기 · 모든 하위 폴더를 포함합니다.</translation></message>
    <message><source>Cannot stage this path</source><translation>이 경로를 자동 커밋에 추가할 수 없음</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>UWF 파일 제외와 겹칩니다. 다음 제외 항목을 먼저 제거하세요:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>파일 저장 대기 항목과 겹칩니다. 다음 경로를 먼저 제거한 후 제외를 추가하세요:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>기존 파일 저장 대기 항목과 경로가 겹칩니다</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>파일 저장 대기 목록에 저장된 항목이 있으면 이 레지스트리 제외가 필요합니다</translation></message>
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
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>볼륨 %1의 경로입니다. 해당 볼륨의 파일 저장 대기 탭에서 추가하세요.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>볼륨 루트는 추가할 수 없습니다. 전체 볼륨이 재귀적으로 커밋되기 때문입니다.</translation></message>
    <message><source>File staging could not be saved</source><translation>실패: 파일 저장 대기 목록 저장</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>갱신을 완료할 수 없어 파일 저장 대기 목록을 다시 불러왔습니다:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>파일 저장 대기: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>안전 종료 또는 다시 시작 전에 자동으로 커밋됩니다.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>파일 저장 대기: %1 · 파일: %2 · 폴더: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>파일 저장 대기 목록을 다시 시작한 후에도 유지하려면 이 레지스트리 제외 항목이 필요합니다.</translation></message>
    <message><source>Built-in</source><translation>기본 제공</translation></message>
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
    <message><source>Name</source><translation>이름</translation></message>
    <message><source>Data</source><translation>데이터</translation></message>
    <message><source>(Default)</source><translation>(기본값)</translation></message>
    <message><source>(value not set)</source><translation>(값 설정 안 됨)</translation></message>
    <message><source>(empty)</source><translation>(비어 있음)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>알 수 없음 (값: 파일이 너무 큼)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>오버레이 파일 보기. &lt;b&gt;진단용 스냅샷입니다.&lt;/b&gt; 커밋 대상을 결정하는 데 사용하지 마세요.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>오버레이 파일 목록은 NTFS 볼륨에서만 지원합니다.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF 필터가 꺼져 검사할 오버레이가 없습니다.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>UWF로 보호되지 않는 볼륨이므로 검사할 오버레이가 없습니다.</translation></message>
    <message><source>Failed: %1</source><translation>실패: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>파일 열거 중 WMI 공급자가 충돌했습니다. 오버레이가 크거나 I/O 부하가 높을 때 발생하는 UWF_Overlay.GetOverlayFiles의 알려진 불안정성입니다.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>메모리가 부족하거나 공급자가 작업을 지원하지 않습니다. NTFS와 충분한 여유 메모리가 필요합니다. 오버레이를 줄이거나 메모리 사용이 큰 앱을 닫고 다시 시도하세요.</translation></message>
    <message><source>Raw error: %1</source><translation>오류 코드: %1</translation></message>
    <message><source>Export overlay file list</source><translation>파일로 내보내기… · 오버레이 · 파일</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>텍스트 파일 (*.txt);;모든 파일 (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>명령 파일 (*.cmd);;모든 파일 (*)</translation></message>
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
    <message><source>✓ %1 persistence: %2</source><translation>✓ 영구 저장 %1: %2</translation></message>
    <message><source>Remove selected</source><translation>선택 항목 제거</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>선택한 제외 항목을 제거합니다. 적용 후 반영됩니다.</translation></message>
    <message><source>Open containing folder</source><translation>포함 폴더 열기</translation></message>
    <message><source>Copy file path</source><translation>파일 경로 복사</translation></message>
    <message><source>Copy registry path</source><translation>레지스트리 경로 복사</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>폴더의 오버레이 변경을 디스크에 커밋…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>파일의 오버레이 변경을 디스크에 커밋…</translation></message>
    <message><source>Copied to clipboard</source><translation>클립보드에 복사됨</translation></message>
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
    <message><source>Unified Write Filter (UWF) Manager</source><translation>통합 쓰기 필터(UWF) 관리자</translation></message>
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
    <message><source>Switch display language</source><translation>표시 언어 전환</translation></message>
    <message><source>Language</source><translation>언어</translation></message>
    <message><source>Theme</source><translation>테마</translation></message>
    <message><source>Toggle light / dark theme</source><translation>밝은/어두운 테마 전환</translation></message>
    <message><source>System information</source><translation>시스템 정보</translation></message>
    <message><source>Diagnostic snapshot</source><translation>진단 스냅샷</translation></message>
    <message><source>Read only</source><translation>읽기 전용</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>대상을 찾지 못했습니다. 오버레이에 커밋할 내용이 없습니다.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>대상이 실제 디스크/영구 레지스트리에 없고 보호 중 생성된 오버레이에만 있습니다. 다시 시작하면 사라지므로 삭제 커밋은 불필요하며 불가능합니다.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>시스템이 매개 변수를 거부했습니다(잘못된 경로 또는 인수).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>작업에 실패했습니다. 파일이 열려 있거나 탐색기에서 폴더를 보고 있는 등 다른 프로세스가 대상을 사용할 수 있습니다. 사용하는 프로그램을 닫고 다시 시도하세요.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>작업에 실패했습니다. 자세한 내용은 로그를 확인하세요.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>작업이 거부되었습니다(코드 %1).</translation></message>
    <message><source>Unknown cause.</source><translation>알 수 없음.</translation></message>
    <message><source>Commit canceled</source><translation>커밋 취소됨</translation></message>

    <message><source>Succeeded</source><translation>성공</translation></message>
    <message><source>Skipped</source><translation>건너뜀</translation></message>
    <message><source>Failed</source><translation>실패</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>성공: %1; 건너뜀: %2; 실패: %3.</translation></message>

    <message><source>Existed before</source><translation>작업 전 존재</translation></message>
    <message><source>Exists after</source><translation>작업 후 존재</translation></message>
    <message><source>Error code</source><translation>오류 코드</translation></message>
    <message><source>Reason</source><translation>이유</translation></message>
    <message><source>Previous page</source><translation>이전 페이지</translation></message>
    <message><source>First page</source><translation>첫 페이지</translation></message>
    <message><source>Last page</source><translation>마지막 페이지</translation></message>
    <message><source>Next page</source><translation>다음 페이지</translation></message>
    <message><source>Copy selected rows</source><translation>선택 행 복사</translation></message>
    <message><source>Copy all</source><translation>모두 복사</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>파일 저장 대기 목록에 저장된 항목이 있으면 이 레지스트리 제외가 필요합니다. 파일 저장 대기: 지우기 → 레지스트리 제외 제거.</translation></message>
    <message><source>Copy current page</source><translation>현재 페이지 복사</translation></message>
    <message><source>No pending changes</source><translation>대기 중인 변경 없음</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (레지스트리 제외 · 전역 설정.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>볼륨 %1: UWF 보호 / 파일 제외 / 파일 저장 대기.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: 지원 안 됨</translation></message>
    <message><source>Review and apply changes</source><translation>변경 검토 및 적용</translation></message>
    <message><source>Confirm apply</source><translation>적용 확인</translation></message>
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
    <message><source>Calculating files for automatic commit…</source><translation>자동 커밋 파일 계산 중…</translation></message>
    <message><source>Committing staged files</source><translation>준비된 파일 커밋 중</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>파일 %2개 중 %1개 처리했습니다.</translation></message>
    <message><source>No additional error details were provided.</source><translation>추가 오류 정보가 없습니다.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>파일 저장 대기이 예기치 않게 중단되었습니다:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>파일 저장 대기이 완료되지 않음: 알 수 없는 오류.</translation></message>
    <message><source>File staging will be skipped</source><translation>파일 저장 대기을 건너뜁니다</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>바로 다시 시작: 파일 저장 대기을 건너뜁니다.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>바로 종료: 파일 저장 대기을 건너뜁니다.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>직접 전원 작업을 선택하여 파일 저장 대기을 건너뛰었습니다.</translation></message>
    <message><source>Safe shutdown failed</source><translation>안전 종료: 실패</translation></message>
    <message><source>Shutdown failed: %1</source><translation>시스템 종료: 실패 (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>안전 다시 시작: 실패</translation></message>
    <message><source>Direct restart failed</source><translation>바로 다시 시작: 실패</translation></message>
    <message><source>Direct shutdown failed</source><translation>바로 종료: 실패</translation></message>
    <message><source>Restart failed: %1</source><translation>다시 시작: 실패 (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>파일 저장 대기 목록을 읽지 못했습니다:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>파일 저장 대기 목록을 읽지 못했습니다: 알 수 없는 오류.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>파일: %1 · 성공: %2 · 건너뜀 (파일): %3 · 건너뜀 (항목): %4 · 실패: %5</translation></message>
    <message><source>File staging preparation</source><translation>파일 저장 대기: 작업 세부 정보</translation></message>
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

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>경로에 드라이브 문자가 없습니다. 대상 볼륨 확인: 실패.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>볼륨 %1의 현재 세션 레코드가 없습니다.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>파일 제외 목록에 포함된 경로입니다.
제외: %1</translation></message>

    <message><source>Continue</source><translation>계속</translation></message>
    <message><source>Delete and commit</source><translation>삭제 및 커밋</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>레지스트리 제외 목록에 포함된 키입니다.
제외: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>현재 세션의 레지스트리 필터 레코드가 없습니다.</translation></message>

    <message><source>Export commands to file</source><translation>명령을 파일로 내보내기</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>파일로 내보내기… · 명령: %1
%2</translation></message>
    <message><source>Import</source><translation>가져오기</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: 명령을 한 줄에 하나씩 입력하거나 붙여 넣으세요. 로그·스크립트 등 텍스트 형식(.txt, .bat, .ps1)에서 해당 명령이 포함된 줄을 추가합니다. 가져오기는 변경을 대기 상태로 만듭니다. 시스템에 쓰려면 도구 모음의 "검토 및 적용"을 사용하세요.</translation></message>
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
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>텍스트 파일 (*.txt *.cmd);;모든 파일 (*)</translation></message>
    <message><source>Status</source><translation>상태</translation></message>
    <message><source>Import failed</source><translation>가져오기 실패</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>내부 오류: 적용 처리기가 등록되지 않았습니다.</translation></message>
    <message><source>Nothing to import</source><translation>가져올 명령 없음</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>입력에서 uwfmgr 명령을 찾지 못했습니다.</translation></message>
    <message><source>Duplicates: %1</source><translation>중복: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>지원 안 됨: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── 일괄 처리 %1 ──</translation></message>
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
    <message><source>Already in the target state — no-op</source><translation>이미 목표 상태이므로 작업이 필요 없습니다</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>경로가 이 볼륨에 없거나 파일 제외를 지원하지 않는 볼륨입니다(예: exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>UWF 차단 목록에서 거부했습니다(시스템 파일 / Windows / 페이지 파일 등)</translation></message>
    <message><source>Invalid size value: %1</source><translation>잘못된 값: 크기: %1</translation></message>
    <message><source>maximum size</source><translation>최대 크기</translation></message>
    <message><source>warning threshold</source><translation>경고 임계값</translation></message>
    <message><source>critical threshold</source><translation>위험 임계값</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>전역 구성을 사용할 수 없어 명령을 대기 중인 변경 사항에 추가하지 않았습니다.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>알 수 없는 볼륨 %1(이 드라이브 문자에 UWF 지원 디스크가 없음)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>볼륨 %1: 이미 목표 상태이므로 작업이 필요 없습니다</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: 경로에 드라이브 문자가 없습니다. 대상 볼륨 확인: 실패.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>볼륨 %1: UWF 지원 안 됨</translation></message>
    <message><source>file exclusion</source><translation>파일 제외</translation></message>
    <message><source>registry exclusion</source><translation>레지스트리 제외</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>레지스트리 제외를 사용할 수 있는 레지스트리 탭이 없습니다</translation></message>
    <message><source>Unsupported command</source><translation>명령: 지원 안 됨</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: 표시 전환.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: 사용 중</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: 사용 안 함</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: 알 수 없음</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>사용 중 %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>고급 모드</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Windows 종료 및 다시 시작과 파일 저장 대기을 연동합니다.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>실패: 고급 모드 상태 읽기 (알 수 없는 오류).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>실패: 서비스 상태 읽기:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>실패: 서비스 상태 읽기 (알 수 없는 오류).</translation></message>
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
    <message><source>File staging completed</source><translation>파일 저장 대기: 성공</translation></message>
    <message><source>File staging failed</source><translation>파일 저장 대기: 실패</translation></message>
    <message><source>All staged targets were processed.</source><translation>모든 대상을 처리했습니다.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>모든 대상을 처리했지만 일부 작업이 실패했습니다.</translation></message>
    <message><source>Show main window</source><translation>메인 창 표시</translation></message>
    <message><source>Restore default position</source><translation>기본 위치로 복원</translation></message>
    <message><source>Show overlay hub</source><translation>오버레이 HUB 표시</translation></message>
    <message><source>Hide overlay hub</source><translation>오버레이 HUB 숨기기</translation></message>
    <message><source>Exit application</source><translation>앱 종료</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ 실패: 시스템 연결</translation></message>
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
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>경로가 없어 삭제할 항목이 없습니다.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>알 수 없는 오류.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>대상 상태 재읽기에 실패하여 작업 결과를 확인할 수 없습니다: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>공급자가 삭제를 수락했지만 신뢰할 수 있는 상태 재확인에서 대상이 여전히 존재합니다.</translation></message>
    <message><source>Unknown</source><translation>알 수 없음</translation></message>
    <message><source>Failed to load logs: %1</source><translation>실패: 로그 불러오기 (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>실패: 로그 불러오기 (알 수 없는 오류).</translation></message>
    <message><source>: unknown error</source><translation>: 알 수 없는 오류</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>실패: 대상 볼륨 확인 (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>실패: 레지스트리 키 열거 (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>실패: 대상 볼륨 확인 (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>선행 작업이 실패했거나 결과를 확인할 수 없어 이 작업을 건너뛰었습니다.</translation></message>
    <message><source>Copy</source><translation>복사</translation></message>
    <message><source>Select All</source><translation>모두 선택</translation></message>
    <message><source>Display language</source><translation>표시 언어</translation></message>
    <message><source>Folder</source><translation>폴더</translation></message>
    <message><source>File</source><translation>파일</translation></message>
    <message><source>Export commands</source><translation>명령 내보내기</translation></message>
    <message><source>Direct restart…</source><translation>바로 다시 시작…</translation></message>
    <message><source>Clear…</source><translation>지우기…</translation></message>
    <message><source>Committing</source><translation>커밋 중</translation></message>
    <message><source> · %1 selected</source><translation> · %1개 선택</translation></message>
    <message><source>%1 changes</source><translation>변경 %1개</translation></message>
    <message><source>%1 commands</source><translation>명령 %1개</translation></message>
    <message><source>%1 items</source><translation>항목 %1개</translation></message>
    <message><source>%1 items · %2</source><translation>항목 %1개 · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>보류 중인 변경 %1개</translation></message>
    <message><source>%1 records</source><translation>기록 %1개</translation></message>
    <message><source>%1 records · %2 total</source><translation>기록 %1개 · 전체 %2개</translation></message>


    <message><source>%1 selected</source><translation>%1개 선택</translation></message>
    <message><source>/ %1 files</source><translation>/ 파일 %1개</translation></message>
    <message><source>32-bit</source><translation>32비트</translation></message>
    <message><source>64-bit</source><translation>64비트</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;에 따라 배포됩니다. 전체 약관은 앱에 포함된 LICENSE 파일에 있습니다.&lt;/p&gt;&lt;p&gt;GPL v3에 따라 재배포하거나 수정할 수 있습니다. 이 소프트웨어는 어떠한 보증도 없이 있는 그대로 제공됩니다.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows는 페이징 파일, 시스템 복원, SuperFetch, 파일 인덱싱, 디스크 조각 모음 및 빠른 시작을 비활성화합니다. 또한 BCD의 bootstatuspolicy를 ignoreallfailures로 설정합니다.&lt;/p&gt;&lt;p&gt;이 설정은 나중에 변경할 수 있습니다. 예를 들어 페이징을 다시 활성화하기 전에 페이징 파일을 보호되지 않은 볼륨으로 옮길 수 있습니다.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>시스템, 권한 및 UWF의 진단 스냅샷입니다.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>이 위치에 파일이 이미 있습니다:
%1

저장하면 파일 내용이 대체됩니다.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Windows Unified Write Filter용 그래픽 관리 도구입니다.
볼륨 보호, 오버레이, 제외 항목 및 저장 대기 파일을 관리합니다.</translation></message>
    <message><source>About this snapshot</source><translation>이 스냅샷 정보</translation></message>
    <message><source>Account</source><translation>계정</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>스크립트나 텍스트에서 UWF 설정을 추가합니다.</translation></message>
    <message><source>Add exclusion</source><translation>제외 항목 추가</translation></message>
    <message><source>Add files or folders</source><translation>파일 또는 폴더 추가</translation></message>
    <message><source>Add selected items</source><translation>선택한 항목 추가</translation></message>
    <message><source>Added</source><translation>추가됨</translation></message>
    <message><source>Administrator</source><translation>관리자</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>모든 변경을 디스크에 저장했습니다. 다시 시작하지 않아도 유지됩니다.</translation></message>
    <message><source>All files (*)</source><translation>모든 파일 (*)</translation></message>
    <message><source>All levels</source><translation>모든 수준</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>진단 스냅샷이 비어 있어도 오버레이에 변경이 없다는 뜻은 아닙니다.</translation></message>
    <message><source>Application path</source><translation>앱 경로</translation></message>
    <message><source>Application result</source><translation>적용 결과</translation></message>
    <message><source>Apply changes</source><translation>변경 적용</translation></message>
    <message><source>Architecture</source><translation>아키텍처</translation></message>
    <message><source>Automatic</source><translation>자동</translation></message>
    <message><source>Automatic file staging</source><translation>대기 파일 자동 저장</translation></message>
    <message><source>Automatic saving incomplete</source><translation>자동 저장 미완료</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>일반 종료 및 다시 시작 시 저장 가능한 대기 파일을 자동으로 저장합니다.</translation></message>
    <message><source>Available</source><translation>사용 가능</translation></message>
    <message><source>Back</source><translation>뒤로</translation></message>
    <message><source>Background service</source><translation>백그라운드 서비스</translation></message>
    <message><source>Binding</source><translation>바인딩</translation></message>
    <message><source>By drive letter</source><translation>드라이브 문자로</translation></message>
    <message><source>By volume ID</source><translation>볼륨 ID로</translation></message>
    <message><source>Cancel reading</source><translation>읽기 취소</translation></message>
    <message><source>Changes already committed are retained.</source><translation>이미 커밋한 변경은 유지됩니다.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>하위 키와 값을 포함하여 UWF 보호에서 제외할 레지스트리 키를 선택하세요.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>파일을 저장할 위치를 선택하세요.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>허용된 드라이브 또는 폴더 안의 위치를 선택하세요.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>다른 폴더를 선택하거나 접근 권한을 확인한 후 다시 시도하세요.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>다른 키를 선택하거나 접근 권한을 확인한 후 다시 시도하세요.</translation></message>
    <message><source>Clear filters</source><translation>필터 지우기</translation></message>
    <message><source>Clear logs</source><translation>로그 지우기</translation></message>
    <message><source>Clear logs?</source><translation>로그를 지울까요?</translation></message>
    <message><source>Collapse</source><translation>접기</translation></message>
    <message><source>Commit changes</source><translation>변경 커밋</translation></message>
    <message><source>Commit changes…</source><translation>변경 커밋…</translation></message>
    <message><source>Commit complete</source><translation>커밋 완료</translation></message>
    <message><source>Commit file changes?</source><translation>파일 변경을 커밋할까요?</translation></message>
    <message><source>Commit folder changes?</source><translation>폴더 변경을 커밋할까요?</translation></message>
    <message><source>Commit registry key changes?</source><translation>레지스트리 키 변경을 커밋할까요?</translation></message>
    <message><source>Commit registry value changes?</source><translation>레지스트리 값 변경을 커밋할까요?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>일반 종료 및 다시 시작 전에 대기 파일을 커밋합니다</translation></message>
    <message><source>Compatibility mode</source><translation>호환 모드</translation></message>
    <message><source>Configuration applied</source><translation>설정 적용됨</translation></message>
    <message><source>Configuration error</source><translation>설정 오류</translation></message>
    <message><source>Configured</source><translation>설정됨</translation></message>
    <message><source>Confirm before continuing</source><translation>계속하기 전에 확인</translation></message>
    <message><source>Connected and authenticated</source><translation>연결 및 인증됨</translation></message>
    <message><source>Connecting…</source><translation>연결 중…</translation></message>
    <message><source>Contact author</source><translation>작성자에게 연락</translation></message>
    <message><source>Continue editing</source><translation>계속 편집</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>다시 시작을 계속하면 저장하지 못한 파일의 변경이 삭제됩니다.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>종료를 계속하면 저장하지 못한 파일의 변경이 삭제됩니다.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Windows 종료와 연동</translation></message>
    <message><source>Copy all logs</source><translation>모든 로그 복사</translation></message>
    <message><source>Copy full report</source><translation>전체 보고서 복사</translation></message>
    <message><source>Copy name</source><translation>이름 복사</translation></message>
    <message><source>Copy path</source><translation>경로 복사</translation></message>
    <message><source>Copy selection</source><translation>선택 내용 복사</translation></message>
    <message><source>Copy version</source><translation>버전 복사</translation></message>
    <message><source>Counting staged files…</source><translation>대기 파일 수를 계산하는 중…</translation></message>
    <message><source>Create</source><translation>만들기</translation></message>
    <message><source>Critical %1 MB</source><translation>위험 %1 MB</translation></message>
    <message><source>Current filter</source><translation>현재 필터</translation></message>
    <message><source>Current run</source><translation>이번 실행</translation></message>
    <message><source>Current selection</source><translation>현재 선택</translation></message>
    <message><source>Current usage</source><translation>현재 사용량</translation></message>
    <message><source>Date modified</source><translation>수정한 날짜</translation></message>
    <message><source>Debug</source><translation>디버그</translation></message>
    <message><source>Delete and commit this file?</source><translation>이 파일을 삭제하고 커밋할까요?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>이 폴더를 삭제하고 커밋할까요?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>이 레지스트리 키를 삭제하고 커밋할까요?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>이 레지스트리 값을 삭제하고 커밋할까요?</translation></message>

    <message><source>Direct restart?</source><translation>바로 다시 시작할까요?</translation></message>
    <message><source>Direct shutdown?</source><translation>바로 종료할까요?</translation></message>
    <message><source>Discard changes</source><translation>변경 취소</translation></message>
    <message><source>Disk</source><translation>디스크</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>도메인 비밀 키 영구 저장</translation></message>
    <message><source>Done</source><translation>완료</translation></message>
    <message><source>Edit commands</source><translation>명령 편집</translation></message>
    <message><source>Editable</source><translation>편집 가능</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>필터를 활성화하고 다시 시작하면 실시간 사용량을 볼 수 있습니다</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>고급 모드는 Windows와 함께 시작되는 백그라운드 서비스를 추가합니다. 앱을 이동하거나 삭제하기 전에 이 서비스를 제거하세요.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>고급 모드를 사용할 수 없습니다. 대기 파일을 저장하려면 안전 종료 또는 안전 다시 시작을 사용하세요.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>HKLM\SOFTWARE\… 입력 후 Enter 키로 이동</translation></message>
    <message><source>Enter a file name</source><translation>파일 이름 입력</translation></message>
    <message><source>Enter a valid file name.</source><translation>올바른 파일 이름을 입력하세요.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>올바른 파일 또는 폴더 이름을 입력하세요.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>올바른 폴더 이름을 입력하세요.</translation></message>
    <message><source>Error</source><translation>오류</translation></message>
    <message><source>Error details</source><translation>오류 세부 정보</translation></message>
    <message><source>Error details and suggestions</source><translation>오류 세부 정보 및 제안</translation></message>
    <message><source>Expand</source><translation>펼치기</translation></message>
    <message><source>Export list</source><translation>목록 내보내기</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>로그를 지우지 못했습니다: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>로그를 지우지 못했습니다: 예기치 않은 오류.</translation></message>
    <message><source>File name</source><translation>파일 이름</translation></message>
    <message><source>File or folder</source><translation>파일 또는 폴더</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>파일 저장이 시작되어 취소할 수 없습니다.</translation></message>
    <message><source>File saving incomplete</source><translation>파일 저장 미완료</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>대기 파일 저장을 건너뜁니다. 보호된 경로의 커밋되지 않은 변경은 다시 시작 후 사라집니다.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>대기 파일 저장을 건너뜁니다. 보호된 경로의 커밋되지 않은 변경은 종료 후 사라집니다.</translation></message>
    <message><source>File type</source><translation>파일 형식</translation></message>
    <message><source>Files and folders</source><translation>파일 및 폴더</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>파일과 폴더를 함께 선택할 수 있습니다.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>저장 대기 파일은 저장되지 않습니다.</translation></message>
    <message><source>Filter is disabled</source><translation>필터가 비활성화되어 있습니다</translation></message>
    <message><source>Fit columns to contents</source><translation>내용에 맞게 열 너비 조정</translation></message>
    <message><source>Folder address</source><translation>폴더 주소</translation></message>
    <message><source>Folder name</source><translation>폴더 이름</translation></message>
    <message><source>Folders</source><translation>폴더</translation></message>
    <message><source>Full configuration</source><translation>전체 설정</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>앞으로 이 키에 기록하는 내용은 UWF로 보호되지 않습니다.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>앞으로 이 경로에 기록하는 내용은 UWF로 보호되지 않습니다.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>전역 레지스트리 제외 · 하위 키 포함</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>상위 폴더로 이동하거나 다른 위치를 선택하세요.</translation></message>
    <message><source>Home</source><translation>홈</translation></message>
    <message><source>Import configuration</source><translation>설정 가져오기</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>가져오기는 구성 초안을 업데이트합니다. 검토 및 적용으로 변경 사항을 시스템에 기록하세요.</translation></message>
    <message><source>Import results</source><translation>가져오기 결과</translation></message>
    <message><source>Imported</source><translation>가져옴</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>보고서에는 구성 초안에 대한 각 명령의 처리 결과가 표시됩니다. 행을 선택하면 전체 명령과 결과를 확인할 수 있습니다.</translation></message>
    <message><source>Imported: %1</source><translation>가져옴: %1</translation></message>
    <message><source>Information</source><translation>정보</translation></message>
    <message><source>Item details</source><translation>항목 세부 정보</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>대기 목록이 크면 저장에 더 오래 걸릴 수 있습니다.</translation></message>
    <message><source>License and copyright</source><translation>라이선스 및 저작권</translation></message>
    <message><source>Line</source><translation>행</translation></message>
    <message><source>Local disk (%1)</source><translation>로컬 디스크 (%1)</translation></message>
    <message><source>Log level</source><translation>로그 수준</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>로그를 읽지 못했습니다. 다시 시도하거나 로그 파일 접근 권한을 확인하세요.</translation></message>
    <message><source>Maximize</source><translation>최대화</translation></message>
    <message><source>Message details</source><translation>메시지 세부 정보</translation></message>
    <message><source>Minimize</source><translation>최소화</translation></message>
    <message><source>Module</source><translation>모듈</translation></message>
    <message><source>More</source><translation>더 보기</translation></message>
    <message><source>Move</source><translation>이동</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>앱이 있는 드라이브가 UWF로 보호되어 있다면 먼저 다시 시작한 후에도 앱 파일이 유지되는지 확인하세요. 대기 파일을 많이 저장하면 종료나 다시 시작에 더 오래 걸릴 수 있습니다.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>여러 파일을 선택할 수 있습니다.</translation></message>
    <message><source>Name and location</source><translation>이름 및 위치</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>이름이 지정된 값: %1 · 이 값만</translation></message>
    <message><source>Navigate to registry path</source><translation>레지스트리 경로로 이동</translation></message>
    <message><source>New folder</source><translation>새 폴더</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>새로 고치면 새 기록이 나타납니다.</translation></message>
    <message><source>Next-session configuration</source><translation>다음 세션 설정</translation></message>
    <message><source>No files to commit</source><translation>커밋할 파일이 없습니다</translation></message>
    <message><source>No logs yet</source><translation>아직 로그가 없습니다</translation></message>
    <message><source>No matching items</source><translation>일치하는 항목이 없습니다</translation></message>
    <message><source>No matching logs</source><translation>일치하는 로그가 없습니다</translation></message>
    <message><source>No overlay files found</source><translation>오버레이 파일을 찾지 못했습니다</translation></message>
    <message><source>No registry key selected</source><translation>선택한 레지스트리 키가 없습니다</translation></message>
    <message><source>None</source><translation>없음</translation></message>
    <message><source>Not configured</source><translation>설정되지 않음</translation></message>
    <message><source>Not connected</source><translation>연결되지 않음</translation></message>
    <message><source>Not installed</source><translation>설치되지 않음</translation></message>
    <message><source>Not written to the system</source><translation>시스템에 기록되지 않음</translation></message>
    <message><source>Off</source><translation>끔</translation></message>
    <message><source>On</source><translation>켬</translation></message>
    <message><source>One uwfmgr command per line</source><translation>한 줄에 uwfmgr 명령 하나</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>일반 사용자가 수정할 수 없는 고정된 폴더에 앱을 보관하세요. 권장 위치:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>대기 파일만 자동으로 저장됩니다. 보호된 경로의 다른 미커밋 변경은 다시 시작 후 사라집니다.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>대기 파일만 자동으로 저장됩니다. 보호된 경로의 다른 미커밋 변경은 종료 후 사라집니다.</translation></message>
    <message><source>Open</source><translation>열기</translation></message>
    <message><source>Open folder</source><translation>폴더 열기</translation></message>
    <message><source>Operating system</source><translation>운영 체제</translation></message>
    <message><source>Operation and security</source><translation>동작 및 보안</translation></message>
    <message><source>Original command</source><translation>원본 명령</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>오버레이 위험 임계값에 도달했습니다. 변경을 저장하고 안전하게 다시 시작하세요.</translation></message>
    <message><source>Overlay maximum size</source><translation>오버레이 최대 크기</translation></message>
    <message><source>Overlay type</source><translation>오버레이 유형</translation></message>
    <message><source>Overlay usage</source><translation>오버레이 사용량</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>오버레이 경고 임계값에 도달했습니다. 변경을 저장하고 안전하게 다시 시작하세요.</translation></message>
    <message><source>Overview</source><translation>개요</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>명령을 붙여넣거나 파일에서 불러오세요…</translation></message>
    <message><source>Pending changes</source><translation>보류 중인 변경</translation></message>
    <message><source>Pending removal</source><translation>제거 대기</translation></message>
    <message><source>Permissions and protection</source><translation>권한 및 보호</translation></message>
    <message><source>Privileges</source><translation>특권</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2개 항목 처리됨</translation></message>
    <message><source>Processing result</source><translation>처리 결과</translation></message>
    <message><source>Protected</source><translation>보호됨</translation></message>
    <message><source>Protected volumes</source><translation>보호된 볼륨</translation></message>
    <message><source>Protection and overlay</source><translation>보호 및 오버레이</translation></message>
    <message><source>Quick access</source><translation>빠른 액세스</translation></message>
    <message><source>Raw report</source><translation>원본 보고서</translation></message>
    <message><source>Read again</source><translation>다시 읽기</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>진단 스냅샷을 가져오려면 다시 읽으세요.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>설정을 읽으면 실시간 사용량을 볼 수 있습니다</translation></message>
    <message><source>Read-only</source><translation>읽기 전용</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>큰 오버레이를 읽는 데는 오래 걸릴 수 있습니다. 언제든 취소할 수 있습니다.</translation></message>
    <message><source>Reading canceled</source><translation>읽기 취소됨</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>이 위치의 파일과 하위 폴더를 읽는 중…</translation></message>
    <message><source>Reading folder</source><translation>폴더 읽는 중</translation></message>
    <message><source>Reading logs</source><translation>로그 읽는 중</translation></message>
    <message><source>Reading overlay</source><translation>오버레이 읽는 중</translation></message>
    <message><source>Ready</source><translation>준비됨</translation></message>
    <message><source>Ready to save</source><translation>저장 준비 완료</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>읽기가 끝나면 이번 실행의 기록이 여기에 나타납니다.</translation></message>
    <message><source>Recursive staging</source><translation>하위 폴더 포함 대기</translation></message>
    <message><source>Refresh folder</source><translation>폴더 새로 고침</translation></message>
    <message><source>Registry key or persistence setting</source><translation>레지스트리 키 또는 영구 저장 설정</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>현재 대상 처리가 끝나면 나머지 작업을 취소할 수 있습니다.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>다음 세션 제외 항목에서 이 키를 제거합니다.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>다음 세션 제외 항목에서 이 경로를 제거합니다.</translation></message>
    <message><source>Removed</source><translation>제거됨</translation></message>
    <message><source>Rename</source><translation>이름 바꾸기</translation></message>
    <message><source>Rename…</source><translation>이름 바꾸기…</translation></message>
    <message><source>Repair required</source><translation>복구 필요</translation></message>
    <message><source>Replace file</source><translation>파일 바꾸기</translation></message>
    <message><source>Replace the existing file?</source><translation>기존 파일을 바꿀까요?</translation></message>
    <message><source>Required configuration</source><translation>필수 설정</translation></message>
    <message><source>Restart anyway</source><translation>그래도 다시 시작</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>저장 후 다시 시작이 진행됩니다. 앱을 열어 두세요.</translation></message>
    <message><source>Restore</source><translation>복원</translation></message>
    <message><source>Retry</source><translation>다시 시도</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>종료를 계속하기 전에 실패 세부 정보를 확인하세요.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>다시 시작하기 전에 실패 세부 정보를 확인하세요.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>작업 세부 정보를 검토하세요. 이미 기록되었을 수 있는 변경을 다시 시도하기 전에 현재 시스템 상태를 읽으세요.</translation></message>
    <message><source>Review the result for each target.</source><translation>각 대상의 결과를 확인하세요.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>이 변경을 시스템에 기록하기 전에 검토하세요.</translation></message>
    <message><source>Running</source><translation>실행 중</translation></message>
    <message><source>Save</source><translation>저장</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>대기 파일을 디스크에 저장한 후 다시 시작합니다.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>대기 파일을 디스크에 저장한 후 종료합니다.</translation></message>
    <message><source>Saving staged files</source><translation>대기 파일 저장 중</translation></message>
    <message><source>Saving to disk</source><translation>디스크에 저장 중</translation></message>
    <message><source>Saving…</source><translation>저장 중…</translation></message>
    <message><source>Scanning staged paths</source><translation>대기 경로 검사 중</translation></message>
    <message><source>Search logs</source><translation>로그 검색</translation></message>
    <message><source>Search messages or modules</source><translation>메시지 또는 모듈 검색</translation></message>
    <message><source>Search paths</source><translation>경로 검색</translation></message>
    <message><source>Search this folder</source><translation>이 폴더 검색</translation></message>
    <message><source>Select a file or enter its name</source><translation>파일 선택 또는 이름 입력</translation></message>
    <message><source>Select a file to open.</source><translation>열 파일을 선택하세요.</translation></message>
    <message><source>Select a folder.</source><translation>폴더를 선택하세요.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>키를 선택하면 하위 트리 전체에, 이름이 지정된 값을 선택하면 해당 값에만 작업합니다.</translation></message>
    <message><source>Select an available subkey</source><translation>사용 가능한 하위 키 선택</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>파일만 선택하거나 폴더를 열어 내용을 살펴보세요.</translation></message>
    <message><source>Select files or folders</source><translation>파일 또는 폴더 선택</translation></message>
    <message><source>Select folder</source><translation>폴더 선택</translation></message>
    <message><source>Select folders only.</source><translation>폴더만 선택하세요.</translation></message>
    <message><source>Select registry key or value</source><translation>레지스트리 키 또는 값 선택</translation></message>
    <message><source>Select registry key or value to delete</source><translation>삭제할 레지스트리 키 또는 값 선택</translation></message>
    <message><source>Select the current folder</source><translation>현재 폴더 선택</translation></message>
    <message><source>Select this file</source><translation>이 파일 선택</translation></message>
    <message><source>Select this registry key</source><translation>이 레지스트리 키 선택</translation></message>
    <message><source>Select this registry value</source><translation>이 레지스트리 값 선택</translation></message>
    <message><source>Selected file</source><translation>선택한 파일</translation></message>
    <message><source>Selected folder</source><translation>선택한 폴더</translation></message>
    <message><source>Selection</source><translation>선택 항목</translation></message>
    <message><source>Service account</source><translation>서비스 계정</translation></message>
    <message><source>Service configuration needs repair</source><translation>서비스 설정을 복구해야 합니다</translation></message>
    <message><source>Service details</source><translation>서비스 세부 정보</translation></message>
    <message><source>Service name</source><translation>서비스 이름</translation></message>
    <message><source>Service status</source><translation>서비스 상태</translation></message>
    <message><source>Service stopped</source><translation>서비스 중지됨</translation></message>
    <message><source>Shut down anyway</source><translation>그래도 종료</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>저장 후 종료가 진행됩니다. 앱을 열어 두세요.</translation></message>
    <message><source>Size</source><translation>크기</translation></message>
    <message><source>Some changes could not be applied</source><translation>일부 변경을 적용하지 못했습니다</translation></message>
    <message><source>Some files could not be saved</source><translation>일부 파일을 저장하지 못했습니다</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>일부 하드웨어 정보를 읽지 못했습니다. 다른 진단 정보는 계속 보고 복사할 수 있습니다.</translation></message>
    <message><source>Source code</source><translation>소스 코드</translation></message>
    <message><source>Staged file</source><translation>저장 대기 파일</translation></message>
    <message><source>Staged path</source><translation>저장 대기 경로</translation></message>
    <message><source>Standard user</source><translation>표준 사용자</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Windows와 함께 자동 시작</translation></message>
    <message><source>Startup type</source><translation>시작 유형</translation></message>
    <message><source>Status unavailable</source><translation>상태 확인 불가</translation></message>
    <message><source>Stopped</source><translation>중지됨</translation></message>
    <message><source>Storage</source><translation>저장소</translation></message>
    <message><source>System</source><translation>시스템</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>UWF를 처음 활성화할 때의 시스템 변경</translation></message>
    <message><source>System memory</source><translation>시스템 메모리</translation></message>
    <message><source>System metadata</source><translation>시스템 메타데이터</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>시스템 메타데이터는 진단용이며 커밋할 수 없습니다.</translation></message>
    <message><source>Takes effect after restart</source><translation>다시 시작 후 적용</translation></message>
    <message><source>Target</source><translation>대상</translation></message>
    <message><source>Terminal Services license persistence</source><translation>터미널 서비스 라이선스 영구 저장</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>UWF 공급자가 열거를 완료하지 못했습니다. 오버레이가 크거나 시스템 리소스가 부족할 때 발생할 수 있습니다.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>설정을 시스템에 기록했습니다. 대부분의 변경은 다시 시작 후 적용됩니다.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>설정이 다음 세션 대상과 일치합니다. 전체 설정에서 명령을 보거나 내보내세요.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>현재 로그 기록을 지우며 복원할 수 없습니다.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>대상 폴더가 없습니다.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>고급 모드 서비스 상태를 읽지 못했습니다. 서비스 권한을 확인하고 다시 시도하세요.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>목록에 파일이 누락되거나 이미 커밋한 파일이 포함될 수 있습니다. 커밋 목록으로 사용하지 마세요.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>선택한 항목이 더 이상 없습니다: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>서비스가 설치되었지만 중지되어 있습니다. 자동 저장을 복원하려면 시작하세요.</translation></message>
    <message><source>This PC</source><translation>내 PC</translation></message>
    <message><source>This folder is empty</source><translation>이 폴더는 비어 있습니다</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>이 보고서에 계정 이름, 장치 식별자 및 파일 경로가 포함될 수 있습니다. 공유하기 전에 확인하세요.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>이 UTF-8 CMD 스크립트를 .cmd 파일로 저장한 후 명령 프롬프트에서 실행하세요. 내보내기에도 같은 스크립트를 사용합니다.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>다른 키워드나 로그 수준을 사용해 보세요.</translation></message>
    <message><source>Try another search term.</source><translation>다른 검색어를 사용해 보세요.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>필터가 활성화된 동안에는 유형과 최대 크기를 변경할 수 없습니다. 필터를 비활성화하고 먼저 다시 시작하세요.</translation></message>
    <message><source>UI agent</source><translation>UI 에이전트</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1줄</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>UWF 설정을 읽지 못했습니다. Windows에 Unified Write Filter가 설치되었는지 확인한 후 다시 읽으세요.</translation></message>
    <message><source>UWF feature</source><translation>UWF 기능</translation></message>
    <message><source>UWF filter</source><translation>UWF 필터</translation></message>
    <message><source>UWF is currently unavailable</source><translation>현재 UWF를 사용할 수 없습니다</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>UWF 안전 다시 시작을 바로 실행합니다.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>UWF 안전 종료를 바로 실행합니다.</translation></message>
    <message><source>Unable to access this folder</source><translation>이 폴더에 접근할 수 없습니다</translation></message>
    <message><source>Unable to access this registry key</source><translation>이 레지스트리 키에 접근할 수 없습니다</translation></message>
    <message><source>Unable to read logs</source><translation>로그를 읽을 수 없습니다</translation></message>
    <message><source>Unable to read service status</source><translation>서비스 상태를 읽을 수 없습니다</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>지금은 오버레이를 읽을 수 없습니다</translation></message>
    <message><source>Unable to read this folder.</source><translation>이 폴더를 읽을 수 없습니다.</translation></message>
    <message><source>Unable to select this item</source><translation>이 항목을 선택할 수 없습니다</translation></message>
    <message><source>Unavailable</source><translation>사용 불가</translation></message>
    <message><source>Undo removal</source><translation>제거 취소</translation></message>
    <message><source>Unprotected</source><translation>보호되지 않음</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>실패한 항목은 저장되지 않았습니다. 행을 선택하면 전체 경로와 원인을 볼 수 있습니다.</translation></message>
    <message><source>Up one folder</source><translation>상위 폴더로</translation></message>
    <message><source>Usage unavailable</source><translation>사용량 확인 불가</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>모든 하위 키를 포함한 전체 키 사용</translation></message>
    <message><source>Version %1</source><translation>버전 %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>앱 기록 및 진단 정보를 확인합니다.</translation></message>
    <message><source>View commands for these changes</source><translation>이 변경의 명령 보기</translation></message>
    <message><source>View failure details</source><translation>실패 세부 정보 보기</translation></message>
    <message><source>View overlay</source><translation>오버레이 보기</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>%1의 파일 진단 스냅샷을 표시합니다.</translation></message>
    <message><source>Volume %1 binding</source><translation>볼륨 %1 바인딩</translation></message>
    <message><source>Volume %1 protection</source><translation>볼륨 %1 보호</translation></message>
    <message><source>Volume identity</source><translation>볼륨 식별 정보</translation></message>
    <message><source>Warning</source><translation>경고</translation></message>
    <message><source>Warning %1 MB</source><translation>경고 %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>전체 키와 모든 하위 키 · 재귀적</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF가 쓰기 보호를 제공하며, 이 앱은 이를 설정하고 관리합니다.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>고급 모드를 사용하지 않을 때는 이 앱의 안전 종료 또는 안전 다시 시작으로 대기 파일을 저장하세요.</translation></message>
    <message><source>Write protection</source><translation>쓰기 보호</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>이 설정 변경을 시스템에 기록할까요? 대부분의 변경은 다시 시작 후 적용됩니다.</translation></message>
    <message><source>staged files</source><translation>저장 대기 파일</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr 명령</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>선택한 경로를 제거 예정으로 표시합니다. 적용 후 반영됩니다.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>적용 후 자동 파일 스테이징 목록에서 제거합니다.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>적용 후 자동 파일 스테이징 목록에 추가합니다.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>이 계획을 적용하면 저장된 스테이징 목록이 비워지고 더 이상 필요하지 않은 관련 레지스트리 제외 항목이 제거됩니다.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>파일 스테이징 변경 사항은 UWF Manager에서 적용해야 하며 uwfmgr 명령으로 내보낼 수 없습니다.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>파일 스테이징은 적용 후 저장됩니다</translation></message>
    <message><source>Overlay snapshot scope</source><translation>오버레이 스냅샷 범위</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>이 목록은 진단용입니다. 커밋해야 할 파일을 판단하는 데 사용할 수 없습니다.</translation></message>
    <message><source>NTFS volumes only</source><translation>NTFS 볼륨만 지원</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>오버레이가 크면 읽기가 느리거나 리소스 부족으로 실패할 수 있습니다.</translation></message>
    <message><source>Small files may be missing</source><translation>작은 파일이 누락될 수 있음</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>디스크 클러스터 크기(일반적으로 4 KB)보다 작은 파일은 목록에 표시되지 않을 수 있습니다.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>커밋 대기 상태를 나타내지 않음</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>이미 커밋된 파일, 제외 경로 및 다른 작업의 영향을 받은 항목도 표시될 수 있습니다.</translation></message>

    <message><source>Command processing result</source><translation>명령 처리 결과</translation></message>
    <message><source>Copy command</source><translation>명령 복사</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1개 분기를 읽지 못함</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1개 분기를 읽지 못했습니다. 발견한 내용으로 계속하거나 실패한 항목만 다시 시도하세요.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>파일 %1개 · 폴더 %2개</translation></message>
    <message><source>%1 values · %2 keys</source><translation>값 %1개 · 키 %2개</translation></message>
    <message><source>Attempts</source><translation>시도 횟수</translation></message>
    <message><source>Cancel remaining tasks</source><translation>남은 작업 취소</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>탐색에 성공한 내용만 커밋합니다. 읽지 못한 분기는 변경하지 않습니다.</translation></message>
    <message><source>Committing changes</source><translation>변경 사항 커밋 중</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>완료된 삭제와 커밋은 되돌릴 수 없습니다.</translation></message>
    <message><source>Continue…</source><translation>계속…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>발견한 대상만 삭제하고 커밋합니다. 읽지 못한 분기와 상위 폴더 또는 키는 유지됩니다.</translation></message>
    <message><source>Deleting and committing</source><translation>삭제 및 커밋 중</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>삭제와 커밋은 되돌릴 수 없습니다. 선택한 범위만 처리하며 대기 중인 UWF 구성은 적용하지 않습니다.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>작업 범위를 확인하고 있습니다. 읽지 못한 분기를 기록하고 나머지 내용의 탐색을 계속합니다.</translation></message>
    <message><source>Export traversal records</source><translation>탐색 기록 내보내기</translation></message>
    <message><source>No actionable content</source><translation>작업 가능한 내용 없음</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>작업 가능한 대상을 찾지 못했습니다. 삭제하거나 커밋한 내용은 없습니다.</translation></message>
    <message><source>No changes have been committed.</source><translation>아직 커밋된 변경 사항 없음</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>아직 삭제하거나 커밋한 내용 없음</translation></message>
    <message><source>Path</source><translation>경로</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>선택한 범위를 영구적으로 삭제하고 삭제 결과를 보호된 볼륨에 저장합니다.</translation></message>
    <message><source>Preparation complete</source><translation>준비 완료</translation></message>
    <message><source>Preparing commit</source><translation>커밋 준비 중</translation></message>
    <message><source>Preparing deletion and commit</source><translation>삭제 및 커밋 준비 중</translation></message>
    <message><source>Process only successfully traversed content</source><translation>탐색에 성공한 내용만 처리</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>선택한 범위 내에서만 탐색하도록 재분석 지점은 건너뜁니다.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>커밋은 다시 시작해도 되돌릴 수 없습니다. 선택한 범위만 처리하며 대기 중인 UWF 구성은 적용하지 않습니다.</translation></message>
    <message><source>Retry failed items</source><translation>실패한 항목 다시 시도</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>선택한 범위의 변경 사항을 보호된 볼륨에 저장합니다.</translation></message>
    <message><source>Stopping the operation</source><translation>작업 중지 중</translation></message>
    <message><source>Successful and failed records are retained</source><translation>성공 및 실패 기록 유지됨</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>폴더가 비어 있지 않습니다. 처리하지 않은 내용은 유지되었습니다.</translation></message>
    <message><source>The operation could not be completed</source><translation>작업을 완료하지 못함</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>레지스트리 키에 새 값이 있습니다. 처리하지 않은 내용은 유지되었습니다.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>레지스트리 키에 하위 키가 남아 있습니다. 처리하지 않은 내용은 유지되었습니다.</translation></message>
    <message><source>The target no longer exists.</source><translation>대상이 더 이상 존재하지 않습니다.</translation></message>
    <message><source>Traversal complete</source><translation>탐색 완료</translation></message>
    <message><source>Traversal completed with unread content</source><translation>탐색 완료, 일부 내용을 읽지 못함</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>탐색 기록이 유지되며 내보낼 수 있습니다.</translation></message>
    <message><source>Traverse again</source><translation>다시 탐색</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>다시 탐색하거나 이 창을 닫으세요.</translation></message>
    <message><source>Traversing the folder</source><translation>폴더 탐색 중</translation></message>
    <message><source>Traversing the registry</source><translation>레지스트리 탐색 중</translation></message>
    <message><source>Unable to prepare the operation</source><translation>작업을 준비할 수 없음</translation></message>
    <message><source>View results</source><translation>결과 보기</translation></message>

    <message><source>All changes committed</source><translation>모든 변경 사항 커밋 완료</translation></message>
    <message><source>Some changes committed</source><translation>일부 변경 사항 커밋 완료</translation></message>
    <message><source>No changes could be committed</source><translation>변경 사항을 커밋하지 못했습니다</translation></message>
    <message><source>No changes to commit</source><translation>커밋할 변경 사항이 없습니다</translation></message>
    <message><source>Commit interrupted</source><translation>커밋 중단됨</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1개 항목을 처리하지 않았습니다.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1개의 레지스트리 작업이 실패했습니다.</translation></message>
    <message><source>The folder does not exist.</source><translation>폴더가 존재하지 않습니다.</translation></message>
    <message><source>Access denied.</source><translation>액세스가 거부되었습니다.</translation></message>
    <message><source>An item with this name already exists.</source><translation>같은 이름의 항목이 이미 있습니다.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>폴더를 만들 수 없습니다. 이름과 권한을 확인하세요.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>항목의 이름을 변경할 수 없습니다. 이름과 권한을 확인하세요.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>이 브라우저에서는 이름 변경을 지원하지 않습니다.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>이 계획을 연 후 파일 스테이징 초안이 변경되었습니다. 새 계획을 검토하세요.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>이 계획을 연 후 저장된 파일 스테이징 목록이 변경되었습니다. 새로 고침하고 변경 사항을 검토하세요.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>저장된 파일 스테이징 목록이 승인한 계획과 일치하지 않습니다.</translation></message>
    <message><source>Export configuration</source><translation>구성 내보내기</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>현재 테마, 언어 및 HUB 표시 상태를 이 애플리케이션과 같은 폴더의 UWF.ini에 저장합니다.</translation></message>
    <message><source>Configuration saved</source><translation>구성이 저장됨</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>현재 인터페이스 설정을 다음 위치에 저장했습니다:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini가 이미 있습니다. 기존 파일은 변경되지 않았습니다:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>UWF.ini를 저장할 수 없습니다:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>바탕 화면</translation></message>
    <message><source>Downloads</source><translation>다운로드</translation></message>
    <message><source>Documents</source><translation>문서</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>파일 저장이 중지되었습니다. 오류를 해결한 후 다시 시도하세요.</translation></message>
    <message><source>No net change for this setting</source><translation>이 설정의 최종 값은 가져오기 전과 같습니다</translation></message>
    <message><source>Final draft %1 → %2</source><translation>초안 최종 값: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>초안 최종 값: %1 → %2(요청 값: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>요청 값: %1 → %2. 이후 명령으로 대체되었습니다. 초안 최종 값 → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>%1 초안을 업데이트했습니다</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>볼륨 %1의 보호 설정 초안을 업데이트했습니다 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>제거할 항목을 선택하세요.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>현재 선택한 항목을 복원할 수 없습니다.</translation></message>
</context>
</TS>
