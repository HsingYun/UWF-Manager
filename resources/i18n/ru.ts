<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>Включить</translation></message>
    <message><source>Disable</source><translation>Отключить</translation></message>
    <message><source>enable</source><translation>включить</translation></message>
    <message><source>disable</source><translation>отключить</translation></message>
    <message><source>Enabled</source><translation>Включено</translation></message>
    <message><source>Disabled</source><translation>Отключено</translation></message>
    <message><source>OK</source><translation>ОК</translation></message>
    <message><source>Cancel</source><translation>Отмена</translation></message>
    <message><source>Close</source><translation>Закрыть</translation></message>
    <message><source>Refresh</source><translation>Обновить</translation></message>
    <message><source>Apply</source><translation>Применить</translation></message>
    <message><source>Add</source><translation>Добавить</translation></message>
    <message><source>Remove</source><translation>Убрать</translation></message>
    <message><source>Yes</source><translation>Да</translation></message>
    <message><source>No</source><translation>Нет</translation></message>
    <message><source>Excluded</source><translation>Исключено</translation></message>
    <message><source>Not excluded</source><translation>Не исключено</translation></message>
    <message><source>File…</source><translation>Файл…</translation></message>
    <message><source>Folder…</source><translation>Папка…</translation></message>
    <message><source>volume ID</source><translation>идентификатор тома</translation></message>
    <message><source>drive letter</source><translation>буква диска</translation></message>
    <message><source>Drive letter</source><translation>Буква диска</translation></message>
    <message><source>Volume ID</source><translation>Идентификатор тома</translation></message>
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>Система «%1» (%2) не распознана как поддерживаемая редакция Windows 10/11. UWF Manager работает в режиме совместимости; часть функций может быть недоступна.</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Состояние защиты тома в текущем сеансе (только чтение).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Защищать том в следующем сеансе. Запись перенаправляется в оверлей и отбрасывается при перезагрузке.</translation></message>
    <message><source>Protection:</source><translation>Защита:</translation></message>
    <message><source>Current session</source><translation>Текущий сеанс</translation></message>
    <message><source>Next session</source><translation>Следующий сеанс</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>Активный сеанс (только чтение). Изменения не вступают в силу в этом сеансе.</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>Сеанс после перезагрузки. Изменения вступят в силу после перезагрузки системы.</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Способ распознавания тома. Буква диска проще, но привязка теряется при её смене, например после добавления или удаления дисков. Идентификатор тома остаётся постоянным.</translation></message>
    <message><source>Bind by:</source><translation>Привязка по:</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Том не поддерживается UWF: </translation></message>
    <message><source>Global settings</source><translation>Общие настройки</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Состояние фильтра UWF в текущем сеансе (только чтение).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Включить UWF в следующем сеансе. Запись на защищённые тома идёт в оверлей и отбрасывается при перезагрузке.</translation></message>
    <message><source>Filter</source><translation>Фильтр</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Для изменения типа и максимального размера:
