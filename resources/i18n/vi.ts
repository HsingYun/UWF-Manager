<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="vi" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Xem trước</translation></message>
    <message><source>enable</source><translation>bật</translation></message>
    <message><source>disable</source><translation>tắt</translation></message>
    <message><source>Enabled</source><translation>Đã bật</translation></message>
    <message><source>Disabled</source><translation>Đã tắt</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Hủy</translation></message>
    <message><source>Close</source><translation>Đóng</translation></message>
    <message><source>Refresh</source><translation>Làm mới</translation></message>
    <message><source>Add</source><translation>Thêm</translation></message>
    <message><source>Remove</source><translation>Xóa khỏi danh sách</translation></message>
    <message><source>Yes</source><translation>Có</translation></message>
    <message><source>No</source><translation>Không</translation></message>
    <message><source>Excluded</source><translation>Đã loại trừ</translation></message>
    <message><source>Not excluded</source><translation>Chưa loại trừ</translation></message>
    <message><source>File…</source><translation>Tệp…</translation></message>
    <message><source>Folder…</source><translation>Thư mục…</translation></message>
    <message><source>volume ID</source><translation>ID ổ đĩa</translation></message>
    <message><source>drive letter</source><translation>ký tự ổ đĩa</translation></message>
    <message><source>Drive letter</source><translation>Ký tự ổ đĩa</translation></message>
    <message><source>Volume ID</source><translation>ID ổ đĩa</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Trạng thái bảo vệ ổ đĩa trong phiên hiện tại (chỉ đọc).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Bảo vệ ổ đĩa trong phiên tiếp theo. Dữ liệu ghi được chuyển vào lớp phủ và bị loại bỏ khi khởi động lại.</translation></message>
    <message><source>Current session</source><translation>Phiên hiện tại</translation></message>
    <message><source>Next session</source><translation>Phiên tiếp theo</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Cách nhận diện ổ đĩa. Ký tự ổ đĩa đơn giản hơn nhưng liên kết mất hiệu lực nếu ký tự đổi, ví dụ sau khi thêm hoặc tháo đĩa. ID ổ đĩa vẫn ổn định.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF không hỗ trợ ổ đĩa này: </translation></message>
    <message><source>Global settings</source><translation>Thiết lập chung</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Trạng thái bộ lọc UWF trong phiên hiện tại (chỉ đọc).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Bật UWF trong phiên tiếp theo. Dữ liệu ghi vào ổ được bảo vệ chuyển vào lớp phủ và bị loại bỏ khi khởi động lại.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Để đổi loại và dung lượng tối đa:
