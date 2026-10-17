<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>Aktivieren</translation></message>
    <message><source>Disable</source><translation>Deaktivieren</translation></message>
    <message><source>enable</source><translation>aktivieren</translation></message>
    <message><source>disable</source><translation>deaktivieren</translation></message>
    <message><source>Enabled</source><translation>Aktiviert</translation></message>
    <message><source>Disabled</source><translation>Deaktiviert</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Abbrechen</translation></message>
    <message><source>Close</source><translation>Schließen</translation></message>
    <message><source>Refresh</source><translation>Aktualisieren</translation></message>
    <message><source>Apply</source><translation>Anwenden</translation></message>
    <message><source>Add</source><translation>Hinzufügen</translation></message>
    <message><source>Remove</source><translation>Entfernen</translation></message>
    <message><source>Yes</source><translation>Ja</translation></message>
    <message><source>No</source><translation>Nein</translation></message>
    <message><source>Excluded</source><translation>Ausgenommen</translation></message>
    <message><source>Not excluded</source><translation>Nicht ausgenommen</translation></message>
    <message><source>File…</source><translation>Datei…</translation></message>
    <message><source>Folder…</source><translation>Ordner…</translation></message>
    <message><source>volume ID</source><translation>Volume-ID</translation></message>
    <message><source>drive letter</source><translation>Laufwerksbuchstabe</translation></message>
    <message><source>Drive letter</source><translation>Laufwerksbuchstabe</translation></message>
    <message><source>Volume ID</source><translation>Volume-ID</translation></message>
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>„%1“ (%2) wird nicht als unterstützte Windows-10/11-Edition erkannt. UWF Manager läuft im Kompatibilitätsmodus; manche Funktionen sind eventuell nicht verfügbar.</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Schutzstatus dieses Volumes in der aktuellen Sitzung (schreibgeschützt).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Dieses Volume in der nächsten Sitzung schützen. Schreibzugriffe werden ins Overlay umgeleitet und beim Neustart verworfen.</translation></message>
    <message><source>Protection:</source><translation>Schutz:</translation></message>
    <message><source>Current session</source><translation>Aktuelle Sitzung</translation></message>
    <message><source>Next session</source><translation>Nächste Sitzung</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>Derzeit aktive Sitzung (schreibgeschützt). Änderungen werden in dieser Sitzung nicht wirksam.</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>Sitzung nach dem Neustart. Änderungen werden nach dem Systemneustart wirksam.</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Volume-Erkennung: Laufwerksbuchstaben sind einfach, die Zuordnung geht bei einer Neuzuweisung aber verloren, etwa nach Festplattenänderungen. Die Volume-ID bleibt bei Buchstabenänderungen stabil.</translation></message>
    <message><source>Bind by:</source><translation>Zuordnung über:</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF unterstützt dieses Volume nicht: </translation></message>
    <message><source>Global settings</source><translation>Globale Einstellungen</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>UWF-Filterstatus in der aktuellen Sitzung (schreibgeschützt).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>UWF-Filter in der nächsten Sitzung aktivieren. Schreibzugriffe auf geschützte Volumes gehen ins Overlay und werden beim Neustart verworfen.</translation></message>
    <message><source>Filter</source><translation>Filter</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Typ und Maximalgröße ändern:
