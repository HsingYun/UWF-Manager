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
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Proteger este volumen en la próxima sesión. Las escrituras se redirigen a la superposición y se descartan al reiniciar.</translation></message>
    <message><source>Current session</source><translation>Sesión actual</translation></message>
    <message><source>Next session</source><translation>Próxima sesión</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identificación del volumen: la letra de unidad es sencilla, pero la asociación se pierde si cambia, por ejemplo al añadir o quitar discos. El ID de volumen permanece estable.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF no admite este volumen: </translation></message>
    <message><source>Global settings</source><translation>Configuración global</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Estado del filtro UWF en la sesión actual (solo lectura).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Activar UWF en la próxima sesión. Las escrituras en volúmenes protegidos van a la superposición y se descartan al reiniciar.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Para cambiar el tipo y tamaño máximo:
1. Desactive el filtro arriba.
2. Reinicie para que quede desactivado.
3. Cambie el ajuste.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Ubicación de la superposición: RAM es más rápida pero consume memoria; Disk usa la unidad del sistema y ofrece más capacidad. Ambas se descartan al reiniciar.</translation></message>
    <message><source>Type</source><translation>Tipo</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Capacidad máxima de la superposición. RAM está limitada por la memoria total. Disk requiere al menos 1024 MB y espacio libre suficiente en el volumen del sistema.</translation></message>
    <message><source>Maximum size</source><translation>Tamaño máximo</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Tamaño máximo · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Umbral de advertencia</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Registra una notificación de advertencia en el registro de eventos cuando el uso de la superposición alcanza este valor. Si ambos eventos están habilitados, este valor debe ser inferior al umbral crítico. El valor 0 deshabilita únicamente el evento de advertencia.</translation></message>
    <message><source>Critical threshold</source><translation>Umbral crítico</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Registra una notificación crítica en el registro de eventos cuando el uso de la superposición alcanza este valor. Si ambos eventos están habilitados, este valor debe ser superior al umbral de advertencia. El valor 0 deshabilita únicamente el evento crítico.</translation></message>
    <message><source>Overlay</source><translation>Superposición</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Tipo de unidad no compatible (solo discos locales fijos).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>El volumen supera el límite de UWF de 16 TiB por volumen protegido.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: se puede proteger el volumen, pero no admite exclusiones ni confirmación por archivo.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Error: Lectura de información del volumen.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 libres / %2</translation></message>
    <message><source>Commit</source><translation>Confirmar en disco</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Confirmar en disco los cambios de este archivo en la superposición / Confirmar cambios del Registro. Esta acción no se puede deshacer.</translation></message>
    <message><source>Commit file changes…</source><translation>Confirmar cambios del archivo…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Seleccionar archivo para confirmar en disco. Confirmar en disco los cambios de este archivo en la superposición.</translation></message>
    <message><source>Commit folder changes…</source><translation>Confirmar cambios de la carpeta…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Seleccionar carpeta para confirmar en disco. Confirmar en disco los cambios de esta carpeta en la superposición. Se incluyen todas las subcarpetas.</translation></message>
    <message><source>Delete and commit file…</source><translation>Eliminar este archivo y confirmar la eliminación en disco…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Seleccionar archivo cuya eliminación confirmar. Eliminar este archivo y confirmar la eliminación en disco.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Eliminar la carpeta y su contenido y confirmar las eliminaciones en disco…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Seleccionar carpeta cuya eliminación confirmar. Eliminar la carpeta y su contenido y confirmar las eliminaciones en disco. Se incluyen todas las subcarpetas.</translation></message>
    <message><source>Commit registry changes…</source><translation>Confirmar cambios del Registro…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Introduzca la clave y, opcionalmente, el nombre del valor. Sin nombre, se confirman todos los valores de la clave y de sus subclaves.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Confirmar eliminación del Registro…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Introduzca la clave que eliminar y un nombre de valor opcional. Sin nombre, se elimina y confirma todo el árbol: la clave, sus valores y subclaves.</translation></message>
    <message><source>File exclusions</source><translation>Exclusiones de archivos</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Archivos y carpetas del volumen excluidos de UWF. Haga doble clic en una entrada para copiar su ruta.</translation></message>
    <message><source>Registry exclusions</source><translation>Exclusiones del Registro</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Las exclusiones del Registro dependen del filtro UWF global, no de la protección del volumen. La lista se comparte entre volúmenes y se muestra una sola vez. Haga doble clic para copiar una ruta.</translation></message>
    <message><source>File staging</source><translation>Archivos para confirmar</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>archivos / carpetas: Se confirman automáticamente antes de un apagado o reinicio seguro. Se incluyen todas las subcarpetas. Mientras haya entradas, esta exclusión del Registro conserva la lista entre sesiones protegidas: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Añadir archivos o una carpeta a la confirmación automática.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Seleccionar archivo para confirmar en disco. Se confirman automáticamente antes de un apagado o reinicio seguro.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Seleccionar carpeta para confirmar en disco. Se confirman automáticamente antes de un apagado o reinicio seguro. Se incluyen todas las subcarpetas.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Seleccionar archivo para confirmar en disco · Archivos para confirmar</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Seleccionar carpeta para confirmar en disco · Archivos para confirmar · Se incluyen todas las subcarpetas.</translation></message>
    <message><source>Cannot stage this path</source><translation>No se puede preparar esta ruta</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>La ruta coincide con una exclusión UWF. Quite primero la exclusión:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>La ruta coincide con una entrada de confirmación automática. Quite primero la entrada:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>La ruta coincide con una entrada de confirmación automática</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Esta exclusión del Registro es necesaria mientras haya entradas de confirmación automática</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>La ruta no existe o no es del tipo esperado:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>No se pudo examinar la ruta:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>No se pueden preparar puntos de reanálisis:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Sin letra de unidad, UWF no puede confirmar esta ruta:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>La ruta está en el volumen %1. Añádala desde la pestaña de confirmación automática de ese volumen.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>No se puede preparar la raíz de un volumen: confirmaría recursivamente todo su contenido.</translation></message>
    <message><source>File staging could not be saved</source><translation>Error: Guardado de lista de archivos para confirmar</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Se recargó la lista tras no poder completar la actualización:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Archivos para confirmar: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Se confirman automáticamente antes de un apagado o reinicio seguro.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Archivos para confirmar: %1 · archivos: %2 · carpetas: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>La confirmación automática de archivos depende de esta exclusión del Registro para conservar su lista tras reiniciar.</translation></message>
    <message><source>Built-in</source><translation>Integrado</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>El filtro UWF está desactivado. No se han acumulado cambios en la superposición que confirmar.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>El sistema de archivos %1 no admite confirmación por archivo.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>UWF no admite este volumen.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Este volumen no admite confirmación de archivos. Solo se puede confirmar el Registro (sus exclusiones son globales).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>El sistema de archivos %1 no admite confirmación por archivo. Solo se puede confirmar el Registro (sus exclusiones son globales).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Este volumen no está protegido por UWF; no hay cambios de archivos que confirmar.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Este volumen no está protegido por UWF; no hay cambios de archivos que confirmar. Solo se puede confirmar el Registro (sus exclusiones son globales).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Seleccionar archivo para confirmar en disco</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Seleccionar carpeta para confirmar en disco</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Seleccionar archivo cuya eliminación confirmar</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Seleccionar carpeta cuya eliminación confirmar</translation></message>
    <message><source>Name</source><translation>Nombre</translation></message>
    <message><source>Data</source><translation>Datos</translation></message>
    <message><source>(Default)</source><translation>(Predeterminado)</translation></message>
    <message><source>(value not set)</source><translation>(valor no establecido)</translation></message>
    <message><source>(empty)</source><translation>(vacío)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Desconocido (Valor: Archivo demasiado grande)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Ver archivos de superposición. &lt;b&gt;Solo es una instantánea de diagnóstico.&lt;/b&gt; No decida qué confirmar basándose en esta lista.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>La lista de archivos de superposición solo funciona en volúmenes NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>El filtro UWF está desactivado; no hay superposiciones que examinar.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Este volumen no está protegido por UWF y no tiene superposición que examinar.</translation></message>
    <message><source>Failed: %1</source><translation>Error: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>El proveedor WMI falló al enumerar archivos. Es una inestabilidad conocida de UWF_Overlay.GetOverlayFiles con superposiciones grandes o mucha carga de E/S.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Memoria insuficiente u operación no admitida. Se requieren NTFS y memoria libre. Pruebe con una superposición menor o cierre aplicaciones que consuman mucha memoria.</translation></message>
    <message><source>Raw error: %1</source><translation>Código de error: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Exportar a archivo… · Superposición · archivos</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Archivos de texto (*.txt);;Todos los archivos (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Archivos de comandos (*.cmd);;Todos los archivos (*)</translation></message>
    <message><source>Export failed</source><translation>Error de exportación</translation></message>
    <message><source>Export finished</source><translation>Exportación completada</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Error: Apertura del archivo (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Error: Escritura del archivo (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Configuración: Escritura del archivo · entradas: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>No se puede excluir un volumen completo (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>No se pueden excluir los archivos de paginación, intercambio e hibernación: UWF depende de ellos.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>No se puede excluir el directorio completo: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>No se puede excluir el directorio completo: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>No se puede excluir el directorio completo: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>No se puede excluir este archivo esencial del sistema: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>No se puede excluir el archivo de Registro de usuario NTUSER.DAT.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>La clave del Registro no puede estar vacía.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>La clave del Registro no puede empezar por una barra inversa.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>La ruta contiene barras inversas consecutivas y no es válida.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Las rutas del Registro usan `\` como separador, no `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>La ruta contiene caracteres de control invisibles y no es válida.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>UWF prohíbe excluir el secreto de la cuenta del equipo del dominio: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Solo se pueden excluir subclaves concretas de estas claves principales:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Añadir archivos o carpetas a las exclusiones. UWF no protegerá esos elementos.</translation></message>
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
    <message><source>Commit folder changes to disk…</source><translation>Confirmar en disco los cambios de esta carpeta en la superposición…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Confirmar en disco los cambios de este archivo en la superposición…</translation></message>
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
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Revisar todos los cambios pendientes y aplicarlos juntos. La mayoría de los cambios se aplican tras el próximo reinicio.</translation></message>
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
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>No se encontró el destino; no hay nada que confirmar en la superposición.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>El destino solo existe en la superposición: se creó bajo UWF y nunca se confirmó en disco o Registro persistente. Desaparecerá al reiniciar; confirmar su eliminación no es necesario ni posible.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>El sistema rechazó un parámetro (ruta o argumento no válido).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>La operación falló. Otro proceso puede estar usando el destino, como una carpeta abierta en el Explorador o un archivo abierto. Cierre esos programas y reintente.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>La operación falló; consulte el registro para más detalles.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operación rechazada (código %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Desconocido.</translation></message>
    <message><source>Commit canceled</source><translation>Confirmación cancelada</translation></message>

    <message><source>Succeeded</source><translation>Correcto</translation></message>
    <message><source>Skipped</source><translation>Omitido</translation></message>
    <message><source>Failed</source><translation>Error</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Correcto: %1; Omitido: %2; Error: %3.</translation></message>

    <message><source>Existed before</source><translation>Existía antes</translation></message>
    <message><source>Exists after</source><translation>Existe después</translation></message>
    <message><source>Error code</source><translation>Código de error</translation></message>
    <message><source>Reason</source><translation>Motivo</translation></message>
    <message><source>Previous page</source><translation>Página anterior</translation></message>
    <message><source>First page</source><translation>Primera página</translation></message>
    <message><source>Last page</source><translation>Última página</translation></message>
    <message><source>Next page</source><translation>Página siguiente</translation></message>
    <message><source>Copy selected rows</source><translation>Copiar filas seleccionadas</translation></message>
    <message><source>Copy all</source><translation>Copiar todo</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Esta exclusión del Registro es necesaria mientras haya entradas de confirmación automática. Archivos para confirmar: Vaciar → Retirada de exclusión del Registro.</translation></message>
    <message><source>Copy current page</source><translation>Copiar página actual</translation></message>
    <message><source>No pending changes</source><translation>No hay cambios pendientes</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Exclusiones del Registro · Configuración global.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volumen %1: Protección UWF / Exclusiones de archivos / Archivos para confirmar.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: No compatible</translation></message>
    <message><source>Review and apply changes</source><translation>Revisar y aplicar cambios</translation></message>
    <message><source>Confirm apply</source><translation>Confirmar aplicación</translation></message>
    <message><source>Result</source><translation>Resultado</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filtro: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Definición del umbral de advertencia: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Definición del umbral crítico: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Definición del tipo de superposición / Definición del tamaño máximo: Para cambiar el tipo y tamaño máximo:
1. Desactive el filtro arriba.
2. Reinicie para que quede desactivado.
3. Cambie el ajuste.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Definición del tipo de superposición: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Definición del tamaño máximo: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Definición del tamaño máximo: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volumen %1 · Protección UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volumen %1 · Identificar por: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volumen %1 · Adición de exclusión de archivo: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volumen %1 · Retirada de exclusión de archivo: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Adición de exclusión del Registro: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Retirada de exclusión del Registro: %1</translation></message>
    <message><source>Time</source><translation>Hora</translation></message>
    <message><source>Level</source><translation>Nivel</translation></message>
    <message><source>Tag</source><translation>Etiqueta</translation></message>
    <message><source>Message</source><translation>Mensaje</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Calculando archivos para confirmar automáticamente…</translation></message>
    <message><source>Committing staged files</source><translation>Confirmando archivos preparados</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 de %2 archivos procesados.</translation></message>
    <message><source>No additional error details were provided.</source><translation>No se proporcionaron más detalles del error.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>La confirmación automática se detuvo inesperadamente:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Confirmación automática incompleta: Error desconocido.</translation></message>
    <message><source>File staging will be skipped</source><translation>Se omitirá la confirmación automática</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Reinicio directo: Se omitirá la confirmación automática.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Apagado directo: Se omitirá la confirmación automática.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Se omitió la confirmación automática al elegir una acción de energía directa.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Apagado seguro: Error</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Apagar: Error (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Reinicio seguro: Error</translation></message>
    <message><source>Direct restart failed</source><translation>Reinicio directo: Error</translation></message>
    <message><source>Direct shutdown failed</source><translation>Apagado directo: Error</translation></message>
    <message><source>Restart failed: %1</source><translation>Reiniciar: Error (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>No se pudo leer la lista de confirmación automática:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>No se pudo leer la lista de confirmación automática: Error desconocido.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>archivos: %1 · Correcto: %2 · Omitido (archivos): %3 · Omitido (entradas): %4 · Error: %5</translation></message>
    <message><source>File staging preparation</source><translation>Archivos para confirmar: Detalles de la operación</translation></message>
    <message><source>Reparse points are not supported.</source><translation>No se admiten puntos de reanálisis.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Solo se pueden preparar rutas absolutas en volúmenes locales.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Solo se pueden preparar rutas absolutas en volúmenes locales.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>No se pudo examinar la ruta preparada.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>La ruta del archivo preparado ahora corresponde a otro tipo.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>La ruta de la carpeta preparada ahora corresponde a otro tipo.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>No se pudo enumerar por completo la carpeta preparada.</translation></message>
    <message><source>The path has no drive letter.</source><translation>La ruta no tiene letra de unidad.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>La raíz del volumen no puede procesarse recursivamente.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>El proveedor UWF rechazó la confirmación.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Error al confirmar: Error desconocido.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Se registraron %1 errores adicionales.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>La ruta no tiene letra de unidad. Identificación del volumen de destino: Error.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>No hay registro de sesión actual para el volumen %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Esta ruta está en las exclusiones de archivos.
Exclusión: %1</translation></message>

    <message><source>Continue</source><translation>Continuar</translation></message>
    <message><source>Delete and commit</source><translation>Eliminar y confirmar</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Esta clave está en las exclusiones del Registro.
Exclusión: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>No hay registro del filtro del Registro para la sesión actual.</translation></message>

    <message><source>Export commands to file</source><translation>Exportar comandos a un archivo</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Exportar a archivo… · Comando: %1
%2</translation></message>
    <message><source>Import</source><translation>Importar</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Escriba o pegue un comando por línea. Cargue texto, registros o scripts (.txt, .bat, .ps1); se añaden las líneas con este comando. La importación prepara los cambios. Solo «Revisar y aplicar» en la barra de herramientas los escribe en el sistema.</translation></message>
    <message><source>Load from file…</source><translation>Cargar desde archivo…</translation></message>
    <message><source>Load recommended configuration</source><translation>Cargar configuración recomendada</translation></message>
    <message><source>Choose recommended configuration</source><translation>Elegir configuración recomendada</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Elija los grupos recomendados que añadir. Puede revisar o borrar comandos antes de importar.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>De la documentación oficial de Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Exclusiones UWF recomendadas por Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Revise antes de importar: las carpetas deben existir para que UWF acepte las exclusiones.</translation></message>
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
    <message><source>Internal error: no applier registered.</source><translation>Error interno: no hay controlador de aplicación registrado.</translation></message>
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
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Hay %1 cambios sin aplicar.

¿Descartarlos y continuar?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Comando uwfmgr incompleto</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Falta el argumento: Tamaño (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>El tamaño debe ser un entero no negativo en MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Falta el argumento: Superposición · Tipo (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Valor no válido: Superposición · Tipo: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Falta el argumento: Volumen (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>El volumen debe ser una letra de unidad, como C:</translation></message>
    <message><source>Missing path argument</source><translation>Falta el argumento: Ruta</translation></message>
    <message><source>Missing registry key argument</source><translation>Falta el argumento: Registro · Clave</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Comillas dobles sin cerrar en el comando</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Argumento adicional inesperado: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Comando uwfmgr no compatible (sin acción de interfaz equivalente)</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Ya está en el estado deseado; no se requiere acción</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>La ruta no pertenece al volumen o este no admite exclusiones (p. ej., exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Rechazado por la lista de bloqueo UWF (archivo de sistema, Windows, paginación, etc.)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valor no válido: Tamaño: %1</translation></message>
    <message><source>maximum size</source><translation>Tamaño máximo</translation></message>
    <message><source>warning threshold</source><translation>Umbral de advertencia</translation></message>
    <message><source>critical threshold</source><translation>Umbral crítico</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>La configuración global no está disponible; el comando no se añadió a los cambios pendientes.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Volumen %1 desconocido (no hay disco apto para UWF con esa letra)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volumen %1: Ya está en el estado deseado; no se requiere acción</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: La ruta no tiene letra de unidad. Identificación del volumen de destino: Error.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volumen %1: UWF No compatible</translation></message>
    <message><source>file exclusion</source><translation>Exclusiones de archivos</translation></message>
    <message><source>registry exclusion</source><translation>Exclusiones del Registro</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Las exclusiones del Registro requieren la pestaña Registro, que no está presente</translation></message>
    <message><source>Unsupported command</source><translation>Comando: No compatible</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Mostrar u ocultar.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Activado</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Desactivado</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Desconocido</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Usado %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Modo avanzado</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordinar la confirmación automática con el apagado y reinicio de Windows.</translation></message>
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
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>El servicio no ha aceptado las notificaciones de preapagado de Windows.</translation></message>
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
    <message><source>File staging completed</source><translation>Archivos para confirmar: Correcto</translation></message>
    <message><source>File staging failed</source><translation>Archivos para confirmar: Error</translation></message>
    <message><source>All staged targets were processed.</source><translation>Se procesaron todos los destinos preparados.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Se procesaron todos los destinos; una o más operaciones fallaron.</translation></message>
    <message><source>Show main window</source><translation>Mostrar ventana principal</translation></message>
    <message><source>Restore default position</source><translation>Restaurar posición predeterminada</translation></message>
    <message><source>Show overlay hub</source><translation>Mostrar HUB de superposición</translation></message>
    <message><source>Hide overlay hub</source><translation>Ocultar HUB de superposición</translation></message>
    <message><source>Exit application</source><translation>Salir de la aplicación</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Error: Conexión al sistema</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Error: Lectura de valores del Registro (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Enumeración de archivos de superposición: Error desconocido.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Error: Acción en filtro: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Error: Lectura del estado del filtro</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Error: Definición del umbral de advertencia</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Error: Definición del umbral crítico</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Error: Lectura del estado de la superposición</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Error: Definición del tipo de superposición</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Error: Definición del tamaño máximo</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Error: Lectura de la configuración de superposición</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Error: Registro del volumen %1 en UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Error: Protección del volumen %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Error: Definición de identificación del volumen %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volumen %1 · Error: Adición de exclusión de archivo %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ La exclusión ya no existe en el volumen %1: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volumen %1 · Error: Retirada de exclusión de archivo %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Error: Lectura de la configuración del volumen</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Error: Adición de exclusión del Registro %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Error: Retirada de exclusión del Registro %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Error: Actualización de persistencia del Registro</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Error: Lectura del filtro del Registro</translation></message>
    <message><source>UWF is not available</source><translation>UWF: No compatible</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Lectura del estado del destino: Error desconocido.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>La ruta no existe; no hay nada que eliminar.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Error desconocido.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>No se pudo confirmar el resultado al fallar la relectura del estado del destino: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>El proveedor aceptó la eliminación, pero la comprobación autoritativa confirma que el destino sigue existiendo.</translation></message>
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
    <message><source>Committing</source><translation>Confirmando</translation></message>
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
Gestione la protección de volúmenes, la superposición, las exclusiones y los archivos pendientes.</translation></message>
    <message><source>About this snapshot</source><translation>Acerca de esta instantánea</translation></message>
    <message><source>Account</source><translation>Cuenta</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Añadir una configuración UWF desde un script o texto.</translation></message>
    <message><source>Add exclusion</source><translation>Añadir exclusión</translation></message>
    <message><source>Add files or folders</source><translation>Añadir archivos o carpetas</translation></message>
    <message><source>Add selected items</source><translation>Añadir elementos seleccionados</translation></message>
    <message><source>Added</source><translation>Añadido</translation></message>
    <message><source>Administrator</source><translation>Administrador</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Todos los cambios se han guardado en el disco y se conservarán sin reiniciar.</translation></message>
    <message><source>All files (*)</source><translation>Todos los archivos (*)</translation></message>
    <message><source>All levels</source><translation>Todos los niveles</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Una instantánea de diagnóstico vacía no significa que la superposición no contenga cambios.</translation></message>
    <message><source>Application path</source><translation>Ruta de la aplicación</translation></message>
    <message><source>Application result</source><translation>Resultado de aplicación</translation></message>
    <message><source>Apply changes</source><translation>Aplicar cambios</translation></message>
    <message><source>Architecture</source><translation>Arquitectura</translation></message>
    <message><source>Automatic</source><translation>Automático</translation></message>
    <message><source>Automatic file staging</source><translation>Guardado automático de archivos pendientes</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Guardado automático incompleto</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Guardar automáticamente los archivos pendientes aptos durante el apagado y reinicio normales.</translation></message>
    <message><source>Available</source><translation>Disponible</translation></message>
    <message><source>Back</source><translation>Atrás</translation></message>
    <message><source>Background service</source><translation>Servicio en segundo plano</translation></message>
    <message><source>Binding</source><translation>Vinculación</translation></message>
    <message><source>By drive letter</source><translation>Por letra de unidad</translation></message>
    <message><source>By volume ID</source><translation>Por ID de volumen</translation></message>
    <message><source>Cancel reading</source><translation>Cancelar lectura</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Los cambios ya confirmados se conservan.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Seleccione la clave del Registro que desea excluir de la protección UWF, incluidas sus subclaves y valores.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Elija dónde guardar el archivo.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Elija una ubicación dentro de la unidad o carpeta permitida.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Elija otra carpeta o compruebe los permisos de acceso y reinténtelo.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Elija otra clave o compruebe los permisos de acceso y reinténtelo.</translation></message>
    <message><source>Clear filters</source><translation>Borrar filtros</translation></message>
    <message><source>Clear logs</source><translation>Borrar registros</translation></message>
    <message><source>Clear logs?</source><translation>¿Borrar los registros?</translation></message>
    <message><source>Collapse</source><translation>Contraer</translation></message>
    <message><source>Commit changes</source><translation>Confirmar cambios</translation></message>
    <message><source>Commit changes…</source><translation>Confirmar cambios…</translation></message>
    <message><source>Commit complete</source><translation>Confirmación completada</translation></message>
    <message><source>Commit file changes?</source><translation>¿Confirmar los cambios del archivo?</translation></message>
    <message><source>Commit folder changes?</source><translation>¿Confirmar los cambios de la carpeta?</translation></message>
    <message><source>Commit registry key changes?</source><translation>¿Confirmar los cambios de la clave del registro?</translation></message>
    <message><source>Commit registry value changes?</source><translation>¿Confirmar los cambios del valor del registro?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Confirma los archivos pendientes antes del apagado y reinicio normales</translation></message>
    <message><source>Compatibility mode</source><translation>Modo de compatibilidad</translation></message>
    <message><source>Configuration applied</source><translation>Configuración aplicada</translation></message>
    <message><source>Configuration error</source><translation>Error de configuración</translation></message>
    <message><source>Configured</source><translation>Configurado</translation></message>
    <message><source>Confirm before continuing</source><translation>Confirmar antes de continuar</translation></message>
    <message><source>Connected and authenticated</source><translation>Conectado y autenticado</translation></message>
    <message><source>Connecting…</source><translation>Conectando…</translation></message>
    <message><source>Contact author</source><translation>Contactar con el autor</translation></message>
    <message><source>Continue editing</source><translation>Continuar editando</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Si continúa el reinicio, se descartarán los cambios de los archivos que no pudieron guardarse.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Si continúa el apagado, se descartarán los cambios de los archivos que no pudieron guardarse.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Coordinar con el apagado de Windows</translation></message>
    <message><source>Copy all logs</source><translation>Copiar todos los registros</translation></message>
    <message><source>Copy full report</source><translation>Copiar informe completo</translation></message>
    <message><source>Copy name</source><translation>Copiar nombre</translation></message>
    <message><source>Copy path</source><translation>Copiar ruta</translation></message>
    <message><source>Copy selection</source><translation>Copiar selección</translation></message>
    <message><source>Copy version</source><translation>Copiar versión</translation></message>
    <message><source>Counting staged files…</source><translation>Contando archivos pendientes…</translation></message>
    <message><source>Create</source><translation>Crear</translation></message>
    <message><source>Critical %1 MB</source><translation>Crítico %1 MB</translation></message>
    <message><source>Current filter</source><translation>Filtro actual</translation></message>
    <message><source>Current run</source><translation>Ejecución actual</translation></message>
    <message><source>Current selection</source><translation>Selección actual</translation></message>
    <message><source>Current usage</source><translation>Uso actual</translation></message>
    <message><source>Date modified</source><translation>Fecha de modificación</translation></message>
    <message><source>Debug</source><translation>Depuración</translation></message>
    <message><source>Delete and commit this file?</source><translation>¿Eliminar este archivo y confirmar su eliminación?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>¿Eliminar esta carpeta y confirmar su eliminación?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>¿Eliminar esta clave del registro y confirmar su eliminación?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>¿Eliminar este valor del registro y confirmar su eliminación?</translation></message>

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
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>El modo avanzado no está disponible. Use Apagado seguro o Reinicio seguro para guardar los archivos pendientes.</translation></message>
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
    <message><source>File saving has started and cannot be canceled.</source><translation>El guardado de archivos ha comenzado y no se puede cancelar.</translation></message>
    <message><source>File saving incomplete</source><translation>Guardado de archivos incompleto</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Se omitirá el guardado de archivos pendientes. Los cambios sin confirmar en rutas protegidas se perderán tras reiniciar.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Se omitirá el guardado de archivos pendientes. Los cambios sin confirmar en rutas protegidas se perderán tras apagar.</translation></message>
    <message><source>File type</source><translation>Tipo de archivo</translation></message>
    <message><source>Files and folders</source><translation>Archivos y carpetas</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Se pueden seleccionar archivos y carpetas a la vez.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Los archivos pendientes no se guardarán.</translation></message>
    <message><source>Filter is disabled</source><translation>El filtro está desactivado</translation></message>
    <message><source>Fit columns to contents</source><translation>Ajustar columnas al contenido</translation></message>
    <message><source>Folder address</source><translation>Dirección de la carpeta</translation></message>
    <message><source>Folder name</source><translation>Nombre de carpeta</translation></message>
    <message><source>Folders</source><translation>Carpetas</translation></message>
    <message><source>Full configuration</source><translation>Configuración completa</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Las futuras escrituras en esta clave no estarán protegidas por UWF.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Las futuras escrituras en esta ruta no estarán protegidas por UWF.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Exclusión global del registro · incluye subclaves</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Suba a la carpeta superior o elija otra ubicación.</translation></message>
    <message><source>Home</source><translation>Carpeta personal</translation></message>
    <message><source>Import configuration</source><translation>Importar configuración</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>La importación actualiza el borrador de configuración. Use Revisar y aplicar para escribir los cambios en el sistema.</translation></message>
    <message><source>Import results</source><translation>Resultados de importación</translation></message>
    <message><source>Imported</source><translation>Importado</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>El informe muestra los resultados de los comandos para el borrador de configuración. Seleccione una fila para ver el comando y el resultado completos.</translation></message>
    <message><source>Imported: %1</source><translation>Importado: %1</translation></message>
    <message><source>Information</source><translation>Información</translation></message>
    <message><source>Item details</source><translation>Detalles del elemento</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Guardar listas grandes de archivos pendientes puede tardar más.</translation></message>
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
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Si UWF protege la unidad donde está la aplicación, asegúrese primero de que sus archivos se conserven tras reiniciar. Guardar de forma permanente muchos archivos pendientes puede alargar el apagado o el reinicio.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Se pueden seleccionar varios archivos.</translation></message>
    <message><source>Name and location</source><translation>Nombre y ubicación</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Valor con nombre: %1 · solo este valor</translation></message>
    <message><source>Navigate to registry path</source><translation>Ir a la ruta del registro</translation></message>
    <message><source>New folder</source><translation>Nueva carpeta</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Los nuevos registros aparecerán al actualizar.</translation></message>
    <message><source>Next-session configuration</source><translation>Configuración de la próxima sesión</translation></message>
    <message><source>No files to commit</source><translation>No hay archivos que confirmar</translation></message>
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
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Solo se guardan automáticamente los archivos pendientes. Los demás cambios sin confirmar en rutas protegidas se perderán tras reiniciar.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Solo se guardan automáticamente los archivos pendientes. Los demás cambios sin confirmar en rutas protegidas se perderán tras apagar.</translation></message>
    <message><source>Open</source><translation>Abrir</translation></message>
    <message><source>Open folder</source><translation>Abrir carpeta</translation></message>
    <message><source>Operating system</source><translation>Sistema operativo</translation></message>
    <message><source>Operation and security</source><translation>Funcionamiento y seguridad</translation></message>
    <message><source>Original command</source><translation>Comando original</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Se alcanzó el umbral crítico de la superposición. Guarde los cambios y reinicie de forma segura.</translation></message>
    <message><source>Overlay maximum size</source><translation>Tamaño máximo de superposición</translation></message>
    <message><source>Overlay type</source><translation>Tipo de superposición</translation></message>
    <message><source>Overlay usage</source><translation>Uso de superposición</translation></message>
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
    <message><source>Ready to save</source><translation>Listo para guardar</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Los registros de la ejecución actual aparecerán aquí al terminar la lectura.</translation></message>
    <message><source>Recursive staging</source><translation>Preparación recursiva</translation></message>
    <message><source>Refresh folder</source><translation>Actualizar carpeta</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Clave del registro o ajuste de persistencia</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Las tareas restantes se pueden cancelar cuando termine el destino actual.</translation></message>
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
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>El reinicio continuará al terminar de guardar. Mantenga abierta la aplicación.</translation></message>
    <message><source>Restore</source><translation>Restaurar</translation></message>
    <message><source>Retry</source><translation>Reintentar</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Revise los detalles de los fallos antes de continuar el apagado.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Revise los detalles de los fallos antes de reiniciar.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Revise los detalles de la operación. Lea el estado actual del sistema antes de reintentar cambios que puedan haberse escrito.</translation></message>
    <message><source>Review the result for each target.</source><translation>Revise el resultado de cada destino.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Revise estos cambios antes de escribirlos en el sistema.</translation></message>
    <message><source>Running</source><translation>En ejecución</translation></message>
    <message><source>Save</source><translation>Guardar</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Guardar los archivos pendientes en disco y reiniciar.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Guardar los archivos pendientes en disco y apagar.</translation></message>
    <message><source>Saving staged files</source><translation>Guardando archivos pendientes</translation></message>
    <message><source>Saving to disk</source><translation>Guardando en disco</translation></message>
    <message><source>Saving…</source><translation>Guardando…</translation></message>
    <message><source>Scanning staged paths</source><translation>Examinando rutas pendientes</translation></message>
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
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>El apagado continuará al terminar de guardar. Mantenga abierta la aplicación.</translation></message>
    <message><source>Size</source><translation>Tamaño</translation></message>
    <message><source>Some changes could not be applied</source><translation>No se pudieron aplicar algunos cambios</translation></message>
    <message><source>Some files could not be saved</source><translation>No se pudieron guardar algunos archivos</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>No se pudo leer parte de la información del hardware. La demás información de diagnóstico se puede seguir consultando y copiando.</translation></message>
    <message><source>Source code</source><translation>Código fuente</translation></message>
    <message><source>Staged file</source><translation>Archivo pendiente</translation></message>
    <message><source>Staged path</source><translation>Ruta pendiente</translation></message>
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
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Los metadatos del sistema son solo para diagnóstico y no se pueden confirmar.</translation></message>
    <message><source>Takes effect after restart</source><translation>Surte efecto tras reiniciar</translation></message>
    <message><source>Target</source><translation>Destino</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistencia de licencias de Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>El proveedor UWF no pudo completar la enumeración. Puede ocurrir con una superposición grande o recursos del sistema insuficientes.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>La configuración se ha escrito en el sistema. La mayoría de los cambios surten efecto tras reiniciar.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>La configuración coincide con el destino de la próxima sesión. Consulte o exporte los comandos en Configuración completa.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Los registros actuales se borrarán y no se podrán recuperar.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>La carpeta de destino no existe.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>No se pudo leer el estado del servicio del modo avanzado. Compruebe los permisos del servicio y reinténtelo.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>La lista puede omitir archivos o incluir archivos ya confirmados. No la use como lista de confirmación.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>El elemento seleccionado ya no existe: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>El servicio está instalado, pero detenido. Inícielo para restablecer el guardado automático.</translation></message>
    <message><source>This PC</source><translation>Este equipo</translation></message>
    <message><source>This folder is empty</source><translation>Esta carpeta está vacía</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Este informe puede contener nombres de cuenta, identificadores de dispositivo y rutas de archivo. Revíselo antes de compartirlo.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Guarde este script CMD en UTF-8 como archivo .cmd y ejecútelo en el símbolo del sistema. La exportación usa el mismo script.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Pruebe otra palabra clave o nivel de registro.</translation></message>
    <message><source>Try another search term.</source><translation>Pruebe otro término de búsqueda.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>El tipo y el tamaño máximo no se pueden cambiar con el filtro activado. Desactive el filtro y reinicie primero.</translation></message>
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
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Los elementos fallidos no se han guardado. Seleccione una fila para ver la ruta completa y el motivo.</translation></message>
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
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Sin modo avanzado, use Apagado seguro o Reinicio seguro en esta aplicación para guardar los archivos pendientes.</translation></message>
    <message><source>Write protection</source><translation>Protección contra escritura</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>¿Escribir estos cambios de configuración en el sistema? La mayoría surten efecto tras reiniciar.</translation></message>
    <message><source>staged files</source><translation>archivos pendientes</translation></message>
    <message><source>uwfmgr commands</source><translation>comandos uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Marcar las rutas seleccionadas para eliminarlas. Se hará efectivo al aplicar.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Quitar de la lista de preparación automática de archivos al aplicar.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Añadir a la lista de preparación automática de archivos al aplicar.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Al aplicar este plan se vacía la lista de preparación guardada y se elimina su exclusión del Registro si ya no es necesaria.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Los cambios de preparación de archivos deben aplicarse en UWF Manager y no pueden exportarse como comandos uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>La preparación de archivos se guarda al aplicar</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Alcance de la instantánea de la superposición</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Esta lista es solo de diagnóstico. No permite determinar qué archivos deben confirmarse.</translation></message>
    <message><source>NTFS volumes only</source><translation>Solo volúmenes NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>La lectura de superposiciones grandes puede ser lenta o fallar por falta de recursos.</translation></message>
    <message><source>Small files may be missing</source><translation>Pueden faltar archivos pequeños</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Los archivos menores que el tamaño del clúster del disco (normalmente 4 KB) pueden no aparecer en la lista.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>No indica confirmaciones pendientes</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>También pueden aparecer archivos ya confirmados, rutas excluidas y elementos afectados por otras operaciones.</translation></message>

    <message><source>Command processing result</source><translation>Resultado del procesamiento del comando</translation></message>
    <message><source>Copy command</source><translation>Copiar comando</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · No se pudieron leer %1 ramas</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>No se pudieron leer %1 ramas. Continúe con el contenido encontrado o reintente solo los elementos fallidos.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 archivos · %2 carpetas</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 valores · %2 claves</translation></message>
    <message><source>Attempts</source><translation>Intentos</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Cancelar tareas restantes</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Confirmar solo el contenido recorrido correctamente. Las ramas ilegibles permanecen sin cambios.</translation></message>
    <message><source>Committing changes</source><translation>Confirmando cambios</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Las eliminaciones y confirmaciones completadas no se pueden deshacer.</translation></message>
    <message><source>Continue…</source><translation>Continuar…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Eliminar y confirmar solo los destinos encontrados. Se conservan las ramas ilegibles y sus carpetas o claves superiores.</translation></message>
    <message><source>Deleting and committing</source><translation>Eliminando y confirmando</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>La eliminación y la confirmación no se pueden deshacer. Solo se procesa el ámbito seleccionado; no se aplica la configuración UWF pendiente.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Determinando el ámbito. Se registran las ramas ilegibles y el recorrido continúa en el resto.</translation></message>
    <message><source>Export traversal records</source><translation>Exportar registro del recorrido</translation></message>
    <message><source>No actionable content</source><translation>No hay contenido procesable</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>No se encontraron destinos procesables. No se ha eliminado ni confirmado nada.</translation></message>
    <message><source>No changes have been committed.</source><translation>No se han confirmado cambios</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>No se ha eliminado ni confirmado nada</translation></message>
    <message><source>Path</source><translation>Ruta</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Eliminar permanentemente el ámbito seleccionado y guardar las eliminaciones en el volumen protegido.</translation></message>
    <message><source>Preparation complete</source><translation>Preparación completada</translation></message>
    <message><source>Preparing commit</source><translation>Preparando la confirmación</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Preparando la eliminación y confirmación</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Procesar solo el contenido recorrido correctamente</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Se omiten los puntos de reanálisis para mantener el recorrido en el ámbito seleccionado.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Un reinicio no deshace una confirmación. Solo se procesa el ámbito seleccionado; no se aplica la configuración UWF pendiente.</translation></message>
    <message><source>Retry failed items</source><translation>Reintentar los fallidos</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Guardar los cambios del ámbito seleccionado en el volumen protegido.</translation></message>
    <message><source>Stopping the operation</source><translation>Deteniendo la operación</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Se conservan los registros de éxitos y errores</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>La carpeta no está vacía; se ha conservado el contenido sin procesar.</translation></message>
    <message><source>The operation could not be completed</source><translation>No se pudo completar la operación</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>La clave del Registro tiene valores nuevos; se ha conservado el contenido sin procesar.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>La clave del Registro aún tiene subclaves; se ha conservado el contenido sin procesar.</translation></message>
    <message><source>The target no longer exists.</source><translation>El destino ya no existe.</translation></message>
    <message><source>Traversal complete</source><translation>Recorrido completado</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Recorrido completado con contenido ilegible</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Se conserva el registro del recorrido y se puede exportar.</translation></message>
    <message><source>Traverse again</source><translation>Recorrer de nuevo</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Recorra de nuevo o cierre esta ventana.</translation></message>
    <message><source>Traversing the folder</source><translation>Recorriendo la carpeta</translation></message>
    <message><source>Traversing the registry</source><translation>Recorriendo el Registro</translation></message>
    <message><source>Unable to prepare the operation</source><translation>No se puede preparar la operación</translation></message>
    <message><source>View results</source><translation>Ver resultados</translation></message>

    <message><source>All changes committed</source><translation>Todos los cambios se han guardado</translation></message>
    <message><source>Some changes committed</source><translation>Algunos cambios se han guardado</translation></message>
    <message><source>No changes could be committed</source><translation>No se pudo guardar ningún cambio</translation></message>
    <message><source>No changes to commit</source><translation>No hay cambios que guardar</translation></message>
    <message><source>Commit interrupted</source><translation>Guardado interrumpido</translation></message>
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
    <message><source>Export configuration</source><translation>Exportar configuración</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Guarda el tema, el idioma y la visibilidad actuales del HUB en UWF.ini junto a esta aplicación.</translation></message>
    <message><source>Configuration saved</source><translation>Configuración guardada</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>La confirmación de archivos se ha detenido. Resuelva el error antes de volver a intentarlo.</translation></message>
    <message><source>No net change for this setting</source><translation>El valor final de este ajuste no ha cambiado tras la importación</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Valor final del borrador: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Valor final del borrador: %1 → %2 (valor solicitado: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Valor solicitado: %1 → %2; sustituido por un comando posterior. Valor final del borrador → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Se actualizó el borrador de %1</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Se actualizó el borrador de la protección del volumen %1 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Seleccione las entradas que desea quitar.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Las entradas seleccionadas no se pueden restaurar en este momento.</translation></message>
</context>
</TS>