1. Tắt bộ lọc bằng công tắc trên.
2. Khởi động lại để bộ lọc ngừng hoạt động.
3. Đổi thiết lập.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Nơi lưu lớp phủ: RAM nhanh hơn nhưng chiếm bộ nhớ; Disk dùng ổ hệ thống và có dung lượng lớn hơn. Cả hai đều bị loại bỏ khi khởi động lại.</translation></message>
    <message><source>Type</source><translation>Loại</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Dung lượng lớp phủ tối đa. RAM bị giới hạn bởi tổng bộ nhớ hệ thống. Disk cần ít nhất 1024 MB và đủ chỗ trống trên ổ hệ thống.</translation></message>
    <message><source>Maximum size</source><translation>Dung lượng tối đa</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Dung lượng tối đa · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Ngưỡng cảnh báo</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Ghi thông báo mức cảnh báo vào nhật ký sự kiện khi mức sử dụng lớp phủ đạt giá trị này. Khi cả hai sự kiện được bật, giá trị này phải thấp hơn ngưỡng nghiêm trọng. Giá trị 0 chỉ tắt sự kiện cảnh báo.</translation></message>
    <message><source>Critical threshold</source><translation>Ngưỡng nghiêm trọng</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Ghi thông báo mức nghiêm trọng vào nhật ký sự kiện khi mức sử dụng lớp phủ đạt giá trị này. Khi cả hai sự kiện được bật, giá trị này phải cao hơn ngưỡng cảnh báo. Giá trị 0 chỉ tắt sự kiện nghiêm trọng.</translation></message>
    <message><source>Overlay</source><translation>Lớp phủ</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Không hỗ trợ loại ổ đĩa này (chỉ hỗ trợ đĩa cố định cục bộ).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Ổ đĩa vượt giới hạn UWF là 16 TiB cho mỗi ổ được bảo vệ.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: có thể bảo vệ ổ đĩa nhưng không hỗ trợ loại trừ hay ghi vĩnh viễn từng tệp.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Thất bại: Đọc thông tin ổ đĩa.</translation></message>
    <message><source>%1 free / %2</source><translation>Trống %1 / %2</translation></message>
    <message><source>Commit</source><translation>Ghi vĩnh viễn</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Ghi vĩnh viễn thay đổi trong lớp phủ của tệp vào đĩa / Ghi vĩnh viễn thay đổi sổ đăng ký. Không thể hoàn tác thao tác này.</translation></message>
    <message><source>Commit file changes…</source><translation>Ghi vĩnh viễn thay đổi tệp…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Chọn tệp cần ghi vĩnh viễn vào đĩa. Ghi vĩnh viễn thay đổi trong lớp phủ của tệp vào đĩa.</translation></message>
    <message><source>Commit folder changes…</source><translation>Ghi vĩnh viễn thay đổi thư mục…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Chọn thư mục cần ghi vĩnh viễn vào đĩa. Ghi vĩnh viễn thay đổi trong lớp phủ của thư mục vào đĩa. Bao gồm toàn bộ thư mục con.</translation></message>
    <message><source>Delete and commit file…</source><translation>Xóa tệp và ghi vĩnh viễn thao tác xóa vào đĩa…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Chọn tệp cần xóa vĩnh viễn. Xóa tệp và ghi vĩnh viễn thao tác xóa vào đĩa.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Xóa thư mục cùng nội dung và ghi vĩnh viễn thao tác xóa vào đĩa…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Chọn thư mục cần xóa vĩnh viễn. Xóa thư mục cùng nội dung và ghi vĩnh viễn thao tác xóa vào đĩa. Bao gồm toàn bộ thư mục con.</translation></message>
    <message><source>Commit registry changes…</source><translation>Ghi vĩnh viễn thay đổi sổ đăng ký…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Nhập khóa và tên giá trị nếu cần. Để trống tên sẽ ghi vĩnh viễn mọi giá trị của khóa cùng toàn bộ khóa con.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Ghi vĩnh viễn thao tác xóa sổ đăng ký…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Nhập khóa cần xóa và tên giá trị nếu cần. Để trống tên sẽ xóa và ghi vĩnh viễn toàn bộ cây gồm khóa, các giá trị và khóa con.</translation></message>
    <message><source>File exclusions</source><translation>Loại trừ tệp</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Tệp và thư mục trên ổ được loại khỏi bảo vệ UWF. Nhấp đúp vào mục để sao chép đường dẫn.</translation></message>
    <message><source>Registry exclusions</source><translation>Loại trừ sổ đăng ký</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Loại trừ sổ đăng ký là thiết lập chung, phụ thuộc bộ lọc UWF chứ không theo trạng thái bảo vệ ổ. Danh sách dùng chung chỉ hiện một lần trong các thẻ ổ đĩa. Nhấp đúp để sao chép đường dẫn.</translation></message>
    <message><source>File staging</source><translation>Tệp chờ ghi tự động</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>tệp / thư mục: Được ghi vĩnh viễn tự động trước khi tắt máy hoặc khởi động lại an toàn. Bao gồm toàn bộ thư mục con. Khi danh sách còn mục, loại trừ sổ đăng ký sau giúp giữ danh sách qua các phiên được bảo vệ: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Thêm tệp hoặc thư mục vào danh sách ghi tự động.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Chọn tệp cần ghi vĩnh viễn vào đĩa. Được ghi vĩnh viễn tự động trước khi tắt máy hoặc khởi động lại an toàn.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Chọn thư mục cần ghi vĩnh viễn vào đĩa. Được ghi vĩnh viễn tự động trước khi tắt máy hoặc khởi động lại an toàn. Bao gồm toàn bộ thư mục con.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Chọn tệp cần ghi vĩnh viễn vào đĩa · Tệp chờ ghi tự động</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Chọn thư mục cần ghi vĩnh viễn vào đĩa · Tệp chờ ghi tự động · Bao gồm toàn bộ thư mục con.</translation></message>
    <message><source>Cannot stage this path</source><translation>Không thể thêm đường dẫn vào danh sách ghi tự động</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Đường dẫn trùng với loại trừ UWF. Hãy xóa mục loại trừ trước:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Đường dẫn trùng với mục ghi tự động. Hãy xóa mục đó trước khi thêm loại trừ:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Đường dẫn trùng với mục ghi tự động hiện có</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Cần giữ loại trừ sổ đăng ký này khi danh sách ghi tự động còn mục đã lưu</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Đường dẫn không tồn tại hoặc sai loại tệp:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Không thể kiểm tra đường dẫn:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Không thể thêm điểm phân tích lại vào danh sách ghi tự động:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Không có ký tự ổ đĩa nên UWF không thể ghi vĩnh viễn:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Đường dẫn nằm trên ổ %1. Hãy thêm từ thẻ ghi tự động của ổ đó.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Không thể thêm thư mục gốc vì sẽ ghi vĩnh viễn đệ quy toàn bộ ổ đĩa.</translation></message>
    <message><source>File staging could not be saved</source><translation>Thất bại: Lưu danh sách tệp chờ ghi tự động</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Danh sách đã được tải lại do không thể hoàn tất cập nhật:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Tệp chờ ghi tự động: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Được ghi vĩnh viễn tự động trước khi tắt máy hoặc khởi động lại an toàn.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Tệp chờ ghi tự động: %1 · tệp: %2 · thư mục: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Chức năng ghi tệp tự động cần mục loại trừ sổ đăng ký này để giữ danh sách sau khi khởi động lại.</translation></message>
    <message><source>Built-in</source><translation>Tích hợp</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Bộ lọc UWF đang tắt. Không có thay đổi tích lũy trong lớp phủ để ghi vĩnh viễn.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Hệ thống tệp %1 không hỗ trợ ghi vĩnh viễn từng tệp.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>UWF không hỗ trợ ổ đĩa này.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Ổ đĩa không hỗ trợ ghi vĩnh viễn tệp. Chỉ có thể ghi vĩnh viễn sổ đăng ký (loại trừ áp dụng chung).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Hệ thống tệp %1 không hỗ trợ ghi vĩnh viễn từng tệp. Chỉ có thể ghi vĩnh viễn sổ đăng ký (loại trừ áp dụng chung).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Ổ đĩa hiện không được UWF bảo vệ; không có thay đổi tệp để ghi vĩnh viễn.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Ổ đĩa hiện không được UWF bảo vệ; không có thay đổi tệp để ghi vĩnh viễn. Chỉ có thể ghi vĩnh viễn sổ đăng ký (loại trừ áp dụng chung).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Chọn tệp cần ghi vĩnh viễn vào đĩa</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Chọn thư mục cần ghi vĩnh viễn vào đĩa</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Chọn tệp cần xóa vĩnh viễn</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Chọn thư mục cần xóa vĩnh viễn</translation></message>
    <message><source>Name</source><translation>Tên</translation></message>
    <message><source>Data</source><translation>Dữ liệu</translation></message>
    <message><source>(Default)</source><translation>(Mặc định)</translation></message>
    <message><source>(value not set)</source><translation>(chưa đặt giá trị)</translation></message>
    <message><source>(empty)</source><translation>(trống)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Không rõ (Giá trị: Tệp quá lớn)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Xem tệp lớp phủ. &lt;b&gt;Chỉ là ảnh chụp chẩn đoán.&lt;/b&gt; Không dùng danh sách này để quyết định tệp cần ghi.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Chỉ hỗ trợ liệt kê tệp lớp phủ trên ổ NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF đang tắt; không có lớp phủ để kiểm tra.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Ổ đĩa không được UWF bảo vệ nên không có lớp phủ để kiểm tra.</translation></message>
    <message><source>Failed: %1</source><translation>Thất bại: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Nhà cung cấp WMI bị sự cố khi liệt kê. Đây là lỗi không ổn định đã biết của UWF_Overlay.GetOverlayFiles khi lớp phủ lớn hoặc tải I/O cao.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Thiếu bộ nhớ hoặc nhà cung cấp không hỗ trợ thao tác. Cần NTFS và bộ nhớ dự phòng. Thử lại với lớp phủ nhỏ hơn hoặc đóng ứng dụng tốn bộ nhớ.</translation></message>
    <message><source>Raw error: %1</source><translation>Mã lỗi: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Xuất ra tệp… · Lớp phủ · tệp</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Tệp văn bản (*.txt);;Tất cả tệp (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Tệp lệnh (*.cmd);;Tất cả tệp (*)</translation></message>
    <message><source>Export failed</source><translation>Xuất thất bại</translation></message>
    <message><source>Export finished</source><translation>Đã xuất xong</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Thất bại: Mở tệp (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Thất bại: Ghi tệp (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Cấu hình: Ghi tệp · mục: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Không thể loại trừ toàn bộ ổ đĩa (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Không thể loại trừ tệp phân trang, hoán đổi và ngủ đông vì UWF phụ thuộc vào chúng.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Không thể loại trừ toàn bộ thư mục: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Không thể loại trừ toàn bộ thư mục: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Không thể loại trừ toàn bộ thư mục: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Không thể loại trừ tệp hệ thống quan trọng này: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Không thể loại trừ tệp sổ đăng ký người dùng NTUSER.DAT.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Khóa sổ đăng ký không được để trống.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Khóa sổ đăng ký không được bắt đầu bằng dấu gạch chéo ngược.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Đường dẫn chứa các dấu gạch chéo ngược liên tiếp nên không hợp lệ.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Đường dẫn sổ đăng ký dùng `\` làm dấu phân cách, không dùng `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Đường dẫn chứa ký tự điều khiển vô hình nên không hợp lệ.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>Tài liệu UWF cấm loại trừ bí mật tài khoản máy tính miền: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Chỉ cho phép loại trừ khóa con cụ thể trong các khóa gốc sau:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Thêm tệp hoặc thư mục vào loại trừ. UWF không bảo vệ các mục bị loại trừ.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Chọn tệp cần thêm · Loại trừ tệp.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Chọn thư mục cần thêm · Loại trừ tệp. Bao gồm toàn bộ thư mục con.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Nhập đường dẫn khóa sổ đăng ký cần loại trừ.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Nhập đường dẫn khóa sổ đăng ký cần loại trừ. / Lưu giữ qua các phiên.</translation></message>
    <message><source>Registry key…</source><translation>Sổ đăng ký · Khóa…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Khóa bí mật miền (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Giấy phép truy cập máy khách dịch vụ đầu cuối (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Giữ bí mật miền (mật khẩu tài khoản máy tính) qua các phiên UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Giữ giấy phép truy cập máy khách dịch vụ đầu cuối qua các phiên UWF.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Lưu giữ qua các phiên %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Xóa mục đã chọn</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Xóa các loại trừ đã chọn. Có hiệu lực sau khi áp dụng.</translation></message>
    <message><source>Open containing folder</source><translation>Mở thư mục chứa</translation></message>
    <message><source>Copy file path</source><translation>Sao chép đường dẫn tệp</translation></message>
    <message><source>Copy registry path</source><translation>Sao chép đường dẫn sổ đăng ký</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Ghi vĩnh viễn thay đổi trong lớp phủ của thư mục vào đĩa…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Ghi vĩnh viễn thay đổi trong lớp phủ của tệp vào đĩa…</translation></message>
    <message><source>Copied to clipboard</source><translation>Đã sao chép vào bảng nhớ tạm</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Chọn tệp cần thêm · Loại trừ tệp. Có thể chọn nhiều tệp.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Chọn thư mục cần thêm · Loại trừ tệp</translation></message>
    <message><source>Add registry exclusion</source><translation>Thêm loại trừ sổ đăng ký</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Đường dẫn %1 không thuộc ổ %2 nên không thể thêm vào loại trừ của ổ này.</translation></message>
    <message><source>Path is not on this volume</source><translation>Đường dẫn không thuộc ổ này</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Không thể thêm loại trừ này</translation></message>
    <message><source>File: %1</source><translation>Tệp… %1</translation></message>
    <message><source>Folder: %1</source><translation>Thư mục… %1</translation></message>
    <message><source>Registry: %1</source><translation>Sổ đăng ký: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Mặc định bị loại trừ; có thể đổi bất cứ lúc nào.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Phiên hiện tại: %1
Phiên tiếp theo: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Thay đổi chờ áp dụng: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Trình quản lý bộ lọc ghi hợp nhất (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Đọc lại trạng thái phiên hiện tại và cấu hình phiên tiếp theo của UWF.</translation></message>
    <message><source>Review and apply</source><translation>Xem lại và áp dụng</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Xem lại mọi thay đổi đang chờ và áp dụng cùng lúc. Phần lớn thay đổi có hiệu lực sau lần khởi động lại tới.</translation></message>
    <message><source>Safe shutdown</source><translation>Tắt máy an toàn</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Hệ thống sẽ tắt an toàn qua UWF. Vẫn dùng được khi lớp phủ UWF đầy.</translation></message>
    <message><source>Safe restart</source><translation>Khởi động lại an toàn</translation></message>
    <message><source>Direct restart</source><translation>Khởi động lại trực tiếp</translation></message>
    <message><source>Direct shutdown</source><translation>Tắt máy trực tiếp</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Hệ thống sẽ khởi động lại an toàn qua UWF. Vẫn dùng được khi lớp phủ UWF đầy.</translation></message>
    <message><source>Log</source><translation>Nhật ký</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Xem nhật ký nội bộ của phiên này để khắc phục sự cố.</translation></message>
    <message><source>About</source><translation>Giới thiệu</translation></message>
    <message><source>Switch display language</source><translation>Đổi ngôn ngữ hiển thị</translation></message>
    <message><source>Language</source><translation>Ngôn ngữ</translation></message>
    <message><source>Theme</source><translation>Giao diện</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Đổi giao diện sáng/tối</translation></message>
    <message><source>System information</source><translation>Thông tin hệ thống</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Ảnh chụp chẩn đoán</translation></message>
    <message><source>Read only</source><translation>Chỉ đọc</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Không tìm thấy đối tượng; lớp phủ không có gì để ghi.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Đối tượng chỉ tồn tại trong lớp phủ, được tạo dưới bảo vệ UWF và chưa ghi vào đĩa hay sổ đăng ký bền vững. Sẽ biến mất khi khởi động lại; không cần và không thể ghi vĩnh viễn thao tác xóa.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Hệ thống từ chối tham số (đường dẫn hoặc đối số không hợp lệ).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Thao tác thất bại. Đối tượng có thể được tiến trình khác sử dụng, như thư mục đang mở trong Explorer hoặc tệp đang mở. Đóng các chương trình đó rồi thử lại.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Thao tác thất bại; xem nhật ký để biết chi tiết.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Thao tác bị từ chối (mã %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Không rõ.</translation></message>
    <message><source>Commit canceled</source><translation>Đã hủy ghi</translation></message>

    <message><source>Succeeded</source><translation>Thành công</translation></message>
    <message><source>Skipped</source><translation>Đã bỏ qua</translation></message>
    <message><source>Failed</source><translation>Thất bại</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Thành công: %1; Đã bỏ qua: %2; Thất bại: %3.</translation></message>

    <message><source>Existed before</source><translation>Tồn tại trước</translation></message>
    <message><source>Exists after</source><translation>Tồn tại sau</translation></message>
    <message><source>Error code</source><translation>Mã lỗi</translation></message>
    <message><source>Reason</source><translation>Lý do</translation></message>
    <message><source>Previous page</source><translation>Trang trước</translation></message>
    <message><source>First page</source><translation>Trang đầu</translation></message>
    <message><source>Last page</source><translation>Trang cuối</translation></message>
    <message><source>Next page</source><translation>Trang sau</translation></message>
    <message><source>Copy selected rows</source><translation>Sao chép hàng đã chọn</translation></message>
    <message><source>Copy all</source><translation>Sao chép tất cả</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Cần giữ loại trừ sổ đăng ký này khi danh sách ghi tự động còn mục đã lưu. Tệp chờ ghi tự động: Xóa hết → Xóa loại trừ sổ đăng ký.</translation></message>
    <message><source>Copy current page</source><translation>Sao chép trang hiện tại</translation></message>
    <message><source>No pending changes</source><translation>Không có thay đổi chờ áp dụng</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Loại trừ sổ đăng ký · Thiết lập chung.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Ổ đĩa %1: Bảo vệ UWF / Loại trừ tệp / Tệp chờ ghi tự động.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Không được hỗ trợ</translation></message>
    <message><source>Review and apply changes</source><translation>Xem lại và áp dụng thay đổi</translation></message>
    <message><source>Confirm apply</source><translation>Xác nhận áp dụng</translation></message>
    <message><source>Result</source><translation>Kết quả</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Bộ lọc: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Đặt ngưỡng cảnh báo: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Đặt ngưỡng nghiêm trọng: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Đặt loại lớp phủ / Đặt dung lượng tối đa: Để đổi loại và dung lượng tối đa:
1. Tắt bộ lọc bằng công tắc trên.
2. Khởi động lại để bộ lọc ngừng hoạt động.
3. Đổi thiết lập.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Đặt loại lớp phủ: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Đặt dung lượng tối đa: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Đặt dung lượng tối đa: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Ổ đĩa %1 · Bảo vệ UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Ổ đĩa %1 · Nhận diện theo: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Ổ đĩa %1 · Thêm loại trừ tệp: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Ổ đĩa %1 · Xóa loại trừ tệp: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Thêm loại trừ sổ đăng ký: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Xóa loại trừ sổ đăng ký: %1</translation></message>
    <message><source>Time</source><translation>Thời gian</translation></message>
    <message><source>Level</source><translation>Mức độ</translation></message>
    <message><source>Tag</source><translation>Nhãn</translation></message>
    <message><source>Message</source><translation>Thông báo</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Đang đếm tệp cần ghi tự động…</translation></message>
    <message><source>Committing staged files</source><translation>Đang ghi tệp chờ</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>Đã xử lý %1/%2 tệp.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Không có thông tin lỗi bổ sung.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Ghi tự động dừng đột ngột:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Ghi tệp tự động chưa hoàn tất: Lỗi không xác định.</translation></message>
    <message><source>File staging will be skipped</source><translation>Sẽ bỏ qua ghi tệp tự động</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Khởi động lại trực tiếp: Sẽ bỏ qua ghi tệp tự động.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Tắt máy trực tiếp: Sẽ bỏ qua ghi tệp tự động.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Đã bỏ qua ghi tự động vì chọn thao tác nguồn trực tiếp.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Tắt máy an toàn: Thất bại</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Tắt máy: Thất bại (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Khởi động lại an toàn: Thất bại</translation></message>
    <message><source>Direct restart failed</source><translation>Khởi động lại trực tiếp: Thất bại</translation></message>
    <message><source>Direct shutdown failed</source><translation>Tắt máy trực tiếp: Thất bại</translation></message>
    <message><source>Restart failed: %1</source><translation>Khởi động lại: Thất bại (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Không thể đọc danh sách ghi tự động:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Không thể đọc danh sách ghi tự động: Lỗi không xác định.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>tệp: %1 · Thành công: %2 · Đã bỏ qua (tệp): %3 · Đã bỏ qua (mục): %4 · Thất bại: %5</translation></message>
    <message><source>File staging preparation</source><translation>Tệp chờ ghi tự động: Chi tiết thao tác</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Không hỗ trợ điểm phân tích lại.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Chỉ có thể thêm đường dẫn tuyệt đối trên ổ cục bộ.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Chỉ có thể thêm đường dẫn tuyệt đối trên ổ cục bộ.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Không thể kiểm tra đường dẫn chờ ghi.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Đường dẫn tệp chờ ghi nay trỏ tới loại đối tượng khác.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Đường dẫn thư mục chờ ghi nay trỏ tới loại đối tượng khác.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Không thể liệt kê đầy đủ thư mục chờ ghi.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Đường dẫn không có ký tự ổ đĩa.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>Không thể xử lý đệ quy thư mục gốc của ổ đĩa.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Nhà cung cấp UWF đã từ chối ghi vĩnh viễn.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Ghi thất bại: Lỗi không xác định.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Đã ghi thêm %1 lỗi vào nhật ký.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Đường dẫn không có ký tự ổ đĩa. Xác định ổ đĩa đích: Thất bại.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Không có bản ghi phiên hiện tại cho ổ %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Đường dẫn nằm trong danh sách loại trừ tệp.
Loại trừ: %1</translation></message>

    <message><source>Continue</source><translation>Tiếp tục</translation></message>
    <message><source>Delete and commit</source><translation>Xóa và ghi vĩnh viễn</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Khóa nằm trong danh sách loại trừ sổ đăng ký.
Loại trừ: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Không có bản ghi bộ lọc sổ đăng ký cho phiên hiện tại.</translation></message>

    <message><source>Export commands to file</source><translation>Xuất lệnh ra tệp</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Xuất ra tệp… · Lệnh: %1
%2</translation></message>
    <message><source>Import</source><translation>Nhập</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Nhập hoặc dán mỗi lệnh trên một dòng. Tải tệp văn bản, nhật ký hoặc kịch bản (.txt, .bat, .ps1); các dòng chứa lệnh này sẽ được thêm. Nhập lệnh chỉ chuẩn bị thay đổi. Chỉ "Xem lại và áp dụng" trên thanh công cụ mới ghi chúng vào hệ thống.</translation></message>
    <message><source>Load from file…</source><translation>Tải từ tệp…</translation></message>
    <message><source>Load recommended configuration</source><translation>Tải cấu hình đề xuất</translation></message>
    <message><source>Choose recommended configuration</source><translation>Chọn cấu hình đề xuất</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Chọn các nhóm cấu hình đề xuất cần thêm. Có thể xem lại hoặc xóa từng lệnh trước khi nhập.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Theo tài liệu chính thức của Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Loại trừ UWF do Microsoft đề xuất</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Xem lại trước khi nhập: thư mục phải tồn tại để UWF chấp nhận loại trừ tệp.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Nguồn: tài liệu Microsoft UWF chính thức về loại trừ phổ biến và hỗ trợ chống mã độc.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Chương trình cải thiện trải nghiệm khách hàng (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Lưu giữ vĩnh viễn: Trạng thái tham gia theo chính sách</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Lưu giữ vĩnh viễn: Trạng thái tham gia cục bộ</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Lưu giữ vĩnh viễn: Cờ tắt tải lên</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Dịch vụ truyền tải thông minh dưới nền (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Lưu giữ vĩnh viễn: Tệp hàng đợi tải xuống</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Lưu giữ vĩnh viễn: Chỉ mục trạng thái truyền tải</translation></message>
    <message><source>Network profiles and policies</source><translation>Hồ sơ và chính sách mạng</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Chính sách GPO mạng không dây</translation></message>
    <message><source>Wired network GPO policy</source><translation>Chính sách GPO mạng có dây</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Chính sách GPO mạng không dây · tệp</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Chính sách GPO mạng có dây · tệp</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Hồ sơ giao diện không dây</translation></message>
    <message><source>Wired network interface profiles</source><translation>Hồ sơ giao diện có dây</translation></message>
    <message><source>Wireless service configuration</source><translation>Cấu hình dịch vụ không dây</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Cấu hình dịch vụ băng rộng di động</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Cấu hình dịch vụ tự cấu hình mạng có dây</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>Hồ sơ mạng XML phụ thuộc thiết bị. Nếu cần, thêm thủ công đường dẫn cụ thể Interfaces\{GUID}\{GUID}.xml.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Giờ mùa hè (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Lưu giữ vĩnh viễn: Định nghĩa múi giờ</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Lưu giữ vĩnh viễn: Thông tin múi giờ đã chọn</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Lưu giữ vĩnh viễn: Tệp sản phẩm và bản cập nhật</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Lưu giữ vĩnh viễn: Chữ ký và trạng thái trong ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Lưu giữ vĩnh viễn: Windows Update · Nhật ký</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Lưu giữ vĩnh viễn: MpCmdRun · Nhật ký</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Lưu giữ vĩnh viễn: Trạng thái sản phẩm trong sổ đăng ký</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Lưu giữ vĩnh viễn: WdBoot · Trạng thái dịch vụ</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Lưu giữ vĩnh viễn: WdFilter · Trạng thái dịch vụ</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Lưu giữ vĩnh viễn: WdNisSvc · Trạng thái dịch vụ</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Lưu giữ vĩnh viễn: WdNisDrv · Trạng thái dịch vụ</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Lưu giữ vĩnh viễn: WinDefend · Trạng thái dịch vụ</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Lưu giữ vĩnh viễn: Tệp chương trình máy khách</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Lưu giữ vĩnh viễn: Windows Update · Nhật ký</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Lưu giữ vĩnh viễn: MpCmdRun · Nhật ký</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Lưu giữ vĩnh viễn: Chữ ký và trạng thái chống mã độc</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Lưu giữ vĩnh viễn: Trạng thái chống mã độc trong sổ đăng ký</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Chọn tệp chứa lệnh uwfmgr</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Tệp văn bản (*.txt *.cmd);;Tất cả tệp (*)</translation></message>
    <message><source>Status</source><translation>Trạng thái</translation></message>
    <message><source>Import failed</source><translation>Nhập thất bại</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Lỗi nội bộ: chưa đăng ký bộ xử lý áp dụng.</translation></message>
    <message><source>Nothing to import</source><translation>Không có lệnh để nhập</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Không tìm thấy lệnh uwfmgr trong dữ liệu nhập.</translation></message>
    <message><source>Duplicates: %1</source><translation>Trùng lặp: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Không được hỗ trợ: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Đợt %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Trùng lặp</translation></message>
    <message><source>Unsupported</source><translation>Không được hỗ trợ</translation></message>
    <message><source>File too large</source><translation>Tệp quá lớn</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Tệp %1 lớn hơn 5 MB nên chưa được phân tích. Hãy lọc thủ công trước.</translation></message>
    <message><source>Cannot read file</source><translation>Không thể đọc tệp</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Thất bại: Mở tệp (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Hủy các thay đổi đang chờ?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Có %1 thay đổi chưa áp dụng.

Hủy bỏ và tiếp tục?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Lệnh uwfmgr chưa đầy đủ</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Thiếu tham số: Dung lượng (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>Dung lượng phải là số nguyên không âm tính bằng MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Thiếu tham số: Lớp phủ · Loại (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Giá trị không hợp lệ: Lớp phủ · Loại: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Thiếu tham số: Ổ đĩa (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Ổ đĩa phải là ký tự như C:</translation></message>
    <message><source>Missing path argument</source><translation>Thiếu tham số: Đường dẫn</translation></message>
    <message><source>Missing registry key argument</source><translation>Thiếu tham số: Sổ đăng ký · Khóa</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Dấu ngoặc kép trong lệnh chưa đóng</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Tham số thừa ngoài dự kiến: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Lệnh uwfmgr không được hỗ trợ (không thể ánh xạ sang thao tác giao diện)</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>Thay đổi chờ áp dụng: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Đã ở trạng thái đích; không cần thao tác</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Đường dẫn không thuộc ổ này hoặc ổ không hỗ trợ loại trừ tệp (ví dụ exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Bị danh sách chặn UWF từ chối (tệp hệ thống / Windows / phân trang / v.v.)</translation></message>
    <message><source>Pending filter %1</source><translation>Thay đổi chờ áp dụng: Bộ lọc %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>Bộ lọc: Đã ở trạng thái đích; không cần thao tác</translation></message>
    <message><source>Pending overlay type → %1</source><translation>Thay đổi chờ áp dụng: Lớp phủ · Loại → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>Lớp phủ · Loại: %1 (Đã ở trạng thái đích; không cần thao tác)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Giá trị không hợp lệ: Dung lượng: %1</translation></message>
    <message><source>maximum size</source><translation>Dung lượng tối đa</translation></message>
    <message><source>warning threshold</source><translation>Ngưỡng cảnh báo</translation></message>
    <message><source>critical threshold</source><translation>Ngưỡng nghiêm trọng</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>Thay đổi chờ áp dụng: Lớp phủ %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>Lớp phủ %1: %2 MB (Đã ở trạng thái đích; không cần thao tác)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Không rõ ổ %1 (không có đĩa đủ điều kiện UWF mang ký tự này)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>Thay đổi chờ áp dụng: Ổ đĩa %1 · Bảo vệ UWF: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Ổ đĩa %1: Đã ở trạng thái đích; không cần thao tác</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Đường dẫn không có ký tự ổ đĩa. Xác định ổ đĩa đích: Thất bại.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Ổ đĩa %1: UWF Không được hỗ trợ</translation></message>
    <message><source>file exclusion</source><translation>Loại trừ tệp</translation></message>
    <message><source>registry exclusion</source><translation>Loại trừ sổ đăng ký</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Không có thẻ sổ đăng ký nên không dùng được loại trừ sổ đăng ký</translation></message>
    <message><source>Unsupported command</source><translation>Lệnh: Không được hỗ trợ</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Hiện hoặc ẩn.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Đã bật</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Đã tắt</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Không rõ</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Đã dùng %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Chế độ nâng cao</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Phối hợp ghi tự động với quá trình tắt và khởi động lại Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Thất bại: Đọc trạng thái chế độ nâng cao (Lỗi không xác định).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Thất bại: Đọc trạng thái dịch vụ:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Thất bại: Đọc trạng thái dịch vụ (Lỗi không xác định).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Dịch vụ chưa được cấu hình thành tiến trình độc lập.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Dịch vụ chưa được cấu hình tự khởi động.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Dịch vụ chưa được cấu hình chạy dưới LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>Dịch vụ chưa chạy.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Đường dẫn hoặc tham số khởi động dịch vụ không khớp tệp thực thi hiện tại.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Thời gian chờ trước khi tắt của dịch vụ chưa được cấu hình đúng.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Dịch vụ chưa khai báo quyền cần để khởi động tác nhân giao diện.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Dịch vụ chưa nhận thông báo trước khi Windows tắt.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Bật chế độ nâng cao</translation></message>
    <message><source>Disable enhanced mode</source><translation>Tắt chế độ nâng cao</translation></message>
    <message><source>Start service</source><translation>Khởi động dịch vụ</translation></message>
    <message><source>Repair enhanced mode</source><translation>Sửa chế độ nâng cao</translation></message>
    <message><source>Delete service</source><translation>Xóa dịch vụ</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Dừng và xóa dịch vụ nâng cao cùng dữ liệu sổ đăng ký còn lại.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Dịch vụ không tồn tại nhưng dữ liệu sổ đăng ký vẫn còn.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Đang chờ xác thực tác nhân giao diện.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Dịch vụ đang chạy nhưng chưa có tác nhân giao diện đã xác thực kết nối.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Dịch vụ hỗ trợ chế độ nâng cao UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Chế độ nâng cao · Cấu hình: Đã áp dụng.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Cấu hình: Đã áp dụng. Thông báo lưu giữ UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Chế độ nâng cao · Cấu hình: Thất bại:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Chế độ nâng cao · Cấu hình: Thất bại (Lỗi không xác định).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Đã xóa dịch vụ nâng cao cùng cấu hình còn lại.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Đã yêu cầu xóa dịch vụ nhưng dọn dữ liệu còn lại báo:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Chế độ nâng cao: Xóa dịch vụ: Thất bại:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Chế độ nâng cao: Xóa dịch vụ: Thất bại (Lỗi không xác định).</translation></message>
    <message><source>File staging completed</source><translation>Tệp chờ ghi tự động: Thành công</translation></message>
    <message><source>File staging failed</source><translation>Tệp chờ ghi tự động: Thất bại</translation></message>
    <message><source>All staged targets were processed.</source><translation>Đã xử lý tất cả đối tượng chờ ghi.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Đã xử lý tất cả đối tượng; có ít nhất một thao tác thất bại.</translation></message>
    <message><source>Show main window</source><translation>Hiện cửa sổ chính</translation></message>
    <message><source>Restore default position</source><translation>Khôi phục vị trí mặc định</translation></message>
    <message><source>Show overlay hub</source><translation>Hiện HUB lớp phủ</translation></message>
    <message><source>Hide overlay hub</source><translation>Ẩn HUB lớp phủ</translation></message>
    <message><source>Exit application</source><translation>Thoát ứng dụng</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Thất bại: Kết nối hệ thống</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Thất bại: Đọc giá trị sổ đăng ký (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Liệt kê tệp lớp phủ: Lỗi không xác định.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Thất bại: Thao tác bộ lọc: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Thất bại: Đọc trạng thái bộ lọc</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Thất bại: Đặt ngưỡng cảnh báo</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Thất bại: Đặt ngưỡng nghiêm trọng</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Thất bại: Đọc trạng thái lớp phủ</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Thất bại: Đặt loại lớp phủ</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Thất bại: Đặt dung lượng tối đa</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Thất bại: Đọc cấu hình lớp phủ</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Thất bại: Đăng ký ổ đĩa %1 với UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Thất bại: Bảo vệ ổ đĩa %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Thất bại: Đặt cách nhận diện ổ đĩa %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Ổ đĩa %1 · Thất bại: Thêm loại trừ tệp %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Loại trừ tệp trên ổ %1 đã không còn: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Ổ đĩa %1 · Thất bại: Xóa loại trừ tệp %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Thất bại: Đọc cấu hình ổ đĩa</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Thất bại: Thêm loại trừ sổ đăng ký %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Thất bại: Xóa loại trừ sổ đăng ký %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Thất bại: Cập nhật tùy chọn lưu giữ sổ đăng ký</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Thất bại: Đọc bộ lọc sổ đăng ký</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Không được hỗ trợ</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Đọc trạng thái đối tượng: Lỗi không xác định.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Đường dẫn không tồn tại; không có gì để xóa.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Lỗi không xác định.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Không thể xác nhận kết quả vì đọc lại trạng thái đối tượng thất bại: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Nhà cung cấp chấp nhận xóa nhưng kiểm tra lại từ nguồn chính xác cho thấy đối tượng vẫn tồn tại.</translation></message>
    <message><source>Unknown</source><translation>Không rõ</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Thất bại: Tải nhật ký (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Thất bại: Tải nhật ký (Lỗi không xác định).</translation></message>
    <message><source>: unknown error</source><translation>: Lỗi không xác định</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Thất bại: Xác định ổ đĩa đích (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Thất bại: Liệt kê khóa sổ đăng ký (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Thất bại: Xác định ổ đĩa đích (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Đã bỏ qua thao tác vì thao tác tiên quyết thất bại hoặc không thể xác nhận kết quả.</translation></message>
    <message><source>Copy</source><translation>Sao chép</translation></message>
    <message><source>Select All</source><translation>Chọn tất cả</translation></message>
    <message><source>Display language</source><translation>Ngôn ngữ hiển thị</translation></message>
    <message><source>Folder</source><translation>Thư mục</translation></message>
    <message><source>File</source><translation>Tệp</translation></message>
    <message><source>Export commands</source><translation>Xuất lệnh</translation></message>
    <message><source>Direct restart…</source><translation>Khởi động lại trực tiếp…</translation></message>
    <message><source>Clear…</source><translation>Xóa hết…</translation></message>
    <message><source>Committing</source><translation>Đang ghi</translation></message>
    <message><source> · %1 selected</source><translation> · Đã chọn %1 mục</translation></message>
    <message><source>%1 changes</source><translation>%1 thay đổi</translation></message>
    <message><source>%1 commands</source><translation>%1 lệnh</translation></message>
    <message><source>%1 items</source><translation>%1 mục</translation></message>
    <message><source>%1 items · %2</source><translation>%1 mục · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 thay đổi đang chờ</translation></message>
    <message><source>%1 records</source><translation>%1 bản ghi</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 bản ghi · tổng cộng %2</translation></message>


    <message><source>%1 selected</source><translation>Đã chọn %1 mục</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 tệp</translation></message>
    <message><source>32-bit</source><translation>32 bit</translation></message>
    <message><source>64-bit</source><translation>64 bit</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Phát hành theo &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Điều khoản đầy đủ nằm trong tệp LICENSE đi kèm ứng dụng.&lt;/p&gt;&lt;p&gt;Bạn có thể phân phối lại hoặc sửa đổi phần mềm theo GPL v3. Phần mềm được cung cấp nguyên trạng, không có bất kỳ bảo đảm nào.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows tắt tệp hoán trang, Khôi phục Hệ thống, SuperFetch, lập chỉ mục tệp, chống phân mảnh đĩa và khởi động nhanh. Đồng thời đặt bootstatuspolicy thành ignoreallfailures trong BCD.&lt;/p&gt;&lt;p&gt;Bạn có thể điều chỉnh các thiết lập này sau đó. Ví dụ, chuyển tệp hoán trang sang một ổ đĩa không được bảo vệ trước khi bật lại tính năng hoán trang.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Ảnh chụp chẩn đoán về hệ thống, đặc quyền và UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>Đã có tệp tại vị trí này:
%1

Việc lưu sẽ thay thế nội dung tệp.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Công cụ quản lý bằng giao diện đồ họa cho Windows Unified Write Filter.
Quản lý bảo vệ ổ đĩa, lớp phủ, ngoại lệ và tệp chờ lưu.</translation></message>
    <message><source>About this snapshot</source><translation>Về ảnh chụp này</translation></message>
    <message><source>Account</source><translation>Tài khoản</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Thêm cấu hình UWF từ tập lệnh hoặc văn bản.</translation></message>
    <message><source>Add exclusion</source><translation>Thêm ngoại lệ</translation></message>
    <message><source>Add files or folders</source><translation>Thêm tệp hoặc thư mục</translation></message>
    <message><source>Add selected items</source><translation>Thêm các mục đã chọn</translation></message>
    <message><source>Added</source><translation>Đã thêm</translation></message>
    <message><source>Administrator</source><translation>Quản trị viên</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Mọi thay đổi đã được lưu vào đĩa và sẽ được giữ lại mà không cần khởi động lại.</translation></message>
    <message><source>All files (*)</source><translation>Tất cả tệp (*)</translation></message>
    <message><source>All levels</source><translation>Mọi mức</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Ảnh chụp chẩn đoán trống không có nghĩa là lớp phủ không chứa thay đổi.</translation></message>
    <message><source>Application path</source><translation>Đường dẫn ứng dụng</translation></message>
    <message><source>Application result</source><translation>Kết quả áp dụng</translation></message>
    <message><source>Apply changes</source><translation>Áp dụng thay đổi</translation></message>
    <message><source>Architecture</source><translation>Kiến trúc</translation></message>
    <message><source>Automatic</source><translation>Tự động</translation></message>
    <message><source>Automatic file staging</source><translation>Tự động lưu tệp chờ</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Tự động lưu chưa hoàn tất</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Tự động lưu các tệp chờ đủ điều kiện khi tắt máy và khởi động lại thông thường.</translation></message>
    <message><source>Available</source><translation>Khả dụng</translation></message>
    <message><source>Back</source><translation>Quay lại</translation></message>
    <message><source>Background service</source><translation>Dịch vụ nền</translation></message>
    <message><source>Binding</source><translation>Liên kết</translation></message>
    <message><source>By drive letter</source><translation>Theo ký tự ổ đĩa</translation></message>
    <message><source>By volume ID</source><translation>Theo ID ổ đĩa</translation></message>
    <message><source>Cancel reading</source><translation>Hủy đọc</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Những thay đổi đã ghi được giữ nguyên.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Chọn khóa sổ đăng ký cần loại trừ khỏi tính năng bảo vệ UWF, bao gồm các khóa con và giá trị của khóa đó.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Chọn vị trí để lưu tệp.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Chọn vị trí trong ổ đĩa hoặc thư mục được phép.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Chọn thư mục khác hoặc kiểm tra quyền truy cập rồi thử lại.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Chọn khóa khác hoặc kiểm tra quyền truy cập rồi thử lại.</translation></message>
    <message><source>Clear filters</source><translation>Xóa bộ lọc</translation></message>
    <message><source>Clear logs</source><translation>Xóa nhật ký</translation></message>
    <message><source>Clear logs?</source><translation>Xóa nhật ký?</translation></message>
    <message><source>Collapse</source><translation>Thu gọn</translation></message>
    <message><source>Commit changes</source><translation>Ghi thay đổi</translation></message>
    <message><source>Commit changes…</source><translation>Ghi thay đổi…</translation></message>
    <message><source>Commit complete</source><translation>Ghi hoàn tất</translation></message>
    <message><source>Commit file changes?</source><translation>Ghi thay đổi của tệp?</translation></message>
    <message><source>Commit folder changes?</source><translation>Ghi thay đổi của thư mục?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Ghi thay đổi của khóa sổ đăng ký?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Ghi thay đổi của giá trị sổ đăng ký?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Ghi tệp chờ trước khi tắt máy và khởi động lại thông thường</translation></message>
    <message><source>Compatibility mode</source><translation>Chế độ tương thích</translation></message>
    <message><source>Configuration applied</source><translation>Đã áp dụng cấu hình</translation></message>
    <message><source>Configuration error</source><translation>Lỗi cấu hình</translation></message>
    <message><source>Configured</source><translation>Đã cấu hình</translation></message>
    <message><source>Confirm before continuing</source><translation>Xác nhận trước khi tiếp tục</translation></message>
    <message><source>Connected and authenticated</source><translation>Đã kết nối và xác thực</translation></message>
    <message><source>Connecting…</source><translation>Đang kết nối…</translation></message>
    <message><source>Contact author</source><translation>Liên hệ tác giả</translation></message>
    <message><source>Continue editing</source><translation>Tiếp tục chỉnh sửa</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Tiếp tục khởi động lại sẽ loại bỏ thay đổi trong các tệp không lưu được.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Tiếp tục tắt máy sẽ loại bỏ thay đổi trong các tệp không lưu được.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Phối hợp với quá trình tắt Windows</translation></message>
    <message><source>Copy all logs</source><translation>Sao chép toàn bộ nhật ký</translation></message>
    <message><source>Copy full report</source><translation>Sao chép báo cáo đầy đủ</translation></message>
    <message><source>Copy name</source><translation>Sao chép tên</translation></message>
    <message><source>Copy path</source><translation>Sao chép đường dẫn</translation></message>
    <message><source>Copy selection</source><translation>Sao chép vùng chọn</translation></message>
    <message><source>Copy version</source><translation>Sao chép phiên bản</translation></message>
    <message><source>Counting staged files…</source><translation>Đang đếm tệp chờ lưu…</translation></message>
    <message><source>Create</source><translation>Tạo</translation></message>
    <message><source>Critical %1 MB</source><translation>Nguy cấp %1 MB</translation></message>
    <message><source>Current filter</source><translation>Bộ lọc hiện tại</translation></message>
    <message><source>Current run</source><translation>Lần chạy hiện tại</translation></message>
    <message><source>Current selection</source><translation>Lựa chọn hiện tại</translation></message>
    <message><source>Current usage</source><translation>Mức sử dụng hiện tại</translation></message>
    <message><source>Date modified</source><translation>Ngày sửa đổi</translation></message>
    <message><source>Debug</source><translation>Gỡ lỗi</translation></message>
    <message><source>Delete and commit this file?</source><translation>Xóa tệp này và ghi việc xóa vào đĩa?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Xóa thư mục này và ghi việc xóa vào đĩa?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Xóa khóa sổ đăng ký này và ghi việc xóa vào đĩa?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Xóa giá trị sổ đăng ký này và ghi việc xóa vào đĩa?</translation></message>

    <message><source>Direct restart?</source><translation>Khởi động lại trực tiếp?</translation></message>
    <message><source>Direct shutdown?</source><translation>Tắt máy trực tiếp?</translation></message>
    <message><source>Discard changes</source><translation>Bỏ thay đổi</translation></message>
    <message><source>Disk</source><translation>Đĩa</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Lưu bền vững khóa bí mật miền</translation></message>
    <message><source>Done</source><translation>Xong</translation></message>
    <message><source>Edit commands</source><translation>Chỉnh sửa lệnh</translation></message>
    <message><source>Editable</source><translation>Có thể chỉnh sửa</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Bật bộ lọc và khởi động lại để xem mức sử dụng trực tiếp</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>Chế độ nâng cao thêm một dịch vụ nền khởi động cùng Windows. Hãy gỡ bỏ dịch vụ trước khi di chuyển hoặc xóa ứng dụng.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>Chế độ nâng cao không khả dụng. Dùng Tắt máy an toàn hoặc Khởi động lại an toàn để lưu các tệp chờ.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Nhập HKLM\SOFTWARE\… rồi nhấn Enter để chuyển tới</translation></message>
    <message><source>Enter a file name</source><translation>Nhập tên tệp</translation></message>
    <message><source>Enter a valid file name.</source><translation>Nhập tên tệp hợp lệ.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Nhập tên tệp hoặc thư mục hợp lệ.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Nhập tên thư mục hợp lệ.</translation></message>
    <message><source>Error</source><translation>Lỗi</translation></message>
    <message><source>Error details</source><translation>Chi tiết lỗi</translation></message>
    <message><source>Error details and suggestions</source><translation>Chi tiết lỗi và đề xuất</translation></message>
    <message><source>Expand</source><translation>Mở rộng</translation></message>
    <message><source>Export list</source><translation>Xuất danh sách</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Không thể xóa nhật ký: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Không thể xóa nhật ký: lỗi ngoài dự kiến.</translation></message>
    <message><source>File name</source><translation>Tên tệp</translation></message>
    <message><source>File or folder</source><translation>Tệp hoặc thư mục</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>Việc lưu tệp đã bắt đầu và không thể hủy.</translation></message>
    <message><source>File saving incomplete</source><translation>Lưu tệp chưa hoàn tất</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Sẽ bỏ qua việc lưu tệp chờ. Các thay đổi chưa ghi trên đường dẫn được bảo vệ sẽ mất sau khi khởi động lại.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Sẽ bỏ qua việc lưu tệp chờ. Các thay đổi chưa ghi trên đường dẫn được bảo vệ sẽ mất sau khi tắt máy.</translation></message>
    <message><source>File type</source><translation>Loại tệp</translation></message>
    <message><source>Files and folders</source><translation>Tệp và thư mục</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Có thể chọn đồng thời tệp và thư mục.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Các tệp chờ sẽ không được lưu.</translation></message>
    <message><source>Filter is disabled</source><translation>Bộ lọc đang tắt</translation></message>
    <message><source>Fit columns to contents</source><translation>Khớp độ rộng cột với nội dung</translation></message>
    <message><source>Folder address</source><translation>Địa chỉ thư mục</translation></message>
    <message><source>Folder name</source><translation>Tên thư mục</translation></message>
    <message><source>Folders</source><translation>Thư mục</translation></message>
    <message><source>Full configuration</source><translation>Cấu hình đầy đủ</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Các lần ghi tiếp theo vào khóa này sẽ không được UWF bảo vệ.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Các lần ghi tiếp theo vào đường dẫn này sẽ không được UWF bảo vệ.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Ngoại lệ sổ đăng ký toàn cục · gồm các khóa con</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Lên thư mục cha hoặc chọn vị trí khác.</translation></message>
    <message><source>Home</source><translation>Thư mục cá nhân</translation></message>
    <message><source>Import configuration</source><translation>Nhập cấu hình</translation></message>
    <message><source>Import creates pending changes. Use Review and apply to write them to the system.</source><translation>Nhập cấu hình tạo các thay đổi đang chờ. Dùng Xem lại và áp dụng để ghi chúng vào hệ thống.</translation></message>
    <message><source>Import results</source><translation>Kết quả nhập</translation></message>
    <message><source>Imported</source><translation>Đã nhập</translation></message>
    <message><source>Imported commands still need to be applied. Select a row to view the complete result.</source><translation>Các lệnh đã nhập vẫn cần được áp dụng. Chọn một hàng để xem toàn bộ kết quả.</translation></message>
    <message><source>Imported: %1</source><translation>Đã nhập: %1</translation></message>
    <message><source>Information</source><translation>Thông tin</translation></message>
    <message><source>Item details</source><translation>Chi tiết mục</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Danh sách tệp chờ lớn có thể mất nhiều thời gian để lưu hơn.</translation></message>
    <message><source>License and copyright</source><translation>Giấy phép và bản quyền</translation></message>
    <message><source>Line</source><translation>Dòng</translation></message>
    <message><source>Local disk (%1)</source><translation>Đĩa cục bộ (%1)</translation></message>
    <message><source>Log level</source><translation>Mức nhật ký</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Đọc nhật ký thất bại. Thử lại hoặc kiểm tra quyền truy cập tệp nhật ký.</translation></message>
    <message><source>Maximize</source><translation>Phóng to</translation></message>
    <message><source>Message details</source><translation>Chi tiết thông báo</translation></message>
    <message><source>Minimize</source><translation>Thu nhỏ</translation></message>
    <message><source>Module</source><translation>Mô-đun</translation></message>
    <message><source>More</source><translation>Thêm</translation></message>
    <message><source>Move</source><translation>Di chuyển</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Nếu ổ đĩa chứa ứng dụng được UWF bảo vệ, trước tiên hãy đảm bảo các tệp ứng dụng vẫn còn sau khi khởi động lại. Việc lưu nhiều tệp chờ có thể khiến quá trình tắt máy hoặc khởi động lại mất nhiều thời gian hơn.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Có thể chọn nhiều tệp.</translation></message>
    <message><source>Name and location</source><translation>Tên và vị trí</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Giá trị có tên: %1 · chỉ giá trị này</translation></message>
    <message><source>Navigate to registry path</source><translation>Chuyển đến đường dẫn sổ đăng ký</translation></message>
    <message><source>New folder</source><translation>Thư mục mới</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Các bản ghi mới sẽ xuất hiện sau khi làm mới.</translation></message>
    <message><source>Next-session configuration</source><translation>Cấu hình phiên tiếp theo</translation></message>
    <message><source>No files to commit</source><translation>Không có tệp để ghi</translation></message>
    <message><source>No logs yet</source><translation>Chưa có nhật ký</translation></message>
    <message><source>No matching items</source><translation>Không có mục phù hợp</translation></message>
    <message><source>No matching logs</source><translation>Không có nhật ký phù hợp</translation></message>
    <message><source>No overlay files found</source><translation>Không tìm thấy tệp trong lớp phủ</translation></message>
    <message><source>No registry key selected</source><translation>Chưa chọn khóa sổ đăng ký</translation></message>
    <message><source>None</source><translation>Không có</translation></message>
    <message><source>Not configured</source><translation>Chưa cấu hình</translation></message>
    <message><source>Not connected</source><translation>Chưa kết nối</translation></message>
    <message><source>Not installed</source><translation>Chưa cài đặt</translation></message>
    <message><source>Not written to the system</source><translation>Chưa ghi vào hệ thống</translation></message>
    <message><source>Off</source><translation>Tắt</translation></message>
    <message><source>On</source><translation>Bật</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Mỗi dòng một lệnh uwfmgr</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Giữ ứng dụng trong một thư mục cố định mà người dùng tiêu chuẩn không thể sửa đổi. Vị trí đề xuất:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Chỉ các tệp chờ được lưu tự động. Các thay đổi chưa ghi khác trên đường dẫn được bảo vệ sẽ mất sau khi khởi động lại.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Chỉ các tệp chờ được lưu tự động. Các thay đổi chưa ghi khác trên đường dẫn được bảo vệ sẽ mất sau khi tắt máy.</translation></message>
    <message><source>Open</source><translation>Mở</translation></message>
    <message><source>Open folder</source><translation>Mở thư mục</translation></message>
    <message><source>Operating system</source><translation>Hệ điều hành</translation></message>
    <message><source>Operation and security</source><translation>Hoạt động và bảo mật</translation></message>
    <message><source>Original command</source><translation>Lệnh gốc</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Lớp phủ đã đạt ngưỡng nguy cấp. Lưu thay đổi và khởi động lại an toàn.</translation></message>
    <message><source>Overlay maximum size</source><translation>Kích thước lớp phủ tối đa</translation></message>
    <message><source>Overlay type</source><translation>Loại lớp phủ</translation></message>
    <message><source>Overlay usage</source><translation>Mức sử dụng lớp phủ</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Lớp phủ đã đạt ngưỡng cảnh báo. Lưu thay đổi và khởi động lại an toàn.</translation></message>
    <message><source>Overview</source><translation>Tổng quan</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Dán lệnh hoặc tải từ tệp…</translation></message>
    <message><source>Pending changes</source><translation>Thay đổi đang chờ</translation></message>
    <message><source>Pending removal</source><translation>Chờ loại bỏ</translation></message>
    <message><source>Permissions and protection</source><translation>Quyền và bảo vệ</translation></message>
    <message><source>Privileges</source><translation>Đặc quyền</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>Đã xử lý %1 / %2 mục</translation></message>
    <message><source>Processing result</source><translation>Kết quả xử lý</translation></message>
    <message><source>Protected</source><translation>Được bảo vệ</translation></message>
    <message><source>Protected volumes</source><translation>Ổ đĩa được bảo vệ</translation></message>
    <message><source>Protection and overlay</source><translation>Bảo vệ và lớp phủ</translation></message>
    <message><source>Quick access</source><translation>Truy cập nhanh</translation></message>
    <message><source>Raw report</source><translation>Báo cáo thô</translation></message>
    <message><source>Read again</source><translation>Đọc lại</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Đọc lại để lấy ảnh chụp chẩn đoán.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Đọc cấu hình để hiển thị mức sử dụng trực tiếp</translation></message>
    <message><source>Read-only</source><translation>Chỉ đọc</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Đọc lớp phủ lớn có thể lâu hơn. Bạn có thể hủy bất cứ lúc nào.</translation></message>
    <message><source>Reading canceled</source><translation>Đã hủy đọc</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Đang đọc tệp và thư mục con tại vị trí này…</translation></message>
    <message><source>Reading folder</source><translation>Đang đọc thư mục</translation></message>
    <message><source>Reading logs</source><translation>Đang đọc nhật ký</translation></message>
    <message><source>Reading overlay</source><translation>Đang đọc lớp phủ</translation></message>
    <message><source>Ready</source><translation>Sẵn sàng</translation></message>
    <message><source>Ready to save</source><translation>Sẵn sàng lưu</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Các bản ghi của lần chạy hiện tại sẽ xuất hiện ở đây khi đọc xong.</translation></message>
    <message><source>Recursive staging</source><translation>Đưa vào danh sách chờ theo đệ quy</translation></message>
    <message><source>Refresh folder</source><translation>Làm mới thư mục</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Khóa sổ đăng ký hoặc thiết lập lưu bền vững</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Có thể hủy các tác vụ còn lại sau khi xử lý xong đích hiện tại.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Bỏ khóa này khỏi ngoại lệ của phiên tiếp theo.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Bỏ đường dẫn này khỏi ngoại lệ của phiên tiếp theo.</translation></message>
    <message><source>Removed</source><translation>Đã loại bỏ</translation></message>
    <message><source>Rename</source><translation>Đổi tên</translation></message>
    <message><source>Rename…</source><translation>Đổi tên…</translation></message>
    <message><source>Repair required</source><translation>Cần sửa chữa</translation></message>
    <message><source>Replace file</source><translation>Thay thế tệp</translation></message>
    <message><source>Replace the existing file?</source><translation>Thay thế tệp hiện có?</translation></message>
    <message><source>Required configuration</source><translation>Cấu hình bắt buộc</translation></message>
    <message><source>Restart anyway</source><translation>Vẫn khởi động lại</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Sẽ tiếp tục khởi động lại sau khi lưu. Giữ ứng dụng mở.</translation></message>
    <message><source>Restore</source><translation>Khôi phục</translation></message>
    <message><source>Retry</source><translation>Thử lại</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Xem chi tiết lỗi trước khi tiếp tục tắt máy.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Xem chi tiết lỗi trước khi khởi động lại.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Xem chi tiết thao tác. Đọc trạng thái hệ thống hiện tại trước khi thử lại các thay đổi có thể đã được ghi.</translation></message>
    <message><source>Review the result for each target.</source><translation>Xem kết quả của từng đích.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Kiểm tra các thay đổi này trước khi ghi vào hệ thống.</translation></message>
    <message><source>Running</source><translation>Đang chạy</translation></message>
    <message><source>Save</source><translation>Lưu</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Lưu tệp chờ vào đĩa rồi khởi động lại.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Lưu tệp chờ vào đĩa rồi tắt máy.</translation></message>
    <message><source>Saving staged files</source><translation>Đang lưu tệp chờ</translation></message>
    <message><source>Saving to disk</source><translation>Đang lưu vào đĩa</translation></message>
    <message><source>Saving…</source><translation>Đang lưu…</translation></message>
    <message><source>Scanning staged paths</source><translation>Đang quét các đường dẫn chờ</translation></message>
    <message><source>Search logs</source><translation>Tìm nhật ký</translation></message>
    <message><source>Search messages or modules</source><translation>Tìm thông báo hoặc mô-đun</translation></message>
    <message><source>Search paths</source><translation>Tìm đường dẫn</translation></message>
    <message><source>Search this folder</source><translation>Tìm trong thư mục này</translation></message>
    <message><source>Select a file or enter its name</source><translation>Chọn tệp hoặc nhập tên</translation></message>
    <message><source>Select a file to open.</source><translation>Chọn tệp để mở.</translation></message>
    <message><source>Select a folder.</source><translation>Chọn thư mục.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Chọn khóa để thao tác trên toàn bộ cây con hoặc chọn giá trị có tên để chỉ thao tác trên giá trị đó.</translation></message>
    <message><source>Select an available subkey</source><translation>Chọn khóa con khả dụng</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Chỉ chọn tệp hoặc mở thư mục để duyệt nội dung.</translation></message>
    <message><source>Select files or folders</source><translation>Chọn tệp hoặc thư mục</translation></message>
    <message><source>Select folder</source><translation>Chọn thư mục</translation></message>
    <message><source>Select folders only.</source><translation>Chỉ chọn thư mục.</translation></message>
    <message><source>Select registry key or value</source><translation>Chọn khóa hoặc giá trị sổ đăng ký</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Chọn khóa hoặc giá trị sổ đăng ký để xóa</translation></message>
    <message><source>Select the current folder</source><translation>Chọn thư mục hiện tại</translation></message>
    <message><source>Select this file</source><translation>Chọn tệp này</translation></message>
    <message><source>Select this registry key</source><translation>Chọn khóa sổ đăng ký này</translation></message>
    <message><source>Select this registry value</source><translation>Chọn giá trị sổ đăng ký này</translation></message>
    <message><source>Selected file</source><translation>Tệp đã chọn</translation></message>
    <message><source>Selected folder</source><translation>Thư mục đã chọn</translation></message>
    <message><source>Selection</source><translation>Lựa chọn</translation></message>
    <message><source>Service account</source><translation>Tài khoản dịch vụ</translation></message>
    <message><source>Service configuration needs repair</source><translation>Cấu hình dịch vụ cần sửa chữa</translation></message>
    <message><source>Service details</source><translation>Chi tiết dịch vụ</translation></message>
    <message><source>Service name</source><translation>Tên dịch vụ</translation></message>
    <message><source>Service status</source><translation>Trạng thái dịch vụ</translation></message>
    <message><source>Service stopped</source><translation>Dịch vụ đã dừng</translation></message>
    <message><source>Shut down anyway</source><translation>Vẫn tắt máy</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>Sẽ tiếp tục tắt máy sau khi lưu. Giữ ứng dụng mở.</translation></message>
    <message><source>Size</source><translation>Kích thước</translation></message>
    <message><source>Some changes could not be applied</source><translation>Không thể áp dụng một số thay đổi</translation></message>
    <message><source>Some files could not be saved</source><translation>Không thể lưu một số tệp</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Không thể đọc một số thông tin phần cứng. Các thông tin chẩn đoán khác vẫn có thể xem và sao chép.</translation></message>
    <message><source>Source code</source><translation>Mã nguồn</translation></message>
    <message><source>Staged file</source><translation>Tệp chờ lưu</translation></message>
    <message><source>Staged path</source><translation>Đường dẫn chờ lưu</translation></message>
    <message><source>Standard user</source><translation>Người dùng tiêu chuẩn</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Tự động khởi chạy cùng Windows</translation></message>
    <message><source>Startup type</source><translation>Kiểu khởi động</translation></message>
    <message><source>Status unavailable</source><translation>Không có trạng thái</translation></message>
    <message><source>Stopped</source><translation>Đã dừng</translation></message>
    <message><source>Storage</source><translation>Lưu trữ</translation></message>
    <message><source>System</source><translation>Hệ thống</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Thay đổi hệ thống khi bật UWF lần đầu</translation></message>
    <message><source>System memory</source><translation>Bộ nhớ hệ thống</translation></message>
    <message><source>System metadata</source><translation>Siêu dữ liệu hệ thống</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Siêu dữ liệu hệ thống chỉ dùng để chẩn đoán và không thể ghi vào đĩa bằng thao tác này.</translation></message>
    <message><source>Takes effect after restart</source><translation>Có hiệu lực sau khi khởi động lại</translation></message>
    <message><source>Target</source><translation>Đích</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Lưu bền vững giấy phép Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Trình cung cấp UWF không thể hoàn tất liệt kê. Điều này có thể xảy ra khi lớp phủ lớn hoặc tài nguyên hệ thống không đủ.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>Cấu hình đã được ghi vào hệ thống. Hầu hết thay đổi có hiệu lực sau khi khởi động lại.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>Cấu hình khớp với đích của phiên tiếp theo. Xem hoặc xuất lệnh trong Cấu hình đầy đủ.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Các bản ghi nhật ký hiện tại sẽ bị xóa và không thể khôi phục.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Thư mục đích không tồn tại.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Không thể đọc trạng thái dịch vụ chế độ nâng cao. Kiểm tra quyền dịch vụ rồi thử lại.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>Danh sách có thể thiếu tệp hoặc chứa tệp đã ghi trước đó. Không dùng danh sách này để quyết định các tệp cần ghi.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Mục đã chọn không còn tồn tại: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Dịch vụ đã cài đặt nhưng đang dừng. Hãy khởi động để khôi phục tự động lưu.</translation></message>
    <message><source>This PC</source><translation>Máy tính này</translation></message>
    <message><source>This folder is empty</source><translation>Thư mục này trống</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Báo cáo có thể chứa tên tài khoản, mã định danh thiết bị và đường dẫn tệp. Kiểm tra trước khi chia sẻ.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Lưu tập lệnh CMD UTF-8 này thành tệp .cmd và chạy trong Command Prompt. Chức năng xuất sử dụng cùng tập lệnh.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Thử từ khóa hoặc mức nhật ký khác.</translation></message>
    <message><source>Try another search term.</source><translation>Thử từ tìm kiếm khác.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Không thể thay đổi loại và kích thước tối đa khi bộ lọc đang bật. Tắt bộ lọc và khởi động lại trước.</translation></message>
    <message><source>UI agent</source><translation>Tác nhân giao diện</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 dòng</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>Không thể đọc cấu hình UWF. Xác nhận Unified Write Filter đã được cài trong Windows rồi đọc lại.</translation></message>
    <message><source>UWF feature</source><translation>Tính năng UWF</translation></message>
    <message><source>UWF filter</source><translation>Bộ lọc UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF hiện không khả dụng</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Sẽ chạy trực tiếp chức năng khởi động lại an toàn của UWF.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>Sẽ chạy trực tiếp chức năng tắt máy an toàn của UWF.</translation></message>
    <message><source>Unable to access this folder</source><translation>Không thể truy cập thư mục này</translation></message>
    <message><source>Unable to access this registry key</source><translation>Không thể truy cập khóa sổ đăng ký này</translation></message>
    <message><source>Unable to read logs</source><translation>Không thể đọc nhật ký</translation></message>
    <message><source>Unable to read service status</source><translation>Không thể đọc trạng thái dịch vụ</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Hiện không thể đọc lớp phủ</translation></message>
    <message><source>Unable to read this folder.</source><translation>Không thể đọc thư mục này.</translation></message>
    <message><source>Unable to select this item</source><translation>Không thể chọn mục này</translation></message>
    <message><source>Unavailable</source><translation>Không khả dụng</translation></message>
    <message><source>Undo removal</source><translation>Hoàn tác loại bỏ</translation></message>
    <message><source>Unprotected</source><translation>Không được bảo vệ</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Các mục thất bại chưa được lưu. Chọn hàng để xem đường dẫn đầy đủ và nguyên nhân.</translation></message>
    <message><source>Up one folder</source><translation>Lên thư mục cha</translation></message>
    <message><source>Usage unavailable</source><translation>Không có mức sử dụng</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Dùng toàn bộ khóa, gồm mọi khóa con</translation></message>
    <message><source>Version %1</source><translation>Phiên bản %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Xem bản ghi ứng dụng và thông tin chẩn đoán.</translation></message>
    <message><source>View commands for these changes</source><translation>Xem lệnh cho các thay đổi này</translation></message>
    <message><source>View failure details</source><translation>Xem chi tiết lỗi</translation></message>
    <message><source>View overlay</source><translation>Xem lớp phủ</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Xem ảnh chụp chẩn đoán tệp của %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Liên kết ổ đĩa %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Bảo vệ ổ đĩa %1</translation></message>
    <message><source>Volume identity</source><translation>Định danh ổ đĩa</translation></message>
    <message><source>Warning</source><translation>Cảnh báo</translation></message>
    <message><source>Warning %1 MB</source><translation>Cảnh báo %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Toàn bộ khóa và mọi khóa con · đệ quy</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF cung cấp bảo vệ ghi. Ứng dụng này cấu hình và quản lý tính năng đó.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Khi không dùng chế độ nâng cao, hãy dùng Tắt máy an toàn hoặc Khởi động lại an toàn trong ứng dụng này để lưu tệp chờ.</translation></message>
    <message><source>Write protection</source><translation>Bảo vệ ghi</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Ghi các thay đổi cấu hình này vào hệ thống? Hầu hết có hiệu lực sau khi khởi động lại.</translation></message>
    <message><source>staged files</source><translation>tệp chờ lưu</translation></message>
    <message><source>uwfmgr commands</source><translation>lệnh uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Đánh dấu các đường dẫn đã chọn để xóa. Có hiệu lực sau khi áp dụng.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Xóa khỏi danh sách lưu tệp tự động sau khi áp dụng.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Thêm vào danh sách lưu tệp tự động sau khi áp dụng.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Áp dụng kế hoạch này sẽ xóa danh sách lưu tệp đã lưu và xóa mục loại trừ sổ đăng ký liên quan khi không còn cần thiết.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Các thay đổi danh sách lưu tệp phải được áp dụng trong UWF Manager và không thể xuất thành lệnh uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Danh sách lưu tệp được lưu sau khi áp dụng</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Phạm vi ảnh chụp lớp phủ</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Danh sách này chỉ dùng để chẩn đoán. Không thể dùng để xác định tệp nào cần được ghi nhận.</translation></message>
    <message><source>NTFS volumes only</source><translation>Chỉ hỗ trợ ổ đĩa NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Lớp phủ lớn có thể đọc chậm hoặc thất bại do thiếu tài nguyên.</translation></message>
    <message><source>Small files may be missing</source><translation>Có thể thiếu các tệp nhỏ</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Các tệp nhỏ hơn kích thước cụm đĩa (thường là 4 KB) có thể không xuất hiện trong danh sách.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Không cho biết trạng thái chờ ghi nhận</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Các tệp đã ghi nhận, đường dẫn bị loại trừ và các mục chịu ảnh hưởng của thao tác khác cũng có thể xuất hiện.</translation></message>

    <message><source>Command processing result</source><translation>Kết quả xử lý lệnh</translation></message>
    <message><source>Copy command</source><translation>Sao chép lệnh</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · Không đọc được %1 nhánh</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>Không đọc được %1 nhánh. Tiếp tục với nội dung đã tìm thấy hoặc chỉ thử lại các mục bị lỗi.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 tệp · %2 thư mục</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 giá trị · %2 khóa</translation></message>
    <message><source>Attempts</source><translation>Số lần thử</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Hủy các tác vụ còn lại</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Chỉ ghi cố định nội dung đã duyệt thành công. Các nhánh không đọc được sẽ giữ nguyên.</translation></message>
    <message><source>Committing changes</source><translation>Đang ghi cố định thay đổi</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Không thể hoàn tác các thao tác xóa và ghi cố định đã hoàn tất.</translation></message>
    <message><source>Continue…</source><translation>Tiếp tục…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Chỉ xóa và ghi cố định các đích đã tìm thấy. Giữ lại các nhánh không đọc được cùng thư mục hoặc khóa cha của chúng.</translation></message>
    <message><source>Deleting and committing</source><translation>Đang xóa và ghi cố định</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Không thể hoàn tác việc xóa và ghi cố định. Chỉ xử lý phạm vi đã chọn; không áp dụng cấu hình UWF đang chờ.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Đang xác định phạm vi. Ghi lại các nhánh không đọc được và tiếp tục duyệt nội dung còn lại.</translation></message>
    <message><source>Export traversal records</source><translation>Xuất nhật ký duyệt</translation></message>
    <message><source>No actionable content</source><translation>Không có nội dung để xử lý</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Không tìm thấy đích có thể xử lý. Chưa xóa hoặc ghi cố định nội dung nào.</translation></message>
    <message><source>No changes have been committed.</source><translation>Chưa ghi cố định thay đổi nào</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Chưa xóa hoặc ghi cố định nội dung nào</translation></message>
    <message><source>Path</source><translation>Đường dẫn</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Xóa vĩnh viễn phạm vi đã chọn và lưu kết quả xóa vào ổ đĩa được bảo vệ.</translation></message>
    <message><source>Preparation complete</source><translation>Đã chuẩn bị xong</translation></message>
    <message><source>Preparing commit</source><translation>Đang chuẩn bị ghi cố định</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Đang chuẩn bị xóa và ghi cố định</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Chỉ xử lý nội dung đã duyệt thành công</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Bỏ qua các điểm phân tích lại để việc duyệt không vượt quá phạm vi đã chọn.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Khởi động lại không hoàn tác việc ghi cố định. Chỉ xử lý phạm vi đã chọn; không áp dụng cấu hình UWF đang chờ.</translation></message>
    <message><source>Retry failed items</source><translation>Thử lại các mục bị lỗi</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Lưu các thay đổi trong phạm vi đã chọn vào ổ đĩa được bảo vệ.</translation></message>
    <message><source>Stopping the operation</source><translation>Đang dừng thao tác</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Đã giữ lại các bản ghi thành công và thất bại</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Thư mục không trống; nội dung chưa xử lý đã được giữ lại.</translation></message>
    <message><source>The operation could not be completed</source><translation>Không thể hoàn tất thao tác</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>Khóa đăng ký có giá trị mới; nội dung chưa xử lý đã được giữ lại.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>Khóa đăng ký vẫn còn khóa con; nội dung chưa xử lý đã được giữ lại.</translation></message>
    <message><source>The target no longer exists.</source><translation>Đích không còn tồn tại.</translation></message>
    <message><source>Traversal complete</source><translation>Đã duyệt xong</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Đã duyệt xong, một số nội dung không đọc được</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Nhật ký duyệt được giữ lại và có thể xuất.</translation></message>
    <message><source>Traverse again</source><translation>Duyệt lại</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Duyệt lại hoặc đóng cửa sổ này.</translation></message>
    <message><source>Traversing the folder</source><translation>Đang duyệt thư mục</translation></message>
    <message><source>Traversing the registry</source><translation>Đang duyệt sổ đăng ký</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Không thể chuẩn bị thao tác</translation></message>
    <message><source>View results</source><translation>Xem kết quả</translation></message>

    <message><source>All changes committed</source><translation>Đã ghi tất cả thay đổi</translation></message>
    <message><source>Some changes committed</source><translation>Đã ghi một phần thay đổi</translation></message>
    <message><source>No changes could be committed</source><translation>Không thể ghi thay đổi nào</translation></message>
    <message><source>No changes to commit</source><translation>Không có thay đổi để ghi</translation></message>
    <message><source>Commit interrupted</source><translation>Đã gián đoạn việc ghi</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 mục chưa được xử lý.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 thao tác sổ đăng ký thất bại.</translation></message>
    <message><source>The folder does not exist.</source><translation>Thư mục không tồn tại.</translation></message>
    <message><source>Access denied.</source><translation>Truy cập bị từ chối.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Đã có mục trùng tên này.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Không thể tạo thư mục. Hãy kiểm tra tên và quyền truy cập.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Không thể đổi tên mục. Hãy kiểm tra tên và quyền truy cập.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Trình duyệt này không hỗ trợ đổi tên.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Bản nháp danh sách tệp chờ lưu đã thay đổi sau khi mở kế hoạch này. Hãy xem lại kế hoạch mới.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Danh sách tệp chờ lưu đã lưu thay đổi sau khi mở kế hoạch này. Hãy làm mới và xem lại các thay đổi.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Danh sách tệp chờ lưu đã lưu không khớp với kế hoạch được chấp nhận.</translation></message>
    <message><source>Export configuration</source><translation>Xuất cấu hình</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Lưu chủ đề, ngôn ngữ và trạng thái hiển thị HUB hiện tại vào UWF.ini trong cùng thư mục với ứng dụng này.</translation></message>
    <message><source>Configuration saved</source><translation>Đã lưu cấu hình</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Đã lưu cài đặt giao diện hiện tại vào:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini đã tồn tại. Tệp hiện có không bị thay đổi:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>Không thể lưu UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Màn hình nền</translation></message>
    <message><source>Downloads</source><translation>Tải xuống</translation></message>
    <message><source>Documents</source><translation>Tài liệu</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Đã dừng ghi tệp tự động. Hãy khắc phục lỗi trước khi thử lại.</translation></message>
</context>
</TS>
