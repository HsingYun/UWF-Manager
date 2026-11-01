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
    <message><source>Disabled</source><comment>service startup type</comment><translation>Đã tắt</translation></message>
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
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Bảo vệ ổ đĩa này trong phiên tiếp theo. Các thao tác ghi vào đường dẫn được bảo vệ sẽ chuyển vào lớp phủ; việc giữ dữ liệu qua các lần khởi động lại tùy thuộc vào cài đặt lớp phủ.</translation></message>
    <message><source>Current session</source><translation>Phiên hiện tại</translation></message>
    <message><source>Next session</source><translation>Phiên tiếp theo</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Cách nhận diện ổ đĩa. Ký tự ổ đĩa đơn giản hơn nhưng liên kết mất hiệu lực nếu ký tự đổi, ví dụ sau khi thêm hoặc tháo đĩa. ID ổ đĩa vẫn ổn định.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF không hỗ trợ ổ đĩa này: </translation></message>
    <message><source>Global settings</source><translation>Thiết lập chung</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Trạng thái bộ lọc UWF trong phiên hiện tại (chỉ đọc).</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Bật UWF trong phiên tiếp theo. Các thao tác ghi vào đường dẫn được bảo vệ sẽ chuyển vào lớp phủ; việc giữ dữ liệu qua các lần khởi động lại tùy thuộc vào cài đặt lớp phủ.</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>Để đổi loại hoặc dung lượng tối đa của lớp phủ:
