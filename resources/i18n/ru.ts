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
    <message><source>Disabled</source><comment>service startup type</comment><translation>Отключено</translation></message>
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
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Защитить этот том в следующем сеансе. Запись по защищённым путям перенаправляется в оверлей; сохранение данных после перезапуска зависит от настроек оверлея.</translation></message>
    <message><source>Current session</source><translation>Текущий сеанс</translation></message>
    <message><source>Next session</source><translation>Следующий сеанс</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Способ распознавания тома. Буква диска проще, но привязка теряется при её смене, например после добавления или удаления дисков. Идентификатор тома остаётся постоянным.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Том не поддерживается UWF: </translation></message>
    <message><source>Global settings</source><translation>Общие настройки</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Состояние фильтра UWF в текущем сеансе (только чтение).</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Включить UWF в следующем сеансе. Запись по защищённым путям перенаправляется в оверлей; сохранение данных после перезапуска зависит от настроек оверлея.</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>Чтобы изменить тип или максимальную ёмкость оверлея:
1. Отключите UWF.
2. Перезапустите устройство.
3. Измените этот параметр.</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM использует системную память. Disk использует системный диск и поддерживает оверлеи большего размера. Постоянный оверлей позволяет сохранять данные оверлея Disk после перезапуска.</translation></message>
    <message><source>Type</source><translation>Тип</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>Ёмкость оверлея RAM ограничена системной памятью. Оверлею Disk требуется не менее 1024 МБ и достаточно свободного места на системном диске.</translation></message>
    <message><source>Maximum capacity</source><translation>Максимальная ёмкость</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>Максимальная ёмкость · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Порог предупреждения</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Записывает предупреждение в журнал событий, когда объём занятого места в оверлее достигает этого значения. Если оба события включены, это значение должно быть ниже критического порога. Значение 0 отключает только событие предупреждения.</translation></message>
    <message><source>Critical threshold</source><translation>Критический порог</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Записывает критическое уведомление в журнал событий, когда объём занятого места в оверлее достигает этого значения. Если оба события включены, это значение должно быть выше порога предупреждения. Значение 0 отключает только критическое событие.</translation></message>
    <message><source>Overlay</source><translation>Оверлей</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Неподдерживаемый тип диска (только локальные фиксированные диски).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Том превышает лимит UWF: 16 TiB на защищённый том.</translation></message>

    <message><source>%1 free / %2</source><translation>Свободно %1 / %2</translation></message>
    <message><source>Commit</source><translation>Зафиксировать</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>Зафиксировать изменения из оверлея на диске или в реестре. Это действие нельзя отменить.</translation></message>
    <message><source>Commit file changes…</source><translation>Зафиксировать изменения файла…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Выберите файл и зафиксируйте его изменения из оверлея на диске.</translation></message>
    <message><source>Commit folder changes…</source><translation>Зафиксировать изменения папки…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Выберите папку и зафиксируйте на диске изменения из оверлея для каждого файла в ней.</translation></message>
    <message><source>Delete and commit file…</source><translation>Удалить файл и зафиксировать удаление на диске…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Выберите файл для удаления и зафиксируйте удаление на диске.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Удалить папку со всем содержимым и зафиксировать удаление на диске…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>Выберите папку для удаления со всем содержимым и вложенными папками, затем зафиксируйте удаления на диске.</translation></message>
    <message><source>Commit registry changes…</source><translation>Зафиксировать изменения реестра…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Введите ключ реестра и необязательное имя значения, затем зафиксируйте изменения в реестре. Если имя пустое, фиксируются все значения ключа и его подключей.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Зафиксировать удаление из реестра…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>Введите ключ реестра и необязательное имя значения. Оставьте имя значения пустым, чтобы удалить ключ со всеми подключами и значениями, а затем зафиксировать удаления.</translation></message>
    <message><source>File exclusions</source><translation>Исключения файлов</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Файлы и папки тома вне защиты UWF. Дважды щёлкните запись для копирования пути.</translation></message>
    <message><source>Registry exclusions</source><translation>Исключения реестра</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>Исключения реестра — глобальные настройки; они отображаются только для тома с Windows.</translation></message>
    <message><source>File staging</source><translation>Список автофиксации</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>Добавить файлы или папку в список автофиксации.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Выберите один или несколько файлов для автоматической фиксации перед безопасным выключением или перезапуском.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Выберите папку, файлы которой вместе с файлами вложенных папок будут фиксироваться перед безопасным выключением или перезапуском.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Выбрать файлы для автоматической фиксации</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Выбрать папку для автоматической рекурсивной фиксации</translation></message>
    <message><source>Cannot stage this path</source><translation>Невозможно добавить путь в автофиксацию</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Этот путь пересекается с файловым исключением UWF. Удалите исключение перед добавлением пути в список:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Этот путь пересекается с записью списка автофиксации. Удалите путь из списка перед добавлением исключения:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Путь пересекается с существующей записью списка автофиксации</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Исключение реестра необходимо, пока список автофиксации не пуст</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Путь не существует или имеет неожиданный тип:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Не удалось проверить путь:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Точки повторной обработки нельзя добавлять в список автофиксации:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>У пути нет буквы диска, поэтому его нельзя добавить в список автофиксации:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Выбранный путь находится на томе %1. Добавьте его на вкладке «Список автофиксации» этого тома.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Корень тома нельзя добавлять: это привело бы к рекурсивной фиксации всего тома.</translation></message>
    <message><source>File staging could not be saved</source><translation>Не удалось сохранить список автофиксации</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Список автофиксации перезагружен из-за сбоя обновления:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Список автофиксации недоступен: %1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Записей в списке: %1 · файлов: %2 · папок: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Список автофиксации использует это исключение реестра, чтобы сохраняться после перезапуска.</translation></message>
    <message><source>Built-in</source><translation>Встроенный</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Фильтр UWF отключён. Изменения в оверлее не накапливались; фиксировать нечего.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Файловая система %1 не поддерживает пофайловую фиксацию.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Этот том не поддерживается UWF.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Файловая система %1 не поддерживает пофайловую фиксацию. Доступна только фиксация реестра (его исключения глобальны).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Том сейчас не защищён UWF; нет изменений файлов для фиксации.</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Выбрать файл для фиксации на диске</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Выбрать папку для фиксации на диске</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Выбрать файл для фиксации удаления</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Выбрать папку для фиксации удаления</translation></message>
    <message><source>Name</source><translation>Имя</translation></message>
    <message><source>Data</source><translation>Данные</translation></message>
    <message><source>(Default)</source><translation>(По умолчанию)</translation></message>
    <message><source>(value not set)</source><translation>(значение не задано)</translation></message>
    <message><source>(empty)</source><translation>(пусто)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Предпросмотр недоступен (значение слишком велико)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Открыть диагностическое представление файлов, находящихся в оверлее этого тома.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Список файлов оверлея доступен только на томах NTFS.</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>Фильтр UWF сейчас отключён; ни на одном томе нет оверлея для просмотра.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Том не защищён UWF и не имеет оверлея для просмотра.</translation></message>
    <message><source>Failed: %1</source><translation>Ошибка: %1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>Предыдущее перечисление, возможно, ещё выполняется. Пока нельзя начать новое чтение.</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>Служба UWF недоступна при перечислении файлов оверлея.</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>Недостаточно памяти или система не поддерживает эту операцию. Функция доступна только для томов NTFS; закройте приложения с высоким потреблением памяти и повторите попытку.</translation></message>
    <message><source>Raw error: %1</source><translation>Код ошибки: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Экспорт списка файлов оверлея</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Текстовые файлы (*.txt);;Все файлы (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Командные файлы (*.cmd);;Все файлы (*)</translation></message>
    <message><source>Export failed</source><translation>Ошибка экспорта</translation></message>
    <message><source>Export finished</source><translation>Экспорт завершён</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Ошибка: Открытие файла (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Ошибка: Запись файла (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Сохранено записей: %1. Путь:
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Нельзя исключить том целиком (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>Файлы подкачки, swap и гибернации нельзя добавлять в исключения.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Нельзя исключить каталог целиком: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Нельзя исключить каталог целиком: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Нельзя исключить каталог целиком: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Нельзя исключить критический системный файл: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Файл реестра пользователя NTUSER.DAT нельзя исключать.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Ключ реестра не может быть пустым.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Ключ реестра не может начинаться с обратной косой черты.</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>Пути реестра не могут содержать нулевые символы.</translation></message>
    <message><source>A registry key component is too long.</source><translation>Один из компонентов ключа реестра слишком длинный.</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>Путь реестра превышает допустимую глубину вложенности UWF.</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>Добавить файл или папку в список исключений.</translation></message>
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
    <message><source>Review and apply</source><translation>Проверка и применение</translation></message>
    <message><source>Review all pending changes and apply them in one batch.</source><translation>Проверить все ожидающие изменения и применить одним пакетом.</translation></message>
    <message><source>Safe shutdown</source><translation>Безопасное выключение</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Система безопасно выключится через UWF. Доступно даже при заполненном оверлее UWF.</translation></message>
    <message><source>Safe restart</source><translation>Безопасный перезапуск</translation></message>
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
    <message><source>The system did not find the target.</source><translation>Система не нашла целевой объект.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Система отклонила параметр: неверный путь или аргумент.</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Ошибка операции. Объект может использоваться другой программой, например Проводником или открытым файлом. Закройте использующие его программы и повторите.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Операция не выполнена; подробности в журнале.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Операция отклонена (код %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Неизвестно.</translation></message>
    <message><source>Commit canceled</source><translation>Фиксация отменена</translation></message>

    <message><source>Succeeded</source><translation>Успешно</translation></message>
    <message><source>Skipped</source><translation>Пропущено</translation></message>
    <message><source>Failed</source><translation>Ошибка</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>Успешно: %1; неудачно: %2.</translation></message>

    <message><source>Existed before</source><translation>Существовал до удаления</translation></message>
    <message><source>Exists after</source><translation>Существует после удаления</translation></message>
    <message><source>Error code</source><translation>Код ошибки</translation></message>
    <message><source>Reason</source><translation>Причина</translation></message>
    <message><source>Previous page</source><translation>Предыдущая страница</translation></message>
    <message><source>First page</source><translation>Первая страница</translation></message>
    <message><source>Last page</source><translation>Последняя страница</translation></message>
    <message><source>Next page</source><translation>Следующая страница</translation></message>
    <message><source>Copy selected rows</source><translation>Копировать выбранные строки</translation></message>
    <message><source>Copy all</source><translation>Копировать всё</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Это исключение реестра необходимо, пока список автофиксации содержит сохранённые записи. Очистите список перед удалением исключения.</translation></message>
    <message><source>Copy current page</source><translation>Копировать текущую страницу</translation></message>
    <message><source>No pending changes</source><translation>Нет ожидающих изменений</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Исключения реестра · Общие настройки.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Перейти к настройкам защиты, исключениям файлов и списку автофиксации тома %1.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Не поддерживается</translation></message>
    <message><source>Review and apply changes</source><translation>Проверка и применение изменений</translation></message>
    <message><source>Confirm apply</source><translation>Подтвердить применение</translation></message>
    <message><source>Result</source><translation>Результат</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Фильтр: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Задание порога предупреждения: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Задание критического порога: %1 MB</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ Тип / максимальная ёмкость не применены: UWF включён. Сначала отключите UWF и перезапустите устройство.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Задание типа оверлея: %1</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ Максимальная ёмкость оверлея установлена: %1 МБ</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Том %1 · Защита UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Том %1 · Привязка по: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Том %1 · Добавление исключения файла: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Том %1 · Удаление исключения файла: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Добавление исключения реестра: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Удаление исключения реестра: %1</translation></message>
    <message><source>Time</source><translation>Время</translation></message>
    <message><source>Level</source><translation>Уровень</translation></message>

    <message><source>Message</source><translation>Сообщение</translation></message>
    <message><source>Committing staged files</source><translation>Фиксация файлов из списка</translation></message>
    <message><source>No additional error details were provided.</source><translation>Дополнительные сведения об ошибке отсутствуют.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Автоматическая фиксация файлов неожиданно остановлена:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Автоматическая фиксация файлов остановлена из-за неизвестной ошибки.</translation></message>
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
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>Найдено файлов: %1 · Зафиксировано: %2 · Пропущено файлов: %3 · Пропущено записей: %4 · Ошибок: %5</translation></message>
    <message><source>File staging preparation</source><translation>Подготовка к фиксации файлов из списка</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Точки повторной обработки не поддерживаются.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Можно добавлять только абсолютные пути на локальных томах.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Можно добавлять только абсолютные пути на локальных томах.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Не удалось проверить путь из списка.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Файл из списка теперь указывает на путь другого типа.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Папка из списка теперь указывает на путь другого типа.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Не удалось полностью перечислить содержимое папки из списка.</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>Нельзя рекурсивно обрабатывать корень тома.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Поставщик UWF отклонил фиксацию.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Ошибка фиксации: Неизвестная ошибка.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Дополнительных ошибок в журнале: %1.</translation></message>



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
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>Импортировать команды uwfmgr в черновик. Изменения записываются только после подтверждения в разделе «Проверка и применение».</translation></message>
    <message><source>Load from file…</source><translation>Загрузить из файла…</translation></message>
    <message><source>Load recommended configuration</source><translation>Загрузить рекомендуемую конфигурацию</translation></message>
    <message><source>Choose recommended configuration</source><translation>Выбрать рекомендуемую конфигурацию</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Выберите группы рекомендуемой конфигурации. Перед импортом можно проверить и удалить отдельные команды.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Из официальной документации Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Исключения UWF, рекомендуемые Microsoft</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>Проверьте эти рекомендации перед импортом.</translation></message>
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
    <message><source>Command import is not ready.</source><translation>Импорт команд не готов.</translation></message>
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
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>Изменений не применено: %1.

Отбросить их и продолжить?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Неполная команда uwfmgr</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Отсутствует аргумент: Размер (MB)</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>Размер должен быть неотрицательным целым числом (МБ)</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Отсутствует аргумент: Оверлей · Тип (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Недопустимое значение: Оверлей · Тип: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Отсутствует аргумент: Том (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Том должен быть буквой диска, например C:</translation></message>
    <message><source>Missing path argument</source><translation>Отсутствует аргумент: Путь</translation></message>
    <message><source>Missing registry key argument</source><translation>Отсутствует аргумент: Реестр · Ключ</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Незакрытая двойная кавычка в команде</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Неожиданный лишний аргумент: %1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>Эту команду uwfmgr нельзя импортировать.</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Нужное состояние уже установлено; действие не требуется</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Путь не на этом томе либо том не поддерживает исключения файлов (например, exFAT / ReFS)</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>Этот путь нельзя добавить в исключения.</translation></message>
    <message><source>Invalid size value: %1</source><translation>Недопустимое значение: Размер: %1</translation></message>
    <message><source>maximum capacity</source><translation>максимальная ёмкость</translation></message>
    <message><source>warning threshold</source><translation>Порог предупреждения</translation></message>
    <message><source>critical threshold</source><translation>Критический порог</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>Глобальная конфигурация недоступна; команда не добавлена в ожидающие изменения.</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>Неизвестный том %1 (нет соответствующей конфигурации тома)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Том %1: Нужное состояние уже установлено; действие не требуется</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>Путь %1 не содержит буквы диска; невозможно определить вкладку тома.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Том %1: UWF Не поддерживается</translation></message>
    <message><source>file exclusion</source><translation>Исключения файлов</translation></message>
    <message><source>registry exclusion</source><translation>Исключения реестра</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Вкладка реестра отсутствует; исключения реестра недоступны</translation></message>
    <message><source>Unsupported command</source><translation>Команда: Не поддерживается</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>Показать или скрыть виджет оверлея.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Включено</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: отключён</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Неизвестно</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Занято %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Расширенный режим</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Согласовать автоматическую фиксацию файлов с выключением и перезапуском Windows.</translation></message>
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
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>Служба сейчас не может получать предварительные уведомления Windows о завершении работы.</translation></message>
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
    <message><source>Show main window</source><translation>Показать главное окно</translation></message>
    <message><source>Restore default position</source><translation>Восстановить положение</translation></message>
    <message><source>Show overlay widget</source><translation>Показать виджет оверлея</translation></message>
    <message><source>Hide overlay widget</source><translation>Скрыть виджет оверлея</translation></message>
    <message><source>Exit application</source><translation>Выйти из приложения</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Ошибка: Чтение значений реестра (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Перечисление файлов оверлея: Неизвестная ошибка.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Ошибка: Действие с фильтром: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Ошибка: Чтение состояния фильтра</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Ошибка: Задание порога предупреждения</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Ошибка: Задание критического порога</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Ошибка: Чтение состояния оверлея</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Ошибка: Задание типа оверлея</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ Не удалось задать максимальную ёмкость</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Ошибка: Чтение конфигурации оверлея</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Ошибка: Защита тома %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Ошибка: Задание привязки тома %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Том %1 · Ошибка: Добавление исключения файла %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Том %1 · Ошибка: Удаление исключения файла %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Ошибка: Чтение конфигурации тома</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Ошибка: Добавление исключения реестра %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Ошибка: Удаление исключения реестра %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Ошибка: Изменение параметров сохранения реестра</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Ошибка: Чтение фильтра реестра</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Не поддерживается</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Чтение состояния объекта: Неизвестная ошибка.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Неизвестная ошибка.</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>Не удалось проверить состояние объекта, поэтому результат операции неизвестен: %1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>Система приняла запрос на удаление, но после проверки объект всё ещё существует.</translation></message>
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
Управление защитой томов, оверлеем, исключениями и файлами из списка автофиксации.</translation></message>
    <message><source>Snapshot details</source><translation>Сведения о снимке</translation></message>
    <message><source>Account</source><translation>Учётная запись</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Добавьте конфигурацию UWF из сценария или текста.</translation></message>
    <message><source>Add exclusion</source><translation>Добавить исключение</translation></message>
    <message><source>Add files or folders</source><translation>Добавить файлы или папки</translation></message>
    <message><source>Add selected items</source><translation>Добавить выбранные элементы</translation></message>
    <message><source>Added</source><translation>Добавлено</translation></message>
    <message><source>Administrator</source><translation>Администратор</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>Выбранные операции завершены. UWF может пропускать исключённые объекты.</translation></message>
    <message><source>All files (*)</source><translation>Все файлы (*)</translation></message>
    <message><source>All levels</source><translation>Все уровни</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Пустой диагностический снимок не означает, что в оверлее нет изменений.</translation></message>
    <message><source>Application path</source><translation>Путь приложения</translation></message>
    <message><source>Application result</source><translation>Результат применения</translation></message>
    <message><source>Apply changes</source><translation>Применить изменения</translation></message>
    <message><source>Architecture</source><translation>Архитектура</translation></message>
    <message><source>Automatic</source><translation>Автоматически</translation></message>
    <message><source>Automatic staged file commits</source><translation>Автоматическая фиксация файлов из списка</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>Автоматическая фиксация файлов не завершена</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>Автоматически фиксировать подходящие файлы из списка при выключении и перезапуске Windows.</translation></message>
    <message><source>Available</source><translation>Доступно</translation></message>
    <message><source>Back</source><translation>Назад</translation></message>
    <message><source>Background service</source><translation>Фоновая служба</translation></message>
    <message><source>Binding</source><translation>Привязка</translation></message>
    <message><source>By drive letter</source><translation>По букве диска</translation></message>
    <message><source>By volume ID</source><translation>По идентификатору тома</translation></message>
    <message><source>Cancel reading</source><translation>Отменить чтение</translation></message>
    <message><source>Completed operations cannot be undone.</source><translation>Завершённые операции нельзя отменить.</translation></message>
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
    <message><source>Commit file changes?</source><translation>Зафиксировать изменения файла?</translation></message>
    <message><source>Commit folder changes?</source><translation>Зафиксировать изменения папки?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Зафиксировать изменения ключа реестра?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Зафиксировать изменения значения реестра?</translation></message>
    <message><source>Compatibility mode</source><translation>Режим совместимости</translation></message>
    <message><source>Configuration applied</source><translation>Конфигурация применена</translation></message>
    <message><source>Configuration error</source><translation>Ошибка конфигурации</translation></message>
    <message><source>Configured</source><translation>Настроено</translation></message>
    <message><source>Confirm before continuing</source><translation>Подтвердите перед продолжением</translation></message>
    <message><source>Connected and authenticated</source><translation>Подключено и проверено</translation></message>
    <message><source>Connecting…</source><translation>Подключение…</translation></message>
    <message><source>Contact author</source><translation>Связаться с автором</translation></message>
    <message><source>Continue editing</source><translation>Продолжить редактирование</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>Если продолжить перезапуск, незафиксированные изменения файлов могут быть потеряны.</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>Если продолжить выключение, незафиксированные изменения файлов могут быть потеряны.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Согласовать с выключением Windows</translation></message>
    <message><source>Copy all logs</source><translation>Копировать весь журнал</translation></message>
    <message><source>Copy full report</source><translation>Копировать полный отчёт</translation></message>
    <message><source>Copy name</source><translation>Копировать имя</translation></message>
    <message><source>Copy path</source><translation>Копировать путь</translation></message>
    <message><source>Copy selection</source><translation>Копировать выделенное</translation></message>
    <message><source>Copy version</source><translation>Копировать версию</translation></message>
    <message><source>Counting staged files…</source><translation>Подсчёт файлов из списка…</translation></message>
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
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>Расширенный режим недоступен. Используйте «Безопасное выключение» или «Безопасный перезапуск» для фиксации файлов из списка.</translation></message>
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
    <message><source>File commits have started and cannot be canceled.</source><translation>Фиксация файлов началась и не может быть отменена.</translation></message>
    <message><source>File commits incomplete</source><translation>Фиксация файлов не завершена</translation></message>
    <message><source>File type</source><translation>Тип файла</translation></message>
    <message><source>Files and folders</source><translation>Файлы и папки</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Файлы и папки можно выбирать вместе.</translation></message>
    <message><source>Filter is disabled</source><translation>Фильтр отключён</translation></message>
    <message><source>Fit columns to contents</source><translation>Подогнать столбцы по содержимому</translation></message>
    <message><source>Folder address</source><translation>Адрес папки</translation></message>
    <message><source>Folder name</source><translation>Имя папки</translation></message>
    <message><source>Folders</source><translation>Папки</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>Добавить этот раздел в исключения следующего сеанса.</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>Добавить этот путь в исключения следующего сеанса.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Глобальное исключение реестра · включая подразделы</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Перейдите в родительскую папку или выберите другое расположение.</translation></message>
    <message><source>Home</source><translation>Личная папка</translation></message>
    <message><source>Import configuration</source><translation>Импорт конфигурации</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>Импорт обновляет черновик. Подтвердите изменения в разделе «Проверка и применение», чтобы записать их в систему.</translation></message>
    <message><source>Import results</source><translation>Результаты импорта</translation></message>
    <message><source>Imported</source><translation>Импортировано</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>В списке показаны результаты команд для черновика конфигурации. Выберите строку, чтобы увидеть полную команду и подробности.</translation></message>
    <message><source>Imported: %1</source><translation>Импортировано: %1</translation></message>
    <message><source>Information</source><translation>Информация</translation></message>
    <message><source>Item details</source><translation>Подробности элемента</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>Фиксация большого числа файлов из списка может занять больше времени.</translation></message>
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
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Если UWF защищает диск приложения, сначала убедитесь, что его файлы сохранятся после перезапуска. Фиксация большого числа файлов из списка может увеличить время выключения или перезапуска.</translation></message>
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
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>Автоматически фиксируются только файлы из списка. Другие незафиксированные изменения могут быть потеряны после перезапуска.</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>Автоматически фиксируются только файлы из списка. Другие незафиксированные изменения могут быть потеряны после выключения.</translation></message>
    <message><source>Open</source><translation>Открыть</translation></message>
    <message><source>Open folder</source><translation>Открыть папку</translation></message>
    <message><source>Operating system</source><translation>Операционная система</translation></message>
    <message><source>Operation and security</source><translation>Работа и безопасность</translation></message>
    <message><source>Original command</source><translation>Исходная команда</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Достигнут критический порог оверлея. Сохраните изменения и безопасно перезагрузите систему.</translation></message>
    <message><source>Maximum overlay capacity</source><translation>Максимальная ёмкость оверлея</translation></message>
    <message><source>Overlay type</source><translation>Тип оверлея</translation></message>
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
    <message><source>Files to commit</source><translation>Файлы для фиксации</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Записи текущего запуска появятся здесь после завершения чтения.</translation></message>

    <message><source>Refresh folder</source><translation>Обновить папку</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Ключ реестра или настройка постоянного хранения</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>После запроса отмены оставшиеся задачи будут остановлены по окончании текущей операции.</translation></message>
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
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>Перезапуск продолжится после фиксации. Не закрывайте приложение.</translation></message>
    <message><source>Restore</source><translation>Восстановить</translation></message>
    <message><source>Retry</source><translation>Повторить</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Просмотрите подробности ошибок перед продолжением выключения.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Просмотрите подробности ошибок перед перезагрузкой.</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>Часть изменений, возможно, уже записана. Обновите состояние системы перед повторной попыткой.</translation></message>
    <message><source>Review the result for each target.</source><translation>Проверьте результат для каждого объекта.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Проверьте эти изменения перед записью в систему.</translation></message>
    <message><source>Running</source><translation>Работает</translation></message>
    <message><source>Save</source><translation>Сохранить</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>Зафиксировать файлы из списка на диске, затем перезапустить устройство.</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>Зафиксировать файлы из списка на диске, затем выключить устройство.</translation></message>

    <message><source>Committing to disk</source><translation>Фиксация на диске</translation></message>
    <message><source>Committing…</source><translation>Фиксация…</translation></message>
    <message><source>Scanning staged paths</source><translation>Сканирование путей из списка</translation></message>
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
    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>Выключение продолжится после фиксации. Не закрывайте приложение.</translation></message>
    <message><source>Size</source><translation>Размер</translation></message>
    <message><source>Some changes could not be applied</source><translation>Не удалось применить некоторые изменения</translation></message>
    <message><source>Some files could not be committed</source><translation>Не удалось зафиксировать некоторые файлы</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Не удалось прочитать часть сведений об оборудовании. Другие диагностические сведения по-прежнему доступны для просмотра и копирования.</translation></message>
    <message><source>Source code</source><translation>Исходный код</translation></message>

    <message><source>Staged path</source><translation>Путь из списка</translation></message>
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
    <message><source>Takes effect when applied</source><translation>Вступает в силу при применении</translation></message>
    <message><source>Used when UWF starts</source><translation>Используется при запуске UWF</translation></message>
    <message><source>Mixed activation times</source><translation>Разное время вступления в силу</translation></message>
    <message><source>Target</source><translation>Объект</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Постоянное хранение лицензий служб терминалов</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Поставщик UWF не смог завершить перечисление. Это может произойти при большом оверлее или нехватке системных ресурсов.</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>Конфигурация записана в систему.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Текущие записи журнала будут удалены без возможности восстановления.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Папка назначения не существует.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Не удалось прочитать состояние службы расширенного режима. Проверьте разрешения службы и повторите попытку.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>В списке могут отсутствовать файлы или присутствовать уже зафиксированные. Не используйте его как список для фиксации.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Выбранный элемент больше не существует: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>Служба установлена, но остановлена. Запустите её, чтобы возобновить автоматическую фиксацию файлов.</translation></message>
    <message><source>This PC</source><translation>Этот компьютер</translation></message>
    <message><source>This folder is empty</source><translation>Эта папка пуста</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Отчёт может содержать имена учётных записей, идентификаторы устройств и пути файлов. Проверьте его перед отправкой другим лицам.</translation></message>

    <message><source>Try another keyword or log level.</source><translation>Попробуйте другое ключевое слово или уровень журнала.</translation></message>
    <message><source>Try another search term.</source><translation>Попробуйте другой поисковый запрос.</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>Перед изменением типа или максимальной ёмкости оверлея отключите UWF и перезапустите устройство.</translation></message>
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
    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>Обработка некоторых элементов не завершена или результат не удалось проверить. Выберите элемент, чтобы увидеть полный путь и подробности.</translation></message>
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
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>Без расширенного режима используйте «Безопасное выключение» или «Безопасный перезапуск» в этом приложении для фиксации файлов из списка.</translation></message>
    <message><source>Write protection</source><translation>Защита от записи</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>Записать эти изменения конфигурации в систему?</translation></message>
    <message><source>staged files</source><translation>файлы из списка</translation></message>
    <message><source>uwfmgr commands</source><translation>команды uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Пометить выбранные пути для удаления. Изменение вступит в силу после применения.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Удалить из списка автофиксации после применения.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Добавить в список автофиксации после применения.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Применение этого плана очистит сохранённый список автофиксации и удалит его исключение реестра, когда оно больше не требуется.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Изменения списка автофиксации нужно применить в UWF Manager; их нельзя экспортировать как команды uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Список автофиксации сохраняется после применения</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Область снимка оверлея</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Этот список предназначен только для диагностики. По нему нельзя определить, какие файлы нужно зафиксировать.</translation></message>
    <message><source>NTFS volumes only</source><translation>Только тома NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Чтение больших оверлеев может быть медленным или завершиться ошибкой из-за нехватки ресурсов.</translation></message>
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
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Удалить и зафиксировать удаления только для объектов, обход которых завершён успешно. Непрочитанные ветви и их родительские папки или ключи будут сохранены.</translation></message>
    <message><source>Deleting and committing</source><translation>Удаление и фиксация</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Удаление и фиксацию нельзя отменить. Обрабатывается только выбранная область; ожидающие настройки UWF не применяются.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Определение области. Непрочитанные ветви записываются в журнал, просмотр остального содержимого продолжается.</translation></message>
    <message><source>Export traversal records</source><translation>Экспорт журнала обхода</translation></message>
    <message><source>No actionable content</source><translation>Нет доступного для обработки содержимого</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Объекты для обработки не найдены. Ничего не удалено и не зафиксировано.</translation></message>
    <message><source>No changes have been committed.</source><translation>Изменения ещё не зафиксированы</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Ничего не удалено и не зафиксировано</translation></message>
    <message><source>Path</source><translation>Путь</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>Безвозвратно удалить выбранное содержимое и зафиксировать удаления.</translation></message>
    <message><source>Preparation complete</source><translation>Подготовка завершена</translation></message>
    <message><source>Preparing commit</source><translation>Подготовка к фиксации</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Подготовка удаления и фиксации</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Обработать только успешно просмотренное содержимое</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Точки повторной обработки пропускаются, чтобы обход не выходил за выбранную область.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Перезапуск не отменяет фиксацию. Обрабатывается только выбранная область; ожидающие настройки UWF не применяются.</translation></message>
    <message><source>Retry failed items</source><translation>Повторить неудачные</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>Зафиксировать изменения в выбранной области.</translation></message>
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

    <message><source>All operations succeeded</source><translation>Все операции выполнены успешно</translation></message>
    <message><source>Some operations succeeded</source><translation>Часть операций выполнена успешно</translation></message>
    <message><source>No operations succeeded</source><translation>Ни одна операция не выполнена успешно</translation></message>
    <message><source>No targets to process</source><translation>Нет объектов для обработки</translation></message>
    <message><source>Commit interrupted</source><translation>Фиксация прервана</translation></message>
    <message><source>%1 items were not processed.</source><translation>Не обработано элементов: %1.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>Не удалось выполнить операций с реестром: %1.</translation></message>
    <message><source>The folder does not exist.</source><translation>Папка не существует.</translation></message>
    <message><source>Access denied.</source><translation>Доступ запрещён.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Элемент с таким именем уже существует.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Не удалось создать папку. Проверьте имя и права доступа.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Не удалось переименовать элемент. Проверьте имя и права доступа.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Этот обозреватель не поддерживает переименование.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Черновик списка автофиксации изменился после открытия этого плана. Проверьте новый план.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Сохранённый список автофиксации изменился после открытия этого плана. Обновите данные и проверьте изменения.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Сохранённый список автофиксации не соответствует принятому плану.</translation></message>
    <message><source>Export interface settings</source><translation>Экспортировать настройки интерфейса</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>Сохранить текущую тему, язык и видимость виджета оверлея в UWF.ini рядом с приложением.</translation></message>
    <message><source>Interface settings saved</source><translation>Настройки интерфейса сохранены</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Обработка списка автофиксации остановлена. Устраните ошибку перед повторной попыткой.</translation></message>
    <message><source>No net change for this setting</source><translation>Итоговое значение этой настройки не изменилось после импорта</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Итоговое значение черновика: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Итоговое значение черновика: %1 → %2 (запрошено: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Запрошено: %1 → %2; заменено последующей командой. Итоговое значение черновика → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Черновик %1 обновлён</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Черновик настройки защиты тома %1 обновлён → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Выберите записи для удаления.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Сейчас невозможно восстановить выбранные записи.</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>Прямая запись в свободное пространство</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>Постоянный оверлей</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>Очистить постоянные данные при следующем запуске</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>Режим носителя только для чтения</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>Использует свободное место на диске для записи, уменьшая заполнение зарезервированного оверлея; поддерживает RAM (не рекомендуется) и Disk (рекомендуется).</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>Сохраняет записи в оверлее Disk после перезапуска; недоступно для оверлеев RAM.</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>Сбрасывает постоянный оверлей: удаляет его записи при следующем запуске и восстанавливает состояние на момент включения оверлея</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>Требуются права администратора; конфигурация оверлея доступна только для чтения.</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>Редактирование временно недоступно. Дождитесь завершения текущей операции.</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>Не удалось прочитать конфигурацию оверлея. Обновите её перед редактированием.</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>Эта система не поддерживает очистку постоянных данных при следующем запуске.</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>Текущий запрос очистки неизвестен. Обновите конфигурацию перед редактированием.</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>Для изменения этого запроса UWF и постоянное наложение Disk должны быть активны в текущем сеансе.</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>Очистка постоянных данных при следующем запуске требует постоянного оверлея Disk. Отмените запрос очистки или сохраните постоянный оверлей.</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>Постоянный оверлей требует Disk. Отключите его или выберите Disk.</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>Режим носителя только для чтения требует RAM. Отключите его или выберите RAM.</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>UWF включён в текущем сеансе. Отключите UWF и перезагрузите систему перед изменением этого параметра.</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>Конфигурация изменилась. Закройте и снова откройте это окно перед редактированием.</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>Дополнительные параметры оверлея</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>Текущий оверлей</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>Следующий оверлей</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>Черновик содержит несовместимые или недоступные параметры оверлея. Вернитесь к редактированию и исправьте конфигурацию.</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>Очистка запланирована на следующий запуск. Отмените запрос очистки перед отключением постоянного оверлея.</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>В следующем сеансе постоянный оверлей Disk не сохраняется. Оставьте его включённым перед запросом очистки.</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>Постоянный оверлей доступен только для Disk; для следующего сеанса выбран оверлей RAM.</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>Для включения режима носителя только для чтения требуется RAM; для следующего сеанса выбран оверлей Disk.</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>Не удалось подтвердить текущее состояние постоянного оверлея. Прочитайте конфигурацию снова.</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>Вступит в силу при следующем запуске после применения.</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>Очистка запланирована</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>Очистка не запланирована</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>Очищает один раз при следующем запуске, затем продолжает сохранять данные. Текущий сеанс не очищается немедленно.</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>Отменяет запланированную очистку при следующем запуске, сохраняя данные постоянного оверлея.</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>Сброс постоянного оверлея · Одноразовый запрос</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Обработка…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>Дождитесь завершения операции.</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · Следующий сеанс</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>Вернуться к настройкам</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>Условия изменения</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>Изменено</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>Проверка конфликтов конфигурации</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>Конфликт конфигурации</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>Условия отключения</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>UWF и постоянное наложение Disk должны быть активны в текущем сеансе. Для очистки данных также необходимо сохранить постоянное наложение Disk в следующем сеансе.</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>Условия включения</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Функция</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>Описание функции</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>Сведения о функции «%1»</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>Устраните эти конфликты конфигурации перед сохранением черновика.</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>Отменить изменения</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>Вступление в силу</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>Сейчас этот параметр нельзя изменить</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>UWF должен быть отключён в текущем сеансе, а оверлей следующего сеанса должен использовать Disk.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>UWF должен быть отключён в текущем сеансе, а оверлей следующего сеанса должен использовать RAM.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>UWF должен быть отключён в текущем сеансе. Перед отключением постоянного оверлея отмените запланированную очистку постоянных данных.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>UWF должен быть отключён в текущем сеансе. Если отключение задано только для следующего сеанса, перед изменением всё равно требуется перезапуск.</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>Условия использования</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Подробнее</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>Только просмотр</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>Почему нельзя изменить «%1»</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>UWF включён в текущем сеансе, поэтому эти параметры наложения изменить нельзя. Отмените изменения или отключите UWF и перезагрузите систему.</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>Посмотреть условия изменения</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>Изменения конфигурации не записаны. Можно повторить попытку.</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>Изменения конфигурации не требовались.</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>Для секрета учётной записи компьютера домена ($MACHINE.ACC) используйте настройку «Постоянное хранение секретного ключа домена».</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>Сам этот раздел нельзя исключить. Выберите один из его подразделов.</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>Исключение реестра должно указывать непустой подраздел HKEY_LOCAL_MACHINE (HKLM).</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>Этот сценарий содержит только настройки, представимые командами uwfmgr. Не включены:</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>Привязка томов</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>Настройки без надёжной адресации по букве диска</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>Сценарий команд</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>Конфигурация соответствует цели следующего сеанса. Просмотреть или экспортировать поддерживаемые настройки можно на вкладке «Сценарий команд».</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>Настройки с управляющими символами в путях или именах</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>Команды, зависящие от пропущенных настроек</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>Исключения UWF не позволяют надёжно зафиксировать изменения этого объекта. Удалите конфликтующее исключение и перезагрузите систему перед фиксацией.</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>Исключения UWF не позволяют надёжно зафиксировать список автофиксации. Удалите конфликтующее исключение и перезагрузите систему перед применением этого изменения.</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF не может гарантировать, что операция сохранит именно этот раздел реестра в качестве цели.</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>Перед удалением исключения реестра подтвердите, что сохранённый список автофиксации пуст.</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>Ожидает подтверждения</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Пусто</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>Системный том сейчас не защищён UWF; изменений реестра для фиксации нет.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>Изменения реестра уже сохранены постоянно; фиксация UWF не требуется.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>Изменения реестра уже сохранены постоянно; UWF не может выполнить это удаление.</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>В путях исключений UWF необходимо использовать обратную косую черту.</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>Пути к файлам не могут содержать нулевые символы.</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>Перед добавлением исключений файлов включите защиту тома в этом черновике.</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>Путь исключения содержит символы, не поддерживаемые UWF.</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>Путь исключения превышает допустимую длину UWF.</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>При активном HORM фиксация изменений файлов и реестра недоступна.</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>При активном HORM запись в реестр недоступна.</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>В режиме носителя только для чтения изменения реестра невозможно сохранить постоянно.</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>В режиме носителя только для чтения фиксация изменений файлов и реестра недоступна.</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>Перехватывает запись на физический носитель, включая метаданные. Исключения не обходят защиту. Фиксация отдельных изменений файлов и реестра недоступна; Full Commit — отдельная глобальная операция.</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>При активном HORM изменение настроек UWF недоступно.</translation></message>
    <message><source>Default value · this value only</source><translation>Значение по умолчанию · только это значение</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>Изменения файлов уже сохранены постоянно; фиксация UWF не требуется.</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>Последнее успешное получение данных об использовании оверлея: %1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>Наблюдаемые размер или тип хранилища оверлея вне допустимого для записи диапазона</translation></message>
    <message><source>Overlay (last observed)</source><translation>Оверлей (последние данные)</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>Не удалось обновить использование оверлея.
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>Не удалось обновить использование оверлея. Показаны последние успешно полученные данные от %1.
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>Не удалось сопоставить путь с локальным томом.</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF: включён (последние данные)</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>Изменения файлов уже сохранены постоянно; UWF не может выполнить это удаление.</translation></message>
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>Свободно %1 GB</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>Требуются права администратора.</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>Выполняется операция. Подождите.</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>Пытается сохранить на диск все добавления, изменения и удаления во всех защищённых томах с момента этого запуска. Это включает файлы и системные настройки; выбрать только часть изменений нельзя.</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>Автоматическое сохранение недоступно в текущем режиме</translation></message>
    <message><source>Cannot be undone once started</source><translation>Нельзя отменить после начала</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>Выберите место хранения дискового оверлея</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>Выберите место хранения дискового оверлея.</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>Изменение конфигурации временно недоступно.</translation></message>
    <message><source>Configured, inactive this session</source><translation>Настроено, не действует в этом сеансе</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>Настроено; вступит в силу после перезапуска</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>Настроено; вступит в силу после перезапуска. Оверлей текущего запуска не будет перенесён в новое место.</translation></message>
    <message><source>Configure…</source><translation>Настроить…</translation></message>
    <message><source>Current UWF</source><translation>Текущее состояние UWF</translation></message>
    <message><source>Currently configured storage volume</source><translation>Том хранения в текущей конфигурации</translation></message>
    <message><source>Default behavior</source><translation>Поведение по умолчанию</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>Отключите HORM и перезапустите компьютер перед полной фиксацией томов.</translation></message>
    <message><source>Disabled · restart required</source><translation>Отключено · требуется перезапуск</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>Отключение HORM не отключает UWF или режим носителя только для чтения и не сохраняет изменения на защищённых дисках. Перезапустите компьютер перед проверкой доступных действий.</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>Включите UWF и перезапустите компьютер перед включением HORM.</translation></message>

    <message><source>Full volume commit</source><translation>Полная фиксация томов</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>Режим полной фиксации томов (включён до перезапуска)</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>Режим полной фиксации томов уже включён. Перезапустите компьютер перед повторным использованием.</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>Полная фиксация томов уже запущена. Перезапустите компьютер перед включением HORM.</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>Полная фиксация томов доступна только при активном режиме носителя только для чтения.</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>Режим полной фиксации томов остаётся включённым до перезапуска. Новые изменения сохраняются напрямую. Отдельная фиксация остаётся недоступной.</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>Полная фиксация томов недоступна в этой системе.</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM включён. Настройки UWF и отдельная фиксация недоступны. Для их изменения отключите HORM и перезапустите компьютер.</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM отключён. Перезапустите компьютер перед изменением настроек UWF.</translation></message>
    <message><source>HORM is not available on this system.</source><translation>HORM недоступен в этой системе.</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM требует текущей конфигурации RAM в режиме носителя только для чтения.</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM требует текущего оверлея RAM.</translation></message>

    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>В режиме Disk реализация системы может включать эту опцию по умолчанию; проверьте текущее состояние в настройках.</translation></message>
    <message><source>Inactive this session</source><translation>Не действует в этом сеансе</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>Последующие изменения также будут сохраняться до перезапуска</translation></message>

    <message><source>Maintenance and recovery</source><translation>Обслуживание и восстановление</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>Перед продолжением убедитесь, что хотите сохранить именно это состояние.</translation></message>

    <message><source>Next startup: %1</source><translation>При следующем запуске: %1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>Нет подходящего тома хранения. Обновите список томов.</translation></message>
    <message><source>Not enabled</source><translation>Не включено</translation></message>
    <message><source>Not selected</source><translation>Не выбрано</translation></message>
    <message><source>Not supported</source><translation>Не поддерживается</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>После начала новые изменения записываются прямо на диск до перезапуска компьютера.</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>Одновременно можно использовать только один том хранения. Выбор нового тома заменяет прежнее место хранения и устанавливает Disk для следующего оверлея.</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>Ёмкость оверлея (сохранить текущую настройку)</translation></message>
    <message><source>Overlay configuration</source><translation>Конфигурация оверлея</translation></message>

    <message><source>Processing…</source><translation>Обработка…</translation></message>
    <message><source>Read state again</source><translation>Прочитать состояние снова</translation></message>
    <message><source>Read the current UWF state first.</source><translation>Сначала прочитайте текущее состояние UWF.</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>Режим носителя только для чтения не поддерживает отдельную фиксацию или автоматическое сохранение подготовленных файлов.</translation></message>
    <message><source>Reading state…</source><translation>Чтение состояния…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>Перезапустите компьютер и прочитайте текущее состояние перед повторным включением HORM.</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>Восстанавливать один и тот же рабочий стол и запущенные программы при каждом запуске.</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>Просмотрите доступные операции фиксации и их требования.</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>Сохранить изменения во всех защищённых томах</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>Сохранить изменения во всех защищённых томах. Ознакомьтесь с последствиями перед продолжением.</translation></message>


    <message><source>State unknown</source><translation>Состояние неизвестно</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>Успешно сохранённые изменения уже нельзя отменить перезапуском.</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>Успешно сохранённые изменения останутся</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>Состояние HORM неизвестно. Прочитайте состояние снова перед продолжением.</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>Состояние HORM неизвестно. Обновите его перед продолжением.</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>Состояние фиксации или HORM неизвестно. Обновите его перед продолжением.</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>Не удалось прочитать текущее состояние UWF. Обновите его перед продолжением.</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>Текущий оверлей использует RAM в режиме носителя только для чтения</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>Не удалось прочитать текущий режим защиты. Обновите состояние.</translation></message>
    <message><source>The result could not be confirmed</source><translation>Не удалось подтвердить результат</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>Это исключение настроено, но не обходит защиту в текущем сеансе.</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>При этом также могут измениться прямая запись в свободное пространство и постоянный оверлей. Проверьте расширенные настройки перед перезапуском.</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>Защита UWF активна в текущем сеансе</translation></message>
    <message><source>%1 (will be replaced)</source><translation>%1 (будет заменён)</translation></message>
    <message><source>Add full volume commit to the draft?</source><translation>Добавить полную фиксацию томов в черновик?</translation></message>
    <message><source>Add to draft</source><translation>Добавить в черновик</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>Действует во всех защищённых томах; начнётся только при применении этого черновика.</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>Применение этого черновика запускает полную фиксацию томов. Сохранённые изменения нельзя отменить перезапуском, а новые изменения сохраняются до перезапуска.</translation></message>
    <message><source>Configure storage location</source><translation>Настроить место хранения</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>Настройка места хранения недоступна в этой системе.</translation></message>
    <message><source>Current: %1</source><translation>Сейчас: %1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>Отключите HORM и перезапустите компьютер перед настройкой места хранения.</translation></message>
    <message><source>Disable saved-state startup?</source><translation>Отключить запуск с сохранённым состоянием?</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>Черновик: %1 · требуется применение</translation></message>
    <message><source>Enable saved-state startup?</source><translation>Включить запуск с сохранённым состоянием?</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>Полная фиксация томов (ожидает применения)</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>Полная фиксация томов ожидает применения. Она начнётся только после подтверждения в окне просмотра и применения.</translation></message>
    <message><source>Hibernate once / resume many</source><translation>Одна гибернация / много восстановлений</translation></message>
    <message><source>Not started</source><translation>Не начато</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>Удалить полную фиксацию томов из ожидающих изменений.</translation></message>
    <message><source>Save draft</source><translation>Сохранить черновик</translation></message>

    <message><source>Will start when applied</source><translation>Начнётся при применении</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>Смонтированный стационарный диск не прошёл проверку защиты. Перед включением HORM проверьте диски в списке.</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>Проверяет подходящие для UWF стационарные диски, которые остаются смонтированными. Размонтированные разделы могут быть пропущены.</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>Не удалось проверить условия включения HORM. Перед продолжением повторно считайте состояние.</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>Проверка защиты смонтированных стационарных дисков</translation></message>
    <message><source>Not using a Disk overlay</source><translation>Оверлей Disk не используется</translation></message>
    <message><source>Selected storage volume</source><translation>Выбранный том хранения</translation></message>
    <message><source>System drive default (%1)</source><translation>По умолчанию на системном диске (%1)</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>Состояние защиты изменилось во время текущего сеанса загрузки. Перед включением HORM перезагрузите компьютер.</translation></message>
    <message><source>Unable to read</source><translation>Не удалось прочитать</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>Выберите диск для хранения временных изменений. Настройка вступит в силу при следующем запуске.</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>Отключите UWF и перезагрузите компьютер перед настройкой места хранения.</translation></message>
    <message><source>Met</source><translation>Выполнено</translation></message>
    <message><source>Not met</source><translation>Не выполнено</translation></message>
    <message><source>Partition</source><translation>Раздел</translation></message>


    <message><source>Protect all</source><translation>Защитить все</translation></message>
    <message><source>Applied</source><translation>Применено</translation></message>
    <message><source>Not applied</source><translation>Не применено</translation></message>
    <message><source>Could not confirm</source><translation>Не удалось подтвердить</translation></message>
    <message><source>Hibernation is not supported</source><translation>Гибернация не поддерживается</translation></message>
    <message><source>Hibernation is not enabled</source><translation>Гибернация не включена</translation></message>
    <message><source>Fast startup only</source><translation>Только быстрый запуск</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>Групповая защита томов недоступна в этой системе.</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>Перед изменением защиты томов отключите HORM и перезагрузите компьютер.</translation></message>
    <message><source>Setup and operation</source><translation>Настройка и порядок работы</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. Подготовьте конфигурацию</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>При отключённом UWF настройте для следующего запуска оверлей RAM, режим носителя только для чтения, защиту томов и включение UWF. Примените черновик. Обеспечьте поддержку гибернации S4 и наличие полного файла гибернации.</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>После перезагрузки убедитесь, что в текущем сеансе включены UWF, оверлей RAM и режим носителя только для чтения, а все подключённые тома фиксированных дисков, поддерживаемые UWF, защищены. Убедитесь, что гибернация системы доступна.</translation></message>
    <message><source>2. Restart and verify</source><translation>2. Перезагрузите и проверьте</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. Включите HORM и выполните гибернацию</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>Добавьте включение HORM в черновик и выполните проверку и применение изменений. Применение включает HORM и соответствует команде:</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>После успешного включения подготовьте рабочий стол и приложения, затем один раз выполните гибернацию для сохранения исходного состояния:</translation></message>
    <message><source>HORM usage</source><translation>Использование HORM</translation></message>
    <message><source>Volume protection</source><translation>Защита томов</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>Настройте защиту всех томов для следующего сеанса.</translation></message>
    <message><source>Unprotect all</source><translation>Снять защиту со всех</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>Следующий запуск: все тома защищены</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>Следующий запуск: все тома без защиты</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>Следующий запуск: индивидуальные настройки томов</translation></message>
    <message><source>System hibernation is available</source><translation>Гибернация системы доступна</translation></message>
    <message><source>Usage guide</source><translation>Инструкция</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>Защита всех томов ожидает применения</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>Защита томов вступит в силу после перезагрузки</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>Нельзя одновременно снять защиту со всех томов и включить HORM.</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>Групповая защита томов не поддерживается.</translation></message>



    <message><source>Storage volume:</source><translation>Том хранения:</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>В текущем сеансе запуска исключения не обходят защиту. Поэлементное сохранение и автоматическое сохранение подготовленных файлов недоступны. Полное сохранение томов затрагивает все защищённые тома; перед продолжением прочитайте описание этой функции.</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>Повторное возобновление из одного и того же состояния гибернации</translation></message>
    <message><source>Remove full volume commit draft</source><translation>Удалить черновик полной фиксации томов</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>Включён режим носителя только для чтения</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>Не удалось прочитать настроенный маркер хранилища.</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>Список автофиксации конфликтует с исключением файла:
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>На этом компьютере недоступна полная гибернация</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>Несколько томов хранилища имеют ненулевой маркер.</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>Не удалось подтвердить дисковый тип оверлея.</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>Не удалось прочитать настроенное расположение хранилища.</translation></message>
    <message><source>The system drive could not be identified.</source><translation>Не удалось определить системный диск.</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>Прямой перезапуск пропускает фиксацию файлов из списка.</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>Прямое выключение пропускает фиксацию файлов из списка.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>Файлы из списка не будут зафиксированы. Незафиксированные изменения могут быть потеряны после перезапуска.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>Файлы из списка не будут зафиксированы. Незафиксированные изменения могут быть потеряны после выключения.</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>Фиксирует файлы из списка перед обычным выключением или перезапуском.</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>Полную фиксацию томов и включение HORM нельзя применить одновременно.</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>Настройку места хранения и включение HORM нельзя применить одновременно.</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>Для настройки места хранения и полной фиксации томов требуются разные текущие режимы UWF.</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>Выберите том для хранения перед применением изменений.</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>Настройка места хранения на диске конфликтует с запрошенным наложением в RAM.</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>Примените изменение фильтра и перезагрузите компьютер перед запросом полной фиксации томов.</translation></message>
    <message><source>Canceled</source><translation>Отменено</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>Проверенный том для хранения больше недоступен. Вернитесь к редактированию и выберите другой том.</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>Этот том нельзя зарегистрировать в UWF, поскольку его шина хранения не поддерживается.</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>Не удалось подтвердить поддержку физического тома. Обновите состояние перед включением защиты.</translation></message>
    <message><source>Paused</source><translation>Приостановлено</translation></message>
    <message><source>Partially paused</source><translation>Частично приостановлено</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>Данные постоянного оверлея сохранятся при следующем запуске. Автоматическая фиксация приостановлена.</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>Приостановленные элементы: %1 · Приостановленные файлы: %2</translation></message>
    <message><source>Read-only ROM</source><translation>ROM только для чтения</translation></message>
    <message><source>%1 changes · %2 commands</source><translation>%1 изменений · %2 команд</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>Файловая система %1: исключения файлов и пофайловая фиксация не поддерживаются.</translation></message>
    <message><source>Applying changes</source><translation>Применение изменений</translation></message>
    <message><source>Automatic commits paused</source><translation>Автоматическая фиксация приостановлена</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>После изменения режима HORM автоматическая фиксация файлов приостанавливается до перезагрузки.</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>Автоматическая фиксация файлов недоступна после FullCommit.</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>Автоматическая фиксация файлов недоступна при активном HORM.</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>Автоматическая фиксация подготовленных файлов приостановлена для этой перезагрузки.</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>Автоматическая фиксация подготовленных файлов приостановлена для этого выключения.</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>Не удалось подтвердить постоянное сохранение конфигурации. Обновите состояние системы перед изменением этого параметра.</translation></message>
    <message><source>Elapsed: %1 s</source><translation>Прошло: %1 с</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>Будут зафиксированы только подготовленные файлы, соответствующие условиям.</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>Режим ROM только для чтения не позволяет постоянно сохранять обычные настройки UWF на защищённом системном томе. Отключите UWF и перезагрузите систему перед их изменением.</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>Пропущенные подготовленные файлы не фиксируются этой операцией.</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>Подготовленные файлы уже находятся на незащищённом носителе. Автоматическая фиксация не требуется.</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>Изменение порогов требует фиксации реестра, недоступной в режиме ROM только для чтения.</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>Ожидание завершения операции Windows. Это может занять некоторое время.</translation></message>
    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM отключён. Автоматическая фиксация подготовленных файлов приостановлена до перезагрузки. Ручные операции зависят от текущих возможностей UWF.</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>Этот снимок конфигурации содержит параметры, требующие отключения UWF в текущем сеансе. Отключите UWF и перезагрузите компьютер перед его выполнением.</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>Действия текущего сеанса (HORM, FullCommit и сброс постоянного наложения; экспортируйте их из раздела «Ожидающие изменения»)</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>Массовое снятие защиты требует подтверждения полного списка томов следующего сеанса.</translation></message>
</context>
</TS>