1. Filter oben deaktivieren.
2. Neu starten, damit der Filter ausgeschaltet ist.
3. Einstellung ändern.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Overlay-Speicherort: RAM ist schneller, belegt aber Arbeitsspeicher. Disk nutzt das Systemlaufwerk und bietet mehr Platz. Beide werden beim Neustart verworfen.</translation></message>
    <message><source>Type</source><translation>Typ</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Maximale Overlay-Kapazität. RAM ist durch den Arbeitsspeicher begrenzt. Disk benötigt mindestens 1024 MB und genügend freien Platz auf dem Systemvolume.</translation></message>
    <message><source>Maximum size</source><translation>Maximale Größe</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Maximale Größe · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Warnschwelle</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>Erzeugt bei diesem Overlay-Verbrauch eine Warnung im Ereignisprotokoll. Muss unter der kritischen Schwelle liegen. 0 deaktiviert das Ereignis.</translation></message>
    <message><source>Critical threshold</source><translation>Kritische Schwelle</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>Erzeugt bei diesem Overlay-Verbrauch ein kritisches Ereignis im Ereignisprotokoll. Muss über der Warnschwelle liegen. 0 deaktiviert das Ereignis.</translation></message>
    <message><source>Used / total</source><translation>Belegt / gesamt</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; Belegt &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warnung &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Kritisch</translation></message>
    <message><source>UWF status unavailable: </source><translation>UWF-Status nicht verfügbar: </translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>Zum Ändern der UWF-Einstellungen sind Administratorrechte nötig. Starten Sie das Programm per Rechtsklick → „Als Administrator ausführen“ neu.</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Laufwerkstyp nicht unterstützt (nur lokale Festplatten).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Das Volume überschreitet die UWF-Grenze von 16 TiB pro geschütztem Volume.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: Volumeschutz möglich, aber keine Dateiausnahmen oder dateiweise Übernahme.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Fehlgeschlagen: Volumeinformationen lesen.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 frei / %2</translation></message>
    <message><source>Commit</source><translation>Übernehmen</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Overlay-Änderungen dieser Datei auf Datenträger übernehmen / Registrierungsänderungen übernehmen. Dieser Vorgang kann nicht rückgängig gemacht werden.</translation></message>
    <message><source>Commit file changes…</source><translation>Dateiänderungen übernehmen…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Datei zur dauerhaften Übernahme auswählen. Overlay-Änderungen dieser Datei auf Datenträger übernehmen.</translation></message>
    <message><source>Commit folder changes…</source><translation>Ordneränderungen übernehmen…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Ordner zur dauerhaften Übernahme auswählen. Overlay-Änderungen dieses Ordners auf Datenträger übernehmen. Alle Unterordner werden einbezogen.</translation></message>
    <message><source>Delete and commit file…</source><translation>Datei löschen und die Löschung dauerhaft übernehmen…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Datei zur dauerhaften Löschung auswählen. Datei löschen und die Löschung dauerhaft übernehmen.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Ordner samt Inhalt löschen und dauerhaft übernehmen…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Ordner zur dauerhaften Löschung auswählen. Ordner samt Inhalt löschen und dauerhaft übernehmen. Alle Unterordner werden einbezogen.</translation></message>
    <message><source>Commit registry changes…</source><translation>Registrierungsänderungen übernehmen…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Registrierungspfad und optional einen Wertnamen eingeben. Ohne Wertnamen werden alle Werte des Schlüssels und sämtlicher Unterschlüssel dauerhaft übernommen.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Registrierungslöschung übernehmen…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Zu löschenden Registrierungspfad und optional Wertnamen eingeben. Ohne Wertnamen werden der gesamte Schlüssel, alle Werte und Unterschlüssel rekursiv gelöscht und die Löschungen dauerhaft übernommen.</translation></message>
    <message><source>File exclusions</source><translation>Dateiausnahmen</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Dateien und Ordner dieses Volumes ohne UWF-Schutz. Ein Doppelklick kopiert den Pfad.</translation></message>
    <message><source>Registry exclusions</source><translation>Registrierungsausnahmen</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Registrierungsausnahmen gelten global und folgen dem UWF-Filter, nicht dem Schutz dieses Volumes. Die gemeinsame Liste erscheint nur einmal in den Volume-Tabs. Doppelklick kopiert den Pfad.</translation></message>
    <message><source>File staging</source><translation>Automatische Dateiübernahme</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>Dateien / Ordner: Vor sicherem Herunterfahren oder Neustart automatisch dauerhaft übernehmen. Alle Unterordner werden einbezogen. Solange die Liste Einträge enthält, bleibt sie durch folgende Registrierungsausnahme auch über geschützte Sitzungen erhalten: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Search staged paths…</source><translation>Automatische Übernahmepfade suchen…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Dateien oder einen Ordner zur automatischen Übernahme hinzufügen.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Datei zur dauerhaften Übernahme auswählen. Vor sicherem Herunterfahren oder Neustart automatisch dauerhaft übernehmen.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Ordner zur dauerhaften Übernahme auswählen. Vor sicherem Herunterfahren oder Neustart automatisch dauerhaft übernehmen. Alle Unterordner werden einbezogen.</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>Ausgewählte Pfade sofort aus der automatischen Übernahme entfernen.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Datei zur dauerhaften Übernahme auswählen · Automatische Dateiübernahme</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Ordner zur dauerhaften Übernahme auswählen · Automatische Dateiübernahme · Alle Unterordner werden einbezogen.</translation></message>
    <message><source>Cannot stage this path</source><translation>Pfad kann nicht zur automatischen Übernahme hinzugefügt werden</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Dieser Pfad überschneidet sich mit einer UWF-Dateiausnahme. Entfernen Sie zuerst die Ausnahme:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Dieser Pfad überschneidet sich mit der automatischen Dateiübernahme. Entfernen Sie zuerst den Eintrag:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Pfad überschneidet sich mit einem vorhandenen Übernahmeeintrag</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Diese Registrierungsausnahme ist erforderlich, solange die Übernahmeliste Einträge enthält</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Pfad existiert nicht oder hat nicht den erwarteten Dateityp:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Pfad konnte nicht geprüft werden:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Analysepunkte können nicht automatisch übernommen werden:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Ohne Laufwerksbuchstaben ist keine UWF-Übernahme möglich:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Dieser Pfad liegt auf Volume %1. Fügen Sie ihn im dortigen Übernahme-Tab hinzu.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Das Volume-Stammverzeichnis kann nicht hinzugefügt werden, da sonst das gesamte Volume rekursiv übernommen würde.</translation></message>
    <message><source>File staging could not be saved</source><translation>Fehlgeschlagen: Automatische Dateiübernahmeliste speichern</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Die Übernahmeliste wurde neu geladen, da die Aktualisierung scheiterte:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Automatische Dateiübernahme: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Vor sicherem Herunterfahren oder Neustart automatisch dauerhaft übernehmen.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Automatische Dateiübernahme: %1 · Dateien: %2 · Ordner: %3</translation></message>
    <message><source>Required by UWF for File staging.</source><translation>Für die automatische UWF-Dateiübernahme erforderlich.</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Der UWF-Filter ist deaktiviert. Es gibt keine gesammelten Overlay-Änderungen zu übernehmen.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Das Dateisystem %1 unterstützt keine dateiweise Übernahme.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Dieses Volume wird von UWF nicht unterstützt.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Dieses Volume unterstützt keine Dateiübernahme. Nur Registrierungsübernahme möglich (Registrierungsausnahmen gelten global).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Das Dateisystem %1 unterstützt keine dateiweise Übernahme. Nur Registrierungsübernahme möglich (Registrierungsausnahmen gelten global).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Dieses Volume ist derzeit nicht UWF-geschützt; es gibt keine Dateiänderungen zu übernehmen.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Dieses Volume ist derzeit nicht UWF-geschützt; es gibt keine Dateiänderungen zu übernehmen. Nur Registrierungsübernahme möglich (Registrierungsausnahmen gelten global).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Datei zur dauerhaften Übernahme auswählen</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Ordner zur dauerhaften Übernahme auswählen</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Datei zur dauerhaften Löschung auswählen</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Ordner zur dauerhaften Löschung auswählen</translation></message>
    <message><source>Commit registry changes</source><translation>Registrierungsänderungen übernehmen</translation></message>
    <message><source>Commit registry deletion</source><translation>Registrierungslöschung übernehmen</translation></message>
    <message><source>Key path:</source><translation>Schlüssel · Pfad:</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>HKLM\Software\... eingeben oder einfügen, mit Enter öffnen</translation></message>
    <message><source>Name</source><translation>Name</translation></message>
    <message><source>Data</source><translation>Daten</translation></message>
    <message><source>Key: %1</source><translation>Schlüssel: %1</translation></message>
    <message><source>Value: %1</source><translation>Wert: %1</translation></message>
    <message><source>(none)</source><translation>(keine)</translation></message>
    <message><source>(Default)</source><translation>(Standard)</translation></message>
    <message><source>(value not set)</source><translation>(Wert nicht festgelegt)</translation></message>
    <message><source>(empty)</source><translation>(leer)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Unbekannt (Wert: Datei zu groß)</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>(gesamter Schlüssel mit Unterschlüsseln)</translation></message>
    <message><source>Overlay files - %1</source><translation>Overlay · Dateien - %1</translation></message>
    <message><source>View overlay files</source><translation>Overlay-Dateien anzeigen</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Overlay-Dateien anzeigen. &lt;b&gt;Nur eine Diagnoseaufnahme.&lt;/b&gt; Nicht zur Auswahl der zu übernehmenden Dateien verwenden.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Overlay-Dateilisten werden nur auf NTFS-Volumes unterstützt.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>Der UWF-Filter ist deaktiviert; kein Volume hat ein prüfbares Overlay.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Dieses Volume ist nicht UWF-geschützt und hat kein prüfbares Overlay.</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;Nur eine Diagnoseaufnahme.&lt;/b&gt; Nicht zur Auswahl der zu übernehmenden Dateien verwenden.</translation></message>
    <message><source>NTFS volumes only.</source><translation>Overlay-Dateilisten werden nur auf NTFS-Volumes unterstützt.</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>Große Overlays benötigen mehr Speicher und Zeit; der Vorgang kann langsam sein oder scheitern.</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>Die Liste ist ungenau: Dateien unter Clustergröße (meist 4 KB) können fehlen. Frühere Übernahmen, ausgenommene Pfade oder andere Vorgänge können zusätzliche Einträge verursachen.</translation></message>
    <message><source>Loading…</source><translation>Wird geladen…</translation></message>
    <message><source>Failed: %1</source><translation>Fehlgeschlagen: %1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>Overlay · Dateien: %1</translation></message>
    <message><source>No files</source><translation>Keine Dateien</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>Seite %1 / %2 · Dateien: %3</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Der WMI-Anbieter ist beim Auflisten abgestürzt. Dies ist eine bekannte Instabilität von UWF_Overlay.GetOverlayFiles bei großen Overlays oder hoher E/A-Last.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Nicht genügend Speicher oder Vorgang vom Anbieter nicht unterstützt. Erfordert NTFS und Speicherreserven. Mit kleinerem Overlay oder nach Schließen speicherintensiver Programme erneut versuchen.</translation></message>
    <message><source>Raw error: %1</source><translation>Fehlercode: %1</translation></message>
    <message><source>Export to file…</source><translation>In Datei exportieren…</translation></message>
    <message><source>Export overlay file list</source><translation>In Datei exportieren… · Overlay · Dateien</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Textdateien (*.txt);;Alle Dateien (*)</translation></message>
    <message><source>Export failed</source><translation>Export fehlgeschlagen</translation></message>
    <message><source>Export finished</source><translation>Export abgeschlossen</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Fehlgeschlagen: Datei öffnen (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Fehlgeschlagen: Datei schreiben (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Konfiguration: Datei schreiben · Einträge: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Ein ganzes Volume (%1) kann nicht ausgenommen werden.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Auslagerungs-, Swap- und Ruhezustandsdateien können nicht ausgenommen werden, da UWF selbst davon abhängt.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Das gesamte Verzeichnis kann nicht ausgenommen werden: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Das gesamte Verzeichnis kann nicht ausgenommen werden: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Das gesamte Verzeichnis kann nicht ausgenommen werden: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Diese wichtige Systemdatei kann nicht ausgenommen werden: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Die benutzerspezifische Registrierungsdatei NTUSER.DAT kann nicht ausgenommen werden.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Der Registrierungsschlüssel darf nicht leer sein.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Ein Registrierungsschlüssel darf nicht mit einem Rückstrich beginnen.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Der Pfad enthält aufeinanderfolgende Rückstriche und ist ungültig.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Registrierungspfade verwenden `\` als Trennzeichen, nicht `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Der Pfad enthält unsichtbare Steuerzeichen und ist ungültig.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>Das Domänen-Computerkontogeheimnis darf laut UWF-Dokumentation nicht ausgenommen werden: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Nur bestimmte Unterschlüssel der folgenden Hauptschlüssel können ausgenommen werden:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Search file paths…</source><translation>Dateipfade suchen…</translation></message>
    <message><source>Search registry keys…</source><translation>Registrierungsschlüssel suchen…</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Dateien oder Ordner zur Ausnahmeliste hinzufügen. Ausnahmen werden nicht durch UWF geschützt.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Datei zum Hinzufügen auswählen · Dateiausnahmen.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Ordner zum Hinzufügen auswählen · Dateiausnahmen. Alle Unterordner werden einbezogen.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Pfad des auszunehmenden Registrierungsschlüssels eingeben.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Pfad des auszunehmenden Registrierungsschlüssels eingeben. / Dauerhafte Speicherung.</translation></message>
    <message><source>Registry key…</source><translation>Registrierung · Schlüssel…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Domänengeheimnis (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Terminaldienste-Clientzugriffslizenz (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Domänengeheimnis (Computerkontokennwort) über UWF-Sitzungen hinweg erhalten.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Terminaldienste-Clientzugriffslizenzen über UWF-Sitzungen hinweg erhalten.</translation></message>
    <message><source>· %1 persistence %2</source><translation>· Dauerhafte Speicherung %1: %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Dauerhafte Speicherung %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Auswahl entfernen</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Ausgewählte Ausnahmen entfernen. Wird nach dem Anwenden wirksam.</translation></message>
    <message><source>Open containing folder</source><translation>Übergeordneten Ordner öffnen</translation></message>
    <message><source>Copy file path</source><translation>Dateipfad kopieren</translation></message>
    <message><source>Copy registry path</source><translation>Registrierungspfad kopieren</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Overlay-Änderungen dieses Ordners auf Datenträger übernehmen…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Overlay-Änderungen dieser Datei auf Datenträger übernehmen…</translation></message>
    <message><source>Copied to clipboard: </source><translation>In die Zwischenablage kopiert: </translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Datei zum Hinzufügen auswählen · Dateiausnahmen. Mehrfachauswahl möglich.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Ordner zum Hinzufügen auswählen · Dateiausnahmen</translation></message>
    <message><source>Add registry exclusion</source><translation>Registrierungsausnahme hinzufügen</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Pfad %1 liegt nicht auf Volume %2 und kann dort nicht als Ausnahme hinzugefügt werden.</translation></message>
    <message><source>Path is not on this volume</source><translation>Pfad liegt nicht auf diesem Volume</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Ausnahme kann nicht hinzugefügt werden</translation></message>
    <message><source>File: %1</source><translation>Datei… %1</translation></message>
    <message><source>Folder: %1</source><translation>Ordner… %1</translation></message>
    <message><source>Registry: %1</source><translation>Registrierung: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Standardmäßig ausgenommen; jederzeit änderbar.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Aktuelle Sitzung: %1
Nächste Sitzung: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Ausstehende Änderungen: %1</translation></message>
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>Einträge: %1 · Hinzuzufügen: %2 · In nächster Sitzung entfernen: %3 · Ausstehende Änderungen: %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Verwaltung des Unified Write Filter (UWF)</translation></message>
    <message><source>Main toolbar</source><translation>Hauptsymbolleiste</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>UWF-Status der aktuellen Sitzung und Konfiguration der nächsten Sitzung neu lesen.</translation></message>
    <message><source>Review and apply</source><translation>Prüfen und anwenden</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Alle ausstehenden Änderungen prüfen und zusammen anwenden. Die meisten Änderungen werden nach dem nächsten Neustart wirksam.</translation></message>
    <message><source>Safe shutdown</source><translation>Sicher herunterfahren</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Das System wird über UWF sicher heruntergefahren. Auch bei vollem UWF-Overlay verfügbar.</translation></message>
    <message><source>Safe restart</source><translation>Sicher neu starten</translation></message>
    <message><source>Direct restart</source><translation>Direkt neu starten</translation></message>
    <message><source>Direct shutdown</source><translation>Direkt herunterfahren</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Das System wird über UWF sicher neu gestartet. Auch bei vollem UWF-Overlay verfügbar.</translation></message>
    <message><source>Log</source><translation>Protokoll</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Internes Protokoll dieser Sitzung zur Fehlersuche anzeigen.</translation></message>
    <message><source>About</source><translation>Über</translation></message>
    <message><source>About this program.</source><translation>Über UWF Manager.</translation></message>
    <message><source>Switch display language</source><translation>Anzeigesprache wechseln</translation></message>
    <message><source>Language</source><translation>Sprache</translation></message>
    <message><source>Theme</source><translation>Design</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Helles/dunkles Design wechseln</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Grafische Oberfläche zum Verwalten von UWF-Filterstatus, Overlay sowie Datei- und Registrierungsausnahmen. Die meisten Änderungen werden nach dem nächsten Neustart wirksam.&lt;/p&gt;&lt;p&gt;Quellcode: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;Lizenz: &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; Der vollständige Lizenztext liegt dem Programm in der Datei LICENSE bei.&lt;/p&gt;&lt;p&gt;Freie Software: Weitergabe und Änderung sind unter GPL v3 erlaubt. Bereitstellung „wie besehen“, ohne jegliche Gewährleistung.&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>Über UWF Manager</translation></message>
    <message><source>System information</source><translation>Systeminformationen</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Diagnosesnapshot</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>Diagnose von System, Sicherheit, UWF, erweitertem Modus und aktuellen Anwendungsprotokollen.</translation></message>
    <message><source>Read only</source><translation>Schreibgeschützt</translation></message>
    <message><source>Diagnostic report</source><translation>Diagnosebericht</translation></message>
    <message><source>Copied</source><translation>Kopiert</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>Diagnose von System, Sicherheit, UWF, erweitertem Modus und aktuellen Anwendungsprotokollen.</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>Der Bericht kann Kontonamen, Gerätekennungen, Dateipfade und aktuelle Protokolle enthalten. Vor dem Teilen prüfen.</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;Dieses Programm benötigt den Windows Unified Write Filter (UWF).&lt;/b&gt; Den Schreibschutz übernimmt die Windows-Funktion UWF. UWF Manager filtert nicht selbst, sondern konfiguriert und verwaltet UWF. UWF muss im System installiert und aktiviert sein.&lt;/p&gt;&lt;p&gt;Beim ersten Aktivieren von UWF werden zur Leistungsverbesserung folgende Systemeinstellungen geändert:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Auslagerungsdateien werden deaktiviert.&lt;/li&gt;&lt;li&gt;Die Systemwiederherstellung wird deaktiviert.&lt;/li&gt;&lt;li&gt;SuperFetch wird deaktiviert.&lt;/li&gt;&lt;li&gt;Der Dateiindizierungsdienst wird abgeschaltet.&lt;/li&gt;&lt;li&gt;Der Defragmentierungsdienst wird abgeschaltet.&lt;/li&gt;&lt;li&gt;Der Schnellstart wird deaktiviert.&lt;/li&gt;&lt;li&gt;Die BCD-Einstellung bootstatuspolicy wird auf ignoreallfailures gesetzt.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;Danach können diese Einstellungen angepasst werden. Beispielsweise lässt sich die Auslagerungsdatei auf ein ungeschütztes Volume verschieben und die Auslagerung wieder aktivieren.&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Ziel nicht gefunden; im Overlay gibt es nichts zu übernehmen.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Das Ziel existiert nur im Overlay, nicht auf Datenträger bzw. in der dauerhaften Registrierung. Es wurde unter UWF erstellt, nie übernommen und verschwindet beim Neustart. Eine Löschübernahme ist weder nötig noch möglich.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Das System hat einen Parameter abgelehnt (ungültiger Pfad oder ungültiges Argument).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Vorgang fehlgeschlagen. Ein anderes Programm könnte das Ziel verwenden, etwa Explorer oder eine geöffnete Datei. Schließen Sie zugreifende Programme und versuchen Sie es erneut.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Vorgang fehlgeschlagen; Details stehen im Protokoll.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Vorgang abgelehnt (Code %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Unbekannt.</translation></message>
    <message><source>Commit canceled</source><translation>Übernahme abgebrochen</translation></message>
    <message><source>Commit result</source><translation>Übernahmeergebnis</translation></message>
    <message><source>Succeeded</source><translation>Erfolgreich</translation></message>
    <message><source>Skipped</source><translation>Übersprungen</translation></message>
    <message><source>Failed</source><translation>Fehlgeschlagen</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Erfolgreich: %1; Übersprungen: %2; Fehlgeschlagen: %3.</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
Vom Benutzer abgebrochen; %1 Einträge nicht verarbeitet.</translation></message>
    <message><source>Category</source><translation>Kategorie</translation></message>
    <message><source>Path</source><translation>Pfad</translation></message>
    <message><source>Existed before</source><translation>Vorher vorhanden</translation></message>
    <message><source>Exists after</source><translation>Danach vorhanden</translation></message>
    <message><source>Error code</source><translation>Fehlercode</translation></message>
    <message><source>Reason</source><translation>Grund</translation></message>
    <message><source>Previous page</source><translation>Vorherige Seite</translation></message>
    <message><source>First page</source><translation>Erste Seite</translation></message>
    <message><source>Last page</source><translation>Letzte Seite</translation></message>
    <message><source>Next page</source><translation>Nächste Seite</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>Seite %1 / %2 · Einträge: %3</translation></message>
    <message><source>Copy selected rows</source><translation>Ausgewählte Zeilen kopieren</translation></message>
    <message><source>Copy all</source><translation>Alles kopieren</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Diese Registrierungsausnahme ist erforderlich, solange die Übernahmeliste Einträge enthält. Automatische Dateiübernahme: Leeren → Registrierungsausnahme entfernen.</translation></message>
    <message><source>0 lines</source><translation>0 Zeilen</translation></message>
    <message><source>%1 lines</source><translation>%1 Zeilen</translation></message>
    <message><source>Loading log entries…</source><translation>Protokolleinträge werden geladen…</translation></message>
    <message><source>No log entries</source><translation>Keine Protokolleinträge</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>Seite %1 / %2 · Zeilen: %3</translation></message>
    <message><source>Copy current page</source><translation>Aktuelle Seite kopieren</translation></message>
    <message><source>Clear</source><translation>Leeren</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>Ausstehende Änderungen: %1</translation></message>
    <message><source>No pending changes</source><translation>Keine ausstehenden Änderungen</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Registrierungsausnahmen · Globale Einstellungen.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: UWF-Schutz / Dateiausnahmen / Automatische Dateiübernahme.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Nicht unterstützt</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>Aktualisieren · Volume: %1</translation></message>
    <message><source>· Filter (global) %1</source><translation>· Filter (UWF): %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· Overlay · Typ → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· Overlay · Maximale Größe → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· Overlay · Warnschwelle → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· Overlay · Kritische Schwelle → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ Typ und Maximalgröße ändern:
1. Filter oben deaktivieren.
2. Neu starten, damit der Filter ausgeschaltet ist.
3. Einstellung ändern.</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· Volume %1 · UWF-Schutz: %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· Volume %1 · Zuordnung über: %2 (kein CLI-Äquivalent; nur in diesem Programm)</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ Dateiausnahmen  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− Dateiausnahmen  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ Registrierungsausnahmen  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− Registrierungsausnahmen  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>Filter (UWF): %1</translation></message>
    <message><source>Overlay type → %1</source><translation>Overlay · Typ → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>Overlay · Maximale Größe → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>Overlay · Warnschwelle → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>Overlay · Kritische Schwelle → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>Volume %1 · UWF-Schutz: %2</translation></message>
    <message><source>File exclusion %1</source><translation>Dateiausnahmen: %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>Registrierungsausnahmen: %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>Ausstehende Änderungen (%1)</translation></message>
    <message><source>Current session configuration</source><translation>Konfiguration der aktuellen Sitzung</translation></message>
    <message><source>Review and apply changes</source><translation>Änderungen prüfen und anwenden</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>Vollständige Konfiguration als uwfmgr-Befehle. Ausstehende Änderungen stehen zuerst in einem eigenen Abschnitt. &lt;span style='color:%1'&gt;Ausstehende Änderungen&lt;/span&gt;. &lt;span style='color:%2'&gt;Anwenden&lt;/span&gt;: Schreibt die Änderungen beim Anklicken ins System. Die meisten Änderungen werden nach dem nächsten Neustart wirksam.</translation></message>
    <message><source>Confirm apply</source><translation>Anwenden bestätigen</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>&lt;span style='color:%1'&gt;Diese Änderungen werden ins System geschrieben&lt;/span&gt;. Die meisten Änderungen werden nach dem nächsten Neustart wirksam.&lt;br&gt;&lt;br&gt;Weiter?</translation></message>
    <message><source>Applied changes</source><translation>Angewendete Änderungen</translation></message>
    <message><source>Result</source><translation>Ergebnis</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filter: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Warnschwelle festlegen: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Kritische Schwelle festlegen: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Overlay-Typ festlegen / Maximale Größe festlegen: Typ und Maximalgröße ändern:
1. Filter oben deaktivieren.
2. Neu starten, damit der Filter ausgeschaltet ist.
3. Einstellung ändern.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Overlay-Typ festlegen: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Maximale Größe festlegen: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Maximale Größe festlegen: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · UWF-Schutz: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Zuordnung über: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1 · Dateiausnahme hinzufügen: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1 · Dateiausnahme entfernen: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Registrierungsausnahme hinzufügen: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Registrierungsausnahme entfernen: %1</translation></message>
    <message><source>Time</source><translation>Zeit</translation></message>
    <message><source>Level</source><translation>Stufe</translation></message>
    <message><source>Tag</source><translation>Tag</translation></message>
    <message><source>Message</source><translation>Meldung</translation></message>
    <message><source>Confirm safe shutdown?</source><translation>Sicher herunterfahren?</translation></message>
    <message><source>Confirm safe restart?</source><translation>Sicher neu starten?</translation></message>
    <message><source>Shut down</source><translation>Herunterfahren</translation></message>
    <message><source>Restart</source><translation>Neu starten</translation></message>
    <message><source>Continue shutdown</source><translation>Herunterfahren fortsetzen</translation></message>
    <message><source>Continue restart</source><translation>Neustart fortsetzen</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>Das System wird über UWF sicher heruntergefahren.</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>Das System wird über UWF sicher neu gestartet.</translation></message>
    <message><source>UWF protection</source><translation>UWF-Schutz</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>Auch bei vollem UWF-Overlay verfügbar.</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Dateien für automatische Übernahme werden ermittelt…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>Vor diesem Vorgang werden %1 Dateien automatisch übernommen. Dateien außerhalb aktuell geschützter Volumes werden übersprungen.</translation></message>
    <message><source>Committing staged files</source><translation>Dateien werden automatisch übernommen</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>Nach der automatischen Dateiübernahme wird der Ein-/Ausschaltvorgang fortgesetzt.</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 von %2 Dateien verarbeitet.</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>Automatische Dateiübernahme nicht abgeschlossen</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>Prüfen Sie die Vorgangsdetails, bevor Sie fortfahren.</translation></message>
    <message><source>Operation details</source><translation>Vorgangsdetails</translation></message>
    <message><source>No additional error details were provided.</source><translation>Keine zusätzlichen Fehlerdetails verfügbar.</translation></message>
    <message><source>Some changes may not be preserved</source><translation>Einige Änderungen bleiben möglicherweise nicht erhalten</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>Beim Fortfahren wird heruntergefahren, ohne Änderungen an nicht übernommenen Dateien zu erhalten.</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>Beim Fortfahren wird neu gestartet, ohne Änderungen an nicht übernommenen Dateien zu erhalten.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Automatische Dateiübernahme unerwartet beendet:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Automatische Dateiübernahme nicht abgeschlossen: Unbekannter Fehler.</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>Nicht übernommene Änderungen gehen verloren</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>Speichern Sie Ihre Arbeit und übernehmen Sie alle dauerhaft benötigten Änderungen, bevor Sie fortfahren.</translation></message>
    <message><source>Confirm direct restart?</source><translation>Direkt neu starten?</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>Direkt herunterfahren?</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>Neu starten. Dateien der Übernahmeliste werden vor dem Neustart nicht übernommen.</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>Herunterfahren. Dateien der Übernahmeliste werden vor dem Herunterfahren nicht übernommen.</translation></message>
    <message><source>File staging will be skipped</source><translation>Automatische Dateiübernahme wird übersprungen</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>Dateien der Übernahmeliste werden vor dem Neustart nicht übernommen.</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>Dateien der Übernahmeliste werden vor dem Herunterfahren nicht übernommen.</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Direkt neu starten: Automatische Dateiübernahme wird übersprungen.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Direkt herunterfahren: Automatische Dateiübernahme wird übersprungen.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Automatische Dateiübernahme wegen direktem Ein-/Ausschaltvorgang übersprungen.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Sicher herunterfahren: Fehlgeschlagen</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Herunterfahren: Fehlgeschlagen (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Sicher neu starten: Fehlgeschlagen</translation></message>
    <message><source>Direct restart failed</source><translation>Direkt neu starten: Fehlgeschlagen</translation></message>
    <message><source>Direct shutdown failed</source><translation>Direkt herunterfahren: Fehlgeschlagen</translation></message>
    <message><source>Restart failed: %1</source><translation>Neu starten: Fehlgeschlagen (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Übernahmeliste konnte nicht gelesen werden:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Übernahmeliste konnte nicht gelesen werden: Unbekannter Fehler.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>Dateien: %1 · Erfolgreich: %2 · Übersprungen (Dateien): %3 · Übersprungen (Einträge): %4 · Fehlgeschlagen: %5</translation></message>
    <message><source>File staging preparation</source><translation>Automatische Dateiübernahme: Vorgangsdetails</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Analysepunkte werden nicht unterstützt.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Nur absolute Pfade auf lokalen Volumes können hinzugefügt werden.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Nur absolute Pfade auf lokalen Volumes können hinzugefügt werden.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Der Übernahmepfad konnte nicht geprüft werden.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Der vorgemerkte Dateipfad hat nun einen anderen Typ.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Der vorgemerkte Ordnerpfad hat nun einen anderen Typ.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Der vorgemerkte Ordner konnte nicht vollständig aufgelistet werden.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Der Pfad hat keinen Laufwerksbuchstaben.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>Das Volume-Stammverzeichnis darf nicht rekursiv verarbeitet werden.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Der UWF-Anbieter hat die Übernahme abgelehnt.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Übernahme fehlgeschlagen: Unbekannter Fehler.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>%1 weitere Fehler wurden protokolliert.</translation></message>
    <message><source>Commit failed</source><translation>Übernahme fehlgeschlagen</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Der Pfad hat keinen Laufwerksbuchstaben. Zielvolume ermitteln: Fehlgeschlagen.</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>Fehlgeschlagen: Volumeinformationen lesen (%1)</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>Für Volume %1 wurde kein Datensatz der aktuellen Sitzung gefunden.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Dieser Pfad ist in der Dateiausnahmeliste.
Ausnahme: %1</translation></message>
    <message><source>No files were found under %1.</source><translation>Unter %1 wurden keine Dateien gefunden.</translation></message>
    <message><source>Commit to disk</source><translation>Auf Datenträger übernehmen</translation></message>
    <message><source>Committing…</source><translation>Wird übernommen…</translation></message>
    <message><source>This action cannot be undone.</source><translation>Dieser Vorgang kann nicht rückgängig gemacht werden.</translation></message>
    <message><source>Continue</source><translation>Weiter</translation></message>
    <message><source>Delete and commit</source><translation>Löschen und übernehmen</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>Overlay-Änderungen dieser Datei auf Datenträger übernehmen</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>Overlay-Änderungen dieses Ordners auf Datenträger übernehmen</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>Dateien: %1. Alle Unterordner werden einbezogen.</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>Datei löschen und die Löschung dauerhaft übernehmen</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>Ordner samt Inhalt löschen und dauerhaft übernehmen</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>Dateien: %1; Ordner: %2. Löschen und übernehmen.</translation></message>
    <message><source>Commit this registry value to disk</source><translation>Registrierungswert auf Datenträger übernehmen</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>Registrierungsschlüssel mit gesamtem Unterbaum übernehmen</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>Wert: %1. (gesamter Schlüssel mit Unterschlüsseln)</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>Registrierungswert löschen und die Löschung übernehmen</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>Registrierungsschlüssel samt Unterbaum löschen und die Löschungen übernehmen</translation></message>
    <message><source>Scanning registry keys…</source><translation>Registrierungsschlüssel auflisten…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>Registrierungsschlüssel auflisten…
Schlüssel: %1</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>Registrierung · Schlüssel · Löschen und übernehmen:</translation></message>
    <message><source>Copy current entry</source><translation>Aktuellen Eintrag kopieren</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>%1 Schlüssel und alle enthaltenen Werte werden rekursiv gelöscht.</translation></message>
    <message><source>Commit file deletion failed</source><translation>Fehlgeschlagen: Dateilöschung übernehmen</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>Der Pfad existiert nicht; es gibt nichts zu löschen.</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Dieser Schlüssel ist in der Registrierungsausnahmeliste.
Ausnahme: %1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>Fehlgeschlagen: Registrierungsfilter lesen (%1)</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>Kein Registrierungsfilter-Datensatz der aktuellen Sitzung gefunden.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>Der Registrierungswert existiert nicht; es gibt nichts zu übernehmen.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>Der Registrierungsschlüssel existiert nicht; es gibt nichts zu übernehmen.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>Der Registrierungswert existiert nicht; es gibt nichts zu löschen.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>Der Registrierungsschlüssel existiert nicht; es gibt nichts zu löschen.</translation></message>
    <message><source>Export commands…</source><translation>Befehle exportieren…</translation></message>
    <message><source>Export commands to file</source><translation>Befehle in Datei exportieren</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>In Datei exportieren… · Befehl: %1
%2</translation></message>
    <message><source>Import</source><translation>Importieren</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Befehle eingeben oder einfügen, einen pro Zeile. Textdateien, Protokolle oder Skripte (.txt, .bat, .ps1) laden; Zeilen mit diesem Befehl werden angehängt. Der Import merkt Änderungen nur vor. Erst „Prüfen und anwenden“ in der Symbolleiste schreibt sie ins System.</translation></message>
    <message><source>Import uwfmgr commands</source><translation>uwfmgr · Importieren</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;uwfmgr&lt;/b&gt;: Befehle eingeben oder einfügen, einen pro Zeile. &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Aus Datei laden…&lt;/b&gt;: Textdateien, Protokolle oder Skripte (.txt, .bat, .ps1) laden; Zeilen mit diesem Befehl werden angehängt. &lt;code&gt;uwfmgr&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Importieren&lt;/b&gt;: &lt;b&gt;Der Import merkt Änderungen nur vor. Erst „Prüfen und anwenden“ in der Symbolleiste schreibt sie ins System.&lt;/b&gt; &lt;b&gt;Prüfen und anwenden&lt;/b&gt;.&lt;/p&gt;</translation></message>
    <message><source>Load from file…</source><translation>Aus Datei laden…</translation></message>
    <message><source>Load recommended configuration</source><translation>Empfohlene Konfiguration laden</translation></message>
    <message><source>Choose recommended configuration</source><translation>Empfohlene Konfiguration auswählen</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Empfohlene Konfigurationsgruppen auswählen. Einzelne Befehle können vor dem Import geprüft oder gelöscht werden.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Aus der offiziellen Microsoft-Dokumentation.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Von Microsoft empfohlene UWF-Ausnahmen</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Vor dem Import prüfen: Ordner müssen vorhanden sein, bevor UWF Dateiausnahmen akzeptiert.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Quelle: Offizielle Microsoft-UWF-Dokumentation zu allgemeinen Filterausnahmen und Antischadsoftware-Unterstützung.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Programm zur Verbesserung der Benutzerfreundlichkeit (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Dauerhaft speichern: Teilnahmestatus der Richtlinie</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Dauerhaft speichern: Lokaler Teilnahmestatus</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Dauerhaft speichern: Upload-Deaktivierungskennzeichen</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Intelligenter Hintergrundübertragungsdienst (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Dauerhaft speichern: Download-Warteschlangendateien</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Dauerhaft speichern: Übertragungsstatusindex</translation></message>
    <message><source>Network profiles and policies</source><translation>Netzwerkprofile und Richtlinien</translation></message>
    <message><source>Wireless network GPO policy</source><translation>WLAN-Gruppenrichtlinie</translation></message>
    <message><source>Wired network GPO policy</source><translation>LAN-Gruppenrichtlinie</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>WLAN-Gruppenrichtlinie · Dateien</translation></message>
    <message><source>Wired network GPO policy files</source><translation>LAN-Gruppenrichtlinie · Dateien</translation></message>
    <message><source>Wireless network interface profiles</source><translation>WLAN-Schnittstellenprofile</translation></message>
    <message><source>Wired network interface profiles</source><translation>LAN-Schnittstellenprofile</translation></message>
    <message><source>Wireless service configuration</source><translation>WLAN-Dienstkonfiguration</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Mobiler Breitbanddienst</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Konfiguration des Dienstes für automatische LAN-Konfiguration</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>Netzwerkprofil-XML-Dateien sind gerätespezifisch. Bei Bedarf konkrete Pfade Interfaces\{GUID}\{GUID}.xml manuell hinzufügen.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Sommerzeit (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Dauerhaft speichern: Zeitzonendefinitionen</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Dauerhaft speichern: Ausgewählte Zeitzone</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Dauerhaft speichern: Produktdateien und Updates</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Dauerhaft speichern: Signaturen und Status in ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Dauerhaft speichern: Windows Update · Protokoll</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Dauerhaft speichern: MpCmdRun · Protokoll</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Dauerhaft speichern: Produktstatus in der Registrierung</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Dauerhaft speichern: WdBoot · Dienststatus</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Dauerhaft speichern: WdFilter · Dienststatus</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Dauerhaft speichern: WdNisSvc · Dienststatus</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Dauerhaft speichern: WdNisDrv · Dienststatus</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Dauerhaft speichern: WinDefend · Dienststatus</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Dauerhaft speichern: Clientprogrammdateien</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Dauerhaft speichern: Windows Update · Protokoll</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Dauerhaft speichern: MpCmdRun · Protokoll</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Dauerhaft speichern: Antischadsoftware-Signaturen und Status</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Dauerhaft speichern: Antischadsoftware-Status in der Registrierung</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Dateien mit uwfmgr-Befehlen auswählen</translation></message>
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>Alle Dateien (*);;Textdateien (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
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
    <message><source>Status</source><translation>Status</translation></message>
    <message><source>Command</source><translation>Befehl</translation></message>
    <message><source>Detail</source><translation>Details</translation></message>
    <message><source>Import failed</source><translation>Import fehlgeschlagen</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Interner Fehler: Keine Anwendungsroutine registriert.</translation></message>
    <message><source>Nothing to import</source><translation>Keine Befehle zu importieren</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Keine uwfmgr-Befehle in der Eingabe gefunden.</translation></message>
    <message><source>Applied: %1</source><translation>Angewendet: %1</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplikat: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Nicht unterstützt: %1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>Gesamt nach %1 Importdurchläufen:</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Importdurchlauf %1 ──</translation></message>
    <message><source>Applied</source><translation>Angewendet</translation></message>
    <message><source>Duplicate</source><translation>Duplikat</translation></message>
    <message><source>Unsupported</source><translation>Nicht unterstützt</translation></message>
    <message><source>File too large</source><translation>Datei zu groß</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Datei %1 ist größer als 5 MB und wurde nicht verarbeitet. Filtern Sie sie zuerst manuell.</translation></message>
    <message><source>Cannot read file</source><translation>Datei nicht lesbar</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Fehlgeschlagen: Datei öffnen (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Ausstehende Änderungen verwerfen?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>%1 Änderungen wurden noch nicht angewendet.

Verwerfen und fortfahren?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Unvollständiger uwfmgr-Befehl</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Fehlendes Argument: Größe (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>Größe muss eine nicht negative ganze Zahl in MB sein</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Fehlendes Argument: Overlay · Typ (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Ungültiger Wert: Overlay · Typ: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Fehlendes Argument: Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Volume muss ein Laufwerksbuchstabe wie C: sein</translation></message>
    <message><source>Missing path argument</source><translation>Fehlendes Argument: Pfad</translation></message>
    <message><source>Missing registry key argument</source><translation>Fehlendes Argument: Registrierung · Schlüssel</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Nicht geschlossenes Anführungszeichen im Befehl</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Unerwartetes zusätzliches Argument: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Nicht unterstützter uwfmgr-Befehl (keine Zuordnung zu einer UI-Aktion)</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>Ausstehende Änderungen: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Zielzustand bereits erreicht; keine Aktion nötig</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Pfad liegt nicht auf diesem Volume oder es unterstützt keine Dateiausnahmen (z. B. exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Von UWF-Sperrliste abgelehnt (Systemdatei / Windows / Auslagerungsdatei usw.)</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>Derselbe Befehl wurde in diesem Durchlauf bereits ausgegeben</translation></message>
    <message><source>Pending filter %1</source><translation>Ausstehende Änderungen: Filter %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>Filter: Zielzustand bereits erreicht; keine Aktion nötig</translation></message>
    <message><source>Pending overlay type → %1</source><translation>Ausstehende Änderungen: Overlay · Typ → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>Overlay · Typ: %1 (Zielzustand bereits erreicht; keine Aktion nötig)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Ungültiger Wert: Größe: %1</translation></message>
    <message><source>maximum size</source><translation>Maximale Größe</translation></message>
    <message><source>warning threshold</source><translation>Warnschwelle</translation></message>
    <message><source>critical threshold</source><translation>Kritische Schwelle</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>Ausstehende Änderungen: Overlay %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>Overlay %1: %2 MB (Zielzustand bereits erreicht; keine Aktion nötig)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Unbekanntes Volume %1 (kein UWF-geeignetes Laufwerk mit diesem Buchstaben)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>Ausstehende Änderungen: Volume %1 · UWF-Schutz: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Zielzustand bereits erreicht; keine Aktion nötig</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Der Pfad hat keinen Laufwerksbuchstaben. Zielvolume ermitteln: Fehlgeschlagen.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Nicht unterstützt</translation></message>
    <message><source>file exclusion</source><translation>Dateiausnahmen</translation></message>
    <message><source>registry exclusion</source><translation>Registrierungsausnahmen</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Registrierungsausnahmen benötigen den hier fehlenden Registrierungstab</translation></message>
    <message><source>Unsupported command</source><translation>Befehl: Nicht unterstützt</translation></message>
    <message><source>Exit</source><translation>Beenden</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Ein-/Ausblenden.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Aktiviert</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Deaktiviert</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Unbekannt</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Belegt %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Erweiterter Modus</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Automatische Dateiübernahme mit Windows-Herunterfahren und -Neustart koordinieren.</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>Installiert einen automatisch startenden LocalSystem-Dienst. Er startet UWF Manager nach dem Windows-Start und versucht, vorgemerkte Dateiänderungen vor dem Herunterfahren oder Neustart dauerhaft zu übernehmen, um UWF-Kompatibilitätsprobleme zu mindern.</translation></message>
    <message><source>System changes</source><translation>Systemänderungen</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>Automatischen Dienst UWFManagerEnhanced anlegen und mit dem aktuellen Programmpfad verknüpfen</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>Konfiguration: Registrierung → Übernehmen (HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>UWF Manager startet nach dem nächsten Windows-Start automatisch; Verschieben des Programms kann den Autostart verhindern</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>Geeignete vorgemerkte Dateien beim normalen Windows-Herunterfahren oder -Neustart übernehmen</translation></message>
    <message><source>Operational and security considerations</source><translation>Betriebs- und Sicherheitshinweise</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>Registriert einen Dienst in der Systemregistrierung; die Anwendung ist dadurch nicht mehr vollständig portabel</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>Viele vorgemerkte Dateien können Herunterfahren und Neustart deutlich verlängern; lassen Sie genügend Zeit zur dauerhaften Übernahme</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>Der erweiterte Modus führt UWF Manager als SYSTEM aus. Können Nichtadministratoren das Programm manipulieren oder durch Schadsoftware ersetzen, ist eine Rechteausweitung auf SYSTEM möglich. Nur an einem administrativ geschützten Ort aktivieren (empfohlen: %ProgramFiles%\UWF Manager\Service\UWF.exe).</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>Fehlgeschlagen: Status des erweiterten Modus lesen</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Fehlgeschlagen: Status des erweiterten Modus lesen (Unbekannter Fehler).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Fehlgeschlagen: Dienststatus lesen:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Fehlgeschlagen: Dienststatus lesen (Unbekannter Fehler).</translation></message>
    <message><source>Status: Disabled</source><translation>Status: Deaktiviert</translation></message>
    <message><source>Status: Enabled</source><translation>Status: Aktiviert</translation></message>
    <message><source>Status: Service stopped</source><translation>Status: Dienst angehalten</translation></message>
    <message><source>Status: Repair required</source><translation>Status: Reparatur erforderlich</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Der Dienst ist nicht als eigener Prozess konfiguriert.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Der Dienst ist nicht für automatischen Start konfiguriert.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Der Dienst ist nicht für die Ausführung als LocalSystem konfiguriert.</translation></message>
    <message><source>The service is not running.</source><translation>Der Dienst läuft nicht.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Programmpfad oder Startargumente des Dienstes passen nicht zu diesem Programm.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Das Zeitlimit für Dienst-Vorabbenachrichtigungen zum Herunterfahren ist falsch konfiguriert.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Der Dienst deklariert nicht die zum Starten des UI-Agenten nötigen Rechte.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Der Dienst nimmt keine Windows-Vorabbenachrichtigungen zum Herunterfahren an.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Erweiterten Modus aktivieren</translation></message>
    <message><source>Disable enhanced mode</source><translation>Erweiterten Modus deaktivieren</translation></message>
    <message><source>Start service</source><translation>Dienst starten</translation></message>
    <message><source>Repair enhanced mode</source><translation>Erweiterten Modus reparieren</translation></message>
    <message><source>Delete service</source><translation>Dienst löschen</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Dienst des erweiterten Modus beenden und samt verbliebenen Registrierungsdaten löschen.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Der Dienst fehlt, aber seine Registrierungsdaten sind noch vorhanden.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Authentifizierung des UI-Agenten wird abgewartet.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Der Dienst läuft, aber kein authentifizierter UI-Agent ist verbunden.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Hilfsdienst für den erweiterten Modus von UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Erweiterter Modus · Konfiguration: Angewendet.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Konfiguration: Angewendet. Meldung der UWF-Persistenz:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Erweiterter Modus · Konfiguration: Fehlgeschlagen:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Erweiterter Modus · Konfiguration: Fehlgeschlagen (Unbekannter Fehler).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Dienst des erweiterten Modus und verbliebene Konfiguration wurden gelöscht.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Dienstlöschung angefordert, aber Bereinigung verbliebener Daten meldete:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Erweiterter Modus: Dienst löschen: Fehlgeschlagen:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Erweiterter Modus: Dienst löschen: Fehlgeschlagen (Unbekannter Fehler).</translation></message>
    <message><source>File staging completed</source><translation>Automatische Dateiübernahme: Erfolgreich</translation></message>
    <message><source>File staging failed</source><translation>Automatische Dateiübernahme: Fehlgeschlagen</translation></message>
    <message><source>All staged targets were processed.</source><translation>Alle vorgemerkten Ziele wurden verarbeitet.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Alle vorgemerkten Ziele verarbeitet; mindestens ein Vorgang ist fehlgeschlagen.</translation></message>
    <message><source>Show main window</source><translation>Hauptfenster anzeigen</translation></message>
    <message><source>Restore default position</source><translation>Standardposition wiederherstellen</translation></message>
    <message><source>Hide overlay hub</source><translation>Overlay-HUB ausblenden</translation></message>
    <message><source>Exit application</source><translation>Anwendung beenden</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Fehlgeschlagen: Mit dem System verbinden</translation></message>
    <message><source>Registry access failed</source><translation>Fehlgeschlagen: Registrierungszugriff</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Fehlgeschlagen: Registrierungswerte lesen (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Overlay-Dateien auflisten: Unbekannter Fehler.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Fehlgeschlagen: Filter %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Fehlgeschlagen: Filterstatus lesen</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Fehlgeschlagen: Warnschwelle festlegen</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Fehlgeschlagen: Kritische Schwelle festlegen</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Fehlgeschlagen: Overlay-Status lesen</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Fehlgeschlagen: Overlay-Typ festlegen</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Fehlgeschlagen: Maximale Größe festlegen</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Fehlgeschlagen: Overlay-Konfiguration lesen</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Fehlgeschlagen: Volume %1 bei UWF registrieren</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Fehlgeschlagen: Volumeschutz %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Fehlgeschlagen: Zuordnung für Volume %1 festlegen</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Fehlgeschlagen: Dateiausnahme hinzufügen %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Dateiausnahme auf Volume %1 bereits nicht vorhanden: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Fehlgeschlagen: Dateiausnahme entfernen %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Fehlgeschlagen: Volumekonfiguration lesen</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Fehlgeschlagen: Registrierungsausnahme hinzufügen %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Fehlgeschlagen: Registrierungsausnahme entfernen %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Fehlgeschlagen: Registrierungspersistenz umschalten</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Fehlgeschlagen: Registrierungsfilter lesen</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Nicht unterstützt</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Zielstatus lesen: Unbekannter Fehler.</translation></message>
    <message><source>The target state could not be read before the operation: %1</source><translation>Zielstatus konnte vor dem Vorgang nicht gelesen werden: %1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Der Pfad existiert nicht; es gibt nichts zu löschen.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Unbekannter Fehler.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Ergebnis nicht bestätigbar, da das erneute Lesen des Zielstatus scheiterte: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Der Anbieter hat die Löschung angenommen, aber die maßgebliche erneute Prüfung zeigt, dass das Ziel noch existiert.</translation></message>
    <message><source>Unknown</source><translation>Unbekannt</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Fehlgeschlagen: Protokoll laden (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Fehlgeschlagen: Protokoll laden (Unbekannter Fehler).</translation></message>
    <message><source>: unknown error</source><translation>: Unbekannter Fehler</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>Fehlgeschlagen: Zielstatus lesen (%1)</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>Fehlgeschlagen: Zielstatus lesen (Unbekannter Fehler).</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>Fehlgeschlagen: Zielvolume ermitteln (%1)</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>Fehlgeschlagen: Registrierungswerte auflisten (%1)</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>Fehlgeschlagen: Registrierungsschlüssel auflisten (%1)</translation></message>
    <message><source>Delete and commit failed</source><translation>Löschen und übernehmen: Fehlgeschlagen</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Fehlgeschlagen: Zielvolume ermitteln (%1): %2</translation></message>
    <message><source>%1/%2 slots</source><translation>%1/%2 Steckplätze</translation></message>
</context>
</TS>
