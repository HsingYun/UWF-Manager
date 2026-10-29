<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="ar" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>معاينة</translation></message>
    <message><source>enable</source><translation>تمكين</translation></message>
    <message><source>disable</source><translation>تعطيل</translation></message>
    <message><source>Enabled</source><translation>مُمكّن</translation></message>
    <message><source>Disabled</source><translation>مُعطّل</translation></message>
    <message><source>OK</source><translation>موافق</translation></message>
    <message><source>Cancel</source><translation>إلغاء</translation></message>
    <message><source>Close</source><translation>إغلاق</translation></message>
    <message><source>Refresh</source><translation>تحديث</translation></message>
    <message><source>Add</source><translation>إضافة</translation></message>
    <message><source>Remove</source><translation>إزالة</translation></message>
    <message><source>Yes</source><translation>نعم</translation></message>
    <message><source>No</source><translation>لا</translation></message>
    <message><source>Excluded</source><translation>مستثنى</translation></message>
    <message><source>Not excluded</source><translation>غير مستثنى</translation></message>
    <message><source>File…</source><translation>ملف…</translation></message>
    <message><source>Folder…</source><translation>مجلد…</translation></message>
    <message><source>volume ID</source><translation>معرّف وحدة التخزين</translation></message>
    <message><source>drive letter</source><translation>حرف محرك الأقراص</translation></message>
    <message><source>Drive letter</source><translation>حرف محرك الأقراص</translation></message>
    <message><source>Volume ID</source><translation>معرّف وحدة التخزين</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>حالة حماية وحدة التخزين في الجلسة الحالية (للقراءة فقط).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>حماية وحدة التخزين في الجلسة التالية. تُوجّه الكتابات إلى التراكب وتُحذف عند إعادة التشغيل.</translation></message>
    <message><source>Current session</source><translation>الجلسة الحالية</translation></message>
    <message><source>Next session</source><translation>الجلسة التالية</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>طريقة تعريف وحدة التخزين. حرف المحرك أبسط، لكن الربط يبطل إذا تغيّر الحرف، مثلًا بعد إضافة أقراص أو إزالتها. يبقى معرّف الوحدة ثابتًا.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>وحدة التخزين غير مدعومة في UWF: </translation></message>
    <message><source>Global settings</source><translation>الإعدادات العامة</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>حالة عامل تصفية UWF في الجلسة الحالية (للقراءة فقط).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>تمكين UWF في الجلسة التالية. تُوجّه الكتابات على الوحدات المحمية إلى التراكب وتُحذف عند إعادة التشغيل.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>لتغيير النوع والحجم الأقصى:
1. عطّل عامل التصفية أعلاه.
2. أعد التشغيل ليصبح معطّلًا.
3. غيّر الإعداد.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>موقع التراكب: RAM أسرع لكنه يستهلك الذاكرة؛ وDisk يستخدم قرص النظام ويوفر سعة أكبر. يُحذف كلاهما عند إعادة التشغيل.</translation></message>
    <message><source>Type</source><translation>النوع</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>السعة القصوى للتراكب. يحد إجمالي الذاكرة من وضع RAM. يتطلب وضع Disk مساحة 1024 MB على الأقل ومساحة خالية كافية على وحدة النظام.</translation></message>
    <message><source>Maximum size</source><translation>الحجم الأقصى</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>الحجم الأقصى · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>حد التحذير</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>يسجّل إشعارًا بمستوى التحذير في سجل الأحداث عندما يصل استخدام طبقة التراكب إلى هذه القيمة. عند تفعيل الحدثين، يجب أن تكون هذه القيمة أقل من العتبة الحرجة. تؤدي القيمة 0 إلى تعطيل حدث التحذير فقط.</translation></message>
    <message><source>Critical threshold</source><translation>الحد الحرج</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>يسجّل إشعارًا بالمستوى الحرج في سجل الأحداث عندما يصل استخدام طبقة التراكب إلى هذه القيمة. عند تفعيل الحدثين، يجب أن تكون هذه القيمة أعلى من عتبة التحذير. تؤدي القيمة 0 إلى تعطيل الحدث الحرج فقط.</translation></message>
    <message><source>Overlay</source><translation>التراكب</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>نوع محرك غير مدعوم (الأقراص المحلية الثابتة فقط).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>تتجاوز الوحدة حد UWF البالغ 16 TiB لكل وحدة محمية.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: يمكن حماية الوحدة، لكن استثناءات الملفات وحفظ كل ملف بشكل دائم غير مدعومين.</translation></message>
    <message><source>Failed to read volume information.</source><translation>فشل: قراءة معلومات وحدة التخزين.</translation></message>
    <message><source>%1 free / %2</source><translation>المتاح %1 / %2</translation></message>
    <message><source>Commit</source><translation>حفظ دائم</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>حفظ تغييرات هذا الملف من التراكب دائمًا على القرص / حفظ تغييرات السجل دائمًا. لا يمكن التراجع عن هذا الإجراء.</translation></message>
    <message><source>Commit file changes…</source><translation>حفظ تغييرات الملف دائمًا…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>اختيار ملف لحفظه دائمًا على القرص. حفظ تغييرات هذا الملف من التراكب دائمًا على القرص.</translation></message>
    <message><source>Commit folder changes…</source><translation>حفظ تغييرات المجلد دائمًا…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>اختيار مجلد لحفظه دائمًا على القرص. حفظ تغييرات هذا المجلد من التراكب دائمًا على القرص. يشمل جميع المجلدات الفرعية.</translation></message>
    <message><source>Delete and commit file…</source><translation>حذف الملف وحفظ الحذف دائمًا على القرص…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>اختيار ملف لحفظ حذفه دائمًا. حذف الملف وحفظ الحذف دائمًا على القرص.</translation></message>
    <message><source>Delete and commit folder…</source><translation>حذف المجلد ومحتوياته وحفظ عمليات الحذف دائمًا على القرص…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>اختيار مجلد لحفظ حذفه دائمًا. حذف المجلد ومحتوياته وحفظ عمليات الحذف دائمًا على القرص. يشمل جميع المجلدات الفرعية.</translation></message>
    <message><source>Commit registry changes…</source><translation>حفظ تغييرات السجل دائمًا…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>أدخل مسار المفتاح واسم القيمة اختياريًا. إذا تُرك الاسم فارغًا، تُحفظ جميع قيم المفتاح وكل مفاتيحه الفرعية دائمًا.</translation></message>
    <message><source>Delete and commit registry…</source><translation>حفظ الحذف من السجل دائمًا…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>أدخل المفتاح المراد حذفه واسم القيمة اختياريًا. ترك الاسم فارغًا يحذف الشجرة كاملة، بما فيها المفتاح وقيمه ومفاتيحه الفرعية، ويحفظ الحذف دائمًا.</translation></message>
    <message><source>File exclusions</source><translation>استثناءات الملفات</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>ملفات ومجلدات الوحدة المستثناة من حماية UWF. انقر نقرًا مزدوجًا على العنصر لنسخ مساره.</translation></message>
    <message><source>Registry exclusions</source><translation>استثناءات السجل</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>استثناءات السجل عامة وتتبع عامل تصفية UWF، لا حماية هذه الوحدة. القائمة مشتركة بين الوحدات وتُعرض مرة واحدة في علامات تبويب الأقراص. انقر مرتين لنسخ المسار.</translation></message>
    <message><source>File staging</source><translation>الحفظ التلقائي للملفات</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>ملفات / مجلدات: تُحفظ دائمًا بشكل تلقائي قبل الإيقاف أو إعادة التشغيل الآمنين. يشمل جميع المجلدات الفرعية. ما دامت القائمة تحتوي على عناصر، يحفظها استثناء السجل التالي بين الجلسات المحمية: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>إضافة ملفات أو مجلد إلى قائمة الحفظ التلقائي.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>اختيار ملف لحفظه دائمًا على القرص. تُحفظ دائمًا بشكل تلقائي قبل الإيقاف أو إعادة التشغيل الآمنين.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>اختيار مجلد لحفظه دائمًا على القرص. تُحفظ دائمًا بشكل تلقائي قبل الإيقاف أو إعادة التشغيل الآمنين. يشمل جميع المجلدات الفرعية.</translation></message>
    <message><source>Select files for automatic commit</source><translation>اختيار ملف لحفظه دائمًا على القرص · الحفظ التلقائي للملفات</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>اختيار مجلد لحفظه دائمًا على القرص · الحفظ التلقائي للملفات · يشمل جميع المجلدات الفرعية.</translation></message>
    <message><source>Cannot stage this path</source><translation>تعذرت إضافة المسار إلى الحفظ التلقائي</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>يتداخل المسار مع استثناء UWF. أزل الاستثناء أولًا:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>يتداخل المسار مع عنصر للحفظ التلقائي. أزل العنصر أولًا قبل إضافة الاستثناء:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>يتداخل المسار مع عنصر حفظ تلقائي موجود</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>استثناء السجل هذا مطلوب ما دامت قائمة الحفظ التلقائي تحتوي على عناصر محفوظة</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>المسار غير موجود أو ليس من نوع الملف المتوقع:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>تعذر فحص المسار:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>لا يمكن إضافة نقاط إعادة التحليل إلى الحفظ التلقائي:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>لا يوجد حرف محرك أقراص، لذا يتعذر الحفظ الدائم عبر UWF:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>المسار على الوحدة %1. أضفه من علامة تبويب الحفظ التلقائي الخاصة بتلك الوحدة.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>لا يمكن إضافة جذر الوحدة، لأن ذلك سيحفظ الوحدة كاملة بشكل متكرر.</translation></message>
    <message><source>File staging could not be saved</source><translation>فشل: حفظ قائمة الحفظ التلقائي</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>أُعيد تحميل القائمة لتعذر إكمال التحديث:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>الحفظ التلقائي للملفات: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>تُحفظ دائمًا بشكل تلقائي قبل الإيقاف أو إعادة التشغيل الآمنين.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>الحفظ التلقائي للملفات: %1 · ملفات: %2 · مجلدات: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>يعتمد الحفظ التلقائي للملفات على استثناء السجل هذا للاحتفاظ بقائمته بعد إعادة التشغيل.</translation></message>
    <message><source>Built-in</source><translation>مدمج</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>عامل تصفية UWF معطّل. لم تتراكم تغييرات في التراكب لحفظها دائمًا.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>نظام الملفات %1 لا يدعم الحفظ الدائم لكل ملف.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>وحدة التخزين هذه غير مدعومة في UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>الوحدة لا تدعم الحفظ الدائم للملفات. يتوفر الحفظ الدائم للسجل فقط (استثناءاته عامة).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>نظام الملفات %1 لا يدعم الحفظ الدائم لكل ملف. يتوفر الحفظ الدائم للسجل فقط (استثناءاته عامة).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>الوحدة غير محمية حاليًا بواسطة UWF؛ لا توجد تغييرات ملفات لحفظها دائمًا.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>الوحدة غير محمية حاليًا بواسطة UWF؛ لا توجد تغييرات ملفات لحفظها دائمًا. يتوفر الحفظ الدائم للسجل فقط (استثناءاته عامة).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>اختيار ملف لحفظه دائمًا على القرص</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>اختيار مجلد لحفظه دائمًا على القرص</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>اختيار ملف لحفظ حذفه دائمًا</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>اختيار مجلد لحفظ حذفه دائمًا</translation></message>
    <message><source>Name</source><translation>الاسم</translation></message>
    <message><source>Data</source><translation>البيانات</translation></message>
    <message><source>(Default)</source><translation>(افتراضي)</translation></message>
    <message><source>(value not set)</source><translation>(لم تُضبط القيمة)</translation></message>
    <message><source>(empty)</source><translation>(فارغ)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>غير معروف (القيمة: الملف كبير جدًا)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>عرض ملفات التراكب. &lt;b&gt;لقطة تشخيصية فقط.&lt;/b&gt; لا تعتمد على هذه القائمة لتحديد ما ينبغي حفظه دائمًا.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>قائمة ملفات التراكب مدعومة على وحدات NTFS فقط.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>عامل UWF معطّل؛ لا يوجد تراكب لفحصه على أي وحدة.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>الوحدة غير محمية بـ UWF، لذا لا يوجد تراكب لفحصه.</translation></message>
    <message><source>Failed: %1</source><translation>فشل: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>تعطل موفر WMI أثناء السرد. هذا خلل معروف في UWF_Overlay.GetOverlayFiles عند كبر التراكب أو ارتفاع ضغط الإدخال والإخراج.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>الذاكرة غير كافية أو العملية غير مدعومة من الموفر. يلزم NTFS وذاكرة متاحة. جرّب تراكبًا أصغر أو أغلق التطبيقات كثيفة الذاكرة.</translation></message>
    <message><source>Raw error: %1</source><translation>رمز الخطأ: %1</translation></message>
    <message><source>Export overlay file list</source><translation>تصدير إلى ملف… · التراكب · ملفات</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>ملفات نصية (*.txt);;كل الملفات (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>ملفات الأوامر (*.cmd);;كل الملفات (*)</translation></message>
    <message><source>Export failed</source><translation>فشل التصدير</translation></message>
    <message><source>Export finished</source><translation>اكتمل التصدير</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>فشل: فتح الملف (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>فشل: الكتابة إلى الملف (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>الإعدادات: الكتابة إلى الملف · عناصر: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>لا يمكن استثناء وحدة التخزين كاملة (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>لا يمكن استثناء ملفات ترحيل الصفحات والتبديل والإسبات، لأن UWF يعتمد عليها.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>لا يمكن استثناء الدليل كاملًا: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>لا يمكن استثناء الدليل كاملًا: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>لا يمكن استثناء الدليل كاملًا: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>لا يمكن استثناء ملف النظام الحرج هذا: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>لا يمكن استثناء ملف سجل المستخدم NTUSER.DAT.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>لا يمكن ترك مفتاح السجل فارغًا.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>لا يمكن أن يبدأ مفتاح السجل بشرطة مائلة عكسية.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>المسار غير صالح لاحتوائه على شرطات مائلة عكسية متتالية.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>تستخدم مسارات السجل `\` فاصلًا، وليس `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>المسار غير صالح لاحتوائه على محارف تحكم غير مرئية.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>تحظر وثائق UWF استثناء سر حساب جهاز المجال: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>يمكن استثناء مفاتيح فرعية محددة ضمن المفاتيح الرئيسية التالية فقط:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>إضافة ملفات أو مجلدات إلى الاستثناءات. لا يحمي UWF العناصر المستثناة.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>اختيار ملف لإضافته · استثناءات الملفات.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>اختيار مجلد لإضافته · استثناءات الملفات. يشمل جميع المجلدات الفرعية.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>أدخل مسار مفتاح السجل المراد استثناؤه.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>أدخل مسار مفتاح السجل المراد استثناؤه. / الاستبقاء.</translation></message>
    <message><source>Registry key…</source><translation>سجل النظام · المفتاح…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>المفتاح السري للمجال (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>ترخيص وصول العميل لخدمات المحطة الطرفية (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>استبقاء سر المجال (كلمة مرور حساب الجهاز) بين جلسات UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>استبقاء تراخيص وصول العميل لخدمات المحطة الطرفية بين جلسات UWF.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ الاستبقاء %1: %2</translation></message>
    <message><source>Remove selected</source><translation>إزالة المحدد</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>إزالة الاستثناءات المحددة. تسري بعد التطبيق.</translation></message>
    <message><source>Open containing folder</source><translation>فتح المجلد الحاوي</translation></message>
    <message><source>Copy file path</source><translation>نسخ مسار الملف</translation></message>
    <message><source>Copy registry path</source><translation>نسخ مسار السجل</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>حفظ تغييرات هذا المجلد من التراكب دائمًا على القرص…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>حفظ تغييرات هذا الملف من التراكب دائمًا على القرص…</translation></message>
    <message><source>Copied to clipboard</source><translation>نُسخ إلى الحافظة</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>اختيار ملف لإضافته · استثناءات الملفات. يمكن تحديد عدة ملفات.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>اختيار مجلد لإضافته · استثناءات الملفات</translation></message>
    <message><source>Add registry exclusion</source><translation>إضافة استثناء سجل</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>المسار %1 ليس على الوحدة %2، لذا لا يمكن إضافته إلى استثناءاتها.</translation></message>
    <message><source>Path is not on this volume</source><translation>المسار ليس على هذه الوحدة</translation></message>
    <message><source>Cannot add this exclusion</source><translation>تعذرت إضافة هذا الاستثناء</translation></message>
    <message><source>File: %1</source><translation>ملف… %1</translation></message>
    <message><source>Folder: %1</source><translation>مجلد… %1</translation></message>
    <message><source>Registry: %1</source><translation>سجل النظام: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>مستثنى افتراضيًا؛ يمكن تغييره في أي وقت.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>الجلسة الحالية: %1
الجلسة التالية: %2</translation></message>
    <message><source>Pending change: %1</source><translation>التغييرات المعلقة: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>مدير عامل تصفية الكتابة الموحد (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>إعادة قراءة حالة جلسة UWF الحالية وإعدادات الجلسة التالية.</translation></message>
    <message><source>Review and apply</source><translation>مراجعة وتطبيق</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>مراجعة جميع التغييرات المعلقة وتطبيقها دفعة واحدة. تسري معظم التغييرات بعد إعادة التشغيل التالية.</translation></message>
    <message><source>Safe shutdown</source><translation>إيقاف آمن</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>سيُوقف النظام بأمان عبر UWF. متاح حتى إذا امتلأ تراكب UWF.</translation></message>
    <message><source>Safe restart</source><translation>إعادة تشغيل آمنة</translation></message>
    <message><source>Direct restart</source><translation>إعادة تشغيل مباشرة</translation></message>
    <message><source>Direct shutdown</source><translation>إيقاف مباشر</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>سيُعاد تشغيل النظام بأمان عبر UWF. متاح حتى إذا امتلأ تراكب UWF.</translation></message>
    <message><source>Log</source><translation>السجل التشخيصي</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>عرض السجل الداخلي المتراكم في هذه الجلسة لاستكشاف المشكلات.</translation></message>
    <message><source>About</source><translation>حول</translation></message>
    <message><source>Switch display language</source><translation>تغيير لغة العرض</translation></message>
    <message><source>Language</source><translation>اللغة</translation></message>
    <message><source>Theme</source><translation>المظهر</translation></message>
    <message><source>Toggle light / dark theme</source><translation>التبديل بين المظهر الفاتح والداكن</translation></message>
    <message><source>System information</source><translation>معلومات النظام</translation></message>
    <message><source>Diagnostic snapshot</source><translation>لقطة تشخيصية</translation></message>
    <message><source>Read only</source><translation>للقراءة فقط</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>لم يُعثر على الهدف؛ لا يوجد شيء في التراكب لحفظه دائمًا.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>الهدف موجود في التراكب فقط؛ أُنشئ تحت حماية UWF ولم يُحفظ على القرص أو في السجل الدائم. سيختفي بإعادة التشغيل؛ حفظ حذفه دائمًا غير مطلوب وغير ممكن.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>رفض النظام معلمة (مسار أو وسيطة غير صالحة).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>فشلت العملية. قد تستخدم عملية أخرى الهدف، مثل مجلد مفتوح في المستكشف أو ملف مفتوح. أغلق البرامج التي تستخدمه ثم أعد المحاولة.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>فشلت العملية؛ راجع السجل للتفاصيل.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>رُفضت العملية (الرمز %1).</translation></message>
    <message><source>Unknown cause.</source><translation>غير معروف.</translation></message>
    <message><source>Commit canceled</source><translation>أُلغي الحفظ الدائم</translation></message>

    <message><source>Succeeded</source><translation>نجح</translation></message>
    <message><source>Skipped</source><translation>تم التخطي</translation></message>
    <message><source>Failed</source><translation>فشل</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>نجح: %1; تم التخطي: %2; فشل: %3.</translation></message>

    <message><source>Existed before</source><translation>موجود قبل العملية</translation></message>
    <message><source>Exists after</source><translation>موجود بعد العملية</translation></message>
    <message><source>Error code</source><translation>رمز الخطأ</translation></message>
    <message><source>Reason</source><translation>السبب</translation></message>
    <message><source>Previous page</source><translation>الصفحة السابقة</translation></message>
    <message><source>First page</source><translation>الصفحة الأولى</translation></message>
    <message><source>Last page</source><translation>الصفحة الأخيرة</translation></message>
    <message><source>Next page</source><translation>الصفحة التالية</translation></message>
    <message><source>Copy selected rows</source><translation>نسخ الصفوف المحددة</translation></message>
    <message><source>Copy all</source><translation>نسخ الكل</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>استثناء السجل هذا مطلوب ما دامت قائمة الحفظ التلقائي تحتوي على عناصر محفوظة. الحفظ التلقائي للملفات: مسح → إزالة استثناء سجل.</translation></message>
    <message><source>Copy current page</source><translation>نسخ الصفحة الحالية</translation></message>
    <message><source>No pending changes</source><translation>لا توجد تغييرات معلقة</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (استثناءات السجل · الإعدادات العامة.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>وحدة التخزين %1: حماية UWF / استثناءات الملفات / الحفظ التلقائي للملفات.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: غير مدعوم</translation></message>
    <message><source>Review and apply changes</source><translation>مراجعة التغييرات وتطبيقها</translation></message>
    <message><source>Confirm apply</source><translation>تأكيد التطبيق</translation></message>
    <message><source>Result</source><translation>النتيجة</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ عامل التصفية: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ ضبط حد التحذير: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ ضبط الحد الحرج: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ ضبط نوع التراكب / ضبط الحجم الأقصى: لتغيير النوع والحجم الأقصى:
1. عطّل عامل التصفية أعلاه.
2. أعد التشغيل ليصبح معطّلًا.
3. غيّر الإعداد.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ ضبط نوع التراكب: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ ضبط الحجم الأقصى: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ ضبط الحجم الأقصى: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ وحدة التخزين %1 · حماية UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ وحدة التخزين %1 · الربط حسب: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ وحدة التخزين %1 · إضافة استثناء ملف: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ وحدة التخزين %1 · إزالة استثناء ملف: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ إضافة استثناء سجل: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ إزالة استثناء سجل: %1</translation></message>
    <message><source>Time</source><translation>الوقت</translation></message>
    <message><source>Level</source><translation>المستوى</translation></message>
    <message><source>Tag</source><translation>الوسم</translation></message>
    <message><source>Message</source><translation>الرسالة</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>جارٍ حساب الملفات للحفظ التلقائي…</translation></message>
    <message><source>Committing staged files</source><translation>جارٍ حفظ الملفات المُعدّة</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>عولج %1 من %2 ملف.</translation></message>
    <message><source>No additional error details were provided.</source><translation>لم تُقدّم تفاصيل خطأ إضافية.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>توقف الحفظ التلقائي بشكل غير متوقع:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>لم يكتمل الحفظ التلقائي للملفات: خطأ غير معروف.</translation></message>
    <message><source>File staging will be skipped</source><translation>سيُتخطّى الحفظ التلقائي للملفات</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>إعادة تشغيل مباشرة: سيُتخطّى الحفظ التلقائي للملفات.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>إيقاف مباشر: سيُتخطّى الحفظ التلقائي للملفات.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>تُخطّي الحفظ التلقائي لاختيار إجراء طاقة مباشر.</translation></message>
    <message><source>Safe shutdown failed</source><translation>إيقاف آمن: فشل</translation></message>
    <message><source>Shutdown failed: %1</source><translation>إيقاف التشغيل: فشل (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>إعادة تشغيل آمنة: فشل</translation></message>
    <message><source>Direct restart failed</source><translation>إعادة تشغيل مباشرة: فشل</translation></message>
    <message><source>Direct shutdown failed</source><translation>إيقاف مباشر: فشل</translation></message>
    <message><source>Restart failed: %1</source><translation>إعادة التشغيل: فشل (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>تعذرت قراءة قائمة الحفظ التلقائي:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>تعذرت قراءة قائمة الحفظ التلقائي: خطأ غير معروف.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>ملفات: %1 · نجح: %2 · تم التخطي (ملفات): %3 · تم التخطي (عناصر): %4 · فشل: %5</translation></message>
    <message><source>File staging preparation</source><translation>الحفظ التلقائي للملفات: تفاصيل العملية</translation></message>
    <message><source>Reparse points are not supported.</source><translation>نقاط إعادة التحليل غير مدعومة.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>يمكن إضافة مسارات مطلقة على وحدات محلية فقط.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>يمكن إضافة مسارات مطلقة على وحدات محلية فقط.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>تعذر فحص المسار المُعدّ للحفظ التلقائي.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>مسار الملف المُعدّ يشير الآن إلى نوع مختلف.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>مسار المجلد المُعدّ يشير الآن إلى نوع مختلف.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>تعذر سرد المجلد المُعدّ بالكامل.</translation></message>
    <message><source>The path has no drive letter.</source><translation>لا يحتوي المسار على حرف محرك أقراص.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>لا يمكن معالجة جذر الوحدة بشكل متكرر.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>رفض موفر UWF الحفظ الدائم.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>فشل الحفظ الدائم: خطأ غير معروف.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>سُجّل %1 فشل إضافي في السجل.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>لا يحتوي المسار على حرف محرك أقراص. تحديد وحدة التخزين المستهدفة: فشل.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>لا يوجد سجل للجلسة الحالية للوحدة %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>المسار ضمن استثناءات الملفات.
الاستثناء: %1</translation></message>

    <message><source>Continue</source><translation>متابعة</translation></message>
    <message><source>Delete and commit</source><translation>حذف وحفظ دائم</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>المفتاح ضمن استثناءات السجل.
الاستثناء: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>لا يوجد سجل لعامل تصفية السجل في الجلسة الحالية.</translation></message>

    <message><source>Export commands to file</source><translation>تصدير الأوامر إلى ملف</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>تصدير إلى ملف… · الأمر: %1
%2</translation></message>
    <message><source>Import</source><translation>استيراد</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: اكتب أو الصق أمرًا واحدًا في كل سطر. حمّل نصًا أو سجلًا أو نصًا برمجيًا (.txt أو .bat أو .ps1)؛ تُضاف الأسطر التي تحتوي على هذا الأمر. الاستيراد يُعدّ التغييرات فقط. لا تُكتب إلى النظام إلا عبر «مراجعة وتطبيق» في شريط الأدوات.</translation></message>
    <message><source>Load from file…</source><translation>تحميل من ملف…</translation></message>
    <message><source>Load recommended configuration</source><translation>تحميل الإعدادات الموصى بها</translation></message>
    <message><source>Choose recommended configuration</source><translation>اختيار الإعدادات الموصى بها</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>اختر مجموعات الإعدادات الموصى بها لإضافتها. يمكنك مراجعة الأوامر أو حذفها قبل الاستيراد.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>من وثائق Microsoft الرسمية.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>استثناءات UWF الموصى بها من Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>راجع قبل الاستيراد: يجب وجود المجلدات كي يقبل UWF استثناءات الملفات.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>المصدر: وثائق Microsoft الرسمية حول UWF، بما فيها استثناءات التصفية الشائعة ودعم مكافحة البرامج الضارة.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>برنامج تحسين تجربة العملاء (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: استبقاء دائم: حالة الموافقة في النهج</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: استبقاء دائم: حالة الموافقة المحلية</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: استبقاء دائم: علامة تعطيل الرفع</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>خدمة النقل الذكي في الخلفية (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: استبقاء دائم: ملفات قائمة انتظار التنزيل</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: استبقاء دائم: فهرس حالة النقل</translation></message>
    <message><source>Network profiles and policies</source><translation>ملفات تعريف الشبكة ونهجها</translation></message>
    <message><source>Wireless network GPO policy</source><translation>نهج GPO للشبكة اللاسلكية</translation></message>
    <message><source>Wired network GPO policy</source><translation>نهج GPO للشبكة السلكية</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>نهج GPO للشبكة اللاسلكية · ملفات</translation></message>
    <message><source>Wired network GPO policy files</source><translation>نهج GPO للشبكة السلكية · ملفات</translation></message>
    <message><source>Wireless network interface profiles</source><translation>ملفات تعريف الواجهات اللاسلكية</translation></message>
    <message><source>Wired network interface profiles</source><translation>ملفات تعريف الواجهات السلكية</translation></message>
    <message><source>Wireless service configuration</source><translation>إعدادات الخدمة اللاسلكية</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>إعدادات خدمة النطاق العريض للأجهزة المحمولة</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>إعدادات خدمة التكوين التلقائي السلكي</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>ملفات تعريف الشبكة XML خاصة بكل جهاز. أضف المسارات الفعلية Interfaces\{GUID}\{GUID}.xml يدويًا عند الحاجة.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>التوقيت الصيفي (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: استبقاء دائم: تعريفات المناطق الزمنية</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: استبقاء دائم: معلومات المنطقة الزمنية المختارة</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: استبقاء دائم: ملفات المنتج وتحديثاته</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: استبقاء دائم: التواقيع والحالة في ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: استبقاء دائم: Windows Update · السجل التشخيصي</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: استبقاء دائم: MpCmdRun · السجل التشخيصي</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: استبقاء دائم: حالة المنتج في السجل</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: استبقاء دائم: WdBoot · حالة الخدمة</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: استبقاء دائم: WdFilter · حالة الخدمة</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: استبقاء دائم: WdNisSvc · حالة الخدمة</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: استبقاء دائم: WdNisDrv · حالة الخدمة</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: استبقاء دائم: WinDefend · حالة الخدمة</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: استبقاء دائم: ملفات برنامج العميل</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: استبقاء دائم: Windows Update · السجل التشخيصي</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: استبقاء دائم: MpCmdRun · السجل التشخيصي</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: استبقاء دائم: تواقيع مكافحة البرامج الضارة وحالتها</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: استبقاء دائم: حالة مكافحة البرامج الضارة في السجل</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>اختيار ملفات تتضمن أوامر uwfmgr</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>ملفات نصية (*.txt *.cmd);;كل الملفات (*)</translation></message>
    <message><source>Status</source><translation>الحالة</translation></message>
    <message><source>Import failed</source><translation>فشل الاستيراد</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>خطأ داخلي: لم يُسجّل معالج للتطبيق.</translation></message>
    <message><source>Nothing to import</source><translation>لا توجد أوامر لاستيرادها</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>لم يُعثر على أوامر uwfmgr في الإدخال.</translation></message>
    <message><source>Duplicates: %1</source><translation>مكرر: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>غير مدعوم: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── الدفعة %1 ──</translation></message>
    <message><source>Duplicate</source><translation>مكرر</translation></message>
    <message><source>Unsupported</source><translation>غير مدعوم</translation></message>
    <message><source>File too large</source><translation>الملف كبير جدًا</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>الملف %1 أكبر من 5 MB ولم يُحلَّل. رشّح محتواه يدويًا أولًا.</translation></message>
    <message><source>Cannot read file</source><translation>تعذرت قراءة الملف</translation></message>
    <message><source>Could not open file %1: %2</source><translation>فشل: فتح الملف (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>هل تريد تجاهل التغييرات المعلقة؟</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>هناك %1 تغيير لم يُطبّق.

هل تريد تجاهلها والمتابعة؟</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>أمر uwfmgr غير مكتمل</translation></message>
    <message><source>Missing size argument (MB)</source><translation>وسيطة مفقودة: الحجم (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>يجب أن يكون الحجم عددًا صحيحًا غير سالب بوحدة MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>وسيطة مفقودة: التراكب · النوع (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>قيمة غير صالحة: التراكب · النوع: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>وسيطة مفقودة: وحدة التخزين (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>يجب تحديد الوحدة بحرف محرك أقراص مثل C:</translation></message>
    <message><source>Missing path argument</source><translation>وسيطة مفقودة: المسار</translation></message>
    <message><source>Missing registry key argument</source><translation>وسيطة مفقودة: سجل النظام · المفتاح</translation></message>
    <message><source>Unclosed double quote in command</source><translation>علامة اقتباس مزدوجة غير مغلقة في الأمر</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>وسيطة إضافية غير متوقعة: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>أمر uwfmgr غير مدعوم (لا يقابله إجراء في الواجهة)</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>التغييرات المعلقة: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>الحالة المطلوبة متحققة؛ لا حاجة إلى إجراء</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>المسار خارج هذه الوحدة أو أنها لا تدعم استثناءات الملفات (مثل exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>مرفوض من قائمة حظر UWF (ملفات النظام أو Windows أو ملف الترحيل وغيرها)</translation></message>
    <message><source>Pending filter %1</source><translation>التغييرات المعلقة: عامل التصفية %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>عامل التصفية: الحالة المطلوبة متحققة؛ لا حاجة إلى إجراء</translation></message>
    <message><source>Pending overlay type → %1</source><translation>التغييرات المعلقة: التراكب · النوع → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>التراكب · النوع: %1 (الحالة المطلوبة متحققة؛ لا حاجة إلى إجراء)</translation></message>
    <message><source>Invalid size value: %1</source><translation>قيمة غير صالحة: الحجم: %1</translation></message>
    <message><source>maximum size</source><translation>الحجم الأقصى</translation></message>
    <message><source>warning threshold</source><translation>حد التحذير</translation></message>
    <message><source>critical threshold</source><translation>الحد الحرج</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>التغييرات المعلقة: التراكب %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>التراكب %1: %2 MB (الحالة المطلوبة متحققة؛ لا حاجة إلى إجراء)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>وحدة غير معروفة %1 (لا يوجد قرص مؤهل لـ UWF بهذا الحرف)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>التغييرات المعلقة: وحدة التخزين %1 · حماية UWF: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>وحدة التخزين %1: الحالة المطلوبة متحققة؛ لا حاجة إلى إجراء</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: لا يحتوي المسار على حرف محرك أقراص. تحديد وحدة التخزين المستهدفة: فشل.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>وحدة التخزين %1: UWF غير مدعوم</translation></message>
    <message><source>file exclusion</source><translation>استثناءات الملفات</translation></message>
    <message><source>registry exclusion</source><translation>استثناءات السجل</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>استثناءات السجل متاحة فقط في علامة تبويب السجل، وهي غير موجودة</translation></message>
    <message><source>Unsupported command</source><translation>الأمر: غير مدعوم</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: إظهار أو إخفاء.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: مُمكّن</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: مُعطّل</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: غير معروف</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>المستخدم %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>الوضع المتقدم</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>تنسيق الحفظ التلقائي مع إيقاف Windows وإعادة تشغيله.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>فشل: قراءة حالة الوضع المتقدم (خطأ غير معروف).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>فشل: قراءة حالة الخدمة:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>فشل: قراءة حالة الخدمة (خطأ غير معروف).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>الخدمة غير مهيّأة كعملية مستقلة.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>الخدمة غير مهيّأة للتشغيل التلقائي.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>الخدمة غير مهيّأة للعمل بحساب LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>الخدمة لا تعمل.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>مسار الخدمة أو وسيطات تشغيلها لا تتطابق مع هذا الملف التنفيذي.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>مهلة إشعار ما قبل الإيقاف للخدمة غير مضبوطة بشكل صحيح.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>الخدمة لا تعلن الصلاحيات اللازمة لبدء وكيل الواجهة.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>لم تقبل الخدمة إشعارات Windows لما قبل الإيقاف.</translation></message>
    <message><source>Enable enhanced mode</source><translation>تمكين الوضع المتقدم</translation></message>
    <message><source>Disable enhanced mode</source><translation>تعطيل الوضع المتقدم</translation></message>
    <message><source>Start service</source><translation>بدء الخدمة</translation></message>
    <message><source>Repair enhanced mode</source><translation>إصلاح الوضع المتقدم</translation></message>
    <message><source>Delete service</source><translation>حذف الخدمة</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>إيقاف خدمة الوضع المتقدم وحذفها مع بياناتها المتبقية في السجل.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>الخدمة غير موجودة، لكن بياناتها في السجل لا تزال موجودة.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>بانتظار إكمال مصادقة وكيل الواجهة.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>الخدمة تعمل، لكن لا يوجد وكيل واجهة مُصادَق عليه متصل.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>خدمة مساعدة الوضع المتقدم في UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>الوضع المتقدم · الإعدادات: تم التطبيق.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>الإعدادات: تم التطبيق. تقرير الاستبقاء من UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>الوضع المتقدم · الإعدادات: فشل:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>الوضع المتقدم · الإعدادات: فشل (خطأ غير معروف).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>حُذفت خدمة الوضع المتقدم وإعداداتها المتبقية.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>طُلب حذف الخدمة، لكن تنظيف البيانات المتبقية أبلغ:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>الوضع المتقدم: حذف الخدمة: فشل:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>الوضع المتقدم: حذف الخدمة: فشل (خطأ غير معروف).</translation></message>
    <message><source>File staging completed</source><translation>الحفظ التلقائي للملفات: نجح</translation></message>
    <message><source>File staging failed</source><translation>الحفظ التلقائي للملفات: فشل</translation></message>
    <message><source>All staged targets were processed.</source><translation>عُولجت جميع الأهداف المُعدّة.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>عُولجت جميع الأهداف؛ فشلت عملية واحدة أو أكثر.</translation></message>
    <message><source>Show main window</source><translation>إظهار النافذة الرئيسية</translation></message>
    <message><source>Restore default position</source><translation>استعادة الموضع الافتراضي</translation></message>
    <message><source>Show overlay hub</source><translation>إظهار HUB التراكب</translation></message>
    <message><source>Hide overlay hub</source><translation>إخفاء HUB التراكب</translation></message>
    <message><source>Exit application</source><translation>إنهاء التطبيق</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ فشل: الاتصال بالنظام</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>فشل: قراءة قيم السجل (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>سرد ملفات التراكب: خطأ غير معروف.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ فشل: إجراء عامل التصفية: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ فشل: قراءة حالة عامل التصفية</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ فشل: ضبط حد التحذير</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ فشل: ضبط الحد الحرج</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ فشل: قراءة حالة التراكب</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ فشل: ضبط نوع التراكب</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ فشل: ضبط الحجم الأقصى</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ فشل: قراءة إعدادات التراكب</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ فشل: تسجيل الوحدة %1 في UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ فشل: حماية الوحدة %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ فشل: ضبط ربط الوحدة %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ وحدة التخزين %1 · فشل: إضافة استثناء ملف %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ استثناء الملف غير موجود أصلًا على الوحدة %1: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ وحدة التخزين %1 · فشل: إزالة استثناء ملف %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ فشل: قراءة إعدادات وحدة التخزين</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ فشل: إضافة استثناء سجل %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ فشل: إزالة استثناء سجل %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ فشل: تحديث خيارات استبقاء السجل</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ فشل: قراءة عامل تصفية السجل</translation></message>
    <message><source>UWF is not available</source><translation>UWF: غير مدعوم</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>قراءة حالة الهدف: خطأ غير معروف.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>المسار غير موجود؛ لا يوجد ما يمكن حذفه.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>خطأ غير معروف.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>تعذر تأكيد النتيجة لفشل إعادة قراءة حالة الهدف: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>قبل الموفر الحذف، لكن إعادة القراءة من المصدر الموثوق تُظهر أن الهدف لا يزال موجودًا.</translation></message>
    <message><source>Unknown</source><translation>غير معروف</translation></message>
    <message><source>Failed to load logs: %1</source><translation>فشل: تحميل السجل التشخيصي (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>فشل: تحميل السجل التشخيصي (خطأ غير معروف).</translation></message>
    <message><source>: unknown error</source><translation>: خطأ غير معروف</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>فشل: تحديد وحدة التخزين المستهدفة (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>فشل: سرد مفاتيح السجل (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>فشل: تحديد وحدة التخزين المستهدفة (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>تم تخطي العملية لأن عملية مسبقة مطلوبة فشلت أو تعذّر تأكيد نتيجتها.</translation></message>
    <message><source>Current selection</source><translation>التحديد الحالي</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>اختيار المفتاح بالكامل مع جميع المفاتيح الفرعية</translation></message>
    <message><source>Select this registry key</source><translation>اختيار مفتاح التسجيل هذا</translation></message>
    <message><source>Select this registry value</source><translation>اختيار قيمة التسجيل هذه</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>اختر موقعًا داخل محرك الأقراص أو المجلد المحدد.</translation></message>
    <message><source>Registry key or persistence setting</source><translation>مفتاح تسجيل أو إعداد استبقاء</translation></message>
    <message><source>Staged path</source><translation>المسار المرحلي</translation></message>
    <message><source>Required configuration</source><translation>إعداد مطلوب</translation></message>
    <message><source>Recursive staging</source><translation>حفظ مرحلي متكرر</translation></message>
    <message><source>Staged file</source><translation>ملف مرحلي</translation></message>
    <message><source>Home</source><translation>المجلد الرئيسي</translation></message>
    <message><source>Unavailable</source><translation>غير متاح</translation></message>
    <message><source>Compatibility mode</source><translation>وضع التوافق</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>وصل استخدام طبقة التراكب إلى حد التحذير. احفظ التغييرات ثم أعد التشغيل بأمان.</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>وصل استخدام طبقة التراكب إلى الحد الحرج. احفظ التغييرات وأعد التشغيل بأمان في أقرب وقت.</translation></message>
    <message><source>Rename</source><translation>إعادة تسمية</translation></message>
    <message><source>Rename…</source><translation>إعادة تسمية…</translation></message>
    <message><source>Open folder</source><translation>فتح المجلد</translation></message>
    <message><source>Select this file</source><translation>اختيار هذا الملف</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>أدخل اسماً صالحاً للملف أو المجلد.</translation></message>
    <message><source>Copy all logs</source><translation>نسخ جميع السجلات</translation></message>
    <message><source>Copy name</source><translation>نسخ الاسم</translation></message>
    <message><source>Fit columns to contents</source><translation>ملاءمة الأعمدة للمحتوى</translation></message>
    <message><source>Restore</source><translation>استعادة</translation></message>
    <message><source>Move</source><translation>نقل</translation></message>
    <message><source>Size</source><translation>تغيير الحجم</translation></message>
    <message><source>Expand</source><translation>توسيع</translation></message>
    <message><source>Collapse</source><translation>طي</translation></message>
    <message><source>Undo removal</source><translation>التراجع عن الإزالة</translation></message>

    <message><source>32-bit</source><translation>32 بت</translation></message>
    <message><source>64-bit</source><translation>64 بت</translation></message>
    <message><source>All files (*)</source><translation>كل الملفات (*)</translation></message>

    <message><source>Changes already committed are retained.</source><translation>تُحتفظ بالتغييرات التي حُفظت بالفعل.</translation></message>
    <message><source>Commit changes</source><translation>حفظ التغييرات</translation></message>
    <message><source>Commit file changes?</source><translation>هل تريد حفظ تغييرات الملف؟</translation></message>
    <message><source>Commit folder changes?</source><translation>هل تريد حفظ تغييرات المجلد؟</translation></message>
    <message><source>Commit registry key changes?</source><translation>هل تريد حفظ تغييرات مفتاح السجل؟</translation></message>
    <message><source>Commit registry value changes?</source><translation>هل تريد حفظ تغييرات قيمة السجل؟</translation></message>
    <message><source>Copy selection</source><translation>نسخ التحديد</translation></message>
    <message><source>Delete and commit this file?</source><translation>هل تريد حذف الملف وحفظ حذفه؟</translation></message>
    <message><source>Delete and commit this folder?</source><translation>هل تريد حذف المجلد وحفظ حذفه؟</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>هل تريد حذف مفتاح السجل وحفظ حذفه؟</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>هل تريد حذف قيمة السجل وحفظ حذفها؟</translation></message>

    <message><source>Direct restart?</source><translation>هل تريد إعادة التشغيل مباشرةً؟</translation></message>
    <message><source>Direct shutdown?</source><translation>هل تريد إيقاف التشغيل مباشرةً؟</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>سيتم تخطي الملفات المجهزة. ستُفقد التغييرات غير المحفوظة في المسارات المحمية بعد إعادة التشغيل.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>سيتم تخطي الملفات المجهزة. ستُفقد التغييرات غير المحفوظة في المسارات المحمية بعد إيقاف التشغيل.</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>تمت معالجة %1 / %2 عنصر</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>يمكن إلغاء المهام المتبقية بعد انتهاء معالجة الهدف الحالي.</translation></message>
    <message><source>Target</source><translation>الهدف</translation></message>

    <message><source>Select registry key or value</source><translation>تحديد مفتاح سجل أو قيمة</translation></message>
    <message><source>Select registry key or value to delete</source><translation>تحديد مفتاح سجل أو قيمة لحذفها</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>الخدمة مثبتة لكنها متوقفة. شغّلها لاستعادة الحفظ التلقائي.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>تعذرت قراءة حالة خدمة الوضع المحسّن. تحقق من أذونات الخدمة وأعد المحاولة.</translation></message>
    <message><source>Committing</source><translation>جارٍ الحفظ</translation></message>
    <message><source>Confirm before continuing</source><translation>أكد قبل المتابعة</translation></message>
    <message><source>Automatic saving incomplete</source><translation>لم يكتمل الحفظ التلقائي</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>اختر مجلدًا آخر أو تحقق من أذونات الوصول وأعد المحاولة.</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF غير متاح حاليًا</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>تعذرت قراءة إعدادات UWF. تأكد من تثبيت عامل تصفية الكتابة الموحد في Windows ثم أعد القراءة.</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>فعّل عامل التصفية وأعد التشغيل لعرض الاستخدام المباشر</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>اقرأ الإعدادات لعرض الاستخدام المباشر</translation></message>
    <message><source>Filter is disabled</source><translation>عامل التصفية معطّل</translation></message>
    <message><source>Usage unavailable</source><translation>الاستخدام غير متاح</translation></message>
    <message><source>More</source><translation>المزيد</translation></message>
    <message><source>UWF filter</source><translation>عامل التصفية UWF</translation></message>
    <message><source>On</source><translation>تشغيل</translation></message>
    <message><source>Off</source><translation>إيقاف</translation></message>
    <message><source>Binding</source><translation>الربط</translation></message>
    <message><source>Protected</source><translation>محمي</translation></message>
    <message><source>View overlay</source><translation>عرض التراكب</translation></message>
    <message><source>Current usage</source><translation>الاستخدام الحالي</translation></message>
    <message><source>Search paths</source><translation>البحث في المسارات</translation></message>
    <message><source>File or folder</source><translation>ملف أو مجلد</translation></message>
    <message><source>Configured</source><translation>مُعدّ</translation></message>
    <message><source>Ready</source><translation>جاهز</translation></message>
    <message><source>%1 items</source><translation>%1 عناصر</translation></message>
    <message><source> · %1 selected</source><translation> · %1 محدد</translation></message>
    <message><source>%1 selected</source><translation>%1 محدد</translation></message>
    <message><source>%1 changes</source><translation>%1 تغييرات</translation></message>
    <message><source>%1 commands</source><translation>%1 أوامر</translation></message>
    <message><source>%1 records</source><translation>%1 سجلات</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 سجلات · الإجمالي %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 تغييرات قيد الانتظار</translation></message>
    <message><source>Pending removal</source><translation>بانتظار الإزالة</translation></message>
    <message><source>Added</source><translation>مضاف</translation></message>
    <message><source>Warning %1 MB</source><translation>تحذير عند %1 MB</translation></message>
    <message><source>Critical %1 MB</source><translation>حد حرج %1 MB</translation></message>
    <message><source>Read-only</source><translation>للقراءة فقط</translation></message>
    <message><source>Read again</source><translation>قراءة مجدداً</translation></message>
    <message><source>Add exclusion</source><translation>إضافة استثناء</translation></message>
    <message><source>No matching items</source><translation>لا توجد عناصر مطابقة</translation></message>
    <message><source>Try another search term.</source><translation>جرّب عبارة بحث أخرى.</translation></message>
    <message><source>Navigate to registry path</source><translation>الانتقال إلى مسار السجل</translation></message>
    <message><source>Copy path</source><translation>نسخ المسار</translation></message>
    <message><source>Minimize</source><translation>تصغير</translation></message>
    <message><source>Maximize</source><translation>تكبير</translation></message>
    <message><source>Quick access</source><translation>الوصول السريع</translation></message>
    <message><source>This folder is empty</source><translation>هذا المجلد فارغ</translation></message>
    <message><source>Choose a location to save the file.</source><translation>اختر موقعاً لحفظ الملف.</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>أضف إعدادات UWF من برنامج نصي أو نص.</translation></message>
    <message><source>Edit commands</source><translation>تحرير الأوامر</translation></message>
    <message><source>Import results</source><translation>نتائج الاستيراد</translation></message>
    <message><source>Imported</source><translation>مستورد</translation></message>
    <message><source>Line</source><translation>السطر</translation></message>
    <message><source>Continue editing</source><translation>متابعة التحرير</translation></message>
    <message><source>Pending changes</source><translation>التغييرات المعلقة</translation></message>
    <message><source>Full configuration</source><translation>الإعدادات الكاملة</translation></message>
    <message><source>Apply changes</source><translation>تطبيق التغييرات</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>حُفظت جميع التغييرات على القرص وستبقى دون إعادة تشغيل.</translation></message>
    <message><source>Service details</source><translation>تفاصيل الخدمة</translation></message>
    <message><source>Background service</source><translation>خدمة الخلفية</translation></message>
    <message><source>Automatic file staging</source><translation>الحفظ التلقائي للملفات</translation></message>
    <message><source>Running</source><translation>قيد التشغيل</translation></message>
    <message><source>Stopped</source><translation>متوقف</translation></message>
    <message><source>Repair required</source><translation>يلزم الإصلاح</translation></message>
    <message><source>Account</source><translation>الحساب</translation></message>
    <message><source>Application path</source><translation>مسار التطبيق</translation></message>
    <message><source>Overview</source><translation>نظرة عامة</translation></message>
    <message><source>Raw report</source><translation>التقرير الأصلي</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>لقطة تشخيصية للنظام والصلاحيات وUWF.</translation></message>
    <message><source>System</source><translation>النظام</translation></message>
    <message><source>Architecture</source><translation>بنية النظام</translation></message>
    <message><source>System memory</source><translation>ذاكرة النظام</translation></message>
    <message><source>All levels</source><translation>جميع المستويات</translation></message>
    <message><source>Search messages or modules</source><translation>البحث في الرسائل أو الوحدات</translation></message>
    <message><source>No logs yet</source><translation>لا توجد سجلات بعد</translation></message>
    <message><source>Unable to read logs</source><translation>تعذرت قراءة السجلات</translation></message>
    <message><source>Error details</source><translation>تفاصيل الخطأ</translation></message>
    <message><source>Cancel reading</source><translation>إلغاء القراءة</translation></message>
    <message><source>Clear…</source><translation>مسح…</translation></message>
    <message><source>Selected file</source><translation>الملف المحدد</translation></message>
    <message><source>No overlay files found</source><translation>لم يُعثر على ملفات في التراكب</translation></message>
    <message><source>Done</source><translation>تم</translation></message>
    <message><source>License and copyright</source><translation>الترخيص وحقوق النشر</translation></message>
    <message><source>Source code</source><translation>الشيفرة المصدرية</translation></message>
    <message><source>Contact author</source><translation>التواصل مع المطوّر</translation></message>
    <message><source>Copy version</source><translation>نسخ الإصدار</translation></message>
    <message><source>Copy</source><translation>انسخ</translation></message>
    <message><source>Select All</source><translation>حدّد الكلّ</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>لا يمكن تغيير النوع والحجم الأقصى أثناء تفعيل عامل التصفية. عطّل عامل التصفية وأعد التشغيل أولاً.</translation></message>
    <message><source>Disk</source><translation>القرص</translation></message>
    <message><source>Protection and overlay</source><translation>الحماية والتراكب</translation></message>
    <message><source>Write protection</source><translation>الحماية من الكتابة</translation></message>
    <message><source>Overlay type</source><translation>نوع التراكب</translation></message>
    <message><source>Storage</source><translation>التخزين</translation></message>
    <message><source>Overlay maximum size</source><translation>الحجم الأقصى للتراكب</translation></message>
    <message><source>Volume %1 protection</source><translation>حماية وحدة التخزين %1</translation></message>
    <message><source>By volume ID</source><translation>حسب معرّف وحدة التخزين</translation></message>
    <message><source>By drive letter</source><translation>حسب حرف محرك الأقراص</translation></message>
    <message><source>Volume %1 binding</source><translation>ربط وحدة التخزين %1</translation></message>
    <message><source>Volume identity</source><translation>هوية وحدة التخزين</translation></message>
    <message><source>Removed</source><translation>أُزيل</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>لن يحمي UWF عمليات الكتابة اللاحقة إلى هذا المسار.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>إزالة هذا المسار من استثناءات الجلسة التالية.</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>لن يحمي UWF عمليات الكتابة اللاحقة إلى هذا المفتاح.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>إزالة هذا المفتاح من استثناءات الجلسة التالية.</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>الحفظ الدائم للمفتاح السري للمجال</translation></message>
    <message><source>Terminal Services license persistence</source><translation>الحفظ الدائم لترخيص خدمات المحطة الطرفية</translation></message>
    <message><source>Original command</source><translation>الأمر الأصلي</translation></message>
    <message><source>Processing result</source><translation>نتيجة المعالجة</translation></message>
    <message><source>Error</source><translation>خطأ</translation></message>
    <message><source>Warning</source><translation>تحذير</translation></message>
    <message><source>Information</source><translation>معلومات</translation></message>
    <message><source>Debug</source><translation>تصحيح الأخطاء</translation></message>
    <message><source>Module</source><translation>الوحدة</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>تعذّر مسح السجلات: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>تعذّر مسح السجلات: خطأ غير متوقع.</translation></message>
    <message><source>Name and location</source><translation>الاسم والموقع</translation></message>
    <message><source>System metadata</source><translation>بيانات النظام الوصفية</translation></message>
    <message><source>Date modified</source><translation>تاريخ التعديل</translation></message>
    <message><source>Folder</source><translation>مجلد</translation></message>
    <message><source>File</source><translation>ملف</translation></message>
    <message><source>Unable to read this folder.</source><translation>تعذّرت قراءة هذا المجلد.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>مجلد الوجهة غير موجود.</translation></message>
    <message><source>Enter a valid file name.</source><translation>أدخل اسم ملف صالحاً.</translation></message>
    <message><source>The selected item no longer exists: %1</source><translation>العنصر المحدد لم يعد موجوداً: %1</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>حدّد ملفات فقط، أو افتح مجلداً لاستعراض محتوياته.</translation></message>
    <message><source>Select a folder.</source><translation>حدّد مجلداً.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>أدخل اسم مجلد صالحاً.</translation></message>
    <message><source>Commit complete</source><translation>اكتمل الحفظ الدائم</translation></message>
    <message><source>Review the result for each target.</source><translation>راجع نتيجة كل عنصر مستهدف.</translation></message>

    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>لم تُحفظ العناصر غير الناجحة. حدّد صفاً لعرض المسار الكامل والسبب.</translation></message>
    <message><source>Item details</source><translation>تفاصيل العنصر</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>راجع هذه التغييرات قبل كتابتها إلى النظام.</translation></message>
    <message><source>Takes effect after restart</source><translation>يسري بعد إعادة التشغيل</translation></message>
    <message><source>View commands for these changes</source><translation>عرض أوامر هذه التغييرات</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>يطابق التكوين إعدادات الجلسة التالية المستهدفة. اعرض الأوامر أو صدّرها من التكوين الكامل.</translation></message>
    <message><source>Next-session configuration</source><translation>تكوين الجلسة التالية</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>احفظ برنامج CMD النصي هذا بترميز UTF-8 في ملف .cmd وشغّله في موجه الأوامر. يستخدم التصدير البرنامج النصي نفسه.</translation></message>
    <message><source>Export commands</source><translation>تصدير الأوامر</translation></message>
    <message><source>Direct restart…</source><translation>إعادة تشغيل مباشرة…</translation></message>
    <message><source>Some changes could not be applied</source><translation>تعذّر تطبيق بعض التغييرات</translation></message>
    <message><source>Configuration applied</source><translation>طُبّق التكوين</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>راجع تفاصيل العملية. اقرأ حالة النظام الحالية قبل إعادة محاولة التغييرات التي ربما كُتبت بالفعل.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>كُتب التكوين إلى النظام. تسري معظم التغييرات بعد إعادة التشغيل.</translation></message>
    <message><source>Application result</source><translation>نتيجة التطبيق</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>هل تريد كتابة تغييرات التكوين هذه إلى النظام؟ تسري معظم التغييرات بعد إعادة التشغيل.</translation></message>
    <message><source>Add files or folders</source><translation>إضافة ملفات أو مجلدات</translation></message>
    <message><source>Overlay usage</source><translation>استخدام التراكب</translation></message>
    <message><source>Import configuration</source><translation>استيراد التكوين</translation></message>
    <message><source>One uwfmgr command per line</source><translation>أمر uwfmgr واحد في كل سطر</translation></message>
    <message><source>uwfmgr commands</source><translation>أوامر uwfmgr</translation></message>
    <message><source>Editable</source><translation>قابل للتحرير</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>الصق الأوامر، أو حمّلها من ملف…</translation></message>
    <message><source>Import creates pending changes. Use Review and apply to write them to the system.</source><translation>ينشئ الاستيراد تغييرات معلّقة. استخدم المراجعة والتطبيق لكتابتها إلى النظام.</translation></message>
    <message><source>Imported commands still need to be applied. Select a row to view the complete result.</source><translation>لا تزال الأوامر المستوردة بحاجة إلى التطبيق. حدّد صفاً لعرض النتيجة الكاملة.</translation></message>
    <message><source>Not written to the system</source><translation>لم يُكتب إلى النظام</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 من الأسطر</translation></message>
    <message><source>Imported: %1</source><translation>تم الاستيراد: %1</translation></message>
    <message><source>Unprotected</source><translation>غير محمي</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>الوضع المتقدم غير متاح. استخدم إيقاف التشغيل الآمن أو إعادة التشغيل الآمنة لحفظ الملفات المرحّلة.</translation></message>
    <message><source>Version %1</source><translation>الإصدار %1</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>مدير رسومي لعامل تصفية الكتابة الموحد في Windows.
إدارة حماية وحدات التخزين والتراكب والاستثناءات والملفات المرحّلة.</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>يوفر Windows UWF الحماية من الكتابة. يتولى هذا التطبيق تكوينها وإدارتها.</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>تغييرات النظام عند تفعيل UWF لأول مرة</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;يعطّل Windows ملفات ترحيل الصفحات واستعادة النظام وSuperFetch وفهرسة الملفات وإلغاء تجزئة القرص وبدء التشغيل السريع. كما يضبط bootstatuspolicy في BCD على ignoreallfailures.&lt;/p&gt;&lt;p&gt;يمكنك تعديل هذه الإعدادات لاحقاً. على سبيل المثال، انقل ملف ترحيل الصفحات إلى وحدة تخزين غير محمية قبل تفعيل ترحيل الصفحات مجدداً.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;صدر بموجب &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;رخصة GNU العمومية العامة، الإصدار 3.0&lt;/a&gt;. تتوفر الشروط الكاملة في ملف LICENSE المرفق بالتطبيق.&lt;/p&gt;&lt;p&gt;يمكنك إعادة توزيعه أو تعديله بموجب GPL v3. يُقدَّم البرنامج كما هو، دون أي ضمان.&lt;/p&gt;</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>عرض سجلات التطبيق ومعلومات التشخيص.</translation></message>
    <message><source>Log level</source><translation>مستوى السجل</translation></message>
    <message><source>Current run</source><translation>التشغيل الحالي</translation></message>
    <message><source>Search logs</source><translation>البحث في السجلات</translation></message>
    <message><source>Message details</source><translation>تفاصيل الرسالة</translation></message>
    <message><source>Clear logs?</source><translation>هل تريد مسح السجلات؟</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>ستُمسح السجلات الحالية ولن يمكن استعادتها.</translation></message>
    <message><source>Clear logs</source><translation>مسح السجلات</translation></message>
    <message><source>Reading logs</source><translation>جارٍ قراءة السجلات</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>ستظهر سجلات التشغيل الحالي هنا عند اكتمال القراءة.</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>فشلت قراءة السجلات. أعد المحاولة أو تحقّق من صلاحية الوصول إلى ملف السجل.</translation></message>
    <message><source>No matching logs</source><translation>لا توجد سجلات مطابقة</translation></message>
    <message><source>Try another keyword or log level.</source><translation>جرّب كلمة بحث أو مستوى سجل آخر.</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>ستظهر السجلات الجديدة بعد التحديث.</translation></message>
    <message><source>Clear filters</source><translation>مسح عوامل التصفية</translation></message>
    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>قد يحتوي هذا التقرير على أسماء الحسابات ومعرّفات الأجهزة ومسارات الملفات. راجعه قبل مشاركته.</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>تعذّرت قراءة بعض معلومات العتاد. تظل معلومات التشخيص الأخرى متاحة للعرض والنسخ.</translation></message>
    <message><source>Copy full report</source><translation>نسخ التقرير الكامل</translation></message>
    <message><source>Service stopped</source><translation>الخدمة متوقفة</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>التنسيق مع إيقاف تشغيل Windows</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>حفظ الملفات المرحّلة المؤهلة تلقائياً أثناء إيقاف التشغيل وإعادة التشغيل العاديين.</translation></message>
    <message><source>Starts automatically with Windows</source><translation>تبدأ تلقائياً مع Windows</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>تحفظ الملفات المرحّلة قبل إيقاف التشغيل وإعادة التشغيل العاديين</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>بدون الوضع المتقدم، استخدم إيقاف التشغيل الآمن أو إعادة التشغيل الآمنة في هذا التطبيق لحفظ الملفات المرحّلة.</translation></message>
    <message><source>Operation and security</source><translation>التشغيل والأمان</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>يضيف الوضع المتقدم خدمة تعمل في الخلفية وتبدأ مع Windows. أزِل الخدمة قبل نقل التطبيق أو حذفه.</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>احتفظ بالتطبيق في مجلد ثابت لا يستطيع المستخدمون العاديون تعديله. الموقع المقترح:</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>إذا كان UWF يحمي محرك الأقراص الذي يحتوي على التطبيق، فتأكد أولًا من بقاء ملفات التطبيق بعد إعادة التشغيل. قد يستغرق إيقاف التشغيل أو إعادة التشغيل وقتًا أطول عند حفظ عدد كبير من الملفات المرحّلة بشكل دائم.</translation></message>
    <message><source>Configuration error</source><translation>خطأ في التكوين</translation></message>
    <message><source>Service status</source><translation>حالة الخدمة</translation></message>
    <message><source>Not installed</source><translation>غير مثبّتة</translation></message>
    <message><source>Service name</source><translation>اسم الخدمة</translation></message>
    <message><source>Startup type</source><translation>نوع بدء التشغيل</translation></message>
    <message><source>Automatic</source><translation>تلقائي</translation></message>
    <message><source>Not configured</source><translation>غير مكوّن</translation></message>
    <message><source>UI agent</source><translation>وكيل واجهة المستخدم</translation></message>
    <message><source>Connected and authenticated</source><translation>متصل وتم التحقق من هويته</translation></message>
    <message><source>Operating system</source><translation>نظام التشغيل</translation></message>
    <message><source>Permissions and protection</source><translation>الأذونات والحماية</translation></message>
    <message><source>Privileges</source><translation>الصلاحيات</translation></message>
    <message><source>Standard user</source><translation>مستخدم عادي</translation></message>
    <message><source>UWF feature</source><translation>ميزة UWF</translation></message>
    <message><source>Available</source><translation>متاح</translation></message>
    <message><source>Current filter</source><translation>عامل التصفية الحالي</translation></message>
    <message><source>Protected volumes</source><translation>وحدات التخزين المحمية</translation></message>
    <message><source>None</source><translation>لا يوجد</translation></message>
    <message><source>Connecting…</source><translation>جارٍ الاتصال…</translation></message>
    <message><source>Not connected</source><translation>غير متصل</translation></message>
    <message><source>Service account</source><translation>حساب الخدمة</translation></message>
    <message><source>Service configuration needs repair</source><translation>يحتاج تكوين الخدمة إلى إصلاح</translation></message>
    <message><source>Retry</source><translation>إعادة المحاولة</translation></message>
    <message><source>Status unavailable</source><translation>الحالة غير متاحة</translation></message>
    <message><source>Unable to read service status</source><translation>تعذّرت قراءة حالة الخدمة</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>عرض لقطة تشخيص الملفات لوحدة التخزين %1.</translation></message>
    <message><source>Local disk (%1)</source><translation>القرص المحلي (%1)</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>قد تُسقط القائمة بعض الملفات أو تتضمن ملفات حُفظت بالفعل. لا تستخدمها قائمةً للحفظ الدائم.</translation></message>
    <message><source>About this snapshot</source><translation>حول هذه اللقطة</translation></message>
    <message><source>Commit changes…</source><translation>حفظ التغييرات…</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>بيانات النظام الوصفية للتشخيص فقط ولا يمكن حفظها دائماً.</translation></message>
    <message><source>Error details and suggestions</source><translation>تفاصيل الخطأ والاقتراحات</translation></message>
    <message><source>Export list</source><translation>تصدير القائمة</translation></message>
    <message><source>Reading overlay</source><translation>جارٍ قراءة التراكب</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>تتعذّر قراءة التراكب حالياً</translation></message>
    <message><source>Reading canceled</source><translation>أُلغيت القراءة</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>قد تستغرق قراءة تراكب كبير وقتاً أطول. يمكنك الإلغاء في أي وقت.</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>تعذّر على موفّر UWF إكمال التعداد. قد يحدث ذلك مع تراكب كبير أو عند عدم كفاية موارد النظام.</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>اقرأ مجدداً للحصول على لقطة تشخيصية.</translation></message>
    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>لا تعني لقطة التشخيص الفارغة أن التراكب لا يحتوي على تغييرات.</translation></message>
    <message><source>%1 items · %2</source><translation>%1 من العناصر · %2</translation></message>
    <message><source>Selected folder</source><translation>المجلد المحدد</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>لن تُحفظ الملفات الموجودة في قسم الملفات المرحّلة.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>حفظ الملفات المرحّلة إلى القرص، ثم إيقاف التشغيل.</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>حفظ الملفات المرحّلة إلى القرص، ثم إعادة التشغيل.</translation></message>
    <message><source>staged files</source><translation>من الملفات المرحّلة</translation></message>
    <message><source>View failure details</source><translation>عرض تفاصيل الفشل</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>تُحفظ الملفات المرحّلة فقط تلقائياً. ستُفقد التغييرات الأخرى غير المحفوظة في المسارات المحمية بعد إيقاف التشغيل.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>تُحفظ الملفات المرحّلة فقط تلقائياً. ستُفقد التغييرات الأخرى غير المحفوظة في المسارات المحمية بعد إعادة التشغيل.</translation></message>
    <message><source>Ready to save</source><translation>جاهز للحفظ</translation></message>
    <message><source>No files to commit</source><translation>لا توجد ملفات للحفظ الدائم</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>قد تستغرق قوائم الملفات المرحّلة الكبيرة وقتاً أطول للحفظ.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>سيُنفَّذ إيقاف التشغيل الآمن لـ UWF مباشرةً.</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>ستُنفَّذ إعادة التشغيل الآمنة لـ UWF مباشرةً.</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 من الملفات</translation></message>
    <message><source>Saving staged files</source><translation>جارٍ حفظ الملفات المرحّلة</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>سيستمر إيقاف التشغيل بعد الحفظ. أبقِ التطبيق مفتوحاً.</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>ستستمر إعادة التشغيل بعد الحفظ. أبقِ التطبيق مفتوحاً.</translation></message>
    <message><source>Saving to disk</source><translation>جارٍ الحفظ إلى القرص</translation></message>
    <message><source>Saving…</source><translation>جارٍ الحفظ…</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>بدأ حفظ الملفات ولا يمكن إلغاؤه.</translation></message>
    <message><source>Some files could not be saved</source><translation>تعذّر حفظ بعض الملفات</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>راجع تفاصيل الفشل قبل متابعة إيقاف التشغيل.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>راجع تفاصيل الفشل قبل إعادة التشغيل.</translation></message>
    <message><source>File saving incomplete</source><translation>حفظ الملفات غير مكتمل</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>ستؤدي متابعة إيقاف التشغيل إلى فقدان التغييرات في الملفات التي تعذّر حفظها.</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>ستؤدي متابعة إعادة التشغيل إلى فقدان التغييرات في الملفات التي تعذّر حفظها.</translation></message>
    <message><source>Shut down anyway</source><translation>إيقاف التشغيل على أي حال</translation></message>
    <message><source>Restart anyway</source><translation>إعادة التشغيل على أي حال</translation></message>
    <message><source>Counting staged files…</source><translation>جارٍ عدّ الملفات المرحّلة…</translation></message>
    <message><source>Scanning staged paths</source><translation>جارٍ فحص المسارات المرحّلة</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>حدّد مفتاحاً للعمل على شجرته الفرعية، أو قيمةً مسمّاة للعمل على تلك القيمة فقط.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>اختر مفتاح السجل المراد استثناؤه من حماية UWF، بما في ذلك مفاتيحه الفرعية وقيمه.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>أدخل HKLM\SOFTWARE\… واضغط Enter للانتقال</translation></message>
    <message><source>Unable to access this registry key</source><translation>تعذّر الوصول إلى مفتاح السجل هذا</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>اختر مفتاحاً آخر، أو تحقّق من أذونات الوصول وأعد المحاولة.</translation></message>
    <message><source>Back</source><translation>رجوع</translation></message>
    <message><source>Up one folder</source><translation>المجلد الأعلى</translation></message>
    <message><source>Refresh folder</source><translation>تحديث المجلد</translation></message>
    <message><source>Folder address</source><translation>عنوان المجلد</translation></message>
    <message><source>Search this folder</source><translation>البحث في هذا المجلد</translation></message>
    <message><source>This PC</source><translation>هذا الكمبيوتر</translation></message>
    <message><source>Folders</source><translation>المجلدات</translation></message>
    <message><source>Files and folders</source><translation>الملفات والمجلدات</translation></message>
    <message><source>New folder</source><translation>مجلد جديد</translation></message>
    <message><source>Selection</source><translation>التحديد</translation></message>
    <message><source>File name</source><translation>اسم الملف</translation></message>
    <message><source>Select the current folder</source><translation>تحديد المجلد الحالي</translation></message>
    <message><source>Select files or folders</source><translation>تحديد ملفات أو مجلدات</translation></message>
    <message><source>Enter a file name</source><translation>أدخل اسم ملف</translation></message>
    <message><source>Select a file or enter its name</source><translation>حدّد ملفاً أو أدخل اسمه</translation></message>
    <message><source>File type</source><translation>نوع الملف</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>يمكن تحديد الملفات والمجلدات معاً.</translation></message>
    <message><source>Select folders only.</source><translation>حدّد مجلدات فقط.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>يمكن تحديد ملفات متعددة.</translation></message>
    <message><source>Select a file to open.</source><translation>حدّد ملفاً لفتحه.</translation></message>
    <message><source>Save</source><translation>حفظ</translation></message>
    <message><source>Add selected items</source><translation>إضافة العناصر المحددة</translation></message>
    <message><source>Select folder</source><translation>تحديد مجلد</translation></message>
    <message><source>Open</source><translation>فتح</translation></message>
    <message><source>Unable to select this item</source><translation>تعذّر تحديد هذا العنصر</translation></message>
    <message><source>Reading folder</source><translation>جارٍ قراءة المجلد</translation></message>
    <message><source>Unable to access this folder</source><translation>تعذّر الوصول إلى هذا المجلد</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>جارٍ قراءة الملفات والمجلدات الفرعية في هذا الموقع…</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>انتقل إلى المجلد الأعلى أو اختر موقعاً آخر.</translation></message>
    <message><source>Replace the existing file?</source><translation>هل تريد استبدال الملف الموجود؟</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>يوجد ملف بالفعل في هذا الموقع:
%1

سيؤدي الحفظ إلى استبدال محتوياته.</translation></message>
    <message><source>Replace file</source><translation>استبدال الملف</translation></message>
    <message><source>Folder name</source><translation>اسم المجلد</translation></message>
    <message><source>Create</source><translation>إنشاء</translation></message>
    <message><source>No registry key selected</source><translation>لم يُحدَّد مفتاح سجل</translation></message>
    <message><source>Select an available subkey</source><translation>حدّد مفتاحاً فرعياً متاحاً</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>استثناء عام للسجل · يشمل المفاتيح الفرعية</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>القيمة المسمّاة: %1 · هذه القيمة فقط</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>المفتاح بالكامل وجميع مفاتيحه الفرعية · بشكل متكرر</translation></message>
    <message><source>Discard changes</source><translation>تجاهل التغييرات</translation></message>
    <message><source>Administrator</source><translation>المسؤول</translation></message>
    <message><source>Display language</source><translation>لغة العرض</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>حدّد المسارات المحددة للإزالة. تسري التغييرات بعد التطبيق.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>إزالة من قائمة تجهيز الملفات تلقائيًا بعد التطبيق.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>إضافة إلى قائمة تجهيز الملفات تلقائيًا بعد التطبيق.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>يؤدي تطبيق هذه الخطة إلى إفراغ قائمة تجهيز الملفات المحفوظة وإزالة استثناء السجل المرتبط بها عندما لا تعود هناك حاجة إليه.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>يجب تطبيق تغييرات تجهيز الملفات في UWF Manager ولا يمكن تصديرها كأوامر uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>تُحفظ قائمة تجهيز الملفات بعد التطبيق</translation></message>
    <message><source>Overlay snapshot scope</source><translation>نطاق لقطة التراكب</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>هذه القائمة للتشخيص فقط. لا يمكن استخدامها لتحديد الملفات التي يجب تثبيت تغييراتها.</translation></message>
    <message><source>NTFS volumes only</source><translation>وحدات تخزين NTFS فقط</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>قد تكون قراءة التراكبات الكبيرة بطيئة أو تفشل بسبب عدم كفاية الموارد.</translation></message>
    <message><source>Small files may be missing</source><translation>قد لا تظهر الملفات الصغيرة</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>قد لا تظهر الملفات الأصغر من حجم مجموعة القرص (عادةً 4 كيلوبايت) في القائمة.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>لا تشير إلى تغييرات تنتظر التثبيت</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>قد تظهر أيضًا الملفات المثبتة مسبقًا والمسارات المستثناة والعناصر المتأثرة بعمليات أخرى.</translation></message>

    <message><source>Command processing result</source><translation>نتيجة معالجة الأمر</translation></message>
    <message><source>Copy command</source><translation>نسخ الأمر</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · تعذرت قراءة %1 من الفروع</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>تعذرت قراءة %1 من الفروع. تابع بالمحتوى المكتشف أو أعد محاولة العناصر التي فشلت فقط.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 من الملفات · %2 من المجلدات</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 من القيم · %2 من المفاتيح</translation></message>
    <message><source>Attempts</source><translation>المحاولات</translation></message>
    <message><source>Cancel remaining tasks</source><translation>إلغاء المهام المتبقية</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>تثبيت المحتوى الذي تم استعراضه بنجاح فقط. تبقى الفروع غير المقروءة دون تغيير.</translation></message>
    <message><source>Committing changes</source><translation>جارٍ تثبيت التغييرات</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>لا يمكن التراجع عن عمليات الحذف والتثبيت المكتملة.</translation></message>
    <message><source>Continue…</source><translation>متابعة…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>حذف الأهداف المكتشفة وتثبيتها فقط. تُحفظ الفروع غير المقروءة والمجلدات أو المفاتيح الأصلية التي تحتويها.</translation></message>
    <message><source>Deleting and committing</source><translation>جارٍ الحذف والتثبيت</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>لا يمكن التراجع عن الحذف والتثبيت. يُعالج النطاق المحدد فقط ولا يُطبّق إعداد UWF المعلّق.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>جارٍ تحديد النطاق. تُسجّل الفروع غير المقروءة ويستمر استعراض بقية المحتوى.</translation></message>
    <message><source>Export traversal records</source><translation>تصدير سجل الاستعراض</translation></message>
    <message><source>No actionable content</source><translation>لا يوجد محتوى قابل للمعالجة</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>لم يُعثر على أهداف قابلة للمعالجة. لم يُحذف أو يُثبّت أي محتوى.</translation></message>
    <message><source>No changes have been committed.</source><translation>لم تُثبّت أي تغييرات بعد</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>لم يُحذف أو يُثبّت أي محتوى</translation></message>
    <message><source>Path</source><translation>المسار</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>حذف النطاق المحدد نهائيًا وحفظ عمليات الحذف على وحدة التخزين المحمية.</translation></message>
    <message><source>Preparation complete</source><translation>اكتمل الإعداد</translation></message>
    <message><source>Preparing commit</source><translation>جارٍ إعداد التثبيت</translation></message>
    <message><source>Preparing deletion and commit</source><translation>جارٍ إعداد الحذف والتثبيت</translation></message>
    <message><source>Process only successfully traversed content</source><translation>معالجة المحتوى الذي تم استعراضه بنجاح فقط</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>يتم تخطي نقاط إعادة التحليل للحفاظ على الاستعراض ضمن النطاق المحدد.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>لا تُلغي إعادة التشغيل التثبيت. يُعالج النطاق المحدد فقط ولا يُطبّق إعداد UWF المعلّق.</translation></message>
    <message><source>Retry failed items</source><translation>إعادة محاولة العناصر الفاشلة</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>حفظ تغييرات النطاق المحدد على وحدة التخزين المحمية.</translation></message>
    <message><source>Stopping the operation</source><translation>جارٍ إيقاف العملية</translation></message>
    <message><source>Successful and failed records are retained</source><translation>حُفظت سجلات النجاح والفشل</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>المجلد غير فارغ؛ تم الاحتفاظ بالمحتوى غير المعالج.</translation></message>
    <message><source>The operation could not be completed</source><translation>تعذر إكمال العملية</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>يحتوي مفتاح السجل على قيم جديدة؛ تم الاحتفاظ بالمحتوى غير المعالج.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>لا يزال مفتاح السجل يحتوي على مفاتيح فرعية؛ تم الاحتفاظ بالمحتوى غير المعالج.</translation></message>
    <message><source>The target no longer exists.</source><translation>لم يعد الهدف موجودًا.</translation></message>
    <message><source>Traversal complete</source><translation>اكتمل الاستعراض</translation></message>
    <message><source>Traversal completed with unread content</source><translation>اكتمل الاستعراض مع وجود محتوى غير مقروء</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>تم الاحتفاظ بسجل الاستعراض ويمكن تصديره.</translation></message>
    <message><source>Traverse again</source><translation>استعراض مجددًا</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>استعرض مجددًا أو أغلق هذه النافذة.</translation></message>
    <message><source>Traversing the folder</source><translation>جارٍ استعراض المجلد</translation></message>
    <message><source>Traversing the registry</source><translation>جارٍ استعراض السجل</translation></message>
    <message><source>Unable to prepare the operation</source><translation>تعذر إعداد العملية</translation></message>
    <message><source>View results</source><translation>عرض النتائج</translation></message>

    <message><source>All changes committed</source><translation>تم تثبيت جميع التغييرات</translation></message>
    <message><source>Some changes committed</source><translation>تم تثبيت بعض التغييرات</translation></message>
    <message><source>No changes could be committed</source><translation>تعذر تثبيت أي تغييرات</translation></message>
    <message><source>No changes to commit</source><translation>لا توجد تغييرات لتثبيتها</translation></message>
    <message><source>Commit interrupted</source><translation>انقطعت عملية التثبيت</translation></message>
    <message><source>%1 items were not processed.</source><translation>لم تتم معالجة %1 من العناصر.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>فشلت %1 عملية في السجل.</translation></message>
    <message><source>The folder does not exist.</source><translation>المجلد غير موجود.</translation></message>
    <message><source>Access denied.</source><translation>تم رفض الوصول.</translation></message>
    <message><source>An item with this name already exists.</source><translation>يوجد عنصر بهذا الاسم بالفعل.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>تعذر إنشاء المجلد. تحقق من الاسم والأذونات.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>تعذرت إعادة تسمية العنصر. تحقق من الاسم والأذونات.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>لا يدعم هذا المستعرض إعادة التسمية.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>تغيرت مسودة قائمة الملفات المجهزة بعد فتح هذه الخطة. راجع خطة جديدة.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>تغيرت قائمة الملفات المجهزة المحفوظة بعد فتح هذه الخطة. حدّث البيانات وراجع التغييرات.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>قائمة الملفات المجهزة المحفوظة لا تطابق الخطة المعتمدة.</translation></message>
    <message><source>Export configuration</source><translation>تصدير الإعدادات</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>احفظ السمة واللغة وحالة إظهار HUB الحالية في ملف UWF.ini بجوار هذا التطبيق.</translation></message>
    <message><source>Configuration saved</source><translation>تم حفظ الإعدادات</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>تم حفظ إعدادات الواجهة الحالية في:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>ملف UWF.ini موجود بالفعل. لم يتم تغيير الملف الموجود:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>تعذّر حفظ UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>سطح المكتب</translation></message>
    <message><source>Downloads</source><translation>التنزيلات</translation></message>
    <message><source>Documents</source><translation>المستندات</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>توقّف الحفظ التلقائي للملفات. عالج الخطأ قبل المحاولة مرة أخرى.</translation></message>
</context>
</TS>
