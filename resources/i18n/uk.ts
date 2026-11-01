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
    <message><source>Disabled</source><comment>service startup type</comment><translation>Вимкнено</translation></message>
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
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Захищати цей том у наступному сеансі. Запис за захищеними шляхами перенаправляється до накладання; збереження даних після перезапуску залежить від налаштувань накладання.</translation></message>
    <message><source>Current session</source><translation>Поточний сеанс</translation></message>
    <message><source>Next session</source><translation>Наступний сеанс</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Спосіб розпізнавання тому. Буква диска простіша, але прив’язка втрачається при її зміні, наприклад після додавання чи вилучення дисків. Ідентифікатор тому залишається сталим.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Том не підтримується UWF: </translation></message>
    <message><source>Global settings</source><translation>Загальні налаштування</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Стан фільтра UWF у поточному сеансі (лише читання).</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Увімкнути UWF у наступному сеансі. Запис за захищеними шляхами перенаправляється до накладання; збереження даних після перезапуску залежить від налаштувань накладання.</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>Щоб змінити тип або максимальну місткість накладання:
1. Вимкніть UWF.
2. Перезапустіть пристрій.
3. Змініть цей параметр.</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM використовує системну пам’ять. Disk використовує системний диск і підтримує більші накладання. Постійне накладання може зберігати дані накладання Disk після перезапуску.</translation></message>
    <message><source>Type</source><translation>Тип</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>Місткість накладання RAM обмежена системною пам’яттю. Накладання Disk потребує щонайменше 1024 МБ і достатньо вільного місця на системному диску.</translation></message>
    <message><source>Maximum capacity</source><translation>Максимальна місткість</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>Максимальна місткість · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Поріг попередження</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Записує попередження до журналу подій, коли обсяг зайнятого місця в накладанні досягає цього значення. Якщо обидві події ввімкнено, це значення має бути нижчим за критичний поріг. Значення 0 вимикає лише подію попередження.</translation></message>
    <message><source>Critical threshold</source><translation>Критичний поріг</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Записує критичне сповіщення до журналу подій, коли обсяг зайнятого місця в накладанні досягає цього значення. Якщо обидві події ввімкнено, це значення має бути вищим за поріг попередження. Значення 0 вимикає лише критичну подію.</translation></message>
    <message><source>Overlay</source><translation>Накладання</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Непідтримуваний тип диска (лише локальні фіксовані диски).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Том перевищує ліміт UWF: 16 TiB на захищений том.</translation></message>

    <message><source>%1 free / %2</source><translation>Вільно %1 / %2</translation></message>
    <message><source>Commit</source><translation>Зафіксувати</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>Зафіксувати зміни з накладання на диску або в реєстрі. Цю дію неможливо скасувати.</translation></message>
    <message><source>Commit file changes…</source><translation>Зафіксувати зміни файлу…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Виберіть файл і зафіксуйте його зміни з накладання на диску.</translation></message>
    <message><source>Commit folder changes…</source><translation>Зафіксувати зміни папки…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Виберіть папку й зафіксуйте на диску зміни з накладання для кожного файлу в ній.</translation></message>
    <message><source>Delete and commit file…</source><translation>Видалити файл і зафіксувати видалення на диску…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Виберіть файл для видалення й зафіксуйте видалення на диску.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Видалити папку з усім вмістом і зафіксувати видалення на диску…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>Виберіть папку для видалення з усім вмістом і вкладеними папками, а потім зафіксуйте видалення на диску.</translation></message>
    <message><source>Commit registry changes…</source><translation>Зафіксувати зміни реєстру…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Введіть ключ реєстру й необов’язкове ім’я значення, а потім зафіксуйте зміни в реєстрі. Якщо ім’я порожнє, фіксуються всі значення ключа та його підключів.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Зафіксувати видалення з реєстру…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>Введіть ключ реєстру й необов’язкове ім’я значення. Залиште ім’я значення порожнім, щоб видалити ключ з усіма підключами та значеннями, а потім зафіксувати видалення.</translation></message>
    <message><source>File exclusions</source><translation>Виключення файлів</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Файли й папки тому поза захистом UWF. Двічі клацніть запис, щоб скопіювати шлях.</translation></message>
    <message><source>Registry exclusions</source><translation>Виключення реєстру</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>Виключення реєстру — глобальні налаштування; вони відображаються лише для тому з Windows.</translation></message>
    <message><source>File staging</source><translation>Список автофіксації</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>Додати файли або папку до списку автофіксації.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Виберіть один або кілька файлів для автоматичної фіксації перед безпечним вимкненням або перезапуском.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Виберіть папку, файли якої разом із файлами вкладених папок фіксуватимуться перед безпечним вимкненням або перезапуском.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Вибрати файли для автоматичної фіксації</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Вибрати папку для автоматичної рекурсивної фіксації</translation></message>
    <message><source>Cannot stage this path</source><translation>Неможливо додати шлях до автофіксації</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Цей шлях перетинається з файловим виключенням UWF. Видаліть виключення, перш ніж додавати шлях до списку:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Цей шлях перетинається із записом списку автофіксації. Видаліть шлях зі списку, перш ніж додавати виключення:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Шлях перетинається з наявним записом списку автофіксації</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Виключення реєстру потрібне, поки список автофіксації не порожній</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Шлях не існує або має неочікуваний тип:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Не вдалося перевірити шлях:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Точки повторної обробки не можна додавати до списку автофіксації:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>Шлях не має літери диска, тому його неможливо додати до списку автофіксації:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Вибраний шлях розташований на томі %1. Додайте його на вкладці «Список автофіксації» цього тому.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Корінь тому не можна додавати: це призвело б до рекурсивної фіксації всього тому.</translation></message>
    <message><source>File staging could not be saved</source><translation>Не вдалося зберегти список автофіксації</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Список автофіксації перезавантажено через збій оновлення:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Список автофіксації недоступний: %1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Записів у списку: %1 · файлів: %2 · папок: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Список автофіксації потребує цього виключення реєстру, щоб зберігатися після перезапуску.</translation></message>
    <message><source>Built-in</source><translation>Вбудований</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Фільтр UWF вимкнено. Зміни в накладанні не накопичувалися; фіксувати нічого.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Файлова система %1 не підтримує пофайлову фіксацію.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Цей том не підтримується UWF.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Файлова система %1 не підтримує пофайлову фіксацію. Доступна лише фіксація реєстру (його виключення глобальні).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Том зараз не захищений UWF; немає змін файлів для фіксації.</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Вибрати файл для фіксації на диску</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Вибрати папку для фіксації на диску</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Вибрати файл для фіксації видалення</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Вибрати папку для фіксації видалення</translation></message>
    <message><source>Name</source><translation>Ім’я</translation></message>
    <message><source>Data</source><translation>Дані</translation></message>
    <message><source>(Default)</source><translation>(Типове)</translation></message>
    <message><source>(value not set)</source><translation>(значення не задано)</translation></message>
    <message><source>(empty)</source><translation>(порожньо)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Попередній перегляд недоступний (значення завелике)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Відкрити діагностичне подання файлів, що зараз перебувають у накладанні цього тому.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Список файлів накладання доступний лише на томах NTFS.</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>Фільтр UWF зараз вимкнено; жоден том не має накладання для перегляду.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Том не захищений UWF і не має накладання для перегляду.</translation></message>
    <message><source>Failed: %1</source><translation>Помилка: %1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>Попереднє перерахування, можливо, ще виконується. Наразі не можна почати нове читання.</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>Служба UWF недоступна під час переліку файлів накладання.</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>Недостатньо пам’яті або система не підтримує цю операцію. Функція доступна лише для томів NTFS; закрийте програми з великим споживанням пам’яті й спробуйте ще раз.</translation></message>
    <message><source>Raw error: %1</source><translation>Код помилки: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Експортувати список файлів накладання</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Текстові файли (*.txt);;Усі файли (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Командні файли (*.cmd);;Усі файли (*)</translation></message>
    <message><source>Export failed</source><translation>Помилка експорту</translation></message>
    <message><source>Export finished</source><translation>Експорт завершено</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Помилка: Відкриття файлу (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Помилка: Запис файлу (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Збережено записів: %1. Шлях:
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Не можна виключити том повністю (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>Файли підкачки, swap і гібернації не можна додавати як виключення.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Не можна виключити каталог повністю: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Не можна виключити каталог повністю: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Не можна виключити каталог повністю: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Не можна виключити критичний системний файл: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Файл реєстру користувача NTUSER.DAT не можна виключати.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Ключ реєстру не може бути порожнім.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Ключ реєстру не може починатися зі зворотної скісної риски.</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>Шляхи реєстру не можуть містити нульових символів.</translation></message>
    <message><source>A registry key component is too long.</source><translation>Один із компонентів ключа реєстру задовгий.</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>Шлях реєстру перевищує граничну глибину вкладення UWF.</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>Додати файл або папку до списку виключень.</translation></message>
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
    <message><source>Review and apply</source><translation>Перевірка й застосування</translation></message>
    <message><source>Review all pending changes and apply them in one batch.</source><translation>Перевірити всі зміни в очікуванні й застосувати одним пакетом.</translation></message>
    <message><source>Safe shutdown</source><translation>Безпечне вимкнення</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Система безпечно вимкнеться через UWF. Доступно навіть за повного накладання UWF.</translation></message>
    <message><source>Safe restart</source><translation>Безпечний перезапуск</translation></message>
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
    <message><source>The system did not find the target.</source><translation>Система не знайшла цільовий об’єкт.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Система відхилила параметр: неправильний шлях або аргумент.</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Помилка операції. Об’єкт може використовувати інша програма, наприклад Провідник чи відкритий файл. Закрийте такі програми й повторіть.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Операцію не виконано; подробиці в журналі.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Операцію відхилено (код %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Невідомо.</translation></message>
    <message><source>Commit canceled</source><translation>Фіксацію скасовано</translation></message>

    <message><source>Succeeded</source><translation>Успішно</translation></message>
    <message><source>Skipped</source><translation>Пропущено</translation></message>
    <message><source>Failed</source><translation>Помилка</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>Успішно: %1; з помилкою: %2.</translation></message>

    <message><source>Existed before</source><translation>Існував до видалення</translation></message>
    <message><source>Exists after</source><translation>Існує після видалення</translation></message>
    <message><source>Error code</source><translation>Код помилки</translation></message>
    <message><source>Reason</source><translation>Причина</translation></message>
    <message><source>Previous page</source><translation>Попередня сторінка</translation></message>
    <message><source>First page</source><translation>Перша сторінка</translation></message>
    <message><source>Last page</source><translation>Остання сторінка</translation></message>
    <message><source>Next page</source><translation>Наступна сторінка</translation></message>
    <message><source>Copy selected rows</source><translation>Копіювати вибрані рядки</translation></message>
    <message><source>Copy all</source><translation>Копіювати все</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Це виключення реєстру потрібне, поки список автофіксації містить збережені записи. Очистьте список перед видаленням виключення.</translation></message>
    <message><source>Copy current page</source><translation>Копіювати поточну сторінку</translation></message>
    <message><source>No pending changes</source><translation>Немає змін в очікуванні</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Виключення реєстру · Загальні налаштування.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Перейти до налаштувань захисту, виключень файлів і списку автофіксації тому %1.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Не підтримується</translation></message>
    <message><source>Review and apply changes</source><translation>Перевірка й застосування змін</translation></message>
    <message><source>Confirm apply</source><translation>Підтвердити застосування</translation></message>
    <message><source>Result</source><translation>Результат</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Фільтр: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Задання порогу попередження: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Задання критичного порогу: %1 MB</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ Тип / максимальну місткість не застосовано: UWF увімкнено. Спочатку вимкніть UWF і перезапустіть пристрій.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Задання типу накладання: %1</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ Максимальну місткість накладання встановлено на %1 МБ</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Том %1 · Захист UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Том %1 · Прив’язка за: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Том %1 · Додавання виключення файлу: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Том %1 · Вилучення виключення файлу: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Додавання виключення реєстру: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Вилучення виключення реєстру: %1</translation></message>
    <message><source>Time</source><translation>Час</translation></message>
    <message><source>Level</source><translation>Рівень</translation></message>

    <message><source>Message</source><translation>Повідомлення</translation></message>
    <message><source>Committing staged files</source><translation>Фіксація файлів зі списку</translation></message>
    <message><source>No additional error details were provided.</source><translation>Додаткові відомості про помилку відсутні.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Автоматична фіксація файлів несподівано зупинилася:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Автоматична фіксація файлів зупинилася через невідому помилку.</translation></message>
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
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>Знайдено файлів: %1 · Зафіксовано: %2 · Пропущено файлів: %3 · Пропущено записів: %4 · Помилок: %5</translation></message>
    <message><source>File staging preparation</source><translation>Підготовка до фіксації файлів зі списку</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Точки повторної обробки не підтримуються.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Можна додавати лише абсолютні шляхи на локальних томах.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Можна додавати лише абсолютні шляхи на локальних томах.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Не вдалося перевірити шлях зі списку.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Файл зі списку тепер указує на шлях іншого типу.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Папка зі списку тепер указує на шлях іншого типу.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Не вдалося повністю перелічити вміст папки зі списку.</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>Не можна рекурсивно обробляти корінь тому.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Постачальник UWF відхилив фіксацію.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Помилка фіксації: Невідома помилка.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Додаткових помилок у журналі: %1.</translation></message>



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
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>Імпортувати команди uwfmgr до чернетки. Зміни записуються лише після підтвердження в розділі «Перевірка й застосування».</translation></message>
    <message><source>Load from file…</source><translation>Завантажити з файлу…</translation></message>
    <message><source>Load recommended configuration</source><translation>Завантажити рекомендовану конфігурацію</translation></message>
    <message><source>Choose recommended configuration</source><translation>Вибрати рекомендовану конфігурацію</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Виберіть групи рекомендованої конфігурації. Перед імпортом можна перевірити й видалити окремі команди.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>З офіційної документації Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Виключення UWF, рекомендовані Microsoft</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>Перевірте ці рекомендації перед імпортом.</translation></message>
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
    <message><source>Command import is not ready.</source><translation>Імпорт команд не готовий.</translation></message>
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
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>Не застосовано змін: %1.

Відкинути їх і продовжити?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Неповна команда uwfmgr</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Відсутній аргумент: Розмір (MB)</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>Розмір має бути невід’ємним цілим числом (МБ)</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Відсутній аргумент: Накладання · Тип (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Неприпустиме значення: Накладання · Тип: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Відсутній аргумент: Том (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Том має бути буквою диска, наприклад C:</translation></message>
    <message><source>Missing path argument</source><translation>Відсутній аргумент: Шлях</translation></message>
    <message><source>Missing registry key argument</source><translation>Відсутній аргумент: Реєстр · Ключ</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Незакрита подвійна лапка в команді</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Неочікуваний зайвий аргумент: %1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>Цю команду uwfmgr неможливо імпортувати.</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Потрібний стан уже встановлено; дія не потрібна</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Шлях не на цьому томі або том не підтримує виключення файлів (наприклад, exFAT / ReFS)</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>Цей шлях неможливо додати до виключень.</translation></message>
    <message><source>Invalid size value: %1</source><translation>Неприпустиме значення: Розмір: %1</translation></message>
    <message><source>maximum capacity</source><translation>максимальна місткість</translation></message>
    <message><source>warning threshold</source><translation>Поріг попередження</translation></message>
    <message><source>critical threshold</source><translation>Критичний поріг</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>Глобальна конфігурація недоступна; команду не додано до змін, що очікують застосування.</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>Невідомий том %1 (немає відповідної конфігурації тому)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Том %1: Потрібний стан уже встановлено; дія не потрібна</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>Шлях %1 не містить літери диска; неможливо визначити вкладку тому.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Том %1: UWF Не підтримується</translation></message>
    <message><source>file exclusion</source><translation>Виключення файлів</translation></message>
    <message><source>registry exclusion</source><translation>Виключення реєстру</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Вкладка реєстру відсутня; виключення реєстру недоступні</translation></message>
    <message><source>Unsupported command</source><translation>Команда: Не підтримується</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>Показати або приховати віджет накладання.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Увімкнено</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: вимкнено</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Невідомо</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Використано %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Розширений режим</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Узгодити автоматичну фіксацію файлів із вимкненням і перезапуском Windows.</translation></message>
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
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>Служба зараз не може отримувати попередні сповіщення Windows про завершення роботи.</translation></message>
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
    <message><source>Show main window</source><translation>Показати головне вікно</translation></message>
    <message><source>Restore default position</source><translation>Відновити типове розташування</translation></message>
    <message><source>Show overlay widget</source><translation>Показати віджет накладання</translation></message>
    <message><source>Hide overlay widget</source><translation>Приховати віджет накладання</translation></message>
    <message><source>Exit application</source><translation>Вийти з програми</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Помилка: Читання значень реєстру (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Перелік файлів накладання: Невідома помилка.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Помилка: Дія з фільтром: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Помилка: Читання стану фільтра</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Помилка: Задання порогу попередження</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Помилка: Задання критичного порогу</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Помилка: Читання стану накладання</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Помилка: Задання типу накладання</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ Не вдалося встановити максимальну місткість</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Помилка: Читання конфігурації накладання</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Помилка: Захист тому %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Помилка: Задання прив’язки тому %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Том %1 · Помилка: Додавання виключення файлу %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Том %1 · Помилка: Вилучення виключення файлу %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Помилка: Читання конфігурації тому</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Помилка: Додавання виключення реєстру %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Помилка: Вилучення виключення реєстру %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Помилка: Зміна параметрів збереження реєстру</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Помилка: Читання фільтра реєстру</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Не підтримується</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Читання стану об’єкта: Невідома помилка.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Невідома помилка.</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>Не вдалося перевірити стан об’єкта, тому результат операції невідомий: %1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>Система прийняла запит на видалення, але після перевірки об’єкт досі існує.</translation></message>
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
Керування захистом томів, накладанням, виключеннями й файлами зі списку автофіксації.</translation></message>
    <message><source>Snapshot details</source><translation>Відомості про знімок</translation></message>
    <message><source>Account</source><translation>Обліковий запис</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Додайте конфігурацію UWF зі сценарію або тексту.</translation></message>
    <message><source>Add exclusion</source><translation>Додати виключення</translation></message>
    <message><source>Add files or folders</source><translation>Додати файли або папки</translation></message>
    <message><source>Add selected items</source><translation>Додати вибрані елементи</translation></message>
    <message><source>Added</source><translation>Додано</translation></message>
    <message><source>Administrator</source><translation>Адміністратор</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>Вибрані операції завершено. UWF може пропускати виключені об’єкти.</translation></message>
    <message><source>All files (*)</source><translation>Усі файли (*)</translation></message>
    <message><source>All levels</source><translation>Усі рівні</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Порожній діагностичний знімок не означає, що накладання не містить змін.</translation></message>
    <message><source>Application path</source><translation>Шлях застосунку</translation></message>
    <message><source>Application result</source><translation>Результат застосування</translation></message>
    <message><source>Apply changes</source><translation>Застосувати зміни</translation></message>
    <message><source>Architecture</source><translation>Архітектура</translation></message>
    <message><source>Automatic</source><translation>Автоматично</translation></message>
    <message><source>Automatic staged file commits</source><translation>Автоматична фіксація файлів зі списку</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>Автоматичну фіксацію файлів не завершено</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>Автоматично фіксувати придатні файли зі списку під час вимкнення та перезапуску Windows.</translation></message>
    <message><source>Available</source><translation>Доступно</translation></message>
    <message><source>Back</source><translation>Назад</translation></message>
    <message><source>Background service</source><translation>Фонова служба</translation></message>
    <message><source>Binding</source><translation>Прив’язка</translation></message>
    <message><source>By drive letter</source><translation>За буквою диска</translation></message>
    <message><source>By volume ID</source><translation>За ідентифікатором тому</translation></message>
    <message><source>Cancel reading</source><translation>Скасувати читання</translation></message>
    <message><source>Completed operations cannot be undone.</source><translation>Завершені операції неможливо скасувати.</translation></message>
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
    <message><source>Commit file changes?</source><translation>Зафіксувати зміни файлу?</translation></message>
    <message><source>Commit folder changes?</source><translation>Зафіксувати зміни папки?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Зафіксувати зміни ключа реєстру?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Зафіксувати зміни значення реєстру?</translation></message>
    <message><source>Compatibility mode</source><translation>Режим сумісності</translation></message>
    <message><source>Configuration applied</source><translation>Конфігурацію застосовано</translation></message>
    <message><source>Configuration error</source><translation>Помилка конфігурації</translation></message>
    <message><source>Configured</source><translation>Налаштовано</translation></message>
    <message><source>Confirm before continuing</source><translation>Підтвердьте перед продовженням</translation></message>
    <message><source>Connected and authenticated</source><translation>Підключено й автентифіковано</translation></message>
    <message><source>Connecting…</source><translation>Підключення…</translation></message>
    <message><source>Contact author</source><translation>Зв’язатися з автором</translation></message>
    <message><source>Continue editing</source><translation>Продовжити редагування</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>Якщо продовжити перезапуск, незафіксовані зміни файлів можуть бути втрачені.</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>Якщо продовжити вимкнення, незафіксовані зміни файлів можуть бути втрачені.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Узгодити з вимкненням Windows</translation></message>
    <message><source>Copy all logs</source><translation>Копіювати весь журнал</translation></message>
    <message><source>Copy full report</source><translation>Копіювати повний звіт</translation></message>
    <message><source>Copy name</source><translation>Копіювати ім’я</translation></message>
    <message><source>Copy path</source><translation>Копіювати шлях</translation></message>
    <message><source>Copy selection</source><translation>Копіювати вибране</translation></message>
    <message><source>Copy version</source><translation>Копіювати версію</translation></message>
    <message><source>Counting staged files…</source><translation>Підрахунок файлів зі списку…</translation></message>
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
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>Розширений режим недоступний. Скористайтеся пунктом «Безпечне вимкнення» або «Безпечний перезапуск», щоб зафіксувати файли зі списку.</translation></message>
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
    <message><source>File commits have started and cannot be canceled.</source><translation>Фіксація файлів почалася, і її неможливо скасувати.</translation></message>
    <message><source>File commits incomplete</source><translation>Фіксацію файлів не завершено</translation></message>
    <message><source>File type</source><translation>Тип файлу</translation></message>
    <message><source>Files and folders</source><translation>Файли та папки</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Файли та папки можна вибирати разом.</translation></message>
    <message><source>Filter is disabled</source><translation>Фільтр вимкнено</translation></message>
    <message><source>Fit columns to contents</source><translation>Припасувати стовпці до вмісту</translation></message>
    <message><source>Folder address</source><translation>Адреса папки</translation></message>
    <message><source>Folder name</source><translation>Ім’я папки</translation></message>
    <message><source>Folders</source><translation>Папки</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>Додати цей розділ до виключень наступного сеансу.</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>Додати цей шлях до виключень наступного сеансу.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Глобальний виключення реєстру · включно з підрозділами</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Перейдіть до батьківської папки або виберіть інше розташування.</translation></message>
    <message><source>Home</source><translation>Особиста папка</translation></message>
    <message><source>Import configuration</source><translation>Імпорт конфігурації</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>Імпорт оновлює чернетку. Підтвердьте зміни в розділі «Перевірка й застосування», щоб записати їх до системи.</translation></message>
    <message><source>Import results</source><translation>Результати імпорту</translation></message>
    <message><source>Imported</source><translation>Імпортовано</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>Список показує результати команд для чернетки конфігурації; виберіть рядок, щоб переглянути повну команду й подробиці.</translation></message>
    <message><source>Imported: %1</source><translation>Імпортовано: %1</translation></message>
    <message><source>Information</source><translation>Інформація</translation></message>
    <message><source>Item details</source><translation>Подробиці елемента</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>Фіксація великої кількості файлів зі списку може тривати довше.</translation></message>
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
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Якщо UWF захищає диск застосунку, спочатку переконайтеся, що його файли збережуться після перезапуску. Фіксація великої кількості файлів зі списку може подовжити вимкнення чи перезапуск.</translation></message>
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
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>Автоматично фіксуються лише файли зі списку. Інші незафіксовані зміни можуть бути втрачені після перезапуску.</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>Автоматично фіксуються лише файли зі списку. Інші незафіксовані зміни можуть бути втрачені після вимкнення.</translation></message>
    <message><source>Open</source><translation>Відкрити</translation></message>
    <message><source>Open folder</source><translation>Відкрити папку</translation></message>
    <message><source>Operating system</source><translation>Операційна система</translation></message>
    <message><source>Operation and security</source><translation>Робота та безпека</translation></message>
    <message><source>Original command</source><translation>Початкова команда</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Досягнуто критичного порога накладання. Збережіть зміни й безпечно перезавантажте систему.</translation></message>
    <message><source>Maximum overlay capacity</source><translation>Максимальна місткість накладання</translation></message>
    <message><source>Overlay type</source><translation>Тип накладання</translation></message>
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
    <message><source>Files to commit</source><translation>Файли для фіксації</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Записи поточного запуску з’являться тут після завершення читання.</translation></message>

    <message><source>Refresh folder</source><translation>Оновити папку</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Ключ реєстру або налаштування постійного зберігання</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>Після запиту на скасування решту завдань буде зупинено, коли поточна операція завершиться.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Вилучити цей ключ із виключень наступного сеансу.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Вилучити цей шлях із виключень наступного сеансу.</translation></message>
    <message><source>Removed</source><translation>Вилучено</translation></message>
    <message><source>Rename</source><translation>Перейменувати</translation></message>
    <message><source>Rename…</source><translation>Перейменувати…</translation></message>
    <message><source>Repair required</source><translation>Потрібне виправлення</translation></message>
    <message><source>Replace file</source><translation>Замінити файл</translation></message>
    <message><source>Replace the existing file?</source><translation>Замінити наявний файл?</translation></message>
    <message><source>Required configuration</source><translation>Необхідна конфігурація</translation></message>
    <message><source>Restart anyway</source><translation>Усе одно перезавантажити</translation></message>
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>Перезапуск продовжиться після фіксації. Не закривайте застосунок.</translation></message>
    <message><source>Restore</source><translation>Відновити</translation></message>
    <message><source>Retry</source><translation>Повторити</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Перегляньте подробиці помилок перед продовженням вимкнення.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Перегляньте подробиці помилок перед перезавантаженням.</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>Деякі зміни, можливо, вже записано. Оновіть стан системи перед повторною спробою.</translation></message>
    <message><source>Review the result for each target.</source><translation>Перевірте результат для кожної цілі.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Перевірте ці зміни перед записом у систему.</translation></message>
    <message><source>Running</source><translation>Працює</translation></message>
    <message><source>Save</source><translation>Зберегти</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>Зафіксувати файли зі списку на диску, а потім перезапустити пристрій.</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>Зафіксувати файли зі списку на диску, а потім вимкнути пристрій.</translation></message>

    <message><source>Committing to disk</source><translation>Фіксація на диску</translation></message>
    <message><source>Committing…</source><translation>Фіксація…</translation></message>
    <message><source>Scanning staged paths</source><translation>Сканування шляхів зі списку</translation></message>
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
    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>Вимкнення продовжиться після фіксації. Не закривайте застосунок.</translation></message>
    <message><source>Size</source><translation>Розмір</translation></message>
    <message><source>Some changes could not be applied</source><translation>Не вдалося застосувати деякі зміни</translation></message>
    <message><source>Some files could not be committed</source><translation>Не вдалося зафіксувати деякі файли</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Не вдалося прочитати частину відомостей про обладнання. Інші діагностичні відомості й надалі можна переглянути та скопіювати.</translation></message>
    <message><source>Source code</source><translation>Початковий код</translation></message>

    <message><source>Staged path</source><translation>Шлях зі списку</translation></message>
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
    <message><source>Takes effect when applied</source><translation>Набуває чинності після застосування</translation></message>
    <message><source>Used when UWF starts</source><translation>Використовується під час запуску UWF</translation></message>
    <message><source>Mixed activation times</source><translation>Різний час набуття чинності</translation></message>
    <message><source>Target</source><translation>Ціль</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Постійне зберігання ліцензій служб терміналів</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Постачальник UWF не зміг завершити перелік. Це може статися за великого накладання або нестачі системних ресурсів.</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>Конфігурацію записано в систему.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Поточні записи журналу буде видалено без можливості відновлення.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Папка призначення не існує.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Не вдалося прочитати стан служби розширеного режиму. Перевірте дозволи служби й повторіть спробу.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>У списку можуть бути пропущені файли або присутні вже зафіксовані. Не використовуйте його як список для фіксації.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Вибраний елемент більше не існує: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>Службу встановлено, але зупинено. Запустіть її, щоб відновити автоматичну фіксацію файлів.</translation></message>
    <message><source>This PC</source><translation>Цей комп’ютер</translation></message>
    <message><source>This folder is empty</source><translation>Ця папка порожня</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Звіт може містити імена облікових записів, ідентифікатори пристроїв і шляхи файлів. Перевірте його перед поширенням.</translation></message>

    <message><source>Try another keyword or log level.</source><translation>Спробуйте інше ключове слово або рівень журналу.</translation></message>
    <message><source>Try another search term.</source><translation>Спробуйте інший пошуковий запит.</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>Перш ніж змінювати тип або максимальну місткість накладання, вимкніть UWF і перезапустіть пристрій.</translation></message>
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
    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>Обробку деяких елементів не завершено або результат не вдалося перевірити. Виберіть елемент, щоб переглянути повний шлях і подробиці.</translation></message>
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
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>Без розширеного режиму скористайтеся в цьому застосунку пунктом «Безпечне вимкнення» або «Безпечний перезапуск», щоб зафіксувати файли зі списку.</translation></message>
    <message><source>Write protection</source><translation>Захист від запису</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>Записати ці зміни конфігурації в систему?</translation></message>
    <message><source>staged files</source><translation>файли зі списку</translation></message>
    <message><source>uwfmgr commands</source><translation>команди uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Позначити вибрані шляхи для видалення. Зміни набудуть чинності після застосування.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Видалити зі списку автофіксації після застосування.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Додати до списку автофіксації після застосування.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Застосування цього плану очистить збережений список автофіксації та видалить його виключення реєстру, коли воно більше не потрібне.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Зміни списку автофіксації потрібно застосувати в UWF Manager; їх не можна експортувати як команди uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Список автофіксації зберігається після застосування</translation></message>
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
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Видалити й зафіксувати видалення лише для об’єктів, обхід яких завершився успішно. Непрочитані гілки та їхні батьківські папки або ключі буде збережено.</translation></message>
    <message><source>Deleting and committing</source><translation>Видалення та фіксація</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Видалення та фіксацію неможливо скасувати. Обробляється лише вибрана область; відкладені налаштування UWF не застосовуються.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Визначення області. Непрочитані гілки записуються до журналу, обхід решти вмісту триває.</translation></message>
    <message><source>Export traversal records</source><translation>Експортувати журнал обходу</translation></message>
    <message><source>No actionable content</source><translation>Немає вмісту для обробки</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Об’єктів для обробки не знайдено. Нічого не видалено й не зафіксовано.</translation></message>
    <message><source>No changes have been committed.</source><translation>Зміни ще не зафіксовано</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Нічого не видалено й не зафіксовано</translation></message>
    <message><source>Path</source><translation>Шлях</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>Назавжди видалити вибраний вміст і зафіксувати видалення.</translation></message>
    <message><source>Preparation complete</source><translation>Підготовку завершено</translation></message>
    <message><source>Preparing commit</source><translation>Підготовка до фіксації</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Підготовка видалення та фіксації</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Обробити лише успішно переглянутий вміст</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Точки повторної обробки пропускаються, щоб обхід не виходив за вибрану область.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Перезапуск не скасовує фіксацію. Обробляється лише вибрана область; відкладені налаштування UWF не застосовуються.</translation></message>
    <message><source>Retry failed items</source><translation>Повторити невдалі</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>Зафіксувати зміни у вибраній області.</translation></message>
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

    <message><source>All operations succeeded</source><translation>Усі операції успішні</translation></message>
    <message><source>Some operations succeeded</source><translation>Частина операцій успішна</translation></message>
    <message><source>No operations succeeded</source><translation>Жодна операція не була успішною</translation></message>
    <message><source>No targets to process</source><translation>Немає об’єктів для обробки</translation></message>
    <message><source>Commit interrupted</source><translation>Фіксацію перервано</translation></message>
    <message><source>%1 items were not processed.</source><translation>Не оброблено елементів: %1.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>Не вдалося виконати операцій із реєстром: %1.</translation></message>
    <message><source>The folder does not exist.</source><translation>Папка не існує.</translation></message>
    <message><source>Access denied.</source><translation>Доступ заборонено.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Елемент із таким ім’ям уже існує.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Не вдалося створити папку. Перевірте ім’я та права доступу.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Не вдалося перейменувати елемент. Перевірте ім’я та права доступу.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Цей оглядач не підтримує перейменування.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Чернетка списку автофіксації змінилася після відкриття цього плану. Перегляньте новий план.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Збережений список автофіксації змінився після відкриття цього плану. Оновіть дані й перегляньте зміни.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Збережений список автофіксації не відповідає прийнятому плану.</translation></message>
    <message><source>Export interface settings</source><translation>Експортувати налаштування інтерфейсу</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>Зберегти поточну тему, мову й видимість віджета накладання до UWF.ini поруч із застосунком.</translation></message>
    <message><source>Interface settings saved</source><translation>Налаштування інтерфейсу збережено</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Обробку списку автофіксації зупинено. Усуньте помилку перед повторною спробою.</translation></message>
    <message><source>No net change for this setting</source><translation>Остаточне значення цього налаштування не змінилося після імпорту</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Кінцеве значення чернетки: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Кінцеве значення чернетки: %1 → %2 (запитано: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Запитано: %1 → %2; замінено наступною командою. Кінцеве значення чернетки → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Чернетку %1 оновлено</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Чернетку налаштування захисту тому %1 оновлено → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Виберіть записи для видалення.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Наразі неможливо відновити вибрані записи.</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>Прямий запис у вільний простір</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>Постійне накладання</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>Очистити постійні дані під час наступного запуску</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>Режим носія лише для читання</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>Використовує вільне місце на диску для записів, зменшуючи заповнення зарезервованого накладання; підтримує RAM (не рекомендовано) та Disk (рекомендовано).</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>Зберігає записи в накладанні Disk після перезапуску; недоступно для накладань RAM.</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>Скидає постійне накладання: видаляє його записи під час наступного запуску та відновлює стан на момент увімкнення накладання</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>Потрібні права адміністратора; конфігурація накладання доступна лише для читання.</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>Редагування тимчасово недоступне. Дочекайтеся завершення поточної операції.</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>Не вдалося прочитати конфігурацію накладання. Оновіть її перед редагуванням.</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>Ця система не підтримує очищення постійних даних під час наступного запуску.</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>Поточний запит очищення невідомий. Оновіть конфігурацію перед редагуванням.</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>Для зміни цього запиту UWF і постійне накладання Disk мають бути активними в поточному сеансі.</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>Для очищення під час наступного запуску потрібно зберегти постійне накладання Disk. Скасуйте запит очищення або збережіть постійне накладання.</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>Постійне накладання потребує Disk. Вимкніть його або виберіть Disk.</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>Режим носія лише для читання потребує RAM. Вимкніть його або виберіть RAM.</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>UWF увімкнено в поточному сеансі. Вимкніть UWF і перезавантажте систему перед зміною цього параметра.</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>Конфігурація змінилася. Закрийте й знову відкрийте це вікно перед редагуванням.</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>Додаткові параметри накладання</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>Поточне накладання</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>Наступне накладання</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>Чернетка містить несумісні або недоступні параметри накладання. Поверніться до редагування та виправте конфігурацію.</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>Очищення заплановано на наступний запуск. Скасуйте запит на очищення, перш ніж вимикати постійне накладання.</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>Наступний сеанс не зберігає постійне накладання Disk. Збережіть його, перш ніж запитувати очищення.</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>Постійне накладання доступне лише для Disk; для наступного сеансу вибрано накладання RAM.</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>Увімкнення режиму носія лише для читання потребує RAM; для наступного сеансу вибрано накладання Disk.</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>Не вдалося підтвердити поточний стан постійного накладання. Прочитайте конфігурацію ще раз.</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>Набуде чинності під час наступного запуску після застосування.</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>Очищення заплановано</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>Очищення не заплановано</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>Очищає один раз під час наступного запуску, потім продовжує зберігати дані. Поточний сеанс не очищається негайно.</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>Скасовує заплановане очищення під час наступного запуску, зберігаючи дані постійного накладання.</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>Скидання постійного накладання · Одноразовий запит</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Обробка…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>Зачекайте на завершення операції.</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · Наступний сеанс</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>Повернутися до налаштувань</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>Умови зміни</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>Змінено</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>Перевірка конфліктів конфігурації</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>Конфлікт конфігурації</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>Умови вимкнення</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>UWF і постійне накладання Disk мають бути активними в поточному сеансі. Для очищення даних також потрібно зберегти постійне накладання Disk у наступному сеансі.</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>Умови ввімкнення</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Функція</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>Опис функції</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>Відомості про функцію «%1»</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>Усуньте ці конфлікти конфігурації, перш ніж зберігати чернетку.</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>Скасувати зміни</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>Набуття чинності</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>Зараз цей параметр не можна змінити</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>UWF має бути вимкнено в поточному сеансі, а накладання наступного сеансу має використовувати Disk.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>UWF має бути вимкнено в поточному сеансі, а накладання наступного сеансу має використовувати RAM.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>UWF має бути вимкнено в поточному сеансі. Перш ніж вимкнути постійне накладання, скасуйте заплановане очищення постійних даних.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>UWF має бути вимкнено в поточному сеансі. Якщо вимкнення задано лише для наступного сеансу, перед зміною все одно потрібен перезапуск.</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>Умови використання</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Докладніше</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>Лише перегляд</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>Чому не можна змінити «%1»</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>UWF увімкнено в поточному сеансі, тому ці параметри накладання змінити неможливо. Скасуйте зміни або вимкніть UWF і перезавантажте систему.</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>Переглянути умови зміни</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>Зміни конфігурації не записано. Можна повторити спробу.</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>Зміни конфігурації не були потрібні.</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>Для секрету облікового запису комп’ютера домену ($MACHINE.ACC) використовуйте налаштування «Постійне зберігання секретного ключа домену».</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>Сам цей розділ не можна виключити. Виберіть один із його підрозділів.</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>Виключення реєстру має вказувати непорожній підрозділ HKEY_LOCAL_MACHINE (HKLM).</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>Цей сценарій містить лише налаштування, які можна подати командами uwfmgr. Не включено:</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>Прив’язка томів</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>Налаштування без надійної адресації за літерою диска</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>Сценарій команд</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>Конфігурація відповідає цілі наступного сеансу. Переглянути або експортувати підтримувані налаштування можна на вкладці «Сценарій команд».</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>Налаштування з керівними символами в шляхах або іменах</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>Команди, що залежать від пропущених налаштувань</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>Виключення UWF не дають змоги надійно зафіксувати зміни цього об’єкта. Видаліть конфліктне виключення та перезавантажте систему перед фіксацією.</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>Виключення UWF не дають змоги надійно зафіксувати список автофіксації. Видаліть конфліктне виключення та перезавантажте систему перед застосуванням цієї зміни.</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF не може гарантувати, що операція збереже саме цей розділ реєстру як ціль.</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>Перш ніж видалити виключення реєстру, підтвердьте, що збережений список автофіксації порожній.</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>Очікує підтвердження</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Порожньо</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>Системний том зараз не захищений UWF; змін реєстру для фіксації немає.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>Зміни реєстру вже збережено постійно; фіксація UWF не потрібна.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>Зміни реєстру вже збережено постійно; UWF не може виконати це видалення.</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>У шляхах виключень UWF потрібно використовувати зворотну скісну риску.</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>Шляхи до файлів не можуть містити нульових символів.</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>Перш ніж додавати виключення файлів, увімкніть захист тому в цій чернетці.</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>Шлях виключення містить символи, які не підтримує UWF.</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>Шлях виключення перевищує граничну довжину UWF.</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>Коли HORM активний, фіксація змін файлів і реєстру недоступна.</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>Коли HORM активний, запис до реєстру недоступний.</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>У режимі носія лише для читання записи до реєстру неможливо зберегти постійно.</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>У режимі носія лише для читання фіксація змін файлів і реєстру недоступна.</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>Перехоплює запис на фізичний носій, зокрема метадані. Виключення не обходять захист. Фіксація окремих змін файлів і реєстру недоступна; Full Commit — окрема глобальна операція.</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>Коли HORM активний, змінювати налаштування UWF неможливо.</translation></message>
    <message><source>Default value · this value only</source><translation>Значення за замовчуванням · лише це значення</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>Зміни файлів уже збережено постійно; фіксація UWF не потрібна.</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>Останнє успішне отримання даних про використання накладання: %1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>Виявлені розмір або тип сховища накладання поза допустимим для запису діапазоном</translation></message>
    <message><source>Overlay (last observed)</source><translation>Накладання (останні дані)</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>Не вдалося оновити використання накладання.
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>Не вдалося оновити використання накладання. Показано останні успішно отримані дані від %1.
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>Не вдалося зіставити шлях із локальним томом.</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF: увімкнено (останні дані)</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>Зміни файлів уже збережено постійно; UWF не може виконати це видалення.</translation></message>
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>Доступно %1 GB</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>Потрібні права адміністратора.</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>Виконується операція. Зачекайте.</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>Намагається зберегти на диск усі додавання, зміни та видалення в усіх захищених томах із моменту цього запуску. Це стосується файлів і системних налаштувань; вибрати лише частину змін неможливо.</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>Автоматичне збереження недоступне в поточному режимі</translation></message>
    <message><source>Cannot be undone once started</source><translation>Неможливо скасувати після початку</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>Виберіть місце зберігання дискового накладання</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>Виберіть місце зберігання дискового накладання.</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>Зміни конфігурації тимчасово недоступні.</translation></message>
    <message><source>Configured, inactive this session</source><translation>Налаштовано, не діє в цьому сеансі</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>Налаштовано; набуде чинності після перезапуску</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>Налаштовано; набуде чинності після перезапуску. Накладання поточного запуску не буде переміщено до нового розташування.</translation></message>
    <message><source>Configure…</source><translation>Налаштувати…</translation></message>
    <message><source>Current UWF</source><translation>Поточний стан UWF</translation></message>
    <message><source>Currently configured storage volume</source><translation>Том зберігання в поточній конфігурації</translation></message>
    <message><source>Default behavior</source><translation>Типова поведінка</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>Вимкніть HORM і перезапустіть комп’ютер перед повною фіксацією томів.</translation></message>
    <message><source>Disabled · restart required</source><translation>Вимкнено · потрібен перезапуск</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>Вимкнення HORM не вимикає UWF або режим носія лише для читання й не зберігає зміни на захищених дисках. Перезапустіть комп’ютер перед перевіркою доступних дій.</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>Увімкніть UWF і перезапустіть комп’ютер перед увімкненням HORM.</translation></message>

    <message><source>Full volume commit</source><translation>Повна фіксація томів</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>Режим повної фіксації томів (увімкнений до перезапуску)</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>Режим повної фіксації томів уже ввімкнено. Перезапустіть комп’ютер перед повторним використанням.</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>Повну фіксацію томів уже розпочато. Перезапустіть комп’ютер перед увімкненням HORM.</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>Повна фіксація томів доступна лише за активного режиму носія лише для читання.</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>Режим повної фіксації томів залишається ввімкненим до перезапуску. Нові зміни зберігаються безпосередньо. Окрема фіксація залишається недоступною.</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>Повна фіксація томів недоступна в цій системі.</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM увімкнено. Налаштування UWF й окрема фіксація недоступні. Для їх зміни вимкніть HORM і перезапустіть комп’ютер.</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM вимкнено. Перезапустіть комп’ютер перед зміною налаштувань UWF.</translation></message>
    <message><source>HORM is not available on this system.</source><translation>HORM недоступний у цій системі.</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM потребує поточної конфігурації RAM у режимі носія лише для читання.</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM потребує поточного накладання RAM.</translation></message>

    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>У режимі Disk реалізація системи може типово вмикати цей параметр; перевірте поточний стан у налаштуваннях.</translation></message>
    <message><source>Inactive this session</source><translation>Не діє в цьому сеансі</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>Подальші зміни також зберігатимуться до перезапуску</translation></message>

    <message><source>Maintenance and recovery</source><translation>Обслуговування та відновлення</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>Перш ніж продовжити, переконайтеся, що хочете зберегти саме цей стан.</translation></message>

    <message><source>Next startup: %1</source><translation>Під час наступного запуску: %1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>Немає придатного тому зберігання. Оновіть список томів.</translation></message>
    <message><source>Not enabled</source><translation>Не ввімкнено</translation></message>
    <message><source>Not selected</source><translation>Не вибрано</translation></message>
    <message><source>Not supported</source><translation>Не підтримується</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>Після початку нові зміни записуються безпосередньо на диск до перезапуску комп’ютера.</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>Одночасно можна використовувати лише один том зберігання. Вибір нового тому замінює попереднє розташування та встановлює Disk для наступного накладання.</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>Місткість накладання (зберегти поточне налаштування)</translation></message>
    <message><source>Overlay configuration</source><translation>Конфігурація накладання</translation></message>

    <message><source>Processing…</source><translation>Обробка…</translation></message>
    <message><source>Read state again</source><translation>Зчитати стан знову</translation></message>
    <message><source>Read the current UWF state first.</source><translation>Спочатку зчитайте поточний стан UWF.</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>Режим носія лише для читання не підтримує окрему фіксацію або автоматичне збереження підготовлених файлів.</translation></message>
    <message><source>Reading state…</source><translation>Зчитування стану…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>Перезапустіть комп’ютер і зчитайте поточний стан перед повторним увімкненням HORM.</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>Відновлювати той самий робочий стіл і запущені програми під час кожного запуску.</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>Перегляньте доступні операції фіксації та їхні вимоги.</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>Зберегти зміни в усіх захищених томах</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>Зберегти зміни в усіх захищених томах. Ознайомтеся з наслідками, перш ніж продовжити.</translation></message>


    <message><source>State unknown</source><translation>Стан невідомий</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>Успішно збережені зміни вже неможливо скасувати перезапуском.</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>Успішно збережені зміни залишаться</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>Стан HORM невідомий. Зчитайте стан знову, перш ніж продовжити.</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>Стан HORM невідомий. Оновіть його, перш ніж продовжити.</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>Стан фіксації або HORM невідомий. Оновіть його, перш ніж продовжити.</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>Не вдалося зчитати поточний стан UWF. Оновіть його, перш ніж продовжити.</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>Поточне накладання використовує RAM у режимі носія лише для читання</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>Не вдалося зчитати поточний режим захисту. Оновіть стан.</translation></message>
    <message><source>The result could not be confirmed</source><translation>Не вдалося підтвердити результат</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>Це виключення налаштовано, але воно не обходить захист у поточному сеансі.</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>Це також може змінити прямий запис у вільний простір і постійне накладання. Перевірте додаткові налаштування перед перезапуском.</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>Захист UWF активний у поточному сеансі</translation></message>
    <message><source>%1 (will be replaced)</source><translation>%1 (буде замінено)</translation></message>
    <message><source>Add full volume commit to the draft?</source><translation>Додати повну фіксацію томів до чернетки?</translation></message>
    <message><source>Add to draft</source><translation>Додати до чернетки</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>Стосується всіх захищених томів; почнеться лише після застосування цієї чернетки.</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>Застосування цієї чернетки запускає повну фіксацію томів. Збережені зміни неможливо скасувати перезапуском, а нові зміни зберігаються до перезапуску.</translation></message>
    <message><source>Configure storage location</source><translation>Налаштувати місце зберігання</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>Налаштування місця зберігання недоступне в цій системі.</translation></message>
    <message><source>Current: %1</source><translation>Зараз: %1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>Вимкніть HORM і перезапустіть комп’ютер перед налаштуванням місця зберігання.</translation></message>
    <message><source>Disable saved-state startup?</source><translation>Вимкнути запуск зі збереженим станом?</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>Чернетка: %1 · потрібне застосування</translation></message>
    <message><source>Enable saved-state startup?</source><translation>Увімкнути запуск зі збереженим станом?</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>Повна фіксація томів (очікує застосування)</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>Повна фіксація томів очікує застосування. Вона почнеться лише після підтвердження у вікні перегляду та застосування.</translation></message>
    <message><source>Hibernate once / resume many</source><translation>Одна гібернація / багато відновлень</translation></message>
    <message><source>Not started</source><translation>Не розпочато</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>Вилучити повну фіксацію томів із незастосованих змін.</translation></message>
    <message><source>Save draft</source><translation>Зберегти чернетку</translation></message>

    <message><source>Will start when applied</source><translation>Почнеться після застосування</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>Змонтований стаціонарний диск не пройшов перевірку захисту. Перевірте диски у списку перед увімкненням HORM.</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>Перевіряє придатні для UWF стаціонарні диски, які залишаються змонтованими. Незмонтовані розділи можуть пропускатися.</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>Не вдалося перевірити передумови HORM. Повторно зчитайте стан, перш ніж продовжити.</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>Перевірка захисту змонтованих стаціонарних дисків</translation></message>
    <message><source>Not using a Disk overlay</source><translation>Накладка Disk не використовується</translation></message>
    <message><source>Selected storage volume</source><translation>Вибраний том зберігання</translation></message>
    <message><source>System drive default (%1)</source><translation>Типово на системному диску (%1)</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>Стан захисту змінився під час цього запуску. Перезавантажте комп’ютер перед увімкненням HORM.</translation></message>
    <message><source>Unable to read</source><translation>Не вдалося прочитати</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>Виберіть диск для зберігання тимчасових змін. Налаштування набуде чинності під час наступного запуску.</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>Вимкніть UWF і перезавантажте комп’ютер перед налаштуванням місця зберігання.</translation></message>
    <message><source>Met</source><translation>Виконано</translation></message>
    <message><source>Not met</source><translation>Не виконано</translation></message>
    <message><source>Partition</source><translation>Розділ</translation></message>


    <message><source>Protect all</source><translation>Захистити всі</translation></message>
    <message><source>Applied</source><translation>Застосовано</translation></message>
    <message><source>Not applied</source><translation>Не застосовано</translation></message>
    <message><source>Could not confirm</source><translation>Не вдалося підтвердити</translation></message>
    <message><source>Hibernation is not supported</source><translation>Гібернація не підтримується</translation></message>
    <message><source>Hibernation is not enabled</source><translation>Гібернацію не ввімкнено</translation></message>
    <message><source>Fast startup only</source><translation>Лише швидкий запуск</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>Груповий захист томів недоступний у цій системі.</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>Перед зміною захисту томів вимкніть HORM і перезапустіть комп’ютер.</translation></message>
    <message><source>Setup and operation</source><translation>Налаштування та порядок роботи</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. Підготуйте конфігурацію</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>Коли UWF вимкнено, налаштуйте для наступного запуску накладку RAM, режим носія лише для читання, захист томів і ввімкнення UWF. Застосуйте чернетку. Забезпечте підтримку гібернації S4 та наявність повного файлу гібернації.</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>Після перезапуску переконайтеся, що в поточному сеансі ввімкнено UWF, накладку RAM і режим носія лише для читання, а всі підключені томи фіксованих дисків, які підтримує UWF, захищено. Перевірте доступність гібернації системи.</translation></message>
    <message><source>2. Restart and verify</source><translation>2. Перезапустіть і перевірте</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. Увімкніть HORM і виконайте гібернацію</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>Додайте ввімкнення HORM до чернетки та виконайте перевірку й застосування змін. Застосування вмикає HORM і відповідає команді:</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>Після успішного ввімкнення підготуйте робочий стіл і програми, а потім один раз виконайте гібернацію, щоб зберегти базовий стан:</translation></message>
    <message><source>HORM usage</source><translation>Використання HORM</translation></message>
    <message><source>Volume protection</source><translation>Захист томів</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>Налаштуйте захист усіх томів для наступного сеансу.</translation></message>
    <message><source>Unprotect all</source><translation>Зняти захист з усіх</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>Наступний запуск: усі томи захищено</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>Наступний запуск: усі томи без захисту</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>Наступний запуск: окремі налаштування томів</translation></message>
    <message><source>System hibernation is available</source><translation>Гібернація системи доступна</translation></message>
    <message><source>Usage guide</source><translation>Інструкція</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>Захист усіх томів очікує застосування</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>Захист томів набуде чинності після перезапуску</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>Неможливо одночасно зняти захист з усіх томів і ввімкнути HORM.</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>Груповий захист томів не підтримується.</translation></message>



    <message><source>Storage volume:</source><translation>Том зберігання:</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>У поточному сеансі запуску виключення не обходять захист. Збереження окремих елементів і автоматичне збереження підготовлених файлів недоступні. Повне збереження томів стосується всіх захищених томів; перш ніж продовжити, ознайомтеся з описом цієї функції.</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>Багаторазове відновлення з того самого стану глибокого сну</translation></message>
    <message><source>Remove full volume commit draft</source><translation>Видалити чернетку повної фіксації томів</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>Увімкнено режим носія лише для читання</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>Не вдалося прочитати налаштований маркер сховища.</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>Список автофіксації конфліктує з виключенням файлу:
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>На цьому комп’ютері недоступна повна гібернація</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>Кілька томів сховища мають ненульовий маркер.</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>Не вдалося підтвердити дисковий тип накладання.</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>Не вдалося прочитати налаштоване розташування сховища.</translation></message>
    <message><source>The system drive could not be identified.</source><translation>Не вдалося визначити системний диск.</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>Прямий перезапуск пропускає фіксацію файлів зі списку.</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>Пряме вимкнення пропускає фіксацію файлів зі списку.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>Файли зі списку не буде зафіксовано. Незафіксовані зміни можуть бути втрачені після перезапуску.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>Файли зі списку не буде зафіксовано. Незафіксовані зміни можуть бути втрачені після вимкнення.</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>Фіксує файли зі списку перед звичайним вимкненням або перезапуском.</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>Повну фіксацію томів і ввімкнення HORM не можна застосувати одночасно.</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>Налаштування місця зберігання та ввімкнення HORM не можна застосувати одночасно.</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>Для налаштування місця зберігання та повної фіксації томів потрібні різні поточні режими UWF.</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>Виберіть том для зберігання перед застосуванням змін.</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>Налаштування місця зберігання на диску конфліктує із запитаним накладанням у RAM.</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>Застосуйте зміну фільтра й перезавантажте комп’ютер перед запитом повної фіксації томів.</translation></message>
    <message><source>Canceled</source><translation>Скасовано</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>Перевірений том для зберігання більше недоступний. Поверніться до редагування та виберіть інший том.</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>Цей том неможливо зареєструвати в UWF, оскільки його шина зберігання не підтримується.</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>Не вдалося підтвердити підтримку фізичного тому. Оновіть стан перед увімкненням захисту.</translation></message>
    <message><source>Paused</source><translation>Призупинено</translation></message>
    <message><source>Partially paused</source><translation>Частково призупинено</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>Дані постійного оверлею збережуться під час наступного запуску. Автоматичну фіксацію призупинено.</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>Призупинені елементи: %1 · Призупинені файли: %2</translation></message>
    <message><source>Read-only ROM</source><translation>ROM лише для читання</translation></message>
    <message><source>%1 changes · %2 commands</source><translation>%1 змін · %2 команд</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>Файлова система %1: виключення файлів і пофайлове закріплення не підтримуються.</translation></message>
    <message><source>Applying changes</source><translation>Застосування змін</translation></message>
    <message><source>Automatic commits paused</source><translation>Автоматичне закріплення призупинено</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>Після зміни режиму HORM автоматичне закріплення файлів призупиняється до перезавантаження.</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>Автоматичне закріплення файлів недоступне після FullCommit.</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>Автоматичне закріплення файлів недоступне, коли HORM активний.</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>Автоматичне закріплення підготовлених файлів призупинено для цього перезавантаження.</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>Автоматичне закріплення підготовлених файлів призупинено для цього вимкнення.</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>Не вдалося підтвердити постійне збереження конфігурації. Оновіть стан системи перед зміною цього параметра.</translation></message>
    <message><source>Elapsed: %1 s</source><translation>Минуло: %1 с</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>Буде закріплено лише підготовлені файли, що відповідають умовам.</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>Режим ROM лише для читання не може постійно зберігати звичайні налаштування UWF на захищеному системному томі. Вимкніть UWF і перезавантажте систему перед їх зміною.</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>Пропущені підготовлені файли не закріплюються цією операцією.</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>Підготовлені файли вже розміщено на незахищеному сховищі. Автоматичне закріплення не потрібне.</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>Зміна порогів потребує закріплення реєстру, недоступного в режимі ROM лише для читання.</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>Очікування завершення операції Windows. Це може тривати певний час.</translation></message>
    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM вимкнено. Автоматичне закріплення підготовлених файлів призупинено до перезавантаження. Ручні операції залежать від поточних можливостей UWF.</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>Цей знімок конфігурації містить параметри, для яких потрібно вимкнути UWF у поточному сеансі. Вимкніть UWF і перезавантажте комп’ютер перед його виконанням.</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>Дії поточного сеансу (HORM, FullCommit і скидання постійного накладання; експортуйте їх із розділу «Зміни, що очікують»)</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>Масове зняття захисту потребує підтвердження повного переліку томів наступного сеансу.</translation></message>
</context>
</TS>
