<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="ru" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Предпросмотр</translation></message>
    <message><source>enable</source><translation>включить</translation></message>
    <message><source>disable</source><translation>отключить</translation></message>
    <message><source>Enabled</source><translation>Включено</translation></message>
    <message><source>Disabled</source><translation>Отключено</translation></message>
    <message><source>OK</source><translation>ОК</translation></message>
    <message><source>Cancel</source><translation>Отмена</translation></message>
    <message><source>Close</source><translation>Закрыть</translation></message>
    <message><source>Refresh</source><translation>Обновить</translation></message>
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
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Состояние защиты тома в текущем сеансе (только чтение).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Защищать том в следующем сеансе. Запись перенаправляется в оверлей и отбрасывается при перезагрузке.</translation></message>
    <message><source>Current session</source><translation>Текущий сеанс</translation></message>
    <message><source>Next session</source><translation>Следующий сеанс</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Способ распознавания тома. Буква диска проще, но привязка теряется при её смене, например после добавления или удаления дисков. Идентификатор тома остаётся постоянным.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Том не поддерживается UWF: </translation></message>
    <message><source>Global settings</source><translation>Общие настройки</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Состояние фильтра UWF в текущем сеансе (только чтение).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Включить UWF в следующем сеансе. Запись на защищённые тома идёт в оверлей и отбрасывается при перезагрузке.</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Записывает предупреждение в журнал событий, когда объём занятого места в оверлее достигает этого значения. Если оба события включены, это значение должно быть ниже критического порога. Значение 0 отключает только событие предупреждения.</translation></message>
    <message><source>Critical threshold</source><translation>Критический порог</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Записывает критическое уведомление в журнал событий, когда объём занятого места в оверлее достигает этого значения. Если оба события включены, это значение должно быть выше порога предупреждения. Значение 0 отключает только критическое событие.</translation></message>
    <message><source>Overlay</source><translation>Оверлей</translation></message>
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
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Добавить файлы или папку в список автофиксации.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Выбрать файл для фиксации на диске. Автоматически фиксируются перед безопасным выключением или перезагрузкой.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Выбрать папку для фиксации на диске. Автоматически фиксируются перед безопасным выключением или перезагрузкой. Включая все вложенные папки.</translation></message>
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
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Автофиксация файлов использует это исключение реестра, чтобы сохранять свой список после перезагрузок.</translation></message>
    <message><source>Built-in</source><translation>Встроенный</translation></message>
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
    <message><source>Name</source><translation>Имя</translation></message>
    <message><source>Data</source><translation>Данные</translation></message>
    <message><source>(Default)</source><translation>(По умолчанию)</translation></message>
    <message><source>(value not set)</source><translation>(значение не задано)</translation></message>
    <message><source>(empty)</source><translation>(пусто)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Неизвестно (Значение: Файл слишком большой)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Просмотр файлов оверлея. &lt;b&gt;Только диагностический снимок.&lt;/b&gt; Не выбирайте по нему данные для фиксации.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Список файлов оверлея доступен только на томах NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF отключён; ни на одном томе нет оверлея для просмотра.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Том не защищён UWF и не имеет оверлея для просмотра.</translation></message>
    <message><source>Failed: %1</source><translation>Ошибка: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Поставщик WMI аварийно завершился. Это известная нестабильность UWF_Overlay.GetOverlayFiles при большом оверлее или высокой нагрузке ввода-вывода.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Недостаточно памяти либо операция не поддерживается. Нужны NTFS и запас памяти. Уменьшите оверлей или закройте программы с высоким потреблением памяти и повторите.</translation></message>
    <message><source>Raw error: %1</source><translation>Код ошибки: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Экспорт в файл… · Оверлей · файлы</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Текстовые файлы (*.txt);;Все файлы (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Командные файлы (*.cmd);;Все файлы (*)</translation></message>
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
    <message><source>✓ %1 persistence: %2</source><translation>✓ Сохранение между сеансами %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Убрать выбранное</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Убрать выбранные исключения. Вступает в силу после применения.</translation></message>
    <message><source>Open containing folder</source><translation>Открыть папку</translation></message>
    <message><source>Copy file path</source><translation>Копировать путь к файлу</translation></message>
    <message><source>Copy registry path</source><translation>Копировать путь реестра</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Зафиксировать изменения папки из оверлея на диске…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Зафиксировать изменения файла из оверлея на диске…</translation></message>
    <message><source>Copied to clipboard</source><translation>Скопировано в буфер обмена</translation></message>
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
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Диспетчер единого фильтра записи (UWF)</translation></message>
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
    <message><source>Switch display language</source><translation>Изменить язык интерфейса</translation></message>
    <message><source>Language</source><translation>Язык</translation></message>
    <message><source>Theme</source><translation>Тема</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Переключить светлую/тёмную тему</translation></message>
    <message><source>System information</source><translation>Сведения о системе</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Диагностический снимок</translation></message>
    <message><source>Read only</source><translation>Только чтение</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Объект не найден; в оверлее нечего фиксировать.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Объект есть только в оверлее: создан под защитой UWF и никогда не фиксировался на диске или в постоянном реестре. Исчезнет при перезагрузке; фиксация удаления не нужна и невозможна.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Система отклонила параметр: неверный путь или аргумент.</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Ошибка операции. Объект может использоваться другой программой, например Проводником или открытым файлом. Закройте использующие его программы и повторите.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Операция не выполнена; подробности в журнале.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Операция отклонена (код %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Неизвестно.</translation></message>
    <message><source>Commit canceled</source><translation>Фиксация отменена</translation></message>

    <message><source>Succeeded</source><translation>Успешно</translation></message>
    <message><source>Skipped</source><translation>Пропущено</translation></message>
    <message><source>Failed</source><translation>Ошибка</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Успешно: %1; Пропущено: %2; Ошибка: %3.</translation></message>

    <message><source>Existed before</source><translation>Существовал до</translation></message>
    <message><source>Exists after</source><translation>Существует после</translation></message>
    <message><source>Error code</source><translation>Код ошибки</translation></message>
    <message><source>Reason</source><translation>Причина</translation></message>
    <message><source>Previous page</source><translation>Предыдущая страница</translation></message>
    <message><source>First page</source><translation>Первая страница</translation></message>
    <message><source>Last page</source><translation>Последняя страница</translation></message>
    <message><source>Next page</source><translation>Следующая страница</translation></message>
    <message><source>Copy selected rows</source><translation>Копировать выбранные строки</translation></message>
    <message><source>Copy all</source><translation>Копировать всё</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Исключение реестра необходимо, пока список автофиксации не пуст. Автофиксация файлов: Очистить → Удаление исключения реестра.</translation></message>
    <message><source>Copy current page</source><translation>Копировать текущую страницу</translation></message>
    <message><source>No pending changes</source><translation>Нет ожидающих изменений</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Исключения реестра · Общие настройки.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Том %1: Защита UWF / Исключения файлов / Автофиксация файлов.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Не поддерживается</translation></message>
    <message><source>Review and apply changes</source><translation>Проверить и применить изменения</translation></message>
    <message><source>Confirm apply</source><translation>Подтвердить применение</translation></message>
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
    <message><source>Calculating files for automatic commit…</source><translation>Подсчёт файлов для автофиксации…</translation></message>
    <message><source>Committing staged files</source><translation>Автофиксация файлов</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>Обработано файлов: %1 из %2.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Дополнительные сведения об ошибке отсутствуют.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Автофиксация неожиданно остановлена:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Автофиксация файлов не завершена: Неизвестная ошибка.</translation></message>
    <message><source>File staging will be skipped</source><translation>Автофиксация файлов будет пропущена</translation></message>
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

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>У пути нет буквы диска. Определение целевого тома: Ошибка.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Нет записи текущего сеанса для тома %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Путь находится в исключениях файлов.
Исключение: %1</translation></message>

    <message><source>Continue</source><translation>Продолжить</translation></message>
    <message><source>Delete and commit</source><translation>Удалить и зафиксировать</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Ключ находится в исключениях реестра.
Исключение: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Нет записи фильтра реестра для текущего сеанса.</translation></message>

    <message><source>Export commands to file</source><translation>Экспортировать команды в файл</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Экспорт в файл… · Команда: %1
%2</translation></message>
    <message><source>Import</source><translation>Импорт</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Введите или вставьте команды, по одной в строке. Загрузите текст, журнал или скрипт (.txt, .bat, .ps1); строки с этой командой будут добавлены. Импорт ставит изменения в очередь. Запись в систему выполняется через «Проверить и применить» на панели инструментов.</translation></message>
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
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Текстовые файлы (*.txt *.cmd);;Все файлы (*)</translation></message>
    <message><source>Status</source><translation>Состояние</translation></message>
    <message><source>Import failed</source><translation>Ошибка импорта</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Внутренняя ошибка: обработчик применения не зарегистрирован.</translation></message>
    <message><source>Nothing to import</source><translation>Нет команд для импорта</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Во вводе не найдены команды uwfmgr.</translation></message>
    <message><source>Duplicates: %1</source><translation>Дубликат: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Не поддерживается: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Пакет %1 ──</translation></message>
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
    <message><source>Already in the target state — no-op</source><translation>Нужное состояние уже установлено; действие не требуется</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Путь не на этом томе либо том не поддерживает исключения файлов (например, exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Отклонено списком запретов UWF (системные файлы, Windows, подкачка и т. п.)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Недопустимое значение: Размер: %1</translation></message>
    <message><source>maximum size</source><translation>Максимальный размер</translation></message>
    <message><source>warning threshold</source><translation>Порог предупреждения</translation></message>
    <message><source>critical threshold</source><translation>Критический порог</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>Глобальная конфигурация недоступна; команда не добавлена в ожидающие изменения.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Неизвестный том %1: нет подходящего для UWF диска с этой буквой</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Том %1: Нужное состояние уже установлено; действие не требуется</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: У пути нет буквы диска. Определение целевого тома: Ошибка.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Том %1: UWF Не поддерживается</translation></message>
    <message><source>file exclusion</source><translation>Исключения файлов</translation></message>
    <message><source>registry exclusion</source><translation>Исключения реестра</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Вкладка реестра отсутствует; исключения реестра недоступны</translation></message>
    <message><source>Unsupported command</source><translation>Команда: Не поддерживается</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Показать или скрыть.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Включено</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Отключено</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Неизвестно</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Занято %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Расширенный режим</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Согласовать автофиксацию с выключением и перезагрузкой Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Ошибка: Чтение состояния расширенного режима (Неизвестная ошибка).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Ошибка: Чтение состояния службы:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Ошибка: Чтение состояния службы (Неизвестная ошибка).</translation></message>
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
    <message><source>Show overlay hub</source><translation>Показать HUB оверлея</translation></message>
    <message><source>Hide overlay hub</source><translation>Скрыть HUB оверлея</translation></message>
    <message><source>Exit application</source><translation>Выйти из приложения</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Ошибка: Подключение к системе</translation></message>
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
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Путь не существует; удалять нечего.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Неизвестная ошибка.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Результат не подтверждён: повторное чтение состояния объекта не удалось: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Поставщик принял удаление, но повторная проверка по достоверному источнику показала, что объект ещё существует.</translation></message>
    <message><source>Unknown</source><translation>Неизвестно</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Ошибка: Загрузка журнала (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Ошибка: Загрузка журнала (Неизвестная ошибка).</translation></message>
    <message><source>: unknown error</source><translation>: Неизвестная ошибка</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Ошибка: Определение целевого тома (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Ошибка: Перечисление ключей реестра (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Ошибка: Определение целевого тома (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Операция пропущена: необходимая предшествующая операция завершилась ошибкой или её результат не удалось подтвердить.</translation></message>
    <message><source>Copy</source><translation>Копировать</translation></message>
    <message><source>Select All</source><translation>Выделить всё</translation></message>
    <message><source>Display language</source><translation>Язык интерфейса</translation></message>
    <message><source>Folder</source><translation>Папка</translation></message>
    <message><source>File</source><translation>Файл</translation></message>
    <message><source>Export commands</source><translation>Экспортировать команды</translation></message>
    <message><source>Direct restart…</source><translation>Перезагрузить напрямую…</translation></message>
    <message><source>Clear…</source><translation>Очистить…</translation></message>
    <message><source>Committing</source><translation>Фиксация</translation></message>
    <message><source> · %1 selected</source><translation> · Выбрано: %1</translation></message>
    <message><source>%1 changes</source><translation>Изменений: %1</translation></message>
    <message><source>%1 commands</source><translation>Команд: %1</translation></message>
    <message><source>%1 items</source><translation>Элементов: %1</translation></message>
    <message><source>%1 items · %2</source><translation>Элементов: %1 · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>Ожидающих изменений: %1</translation></message>
    <message><source>%1 records</source><translation>Записей: %1</translation></message>
    <message><source>%1 records · %2 total</source><translation>Записей: %1 · всего %2</translation></message>


    <message><source>%1 selected</source><translation>Выбрано: %1</translation></message>
    <message><source>/ %1 files</source><translation>/ файлов: %1</translation></message>
    <message><source>32-bit</source><translation>32-разрядная</translation></message>
    <message><source>64-bit</source><translation>64-разрядная</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Распространяется на условиях &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Полные условия приведены в файле LICENSE, включённом в приложение.&lt;/p&gt;&lt;p&gt;Вы можете распространять и изменять программу на условиях GPL v3. Программа предоставляется как есть, без каких-либо гарантий.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows отключает файлы подкачки, восстановление системы, SuperFetch, индексирование файлов, дефрагментацию диска и быстрый запуск. Также в BCD параметру bootstatuspolicy присваивается значение ignoreallfailures.&lt;/p&gt;&lt;p&gt;Эти настройки можно изменить позже. Например, перенесите файл подкачки на незащищённый том перед повторным включением подкачки.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Диагностический снимок системы, привилегий и UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>В этом расположении уже есть файл:
%1

При сохранении его содержимое будет заменено.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Графический диспетчер Windows Unified Write Filter.
Управление защитой томов, оверлеем, исключениями и файлами для сохранения.</translation></message>
    <message><source>About this snapshot</source><translation>Об этом снимке</translation></message>
    <message><source>Account</source><translation>Учётная запись</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Добавьте конфигурацию UWF из сценария или текста.</translation></message>
    <message><source>Add exclusion</source><translation>Добавить исключение</translation></message>
    <message><source>Add files or folders</source><translation>Добавить файлы или папки</translation></message>
    <message><source>Add selected items</source><translation>Добавить выбранные элементы</translation></message>
    <message><source>Added</source><translation>Добавлено</translation></message>
    <message><source>Administrator</source><translation>Администратор</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Все изменения сохранены на диске и останутся без перезагрузки.</translation></message>
    <message><source>All files (*)</source><translation>Все файлы (*)</translation></message>
    <message><source>All levels</source><translation>Все уровни</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Пустой диагностический снимок не означает, что в оверлее нет изменений.</translation></message>
    <message><source>Application path</source><translation>Путь приложения</translation></message>
    <message><source>Application result</source><translation>Результат применения</translation></message>
    <message><source>Apply changes</source><translation>Применить изменения</translation></message>
    <message><source>Architecture</source><translation>Архитектура</translation></message>
    <message><source>Automatic</source><translation>Автоматически</translation></message>
    <message><source>Automatic file staging</source><translation>Автосохранение подготовленных файлов</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Автосохранение не завершено</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Автоматически сохранять подходящие подготовленные файлы при обычном выключении и перезагрузке.</translation></message>
    <message><source>Available</source><translation>Доступно</translation></message>
    <message><source>Back</source><translation>Назад</translation></message>
    <message><source>Background service</source><translation>Фоновая служба</translation></message>
    <message><source>Binding</source><translation>Привязка</translation></message>
    <message><source>By drive letter</source><translation>По букве диска</translation></message>
    <message><source>By volume ID</source><translation>По идентификатору тома</translation></message>
    <message><source>Cancel reading</source><translation>Отменить чтение</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Уже зафиксированные изменения сохранены.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Выберите раздел реестра для исключения из защиты UWF, включая его подразделы и значения.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Выберите расположение для сохранения файла.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Выберите расположение в разрешённом диске или папке.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Выберите другую папку или проверьте права доступа и повторите попытку.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Выберите другой ключ или проверьте права доступа и повторите попытку.</translation></message>
    <message><source>Clear filters</source><translation>Сбросить фильтры</translation></message>
    <message><source>Clear logs</source><translation>Очистить журнал</translation></message>
    <message><source>Clear logs?</source><translation>Очистить журнал?</translation></message>
    <message><source>Collapse</source><translation>Свернуть</translation></message>
    <message><source>Commit changes</source><translation>Зафиксировать изменения</translation></message>
    <message><source>Commit changes…</source><translation>Зафиксировать изменения…</translation></message>
    <message><source>Commit complete</source><translation>Фиксация завершена</translation></message>
    <message><source>Commit file changes?</source><translation>Зафиксировать изменения файла?</translation></message>
    <message><source>Commit folder changes?</source><translation>Зафиксировать изменения папки?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Зафиксировать изменения ключа реестра?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Зафиксировать изменения значения реестра?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Фиксирует подготовленные файлы перед обычным выключением и перезагрузкой</translation></message>
    <message><source>Compatibility mode</source><translation>Режим совместимости</translation></message>
    <message><source>Configuration applied</source><translation>Конфигурация применена</translation></message>
    <message><source>Configuration error</source><translation>Ошибка конфигурации</translation></message>
    <message><source>Configured</source><translation>Настроено</translation></message>
    <message><source>Confirm before continuing</source><translation>Подтвердите перед продолжением</translation></message>
    <message><source>Connected and authenticated</source><translation>Подключено и проверено</translation></message>
    <message><source>Connecting…</source><translation>Подключение…</translation></message>
    <message><source>Contact author</source><translation>Связаться с автором</translation></message>
    <message><source>Continue editing</source><translation>Продолжить редактирование</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Продолжение перезагрузки приведёт к потере изменений в файлах, которые не удалось сохранить.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Продолжение выключения приведёт к потере изменений в файлах, которые не удалось сохранить.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Согласовать с выключением Windows</translation></message>
    <message><source>Copy all logs</source><translation>Копировать весь журнал</translation></message>
    <message><source>Copy full report</source><translation>Копировать полный отчёт</translation></message>
    <message><source>Copy name</source><translation>Копировать имя</translation></message>
    <message><source>Copy path</source><translation>Копировать путь</translation></message>
    <message><source>Copy selection</source><translation>Копировать выделенное</translation></message>
    <message><source>Copy version</source><translation>Копировать версию</translation></message>
    <message><source>Counting staged files…</source><translation>Подсчёт подготовленных файлов…</translation></message>
    <message><source>Create</source><translation>Создать</translation></message>
    <message><source>Critical %1 MB</source><translation>Критический порог %1 МБ</translation></message>
    <message><source>Current filter</source><translation>Текущий фильтр</translation></message>
    <message><source>Current run</source><translation>Текущий запуск</translation></message>
    <message><source>Current selection</source><translation>Текущий выбор</translation></message>
    <message><source>Current usage</source><translation>Текущее использование</translation></message>
    <message><source>Date modified</source><translation>Дата изменения</translation></message>
    <message><source>Debug</source><translation>Отладка</translation></message>
    <message><source>Delete and commit this file?</source><translation>Удалить этот файл и зафиксировать удаление?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Удалить эту папку и зафиксировать удаление?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Удалить этот ключ реестра и зафиксировать удаление?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Удалить это значение реестра и зафиксировать удаление?</translation></message>

    <message><source>Direct restart?</source><translation>Перезагрузить напрямую?</translation></message>
    <message><source>Direct shutdown?</source><translation>Выключить напрямую?</translation></message>
    <message><source>Discard changes</source><translation>Отбросить изменения</translation></message>
    <message><source>Disk</source><translation>Диск</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Постоянное хранение секретного ключа домена</translation></message>
    <message><source>Done</source><translation>Готово</translation></message>
    <message><source>Edit commands</source><translation>Редактировать команды</translation></message>
    <message><source>Editable</source><translation>Можно редактировать</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Включите фильтр и перезагрузите систему для показа текущего использования</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>Расширенный режим добавляет фоновую службу, которая запускается вместе с Windows. Удалите службу перед перемещением или удалением приложения.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>Расширенный режим недоступен. Для сохранения подготовленных файлов используйте Безопасное выключение или Безопасную перезагрузку.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Введите HKLM\SOFTWARE\… и нажмите Enter для перехода</translation></message>
    <message><source>Enter a file name</source><translation>Введите имя файла</translation></message>
    <message><source>Enter a valid file name.</source><translation>Введите допустимое имя файла.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Введите допустимое имя файла или папки.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Введите допустимое имя папки.</translation></message>
    <message><source>Error</source><translation>Ошибка</translation></message>
    <message><source>Error details</source><translation>Подробности ошибки</translation></message>
    <message><source>Error details and suggestions</source><translation>Подробности ошибки и рекомендации</translation></message>
    <message><source>Expand</source><translation>Развернуть</translation></message>
    <message><source>Export list</source><translation>Экспортировать список</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Не удалось очистить журнал: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Не удалось очистить журнал: непредвиденная ошибка.</translation></message>
    <message><source>File name</source><translation>Имя файла</translation></message>
    <message><source>File or folder</source><translation>Файл или папка</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>Сохранение файлов началось и не может быть отменено.</translation></message>
    <message><source>File saving incomplete</source><translation>Сохранение файлов не завершено</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Сохранение подготовленных файлов будет пропущено. Незафиксированные изменения в защищённых путях будут потеряны после перезагрузки.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Сохранение подготовленных файлов будет пропущено. Незафиксированные изменения в защищённых путях будут потеряны после выключения.</translation></message>
    <message><source>File type</source><translation>Тип файла</translation></message>
    <message><source>Files and folders</source><translation>Файлы и папки</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Файлы и папки можно выбирать вместе.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Подготовленные файлы не будут сохранены.</translation></message>
    <message><source>Filter is disabled</source><translation>Фильтр отключён</translation></message>
    <message><source>Fit columns to contents</source><translation>Подогнать столбцы по содержимому</translation></message>
    <message><source>Folder address</source><translation>Адрес папки</translation></message>
    <message><source>Folder name</source><translation>Имя папки</translation></message>
    <message><source>Folders</source><translation>Папки</translation></message>
    <message><source>Full configuration</source><translation>Полная конфигурация</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Последующие записи в этот ключ не будут защищены UWF.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Последующие записи в этот путь не будут защищены UWF.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Глобальное исключение реестра · включая подразделы</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Перейдите в родительскую папку или выберите другое расположение.</translation></message>
    <message><source>Home</source><translation>Личная папка</translation></message>
    <message><source>Import configuration</source><translation>Импорт конфигурации</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>Импорт обновляет черновик конфигурации. Используйте Проверить и применить, чтобы записать изменения в систему.</translation></message>
    <message><source>Import results</source><translation>Результаты импорта</translation></message>
    <message><source>Imported</source><translation>Импортировано</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>В отчёте перечислены результаты команд для черновика конфигурации. Выберите строку, чтобы просмотреть полную команду и результат.</translation></message>
    <message><source>Imported: %1</source><translation>Импортировано: %1</translation></message>
    <message><source>Information</source><translation>Информация</translation></message>
    <message><source>Item details</source><translation>Подробности элемента</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Сохранение большого списка подготовленных файлов может занять больше времени.</translation></message>
    <message><source>License and copyright</source><translation>Лицензия и авторские права</translation></message>
    <message><source>Line</source><translation>Строка</translation></message>
    <message><source>Local disk (%1)</source><translation>Локальный диск (%1)</translation></message>
    <message><source>Log level</source><translation>Уровень журнала</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Не удалось прочитать журнал. Повторите попытку или проверьте доступ к файлу журнала.</translation></message>
    <message><source>Maximize</source><translation>Развернуть окно</translation></message>
    <message><source>Message details</source><translation>Подробности сообщения</translation></message>
    <message><source>Minimize</source><translation>Свернуть окно</translation></message>
    <message><source>Module</source><translation>Модуль</translation></message>
    <message><source>More</source><translation>Ещё</translation></message>
    <message><source>Move</source><translation>Переместить</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Если диск с приложением защищён UWF, сначала убедитесь, что файлы приложения сохранятся после перезагрузки. Сохранение большого количества подготовленных файлов может увеличить время выключения или перезагрузки.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Можно выбрать несколько файлов.</translation></message>
    <message><source>Name and location</source><translation>Имя и расположение</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Именованное значение: %1 · только это значение</translation></message>
    <message><source>Navigate to registry path</source><translation>Перейти к пути реестра</translation></message>
    <message><source>New folder</source><translation>Новая папка</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Новые записи появятся после обновления.</translation></message>
    <message><source>Next-session configuration</source><translation>Конфигурация следующего сеанса</translation></message>
    <message><source>No files to commit</source><translation>Нет файлов для фиксации</translation></message>
    <message><source>No logs yet</source><translation>В журнале ещё нет записей</translation></message>
    <message><source>No matching items</source><translation>Нет подходящих элементов</translation></message>
    <message><source>No matching logs</source><translation>Нет подходящих записей журнала</translation></message>
    <message><source>No overlay files found</source><translation>Файлы оверлея не найдены</translation></message>
    <message><source>No registry key selected</source><translation>Ключ реестра не выбран</translation></message>
    <message><source>None</source><translation>Нет</translation></message>
    <message><source>Not configured</source><translation>Не настроено</translation></message>
    <message><source>Not connected</source><translation>Не подключено</translation></message>
    <message><source>Not installed</source><translation>Не установлено</translation></message>
    <message><source>Not written to the system</source><translation>Не записано в систему</translation></message>
    <message><source>Off</source><translation>Выкл.</translation></message>
    <message><source>On</source><translation>Вкл.</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Одна команда uwfmgr в строке</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Храните приложение в постоянной папке, которую обычные пользователи не могут изменять. Рекомендуемое расположение:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Автоматически сохраняются только подготовленные файлы. Другие незафиксированные изменения в защищённых путях будут потеряны после перезагрузки.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Автоматически сохраняются только подготовленные файлы. Другие незафиксированные изменения в защищённых путях будут потеряны после выключения.</translation></message>
    <message><source>Open</source><translation>Открыть</translation></message>
    <message><source>Open folder</source><translation>Открыть папку</translation></message>
    <message><source>Operating system</source><translation>Операционная система</translation></message>
    <message><source>Operation and security</source><translation>Работа и безопасность</translation></message>
    <message><source>Original command</source><translation>Исходная команда</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Достигнут критический порог оверлея. Сохраните изменения и безопасно перезагрузите систему.</translation></message>
    <message><source>Overlay maximum size</source><translation>Максимальный размер оверлея</translation></message>
    <message><source>Overlay type</source><translation>Тип оверлея</translation></message>
    <message><source>Overlay usage</source><translation>Использование оверлея</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Достигнут порог предупреждения оверлея. Сохраните изменения и безопасно перезагрузите систему.</translation></message>
    <message><source>Overview</source><translation>Обзор</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Вставьте команды или загрузите их из файла…</translation></message>
    <message><source>Pending changes</source><translation>Ожидающие изменения</translation></message>
    <message><source>Pending removal</source><translation>Ожидает удаления</translation></message>
    <message><source>Permissions and protection</source><translation>Разрешения и защита</translation></message>
    <message><source>Privileges</source><translation>Привилегии</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>Обработано %1 / %2 элементов</translation></message>
    <message><source>Processing result</source><translation>Результат обработки</translation></message>
    <message><source>Protected</source><translation>Защищено</translation></message>
    <message><source>Protected volumes</source><translation>Защищённые тома</translation></message>
    <message><source>Protection and overlay</source><translation>Защита и оверлей</translation></message>
    <message><source>Quick access</source><translation>Быстрый доступ</translation></message>
    <message><source>Raw report</source><translation>Исходный отчёт</translation></message>
    <message><source>Read again</source><translation>Прочитать снова</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Прочитайте снова для получения диагностического снимка.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Прочитайте конфигурацию для показа текущего использования</translation></message>
    <message><source>Read-only</source><translation>Только чтение</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Чтение большого оверлея может занять больше времени. Его можно отменить в любой момент.</translation></message>
    <message><source>Reading canceled</source><translation>Чтение отменено</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Чтение файлов и подпапок в этом расположении…</translation></message>
    <message><source>Reading folder</source><translation>Чтение папки</translation></message>
    <message><source>Reading logs</source><translation>Чтение журнала</translation></message>
    <message><source>Reading overlay</source><translation>Чтение оверлея</translation></message>
    <message><source>Ready</source><translation>Готовность</translation></message>
    <message><source>Ready to save</source><translation>Готово к сохранению</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Записи текущего запуска появятся здесь после завершения чтения.</translation></message>
    <message><source>Recursive staging</source><translation>Подготовка с подпапками</translation></message>
    <message><source>Refresh folder</source><translation>Обновить папку</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Ключ реестра или настройка постоянного хранения</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Оставшиеся задачи можно отменить после завершения текущего объекта.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Удалить этот ключ из исключений следующего сеанса.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Удалить этот путь из исключений следующего сеанса.</translation></message>
    <message><source>Removed</source><translation>Удалено</translation></message>
    <message><source>Rename</source><translation>Переименовать</translation></message>
    <message><source>Rename…</source><translation>Переименовать…</translation></message>
    <message><source>Repair required</source><translation>Требуется исправление</translation></message>
    <message><source>Replace file</source><translation>Заменить файл</translation></message>
    <message><source>Replace the existing file?</source><translation>Заменить существующий файл?</translation></message>
    <message><source>Required configuration</source><translation>Необходимая конфигурация</translation></message>
    <message><source>Restart anyway</source><translation>Всё равно перезагрузить</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Перезагрузка продолжится после сохранения. Оставьте приложение открытым.</translation></message>
    <message><source>Restore</source><translation>Восстановить</translation></message>
    <message><source>Retry</source><translation>Повторить</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Просмотрите подробности ошибок перед продолжением выключения.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Просмотрите подробности ошибок перед перезагрузкой.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Просмотрите подробности операции. Перед повторной попыткой изменений, которые уже могли быть записаны, прочитайте текущее состояние системы.</translation></message>
    <message><source>Review the result for each target.</source><translation>Проверьте результат для каждого объекта.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Проверьте эти изменения перед записью в систему.</translation></message>
    <message><source>Running</source><translation>Работает</translation></message>
    <message><source>Save</source><translation>Сохранить</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Сохранить подготовленные файлы на диск и перезагрузить систему.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Сохранить подготовленные файлы на диск и выключить систему.</translation></message>
    <message><source>Saving staged files</source><translation>Сохранение подготовленных файлов</translation></message>
    <message><source>Saving to disk</source><translation>Сохранение на диск</translation></message>
    <message><source>Saving…</source><translation>Сохранение…</translation></message>
    <message><source>Scanning staged paths</source><translation>Сканирование подготовленных путей</translation></message>
    <message><source>Search logs</source><translation>Поиск в журнале</translation></message>
    <message><source>Search messages or modules</source><translation>Поиск сообщений или модулей</translation></message>
    <message><source>Search paths</source><translation>Поиск путей</translation></message>
    <message><source>Search this folder</source><translation>Поиск в этой папке</translation></message>
    <message><source>Select a file or enter its name</source><translation>Выберите файл или введите его имя</translation></message>
    <message><source>Select a file to open.</source><translation>Выберите файл для открытия.</translation></message>
    <message><source>Select a folder.</source><translation>Выберите папку.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Выберите ключ для операции со всем его поддеревом или именованное значение для операции только с этим значением.</translation></message>
    <message><source>Select an available subkey</source><translation>Выбрать доступный подраздел</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Выберите только файлы или откройте папку для просмотра её содержимого.</translation></message>
    <message><source>Select files or folders</source><translation>Выбрать файлы или папки</translation></message>
    <message><source>Select folder</source><translation>Выбрать папку</translation></message>
    <message><source>Select folders only.</source><translation>Выбирайте только папки.</translation></message>
    <message><source>Select registry key or value</source><translation>Выбрать ключ или значение реестра</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Выбрать ключ или значение реестра для удаления</translation></message>
    <message><source>Select the current folder</source><translation>Выбрать текущую папку</translation></message>
    <message><source>Select this file</source><translation>Выбрать этот файл</translation></message>
    <message><source>Select this registry key</source><translation>Выбрать этот ключ реестра</translation></message>
    <message><source>Select this registry value</source><translation>Выбрать это значение реестра</translation></message>
    <message><source>Selected file</source><translation>Выбранный файл</translation></message>
    <message><source>Selected folder</source><translation>Выбранная папка</translation></message>
    <message><source>Selection</source><translation>Выбор</translation></message>
    <message><source>Service account</source><translation>Учётная запись службы</translation></message>
    <message><source>Service configuration needs repair</source><translation>Конфигурацию службы необходимо исправить</translation></message>
    <message><source>Service details</source><translation>Подробности службы</translation></message>
    <message><source>Service name</source><translation>Имя службы</translation></message>
    <message><source>Service status</source><translation>Состояние службы</translation></message>
    <message><source>Service stopped</source><translation>Служба остановлена</translation></message>
    <message><source>Shut down anyway</source><translation>Всё равно выключить</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>Выключение продолжится после сохранения. Оставьте приложение открытым.</translation></message>
    <message><source>Size</source><translation>Размер</translation></message>
    <message><source>Some changes could not be applied</source><translation>Не удалось применить некоторые изменения</translation></message>
    <message><source>Some files could not be saved</source><translation>Не удалось сохранить некоторые файлы</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Не удалось прочитать часть сведений об оборудовании. Другие диагностические сведения по-прежнему доступны для просмотра и копирования.</translation></message>
    <message><source>Source code</source><translation>Исходный код</translation></message>
    <message><source>Staged file</source><translation>Файл для сохранения</translation></message>
    <message><source>Staged path</source><translation>Путь для сохранения</translation></message>
    <message><source>Standard user</source><translation>Обычный пользователь</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Автоматический запуск с Windows</translation></message>
    <message><source>Startup type</source><translation>Тип запуска</translation></message>
    <message><source>Status unavailable</source><translation>Состояние недоступно</translation></message>
    <message><source>Stopped</source><translation>Остановлено</translation></message>
    <message><source>Storage</source><translation>Хранилище</translation></message>
    <message><source>System</source><translation>Система</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Изменения системы при первом включении UWF</translation></message>
    <message><source>System memory</source><translation>Системная память</translation></message>
    <message><source>System metadata</source><translation>Системные метаданные</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Системные метаданные предназначены только для диагностики и не могут быть зафиксированы.</translation></message>
    <message><source>Takes effect after restart</source><translation>Вступает в силу после перезагрузки</translation></message>
    <message><source>Target</source><translation>Объект</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Постоянное хранение лицензий служб терминалов</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Поставщик UWF не смог завершить перечисление. Это может произойти при большом оверлее или нехватке системных ресурсов.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>Конфигурация записана в систему. Большинство изменений вступит в силу после перезагрузки.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>Конфигурация соответствует целевой конфигурации следующего сеанса. Просмотрите или экспортируйте команды в разделе Полная конфигурация.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Текущие записи журнала будут удалены без возможности восстановления.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Папка назначения не существует.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Не удалось прочитать состояние службы расширенного режима. Проверьте разрешения службы и повторите попытку.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>В списке могут отсутствовать файлы или присутствовать уже зафиксированные. Не используйте его как список для фиксации.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Выбранный элемент больше не существует: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Служба установлена, но остановлена. Запустите её для восстановления автосохранения.</translation></message>
    <message><source>This PC</source><translation>Этот компьютер</translation></message>
    <message><source>This folder is empty</source><translation>Эта папка пуста</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Отчёт может содержать имена учётных записей, идентификаторы устройств и пути файлов. Проверьте его перед отправкой другим лицам.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Сохраните этот CMD-скрипт в UTF-8 в файл .cmd и запустите его в командной строке. При экспорте используется тот же скрипт.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Попробуйте другое ключевое слово или уровень журнала.</translation></message>
    <message><source>Try another search term.</source><translation>Попробуйте другой поисковый запрос.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Тип и максимальный размер нельзя менять при включённом фильтре. Сначала отключите фильтр и перезагрузите систему.</translation></message>
    <message><source>UI agent</source><translation>Агент интерфейса</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · строк: %1</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>Не удалось прочитать конфигурацию UWF. Убедитесь, что Unified Write Filter установлен в Windows, и прочитайте снова.</translation></message>
    <message><source>UWF feature</source><translation>Компонент UWF</translation></message>
    <message><source>UWF filter</source><translation>Фильтр UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF сейчас недоступен</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Безопасная перезагрузка UWF будет выполнена напрямую.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>Безопасное выключение UWF будет выполнено напрямую.</translation></message>
    <message><source>Unable to access this folder</source><translation>Нет доступа к этой папке</translation></message>
    <message><source>Unable to access this registry key</source><translation>Нет доступа к этому ключу реестра</translation></message>
    <message><source>Unable to read logs</source><translation>Не удалось прочитать журнал</translation></message>
    <message><source>Unable to read service status</source><translation>Не удалось прочитать состояние службы</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Сейчас не удаётся прочитать оверлей</translation></message>
    <message><source>Unable to read this folder.</source><translation>Не удалось прочитать эту папку.</translation></message>
    <message><source>Unable to select this item</source><translation>Невозможно выбрать этот элемент</translation></message>
    <message><source>Unavailable</source><translation>Недоступно</translation></message>
    <message><source>Undo removal</source><translation>Отменить удаление</translation></message>
    <message><source>Unprotected</source><translation>Не защищено</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Неуспешные элементы не сохранены. Выберите строку, чтобы увидеть полный путь и причину.</translation></message>
    <message><source>Up one folder</source><translation>На папку выше</translation></message>
    <message><source>Usage unavailable</source><translation>Использование недоступно</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Использовать весь ключ, включая все подразделы</translation></message>
    <message><source>Version %1</source><translation>Версия %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Просмотр записей приложения и диагностической информации.</translation></message>
    <message><source>View commands for these changes</source><translation>Просмотреть команды этих изменений</translation></message>
    <message><source>View failure details</source><translation>Просмотреть подробности ошибок</translation></message>
    <message><source>View overlay</source><translation>Просмотреть оверлей</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Просмотр диагностического снимка файлов для %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Привязка тома %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Защита тома %1</translation></message>
    <message><source>Volume identity</source><translation>Идентификатор тома</translation></message>
    <message><source>Warning</source><translation>Предупреждение</translation></message>
    <message><source>Warning %1 MB</source><translation>Порог предупреждения %1 МБ</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Весь ключ и все подразделы · рекурсивно</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Защиту от записи обеспечивает Windows UWF. Приложение настраивает и управляет ею.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Без расширенного режима используйте Безопасное выключение или Безопасную перезагрузку в этом приложении для сохранения подготовленных файлов.</translation></message>
    <message><source>Write protection</source><translation>Защита от записи</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Записать эти изменения конфигурации в систему? Большинство вступит в силу после перезагрузки.</translation></message>
    <message><source>staged files</source><translation>файлы для сохранения</translation></message>
    <message><source>uwfmgr commands</source><translation>команды uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Пометить выбранные пути для удаления. Изменение вступит в силу после применения.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Удалить из списка автоматического сохранения файлов после применения.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Добавить в список автоматического сохранения файлов после применения.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Применение этого плана очистит сохранённый список файлов и удалит связанное исключение реестра, если оно больше не требуется.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Изменения списка файлов нужно применить в UWF Manager; их нельзя экспортировать как команды uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Список файлов сохраняется после применения</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Область снимка наложения</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Этот список предназначен только для диагностики. По нему нельзя определить, какие файлы нужно зафиксировать.</translation></message>
    <message><source>NTFS volumes only</source><translation>Только тома NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Чтение больших наложений может быть медленным или завершиться ошибкой из-за нехватки ресурсов.</translation></message>
    <message><source>Small files may be missing</source><translation>Небольшие файлы могут отсутствовать</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Файлы меньше размера кластера диска (обычно 4 КБ) могут не отображаться в списке.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Не показывает ожидающие фиксации изменения</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>В списке также могут быть уже зафиксированные файлы, исключённые пути и элементы, затронутые другими операциями.</translation></message>

    <message><source>Command processing result</source><translation>Результат обработки команды</translation></message>
    <message><source>Copy command</source><translation>Копировать команду</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · Не удалось прочитать ветвей: %1</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>Не удалось прочитать ветвей: %1. Продолжите с найденным содержимым или повторите только неудачные операции чтения.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>Файлов: %1 · папок: %2</translation></message>
    <message><source>%1 values · %2 keys</source><translation>Значений: %1 · ключей: %2</translation></message>
    <message><source>Attempts</source><translation>Попытки</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Отменить оставшиеся задачи</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Зафиксировать только успешно просмотренное содержимое. Непрочитанные ветви останутся без изменений.</translation></message>
    <message><source>Committing changes</source><translation>Фиксация изменений</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Завершённые удаления и фиксации нельзя отменить.</translation></message>
    <message><source>Continue…</source><translation>Продолжить…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Удалить и зафиксировать только найденные объекты. Непрочитанные ветви и их родительские папки или ключи будут сохранены.</translation></message>
    <message><source>Deleting and committing</source><translation>Удаление и фиксация</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Удаление и фиксацию нельзя отменить. Обрабатывается только выбранная область; ожидающие настройки UWF не применяются.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Определение области. Непрочитанные ветви записываются в журнал, просмотр остального содержимого продолжается.</translation></message>
    <message><source>Export traversal records</source><translation>Экспорт журнала обхода</translation></message>
    <message><source>No actionable content</source><translation>Нет доступного для обработки содержимого</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Объекты для обработки не найдены. Ничего не удалено и не зафиксировано.</translation></message>
    <message><source>No changes have been committed.</source><translation>Изменения ещё не зафиксированы</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Ничего не удалено и не зафиксировано</translation></message>
    <message><source>Path</source><translation>Путь</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Безвозвратно удалить выбранную область и сохранить удаления на защищённом томе.</translation></message>
    <message><source>Preparation complete</source><translation>Подготовка завершена</translation></message>
    <message><source>Preparing commit</source><translation>Подготовка фиксации</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Подготовка удаления и фиксации</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Обработать только успешно просмотренное содержимое</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Точки повторной обработки пропускаются, чтобы обход не выходил за выбранную область.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Перезапуск не отменяет фиксацию. Обрабатывается только выбранная область; ожидающие настройки UWF не применяются.</translation></message>
    <message><source>Retry failed items</source><translation>Повторить неудачные</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Сохранить изменения выбранной области на защищённом томе.</translation></message>
    <message><source>Stopping the operation</source><translation>Остановка операции</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Записи об успешном и неудачном чтении сохранены</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Папка не пуста; необработанное содержимое сохранено.</translation></message>
    <message><source>The operation could not be completed</source><translation>Не удалось завершить операцию</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>В ключе реестра появились новые значения; необработанное содержимое сохранено.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>В ключе реестра ещё есть подразделы; необработанное содержимое сохранено.</translation></message>
    <message><source>The target no longer exists.</source><translation>Объект больше не существует.</translation></message>
    <message><source>Traversal complete</source><translation>Обход завершён</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Обход завершён, часть содержимого не прочитана</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Журнал обхода сохранён и доступен для экспорта.</translation></message>
    <message><source>Traverse again</source><translation>Повторить обход</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Повторите обход или закройте это окно.</translation></message>
    <message><source>Traversing the folder</source><translation>Обход папки</translation></message>
    <message><source>Traversing the registry</source><translation>Обход реестра</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Не удалось подготовить операцию</translation></message>
    <message><source>View results</source><translation>Показать результаты</translation></message>

    <message><source>All changes committed</source><translation>Все изменения сохранены</translation></message>
    <message><source>Some changes committed</source><translation>Изменения сохранены частично</translation></message>
    <message><source>No changes could be committed</source><translation>Не удалось сохранить изменения</translation></message>
    <message><source>No changes to commit</source><translation>Нет изменений для сохранения</translation></message>
    <message><source>Commit interrupted</source><translation>Сохранение прервано</translation></message>
    <message><source>%1 items were not processed.</source><translation>Не обработано элементов: %1.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>Не удалось выполнить операций с реестром: %1.</translation></message>
    <message><source>The folder does not exist.</source><translation>Папка не существует.</translation></message>
    <message><source>Access denied.</source><translation>Доступ запрещён.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Элемент с таким именем уже существует.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Не удалось создать папку. Проверьте имя и права доступа.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Не удалось переименовать элемент. Проверьте имя и права доступа.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Этот обозреватель не поддерживает переименование.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Черновик списка подготовленных файлов изменился после открытия этого плана. Проверьте новый план.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Сохранённый список подготовленных файлов изменился после открытия этого плана. Обновите данные и проверьте изменения.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Сохранённый список подготовленных файлов не соответствует принятому плану.</translation></message>
    <message><source>Export configuration</source><translation>Экспортировать конфигурацию</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Сохранить текущую тему, язык и состояние отображения HUB в файл UWF.ini рядом с приложением.</translation></message>
    <message><source>Configuration saved</source><translation>Конфигурация сохранена</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Текущие настройки интерфейса сохранены в:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini уже существует. Существующий файл не изменён:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>Не удалось сохранить UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Рабочий стол</translation></message>
    <message><source>Downloads</source><translation>Загрузки</translation></message>
    <message><source>Documents</source><translation>Документы</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Автофиксация файлов остановлена. Устраните ошибку перед повторной попыткой.</translation></message>
    <message><source>No net change for this setting</source><translation>Итоговое значение этой настройки не изменилось после импорта</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Итоговое значение черновика: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Итоговое значение черновика: %1 → %2 (запрошено: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Запрошено: %1 → %2; заменено последующей командой. Итоговое значение черновика → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Черновик %1 обновлён</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Черновик настройки защиты тома %1 обновлён → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Выберите записи для удаления.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Сейчас невозможно восстановить выбранные записи.</translation></message>
</context>
</TS>
