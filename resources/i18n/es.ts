<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="es" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Vista previa</translation></message>
    <message><source>enable</source><translation>activar</translation></message>
    <message><source>disable</source><translation>desactivar</translation></message>
    <message><source>Enabled</source><translation>Activado</translation></message>
    <message><source>Disabled</source><translation>Desactivado</translation></message>
    <message><source>Disabled</source><comment>service startup type</comment><translation>Desactivado</translation></message>
    <message><source>OK</source><translation>Aceptar</translation></message>
    <message><source>Cancel</source><translation>Cancelar</translation></message>
    <message><source>Close</source><translation>Cerrar</translation></message>
    <message><source>Refresh</source><translation>Actualizar</translation></message>
    <message><source>Add</source><translation>Añadir</translation></message>
    <message><source>Remove</source><translation>Quitar</translation></message>
    <message><source>Yes</source><translation>Sí</translation></message>
    <message><source>No</source><translation>No</translation></message>
    <message><source>Excluded</source><translation>Excluido</translation></message>
    <message><source>Not excluded</source><translation>No excluido</translation></message>
    <message><source>File…</source><translation>Archivo…</translation></message>
    <message><source>Folder…</source><translation>Carpeta…</translation></message>
    <message><source>volume ID</source><translation>ID de volumen</translation></message>
    <message><source>drive letter</source><translation>letra de unidad</translation></message>
    <message><source>Drive letter</source><translation>Letra de unidad</translation></message>
    <message><source>Volume ID</source><translation>ID de volumen</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Estado de protección del volumen en la sesión actual (solo lectura).</translation></message>
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Proteger este volumen en la próxima sesión. Las escrituras en rutas protegidas se redirigen a la superposición; su conservación tras reiniciar depende de los ajustes de la superposición.</translation></message>
    <message><source>Current session</source><translation>Sesión actual</translation></message>
    <message><source>Next session</source><translation>Próxima sesión</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identificación del volumen: la letra de unidad es sencilla, pero la asociación se pierde si cambia, por ejemplo al añadir o quitar discos. El ID de volumen permanece estable.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF no admite este volumen: </translation></message>
    <message><source>Global settings</source><translation>Configuración global</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Estado del filtro UWF en la sesión actual (solo lectura).</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Activar UWF en la próxima sesión. Las escrituras en rutas protegidas se redirigen a la superposición; su conservación tras reiniciar depende de los ajustes de la superposición.</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>Para cambiar el tipo o la capacidad máxima de la superposición:
1. Desactive UWF.
2. Reinicie el dispositivo.
3. Cambie este ajuste.</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM usa la memoria del sistema. Disk usa la unidad del sistema y admite superposiciones más grandes. La superposición persistente puede conservar los datos de una superposición Disk entre reinicios.</translation></message>
    <message><source>Type</source><translation>Tipo</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>La capacidad de la superposición RAM está limitada por la memoria del sistema. Una superposición Disk requiere al menos 1024 MB y espacio libre suficiente en la unidad del sistema.</translation></message>
    <message><source>Maximum capacity</source><translation>Capacidad máxima</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>Capacidad máxima · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Umbral de advertencia</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Registra una notificación de advertencia en el registro de eventos cuando el uso de la superposición alcanza este valor. Si ambos eventos están habilitados, este valor debe ser inferior al umbral crítico. El valor 0 deshabilita únicamente el evento de advertencia.</translation></message>
    <message><source>Critical threshold</source><translation>Umbral crítico</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Registra una notificación crítica en el registro de eventos cuando el uso de la superposición alcanza este valor. Si ambos eventos están habilitados, este valor debe ser superior al umbral de advertencia. El valor 0 deshabilita únicamente el evento crítico.</translation></message>
    <message><source>Overlay</source><translation>Superposición</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Tipo de unidad no compatible (solo discos locales fijos).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>El volumen supera el límite de UWF de 16 TiB por volumen protegido.</translation></message>

    <message><source>%1 free / %2</source><translation>%1 libres / %2</translation></message>
    <message><source>Commit</source><translation>Consolidar</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>Consolidar los cambios de la superposición en el disco o en el Registro. Esta acción no se puede deshacer.</translation></message>
    <message><source>Commit file changes…</source><translation>Consolidar cambios del archivo…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Seleccione un archivo y consolide sus cambios de la superposición en el disco.</translation></message>
    <message><source>Commit folder changes…</source><translation>Consolidar cambios de la carpeta…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Seleccione una carpeta y consolide en el disco los cambios de la superposición de todos sus archivos.</translation></message>
    <message><source>Delete and commit file…</source><translation>Eliminar y consolidar archivo…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Seleccione un archivo para eliminarlo y consolide la eliminación en el disco.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Eliminar y consolidar carpeta…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>Seleccione una carpeta para eliminarla recursivamente con todo su contenido y consolide las eliminaciones en el disco.</translation></message>
    <message><source>Commit registry changes…</source><translation>Consolidar cambios del Registro…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Introduzca una clave del Registro y, opcionalmente, un nombre de valor para consolidar los cambios. Si deja vacío el nombre del valor, se consolidan todos los valores de la clave y de todas sus subclaves.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Eliminar y consolidar en el Registro…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>Introduzca una clave del Registro y, opcionalmente, un nombre de valor. Deje vacío el nombre del valor para eliminar la clave y todas sus subclaves y valores y, después, consolidar las eliminaciones.</translation></message>
    <message><source>File exclusions</source><translation>Exclusiones de archivos</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Archivos y carpetas del volumen excluidos de UWF. Haga doble clic en una entrada para copiar su ruta.</translation></message>
    <message><source>Registry exclusions</source><translation>Exclusiones del Registro</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>Las exclusiones del Registro son globales y solo se muestran en la partición de Windows.</translation></message>
    <message><source>File staging</source><translation>Archivos preparados</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>Añadir archivos o una carpeta a la lista de consolidación automática.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Seleccione uno o más archivos para consolidarlos automáticamente antes del apagado o reinicio seguro.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Seleccione una carpeta cuyos archivos se consolidarán recursivamente antes del apagado o reinicio seguro.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Seleccionar archivos para consolidación automática</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Seleccionar carpeta para consolidación automática recursiva</translation></message>
    <message><source>Cannot stage this path</source><translation>No se puede preparar esta ruta</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Esta ruta se solapa con una exclusión de archivos UWF. Quite la exclusión antes de añadir la ruta a la lista de archivos preparados:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Esta ruta se solapa con una entrada de la lista de archivos preparados. Quite la ruta preparada antes de añadir la exclusión:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>La ruta se solapa con una entrada de la lista de archivos preparados</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Esta exclusión del Registro es necesaria mientras la lista de archivos preparados contenga entradas guardadas</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>La ruta no existe o no es del tipo esperado:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>No se pudo examinar la ruta:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Los puntos de reanálisis no se pueden añadir a la lista de archivos preparados:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>La ruta no tiene letra de unidad y no se puede añadir a «Archivos preparados»:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>La ruta seleccionada está en el volumen %1. Añádala desde la pestaña «Archivos preparados» de ese volumen.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>No se puede preparar la raíz de un volumen, ya que se consolidaría recursivamente todo su contenido.</translation></message>
    <message><source>File staging could not be saved</source><translation>No se pudo guardar la lista de archivos preparados</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Se recargó la lista de archivos preparados porque no se pudo completar la actualización:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>La función Archivos preparados no está disponible: %1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>%1 entradas preparadas · %2 archivos · %3 carpetas</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Archivos preparados depende de esta exclusión del Registro para conservar su lista entre reinicios.</translation></message>
    <message><source>Built-in</source><translation>Integrado</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>El filtro UWF está desactivado. No se han acumulado cambios en la superposición que consolidar.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>El sistema de archivos %1 no admite consolidación por archivo.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>UWF no admite este volumen.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>El sistema de archivos %1 no admite consolidación por archivo. Solo se puede consolidar el Registro (sus exclusiones son globales).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Este volumen no está protegido por UWF; no hay cambios de archivos que consolidar.</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Seleccionar archivo para consolidar en disco</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Seleccionar carpeta para consolidar en disco</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Seleccionar archivo cuya eliminación consolidar</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Seleccionar carpeta cuya eliminación consolidar</translation></message>
    <message><source>Name</source><translation>Nombre</translation></message>
    <message><source>Data</source><translation>Datos</translation></message>
    <message><source>(Default)</source><translation>(Predeterminado)</translation></message>
    <message><source>(value not set)</source><translation>(valor no establecido)</translation></message>
    <message><source>(empty)</source><translation>(vacío)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Vista previa no disponible (el valor es demasiado grande)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Abrir una vista de diagnóstico de los archivos almacenados actualmente en la superposición de este volumen.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>La lista de archivos de superposición solo funciona en volúmenes NTFS.</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>El filtro UWF está desactivado; no hay superposiciones que examinar.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Este volumen no está protegido por UWF y no tiene superposición que examinar.</translation></message>
    <message><source>Failed: %1</source><translation>Error: %1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>Es posible que la enumeración anterior siga en curso. Todavía no se puede iniciar una nueva lectura.</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>El servicio UWF no está disponible al enumerar los archivos de la superposición.</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>Memoria insuficiente o el sistema no admite esta operación. Esta función requiere un volumen NTFS; cierre las aplicaciones que consuman mucha memoria y vuelva a intentarlo.</translation></message>
    <message><source>Raw error: %1</source><translation>Código de error: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Exportar a archivo… · Superposición · archivos</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Archivos de texto (*.txt);;Todos los archivos (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Archivos de comandos (*.cmd);;Todos los archivos (*)</translation></message>
    <message><source>Export failed</source><translation>Error de exportación</translation></message>
    <message><source>Export finished</source><translation>Exportación completada</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Error: Apertura del archivo (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Error: Escritura del archivo (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Se guardaron %1 entradas en:
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>No se puede excluir un volumen completo (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>Los archivos de paginación, intercambio e hibernación no se pueden añadir como exclusiones.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>No se puede excluir el directorio completo: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>No se puede excluir el directorio completo: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>No se puede excluir el directorio completo: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>No se puede excluir este archivo esencial del sistema: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>No se puede excluir el archivo de Registro de usuario NTUSER.DAT.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>La clave del Registro no puede estar vacía.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>La clave del Registro no puede empezar por una barra inversa.</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>Las rutas del Registro no pueden contener caracteres nulos.</translation></message>
    <message><source>A registry key component is too long.</source><translation>Un componente de la clave del Registro es demasiado largo.</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>La ruta del Registro supera el límite de anidamiento de UWF.</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>Agregar un archivo o una carpeta a la lista de exclusiones.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Seleccionar archivo para añadir · Exclusiones de archivos.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Seleccionar carpeta para añadir · Exclusiones de archivos. Se incluyen todas las subcarpetas.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Introduzca la ruta de la clave del Registro que excluir.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Introduzca la ruta de la clave del Registro que excluir. / Persistencia.</translation></message>
    <message><source>Registry key…</source><translation>Registro · Clave…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Clave secreta del dominio (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Licencia de acceso de cliente de Servicios de Terminal Server (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Conservar la clave secreta del dominio (contraseña de la cuenta del equipo) entre sesiones UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Conservar las licencias de acceso de cliente de Terminal Server entre sesiones UWF.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistencia %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Quitar selección</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Quitar las exclusiones seleccionadas. Surte efecto al aplicar.</translation></message>
    <message><source>Open containing folder</source><translation>Abrir carpeta contenedora</translation></message>
    <message><source>Copy file path</source><translation>Copiar ruta del archivo</translation></message>
    <message><source>Copy registry path</source><translation>Copiar ruta del Registro</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Consolidar cambios de la carpeta en el disco…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Consolidar cambios del archivo en el disco…</translation></message>
    <message><source>Copied to clipboard</source><translation>Copiado al portapapeles</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Seleccionar archivo para añadir · Exclusiones de archivos. Puede seleccionar varios archivos.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Seleccionar carpeta para añadir · Exclusiones de archivos</translation></message>
    <message><source>Add registry exclusion</source><translation>Adición de exclusión del Registro</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>La ruta %1 no está en el volumen %2 y no puede añadirse a sus exclusiones.</translation></message>
    <message><source>Path is not on this volume</source><translation>La ruta no pertenece a este volumen</translation></message>
    <message><source>Cannot add this exclusion</source><translation>No se puede añadir esta exclusión</translation></message>
    <message><source>File: %1</source><translation>Archivo… %1</translation></message>
    <message><source>Folder: %1</source><translation>Carpeta… %1</translation></message>
    <message><source>Registry: %1</source><translation>Registro: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Excluido de forma predeterminada; puede cambiarlo.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Sesión actual: %1
Próxima sesión: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Cambios pendientes: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Administrador del filtro de escritura unificado (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Volver a leer el estado actual y la configuración de la próxima sesión de UWF.</translation></message>
    <message><source>Review and apply</source><translation>Revisar y aplicar</translation></message>
    <message><source>Review all pending changes and apply them in one batch.</source><translation>Revisar todos los cambios pendientes y aplicarlos juntos.</translation></message>
    <message><source>Safe shutdown</source><translation>Apagado seguro</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>El sistema se apagará de forma segura mediante UWF. Disponible incluso con la superposición UWF llena.</translation></message>
    <message><source>Safe restart</source><translation>Reinicio seguro</translation></message>
    <message><source>Direct restart</source><translation>Reinicio directo</translation></message>
    <message><source>Direct shutdown</source><translation>Apagado directo</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>El sistema se reiniciará de forma segura mediante UWF. Disponible incluso con la superposición UWF llena.</translation></message>
    <message><source>Log</source><translation>Registro de actividad</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Ver el registro interno de esta sesión para resolver problemas.</translation></message>
    <message><source>About</source><translation>Acerca de</translation></message>
    <message><source>Switch display language</source><translation>Cambiar idioma de visualización</translation></message>
    <message><source>Language</source><translation>Idioma</translation></message>
    <message><source>Theme</source><translation>Tema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Cambiar entre tema claro y oscuro</translation></message>
    <message><source>System information</source><translation>Información del sistema</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Instantánea de diagnóstico</translation></message>
    <message><source>Read only</source><translation>Solo lectura</translation></message>
    <message><source>The system did not find the target.</source><translation>El sistema no encontró el destino.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>El sistema rechazó un parámetro (ruta o argumento no válido).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>La operación falló. Otro proceso puede estar usando el destino, como una carpeta abierta en el Explorador o un archivo abierto. Cierre esos programas y reintente.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>La operación falló; consulte el registro para más detalles.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operación rechazada (código %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Desconocido.</translation></message>
    <message><source>Commit canceled</source><translation>Consolidación cancelada</translation></message>

    <message><source>Succeeded</source><translation>Correcto</translation></message>
    <message><source>Skipped</source><translation>Omitido</translation></message>
    <message><source>Failed</source><translation>Error</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>Correctos: %1; fallidos: %2.</translation></message>

    <message><source>Existed before</source><translation>Existía antes de la eliminación</translation></message>
    <message><source>Exists after</source><translation>Existe después de la eliminación</translation></message>
    <message><source>Error code</source><translation>Código de error</translation></message>
    <message><source>Reason</source><translation>Motivo</translation></message>
    <message><source>Previous page</source><translation>Página anterior</translation></message>
    <message><source>First page</source><translation>Primera página</translation></message>
    <message><source>Last page</source><translation>Última página</translation></message>
    <message><source>Next page</source><translation>Página siguiente</translation></message>
    <message><source>Copy selected rows</source><translation>Copiar filas seleccionadas</translation></message>
    <message><source>Copy all</source><translation>Copiar todo</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Esta exclusión del Registro es necesaria mientras la lista de archivos preparados contenga entradas guardadas. Vacíe la lista antes de quitar la exclusión.</translation></message>
    <message><source>Copy current page</source><translation>Copiar página actual</translation></message>
    <message><source>No pending changes</source><translation>No hay cambios pendientes</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Exclusiones del Registro · Configuración global.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Ir a la protección, las exclusiones de archivos y los archivos preparados del volumen %1.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: No compatible</translation></message>
    <message><source>Review and apply changes</source><translation>Revisar y aplicar cambios</translation></message>
    <message><source>Confirm apply</source><translation>Confirmar aplicación</translation></message>
    <message><source>Result</source><translation>Resultado</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filtro: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Umbral de advertencia de la superposición establecido en %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Umbral crítico de la superposición establecido en %1 MB</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ Tipo / capacidad máxima no aplicados: UWF está activado. Desactive UWF y reinicie primero.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Tipo de superposición establecido en %1</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ Capacidad máxima de la superposición establecida en %1 MB</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volumen %1 · Protección UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volumen %1 · Identificar por: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volumen %1: exclusión de archivo añadida: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volumen %1: exclusión de archivo eliminada: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Exclusión del Registro añadida: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Exclusión del Registro eliminada: %1</translation></message>
    <message><source>Time</source><translation>Hora</translation></message>
    <message><source>Level</source><translation>Nivel</translation></message>

    <message><source>Message</source><translation>Mensaje</translation></message>
    <message><source>Committing staged files</source><translation>Consolidando archivos preparados</translation></message>
    <message><source>No additional error details were provided.</source><translation>No se proporcionaron más detalles del error.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>La consolidación automática de archivos preparados se detuvo inesperadamente:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>La consolidación automática de archivos preparados se detuvo por un error desconocido.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Apagado seguro: Error</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Apagar: Error (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Reinicio seguro: Error</translation></message>
    <message><source>Direct restart failed</source><translation>Reinicio directo: Error</translation></message>
    <message><source>Direct shutdown failed</source><translation>Apagado directo: Error</translation></message>
    <message><source>Restart failed: %1</source><translation>Reiniciar: Error (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>No se pudo leer la lista de archivos preparados:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>No se pudo leer la lista de archivos preparados por un error desconocido.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>Archivos encontrados: %1 · Consolidados: %2 · Archivos omitidos: %3 · Entradas omitidas: %4 · Fallidos: %5</translation></message>
    <message><source>File staging preparation</source><translation>Preparación de archivos</translation></message>
    <message><source>Reparse points are not supported.</source><translation>No se admiten puntos de reanálisis.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Solo se pueden preparar rutas absolutas de volúmenes locales.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Solo se pueden añadir rutas absolutas de volúmenes locales a la lista de archivos preparados:
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>No se pudo examinar la ruta preparada.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>El archivo preparado ahora corresponde a otro tipo de ruta.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>La carpeta preparada ahora corresponde a otro tipo de ruta.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>No se pudo enumerar por completo la carpeta preparada.</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>La raíz del volumen no puede procesarse recursivamente.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>El proveedor UWF rechazó la consolidación.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Error al consolidar: Error desconocido.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Se registraron %1 errores adicionales.</translation></message>



    <message><source>No current-session record found for volume %1.</source><translation>No hay registro de sesión actual para el volumen %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Esta ruta está en las exclusiones de archivos.
Exclusión: %1</translation></message>

    <message><source>Continue</source><translation>Continuar</translation></message>
    <message><source>Delete and commit</source><translation>Eliminar y consolidar</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Esta clave está en las exclusiones del Registro.
Exclusión: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>No hay registro del filtro del Registro para la sesión actual.</translation></message>

    <message><source>Export commands to file</source><translation>Exportar comandos a un archivo</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Exportar a archivo… · Comando: %1
%2</translation></message>
    <message><source>Import</source><translation>Importar</translation></message>
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>Importar comandos uwfmgr al borrador. Los cambios solo se escriben tras confirmarlos en «Revisar y aplicar».</translation></message>
    <message><source>Load from file…</source><translation>Cargar desde archivo…</translation></message>
    <message><source>Load recommended configuration</source><translation>Cargar configuración recomendada</translation></message>
    <message><source>Choose recommended configuration</source><translation>Elegir configuración recomendada</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Elija los grupos recomendados que añadir. Puede revisar o borrar comandos antes de importar.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>De la documentación oficial de Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Exclusiones UWF recomendadas por Microsoft</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>Revise estas recomendaciones antes de importarlas.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Fuente: documentación oficial de Microsoft sobre UWF, exclusiones comunes y compatibilidad antimalware.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Programa para la mejora de la experiencia del usuario (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Conservar permanentemente: Estado de participación por directiva</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Conservar permanentemente: Estado de participación local</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Conservar permanentemente: Indicador de carga desactivada</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Servicio de transferencia inteligente en segundo plano (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Conservar permanentemente: Archivos de cola de descarga</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Conservar permanentemente: Índice del estado de transferencia</translation></message>
    <message><source>Network profiles and policies</source><translation>Perfiles y directivas de red</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Directiva GPO de red inalámbrica</translation></message>
    <message><source>Wired network GPO policy</source><translation>Directiva GPO de red cableada</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Directiva GPO de red inalámbrica · archivos</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Directiva GPO de red cableada · archivos</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Perfiles de interfaz inalámbrica</translation></message>
    <message><source>Wired network interface profiles</source><translation>Perfiles de interfaz cableada</translation></message>
    <message><source>Wireless service configuration</source><translation>Configuración del servicio inalámbrico</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Configuración del servicio de banda ancha móvil</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Configuración del servicio de configuración automática de redes cableadas</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>Los perfiles XML dependen del dispositivo. Añada manualmente las rutas concretas Interfaces\{GUID}\{GUID}.xml si las necesita.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Horario de verano (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Conservar permanentemente: Definiciones de zona horaria</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Conservar permanentemente: Información de zona horaria seleccionada</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Conservar permanentemente: Archivos del producto y actualizaciones</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Conservar permanentemente: Firmas y estado en ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Conservar permanentemente: Windows Update · Registro de actividad</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Conservar permanentemente: MpCmdRun · Registro de actividad</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Conservar permanentemente: Estado del producto en el Registro</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Conservar permanentemente: WdBoot · Estado del servicio</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Conservar permanentemente: WdFilter · Estado del servicio</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Conservar permanentemente: WdNisSvc · Estado del servicio</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Conservar permanentemente: WdNisDrv · Estado del servicio</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Conservar permanentemente: WinDefend · Estado del servicio</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Conservar permanentemente: Archivos del programa cliente</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Conservar permanentemente: Windows Update · Registro de actividad</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Conservar permanentemente: MpCmdRun · Registro de actividad</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Conservar permanentemente: Firmas y estado antimalware</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Conservar permanentemente: Estado antimalware en el Registro</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Elegir archivos con comandos uwfmgr</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Archivos de texto (*.txt *.cmd);;Todos los archivos (*)</translation></message>
    <message><source>Status</source><translation>Estado</translation></message>
    <message><source>Import failed</source><translation>Error de importación</translation></message>
    <message><source>Command import is not ready.</source><translation>La importación de comandos aún no está lista.</translation></message>
    <message><source>Nothing to import</source><translation>No hay comandos para importar</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>No se encontraron comandos uwfmgr en la entrada.</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplicado: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>No compatible: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Lote %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Duplicado</translation></message>
    <message><source>Unsupported</source><translation>No compatible</translation></message>
    <message><source>File too large</source><translation>Archivo demasiado grande</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>El archivo %1 supera 5 MB y no se analizó. Fíltrelo primero manualmente.</translation></message>
    <message><source>Cannot read file</source><translation>No se puede leer el archivo</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Error: Apertura del archivo (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>¿Descartar los cambios pendientes?</translation></message>
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>Hay %1 cambio(s) sin aplicar.

¿Descartarlos y continuar?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Comando uwfmgr incompleto</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Falta el argumento: Tamaño (MB)</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>El tamaño debe ser un número entero no negativo (MB)</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Falta el argumento: Superposición · Tipo (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Valor no válido: Superposición · Tipo: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Falta el argumento: Volumen (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>El volumen debe ser una letra de unidad, como C:</translation></message>
    <message><source>Missing path argument</source><translation>Falta el argumento: Ruta</translation></message>
    <message><source>Missing registry key argument</source><translation>Falta el argumento: Registro · Clave</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Comillas dobles sin cerrar en el comando</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Argumento adicional inesperado: %1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>Este comando uwfmgr no se puede importar.</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Ya está en el estado deseado; no se requiere acción</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>La ruta no pertenece al volumen o este no admite exclusiones (p. ej., exFAT / ReFS)</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>Esta ruta no se puede añadir como exclusión.</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valor no válido: Tamaño: %1</translation></message>
    <message><source>maximum capacity</source><translation>capacidad máxima</translation></message>
    <message><source>warning threshold</source><translation>Umbral de advertencia</translation></message>
    <message><source>critical threshold</source><translation>Umbral crítico</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>La configuración global no está disponible; el comando no se añadió a los cambios pendientes.</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>Volumen %1 desconocido (no hay una configuración de volumen coincidente)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volumen %1: Ya está en el estado deseado; no se requiere acción</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>La ruta %1 no tiene letra de unidad; no se puede identificar la pestaña de su volumen.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volumen %1: UWF No compatible</translation></message>
    <message><source>file exclusion</source><translation>Exclusiones de archivos</translation></message>
    <message><source>registry exclusion</source><translation>Exclusiones del Registro</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Las exclusiones del Registro requieren la pestaña Registro, que no está presente</translation></message>
    <message><source>Unsupported command</source><translation>Comando: No compatible</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>Mostrar u ocultar el widget de superposición.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Activado</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Desactivado</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Desconocido</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Usado %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Modo avanzado</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordinar la consolidación automática de archivos preparados con el apagado y reinicio de Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Error: Lectura del estado del modo avanzado (Error desconocido).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Error: Lectura del estado del servicio:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Error: Lectura del estado del servicio (Error desconocido).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>El servicio no está configurado como proceso independiente.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>El servicio no está configurado para inicio automático.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>El servicio no está configurado para ejecutarse como LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>El servicio no está en ejecución.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>La ruta o los argumentos del servicio no coinciden con este ejecutable.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>El tiempo de espera de preapagado del servicio no está bien configurado.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>El servicio no declara los privilegios para iniciar el agente de interfaz.</translation></message>
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>El servicio no puede recibir actualmente las notificaciones de preapagado de Windows.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Activar modo avanzado</translation></message>
    <message><source>Disable enhanced mode</source><translation>Desactivar modo avanzado</translation></message>
    <message><source>Start service</source><translation>Iniciar servicio</translation></message>
    <message><source>Repair enhanced mode</source><translation>Reparar modo avanzado</translation></message>
    <message><source>Delete service</source><translation>Eliminar servicio</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Detener y eliminar el servicio avanzado y todos sus datos residuales del Registro.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>El servicio no existe, pero quedan sus datos en el Registro.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Esperando la autenticación del agente de interfaz.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>El servicio está en ejecución, pero no hay agente de interfaz autenticado conectado.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Servicio auxiliar del modo avanzado de UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Modo avanzado · Configuración: Aplicado.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Configuración: Aplicado. Informe de persistencia UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Modo avanzado · Configuración: Error:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Modo avanzado · Configuración: Error (Error desconocido).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Se eliminaron el servicio avanzado y su configuración residual.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Se solicitó eliminar el servicio, pero la limpieza residual informó:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Modo avanzado: Eliminar servicio: Error:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Modo avanzado: Eliminar servicio: Error (Error desconocido).</translation></message>
    <message><source>Show main window</source><translation>Mostrar ventana principal</translation></message>
    <message><source>Restore default position</source><translation>Restaurar posición predeterminada</translation></message>
    <message><source>Show overlay widget</source><translation>Mostrar widget de superposición</translation></message>
    <message><source>Hide overlay widget</source><translation>Ocultar widget de superposición</translation></message>
    <message><source>Exit application</source><translation>Salir de la aplicación</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Error: Lectura de valores del Registro (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Enumeración de archivos de superposición: Error desconocido.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Error: Acción en filtro: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Error: Lectura del estado del filtro</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Error: Definición del umbral de advertencia</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Error: Definición del umbral crítico</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Error: Lectura del estado de la superposición</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Error: Definición del tipo de superposición</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ No se pudo establecer la capacidad máxima</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Error: Lectura de la configuración de superposición</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Error: Protección del volumen %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Error: Definición de identificación del volumen %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volumen %1 · Error: Adición de exclusión de archivo %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volumen %1 · Error: Retirada de exclusión de archivo %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Error: Lectura de la configuración del volumen</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Error: Adición de exclusión del Registro %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Error: Retirada de exclusión del Registro %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Error: Actualización de persistencia del Registro</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Error: Lectura del filtro del Registro</translation></message>
    <message><source>UWF is not available</source><translation>UWF: No compatible</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Lectura del estado del destino: Error desconocido.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Error desconocido.</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>No se pudo verificar el estado del destino, por lo que se desconoce el resultado de la operación: %1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>El sistema aceptó la solicitud de eliminación, pero el destino sigue existiendo tras la verificación.</translation></message>
    <message><source>Unknown</source><translation>Desconocido</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Error: Carga del registro de actividad (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Error: Carga del registro de actividad (Error desconocido).</translation></message>
    <message><source>: unknown error</source><translation>: Error desconocido</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Error: Identificación del volumen de destino (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Error: Enumeración de claves del Registro (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Error: Identificación del volumen de destino (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Operación omitida porque una operación previa falló o no pudo confirmarse.</translation></message>
    <message><source>Copy</source><translation>Copiar</translation></message>
    <message><source>Select All</source><translation>Seleccionar todo</translation></message>
    <message><source>Display language</source><translation>Idioma de visualización</translation></message>
    <message><source>Folder</source><translation>Carpeta</translation></message>
    <message><source>File</source><translation>Archivo</translation></message>
    <message><source>Export commands</source><translation>Exportar comandos</translation></message>
    <message><source>Direct restart…</source><translation>Reinicio directo…</translation></message>
    <message><source>Clear…</source><translation>Vaciar…</translation></message>
    <message><source>Committing</source><translation>Consolidando</translation></message>
    <message><source> · %1 selected</source><translation> · %1 seleccionados</translation></message>
    <message><source>%1 changes</source><translation>%1 cambios</translation></message>
    <message><source>%1 commands</source><translation>%1 comandos</translation></message>
    <message><source>%1 items</source><translation>%1 elementos</translation></message>
    <message><source>%1 items · %2</source><translation>%1 elementos · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 cambios pendientes</translation></message>
    <message><source>%1 records</source><translation>%1 registros</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 registros · %2 en total</translation></message>


    <message><source>%1 selected</source><translation>%1 seleccionados</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 archivos</translation></message>
    <message><source>32-bit</source><translation>32 bits</translation></message>
    <message><source>64-bit</source><translation>64 bits</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Distribuido bajo la &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Las condiciones completas están en el archivo LICENSE incluido con la aplicación.&lt;/p&gt;&lt;p&gt;Puede redistribuir o modificar el software bajo GPL v3. Se proporciona tal cual, sin ninguna garantía.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows desactiva los archivos de paginación, Restaurar sistema, SuperFetch, la indexación de archivos, la desfragmentación y el inicio rápido. También establece bootstatuspolicy en ignoreallfailures en el BCD.&lt;/p&gt;&lt;p&gt;Puede cambiar estos ajustes después. Por ejemplo, mueva el archivo de paginación a un volumen sin protección antes de volver a activar la paginación.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Una instantánea de diagnóstico del sistema, los privilegios y UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>Ya existe un archivo en esta ubicación:
%1

Al guardar se reemplazará su contenido.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Un administrador gráfico de Windows Unified Write Filter.
Gestione la protección de volúmenes, la superposición, las exclusiones y los archivos preparados.</translation></message>
    <message><source>Snapshot details</source><translation>Detalles de la instantánea</translation></message>
    <message><source>Account</source><translation>Cuenta</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Añadir una configuración UWF desde un script o texto.</translation></message>
    <message><source>Add exclusion</source><translation>Añadir exclusión</translation></message>
    <message><source>Add files or folders</source><translation>Añadir archivos o carpetas</translation></message>
    <message><source>Add selected items</source><translation>Añadir elementos seleccionados</translation></message>
    <message><source>Added</source><translation>Añadido</translation></message>
    <message><source>Administrator</source><translation>Administrador</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>Las operaciones seleccionadas finalizaron. UWF puede omitir los destinos excluidos.</translation></message>
    <message><source>All files (*)</source><translation>Todos los archivos (*)</translation></message>
    <message><source>All levels</source><translation>Todos los niveles</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Una instantánea de diagnóstico vacía no significa que la superposición no contenga cambios.</translation></message>
    <message><source>Application path</source><translation>Ruta de la aplicación</translation></message>
    <message><source>Application result</source><translation>Resultado de aplicación</translation></message>
    <message><source>Apply changes</source><translation>Aplicar cambios</translation></message>
    <message><source>Architecture</source><translation>Arquitectura</translation></message>
    <message><source>Automatic</source><translation>Automático</translation></message>
    <message><source>Automatic staged file commits</source><translation>Consolidación automática de archivos preparados</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>Consolidación automática de archivos incompleta</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>Consolidar automáticamente los archivos preparados aptos durante el apagado y reinicio de Windows.</translation></message>
    <message><source>Available</source><translation>Disponible</translation></message>
    <message><source>Back</source><translation>Atrás</translation></message>
    <message><source>Background service</source><translation>Servicio en segundo plano</translation></message>
    <message><source>Binding</source><translation>Vinculación</translation></message>
    <message><source>By drive letter</source><translation>Por letra de unidad</translation></message>
    <message><source>By volume ID</source><translation>Por ID de volumen</translation></message>
    <message><source>Cancel reading</source><translation>Cancelar lectura</translation></message>
    <message><source>Completed operations cannot be undone.</source><translation>Las operaciones completadas no se pueden deshacer.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Seleccione la clave del Registro que desea excluir de la protección UWF, incluidas sus subclaves y valores.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Elija dónde guardar el archivo.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Elija una ubicación dentro de la unidad o carpeta permitida.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Elija otra carpeta o compruebe los permisos de acceso y reinténtelo.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Elija otra clave o compruebe los permisos de acceso y reinténtelo.</translation></message>
    <message><source>Clear filters</source><translation>Borrar filtros</translation></message>
    <message><source>Clear logs</source><translation>Borrar registros</translation></message>
    <message><source>Clear logs?</source><translation>¿Borrar los registros?</translation></message>
    <message><source>Collapse</source><translation>Contraer</translation></message>
    <message><source>Commit changes</source><translation>Consolidar cambios</translation></message>
    <message><source>Commit changes…</source><translation>Consolidar cambios…</translation></message>
    <message><source>Commit file changes?</source><translation>¿Consolidar los cambios del archivo?</translation></message>
    <message><source>Commit folder changes?</source><translation>¿Consolidar los cambios de la carpeta?</translation></message>
    <message><source>Commit registry key changes?</source><translation>¿Consolidar los cambios de la clave del registro?</translation></message>
    <message><source>Commit registry value changes?</source><translation>¿Consolidar los cambios del valor del registro?</translation></message>
    <message><source>Compatibility mode</source><translation>Modo de compatibilidad</translation></message>
    <message><source>Configuration applied</source><translation>Configuración aplicada</translation></message>
    <message><source>Configuration error</source><translation>Error de configuración</translation></message>
    <message><source>Configured</source><translation>Configurado</translation></message>
    <message><source>Confirm before continuing</source><translation>Confirmar antes de continuar</translation></message>
    <message><source>Connected and authenticated</source><translation>Conectado y autenticado</translation></message>
    <message><source>Connecting…</source><translation>Conectando…</translation></message>
    <message><source>Contact author</source><translation>Contactar con el autor</translation></message>
    <message><source>Continue editing</source><translation>Continuar editando</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>Si continúa el reinicio, los cambios de archivos sin consolidar pueden perderse.</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>Si continúa el apagado, los cambios de archivos sin consolidar pueden perderse.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Coordinar con el apagado de Windows</translation></message>
    <message><source>Copy all logs</source><translation>Copiar todos los registros</translation></message>
    <message><source>Copy full report</source><translation>Copiar informe completo</translation></message>
    <message><source>Copy name</source><translation>Copiar nombre</translation></message>
    <message><source>Copy path</source><translation>Copiar ruta</translation></message>
    <message><source>Copy selection</source><translation>Copiar selección</translation></message>
    <message><source>Copy version</source><translation>Copiar versión</translation></message>
    <message><source>Counting staged files…</source><translation>Contando archivos preparados…</translation></message>
    <message><source>Create</source><translation>Crear</translation></message>
    <message><source>Critical %1 MB</source><translation>Crítico %1 MB</translation></message>
    <message><source>Current filter</source><translation>Filtro actual</translation></message>
    <message><source>Current run</source><translation>Ejecución actual</translation></message>
    <message><source>Current selection</source><translation>Selección actual</translation></message>
    <message><source>Current usage</source><translation>Uso actual</translation></message>
    <message><source>Date modified</source><translation>Fecha de modificación</translation></message>
    <message><source>Debug</source><translation>Depuración</translation></message>
    <message><source>Delete and commit this file?</source><translation>¿Eliminar este archivo y consolidar su eliminación?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>¿Eliminar esta carpeta y consolidar su eliminación?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>¿Eliminar esta clave del registro y consolidar su eliminación?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>¿Eliminar este valor del registro y consolidar su eliminación?</translation></message>

    <message><source>Direct restart?</source><translation>¿Reiniciar directamente?</translation></message>
    <message><source>Direct shutdown?</source><translation>¿Apagar directamente?</translation></message>
    <message><source>Discard changes</source><translation>Descartar cambios</translation></message>
    <message><source>Disk</source><translation>Disco</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Persistencia de la clave secreta de dominio</translation></message>
    <message><source>Done</source><translation>Hecho</translation></message>
    <message><source>Edit commands</source><translation>Editar comandos</translation></message>
    <message><source>Editable</source><translation>Editable</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Active el filtro y reinicie para mostrar el uso real</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>El modo avanzado añade un servicio en segundo plano que se inicia con Windows. Elimine el servicio antes de mover o borrar la aplicación.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>El modo mejorado no está disponible. Use «Apagado seguro» o «Reinicio seguro» para consolidar los archivos preparados.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Escriba HKLM\SOFTWARE\… y pulse Intro para navegar</translation></message>
    <message><source>Enter a file name</source><translation>Introducir un nombre de archivo</translation></message>
    <message><source>Enter a valid file name.</source><translation>Introduzca un nombre de archivo válido.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Introduzca un nombre de archivo o carpeta válido.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Introduzca un nombre de carpeta válido.</translation></message>
    <message><source>Error</source><translation>Error</translation></message>
    <message><source>Error details</source><translation>Detalles del error</translation></message>
    <message><source>Error details and suggestions</source><translation>Detalles del error y sugerencias</translation></message>
    <message><source>Expand</source><translation>Expandir</translation></message>
    <message><source>Export list</source><translation>Exportar lista</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>No se pudieron borrar los registros: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>No se pudieron borrar los registros: error inesperado.</translation></message>
    <message><source>File name</source><translation>Nombre de archivo</translation></message>
    <message><source>File or folder</source><translation>Archivo o carpeta</translation></message>
    <message><source>File commits have started and cannot be canceled.</source><translation>La consolidación de archivos ha comenzado y no se puede cancelar.</translation></message>
    <message><source>File commits incomplete</source><translation>Consolidación de archivos incompleta</translation></message>
    <message><source>File type</source><translation>Tipo de archivo</translation></message>
    <message><source>Files and folders</source><translation>Archivos y carpetas</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Se pueden seleccionar archivos y carpetas a la vez.</translation></message>
    <message><source>Filter is disabled</source><translation>El filtro está desactivado</translation></message>
    <message><source>Fit columns to contents</source><translation>Ajustar columnas al contenido</translation></message>
    <message><source>Folder address</source><translation>Dirección de la carpeta</translation></message>
    <message><source>Folder name</source><translation>Nombre de carpeta</translation></message>
    <message><source>Folders</source><translation>Carpetas</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>Agregar esta clave a las exclusiones de la próxima sesión.</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>Agregar esta ruta a las exclusiones de la próxima sesión.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Exclusión global del registro · incluye subclaves</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Suba a la carpeta superior o elija otra ubicación.</translation></message>
    <message><source>Home</source><translation>Carpeta personal</translation></message>
    <message><source>Import configuration</source><translation>Importar configuración</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>La importación actualiza el borrador. Confirme los cambios en «Revisar y aplicar» para escribirlos en el sistema.</translation></message>
    <message><source>Import results</source><translation>Resultados de importación</translation></message>
    <message><source>Imported</source><translation>Importado</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>La lista muestra los resultados de los comandos sobre el borrador de configuración; seleccione una fila para ver el comando completo y los detalles.</translation></message>
    <message><source>Imported: %1</source><translation>Importado: %1</translation></message>
    <message><source>Information</source><translation>Información</translation></message>
    <message><source>Item details</source><translation>Detalles del elemento</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>Consolidar muchos archivos preparados puede llevar más tiempo.</translation></message>
    <message><source>License and copyright</source><translation>Licencia y derechos de autor</translation></message>
    <message><source>Line</source><translation>Línea</translation></message>
    <message><source>Local disk (%1)</source><translation>Disco local (%1)</translation></message>
    <message><source>Log level</source><translation>Nivel de registro</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Error al leer los registros. Reinténtelo o compruebe el acceso al archivo de registro.</translation></message>
    <message><source>Maximize</source><translation>Maximizar</translation></message>
    <message><source>Message details</source><translation>Detalles del mensaje</translation></message>
    <message><source>Minimize</source><translation>Minimizar</translation></message>
    <message><source>Module</source><translation>Módulo</translation></message>
    <message><source>More</source><translation>Más</translation></message>
    <message><source>Move</source><translation>Mover</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Si UWF protege la unidad de la aplicación, asegúrese primero de que sus archivos se conserven tras reiniciar. Consolidar muchos archivos preparados puede alargar el apagado o el reinicio.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Se pueden seleccionar varios archivos.</translation></message>
    <message><source>Name and location</source><translation>Nombre y ubicación</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Valor con nombre: %1 · solo este valor</translation></message>
    <message><source>Navigate to registry path</source><translation>Ir a la ruta del registro</translation></message>
    <message><source>New folder</source><translation>Nueva carpeta</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Los nuevos registros aparecerán al actualizar.</translation></message>
    <message><source>Next-session configuration</source><translation>Configuración de la próxima sesión</translation></message>
    <message><source>No files to commit</source><translation>No hay archivos que consolidar</translation></message>
    <message><source>No logs yet</source><translation>Aún no hay registros</translation></message>
    <message><source>No matching items</source><translation>No hay elementos coincidentes</translation></message>
    <message><source>No matching logs</source><translation>No hay registros coincidentes</translation></message>
    <message><source>No overlay files found</source><translation>No se encontraron archivos de superposición</translation></message>
    <message><source>No registry key selected</source><translation>No hay ninguna clave del registro seleccionada</translation></message>
    <message><source>None</source><translation>Ninguno</translation></message>
    <message><source>Not configured</source><translation>Sin configurar</translation></message>
    <message><source>Not connected</source><translation>Sin conexión</translation></message>
    <message><source>Not installed</source><translation>No instalado</translation></message>
    <message><source>Not written to the system</source><translation>No escrito en el sistema</translation></message>
    <message><source>Off</source><translation>Desactivado</translation></message>
    <message><source>On</source><translation>Activado</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Un comando uwfmgr por línea</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Mantenga la aplicación en una carpeta fija que los usuarios estándar no puedan modificar. Ubicación sugerida:</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>Solo se consolidan automáticamente los archivos preparados. Los demás cambios sin consolidar pueden perderse tras reiniciar.</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>Solo se consolidan automáticamente los archivos preparados. Los demás cambios sin consolidar pueden perderse tras apagar.</translation></message>
    <message><source>Open</source><translation>Abrir</translation></message>
    <message><source>Open folder</source><translation>Abrir carpeta</translation></message>
    <message><source>Operating system</source><translation>Sistema operativo</translation></message>
    <message><source>Operation and security</source><translation>Funcionamiento y seguridad</translation></message>
    <message><source>Original command</source><translation>Comando original</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Se alcanzó el umbral crítico de la superposición. Guarde los cambios y reinicie de forma segura.</translation></message>
    <message><source>Maximum overlay capacity</source><translation>Capacidad máxima de la superposición</translation></message>
    <message><source>Overlay type</source><translation>Tipo de superposición</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Se alcanzó el umbral de advertencia de la superposición. Guarde los cambios y reinicie de forma segura.</translation></message>
    <message><source>Overview</source><translation>Resumen</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Pegue comandos o cárguelos desde un archivo…</translation></message>
    <message><source>Pending changes</source><translation>Cambios pendientes</translation></message>
    <message><source>Pending removal</source><translation>Eliminación pendiente</translation></message>
    <message><source>Permissions and protection</source><translation>Permisos y protección</translation></message>
    <message><source>Privileges</source><translation>Privilegios</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2 elementos procesados</translation></message>
    <message><source>Processing result</source><translation>Resultado del procesamiento</translation></message>
    <message><source>Protected</source><translation>Protegido</translation></message>
    <message><source>Protected volumes</source><translation>Volúmenes protegidos</translation></message>
    <message><source>Protection and overlay</source><translation>Protección y superposición</translation></message>
    <message><source>Quick access</source><translation>Acceso rápido</translation></message>
    <message><source>Raw report</source><translation>Informe sin procesar</translation></message>
    <message><source>Read again</source><translation>Volver a leer</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Vuelva a leer para obtener una instantánea de diagnóstico.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Lea la configuración para mostrar el uso real</translation></message>
    <message><source>Read-only</source><translation>Solo lectura</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Leer una superposición grande puede tardar más. Puede cancelar en cualquier momento.</translation></message>
    <message><source>Reading canceled</source><translation>Lectura cancelada</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Leyendo archivos y subcarpetas de esta ubicación…</translation></message>
    <message><source>Reading folder</source><translation>Leyendo carpeta</translation></message>
    <message><source>Reading logs</source><translation>Leyendo registros</translation></message>
    <message><source>Reading overlay</source><translation>Leyendo superposición</translation></message>
    <message><source>Ready</source><translation>Listo</translation></message>
    <message><source>Files to commit</source><translation>Archivos por consolidar</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Los registros de la ejecución actual aparecerán aquí al terminar la lectura.</translation></message>

    <message><source>Refresh folder</source><translation>Actualizar carpeta</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Clave del registro o ajuste de persistencia</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>Al cancelar, las tareas pendientes se detendrán cuando termine la operación actual.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Quitar esta clave de las exclusiones de la próxima sesión.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Quitar esta ruta de las exclusiones de la próxima sesión.</translation></message>
    <message><source>Removed</source><translation>Eliminado</translation></message>
    <message><source>Rename</source><translation>Cambiar nombre</translation></message>
    <message><source>Rename…</source><translation>Cambiar nombre…</translation></message>
    <message><source>Repair required</source><translation>Requiere reparación</translation></message>
    <message><source>Replace file</source><translation>Reemplazar archivo</translation></message>
    <message><source>Replace the existing file?</source><translation>¿Reemplazar el archivo existente?</translation></message>
    <message><source>Required configuration</source><translation>Configuración necesaria</translation></message>
    <message><source>Restart anyway</source><translation>Reiniciar de todos modos</translation></message>
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>El reinicio continuará tras la consolidación. Mantenga abierta la aplicación.</translation></message>
    <message><source>Restore</source><translation>Restaurar</translation></message>
    <message><source>Retry</source><translation>Reintentar</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Revise los detalles de los fallos antes de continuar el apagado.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Revise los detalles de los fallos antes de reiniciar.</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>Es posible que ya se hayan escrito algunos cambios. Actualice el estado del sistema antes de volver a intentarlo.</translation></message>
    <message><source>Review the result for each target.</source><translation>Revise el resultado de cada destino.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Revise estos cambios antes de escribirlos en el sistema.</translation></message>
    <message><source>Running</source><translation>En ejecución</translation></message>
    <message><source>Save</source><translation>Guardar</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>Consolidar los archivos preparados en el disco y después reiniciar.</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>Consolidar los archivos preparados en el disco y después apagar.</translation></message>

    <message><source>Committing to disk</source><translation>Consolidando en el disco</translation></message>
    <message><source>Committing…</source><translation>Consolidando…</translation></message>
    <message><source>Scanning staged paths</source><translation>Examinando rutas preparadas</translation></message>
    <message><source>Search logs</source><translation>Buscar en registros</translation></message>
    <message><source>Search messages or modules</source><translation>Buscar mensajes o módulos</translation></message>
    <message><source>Search paths</source><translation>Buscar rutas</translation></message>
    <message><source>Search this folder</source><translation>Buscar en esta carpeta</translation></message>
    <message><source>Select a file or enter its name</source><translation>Seleccionar un archivo o introducir su nombre</translation></message>
    <message><source>Select a file to open.</source><translation>Seleccione un archivo para abrir.</translation></message>
    <message><source>Select a folder.</source><translation>Seleccione una carpeta.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Seleccione una clave para operar sobre todo su subárbol, o un valor con nombre para operar solo sobre ese valor.</translation></message>
    <message><source>Select an available subkey</source><translation>Seleccionar una subclave disponible</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Seleccione solo archivos o abra una carpeta para explorar su contenido.</translation></message>
    <message><source>Select files or folders</source><translation>Seleccionar archivos o carpetas</translation></message>
    <message><source>Select folder</source><translation>Seleccionar carpeta</translation></message>
    <message><source>Select folders only.</source><translation>Seleccione solo carpetas.</translation></message>
    <message><source>Select registry key or value</source><translation>Seleccionar clave o valor del registro</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Seleccionar clave o valor del registro para eliminar</translation></message>
    <message><source>Select the current folder</source><translation>Seleccionar la carpeta actual</translation></message>
    <message><source>Select this file</source><translation>Seleccionar este archivo</translation></message>
    <message><source>Select this registry key</source><translation>Seleccionar esta clave del registro</translation></message>
    <message><source>Select this registry value</source><translation>Seleccionar este valor del registro</translation></message>
    <message><source>Selected file</source><translation>Archivo seleccionado</translation></message>
    <message><source>Selected folder</source><translation>Carpeta seleccionada</translation></message>
    <message><source>Selection</source><translation>Selección</translation></message>
    <message><source>Service account</source><translation>Cuenta del servicio</translation></message>
    <message><source>Service configuration needs repair</source><translation>Es necesario reparar la configuración del servicio</translation></message>
    <message><source>Service details</source><translation>Detalles del servicio</translation></message>
    <message><source>Service name</source><translation>Nombre del servicio</translation></message>
    <message><source>Service status</source><translation>Estado del servicio</translation></message>
    <message><source>Service stopped</source><translation>Servicio detenido</translation></message>
    <message><source>Shut down anyway</source><translation>Apagar de todos modos</translation></message>
    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>El apagado continuará tras la consolidación. Mantenga abierta la aplicación.</translation></message>
    <message><source>Size</source><translation>Tamaño</translation></message>
    <message><source>Some changes could not be applied</source><translation>No se pudieron aplicar algunos cambios</translation></message>
    <message><source>Some files could not be committed</source><translation>No se pudieron consolidar algunos archivos</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>No se pudo leer parte de la información del hardware. La demás información de diagnóstico se puede seguir consultando y copiando.</translation></message>
    <message><source>Source code</source><translation>Código fuente</translation></message>

    <message><source>Staged path</source><translation>Ruta preparada</translation></message>
    <message><source>Standard user</source><translation>Usuario estándar</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Se inicia automáticamente con Windows</translation></message>
    <message><source>Startup type</source><translation>Tipo de inicio</translation></message>
    <message><source>Status unavailable</source><translation>Estado no disponible</translation></message>
    <message><source>Stopped</source><translation>Detenido</translation></message>
    <message><source>Storage</source><translation>Almacenamiento</translation></message>
    <message><source>System</source><translation>Sistema</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Cambios del sistema al activar UWF por primera vez</translation></message>
    <message><source>System memory</source><translation>Memoria del sistema</translation></message>
    <message><source>System metadata</source><translation>Metadatos del sistema</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Los metadatos del sistema son solo para diagnóstico y no se pueden consolidar.</translation></message>
    <message><source>Takes effect after restart</source><translation>Surte efecto tras reiniciar</translation></message>
    <message><source>Takes effect when applied</source><translation>Entra en vigor al aplicar</translation></message>
    <message><source>Used when UWF starts</source><translation>Se usa al iniciar UWF</translation></message>
    <message><source>Mixed activation times</source><translation>Activación en distintos momentos</translation></message>
    <message><source>Target</source><translation>Destino</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistencia de licencias de Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>El proveedor UWF no pudo completar la enumeración. Puede ocurrir con una superposición grande o recursos del sistema insuficientes.</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>La configuración se ha escrito en el sistema.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Los registros actuales se borrarán y no se podrán recuperar.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>La carpeta de destino no existe.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>No se pudo leer el estado del servicio del modo avanzado. Compruebe los permisos del servicio y reinténtelo.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>La lista puede omitir archivos o incluir archivos ya consolidados. No la use como lista de consolidación.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>El elemento seleccionado ya no existe: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>El servicio está instalado, pero detenido. Inícielo para reanudar la consolidación automática de archivos.</translation></message>
    <message><source>This PC</source><translation>Este equipo</translation></message>
    <message><source>This folder is empty</source><translation>Esta carpeta está vacía</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Este informe puede contener nombres de cuenta, identificadores de dispositivo y rutas de archivo. Revíselo antes de compartirlo.</translation></message>

    <message><source>Try another keyword or log level.</source><translation>Pruebe otra palabra clave o nivel de registro.</translation></message>
    <message><source>Try another search term.</source><translation>Pruebe otro término de búsqueda.</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>Desactive UWF y reinicie antes de cambiar el tipo o la capacidad máxima de la superposición.</translation></message>
    <message><source>UI agent</source><translation>Agente de interfaz</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 líneas</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>No se pudo leer la configuración UWF. Compruebe que Unified Write Filter esté instalado en Windows y vuelva a leer.</translation></message>
    <message><source>UWF feature</source><translation>Característica UWF</translation></message>
    <message><source>UWF filter</source><translation>Filtro UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF no está disponible actualmente</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Se ejecutará directamente el reinicio seguro de UWF.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>Se ejecutará directamente el apagado seguro de UWF.</translation></message>
    <message><source>Unable to access this folder</source><translation>No se puede acceder a esta carpeta</translation></message>
    <message><source>Unable to access this registry key</source><translation>No se puede acceder a esta clave del registro</translation></message>
    <message><source>Unable to read logs</source><translation>No se pueden leer los registros</translation></message>
    <message><source>Unable to read service status</source><translation>No se puede leer el estado del servicio</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>No se puede leer la superposición ahora</translation></message>
    <message><source>Unable to read this folder.</source><translation>No se puede leer esta carpeta.</translation></message>
    <message><source>Unable to select this item</source><translation>No se puede seleccionar este elemento</translation></message>
    <message><source>Unavailable</source><translation>No disponible</translation></message>
    <message><source>Undo removal</source><translation>Deshacer eliminación</translation></message>
    <message><source>Unprotected</source><translation>Sin protección</translation></message>
    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>Algunos elementos no se completaron o no pudieron verificarse. Seleccione un elemento para ver su ruta completa y los detalles.</translation></message>
    <message><source>Up one folder</source><translation>Subir una carpeta</translation></message>
    <message><source>Usage unavailable</source><translation>Uso no disponible</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Usar la clave completa, incluidas todas las subclaves</translation></message>
    <message><source>Version %1</source><translation>Versión %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Ver registros de la aplicación e información de diagnóstico.</translation></message>
    <message><source>View commands for these changes</source><translation>Ver comandos de estos cambios</translation></message>
    <message><source>View failure details</source><translation>Ver detalles de los fallos</translation></message>
    <message><source>View overlay</source><translation>Ver superposición</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Ver la instantánea de diagnóstico de archivos de %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Vinculación del volumen %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Protección del volumen %1</translation></message>
    <message><source>Volume identity</source><translation>Identidad del volumen</translation></message>
    <message><source>Warning</source><translation>Advertencia</translation></message>
    <message><source>Warning %1 MB</source><translation>Advertencia %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Clave completa y todas las subclaves · recursivo</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF proporciona la protección contra escritura. Esta aplicación la configura y administra.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>Sin el modo mejorado, use «Apagado seguro» o «Reinicio seguro» en esta aplicación para consolidar los archivos preparados.</translation></message>
    <message><source>Write protection</source><translation>Protección contra escritura</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>¿Escribir estos cambios de configuración en el sistema?</translation></message>
    <message><source>staged files</source><translation>archivos preparados</translation></message>
    <message><source>uwfmgr commands</source><translation>comandos uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Marcar las rutas seleccionadas para eliminarlas. Se hará efectivo al aplicar.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Quitar de la lista de archivos preparados después de aplicar.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Añadir a la lista de archivos preparados después de aplicar.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Al aplicar este plan se vacía la lista guardada de archivos preparados y se elimina su exclusión del Registro cuando ya no sea necesaria.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Los cambios de Archivos preparados deben aplicarse en UWF Manager y no pueden exportarse como comandos uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>La lista de archivos preparados se guarda después de aplicar</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Alcance de la instantánea de la superposición</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Esta lista es solo de diagnóstico. No permite determinar qué archivos deben consolidarse.</translation></message>
    <message><source>NTFS volumes only</source><translation>Solo volúmenes NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>La lectura de superposiciones grandes puede ser lenta o fallar por falta de recursos.</translation></message>
    <message><source>Small files may be missing</source><translation>Pueden faltar archivos pequeños</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Los archivos menores que el tamaño del clúster del disco (normalmente 4 KB) pueden no aparecer en la lista.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>No indica consolidaciones pendientes</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>También pueden aparecer archivos ya consolidados, rutas excluidas y elementos afectados por otras operaciones.</translation></message>

    <message><source>Command processing result</source><translation>Resultado del procesamiento del comando</translation></message>
    <message><source>Copy command</source><translation>Copiar comando</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · No se pudieron leer %1 ramas</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>No se pudieron leer %1 ramas. Continúe con el contenido encontrado o reintente solo los elementos fallidos.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 archivos · %2 carpetas</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 valores · %2 claves</translation></message>
    <message><source>Attempts</source><translation>Intentos</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Cancelar tareas restantes</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Consolidar solo el contenido recorrido correctamente. Las ramas sin leer permanecen sin cambios.</translation></message>
    <message><source>Committing changes</source><translation>Consolidando cambios</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Las eliminaciones y consolidaciones completadas no se pueden deshacer.</translation></message>
    <message><source>Continue…</source><translation>Continuar…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Eliminar y consolidar solo los destinos recorridos correctamente; se conservan las ramas que no se pudieron leer y sus carpetas o claves superiores.</translation></message>
    <message><source>Deleting and committing</source><translation>Eliminando y consolidando</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>La eliminación y la consolidación no se pueden deshacer. Solo se procesa el ámbito seleccionado; no se aplica la configuración UWF pendiente.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Determinando el ámbito. Se registran las ramas ilegibles y el recorrido continúa en el resto.</translation></message>
    <message><source>Export traversal records</source><translation>Exportar registro del recorrido</translation></message>
    <message><source>No actionable content</source><translation>No hay contenido procesable</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>No se encontraron destinos procesables. No se ha eliminado ni consolidado nada.</translation></message>
    <message><source>No changes have been committed.</source><translation>No se han consolidado cambios</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>No se ha eliminado ni consolidado nada</translation></message>
    <message><source>Path</source><translation>Ruta</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>Eliminar permanentemente el contenido seleccionado y consolidar las eliminaciones.</translation></message>
    <message><source>Preparation complete</source><translation>Preparación completada</translation></message>
    <message><source>Preparing commit</source><translation>Preparando la consolidación</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Preparando la eliminación y consolidación</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Procesar solo el contenido recorrido correctamente</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Se omiten los puntos de reanálisis para mantener el recorrido en el ámbito seleccionado.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Un reinicio no deshace una consolidación. Solo se procesa el ámbito seleccionado; no se aplica la configuración UWF pendiente.</translation></message>
    <message><source>Retry failed items</source><translation>Reintentar los fallidos</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>Consolidar los cambios del ámbito seleccionado.</translation></message>
    <message><source>Stopping the operation</source><translation>Deteniendo la operación</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Se conservan los registros de éxitos y errores</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>La carpeta no está vacía; se ha conservado el contenido sin procesar.</translation></message>
    <message><source>The operation could not be completed</source><translation>No se pudo completar la operación</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>La clave del Registro tiene valores nuevos; se ha conservado el contenido sin procesar.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>La clave del Registro aún tiene subclaves; se ha conservado el contenido sin procesar.</translation></message>
    <message><source>The target no longer exists.</source><translation>El destino ya no existe.</translation></message>
    <message><source>Traversal complete</source><translation>Recorrido completado</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Recorrido finalizado con contenido sin leer</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Se conserva el registro del recorrido y se puede exportar.</translation></message>
    <message><source>Traverse again</source><translation>Recorrer de nuevo</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Recorra de nuevo o cierre esta ventana.</translation></message>
    <message><source>Traversing the folder</source><translation>Recorriendo la carpeta</translation></message>
    <message><source>Traversing the registry</source><translation>Recorriendo el Registro</translation></message>
    <message><source>Unable to prepare the operation</source><translation>No se puede preparar la operación</translation></message>
    <message><source>View results</source><translation>Ver resultados</translation></message>

    <message><source>All operations succeeded</source><translation>Todas las operaciones se completaron correctamente</translation></message>
    <message><source>Some operations succeeded</source><translation>Algunas operaciones se completaron correctamente</translation></message>
    <message><source>No operations succeeded</source><translation>Ninguna operación se completó correctamente</translation></message>
    <message><source>No targets to process</source><translation>No hay destinos que procesar</translation></message>
    <message><source>Commit interrupted</source><translation>Consolidación interrumpida</translation></message>
    <message><source>%1 items were not processed.</source><translation>No se procesaron %1 elementos.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>Han fallado %1 operaciones del Registro.</translation></message>
    <message><source>The folder does not exist.</source><translation>La carpeta no existe.</translation></message>
    <message><source>Access denied.</source><translation>Acceso denegado.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Ya existe un elemento con este nombre.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>No se pudo crear la carpeta. Compruebe el nombre y los permisos.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>No se pudo cambiar el nombre del elemento. Compruebe el nombre y los permisos.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Este explorador no permite cambiar nombres.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>El borrador de archivos preparados cambió después de abrir este plan. Revise un nuevo plan.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>La lista guardada de archivos preparados cambió después de abrir este plan. Actualice y revise los cambios.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>La lista guardada de archivos preparados no coincide con el plan aceptado.</translation></message>
    <message><source>Export interface settings</source><translation>Exportar ajustes de la interfaz</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>Guardar el tema, el idioma y la visibilidad actuales del widget de superposición en UWF.ini junto a esta aplicación.</translation></message>
    <message><source>Interface settings saved</source><translation>Ajustes de la interfaz guardados</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>La configuración actual de la interfaz se ha guardado en:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini ya existe. El archivo existente no se ha modificado:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>No se pudo guardar UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Escritorio</translation></message>
    <message><source>Downloads</source><translation>Descargas</translation></message>
    <message><source>Documents</source><translation>Documentos</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>La consolidación de archivos preparados se detuvo. Resuelva el error antes de volver a intentarlo.</translation></message>
    <message><source>No net change for this setting</source><translation>El valor final de este ajuste no ha cambiado tras la importación</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Valor final del borrador: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Valor final del borrador: %1 → %2 (valor solicitado: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Valor solicitado: %1 → %2; sustituido por un comando posterior. Valor final del borrador → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Se actualizó el borrador de %1</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Se actualizó el borrador de la protección del volumen %1 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Seleccione las entradas que desea quitar.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Las entradas seleccionadas no se pueden restaurar en este momento.</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>Paso directo por espacio libre</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>Superposición persistente</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>Borrar datos persistentes en el próximo inicio</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>Modo de medios de solo lectura</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>Utiliza espacio libre del disco para las escrituras y reduce el uso de la superposición reservada; admite RAM (no recomendado) y Disk (recomendado).</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>Conserva las escrituras de la superposición Disk tras los reinicios; no está disponible para superposiciones RAM.</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>Restablece la superposición persistente: borra sus escrituras en el próximo inicio y restaura el estado de cuando se habilitó la superposición</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>Se requieren privilegios de administrador; la configuración de la superposición es de solo lectura.</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>La edición no está disponible temporalmente. Espere a que termine la operación actual.</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>No se pudo leer la configuración de la superposición. Actualice antes de editar.</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>Este sistema no admite borrar datos persistentes en el próximo inicio.</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>Se desconoce la solicitud de borrado actual. Actualice la configuración antes de editar.</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>Para cambiar esta solicitud, UWF y la superposición persistente Disk deben estar activos en la sesión actual.</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>El borrado en el próximo inicio requiere conservar la superposición persistente Disk. Cancele el borrado o conserve la superposición persistente.</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>La superposición persistente requiere Disk. Desactívela o seleccione Disk.</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>El modo de medios de solo lectura requiere RAM. Desactívelo o seleccione RAM.</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>UWF está habilitado en la sesión actual. Deshabilite UWF y reinicie antes de cambiar esta configuración.</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>La configuración ha cambiado. Cierre y vuelva a abrir este diálogo antes de editar.</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>Configuración avanzada de la superposición</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>Superposición actual</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>Próxima superposición</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>El borrador contiene opciones de superposición incompatibles o no disponibles. Vuelva a la edición y corrija la configuración.</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>Hay un borrado programado para el próximo inicio. Cancele la solicitud antes de desactivar la superposición persistente.</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>La próxima sesión no conservará la superposición persistente Disk. Consérvela antes de solicitar un borrado.</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>La superposición persistente solo está disponible para Disk; la próxima superposición es RAM.</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>El modo de medios de solo lectura requiere RAM; la próxima superposición es Disk.</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>No se pudo confirmar el estado actual de la superposición persistente. Lea de nuevo la configuración.</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>Tras aplicar, tendrá efecto en el próximo inicio.</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>Borrado programado</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>Sin borrado programado</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>Borra una sola vez en el próximo inicio y después conserva los datos. La sesión actual no se borra inmediatamente.</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>Cancela el borrado programado en el próximo inicio y conserva los datos de la superposición persistente.</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>Restablecer superposición persistente · Solicitud única</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Procesando…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>Espere a que finalice la operación.</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · Próxima sesión</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>Volver a la configuración</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>Requisitos para cambiar</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>Modificado</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>Comprobar conflictos de configuración</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>Conflicto de configuración</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>Requisitos para desactivar</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>UWF y la superposición persistente Disk deben estar activos en la sesión actual. Para borrar los datos, también se debe conservar la superposición persistente Disk en la siguiente sesión.</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>Requisitos para activar</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Función</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>Descripción de la función</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>Acerca de %1</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>Resuelva estos conflictos de configuración antes de conservar el borrador.</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>Revertir</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>Cuándo entra en vigor</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>Esta opción no se puede cambiar en este momento</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>UWF debe estar desactivado en la sesión actual y la superposición de la próxima sesión debe usar Disk.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>UWF debe estar desactivado en la sesión actual y la superposición de la próxima sesión debe usar RAM.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>UWF debe estar desactivado en la sesión actual. Cancele cualquier borrado programado de datos persistentes antes de desactivar la superposición persistente.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>UWF debe estar desactivado en la sesión actual. Desactivarlo solo para la próxima sesión sigue requiriendo reiniciar antes de editar.</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>Requisitos de uso</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Ver detalles</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>Solo consulta</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>Por qué no se puede cambiar %1</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>UWF está habilitado en la sesión actual, por lo que no se pueden cambiar estas configuraciones de superposición. Revierta los cambios o deshabilite UWF y reinicie.</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>Ver requisitos para cambiar</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>No se ha escrito ningún cambio de configuración. Puede volver a intentarlo.</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>No era necesario cambiar la configuración.</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>Use «Persistencia de la clave secreta de dominio» para el secreto de la cuenta de equipo del dominio ($MACHINE.ACC).</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>Esta clave no se puede excluir. Seleccione una de sus subclaves.</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>Las exclusiones del Registro deben especificar una subclave no vacía de HKEY_LOCAL_MACHINE (HKLM).</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>Este script solo incluye ajustes que pueden expresarse mediante comandos uwfmgr. No incluye:</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>Vinculación de volúmenes</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>Ajustes sin una dirección fiable por letra de unidad</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>Script de comandos</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>La configuración coincide con el objetivo de la próxima sesión. Puede ver o exportar los ajustes compatibles en «Script de comandos».</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>Ajustes con caracteres de control en rutas o nombres</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>Comandos que dependen de ajustes omitidos</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>Las exclusiones de UWF impiden consolidar este destino de forma fiable. Quite la exclusión en conflicto y reinicie antes de consolidarlo.</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>Las exclusiones de UWF impiden consolidar de forma fiable la lista de archivos preparados. Quite la exclusión en conflicto y reinicie antes de aplicar este cambio.</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF no puede garantizar que la operación mantenga esta clave como destino exacto del Registro.</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>Confirme que la lista guardada de archivos preparados está vacía antes de quitar su exclusión del Registro.</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>Pendiente de confirmación</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Vacía</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>El volumen del sistema no está protegido actualmente por UWF; no hay cambios del Registro que consolidar.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>Los cambios del Registro ya son persistentes; no es necesario consolidarlos mediante UWF.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>Los cambios del Registro ya son persistentes; UWF no puede realizar esta eliminación.</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>Las rutas de exclusión de UWF deben usar barras invertidas.</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>Las rutas de archivo no pueden contener caracteres nulos.</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>Active la protección del volumen en este borrador antes de añadir exclusiones de archivos.</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>La ruta de exclusión contiene caracteres no admitidos por UWF.</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>La ruta de exclusión supera el límite de longitud de UWF.</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>No se pueden confirmar cambios de archivos y del Registro mientras HORM está activo.</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>No se puede escribir en el Registro mientras HORM está activo.</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>Las escrituras en el Registro no pueden guardarse de forma permanente mientras el modo de medios de solo lectura está activo.</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>No se pueden confirmar cambios de archivos y del Registro mientras el modo de medios de solo lectura está activo.</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>Intercepta las escrituras en medios físicos, incluidos los metadatos. Las exclusiones no eluden la protección. No se pueden confirmar cambios individuales de archivos y del Registro; Full Commit es una operación global independiente.</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>No se puede cambiar la configuración de UWF mientras HORM está activo.</translation></message>
    <message><source>Default value · this value only</source><translation>Valor predeterminado · solo este valor</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>Los cambios de los archivos ya son persistentes; no es necesario confirmarlos con UWF.</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>Última observación correcta del uso de la superposición: %1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>El almacenamiento de superposición observado está fuera del intervalo de tamaño o tipo que se puede escribir</translation></message>
    <message><source>Overlay (last observed)</source><translation>Superposición (última observación)</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>No se pudo actualizar el uso de la superposición.
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>No se pudo actualizar el uso de la superposición. Se muestra la última observación correcta de %1.
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>No se pudo asociar la ruta a un volumen local.</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF: activado (última observación)</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>Los cambios de los archivos ya son persistentes; UWF no puede realizar esta eliminación.</translation></message>
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>%1 GB disponibles</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>Se requieren permisos de administrador.</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>Hay una operación en curso. Espere.</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>Intenta guardar todos los elementos añadidos, modificados y eliminados en cada volumen protegido desde este inicio. Esto incluye archivos y ajustes del sistema; no se pueden seleccionar solo algunos cambios.</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>El guardado automático no está disponible en el modo actual</translation></message>
    <message><source>Cannot be undone once started</source><translation>No se puede deshacer una vez iniciado</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>Elegir dónde almacenar la superposición Disk</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>Elija dónde almacenar la superposición Disk.</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>Los cambios de configuración no están disponibles temporalmente.</translation></message>
    <message><source>Configured, inactive this session</source><translation>Configurado, inactivo en esta sesión</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>Configurado; se aplicará después de reiniciar</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>Configurado; se aplicará después de reiniciar. La superposición de este inicio no se moverá a la nueva ubicación.</translation></message>
    <message><source>Configure…</source><translation>Configurar…</translation></message>
    <message><source>Current UWF</source><translation>UWF en la sesión actual</translation></message>
    <message><source>Currently configured storage volume</source><translation>Volumen de almacenamiento configurado actualmente</translation></message>
    <message><source>Default behavior</source><translation>Comportamiento predeterminado</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>Desactive HORM y reinicie antes de consolidar el volumen completo.</translation></message>
    <message><source>Disabled · restart required</source><translation>Desactivado · es necesario reiniciar</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>Desactivar HORM no desactiva UWF ni el modo de medios de solo lectura, ni guarda los cambios de los discos protegidos. Reinicie antes de consultar las acciones disponibles.</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>Active UWF y reinicie antes de activar HORM.</translation></message>

    <message><source>Full volume commit</source><translation>Consolidar volumen completo</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>Modo de consolidación del volumen completo (activo hasta el reinicio)</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>El modo de consolidación del volumen completo ya está activado. Reinicie antes de volver a usarlo.</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>La consolidación del volumen completo ha comenzado. Reinicie antes de activar HORM.</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>La consolidación del volumen completo solo está disponible cuando el modo de medios de solo lectura está activo.</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>El modo de consolidación del volumen completo permanece activado hasta el reinicio. Los cambios nuevos se guardan directamente. Las consolidaciones individuales siguen sin estar disponibles.</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>La consolidación del volumen completo no está disponible en este sistema.</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM está activo. Los ajustes de UWF y las consolidaciones individuales no están disponibles. Desactive HORM y reinicie antes de hacer cambios.</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM está desactivado. Reinicie el equipo antes de cambiar los ajustes de UWF.</translation></message>
    <message><source>HORM is not available on this system.</source><translation>HORM no está disponible en este sistema.</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM requiere que la configuración actual sea RAM con medios de solo lectura.</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM requiere que la superposición actual sea RAM.</translation></message>

    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>En el modo Disk, la implementación del sistema puede activar esta opción de forma predeterminada; compruebe el estado actual que se muestra en los ajustes.</translation></message>
    <message><source>Inactive this session</source><translation>Inactivo en esta sesión</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>Los cambios posteriores también se guardarán hasta que reinicie</translation></message>

    <message><source>Maintenance and recovery</source><translation>Mantenimiento y recuperación</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>Antes de continuar, compruebe que este es el estado que desea conservar.</translation></message>

    <message><source>Next startup: %1</source><translation>Próximo inicio: %1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>No hay ningún volumen de almacenamiento adecuado disponible. Actualice la lista de volúmenes.</translation></message>
    <message><source>Not enabled</source><translation>Sin activar</translation></message>
    <message><source>Not selected</source><translation>Sin seleccionar</translation></message>
    <message><source>Not supported</source><translation>No compatible</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>Una vez iniciada la operación, los cambios nuevos se escriben directamente en el disco hasta que reinicie el equipo.</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>Solo se puede usar un volumen de almacenamiento a la vez. Al elegir uno nuevo, se sustituye la ubicación anterior y se establece Disk para la próxima superposición.</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>Capacidad de la superposición (conservar el ajuste actual)</translation></message>
    <message><source>Overlay configuration</source><translation>Configuración de la superposición</translation></message>

    <message><source>Processing…</source><translation>Procesando…</translation></message>
    <message><source>Read state again</source><translation>Volver a leer el estado</translation></message>
    <message><source>Read the current UWF state first.</source><translation>Lea primero el estado actual de UWF.</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>El modo de medios de solo lectura no admite consolidaciones individuales ni el guardado automático de archivos preparados.</translation></message>
    <message><source>Reading state…</source><translation>Leyendo el estado…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>Reinicie y lea el estado actual antes de volver a activar HORM.</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>Restaurar el mismo escritorio y los mismos programas en ejecución en cada inicio.</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>Consulte las operaciones de consolidación disponibles y sus requisitos.</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>Guardar los cambios en todos los volúmenes protegidos</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>Guardar los cambios en todos los volúmenes protegidos. Revise las consecuencias antes de continuar.</translation></message>


    <message><source>State unknown</source><translation>Estado desconocido</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>Los cambios guardados correctamente ya no se pueden deshacer reiniciando.</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>Los cambios guardados correctamente se conservarán</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>Se desconoce el estado de HORM. Vuelva a leer el estado antes de continuar.</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>Se desconoce el estado de HORM. Actualice el estado antes de continuar.</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>Se desconoce el estado de consolidación o de HORM. Actualice el estado antes de continuar.</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>No se ha podido leer el estado actual de UWF. Actualice el estado antes de continuar.</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>La superposición actual usa RAM con el modo de medios de solo lectura</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>No se ha podido leer el modo de protección actual. Actualice el estado.</translation></message>
    <message><source>The result could not be confirmed</source><translation>No se ha podido confirmar el resultado</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>Esta exclusión está configurada, pero no omite la protección en la sesión actual.</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>Esto también puede cambiar el paso directo por espacio libre y la superposición persistente. Revise los ajustes avanzados antes de reiniciar.</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>La protección UWF está activa en la sesión actual</translation></message>
    <message><source>%1 (will be replaced)</source><translation>%1 (se sustituirá)</translation></message>
    <message><source>Add full volume commit to the draft?</source><translation>¿Añadir la consolidación del volumen completo al borrador?</translation></message>
    <message><source>Add to draft</source><translation>Añadir al borrador</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>Se aplica a todos los volúmenes protegidos; solo comienza cuando se aplica este borrador.</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>Aplicar este borrador inicia la consolidación del volumen completo. Los cambios guardados no se pueden deshacer reiniciando y los cambios nuevos se guardan hasta el reinicio.</translation></message>
    <message><source>Configure storage location</source><translation>Configurar ubicación de almacenamiento</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>La configuración de la ubicación de almacenamiento no está disponible en este sistema.</translation></message>
    <message><source>Current: %1</source><translation>Actual: %1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>Desactive HORM y reinicie antes de configurar la ubicación de almacenamiento.</translation></message>
    <message><source>Disable saved-state startup?</source><translation>¿Desactivar el inicio desde el estado guardado?</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>Borrador: %1 · pendiente de aplicar</translation></message>
    <message><source>Enable saved-state startup?</source><translation>¿Activar el inicio desde el estado guardado?</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>Consolidar volumen completo (pendiente de aplicar)</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>La consolidación del volumen completo está pendiente. Solo comenzará después de confirmar en «Revisar y aplicar».</translation></message>
    <message><source>Hibernate once / resume many</source><translation>Hibernar una vez / reanudar muchas veces</translation></message>
    <message><source>Not started</source><translation>Sin iniciar</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>Quitar la consolidación del volumen completo de los cambios pendientes.</translation></message>
    <message><source>Save draft</source><translation>Guardar borrador</translation></message>

    <message><source>Will start when applied</source><translation>Comenzará al aplicar</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>Un disco fijo montado no ha superado la comprobación de protección. Revise los discos de la lista antes de activar HORM.</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>Comprueba los discos fijos aptos para UWF que siguen montados. Las particiones desmontadas pueden omitirse.</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>No se han podido verificar los requisitos de HORM. Vuelva a leer el estado antes de continuar.</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>Comprobación de protección de discos fijos montados</translation></message>
    <message><source>Not using a Disk overlay</source><translation>No se está usando una superposición Disk</translation></message>
    <message><source>Selected storage volume</source><translation>Volumen de almacenamiento seleccionado</translation></message>
    <message><source>System drive default (%1)</source><translation>Predeterminado: unidad del sistema (%1)</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>El estado de protección ha cambiado durante este inicio. Reinicie antes de activar HORM.</translation></message>
    <message><source>Unable to read</source><translation>No se puede leer</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>Elija el disco donde almacenar los cambios temporales. Se aplicará en el próximo inicio.</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>Desactive UWF y reinicie antes de configurar la ubicación de almacenamiento.</translation></message>
    <message><source>Met</source><translation>Cumplido</translation></message>
    <message><source>Not met</source><translation>No cumplido</translation></message>
    <message><source>Partition</source><translation>Partición</translation></message>


    <message><source>Protect all</source><translation>Proteger todos</translation></message>
    <message><source>Hibernation is not supported</source><translation>La hibernación no es compatible</translation></message>
    <message><source>Hibernation is not enabled</source><translation>La hibernación no está activada</translation></message>
    <message><source>Fast startup only</source><translation>Solo inicio rápido</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>La protección de volúmenes por lotes no está disponible en este sistema.</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>Desactive HORM y reinicie antes de cambiar la protección de los volúmenes.</translation></message>
    <message><source>Setup and operation</source><translation>Configuración y funcionamiento</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. Preparar la configuración</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>Con UWF desactivado en la sesión actual, configure la superposición RAM, el modo de medios de solo lectura, la protección de los volúmenes y la activación de UWF para el próximo inicio. Aplique el borrador. Prepare la hibernación S4 con un archivo de hibernación completo y presente.</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>Reinicie y compruebe en el estado actual que UWF esté activado, que se usen una superposición RAM y el modo de medios de solo lectura, y que todos los volúmenes fijos montados aptos para UWF estén protegidos. Compruebe que la hibernación del sistema esté disponible.</translation></message>
    <message><source>2. Restart and verify</source><translation>2. Reiniciar y comprobar</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. Activar HORM e hibernar</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>Añada «Activar HORM» al borrador y use «Revisar y aplicar». Al aplicar el borrador se activa HORM, lo que equivale a:</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>Tras activar HORM correctamente, prepare el escritorio y las aplicaciones y, después, hiberne una vez para guardar el estado inicial:</translation></message>
    <message><source>HORM usage</source><translation>Uso de HORM</translation></message>
    <message><source>Volume protection</source><translation>Protección de volúmenes</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>Configure la protección de todos los volúmenes para la próxima sesión.</translation></message>
    <message><source>Unprotect all</source><translation>Desproteger todos</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>Próximo inicio: todos los volúmenes protegidos</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>Próximo inicio: ningún volumen protegido</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>Próximo inicio: configuración individual de volúmenes</translation></message>
    <message><source>System hibernation is available</source><translation>La hibernación del sistema está disponible</translation></message>
    <message><source>Usage guide</source><translation>Guía de uso</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>La protección de todos los volúmenes está pendiente de aplicar</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>La protección de los volúmenes surte efecto después de reiniciar</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>No se pueden aplicar a la vez la desprotección de todos los volúmenes y la activación de HORM.</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>No se admite la protección de volúmenes por lotes.</translation></message>
    <message><source>Applied</source><translation>Aplicado</translation></message>
    <message><source>Not applied</source><translation>No aplicado</translation></message>
    <message><source>Could not confirm</source><translation>No se pudo confirmar</translation></message>



    <message><source>Storage volume:</source><translation>Volumen de almacenamiento:</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>Las exclusiones no omiten la protección durante esta sesión de inicio. No están disponibles el guardado individual ni el guardado automático de archivos preparados. El guardado completo de volúmenes afecta a todos los volúmenes protegidos; revise su descripción antes de continuar.</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>Reanudar repetidamente el mismo estado de hibernación</translation></message>
    <message><source>Remove full volume commit draft</source><translation>Quitar el borrador de consolidación del volumen completo</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>El modo de medios de solo lectura está activo</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>No se pudo leer un marcador de almacenamiento configurado.</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>Los archivos preparados entran en conflicto con una exclusión de archivos:
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>La hibernación completa no está disponible en este equipo</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>Más de un volumen de almacenamiento tiene un marcador distinto de cero.</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>No se pudo confirmar que la superposición sea de tipo Disco.</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>No se pudo leer la ubicación de almacenamiento configurada.</translation></message>
    <message><source>The system drive could not be identified.</source><translation>No se pudo identificar la unidad del sistema.</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>El reinicio directo omite la consolidación de los archivos preparados.</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>El apagado directo omite la consolidación de los archivos preparados.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>Los archivos preparados no se consolidarán. Los cambios sin consolidar pueden perderse tras reiniciar.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>Los archivos preparados no se consolidarán. Los cambios sin consolidar pueden perderse tras apagar.</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>Consolida los archivos preparados antes de un apagado o reinicio normal.</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>La consolidación del volumen completo y la activación de HORM no se pueden aplicar juntas.</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>La configuración de la ubicación de almacenamiento y la activación de HORM no se pueden aplicar juntas.</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>La configuración de la ubicación de almacenamiento y la consolidación del volumen completo requieren distintos modos actuales de UWF.</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>Elija un volumen de almacenamiento antes de aplicar los cambios.</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>La configuración de una ubicación de almacenamiento en disco entra en conflicto con la superposición RAM solicitada.</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>Aplique el cambio del filtro y reinicie antes de solicitar la consolidación del volumen completo.</translation></message>
    <message><source>Canceled</source><translation>Cancelado</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>El volumen de almacenamiento revisado ya no está disponible. Vuelva a editar y elija otro volumen.</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>Este volumen no se puede registrar en UWF porque su bus de almacenamiento no es compatible.</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>No se pudo confirmar la compatibilidad del volumen físico. Actualice el estado antes de activar la protección.</translation></message>
    <message><source>Paused</source><translation>En pausa</translation></message>
    <message><source>Partially paused</source><translation>Parcialmente en pausa</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>Los datos de la superposición persistente se conservarán en el próximo inicio. Las confirmaciones automáticas están en pausa.</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>Entradas en pausa: %1 · Archivos en pausa: %2</translation></message>
    <message><source>Read-only ROM</source><translation>ROM de solo lectura</translation></message>
    <message><source>%1 changes · %2 commands</source><translation>%1 cambios · %2 comandos</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>Sistema de archivos %1: no admite exclusiones de archivos ni confirmaciones por archivo.</translation></message>
    <message><source>Applying changes</source><translation>Aplicando cambios</translation></message>
    <message><source>Automatic commits paused</source><translation>Confirmaciones automáticas en pausa</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>Las confirmaciones automáticas de archivos se pausan hasta reiniciar tras un cambio del modo HORM.</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>Las confirmaciones automáticas de archivos no están disponibles después de FullCommit.</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>Las confirmaciones automáticas de archivos no están disponibles mientras HORM está activo.</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>Las confirmaciones automáticas de archivos preparados se pausan para este reinicio.</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>Las confirmaciones automáticas de archivos preparados se pausan para este apagado.</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>No se pudo confirmar la persistencia de la configuración. Actualice el estado del sistema antes de cambiar este ajuste.</translation></message>
    <message><source>Elapsed: %1 s</source><translation>Tiempo transcurrido: %1 s</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>Solo se confirmarán los archivos preparados que cumplan los requisitos.</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>El modo ROM de solo lectura no puede guardar de forma persistente los ajustes habituales de UWF en el volumen del sistema protegido. Deshabilite UWF y reinicie antes de cambiarlos.</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>Esta operación no confirma los archivos preparados omitidos.</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>Los archivos preparados ya están en almacenamiento sin protección. No se necesita una confirmación automática.</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>Los cambios de umbral requieren una confirmación del Registro, que no está disponible en ROM de solo lectura.</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>Esperando a que Windows termine. Puede tardar un tiempo.</translation></message>
    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM está deshabilitado. Las confirmaciones automáticas de archivos preparados se pausan hasta reiniciar. Las operaciones manuales dependen de las capacidades actuales de UWF.</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>Esta instantánea de configuración incluye ajustes que requieren UWF deshabilitado en la sesión actual. Deshabilite UWF y reinicie antes de ejecutarla.</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>Acciones de la sesión actual (HORM, FullCommit y restablecimiento de la superposición persistente; expórtelas desde Cambios pendientes)</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>La desprotección en bloque requiere confirmar el conjunto completo de volúmenes de la próxima sesión.</translation></message>
</context>
</TS>