1. Tắt UWF.
2. Khởi động lại thiết bị.
3. Đổi cài đặt này.</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM dùng bộ nhớ hệ thống. Disk dùng ổ đĩa hệ thống và hỗ trợ lớp phủ lớn hơn. Lớp phủ bền vững có thể giữ dữ liệu lớp phủ Disk qua các lần khởi động lại.</translation></message>
    <message><source>Type</source><translation>Loại</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>Dung lượng lớp phủ RAM bị giới hạn bởi bộ nhớ hệ thống. Lớp phủ Disk cần ít nhất 1024 MB và đủ chỗ trống trên ổ đĩa hệ thống.</translation></message>
    <message><source>Maximum capacity</source><translation>Dung lượng tối đa</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>Dung lượng tối đa · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Ngưỡng cảnh báo</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Ghi thông báo mức cảnh báo vào nhật ký sự kiện khi mức sử dụng lớp phủ đạt giá trị này. Khi cả hai sự kiện được bật, giá trị này phải thấp hơn ngưỡng nghiêm trọng. Giá trị 0 chỉ tắt sự kiện cảnh báo.</translation></message>
    <message><source>Critical threshold</source><translation>Ngưỡng nghiêm trọng</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Ghi thông báo mức nghiêm trọng vào nhật ký sự kiện khi mức sử dụng lớp phủ đạt giá trị này. Khi cả hai sự kiện được bật, giá trị này phải cao hơn ngưỡng cảnh báo. Giá trị 0 chỉ tắt sự kiện nghiêm trọng.</translation></message>
    <message><source>Overlay</source><translation>Lớp phủ</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Không hỗ trợ loại ổ đĩa này (chỉ hỗ trợ đĩa cố định cục bộ).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Ổ đĩa vượt giới hạn UWF là 16 TiB cho mỗi ổ được bảo vệ.</translation></message>

    <message><source>%1 free / %2</source><translation>Trống %1 / %2</translation></message>
    <message><source>Commit</source><translation>Ghi vĩnh viễn</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>Ghi vĩnh viễn các thay đổi từ lớp phủ xuống đĩa hoặc vào sổ đăng ký. Không thể hoàn tác.</translation></message>
    <message><source>Commit file changes…</source><translation>Ghi vĩnh viễn thay đổi tệp…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Chọn tệp và ghi vĩnh viễn các thay đổi của tệp trong lớp phủ xuống đĩa.</translation></message>
    <message><source>Commit folder changes…</source><translation>Ghi vĩnh viễn thay đổi thư mục…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Chọn thư mục và ghi vĩnh viễn các thay đổi trong lớp phủ của từng tệp bên trong xuống đĩa.</translation></message>
    <message><source>Delete and commit file…</source><translation>Xóa tệp và ghi vĩnh viễn thao tác xóa vào đĩa…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Chọn tệp cần xóa và ghi vĩnh viễn thao tác xóa xuống đĩa.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Xóa thư mục cùng nội dung và ghi vĩnh viễn thao tác xóa vào đĩa…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>Chọn thư mục cần xóa cùng toàn bộ nội dung và thư mục con, rồi ghi vĩnh viễn các thao tác xóa xuống đĩa.</translation></message>
    <message><source>Commit registry changes…</source><translation>Ghi vĩnh viễn thay đổi sổ đăng ký…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Nhập khóa sổ đăng ký và tên giá trị tùy chọn, rồi ghi vĩnh viễn các thay đổi vào sổ đăng ký. Để trống tên giá trị để ghi vĩnh viễn mọi giá trị của khóa và các khóa con.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Ghi vĩnh viễn thao tác xóa sổ đăng ký…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>Nhập khóa sổ đăng ký và tên giá trị tùy chọn. Để trống tên giá trị để xóa khóa cùng mọi khóa con và giá trị, rồi ghi vĩnh viễn các thao tác xóa.</translation></message>
    <message><source>File exclusions</source><translation>Loại trừ tệp</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Tệp và thư mục trên ổ được loại khỏi bảo vệ UWF. Nhấp đúp vào mục để sao chép đường dẫn.</translation></message>
    <message><source>Registry exclusions</source><translation>Loại trừ sổ đăng ký</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>Loại trừ sổ đăng ký là thiết lập chung, chỉ hiển thị ở phân vùng cài Windows.</translation></message>
    <message><source>File staging</source><translation>Danh sách ghi tự động</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>Thêm tệp hoặc thư mục vào danh sách ghi tự động.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Chọn một hoặc nhiều tệp để tự động ghi vĩnh viễn trước khi tắt máy hoặc khởi động lại an toàn.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Chọn thư mục để ghi vĩnh viễn các tệp bên trong, kể cả thư mục con, trước khi tắt máy hoặc khởi động lại an toàn.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Chọn tệp để tự động ghi vĩnh viễn</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Chọn thư mục để tự động ghi vĩnh viễn cùng thư mục con</translation></message>
    <message><source>Cannot stage this path</source><translation>Không thể thêm đường dẫn vào danh sách ghi tự động</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Đường dẫn này chồng lấn với một loại trừ tệp UWF. Xóa loại trừ trước khi thêm đường dẫn vào danh sách:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Đường dẫn này chồng lấn với một mục trong danh sách ghi tự động. Xóa đường dẫn khỏi danh sách trước khi thêm loại trừ:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Đường dẫn chồng lấn với một mục hiện có trong danh sách ghi tự động</translation></message>
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
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>Đường dẫn không có ký tự ổ đĩa nên không thể thêm vào danh sách ghi tự động:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Đường dẫn đã chọn nằm trên ổ %1. Hãy thêm từ thẻ Danh sách ghi tự động của ổ đó.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Không thể thêm thư mục gốc vì sẽ ghi vĩnh viễn đệ quy toàn bộ ổ đĩa.</translation></message>
    <message><source>File staging could not be saved</source><translation>Không thể lưu danh sách ghi tự động</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Đã tải lại trạng thái danh sách ghi tự động do không thể hoàn tất cập nhật:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Danh sách ghi tự động không khả dụng: %1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Mục trong danh sách: %1 · tệp: %2 · thư mục: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Danh sách ghi tự động cần mục loại trừ sổ đăng ký này để giữ lại qua các lần khởi động lại.</translation></message>
    <message><source>Built-in</source><translation>Tích hợp</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Bộ lọc UWF đang tắt. Không có thay đổi tích lũy trong lớp phủ để ghi vĩnh viễn.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Hệ thống tệp %1 không hỗ trợ ghi vĩnh viễn từng tệp.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>UWF không hỗ trợ ổ đĩa này.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Hệ thống tệp %1 không hỗ trợ ghi vĩnh viễn từng tệp. Chỉ có thể ghi vĩnh viễn sổ đăng ký (loại trừ áp dụng chung).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Ổ đĩa hiện không được UWF bảo vệ; không có thay đổi tệp để ghi vĩnh viễn.</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Chọn tệp cần ghi vĩnh viễn vào đĩa</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Chọn thư mục cần ghi vĩnh viễn vào đĩa</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Chọn tệp có thao tác xóa cần được ghi vĩnh viễn</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Chọn thư mục có thao tác xóa cần được ghi vĩnh viễn</translation></message>
    <message><source>Name</source><translation>Tên</translation></message>
    <message><source>Data</source><translation>Dữ liệu</translation></message>
    <message><source>(Default)</source><translation>(Mặc định)</translation></message>
    <message><source>(value not set)</source><translation>(chưa đặt giá trị)</translation></message>
    <message><source>(empty)</source><translation>(trống)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Không thể xem trước (giá trị quá lớn)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Mở chế độ chẩn đoán các tệp đang được lưu tạm trong lớp phủ của ổ đĩa này.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Chỉ hỗ trợ liệt kê tệp lớp phủ trên ổ NTFS.</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>Bộ lọc UWF hiện đã tắt; không ổ đĩa nào có lớp phủ để xem.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Ổ đĩa không được UWF bảo vệ nên không có lớp phủ để kiểm tra.</translation></message>
    <message><source>Failed: %1</source><translation>Thất bại: %1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>Lần liệt kê trước có thể vẫn đang chạy. Chưa thể bắt đầu lượt đọc mới.</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>Dịch vụ UWF không khả dụng khi liệt kê các tệp lớp phủ.</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>Không đủ bộ nhớ, hoặc hệ thống không hỗ trợ thao tác này. Tính năng này chỉ hỗ trợ ổ đĩa NTFS; hãy đóng các ứng dụng dùng nhiều bộ nhớ rồi thử lại.</translation></message>
    <message><source>Raw error: %1</source><translation>Mã lỗi: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Xuất danh sách tệp trong lớp phủ</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Tệp văn bản (*.txt);;Tất cả tệp (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Tệp lệnh (*.cmd);;Tất cả tệp (*)</translation></message>
    <message><source>Export failed</source><translation>Xuất thất bại</translation></message>
    <message><source>Export finished</source><translation>Đã xuất xong</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Thất bại: Mở tệp (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Thất bại: Ghi tệp (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Đã lưu %1 mục vào:
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Không thể loại trừ toàn bộ ổ đĩa (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>Không thể thêm tệp phân trang, hoán đổi và ngủ đông làm mục loại trừ.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Không thể loại trừ toàn bộ thư mục: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Không thể loại trừ toàn bộ thư mục: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Không thể loại trừ toàn bộ thư mục: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Không thể loại trừ tệp hệ thống quan trọng này: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Không thể loại trừ tệp sổ đăng ký người dùng NTUSER.DAT.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Khóa sổ đăng ký không được để trống.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Khóa sổ đăng ký không được bắt đầu bằng dấu gạch chéo ngược.</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>Đường dẫn sổ đăng ký không được chứa ký tự null.</translation></message>
    <message><source>A registry key component is too long.</source><translation>Một thành phần khóa sổ đăng ký quá dài.</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>Đường dẫn sổ đăng ký vượt quá giới hạn độ sâu lồng nhau của UWF.</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>Thêm tệp hoặc thư mục vào danh sách loại trừ.</translation></message>
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
    <message><source>Review all pending changes and apply them in one batch.</source><translation>Xem lại mọi thay đổi đang chờ và áp dụng cùng lúc.</translation></message>
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
    <message><source>The system did not find the target.</source><translation>Hệ thống không tìm thấy đối tượng.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Hệ thống từ chối tham số (đường dẫn hoặc đối số không hợp lệ).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Thao tác thất bại. Đối tượng có thể được tiến trình khác sử dụng, như thư mục đang mở trong Explorer hoặc tệp đang mở. Đóng các chương trình đó rồi thử lại.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Thao tác thất bại; xem nhật ký để biết chi tiết.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Thao tác bị từ chối (mã %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Không rõ.</translation></message>
    <message><source>Commit canceled</source><translation>Đã hủy ghi vĩnh viễn</translation></message>

    <message><source>Succeeded</source><translation>Thành công</translation></message>
    <message><source>Skipped</source><translation>Đã bỏ qua</translation></message>
    <message><source>Failed</source><translation>Thất bại</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>Thành công: %1; thất bại: %2.</translation></message>

    <message><source>Existed before</source><translation>Tồn tại trước khi xóa</translation></message>
    <message><source>Exists after</source><translation>Tồn tại sau khi xóa</translation></message>
    <message><source>Error code</source><translation>Mã lỗi</translation></message>
    <message><source>Reason</source><translation>Lý do</translation></message>
    <message><source>Previous page</source><translation>Trang trước</translation></message>
    <message><source>First page</source><translation>Trang đầu</translation></message>
    <message><source>Last page</source><translation>Trang cuối</translation></message>
    <message><source>Next page</source><translation>Trang sau</translation></message>
    <message><source>Copy selected rows</source><translation>Sao chép hàng đã chọn</translation></message>
    <message><source>Copy all</source><translation>Sao chép tất cả</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Cần giữ loại trừ sổ đăng ký này khi danh sách ghi tự động còn mục đã lưu. Xóa hết danh sách trước khi xóa loại trừ.</translation></message>
    <message><source>Copy current page</source><translation>Sao chép trang hiện tại</translation></message>
    <message><source>No pending changes</source><translation>Không có thay đổi chờ áp dụng</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Loại trừ sổ đăng ký · Thiết lập chung.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Chuyển đến cài đặt bảo vệ, loại trừ tệp và danh sách ghi tự động của ổ đĩa %1.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Không được hỗ trợ</translation></message>
    <message><source>Review and apply changes</source><translation>Xem lại và áp dụng thay đổi</translation></message>
    <message><source>Confirm apply</source><translation>Xác nhận áp dụng</translation></message>
    <message><source>Result</source><translation>Kết quả</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Bộ lọc: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Đặt ngưỡng cảnh báo: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Đặt ngưỡng nghiêm trọng: %1 MB</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ Chưa áp dụng loại / dung lượng tối đa: UWF đang bật. Hãy tắt UWF và khởi động lại trước.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Đặt loại lớp phủ: %1</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ Đã đặt dung lượng lớp phủ tối đa thành %1 MB</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Ổ đĩa %1 · Bảo vệ UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Ổ đĩa %1 · Nhận diện theo: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Ổ đĩa %1 · Thêm loại trừ tệp: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Ổ đĩa %1 · Xóa loại trừ tệp: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Thêm loại trừ sổ đăng ký: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Xóa loại trừ sổ đăng ký: %1</translation></message>
    <message><source>Time</source><translation>Thời gian</translation></message>
    <message><source>Level</source><translation>Mức độ</translation></message>

    <message><source>Message</source><translation>Thông báo</translation></message>
    <message><source>Committing staged files</source><translation>Đang ghi vĩnh viễn các tệp trong danh sách</translation></message>
    <message><source>No additional error details were provided.</source><translation>Không có thông tin lỗi bổ sung.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Tự động ghi vĩnh viễn tệp dừng đột ngột:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Tự động ghi vĩnh viễn tệp dừng do lỗi không xác định.</translation></message>
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
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>Tệp tìm thấy: %1 · Đã ghi vĩnh viễn: %2 · Tệp bị bỏ qua: %3 · Mục bị bỏ qua: %4 · Thất bại: %5</translation></message>
    <message><source>File staging preparation</source><translation>Chuẩn bị ghi vĩnh viễn các tệp trong danh sách</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Không hỗ trợ điểm phân tích lại.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Chỉ có thể thêm đường dẫn tuyệt đối trên ổ cục bộ.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Chỉ có thể thêm đường dẫn tuyệt đối trên ổ cục bộ.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Không thể kiểm tra đường dẫn trong danh sách.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Tệp trong danh sách hiện trỏ tới một loại đường dẫn khác.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Thư mục trong danh sách hiện trỏ tới một loại đường dẫn khác.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Không thể liệt kê đầy đủ thư mục trong danh sách.</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>Không thể xử lý đệ quy thư mục gốc của ổ đĩa.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Nhà cung cấp UWF đã từ chối ghi vĩnh viễn.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Ghi vĩnh viễn thất bại do lỗi không xác định.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Đã ghi thêm %1 lỗi vào nhật ký.</translation></message>



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
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>Nhập lệnh uwfmgr vào bản nháp. Các thay đổi chỉ được ghi sau khi xác nhận trong Xem lại và áp dụng.</translation></message>
    <message><source>Load from file…</source><translation>Tải từ tệp…</translation></message>
    <message><source>Load recommended configuration</source><translation>Tải cấu hình đề xuất</translation></message>
    <message><source>Choose recommended configuration</source><translation>Chọn cấu hình đề xuất</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Chọn các nhóm cấu hình đề xuất cần thêm. Có thể xem lại hoặc xóa từng lệnh trước khi nhập.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Theo tài liệu chính thức của Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Loại trừ UWF do Microsoft đề xuất</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>Xem lại các khuyến nghị này trước khi nhập.</translation></message>
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
    <message><source>Command import is not ready.</source><translation>Chức năng nhập lệnh chưa sẵn sàng.</translation></message>
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
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>Có %1 thay đổi chưa được áp dụng.

Bỏ các thay đổi này và tiếp tục?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Lệnh uwfmgr chưa đầy đủ</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Thiếu tham số: Dung lượng (MB)</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>Kích thước phải là số nguyên không âm (MB)</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Thiếu tham số: Lớp phủ · Loại (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Giá trị không hợp lệ: Lớp phủ · Loại: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Thiếu tham số: Ổ đĩa (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Ổ đĩa phải là ký tự như C:</translation></message>
    <message><source>Missing path argument</source><translation>Thiếu tham số: Đường dẫn</translation></message>
    <message><source>Missing registry key argument</source><translation>Thiếu tham số: Sổ đăng ký · Khóa</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Dấu ngoặc kép trong lệnh chưa đóng</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Tham số thừa ngoài dự kiến: %1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>Không thể nhập lệnh uwfmgr này.</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Đã ở trạng thái đích; không cần thao tác</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Đường dẫn không thuộc ổ này hoặc ổ không hỗ trợ loại trừ tệp (ví dụ exFAT / ReFS)</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>Không thể thêm đường dẫn này làm loại trừ.</translation></message>
    <message><source>Invalid size value: %1</source><translation>Giá trị không hợp lệ: Dung lượng: %1</translation></message>
    <message><source>maximum capacity</source><translation>dung lượng tối đa</translation></message>
    <message><source>warning threshold</source><translation>Ngưỡng cảnh báo</translation></message>
    <message><source>critical threshold</source><translation>Ngưỡng nghiêm trọng</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>Cấu hình chung không khả dụng; lệnh chưa được thêm vào các thay đổi chờ áp dụng.</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>Không rõ ổ %1 (không có cấu hình ổ đĩa phù hợp)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Ổ đĩa %1: Đã ở trạng thái đích; không cần thao tác</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>Đường dẫn %1 không có ký tự ổ đĩa; không thể xác định thẻ ổ đĩa tương ứng.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Ổ đĩa %1: UWF Không được hỗ trợ</translation></message>
    <message><source>file exclusion</source><translation>Loại trừ tệp</translation></message>
    <message><source>registry exclusion</source><translation>Loại trừ sổ đăng ký</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Không có thẻ sổ đăng ký nên không dùng được loại trừ sổ đăng ký</translation></message>
    <message><source>Unsupported command</source><translation>Lệnh: Không được hỗ trợ</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>Hiện hoặc ẩn tiện ích lớp phủ.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Đã bật</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Đã tắt</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Không rõ</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Đã dùng %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Chế độ nâng cao</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Phối hợp tự động ghi vĩnh viễn tệp với quá trình tắt và khởi động lại Windows.</translation></message>
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
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>Dịch vụ hiện không thể nhận thông báo trước khi Windows tắt.</translation></message>
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
    <message><source>Show main window</source><translation>Hiện cửa sổ chính</translation></message>
    <message><source>Restore default position</source><translation>Khôi phục vị trí mặc định</translation></message>
    <message><source>Show overlay widget</source><translation>Hiện tiện ích lớp phủ</translation></message>
    <message><source>Hide overlay widget</source><translation>Ẩn tiện ích lớp phủ</translation></message>
    <message><source>Exit application</source><translation>Thoát ứng dụng</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Thất bại: Đọc giá trị sổ đăng ký (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Liệt kê tệp lớp phủ: Lỗi không xác định.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Thất bại: Thao tác bộ lọc: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Thất bại: Đọc trạng thái bộ lọc</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Thất bại: Đặt ngưỡng cảnh báo</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Thất bại: Đặt ngưỡng nghiêm trọng</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Thất bại: Đọc trạng thái lớp phủ</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Thất bại: Đặt loại lớp phủ</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ Không thể đặt dung lượng tối đa</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Thất bại: Đọc cấu hình lớp phủ</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Thất bại: Bảo vệ ổ đĩa %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Thất bại: Đặt cách nhận diện ổ đĩa %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Ổ đĩa %1 · Thất bại: Thêm loại trừ tệp %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Ổ đĩa %1 · Thất bại: Xóa loại trừ tệp %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Thất bại: Đọc cấu hình ổ đĩa</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Thất bại: Thêm loại trừ sổ đăng ký %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Thất bại: Xóa loại trừ sổ đăng ký %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Thất bại: Cập nhật tùy chọn lưu giữ sổ đăng ký</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Thất bại: Đọc bộ lọc sổ đăng ký</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Không được hỗ trợ</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Đọc trạng thái đối tượng: Lỗi không xác định.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Lỗi không xác định.</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>Không thể xác minh trạng thái đích nên chưa rõ kết quả thao tác: %1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>Hệ thống đã chấp nhận yêu cầu xóa, nhưng đích vẫn tồn tại sau khi xác minh.</translation></message>
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
    <message><source>Committing</source><translation>Đang ghi vĩnh viễn</translation></message>
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
Quản lý bảo vệ ổ đĩa, lớp phủ, loại trừ và tệp trong danh sách ghi tự động.</translation></message>
    <message><source>Snapshot details</source><translation>Chi tiết ảnh chụp trạng thái</translation></message>
    <message><source>Account</source><translation>Tài khoản</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Thêm cấu hình UWF từ tập lệnh hoặc văn bản.</translation></message>
    <message><source>Add exclusion</source><translation>Thêm loại trừ</translation></message>
    <message><source>Add files or folders</source><translation>Thêm tệp hoặc thư mục</translation></message>
    <message><source>Add selected items</source><translation>Thêm các mục đã chọn</translation></message>
    <message><source>Added</source><translation>Đã thêm</translation></message>
    <message><source>Administrator</source><translation>Quản trị viên</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>Các thao tác đã chọn đã hoàn tất. UWF có thể bỏ qua các mục tiêu bị loại trừ.</translation></message>
    <message><source>All files (*)</source><translation>Tất cả tệp (*)</translation></message>
    <message><source>All levels</source><translation>Mọi mức</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Ảnh chụp chẩn đoán trống không có nghĩa là lớp phủ không chứa thay đổi.</translation></message>
    <message><source>Application path</source><translation>Đường dẫn ứng dụng</translation></message>
    <message><source>Application result</source><translation>Kết quả áp dụng</translation></message>
    <message><source>Apply changes</source><translation>Áp dụng thay đổi</translation></message>
    <message><source>Architecture</source><translation>Kiến trúc</translation></message>
    <message><source>Automatic</source><translation>Tự động</translation></message>
    <message><source>Automatic staged file commits</source><translation>Tự động ghi vĩnh viễn các tệp trong danh sách</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>Chưa hoàn tất ghi vĩnh viễn tệp tự động</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>Tự động ghi vĩnh viễn các tệp đủ điều kiện trong danh sách khi Windows tắt hoặc khởi động lại.</translation></message>
    <message><source>Available</source><translation>Khả dụng</translation></message>
    <message><source>Back</source><translation>Quay lại</translation></message>
    <message><source>Background service</source><translation>Dịch vụ nền</translation></message>
    <message><source>Binding</source><translation>Liên kết</translation></message>
    <message><source>By drive letter</source><translation>Theo ký tự ổ đĩa</translation></message>
    <message><source>By volume ID</source><translation>Theo ID ổ đĩa</translation></message>
    <message><source>Cancel reading</source><translation>Hủy đọc</translation></message>
    <message><source>Completed operations cannot be undone.</source><translation>Không thể hoàn tác các thao tác đã hoàn tất.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Chọn khóa sổ đăng ký cần loại trừ khỏi tính năng bảo vệ UWF, bao gồm các khóa con và giá trị của khóa đó.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Chọn vị trí để lưu tệp.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Chọn vị trí trong ổ đĩa hoặc thư mục được phép.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Chọn thư mục khác hoặc kiểm tra quyền truy cập rồi thử lại.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Chọn khóa khác hoặc kiểm tra quyền truy cập rồi thử lại.</translation></message>
    <message><source>Clear filters</source><translation>Xóa bộ lọc</translation></message>
    <message><source>Clear logs</source><translation>Xóa nhật ký</translation></message>
    <message><source>Clear logs?</source><translation>Xóa nhật ký?</translation></message>
    <message><source>Collapse</source><translation>Thu gọn</translation></message>
    <message><source>Commit changes</source><translation>Ghi vĩnh viễn các thay đổi</translation></message>
    <message><source>Commit changes…</source><translation>Ghi vĩnh viễn các thay đổi…</translation></message>
    <message><source>Commit file changes?</source><translation>Ghi vĩnh viễn các thay đổi của tệp?</translation></message>
    <message><source>Commit folder changes?</source><translation>Ghi vĩnh viễn các thay đổi của thư mục?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Ghi vĩnh viễn các thay đổi của khóa sổ đăng ký?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Ghi vĩnh viễn các thay đổi của giá trị sổ đăng ký?</translation></message>
    <message><source>Compatibility mode</source><translation>Chế độ tương thích</translation></message>
    <message><source>Configuration applied</source><translation>Đã áp dụng cấu hình</translation></message>
    <message><source>Configuration error</source><translation>Lỗi cấu hình</translation></message>
    <message><source>Configured</source><translation>Đã cấu hình</translation></message>
    <message><source>Confirm before continuing</source><translation>Xác nhận trước khi tiếp tục</translation></message>
    <message><source>Connected and authenticated</source><translation>Đã kết nối và xác thực</translation></message>
    <message><source>Connecting…</source><translation>Đang kết nối…</translation></message>
    <message><source>Contact author</source><translation>Liên hệ tác giả</translation></message>
    <message><source>Continue editing</source><translation>Tiếp tục chỉnh sửa</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>Nếu tiếp tục khởi động lại, những thay đổi tệp chưa được ghi vĩnh viễn có thể bị mất.</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>Nếu tiếp tục tắt máy, những thay đổi tệp chưa được ghi vĩnh viễn có thể bị mất.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Phối hợp với quá trình tắt Windows</translation></message>
    <message><source>Copy all logs</source><translation>Sao chép toàn bộ nhật ký</translation></message>
    <message><source>Copy full report</source><translation>Sao chép báo cáo đầy đủ</translation></message>
    <message><source>Copy name</source><translation>Sao chép tên</translation></message>
    <message><source>Copy path</source><translation>Sao chép đường dẫn</translation></message>
    <message><source>Copy selection</source><translation>Sao chép vùng chọn</translation></message>
    <message><source>Copy version</source><translation>Sao chép phiên bản</translation></message>
    <message><source>Counting staged files…</source><translation>Đang đếm tệp trong danh sách…</translation></message>
    <message><source>Create</source><translation>Tạo</translation></message>
    <message><source>Critical %1 MB</source><translation>Nguy cấp %1 MB</translation></message>
    <message><source>Current filter</source><translation>Bộ lọc hiện tại</translation></message>
    <message><source>Current run</source><translation>Lần chạy hiện tại</translation></message>
    <message><source>Current selection</source><translation>Lựa chọn hiện tại</translation></message>
    <message><source>Current usage</source><translation>Mức sử dụng hiện tại</translation></message>
    <message><source>Date modified</source><translation>Ngày sửa đổi</translation></message>
    <message><source>Debug</source><translation>Gỡ lỗi</translation></message>
    <message><source>Delete and commit this file?</source><translation>Xóa tệp này và ghi vĩnh viễn thao tác xóa?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Xóa thư mục này và ghi vĩnh viễn thao tác xóa?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Xóa khóa sổ đăng ký này và ghi vĩnh viễn thao tác xóa?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Xóa giá trị sổ đăng ký này và ghi vĩnh viễn thao tác xóa?</translation></message>

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
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>Chế độ nâng cao không khả dụng. Hãy chọn Tắt máy an toàn hoặc Khởi động lại an toàn để ghi vĩnh viễn các tệp trong danh sách.</translation></message>
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
    <message><source>File commits have started and cannot be canceled.</source><translation>Đã bắt đầu ghi vĩnh viễn các tệp và không thể hủy.</translation></message>
    <message><source>File commits incomplete</source><translation>Chưa hoàn tất ghi vĩnh viễn các tệp</translation></message>
    <message><source>File type</source><translation>Loại tệp</translation></message>
    <message><source>Files and folders</source><translation>Tệp và thư mục</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Có thể chọn đồng thời tệp và thư mục.</translation></message>
    <message><source>Filter is disabled</source><translation>Bộ lọc đang tắt</translation></message>
    <message><source>Fit columns to contents</source><translation>Điều chỉnh cột theo nội dung</translation></message>
    <message><source>Folder address</source><translation>Địa chỉ thư mục</translation></message>
    <message><source>Folder name</source><translation>Tên thư mục</translation></message>
    <message><source>Folders</source><translation>Thư mục</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>Thêm khóa này vào danh sách loại trừ của phiên tiếp theo.</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>Thêm đường dẫn này vào danh sách loại trừ của phiên tiếp theo.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Ngoại lệ sổ đăng ký toàn cục · gồm các khóa con</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Lên thư mục cha hoặc chọn vị trí khác.</translation></message>
    <message><source>Home</source><translation>Thư mục cá nhân</translation></message>
    <message><source>Import configuration</source><translation>Nhập cấu hình</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>Thao tác nhập cập nhật bản nháp. Xác nhận các thay đổi trong Xem lại và áp dụng để ghi vào hệ thống.</translation></message>
    <message><source>Import results</source><translation>Kết quả nhập</translation></message>
    <message><source>Imported</source><translation>Đã nhập</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>Danh sách hiển thị kết quả xử lý lệnh đối với bản nháp cấu hình; chọn một hàng để xem đầy đủ lệnh và chi tiết.</translation></message>
    <message><source>Imported: %1</source><translation>Đã nhập: %1</translation></message>
    <message><source>Information</source><translation>Thông tin</translation></message>
    <message><source>Item details</source><translation>Chi tiết mục</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>Ghi vĩnh viễn nhiều tệp trong danh sách có thể mất thêm thời gian.</translation></message>
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
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Nếu UWF bảo vệ ổ đĩa ứng dụng, hãy đảm bảo các tệp ứng dụng vẫn còn sau khi khởi động lại. Ghi vĩnh viễn nhiều tệp trong danh sách có thể khiến quá trình tắt hoặc khởi động lại mất thêm thời gian.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Có thể chọn nhiều tệp.</translation></message>
    <message><source>Name and location</source><translation>Tên và vị trí</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Giá trị có tên: %1 · chỉ giá trị này</translation></message>
    <message><source>Navigate to registry path</source><translation>Chuyển đến đường dẫn sổ đăng ký</translation></message>
    <message><source>New folder</source><translation>Thư mục mới</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Các bản ghi mới sẽ xuất hiện sau khi làm mới.</translation></message>
    <message><source>Next-session configuration</source><translation>Cấu hình phiên tiếp theo</translation></message>
    <message><source>No files to commit</source><translation>Không có tệp để ghi vĩnh viễn</translation></message>
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
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>Chỉ các tệp trong danh sách được ghi vĩnh viễn tự động. Những thay đổi khác chưa được ghi vĩnh viễn có thể bị mất sau khi khởi động lại.</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>Chỉ các tệp trong danh sách được ghi vĩnh viễn tự động. Những thay đổi khác chưa được ghi vĩnh viễn có thể bị mất sau khi tắt máy.</translation></message>
    <message><source>Open</source><translation>Mở</translation></message>
    <message><source>Open folder</source><translation>Mở thư mục</translation></message>
    <message><source>Operating system</source><translation>Hệ điều hành</translation></message>
    <message><source>Operation and security</source><translation>Hoạt động và bảo mật</translation></message>
    <message><source>Original command</source><translation>Lệnh gốc</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Lớp phủ đã đạt ngưỡng nguy cấp. Lưu thay đổi và khởi động lại an toàn.</translation></message>
    <message><source>Maximum overlay capacity</source><translation>Dung lượng lớp phủ tối đa</translation></message>
    <message><source>Overlay type</source><translation>Loại lớp phủ</translation></message>
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
    <message><source>Files to commit</source><translation>Tệp chờ ghi vĩnh viễn</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Các bản ghi của lần chạy hiện tại sẽ xuất hiện ở đây khi đọc xong.</translation></message>

    <message><source>Refresh folder</source><translation>Làm mới thư mục</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Khóa sổ đăng ký hoặc thiết lập lưu bền vững</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>Sau khi yêu cầu hủy, các tác vụ còn lại sẽ dừng khi thao tác hiện tại kết thúc.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Bỏ khóa này khỏi loại trừ của phiên tiếp theo.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Bỏ đường dẫn này khỏi loại trừ của phiên tiếp theo.</translation></message>
    <message><source>Removed</source><translation>Đã loại bỏ</translation></message>
    <message><source>Rename</source><translation>Đổi tên</translation></message>
    <message><source>Rename…</source><translation>Đổi tên…</translation></message>
    <message><source>Repair required</source><translation>Cần sửa chữa</translation></message>
    <message><source>Replace file</source><translation>Thay thế tệp</translation></message>
    <message><source>Replace the existing file?</source><translation>Thay thế tệp hiện có?</translation></message>
    <message><source>Required configuration</source><translation>Cấu hình bắt buộc</translation></message>
    <message><source>Restart anyway</source><translation>Vẫn khởi động lại</translation></message>
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>Quá trình khởi động lại sẽ tiếp tục sau khi ghi vĩnh viễn. Hãy giữ ứng dụng mở.</translation></message>
    <message><source>Restore</source><translation>Khôi phục</translation></message>
    <message><source>Retry</source><translation>Thử lại</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Xem chi tiết lỗi trước khi tiếp tục tắt máy.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Xem chi tiết lỗi trước khi khởi động lại.</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>Một số thay đổi có thể đã được ghi. Hãy làm mới trạng thái hệ thống trước khi thử lại.</translation></message>
    <message><source>Review the result for each target.</source><translation>Xem kết quả của từng đích.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Kiểm tra các thay đổi này trước khi ghi vào hệ thống.</translation></message>
    <message><source>Running</source><translation>Đang chạy</translation></message>
    <message><source>Save</source><translation>Lưu</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>Ghi vĩnh viễn các tệp trong danh sách xuống đĩa, rồi khởi động lại.</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>Ghi vĩnh viễn các tệp trong danh sách xuống đĩa, rồi tắt máy.</translation></message>

    <message><source>Committing to disk</source><translation>Đang ghi vĩnh viễn xuống đĩa</translation></message>
    <message><source>Committing…</source><translation>Đang ghi vĩnh viễn…</translation></message>
    <message><source>Scanning staged paths</source><translation>Đang quét các đường dẫn trong danh sách</translation></message>
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
    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>Quá trình tắt máy sẽ tiếp tục sau khi ghi vĩnh viễn. Hãy giữ ứng dụng mở.</translation></message>
    <message><source>Size</source><translation>Kích thước</translation></message>
    <message><source>Some changes could not be applied</source><translation>Không thể áp dụng một số thay đổi</translation></message>
    <message><source>Some files could not be committed</source><translation>Không thể ghi vĩnh viễn một số tệp</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Không thể đọc một số thông tin phần cứng. Các thông tin chẩn đoán khác vẫn có thể xem và sao chép.</translation></message>
    <message><source>Source code</source><translation>Mã nguồn</translation></message>

    <message><source>Staged path</source><translation>Đường dẫn trong danh sách</translation></message>
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
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Siêu dữ liệu hệ thống chỉ dùng để chẩn đoán và không thể ghi vĩnh viễn.</translation></message>
    <message><source>Takes effect after restart</source><translation>Có hiệu lực sau khi khởi động lại</translation></message>
    <message><source>Takes effect when applied</source><translation>Có hiệu lực khi áp dụng</translation></message>
    <message><source>Used when UWF starts</source><translation>Được dùng khi UWF khởi động</translation></message>
    <message><source>Mixed activation times</source><translation>Thời điểm có hiệu lực khác nhau</translation></message>
    <message><source>Target</source><translation>Đích</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Lưu bền vững giấy phép Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Trình cung cấp UWF không thể hoàn tất liệt kê. Điều này có thể xảy ra khi lớp phủ lớn hoặc tài nguyên hệ thống không đủ.</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>Cấu hình đã được ghi vào hệ thống.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Các bản ghi nhật ký hiện tại sẽ bị xóa và không thể khôi phục.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Thư mục đích không tồn tại.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Không thể đọc trạng thái dịch vụ chế độ nâng cao. Kiểm tra quyền dịch vụ rồi thử lại.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>Danh sách có thể thiếu tệp hoặc chứa tệp đã ghi vĩnh viễn. Không dùng làm danh sách để ghi vĩnh viễn.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Mục đã chọn không còn tồn tại: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>Dịch vụ đã được cài đặt nhưng đang dừng. Hãy khởi chạy để tiếp tục tự động ghi vĩnh viễn các tệp.</translation></message>
    <message><source>This PC</source><translation>Máy tính này</translation></message>
    <message><source>This folder is empty</source><translation>Thư mục này trống</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Báo cáo có thể chứa tên tài khoản, mã định danh thiết bị và đường dẫn tệp. Kiểm tra trước khi chia sẻ.</translation></message>

    <message><source>Try another keyword or log level.</source><translation>Thử từ khóa hoặc mức nhật ký khác.</translation></message>
    <message><source>Try another search term.</source><translation>Thử từ tìm kiếm khác.</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>Tắt UWF và khởi động lại trước khi đổi loại hoặc dung lượng tối đa của lớp phủ.</translation></message>
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
    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>Một số mục chưa được xử lý xong hoặc không thể xác minh. Chọn một mục để xem đường dẫn đầy đủ và chi tiết.</translation></message>
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
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>Khi không dùng chế độ nâng cao, hãy chọn Tắt máy an toàn hoặc Khởi động lại an toàn trong ứng dụng này để ghi vĩnh viễn các tệp trong danh sách.</translation></message>
    <message><source>Write protection</source><translation>Bảo vệ ghi</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>Ghi các thay đổi cấu hình này vào hệ thống?</translation></message>
    <message><source>staged files</source><translation>tệp trong danh sách</translation></message>
    <message><source>uwfmgr commands</source><translation>lệnh uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Đánh dấu các đường dẫn đã chọn để xóa. Có hiệu lực sau khi áp dụng.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Xóa khỏi danh sách ghi tự động sau khi áp dụng.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Thêm vào danh sách ghi tự động sau khi áp dụng.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Áp dụng kế hoạch này sẽ xóa hết danh sách ghi tự động đã lưu và xóa loại trừ sổ đăng ký khi không còn cần thiết.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Thay đổi danh sách ghi tự động phải được áp dụng trong UWF Manager và không thể xuất thành lệnh uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Danh sách ghi tự động được lưu sau khi áp dụng</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Phạm vi ảnh chụp lớp phủ</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Danh sách này chỉ dùng để chẩn đoán. Không thể xác định tệp nào cần được ghi vĩnh viễn.</translation></message>
    <message><source>NTFS volumes only</source><translation>Chỉ hỗ trợ ổ đĩa NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Lớp phủ lớn có thể đọc chậm hoặc thất bại do thiếu tài nguyên.</translation></message>
    <message><source>Small files may be missing</source><translation>Có thể thiếu các tệp nhỏ</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Các tệp nhỏ hơn kích thước cụm đĩa (thường là 4 KB) có thể không xuất hiện trong danh sách.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Không cho biết các lần ghi vĩnh viễn đang chờ</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Các tệp đã ghi vĩnh viễn, đường dẫn được loại trừ và các mục chịu ảnh hưởng của thao tác khác cũng có thể xuất hiện.</translation></message>

    <message><source>Command processing result</source><translation>Kết quả xử lý lệnh</translation></message>
    <message><source>Copy command</source><translation>Sao chép lệnh</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · Không đọc được %1 nhánh</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>Không đọc được %1 nhánh. Tiếp tục với nội dung đã tìm thấy hoặc chỉ thử lại các mục bị lỗi.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 tệp · %2 thư mục</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 giá trị · %2 khóa</translation></message>
    <message><source>Attempts</source><translation>Số lần thử</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Hủy các tác vụ còn lại</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Chỉ ghi vĩnh viễn nội dung đã duyệt thành công. Các nhánh không đọc được giữ nguyên.</translation></message>
    <message><source>Committing changes</source><translation>Đang ghi vĩnh viễn các thay đổi</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Không thể hoàn tác các thao tác xóa và ghi vĩnh viễn đã hoàn tất.</translation></message>
    <message><source>Continue…</source><translation>Tiếp tục…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Chỉ xóa các đích đã duyệt thành công và ghi vĩnh viễn thao tác xóa; giữ lại các nhánh không đọc được cùng thư mục hoặc khóa cha của chúng.</translation></message>
    <message><source>Deleting and committing</source><translation>Đang xóa và ghi vĩnh viễn</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Không thể hoàn tác việc xóa và ghi vĩnh viễn. Chỉ xử lý phạm vi đã chọn; không áp dụng cấu hình UWF đang chờ.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Đang xác định phạm vi. Ghi lại các nhánh không đọc được và tiếp tục duyệt nội dung còn lại.</translation></message>
    <message><source>Export traversal records</source><translation>Xuất nhật ký duyệt</translation></message>
    <message><source>No actionable content</source><translation>Không có nội dung để xử lý</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Không tìm thấy đích có thể xử lý. Chưa xóa hoặc ghi vĩnh viễn nội dung nào.</translation></message>
    <message><source>No changes have been committed.</source><translation>Chưa ghi vĩnh viễn thay đổi nào.</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Chưa xóa hoặc ghi vĩnh viễn nội dung nào.</translation></message>
    <message><source>Path</source><translation>Đường dẫn</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>Xóa vĩnh viễn nội dung đã chọn và ghi vĩnh viễn thao tác xóa.</translation></message>
    <message><source>Preparation complete</source><translation>Đã chuẩn bị xong</translation></message>
    <message><source>Preparing commit</source><translation>Đang chuẩn bị ghi vĩnh viễn</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Đang chuẩn bị xóa và ghi vĩnh viễn</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Chỉ xử lý nội dung đã duyệt thành công</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Bỏ qua các điểm phân tích lại để việc duyệt không vượt quá phạm vi đã chọn.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Khởi động lại không hoàn tác việc ghi vĩnh viễn. Chỉ xử lý phạm vi đã chọn; không áp dụng cấu hình UWF đang chờ.</translation></message>
    <message><source>Retry failed items</source><translation>Thử lại các mục bị lỗi</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>Ghi vĩnh viễn các thay đổi trong phạm vi đã chọn.</translation></message>
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

    <message><source>All operations succeeded</source><translation>Tất cả thao tác thành công</translation></message>
    <message><source>Some operations succeeded</source><translation>Một số thao tác thành công</translation></message>
    <message><source>No operations succeeded</source><translation>Không có thao tác nào thành công</translation></message>
    <message><source>No targets to process</source><translation>Không có mục tiêu cần xử lý</translation></message>
    <message><source>Commit interrupted</source><translation>Ghi vĩnh viễn bị gián đoạn</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 mục chưa được xử lý.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 thao tác sổ đăng ký thất bại.</translation></message>
    <message><source>The folder does not exist.</source><translation>Thư mục không tồn tại.</translation></message>
    <message><source>Access denied.</source><translation>Truy cập bị từ chối.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Đã có mục trùng tên này.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Không thể tạo thư mục. Hãy kiểm tra tên và quyền truy cập.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Không thể đổi tên mục. Hãy kiểm tra tên và quyền truy cập.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Trình duyệt này không hỗ trợ đổi tên.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Bản nháp danh sách ghi tự động đã thay đổi sau khi mở kế hoạch này. Hãy xem lại kế hoạch mới.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Danh sách ghi tự động đã lưu thay đổi sau khi mở kế hoạch này. Hãy làm mới và xem lại các thay đổi.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Danh sách ghi tự động đã lưu không khớp với kế hoạch được chấp nhận.</translation></message>
    <message><source>Export interface settings</source><translation>Xuất cài đặt giao diện</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>Lưu giao diện, ngôn ngữ và trạng thái hiển thị tiện ích lớp phủ hiện tại vào UWF.ini bên cạnh ứng dụng này.</translation></message>
    <message><source>Interface settings saved</source><translation>Đã lưu cài đặt giao diện</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Đã dừng xử lý danh sách ghi tự động. Hãy khắc phục lỗi trước khi thử lại.</translation></message>
    <message><source>No net change for this setting</source><translation>Giá trị cuối cùng của cài đặt này không thay đổi sau khi nhập</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Giá trị bản nháp cuối cùng: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Giá trị bản nháp cuối cùng: %1 → %2 (giá trị yêu cầu: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Giá trị yêu cầu: %1 → %2; đã được thay thế bằng lệnh sau. Giá trị bản nháp cuối cùng → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Đã cập nhật bản nháp của %1</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Đã cập nhật bản nháp cài đặt bảo vệ ổ đĩa %1 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Chọn các mục cần xóa.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Hiện không thể khôi phục các mục đã chọn.</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>Ghi trực tiếp qua dung lượng trống</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>Lớp phủ bền vững</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>Xóa dữ liệu bền vững ở lần khởi động tiếp theo</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>Chế độ phương tiện chỉ đọc</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>Dùng dung lượng đĩa trống để ghi, giảm mức sử dụng lớp phủ dành riêng; hỗ trợ RAM (không khuyến nghị) và Disk (khuyến nghị).</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>Giữ các thay đổi ghi trong lớp phủ Disk sau khi khởi động lại; không dùng được với lớp phủ RAM.</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>Đặt lại lớp phủ bền vững: xóa các thay đổi ghi ở lần khởi động tiếp theo và khôi phục trạng thái khi lớp phủ được bật</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>Cần quyền quản trị viên; cấu hình lớp phủ chỉ có thể xem.</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>Tạm thời không thể chỉnh sửa cấu hình. Hãy đợi thao tác hiện tại hoàn tất.</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>Không thể đọc cấu hình lớp phủ. Hãy làm mới trước khi chỉnh sửa.</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>Hệ thống này không hỗ trợ xóa dữ liệu bền vững ở lần khởi động tiếp theo.</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>Không xác định được yêu cầu xóa hiện tại. Hãy làm mới cấu hình trước khi chỉnh sửa.</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>Để thay đổi yêu cầu này, UWF và lớp phủ bền vững Disk phải đang hoạt động trong phiên hiện tại.</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>Việc xóa ở lần khởi động tiếp theo yêu cầu giữ lớp phủ bền vững Disk. Hãy hủy yêu cầu xóa hoặc giữ lớp phủ bền vững.</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>Lớp phủ bền vững yêu cầu Disk. Hãy tắt lớp phủ bền vững hoặc chọn Disk.</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>Chế độ phương tiện chỉ đọc yêu cầu RAM. Hãy tắt chế độ này hoặc chọn RAM.</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>UWF đang bật trong phiên hiện tại. Hãy tắt UWF và khởi động lại trước khi thay đổi cài đặt này.</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>Cấu hình đã thay đổi. Hãy đóng rồi mở lại hộp thoại này trước khi chỉnh sửa.</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>Cài đặt lớp phủ nâng cao</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>Lớp phủ hiện tại</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>Lớp phủ tiếp theo</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>Bản nháp chứa cài đặt lớp phủ không tương thích hoặc không khả dụng. Hãy quay lại chỉnh sửa và sửa cấu hình lớp phủ.</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>Đã lên lịch xóa sạch ở lần khởi động tiếp theo. Hủy yêu cầu xóa sạch trước khi tắt lớp phủ bền vững.</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>Phiên tiếp theo không giữ lớp phủ bền vững Disk. Hãy giữ lớp phủ này trước khi yêu cầu xóa sạch.</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>Lớp phủ bền vững chỉ dùng được với Disk; lớp phủ của phiên tiếp theo là RAM.</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>Bật chế độ Phương tiện chỉ đọc cần RAM; lớp phủ của phiên tiếp theo là Disk.</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>Không thể xác nhận trạng thái lớp phủ bền vững hiện tại. Hãy đọc lại cấu hình.</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>Sẽ có hiệu lực ở lần khởi động tiếp theo sau khi áp dụng.</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>Đã lên lịch xóa</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>Chưa lên lịch xóa</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>Chỉ xóa một lần ở lần khởi động tiếp theo, sau đó tiếp tục giữ dữ liệu. Phiên hiện tại không bị xóa ngay lập tức.</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>Hủy yêu cầu xóa đã lên lịch ở lần khởi động tiếp theo và giữ dữ liệu lớp phủ bền vững.</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>Đặt lại lớp phủ bền vững · Yêu cầu một lần</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Đang xử lý…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>Vui lòng đợi thao tác hoàn tất.</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · Phiên tiếp theo</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>Quay lại cài đặt</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>Điều kiện thay đổi</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>Đã thay đổi</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>Kiểm tra xung đột cấu hình</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>Xung đột cấu hình</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>Điều kiện tắt</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>UWF và lớp phủ bền vững Disk phải đang hoạt động trong phiên hiện tại. Để xóa dữ liệu, cũng cần giữ lớp phủ bền vững Disk cho phiên tiếp theo.</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>Điều kiện bật</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Tính năng</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>Mô tả tính năng</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>Tìm hiểu về %1</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>Giải quyết các xung đột cấu hình sau trước khi giữ bản nháp.</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>Hoàn nguyên</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>Thời điểm có hiệu lực</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>Hiện không thể thay đổi cài đặt này</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>UWF phải được tắt trong phiên hiện tại và lớp phủ của phiên tiếp theo phải dùng Disk.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>UWF phải được tắt trong phiên hiện tại và lớp phủ của phiên tiếp theo phải dùng RAM.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>UWF phải được tắt trong phiên hiện tại. Hủy yêu cầu xóa dữ liệu bền vững đã lên lịch trước khi tắt lớp phủ bền vững.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>UWF phải được tắt trong phiên hiện tại. Nếu chỉ đặt UWF tắt ở phiên tiếp theo thì vẫn cần khởi động lại trước khi chỉnh sửa.</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>Điều kiện sử dụng</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Xem chi tiết</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>Chỉ xem</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>Lý do không thể thay đổi %1</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>UWF đang bật trong phiên hiện tại nên không thể thay đổi các cài đặt lớp phủ này. Hãy hoàn tác thay đổi hoặc tắt UWF và khởi động lại.</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>Xem điều kiện thay đổi</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>Chưa ghi thay đổi cấu hình nào. Bạn có thể thử lại.</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>Không cần thay đổi cấu hình.</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>Dùng “Lưu bền vững khóa bí mật miền” cho bí mật tài khoản máy tính miền ($MACHINE.ACC).</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>Không thể loại trừ chính khóa này. Hãy chọn một khóa con của nó.</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>Loại trừ sổ đăng ký phải chỉ định khóa con không trống trong HKEY_LOCAL_MACHINE (HKLM).</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>Tập lệnh này chỉ chứa các thiết đặt có thể biểu diễn bằng lệnh uwfmgr. Không bao gồm:</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>Liên kết ổ đĩa</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>Thiết đặt không thể xác định chính xác bằng ký tự ổ đĩa</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>Tập lệnh</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>Cấu hình khớp với mục tiêu của phiên tiếp theo. Xem hoặc xuất các thiết đặt được hỗ trợ trong “Tập lệnh”.</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>Thiết đặt có ký tự điều khiển trong đường dẫn hoặc tên</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>Lệnh phụ thuộc vào các thiết đặt bị lược bỏ</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>Các loại trừ UWF ngăn việc ghi vĩnh viễn mục tiêu này một cách tin cậy. Hãy xóa loại trừ xung đột và khởi động lại trước khi ghi.</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>Các loại trừ UWF ngăn việc ghi vĩnh viễn danh sách ghi tự động một cách tin cậy. Hãy xóa loại trừ xung đột và khởi động lại trước khi áp dụng thay đổi này.</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF không thể bảo đảm thao tác vẫn nhắm chính xác đến khóa sổ đăng ký này.</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>Xác nhận danh sách ghi tự động đã lưu là rỗng trước khi xóa loại trừ sổ đăng ký của nó.</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>Chờ xác nhận</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Rỗng</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>Ổ đĩa hệ thống hiện không được UWF bảo vệ; không có thay đổi sổ đăng ký nào cần ghi vĩnh viễn.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>Các thay đổi sổ đăng ký đã được lưu vĩnh viễn; không cần ghi qua UWF.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>Các thay đổi sổ đăng ký đã được lưu vĩnh viễn; UWF không thể thực hiện thao tác xóa này.</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>Đường dẫn loại trừ UWF phải dùng dấu gạch chéo ngược.</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>Đường dẫn tệp không được chứa ký tự null.</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>Bật bảo vệ ổ đĩa trong bản nháp này trước khi thêm loại trừ tệp.</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>Đường dẫn loại trừ chứa ký tự không được UWF hỗ trợ.</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>Đường dẫn loại trừ vượt quá giới hạn độ dài của UWF.</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>Không thể ghi nhận thay đổi tệp và sổ đăng ký khi HORM đang hoạt động.</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>Không thể ghi vào sổ đăng ký khi HORM đang hoạt động.</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>Không thể lưu vĩnh viễn các thao tác ghi vào sổ đăng ký khi chế độ phương tiện chỉ đọc đang hoạt động.</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>Không thể ghi nhận thay đổi tệp và sổ đăng ký khi chế độ phương tiện chỉ đọc đang hoạt động.</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>Chặn các thao tác ghi vào phương tiện vật lý, bao gồm siêu dữ liệu. Các mục loại trừ không bỏ qua bảo vệ. Không thể ghi nhận riêng lẻ các thay đổi tệp và sổ đăng ký; Full Commit là một thao tác toàn cục riêng biệt.</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>Không thể thay đổi cài đặt UWF khi HORM đang hoạt động.</translation></message>
    <message><source>Default value · this value only</source><translation>Giá trị mặc định · chỉ giá trị này</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>Các thay đổi tệp đã được lưu bền vững; không cần thực hiện commit UWF.</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>Lần quan sát thành công gần nhất về mức sử dụng lớp phủ: %1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>Bộ lưu trữ lớp phủ được quan sát nằm ngoài phạm vi kích thước hoặc loại có thể ghi</translation></message>
    <message><source>Overlay (last observed)</source><translation>Lớp phủ (quan sát gần nhất)</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>Không thể làm mới mức sử dụng lớp phủ.
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>Không thể làm mới mức sử dụng lớp phủ. Đang hiển thị lần quan sát thành công gần nhất từ %1.
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>Không thể ánh xạ đường dẫn tới ổ đĩa cục bộ.</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF: đã bật (quan sát gần nhất)</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>Các thay đổi tệp đã được lưu bền vững; UWF không thể thực hiện thao tác xóa này.</translation></message>
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>Còn trống %1 GB</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>Cần có quyền quản trị viên.</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>Một thao tác đang diễn ra. Vui lòng chờ.</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>Thử lưu toàn bộ nội dung thêm mới, chỉnh sửa và xóa trên mọi ổ đĩa được bảo vệ kể từ lần khởi động này. Phạm vi bao gồm tệp và thiết đặt hệ thống; bạn không thể chỉ chọn một phần thay đổi.</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>Không thể lưu tự động trong chế độ hiện tại</translation></message>
    <message><source>Cannot be undone once started</source><translation>Không thể hoàn tác sau khi bắt đầu</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>Chọn nơi lưu lớp phủ trên đĩa</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>Chọn nơi lưu lớp phủ trên đĩa.</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>Tạm thời không thể thay đổi cấu hình.</translation></message>
    <message><source>Configured, inactive this session</source><translation>Đã cấu hình, không có hiệu lực trong phiên này</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>Đã cấu hình; có hiệu lực sau khi khởi động lại</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>Đã cấu hình và sẽ có hiệu lực sau khi khởi động lại. Lớp phủ của lần khởi động này sẽ không được chuyển sang vị trí mới.</translation></message>
    <message><source>Configure…</source><translation>Cấu hình…</translation></message>
    <message><source>Current UWF</source><translation>UWF hiện tại</translation></message>
    <message><source>Currently configured storage volume</source><translation>Ổ lưu trữ hiện được cấu hình</translation></message>
    <message><source>Default behavior</source><translation>Hành vi mặc định</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>Hãy tắt HORM và khởi động lại trước khi ghi vĩnh viễn toàn bộ ổ đĩa.</translation></message>
    <message><source>Disabled · restart required</source><translation>Đã tắt · cần khởi động lại</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>Tắt HORM không tắt UWF hay chế độ phương tiện chỉ đọc và không lưu thay đổi trên các ổ đĩa được bảo vệ. Hãy khởi động lại trước khi kiểm tra các thao tác có thể dùng.</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>Hãy bật UWF và khởi động lại trước khi bật HORM.</translation></message>

    <message><source>Full volume commit</source><translation>Ghi vĩnh viễn toàn bộ ổ đĩa</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>Chế độ ghi vĩnh viễn toàn bộ ổ đĩa (bật đến khi khởi động lại)</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>Chế độ ghi vĩnh viễn toàn bộ ổ đĩa đã được bật. Hãy khởi động lại trước khi sử dụng lại.</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>Đã bắt đầu ghi vĩnh viễn toàn bộ ổ đĩa. Hãy khởi động lại trước khi bật HORM.</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>Chỉ có thể ghi vĩnh viễn toàn bộ ổ đĩa khi chế độ phương tiện chỉ đọc đang bật.</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>Chế độ ghi vĩnh viễn toàn bộ ổ đĩa vẫn được bật cho đến khi khởi động lại. Những thay đổi mới được lưu trực tiếp. Vẫn không thể ghi vĩnh viễn từng mục.</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>Không thể ghi vĩnh viễn toàn bộ ổ đĩa trên hệ thống này.</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM đang bật. Không thể thay đổi thiết đặt UWF hay ghi vĩnh viễn từng mục. Hãy tắt HORM và khởi động lại trước khi thay đổi.</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM đã tắt. Hãy khởi động lại máy tính trước khi thay đổi thiết đặt UWF.</translation></message>
    <message><source>HORM is not available on this system.</source><translation>Hệ thống này không hỗ trợ HORM.</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM yêu cầu cấu hình phương tiện chỉ đọc trên lớp phủ RAM hiện tại.</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM yêu cầu lớp phủ hiện tại là RAM.</translation></message>

    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>Trong chế độ Disk, cách triển khai của hệ thống có thể bật tùy chọn này theo mặc định; hãy kiểm tra trạng thái hiện tại hiển thị trong thiết đặt.</translation></message>
    <message><source>Inactive this session</source><translation>Không có hiệu lực trong phiên này</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>Các thay đổi tiếp theo cũng sẽ được lưu cho đến khi khởi động lại</translation></message>

    <message><source>Maintenance and recovery</source><translation>Bảo trì và khôi phục</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>Hãy bảo đảm đây là trạng thái bạn muốn giữ lại trước khi tiếp tục.</translation></message>

    <message><source>Next startup: %1</source><translation>Lần khởi động tiếp theo: %1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>Không có ổ lưu trữ phù hợp. Hãy làm mới danh sách ổ đĩa.</translation></message>
    <message><source>Not enabled</source><translation>Chưa bật</translation></message>
    <message><source>Not selected</source><translation>Chưa chọn</translation></message>
    <message><source>Not supported</source><translation>Không hỗ trợ</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>Sau khi bắt đầu, các thay đổi mới sẽ được ghi trực tiếp vào ổ đĩa cho đến khi bạn khởi động lại máy tính.</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>Mỗi lần chỉ dùng được một ổ lưu trữ. Chọn ổ mới sẽ thay thế vị trí cũ và đặt lớp phủ cho lần khởi động tiếp theo thành Disk.</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>Dung lượng lớp phủ (giữ thiết đặt hiện tại)</translation></message>
    <message><source>Overlay configuration</source><translation>Cấu hình lớp phủ</translation></message>

    <message><source>Processing…</source><translation>Đang xử lý…</translation></message>
    <message><source>Read state again</source><translation>Đọc lại trạng thái</translation></message>
    <message><source>Read the current UWF state first.</source><translation>Hãy đọc trạng thái UWF hiện tại trước.</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>Chế độ phương tiện chỉ đọc không hỗ trợ ghi vĩnh viễn từng mục hay lưu tự động các tệp trong danh sách lưu tạm.</translation></message>
    <message><source>Reading state…</source><translation>Đang đọc trạng thái…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>Hãy khởi động lại và đọc trạng thái hiện tại trước khi bật lại HORM.</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>Khôi phục cùng một màn hình nền và các chương trình đang chạy sau mỗi lần khởi động.</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>Xem các thao tác ghi vĩnh viễn có thể dùng và điều kiện thực hiện.</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>Lưu thay đổi trên tất cả ổ đĩa được bảo vệ</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>Lưu thay đổi trên tất cả ổ đĩa được bảo vệ. Hãy xem các ảnh hưởng trước khi tiếp tục.</translation></message>


    <message><source>State unknown</source><translation>Không rõ trạng thái</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>Không thể hoàn tác những thay đổi đã lưu thành công bằng cách khởi động lại.</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>Những thay đổi đã lưu thành công sẽ được giữ lại</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>Không rõ trạng thái HORM. Hãy đọc lại trạng thái trước khi tiếp tục.</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>Không rõ trạng thái HORM. Hãy làm mới trước khi tiếp tục.</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>Không rõ trạng thái ghi vĩnh viễn hoặc HORM. Hãy làm mới trước khi tiếp tục.</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>Không đọc được trạng thái UWF hiện tại. Hãy làm mới trước khi tiếp tục.</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>Lớp phủ hiện tại dùng chế độ phương tiện chỉ đọc RAM</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>Không đọc được chế độ bảo vệ hiện tại. Hãy làm mới trạng thái.</translation></message>
    <message><source>The result could not be confirmed</source><translation>Không xác nhận được kết quả</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>Mục loại trừ này đã được cấu hình nhưng không vượt qua bảo vệ trong phiên hiện tại.</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>Thao tác này cũng có thể thay đổi việc ghi trực tiếp qua dung lượng trống và lớp phủ bền vững. Hãy xem lại thiết đặt nâng cao trước khi khởi động lại.</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>Bảo vệ UWF đang bật trong phiên hiện tại</translation></message>
    <message><source>%1 (will be replaced)</source><translation>%1 (sẽ được thay thế)</translation></message>
    <message><source>Add full volume commit to the draft?</source><translation>Thêm thao tác ghi vĩnh viễn toàn bộ ổ đĩa vào bản nháp?</translation></message>
    <message><source>Add to draft</source><translation>Thêm vào bản nháp</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>Áp dụng cho mọi ổ đĩa được bảo vệ; chỉ bắt đầu khi bản nháp này được áp dụng.</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>Áp dụng bản nháp này sẽ bắt đầu ghi vĩnh viễn toàn bộ ổ đĩa. Không thể hoàn tác các thay đổi đã lưu bằng cách khởi động lại, và các thay đổi mới sẽ được lưu cho đến khi khởi động lại.</translation></message>
    <message><source>Configure storage location</source><translation>Cấu hình vị trí lưu trữ</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>Hệ thống này không hỗ trợ cấu hình vị trí lưu trữ.</translation></message>
    <message><source>Current: %1</source><translation>Hiện tại: %1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>Hãy tắt HORM và khởi động lại trước khi cấu hình vị trí lưu trữ.</translation></message>
    <message><source>Disable saved-state startup?</source><translation>Tắt khởi động từ trạng thái đã lưu?</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>Bản nháp: %1 · cần áp dụng</translation></message>
    <message><source>Enable saved-state startup?</source><translation>Bật khởi động từ trạng thái đã lưu?</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>Ghi vĩnh viễn toàn bộ ổ đĩa (chờ áp dụng)</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>Thao tác ghi vĩnh viễn toàn bộ ổ đĩa đang ở trong bản nháp. Thao tác chỉ bắt đầu sau khi bạn xác nhận trong Xem lại và áp dụng.</translation></message>
    <message><source>Hibernate once / resume many</source><translation>Ngủ đông một lần / khôi phục nhiều lần</translation></message>
    <message><source>Not started</source><translation>Chưa bắt đầu</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>Xóa thao tác ghi vĩnh viễn toàn bộ ổ đĩa khỏi các thay đổi đang chờ.</translation></message>
    <message><source>Save draft</source><translation>Lưu bản nháp</translation></message>

    <message><source>Will start when applied</source><translation>Sẽ bắt đầu khi áp dụng</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>Một ổ đĩa cố định đang được gắn kết không vượt qua bước kiểm tra bảo vệ. Hãy xem lại các ổ đĩa trong danh sách trước khi bật HORM.</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>Kiểm tra các ổ đĩa cố định đủ điều kiện dùng UWF và vẫn được gắn kết. Các phân vùng chưa gắn kết có thể được bỏ qua.</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>Không thể xác minh các điều kiện tiên quyết của HORM. Hãy đọc lại trạng thái trước khi tiếp tục.</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>Kiểm tra bảo vệ ổ đĩa cố định đang gắn kết</translation></message>
    <message><source>Not using a Disk overlay</source><translation>Không sử dụng lớp phủ Disk</translation></message>
    <message><source>Selected storage volume</source><translation>Ổ lưu trữ được chọn</translation></message>
    <message><source>System drive default (%1)</source><translation>Mặc định trên ổ đĩa hệ thống (%1)</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>Trạng thái bảo vệ đã thay đổi trong lần khởi động này. Hãy khởi động lại trước khi bật HORM.</translation></message>
    <message><source>Unable to read</source><translation>Không thể đọc</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>Chọn ổ đĩa để lưu các thay đổi tạm thời. Có hiệu lực vào lần khởi động tiếp theo.</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>Tắt UWF và khởi động lại trước khi cấu hình vị trí lưu trữ.</translation></message>
    <message><source>Met</source><translation>Đã đáp ứng</translation></message>
    <message><source>Not met</source><translation>Chưa đáp ứng</translation></message>
    <message><source>Partition</source><translation>Phân vùng</translation></message>


    <message><source>Protect all</source><translation>Bảo vệ tất cả</translation></message>
    <message><source>Hibernation is not supported</source><translation>Không hỗ trợ chế độ ngủ đông</translation></message>
    <message><source>Hibernation is not enabled</source><translation>Chưa bật chế độ ngủ đông</translation></message>
    <message><source>Fast startup only</source><translation>Chỉ hỗ trợ khởi động nhanh</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>Không thể dùng bảo vệ ổ đĩa hàng loạt trên hệ thống này.</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>Tắt HORM và khởi động lại trước khi thay đổi bảo vệ ổ đĩa.</translation></message>
    <message><source>Setup and operation</source><translation>Thiết lập và vận hành</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. Chuẩn bị cấu hình</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>Khi UWF hiện đang tắt, cấu hình lớp phủ RAM, chế độ phương tiện chỉ đọc, bảo vệ ổ đĩa và bật UWF cho lần khởi động tiếp theo. Áp dụng bản nháp. Chuẩn bị hỗ trợ ngủ đông S4 và bảo đảm có tệp ngủ đông loại đầy đủ.</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>Khởi động lại, rồi xác nhận phiên hiện tại đã bật UWF, dùng lớp phủ RAM và chế độ phương tiện chỉ đọc, đồng thời bảo vệ tất cả ổ đĩa cố định đã gắn kết mà UWF có thể bảo vệ. Xác nhận chế độ ngủ đông của hệ thống khả dụng.</translation></message>
    <message><source>2. Restart and verify</source><translation>2. Khởi động lại và xác minh</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. Bật HORM và chuyển sang ngủ đông</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>Thêm thao tác bật HORM vào bản nháp, rồi dùng Xem lại và áp dụng. Áp dụng bản nháp sẽ thực hiện thao tác bật HORM, tương đương với:</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>Sau khi bật thành công, chuẩn bị màn hình nền và ứng dụng, rồi chuyển sang ngủ đông một lần để lưu trạng thái cơ sở:</translation></message>
    <message><source>HORM usage</source><translation>Cách sử dụng HORM</translation></message>
    <message><source>Volume protection</source><translation>Bảo vệ ổ đĩa</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>Cấu hình bảo vệ cho tất cả ổ đĩa trong phiên tiếp theo.</translation></message>
    <message><source>Unprotect all</source><translation>Bỏ bảo vệ tất cả</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>Lần khởi động tiếp theo: bảo vệ tất cả ổ đĩa</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>Lần khởi động tiếp theo: bỏ bảo vệ tất cả ổ đĩa</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>Lần khởi động tiếp theo: thiết lập riêng từng ổ đĩa</translation></message>
    <message><source>System hibernation is available</source><translation>Chế độ ngủ đông của hệ thống khả dụng</translation></message>
    <message><source>Usage guide</source><translation>Hướng dẫn sử dụng</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>Thiết lập bảo vệ cho tất cả ổ đĩa đang chờ áp dụng</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>Bảo vệ ổ đĩa có hiệu lực sau khi khởi động lại</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>Không thể áp dụng đồng thời bỏ bảo vệ tất cả và bật HORM.</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>Không hỗ trợ bảo vệ ổ đĩa hàng loạt.</translation></message>
    <message><source>Applied</source><translation>Đã áp dụng</translation></message>
    <message><source>Not applied</source><translation>Chưa áp dụng</translation></message>
    <message><source>Could not confirm</source><translation>Không thể xác nhận</translation></message>



    <message><source>Storage volume:</source><translation>Ổ lưu trữ:</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>Các mục loại trừ không bỏ qua bảo vệ trong phiên khởi động này. Không thể lưu từng mục hoặc tự động lưu các tệp đã chuẩn bị. Lưu toàn bộ ổ đĩa ảnh hưởng đến tất cả ổ đĩa được bảo vệ; hãy xem mô tả trước khi tiếp tục.</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>Khôi phục lặp lại cùng một trạng thái ngủ đông</translation></message>
    <message><source>Remove full volume commit draft</source><translation>Xóa bản nháp ghi vĩnh viễn toàn bộ ổ đĩa</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>Chế độ phương tiện chỉ đọc đang bật</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>Không thể đọc dấu đánh dấu lưu trữ đã cấu hình.</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>Danh sách ghi tự động xung đột với mục loại trừ tệp:
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>Máy tính này không hỗ trợ ngủ đông đầy đủ</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>Nhiều ổ đĩa lưu trữ có dấu đánh dấu khác không.</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>Không thể xác nhận lớp phủ thuộc loại Đĩa.</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>Không thể đọc vị trí lưu trữ đã cấu hình.</translation></message>
    <message><source>The system drive could not be identified.</source><translation>Không thể xác định ổ đĩa hệ thống.</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>Khởi động lại trực tiếp bỏ qua bước ghi vĩnh viễn các tệp trong danh sách.</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>Tắt máy trực tiếp bỏ qua bước ghi vĩnh viễn các tệp trong danh sách.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>Các tệp trong danh sách sẽ không được ghi vĩnh viễn. Những thay đổi chưa được ghi vĩnh viễn có thể bị mất sau khi khởi động lại.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>Các tệp trong danh sách sẽ không được ghi vĩnh viễn. Những thay đổi chưa được ghi vĩnh viễn có thể bị mất sau khi tắt máy.</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>Ghi vĩnh viễn tệp trong danh sách trước khi tắt máy hoặc khởi động lại thông thường.</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>Không thể áp dụng đồng thời việc ghi vĩnh viễn toàn bộ ổ đĩa và bật HORM.</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>Không thể áp dụng đồng thời việc cấu hình vị trí lưu trữ và bật HORM.</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>Việc cấu hình vị trí lưu trữ và ghi vĩnh viễn toàn bộ ổ đĩa yêu cầu các chế độ UWF hiện tại khác nhau.</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>Chọn ổ đĩa lưu trữ trước khi áp dụng thay đổi.</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>Việc cấu hình vị trí lưu trữ trên đĩa xung đột với lớp phủ RAM được yêu cầu.</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>Áp dụng thay đổi bộ lọc và khởi động lại trước khi yêu cầu ghi vĩnh viễn toàn bộ ổ đĩa.</translation></message>
    <message><source>Canceled</source><translation>Đã hủy</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>Ổ đĩa lưu trữ đã xem xét không còn khả dụng. Quay lại chỉnh sửa và chọn ổ đĩa khác.</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>Không thể đăng ký ổ đĩa này với UWF vì bus lưu trữ của ổ đĩa không được hỗ trợ.</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>Không thể xác nhận khả năng hỗ trợ ổ đĩa vật lý. Làm mới trạng thái trước khi bật bảo vệ.</translation></message>
    <message><source>Paused</source><translation>Tạm dừng</translation></message>
    <message><source>Partially paused</source><translation>Tạm dừng một phần</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>Dữ liệu lớp phủ bền vững sẽ được giữ lại ở lần khởi động tiếp theo. Tự động ghi dữ liệu đã tạm dừng.</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>Mục tạm dừng: %1 · Tệp tạm dừng: %2</translation></message>
    <message><source>Read-only ROM</source><translation>ROM chỉ đọc</translation></message>
    <message><source>%1 changes · %2 commands</source><translation>%1 thay đổi · %2 lệnh</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>Hệ thống tệp %1: không hỗ trợ loại trừ tệp và xác nhận từng tệp.</translation></message>
    <message><source>Applying changes</source><translation>Đang áp dụng thay đổi</translation></message>
    <message><source>Automatic commits paused</source><translation>Đã tạm dừng xác nhận tự động</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>Xác nhận tệp tự động được tạm dừng đến khi khởi động lại sau khi thay đổi chế độ HORM.</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>Không thể xác nhận tệp tự động sau FullCommit.</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>Không thể xác nhận tệp tự động khi HORM đang hoạt động.</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>Tạm dừng xác nhận tự động các tệp đã chuẩn bị cho lần khởi động lại này.</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>Tạm dừng xác nhận tự động các tệp đã chuẩn bị cho lần tắt máy này.</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>Không thể xác nhận cấu hình được lưu bền vững. Hãy làm mới trạng thái hệ thống trước khi thay đổi thiết đặt này.</translation></message>
    <message><source>Elapsed: %1 s</source><translation>Đã trôi qua: %1 giây</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>Chỉ xác nhận các tệp đã chuẩn bị đáp ứng điều kiện.</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>ROM chỉ đọc không thể lưu bền vững các thiết đặt UWF thông thường trên ổ đĩa hệ thống được bảo vệ. Hãy tắt UWF và khởi động lại trước khi thay đổi.</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>Thao tác này không xác nhận các tệp đã chuẩn bị bị bỏ qua.</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>Các tệp đã chuẩn bị đã nằm trên bộ lưu trữ không được bảo vệ. Không cần xác nhận tự động.</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>Thay đổi ngưỡng cần xác nhận sổ đăng ký, không khả dụng trong ROM chỉ đọc.</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>Đang chờ Windows hoàn tất. Quá trình này có thể mất một lúc.</translation></message>
    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM đã tắt. Tạm dừng xác nhận tự động các tệp đã chuẩn bị đến khi khởi động lại. Thao tác thủ công tuân theo các khả năng UWF hiện tại.</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>Ảnh chụp cấu hình này bao gồm các thiết đặt yêu cầu tắt UWF trong phiên hiện tại. Hãy tắt UWF và khởi động lại trước khi chạy.</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>Thao tác của phiên hiện tại (HORM, FullCommit và đặt lại lớp phủ bền vững; xuất từ Thay đổi đang chờ)</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>Gỡ bảo vệ hàng loạt yêu cầu xác nhận đầy đủ phạm vi ổ đĩa của phiên tiếp theo.</translation></message>
</context>
</TS>
