<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="tr" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Önizleme</translation></message>
    <message><source>enable</source><translation>etkinleştir</translation></message>
    <message><source>disable</source><translation>devre dışı bırak</translation></message>
    <message><source>Enabled</source><translation>Etkin</translation></message>
    <message><source>Disabled</source><translation>Devre dışı</translation></message>
    <message><source>OK</source><translation>Tamam</translation></message>
    <message><source>Cancel</source><translation>İptal</translation></message>
    <message><source>Close</source><translation>Kapat</translation></message>
    <message><source>Refresh</source><translation>Yenile</translation></message>
    <message><source>Add</source><translation>Ekle</translation></message>
    <message><source>Remove</source><translation>Kaldır</translation></message>
    <message><source>Yes</source><translation>Evet</translation></message>
    <message><source>No</source><translation>Hayır</translation></message>
    <message><source>Excluded</source><translation>Hariç</translation></message>
    <message><source>Not excluded</source><translation>Hariç değil</translation></message>
    <message><source>File…</source><translation>Dosya…</translation></message>
    <message><source>Folder…</source><translation>Klasör…</translation></message>
    <message><source>volume ID</source><translation>birim kimliği</translation></message>
    <message><source>drive letter</source><translation>sürücü harfi</translation></message>
    <message><source>Drive letter</source><translation>Sürücü harfi</translation></message>
    <message><source>Volume ID</source><translation>Birim kimliği</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Bu birimin geçerli oturumdaki koruma durumu (salt okunur).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Sonraki oturumda bu birimi korur. Yazma işlemleri yer paylaşımına yönlendirilir ve yeniden başlatmada silinir.</translation></message>
    <message><source>Current session</source><translation>Geçerli oturum</translation></message>
    <message><source>Next session</source><translation>Sonraki oturum</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Birimi tanımlama yöntemi. Sürücü harfi daha basittir; ancak disk ekleme veya çıkarma sonrası harf değişirse bağ kopar. Birim kimliği sabit kalır.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF bu birimi desteklemiyor: </translation></message>
    <message><source>Global settings</source><translation>Genel ayarlar</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Geçerli oturumdaki UWF filtre durumu (salt okunur).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Sonraki oturumda UWF’yi etkinleştirir. Korunan birimlere yazmalar yer paylaşımına yönlendirilir ve yeniden başlatmada silinir.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Tür ve en büyük boyutu değiştirmek için:
