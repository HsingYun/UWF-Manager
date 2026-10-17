<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>Activar</translation></message>
    <message><source>Disable</source><translation>Desactivar</translation></message>
    <message><source>enable</source><translation>activar</translation></message>
    <message><source>disable</source><translation>desactivar</translation></message>
    <message><source>Enabled</source><translation>Activado</translation></message>
    <message><source>Disabled</source><translation>Desactivado</translation></message>
    <message><source>OK</source><translation>Aceptar</translation></message>
    <message><source>Cancel</source><translation>Cancelar</translation></message>
    <message><source>Close</source><translation>Cerrar</translation></message>
    <message><source>Refresh</source><translation>Actualizar</translation></message>
    <message><source>Apply</source><translation>Aplicar</translation></message>
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
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>El sistema «%1» (%2) no se reconoce como una edición compatible de Windows 10/11. UWF Manager se ejecuta en modo de compatibilidad; algunas funciones pueden no estar disponibles.</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Estado de protección del volumen en la sesión actual (solo lectura).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Proteger este volumen en la próxima sesión. Las escrituras se redirigen a la superposición y se descartan al reiniciar.</translation></message>
    <message><source>Protection:</source><translation>Protección:</translation></message>
    <message><source>Current session</source><translation>Sesión actual</translation></message>
    <message><source>Next session</source><translation>Próxima sesión</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>Sesión en curso (solo lectura). Los cambios no se aplican en esta sesión.</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>Sesión posterior al reinicio. Los cambios se aplican al reiniciar el sistema.</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identificación del volumen: la letra de unidad es sencilla, pero la asociación se pierde si cambia, por ejemplo al añadir o quitar discos. El ID de volumen permanece estable.</translation></message>
    <message><source>Bind by:</source><translation>Identificar por:</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF no admite este volumen: </translation></message>
    <message><source>Global settings</source><translation>Configuración global</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Estado del filtro UWF en la sesión actual (solo lectura).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Activar UWF en la próxima sesión. Las escrituras en volúmenes protegidos van a la superposición y se descartan al reiniciar.</translation></message>
    <message><source>Filter</source><translation>Filtro</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>Al alcanzar este uso, registra una advertencia en el registro de eventos. Debe ser menor que el umbral crítico. 0 desactiva el evento.</translation></message>
    <message><source>Critical threshold</source><translation>Umbral crítico</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>Al alcanzar este uso, registra un evento crítico. Debe ser mayor que el umbral de advertencia. 0 desactiva el evento.</translation></message>
    <message><source>Used / total</source><translation>Usado / total</translation></message>
    <message><source>Overlay</source><translation>Superposición</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; Usado &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Advertencia &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Crítico</translation></message>
    <message><source>UWF status unavailable: </source><translation>Estado de UWF no disponible: </translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>Para cambiar UWF se necesitan permisos de administrador. Reinicie el programa con clic derecho → «Ejecutar como administrador».</translation></message>
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
    <message><source>Search staged paths…</source><translation>Buscar rutas para confirmar…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Añadir archivos o una carpeta a la confirmación automática.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Seleccionar archivo para confirmar en disco. Se confirman automáticamente antes de un apagado o reinicio seguro.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Seleccionar carpeta para confirmar en disco. Se confirman automáticamente antes de un apagado o reinicio seguro. Se incluyen todas las subcarpetas.</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>Quitar inmediatamente las rutas seleccionadas de la confirmación automática.</translation></message>
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
    <message><source>Required by UWF for File staging.</source><translation>Necesario para la confirmación automática de UWF.</translation></message>
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
    <message><source>Commit registry changes</source><translation>Confirmar cambios del Registro</translation></message>
    <message><source>Commit registry deletion</source><translation>Confirmar eliminación del Registro</translation></message>
    <message><source>Key path:</source><translation>Clave · Ruta:</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>Escriba o pegue HKLM\Software\... y pulse Intro</translation></message>
    <message><source>Name</source><translation>Nombre</translation></message>
    <message><source>Data</source><translation>Datos</translation></message>
    <message><source>Key: %1</source><translation>Clave: %1</translation></message>
    <message><source>Value: %1</source><translation>Valor: %1</translation></message>
    <message><source>(none)</source><translation>(ninguno)</translation></message>
    <message><source>(Default)</source><translation>(Predeterminado)</translation></message>
    <message><source>(value not set)</source><translation>(valor no establecido)</translation></message>
    <message><source>(empty)</source><translation>(vacío)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Desconocido (Valor: Archivo demasiado grande)</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>(clave completa y todas sus subclaves)</translation></message>
    <message><source>Overlay files - %1</source><translation>Superposición · archivos - %1</translation></message>
    <message><source>View overlay files</source><translation>Ver archivos de superposición</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Ver archivos de superposición. &lt;b&gt;Solo es una instantánea de diagnóstico.&lt;/b&gt; No decida qué confirmar basándose en esta lista.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>La lista de archivos de superposición solo funciona en volúmenes NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>El filtro UWF está desactivado; no hay superposiciones que examinar.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Este volumen no está protegido por UWF y no tiene superposición que examinar.</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;Solo es una instantánea de diagnóstico.&lt;/b&gt; No decida qué confirmar basándose en esta lista.</translation></message>
    <message><source>NTFS volumes only.</source><translation>La lista de archivos de superposición solo funciona en volúmenes NTFS.</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>Una superposición grande consume más memoria y tiempo; puede tardar mucho o fallar.</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>La lista no es exacta: pueden faltar archivos menores que un clúster (normalmente 4 KB) y aparecer archivos ya confirmados, excluidos o afectados por otras operaciones.</translation></message>
    <message><source>Loading…</source><translation>Cargando…</translation></message>
    <message><source>Failed: %1</source><translation>Error: %1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>Superposición · archivos: %1</translation></message>
    <message><source>No files</source><translation>No hay archivos</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>Página %1 / %2 · archivos: %3</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>El proveedor WMI falló al enumerar archivos. Es una inestabilidad conocida de UWF_Overlay.GetOverlayFiles con superposiciones grandes o mucha carga de E/S.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Memoria insuficiente u operación no admitida. Se requieren NTFS y memoria libre. Pruebe con una superposición menor o cierre aplicaciones que consuman mucha memoria.</translation></message>
    <message><source>Raw error: %1</source><translation>Código de error: %1</translation></message>
    <message><source>Export to file…</source><translation>Exportar a archivo…</translation></message>
    <message><source>Export overlay file list</source><translation>Exportar a archivo… · Superposición · archivos</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Archivos de texto (*.txt);;Todos los archivos (*)</translation></message>
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
    <message><source>Search file paths…</source><translation>Buscar rutas de archivos…</translation></message>
    <message><source>Search registry keys…</source><translation>Buscar claves del Registro…</translation></message>
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
    <message><source>· %1 persistence %2</source><translation>· Persistencia %1: %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistencia %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Quitar selección</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Quitar las exclusiones seleccionadas. Surte efecto al aplicar.</translation></message>
    <message><source>Open containing folder</source><translation>Abrir carpeta contenedora</translation></message>
    <message><source>Copy file path</source><translation>Copiar ruta del archivo</translation></message>
    <message><source>Copy registry path</source><translation>Copiar ruta del Registro</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Confirmar en disco los cambios de esta carpeta en la superposición…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Confirmar en disco los cambios de este archivo en la superposición…</translation></message>
    <message><source>Copied to clipboard: </source><translation>Copiado al portapapeles: </translation></message>
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
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>entradas: %1 · Para añadir: %2 · Para quitar en la próxima sesión: %3 · Cambios pendientes: %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Administrador del filtro de escritura unificado (UWF)</translation></message>
    <message><source>Main toolbar</source><translation>Barra de herramientas principal</translation></message>
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
    <message><source>About this program.</source><translation>Acerca de UWF Manager.</translation></message>
    <message><source>Switch display language</source><translation>Cambiar idioma de visualización</translation></message>
    <message><source>Language</source><translation>Idioma</translation></message>
    <message><source>Theme</source><translation>Tema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Cambiar entre tema claro y oscuro</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Interfaz gráfica para administrar el filtro UWF, la superposición y las exclusiones de archivos y del Registro. La mayoría de los cambios se aplican tras el próximo reinicio.&lt;/p&gt;&lt;p&gt;Código fuente: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;Licencia: &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; El texto completo de la licencia está en el archivo LICENSE incluido.&lt;/p&gt;&lt;p&gt;Software libre: puede redistribuirlo y modificarlo según la GPL v3. Se proporciona «tal cual», sin garantía alguna.&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>Acerca de UWF Manager</translation></message>
    <message><source>System information</source><translation>Información del sistema</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Instantánea de diagnóstico</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>Diagnóstico del sistema, seguridad, UWF, modo avanzado y registros recientes.</translation></message>
    <message><source>Read only</source><translation>Solo lectura</translation></message>
    <message><source>Diagnostic report</source><translation>Informe de diagnóstico</translation></message>
    <message><source>Copied</source><translation>Copiado</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>Diagnóstico del sistema, seguridad, UWF, modo avanzado y registros recientes.</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>El informe puede contener nombres de cuenta, identificadores de dispositivo, rutas y registros recientes. Revíselo antes de compartirlo.</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;Este programa depende del filtro de escritura unificado de Windows (UWF).&lt;/b&gt; La protección la realiza UWF integrado en Windows. UWF Manager solo configura y administra UWF; no filtra escrituras. UWF debe estar instalado y activado en el sistema.&lt;/p&gt;&lt;p&gt;Al activar UWF por primera vez, se realizan estos cambios para mejorar el rendimiento:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Se desactivan los archivos de paginación.&lt;/li&gt;&lt;li&gt;Se desactiva Restaurar sistema.&lt;/li&gt;&lt;li&gt;Se desactiva SuperFetch.&lt;/li&gt;&lt;li&gt;Se desactiva el servicio de indexación de archivos.&lt;/li&gt;&lt;li&gt;Se desactiva el servicio de desfragmentación.&lt;/li&gt;&lt;li&gt;Se desactiva el inicio rápido.&lt;/li&gt;&lt;li&gt;El ajuste BCD bootstatuspolicy se establece en ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;Después puede cambiar estos ajustes. Por ejemplo, puede mover el archivo de paginación a un volumen no protegido y reactivar la paginación.&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>No se encontró el destino; no hay nada que confirmar en la superposición.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>El destino solo existe en la superposición: se creó bajo UWF y nunca se confirmó en disco o Registro persistente. Desaparecerá al reiniciar; confirmar su eliminación no es necesario ni posible.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>El sistema rechazó un parámetro (ruta o argumento no válido).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>La operación falló. Otro proceso puede estar usando el destino, como una carpeta abierta en el Explorador o un archivo abierto. Cierre esos programas y reintente.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>La operación falló; consulte el registro para más detalles.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operación rechazada (código %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Desconocido.</translation></message>
    <message><source>Commit canceled</source><translation>Confirmación cancelada</translation></message>
    <message><source>Commit result</source><translation>Resultado de confirmación</translation></message>
    <message><source>Succeeded</source><translation>Correcto</translation></message>
    <message><source>Skipped</source><translation>Omitido</translation></message>
    <message><source>Failed</source><translation>Error</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Correcto: %1; Omitido: %2; Error: %3.</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
Cancelado por el usuario; %1 entradas sin procesar.</translation></message>
    <message><source>Category</source><translation>Categoría</translation></message>
    <message><source>Path</source><translation>Ruta</translation></message>
    <message><source>Existed before</source><translation>Existía antes</translation></message>
    <message><source>Exists after</source><translation>Existe después</translation></message>
    <message><source>Error code</source><translation>Código de error</translation></message>
    <message><source>Reason</source><translation>Motivo</translation></message>
    <message><source>Previous page</source><translation>Página anterior</translation></message>
    <message><source>First page</source><translation>Primera página</translation></message>
    <message><source>Last page</source><translation>Última página</translation></message>
    <message><source>Next page</source><translation>Página siguiente</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>Página %1 / %2 · entradas: %3</translation></message>
    <message><source>Copy selected rows</source><translation>Copiar filas seleccionadas</translation></message>
    <message><source>Copy all</source><translation>Copiar todo</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Esta exclusión del Registro es necesaria mientras haya entradas de confirmación automática. Archivos para confirmar: Vaciar → Retirada de exclusión del Registro.</translation></message>
    <message><source>0 lines</source><translation>0 líneas</translation></message>
    <message><source>%1 lines</source><translation>%1 líneas</translation></message>
    <message><source>Loading log entries…</source><translation>Cargando registro…</translation></message>
    <message><source>No log entries</source><translation>No hay entradas</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>Página %1 / %2 · líneas: %3</translation></message>
    <message><source>Copy current page</source><translation>Copiar página actual</translation></message>
    <message><source>Clear</source><translation>Vaciar</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>Cambios pendientes: %1</translation></message>
    <message><source>No pending changes</source><translation>No hay cambios pendientes</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Exclusiones del Registro · Configuración global.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volumen %1: Protección UWF / Exclusiones de archivos / Archivos para confirmar.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: No compatible</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>Actualizar · Volumen: %1</translation></message>
    <message><source>· Filter (global) %1</source><translation>· Filtro (UWF): %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· Superposición · Tipo → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· Superposición · Tamaño máximo → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· Superposición · Umbral de advertencia → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· Superposición · Umbral crítico → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ Para cambiar el tipo y tamaño máximo:
1. Desactive el filtro arriba.
2. Reinicie para que quede desactivado.
3. Cambie el ajuste.</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· Volumen %1 · Protección UWF: %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· Volumen %1 · Identificar por: %2 (sin equivalente de CLI; solo en este programa)</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ Exclusiones de archivos  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− Exclusiones de archivos  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ Exclusiones del Registro  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− Exclusiones del Registro  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>Filtro (UWF): %1</translation></message>
    <message><source>Overlay type → %1</source><translation>Superposición · Tipo → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>Superposición · Tamaño máximo → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>Superposición · Umbral de advertencia → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>Superposición · Umbral crítico → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>Volumen %1 · Protección UWF: %2</translation></message>
    <message><source>File exclusion %1</source><translation>Exclusiones de archivos: %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>Exclusiones del Registro: %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>Cambios pendientes (%1)</translation></message>
    <message><source>Current session configuration</source><translation>Configuración de la sesión actual</translation></message>
    <message><source>Review and apply changes</source><translation>Revisar y aplicar cambios</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>Configuración completa como comandos uwfmgr. Los cambios pendientes se muestran primero en una sección separada. &lt;span style='color:%1'&gt;Cambios pendientes&lt;/span&gt;. &lt;span style='color:%2'&gt;Aplicar&lt;/span&gt;: Pulse para escribir los cambios en el sistema. La mayoría de los cambios se aplican tras el próximo reinicio.</translation></message>
    <message><source>Confirm apply</source><translation>Confirmar aplicación</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>&lt;span style='color:%1'&gt;Estos cambios se escribirán en el sistema&lt;/span&gt;. La mayoría de los cambios se aplican tras el próximo reinicio.&lt;br&gt;&lt;br&gt;Continuar?</translation></message>
    <message><source>Applied changes</source><translation>Cambios aplicados</translation></message>
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
    <message><source>Confirm safe shutdown?</source><translation>Apagado seguro?</translation></message>
    <message><source>Confirm safe restart?</source><translation>Reinicio seguro?</translation></message>
    <message><source>Shut down</source><translation>Apagar</translation></message>
    <message><source>Restart</source><translation>Reiniciar</translation></message>
    <message><source>Continue shutdown</source><translation>Continuar apagado</translation></message>
    <message><source>Continue restart</source><translation>Continuar reinicio</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>El sistema se apagará de forma segura mediante UWF.</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>El sistema se reiniciará de forma segura mediante UWF.</translation></message>
    <message><source>UWF protection</source><translation>Protección UWF</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>Disponible incluso con la superposición UWF llena.</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Calculando archivos para confirmar automáticamente…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>Se confirmarán automáticamente %1 archivos antes de la operación. Se omiten los archivos de volúmenes no protegidos actualmente.</translation></message>
    <message><source>Committing staged files</source><translation>Confirmando archivos preparados</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>La operación de energía continuará tras la confirmación automática.</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 de %2 archivos procesados.</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>Confirmación automática incompleta</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>Revise los detalles antes de decidir si continúa.</translation></message>
    <message><source>Operation details</source><translation>Detalles de la operación</translation></message>
    <message><source>No additional error details were provided.</source><translation>No se proporcionaron más detalles del error.</translation></message>
    <message><source>Some changes may not be preserved</source><translation>Es posible que algunos cambios no se conserven</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>Si continúa, el sistema se apagará sin conservar los archivos que no se pudieron confirmar.</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>Si continúa, el sistema se reiniciará sin conservar los archivos que no se pudieron confirmar.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>La confirmación automática se detuvo inesperadamente:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Confirmación automática incompleta: Error desconocido.</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>Los cambios sin confirmar se perderán</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>Antes de continuar, guarde su trabajo y confirme todos los cambios que quiera conservar permanentemente.</translation></message>
    <message><source>Confirm direct restart?</source><translation>Reinicio directo?</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>Apagado directo?</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>Reiniciar. Los archivos preparados no se confirmarán antes del reinicio.</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>Apagar. Los archivos preparados no se confirmarán antes del apagado.</translation></message>
    <message><source>File staging will be skipped</source><translation>Se omitirá la confirmación automática</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>Los archivos preparados no se confirmarán antes del reinicio.</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>Los archivos preparados no se confirmarán antes del apagado.</translation></message>
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
    <message><source>Commit failed</source><translation>Error al confirmar</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>La ruta no tiene letra de unidad. Identificación del volumen de destino: Error.</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>Error: Lectura de información del volumen (%1)</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>No hay registro de sesión actual para el volumen %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Esta ruta está en las exclusiones de archivos.
Exclusión: %1</translation></message>
    <message><source>No files were found under %1.</source><translation>No se encontraron archivos en %1.</translation></message>
    <message><source>Commit to disk</source><translation>Confirmar en disco</translation></message>
    <message><source>Committing…</source><translation>Confirmando…</translation></message>
    <message><source>This action cannot be undone.</source><translation>Esta acción no se puede deshacer.</translation></message>
    <message><source>Continue</source><translation>Continuar</translation></message>
    <message><source>Delete and commit</source><translation>Eliminar y confirmar</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>Confirmar en disco los cambios de este archivo en la superposición</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>Confirmar en disco los cambios de esta carpeta en la superposición</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>archivos: %1. Se incluyen todas las subcarpetas.</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>Eliminar este archivo y confirmar la eliminación en disco</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>Eliminar la carpeta y su contenido y confirmar las eliminaciones en disco</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>archivos: %1; carpetas: %2. Eliminar y confirmar.</translation></message>
    <message><source>Commit this registry value to disk</source><translation>Confirmar este valor del Registro en disco</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>Confirmar esta clave del Registro y todo su subárbol en disco</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>Valor: %1. (clave completa y todas sus subclaves)</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>Eliminar este valor del Registro y confirmar su eliminación en disco</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>Eliminar esta clave del Registro y su subárbol y confirmar las eliminaciones en disco</translation></message>
    <message><source>Scanning registry keys…</source><translation>Enumeración de claves del Registro…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>Enumeración de claves del Registro…
Clave: %1</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>Registro · Clave · Eliminar y confirmar:</translation></message>
    <message><source>Copy current entry</source><translation>Copiar entrada actual</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>Se eliminarán recursivamente %1 claves y todos sus valores.</translation></message>
    <message><source>Commit file deletion failed</source><translation>Error: Confirmación de eliminación del archivo</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>La ruta no existe; no hay nada que eliminar.</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Esta clave está en las exclusiones del Registro.
Exclusión: %1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>Error: Lectura del filtro del Registro (%1)</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>No hay registro del filtro del Registro para la sesión actual.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>El valor del Registro no existe; no hay nada que confirmar.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>La clave del Registro no existe; no hay nada que confirmar.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>El valor del Registro no existe; no hay nada que eliminar.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>La clave del Registro no existe; no hay nada que eliminar.</translation></message>
    <message><source>Export commands…</source><translation>Exportar comandos…</translation></message>
    <message><source>Export commands to file</source><translation>Exportar comandos a un archivo</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Exportar a archivo… · Comando: %1
%2</translation></message>
    <message><source>Import</source><translation>Importar</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Escriba o pegue un comando por línea. Cargue texto, registros o scripts (.txt, .bat, .ps1); se añaden las líneas con este comando. La importación prepara los cambios. Solo «Revisar y aplicar» en la barra de herramientas los escribe en el sistema.</translation></message>
    <message><source>Import uwfmgr commands</source><translation>uwfmgr · Importar</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;uwfmgr&lt;/b&gt;: Escriba o pegue un comando por línea. &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Cargar desde archivo…&lt;/b&gt;: Cargue texto, registros o scripts (.txt, .bat, .ps1); se añaden las líneas con este comando. &lt;code&gt;uwfmgr&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Importar&lt;/b&gt;: &lt;b&gt;La importación prepara los cambios. Solo «Revisar y aplicar» en la barra de herramientas los escribe en el sistema.&lt;/b&gt; &lt;b&gt;Revisar y aplicar&lt;/b&gt;.&lt;/p&gt;</translation></message>
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
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>Todos los archivos (*);;Archivos de texto (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
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
    <message><source>Status</source><translation>Estado</translation></message>
    <message><source>Command</source><translation>Comando</translation></message>
    <message><source>Detail</source><translation>Detalle</translation></message>
    <message><source>Import failed</source><translation>Error de importación</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Error interno: no hay controlador de aplicación registrado.</translation></message>
    <message><source>Nothing to import</source><translation>No hay comandos para importar</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>No se encontraron comandos uwfmgr en la entrada.</translation></message>
    <message><source>Applied: %1</source><translation>Aplicado: %1</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplicado: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>No compatible: %1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>Acumulado tras %1 lotes:</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Lote %1 ──</translation></message>
    <message><source>Applied</source><translation>Aplicado</translation></message>
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
    <message><source>Queued as a pending %1 change</source><translation>Cambios pendientes: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Ya está en el estado deseado; no se requiere acción</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>La ruta no pertenece al volumen o este no admite exclusiones (p. ej., exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Rechazado por la lista de bloqueo UWF (archivo de sistema, Windows, paginación, etc.)</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>Ya se procesó este comando en el mismo lote</translation></message>
    <message><source>Pending filter %1</source><translation>Cambios pendientes: Filtro %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>Filtro: Ya está en el estado deseado; no se requiere acción</translation></message>
    <message><source>Pending overlay type → %1</source><translation>Cambios pendientes: Superposición · Tipo → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>Superposición · Tipo: %1 (Ya está en el estado deseado; no se requiere acción)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valor no válido: Tamaño: %1</translation></message>
    <message><source>maximum size</source><translation>Tamaño máximo</translation></message>
    <message><source>warning threshold</source><translation>Umbral de advertencia</translation></message>
    <message><source>critical threshold</source><translation>Umbral crítico</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>Cambios pendientes: Superposición %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>Superposición %1: %2 MB (Ya está en el estado deseado; no se requiere acción)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Volumen %1 desconocido (no hay disco apto para UWF con esa letra)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>Cambios pendientes: Volumen %1 · Protección UWF: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volumen %1: Ya está en el estado deseado; no se requiere acción</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: La ruta no tiene letra de unidad. Identificación del volumen de destino: Error.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volumen %1: UWF No compatible</translation></message>
    <message><source>file exclusion</source><translation>Exclusiones de archivos</translation></message>
    <message><source>registry exclusion</source><translation>Exclusiones del Registro</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Las exclusiones del Registro requieren la pestaña Registro, que no está presente</translation></message>
    <message><source>Unsupported command</source><translation>Comando: No compatible</translation></message>
    <message><source>Exit</source><translation>Salir</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Mostrar u ocultar.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Activado</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Desactivado</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Desconocido</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Usado %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Modo avanzado</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordinar la confirmación automática con el apagado y reinicio de Windows.</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>Instala un servicio LocalSystem de inicio automático. Inicia UWF Manager con Windows e intenta confirmar los cambios de archivos preparados antes de apagar o reiniciar, para mejorar la compatibilidad de aplicaciones que entran en conflicto con UWF.</translation></message>
    <message><source>System changes</source><translation>Cambios en el sistema</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>Crear el servicio automático UWFManagerEnhanced asociado al ejecutable actual</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>Configuración: Registro → Confirmar en disco (HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>Iniciar UWF Manager tras el próximo inicio de Windows; mover el ejecutable puede impedir el inicio automático</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>Confirmar archivos preparados aptos durante el apagado o reinicio normal de Windows</translation></message>
    <message><source>Operational and security considerations</source><translation>Consideraciones de uso y seguridad</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>Registra un servicio en el Registro del sistema; la aplicación deja de ser totalmente portátil</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>Muchos archivos preparados pueden prolongar el apagado o reinicio; deje tiempo suficiente para conservar los cambios</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>El modo avanzado ejecuta UWF Manager como SYSTEM. Si un no administrador modifica el programa o lo sustituye por uno malicioso, podría obtener privilegios SYSTEM. Actívelo solo desde una ubicación protegida por permisos de administrador (recomendada: %ProgramFiles%\UWF Manager\Service\UWF.exe).</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>Error: Lectura del estado del modo avanzado</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Error: Lectura del estado del modo avanzado (Error desconocido).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Error: Lectura del estado del servicio:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Error: Lectura del estado del servicio (Error desconocido).</translation></message>
    <message><source>Status: Disabled</source><translation>Estado: Desactivado</translation></message>
    <message><source>Status: Enabled</source><translation>Estado: Activado</translation></message>
    <message><source>Status: Service stopped</source><translation>Estado: servicio detenido</translation></message>
    <message><source>Status: Repair required</source><translation>Estado: requiere reparación</translation></message>
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
    <message><source>Hide overlay hub</source><translation>Ocultar HUB de superposición</translation></message>
    <message><source>Exit application</source><translation>Salir de la aplicación</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Error: Conexión al sistema</translation></message>
    <message><source>Registry access failed</source><translation>Error: Acceso al Registro</translation></message>
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
    <message><source>The target state could not be read before the operation: %1</source><translation>No se pudo leer el estado del destino antes de la operación: %1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>La ruta no existe; no hay nada que eliminar.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Error desconocido.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>No se pudo confirmar el resultado al fallar la relectura del estado del destino: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>El proveedor aceptó la eliminación, pero la comprobación autoritativa confirma que el destino sigue existiendo.</translation></message>
    <message><source>Unknown</source><translation>Desconocido</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Error: Carga del registro de actividad (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Error: Carga del registro de actividad (Error desconocido).</translation></message>
    <message><source>: unknown error</source><translation>: Error desconocido</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>Error: Lectura del estado del destino (%1)</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>Error: Lectura del estado del destino (Error desconocido).</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>Error: Identificación del volumen de destino (%1)</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>Error: Enumeración de valores del Registro (%1)</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>Error: Enumeración de claves del Registro (%1)</translation></message>
    <message><source>Delete and commit failed</source><translation>Eliminar y confirmar: Error</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Error: Identificación del volumen de destino (%1): %2</translation></message>
    <message><source>%1/%2 slots</source><translation>%1/%2 ranuras</translation></message>
</context>
</TS>
