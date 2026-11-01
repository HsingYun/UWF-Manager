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
    <message><source>Disabled</source><comment>service startup type</comment><translation>사용 안 함</translation></message>
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
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>다음 세션에서 이 볼륨을 보호합니다. 보호된 경로에 대한 쓰기는 오버레이로 리디렉션되며, 다시 시작 후 유지 여부는 오버레이 설정에 따라 달라집니다.</translation></message>
    <message><source>Current session</source><translation>현재 세션</translation></message>
    <message><source>Next session</source><translation>다음 세션</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>볼륨 식별 방식입니다. 드라이브 문자는 간단하지만 디스크 추가·제거로 문자가 바뀌면 연결이 끊어집니다. 볼륨 ID는 문자가 바뀌어도 유지됩니다.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF에서 지원하지 않는 볼륨: </translation></message>
    <message><source>Global settings</source><translation>전역 설정</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>현재 세션의 UWF 필터 상태입니다(읽기 전용).</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>다음 세션에서 UWF를 사용합니다. 보호된 경로에 대한 쓰기는 오버레이로 리디렉션되며, 다시 시작 후 유지 여부는 오버레이 설정에 따라 달라집니다.</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>오버레이 유형이나 최대 용량을 변경하려면:
1. UWF를 사용 안 함으로 설정합니다.
2. 장치를 다시 시작합니다.
3. 이 설정을 변경합니다.</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM은 시스템 메모리를 사용합니다. Disk는 시스템 드라이브를 사용하며 더 큰 오버레이를 지원합니다. 영구 오버레이를 사용하면 다시 시작 후에도 Disk 오버레이 데이터를 유지할 수 있습니다.</translation></message>
    <message><source>Type</source><translation>유형</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>RAM 오버레이 용량은 시스템 메모리의 제한을 받습니다. Disk 오버레이에는 최소 1024 MB와 시스템 드라이브의 충분한 여유 공간이 필요합니다.</translation></message>
    <message><source>Maximum capacity</source><translation>최대 용량</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>최대 용량 · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>경고 임계값</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>오버레이 사용량이 이 값에 도달하면 이벤트 로그에 경고 수준 알림을 기록합니다. 두 이벤트가 모두 활성화된 경우 이 값은 위험 임계값보다 작아야 합니다. 0으로 설정하면 경고 이벤트만 비활성화됩니다.</translation></message>
    <message><source>Critical threshold</source><translation>위험 임계값</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>오버레이 사용량이 이 값에 도달하면 이벤트 로그에 위험 수준 알림을 기록합니다. 두 이벤트가 모두 활성화된 경우 이 값은 경고 임계값보다 커야 합니다. 0으로 설정하면 위험 이벤트만 비활성화됩니다.</translation></message>
    <message><source>Overlay</source><translation>오버레이</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>지원하지 않는 드라이브 유형입니다(로컬 고정 디스크만 지원).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>볼륨 용량이 UWF의 보호 볼륨당 한도인 16 TiB를 초과합니다.</translation></message>

    <message><source>%1 free / %2</source><translation>여유 %1 / %2</translation></message>
    <message><source>Commit</source><translation>커밋</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>오버레이의 변경 사항을 디스크 또는 레지스트리에 커밋합니다. 이 작업은 되돌릴 수 없습니다.</translation></message>
    <message><source>Commit file changes…</source><translation>파일 변경 커밋…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>파일을 선택하여 오버레이의 변경 사항을 디스크에 커밋합니다.</translation></message>
    <message><source>Commit folder changes…</source><translation>폴더 변경 커밋…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>폴더를 선택하여 폴더 안의 모든 파일에 대한 오버레이 변경 사항을 디스크에 커밋합니다.</translation></message>
    <message><source>Delete and commit file…</source><translation>파일 삭제 및 커밋…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>삭제할 파일을 선택하고 삭제 내용을 디스크에 커밋합니다.</translation></message>
    <message><source>Delete and commit folder…</source><translation>폴더 삭제 및 커밋…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>폴더를 선택하여 해당 폴더와 모든 내용을 재귀적으로 삭제하고 삭제 내용을 디스크에 커밋합니다.</translation></message>
    <message><source>Commit registry changes…</source><translation>레지스트리 변경 커밋…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>키 경로와 값 이름(선택)을 입력해 변경을 커밋합니다. 값 이름을 비우면 키와 모든 하위 키의 모든 값을 커밋합니다.</translation></message>
    <message><source>Delete and commit registry…</source><translation>레지스트리 삭제 및 커밋…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>레지스트리 키와 값 이름(선택 사항)을 입력하세요. 값 이름을 비워 두면 키와 모든 하위 키 및 값을 삭제한 후 삭제 내용을 커밋합니다.</translation></message>
    <message><source>File exclusions</source><translation>파일 제외</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>UWF 보호에서 제외되는 파일과 폴더입니다. 항목을 두 번 클릭하면 경로를 복사합니다.</translation></message>
    <message><source>Registry exclusions</source><translation>레지스트리 제외</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>레지스트리 제외 항목은 전역 설정이며, Windows가 설치된 볼륨에만 표시됩니다.</translation></message>
    <message><source>File staging</source><translation>파일 스테이징</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>파일 또는 폴더를 자동 커밋 목록에 추가합니다.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>안전 종료 또는 다시 시작 전에 자동으로 커밋할 파일을 하나 이상 선택하세요.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>안전 종료 또는 다시 시작 전에 파일을 재귀적으로 커밋할 폴더를 선택하세요.</translation></message>
    <message><source>Select files for automatic commit</source><translation>자동으로 커밋할 파일 선택</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>재귀적으로 자동 커밋할 폴더 선택</translation></message>
    <message><source>Cannot stage this path</source><translation>이 경로를 자동 커밋에 추가할 수 없음</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>이 경로는 UWF 파일 제외 항목과 겹칩니다. 스테이징 대상 경로를 추가하기 전에 다음 제외 항목을 제거하세요:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>이 경로는 파일 스테이징 항목과 겹칩니다. 제외 항목을 추가하기 전에 다음 스테이징 대상 경로를 제거하세요:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>이 경로는 기존 파일 스테이징 항목과 겹칩니다</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>파일 스테이징에 저장된 항목이 있는 동안에는 이 레지스트리 제외 항목이 필요합니다</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>경로가 없거나 예상한 파일 유형이 아닙니다:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>경로를 검사할 수 없습니다:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>재분석 지점은 자동 커밋에 추가할 수 없습니다:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>경로에 드라이브 문자가 없어 파일 스테이징에 추가할 수 없습니다:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>볼륨 %1의 경로입니다. 해당 볼륨의 파일 스테이징 탭에서 추가하세요.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>볼륨 루트는 추가할 수 없습니다. 전체 볼륨이 재귀적으로 커밋되기 때문입니다.</translation></message>
    <message><source>File staging could not be saved</source><translation>파일 스테이징 목록을 저장하지 못했습니다</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>갱신을 완료할 수 없어 파일 스테이징 목록을 다시 불러왔습니다:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>파일 스테이징을 사용할 수 없습니다: %1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>스테이징 항목 %1개 · 파일 %2개 · 폴더 %3개</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>파일 스테이징 목록을 다시 시작한 후에도 유지하려면 이 레지스트리 제외 항목이 필요합니다.</translation></message>
    <message><source>Built-in</source><translation>기본 제공</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>UWF 필터가 꺼져 오버레이 변경이 누적되지 않았으므로 커밋할 내용이 없습니다.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>%1 파일 시스템은 파일별 커밋을 지원하지 않습니다.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>이 볼륨은 UWF에서 지원하지 않습니다.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>%1 파일 시스템은 파일별 커밋을 지원하지 않습니다. 레지스트리 커밋만 가능합니다(레지스트리 제외는 전역 설정).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>현재 UWF로 보호되지 않는 볼륨이므로 커밋할 파일 변경이 없습니다.</translation></message>
    <message><source>Select a file to commit to disk</source><translation>디스크에 커밋할 파일 선택</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>디스크에 커밋할 폴더 선택</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>삭제를 커밋할 파일 선택</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>삭제를 커밋할 폴더 선택</translation></message>
    <message><source>Name</source><translation>이름</translation></message>
    <message><source>Data</source><translation>데이터</translation></message>
    <message><source>(Default)</source><translation>(기본값)</translation></message>
    <message><source>(value not set)</source><translation>(값 설정 안 됨)</translation></message>
    <message><source>(empty)</source><translation>(비어 있음)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>미리 보기 불가(값이 너무 큼)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>이 볼륨의 오버레이에 현재 캐시된 파일을 보여 주는 진단 보기를 엽니다.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>오버레이 파일 목록은 NTFS 볼륨에서만 지원합니다.</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>UWF 필터가 꺼져 있어 어느 볼륨에도 조회할 오버레이가 없습니다.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>UWF로 보호되지 않는 볼륨이므로 검사할 오버레이가 없습니다.</translation></message>
    <message><source>Failed: %1</source><translation>실패: %1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>이전 열거 작업이 아직 실행 중일 수 있습니다. 아직 새 읽기를 시작할 수 없습니다.</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>오버레이 파일을 열거하는 동안 UWF 서비스를 사용할 수 없습니다.</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>메모리가 부족하거나 시스템이 이 작업을 지원하지 않습니다. 이 기능은 NTFS 볼륨에서만 사용할 수 있습니다. 메모리를 많이 사용하는 앱을 닫고 다시 시도하세요.</translation></message>
    <message><source>Raw error: %1</source><translation>원본 오류: %1</translation></message>
    <message><source>Export overlay file list</source><translation>오버레이 파일 목록 내보내기</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>텍스트 파일 (*.txt);;모든 파일 (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>명령 파일 (*.cmd);;모든 파일 (*)</translation></message>
    <message><source>Export failed</source><translation>내보내기 실패</translation></message>
    <message><source>Export finished</source><translation>내보내기 완료</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>실패: 파일 열기 (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>실패: 파일 쓰기 (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>항목 %1개를 다음 위치에 저장했습니다:
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>볼륨 전체(%1)는 제외할 수 없습니다.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>페이징 파일, 스왑 파일 및 최대 절전 모드 파일은 제외 항목으로 추가할 수 없습니다.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>전체 디렉터리는 제외할 수 없습니다: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>전체 디렉터리는 제외할 수 없습니다: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>전체 디렉터리는 제외할 수 없습니다: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>중요 시스템 파일은 제외할 수 없습니다: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>사용자별 레지스트리 파일 NTUSER.DAT는 제외할 수 없습니다.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>레지스트리 키는 비워 둘 수 없습니다.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>레지스트리 키는 역슬래시로 시작할 수 없습니다.</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>레지스트리 경로에는 널 문자를 포함할 수 없습니다.</translation></message>
    <message><source>A registry key component is too long.</source><translation>레지스트리 키 구성 요소가 너무 깁니다.</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>레지스트리 경로가 UWF의 중첩 제한을 초과합니다.</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>파일 또는 폴더를 제외 목록에 추가합니다.</translation></message>
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
    <message><source>File: %1</source><translation>파일: %1</translation></message>
    <message><source>Folder: %1</source><translation>폴더: %1</translation></message>
    <message><source>Registry: %1</source><translation>레지스트리: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>기본적으로 제외됩니다. 언제든 변경할 수 있습니다.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>현재 세션: %1
다음 세션: %2</translation></message>
    <message><source>Pending change: %1</source><translation>대기 중인 변경: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>통합 쓰기 필터(UWF) 관리자</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>UWF 현재 세션 상태와 다음 세션 구성을 다시 읽습니다.</translation></message>
    <message><source>Review and apply</source><translation>검토 및 적용</translation></message>
    <message><source>Review all pending changes and apply them in one batch.</source><translation>대기 중인 변경을 모두 검토하고 한 번에 적용합니다.</translation></message>
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
    <message><source>The system did not find the target.</source><translation>시스템에서 대상을 찾지 못했습니다.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>시스템이 매개 변수를 거부했습니다(잘못된 경로 또는 인수).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>작업에 실패했습니다. 파일이 열려 있거나 탐색기에서 폴더를 보고 있는 등 다른 프로세스가 대상을 사용할 수 있습니다. 사용하는 프로그램을 닫고 다시 시도하세요.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>작업에 실패했습니다. 자세한 내용은 로그를 확인하세요.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>작업이 거부되었습니다(코드 %1).</translation></message>
    <message><source>Unknown cause.</source><translation>원인을 알 수 없습니다.</translation></message>
    <message><source>Commit canceled</source><translation>커밋 취소됨</translation></message>

    <message><source>Succeeded</source><translation>성공</translation></message>
    <message><source>Skipped</source><translation>건너뜀</translation></message>
    <message><source>Failed</source><translation>실패</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>성공: %1개, 실패: %2개.</translation></message>

    <message><source>Existed before</source><translation>삭제 전 존재</translation></message>
    <message><source>Exists after</source><translation>삭제 후 존재</translation></message>
    <message><source>Error code</source><translation>오류 코드</translation></message>
    <message><source>Reason</source><translation>이유</translation></message>
    <message><source>Previous page</source><translation>이전 페이지</translation></message>
    <message><source>First page</source><translation>첫 페이지</translation></message>
    <message><source>Last page</source><translation>마지막 페이지</translation></message>
    <message><source>Next page</source><translation>다음 페이지</translation></message>
    <message><source>Copy selected rows</source><translation>선택 행 복사</translation></message>
    <message><source>Copy all</source><translation>모두 복사</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>파일 스테이징에 저장된 항목이 있는 동안에는 이 레지스트리 제외 항목이 필요합니다. 제거하기 전에 파일 스테이징 목록을 비우세요.</translation></message>
    <message><source>Copy current page</source><translation>현재 페이지 복사</translation></message>
    <message><source>No pending changes</source><translation>대기 중인 변경 없음</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (레지스트리 제외 · 전역 설정.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>볼륨 %1의 UWF 보호 설정, 파일 제외 및 파일 스테이징으로 전환합니다.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF 네임스페이스를 사용할 수 없습니다</translation></message>
    <message><source>Review and apply changes</source><translation>변경 검토 및 적용</translation></message>
    <message><source>Confirm apply</source><translation>적용 확인</translation></message>
    <message><source>Result</source><translation>결과</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ 필터: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ 경고 임계값 설정: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ 위험 임계값 설정: %1 MB</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ 유형/최대 용량을 적용하지 못했습니다. UWF가 사용 중입니다. 먼저 UWF를 사용 안 함으로 설정하고 다시 시작하세요.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ 오버레이 유형 설정: %1</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ 오버레이 최대 용량을 %1 MB로 설정했습니다</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ 볼륨 %1 · UWF 보호: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ 볼륨 %1 · 연결 기준: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ 볼륨 %1 · 파일 제외 추가: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ 볼륨 %1 · 파일 제외 제거: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ 레지스트리 제외 추가: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ 레지스트리 제외 제거: %1</translation></message>
    <message><source>Time</source><translation>시간</translation></message>
    <message><source>Level</source><translation>수준</translation></message>

    <message><source>Message</source><translation>메시지</translation></message>
    <message><source>Committing staged files</source><translation>스테이징 대상 파일 커밋 중</translation></message>
    <message><source>No additional error details were provided.</source><translation>추가 오류 정보가 없습니다.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>자동 파일 스테이징이 예기치 않게 중단되었습니다:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>알 수 없는 오류로 자동 파일 스테이징이 중단되었습니다.</translation></message>
    <message><source>Safe shutdown failed</source><translation>안전 종료: 실패</translation></message>
    <message><source>Shutdown failed: %1</source><translation>시스템 종료: 실패 (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>안전 다시 시작: 실패</translation></message>
    <message><source>Direct restart failed</source><translation>바로 다시 시작: 실패</translation></message>
    <message><source>Direct shutdown failed</source><translation>바로 종료: 실패</translation></message>
    <message><source>Restart failed: %1</source><translation>다시 시작: 실패 (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>파일 스테이징 목록을 읽지 못했습니다:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>알 수 없는 오류로 파일 스테이징 목록을 읽지 못했습니다.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>발견한 파일: %1 · 커밋됨: %2 · 건너뛴 파일: %3 · 건너뛴 항목: %4 · 실패: %5</translation></message>
    <message><source>File staging preparation</source><translation>파일 스테이징 준비</translation></message>
    <message><source>Reparse points are not supported.</source><translation>재분석 지점을 지원하지 않습니다.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>로컬 볼륨의 절대 경로만 추가할 수 있습니다.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>로컬 볼륨의 절대 경로만 추가할 수 있습니다.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>자동 커밋 경로를 검사할 수 없습니다.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>등록한 파일의 경로 유형이 변경되었습니다.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>등록한 폴더의 경로 유형이 변경되었습니다.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>등록한 폴더의 내용을 완전히 열거하지 못했습니다.</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>볼륨 루트를 재귀적으로 처리할 수 없습니다.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>UWF 공급자가 커밋을 거부했습니다.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>알 수 없는 오류로 커밋하지 못했습니다.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>추가 실패 %1건을 로그에 기록했습니다.</translation></message>



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
%2</source><translation>명령 %1개를 다음 위치로 내보냈습니다:
%2</translation></message>
    <message><source>Import</source><translation>가져오기</translation></message>
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>uwfmgr 명령을 초안으로 가져옵니다. 검토 및 적용에서 확인한 후에만 변경 사항이 기록됩니다.</translation></message>
    <message><source>Load from file…</source><translation>파일에서 불러오기…</translation></message>
    <message><source>Load recommended configuration</source><translation>권장 구성 불러오기</translation></message>
    <message><source>Choose recommended configuration</source><translation>권장 구성 선택</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>추가할 권장 구성 그룹을 선택하세요. 가져오기 전에 개별 명령을 검토하거나 삭제할 수 있습니다.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Microsoft 공식 문서에서 가져왔습니다.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Microsoft 권장 UWF 제외 항목</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>가져오기 전에 이 권장 사항을 검토하세요.</translation></message>
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
    <message><source>Command import is not ready.</source><translation>명령 가져오기가 아직 준비되지 않았습니다.</translation></message>
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
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>변경 사항 %1개가 아직 적용되지 않았습니다.

변경 사항을 버리고 계속할까요?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>불완전한 uwfmgr 명령</translation></message>
    <message><source>Missing size argument (MB)</source><translation>누락된 인수: 크기 (MB)</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>용량은 0 이상의 정수(MB)여야 합니다</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>누락된 인수: 오버레이 · 유형 (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>잘못된 값: 오버레이 · 유형: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>누락된 인수: 볼륨 (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>볼륨은 C: 같은 드라이브 문자여야 합니다</translation></message>
    <message><source>Missing path argument</source><translation>누락된 인수: 경로</translation></message>
    <message><source>Missing registry key argument</source><translation>누락된 인수: 레지스트리 · 키</translation></message>
    <message><source>Unclosed double quote in command</source><translation>명령의 큰따옴표가 닫히지 않았습니다</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>예상하지 못한 추가 인수: %1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>이 uwfmgr 명령은 가져올 수 없습니다.</translation></message>
    <message><source>Already in the target state — no-op</source><translation>이미 목표 상태이므로 작업이 필요 없습니다</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>경로가 이 볼륨에 없거나 파일 제외를 지원하지 않는 볼륨입니다(예: exFAT / ReFS)</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>이 경로는 제외 항목으로 추가할 수 없습니다.</translation></message>
    <message><source>Invalid size value: %1</source><translation>잘못된 값: 크기: %1</translation></message>
    <message><source>maximum capacity</source><translation>최대 용량</translation></message>
    <message><source>warning threshold</source><translation>경고 임계값</translation></message>
    <message><source>critical threshold</source><translation>위험 임계값</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>전역 구성을 사용할 수 없어 명령을 대기 중인 변경 사항에 추가하지 않았습니다.</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>알 수 없는 볼륨 %1(일치하는 볼륨 구성이 없음)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>볼륨 %1: 이미 목표 상태이므로 작업이 필요 없습니다</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>경로 %1에 드라이브 문자가 없어 해당 볼륨 탭을 식별할 수 없습니다.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>볼륨 %1: UWF 지원 안 됨</translation></message>
    <message><source>file exclusion</source><translation>파일 제외</translation></message>
    <message><source>registry exclusion</source><translation>레지스트리 제외</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>레지스트리 제외를 사용할 수 있는 레지스트리 탭이 없습니다</translation></message>
    <message><source>Unsupported command</source><translation>명령: 지원 안 됨</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>오버레이 위젯을 표시하거나 숨깁니다.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: 사용 중</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: 사용 안 함</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: 알 수 없음</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>사용 중 %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>고급 모드</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Windows 종료 및 다시 시작과 파일 스테이징을 연동합니다.</translation></message>
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
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>현재 서비스는 Windows 종료 사전 알림을 받을 수 없습니다.</translation></message>
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
%1</source><translation>서비스 구성은 적용되었지만 UWF 영구 유지에서 다음 문제가 보고되었습니다:
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
    <message><source>Show main window</source><translation>메인 창 표시</translation></message>
    <message><source>Restore default position</source><translation>기본 위치로 복원</translation></message>
    <message><source>Show overlay widget</source><translation>오버레이 위젯 표시</translation></message>
    <message><source>Hide overlay widget</source><translation>오버레이 위젯 숨기기</translation></message>
    <message><source>Exit application</source><translation>앱 종료</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>실패: 레지스트리 값 읽기 (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>오버레이 파일 열거: 알 수 없는 오류.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ 실패: 필터 %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ 실패: 필터 상태 읽기</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ 실패: 경고 임계값 설정</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ 실패: 위험 임계값 설정</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ 실패: 오버레이 상태 읽기</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ 실패: 오버레이 유형 설정</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ 최대 용량을 설정하지 못했습니다</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ 실패: 오버레이 구성 읽기</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ 실패: 볼륨 %2 보호 %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ 실패: 볼륨 %1 연결 설정</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ 볼륨 %1 · 실패: 파일 제외 추가 %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ 볼륨 %1 · 실패: 파일 제외 제거 %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ 실패: 볼륨 구성 읽기</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ 실패: 레지스트리 제외 추가 %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ 실패: 레지스트리 제외 제거 %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ 실패: 레지스트리 영구 저장 스위치 갱신</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ 실패: 레지스트리 필터 읽기</translation></message>
    <message><source>UWF is not available</source><translation>UWF를 사용할 수 없습니다</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>대상 상태 읽기: 알 수 없는 오류.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>알 수 없는 오류.</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>대상 상태를 확인할 수 없어 작업 결과를 알 수 없습니다: %1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>시스템이 삭제 요청을 수락했지만 확인 결과 대상이 여전히 존재합니다.</translation></message>
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
볼륨 보호, 오버레이, 제외 항목 및 스테이징 대상 파일을 관리합니다.</translation></message>
    <message><source>Snapshot details</source><translation>스냅샷 세부 정보</translation></message>
    <message><source>Account</source><translation>계정</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>스크립트나 텍스트에서 UWF 설정을 추가합니다.</translation></message>
    <message><source>Add exclusion</source><translation>제외 항목 추가</translation></message>
    <message><source>Add files or folders</source><translation>파일 또는 폴더 추가</translation></message>
    <message><source>Add selected items</source><translation>선택한 항목 추가</translation></message>
    <message><source>Added</source><translation>추가됨</translation></message>
    <message><source>Administrator</source><translation>관리자</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>선택한 작업이 완료되었습니다. UWF는 제외된 대상을 건너뛸 수 있습니다.</translation></message>
    <message><source>All files (*)</source><translation>모든 파일 (*)</translation></message>
    <message><source>All levels</source><translation>모든 수준</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>진단 스냅샷이 비어 있어도 오버레이에 변경이 없다는 뜻은 아닙니다.</translation></message>
    <message><source>Application path</source><translation>앱 경로</translation></message>
    <message><source>Application result</source><translation>적용 결과</translation></message>
    <message><source>Apply changes</source><translation>변경 적용</translation></message>
    <message><source>Architecture</source><translation>아키텍처</translation></message>
    <message><source>Automatic</source><translation>자동</translation></message>
    <message><source>Automatic staged file commits</source><translation>스테이징 대상 파일 자동 커밋</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>자동 파일 커밋 미완료</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>Windows 종료 및 다시 시작 시 조건에 맞는 스테이징 대상 파일을 자동으로 커밋합니다.</translation></message>
    <message><source>Available</source><translation>사용 가능</translation></message>
    <message><source>Back</source><translation>뒤로</translation></message>
    <message><source>Background service</source><translation>백그라운드 서비스</translation></message>
    <message><source>Binding</source><translation>바인딩</translation></message>
    <message><source>By drive letter</source><translation>드라이브 문자로</translation></message>
    <message><source>By volume ID</source><translation>볼륨 ID로</translation></message>
    <message><source>Cancel reading</source><translation>읽기 취소</translation></message>
    <message><source>Completed operations cannot be undone.</source><translation>완료된 작업은 되돌릴 수 없습니다.</translation></message>
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
    <message><source>Commit file changes?</source><translation>파일 변경을 커밋할까요?</translation></message>
    <message><source>Commit folder changes?</source><translation>폴더 변경을 커밋할까요?</translation></message>
    <message><source>Commit registry key changes?</source><translation>레지스트리 키 변경을 커밋할까요?</translation></message>
    <message><source>Commit registry value changes?</source><translation>레지스트리 값 변경을 커밋할까요?</translation></message>
    <message><source>Compatibility mode</source><translation>호환 모드</translation></message>
    <message><source>Configuration applied</source><translation>설정 적용됨</translation></message>
    <message><source>Configuration error</source><translation>설정 오류</translation></message>
    <message><source>Configured</source><translation>설정됨</translation></message>
    <message><source>Confirm before continuing</source><translation>계속하기 전에 확인</translation></message>
    <message><source>Connected and authenticated</source><translation>연결 및 인증됨</translation></message>
    <message><source>Connecting…</source><translation>연결 중…</translation></message>
    <message><source>Contact author</source><translation>작성자에게 연락</translation></message>
    <message><source>Continue editing</source><translation>계속 편집</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>다시 시작을 계속하면 커밋되지 않은 파일 변경 사항이 손실될 수 있습니다.</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>종료를 계속하면 커밋되지 않은 파일 변경 사항이 손실될 수 있습니다.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Windows 종료와 연동</translation></message>
    <message><source>Copy all logs</source><translation>모든 로그 복사</translation></message>
    <message><source>Copy full report</source><translation>전체 보고서 복사</translation></message>
    <message><source>Copy name</source><translation>이름 복사</translation></message>
    <message><source>Copy path</source><translation>경로 복사</translation></message>
    <message><source>Copy selection</source><translation>선택 내용 복사</translation></message>
    <message><source>Copy version</source><translation>버전 복사</translation></message>
    <message><source>Counting staged files…</source><translation>스테이징 대상 파일 수를 계산하는 중…</translation></message>
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
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>고급 모드를 사용할 수 없습니다. 안전 종료 또는 안전 다시 시작으로 스테이징 대상 파일을 커밋하세요.</translation></message>
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
    <message><source>File commits have started and cannot be canceled.</source><translation>파일 커밋이 시작되어 취소할 수 없습니다.</translation></message>
    <message><source>File commits incomplete</source><translation>파일 커밋 미완료</translation></message>
    <message><source>File type</source><translation>파일 형식</translation></message>
    <message><source>Files and folders</source><translation>파일 및 폴더</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>파일과 폴더를 함께 선택할 수 있습니다.</translation></message>
    <message><source>Filter is disabled</source><translation>필터가 비활성화되어 있습니다</translation></message>
    <message><source>Fit columns to contents</source><translation>내용에 맞게 열 너비 조정</translation></message>
    <message><source>Folder address</source><translation>폴더 주소</translation></message>
    <message><source>Folder name</source><translation>폴더 이름</translation></message>
    <message><source>Folders</source><translation>폴더</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>이 키를 다음 세션의 제외 목록에 추가합니다.</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>이 경로를 다음 세션의 제외 목록에 추가합니다.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>전역 레지스트리 제외 · 하위 키 포함</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>상위 폴더로 이동하거나 다른 위치를 선택하세요.</translation></message>
    <message><source>Home</source><translation>홈</translation></message>
    <message><source>Import configuration</source><translation>설정 가져오기</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>가져오기는 초안을 갱신합니다. 검토 및 적용에서 변경 사항을 확인하면 시스템에 기록됩니다.</translation></message>
    <message><source>Import results</source><translation>가져오기 결과</translation></message>
    <message><source>Imported</source><translation>가져옴</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>목록에는 구성 초안에 대한 명령 처리 결과가 표시됩니다. 행을 선택하면 전체 명령과 세부 정보를 볼 수 있습니다.</translation></message>
    <message><source>Imported: %1</source><translation>가져옴: %1</translation></message>
    <message><source>Information</source><translation>정보</translation></message>
    <message><source>Item details</source><translation>항목 세부 정보</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>많은 스테이징 대상 파일을 커밋하면 시간이 더 걸릴 수 있습니다.</translation></message>
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
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>앱이 있는 드라이브가 UWF로 보호되어 있다면 먼저 다시 시작한 후에도 앱 파일이 유지되는지 확인하세요. 많은 스테이징 대상 파일을 커밋하면 종료나 다시 시작에 시간이 더 걸릴 수 있습니다.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>여러 파일을 선택할 수 있습니다.</translation></message>
    <message><source>Name and location</source><translation>이름 및 위치</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>이름이 지정된 값: %1 · 이 값만</translation></message>
    <message><source>Navigate to registry path</source><translation>레지스트리 경로로 이동</translation></message>
    <message><source>New folder</source><translation>새 폴더</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>새로 고치면 새 기록이 나타납니다.</translation></message>
    <message><source>Next-session configuration</source><translation>다음 세션 구성</translation></message>
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
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>스테이징 대상 파일만 자동으로 커밋됩니다. 그 밖의 커밋되지 않은 변경 사항은 다시 시작 후 손실될 수 있습니다.</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>스테이징 대상 파일만 자동으로 커밋됩니다. 그 밖의 커밋되지 않은 변경 사항은 종료 후 손실될 수 있습니다.</translation></message>
    <message><source>Open</source><translation>열기</translation></message>
    <message><source>Open folder</source><translation>폴더 열기</translation></message>
    <message><source>Operating system</source><translation>운영 체제</translation></message>
    <message><source>Operation and security</source><translation>동작 및 보안</translation></message>
    <message><source>Original command</source><translation>원본 명령</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>오버레이 위험 임계값에 도달했습니다. 변경을 저장하고 안전하게 다시 시작하세요.</translation></message>
    <message><source>Maximum overlay capacity</source><translation>오버레이 최대 용량</translation></message>
    <message><source>Overlay type</source><translation>오버레이 유형</translation></message>
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
    <message><source>Files to commit</source><translation>커밋할 파일</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>읽기가 끝나면 이번 실행의 기록이 여기에 나타납니다.</translation></message>

    <message><source>Refresh folder</source><translation>폴더 새로 고침</translation></message>
    <message><source>Registry key or persistence setting</source><translation>레지스트리 키 또는 영구 저장 설정</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>취소를 요청하면 현재 작업이 끝나는 대로 남은 작업을 중지합니다.</translation></message>
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
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>커밋 후 다시 시작이 계속됩니다. 앱을 열어 두세요.</translation></message>
    <message><source>Restore</source><translation>복원</translation></message>
    <message><source>Retry</source><translation>다시 시도</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>종료를 계속하기 전에 실패 세부 정보를 확인하세요.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>다시 시작하기 전에 실패 세부 정보를 확인하세요.</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>일부 변경 사항은 이미 기록되었을 수 있습니다. 다시 시도하기 전에 시스템 상태를 새로 고치세요.</translation></message>
    <message><source>Review the result for each target.</source><translation>각 대상의 결과를 확인하세요.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>이 변경을 시스템에 기록하기 전에 검토하세요.</translation></message>
    <message><source>Running</source><translation>실행 중</translation></message>
    <message><source>Save</source><translation>저장</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>스테이징 대상 파일을 디스크에 커밋한 후 다시 시작합니다.</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>스테이징 대상 파일을 디스크에 커밋한 후 종료합니다.</translation></message>

    <message><source>Committing to disk</source><translation>디스크에 커밋 중</translation></message>
    <message><source>Committing…</source><translation>커밋 중…</translation></message>
    <message><source>Scanning staged paths</source><translation>스테이징 대상 경로 검사 중</translation></message>
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
    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>커밋 후 종료가 계속됩니다. 앱을 열어 두세요.</translation></message>
    <message><source>Size</source><translation>크기</translation></message>
    <message><source>Some changes could not be applied</source><translation>일부 변경을 적용하지 못했습니다</translation></message>
    <message><source>Some files could not be committed</source><translation>일부 파일을 커밋하지 못했습니다</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>일부 하드웨어 정보를 읽지 못했습니다. 다른 진단 정보는 계속 보고 복사할 수 있습니다.</translation></message>
    <message><source>Source code</source><translation>소스 코드</translation></message>

    <message><source>Staged path</source><translation>스테이징 대상 경로</translation></message>
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
    <message><source>Takes effect when applied</source><translation>적용 시 반영</translation></message>
    <message><source>Used when UWF starts</source><translation>UWF 시작 시 사용</translation></message>
    <message><source>Mixed activation times</source><translation>서로 다른 반영 시점</translation></message>
    <message><source>Target</source><translation>대상</translation></message>
    <message><source>Terminal Services license persistence</source><translation>터미널 서비스 라이선스 영구 저장</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>UWF 공급자가 열거를 완료하지 못했습니다. 오버레이가 크거나 시스템 리소스가 부족할 때 발생할 수 있습니다.</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>구성이 시스템에 기록되었습니다.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>현재 로그 기록을 지우며 복원할 수 없습니다.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>대상 폴더가 없습니다.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>고급 모드 서비스 상태를 읽지 못했습니다. 서비스 권한을 확인하고 다시 시도하세요.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>목록에 파일이 누락되거나 이미 커밋한 파일이 포함될 수 있습니다. 커밋 목록으로 사용하지 마세요.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>선택한 항목이 더 이상 없습니다: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>서비스가 설치되었지만 중지되어 있습니다. 자동 파일 커밋을 재개하려면 서비스를 시작하세요.</translation></message>
    <message><source>This PC</source><translation>내 PC</translation></message>
    <message><source>This folder is empty</source><translation>이 폴더는 비어 있습니다</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>이 보고서에 계정 이름, 장치 식별자 및 파일 경로가 포함될 수 있습니다. 공유하기 전에 확인하세요.</translation></message>

    <message><source>Try another keyword or log level.</source><translation>다른 키워드나 로그 수준을 사용해 보세요.</translation></message>
    <message><source>Try another search term.</source><translation>다른 검색어를 사용해 보세요.</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>오버레이 유형이나 최대 용량을 변경하기 전에 UWF를 사용 안 함으로 설정하고 다시 시작하세요.</translation></message>
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
    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>일부 항목이 완료되지 않았거나 확인되지 않았습니다. 항목을 선택하면 전체 경로와 세부 정보를 볼 수 있습니다.</translation></message>
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
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>고급 모드를 사용하지 않을 때는 이 앱의 안전 종료 또는 안전 다시 시작으로 스테이징 대상 파일을 커밋하세요.</translation></message>
    <message><source>Write protection</source><translation>쓰기 보호</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>이 구성 변경 사항을 시스템에 기록하시겠습니까?</translation></message>
    <message><source>staged files</source><translation>스테이징 대상 파일</translation></message>
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
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>탐색에 성공한 대상만 삭제하고 커밋합니다. 읽지 못한 분기와 상위 폴더 또는 키는 유지됩니다.</translation></message>
    <message><source>Deleting and committing</source><translation>삭제 및 커밋 중</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>삭제와 커밋은 되돌릴 수 없습니다. 선택한 범위만 처리하며 대기 중인 UWF 구성은 적용하지 않습니다.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>작업 범위를 확인하고 있습니다. 읽지 못한 분기를 기록하고 나머지 내용의 탐색을 계속합니다.</translation></message>
    <message><source>Export traversal records</source><translation>탐색 기록 내보내기</translation></message>
    <message><source>No actionable content</source><translation>작업 가능한 내용 없음</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>작업 가능한 대상을 찾지 못했습니다. 삭제하거나 커밋한 내용은 없습니다.</translation></message>
    <message><source>No changes have been committed.</source><translation>아직 커밋된 변경 사항 없음</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>아직 삭제하거나 커밋한 내용 없음</translation></message>
    <message><source>Path</source><translation>경로</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>선택한 내용을 영구적으로 삭제하고 삭제 내용을 커밋합니다.</translation></message>
    <message><source>Preparation complete</source><translation>준비 완료</translation></message>
    <message><source>Preparing commit</source><translation>커밋 준비 중</translation></message>
    <message><source>Preparing deletion and commit</source><translation>삭제 및 커밋 준비 중</translation></message>
    <message><source>Process only successfully traversed content</source><translation>탐색에 성공한 내용만 처리</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>선택한 범위 내에서만 탐색하도록 재분석 지점은 건너뜁니다.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>커밋은 다시 시작해도 되돌릴 수 없습니다. 선택한 범위만 처리하며 대기 중인 UWF 구성은 적용하지 않습니다.</translation></message>
    <message><source>Retry failed items</source><translation>실패한 항목 다시 시도</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>선택한 범위의 변경 사항을 커밋합니다.</translation></message>
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

    <message><source>All operations succeeded</source><translation>모든 작업 성공</translation></message>
    <message><source>Some operations succeeded</source><translation>일부 작업 성공</translation></message>
    <message><source>No operations succeeded</source><translation>성공한 작업 없음</translation></message>
    <message><source>No targets to process</source><translation>처리할 대상 없음</translation></message>
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
    <message><source>Export interface settings</source><translation>인터페이스 설정 내보내기</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>현재 테마, 언어 및 오버레이 위젯 표시 상태를 이 앱과 같은 위치의 UWF.ini에 저장합니다.</translation></message>
    <message><source>Interface settings saved</source><translation>인터페이스 설정이 저장되었습니다</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>파일 스테이징이 중지되었습니다. 오류를 해결한 후 다시 시도하세요.</translation></message>
    <message><source>No net change for this setting</source><translation>이 설정의 최종 값은 가져오기 전과 같습니다</translation></message>
    <message><source>Final draft %1 → %2</source><translation>초안 최종 값: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>초안 최종 값: %1 → %2(요청 값: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>요청 값: %1 → %2. 이후 명령으로 대체되었습니다. 초안 최종 값 → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>%1 초안을 업데이트했습니다</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>볼륨 %1의 보호 설정 초안을 업데이트했습니다 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>제거할 항목을 선택하세요.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>현재 선택한 항목을 복원할 수 없습니다.</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>여유 공간 통과</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>영구 오버레이</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>다음 시작 시 영구 데이터 지우기</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>읽기 전용 미디어 모드</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>디스크 여유 공간에 쓰기를 저장하여 예약 오버레이 사용량을 줄입니다. RAM(권장하지 않음)과 Disk(권장)를 지원합니다.</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>Disk 오버레이의 쓰기를 다시 시작 후에도 유지합니다. RAM 오버레이에서는 사용할 수 없습니다.</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>영구 오버레이를 초기화합니다. 다음 시작 시 쓰기를 지워 오버레이를 사용하도록 설정한 시점의 상태로 복원합니다</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>관리자 권한이 필요합니다. 현재는 오버레이 구성을 볼 수만 있습니다.</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>구성을 일시적으로 편집할 수 없습니다. 현재 작업이 끝날 때까지 기다리세요.</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>오버레이 구성을 읽을 수 없습니다. 새로 고친 후 편집하세요.</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>이 시스템은 다음 시작 시 영구 데이터 지우기를 지원하지 않습니다.</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>현재 지우기 요청을 알 수 없습니다. 구성을 새로 고친 후 편집하세요.</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>이 요청을 변경하려면 현재 세션에서 UWF 및 Disk 영구 오버레이가 활성화되어 있어야 합니다.</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>다음 시작 시 데이터를 지우려면 Disk 영구 오버레이를 유지해야 합니다. 지우기 요청을 취소하거나 영구 오버레이를 유지하세요.</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>영구 오버레이에는 Disk가 필요합니다. 영구 오버레이를 끄거나 Disk를 선택하세요.</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>읽기 전용 미디어 모드에는 RAM이 필요합니다. 이 모드를 끄거나 RAM을 선택하세요.</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>현재 세션에서 UWF가 활성화되어 있습니다. 이 설정을 변경하려면 UWF를 비활성화하고 다시 시작하세요.</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>구성이 변경되었습니다. 이 대화 상자를 닫았다가 다시 열어 편집하세요.</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>오버레이 고급 설정</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>현재 오버레이</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>다음 오버레이</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>초안에 호환되지 않거나 사용할 수 없는 오버레이 설정이 있습니다. 편집으로 돌아가 오버레이 구성을 수정하세요.</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>다음 시작 시 지우기가 예약되어 있습니다. 영구 오버레이를 끄기 전에 지우기 요청을 취소하세요.</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>다음 세션에서 Disk 영구 오버레이를 유지하도록 설정되어 있지 않습니다. 지우기를 요청하기 전에 유지하도록 설정하세요.</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>영구 오버레이는 Disk에서만 사용할 수 있지만, 다음 세션의 오버레이는 RAM입니다.</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>읽기 전용 미디어 모드에는 RAM이 필요하지만, 다음 세션의 오버레이는 Disk입니다.</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>현재 영구 오버레이 상태를 확인할 수 없습니다. 구성을 다시 읽으세요.</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>적용하면 다음 시작 시 반영됩니다.</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>지우기 예약됨</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>지우기 예약 안 됨</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>다음 시작 시 한 번만 지운 후 데이터를 계속 유지합니다. 현재 세션은 즉시 지워지지 않습니다.</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>다음 시작 시 지우기 예약을 취소하고 영구 오버레이 데이터를 유지합니다.</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>영구 오버레이 초기화 · 일회성 요청</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>처리 중…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>작업이 완료될 때까지 기다려 주세요.</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · 다음 세션</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>설정으로 돌아가기</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>변경 조건</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>변경됨</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>구성 충돌 확인</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>구성 충돌</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>비활성화 조건</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>현재 세션에서 UWF 및 Disk 영구 오버레이가 활성화되어 있어야 합니다. 데이터를 지우려면 다음 세션에서도 Disk 영구 오버레이를 유지해야 합니다.</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>활성화 조건</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>기능</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>기능 설명</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>%1 알아보기</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>초안을 유지하기 전에 다음 구성 충돌을 해결하세요.</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>되돌리기</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>반영 시점</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>현재 이 설정을 변경할 수 없습니다</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>현재 세션에서 UWF가 꺼져 있고, 다음 세션의 오버레이가 Disk로 설정되어 있어야 합니다.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>현재 세션에서 UWF가 꺼져 있고, 다음 세션의 오버레이가 RAM으로 설정되어 있어야 합니다.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>현재 세션에서 UWF가 꺼져 있어야 합니다. 영구 오버레이를 끄기 전에 예약된 영구 데이터 지우기 요청을 취소하세요.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>현재 세션에서 UWF가 꺼져 있어야 합니다. 다음 세션에서만 꺼지도록 설정한 경우에도 편집하려면 다시 시작해야 합니다.</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>사용 조건</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>세부 정보 보기</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>보기 전용</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>%1 변경이 제한되는 이유</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>현재 세션에서 UWF가 활성화되어 있어 이러한 오버레이 설정을 변경할 수 없습니다. 변경 사항을 되돌리거나 UWF를 비활성화하고 다시 시작하세요.</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>변경 조건 보기</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>구성 변경 사항이 기록되지 않았습니다. 다시 시도할 수 있습니다.</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>구성을 변경할 필요가 없었습니다.</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>도메인 컴퓨터 계정 비밀($MACHINE.ACC)은 “도메인 비밀 키 영구 저장” 설정을 사용하세요.</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>이 키 자체는 제외할 수 없습니다. 하위 키를 선택하세요.</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>레지스트리 제외 항목은 HKEY_LOCAL_MACHINE(HKLM) 아래의 비어 있지 않은 하위 키를 지정해야 합니다.</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>이 스크립트에는 uwfmgr 명령으로 표현할 수 있는 설정만 포함됩니다. 다음 항목은 포함되지 않습니다:</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>볼륨 바인딩</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>드라이브 문자로 안정적으로 지정할 수 없는 설정</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>명령 스크립트</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>구성이 다음 세션의 대상과 일치합니다. “명령 스크립트”에서 지원되는 설정을 확인하거나 내보낼 수 있습니다.</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>경로나 이름에 제어 문자가 포함된 설정</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>생략된 설정에 종속된 명령</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>UWF 제외 설정으로 인해 이 대상의 변경 사항을 확실하게 커밋할 수 없습니다. 충돌하는 제외 항목을 제거하고 다시 시작한 후 커밋하세요.</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>UWF 제외 설정으로 인해 파일 스테이징 목록을 확실하게 커밋할 수 없습니다. 충돌하는 제외 항목을 제거하고 다시 시작한 후 이 변경 사항을 적용하세요.</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF는 이 키에 대한 작업이 지정한 레지스트리 대상을 유지함을 보장할 수 없습니다.</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>레지스트리 제외 항목을 제거하기 전에 저장된 파일 스테이징 목록이 비어 있는지 확인합니다.</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>확인 대기 중</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>비어 있음</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>시스템 볼륨이 현재 UWF로 보호되지 않아 커밋할 레지스트리 변경 사항이 없습니다.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>레지스트리 변경 사항은 이미 영구 저장되어 UWF 커밋이 필요하지 않습니다.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>레지스트리 변경 사항은 이미 영구 저장되어 UWF가 이 삭제를 수행할 수 없습니다.</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>UWF 제외 경로에는 백슬래시를 사용해야 합니다.</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>파일 경로에는 널 문자를 포함할 수 없습니다.</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>파일 제외 항목을 추가하기 전에 이 초안에서 볼륨 보호를 활성화하세요.</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>제외 경로에 UWF가 지원하지 않는 문자가 포함되어 있습니다.</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>제외 경로가 UWF의 길이 제한을 초과합니다.</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>HORM이 활성화된 동안에는 파일 및 레지스트리 변경 내용을 커밋할 수 없습니다.</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>HORM이 활성화된 동안에는 레지스트리에 쓸 수 없습니다.</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>읽기 전용 미디어 모드가 활성화된 동안에는 레지스트리 쓰기를 영구적으로 저장할 수 없습니다.</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>읽기 전용 미디어 모드가 활성화된 동안에는 파일 및 레지스트리 변경 내용을 커밋할 수 없습니다.</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>메타데이터를 포함한 물리적 미디어 쓰기를 가로챕니다. 제외 항목도 보호를 우회하지 않습니다. 파일 및 레지스트리 변경 내용을 개별적으로 커밋할 수 없습니다. Full Commit은 별도의 전역 작업입니다.</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>HORM이 활성화된 동안에는 UWF 설정을 변경할 수 없습니다.</translation></message>
    <message><source>Default value · this value only</source><translation>기본값 · 이 값만</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>파일 변경 사항이 이미 영구 저장되어 UWF 커밋이 필요하지 않습니다.</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>오버레이 사용량을 마지막으로 가져온 시간: %1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>확인된 오버레이 저장소의 크기 또는 유형이 쓰기 가능한 범위를 벗어납니다</translation></message>
    <message><source>Overlay (last observed)</source><translation>오버레이(마지막 확인)</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>오버레이 사용량을 새로 고치지 못했습니다.
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>오버레이 사용량을 새로 고치지 못했습니다. %1에 마지막으로 가져온 정보를 표시합니다.
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>경로를 로컬 볼륨으로 확인할 수 없습니다.</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF: 사용(마지막 확인)</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>파일 변경 사항이 이미 영구 저장되어 UWF가 이 삭제를 수행할 수 없습니다.</translation></message>
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>%1 GB 사용 가능</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>관리자 권한이 필요합니다.</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>작업이 진행 중입니다. 잠시 기다려 주세요.</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>이번 시작 이후 모든 보호된 볼륨에서 추가, 수정 및 삭제된 내용의 저장을 시도합니다. 파일과 시스템 설정이 모두 포함되며, 일부 변경 내용만 선택할 수는 없습니다.</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>현재 모드에서는 자동 저장을 사용할 수 없음</translation></message>
    <message><source>Cannot be undone once started</source><translation>시작한 후에는 취소할 수 없음</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>디스크 오버레이 저장 위치 선택</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>디스크 오버레이를 저장할 위치를 선택하세요.</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>일시적으로 설정을 변경할 수 없습니다.</translation></message>
    <message><source>Configured, inactive this session</source><translation>설정됨, 현재 세션에서는 비활성</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>설정됨, 다시 시작한 후 적용</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>설정되었으며 다시 시작한 후 적용됩니다. 이번 시작에서 사용 중인 오버레이는 새 위치로 이동하지 않습니다.</translation></message>
    <message><source>Configure…</source><translation>설정…</translation></message>
    <message><source>Current UWF</source><translation>현재 UWF</translation></message>
    <message><source>Currently configured storage volume</source><translation>현재 설정된 저장 볼륨</translation></message>
    <message><source>Default behavior</source><translation>기본 동작</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>전체 볼륨 커밋을 실행하기 전에 HORM을 사용하지 않도록 설정하고 다시 시작하세요.</translation></message>
    <message><source>Disabled · restart required</source><translation>사용 안 함 · 다시 시작 필요</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>HORM을 사용하지 않도록 설정해도 UWF나 읽기 전용 미디어 모드는 꺼지지 않으며 보호된 디스크의 변경 내용도 저장되지 않습니다. 다시 시작한 후 사용 가능한 작업을 확인하세요.</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>HORM을 사용하도록 설정하기 전에 UWF를 사용하도록 설정하고 다시 시작하세요.</translation></message>

    <message><source>Full volume commit</source><translation>전체 볼륨 커밋</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>전체 볼륨 커밋 모드(다시 시작할 때까지 활성화)</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>전체 볼륨 커밋 모드가 이미 활성화되어 있습니다. 다시 사용하려면 컴퓨터를 다시 시작하세요.</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>전체 볼륨 커밋이 시작되었습니다. HORM을 사용하도록 설정하기 전에 다시 시작하세요.</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>전체 볼륨 커밋은 읽기 전용 미디어 모드가 활성화된 경우에만 사용할 수 있습니다.</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>전체 볼륨 커밋 모드는 다시 시작할 때까지 활성 상태를 유지합니다. 새 변경 내용은 직접 저장됩니다. 개별 커밋은 계속 사용할 수 없습니다.</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>이 시스템에서는 전체 볼륨 커밋을 사용할 수 없습니다.</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM이 활성화되어 있습니다. UWF 설정 변경과 개별 커밋을 사용할 수 없습니다. HORM을 사용하지 않도록 설정하고 다시 시작한 후 변경하세요.</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM이 비활성화되어 있습니다. UWF 설정을 변경하기 전에 컴퓨터를 다시 시작하세요.</translation></message>
    <message><source>HORM is not available on this system.</source><translation>이 시스템에서는 HORM을 사용할 수 없습니다.</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM을 사용하려면 현재 RAM 오버레이가 읽기 전용 미디어 모드로 설정되어 있어야 합니다.</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM을 사용하려면 현재 RAM 오버레이가 필요합니다.</translation></message>

    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>Disk 모드에서는 시스템 구현에 따라 이 옵션이 기본으로 켜질 수 있습니다. 설정에 표시된 현재 상태를 확인하세요.</translation></message>
    <message><source>Inactive this session</source><translation>현재 세션에서 비활성</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>다시 시작할 때까지 이후 변경 내용도 저장됨</translation></message>

    <message><source>Maintenance and recovery</source><translation>유지 관리 및 복구</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>계속하기 전에 현재 상태가 유지하려는 상태인지 확인하세요.</translation></message>

    <message><source>Next startup: %1</source><translation>다음 시작: %1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>적절한 저장 볼륨이 없습니다. 볼륨 목록을 새로 고치세요.</translation></message>
    <message><source>Not enabled</source><translation>사용 설정되지 않음</translation></message>
    <message><source>Not selected</source><translation>선택되지 않음</translation></message>
    <message><source>Not supported</source><translation>지원되지 않음</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>시작한 후에는 컴퓨터를 다시 시작할 때까지 새 변경 내용이 디스크에 직접 기록됩니다.</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>저장 볼륨은 한 번에 하나만 사용할 수 있습니다. 새 볼륨을 선택하면 이전 위치가 대체되고 다음 오버레이가 Disk로 설정됩니다.</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>오버레이 용량(현재 설정 유지)</translation></message>
    <message><source>Overlay configuration</source><translation>오버레이 설정</translation></message>

    <message><source>Processing…</source><translation>처리 중…</translation></message>
    <message><source>Read state again</source><translation>상태 다시 읽기</translation></message>
    <message><source>Read the current UWF state first.</source><translation>먼저 현재 UWF 상태를 읽으세요.</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>읽기 전용 미디어 모드에서는 개별 커밋이나 스테이징된 파일의 자동 저장을 지원하지 않습니다.</translation></message>
    <message><source>Reading state…</source><translation>상태 읽는 중…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>HORM을 다시 사용하도록 설정하기 전에 다시 시작하고 현재 상태를 읽으세요.</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>시작할 때마다 같은 바탕 화면과 실행 중인 프로그램을 복원합니다.</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>사용 가능한 커밋 작업과 실행 조건을 확인하세요.</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>모든 보호된 볼륨의 변경 내용 저장</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>모든 보호된 볼륨의 변경 내용을 저장합니다. 계속하기 전에 영향을 확인하세요.</translation></message>


    <message><source>State unknown</source><translation>상태 알 수 없음</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>저장에 성공한 변경 내용은 다시 시작해도 되돌릴 수 없습니다.</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>저장에 성공한 변경 내용은 유지됨</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>HORM 상태를 알 수 없습니다. 계속하기 전에 상태를 다시 읽으세요.</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>HORM 상태를 알 수 없습니다. 계속하기 전에 새로 고치세요.</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>커밋 또는 HORM 상태를 알 수 없습니다. 계속하기 전에 새로 고치세요.</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>현재 UWF 상태를 읽지 못했습니다. 계속하기 전에 새로 고치세요.</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>현재 오버레이가 RAM 읽기 전용 미디어 모드를 사용함</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>현재 보호 모드를 읽지 못했습니다. 상태를 새로 고치세요.</translation></message>
    <message><source>The result could not be confirmed</source><translation>결과를 확인하지 못함</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>이 제외 항목은 설정되어 있지만 현재 세션에서는 보호를 우회하지 않습니다.</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>여유 공간 통과와 영구 오버레이 설정도 변경될 수 있습니다. 다시 시작하기 전에 고급 설정을 확인하세요.</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>현재 세션에서 UWF 보호가 활성화됨</translation></message>
    <message><source>%1 (will be replaced)</source><translation>%1(대체 예정)</translation></message>
    <message><source>Add full volume commit to the draft?</source><translation>전체 볼륨 커밋을 초안에 추가할까요?</translation></message>
    <message><source>Add to draft</source><translation>초안에 추가</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>모든 보호된 볼륨에 적용되며, 이 초안을 적용할 때만 시작됩니다.</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>이 초안을 적용하면 전체 볼륨 커밋이 시작됩니다. 저장된 변경 내용은 다시 시작해도 되돌릴 수 없으며, 다시 시작할 때까지 새 변경 내용도 저장됩니다.</translation></message>
    <message><source>Configure storage location</source><translation>저장 위치 설정</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>이 시스템에서는 저장 위치를 설정할 수 없습니다.</translation></message>
    <message><source>Current: %1</source><translation>현재: %1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>저장 위치를 설정하기 전에 HORM을 사용하지 않도록 설정하고 다시 시작하세요.</translation></message>
    <message><source>Disable saved-state startup?</source><translation>저장된 상태로 시작하는 기능을 끌까요?</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>초안: %1 · 적용 필요</translation></message>
    <message><source>Enable saved-state startup?</source><translation>저장된 상태로 시작하는 기능을 켤까요?</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>전체 볼륨 커밋(적용 대기 중)</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>전체 볼륨 커밋이 초안에 있습니다. ‘검토 및 적용’에서 확인한 후에만 시작됩니다.</translation></message>
    <message><source>Hibernate once / resume many</source><translation>한 번 최대 절전 / 여러 번 복원</translation></message>
    <message><source>Not started</source><translation>시작되지 않음</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>보류 중인 변경 내용에서 전체 볼륨 커밋을 제거합니다.</translation></message>
    <message><source>Save draft</source><translation>초안 저장</translation></message>

    <message><source>Will start when applied</source><translation>적용 시 시작됨</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>탑재된 고정 디스크가 보호 검사를 통과하지 못했습니다. HORM 활성화 전에 목록의 디스크를 확인하세요.</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>UWF 적용 조건을 충족하고 계속 탑재된 고정 디스크를 검사합니다. 탑재되지 않은 파티션은 건너뛸 수 있습니다.</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>HORM 사전 조건을 확인할 수 없습니다. 계속하기 전에 상태를 다시 읽으세요.</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>탑재된 고정 디스크 보호 검사</translation></message>
    <message><source>Not using a Disk overlay</source><translation>Disk 오버레이를 사용하지 않음</translation></message>
    <message><source>Selected storage volume</source><translation>선택한 저장 볼륨</translation></message>
    <message><source>System drive default (%1)</source><translation>시스템 드라이브 기본값 (%1)</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>이번 부팅 중에 보호 상태가 변경되었습니다. HORM 활성화 전에 다시 시작하세요.</translation></message>
    <message><source>Unable to read</source><translation>읽을 수 없음</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>임시 변경 사항을 저장할 디스크를 선택하세요. 다음 부팅 때 적용됩니다.</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>저장 위치를 설정하기 전에 UWF 보호를 해제하고 다시 시작하세요.</translation></message>
    <message><source>Met</source><translation>충족</translation></message>
    <message><source>Not met</source><translation>미충족</translation></message>
    <message><source>Partition</source><translation>파티션</translation></message>


    <message><source>Protect all</source><translation>모두 보호</translation></message>
    <message><source>Hibernation is not supported</source><translation>최대 절전 모드를 지원하지 않음</translation></message>
    <message><source>Hibernation is not enabled</source><translation>최대 절전 모드가 활성화되지 않음</translation></message>
    <message><source>Fast startup only</source><translation>빠른 시작만 지원</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>이 시스템에서는 볼륨 일괄 보호를 사용할 수 없습니다.</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>볼륨 보호를 변경하기 전에 HORM을 비활성화하고 다시 시작하세요.</translation></message>
    <message><source>Setup and operation</source><translation>설정 및 사용</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. 구성 준비</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>현재 UWF가 비활성화된 상태에서 다음 시작에 사용할 RAM 오버레이, 읽기 전용 미디어 모드, 볼륨 보호 및 UWF 활성화를 구성하세요. 초안을 적용하고 S4 최대 절전 모드와 완전한 형식의 최대 절전 모드 파일을 준비하세요.</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>다시 시작한 후 현재 세션에서 UWF 활성화, RAM 오버레이, 읽기 전용 미디어 모드 및 UWF로 보호할 수 있는 마운트된 모든 고정 디스크 볼륨의 보호가 확인되는지 점검하세요. 시스템 최대 절전 모드도 사용할 수 있는지 확인하세요.</translation></message>
    <message><source>2. Restart and verify</source><translation>2. 다시 시작하고 확인</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. HORM 활성화 후 최대 절전 모드 진입</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>HORM 활성화를 초안에 추가한 다음 ‘검토 및 적용’을 사용하세요. 초안을 적용하면 다음 명령에 해당하는 HORM 활성화 작업이 실행됩니다:</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>활성화에 성공하면 바탕 화면과 프로그램을 준비한 다음 최대 절전 모드로 한 번 전환하여 기준 상태를 저장하세요:</translation></message>
    <message><source>HORM usage</source><translation>HORM 사용 방법</translation></message>
    <message><source>Volume protection</source><translation>볼륨 보호</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>다음 세션의 모든 볼륨 보호 상태를 구성합니다.</translation></message>
    <message><source>Unprotect all</source><translation>모두 보호 해제</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>다음 시작: 모든 볼륨 보호</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>다음 시작: 모든 볼륨 보호 해제</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>다음 시작: 볼륨별 설정</translation></message>
    <message><source>System hibernation is available</source><translation>시스템 최대 절전 모드 사용 가능</translation></message>
    <message><source>Usage guide</source><translation>사용 안내</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>모든 볼륨의 보호 설정이 적용 대기 중입니다</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>볼륨 보호는 다시 시작한 후 적용됩니다</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>모든 보호 해제와 HORM 활성화를 함께 적용할 수 없습니다.</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>볼륨 일괄 보호를 지원하지 않습니다.</translation></message>
    <message><source>Applied</source><translation>적용됨</translation></message>
    <message><source>Not applied</source><translation>미적용</translation></message>
    <message><source>Could not confirm</source><translation>확인할 수 없음</translation></message>



    <message><source>Storage volume:</source><translation>저장 볼륨:</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>이번 부팅 세션에서는 제외 항목도 보호를 우회하지 못합니다. 개별 커밋과 준비된 파일의 자동 커밋을 사용할 수 없습니다. 전체 볼륨 커밋은 보호된 모든 볼륨에 영향을 줍니다. 계속하기 전에 설명을 확인하세요.</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>동일한 최대 절전 상태에서 반복해서 재개</translation></message>
    <message><source>Remove full volume commit draft</source><translation>전체 볼륨 커밋 초안 제거</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>읽기 전용 미디어 모드가 활성화됨</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>구성된 저장소 마커를 읽을 수 없습니다.</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>파일 스테이징이 파일 제외 항목과 충돌합니다:
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>이 컴퓨터에서는 전체 최대 절전 모드를 사용할 수 없습니다</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>둘 이상의 저장소 볼륨에 0이 아닌 마커가 있습니다.</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>오버레이가 디스크 유형인지 확인할 수 없습니다.</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>구성된 저장 위치를 읽을 수 없습니다.</translation></message>
    <message><source>The system drive could not be identified.</source><translation>시스템 드라이브를 식별할 수 없습니다.</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>바로 다시 시작하면 스테이징 대상 파일의 커밋을 건너뜁니다.</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>바로 종료하면 스테이징 대상 파일의 커밋을 건너뜁니다.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>스테이징 대상 파일을 커밋하지 않습니다. 커밋되지 않은 변경 사항은 다시 시작 후 손실될 수 있습니다.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>스테이징 대상 파일을 커밋하지 않습니다. 커밋되지 않은 변경 사항은 종료 후 손실될 수 있습니다.</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>일반 종료 또는 다시 시작 전에 스테이징 대상 파일을 커밋합니다.</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>전체 볼륨 커밋과 HORM 활성화는 동시에 적용할 수 없습니다.</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>저장 위치 구성과 HORM 활성화는 동시에 적용할 수 없습니다.</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>저장 위치 구성과 전체 볼륨 커밋은 현재 UWF 모드의 요구 사항이 다릅니다.</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>적용하기 전에 저장 볼륨을 선택하세요.</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>디스크 저장 위치 구성이 요청한 RAM 오버레이와 충돌합니다.</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>전체 볼륨 커밋을 요청하기 전에 필터 변경을 적용하고 다시 시작하세요.</translation></message>
    <message><source>Canceled</source><translation>취소됨</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>검토한 저장 볼륨을 더 이상 사용할 수 없습니다. 편집으로 돌아가 다른 볼륨을 선택하세요.</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>이 볼륨의 저장 장치 버스가 지원되지 않아 UWF에 등록할 수 없습니다.</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>물리 볼륨의 지원 여부를 확인할 수 없습니다. 보호를 활성화하기 전에 새로 고치세요.</translation></message>
    <message><source>Paused</source><translation>일시 중지</translation></message>
    <message><source>Partially paused</source><translation>일부 일시 중지</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>영구 오버레이 데이터가 다음 시작 시에도 유지되므로 자동 커밋이 일시 중지되었습니다.</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>일시 중지된 항목: %1 · 일시 중지된 파일: %2</translation></message>
    <message><source>Read-only ROM</source><translation>읽기 전용 ROM</translation></message>
    <message><source>%1 changes · %2 commands</source><translation>변경 %1개 · 명령 %2개</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>%1 파일 시스템: 파일 제외 및 파일별 커밋을 지원하지 않습니다.</translation></message>
    <message><source>Applying changes</source><translation>변경 사항 적용 중</translation></message>
    <message><source>Automatic commits paused</source><translation>자동 커밋 일시 중지됨</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>HORM 모드 변경 후 다시 시작할 때까지 자동 파일 커밋이 일시 중지됩니다.</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>FullCommit 후에는 자동 파일 커밋을 사용할 수 없습니다.</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>HORM이 활성화된 동안에는 자동 파일 커밋을 사용할 수 없습니다.</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>이번 다시 시작에서는 준비된 파일의 자동 커밋이 일시 중지됩니다.</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>이번 종료에서는 준비된 파일의 자동 커밋이 일시 중지됩니다.</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>구성의 영구 저장 여부를 확인할 수 없습니다. 이 설정을 변경하기 전에 시스템 상태를 새로 고치세요.</translation></message>
    <message><source>Elapsed: %1 s</source><translation>경과 시간: %1초</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>조건을 충족하는 준비된 파일만 커밋됩니다.</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>읽기 전용 ROM에서는 보호된 시스템 볼륨의 일반 UWF 설정을 영구 저장할 수 없습니다. 변경하기 전에 UWF를 비활성화하고 다시 시작하세요.</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>건너뛴 준비된 파일은 이 작업에서 커밋되지 않습니다.</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>준비된 파일이 이미 보호되지 않은 저장소에 있습니다. 자동 커밋이 필요하지 않습니다.</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>임계값 변경에는 레지스트리 커밋이 필요하며, 읽기 전용 ROM에서는 사용할 수 없습니다.</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>Windows가 완료될 때까지 기다리는 중입니다. 시간이 걸릴 수 있습니다.</translation></message>
    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM이 비활성화되었습니다. 다시 시작할 때까지 준비된 파일의 자동 커밋이 일시 중지됩니다. 수동 작업은 현재 UWF 기능에 따라 수행됩니다.</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>이 구성 스냅샷에는 현재 세션에서 UWF를 비활성화해야 하는 설정이 포함되어 있습니다. 실행하기 전에 UWF를 비활성화하고 다시 시작하세요.</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>현재 세션 작업(HORM, FullCommit 및 영구 오버레이 초기화; 보류 중인 변경에서 내보내기)</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>일괄 보호 해제에는 다음 세션의 전체 볼륨 범위 확인이 필요합니다.</translation></message>
</context>
</TS>