1. Отключите фильтр выше.
2. Перезагрузите систему, чтобы отключение вступило в силу.
3. Измените настройку.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Хранилище оверлея: RAM быстрее, но занимает память; Disk использует системный диск и даёт больше места. Оба очищаются при перезагрузке.</translation></message>
    <message><source>Type</source><translation>Тип</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Максимальная ёмкость оверлея. RAM ограничена объёмом памяти. Для Disk нужно минимум 1024 MB и достаточно свободного места на системном томе.</translation></message>
    <message><source>Maximum size</source><translation>Максимальный размер</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Максимальный размер · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Порог предупреждения</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>При достижении этого объёма записывается предупреждение в журнал событий. Значение должно быть ниже критического порога. 0 отключает событие.</translation></message>
    <message><source>Critical threshold</source><translation>Критический порог</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>При достижении этого объёма записывается критическое событие в журнал. Значение должно быть выше порога предупреждения. 0 отключает событие.</translation></message>
    <message><source>Used / total</source><translation>Занято / всего</translation></message>
    <message><source>Overlay</source><translation>Оверлей</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; Занято &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Предупреждение &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Критично</translation></message>
    <message><source>UWF status unavailable: </source><translation>Состояние UWF недоступно: </translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>Для изменения UWF нужны права администратора. Перезапустите программу через правую кнопку мыши → «Запуск от имени администратора».</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Неподдерживаемый тип диска (только локальные фиксированные диски).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Том превышает лимит UWF: 16 TiB на защищённый том.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: защита тома возможна, но исключения файлов и пофайловая фиксация не поддерживаются.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Ошибка: Чтение сведений о томе.</translation></message>
    <message><source>%1 free / %2</source><translation>Свободно %1 / %2</translation></message>
    <message><source>Commit</source><translation>Зафиксировать</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Зафиксировать изменения файла из оверлея на диске / Зафиксировать изменения реестра. Это действие нельзя отменить.</translation></message>
    <message><source>Commit file changes…</source><translation>Зафиксировать изменения файла…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Выбрать файл для фиксации на диске. Зафиксировать изменения файла из оверлея на диске.</translation></message>
    <message><source>Commit folder changes…</source><translation>Зафиксировать изменения папки…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Выбрать папку для фиксации на диске. Зафиксировать изменения папки из оверлея на диске. Включая все вложенные папки.</translation></message>
    <message><source>Delete and commit file…</source><translation>Удалить файл и зафиксировать удаление на диске…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Выбрать файл для фиксации удаления. Удалить файл и зафиксировать удаление на диске.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Удалить папку со всем содержимым и зафиксировать удаление на диске…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Выбрать папку для фиксации удаления. Удалить папку со всем содержимым и зафиксировать удаление на диске. Включая все вложенные папки.</translation></message>
    <message><source>Commit registry changes…</source><translation>Зафиксировать изменения реестра…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Введите путь ключа и необязательное имя значения. Если имя пустое, фиксируются все значения ключа и всех подразделов.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Зафиксировать удаление из реестра…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Введите ключ для удаления и необязательное имя значения. Пустое имя означает удаление и фиксацию всего дерева: ключа, значений и подразделов.</translation></message>
    <message><source>File exclusions</source><translation>Исключения файлов</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Файлы и папки тома вне защиты UWF. Дважды щёлкните запись для копирования пути.</translation></message>
    <message><source>Registry exclusions</source><translation>Исключения реестра</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Исключения реестра общие и зависят от глобального фильтра UWF, а не защиты тома. Список общий для всех томов и показан только на одной вкладке. Двойной щелчок копирует путь.</translation></message>
    <message><source>File staging</source><translation>Автофиксация файлов</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>файлы / папки: Автоматически фиксируются перед безопасным выключением или перезагрузкой. Включая все вложенные папки. Пока список не пуст, следующее исключение реестра сохраняет его между защищёнными сеансами: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Search staged paths…</source><translation>Поиск путей автофиксации…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Добавить файлы или папку в список автофиксации.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Выбрать файл для фиксации на диске. Автоматически фиксируются перед безопасным выключением или перезагрузкой.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Выбрать папку для фиксации на диске. Автоматически фиксируются перед безопасным выключением или перезагрузкой. Включая все вложенные папки.</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>Немедленно убрать выбранные пути из автофиксации.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Выбрать файл для фиксации на диске · Автофиксация файлов</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Выбрать папку для фиксации на диске · Автофиксация файлов · Включая все вложенные папки.</translation></message>
    <message><source>Cannot stage this path</source><translation>Невозможно добавить путь в автофиксацию</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Путь пересекается с исключением UWF. Сначала удалите исключение:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Путь пересекается с автофиксацией. Сначала уберите путь из списка:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Путь пересекается с записью автофиксации</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Исключение реестра необходимо, пока список автофиксации не пуст</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Путь не существует или имеет неожиданный тип:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Не удалось проверить путь:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Точки повторной обработки нельзя добавлять в автофиксацию:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Нет буквы диска, фиксация UWF невозможна:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Путь находится на томе %1. Добавьте его на вкладке автофиксации этого тома.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Корень тома нельзя добавлять: это привело бы к рекурсивной фиксации всего тома.</translation></message>
    <message><source>File staging could not be saved</source><translation>Ошибка: Сохранение списка автофиксации</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Список автофиксации перезагружен из-за сбоя обновления:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Автофиксация файлов: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Автоматически фиксируются перед безопасным выключением или перезагрузкой.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Автофиксация файлов: %1 · файлы: %2 · папки: %3</translation></message>
    <message><source>Required by UWF for File staging.</source><translation>Необходимо для автофиксации UWF.</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Фильтр UWF отключён. Изменения в оверлее не накапливались; фиксировать нечего.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Файловая система %1 не поддерживает пофайловую фиксацию.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Этот том не поддерживается UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Том не поддерживает фиксацию файлов. Доступна только фиксация реестра (его исключения глобальны).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Файловая система %1 не поддерживает пофайловую фиксацию. Доступна только фиксация реестра (его исключения глобальны).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Том сейчас не защищён UWF; нет изменений файлов для фиксации.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Том сейчас не защищён UWF; нет изменений файлов для фиксации. Доступна только фиксация реестра (его исключения глобальны).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Выбрать файл для фиксации на диске</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Выбрать папку для фиксации на диске</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Выбрать файл для фиксации удаления</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Выбрать папку для фиксации удаления</translation></message>
    <message><source>Commit registry changes</source><translation>Зафиксировать изменения реестра</translation></message>
    <message><source>Commit registry deletion</source><translation>Зафиксировать удаление из реестра</translation></message>
    <message><source>Key path:</source><translation>Ключ · Путь:</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>Введите или вставьте HKLM\Software\... и нажмите Enter</translation></message>
    <message><source>Name</source><translation>Имя</translation></message>
    <message><source>Data</source><translation>Данные</translation></message>
    <message><source>Key: %1</source><translation>Ключ: %1</translation></message>
    <message><source>Value: %1</source><translation>Значение: %1</translation></message>
    <message><source>(none)</source><translation>(нет)</translation></message>
    <message><source>(Default)</source><translation>(По умолчанию)</translation></message>
    <message><source>(value not set)</source><translation>(значение не задано)</translation></message>
    <message><source>(empty)</source><translation>(пусто)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Неизвестно (Значение: Файл слишком большой)</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>(весь ключ со всеми подразделами)</translation></message>
    <message><source>Overlay files - %1</source><translation>Оверлей · файлы - %1</translation></message>
    <message><source>View overlay files</source><translation>Просмотр файлов оверлея</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Просмотр файлов оверлея. &lt;b&gt;Только диагностический снимок.&lt;/b&gt; Не выбирайте по нему данные для фиксации.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Список файлов оверлея доступен только на томах NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF отключён; ни на одном томе нет оверлея для просмотра.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Том не защищён UWF и не имеет оверлея для просмотра.</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;Только диагностический снимок.&lt;/b&gt; Не выбирайте по нему данные для фиксации.</translation></message>
    <message><source>NTFS volumes only.</source><translation>Список файлов оверлея доступен только на томах NTFS.</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>Большой оверлей требует больше памяти и времени; операция может замедлиться или завершиться ошибкой.</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>Список неточен: файлы меньше кластера (обычно 4 KB) могут отсутствовать, а уже зафиксированные, исключённые или затронутые другими операциями — появляться.</translation></message>
    <message><source>Loading…</source><translation>Загрузка…</translation></message>
    <message><source>Failed: %1</source><translation>Ошибка: %1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>Оверлей · файлы: %1</translation></message>
    <message><source>No files</source><translation>Нет файлов</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>Страница %1 / %2 · файлы: %3</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Поставщик WMI аварийно завершился. Это известная нестабильность UWF_Overlay.GetOverlayFiles при большом оверлее или высокой нагрузке ввода-вывода.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Недостаточно памяти либо операция не поддерживается. Нужны NTFS и запас памяти. Уменьшите оверлей или закройте программы с высоким потреблением памяти и повторите.</translation></message>
    <message><source>Raw error: %1</source><translation>Код ошибки: %1</translation></message>
    <message><source>Export to file…</source><translation>Экспорт в файл…</translation></message>
    <message><source>Export overlay file list</source><translation>Экспорт в файл… · Оверлей · файлы</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Текстовые файлы (*.txt);;Все файлы (*)</translation></message>
    <message><source>Export failed</source><translation>Ошибка экспорта</translation></message>
    <message><source>Export finished</source><translation>Экспорт завершён</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Ошибка: Открытие файла (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Ошибка: Запись файла (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Конфигурация: Запись файла · записи: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Нельзя исключить том целиком (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Файлы подкачки, swap и гибернации нельзя исключать: от них зависит сам UWF.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Нельзя исключить каталог целиком: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Нельзя исключить каталог целиком: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Нельзя исключить каталог целиком: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Нельзя исключить критический системный файл: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Файл реестра пользователя NTUSER.DAT нельзя исключать.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Ключ реестра не может быть пустым.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Ключ реестра не может начинаться с обратной косой черты.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Путь содержит повторяющиеся обратные косые черты и недопустим.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Разделитель пути реестра — `\`, а не `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Путь содержит невидимые управляющие символы и недопустим.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>Документация UWF запрещает исключать секрет учётной записи компьютера домена: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Допустимы только конкретные подразделы следующих корневых ключей:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Search file paths…</source><translation>Поиск путей файлов…</translation></message>
    <message><source>Search registry keys…</source><translation>Поиск ключей реестра…</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Добавить файлы или папки в исключения. Они не будут защищены UWF.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Выбрать файл для добавления · Исключения файлов.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Выбрать папку для добавления · Исключения файлов. Включая все вложенные папки.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Введите путь ключа реестра для исключения.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Введите путь ключа реестра для исключения. / Сохранение между сеансами.</translation></message>
    <message><source>Registry key…</source><translation>Реестр · Ключ…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Секретный ключ домена (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Клиентская лицензия служб терминалов (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Сохранять секрет домена (пароль учётной записи компьютера) между сеансами UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Сохранять клиентские лицензии служб терминалов между сеансами UWF.</translation></message>
    <message><source>· %1 persistence %2</source><translation>· Сохранение между сеансами %1: %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Сохранение между сеансами %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Убрать выбранное</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Убрать выбранные исключения. Вступает в силу после применения.</translation></message>
    <message><source>Open containing folder</source><translation>Открыть папку</translation></message>
    <message><source>Copy file path</source><translation>Копировать путь к файлу</translation></message>
    <message><source>Copy registry path</source><translation>Копировать путь реестра</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Зафиксировать изменения папки из оверлея на диске…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Зафиксировать изменения файла из оверлея на диске…</translation></message>
    <message><source>Copied to clipboard: </source><translation>Скопировано в буфер обмена: </translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Выбрать файл для добавления · Исключения файлов. Можно выбрать несколько файлов.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Выбрать папку для добавления · Исключения файлов</translation></message>
    <message><source>Add registry exclusion</source><translation>Добавление исключения реестра</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Путь %1 не на томе %2; его нельзя добавить в исключения этого тома.</translation></message>
    <message><source>Path is not on this volume</source><translation>Путь не на этом томе</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Невозможно добавить исключение</translation></message>
    <message><source>File: %1</source><translation>Файл… %1</translation></message>
    <message><source>Folder: %1</source><translation>Папка… %1</translation></message>
    <message><source>Registry: %1</source><translation>Реестр: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Исключено по умолчанию; можно изменить.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Текущий сеанс: %1
Следующий сеанс: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Ожидающие изменения: %1</translation></message>
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>записи: %1 · К добавлению: %2 · К удалению в следующем сеансе: %3 · Ожидающие изменения: %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Диспетчер единого фильтра записи (UWF)</translation></message>
    <message><source>Main toolbar</source><translation>Главная панель инструментов</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Перечитать состояние текущего и конфигурацию следующего сеанса UWF.</translation></message>
    <message><source>Review and apply</source><translation>Проверить и применить</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Проверить все ожидающие изменения и применить одним пакетом. Большинство изменений вступит в силу после следующей перезагрузки.</translation></message>
    <message><source>Safe shutdown</source><translation>Безопасное выключение</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Система безопасно выключится через UWF. Доступно даже при заполненном оверлее UWF.</translation></message>
    <message><source>Safe restart</source><translation>Безопасная перезагрузка</translation></message>
    <message><source>Direct restart</source><translation>Перезагрузить напрямую</translation></message>
    <message><source>Direct shutdown</source><translation>Выключить напрямую</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Система безопасно перезагрузится через UWF. Доступно даже при заполненном оверлее UWF.</translation></message>
    <message><source>Log</source><translation>Журнал</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Открыть внутренний журнал текущего сеанса для диагностики.</translation></message>
    <message><source>About</source><translation>О программе</translation></message>
    <message><source>About this program.</source><translation>О программе UWF Manager.</translation></message>
    <message><source>Switch display language</source><translation>Изменить язык интерфейса</translation></message>
    <message><source>Language</source><translation>Язык</translation></message>
    <message><source>Theme</source><translation>Тема</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Переключить светлую/тёмную тему</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Графический интерфейс управления фильтром UWF, оверлеем, исключениями файлов и реестра. Большинство изменений вступит в силу после следующей перезагрузки.&lt;/p&gt;&lt;p&gt;Исходный код: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;Лицензия: &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; Полный текст лицензии находится в прилагаемом файле LICENSE.&lt;/p&gt;&lt;p&gt;Свободное ПО: разрешено распространение и изменение на условиях GPL v3. Предоставляется «как есть», без каких-либо гарантий.&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>О программе UWF Manager</translation></message>
    <message><source>System information</source><translation>Сведения о системе</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Диагностический снимок</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>Диагностика системы, безопасности, UWF, расширенного режима и последних журналов.</translation></message>
    <message><source>Read only</source><translation>Только чтение</translation></message>
    <message><source>Diagnostic report</source><translation>Диагностический отчёт</translation></message>
    <message><source>Copied</source><translation>Скопировано</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>Диагностика системы, безопасности, UWF, расширенного режима и последних журналов.</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>Отчёт может содержать имена учётных записей, идентификаторы устройств, пути и журналы. Проверьте его перед отправкой.</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;Программа зависит от единого фильтра записи Windows (UWF).&lt;/b&gt; Защиту выполняет UWF в составе Windows. UWF Manager сам не фильтрует запись, а только настраивает и управляет UWF. Компонент должен быть установлен и включён в системе.&lt;/p&gt;&lt;p&gt;При первом включении UWF для улучшения производительности изменяет следующие настройки системы:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Отключаются файлы подкачки.&lt;/li&gt;&lt;li&gt;Отключается восстановление системы.&lt;/li&gt;&lt;li&gt;Отключается SuperFetch.&lt;/li&gt;&lt;li&gt;Отключается служба индексирования файлов.&lt;/li&gt;&lt;li&gt;Отключается служба дефрагментации.&lt;/li&gt;&lt;li&gt;Отключается быстрый запуск.&lt;/li&gt;&lt;li&gt;Параметр BCD bootstatuspolicy устанавливается в ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;После включения настройки можно изменить. Например, перенести файл подкачки на незащищённый том и снова включить подкачку.&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Объект не найден; в оверлее нечего фиксировать.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Объект есть только в оверлее: создан под защитой UWF и никогда не фиксировался на диске или в постоянном реестре. Исчезнет при перезагрузке; фиксация удаления не нужна и невозможна.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Система отклонила параметр: неверный путь или аргумент.</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Ошибка операции. Объект может использоваться другой программой, например Проводником или открытым файлом. Закройте использующие его программы и повторите.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Операция не выполнена; подробности в журнале.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Операция отклонена (код %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Неизвестно.</translation></message>
    <message><source>Commit canceled</source><translation>Фиксация отменена</translation></message>
    <message><source>Commit result</source><translation>Результат фиксации</translation></message>
    <message><source>Succeeded</source><translation>Успешно</translation></message>
    <message><source>Skipped</source><translation>Пропущено</translation></message>
    <message><source>Failed</source><translation>Ошибка</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Успешно: %1; Пропущено: %2; Ошибка: %3.</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
Отменено пользователем; не обработано записей: %1.</translation></message>
    <message><source>Category</source><translation>Категория</translation></message>
    <message><source>Path</source><translation>Путь</translation></message>
    <message><source>Existed before</source><translation>Существовал до</translation></message>
    <message><source>Exists after</source><translation>Существует после</translation></message>
    <message><source>Error code</source><translation>Код ошибки</translation></message>
    <message><source>Reason</source><translation>Причина</translation></message>
    <message><source>Previous page</source><translation>Предыдущая страница</translation></message>
    <message><source>First page</source><translation>Первая страница</translation></message>
    <message><source>Last page</source><translation>Последняя страница</translation></message>
    <message><source>Next page</source><translation>Следующая страница</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>Страница %1 / %2 · записи: %3</translation></message>
    <message><source>Copy selected rows</source><translation>Копировать выбранные строки</translation></message>
    <message><source>Copy all</source><translation>Копировать всё</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Исключение реестра необходимо, пока список автофиксации не пуст. Автофиксация файлов: Очистить → Удаление исключения реестра.</translation></message>
    <message><source>0 lines</source><translation>0 строки</translation></message>
    <message><source>%1 lines</source><translation>%1 строки</translation></message>
    <message><source>Loading log entries…</source><translation>Загрузка журнала…</translation></message>
    <message><source>No log entries</source><translation>Журнал пуст</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>Страница %1 / %2 · строки: %3</translation></message>
    <message><source>Copy current page</source><translation>Копировать текущую страницу</translation></message>
    <message><source>Clear</source><translation>Очистить</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>Ожидающие изменения: %1</translation></message>
    <message><source>No pending changes</source><translation>Нет ожидающих изменений</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Исключения реестра · Общие настройки.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Том %1: Защита UWF / Исключения файлов / Автофиксация файлов.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Не поддерживается</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>Обновить · Том: %1</translation></message>
    <message><source>· Filter (global) %1</source><translation>· Фильтр (UWF): %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· Оверлей · Тип → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· Оверлей · Максимальный размер → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· Оверлей · Порог предупреждения → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· Оверлей · Критический порог → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ Для изменения типа и максимального размера:
1. Отключите фильтр выше.
2. Перезагрузите систему, чтобы отключение вступило в силу.
3. Измените настройку.</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· Том %1 · Защита UWF: %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· Том %1 · Привязка по: %2 (без аналога CLI; только в этой программе)</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ Исключения файлов  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− Исключения файлов  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ Исключения реестра  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− Исключения реестра  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>Фильтр (UWF): %1</translation></message>
    <message><source>Overlay type → %1</source><translation>Оверлей · Тип → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>Оверлей · Максимальный размер → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>Оверлей · Порог предупреждения → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>Оверлей · Критический порог → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>Том %1 · Защита UWF: %2</translation></message>
    <message><source>File exclusion %1</source><translation>Исключения файлов: %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>Исключения реестра: %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>Ожидающие изменения (%1)</translation></message>
    <message><source>Current session configuration</source><translation>Конфигурация текущего сеанса</translation></message>
    <message><source>Review and apply changes</source><translation>Проверить и применить изменения</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>Полная конфигурация в виде команд uwfmgr. Ожидающие изменения показаны первыми в отдельном разделе. &lt;span style='color:%1'&gt;Ожидающие изменения&lt;/span&gt;. &lt;span style='color:%2'&gt;Применить&lt;/span&gt;: Нажмите для записи изменений в систему. Большинство изменений вступит в силу после следующей перезагрузки.</translation></message>
    <message><source>Confirm apply</source><translation>Подтвердить применение</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>&lt;span style='color:%1'&gt;Эти изменения будут записаны в систему&lt;/span&gt;. Большинство изменений вступит в силу после следующей перезагрузки.&lt;br&gt;&lt;br&gt;Продолжить?</translation></message>
    <message><source>Applied changes</source><translation>Применённые изменения</translation></message>
    <message><source>Result</source><translation>Результат</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Фильтр: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Задание порога предупреждения: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Задание критического порога: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Задание типа оверлея / Задание максимального размера: Для изменения типа и максимального размера:
1. Отключите фильтр выше.
2. Перезагрузите систему, чтобы отключение вступило в силу.
3. Измените настройку.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Задание типа оверлея: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Задание максимального размера: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Задание максимального размера: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Том %1 · Защита UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Том %1 · Привязка по: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Том %1 · Добавление исключения файла: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Том %1 · Удаление исключения файла: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Добавление исключения реестра: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Удаление исключения реестра: %1</translation></message>
    <message><source>Time</source><translation>Время</translation></message>
    <message><source>Level</source><translation>Уровень</translation></message>
    <message><source>Tag</source><translation>Метка</translation></message>
    <message><source>Message</source><translation>Сообщение</translation></message>
    <message><source>Confirm safe shutdown?</source><translation>Безопасное выключение?</translation></message>
    <message><source>Confirm safe restart?</source><translation>Безопасная перезагрузка?</translation></message>
    <message><source>Shut down</source><translation>Выключить</translation></message>
    <message><source>Restart</source><translation>Перезагрузить</translation></message>
    <message><source>Continue shutdown</source><translation>Продолжить выключение</translation></message>
    <message><source>Continue restart</source><translation>Продолжить перезагрузку</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>Система безопасно выключится через UWF.</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>Система безопасно перезагрузится через UWF.</translation></message>
    <message><source>UWF protection</source><translation>Защита UWF</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>Доступно даже при заполненном оверлее UWF.</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Подсчёт файлов для автофиксации…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>Перед операцией автоматически фиксируются файлы: %1. Файлы вне защищённых сейчас томов пропускаются.</translation></message>
    <message><source>Committing staged files</source><translation>Автофиксация файлов</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>Операция питания продолжится после автофиксации файлов.</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>Обработано файлов: %1 из %2.</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>Автофиксация файлов не завершена</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>Проверьте подробности операции перед продолжением.</translation></message>
    <message><source>Operation details</source><translation>Подробности операции</translation></message>
    <message><source>No additional error details were provided.</source><translation>Дополнительные сведения об ошибке отсутствуют.</translation></message>
    <message><source>Some changes may not be preserved</source><translation>Часть изменений может не сохраниться</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>При продолжении система выключится без сохранения файлов, которые не удалось зафиксировать.</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>При продолжении система перезагрузится без сохранения файлов, которые не удалось зафиксировать.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Автофиксация неожиданно остановлена:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Автофиксация файлов не завершена: Неизвестная ошибка.</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>Незафиксированные изменения будут потеряны</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>Сохраните работу и зафиксируйте все нужные изменения перед продолжением.</translation></message>
    <message><source>Confirm direct restart?</source><translation>Перезагрузить напрямую?</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>Выключить напрямую?</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>Перезагрузить. Файлы списка автофиксации не будут зафиксированы перед перезагрузкой.</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>Выключить. Файлы списка автофиксации не будут зафиксированы перед выключением.</translation></message>
    <message><source>File staging will be skipped</source><translation>Автофиксация файлов будет пропущена</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>Файлы списка автофиксации не будут зафиксированы перед перезагрузкой.</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>Файлы списка автофиксации не будут зафиксированы перед выключением.</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Перезагрузить напрямую: Автофиксация файлов будет пропущена.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Выключить напрямую: Автофиксация файлов будет пропущена.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Автофиксация пропущена из-за выбора прямого действия питания.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Безопасное выключение: Ошибка</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Выключить: Ошибка (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Безопасная перезагрузка: Ошибка</translation></message>
    <message><source>Direct restart failed</source><translation>Перезагрузить напрямую: Ошибка</translation></message>
    <message><source>Direct shutdown failed</source><translation>Выключить напрямую: Ошибка</translation></message>
    <message><source>Restart failed: %1</source><translation>Перезагрузить: Ошибка (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Не удалось прочитать список автофиксации:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Не удалось прочитать список автофиксации: Неизвестная ошибка.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>файлы: %1 · Успешно: %2 · Пропущено (файлы): %3 · Пропущено (записи): %4 · Ошибка: %5</translation></message>
    <message><source>File staging preparation</source><translation>Автофиксация файлов: Подробности операции</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Точки повторной обработки не поддерживаются.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Можно добавлять только абсолютные пути на локальных томах.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Можно добавлять только абсолютные пути на локальных томах.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Не удалось проверить путь автофиксации.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Путь добавленного файла теперь указывает на объект другого типа.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Путь добавленной папки теперь указывает на объект другого типа.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Не удалось полностью перечислить содержимое папки.</translation></message>
    <message><source>The path has no drive letter.</source><translation>У пути нет буквы диска.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>Нельзя рекурсивно обрабатывать корень тома.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Поставщик UWF отклонил фиксацию.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Ошибка фиксации: Неизвестная ошибка.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Дополнительных ошибок в журнале: %1.</translation></message>
    <message><source>Commit failed</source><translation>Ошибка фиксации</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>У пути нет буквы диска. Определение целевого тома: Ошибка.</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>Ошибка: Чтение сведений о томе (%1)</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>Нет записи текущего сеанса для тома %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Путь находится в исключениях файлов.
Исключение: %1</translation></message>
    <message><source>No files were found under %1.</source><translation>В %1 файлы не найдены.</translation></message>
    <message><source>Commit to disk</source><translation>Зафиксировать на диске</translation></message>
    <message><source>Committing…</source><translation>Фиксация…</translation></message>
    <message><source>This action cannot be undone.</source><translation>Это действие нельзя отменить.</translation></message>
    <message><source>Continue</source><translation>Продолжить</translation></message>
    <message><source>Delete and commit</source><translation>Удалить и зафиксировать</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>Зафиксировать изменения файла из оверлея на диске</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>Зафиксировать изменения папки из оверлея на диске</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>файлы: %1. Включая все вложенные папки.</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>Удалить файл и зафиксировать удаление на диске</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>Удалить папку со всем содержимым и зафиксировать удаление на диске</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>файлы: %1; папки: %2. Удалить и зафиксировать.</translation></message>
    <message><source>Commit this registry value to disk</source><translation>Зафиксировать значение реестра на диске</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>Зафиксировать ключ реестра со всем поддеревом на диске</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>Значение: %1. (весь ключ со всеми подразделами)</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>Удалить значение реестра и зафиксировать удаление на диске</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>Удалить ключ реестра со всем поддеревом и зафиксировать удаление</translation></message>
    <message><source>Scanning registry keys…</source><translation>Перечисление ключей реестра…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>Перечисление ключей реестра…
Ключ: %1</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>Реестр · Ключ · Удалить и зафиксировать:</translation></message>
    <message><source>Copy current entry</source><translation>Копировать текущую запись</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>Будут рекурсивно удалены ключи (%1) и все их значения.</translation></message>
    <message><source>Commit file deletion failed</source><translation>Ошибка: Фиксация удаления файла</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>Путь не существует; удалять нечего.</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Ключ находится в исключениях реестра.
Исключение: %1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>Ошибка: Чтение фильтра реестра (%1)</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>Нет записи фильтра реестра для текущего сеанса.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>Значение реестра не существует; фиксировать нечего.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>Ключ реестра не существует; фиксировать нечего.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>Значение реестра не существует; удалять нечего.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>Ключ реестра не существует; удалять нечего.</translation></message>
    <message><source>Export commands…</source><translation>Экспортировать команды…</translation></message>
    <message><source>Export commands to file</source><translation>Экспортировать команды в файл</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Экспорт в файл… · Команда: %1
%2</translation></message>
    <message><source>Import</source><translation>Импорт</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Введите или вставьте команды, по одной в строке. Загрузите текст, журнал или скрипт (.txt, .bat, .ps1); строки с этой командой будут добавлены. Импорт ставит изменения в очередь. Запись в систему выполняется через «Проверить и применить» на панели инструментов.</translation></message>
    <message><source>Import uwfmgr commands</source><translation>uwfmgr · Импорт</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;uwfmgr&lt;/b&gt;: Введите или вставьте команды, по одной в строке. &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Загрузить из файла…&lt;/b&gt;: Загрузите текст, журнал или скрипт (.txt, .bat, .ps1); строки с этой командой будут добавлены. &lt;code&gt;uwfmgr&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Импорт&lt;/b&gt;: &lt;b&gt;Импорт ставит изменения в очередь. Запись в систему выполняется через «Проверить и применить» на панели инструментов.&lt;/b&gt; &lt;b&gt;Проверить и применить&lt;/b&gt;.&lt;/p&gt;</translation></message>
    <message><source>Load from file…</source><translation>Загрузить из файла…</translation></message>
    <message><source>Load recommended configuration</source><translation>Загрузить рекомендуемую конфигурацию</translation></message>
    <message><source>Choose recommended configuration</source><translation>Выбрать рекомендуемую конфигурацию</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Выберите группы рекомендуемой конфигурации. Перед импортом можно проверить и удалить отдельные команды.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Из официальной документации Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Исключения UWF, рекомендуемые Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Проверьте перед импортом: папки должны существовать, чтобы UWF принял исключения файлов.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Источник: официальная документация Microsoft UWF об исключениях фильтра и поддержке антивирусов.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Программа улучшения качества ПО (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Сохранять между сеансами: Согласие на участие в политике</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Сохранять между сеансами: Локальное согласие на участие</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Сохранять между сеансами: Флаг запрета отправки</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Фоновая интеллектуальная служба передачи (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Сохранять между сеансами: Файлы очереди загрузки</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Сохранять между сеансами: Индекс состояния передачи</translation></message>
    <message><source>Network profiles and policies</source><translation>Сетевые профили и политики</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Групповая политика беспроводной сети</translation></message>
    <message><source>Wired network GPO policy</source><translation>Групповая политика проводной сети</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Групповая политика беспроводной сети · файлы</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Групповая политика проводной сети · файлы</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Профили беспроводных интерфейсов</translation></message>
    <message><source>Wired network interface profiles</source><translation>Профили проводных интерфейсов</translation></message>
    <message><source>Wireless service configuration</source><translation>Настройки беспроводной службы</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Настройки мобильного широкополосного доступа</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Настройки службы автонастройки проводной сети</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>XML-профили сети зависят от устройства. При необходимости вручную добавьте точные пути Interfaces\{GUID}\{GUID}.xml.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Летнее время (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Сохранять между сеансами: Определения часовых поясов</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Сохранять между сеансами: Выбранный часовой пояс</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Сохранять между сеансами: Файлы продукта и обновления</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Сохранять между сеансами: Сигнатуры и состояние в ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Сохранять между сеансами: Windows Update · Журнал</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Сохранять между сеансами: MpCmdRun · Журнал</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Сохранять между сеансами: Состояние продукта в реестре</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Сохранять между сеансами: WdBoot · Состояние службы</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Сохранять между сеансами: WdFilter · Состояние службы</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Сохранять между сеансами: WdNisSvc · Состояние службы</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Сохранять между сеансами: WdNisDrv · Состояние службы</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Сохранять между сеансами: WinDefend · Состояние службы</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Сохранять между сеансами: Файлы клиентской программы</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Сохранять между сеансами: Windows Update · Журнал</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Сохранять между сеансами: MpCmdRun · Журнал</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Сохранять между сеансами: Сигнатуры и состояние антивируса</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Сохранять между сеансами: Состояние антивируса в реестре</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Выбрать файлы с командами uwfmgr</translation></message>
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>Все файлы (*);;Текстовые файлы (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
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
    <message><source>Status</source><translation>Состояние</translation></message>
    <message><source>Command</source><translation>Команда</translation></message>
    <message><source>Detail</source><translation>Подробности</translation></message>
    <message><source>Import failed</source><translation>Ошибка импорта</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Внутренняя ошибка: обработчик применения не зарегистрирован.</translation></message>
    <message><source>Nothing to import</source><translation>Нет команд для импорта</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Во вводе не найдены команды uwfmgr.</translation></message>
    <message><source>Applied: %1</source><translation>Применено: %1</translation></message>
    <message><source>Duplicates: %1</source><translation>Дубликат: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Не поддерживается: %1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>Итого после пакетов (%1):</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Пакет %1 ──</translation></message>
    <message><source>Applied</source><translation>Применено</translation></message>
    <message><source>Duplicate</source><translation>Дубликат</translation></message>
    <message><source>Unsupported</source><translation>Не поддерживается</translation></message>
    <message><source>File too large</source><translation>Файл слишком большой</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Файл %1 больше 5 MB и не разобран. Сначала вручную отфильтруйте содержимое.</translation></message>
    <message><source>Cannot read file</source><translation>Не удалось прочитать файл</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Ошибка: Открытие файла (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Отбросить ожидающие изменения?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Неприменённых изменений: %1.

Отбросить и продолжить?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Неполная команда uwfmgr</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Отсутствует аргумент: Размер (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>Размер должен быть целым неотрицательным числом в MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Отсутствует аргумент: Оверлей · Тип (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Недопустимое значение: Оверлей · Тип: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Отсутствует аргумент: Том (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Том должен быть буквой диска, например C:</translation></message>
    <message><source>Missing path argument</source><translation>Отсутствует аргумент: Путь</translation></message>
    <message><source>Missing registry key argument</source><translation>Отсутствует аргумент: Реестр · Ключ</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Незакрытая двойная кавычка в команде</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Неожиданный лишний аргумент: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Неподдерживаемая команда uwfmgr: нет соответствующего действия интерфейса</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>Ожидающие изменения: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Нужное состояние уже установлено; действие не требуется</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Путь не на этом томе либо том не поддерживает исключения файлов (например, exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Отклонено списком запретов UWF (системные файлы, Windows, подкачка и т. п.)</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>Такая команда уже встречалась в этом пакете</translation></message>
    <message><source>Pending filter %1</source><translation>Ожидающие изменения: Фильтр %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>Фильтр: Нужное состояние уже установлено; действие не требуется</translation></message>
    <message><source>Pending overlay type → %1</source><translation>Ожидающие изменения: Оверлей · Тип → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>Оверлей · Тип: %1 (Нужное состояние уже установлено; действие не требуется)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Недопустимое значение: Размер: %1</translation></message>
    <message><source>maximum size</source><translation>Максимальный размер</translation></message>
    <message><source>warning threshold</source><translation>Порог предупреждения</translation></message>
    <message><source>critical threshold</source><translation>Критический порог</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>Ожидающие изменения: Оверлей %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>Оверлей %1: %2 MB (Нужное состояние уже установлено; действие не требуется)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Неизвестный том %1: нет подходящего для UWF диска с этой буквой</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>Ожидающие изменения: Том %1 · Защита UWF: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Том %1: Нужное состояние уже установлено; действие не требуется</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: У пути нет буквы диска. Определение целевого тома: Ошибка.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Том %1: UWF Не поддерживается</translation></message>
    <message><source>file exclusion</source><translation>Исключения файлов</translation></message>
    <message><source>registry exclusion</source><translation>Исключения реестра</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Вкладка реестра отсутствует; исключения реестра недоступны</translation></message>
    <message><source>Unsupported command</source><translation>Команда: Не поддерживается</translation></message>
    <message><source>Exit</source><translation>Выход</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Показать или скрыть.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Включено</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Отключено</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Неизвестно</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Занято %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Расширенный режим</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Согласовать автофиксацию с выключением и перезагрузкой Windows.</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>Устанавливает автоматически запускаемую службу LocalSystem. Она запускает UWF Manager после старта Windows и пытается фиксировать выбранные файлы перед выключением или перезагрузкой для улучшения совместимости приложений с UWF.</translation></message>
    <message><source>System changes</source><translation>Изменения системы</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>Создаёт автоматическую службу UWFManagerEnhanced с текущим путём программы</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>Конфигурация: Реестр → Зафиксировать (HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>Автозапуск UWF Manager со следующего старта Windows; перенос программы может нарушить автозапуск</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>Фиксирует подходящие файлы при обычном выключении и перезагрузке Windows</translation></message>
    <message><source>Operational and security considerations</source><translation>Эксплуатация и безопасность</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>Регистрирует службу в системном реестре; приложение перестаёт быть полностью переносимым</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>Большое число файлов может сильно замедлить выключение и перезагрузку; выделите достаточно времени на сохранение</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>Расширенный режим запускает UWF Manager от SYSTEM. Если обычный пользователь изменит программу или заменит её вредоносной, он может получить права SYSTEM. Включайте режим только из защищённого правами администратора расположения (рекомендуется: %ProgramFiles%\UWF Manager\Service\UWF.exe).</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>Ошибка: Чтение состояния расширенного режима</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Ошибка: Чтение состояния расширенного режима (Неизвестная ошибка).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Ошибка: Чтение состояния службы:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Ошибка: Чтение состояния службы (Неизвестная ошибка).</translation></message>
    <message><source>Status: Disabled</source><translation>Состояние: Отключено</translation></message>
    <message><source>Status: Enabled</source><translation>Состояние: Включено</translation></message>
    <message><source>Status: Service stopped</source><translation>Состояние: служба остановлена</translation></message>
    <message><source>Status: Repair required</source><translation>Состояние: требуется восстановление</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Служба не настроена как отдельный процесс.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Служба не настроена на автоматический запуск.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Служба не настроена для работы от LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>Служба не запущена.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Путь или аргументы запуска службы не соответствуют текущей программе.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Неверно настроено время ожидания предзавершения службы.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Служба не объявляет права для запуска агента интерфейса.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Служба не принимает уведомления Windows о предзавершении.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Включить расширенный режим</translation></message>
    <message><source>Disable enhanced mode</source><translation>Отключить расширенный режим</translation></message>
    <message><source>Start service</source><translation>Запустить службу</translation></message>
    <message><source>Repair enhanced mode</source><translation>Восстановить расширенный режим</translation></message>
    <message><source>Delete service</source><translation>Удалить службу</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Остановить и удалить службу расширенного режима со всеми оставшимися данными реестра.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Службы нет, но её данные реестра остались.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Ожидание аутентификации агента интерфейса.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Служба запущена, но аутентифицированный агент интерфейса не подключён.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Вспомогательная служба расширенного режима UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Расширенный режим · Конфигурация: Применено.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Конфигурация: Применено. Отчёт сохранения UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Расширенный режим · Конфигурация: Ошибка:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Расширенный режим · Конфигурация: Ошибка (Неизвестная ошибка).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Служба расширенного режима и оставшиеся настройки удалены.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Удаление службы запрошено, но очистка остатков сообщила:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Расширенный режим: Удалить службу: Ошибка:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Расширенный режим: Удалить службу: Ошибка (Неизвестная ошибка).</translation></message>
    <message><source>File staging completed</source><translation>Автофиксация файлов: Успешно</translation></message>
    <message><source>File staging failed</source><translation>Автофиксация файлов: Ошибка</translation></message>
    <message><source>All staged targets were processed.</source><translation>Все цели автофиксации обработаны.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Все объекты обработаны; одна или несколько операций завершились ошибкой.</translation></message>
    <message><source>Show main window</source><translation>Показать главное окно</translation></message>
    <message><source>Restore default position</source><translation>Восстановить положение</translation></message>
    <message><source>Hide overlay hub</source><translation>Скрыть HUB оверлея</translation></message>
    <message><source>Exit application</source><translation>Выйти из приложения</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Ошибка: Подключение к системе</translation></message>
    <message><source>Registry access failed</source><translation>Ошибка: Доступ к реестру</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Ошибка: Чтение значений реестра (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Перечисление файлов оверлея: Неизвестная ошибка.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Ошибка: Действие с фильтром: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Ошибка: Чтение состояния фильтра</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Ошибка: Задание порога предупреждения</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Ошибка: Задание критического порога</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Ошибка: Чтение состояния оверлея</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Ошибка: Задание типа оверлея</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Ошибка: Задание максимального размера</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Ошибка: Чтение конфигурации оверлея</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Ошибка: Регистрация тома %1 в UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Ошибка: Защита тома %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Ошибка: Задание привязки тома %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Том %1 · Ошибка: Добавление исключения файла %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Исключение файла на томе %1 уже отсутствует: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Том %1 · Ошибка: Удаление исключения файла %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Ошибка: Чтение конфигурации тома</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Ошибка: Добавление исключения реестра %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Ошибка: Удаление исключения реестра %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Ошибка: Изменение параметров сохранения реестра</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Ошибка: Чтение фильтра реестра</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Не поддерживается</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Чтение состояния объекта: Неизвестная ошибка.</translation></message>
    <message><source>The target state could not be read before the operation: %1</source><translation>Не удалось прочитать состояние объекта до операции: %1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Путь не существует; удалять нечего.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Неизвестная ошибка.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Результат не подтверждён: повторное чтение состояния объекта не удалось: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Поставщик принял удаление, но повторная проверка по достоверному источнику показала, что объект ещё существует.</translation></message>
    <message><source>Unknown</source><translation>Неизвестно</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Ошибка: Загрузка журнала (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Ошибка: Загрузка журнала (Неизвестная ошибка).</translation></message>
    <message><source>: unknown error</source><translation>: Неизвестная ошибка</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>Ошибка: Чтение состояния объекта (%1)</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>Ошибка: Чтение состояния объекта (Неизвестная ошибка).</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>Ошибка: Определение целевого тома (%1)</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>Ошибка: Перечисление значений реестра (%1)</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>Ошибка: Перечисление ключей реестра (%1)</translation></message>
    <message><source>Delete and commit failed</source><translation>Удалить и зафиксировать: Ошибка</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Ошибка: Определение целевого тома (%1): %2</translation></message>
    <message><source>%1/%2 slots</source><translation>Слоты: %1/%2</translation></message>
</context>
</TS>
