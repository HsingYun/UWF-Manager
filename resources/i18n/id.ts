<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="id" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Pratinjau</translation></message>
    <message><source>enable</source><translation>aktifkan</translation></message>
    <message><source>disable</source><translation>nonaktifkan</translation></message>
    <message><source>Enabled</source><translation>Aktif</translation></message>
    <message><source>Disabled</source><translation>Nonaktif</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Batal</translation></message>
    <message><source>Close</source><translation>Tutup</translation></message>
    <message><source>Refresh</source><translation>Segarkan</translation></message>
    <message><source>Add</source><translation>Tambah</translation></message>
    <message><source>Remove</source><translation>Hapus</translation></message>
    <message><source>Yes</source><translation>Ya</translation></message>
    <message><source>No</source><translation>Tidak</translation></message>
    <message><source>Excluded</source><translation>Dikecualikan</translation></message>
    <message><source>Not excluded</source><translation>Tidak dikecualikan</translation></message>
    <message><source>File…</source><translation>File…</translation></message>
    <message><source>Folder…</source><translation>Folder…</translation></message>
    <message><source>volume ID</source><translation>ID volume</translation></message>
    <message><source>drive letter</source><translation>huruf drive</translation></message>
    <message><source>Drive letter</source><translation>Huruf drive</translation></message>
    <message><source>Volume ID</source><translation>ID volume</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Status perlindungan volume pada sesi saat ini (hanya baca).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Lindungi volume pada sesi berikutnya. Penulisan dialihkan ke overlay dan dibuang saat mulai ulang.</translation></message>
    <message><source>Current session</source><translation>Sesi saat ini</translation></message>
    <message><source>Next session</source><translation>Sesi berikutnya</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Cara mengenali volume. Huruf drive lebih sederhana, tetapi ikatan terputus jika huruf berubah, misalnya setelah menambah atau melepas disk. ID volume tetap stabil.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Volume tidak didukung UWF: </translation></message>
    <message><source>Global settings</source><translation>Pengaturan global</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Status filter UWF pada sesi saat ini (hanya baca).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Aktifkan UWF pada sesi berikutnya. Penulisan ke volume terlindungi masuk ke overlay dan dibuang saat mulai ulang.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Untuk mengubah jenis dan ukuran maksimum:
1. Nonaktifkan filter di atas.
2. Mulai ulang agar filter nonaktif.
3. Ubah pengaturan.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Lokasi overlay: RAM lebih cepat tetapi memakai memori; Disk memakai drive sistem dengan kapasitas lebih besar. Keduanya dibuang saat mulai ulang.</translation></message>
    <message><source>Type</source><translation>Jenis</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Kapasitas overlay maksimum. RAM dibatasi total memori sistem. Disk memerlukan minimal 1024 MB dan ruang kosong yang cukup pada volume sistem.</translation></message>
    <message><source>Maximum size</source><translation>Ukuran maksimum</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Ukuran maksimum · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Ambang peringatan</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Mencatat pemberitahuan tingkat peringatan ke log peristiwa saat penggunaan overlay mencapai nilai ini. Jika kedua peristiwa diaktifkan, nilai ini harus lebih rendah daripada ambang kritis. Nilai 0 hanya menonaktifkan peristiwa peringatan.</translation></message>
    <message><source>Critical threshold</source><translation>Ambang kritis</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Mencatat pemberitahuan tingkat kritis ke log peristiwa saat penggunaan overlay mencapai nilai ini. Jika kedua peristiwa diaktifkan, nilai ini harus lebih tinggi daripada ambang peringatan. Nilai 0 hanya menonaktifkan peristiwa kritis.</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Jenis drive tidak didukung (hanya disk lokal tetap).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Volume melebihi batas UWF sebesar 16 TiB per volume terlindungi.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: volume dapat dilindungi, tetapi pengecualian dan penyimpanan per file tidak didukung.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Gagal: Membaca informasi volume.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 kosong / %2</translation></message>
    <message><source>Commit</source><translation>Simpan permanen</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Simpan perubahan overlay file ini secara permanen ke disk / Simpan perubahan registri secara permanen. Tindakan ini tidak dapat dibatalkan.</translation></message>
    <message><source>Commit file changes…</source><translation>Simpan perubahan file secara permanen…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Pilih file untuk disimpan permanen ke disk. Simpan perubahan overlay file ini secara permanen ke disk.</translation></message>
    <message><source>Commit folder changes…</source><translation>Simpan perubahan folder secara permanen…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Pilih folder untuk disimpan permanen ke disk. Simpan perubahan overlay folder ini secara permanen ke disk. Termasuk semua subfolder.</translation></message>
    <message><source>Delete and commit file…</source><translation>Hapus file dan simpan penghapusannya secara permanen ke disk…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Pilih file untuk dihapus permanen. Hapus file dan simpan penghapusannya secara permanen ke disk.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Hapus folder beserta isinya dan simpan penghapusannya secara permanen…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Pilih folder untuk dihapus permanen. Hapus folder beserta isinya dan simpan penghapusannya secara permanen. Termasuk semua subfolder.</translation></message>
    <message><source>Commit registry changes…</source><translation>Simpan perubahan registri secara permanen…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Masukkan kunci dan nama nilai opsional. Jika nama kosong, semua nilai pada kunci beserta seluruh subkuncinya disimpan permanen.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Simpan penghapusan registri secara permanen…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Masukkan kunci yang akan dihapus dan nama nilai opsional. Jika nama kosong, seluruh pohon—kunci, nilai, dan subkunci—dihapus secara permanen.</translation></message>
    <message><source>File exclusions</source><translation>Pengecualian file</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>File dan folder volume yang dikecualikan dari UWF. Klik dua kali entri untuk menyalin jalurnya.</translation></message>
    <message><source>Registry exclusions</source><translation>Pengecualian registri</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Pengecualian registri bersifat global dan mengikuti filter UWF, bukan perlindungan volume. Daftar bersama hanya ditampilkan sekali di tab disk. Klik dua kali untuk menyalin jalur.</translation></message>
    <message><source>File staging</source><translation>Penyimpanan file otomatis</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>file / folder: Disimpan permanen secara otomatis sebelum mematikan atau memulai ulang dengan aman. Termasuk semua subfolder. Selama daftar berisi entri, pengecualian registri berikut mempertahankan daftar antar sesi terlindungi: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Tambahkan file atau folder ke daftar penyimpanan otomatis.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Pilih file untuk disimpan permanen ke disk. Disimpan permanen secara otomatis sebelum mematikan atau memulai ulang dengan aman.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Pilih folder untuk disimpan permanen ke disk. Disimpan permanen secara otomatis sebelum mematikan atau memulai ulang dengan aman. Termasuk semua subfolder.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Pilih file untuk disimpan permanen ke disk · Penyimpanan file otomatis</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Pilih folder untuk disimpan permanen ke disk · Penyimpanan file otomatis · Termasuk semua subfolder.</translation></message>
    <message><source>Cannot stage this path</source><translation>Jalur tidak dapat ditambahkan ke penyimpanan otomatis</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Jalur bertumpang tindih dengan pengecualian UWF. Hapus pengecualian terlebih dahulu:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Jalur bertumpang tindih dengan entri penyimpanan otomatis. Hapus entri terlebih dahulu:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Jalur bertumpang tindih dengan entri penyimpanan otomatis</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Pengecualian registri ini diperlukan selama daftar penyimpanan otomatis berisi entri</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Jalur tidak ada atau bukan jenis yang diharapkan:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Jalur tidak dapat diperiksa:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Titik reparse tidak dapat ditambahkan ke penyimpanan otomatis:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Tanpa huruf drive, UWF tidak dapat menyimpan jalur ini:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Jalur berada di volume %1. Tambahkan lewat tab penyimpanan otomatis volume tersebut.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Akar volume tidak dapat ditambahkan karena akan menyimpan seluruh volume secara rekursif.</translation></message>
    <message><source>File staging could not be saved</source><translation>Gagal: Menyimpan daftar penyimpanan otomatis</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Daftar dimuat ulang karena pembaruan tidak dapat diselesaikan:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Penyimpanan file otomatis: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Disimpan permanen secara otomatis sebelum mematikan atau memulai ulang dengan aman.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Penyimpanan file otomatis: %1 · file: %2 · folder: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Penyimpanan file otomatis memerlukan pengecualian registri ini agar daftarnya tetap tersimpan setelah memulai ulang.</translation></message>
    <message><source>Built-in</source><translation>Bawaan</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Filter UWF nonaktif. Tidak ada perubahan overlay yang terkumpul untuk disimpan.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Sistem file %1 tidak mendukung penyimpanan per file.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Volume ini tidak didukung UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Volume tidak mendukung penyimpanan file. Hanya penyimpanan registri yang tersedia (pengecualiannya global).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Sistem file %1 tidak mendukung penyimpanan per file. Hanya penyimpanan registri yang tersedia (pengecualiannya global).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Volume tidak sedang dilindungi UWF; tidak ada perubahan file untuk disimpan.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Volume tidak sedang dilindungi UWF; tidak ada perubahan file untuk disimpan. Hanya penyimpanan registri yang tersedia (pengecualiannya global).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Pilih file untuk disimpan permanen ke disk</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Pilih folder untuk disimpan permanen ke disk</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Pilih file untuk dihapus permanen</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Pilih folder untuk dihapus permanen</translation></message>
    <message><source>Name</source><translation>Nama</translation></message>
    <message><source>Data</source><translation>Data</translation></message>
    <message><source>(Default)</source><translation>(Default)</translation></message>
    <message><source>(value not set)</source><translation>(nilai belum diatur)</translation></message>
    <message><source>(empty)</source><translation>(kosong)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Tidak diketahui (Nilai: File terlalu besar)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Lihat file overlay. &lt;b&gt;Hanya cuplikan diagnostik.&lt;/b&gt; Jangan gunakan daftar ini untuk menentukan file yang disimpan.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Daftar file overlay hanya didukung pada volume NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF nonaktif; tidak ada overlay untuk diperiksa.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Volume tidak dilindungi UWF sehingga tidak memiliki overlay untuk diperiksa.</translation></message>
    <message><source>Failed: %1</source><translation>Gagal: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Penyedia WMI berhenti saat enumerasi. Ini ketidakstabilan UWF_Overlay.GetOverlayFiles yang diketahui pada overlay besar atau beban I/O tinggi.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Memori tidak cukup atau operasi tidak didukung penyedia. Memerlukan NTFS dan cadangan memori. Coba overlay lebih kecil atau tutup aplikasi boros memori.</translation></message>
    <message><source>Raw error: %1</source><translation>Kode kesalahan: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Ekspor ke file… · Overlay · file</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>File teks (*.txt);;Semua file (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>File perintah (*.cmd);;Semua file (*)</translation></message>
    <message><source>Export failed</source><translation>Ekspor gagal</translation></message>
    <message><source>Export finished</source><translation>Ekspor selesai</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Gagal: Membuka file (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Gagal: Menulis file (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Konfigurasi: Menulis file · entri: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Seluruh volume (%1) tidak dapat dikecualikan.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>File paging, swap, dan hibernasi tidak dapat dikecualikan karena UWF bergantung padanya.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Seluruh direktori tidak dapat dikecualikan: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Seluruh direktori tidak dapat dikecualikan: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Seluruh direktori tidak dapat dikecualikan: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>File sistem penting ini tidak dapat dikecualikan: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>File registri pengguna NTUSER.DAT tidak dapat dikecualikan.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Kunci registri tidak boleh kosong.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Kunci registri tidak boleh diawali garis miring terbalik.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Jalur berisi garis miring terbalik berurutan dan tidak valid.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Jalur registri memakai `\` sebagai pemisah, bukan `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Jalur berisi karakter kontrol tersembunyi dan tidak valid.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>Dokumentasi UWF melarang pengecualian rahasia akun komputer domain: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Hanya subkunci tertentu di bawah kunci tingkat atas berikut yang dapat dikecualikan:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Tambahkan file atau folder ke pengecualian. Item yang dikecualikan tidak dilindungi UWF.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Pilih file untuk ditambahkan · Pengecualian file.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Pilih folder untuk ditambahkan · Pengecualian file. Termasuk semua subfolder.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Masukkan jalur kunci registri untuk dikecualikan.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Masukkan jalur kunci registri untuk dikecualikan. / Persistensi.</translation></message>
    <message><source>Registry key…</source><translation>Registri · Kunci…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Kunci rahasia domain (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Lisensi akses klien Terminal Services (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Simpan rahasia domain (kata sandi akun komputer) antar sesi UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Simpan lisensi akses klien Terminal Services antar sesi UWF.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistensi %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Hapus yang dipilih</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Hapus pengecualian terpilih. Berlaku setelah diterapkan.</translation></message>
    <message><source>Open containing folder</source><translation>Buka folder lokasi</translation></message>
    <message><source>Copy file path</source><translation>Salin jalur file</translation></message>
    <message><source>Copy registry path</source><translation>Salin jalur registri</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Simpan perubahan overlay folder ini secara permanen ke disk…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Simpan perubahan overlay file ini secara permanen ke disk…</translation></message>
    <message><source>Copied to clipboard</source><translation>Disalin ke papan klip</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Pilih file untuk ditambahkan · Pengecualian file. Beberapa file dapat dipilih.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Pilih folder untuk ditambahkan · Pengecualian file</translation></message>
    <message><source>Add registry exclusion</source><translation>Menambah pengecualian registri</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Jalur %1 bukan di volume %2 dan tidak dapat ditambahkan ke pengecualiannya.</translation></message>
    <message><source>Path is not on this volume</source><translation>Jalur bukan pada volume ini</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Pengecualian tidak dapat ditambahkan</translation></message>
    <message><source>File: %1</source><translation>File… %1</translation></message>
    <message><source>Folder: %1</source><translation>Folder… %1</translation></message>
    <message><source>Registry: %1</source><translation>Registri: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Dikecualikan secara default; dapat diubah kapan saja.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Sesi saat ini: %1
Sesi berikutnya: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Perubahan tertunda: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Pengelola Filter Tulis Terpadu (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Baca ulang status UWF saat ini dan konfigurasi sesi berikutnya.</translation></message>
    <message><source>Review and apply</source><translation>Tinjau dan terapkan</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Tinjau semua perubahan tertunda dan terapkan sekaligus. Sebagian besar perubahan berlaku setelah mulai ulang berikutnya.</translation></message>
    <message><source>Safe shutdown</source><translation>Matikan dengan aman</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Sistem akan dimatikan dengan aman melalui UWF. Tetap tersedia meski overlay UWF penuh.</translation></message>
    <message><source>Safe restart</source><translation>Mulai ulang dengan aman</translation></message>
    <message><source>Direct restart</source><translation>Langsung mulai ulang</translation></message>
    <message><source>Direct shutdown</source><translation>Langsung matikan</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Sistem akan dimulai ulang dengan aman melalui UWF. Tetap tersedia meski overlay UWF penuh.</translation></message>
    <message><source>Log</source><translation>Log</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Lihat log internal sesi ini untuk pemecahan masalah.</translation></message>
    <message><source>About</source><translation>Tentang</translation></message>
    <message><source>Switch display language</source><translation>Ganti bahasa tampilan</translation></message>
    <message><source>Language</source><translation>Bahasa</translation></message>
    <message><source>Theme</source><translation>Tema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Ganti tema terang/gelap</translation></message>
    <message><source>System information</source><translation>Informasi sistem</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Cuplikan diagnostik</translation></message>
    <message><source>Read only</source><translation>Hanya baca</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Target tidak ditemukan; tidak ada yang disimpan dari overlay.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Target hanya ada di overlay: dibuat dalam perlindungan UWF dan belum disimpan ke disk atau registri permanen. Hilang saat mulai ulang; penyimpanan penghapusan tidak diperlukan dan tidak mungkin.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Sistem menolak parameter (jalur atau argumen tidak valid).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Operasi gagal. Target mungkin digunakan proses lain, misalnya folder terbuka di Explorer atau file sedang dibuka. Tutup program yang menggunakannya lalu coba lagi.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Operasi gagal; lihat detail di log.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operasi ditolak (kode %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Tidak diketahui.</translation></message>
    <message><source>Commit canceled</source><translation>Penyimpanan dibatalkan</translation></message>

    <message><source>Succeeded</source><translation>Berhasil</translation></message>
    <message><source>Skipped</source><translation>Dilewati</translation></message>
    <message><source>Failed</source><translation>Gagal</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Berhasil: %1; Dilewati: %2; Gagal: %3.</translation></message>

    <message><source>Existed before</source><translation>Ada sebelum</translation></message>
    <message><source>Exists after</source><translation>Ada sesudah</translation></message>
    <message><source>Error code</source><translation>Kode kesalahan</translation></message>
    <message><source>Reason</source><translation>Alasan</translation></message>
    <message><source>Previous page</source><translation>Halaman sebelumnya</translation></message>
    <message><source>First page</source><translation>Halaman pertama</translation></message>
    <message><source>Last page</source><translation>Halaman terakhir</translation></message>
    <message><source>Next page</source><translation>Halaman berikutnya</translation></message>
    <message><source>Copy selected rows</source><translation>Salin baris terpilih</translation></message>
    <message><source>Copy all</source><translation>Salin semua</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Pengecualian registri ini diperlukan selama daftar penyimpanan otomatis berisi entri. Penyimpanan file otomatis: Bersihkan → Menghapus pengecualian registri.</translation></message>
    <message><source>Copy current page</source><translation>Salin halaman saat ini</translation></message>
    <message><source>No pending changes</source><translation>Tidak ada perubahan tertunda</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Pengecualian registri · Pengaturan global.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: Perlindungan UWF / Pengecualian file / Penyimpanan file otomatis.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Tidak didukung</translation></message>
    <message><source>Review and apply changes</source><translation>Tinjau dan terapkan perubahan</translation></message>
    <message><source>Confirm apply</source><translation>Konfirmasi penerapan</translation></message>
    <message><source>Result</source><translation>Hasil</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filter: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Mengatur ambang peringatan: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Mengatur ambang kritis: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Mengatur jenis overlay / Mengatur ukuran maksimum: Untuk mengubah jenis dan ukuran maksimum:
1. Nonaktifkan filter di atas.
2. Mulai ulang agar filter nonaktif.
3. Ubah pengaturan.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Mengatur jenis overlay: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Mengatur ukuran maksimum: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Mengatur ukuran maksimum: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · Perlindungan UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Identifikasi melalui: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1 · Menambah pengecualian file: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1 · Menghapus pengecualian file: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Menambah pengecualian registri: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Menghapus pengecualian registri: %1</translation></message>
    <message><source>Time</source><translation>Waktu</translation></message>
    <message><source>Level</source><translation>Tingkat</translation></message>
    <message><source>Tag</source><translation>Tag</translation></message>
    <message><source>Message</source><translation>Pesan</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Menghitung file untuk penyimpanan otomatis…</translation></message>
    <message><source>Committing staged files</source><translation>Menyimpan file yang disiapkan</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 dari %2 file diproses.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Tidak ada detail kesalahan tambahan.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Penyimpanan otomatis berhenti tiba-tiba:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Penyimpanan otomatis tidak selesai: Kesalahan tidak diketahui.</translation></message>
    <message><source>File staging will be skipped</source><translation>Penyimpanan otomatis akan dilewati</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Langsung mulai ulang: Penyimpanan otomatis akan dilewati.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Langsung matikan: Penyimpanan otomatis akan dilewati.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Penyimpanan otomatis dilewati karena tindakan daya langsung dipilih.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Matikan dengan aman: Gagal</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Matikan: Gagal (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Mulai ulang dengan aman: Gagal</translation></message>
    <message><source>Direct restart failed</source><translation>Langsung mulai ulang: Gagal</translation></message>
    <message><source>Direct shutdown failed</source><translation>Langsung matikan: Gagal</translation></message>
    <message><source>Restart failed: %1</source><translation>Mulai ulang: Gagal (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Daftar penyimpanan otomatis tidak dapat dibaca:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Daftar penyimpanan otomatis tidak dapat dibaca: Kesalahan tidak diketahui.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>file: %1 · Berhasil: %2 · Dilewati (file): %3 · Dilewati (entri): %4 · Gagal: %5</translation></message>
    <message><source>File staging preparation</source><translation>Penyimpanan file otomatis: Detail operasi</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Titik reparse tidak didukung.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Hanya jalur absolut pada volume lokal yang dapat ditambahkan.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Hanya jalur absolut pada volume lokal yang dapat ditambahkan.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Jalur penyimpanan otomatis tidak dapat diperiksa.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Jalur file yang disiapkan kini menunjuk ke jenis berbeda.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Jalur folder yang disiapkan kini menunjuk ke jenis berbeda.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Folder yang disiapkan tidak dapat didaftar seluruhnya.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Jalur tidak memiliki huruf drive.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>Akar volume tidak dapat diproses secara rekursif.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Penyedia UWF menolak penyimpanan permanen.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Penyimpanan gagal: Kesalahan tidak diketahui.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>%1 kegagalan tambahan dicatat di log.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Jalur tidak memiliki huruf drive. Menentukan volume target: Gagal.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Tidak ada catatan sesi saat ini untuk volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Jalur ada dalam pengecualian file.
Pengecualian: %1</translation></message>

    <message><source>Continue</source><translation>Lanjutkan</translation></message>
    <message><source>Delete and commit</source><translation>Hapus dan simpan permanen</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Kunci ada dalam pengecualian registri.
Pengecualian: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Tidak ada catatan filter registri untuk sesi saat ini.</translation></message>

    <message><source>Export commands to file</source><translation>Ekspor perintah ke file</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Ekspor ke file… · Perintah: %1
%2</translation></message>
    <message><source>Import</source><translation>Impor</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Ketik atau tempel satu perintah per baris. Muat teks, log, atau skrip (.txt, .bat, .ps1); baris yang berisi perintah ini ditambahkan. Impor menyiapkan perubahan. Hanya “Tinjau dan terapkan” di bilah alat yang menuliskannya ke sistem.</translation></message>
    <message><source>Load from file…</source><translation>Muat dari file…</translation></message>
    <message><source>Load recommended configuration</source><translation>Muat konfigurasi yang disarankan</translation></message>
    <message><source>Choose recommended configuration</source><translation>Pilih konfigurasi yang disarankan</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Pilih kelompok konfigurasi yang akan ditambahkan. Tinjau atau hapus tiap perintah sebelum mengimpor.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Dari dokumentasi resmi Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Pengecualian UWF rekomendasi Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Tinjau sebelum impor: folder harus ada agar UWF menerima pengecualian.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Sumber: dokumentasi resmi Microsoft UWF tentang pengecualian umum dan dukungan antimalware.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Program Peningkatan Pengalaman Pelanggan (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Simpan permanen: Status persetujuan kebijakan</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Simpan permanen: Status persetujuan lokal</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Simpan permanen: Penanda penonaktifan unggahan</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Layanan Transfer Cerdas Latar Belakang (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Simpan permanen: File antrean unduhan</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Simpan permanen: Indeks status transfer</translation></message>
    <message><source>Network profiles and policies</source><translation>Profil dan kebijakan jaringan</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Kebijakan GPO jaringan nirkabel</translation></message>
    <message><source>Wired network GPO policy</source><translation>Kebijakan GPO jaringan kabel</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Kebijakan GPO jaringan nirkabel · file</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Kebijakan GPO jaringan kabel · file</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Profil antarmuka nirkabel</translation></message>
    <message><source>Wired network interface profiles</source><translation>Profil antarmuka kabel</translation></message>
    <message><source>Wireless service configuration</source><translation>Konfigurasi layanan nirkabel</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Konfigurasi layanan broadband seluler</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Konfigurasi layanan konfigurasi otomatis kabel</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>Profil XML jaringan khusus tiap perangkat. Tambahkan jalur Interfaces\{GUID}\{GUID}.xml yang tepat secara manual jika perlu.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Waktu musim panas (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Simpan permanen: Definisi zona waktu</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Simpan permanen: Informasi zona waktu terpilih</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Simpan permanen: File produk dan pembaruan</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Simpan permanen: Tanda tangan dan status ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Simpan permanen: Windows Update · Log</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Simpan permanen: MpCmdRun · Log</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Simpan permanen: Status produk di registri</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Simpan permanen: WdBoot · Status layanan</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Simpan permanen: WdFilter · Status layanan</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Simpan permanen: WdNisSvc · Status layanan</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Simpan permanen: WdNisDrv · Status layanan</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Simpan permanen: WinDefend · Status layanan</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Simpan permanen: File program klien</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Simpan permanen: Windows Update · Log</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Simpan permanen: MpCmdRun · Log</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Simpan permanen: Tanda tangan dan status antimalware</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Simpan permanen: Status antimalware di registri</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Pilih file yang berisi perintah uwfmgr</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>File teks (*.txt *.cmd);;Semua file (*)</translation></message>
    <message><source>Status</source><translation>Status</translation></message>
    <message><source>Import failed</source><translation>Impor gagal</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Kesalahan internal: tidak ada penangan penerapan terdaftar.</translation></message>
    <message><source>Nothing to import</source><translation>Tidak ada perintah untuk diimpor</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Tidak ada perintah uwfmgr dalam masukan.</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplikat: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Tidak didukung: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Kelompok %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Duplikat</translation></message>
    <message><source>Unsupported</source><translation>Tidak didukung</translation></message>
    <message><source>File too large</source><translation>File terlalu besar</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>File %1 lebih besar dari 5 MB dan tidak diurai. Saring secara manual terlebih dahulu.</translation></message>
    <message><source>Cannot read file</source><translation>File tidak dapat dibaca</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Gagal: Membuka file (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Buang perubahan tertunda?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Ada %1 perubahan yang belum diterapkan.

Buang dan lanjutkan?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Perintah uwfmgr tidak lengkap</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Argumen tidak ada: Ukuran (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>Ukuran harus bilangan bulat nonnegatif dalam MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Argumen tidak ada: Overlay · Jenis (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Nilai tidak valid: Overlay · Jenis: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Argumen tidak ada: Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Volume harus berupa huruf drive, misalnya C:</translation></message>
    <message><source>Missing path argument</source><translation>Argumen tidak ada: Jalur</translation></message>
    <message><source>Missing registry key argument</source><translation>Argumen tidak ada: Registri · Kunci</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Tanda kutip ganda belum ditutup dalam perintah</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Argumen tambahan tidak diharapkan: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Perintah uwfmgr tidak didukung (tidak ada tindakan antarmuka yang sesuai)</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Sudah dalam status target; tidak perlu tindakan</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Jalur bukan pada volume ini atau volume tidak mendukung pengecualian (mis. exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Ditolak daftar blokir UWF (file sistem / Windows / paging / dll.)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Nilai tidak valid: Ukuran: %1</translation></message>
    <message><source>maximum size</source><translation>Ukuran maksimum</translation></message>
    <message><source>warning threshold</source><translation>Ambang peringatan</translation></message>
    <message><source>critical threshold</source><translation>Ambang kritis</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>Konfigurasi global tidak tersedia; perintah tidak ditambahkan ke perubahan tertunda.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Volume %1 tidak dikenal (tidak ada disk yang memenuhi syarat UWF dengan huruf ini)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Sudah dalam status target; tidak perlu tindakan</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Jalur tidak memiliki huruf drive. Menentukan volume target: Gagal.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Tidak didukung</translation></message>
    <message><source>file exclusion</source><translation>Pengecualian file</translation></message>
    <message><source>registry exclusion</source><translation>Pengecualian registri</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Tab registri tidak ada; pengecualian registri tidak tersedia</translation></message>
    <message><source>Unsupported command</source><translation>Perintah: Tidak didukung</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Tampilkan/sembunyikan.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Aktif</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Nonaktif</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Tidak diketahui</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Terpakai %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Mode lanjutan</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Koordinasikan penyimpanan otomatis dengan mematikan dan memulai ulang Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Gagal: Membaca status mode lanjutan (Kesalahan tidak diketahui).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Gagal: Membaca status layanan:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Gagal: Membaca status layanan (Kesalahan tidak diketahui).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Layanan tidak dikonfigurasi sebagai proses mandiri.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Layanan tidak dikonfigurasi untuk mulai otomatis.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Layanan tidak dikonfigurasi untuk berjalan sebagai LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>Layanan tidak berjalan.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Jalur atau argumen startup layanan tidak sesuai dengan program ini.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Batas waktu prapematian layanan tidak diatur dengan benar.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Layanan tidak mendeklarasikan hak untuk menjalankan agen antarmuka.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Layanan belum menerima pemberitahuan prapematian Windows.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Aktifkan mode lanjutan</translation></message>
    <message><source>Disable enhanced mode</source><translation>Nonaktifkan mode lanjutan</translation></message>
    <message><source>Start service</source><translation>Jalankan layanan</translation></message>
    <message><source>Repair enhanced mode</source><translation>Perbaiki mode lanjutan</translation></message>
    <message><source>Delete service</source><translation>Hapus layanan</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Hentikan dan hapus layanan mode lanjutan beserta semua data registri sisanya.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Layanan tidak ada, tetapi data registrinya masih tersisa.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Menunggu autentikasi agen antarmuka.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Layanan berjalan, tetapi tidak ada agen antarmuka terautentikasi yang terhubung.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Layanan pembantu mode lanjutan UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Mode lanjutan · Konfigurasi: Diterapkan.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Konfigurasi: Diterapkan. Laporan persistensi UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Mode lanjutan · Konfigurasi: Gagal:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Mode lanjutan · Konfigurasi: Gagal (Kesalahan tidak diketahui).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Layanan mode lanjutan dan konfigurasi sisanya dihapus.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Penghapusan layanan diminta, tetapi pembersihan sisa melaporkan:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Mode lanjutan: Hapus layanan: Gagal:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Mode lanjutan: Hapus layanan: Gagal (Kesalahan tidak diketahui).</translation></message>
    <message><source>File staging completed</source><translation>Penyimpanan file otomatis: Berhasil</translation></message>
    <message><source>File staging failed</source><translation>Penyimpanan file otomatis: Gagal</translation></message>
    <message><source>All staged targets were processed.</source><translation>Semua target yang disiapkan telah diproses.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Semua target diproses; satu atau beberapa operasi gagal.</translation></message>
    <message><source>Show main window</source><translation>Tampilkan jendela utama</translation></message>
    <message><source>Restore default position</source><translation>Pulihkan posisi default</translation></message>
    <message><source>Show overlay hub</source><translation>Tampilkan HUB overlay</translation></message>
    <message><source>Hide overlay hub</source><translation>Sembunyikan HUB overlay</translation></message>
    <message><source>Exit application</source><translation>Keluar dari aplikasi</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Gagal: Menghubungkan ke sistem</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Gagal: Membaca nilai registri (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Mencantumkan file overlay: Kesalahan tidak diketahui.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Gagal: Tindakan filter: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Gagal: Membaca status filter</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Gagal: Mengatur ambang peringatan</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Gagal: Mengatur ambang kritis</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Gagal: Membaca status overlay</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Gagal: Mengatur jenis overlay</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Gagal: Mengatur ukuran maksimum</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Gagal: Membaca konfigurasi overlay</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Gagal: Mendaftarkan volume %1 ke UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Gagal: Perlindungan volume %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Gagal: Mengatur ikatan volume %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Gagal: Menambah pengecualian file %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Pengecualian file pada volume %1 sudah tidak ada: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Gagal: Menghapus pengecualian file %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Gagal: Membaca konfigurasi volume</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Gagal: Menambah pengecualian registri %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Gagal: Menghapus pengecualian registri %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Gagal: Memperbarui opsi persistensi registri</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Gagal: Membaca filter registri</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Tidak didukung</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Membaca status target: Kesalahan tidak diketahui.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Jalur tidak ada; tidak ada yang dihapus.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Kesalahan tidak diketahui.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Hasil tidak dapat dikonfirmasi karena pembacaan ulang status target gagal: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Penyedia menerima penghapusan, tetapi pemeriksaan ulang dari sumber otoritatif menunjukkan target masih ada.</translation></message>
    <message><source>Unknown</source><translation>Tidak diketahui</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Gagal: Memuat log (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Gagal: Memuat log (Kesalahan tidak diketahui).</translation></message>
    <message><source>: unknown error</source><translation>: Kesalahan tidak diketahui</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Gagal: Menentukan volume target (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Gagal: Mencantumkan kunci registri (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Gagal: Menentukan volume target (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Operasi dilewati karena operasi prasyarat gagal atau hasilnya tidak dapat dikonfirmasi.</translation></message>
    <message><source>Copy</source><translation>Salin</translation></message>
    <message><source>Select All</source><translation>Pilih Semua</translation></message>
    <message><source>Display language</source><translation>Bahasa tampilan</translation></message>
    <message><source>Folder</source><translation>Folder</translation></message>
    <message><source>File</source><translation>File</translation></message>
    <message><source>Export commands</source><translation>Ekspor perintah</translation></message>
    <message><source>Direct restart…</source><translation>Langsung mulai ulang…</translation></message>
    <message><source>Clear…</source><translation>Bersihkan…</translation></message>
    <message><source>Committing</source><translation>Menyimpan</translation></message>
    <message><source> · %1 selected</source><translation> · %1 dipilih</translation></message>
    <message><source>%1 changes</source><translation>%1 perubahan</translation></message>
    <message><source>%1 commands</source><translation>%1 perintah</translation></message>
    <message><source>%1 items</source><translation>%1 item</translation></message>
    <message><source>%1 items · %2</source><translation>%1 item · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 perubahan tertunda</translation></message>
    <message><source>%1 records</source><translation>%1 catatan</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 catatan · total %2</translation></message>


    <message><source>%1 selected</source><translation>%1 dipilih</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 file</translation></message>
    <message><source>32-bit</source><translation>32 bit</translation></message>
    <message><source>64-bit</source><translation>64 bit</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Dirilis berdasarkan &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Ketentuan lengkap ada dalam file LICENSE yang disertakan bersama aplikasi.&lt;/p&gt;&lt;p&gt;Anda boleh mendistribusikan ulang atau mengubah perangkat lunak berdasarkan GPL v3. Perangkat lunak disediakan apa adanya, tanpa jaminan apa pun.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows menonaktifkan file halaman, Pemulihan Sistem, SuperFetch, pengindeksan file, defragmentasi disk, dan mulai cepat. Windows juga mengatur bootstatuspolicy ke ignoreallfailures dalam BCD.&lt;/p&gt;&lt;p&gt;Pengaturan ini dapat diubah setelahnya. Misalnya, pindahkan file halaman ke volume yang tidak dilindungi sebelum mengaktifkan kembali paging.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Snapshot diagnostik sistem, hak istimewa, dan UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>File sudah ada di lokasi ini:
%1

Menyimpan akan mengganti isinya.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Pengelola grafis untuk Windows Unified Write Filter.
Kelola perlindungan volume, overlay, pengecualian, dan file yang disiapkan.</translation></message>
    <message><source>About this snapshot</source><translation>Tentang snapshot ini</translation></message>
    <message><source>Account</source><translation>Akun</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Tambahkan konfigurasi UWF dari skrip atau teks.</translation></message>
    <message><source>Add exclusion</source><translation>Tambah pengecualian</translation></message>
    <message><source>Add files or folders</source><translation>Tambah file atau folder</translation></message>
    <message><source>Add selected items</source><translation>Tambah item terpilih</translation></message>
    <message><source>Added</source><translation>Ditambahkan</translation></message>
    <message><source>Administrator</source><translation>Administrator</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Semua perubahan telah disimpan ke disk dan akan tetap ada tanpa memulai ulang.</translation></message>
    <message><source>All files (*)</source><translation>Semua file (*)</translation></message>
    <message><source>All levels</source><translation>Semua tingkat</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Snapshot diagnostik yang kosong bukan berarti overlay tidak berisi perubahan.</translation></message>
    <message><source>Application path</source><translation>Jalur aplikasi</translation></message>
    <message><source>Application result</source><translation>Hasil penerapan</translation></message>
    <message><source>Apply changes</source><translation>Terapkan perubahan</translation></message>
    <message><source>Architecture</source><translation>Arsitektur</translation></message>
    <message><source>Automatic</source><translation>Otomatis</translation></message>
    <message><source>Automatic file staging</source><translation>Penyimpanan otomatis file yang disiapkan</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Penyimpanan otomatis belum selesai</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Simpan otomatis file yang disiapkan dan memenuhi syarat saat mematikan atau memulai ulang secara normal.</translation></message>
    <message><source>Available</source><translation>Tersedia</translation></message>
    <message><source>Back</source><translation>Kembali</translation></message>
    <message><source>Background service</source><translation>Layanan latar belakang</translation></message>
    <message><source>Binding</source><translation>Pengikatan</translation></message>
    <message><source>By drive letter</source><translation>Berdasarkan huruf drive</translation></message>
    <message><source>By volume ID</source><translation>Berdasarkan ID volume</translation></message>
    <message><source>Cancel reading</source><translation>Batalkan pembacaan</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Perubahan yang sudah dikomit tetap disimpan.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Pilih kunci registri yang akan dikecualikan dari perlindungan UWF, termasuk subkunci dan nilainya.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Pilih lokasi untuk menyimpan file.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Pilih lokasi dalam drive atau folder yang diizinkan.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Pilih folder lain atau periksa izin akses, lalu coba lagi.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Pilih kunci lain atau periksa izin akses, lalu coba lagi.</translation></message>
    <message><source>Clear filters</source><translation>Hapus filter</translation></message>
    <message><source>Clear logs</source><translation>Hapus log</translation></message>
    <message><source>Clear logs?</source><translation>Hapus log?</translation></message>
    <message><source>Collapse</source><translation>Ciutkan</translation></message>
    <message><source>Commit changes</source><translation>Komit perubahan</translation></message>
    <message><source>Commit changes…</source><translation>Komit perubahan…</translation></message>
    <message><source>Commit complete</source><translation>Komit selesai</translation></message>
    <message><source>Commit file changes?</source><translation>Komit perubahan file?</translation></message>
    <message><source>Commit folder changes?</source><translation>Komit perubahan folder?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Komit perubahan kunci registri?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Komit perubahan nilai registri?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Mengomit file yang disiapkan sebelum mematikan atau memulai ulang secara normal</translation></message>
    <message><source>Compatibility mode</source><translation>Mode kompatibilitas</translation></message>
    <message><source>Configuration applied</source><translation>Konfigurasi diterapkan</translation></message>
    <message><source>Configuration error</source><translation>Kesalahan konfigurasi</translation></message>
    <message><source>Configured</source><translation>Dikonfigurasi</translation></message>
    <message><source>Confirm before continuing</source><translation>Konfirmasi sebelum melanjutkan</translation></message>
    <message><source>Connected and authenticated</source><translation>Terhubung dan terautentikasi</translation></message>
    <message><source>Connecting…</source><translation>Menghubungkan…</translation></message>
    <message><source>Contact author</source><translation>Hubungi pengembang</translation></message>
    <message><source>Continue editing</source><translation>Lanjutkan mengedit</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Melanjutkan mulai ulang akan membuang perubahan pada file yang tidak dapat disimpan.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Melanjutkan pematian akan membuang perubahan pada file yang tidak dapat disimpan.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Koordinasikan dengan pematian Windows</translation></message>
    <message><source>Copy all logs</source><translation>Salin semua log</translation></message>
    <message><source>Copy full report</source><translation>Salin laporan lengkap</translation></message>
    <message><source>Copy name</source><translation>Salin nama</translation></message>
    <message><source>Copy path</source><translation>Salin jalur</translation></message>
    <message><source>Copy selection</source><translation>Salin pilihan</translation></message>
    <message><source>Copy version</source><translation>Salin versi</translation></message>
    <message><source>Counting staged files…</source><translation>Menghitung file yang disiapkan…</translation></message>
    <message><source>Create</source><translation>Buat</translation></message>
    <message><source>Critical %1 MB</source><translation>Kritis %1 MB</translation></message>
    <message><source>Current filter</source><translation>Filter saat ini</translation></message>
    <message><source>Current run</source><translation>Proses saat ini</translation></message>
    <message><source>Current selection</source><translation>Pilihan saat ini</translation></message>
    <message><source>Current usage</source><translation>Penggunaan saat ini</translation></message>
    <message><source>Date modified</source><translation>Tanggal diubah</translation></message>
    <message><source>Debug</source><translation>Debug</translation></message>
    <message><source>Delete and commit this file?</source><translation>Hapus file ini dan komit penghapusannya?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Hapus folder ini dan komit penghapusannya?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Hapus kunci registri ini dan komit penghapusannya?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Hapus nilai registri ini dan komit penghapusannya?</translation></message>

    <message><source>Direct restart?</source><translation>Mulai ulang langsung?</translation></message>
    <message><source>Direct shutdown?</source><translation>Matikan langsung?</translation></message>
    <message><source>Discard changes</source><translation>Buang perubahan</translation></message>
    <message><source>Disk</source><translation>Disk</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Persistensi kunci rahasia domain</translation></message>
    <message><source>Done</source><translation>Selesai</translation></message>
    <message><source>Edit commands</source><translation>Edit perintah</translation></message>
    <message><source>Editable</source><translation>Dapat diedit</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Aktifkan filter dan mulai ulang untuk menampilkan penggunaan langsung</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>Mode lanjutan menambahkan layanan latar belakang yang dimulai bersama Windows. Hapus layanan ini sebelum memindahkan atau menghapus aplikasi.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>Mode lanjutan tidak tersedia. Gunakan Matikan dengan aman atau Mulai ulang dengan aman untuk menyimpan file yang disiapkan.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Masukkan HKLM\SOFTWARE\… lalu tekan Enter untuk menavigasi</translation></message>
    <message><source>Enter a file name</source><translation>Masukkan nama file</translation></message>
    <message><source>Enter a valid file name.</source><translation>Masukkan nama file yang valid.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Masukkan nama file atau folder yang valid.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Masukkan nama folder yang valid.</translation></message>
    <message><source>Error</source><translation>Kesalahan</translation></message>
    <message><source>Error details</source><translation>Detail kesalahan</translation></message>
    <message><source>Error details and suggestions</source><translation>Detail kesalahan dan saran</translation></message>
    <message><source>Expand</source><translation>Perluas</translation></message>
    <message><source>Export list</source><translation>Ekspor daftar</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Gagal menghapus log: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Gagal menghapus log: kesalahan tak terduga.</translation></message>
    <message><source>File name</source><translation>Nama file</translation></message>
    <message><source>File or folder</source><translation>File atau folder</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>Penyimpanan file telah dimulai dan tidak dapat dibatalkan.</translation></message>
    <message><source>File saving incomplete</source><translation>Penyimpanan file belum selesai</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Penyimpanan file yang disiapkan akan dilewati. Perubahan yang belum dikomit pada jalur terlindungi akan hilang setelah mulai ulang.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Penyimpanan file yang disiapkan akan dilewati. Perubahan yang belum dikomit pada jalur terlindungi akan hilang setelah pematian.</translation></message>
    <message><source>File type</source><translation>Jenis file</translation></message>
    <message><source>Files and folders</source><translation>File dan folder</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>File dan folder dapat dipilih bersamaan.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>File yang disiapkan tidak akan disimpan.</translation></message>
    <message><source>Filter is disabled</source><translation>Filter dinonaktifkan</translation></message>
    <message><source>Fit columns to contents</source><translation>Sesuaikan kolom dengan isi</translation></message>
    <message><source>Folder address</source><translation>Alamat folder</translation></message>
    <message><source>Folder name</source><translation>Nama folder</translation></message>
    <message><source>Folders</source><translation>Folder-folder</translation></message>
    <message><source>Full configuration</source><translation>Konfigurasi lengkap</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Penulisan berikutnya ke kunci ini tidak akan dilindungi UWF.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Penulisan berikutnya ke jalur ini tidak akan dilindungi UWF.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Pengecualian registri global · termasuk subkunci</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Naik satu folder atau pilih lokasi lain.</translation></message>
    <message><source>Home</source><translation>Folder pribadi</translation></message>
    <message><source>Import configuration</source><translation>Impor konfigurasi</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>Impor memperbarui draf konfigurasi. Gunakan Tinjau dan terapkan untuk menulis perubahan ke sistem.</translation></message>
    <message><source>Import results</source><translation>Hasil impor</translation></message>
    <message><source>Imported</source><translation>Diimpor</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>Laporan mencantumkan hasil perintah untuk draf konfigurasi. Pilih satu baris untuk melihat perintah dan hasil selengkapnya.</translation></message>
    <message><source>Imported: %1</source><translation>Diimpor: %1</translation></message>
    <message><source>Information</source><translation>Informasi</translation></message>
    <message><source>Item details</source><translation>Detail item</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Daftar file yang disiapkan dalam jumlah besar dapat memerlukan waktu penyimpanan lebih lama.</translation></message>
    <message><source>License and copyright</source><translation>Lisensi dan hak cipta</translation></message>
    <message><source>Line</source><translation>Baris</translation></message>
    <message><source>Local disk (%1)</source><translation>Disk lokal (%1)</translation></message>
    <message><source>Log level</source><translation>Tingkat log</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Gagal membaca log. Coba lagi atau periksa akses ke file log.</translation></message>
    <message><source>Maximize</source><translation>Maksimalkan</translation></message>
    <message><source>Message details</source><translation>Detail pesan</translation></message>
    <message><source>Minimize</source><translation>Minimalkan</translation></message>
    <message><source>Module</source><translation>Modul</translation></message>
    <message><source>More</source><translation>Lainnya</translation></message>
    <message><source>Move</source><translation>Pindahkan</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Jika drive tempat aplikasi berada dilindungi UWF, pastikan terlebih dahulu bahwa file aplikasi tetap ada setelah komputer dimulai ulang. Menyimpan banyak file yang disiapkan dapat memperlama proses mematikan atau memulai ulang komputer.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Beberapa file dapat dipilih.</translation></message>
    <message><source>Name and location</source><translation>Nama dan lokasi</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Nilai bernama: %1 · hanya nilai ini</translation></message>
    <message><source>Navigate to registry path</source><translation>Buka jalur registri</translation></message>
    <message><source>New folder</source><translation>Folder baru</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Catatan baru akan muncul setelah disegarkan.</translation></message>
    <message><source>Next-session configuration</source><translation>Konfigurasi sesi berikutnya</translation></message>
    <message><source>No files to commit</source><translation>Tidak ada file untuk dikomit</translation></message>
    <message><source>No logs yet</source><translation>Belum ada log</translation></message>
    <message><source>No matching items</source><translation>Tidak ada item yang cocok</translation></message>
    <message><source>No matching logs</source><translation>Tidak ada log yang cocok</translation></message>
    <message><source>No overlay files found</source><translation>Tidak ditemukan file overlay</translation></message>
    <message><source>No registry key selected</source><translation>Tidak ada kunci registri yang dipilih</translation></message>
    <message><source>None</source><translation>Tidak ada</translation></message>
    <message><source>Not configured</source><translation>Belum dikonfigurasi</translation></message>
    <message><source>Not connected</source><translation>Tidak terhubung</translation></message>
    <message><source>Not installed</source><translation>Belum dipasang</translation></message>
    <message><source>Not written to the system</source><translation>Belum ditulis ke sistem</translation></message>
    <message><source>Off</source><translation>Nonaktif</translation></message>
    <message><source>On</source><translation>Aktif</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Satu perintah uwfmgr per baris</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Simpan aplikasi di folder tetap yang tidak dapat diubah oleh pengguna standar. Lokasi yang disarankan:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Hanya file yang disiapkan yang disimpan otomatis. Perubahan lain yang belum dikomit pada jalur terlindungi akan hilang setelah mulai ulang.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Hanya file yang disiapkan yang disimpan otomatis. Perubahan lain yang belum dikomit pada jalur terlindungi akan hilang setelah pematian.</translation></message>
    <message><source>Open</source><translation>Buka</translation></message>
    <message><source>Open folder</source><translation>Buka folder</translation></message>
    <message><source>Operating system</source><translation>Sistem operasi</translation></message>
    <message><source>Operation and security</source><translation>Operasi dan keamanan</translation></message>
    <message><source>Original command</source><translation>Perintah asli</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Ambang kritis overlay tercapai. Simpan perubahan dan mulai ulang dengan aman.</translation></message>
    <message><source>Overlay maximum size</source><translation>Ukuran maksimum overlay</translation></message>
    <message><source>Overlay type</source><translation>Jenis overlay</translation></message>
    <message><source>Overlay usage</source><translation>Penggunaan overlay</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Ambang peringatan overlay tercapai. Simpan perubahan dan mulai ulang dengan aman.</translation></message>
    <message><source>Overview</source><translation>Ringkasan</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Tempel perintah atau muat dari file…</translation></message>
    <message><source>Pending changes</source><translation>Perubahan tertunda</translation></message>
    <message><source>Pending removal</source><translation>Menunggu penghapusan</translation></message>
    <message><source>Permissions and protection</source><translation>Izin dan perlindungan</translation></message>
    <message><source>Privileges</source><translation>Hak istimewa</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2 item diproses</translation></message>
    <message><source>Processing result</source><translation>Hasil pemrosesan</translation></message>
    <message><source>Protected</source><translation>Dilindungi</translation></message>
    <message><source>Protected volumes</source><translation>Volume terlindungi</translation></message>
    <message><source>Protection and overlay</source><translation>Perlindungan dan overlay</translation></message>
    <message><source>Quick access</source><translation>Akses cepat</translation></message>
    <message><source>Raw report</source><translation>Laporan mentah</translation></message>
    <message><source>Read again</source><translation>Baca lagi</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Baca lagi untuk mengambil snapshot diagnostik.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Baca konfigurasi untuk menampilkan penggunaan langsung</translation></message>
    <message><source>Read-only</source><translation>Hanya baca</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Membaca overlay besar dapat memerlukan waktu lebih lama. Anda dapat membatalkan kapan saja.</translation></message>
    <message><source>Reading canceled</source><translation>Pembacaan dibatalkan</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Membaca file dan subfolder di lokasi ini…</translation></message>
    <message><source>Reading folder</source><translation>Membaca folder</translation></message>
    <message><source>Reading logs</source><translation>Membaca log</translation></message>
    <message><source>Reading overlay</source><translation>Membaca overlay</translation></message>
    <message><source>Ready</source><translation>Siap</translation></message>
    <message><source>Ready to save</source><translation>Siap disimpan</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Catatan proses saat ini akan muncul di sini setelah pembacaan selesai.</translation></message>
    <message><source>Recursive staging</source><translation>Penyiapan rekursif</translation></message>
    <message><source>Refresh folder</source><translation>Segarkan folder</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Kunci registri atau pengaturan persistensi</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Tugas tersisa dapat dibatalkan setelah target saat ini selesai.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Hapus kunci ini dari pengecualian sesi berikutnya.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Hapus jalur ini dari pengecualian sesi berikutnya.</translation></message>
    <message><source>Removed</source><translation>Dihapus</translation></message>
    <message><source>Rename</source><translation>Ubah nama</translation></message>
    <message><source>Rename…</source><translation>Ubah nama…</translation></message>
    <message><source>Repair required</source><translation>Perlu perbaikan</translation></message>
    <message><source>Replace file</source><translation>Ganti file</translation></message>
    <message><source>Replace the existing file?</source><translation>Ganti file yang ada?</translation></message>
    <message><source>Required configuration</source><translation>Konfigurasi wajib</translation></message>
    <message><source>Restart anyway</source><translation>Tetap mulai ulang</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Mulai ulang akan dilanjutkan setelah penyimpanan. Biarkan aplikasi tetap terbuka.</translation></message>
    <message><source>Restore</source><translation>Pulihkan</translation></message>
    <message><source>Retry</source><translation>Coba lagi</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Tinjau detail kegagalan sebelum melanjutkan pematian.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Tinjau detail kegagalan sebelum memulai ulang.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Tinjau detail operasi. Baca status sistem saat ini sebelum mencoba kembali perubahan yang mungkin sudah ditulis.</translation></message>
    <message><source>Review the result for each target.</source><translation>Tinjau hasil untuk setiap target.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Tinjau perubahan ini sebelum menuliskannya ke sistem.</translation></message>
    <message><source>Running</source><translation>Berjalan</translation></message>
    <message><source>Save</source><translation>Simpan</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Simpan file yang disiapkan ke disk, lalu mulai ulang.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Simpan file yang disiapkan ke disk, lalu matikan.</translation></message>
    <message><source>Saving staged files</source><translation>Menyimpan file yang disiapkan</translation></message>
    <message><source>Saving to disk</source><translation>Menyimpan ke disk</translation></message>
    <message><source>Saving…</source><translation>Menyimpan…</translation></message>
    <message><source>Scanning staged paths</source><translation>Memindai jalur yang disiapkan</translation></message>
    <message><source>Search logs</source><translation>Cari log</translation></message>
    <message><source>Search messages or modules</source><translation>Cari pesan atau modul</translation></message>
    <message><source>Search paths</source><translation>Cari jalur</translation></message>
    <message><source>Search this folder</source><translation>Cari di folder ini</translation></message>
    <message><source>Select a file or enter its name</source><translation>Pilih file atau masukkan namanya</translation></message>
    <message><source>Select a file to open.</source><translation>Pilih file untuk dibuka.</translation></message>
    <message><source>Select a folder.</source><translation>Pilih folder.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Pilih kunci untuk mengoperasikan seluruh subpohonnya, atau pilih nilai bernama untuk mengoperasikan nilai itu saja.</translation></message>
    <message><source>Select an available subkey</source><translation>Pilih subkunci yang tersedia</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Pilih file saja atau buka folder untuk menelusuri isinya.</translation></message>
    <message><source>Select files or folders</source><translation>Pilih file atau folder</translation></message>
    <message><source>Select folder</source><translation>Pilih folder</translation></message>
    <message><source>Select folders only.</source><translation>Pilih folder saja.</translation></message>
    <message><source>Select registry key or value</source><translation>Pilih kunci atau nilai registri</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Pilih kunci atau nilai registri untuk dihapus</translation></message>
    <message><source>Select the current folder</source><translation>Pilih folder saat ini</translation></message>
    <message><source>Select this file</source><translation>Pilih file ini</translation></message>
    <message><source>Select this registry key</source><translation>Pilih kunci registri ini</translation></message>
    <message><source>Select this registry value</source><translation>Pilih nilai registri ini</translation></message>
    <message><source>Selected file</source><translation>File terpilih</translation></message>
    <message><source>Selected folder</source><translation>Folder terpilih</translation></message>
    <message><source>Selection</source><translation>Pilihan</translation></message>
    <message><source>Service account</source><translation>Akun layanan</translation></message>
    <message><source>Service configuration needs repair</source><translation>Konfigurasi layanan perlu diperbaiki</translation></message>
    <message><source>Service details</source><translation>Detail layanan</translation></message>
    <message><source>Service name</source><translation>Nama layanan</translation></message>
    <message><source>Service status</source><translation>Status layanan</translation></message>
    <message><source>Service stopped</source><translation>Layanan dihentikan</translation></message>
    <message><source>Shut down anyway</source><translation>Tetap matikan</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>Pematian akan dilanjutkan setelah penyimpanan. Biarkan aplikasi tetap terbuka.</translation></message>
    <message><source>Size</source><translation>Ukuran</translation></message>
    <message><source>Some changes could not be applied</source><translation>Beberapa perubahan tidak dapat diterapkan</translation></message>
    <message><source>Some files could not be saved</source><translation>Beberapa file tidak dapat disimpan</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Sebagian informasi perangkat keras tidak dapat dibaca. Informasi diagnostik lainnya tetap dapat dilihat dan disalin.</translation></message>
    <message><source>Source code</source><translation>Kode sumber</translation></message>
    <message><source>Staged file</source><translation>File yang disiapkan</translation></message>
    <message><source>Staged path</source><translation>Jalur yang disiapkan</translation></message>
    <message><source>Standard user</source><translation>Pengguna standar</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Mulai otomatis bersama Windows</translation></message>
    <message><source>Startup type</source><translation>Jenis mulai</translation></message>
    <message><source>Status unavailable</source><translation>Status tidak tersedia</translation></message>
    <message><source>Stopped</source><translation>Dihentikan</translation></message>
    <message><source>Storage</source><translation>Penyimpanan</translation></message>
    <message><source>System</source><translation>Sistem</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Perubahan sistem saat UWF pertama kali diaktifkan</translation></message>
    <message><source>System memory</source><translation>Memori sistem</translation></message>
    <message><source>System metadata</source><translation>Metadata sistem</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Metadata sistem hanya untuk diagnosis dan tidak dapat dikomit.</translation></message>
    <message><source>Takes effect after restart</source><translation>Berlaku setelah mulai ulang</translation></message>
    <message><source>Target</source><translation>Target</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistensi lisensi Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Penyedia UWF tidak dapat menyelesaikan enumerasi. Ini dapat terjadi jika overlay besar atau sumber daya sistem tidak mencukupi.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>Konfigurasi telah ditulis ke sistem. Sebagian besar perubahan berlaku setelah mulai ulang.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>Konfigurasi sesuai dengan target sesi berikutnya. Lihat atau ekspor perintah dalam Konfigurasi lengkap.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Catatan log saat ini akan dihapus dan tidak dapat dipulihkan.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Folder tujuan tidak ada.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Status layanan mode lanjutan tidak dapat dibaca. Periksa izin layanan dan coba lagi.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>Daftar dapat melewatkan file atau menyertakan file yang sudah dikomit. Jangan gunakan sebagai daftar komit.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Item terpilih sudah tidak ada: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Layanan terpasang tetapi berhenti. Mulai layanan untuk memulihkan penyimpanan otomatis.</translation></message>
    <message><source>This PC</source><translation>PC ini</translation></message>
    <message><source>This folder is empty</source><translation>Folder ini kosong</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Laporan ini mungkin berisi nama akun, pengenal perangkat, dan jalur file. Tinjau sebelum membagikannya.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Simpan skrip CMD UTF-8 ini sebagai file .cmd dan jalankan di Command Prompt. Ekspor menggunakan skrip yang sama.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Coba kata kunci atau tingkat log lain.</translation></message>
    <message><source>Try another search term.</source><translation>Coba istilah pencarian lain.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Jenis dan ukuran maksimum tidak dapat diubah saat filter aktif. Nonaktifkan filter dan mulai ulang terlebih dahulu.</translation></message>
    <message><source>UI agent</source><translation>Agen antarmuka</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 baris</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>Konfigurasi UWF tidak dapat dibaca. Pastikan Unified Write Filter terpasang di Windows, lalu baca lagi.</translation></message>
    <message><source>UWF feature</source><translation>Fitur UWF</translation></message>
    <message><source>UWF filter</source><translation>Filter UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF saat ini tidak tersedia</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Mulai ulang aman UWF akan dijalankan langsung.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>Pematian aman UWF akan dijalankan langsung.</translation></message>
    <message><source>Unable to access this folder</source><translation>Tidak dapat mengakses folder ini</translation></message>
    <message><source>Unable to access this registry key</source><translation>Tidak dapat mengakses kunci registri ini</translation></message>
    <message><source>Unable to read logs</source><translation>Tidak dapat membaca log</translation></message>
    <message><source>Unable to read service status</source><translation>Tidak dapat membaca status layanan</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Tidak dapat membaca overlay saat ini</translation></message>
    <message><source>Unable to read this folder.</source><translation>Tidak dapat membaca folder ini.</translation></message>
    <message><source>Unable to select this item</source><translation>Tidak dapat memilih item ini</translation></message>
    <message><source>Unavailable</source><translation>Tidak tersedia</translation></message>
    <message><source>Undo removal</source><translation>Urungkan penghapusan</translation></message>
    <message><source>Unprotected</source><translation>Tidak dilindungi</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Item yang gagal belum disimpan. Pilih baris untuk melihat jalur lengkap dan alasannya.</translation></message>
    <message><source>Up one folder</source><translation>Naik satu folder</translation></message>
    <message><source>Usage unavailable</source><translation>Penggunaan tidak tersedia</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Gunakan seluruh kunci, termasuk semua subkunci</translation></message>
    <message><source>Version %1</source><translation>Versi %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Lihat catatan aplikasi dan informasi diagnostik.</translation></message>
    <message><source>View commands for these changes</source><translation>Lihat perintah untuk perubahan ini</translation></message>
    <message><source>View failure details</source><translation>Lihat detail kegagalan</translation></message>
    <message><source>View overlay</source><translation>Lihat overlay</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Lihat snapshot diagnostik file untuk %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Pengikatan volume %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Perlindungan volume %1</translation></message>
    <message><source>Volume identity</source><translation>Identitas volume</translation></message>
    <message><source>Warning</source><translation>Peringatan</translation></message>
    <message><source>Warning %1 MB</source><translation>Peringatan %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Seluruh kunci dan semua subkunci · rekursif</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF menyediakan perlindungan penulisan. Aplikasi ini mengonfigurasi dan mengelolanya.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Tanpa mode lanjutan, gunakan Matikan dengan aman atau Mulai ulang dengan aman dalam aplikasi ini untuk menyimpan file yang disiapkan.</translation></message>
    <message><source>Write protection</source><translation>Perlindungan penulisan</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Tulis perubahan konfigurasi ini ke sistem? Sebagian besar berlaku setelah mulai ulang.</translation></message>
    <message><source>staged files</source><translation>file yang disiapkan</translation></message>
    <message><source>uwfmgr commands</source><translation>perintah uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Tandai jalur yang dipilih untuk dihapus. Berlaku setelah diterapkan.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Hapus dari daftar penyiapan berkas otomatis setelah diterapkan.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Tambahkan ke daftar penyiapan berkas otomatis setelah diterapkan.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Menerapkan rencana ini akan mengosongkan daftar penyiapan yang tersimpan dan menghapus pengecualian registri terkait jika tidak lagi diperlukan.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Perubahan penyiapan berkas harus diterapkan di UWF Manager dan tidak dapat diekspor sebagai perintah uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Daftar penyiapan berkas disimpan setelah diterapkan</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Cakupan snapshot overlay</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Daftar ini hanya untuk diagnostik. Daftar ini tidak dapat menentukan file mana yang harus dikomit.</translation></message>
    <message><source>NTFS volumes only</source><translation>Hanya volume NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Overlay besar mungkin lambat dibaca atau gagal karena sumber daya tidak mencukupi.</translation></message>
    <message><source>Small files may be missing</source><translation>File kecil mungkin tidak tercantum</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>File yang lebih kecil dari ukuran klaster disk (biasanya 4 KB) mungkin tidak muncul dalam daftar.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Tidak menunjukkan komit yang tertunda</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>File yang sudah dikomit, jalur yang dikecualikan, dan item yang terpengaruh operasi lain juga dapat muncul.</translation></message>

    <message><source>Command processing result</source><translation>Hasil pemrosesan perintah</translation></message>
    <message><source>Copy command</source><translation>Salin perintah</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 cabang tidak dapat dibaca</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 cabang tidak dapat dibaca. Lanjutkan dengan konten yang ditemukan atau coba lagi hanya item yang gagal.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 berkas · %2 folder</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 nilai · %2 kunci</translation></message>
    <message><source>Attempts</source><translation>Percobaan</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Batalkan tugas tersisa</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Simpan permanen hanya konten yang berhasil ditelusuri. Cabang yang tidak terbaca tetap tidak berubah.</translation></message>
    <message><source>Committing changes</source><translation>Menyimpan perubahan permanen</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Penghapusan dan penyimpanan permanen yang selesai tidak dapat dibatalkan.</translation></message>
    <message><source>Continue…</source><translation>Lanjutkan…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Hapus dan simpan permanen hanya target yang ditemukan. Cabang yang tidak terbaca beserta folder atau kunci induknya dipertahankan.</translation></message>
    <message><source>Deleting and committing</source><translation>Menghapus dan menyimpan permanen</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Penghapusan dan penyimpanan permanen tidak dapat dibatalkan. Hanya cakupan yang dipilih diproses; konfigurasi UWF tertunda tidak diterapkan.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Menentukan cakupan. Cabang yang tidak terbaca dicatat dan penelusuran konten lainnya dilanjutkan.</translation></message>
    <message><source>Export traversal records</source><translation>Ekspor catatan penelusuran</translation></message>
    <message><source>No actionable content</source><translation>Tidak ada konten yang dapat diproses</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Tidak ditemukan target yang dapat diproses. Tidak ada yang dihapus atau disimpan permanen.</translation></message>
    <message><source>No changes have been committed.</source><translation>Belum ada perubahan yang disimpan permanen</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Belum ada yang dihapus atau disimpan permanen</translation></message>
    <message><source>Path</source><translation>Jalur</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Hapus cakupan yang dipilih secara permanen dan simpan hasil penghapusan ke volume terlindungi.</translation></message>
    <message><source>Preparation complete</source><translation>Persiapan selesai</translation></message>
    <message><source>Preparing commit</source><translation>Menyiapkan penyimpanan permanen</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Menyiapkan penghapusan dan penyimpanan permanen</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Proses hanya konten yang berhasil ditelusuri</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Titik reparse dilewati agar penelusuran tetap dalam cakupan yang dipilih.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Memulai ulang tidak membatalkan penyimpanan permanen. Hanya cakupan yang dipilih diproses; konfigurasi UWF tertunda tidak diterapkan.</translation></message>
    <message><source>Retry failed items</source><translation>Coba lagi item yang gagal</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Simpan perubahan dalam cakupan yang dipilih ke volume terlindungi.</translation></message>
    <message><source>Stopping the operation</source><translation>Menghentikan operasi</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Catatan keberhasilan dan kegagalan dipertahankan</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Folder tidak kosong; konten yang belum diproses dipertahankan.</translation></message>
    <message><source>The operation could not be completed</source><translation>Operasi tidak dapat diselesaikan</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>Kunci registri memiliki nilai baru; konten yang belum diproses dipertahankan.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>Kunci registri masih memiliki subkunci; konten yang belum diproses dipertahankan.</translation></message>
    <message><source>The target no longer exists.</source><translation>Target sudah tidak ada.</translation></message>
    <message><source>Traversal complete</source><translation>Penelusuran selesai</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Penelusuran selesai dengan konten yang tidak terbaca</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Catatan penelusuran dipertahankan dan dapat diekspor.</translation></message>
    <message><source>Traverse again</source><translation>Telusuri lagi</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Telusuri lagi atau tutup jendela ini.</translation></message>
    <message><source>Traversing the folder</source><translation>Menelusuri folder</translation></message>
    <message><source>Traversing the registry</source><translation>Menelusuri registri</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Operasi tidak dapat disiapkan</translation></message>
    <message><source>View results</source><translation>Lihat hasil</translation></message>

    <message><source>All changes committed</source><translation>Semua perubahan berhasil disimpan</translation></message>
    <message><source>Some changes committed</source><translation>Sebagian perubahan berhasil disimpan</translation></message>
    <message><source>No changes could be committed</source><translation>Tidak ada perubahan yang berhasil disimpan</translation></message>
    <message><source>No changes to commit</source><translation>Tidak ada perubahan untuk disimpan</translation></message>
    <message><source>Commit interrupted</source><translation>Penyimpanan terhenti</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 item belum diproses.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 operasi registri gagal.</translation></message>
    <message><source>The folder does not exist.</source><translation>Folder tidak ada.</translation></message>
    <message><source>Access denied.</source><translation>Akses ditolak.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Item dengan nama ini sudah ada.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Folder tidak dapat dibuat. Periksa nama dan izin akses.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Nama item tidak dapat diubah. Periksa nama dan izin akses.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Peramban ini tidak mendukung penggantian nama.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Draf daftar berkas yang disiapkan berubah setelah rencana ini dibuka. Tinjau rencana baru.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Daftar tersimpan berkas yang disiapkan berubah setelah rencana ini dibuka. Muat ulang dan tinjau perubahannya.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Daftar tersimpan berkas yang disiapkan tidak sesuai dengan rencana yang disetujui.</translation></message>
    <message><source>Export configuration</source><translation>Ekspor konfigurasi</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Simpan tema, bahasa, dan visibilitas HUB saat ini ke UWF.ini di folder yang sama dengan aplikasi ini.</translation></message>
    <message><source>Configuration saved</source><translation>Konfigurasi disimpan</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Pengaturan antarmuka saat ini disimpan ke:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini sudah ada. File yang ada tidak diubah:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>Tidak dapat menyimpan UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Desktop</translation></message>
    <message><source>Downloads</source><translation>Unduhan</translation></message>
    <message><source>Documents</source><translation>Dokumen</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Penyimpanan file otomatis dihentikan. Atasi kesalahan sebelum mencoba lagi.</translation></message>
    <message><source>No net change for this setting</source><translation>Nilai akhir pengaturan ini tidak berubah setelah impor</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Nilai akhir draf: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Nilai akhir draf: %1 → %2 (diminta: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Diminta: %1 → %2; digantikan oleh perintah berikutnya. Nilai akhir draf → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Draf %1 diperbarui</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Draf pengaturan perlindungan volume %1 diperbarui → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Pilih entri yang akan dihapus.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Entri yang dipilih saat ini tidak dapat dipulihkan.</translation></message>
</context>
</TS>
