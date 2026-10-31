<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="uk" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Попередній перегляд</translation></message>
    <message><source>enable</source><translation>увімкнути</translation></message>
    <message><source>disable</source><translation>вимкнути</translation></message>
    <message><source>Enabled</source><translation>Увімкнено</translation></message>
    <message><source>Disabled</source><translation>Вимкнено</translation></message>
    <message><source>OK</source><translation>Гаразд</translation></message>
    <message><source>Cancel</source><translation>Скасувати</translation></message>
    <message><source>Close</source><translation>Закрити</translation></message>
    <message><source>Refresh</source><translation>Оновити</translation></message>
    <message><source>Add</source><translation>Додати</translation></message>
    <message><source>Remove</source><translation>Прибрати</translation></message>
    <message><source>Yes</source><translation>Так</translation></message>
    <message><source>No</source><translation>Ні</translation></message>
    <message><source>Excluded</source><translation>Виключено</translation></message>
    <message><source>Not excluded</source><translation>Не виключено</translation></message>
    <message><source>File…</source><translation>Файл…</translation></message>
    <message><source>Folder…</source><translation>Папка…</translation></message>
    <message><source>volume ID</source><translation>ідентифікатор тому</translation></message>
    <message><source>drive letter</source><translation>буква диска</translation></message>
    <message><source>Drive letter</source><translation>Буква диска</translation></message>
    <message><source>Volume ID</source><translation>Ідентифікатор тому</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Стан захисту тому в поточному сеансі (лише читання).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Захищати том у наступному сеансі. Записи перенаправляються до накладання й відкидаються під час перезавантаження.</translation></message>
    <message><source>Current session</source><translation>Поточний сеанс</translation></message>
    <message><source>Next session</source><translation>Наступний сеанс</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Спосіб розпізнавання тому. Буква диска простіша, але прив’язка втрачається при її зміні, наприклад після додавання чи вилучення дисків. Ідентифікатор тому залишається сталим.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Том не підтримується UWF: </translation></message>
    <message><source>Global settings</source><translation>Загальні налаштування</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Стан фільтра UWF у поточному сеансі (лише читання).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Увімкнути UWF у наступному сеансі. Записи до захищених томів ідуть у накладання й відкидаються під час перезавантаження.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Щоб змінити тип і максимальний розмір:
1. Вимкніть фільтр вище.
2. Перезавантажте систему, щоб вимкнення набуло чинності.
3. Змініть налаштування.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Сховище накладання: RAM швидша, але займає пам’ять; Disk використовує системний диск і дає більше місця. Обидва очищаються під час перезапуску.</translation></message>
    <message><source>Type</source><translation>Тип</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Максимальна місткість накладання. RAM обмежена обсягом пам’яті. Для Disk потрібно щонайменше 1024 MB і достатньо вільного місця на системному томі.</translation></message>
    <message><source>Maximum size</source><translation>Максимальний розмір</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Максимальний розмір · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Поріг попередження</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Записує попередження до журналу подій, коли обсяг зайнятого місця в оверлеї досягає цього значення. Якщо обидві події ввімкнено, це значення має бути нижчим за критичний поріг. Значення 0 вимикає лише подію попередження.</translation></message>
    <message><source>Critical threshold</source><translation>Критичний поріг</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Записує критичне сповіщення до журналу подій, коли обсяг зайнятого місця в оверлеї досягає цього значення. Якщо обидві події ввімкнено, це значення має бути вищим за поріг попередження. Значення 0 вимикає лише критичну подію.</translation></message>
    <message><source>Overlay</source><translation>Накладання</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Непідтримуваний тип диска (лише локальні фіксовані диски).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Том перевищує ліміт UWF: 16 TiB на захищений том.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: захист тому можливий, але виключення файлів і пофайлова фіксація не підтримуються.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Помилка: Читання відомостей про том.</translation></message>
    <message><source>%1 free / %2</source><translation>Вільно %1 / %2</translation></message>
    <message><source>Commit</source><translation>Зафіксувати</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Зафіксувати зміни файлу з накладання на диску / Зафіксувати зміни реєстру. Цю дію неможливо скасувати.</translation></message>
    <message><source>Commit file changes…</source><translation>Зафіксувати зміни файлу…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Вибрати файл для фіксації на диску. Зафіксувати зміни файлу з накладання на диску.</translation></message>
    <message><source>Commit folder changes…</source><translation>Зафіксувати зміни папки…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Вибрати папку для фіксації на диску. Зафіксувати зміни папки з накладання на диску. Включно з усіма вкладеними папками.</translation></message>
    <message><source>Delete and commit file…</source><translation>Видалити файл і зафіксувати видалення на диску…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Вибрати файл для фіксації видалення. Видалити файл і зафіксувати видалення на диску.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Видалити папку з усім вмістом і зафіксувати видалення на диску…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Вибрати папку для фіксації видалення. Видалити папку з усім вмістом і зафіксувати видалення на диску. Включно з усіма вкладеними папками.</translation></message>
    <message><source>Commit registry changes…</source><translation>Зафіксувати зміни реєстру…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Введіть шлях ключа й необов’язкове ім’я значення. Якщо ім’я порожнє, фіксуються всі значення ключа та всіх підрозділів.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Зафіксувати видалення з реєстру…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Введіть ключ для видалення й необов’язкове ім’я значення. Порожнє ім’я означає видалення й фіксацію всього дерева: ключа, значень і підрозділів.</translation></message>
    <message><source>File exclusions</source><translation>Виключення файлів</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Файли й папки тому поза захистом UWF. Двічі клацніть запис, щоб скопіювати шлях.</translation></message>
    <message><source>Registry exclusions</source><translation>Виключення реєстру</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Виключення реєстру спільні й залежать від глобального фільтра UWF, а не захисту тому. Список спільний для всіх томів і показаний лише раз. Подвійне клацання копіює шлях.</translation></message>
    <message><source>File staging</source><translation>Автофіксація файлів</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>файли / папки: Автоматично фіксуються перед безпечним вимкненням або перезапуском. Включно з усіма вкладеними папками. Поки список не порожній, таке виключення реєстру зберігає його між захищеними сеансами: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Додати файли або папку до списку автофіксації.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Вибрати файл для фіксації на диску. Автоматично фіксуються перед безпечним вимкненням або перезапуском.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Вибрати папку для фіксації на диску. Автоматично фіксуються перед безпечним вимкненням або перезапуском. Включно з усіма вкладеними папками.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Вибрати файл для фіксації на диску · Автофіксація файлів</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Вибрати папку для фіксації на диску · Автофіксація файлів · Включно з усіма вкладеними папками.</translation></message>
    <message><source>Cannot stage this path</source><translation>Неможливо додати шлях до автофіксації</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Шлях перетинається з виключенням UWF. Спочатку вилучіть виключення:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Шлях перетинається з автофіксацією. Спочатку приберіть шлях зі списку:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Шлях перетинається із записом автофіксації</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Виключення реєстру потрібне, поки список автофіксації не порожній</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Шлях не існує або має неочікуваний тип:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Не вдалося перевірити шлях:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Точки повторної обробки не можна додавати до автофіксації:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Немає букви диска, фіксація UWF неможлива:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Шлях розташований на томі %1. Додайте його на вкладці автофіксації цього тому.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Корінь тому не можна додавати: це призвело б до рекурсивної фіксації всього тому.</translation></message>
    <message><source>File staging could not be saved</source><translation>Помилка: Збереження списку автофіксації</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Список автофіксації перезавантажено через збій оновлення:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Автофіксація файлів: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Автоматично фіксуються перед безпечним вимкненням або перезапуском.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Автофіксація файлів: %1 · файли: %2 · папки: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Автофіксація файлів потребує цього виключення реєстру, щоб зберігати свій список після перезавантажень.</translation></message>
    <message><source>Built-in</source><translation>Вбудований</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Фільтр UWF вимкнено. Зміни в накладанні не накопичувалися; фіксувати нічого.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Файлова система %1 не підтримує пофайлову фіксацію.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Цей том не підтримується UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Том не підтримує фіксацію файлів. Доступна лише фіксація реєстру (його виключення глобальні).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Файлова система %1 не підтримує пофайлову фіксацію. Доступна лише фіксація реєстру (його виключення глобальні).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Том зараз не захищений UWF; немає змін файлів для фіксації.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Том зараз не захищений UWF; немає змін файлів для фіксації. Доступна лише фіксація реєстру (його виключення глобальні).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Вибрати файл для фіксації на диску</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Вибрати папку для фіксації на диску</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Вибрати файл для фіксації видалення</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Вибрати папку для фіксації видалення</translation></message>
    <message><source>Name</source><translation>Ім’я</translation></message>
    <message><source>Data</source><translation>Дані</translation></message>
    <message><source>(Default)</source><translation>(Типове)</translation></message>
    <message><source>(value not set)</source><translation>(значення не задано)</translation></message>
    <message><source>(empty)</source><translation>(порожньо)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Невідомо (Значення: Файл завеликий)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Перегляд файлів накладання. &lt;b&gt;Лише діагностичний знімок.&lt;/b&gt; Не обирайте за ним дані для фіксації.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Список файлів накладання доступний лише на томах NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF вимкнено; жоден том не має накладання для перегляду.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Том не захищений UWF і не має накладання для перегляду.</translation></message>
    <message><source>Failed: %1</source><translation>Помилка: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Постачальник WMI аварійно завершив роботу. Це відома нестабільність UWF_Overlay.GetOverlayFiles за великого накладання чи високого навантаження введення-виведення.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Недостатньо пам’яті або операція не підтримується. Потрібні NTFS і запас пам’яті. Зменште накладання чи закрийте програми з великим споживанням пам’яті й повторіть.</translation></message>
    <message><source>Raw error: %1</source><translation>Код помилки: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Експорт у файл… · Накладання · файли</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Текстові файли (*.txt);;Усі файли (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Командні файли (*.cmd);;Усі файли (*)</translation></message>
    <message><source>Export failed</source><translation>Помилка експорту</translation></message>
    <message><source>Export finished</source><translation>Експорт завершено</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Помилка: Відкриття файлу (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Помилка: Запис файлу (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Конфігурація: Запис файлу · записи: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Не можна виключити том повністю (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Файли підкачки, swap і гібернації не можна виключати: від них залежить UWF.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Не можна виключити каталог повністю: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Не можна виключити каталог повністю: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Не можна виключити каталог повністю: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Не можна виключити критичний системний файл: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Файл реєстру користувача NTUSER.DAT не можна виключати.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Ключ реєстру не може бути порожнім.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Ключ реєстру не може починатися зі зворотної скісної риски.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Шлях містить послідовні зворотні скісні риски й неприпустимий.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Роздільник шляху реєстру — `\`, а не `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Шлях містить невидимі керувальні символи й неприпустимий.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>Документація UWF забороняє виключати секрет облікового запису комп’ютера домену: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Дозволені лише конкретні підрозділи таких кореневих ключів:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Додати файли чи папки до виключень. Вони не будуть захищені UWF.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Вибрати файл для додавання · Виключення файлів.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Вибрати папку для додавання · Виключення файлів. Включно з усіма вкладеними папками.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Введіть шлях ключа реєстру для виключення.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Введіть шлях ключа реєстру для виключення. / Збереження між сеансами.</translation></message>
    <message><source>Registry key…</source><translation>Реєстр · Ключ…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Секретний ключ домену (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Клієнтська ліцензія служб терміналів (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Зберігати секрет домену (пароль облікового запису комп’ютера) між сеансами UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Зберігати клієнтські ліцензії служб терміналів між сеансами UWF.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Збереження між сеансами %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Прибрати вибране</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Прибрати вибрані виключення. Набуває чинності після застосування.</translation></message>
    <message><source>Open containing folder</source><translation>Відкрити папку</translation></message>
    <message><source>Copy file path</source><translation>Копіювати шлях до файлу</translation></message>
    <message><source>Copy registry path</source><translation>Копіювати шлях реєстру</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Зафіксувати зміни папки з накладання на диску…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Зафіксувати зміни файлу з накладання на диску…</translation></message>
    <message><source>Copied to clipboard</source><translation>Скопійовано до буфера обміну</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Вибрати файл для додавання · Виключення файлів. Можна вибрати кілька файлів.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Вибрати папку для додавання · Виключення файлів</translation></message>
    <message><source>Add registry exclusion</source><translation>Додавання виключення реєстру</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Шлях %1 не на томі %2; його не можна додати до виключень цього тому.</translation></message>
    <message><source>Path is not on this volume</source><translation>Шлях не на цьому томі</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Неможливо додати виключення</translation></message>
    <message><source>File: %1</source><translation>Файл… %1</translation></message>
    <message><source>Folder: %1</source><translation>Папка… %1</translation></message>
    <message><source>Registry: %1</source><translation>Реєстр: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Виключено типово; можна змінити.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Поточний сеанс: %1
Наступний сеанс: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Зміни в очікуванні: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Диспетчер уніфікованого фільтра запису (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Перечитати стан поточного й конфігурацію наступного сеансу UWF.</translation></message>
    <message><source>Review and apply</source><translation>Перевірити й застосувати</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Перевірити всі зміни в очікуванні й застосувати одним пакетом. Більшість змін набуде чинності після наступного перезапуску.</translation></message>
    <message><source>Safe shutdown</source><translation>Безпечне вимкнення</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Система безпечно вимкнеться через UWF. Доступно навіть за повного накладання UWF.</translation></message>
    <message><source>Safe restart</source><translation>Безпечне перезавантаження</translation></message>
    <message><source>Direct restart</source><translation>Перезавантажити напряму</translation></message>
    <message><source>Direct shutdown</source><translation>Вимкнути напряму</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Система безпечно перезапуститься через UWF. Доступно навіть за повного накладання UWF.</translation></message>
    <message><source>Log</source><translation>Журнал</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Відкрити внутрішній журнал поточного сеансу для діагностики.</translation></message>
    <message><source>About</source><translation>Про програму</translation></message>
    <message><source>Switch display language</source><translation>Змінити мову інтерфейсу</translation></message>
    <message><source>Language</source><translation>Мова</translation></message>
    <message><source>Theme</source><translation>Тема</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Перемкнути світлу/темну тему</translation></message>
    <message><source>System information</source><translation>Відомості про систему</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Діагностичний знімок</translation></message>
    <message><source>Read only</source><translation>Лише читання</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Об’єкт не знайдено; у накладанні нічого фіксувати.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Об’єкт є лише в накладанні: створений під захистом UWF і ніколи не фіксувався на диску чи в постійному реєстрі. Зникне після перезапуску; фіксація видалення не потрібна й неможлива.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Система відхилила параметр: неправильний шлях або аргумент.</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Помилка операції. Об’єкт може використовувати інша програма, наприклад Провідник чи відкритий файл. Закрийте такі програми й повторіть.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Операцію не виконано; подробиці в журналі.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Операцію відхилено (код %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Невідомо.</translation></message>
    <message><source>Commit canceled</source><translation>Фіксацію скасовано</translation></message>

    <message><source>Succeeded</source><translation>Успішно</translation></message>
    <message><source>Skipped</source><translation>Пропущено</translation></message>
    <message><source>Failed</source><translation>Помилка</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Успішно: %1; Пропущено: %2; Помилка: %3.</translation></message>

    <message><source>Existed before</source><translation>Існував до</translation></message>
    <message><source>Exists after</source><translation>Існує після</translation></message>
    <message><source>Error code</source><translation>Код помилки</translation></message>
    <message><source>Reason</source><translation>Причина</translation></message>
    <message><source>Previous page</source><translation>Попередня сторінка</translation></message>
    <message><source>First page</source><translation>Перша сторінка</translation></message>
    <message><source>Last page</source><translation>Остання сторінка</translation></message>
    <message><source>Next page</source><translation>Наступна сторінка</translation></message>
    <message><source>Copy selected rows</source><translation>Копіювати вибрані рядки</translation></message>
    <message><source>Copy all</source><translation>Копіювати все</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Виключення реєстру потрібне, поки список автофіксації не порожній. Автофіксація файлів: Очистити → Вилучення виключення реєстру.</translation></message>
    <message><source>Copy current page</source><translation>Копіювати поточну сторінку</translation></message>
    <message><source>No pending changes</source><translation>Немає змін в очікуванні</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Виключення реєстру · Загальні налаштування.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Том %1: Захист UWF / Виключення файлів / Автофіксація файлів.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Не підтримується</translation></message>
    <message><source>Review and apply changes</source><translation>Перевірити й застосувати зміни</translation></message>
    <message><source>Confirm apply</source><translation>Підтвердити застосування</translation></message>
    <message><source>Result</source><translation>Результат</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Фільтр: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Задання порогу попередження: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Задання критичного порогу: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Задання типу накладання / Задання максимального розміру: Щоб змінити тип і максимальний розмір:
1. Вимкніть фільтр вище.
2. Перезавантажте систему, щоб вимкнення набуло чинності.
3. Змініть налаштування.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Задання типу накладання: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Задання максимального розміру: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Задання максимального розміру: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Том %1 · Захист UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Том %1 · Прив’язка за: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Том %1 · Додавання виключення файлу: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Том %1 · Вилучення виключення файлу: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Додавання виключення реєстру: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Вилучення виключення реєстру: %1</translation></message>
    <message><source>Time</source><translation>Час</translation></message>
    <message><source>Level</source><translation>Рівень</translation></message>
    <message><source>Tag</source><translation>Позначка</translation></message>
    <message><source>Message</source><translation>Повідомлення</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Підрахунок файлів для автофіксації…</translation></message>
    <message><source>Committing staged files</source><translation>Автофіксація файлів</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>Оброблено файлів: %1 із %2.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Додаткові відомості про помилку відсутні.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Автофіксація несподівано зупинилася:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Автофіксацію файлів не завершено: Невідома помилка.</translation></message>
    <message><source>File staging will be skipped</source><translation>Автофіксацію файлів буде пропущено</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Перезавантажити напряму: Автофіксацію файлів буде пропущено.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Вимкнути напряму: Автофіксацію файлів буде пропущено.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Автофіксацію пропущено через вибір прямої дії живлення.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Безпечне вимкнення: Помилка</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Вимкнути: Помилка (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Безпечне перезавантаження: Помилка</translation></message>
    <message><source>Direct restart failed</source><translation>Перезавантажити напряму: Помилка</translation></message>
    <message><source>Direct shutdown failed</source><translation>Вимкнути напряму: Помилка</translation></message>
    <message><source>Restart failed: %1</source><translation>Перезавантажити: Помилка (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Не вдалося прочитати список автофіксації:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Не вдалося прочитати список автофіксації: Невідома помилка.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>файли: %1 · Успішно: %2 · Пропущено (файли): %3 · Пропущено (записи): %4 · Помилка: %5</translation></message>
    <message><source>File staging preparation</source><translation>Автофіксація файлів: Подробиці операції</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Точки повторної обробки не підтримуються.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Можна додавати лише абсолютні шляхи на локальних томах.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Можна додавати лише абсолютні шляхи на локальних томах.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Не вдалося перевірити шлях автофіксації.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Шлях доданого файлу тепер указує на об’єкт іншого типу.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Шлях доданої папки тепер указує на об’єкт іншого типу.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Не вдалося повністю перелічити вміст папки.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Шлях не має букви диска.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>Не можна рекурсивно обробляти корінь тому.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Постачальник UWF відхилив фіксацію.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Помилка фіксації: Невідома помилка.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Додаткових помилок у журналі: %1.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Шлях не має букви диска. Визначення цільового тому: Помилка.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Немає запису поточного сеансу для тому %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Шлях у виключеннях файлів.
Виключення: %1</translation></message>

    <message><source>Continue</source><translation>Продовжити</translation></message>
    <message><source>Delete and commit</source><translation>Видалити й зафіксувати</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Ключ у виключеннях реєстру.
Виключення: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Немає запису фільтра реєстру для поточного сеансу.</translation></message>

    <message><source>Export commands to file</source><translation>Експортувати команди у файл</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Експорт у файл… · Команда: %1
%2</translation></message>
    <message><source>Import</source><translation>Імпорт</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Введіть або вставте команди, по одній у рядку. Завантажте текст, журнал чи скрипт (.txt, .bat, .ps1); рядки з цією командою буде додано. Імпорт ставить зміни в чергу. Запис до системи виконується через «Перевірити й застосувати» на панелі інструментів.</translation></message>
    <message><source>Load from file…</source><translation>Завантажити з файлу…</translation></message>
    <message><source>Load recommended configuration</source><translation>Завантажити рекомендовану конфігурацію</translation></message>
    <message><source>Choose recommended configuration</source><translation>Вибрати рекомендовану конфігурацію</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Виберіть групи рекомендованої конфігурації. Перед імпортом можна перевірити й видалити окремі команди.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>З офіційної документації Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Виключення UWF, рекомендовані Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Перевірте перед імпортом: папки мають існувати, щоб UWF прийняв виключення файлів.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Джерело: офіційна документація Microsoft UWF про виключення фільтра й підтримку антивірусів.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Програма підвищення якості ПЗ (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Зберігати між сеансами: Згода на участь у політиці</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Зберігати між сеансами: Локальна згода на участь</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Зберігати між сеансами: Прапорець заборони надсилання</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Фонова інтелектуальна служба передавання (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Зберігати між сеансами: Файли черги завантаження</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Зберігати між сеансами: Індекс стану передавання</translation></message>
    <message><source>Network profiles and policies</source><translation>Мережеві профілі й політики</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Групова політика бездротової мережі</translation></message>
    <message><source>Wired network GPO policy</source><translation>Групова політика дротової мережі</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Групова політика бездротової мережі · файли</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Групова політика дротової мережі · файли</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Профілі бездротових інтерфейсів</translation></message>
    <message><source>Wired network interface profiles</source><translation>Профілі дротових інтерфейсів</translation></message>
    <message><source>Wireless service configuration</source><translation>Налаштування бездротової служби</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Налаштування мобільного широкосмугового доступу</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Налаштування служби автоналаштування дротової мережі</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>XML-профілі мережі залежать від пристрою. За потреби вручну додайте точні шляхи Interfaces\{GUID}\{GUID}.xml.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Літній час (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Зберігати між сеансами: Визначення часових поясів</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Зберігати між сеансами: Вибраний часовий пояс</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Зберігати між сеансами: Файли продукту й оновлення</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Зберігати між сеансами: Сигнатури й стан у ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Зберігати між сеансами: Windows Update · Журнал</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Зберігати між сеансами: MpCmdRun · Журнал</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Зберігати між сеансами: Стан продукту в реєстрі</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Зберігати між сеансами: WdBoot · Стан служби</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Зберігати між сеансами: WdFilter · Стан служби</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Зберігати між сеансами: WdNisSvc · Стан служби</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Зберігати між сеансами: WdNisDrv · Стан служби</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Зберігати між сеансами: WinDefend · Стан служби</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Зберігати між сеансами: Файли клієнтської програми</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Зберігати між сеансами: Windows Update · Журнал</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Зберігати між сеансами: MpCmdRun · Журнал</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Зберігати між сеансами: Сигнатури й стан антивірусу</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Зберігати між сеансами: Стан антивірусу в реєстрі</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Вибрати файли з командами uwfmgr</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Текстові файли (*.txt *.cmd);;Усі файли (*)</translation></message>
    <message><source>Status</source><translation>Стан</translation></message>
    <message><source>Import failed</source><translation>Помилка імпорту</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Внутрішня помилка: обробник застосування не зареєстрований.</translation></message>
    <message><source>Nothing to import</source><translation>Немає команд для імпорту</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>У введених даних не знайдено команд uwfmgr.</translation></message>
    <message><source>Duplicates: %1</source><translation>Дублікат: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Не підтримується: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Пакет %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Дублікат</translation></message>
    <message><source>Unsupported</source><translation>Не підтримується</translation></message>
    <message><source>File too large</source><translation>Файл завеликий</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Файл %1 більший за 5 MB і не розібраний. Спочатку вручну відфільтруйте вміст.</translation></message>
    <message><source>Cannot read file</source><translation>Не вдалося прочитати файл</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Помилка: Відкриття файлу (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Відкинути зміни в очікуванні?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Незастосованих змін: %1.

Відкинути й продовжити?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Неповна команда uwfmgr</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Відсутній аргумент: Розмір (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>Розмір має бути цілим невід’ємним числом у MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Відсутній аргумент: Накладання · Тип (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Неприпустиме значення: Накладання · Тип: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Відсутній аргумент: Том (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Том має бути буквою диска, наприклад C:</translation></message>
    <message><source>Missing path argument</source><translation>Відсутній аргумент: Шлях</translation></message>
    <message><source>Missing registry key argument</source><translation>Відсутній аргумент: Реєстр · Ключ</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Незакрита подвійна лапка в команді</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Неочікуваний зайвий аргумент: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Непідтримувана команда uwfmgr: немає відповідної дії інтерфейсу</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Потрібний стан уже встановлено; дія не потрібна</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Шлях не на цьому томі або том не підтримує виключення файлів (наприклад, exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Відхилено списком заборон UWF (системні файли, Windows, підкачка тощо)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Неприпустиме значення: Розмір: %1</translation></message>
    <message><source>maximum size</source><translation>Максимальний розмір</translation></message>
    <message><source>warning threshold</source><translation>Поріг попередження</translation></message>
    <message><source>critical threshold</source><translation>Критичний поріг</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>Глобальна конфігурація недоступна; команду не додано до змін, що очікують застосування.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Невідомий том %1: немає придатного для UWF диска з цією буквою</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Том %1: Потрібний стан уже встановлено; дія не потрібна</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Шлях не має букви диска. Визначення цільового тому: Помилка.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Том %1: UWF Не підтримується</translation></message>
    <message><source>file exclusion</source><translation>Виключення файлів</translation></message>
    <message><source>registry exclusion</source><translation>Виключення реєстру</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Вкладка реєстру відсутня; виключення реєстру недоступні</translation></message>
    <message><source>Unsupported command</source><translation>Команда: Не підтримується</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Показати або приховати.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Увімкнено</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Вимкнено</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Невідомо</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Використано %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Розширений режим</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Узгодити автофіксацію з вимкненням і перезапуском Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Помилка: Читання стану розширеного режиму (Невідома помилка).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Помилка: Читання стану служби:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Помилка: Читання стану служби (Невідома помилка).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Службу не налаштовано як окремий процес.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Службу не налаштовано на автоматичний запуск.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Службу не налаштовано для роботи від LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>Служба не працює.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Шлях чи аргументи запуску служби не відповідають поточній програмі.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Неправильно налаштовано час очікування попереднього завершення служби.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Служба не оголошує права для запуску агента інтерфейсу.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Служба не приймає сповіщення Windows про попереднє завершення.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Увімкнути розширений режим</translation></message>
    <message><source>Disable enhanced mode</source><translation>Вимкнути розширений режим</translation></message>
    <message><source>Start service</source><translation>Запустити службу</translation></message>
    <message><source>Repair enhanced mode</source><translation>Відновити розширений режим</translation></message>
    <message><source>Delete service</source><translation>Видалити службу</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Зупинити й видалити службу розширеного режиму з усіма залишковими даними реєстру.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Служби немає, але її дані реєстру залишилися.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Очікування автентифікації агента інтерфейсу.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Служба працює, але автентифікований агент інтерфейсу не підключений.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Допоміжна служба розширеного режиму UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Розширений режим · Конфігурація: Застосовано.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Конфігурація: Застосовано. Звіт збереження UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Розширений режим · Конфігурація: Помилка:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Розширений режим · Конфігурація: Помилка (Невідома помилка).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Службу розширеного режиму й залишкові налаштування видалено.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Видалення служби запитано, але очищення залишків повідомило:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Розширений режим: Видалити службу: Помилка:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Розширений режим: Видалити службу: Помилка (Невідома помилка).</translation></message>
    <message><source>File staging completed</source><translation>Автофіксація файлів: Успішно</translation></message>
    <message><source>File staging failed</source><translation>Автофіксація файлів: Помилка</translation></message>
    <message><source>All staged targets were processed.</source><translation>Усі цілі автофіксації оброблено.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Усі об’єкти оброблено; одна чи кілька операцій завершилися помилкою.</translation></message>
    <message><source>Show main window</source><translation>Показати головне вікно</translation></message>
    <message><source>Restore default position</source><translation>Відновити типове розташування</translation></message>
    <message><source>Show overlay hub</source><translation>Показати HUB накладання</translation></message>
    <message><source>Hide overlay hub</source><translation>Приховати HUB накладання</translation></message>
    <message><source>Exit application</source><translation>Вийти з програми</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Помилка: Підключення до системи</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Помилка: Читання значень реєстру (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Перелік файлів накладання: Невідома помилка.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Помилка: Дія з фільтром: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Помилка: Читання стану фільтра</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Помилка: Задання порогу попередження</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Помилка: Задання критичного порогу</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Помилка: Читання стану накладання</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Помилка: Задання типу накладання</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Помилка: Задання максимального розміру</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Помилка: Читання конфігурації накладання</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Помилка: Реєстрація тому %1 в UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Помилка: Захист тому %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Помилка: Задання прив’язки тому %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Том %1 · Помилка: Додавання виключення файлу %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Виключення файлу на томі %1 вже відсутнє: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Том %1 · Помилка: Вилучення виключення файлу %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Помилка: Читання конфігурації тому</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Помилка: Додавання виключення реєстру %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Помилка: Вилучення виключення реєстру %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Помилка: Зміна параметрів збереження реєстру</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Помилка: Читання фільтра реєстру</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Не підтримується</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Читання стану об’єкта: Невідома помилка.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Шлях не існує; видаляти нічого.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Невідома помилка.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Результат не підтверджено: повторне читання стану об’єкта не вдалося: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Постачальник прийняв видалення, але повторна перевірка за надійним джерелом показала, що об’єкт досі існує.</translation></message>
    <message><source>Unknown</source><translation>Невідомо</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Помилка: Завантаження журналу (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Помилка: Завантаження журналу (Невідома помилка).</translation></message>
    <message><source>: unknown error</source><translation>: Невідома помилка</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Помилка: Визначення цільового тому (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Помилка: Перелік ключів реєстру (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Помилка: Визначення цільового тому (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Операцію пропущено: необхідна попередня операція завершилася помилкою або її результат не вдалося підтвердити.</translation></message>
    <message><source>Copy</source><translation>Копіювати</translation></message>
    <message><source>Select All</source><translation>Виділити все</translation></message>
    <message><source>Display language</source><translation>Мова інтерфейсу</translation></message>
    <message><source>Folder</source><translation>Папка</translation></message>
    <message><source>File</source><translation>Файл</translation></message>
    <message><source>Export commands</source><translation>Експортувати команди</translation></message>
    <message><source>Direct restart…</source><translation>Перезавантажити напряму…</translation></message>
    <message><source>Clear…</source><translation>Очистити…</translation></message>
    <message><source>Committing</source><translation>Фіксація</translation></message>
    <message><source> · %1 selected</source><translation> · Вибрано: %1</translation></message>
    <message><source>%1 changes</source><translation>Змін: %1</translation></message>
    <message><source>%1 commands</source><translation>Команд: %1</translation></message>
    <message><source>%1 items</source><translation>Елементів: %1</translation></message>
    <message><source>%1 items · %2</source><translation>Елементів: %1 · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>Змін, що очікують: %1</translation></message>
    <message><source>%1 records</source><translation>Записів: %1</translation></message>
    <message><source>%1 records · %2 total</source><translation>Записів: %1 · усього %2</translation></message>


    <message><source>%1 selected</source><translation>Вибрано: %1</translation></message>
    <message><source>/ %1 files</source><translation>/ файлів: %1</translation></message>
    <message><source>32-bit</source><translation>32-розрядна</translation></message>
    <message><source>64-bit</source><translation>64-розрядна</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Поширюється на умовах &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Повні умови містяться у файлі LICENSE, що входить до складу застосунку.&lt;/p&gt;&lt;p&gt;Ви можете поширювати та змінювати програму на умовах GPL v3. Програма надається як є, без жодних гарантій.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows вимикає файли підкачування, Відновлення системи, SuperFetch, індексування файлів, дефрагментацію диска та швидкий запуск. Також у BCD параметру bootstatuspolicy надається значення ignoreallfailures.&lt;/p&gt;&lt;p&gt;Ці налаштування можна змінити згодом. Наприклад, перенесіть файл підкачування на незахищений том перед повторним увімкненням підкачування.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Діагностичний знімок системи, привілеїв та UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>У цьому розташуванні вже є файл:
%1

Збереження замінить його вміст.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Графічний диспетчер Windows Unified Write Filter.
Керування захистом томів, накладанням, винятками та файлами для збереження.</translation></message>
    <message><source>About this snapshot</source><translation>Про цей знімок</translation></message>
    <message><source>Account</source><translation>Обліковий запис</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Додайте конфігурацію UWF зі сценарію або тексту.</translation></message>
    <message><source>Add exclusion</source><translation>Додати виняток</translation></message>
    <message><source>Add files or folders</source><translation>Додати файли або папки</translation></message>
    <message><source>Add selected items</source><translation>Додати вибрані елементи</translation></message>
    <message><source>Added</source><translation>Додано</translation></message>
    <message><source>Administrator</source><translation>Адміністратор</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Усі зміни збережено на диску. Вони залишаться без перезавантаження.</translation></message>
    <message><source>All files (*)</source><translation>Усі файли (*)</translation></message>
    <message><source>All levels</source><translation>Усі рівні</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Порожній діагностичний знімок не означає, що накладання не містить змін.</translation></message>
    <message><source>Application path</source><translation>Шлях застосунку</translation></message>
    <message><source>Application result</source><translation>Результат застосування</translation></message>
    <message><source>Apply changes</source><translation>Застосувати зміни</translation></message>
    <message><source>Architecture</source><translation>Архітектура</translation></message>
    <message><source>Automatic</source><translation>Автоматично</translation></message>
    <message><source>Automatic file staging</source><translation>Автозбереження підготовлених файлів</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Автозбереження не завершено</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Автоматично зберігати придатні підготовлені файли під час звичайного вимкнення й перезавантаження.</translation></message>
    <message><source>Available</source><translation>Доступно</translation></message>
    <message><source>Back</source><translation>Назад</translation></message>
    <message><source>Background service</source><translation>Фонова служба</translation></message>
    <message><source>Binding</source><translation>Прив’язка</translation></message>
    <message><source>By drive letter</source><translation>За буквою диска</translation></message>
    <message><source>By volume ID</source><translation>За ідентифікатором тому</translation></message>
    <message><source>Cancel reading</source><translation>Скасувати читання</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Уже зафіксовані зміни збережено.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Виберіть розділ реєстру для виключення із захисту UWF, включно з його підрозділами та значеннями.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Виберіть розташування для збереження файлу.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Виберіть розташування в дозволеному диску або папці.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Виберіть іншу папку або перевірте права доступу й повторіть спробу.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Виберіть інший ключ або перевірте права доступу й повторіть спробу.</translation></message>
    <message><source>Clear filters</source><translation>Скинути фільтри</translation></message>
    <message><source>Clear logs</source><translation>Очистити журнал</translation></message>
    <message><source>Clear logs?</source><translation>Очистити журнал?</translation></message>
    <message><source>Collapse</source><translation>Згорнути</translation></message>
    <message><source>Commit changes</source><translation>Зафіксувати зміни</translation></message>
    <message><source>Commit changes…</source><translation>Зафіксувати зміни…</translation></message>
    <message><source>Commit complete</source><translation>Фіксацію завершено</translation></message>
    <message><source>Commit file changes?</source><translation>Зафіксувати зміни файлу?</translation></message>
    <message><source>Commit folder changes?</source><translation>Зафіксувати зміни папки?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Зафіксувати зміни ключа реєстру?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Зафіксувати зміни значення реєстру?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Фіксує підготовлені файли перед звичайним вимкненням і перезавантаженням</translation></message>
    <message><source>Compatibility mode</source><translation>Режим сумісності</translation></message>
    <message><source>Configuration applied</source><translation>Конфігурацію застосовано</translation></message>
    <message><source>Configuration error</source><translation>Помилка конфігурації</translation></message>
    <message><source>Configured</source><translation>Налаштовано</translation></message>
    <message><source>Confirm before continuing</source><translation>Підтвердьте перед продовженням</translation></message>
    <message><source>Connected and authenticated</source><translation>Підключено й автентифіковано</translation></message>
    <message><source>Connecting…</source><translation>Підключення…</translation></message>
    <message><source>Contact author</source><translation>Зв’язатися з автором</translation></message>
    <message><source>Continue editing</source><translation>Продовжити редагування</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Продовження перезавантаження призведе до втрати змін у файлах, які не вдалося зберегти.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Продовження вимкнення призведе до втрати змін у файлах, які не вдалося зберегти.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Узгодити з вимкненням Windows</translation></message>
    <message><source>Copy all logs</source><translation>Копіювати весь журнал</translation></message>
    <message><source>Copy full report</source><translation>Копіювати повний звіт</translation></message>
    <message><source>Copy name</source><translation>Копіювати ім’я</translation></message>
    <message><source>Copy path</source><translation>Копіювати шлях</translation></message>
    <message><source>Copy selection</source><translation>Копіювати вибране</translation></message>
    <message><source>Copy version</source><translation>Копіювати версію</translation></message>
    <message><source>Counting staged files…</source><translation>Підрахунок підготовлених файлів…</translation></message>
    <message><source>Create</source><translation>Створити</translation></message>
    <message><source>Critical %1 MB</source><translation>Критичний поріг %1 МБ</translation></message>
    <message><source>Current filter</source><translation>Поточний фільтр</translation></message>
    <message><source>Current run</source><translation>Поточний запуск</translation></message>
    <message><source>Current selection</source><translation>Поточний вибір</translation></message>
    <message><source>Current usage</source><translation>Поточне використання</translation></message>
    <message><source>Date modified</source><translation>Дата змінення</translation></message>
    <message><source>Debug</source><translation>Налагодження</translation></message>
    <message><source>Delete and commit this file?</source><translation>Видалити цей файл і зафіксувати видалення?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Видалити цю папку й зафіксувати видалення?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Видалити цей ключ реєстру й зафіксувати видалення?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Видалити це значення реєстру й зафіксувати видалення?</translation></message>

    <message><source>Direct restart?</source><translation>Перезавантажити безпосередньо?</translation></message>
    <message><source>Direct shutdown?</source><translation>Вимкнути безпосередньо?</translation></message>
    <message><source>Discard changes</source><translation>Відкинути зміни</translation></message>
    <message><source>Disk</source><translation>Диск</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Постійне зберігання секретного ключа домену</translation></message>
    <message><source>Done</source><translation>Готово</translation></message>
    <message><source>Edit commands</source><translation>Редагувати команди</translation></message>
    <message><source>Editable</source><translation>Можна редагувати</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Увімкніть фільтр і перезавантажте систему, щоб бачити поточне використання</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>Розширений режим додає фонову службу, яка запускається разом із Windows. Видаліть службу перед переміщенням або видаленням застосунку.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>Розширений режим недоступний. Щоб зберегти підготовлені файли, скористайтеся командами «Безпечне вимкнення» або «Безпечне перезавантаження».</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Введіть HKLM\SOFTWARE\… і натисніть Enter для переходу</translation></message>
    <message><source>Enter a file name</source><translation>Введіть ім’я файлу</translation></message>
    <message><source>Enter a valid file name.</source><translation>Введіть припустиме ім’я файлу.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Введіть припустиме ім’я файлу або папки.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Введіть припустиме ім’я папки.</translation></message>
    <message><source>Error</source><translation>Помилка</translation></message>
    <message><source>Error details</source><translation>Подробиці помилки</translation></message>
    <message><source>Error details and suggestions</source><translation>Подробиці помилки та рекомендації</translation></message>
    <message><source>Expand</source><translation>Розгорнути</translation></message>
    <message><source>Export list</source><translation>Експортувати список</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Не вдалося очистити журнал: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Не вдалося очистити журнал: непередбачена помилка.</translation></message>
    <message><source>File name</source><translation>Ім’я файлу</translation></message>
    <message><source>File or folder</source><translation>Файл або папка</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>Збереження файлів розпочалося, його неможливо скасувати.</translation></message>
    <message><source>File saving incomplete</source><translation>Збереження файлів не завершено</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Збереження підготовлених файлів буде пропущено. Незафіксовані зміни в захищених шляхах буде втрачено після перезавантаження.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Збереження підготовлених файлів буде пропущено. Незафіксовані зміни в захищених шляхах буде втрачено після вимкнення.</translation></message>
    <message><source>File type</source><translation>Тип файлу</translation></message>
    <message><source>Files and folders</source><translation>Файли та папки</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Файли та папки можна вибирати разом.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Підготовлені файли не буде збережено.</translation></message>
    <message><source>Filter is disabled</source><translation>Фільтр вимкнено</translation></message>
    <message><source>Fit columns to contents</source><translation>Припасувати стовпці до вмісту</translation></message>
    <message><source>Folder address</source><translation>Адреса папки</translation></message>
    <message><source>Folder name</source><translation>Ім’я папки</translation></message>
    <message><source>Folders</source><translation>Папки</translation></message>
    <message><source>Full configuration</source><translation>Повна конфігурація</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Подальші записи в цей ключ не будуть захищені UWF.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Подальші записи в цей шлях не будуть захищені UWF.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Глобальний виняток реєстру · включно з підрозділами</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Перейдіть до батьківської папки або виберіть інше розташування.</translation></message>
    <message><source>Home</source><translation>Особиста папка</translation></message>
    <message><source>Import configuration</source><translation>Імпорт конфігурації</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>Імпорт оновлює чернетку конфігурації. Скористайтеся командою «Перевірити й застосувати», щоб записати зміни в систему.</translation></message>
    <message><source>Import results</source><translation>Результати імпорту</translation></message>
    <message><source>Imported</source><translation>Імпортовано</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>У звіті наведено результати команд для чернетки конфігурації. Виберіть рядок, щоб переглянути повну команду та результат.</translation></message>
    <message><source>Imported: %1</source><translation>Імпортовано: %1</translation></message>
    <message><source>Information</source><translation>Інформація</translation></message>
    <message><source>Item details</source><translation>Подробиці елемента</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Збереження великого списку підготовлених файлів може тривати довше.</translation></message>
    <message><source>License and copyright</source><translation>Ліцензія та авторські права</translation></message>
    <message><source>Line</source><translation>Рядок</translation></message>
    <message><source>Local disk (%1)</source><translation>Локальний диск (%1)</translation></message>
    <message><source>Log level</source><translation>Рівень журналу</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Не вдалося прочитати журнал. Повторіть спробу або перевірте доступ до файлу журналу.</translation></message>
    <message><source>Maximize</source><translation>Розгорнути вікно</translation></message>
    <message><source>Message details</source><translation>Подробиці повідомлення</translation></message>
    <message><source>Minimize</source><translation>Згорнути вікно</translation></message>
    <message><source>Module</source><translation>Модуль</translation></message>
    <message><source>More</source><translation>Ще</translation></message>
    <message><source>Move</source><translation>Перемістити</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Якщо диск із застосунком захищено UWF, спочатку переконайтеся, що файли застосунку збережуться після перезавантаження. Збереження великої кількості підготовлених файлів може подовжити вимкнення чи перезавантаження.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Можна вибрати кілька файлів.</translation></message>
    <message><source>Name and location</source><translation>Ім’я та розташування</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Іменоване значення: %1 · лише це значення</translation></message>
    <message><source>Navigate to registry path</source><translation>Перейти до шляху реєстру</translation></message>
    <message><source>New folder</source><translation>Нова папка</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Нові записи з’являться після оновлення.</translation></message>
    <message><source>Next-session configuration</source><translation>Конфігурація наступного сеансу</translation></message>
    <message><source>No files to commit</source><translation>Немає файлів для фіксації</translation></message>
    <message><source>No logs yet</source><translation>У журналі ще немає записів</translation></message>
    <message><source>No matching items</source><translation>Немає відповідних елементів</translation></message>
    <message><source>No matching logs</source><translation>Немає відповідних записів журналу</translation></message>
    <message><source>No overlay files found</source><translation>Файли накладання не знайдено</translation></message>
    <message><source>No registry key selected</source><translation>Ключ реєстру не вибрано</translation></message>
    <message><source>None</source><translation>Немає</translation></message>
    <message><source>Not configured</source><translation>Не налаштовано</translation></message>
    <message><source>Not connected</source><translation>Не підключено</translation></message>
    <message><source>Not installed</source><translation>Не встановлено</translation></message>
    <message><source>Not written to the system</source><translation>Не записано в систему</translation></message>
    <message><source>Off</source><translation>Вимк.</translation></message>
    <message><source>On</source><translation>Увімк.</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Одна команда uwfmgr на рядок</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Зберігайте застосунок у постійній папці, яку звичайні користувачі не можуть змінювати. Рекомендоване розташування:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Автоматично зберігаються лише підготовлені файли. Інші незафіксовані зміни в захищених шляхах буде втрачено після перезавантаження.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Автоматично зберігаються лише підготовлені файли. Інші незафіксовані зміни в захищених шляхах буде втрачено після вимкнення.</translation></message>
    <message><source>Open</source><translation>Відкрити</translation></message>
    <message><source>Open folder</source><translation>Відкрити папку</translation></message>
    <message><source>Operating system</source><translation>Операційна система</translation></message>
    <message><source>Operation and security</source><translation>Робота та безпека</translation></message>
    <message><source>Original command</source><translation>Початкова команда</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Досягнуто критичного порога накладання. Збережіть зміни й безпечно перезавантажте систему.</translation></message>
    <message><source>Overlay maximum size</source><translation>Максимальний розмір накладання</translation></message>
    <message><source>Overlay type</source><translation>Тип накладання</translation></message>
    <message><source>Overlay usage</source><translation>Використання накладання</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Досягнуто порога попередження накладання. Збережіть зміни й безпечно перезавантажте систему.</translation></message>
    <message><source>Overview</source><translation>Огляд</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Вставте команди або завантажте їх із файлу…</translation></message>
    <message><source>Pending changes</source><translation>Зміни, що очікують</translation></message>
    <message><source>Pending removal</source><translation>Очікує видалення</translation></message>
    <message><source>Permissions and protection</source><translation>Дозволи та захист</translation></message>
    <message><source>Privileges</source><translation>Привілеї</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>Оброблено %1 / %2 елементів</translation></message>
    <message><source>Processing result</source><translation>Результат обробки</translation></message>
    <message><source>Protected</source><translation>Захищено</translation></message>
    <message><source>Protected volumes</source><translation>Захищені томи</translation></message>
    <message><source>Protection and overlay</source><translation>Захист і накладання</translation></message>
    <message><source>Quick access</source><translation>Швидкий доступ</translation></message>
    <message><source>Raw report</source><translation>Початковий звіт</translation></message>
    <message><source>Read again</source><translation>Прочитати знову</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Прочитайте знову, щоб отримати діагностичний знімок.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Прочитайте конфігурацію, щоб бачити поточне використання</translation></message>
    <message><source>Read-only</source><translation>Лише читання</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Читання великого накладання може тривати довше. Його можна скасувати будь-коли.</translation></message>
    <message><source>Reading canceled</source><translation>Читання скасовано</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Читання файлів і вкладених папок у цьому розташуванні…</translation></message>
    <message><source>Reading folder</source><translation>Читання папки</translation></message>
    <message><source>Reading logs</source><translation>Читання журналу</translation></message>
    <message><source>Reading overlay</source><translation>Читання накладання</translation></message>
    <message><source>Ready</source><translation>Готовність</translation></message>
    <message><source>Ready to save</source><translation>Готово до збереження</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Записи поточного запуску з’являться тут після завершення читання.</translation></message>
    <message><source>Recursive staging</source><translation>Підготовка з вкладеними папками</translation></message>
    <message><source>Refresh folder</source><translation>Оновити папку</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Ключ реєстру або налаштування постійного зберігання</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Решту завдань можна скасувати після завершення поточної цілі.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Вилучити цей ключ із винятків наступного сеансу.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Вилучити цей шлях із винятків наступного сеансу.</translation></message>
    <message><source>Removed</source><translation>Вилучено</translation></message>
    <message><source>Rename</source><translation>Перейменувати</translation></message>
    <message><source>Rename…</source><translation>Перейменувати…</translation></message>
    <message><source>Repair required</source><translation>Потрібне виправлення</translation></message>
    <message><source>Replace file</source><translation>Замінити файл</translation></message>
    <message><source>Replace the existing file?</source><translation>Замінити наявний файл?</translation></message>
    <message><source>Required configuration</source><translation>Необхідна конфігурація</translation></message>
    <message><source>Restart anyway</source><translation>Усе одно перезавантажити</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Перезавантаження продовжиться після збереження. Залиште застосунок відкритим.</translation></message>
    <message><source>Restore</source><translation>Відновити</translation></message>
    <message><source>Retry</source><translation>Повторити</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Перегляньте подробиці помилок перед продовженням вимкнення.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Перегляньте подробиці помилок перед перезавантаженням.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Перегляньте подробиці операції. Прочитайте поточний стан системи перед повторенням змін, які вже могли бути записані.</translation></message>
    <message><source>Review the result for each target.</source><translation>Перевірте результат для кожної цілі.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Перевірте ці зміни перед записом у систему.</translation></message>
    <message><source>Running</source><translation>Працює</translation></message>
    <message><source>Save</source><translation>Зберегти</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Зберегти підготовлені файли на диск і перезавантажити систему.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Зберегти підготовлені файли на диск і вимкнути систему.</translation></message>
    <message><source>Saving staged files</source><translation>Збереження підготовлених файлів</translation></message>
    <message><source>Saving to disk</source><translation>Збереження на диск</translation></message>
    <message><source>Saving…</source><translation>Збереження…</translation></message>
    <message><source>Scanning staged paths</source><translation>Сканування підготовлених шляхів</translation></message>
    <message><source>Search logs</source><translation>Пошук у журналі</translation></message>
    <message><source>Search messages or modules</source><translation>Пошук повідомлень або модулів</translation></message>
    <message><source>Search paths</source><translation>Пошук шляхів</translation></message>
    <message><source>Search this folder</source><translation>Пошук у цій папці</translation></message>
    <message><source>Select a file or enter its name</source><translation>Виберіть файл або введіть його ім’я</translation></message>
    <message><source>Select a file to open.</source><translation>Виберіть файл для відкриття.</translation></message>
    <message><source>Select a folder.</source><translation>Виберіть папку.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Виберіть ключ для операції з усім його піддеревом або іменоване значення для операції лише з цим значенням.</translation></message>
    <message><source>Select an available subkey</source><translation>Вибрати доступний підрозділ</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Виберіть лише файли або відкрийте папку, щоб переглянути її вміст.</translation></message>
    <message><source>Select files or folders</source><translation>Вибрати файли або папки</translation></message>
    <message><source>Select folder</source><translation>Вибрати папку</translation></message>
    <message><source>Select folders only.</source><translation>Вибирайте лише папки.</translation></message>
    <message><source>Select registry key or value</source><translation>Вибрати ключ або значення реєстру</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Вибрати ключ або значення реєстру для видалення</translation></message>
    <message><source>Select the current folder</source><translation>Вибрати поточну папку</translation></message>
    <message><source>Select this file</source><translation>Вибрати цей файл</translation></message>
    <message><source>Select this registry key</source><translation>Вибрати цей ключ реєстру</translation></message>
    <message><source>Select this registry value</source><translation>Вибрати це значення реєстру</translation></message>
    <message><source>Selected file</source><translation>Вибраний файл</translation></message>
    <message><source>Selected folder</source><translation>Вибрана папка</translation></message>
    <message><source>Selection</source><translation>Вибір</translation></message>
    <message><source>Service account</source><translation>Обліковий запис служби</translation></message>
    <message><source>Service configuration needs repair</source><translation>Конфігурацію служби потрібно виправити</translation></message>
    <message><source>Service details</source><translation>Подробиці служби</translation></message>
    <message><source>Service name</source><translation>Ім’я служби</translation></message>
    <message><source>Service status</source><translation>Стан служби</translation></message>
    <message><source>Service stopped</source><translation>Службу зупинено</translation></message>
    <message><source>Shut down anyway</source><translation>Усе одно вимкнути</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>Вимкнення продовжиться після збереження. Залиште застосунок відкритим.</translation></message>
    <message><source>Size</source><translation>Розмір</translation></message>
    <message><source>Some changes could not be applied</source><translation>Не вдалося застосувати деякі зміни</translation></message>
    <message><source>Some files could not be saved</source><translation>Не вдалося зберегти деякі файли</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Не вдалося прочитати частину відомостей про обладнання. Інші діагностичні відомості й надалі можна переглянути та скопіювати.</translation></message>
    <message><source>Source code</source><translation>Початковий код</translation></message>
    <message><source>Staged file</source><translation>Файл для збереження</translation></message>
    <message><source>Staged path</source><translation>Шлях для збереження</translation></message>
    <message><source>Standard user</source><translation>Звичайний користувач</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Автоматичний запуск із Windows</translation></message>
    <message><source>Startup type</source><translation>Тип запуску</translation></message>
    <message><source>Status unavailable</source><translation>Стан недоступний</translation></message>
    <message><source>Stopped</source><translation>Зупинено</translation></message>
    <message><source>Storage</source><translation>Сховище</translation></message>
    <message><source>System</source><translation>Система</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Зміни системи під час першого ввімкнення UWF</translation></message>
    <message><source>System memory</source><translation>Системна пам’ять</translation></message>
    <message><source>System metadata</source><translation>Системні метадані</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Системні метадані призначені лише для діагностики й не можуть бути зафіксовані.</translation></message>
    <message><source>Takes effect after restart</source><translation>Набирає чинності після перезавантаження</translation></message>
    <message><source>Target</source><translation>Ціль</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Постійне зберігання ліцензій служб терміналів</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Постачальник UWF не зміг завершити перелік. Це може статися за великого накладання або нестачі системних ресурсів.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>Конфігурацію записано в систему. Більшість змін набере чинності після перезавантаження.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>Конфігурація відповідає цільовій конфігурації наступного сеансу. Перегляньте або експортуйте команди в розділі «Повна конфігурація».</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Поточні записи журналу буде видалено без можливості відновлення.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Папка призначення не існує.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Не вдалося прочитати стан служби розширеного режиму. Перевірте дозволи служби й повторіть спробу.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>У списку можуть бути пропущені файли або присутні вже зафіксовані. Не використовуйте його як список для фіксації.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Вибраний елемент більше не існує: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Службу встановлено, але зупинено. Запустіть її, щоб відновити автозбереження.</translation></message>
    <message><source>This PC</source><translation>Цей комп’ютер</translation></message>
    <message><source>This folder is empty</source><translation>Ця папка порожня</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Звіт може містити імена облікових записів, ідентифікатори пристроїв і шляхи файлів. Перевірте його перед поширенням.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Збережіть цей CMD-скрипт у UTF-8 у файл .cmd і запустіть його в командному рядку. Експорт використовує той самий скрипт.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Спробуйте інше ключове слово або рівень журналу.</translation></message>
    <message><source>Try another search term.</source><translation>Спробуйте інший пошуковий запит.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Тип і максимальний розмір неможливо змінити за ввімкненого фільтра. Спочатку вимкніть фільтр і перезавантажте систему.</translation></message>
    <message><source>UI agent</source><translation>Агент інтерфейсу</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · рядків: %1</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>Не вдалося прочитати конфігурацію UWF. Переконайтеся, що Unified Write Filter встановлено у Windows, і прочитайте знову.</translation></message>
    <message><source>UWF feature</source><translation>Компонент UWF</translation></message>
    <message><source>UWF filter</source><translation>Фільтр UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF зараз недоступний</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Безпечне перезавантаження UWF буде виконано безпосередньо.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>Безпечне вимкнення UWF буде виконано безпосередньо.</translation></message>
    <message><source>Unable to access this folder</source><translation>Немає доступу до цієї папки</translation></message>
    <message><source>Unable to access this registry key</source><translation>Немає доступу до цього ключа реєстру</translation></message>
    <message><source>Unable to read logs</source><translation>Не вдалося прочитати журнал</translation></message>
    <message><source>Unable to read service status</source><translation>Не вдалося прочитати стан служби</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Зараз не вдається прочитати накладання</translation></message>
    <message><source>Unable to read this folder.</source><translation>Не вдалося прочитати цю папку.</translation></message>
    <message><source>Unable to select this item</source><translation>Неможливо вибрати цей елемент</translation></message>
    <message><source>Unavailable</source><translation>Недоступно</translation></message>
    <message><source>Undo removal</source><translation>Скасувати вилучення</translation></message>
    <message><source>Unprotected</source><translation>Не захищено</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Неуспішні елементи не збережено. Виберіть рядок, щоб побачити повний шлях і причину.</translation></message>
    <message><source>Up one folder</source><translation>На папку вище</translation></message>
    <message><source>Usage unavailable</source><translation>Використання недоступне</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Використовувати весь ключ, включно з усіма підрозділами</translation></message>
    <message><source>Version %1</source><translation>Версія %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Перегляд записів застосунку та діагностичної інформації.</translation></message>
    <message><source>View commands for these changes</source><translation>Переглянути команди цих змін</translation></message>
    <message><source>View failure details</source><translation>Переглянути подробиці помилок</translation></message>
    <message><source>View overlay</source><translation>Переглянути накладання</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Перегляд діагностичного знімка файлів для %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Прив’язка тому %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Захист тому %1</translation></message>
    <message><source>Volume identity</source><translation>Ідентифікатор тому</translation></message>
    <message><source>Warning</source><translation>Попередження</translation></message>
    <message><source>Warning %1 MB</source><translation>Поріг попередження %1 МБ</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Увесь ключ і всі підрозділи · рекурсивно</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Захист від запису забезпечує Windows UWF. Цей застосунок налаштовує його та керує ним.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Без розширеного режиму скористайтеся командами «Безпечне вимкнення» або «Безпечне перезавантаження» в цьому застосунку, щоб зберегти підготовлені файли.</translation></message>
    <message><source>Write protection</source><translation>Захист від запису</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Записати ці зміни конфігурації в систему? Більшість набере чинності після перезавантаження.</translation></message>
    <message><source>staged files</source><translation>файли для збереження</translation></message>
    <message><source>uwfmgr commands</source><translation>команди uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Позначити вибрані шляхи для видалення. Зміни набудуть чинності після застосування.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Видалити зі списку автоматичного збереження файлів після застосування.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Додати до списку автоматичного збереження файлів після застосування.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Застосування цього плану очистить збережений список файлів і видалить пов’язане виключення реєстру, якщо воно більше не потрібне.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Зміни списку файлів потрібно застосувати в UWF Manager; їх не можна експортувати як команди uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Список файлів зберігається після застосування</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Область знімка накладання</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Цей список призначений лише для діагностики. За ним не можна визначити, які файли потрібно зафіксувати.</translation></message>
    <message><source>NTFS volumes only</source><translation>Лише томи NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Читання великих накладань може бути повільним або завершитися помилкою через брак ресурсів.</translation></message>
    <message><source>Small files may be missing</source><translation>Малі файли можуть бути відсутні</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Файли, менші за розмір кластера диска (зазвичай 4 КБ), можуть не відображатися в списку.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Не показує зміни, що очікують фіксації</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Також можуть з’являтися вже зафіксовані файли, виключені шляхи та елементи, на які вплинули інші операції.</translation></message>

    <message><source>Command processing result</source><translation>Результат обробки команди</translation></message>
    <message><source>Copy command</source><translation>Копіювати команду</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · Не вдалося прочитати гілок: %1</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>Не вдалося прочитати гілок: %1. Продовжте зі знайденим вмістом або повторіть лише невдалі операції читання.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>Файлів: %1 · тек: %2</translation></message>
    <message><source>%1 values · %2 keys</source><translation>Значень: %1 · ключів: %2</translation></message>
    <message><source>Attempts</source><translation>Спроби</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Скасувати решту завдань</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Зафіксувати лише успішно переглянутий вміст. Непрочитані гілки залишаться без змін.</translation></message>
    <message><source>Committing changes</source><translation>Фіксація змін</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Завершені видалення та фіксації неможливо скасувати.</translation></message>
    <message><source>Continue…</source><translation>Продовжити…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Видалити й зафіксувати лише знайдені об’єкти. Непрочитані гілки та їхні батьківські теки або ключі буде збережено.</translation></message>
    <message><source>Deleting and committing</source><translation>Видалення та фіксація</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Видалення та фіксацію неможливо скасувати. Обробляється лише вибрана область; відкладені налаштування UWF не застосовуються.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Визначення області. Непрочитані гілки записуються до журналу, обхід решти вмісту триває.</translation></message>
    <message><source>Export traversal records</source><translation>Експортувати журнал обходу</translation></message>
    <message><source>No actionable content</source><translation>Немає вмісту для обробки</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Об’єктів для обробки не знайдено. Нічого не видалено й не зафіксовано.</translation></message>
    <message><source>No changes have been committed.</source><translation>Зміни ще не зафіксовано</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Нічого не видалено й не зафіксовано</translation></message>
    <message><source>Path</source><translation>Шлях</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Назавжди видалити вибрану область і зберегти видалення на захищеному томі.</translation></message>
    <message><source>Preparation complete</source><translation>Підготовку завершено</translation></message>
    <message><source>Preparing commit</source><translation>Підготовка фіксації</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Підготовка видалення та фіксації</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Обробити лише успішно переглянутий вміст</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Точки повторної обробки пропускаються, щоб обхід не виходив за вибрану область.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Перезапуск не скасовує фіксацію. Обробляється лише вибрана область; відкладені налаштування UWF не застосовуються.</translation></message>
    <message><source>Retry failed items</source><translation>Повторити невдалі</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Зберегти зміни вибраної області на захищеному томі.</translation></message>
    <message><source>Stopping the operation</source><translation>Зупинення операції</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Записи про успішне та невдале читання збережено</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Тека не порожня; необроблений вміст збережено.</translation></message>
    <message><source>The operation could not be completed</source><translation>Не вдалося завершити операцію</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>У ключі реєстру з’явилися нові значення; необроблений вміст збережено.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>Ключ реєстру ще містить підрозділи; необроблений вміст збережено.</translation></message>
    <message><source>The target no longer exists.</source><translation>Об’єкт більше не існує.</translation></message>
    <message><source>Traversal complete</source><translation>Обхід завершено</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Обхід завершено, частину вмісту не прочитано</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Журнал обходу збережено, його можна експортувати.</translation></message>
    <message><source>Traverse again</source><translation>Повторити обхід</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Повторіть обхід або закрийте це вікно.</translation></message>
    <message><source>Traversing the folder</source><translation>Обхід теки</translation></message>
    <message><source>Traversing the registry</source><translation>Обхід реєстру</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Не вдалося підготувати операцію</translation></message>
    <message><source>View results</source><translation>Переглянути результати</translation></message>

    <message><source>All changes committed</source><translation>Усі зміни збережено</translation></message>
    <message><source>Some changes committed</source><translation>Зміни збережено частково</translation></message>
    <message><source>No changes could be committed</source><translation>Не вдалося зберегти зміни</translation></message>
    <message><source>No changes to commit</source><translation>Немає змін для збереження</translation></message>
    <message><source>Commit interrupted</source><translation>Збереження перервано</translation></message>
    <message><source>%1 items were not processed.</source><translation>Не оброблено елементів: %1.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>Не вдалося виконати операцій із реєстром: %1.</translation></message>
    <message><source>The folder does not exist.</source><translation>Папка не існує.</translation></message>
    <message><source>Access denied.</source><translation>Доступ заборонено.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Елемент із таким ім’ям уже існує.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Не вдалося створити папку. Перевірте ім’я та права доступу.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Не вдалося перейменувати елемент. Перевірте ім’я та права доступу.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Цей оглядач не підтримує перейменування.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Чернетка списку підготовлених файлів змінилася після відкриття цього плану. Перегляньте новий план.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Збережений список підготовлених файлів змінився після відкриття цього плану. Оновіть дані та перегляньте зміни.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Збережений список підготовлених файлів не відповідає прийнятому плану.</translation></message>
    <message><source>Export configuration</source><translation>Експортувати конфігурацію</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Зберегти поточну тему, мову та стан відображення HUB у файл UWF.ini поруч із цією програмою.</translation></message>
    <message><source>Configuration saved</source><translation>Конфігурацію збережено</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Поточні налаштування інтерфейсу збережено до:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini вже існує. Наявний файл не змінено:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>Не вдалося зберегти UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Робочий стіл</translation></message>
    <message><source>Downloads</source><translation>Завантаження</translation></message>
    <message><source>Documents</source><translation>Документи</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Автофіксацію файлів зупинено. Усуньте помилку перед повторною спробою.</translation></message>
    <message><source>No net change for this setting</source><translation>Остаточне значення цього налаштування не змінилося після імпорту</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Кінцеве значення чернетки: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Кінцеве значення чернетки: %1 → %2 (запитано: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Запитано: %1 → %2; замінено наступною командою. Кінцеве значення чернетки → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Чернетку %1 оновлено</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Чернетку налаштування захисту тому %1 оновлено → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Виберіть записи для видалення.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Наразі неможливо відновити вибрані записи.</translation></message>
</context>
</TS>