1. Yukarıdan filtreyi devre dışı bırakın.
2. Filtrenin kapanması için yeniden başlatın.
3. Ayarı değiştirin.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Yer paylaşımı konumu: RAM hızlıdır ancak bellek kullanır; Disk sistem sürücüsünü kullanır ve daha fazla kapasite sunar. İkisi de yeniden başlatmada silinir.</translation></message>
    <message><source>Type</source><translation>Tür</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>En büyük yer paylaşımı kapasitesi. RAM toplam bellekle sınırlıdır. Disk en az 1024 MB ve sistem biriminde yeterli boş alan gerektirir.</translation></message>
    <message><source>Maximum size</source><translation>En büyük boyut</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>En büyük boyut · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Uyarı eşiği</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Katman kullanımı bu değere ulaştığında olay günlüğüne uyarı düzeyinde bir bildirim kaydeder. Her iki olay da etkinse bu değer kritik eşikten düşük olmalıdır. 0 değeri yalnızca uyarı olayını devre dışı bırakır.</translation></message>
    <message><source>Critical threshold</source><translation>Kritik eşik</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Katman kullanımı bu değere ulaştığında olay günlüğüne kritik düzeyde bir bildirim kaydeder. Her iki olay da etkinse bu değer uyarı eşiğinden yüksek olmalıdır. 0 değeri yalnızca kritik olayı devre dışı bırakır.</translation></message>
    <message><source>Overlay</source><translation>Yer paylaşımı</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Desteklenmeyen sürücü türü (yalnızca yerel sabit diskler).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Birim, UWF’nin korunan birim başına 16 TiB sınırını aşıyor.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: birim korunabilir ancak dosya dışlamaları ve dosya bazında kalıcı kayıt desteklenmez.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Başarısız: Birim bilgilerini okuma.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 boş / %2</translation></message>
    <message><source>Commit</source><translation>Kalıcı kaydet</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Bu dosyanın yer paylaşımı değişikliklerini diske kalıcı kaydet / Kayıt defteri değişikliklerini kalıcı kaydet. Bu işlem geri alınamaz.</translation></message>
    <message><source>Commit file changes…</source><translation>Dosya değişikliklerini kalıcı kaydet…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Diske kalıcı kaydedilecek dosyayı seç. Bu dosyanın yer paylaşımı değişikliklerini diske kalıcı kaydet.</translation></message>
    <message><source>Commit folder changes…</source><translation>Klasör değişikliklerini kalıcı kaydet…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Diske kalıcı kaydedilecek klasörü seç. Bu klasörün yer paylaşımı değişikliklerini diske kalıcı kaydet. Tüm alt klasörler dahildir.</translation></message>
    <message><source>Delete and commit file…</source><translation>Dosyayı sil ve silmeyi diske kalıcı kaydet…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Silinmesi kalıcı kaydedilecek dosyayı seç. Dosyayı sil ve silmeyi diske kalıcı kaydet.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Klasörü ve içeriğini sil, silmeleri diske kalıcı kaydet…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Silinmesi kalıcı kaydedilecek klasörü seç. Klasörü ve içeriğini sil, silmeleri diske kalıcı kaydet. Tüm alt klasörler dahildir.</translation></message>
    <message><source>Commit registry changes…</source><translation>Kayıt defteri değişikliklerini kalıcı kaydet…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Anahtarı ve isteğe bağlı değer adını girin. Değer adı boşsa anahtarın ve tüm alt anahtarlarının bütün değerleri kalıcı kaydedilir.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Kayıt defteri silmesini kalıcı kaydet…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Silinecek anahtarı ve isteğe bağlı değer adını girin. Ad boşsa anahtar, tüm değerleri ve alt anahtarları silinir ve silmeler kalıcı kaydedilir.</translation></message>
    <message><source>File exclusions</source><translation>Dosya dışlamaları</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Bu birimde UWF koruması dışında tutulan dosya ve klasörler. Yolu kopyalamak için girdiye çift tıklayın.</translation></message>
    <message><source>Registry exclusions</source><translation>Kayıt defteri dışlamaları</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Kayıt defteri dışlamaları geneldir; birim korumasına değil UWF filtresine bağlıdır. Ortak liste disk sekmelerinde yalnızca bir kez gösterilir. Yolu kopyalamak için çift tıklayın.</translation></message>
    <message><source>File staging</source><translation>Otomatik dosya kaydı</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>dosya / klasör: Güvenli kapatma veya yeniden başlatma öncesinde otomatik kalıcı kayıt. Tüm alt klasörler dahildir. Liste doluyken şu kayıt defteri dışlaması listeyi korunan oturumlar arasında saklar: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Otomatik kayıt listesine dosya veya klasör ekleyin.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Diske kalıcı kaydedilecek dosyayı seç. Güvenli kapatma veya yeniden başlatma öncesinde otomatik kalıcı kayıt.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Diske kalıcı kaydedilecek klasörü seç. Güvenli kapatma veya yeniden başlatma öncesinde otomatik kalıcı kayıt. Tüm alt klasörler dahildir.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Diske kalıcı kaydedilecek dosyayı seç · Otomatik dosya kaydı</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Diske kalıcı kaydedilecek klasörü seç · Otomatik dosya kaydı · Tüm alt klasörler dahildir.</translation></message>
    <message><source>Cannot stage this path</source><translation>Bu yol otomatik kayda eklenemiyor</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Yol bir UWF dışlamasıyla çakışıyor. Önce dışlamayı kaldırın:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Yol otomatik kayıt girdisiyle çakışıyor. Dışlamadan önce girdiyi kaldırın:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Yol mevcut bir otomatik kayıt girdisiyle çakışıyor</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Otomatik kayıt listesi boş değilken bu kayıt defteri dışlaması gereklidir</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Yol yok veya beklenen dosya türünde değil:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Yol incelenemedi:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Yeniden ayrıştırma noktaları otomatik kayda eklenemez:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Sürücü harfi yok; UWF kalıcı kaydedemez:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Yol %1 biriminde. İlgili birimin otomatik dosya kaydı sekmesinden ekleyin.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Birim kökü eklenemez; aksi hâlde tüm birim özyinelemeli kaydedilir.</translation></message>
    <message><source>File staging could not be saved</source><translation>Başarısız: Otomatik dosya kaydı listesini saklama</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Güncelleme tamamlanamadığından otomatik kayıt listesi yeniden yüklendi:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Otomatik dosya kaydı: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Güvenli kapatma veya yeniden başlatma öncesinde otomatik kalıcı kayıt.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Otomatik dosya kaydı: %1 · dosya: %2 · klasör: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Otomatik dosya kaydı, yeniden başlatmalardan sonra listesini korumak için bu kayıt defteri dışlamasına ihtiyaç duyar.</translation></message>
    <message><source>Built-in</source><translation>Yerleşik</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>UWF filtresi devre dışı. Yer paylaşımında birikmiş değişiklik olmadığından kaydedilecek bir şey yok.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>%1 dosya sistemi dosya bazında kalıcı kaydı desteklemiyor.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Bu birim UWF tarafından desteklenmiyor.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Bu birim dosya kalıcı kaydını desteklemiyor. Yalnızca kayıt defteri kalıcı kaydı kullanılabilir (dışlamaları geneldir).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>%1 dosya sistemi dosya bazında kalıcı kaydı desteklemiyor. Yalnızca kayıt defteri kalıcı kaydı kullanılabilir (dışlamaları geneldir).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Birim şu anda UWF ile korunmuyor; kaydedilecek dosya değişikliği yok.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Birim şu anda UWF ile korunmuyor; kaydedilecek dosya değişikliği yok. Yalnızca kayıt defteri kalıcı kaydı kullanılabilir (dışlamaları geneldir).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Diske kalıcı kaydedilecek dosyayı seç</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Diske kalıcı kaydedilecek klasörü seç</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Silinmesi kalıcı kaydedilecek dosyayı seç</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Silinmesi kalıcı kaydedilecek klasörü seç</translation></message>
    <message><source>Name</source><translation>Ad</translation></message>
    <message><source>Data</source><translation>Veri</translation></message>
    <message><source>(Default)</source><translation>(Varsayılan)</translation></message>
    <message><source>(value not set)</source><translation>(değer ayarlanmadı)</translation></message>
    <message><source>(empty)</source><translation>(boş)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Bilinmiyor (Değer: Dosya çok büyük)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Yer paylaşımı dosyalarını göster. &lt;b&gt;Yalnızca tanılama görüntüsüdür.&lt;/b&gt; Kaydedilecekleri bu listeye göre belirlemeyin.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Yer paylaşımı dosyaları yalnızca NTFS birimlerinde listelenebilir.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF devre dışı; incelenebilecek yer paylaşımı yok.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Birim UWF ile korunmuyor; incelenebilecek yer paylaşımı yok.</translation></message>
    <message><source>Failed: %1</source><translation>Başarısız: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Dosyalar listelenirken WMI sağlayıcısı çöktü. Büyük yer paylaşımlarında veya yüksek G/Ç yükünde UWF_Overlay.GetOverlayFiles için bilinen bir kararsızlıktır.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Bellek yetersiz veya sağlayıcı işlemi desteklemiyor. NTFS ve boş bellek gerekir. Daha küçük yer paylaşımıyla veya bellek tüketen uygulamaları kapattıktan sonra yeniden deneyin.</translation></message>
    <message><source>Raw error: %1</source><translation>Hata kodu: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Dosyaya aktar… · Yer paylaşımı · dosya</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Metin dosyaları (*.txt);;Tüm dosyalar (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Komut dosyaları (*.cmd);;Tüm dosyalar (*)</translation></message>
    <message><source>Export failed</source><translation>Dışa aktarma başarısız</translation></message>
    <message><source>Export finished</source><translation>Dışa aktarma tamamlandı</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Başarısız: Dosyayı açma (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Başarısız: Dosyaya yazma (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Yapılandırma: Dosyaya yazma · girdi: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Birimin tamamı (%1) dışlanamaz.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Disk belleği, takas ve hazırda bekletme dosyaları dışlanamaz; UWF bu dosyalara bağlıdır.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Dizinin tamamı dışlanamaz: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Dizinin tamamı dışlanamaz: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Dizinin tamamı dışlanamaz: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Bu kritik sistem dosyası dışlanamaz: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Kullanıcı kayıt defteri dosyası NTUSER.DAT dışlanamaz.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Kayıt defteri anahtarı boş olamaz.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Kayıt defteri anahtarı ters eğik çizgiyle başlayamaz.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Yol art arda ters eğik çizgiler içeriyor ve geçersiz.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Kayıt defteri yollarında ayırıcı `\` olmalıdır, `/` kullanmayın.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Yol görünmez denetim karakterleri içeriyor ve geçersiz.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>UWF belgeleri etki alanı bilgisayar hesabı sırrının dışlanmasını yasaklar: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Yalnızca şu üst düzey anahtarların belirli alt anahtarları dışlanabilir:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Dosya veya klasörü dışlama listesine ekleyin. Dışlananlar UWF ile korunmaz.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Eklenecek dosyayı seç · Dosya dışlamaları.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Eklenecek klasörü seç · Dosya dışlamaları. Tüm alt klasörler dahildir.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Dışlanacak kayıt defteri anahtarının yolunu girin.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Dışlanacak kayıt defteri anahtarının yolunu girin. / Kalıcılık.</translation></message>
    <message><source>Registry key…</source><translation>Kayıt defteri · Anahtar…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Etki alanı gizli anahtarı (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Terminal Hizmetleri istemci erişim lisansı (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Etki alanı sırrını (bilgisayar hesabı parolası) UWF oturumları arasında saklar.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Terminal Hizmetleri istemci lisanslarını UWF oturumları arasında saklar.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Kalıcılık %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Seçilenleri kaldır</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Seçili dışlamaları kaldırın. Uygulamadan sonra geçerli olur.</translation></message>
    <message><source>Open containing folder</source><translation>İçeren klasörü aç</translation></message>
    <message><source>Copy file path</source><translation>Dosya yolunu kopyala</translation></message>
    <message><source>Copy registry path</source><translation>Kayıt defteri yolunu kopyala</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Bu klasörün yer paylaşımı değişikliklerini diske kalıcı kaydet…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Bu dosyanın yer paylaşımı değişikliklerini diske kalıcı kaydet…</translation></message>
    <message><source>Copied to clipboard</source><translation>Panoya kopyalandı</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Eklenecek dosyayı seç · Dosya dışlamaları. Birden fazla dosya seçilebilir.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Eklenecek klasörü seç · Dosya dışlamaları</translation></message>
    <message><source>Add registry exclusion</source><translation>Kayıt defteri dışlaması ekleme</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>%1 yolu %2 biriminde değil; bu birimin dışlamalarına eklenemez.</translation></message>
    <message><source>Path is not on this volume</source><translation>Yol bu birimde değil</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Bu dışlama eklenemiyor</translation></message>
    <message><source>File: %1</source><translation>Dosya… %1</translation></message>
    <message><source>Folder: %1</source><translation>Klasör… %1</translation></message>
    <message><source>Registry: %1</source><translation>Kayıt defteri: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Varsayılan olarak dışlanır; istediğiniz zaman değiştirilebilir.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Geçerli oturum: %1
Sonraki oturum: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Bekleyen değişiklikler: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Birleşik Yazma Filtresi (UWF) Yöneticisi</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Geçerli UWF durumunu ve sonraki oturum yapılandırmasını yeniden okur.</translation></message>
    <message><source>Review and apply</source><translation>İncele ve uygula</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Bekleyen tüm değişiklikleri inceleyip topluca uygular. Çoğu değişiklik sonraki yeniden başlatmada geçerli olur.</translation></message>
    <message><source>Safe shutdown</source><translation>Güvenli kapat</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Sistem UWF üzerinden güvenli kapatılacak. UWF yer paylaşımı dolu olsa da kullanılabilir.</translation></message>
    <message><source>Safe restart</source><translation>Güvenli yeniden başlat</translation></message>
    <message><source>Direct restart</source><translation>Doğrudan yeniden başlat</translation></message>
    <message><source>Direct shutdown</source><translation>Doğrudan kapat</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Sistem UWF üzerinden güvenli yeniden başlatılacak. UWF yer paylaşımı dolu olsa da kullanılabilir.</translation></message>
    <message><source>Log</source><translation>Günlük</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Sorun giderme için bu oturumun iç günlüğünü gösterir.</translation></message>
    <message><source>About</source><translation>Hakkında</translation></message>
    <message><source>Switch display language</source><translation>Görüntüleme dilini değiştir</translation></message>
    <message><source>Language</source><translation>Dil</translation></message>
    <message><source>Theme</source><translation>Tema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Açık/koyu temayı değiştir</translation></message>
    <message><source>System information</source><translation>Sistem bilgileri</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Tanılama anlık görüntüsü</translation></message>
    <message><source>Read only</source><translation>Salt okunur</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Hedef bulunamadı; yer paylaşımında kaydedilecek bir şey yok.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Hedef yalnızca yer paylaşımında: UWF altında oluşturulmuş, diske veya kalıcı kayıt defterine hiç kaydedilmemiş. Yeniden başlatmada kaybolur; silmeyi kaydetmek gereksiz ve olanaksızdır.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Sistem bir parametreyi reddetti (geçersiz yol veya bağımsız değişken).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>İşlem başarısız. Hedef başka bir işlemde kullanılıyor olabilir; örneğin klasör Dosya Gezgini’nde veya dosya bir uygulamada açık olabilir. İlgili programları kapatıp yeniden deneyin.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>İşlem başarısız; ayrıntılar günlükte.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>İşlem reddedildi (kod %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Bilinmiyor.</translation></message>
    <message><source>Commit canceled</source><translation>Kalıcı kayıt iptal edildi</translation></message>

    <message><source>Succeeded</source><translation>Başarılı</translation></message>
    <message><source>Skipped</source><translation>Atlandı</translation></message>
    <message><source>Failed</source><translation>Başarısız</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Başarılı: %1; Atlandı: %2; Başarısız: %3.</translation></message>

    <message><source>Existed before</source><translation>Önceden var</translation></message>
    <message><source>Exists after</source><translation>Sonrasında var</translation></message>
    <message><source>Error code</source><translation>Hata kodu</translation></message>
    <message><source>Reason</source><translation>Neden</translation></message>
    <message><source>Previous page</source><translation>Önceki sayfa</translation></message>
    <message><source>First page</source><translation>İlk sayfa</translation></message>
    <message><source>Last page</source><translation>Son sayfa</translation></message>
    <message><source>Next page</source><translation>Sonraki sayfa</translation></message>
    <message><source>Copy selected rows</source><translation>Seçili satırları kopyala</translation></message>
    <message><source>Copy all</source><translation>Tümünü kopyala</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Otomatik kayıt listesi boş değilken bu kayıt defteri dışlaması gereklidir. Otomatik dosya kaydı: Temizle → Kayıt defteri dışlamasını kaldırma.</translation></message>
    <message><source>Copy current page</source><translation>Geçerli sayfayı kopyala</translation></message>
    <message><source>No pending changes</source><translation>Bekleyen değişiklik yok</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Kayıt defteri dışlamaları · Genel ayarlar.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Birim %1: UWF koruması / Dosya dışlamaları / Otomatik dosya kaydı.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Desteklenmiyor</translation></message>
    <message><source>Review and apply changes</source><translation>Değişiklikleri incele ve uygula</translation></message>
    <message><source>Confirm apply</source><translation>Uygulamayı onayla</translation></message>
    <message><source>Result</source><translation>Sonuç</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filtre: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Uyarı eşiğini ayarlama: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Kritik eşiği ayarlama: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Yer paylaşımı türünü ayarlama / En büyük boyutu ayarlama: Tür ve en büyük boyutu değiştirmek için:
1. Yukarıdan filtreyi devre dışı bırakın.
2. Filtrenin kapanması için yeniden başlatın.
3. Ayarı değiştirin.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Yer paylaşımı türünü ayarlama: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ En büyük boyutu ayarlama: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ En büyük boyutu ayarlama: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Birim %1 · UWF koruması: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Birim %1 · Bağlama ölçütü: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Birim %1 · Dosya dışlaması ekleme: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Birim %1 · Dosya dışlamasını kaldırma: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Kayıt defteri dışlaması ekleme: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Kayıt defteri dışlamasını kaldırma: %1</translation></message>
    <message><source>Time</source><translation>Zaman</translation></message>
    <message><source>Level</source><translation>Düzey</translation></message>
    <message><source>Tag</source><translation>Etiket</translation></message>
    <message><source>Message</source><translation>İleti</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Otomatik kaydedilecek dosyalar hesaplanıyor…</translation></message>
    <message><source>Committing staged files</source><translation>Hazırlanan dosyalar kaydediliyor</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%2 dosyadan %1 işlendi.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Ek hata ayrıntısı sağlanmadı.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Otomatik kayıt beklenmedik şekilde durdu:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Otomatik dosya kaydı tamamlanmadı: Bilinmeyen hata.</translation></message>
    <message><source>File staging will be skipped</source><translation>Otomatik dosya kaydı atlanacak</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Doğrudan yeniden başlat: Otomatik dosya kaydı atlanacak.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Doğrudan kapat: Otomatik dosya kaydı atlanacak.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Doğrudan güç işlemi seçildiğinden otomatik kayıt atlandı.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Güvenli kapat: Başarısız</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Kapat: Başarısız (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Güvenli yeniden başlat: Başarısız</translation></message>
    <message><source>Direct restart failed</source><translation>Doğrudan yeniden başlat: Başarısız</translation></message>
    <message><source>Direct shutdown failed</source><translation>Doğrudan kapat: Başarısız</translation></message>
    <message><source>Restart failed: %1</source><translation>Yeniden başlat: Başarısız (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Otomatik kayıt listesi okunamadı:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Otomatik kayıt listesi okunamadı: Bilinmeyen hata.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>dosya: %1 · Başarılı: %2 · Atlandı (dosya): %3 · Atlandı (girdi): %4 · Başarısız: %5</translation></message>
    <message><source>File staging preparation</source><translation>Otomatik dosya kaydı: İşlem ayrıntıları</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Yeniden ayrıştırma noktaları desteklenmiyor.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Yalnızca yerel birimlerdeki mutlak yollar eklenebilir.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Yalnızca yerel birimlerdeki mutlak yollar eklenebilir.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Otomatik kayıt yolu incelenemedi.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Hazırlanan dosya yolu artık farklı bir türü gösteriyor.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Hazırlanan klasör yolu artık farklı bir türü gösteriyor.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Hazırlanan klasörün tamamı listelenemedi.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Yolun sürücü harfi yok.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>Birim kökü özyinelemeli işlenemez.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>UWF sağlayıcısı kalıcı kaydı reddetti.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Kalıcı kayıt başarısız: Bilinmeyen hata.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>%1 ek hata günlüğe yazıldı.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Yolun sürücü harfi yok. Hedef birimi belirleme: Başarısız.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>%1 birimi için geçerli oturum kaydı yok.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Yol dosya dışlama listesinde.
Dışlama: %1</translation></message>

    <message><source>Continue</source><translation>Devam</translation></message>
    <message><source>Delete and commit</source><translation>Sil ve kalıcı kaydet</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Anahtar kayıt defteri dışlama listesinde.
Dışlama: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Geçerli oturum için kayıt defteri filtresi kaydı yok.</translation></message>

    <message><source>Export commands to file</source><translation>Komutları dosyaya aktar</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Dosyaya aktar… · Komut: %1
%2</translation></message>
    <message><source>Import</source><translation>İçe aktar</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Her satıra bir komut yazın veya yapıştırın. Metin, günlük veya betik (.txt, .bat, .ps1) yükleyin; bu komutu içeren satırlar eklenir. İçe aktarma değişiklikleri hazırlar. Sisteme yazmak için araç çubuğundaki “İncele ve uygula” seçeneğini kullanın.</translation></message>
    <message><source>Load from file…</source><translation>Dosyadan yükle…</translation></message>
    <message><source>Load recommended configuration</source><translation>Önerilen yapılandırmayı yükle</translation></message>
    <message><source>Choose recommended configuration</source><translation>Önerilen yapılandırmayı seç</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Eklenecek önerilen grupları seçin. İçe aktarmadan önce komutları tek tek inceleyebilir veya silebilirsiniz.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Microsoft’un resmî belgelerinden.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Microsoft’un önerdiği UWF dışlamaları</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>İçe aktarmadan önce inceleyin: UWF’nin dışlamaları kabul etmesi için klasörler mevcut olmalıdır.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Kaynak: genel filtre dışlamaları ve kötü amaçlı yazılım önleme desteği için resmî Microsoft UWF belgeleri.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Müşteri Deneyimini Geliştirme Programı (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Kalıcı sakla: İlke katılım durumu</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Kalıcı sakla: Yerel katılım durumu</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Kalıcı sakla: Yüklemeyi devre dışı bırakma işareti</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Arka Plan Akıllı Aktarım Hizmeti (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Kalıcı sakla: İndirme kuyruğu dosyaları</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Kalıcı sakla: Aktarım durumu dizini</translation></message>
    <message><source>Network profiles and policies</source><translation>Ağ profilleri ve ilkeleri</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Kablosuz ağ GPO ilkesi</translation></message>
    <message><source>Wired network GPO policy</source><translation>Kablolu ağ GPO ilkesi</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Kablosuz ağ GPO ilkesi · dosya</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Kablolu ağ GPO ilkesi · dosya</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Kablosuz arabirim profilleri</translation></message>
    <message><source>Wired network interface profiles</source><translation>Kablolu arabirim profilleri</translation></message>
    <message><source>Wireless service configuration</source><translation>Kablosuz hizmet yapılandırması</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Mobil geniş bant hizmeti yapılandırması</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Kablolu Otomatik Yapılandırma hizmeti ayarları</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>Ağ profili XML dosyaları cihaza özeldir. Gerekirse gerçek Interfaces\{GUID}\{GUID}.xml yollarını elle ekleyin.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Yaz saati (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Kalıcı sakla: Saat dilimi tanımları</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Kalıcı sakla: Seçili saat dilimi bilgileri</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Kalıcı sakla: Ürün dosyaları ve güncellemeler</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Kalıcı sakla: ProgramData imzaları ve durumu</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Kalıcı sakla: Windows Update · Günlük</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Kalıcı sakla: MpCmdRun · Günlük</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Kalıcı sakla: Ürünün kayıt defteri durumu</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Kalıcı sakla: WdBoot · Hizmet durumu</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Kalıcı sakla: WdFilter · Hizmet durumu</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Kalıcı sakla: WdNisSvc · Hizmet durumu</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Kalıcı sakla: WdNisDrv · Hizmet durumu</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Kalıcı sakla: WinDefend · Hizmet durumu</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Kalıcı sakla: İstemci program dosyaları</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Kalıcı sakla: Windows Update · Günlük</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Kalıcı sakla: MpCmdRun · Günlük</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Kalıcı sakla: Kötü amaçlı yazılım önleme imzaları ve durumu</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Kalıcı sakla: Kötü amaçlı yazılım önlemenin kayıt defteri durumu</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>uwfmgr komutları içeren dosyaları seç</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Metin dosyaları (*.txt *.cmd);;Tüm dosyalar (*)</translation></message>
    <message><source>Status</source><translation>Durum</translation></message>
    <message><source>Import failed</source><translation>İçe aktarma başarısız</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>İç hata: uygulayıcı kaydedilmemiş.</translation></message>
    <message><source>Nothing to import</source><translation>İçe aktarılacak komut yok</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Girdide uwfmgr komutu bulunamadı.</translation></message>
    <message><source>Duplicates: %1</source><translation>Yinelenen: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Desteklenmiyor: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Toplu işlem %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Yinelenen</translation></message>
    <message><source>Unsupported</source><translation>Desteklenmiyor</translation></message>
    <message><source>File too large</source><translation>Dosya çok büyük</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>%1 dosyası 5 MB’tan büyük olduğundan ayrıştırılmadı. Önce elle süzün.</translation></message>
    <message><source>Cannot read file</source><translation>Dosya okunamıyor</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Başarısız: Dosyayı açma (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Bekleyen değişiklikler atılsın mı?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Uygulanmamış %1 değişiklik var.

Atıp devam edilsin mi?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Eksik uwfmgr komutu</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Eksik bağımsız değişken: Boyut (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>Boyut MB cinsinden negatif olmayan bir tam sayı olmalıdır</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Eksik bağımsız değişken: Yer paylaşımı · Tür (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Geçersiz değer: Yer paylaşımı · Tür: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Eksik bağımsız değişken: Birim (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Birim C: gibi bir sürücü harfi olmalıdır</translation></message>
    <message><source>Missing path argument</source><translation>Eksik bağımsız değişken: Yol</translation></message>
    <message><source>Missing registry key argument</source><translation>Eksik bağımsız değişken: Kayıt defteri · Anahtar</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Komutta kapatılmamış çift tırnak</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Beklenmeyen ek bağımsız değişken: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Desteklenmeyen uwfmgr komutu (arayüz işlemiyle eşleştirilemiyor)</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>Bekleyen değişiklikler: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Zaten hedef durumda; işlem gerekmiyor</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Yol bu birimde değil veya birim dosya dışlamalarını desteklemiyor (ör. exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>UWF engel listesi reddetti (sistem dosyası / Windows / disk belleği vb.)</translation></message>
    <message><source>Pending filter %1</source><translation>Bekleyen değişiklikler: Filtre %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>Filtre: Zaten hedef durumda; işlem gerekmiyor</translation></message>
    <message><source>Pending overlay type → %1</source><translation>Bekleyen değişiklikler: Yer paylaşımı · Tür → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>Yer paylaşımı · Tür: %1 (Zaten hedef durumda; işlem gerekmiyor)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Geçersiz değer: Boyut: %1</translation></message>
    <message><source>maximum size</source><translation>En büyük boyut</translation></message>
    <message><source>warning threshold</source><translation>Uyarı eşiği</translation></message>
    <message><source>critical threshold</source><translation>Kritik eşik</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>Bekleyen değişiklikler: Yer paylaşımı %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>Yer paylaşımı %1: %2 MB (Zaten hedef durumda; işlem gerekmiyor)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Bilinmeyen birim %1 (bu harfte UWF’ye uygun disk yok)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>Bekleyen değişiklikler: Birim %1 · UWF koruması: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Birim %1: Zaten hedef durumda; işlem gerekmiyor</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Yolun sürücü harfi yok. Hedef birimi belirleme: Başarısız.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Birim %1: UWF Desteklenmiyor</translation></message>
    <message><source>file exclusion</source><translation>Dosya dışlamaları</translation></message>
    <message><source>registry exclusion</source><translation>Kayıt defteri dışlamaları</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Kayıt defteri sekmesi bulunmadığından kayıt defteri dışlamaları kullanılamıyor</translation></message>
    <message><source>Unsupported command</source><translation>Komut: Desteklenmiyor</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Göster veya gizle.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Etkin</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Devre dışı</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Bilinmiyor</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Kullanılan %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Gelişmiş mod</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Otomatik kaydı Windows kapatma ve yeniden başlatmayla eşgüdümler.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Başarısız: Gelişmiş mod durumunu okuma (Bilinmeyen hata).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Başarısız: Hizmet durumunu okuma:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Başarısız: Hizmet durumunu okuma (Bilinmeyen hata).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Hizmet bağımsız işlem olarak yapılandırılmamış.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Hizmet otomatik başlatılacak şekilde ayarlanmamış.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Hizmet LocalSystem olarak çalışacak şekilde ayarlanmamış.</translation></message>
    <message><source>The service is not running.</source><translation>Hizmet çalışmıyor.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Hizmet dosya yolu veya başlangıç bağımsız değişkenleri bu programla eşleşmiyor.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Hizmetin kapatma öncesi zaman aşımı yanlış ayarlanmış.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Hizmet, arayüz aracısını başlatmak için gereken yetkileri bildirmiyor.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Hizmet Windows kapatma öncesi bildirimlerini kabul etmedi.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Gelişmiş modu etkinleştir</translation></message>
    <message><source>Disable enhanced mode</source><translation>Gelişmiş modu devre dışı bırak</translation></message>
    <message><source>Start service</source><translation>Hizmeti başlat</translation></message>
    <message><source>Repair enhanced mode</source><translation>Gelişmiş modu onar</translation></message>
    <message><source>Delete service</source><translation>Hizmeti sil</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Gelişmiş mod hizmetini durdurup kalan kayıt defteri verileriyle birlikte siler.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Hizmet yok ancak kayıt defteri verileri kalmış.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Arayüz aracısının kimlik doğrulaması bekleniyor.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Hizmet çalışıyor ancak kimliği doğrulanmış bir arayüz aracısı bağlı değil.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>UWF Manager gelişmiş mod yardımcı hizmeti</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Gelişmiş mod · Yapılandırma: Uygulandı.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Yapılandırma: Uygulandı. UWF kalıcılık raporu:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Gelişmiş mod · Yapılandırma: Başarısız:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Gelişmiş mod · Yapılandırma: Başarısız (Bilinmeyen hata).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Gelişmiş mod hizmeti ve kalan yapılandırma silindi.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Hizmet silme istendi ancak kalan verileri temizleme şu bildirimi verdi:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Gelişmiş mod: Hizmeti sil: Başarısız:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Gelişmiş mod: Hizmeti sil: Başarısız (Bilinmeyen hata).</translation></message>
    <message><source>File staging completed</source><translation>Otomatik dosya kaydı: Başarılı</translation></message>
    <message><source>File staging failed</source><translation>Otomatik dosya kaydı: Başarısız</translation></message>
    <message><source>All staged targets were processed.</source><translation>Hazırlanan tüm hedefler işlendi.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Tüm hedefler işlendi; bir veya daha fazla işlem başarısız oldu.</translation></message>
    <message><source>Show main window</source><translation>Ana pencereyi göster</translation></message>
    <message><source>Restore default position</source><translation>Varsayılan konumu geri yükle</translation></message>
    <message><source>Show overlay hub</source><translation>Yer paylaşımı HUB’ını göster</translation></message>
    <message><source>Hide overlay hub</source><translation>Yer paylaşımı HUB’ını gizle</translation></message>
    <message><source>Exit application</source><translation>Uygulamadan çık</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Başarısız: Sisteme bağlanma</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Başarısız: Kayıt defteri değerlerini okuma (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Yer paylaşımı dosyalarını listeleme: Bilinmeyen hata.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Başarısız: Filtre işlemi: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Başarısız: Filtre durumunu okuma</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Başarısız: Uyarı eşiğini ayarlama</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Başarısız: Kritik eşiği ayarlama</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Başarısız: Yer paylaşımı durumunu okuma</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Başarısız: Yer paylaşımı türünü ayarlama</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Başarısız: En büyük boyutu ayarlama</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Başarısız: Yer paylaşımı yapılandırmasını okuma</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Başarısız: %1 birimini UWF’ye kaydetme</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Başarısız: %2 birim koruması: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Başarısız: %1 biriminin bağlamasını ayarlama</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Birim %1 · Başarısız: Dosya dışlaması ekleme %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ %1 biriminde dosya dışlaması zaten yok: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Birim %1 · Başarısız: Dosya dışlamasını kaldırma %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Başarısız: Birim yapılandırmasını okuma</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Başarısız: Kayıt defteri dışlaması ekleme %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Başarısız: Kayıt defteri dışlamasını kaldırma %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Başarısız: Kayıt defteri kalıcılık anahtarlarını güncelleme</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Başarısız: Kayıt defteri filtresini okuma</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Desteklenmiyor</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Hedef durumunu okuma: Bilinmeyen hata.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Yol yok; silinecek bir şey yok.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Bilinmeyen hata.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Hedef durumu yeniden okunamadığından sonuç doğrulanamadı: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Sağlayıcı silmeyi kabul etti ancak yetkili kaynaktan yeniden okumada hedef hâlâ mevcut.</translation></message>
    <message><source>Unknown</source><translation>Bilinmiyor</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Başarısız: Günlüğü yükleme (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Başarısız: Günlüğü yükleme (Bilinmeyen hata).</translation></message>
    <message><source>: unknown error</source><translation>: Bilinmeyen hata</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Başarısız: Hedef birimi belirleme (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Başarısız: Kayıt defteri anahtarlarını listeleme (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Başarısız: Hedef birimi belirleme (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Önkoşul olan bir işlem başarısız olduğu veya doğrulanamadığı için bu işlem atlandı.</translation></message>
    <message><source>Copy</source><translation>Kopyala</translation></message>
    <message><source>Select All</source><translation>Tümünü Seç</translation></message>
    <message><source>Display language</source><translation>Görüntüleme dili</translation></message>
    <message><source>Folder</source><translation>Klasör</translation></message>
    <message><source>File</source><translation>Dosya</translation></message>
    <message><source>Export commands</source><translation>Komutları dışa aktar</translation></message>
    <message><source>Direct restart…</source><translation>Doğrudan yeniden başlat…</translation></message>
    <message><source>Clear…</source><translation>Temizle…</translation></message>
    <message><source>Committing</source><translation>Kalıcı kaydediliyor</translation></message>
    <message><source> · %1 selected</source><translation> · %1 seçildi</translation></message>
    <message><source>%1 changes</source><translation>%1 değişiklik</translation></message>
    <message><source>%1 commands</source><translation>%1 komut</translation></message>
    <message><source>%1 items</source><translation>%1 öğe</translation></message>
    <message><source>%1 items · %2</source><translation>%1 öğe · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 bekleyen değişiklik</translation></message>
    <message><source>%1 records</source><translation>%1 kayıt</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 kayıt · toplam %2</translation></message>


    <message><source>%1 selected</source><translation>%1 seçildi</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 dosya</translation></message>
    <message><source>32-bit</source><translation>32 bit</translation></message>
    <message><source>64-bit</source><translation>64 bit</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt; kapsamında yayımlanmıştır. Koşulların tamamı uygulamayla birlikte gelen LICENSE dosyasındadır.&lt;/p&gt;&lt;p&gt;Yazılımı GPL v3 kapsamında yeniden dağıtabilir veya değiştirebilirsiniz. Yazılım olduğu gibi, hiçbir garanti olmadan sunulur.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows disk belleği dosyalarını, Sistem Geri Yükleme’yi, SuperFetch’i, dosya dizinlemeyi, disk birleştirmeyi ve hızlı başlatmayı devre dışı bırakır. Ayrıca BCD içindeki bootstatuspolicy değerini ignoreallfailures olarak ayarlar.&lt;/p&gt;&lt;p&gt;Bu ayarları sonradan değiştirebilirsiniz. Örneğin disk belleğini yeniden etkinleştirmeden önce disk belleği dosyasını korunmayan bir birime taşıyın.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Sistemin, ayrıcalıkların ve UWF’nin tanılama anlık görüntüsü.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>Bu konumda bir dosya zaten var:
%1

Kaydetmek dosyanın içeriğini değiştirecek.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Windows Unified Write Filter için grafik yönetim aracı.
Birim korumasını, yer paylaşımını, dışlamaları ve hazırlanmış dosyaları yönetin.</translation></message>
    <message><source>About this snapshot</source><translation>Bu anlık görüntü hakkında</translation></message>
    <message><source>Account</source><translation>Hesap</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Bir betik veya metinden UWF yapılandırması ekleyin.</translation></message>
    <message><source>Add exclusion</source><translation>Dışlama ekle</translation></message>
    <message><source>Add files or folders</source><translation>Dosya veya klasör ekle</translation></message>
    <message><source>Add selected items</source><translation>Seçili öğeleri ekle</translation></message>
    <message><source>Added</source><translation>Eklendi</translation></message>
    <message><source>Administrator</source><translation>Yönetici</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Tüm değişiklikler diske kaydedildi ve yeniden başlatmadan korunacak.</translation></message>
    <message><source>All files (*)</source><translation>Tüm dosyalar (*)</translation></message>
    <message><source>All levels</source><translation>Tüm düzeyler</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Tanılama anlık görüntüsünün boş olması, yer paylaşımında değişiklik olmadığı anlamına gelmez.</translation></message>
    <message><source>Application path</source><translation>Uygulama yolu</translation></message>
    <message><source>Application result</source><translation>Uygulama sonucu</translation></message>
    <message><source>Apply changes</source><translation>Değişiklikleri uygula</translation></message>
    <message><source>Architecture</source><translation>Mimari</translation></message>
    <message><source>Automatic</source><translation>Otomatik</translation></message>
    <message><source>Automatic file staging</source><translation>Hazırlanmış dosyaları otomatik kaydetme</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Otomatik kaydetme tamamlanmadı</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Normal kapatma ve yeniden başlatma sırasında uygun hazırlanmış dosyaları otomatik kaydet.</translation></message>
    <message><source>Available</source><translation>Kullanılabilir</translation></message>
    <message><source>Back</source><translation>Geri</translation></message>
    <message><source>Background service</source><translation>Arka plan hizmeti</translation></message>
    <message><source>Binding</source><translation>Bağlama</translation></message>
    <message><source>By drive letter</source><translation>Sürücü harfine göre</translation></message>
    <message><source>By volume ID</source><translation>Birim kimliğine göre</translation></message>
    <message><source>Cancel reading</source><translation>Okumayı iptal et</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Önceden kalıcılaştırılan değişiklikler korunur.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Alt anahtarları ve değerleriyle birlikte UWF korumasından hariç tutulacak kayıt defteri anahtarını seçin.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Dosyanın kaydedileceği konumu seçin.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>İzin verilen sürücü veya klasör içinde bir konum seçin.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Başka bir klasör seçin veya erişim izinlerini denetleyip yeniden deneyin.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Başka bir anahtar seçin veya erişim izinlerini denetleyip yeniden deneyin.</translation></message>
    <message><source>Clear filters</source><translation>Filtreleri temizle</translation></message>
    <message><source>Clear logs</source><translation>Günlükleri temizle</translation></message>
    <message><source>Clear logs?</source><translation>Günlükler temizlensin mi?</translation></message>
    <message><source>Collapse</source><translation>Daralt</translation></message>
    <message><source>Commit changes</source><translation>Değişiklikleri kalıcılaştır</translation></message>
    <message><source>Commit changes…</source><translation>Değişiklikleri kalıcılaştır…</translation></message>
    <message><source>Commit complete</source><translation>Kalıcılaştırma tamamlandı</translation></message>
    <message><source>Commit file changes?</source><translation>Dosya değişiklikleri kalıcılaştırılsın mı?</translation></message>
    <message><source>Commit folder changes?</source><translation>Klasör değişiklikleri kalıcılaştırılsın mı?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Kayıt defteri anahtarı değişiklikleri kalıcılaştırılsın mı?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Kayıt defteri değeri değişiklikleri kalıcılaştırılsın mı?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Normal kapatma ve yeniden başlatmadan önce hazırlanmış dosyaları kalıcılaştırır</translation></message>
    <message><source>Compatibility mode</source><translation>Uyumluluk modu</translation></message>
    <message><source>Configuration applied</source><translation>Yapılandırma uygulandı</translation></message>
    <message><source>Configuration error</source><translation>Yapılandırma hatası</translation></message>
    <message><source>Configured</source><translation>Yapılandırıldı</translation></message>
    <message><source>Confirm before continuing</source><translation>Devam etmeden önce onaylayın</translation></message>
    <message><source>Connected and authenticated</source><translation>Bağlandı ve kimlik doğrulandı</translation></message>
    <message><source>Connecting…</source><translation>Bağlanıyor…</translation></message>
    <message><source>Contact author</source><translation>Geliştiriciyle iletişim</translation></message>
    <message><source>Continue editing</source><translation>Düzenlemeye devam et</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Yeniden başlatmaya devam etmek, kaydedilemeyen dosyalardaki değişiklikleri silecektir.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Kapatmaya devam etmek, kaydedilemeyen dosyalardaki değişiklikleri silecektir.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Windows kapatmasıyla eşgüdümle</translation></message>
    <message><source>Copy all logs</source><translation>Tüm günlükleri kopyala</translation></message>
    <message><source>Copy full report</source><translation>Raporun tamamını kopyala</translation></message>
    <message><source>Copy name</source><translation>Adı kopyala</translation></message>
    <message><source>Copy path</source><translation>Yolu kopyala</translation></message>
    <message><source>Copy selection</source><translation>Seçimi kopyala</translation></message>
    <message><source>Copy version</source><translation>Sürümü kopyala</translation></message>
    <message><source>Counting staged files…</source><translation>Hazırlanmış dosyalar sayılıyor…</translation></message>
    <message><source>Create</source><translation>Oluştur</translation></message>
    <message><source>Critical %1 MB</source><translation>Kritik %1 MB</translation></message>
    <message><source>Current filter</source><translation>Geçerli filtre</translation></message>
    <message><source>Current run</source><translation>Geçerli çalıştırma</translation></message>
    <message><source>Current selection</source><translation>Geçerli seçim</translation></message>
    <message><source>Current usage</source><translation>Geçerli kullanım</translation></message>
    <message><source>Date modified</source><translation>Değiştirilme tarihi</translation></message>
    <message><source>Debug</source><translation>Hata ayıklama</translation></message>
    <message><source>Delete and commit this file?</source><translation>Bu dosya silinsin ve silme işlemi kalıcılaştırılsın mı?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Bu klasör silinsin ve silme işlemi kalıcılaştırılsın mı?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Bu kayıt defteri anahtarı silinsin ve silme işlemi kalıcılaştırılsın mı?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Bu kayıt defteri değeri silinsin ve silme işlemi kalıcılaştırılsın mı?</translation></message>

    <message><source>Direct restart?</source><translation>Doğrudan yeniden başlatılsın mı?</translation></message>
    <message><source>Direct shutdown?</source><translation>Doğrudan kapatılsın mı?</translation></message>
    <message><source>Discard changes</source><translation>Değişiklikleri at</translation></message>
    <message><source>Disk</source><translation>Disk</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Etki alanı gizli anahtarını kalıcı tutma</translation></message>
    <message><source>Done</source><translation>Bitti</translation></message>
    <message><source>Edit commands</source><translation>Komutları düzenle</translation></message>
    <message><source>Editable</source><translation>Düzenlenebilir</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Canlı kullanımı görmek için filtreyi etkinleştirin ve yeniden başlatın</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>Gelişmiş mod, Windows ile birlikte başlayan bir arka plan hizmeti ekler. Uygulamayı taşımadan veya silmeden önce hizmeti kaldırın.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>Gelişmiş mod kullanılamıyor. Hazırlanmış dosyaları kaydetmek için Güvenli kapat veya Güvenli yeniden başlat seçeneğini kullanın.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>HKLM\SOFTWARE\… girin ve gitmek için Enter’a basın</translation></message>
    <message><source>Enter a file name</source><translation>Dosya adı girin</translation></message>
    <message><source>Enter a valid file name.</source><translation>Geçerli bir dosya adı girin.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Geçerli bir dosya veya klasör adı girin.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Geçerli bir klasör adı girin.</translation></message>
    <message><source>Error</source><translation>Hata</translation></message>
    <message><source>Error details</source><translation>Hata ayrıntıları</translation></message>
    <message><source>Error details and suggestions</source><translation>Hata ayrıntıları ve öneriler</translation></message>
    <message><source>Expand</source><translation>Genişlet</translation></message>
    <message><source>Export list</source><translation>Listeyi dışa aktar</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Günlükler temizlenemedi: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Günlükler temizlenemedi: beklenmeyen hata.</translation></message>
    <message><source>File name</source><translation>Dosya adı</translation></message>
    <message><source>File or folder</source><translation>Dosya veya klasör</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>Dosyaları kaydetme başladı ve iptal edilemez.</translation></message>
    <message><source>File saving incomplete</source><translation>Dosyaları kaydetme tamamlanmadı</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Hazırlanmış dosyaları kaydetme atlanacak. Korunan yollardaki kalıcılaştırılmamış değişiklikler yeniden başlatmadan sonra kaybolacak.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Hazırlanmış dosyaları kaydetme atlanacak. Korunan yollardaki kalıcılaştırılmamış değişiklikler kapatmadan sonra kaybolacak.</translation></message>
    <message><source>File type</source><translation>Dosya türü</translation></message>
    <message><source>Files and folders</source><translation>Dosyalar ve klasörler</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Dosyalar ve klasörler birlikte seçilebilir.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Hazırlanmış dosyalar kaydedilmeyecek.</translation></message>
    <message><source>Filter is disabled</source><translation>Filtre devre dışı</translation></message>
    <message><source>Fit columns to contents</source><translation>Sütunları içeriğe sığdır</translation></message>
    <message><source>Folder address</source><translation>Klasör adresi</translation></message>
    <message><source>Folder name</source><translation>Klasör adı</translation></message>
    <message><source>Folders</source><translation>Klasörler</translation></message>
    <message><source>Full configuration</source><translation>Tam yapılandırma</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Bu anahtara sonraki yazmalar UWF tarafından korunmayacak.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Bu yola sonraki yazmalar UWF tarafından korunmayacak.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Genel kayıt defteri dışlaması · alt anahtarlar dahil</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Bir üst klasöre gidin veya başka bir konum seçin.</translation></message>
    <message><source>Home</source><translation>Giriş klasörü</translation></message>
    <message><source>Import configuration</source><translation>Yapılandırmayı içe aktar</translation></message>
    <message><source>Import creates pending changes. Use Review and apply to write them to the system.</source><translation>İçe aktarma bekleyen değişiklikler oluşturur. Bunları sisteme yazmak için İncele ve uygula seçeneğini kullanın.</translation></message>
    <message><source>Import results</source><translation>İçe aktarma sonuçları</translation></message>
    <message><source>Imported</source><translation>İçe aktarıldı</translation></message>
    <message><source>Imported commands still need to be applied. Select a row to view the complete result.</source><translation>İçe aktarılan komutların hâlâ uygulanması gerekiyor. Sonucun tamamını görmek için bir satır seçin.</translation></message>
    <message><source>Imported: %1</source><translation>İçe aktarıldı: %1</translation></message>
    <message><source>Information</source><translation>Bilgi</translation></message>
    <message><source>Item details</source><translation>Öğe ayrıntıları</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Büyük hazırlanmış dosya listelerini kaydetmek daha uzun sürebilir.</translation></message>
    <message><source>License and copyright</source><translation>Lisans ve telif hakkı</translation></message>
    <message><source>Line</source><translation>Satır</translation></message>
    <message><source>Local disk (%1)</source><translation>Yerel disk (%1)</translation></message>
    <message><source>Log level</source><translation>Günlük düzeyi</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Günlükler okunamadı. Yeniden deneyin veya günlük dosyasına erişimi denetleyin.</translation></message>
    <message><source>Maximize</source><translation>Ekranı kapla</translation></message>
    <message><source>Message details</source><translation>İleti ayrıntıları</translation></message>
    <message><source>Minimize</source><translation>Simge durumuna küçült</translation></message>
    <message><source>Module</source><translation>Modül</translation></message>
    <message><source>More</source><translation>Diğer</translation></message>
    <message><source>Move</source><translation>Taşı</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Uygulamanın bulunduğu sürücü UWF ile korunuyorsa, önce uygulama dosyalarının yeniden başlatmadan sonra da kalacağından emin olun. Çok sayıda hazırlanmış dosyayı kalıcı olarak kaydetmek, kapatmayı veya yeniden başlatmayı uzatabilir.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Birden fazla dosya seçilebilir.</translation></message>
    <message><source>Name and location</source><translation>Ad ve konum</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Adlandırılmış değer: %1 · yalnızca bu değer</translation></message>
    <message><source>Navigate to registry path</source><translation>Kayıt defteri yoluna git</translation></message>
    <message><source>New folder</source><translation>Yeni klasör</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Yeniledikten sonra yeni kayıtlar görünecek.</translation></message>
    <message><source>Next-session configuration</source><translation>Sonraki oturum yapılandırması</translation></message>
    <message><source>No files to commit</source><translation>Kalıcılaştırılacak dosya yok</translation></message>
    <message><source>No logs yet</source><translation>Henüz günlük yok</translation></message>
    <message><source>No matching items</source><translation>Eşleşen öğe yok</translation></message>
    <message><source>No matching logs</source><translation>Eşleşen günlük yok</translation></message>
    <message><source>No overlay files found</source><translation>Yer paylaşımı dosyası bulunamadı</translation></message>
    <message><source>No registry key selected</source><translation>Kayıt defteri anahtarı seçilmedi</translation></message>
    <message><source>None</source><translation>Yok</translation></message>
    <message><source>Not configured</source><translation>Yapılandırılmadı</translation></message>
    <message><source>Not connected</source><translation>Bağlı değil</translation></message>
    <message><source>Not installed</source><translation>Yüklü değil</translation></message>
    <message><source>Not written to the system</source><translation>Sisteme yazılmadı</translation></message>
    <message><source>Off</source><translation>Kapalı</translation></message>
    <message><source>On</source><translation>Açık</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Her satırda bir uwfmgr komutu</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Uygulamayı standart kullanıcıların değiştiremeyeceği sabit bir klasörde tutun. Önerilen konum:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Yalnızca hazırlanmış dosyalar otomatik kaydedilir. Korunan yollardaki diğer kalıcılaştırılmamış değişiklikler yeniden başlatmadan sonra kaybolacak.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Yalnızca hazırlanmış dosyalar otomatik kaydedilir. Korunan yollardaki diğer kalıcılaştırılmamış değişiklikler kapatmadan sonra kaybolacak.</translation></message>
    <message><source>Open</source><translation>Aç</translation></message>
    <message><source>Open folder</source><translation>Klasörü aç</translation></message>
    <message><source>Operating system</source><translation>İşletim sistemi</translation></message>
    <message><source>Operation and security</source><translation>İşleyiş ve güvenlik</translation></message>
    <message><source>Original command</source><translation>Özgün komut</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Yer paylaşımı kritik eşiğine ulaşıldı. Değişiklikleri kaydedip güvenli yeniden başlatın.</translation></message>
    <message><source>Overlay maximum size</source><translation>Yer paylaşımı en büyük boyutu</translation></message>
    <message><source>Overlay type</source><translation>Yer paylaşımı türü</translation></message>
    <message><source>Overlay usage</source><translation>Yer paylaşımı kullanımı</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Yer paylaşımı uyarı eşiğine ulaşıldı. Değişiklikleri kaydedip güvenli yeniden başlatın.</translation></message>
    <message><source>Overview</source><translation>Genel bakış</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Komutları yapıştırın veya dosyadan yükleyin…</translation></message>
    <message><source>Pending changes</source><translation>Bekleyen değişiklikler</translation></message>
    <message><source>Pending removal</source><translation>Kaldırılmayı bekliyor</translation></message>
    <message><source>Permissions and protection</source><translation>İzinler ve koruma</translation></message>
    <message><source>Privileges</source><translation>Ayrıcalıklar</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2 öğe işlendi</translation></message>
    <message><source>Processing result</source><translation>İşleme sonucu</translation></message>
    <message><source>Protected</source><translation>Korunuyor</translation></message>
    <message><source>Protected volumes</source><translation>Korunan birimler</translation></message>
    <message><source>Protection and overlay</source><translation>Koruma ve yer paylaşımı</translation></message>
    <message><source>Quick access</source><translation>Hızlı erişim</translation></message>
    <message><source>Raw report</source><translation>Ham rapor</translation></message>
    <message><source>Read again</source><translation>Yeniden oku</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Tanılama anlık görüntüsü almak için yeniden okuyun.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Canlı kullanımı görmek için yapılandırmayı okuyun</translation></message>
    <message><source>Read-only</source><translation>Salt okunur</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Büyük bir yer paylaşımını okumak daha uzun sürebilir. İstediğiniz zaman iptal edebilirsiniz.</translation></message>
    <message><source>Reading canceled</source><translation>Okuma iptal edildi</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Bu konumdaki dosya ve alt klasörler okunuyor…</translation></message>
    <message><source>Reading folder</source><translation>Klasör okunuyor</translation></message>
    <message><source>Reading logs</source><translation>Günlükler okunuyor</translation></message>
    <message><source>Reading overlay</source><translation>Yer paylaşımı okunuyor</translation></message>
    <message><source>Ready</source><translation>Hazır</translation></message>
    <message><source>Ready to save</source><translation>Kaydetmeye hazır</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Okuma tamamlandığında geçerli çalıştırmanın kayıtları burada görünecek.</translation></message>
    <message><source>Recursive staging</source><translation>Özyinelemeli hazırlama</translation></message>
    <message><source>Refresh folder</source><translation>Klasörü yenile</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Kayıt defteri anahtarı veya kalıcılık ayarı</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Geçerli hedef bittikten sonra kalan görevler iptal edilebilir.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Bu anahtarı sonraki oturum dışlamalarından kaldır.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Bu yolu sonraki oturum dışlamalarından kaldır.</translation></message>
    <message><source>Removed</source><translation>Kaldırıldı</translation></message>
    <message><source>Rename</source><translation>Yeniden adlandır</translation></message>
    <message><source>Rename…</source><translation>Yeniden adlandır…</translation></message>
    <message><source>Repair required</source><translation>Onarım gerekli</translation></message>
    <message><source>Replace file</source><translation>Dosyayı değiştir</translation></message>
    <message><source>Replace the existing file?</source><translation>Var olan dosya değiştirilsin mi?</translation></message>
    <message><source>Required configuration</source><translation>Gerekli yapılandırma</translation></message>
    <message><source>Restart anyway</source><translation>Yine de yeniden başlat</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Kaydettikten sonra yeniden başlatma devam edecek. Uygulamayı açık tutun.</translation></message>
    <message><source>Restore</source><translation>Geri yükle</translation></message>
    <message><source>Retry</source><translation>Yeniden dene</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Kapatmaya devam etmeden önce hata ayrıntılarını inceleyin.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Yeniden başlatmadan önce hata ayrıntılarını inceleyin.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>İşlem ayrıntılarını inceleyin. Yazılmış olabilecek değişiklikleri yeniden denemeden önce geçerli sistem durumunu okuyun.</translation></message>
    <message><source>Review the result for each target.</source><translation>Her hedefin sonucunu inceleyin.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Bu değişiklikleri sisteme yazmadan önce inceleyin.</translation></message>
    <message><source>Running</source><translation>Çalışıyor</translation></message>
    <message><source>Save</source><translation>Kaydet</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Hazırlanmış dosyaları diske kaydet ve yeniden başlat.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Hazırlanmış dosyaları diske kaydet ve kapat.</translation></message>
    <message><source>Saving staged files</source><translation>Hazırlanmış dosyalar kaydediliyor</translation></message>
    <message><source>Saving to disk</source><translation>Diske kaydediliyor</translation></message>
    <message><source>Saving…</source><translation>Kaydediliyor…</translation></message>
    <message><source>Scanning staged paths</source><translation>Hazırlanmış yollar taranıyor</translation></message>
    <message><source>Search logs</source><translation>Günlüklerde ara</translation></message>
    <message><source>Search messages or modules</source><translation>İleti veya modüllerde ara</translation></message>
    <message><source>Search paths</source><translation>Yollarda ara</translation></message>
    <message><source>Search this folder</source><translation>Bu klasörde ara</translation></message>
    <message><source>Select a file or enter its name</source><translation>Bir dosya seçin veya adını girin</translation></message>
    <message><source>Select a file to open.</source><translation>Açılacak bir dosya seçin.</translation></message>
    <message><source>Select a folder.</source><translation>Bir klasör seçin.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Alt ağacının tamamında işlem yapmak için anahtar, yalnızca ilgili değerde işlem yapmak için adlandırılmış bir değer seçin.</translation></message>
    <message><source>Select an available subkey</source><translation>Kullanılabilir bir alt anahtar seç</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Yalnızca dosyaları seçin veya içeriğine göz atmak için bir klasör açın.</translation></message>
    <message><source>Select files or folders</source><translation>Dosya veya klasör seç</translation></message>
    <message><source>Select folder</source><translation>Klasör seç</translation></message>
    <message><source>Select folders only.</source><translation>Yalnızca klasörleri seçin.</translation></message>
    <message><source>Select registry key or value</source><translation>Kayıt defteri anahtarı veya değeri seç</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Silinecek kayıt defteri anahtarı veya değeri seç</translation></message>
    <message><source>Select the current folder</source><translation>Geçerli klasörü seç</translation></message>
    <message><source>Select this file</source><translation>Bu dosyayı seç</translation></message>
    <message><source>Select this registry key</source><translation>Bu kayıt defteri anahtarını seç</translation></message>
    <message><source>Select this registry value</source><translation>Bu kayıt defteri değerini seç</translation></message>
    <message><source>Selected file</source><translation>Seçili dosya</translation></message>
    <message><source>Selected folder</source><translation>Seçili klasör</translation></message>
    <message><source>Selection</source><translation>Seçim</translation></message>
    <message><source>Service account</source><translation>Hizmet hesabı</translation></message>
    <message><source>Service configuration needs repair</source><translation>Hizmet yapılandırımı onarılmalı</translation></message>
    <message><source>Service details</source><translation>Hizmet ayrıntıları</translation></message>
    <message><source>Service name</source><translation>Hizmet adı</translation></message>
    <message><source>Service status</source><translation>Hizmet durumu</translation></message>
    <message><source>Service stopped</source><translation>Hizmet durduruldu</translation></message>
    <message><source>Shut down anyway</source><translation>Yine de kapat</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>Kaydettikten sonra kapatma devam edecek. Uygulamayı açık tutun.</translation></message>
    <message><source>Size</source><translation>Boyut</translation></message>
    <message><source>Some changes could not be applied</source><translation>Bazı değişiklikler uygulanamadı</translation></message>
    <message><source>Some files could not be saved</source><translation>Bazı dosyalar kaydedilemedi</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Bazı donanım bilgileri okunamadı. Diğer tanılama bilgileri hâlâ görüntülenip kopyalanabilir.</translation></message>
    <message><source>Source code</source><translation>Kaynak kodu</translation></message>
    <message><source>Staged file</source><translation>Hazırlanmış dosya</translation></message>
    <message><source>Staged path</source><translation>Hazırlanmış yol</translation></message>
    <message><source>Standard user</source><translation>Standart kullanıcı</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Windows ile otomatik başlar</translation></message>
    <message><source>Startup type</source><translation>Başlangıç türü</translation></message>
    <message><source>Status unavailable</source><translation>Durum kullanılamıyor</translation></message>
    <message><source>Stopped</source><translation>Durduruldu</translation></message>
    <message><source>Storage</source><translation>Depolama</translation></message>
    <message><source>System</source><translation>Sistem</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>UWF ilk etkinleştirildiğinde yapılan sistem değişiklikleri</translation></message>
    <message><source>System memory</source><translation>Sistem belleği</translation></message>
    <message><source>System metadata</source><translation>Sistem meta verileri</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Sistem meta verileri yalnızca tanılama içindir ve kalıcılaştırılamaz.</translation></message>
    <message><source>Takes effect after restart</source><translation>Yeniden başlattıktan sonra geçerli</translation></message>
    <message><source>Target</source><translation>Hedef</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Terminal Services lisanslarını kalıcı tutma</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>UWF sağlayıcısı listelemeyi tamamlayamadı. Büyük bir yer paylaşımında veya yetersiz sistem kaynaklarında bu durum oluşabilir.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>Yapılandırma sisteme yazıldı. Çoğu değişiklik yeniden başlattıktan sonra geçerli olur.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>Yapılandırma sonraki oturum hedefiyle eşleşiyor. Tam yapılandırma bölümünde komutları görüntüleyin veya dışa aktarın.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Geçerli günlük kayıtları silinecek ve geri getirilemeyecek.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Hedef klasör yok.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Gelişmiş mod hizmet durumu okunamadı. Hizmet izinlerini denetleyip yeniden deneyin.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>Liste bazı dosyaları atlayabilir veya önceden kalıcılaştırılan dosyaları içerebilir. Kalıcılaştırma listesi olarak kullanmayın.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Seçili öğe artık yok: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Hizmet yüklü ancak durdurulmuş. Otomatik kaydetmeyi geri getirmek için başlatın.</translation></message>
    <message><source>This PC</source><translation>Bu bilgisayar</translation></message>
    <message><source>This folder is empty</source><translation>Bu klasör boş</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Bu rapor hesap adları, cihaz tanımlayıcıları ve dosya yolları içerebilir. Paylaşmadan önce inceleyin.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Bu UTF-8 CMD betiğini .cmd dosyası olarak kaydedin ve Komut İstemi’nde çalıştırın. Dışa aktarma aynı betiği kullanır.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Başka bir anahtar sözcük veya günlük düzeyi deneyin.</translation></message>
    <message><source>Try another search term.</source><translation>Başka bir arama terimi deneyin.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Filtre etkin olduğu sürece tür ve en büyük boyut değiştirilemez. Önce filtreyi devre dışı bırakıp yeniden başlatın.</translation></message>
    <message><source>UI agent</source><translation>Arayüz aracısı</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 satır</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>UWF yapılandırması okunamadı. Unified Write Filter’ın Windows’ta yüklü olduğunu doğrulayın ve yeniden okuyun.</translation></message>
    <message><source>UWF feature</source><translation>UWF özelliği</translation></message>
    <message><source>UWF filter</source><translation>UWF filtresi</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF şu anda kullanılamıyor</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>UWF güvenli yeniden başlatma doğrudan çalışacak.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>UWF güvenli kapatma doğrudan çalışacak.</translation></message>
    <message><source>Unable to access this folder</source><translation>Bu klasöre erişilemiyor</translation></message>
    <message><source>Unable to access this registry key</source><translation>Bu kayıt defteri anahtarına erişilemiyor</translation></message>
    <message><source>Unable to read logs</source><translation>Günlükler okunamıyor</translation></message>
    <message><source>Unable to read service status</source><translation>Hizmet durumu okunamıyor</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Yer paylaşımı şu anda okunamıyor</translation></message>
    <message><source>Unable to read this folder.</source><translation>Bu klasör okunamıyor.</translation></message>
    <message><source>Unable to select this item</source><translation>Bu öğe seçilemiyor</translation></message>
    <message><source>Unavailable</source><translation>Kullanılamıyor</translation></message>
    <message><source>Undo removal</source><translation>Kaldırmayı geri al</translation></message>
    <message><source>Unprotected</source><translation>Korunmuyor</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Başarısız öğeler kaydedilmedi. Tam yolu ve nedeni görmek için bir satır seçin.</translation></message>
    <message><source>Up one folder</source><translation>Bir üst klasöre git</translation></message>
    <message><source>Usage unavailable</source><translation>Kullanım kullanılamıyor</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Tüm alt anahtarlar dahil anahtarın tamamını kullan</translation></message>
    <message><source>Version %1</source><translation>Sürüm %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Uygulama kayıtlarını ve tanılama bilgilerini görüntüleyin.</translation></message>
    <message><source>View commands for these changes</source><translation>Bu değişikliklerin komutlarını görüntüle</translation></message>
    <message><source>View failure details</source><translation>Hata ayrıntılarını görüntüle</translation></message>
    <message><source>View overlay</source><translation>Yer paylaşımını görüntüle</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>%1 için dosya tanılama anlık görüntüsünü görüntüleyin.</translation></message>
    <message><source>Volume %1 binding</source><translation>%1 birimini bağlama</translation></message>
    <message><source>Volume %1 protection</source><translation>%1 birimini koruma</translation></message>
    <message><source>Volume identity</source><translation>Birim kimliği</translation></message>
    <message><source>Warning</source><translation>Uyarı</translation></message>
    <message><source>Warning %1 MB</source><translation>Uyarı %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Anahtarın tamamı ve tüm alt anahtarlar · özyinelemeli</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Yazma korumasını Windows UWF sağlar. Bu uygulama korumayı yapılandırır ve yönetir.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Gelişmiş mod olmadan hazırlanmış dosyaları kaydetmek için bu uygulamadaki Güvenli kapat veya Güvenli yeniden başlat seçeneğini kullanın.</translation></message>
    <message><source>Write protection</source><translation>Yazma koruması</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Bu yapılandırma değişiklikleri sisteme yazılsın mı? Çoğu değişiklik yeniden başlattıktan sonra geçerli olur.</translation></message>
    <message><source>staged files</source><translation>hazırlanmış dosyalar</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr komutları</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Seçili yolları kaldırılmak üzere işaretleyin. Uygulandıktan sonra geçerli olur.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Uygulandıktan sonra otomatik dosya hazırlama listesinden kaldır.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Uygulandıktan sonra otomatik dosya hazırlama listesine ekle.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Bu plan uygulandığında kayıtlı dosya hazırlama listesi temizlenir ve artık gerekmiyorsa ilişkili kayıt defteri dışlaması kaldırılır.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Dosya hazırlama değişiklikleri UWF Manager içinde uygulanmalıdır ve uwfmgr komutları olarak dışa aktarılamaz.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Dosya hazırlama listesi uygulandıktan sonra kaydedilir</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Yer paylaşımı anlık görüntüsünün kapsamı</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Bu liste yalnızca tanılama içindir. Hangi dosyaların kalıcılaştırılması gerektiğini belirlemek için kullanılamaz.</translation></message>
    <message><source>NTFS volumes only</source><translation>Yalnızca NTFS birimleri</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Büyük yer paylaşımlarının okunması yavaş olabilir veya yetersiz kaynak nedeniyle başarısız olabilir.</translation></message>
    <message><source>Small files may be missing</source><translation>Küçük dosyalar eksik olabilir</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Disk kümesi boyutundan (genellikle 4 KB) küçük dosyalar listede görünmeyebilir.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Bekleyen kalıcılaştırmaları göstermez</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Önceden kalıcılaştırılmış dosyalar, dışlanan yollar ve diğer işlemlerden etkilenen öğeler de görünebilir.</translation></message>

    <message><source>Command processing result</source><translation>Komut işleme sonucu</translation></message>
    <message><source>Copy command</source><translation>Komutu kopyala</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 dal okunamadı</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 dal okunamadı. Bulunan içerikle devam edin veya yalnızca başarısız öğeleri yeniden deneyin.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 dosya · %2 klasör</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 değer · %2 anahtar</translation></message>
    <message><source>Attempts</source><translation>Deneme sayısı</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Kalan görevleri iptal et</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Yalnızca başarıyla taranan içeriği kalıcılaştır. Okunamayan dallar değişmeden kalır.</translation></message>
    <message><source>Committing changes</source><translation>Değişiklikler kalıcılaştırılıyor</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Tamamlanan silme ve kalıcılaştırma işlemleri geri alınamaz.</translation></message>
    <message><source>Continue…</source><translation>Devam et…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Yalnızca bulunan hedefleri sil ve kalıcılaştır. Okunamayan dallar ve üst klasörleri veya anahtarları korunur.</translation></message>
    <message><source>Deleting and committing</source><translation>Siliniyor ve kalıcılaştırılıyor</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Silme ve kalıcılaştırma geri alınamaz. Yalnızca seçili kapsam işlenir; bekleyen UWF yapılandırması uygulanmaz.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Kapsam belirleniyor. Okunamayan dallar kaydedilirken diğer içerik taranmaya devam eder.</translation></message>
    <message><source>Export traversal records</source><translation>Tarama kayıtlarını dışa aktar</translation></message>
    <message><source>No actionable content</source><translation>İşlenebilir içerik yok</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>İşlenebilir hedef bulunamadı. Hiçbir şey silinmedi veya kalıcılaştırılmadı.</translation></message>
    <message><source>No changes have been committed.</source><translation>Henüz hiçbir değişiklik kalıcılaştırılmadı</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Hiçbir şey silinmedi veya kalıcılaştırılmadı</translation></message>
    <message><source>Path</source><translation>Yol</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Seçili kapsamı kalıcı olarak sil ve silme işlemlerini korunan birime kaydet.</translation></message>
    <message><source>Preparation complete</source><translation>Hazırlık tamamlandı</translation></message>
    <message><source>Preparing commit</source><translation>Kalıcılaştırma hazırlanıyor</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Silme ve kalıcılaştırma hazırlanıyor</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Yalnızca başarıyla taranan içeriği işle</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Taramanın seçili kapsam içinde kalması için yeniden ayrıştırma noktaları atlanır.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Yeniden başlatma, kalıcılaştırmayı geri almaz. Yalnızca seçili kapsam işlenir; bekleyen UWF yapılandırması uygulanmaz.</translation></message>
    <message><source>Retry failed items</source><translation>Başarısız öğeleri yeniden dene</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Seçili kapsamdaki değişiklikleri korunan birime kaydet.</translation></message>
    <message><source>Stopping the operation</source><translation>İşlem durduruluyor</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Başarılı ve başarısız tarama kayıtları korundu</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Klasör boş değil; işlenmemiş içerik korundu.</translation></message>
    <message><source>The operation could not be completed</source><translation>İşlem tamamlanamadı</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>Kayıt defteri anahtarında yeni değerler var; işlenmemiş içerik korundu.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>Kayıt defteri anahtarının hâlâ alt anahtarları var; işlenmemiş içerik korundu.</translation></message>
    <message><source>The target no longer exists.</source><translation>Hedef artık mevcut değil.</translation></message>
    <message><source>Traversal complete</source><translation>Tarama tamamlandı</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Tarama tamamlandı, bazı içerikler okunamadı</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Tarama kayıtları korunur ve dışa aktarılabilir.</translation></message>
    <message><source>Traverse again</source><translation>Yeniden tara</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Yeniden tarayın veya bu pencereyi kapatın.</translation></message>
    <message><source>Traversing the folder</source><translation>Klasör taranıyor</translation></message>
    <message><source>Traversing the registry</source><translation>Kayıt defteri taranıyor</translation></message>
    <message><source>Unable to prepare the operation</source><translation>İşlem hazırlanamadı</translation></message>
    <message><source>View results</source><translation>Sonuçları görüntüle</translation></message>

    <message><source>All changes committed</source><translation>Tüm değişiklikler kaydedildi</translation></message>
    <message><source>Some changes committed</source><translation>Bazı değişiklikler kaydedildi</translation></message>
    <message><source>No changes could be committed</source><translation>Hiçbir değişiklik kaydedilemedi</translation></message>
    <message><source>No changes to commit</source><translation>Kaydedilecek değişiklik yok</translation></message>
    <message><source>Commit interrupted</source><translation>Kaydetme kesintiye uğradı</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 öğe işlenmedi.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 kayıt defteri işlemi başarısız oldu.</translation></message>
    <message><source>The folder does not exist.</source><translation>Klasör mevcut değil.</translation></message>
    <message><source>Access denied.</source><translation>Erişim reddedildi.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Bu ada sahip bir öğe zaten var.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Klasör oluşturulamadı. Adı ve izinleri kontrol edin.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Öğe yeniden adlandırılamadı. Adı ve izinleri kontrol edin.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Bu tarayıcı yeniden adlandırmayı desteklemiyor.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Bu plan açıldıktan sonra dosya hazırlama taslağı değişti. Yeni bir planı gözden geçirin.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Bu plan açıldıktan sonra kaydedilmiş dosya hazırlama listesi değişti. Yenileyip değişiklikleri gözden geçirin.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Kaydedilmiş dosya hazırlama listesi onaylanan planla eşleşmiyor.</translation></message>
    <message><source>Export configuration</source><translation>Yapılandırmayı dışa aktar</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Geçerli temayı, dili ve HUB görünürlüğünü bu uygulamanın yanındaki UWF.ini dosyasına kaydedin.</translation></message>
    <message><source>Configuration saved</source><translation>Yapılandırma kaydedildi</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Geçerli arayüz ayarları şu konuma kaydedildi:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini zaten var. Mevcut dosya değiştirilmedi:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>UWF.ini kaydedilemedi:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Masaüstü</translation></message>
    <message><source>Downloads</source><translation>İndirilenler</translation></message>
    <message><source>Documents</source><translation>Belgeler</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Otomatik dosya kaydı durduruldu. Yeniden denemeden önce hatayı giderin.</translation></message>
</context>
</TS>
