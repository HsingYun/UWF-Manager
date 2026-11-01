<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="de" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Vorschau</translation></message>
    <message><source>enable</source><translation>aktivieren</translation></message>
    <message><source>disable</source><translation>deaktivieren</translation></message>
    <message><source>Enabled</source><translation>Aktiviert</translation></message>
    <message><source>Disabled</source><translation>Deaktiviert</translation></message>
    <message><source>Disabled</source><comment>service startup type</comment><translation>Deaktiviert</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Abbrechen</translation></message>
    <message><source>Close</source><translation>Schließen</translation></message>
    <message><source>Refresh</source><translation>Aktualisieren</translation></message>
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
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Schutzstatus dieses Volumes in der aktuellen Sitzung (schreibgeschützt).</translation></message>
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Dieses Volume in der nächsten Sitzung schützen. Schreibzugriffe auf geschützte Pfade werden ins Overlay umgeleitet; ob die Daten Neustarts überdauern, hängt von den Overlay-Einstellungen ab.</translation></message>
    <message><source>Current session</source><translation>Aktuelle Sitzung</translation></message>
    <message><source>Next session</source><translation>Nächste Sitzung</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Volume-Erkennung: Laufwerksbuchstaben sind einfach, die Zuordnung geht bei einer Neuzuweisung aber verloren, etwa nach Festplattenänderungen. Die Volume-ID bleibt bei Buchstabenänderungen stabil.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF unterstützt dieses Volume nicht: </translation></message>
    <message><source>Global settings</source><translation>Globale Einstellungen</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>UWF-Filterstatus in der aktuellen Sitzung (schreibgeschützt).</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>UWF in der nächsten Sitzung aktivieren. Schreibzugriffe auf geschützte Pfade werden ins Overlay umgeleitet; ob die Daten Neustarts überdauern, hängt von den Overlay-Einstellungen ab.</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>So ändern Sie den Overlay-Typ oder die maximale Kapazität:
1. UWF deaktivieren.
2. Gerät neu starten.
3. Diese Einstellung ändern.</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM nutzt den Arbeitsspeicher. Disk nutzt das Systemlaufwerk und unterstützt größere Overlays. Ein persistentes Overlay kann Daten im Disk-Overlay über Neustarts hinweg erhalten.</translation></message>
    <message><source>Type</source><translation>Typ</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>Die Kapazität eines RAM-Overlays ist durch den Arbeitsspeicher begrenzt. Ein Disk-Overlay benötigt mindestens 1024 MB und genügend freien Speicher auf dem Systemlaufwerk.</translation></message>
    <message><source>Maximum capacity</source><translation>Maximale Kapazität</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>Maximale Kapazität · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Warnschwelle</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Schreibt eine Warnmeldung in das Ereignisprotokoll, sobald die Overlay-Belegung diesen Wert erreicht. Wenn beide Ereignisse aktiviert sind, muss dieser Wert unter dem kritischen Schwellenwert liegen. Mit 0 wird nur das Warnereignis deaktiviert.</translation></message>
    <message><source>Critical threshold</source><translation>Kritische Schwelle</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Schreibt eine kritische Meldung in das Ereignisprotokoll, sobald die Overlay-Belegung diesen Wert erreicht. Wenn beide Ereignisse aktiviert sind, muss dieser Wert über dem Warnschwellenwert liegen. Mit 0 wird nur das kritische Ereignis deaktiviert.</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Laufwerkstyp nicht unterstützt (nur lokale Festplatten).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Das Volume überschreitet die UWF-Grenze von 16 TiB pro geschütztem Volume.</translation></message>

    <message><source>%1 free / %2</source><translation>%1 frei / %2</translation></message>
    <message><source>Commit</source><translation>Übernehmen</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>Änderungen aus dem Overlay auf den Datenträger oder in die Registrierung übernehmen. Dies kann nicht rückgängig gemacht werden.</translation></message>
    <message><source>Commit file changes…</source><translation>Dateiänderungen übernehmen…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Eine Datei auswählen und ihre Änderungen aus dem Overlay auf den Datenträger übernehmen.</translation></message>
    <message><source>Commit folder changes…</source><translation>Ordneränderungen übernehmen…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Einen Ordner auswählen und die Overlay-Änderungen aller darin enthaltenen Dateien auf den Datenträger übernehmen.</translation></message>
    <message><source>Delete and commit file…</source><translation>Datei löschen und die Löschung dauerhaft übernehmen…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Eine Datei zum Löschen auswählen und die Löschung auf den Datenträger übernehmen.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Ordner samt Inhalt löschen und dauerhaft übernehmen…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>Einen Ordner auswählen, ihn samt gesamtem Inhalt rekursiv löschen und die Löschungen auf den Datenträger übernehmen.</translation></message>
    <message><source>Commit registry changes…</source><translation>Registrierungsänderungen übernehmen…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Registrierungspfad und optional einen Wertnamen eingeben. Ohne Wertnamen werden alle Werte des Schlüssels und sämtlicher Unterschlüssel dauerhaft übernommen.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Registrierungslöschung übernehmen…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>Geben Sie einen Registrierungsschlüssel und optional einen Wertnamen ein. Lassen Sie den Wertnamen leer, um den Schlüssel samt allen Unterschlüsseln und Werten zu löschen und die Löschungen anschließend zu übernehmen.</translation></message>
    <message><source>File exclusions</source><translation>Dateiausschlüsse</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Dateien und Ordner dieses Volumes ohne UWF-Schutz. Ein Doppelklick kopiert den Pfad.</translation></message>
    <message><source>Registry exclusions</source><translation>Registrierungsausschlüsse</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>Registrierungsausschlüsse gelten global und werden nur auf der Windows-Partition angezeigt.</translation></message>
    <message><source>File staging</source><translation>Dateivormerkung</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>Dateien oder einen Ordner zur automatischen Übernahme hinzufügen.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Eine oder mehrere Dateien auswählen, die vor sicherem Herunterfahren oder Neustart automatisch übernommen werden.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Einen Ordner auswählen, dessen Dateien vor sicherem Herunterfahren oder Neustart rekursiv übernommen werden.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Dateien für die automatische Übernahme auswählen</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Ordner für die automatische rekursive Übernahme auswählen</translation></message>
    <message><source>Cannot stage this path</source><translation>Pfad kann nicht zur automatischen Übernahme hinzugefügt werden</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Dieser Pfad überschneidet sich mit einem UWF-Dateiausschluss. Entfernen Sie den Ausschluss, bevor Sie den Pfad vormerken:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Dieser Pfad überschneidet sich mit einem Eintrag der Dateivormerkung. Entfernen Sie den vorgemerkten Pfad, bevor Sie den Ausschluss hinzufügen:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Der Pfad überschneidet sich mit einem vorhandenen Eintrag der Dateivormerkung</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Dieser Registrierungsausschluss ist erforderlich, solange die Dateivormerkung gespeicherte Einträge enthält</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Pfad existiert nicht oder hat nicht den erwarteten Dateityp:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Pfad konnte nicht geprüft werden:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Analysepunkte können nicht zur Dateivormerkung hinzugefügt werden:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>Der Pfad hat keinen Laufwerkbuchstaben und kann nicht zur Dateivormerkung hinzugefügt werden:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Der ausgewählte Pfad liegt auf Volume %1. Fügen Sie ihn über dessen Tab „Dateivormerkung“ hinzu.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Das Volume-Stammverzeichnis kann nicht vorgemerkt werden, da sonst das gesamte Volume rekursiv übernommen würde.</translation></message>
    <message><source>File staging could not be saved</source><translation>Die Dateivormerkungsliste konnte nicht gespeichert werden</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Die Dateivormerkung wurde neu geladen, da die Aktualisierung nicht abgeschlossen werden konnte:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Dateivormerkung nicht verfügbar: %1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>%1 vorgemerkte Einträge · %2 Dateien · %3 Ordner</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Die Dateivormerkung benötigt diesen Registrierungsausschluss, damit ihre Liste nach Neustarts erhalten bleibt.</translation></message>
    <message><source>Built-in</source><translation>Integriert</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Der UWF-Filter ist deaktiviert. Es gibt keine gesammelten Overlay-Änderungen zu übernehmen.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Das Dateisystem %1 unterstützt keine dateiweise Übernahme.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Dieses Volume wird von UWF nicht unterstützt.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Das Dateisystem %1 unterstützt keine dateiweise Übernahme. Nur die Registrierungsübernahme ist verfügbar (Registrierungsausschlüsse gelten global).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Dieses Volume ist derzeit nicht UWF-geschützt; es gibt keine Dateiänderungen zu übernehmen.</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Datei zur dauerhaften Übernahme auswählen</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Ordner zur dauerhaften Übernahme auswählen</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Datei zur dauerhaften Löschung auswählen</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Ordner zur dauerhaften Löschung auswählen</translation></message>
    <message><source>Name</source><translation>Name</translation></message>
    <message><source>Data</source><translation>Daten</translation></message>
    <message><source>(Default)</source><translation>(Standard)</translation></message>
    <message><source>(value not set)</source><translation>(Wert nicht festgelegt)</translation></message>
    <message><source>(empty)</source><translation>(leer)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Vorschau nicht verfügbar (Wert ist zu groß)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Eine Diagnoseansicht der Dateien öffnen, die derzeit für dieses Volume im Overlay zwischengespeichert sind.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Overlay-Dateilisten werden nur auf NTFS-Volumes unterstützt.</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>Der UWF-Filter ist deaktiviert; kein Volume hat ein prüfbares Overlay.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Dieses Volume ist nicht UWF-geschützt und hat kein prüfbares Overlay.</translation></message>
    <message><source>Failed: %1</source><translation>Fehlgeschlagen: %1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>Die vorherige Auflistung wird möglicherweise noch ausgeführt. Ein neuer Lesevorgang kann noch nicht gestartet werden.</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>Der UWF-Dienst ist beim Auflisten der Overlay-Dateien nicht verfügbar.</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>Nicht genügend Arbeitsspeicher oder dieser Vorgang wird nicht unterstützt. Diese Funktion erfordert ein NTFS-Volume; schließen Sie speicherintensive Anwendungen und versuchen Sie es erneut.</translation></message>
    <message><source>Raw error: %1</source><translation>Fehlercode: %1</translation></message>
    <message><source>Export overlay file list</source><translation>In Datei exportieren… · Overlay · Dateien</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Textdateien (*.txt);;Alle Dateien (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Befehlsdateien (*.cmd);;Alle Dateien (*)</translation></message>
    <message><source>Export failed</source><translation>Export fehlgeschlagen</translation></message>
    <message><source>Export finished</source><translation>Export abgeschlossen</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Fehlgeschlagen: Datei öffnen (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Fehlgeschlagen: Datei schreiben (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>%1 Einträge gespeichert unter:
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Ein ganzes Volume (%1) kann nicht ausgenommen werden.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>Auslagerungs-, Swap- und Ruhezustandsdateien können nicht als Ausschlüsse hinzugefügt werden.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Das gesamte Verzeichnis kann nicht ausgenommen werden: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Das gesamte Verzeichnis kann nicht ausgenommen werden: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Das gesamte Verzeichnis kann nicht ausgenommen werden: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Diese wichtige Systemdatei kann nicht ausgenommen werden: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Die benutzerspezifische Registrierungsdatei NTUSER.DAT kann nicht ausgenommen werden.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Der Registrierungsschlüssel darf nicht leer sein.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Ein Registrierungsschlüssel darf nicht mit einem Rückstrich beginnen.</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>Registrierungspfade dürfen keine Nullzeichen enthalten.</translation></message>
    <message><source>A registry key component is too long.</source><translation>Ein Bestandteil des Registrierungsschlüssels ist zu lang.</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>Der Registrierungspfad überschreitet die Verschachtelungsgrenze von UWF.</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>Eine Datei oder einen Ordner zur Ausschlussliste hinzufügen.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Datei zum Hinzufügen auswählen · Dateiausschlüsse.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Ordner zum Hinzufügen auswählen · Dateiausschlüsse. Alle Unterordner werden einbezogen.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Pfad des auszunehmenden Registrierungsschlüssels eingeben.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Pfad des auszunehmenden Registrierungsschlüssels eingeben. / Dauerhafte Speicherung.</translation></message>
    <message><source>Registry key…</source><translation>Registrierung · Schlüssel…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Domänengeheimnis (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Terminaldienste-Clientzugriffslizenz (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Domänengeheimnis (Computerkontokennwort) über UWF-Sitzungen hinweg erhalten.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Terminaldienste-Clientzugriffslizenzen über UWF-Sitzungen hinweg erhalten.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Dauerhafte Speicherung %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Auswahl entfernen</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Ausgewählte Ausschlüsse entfernen. Wird nach dem Anwenden wirksam.</translation></message>
    <message><source>Open containing folder</source><translation>Übergeordneten Ordner öffnen</translation></message>
    <message><source>Copy file path</source><translation>Dateipfad kopieren</translation></message>
    <message><source>Copy registry path</source><translation>Registrierungspfad kopieren</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Overlay-Änderungen dieses Ordners auf Datenträger übernehmen…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Overlay-Änderungen dieser Datei auf Datenträger übernehmen…</translation></message>
    <message><source>Copied to clipboard</source><translation>In die Zwischenablage kopiert</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Datei zum Hinzufügen auswählen · Dateiausschlüsse. Mehrfachauswahl möglich.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Ordner zum Hinzufügen auswählen · Dateiausschlüsse</translation></message>
    <message><source>Add registry exclusion</source><translation>Registrierungsausschluss hinzufügen</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Pfad %1 liegt nicht auf Volume %2 und kann dort nicht als Ausschluss hinzugefügt werden.</translation></message>
    <message><source>Path is not on this volume</source><translation>Pfad liegt nicht auf diesem Volume</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Ausschluss kann nicht hinzugefügt werden</translation></message>
    <message><source>File: %1</source><translation>Datei… %1</translation></message>
    <message><source>Folder: %1</source><translation>Ordner… %1</translation></message>
    <message><source>Registry: %1</source><translation>Registrierung: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Standardmäßig ausgenommen; jederzeit änderbar.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Aktuelle Sitzung: %1
Nächste Sitzung: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Ausstehende Änderungen: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Verwaltung des Unified Write Filter (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>UWF-Status der aktuellen Sitzung und Konfiguration der nächsten Sitzung neu lesen.</translation></message>
    <message><source>Review and apply</source><translation>Prüfen und anwenden</translation></message>
    <message><source>Review all pending changes and apply them in one batch.</source><translation>Alle ausstehenden Änderungen prüfen und zusammen anwenden.</translation></message>
    <message><source>Safe shutdown</source><translation>Sicher herunterfahren</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Das System wird über UWF sicher heruntergefahren. Auch bei vollem UWF-Overlay verfügbar.</translation></message>
    <message><source>Safe restart</source><translation>Sicher neu starten</translation></message>
    <message><source>Direct restart</source><translation>Direkt neu starten</translation></message>
    <message><source>Direct shutdown</source><translation>Direkt herunterfahren</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Das System wird über UWF sicher neu gestartet. Auch bei vollem UWF-Overlay verfügbar.</translation></message>
    <message><source>Log</source><translation>Protokoll</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Internes Protokoll dieser Sitzung zur Fehlersuche anzeigen.</translation></message>
    <message><source>About</source><translation>Über</translation></message>
    <message><source>Switch display language</source><translation>Anzeigesprache wechseln</translation></message>
    <message><source>Language</source><translation>Sprache</translation></message>
    <message><source>Theme</source><translation>Design</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Helles/dunkles Design wechseln</translation></message>
    <message><source>System information</source><translation>Systeminformationen</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Diagnosesnapshot</translation></message>
    <message><source>Read only</source><translation>Schreibgeschützt</translation></message>
    <message><source>The system did not find the target.</source><translation>Das System hat das Ziel nicht gefunden.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Das System hat einen Parameter abgelehnt (ungültiger Pfad oder ungültiges Argument).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Vorgang fehlgeschlagen. Ein anderes Programm könnte das Ziel verwenden, etwa Explorer oder eine geöffnete Datei. Schließen Sie zugreifende Programme und versuchen Sie es erneut.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Vorgang fehlgeschlagen; Details stehen im Protokoll.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Vorgang abgelehnt (Code %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Unbekannt.</translation></message>
    <message><source>Commit canceled</source><translation>Übernahme abgebrochen</translation></message>

    <message><source>Succeeded</source><translation>Erfolgreich</translation></message>
    <message><source>Skipped</source><translation>Übersprungen</translation></message>
    <message><source>Failed</source><translation>Fehlgeschlagen</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>Erfolgreich: %1; fehlgeschlagen: %2.</translation></message>

    <message><source>Existed before</source><translation>Vor dem Löschen vorhanden</translation></message>
    <message><source>Exists after</source><translation>Nach dem Löschen vorhanden</translation></message>
    <message><source>Error code</source><translation>Fehlercode</translation></message>
    <message><source>Reason</source><translation>Grund</translation></message>
    <message><source>Previous page</source><translation>Vorherige Seite</translation></message>
    <message><source>First page</source><translation>Erste Seite</translation></message>
    <message><source>Last page</source><translation>Letzte Seite</translation></message>
    <message><source>Next page</source><translation>Nächste Seite</translation></message>
    <message><source>Copy selected rows</source><translation>Ausgewählte Zeilen kopieren</translation></message>
    <message><source>Copy all</source><translation>Alles kopieren</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Dieser Registrierungsausschluss ist erforderlich, solange die Dateivormerkung gespeicherte Einträge enthält. Leeren Sie die Dateivormerkungsliste, bevor Sie den Ausschluss entfernen.</translation></message>
    <message><source>Copy current page</source><translation>Aktuelle Seite kopieren</translation></message>
    <message><source>No pending changes</source><translation>Keine ausstehenden Änderungen</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Registrierungsausschlüsse · Globale Einstellungen.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Zu Schutz, Dateiausschlüssen und Dateivormerkung für Volume %1 wechseln.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Nicht unterstützt</translation></message>
    <message><source>Review and apply changes</source><translation>Änderungen prüfen und anwenden</translation></message>
    <message><source>Confirm apply</source><translation>Anwenden bestätigen</translation></message>
    <message><source>Result</source><translation>Ergebnis</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filter: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Overlay-Warnschwellenwert auf %1 MB gesetzt</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Kritischer Overlay-Schwellenwert auf %1 MB gesetzt</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ Typ / maximale Kapazität nicht angewendet: UWF ist aktiviert. Deaktivieren Sie UWF und starten Sie zuerst neu.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Overlay-Typ auf %1 gesetzt</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ Maximale Overlay-Kapazität auf %1 MB gesetzt</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · UWF-Schutz: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Zuordnung über: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1: Dateiausschluss hinzugefügt: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1: Dateiausschluss entfernt: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Registrierungsausschluss hinzugefügt: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Registrierungsausschluss entfernt: %1</translation></message>
    <message><source>Time</source><translation>Zeit</translation></message>
    <message><source>Level</source><translation>Stufe</translation></message>

    <message><source>Message</source><translation>Meldung</translation></message>
    <message><source>Committing staged files</source><translation>Vorgemerkte Dateien werden übernommen</translation></message>
    <message><source>No additional error details were provided.</source><translation>Keine zusätzlichen Fehlerdetails verfügbar.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Die automatische Übernahme vorgemerkter Dateien wurde unerwartet beendet:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Die automatische Übernahme vorgemerkter Dateien wurde wegen eines unbekannten Fehlers beendet.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Sicher herunterfahren: Fehlgeschlagen</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Herunterfahren: Fehlgeschlagen (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Sicher neu starten: Fehlgeschlagen</translation></message>
    <message><source>Direct restart failed</source><translation>Direkt neu starten: Fehlgeschlagen</translation></message>
    <message><source>Direct shutdown failed</source><translation>Direkt herunterfahren: Fehlgeschlagen</translation></message>
    <message><source>Restart failed: %1</source><translation>Neu starten: Fehlgeschlagen (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Die Dateivormerkungsliste konnte nicht gelesen werden:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Die Dateivormerkungsliste konnte wegen eines unbekannten Fehlers nicht gelesen werden.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>Gefundene Dateien: %1 · Übernommen: %2 · Übersprungene Dateien: %3 · Übersprungene Einträge: %4 · Fehlgeschlagen: %5</translation></message>
    <message><source>File staging preparation</source><translation>Vorbereitung der Dateivormerkung</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Analysepunkte werden nicht unterstützt.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Nur absolute Pfade auf lokalen Volumes können vorgemerkt werden.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Nur absolute Pfade auf lokalen Volumes können zur Dateivormerkung hinzugefügt werden:
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Der vorgemerkte Pfad konnte nicht geprüft werden.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Der vorgemerkte Dateipfad verweist nun auf einen anderen Pfadtyp.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Der vorgemerkte Ordnerpfad verweist nun auf einen anderen Pfadtyp.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Der vorgemerkte Ordner konnte nicht vollständig aufgelistet werden.</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>Das Volume-Stammverzeichnis darf nicht rekursiv verarbeitet werden.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Der UWF-Anbieter hat die Übernahme abgelehnt.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Übernahme fehlgeschlagen: Unbekannter Fehler.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>%1 weitere Fehler wurden protokolliert.</translation></message>



    <message><source>No current-session record found for volume %1.</source><translation>Für Volume %1 wurde kein Datensatz der aktuellen Sitzung gefunden.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Dieser Pfad ist in der Dateiausschlussliste.
Ausschluss: %1</translation></message>

    <message><source>Continue</source><translation>Weiter</translation></message>
    <message><source>Delete and commit</source><translation>Löschen und übernehmen</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Dieser Schlüssel ist in der Registrierungsausschlussliste.
Ausschluss: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Kein Registrierungsfilter-Datensatz der aktuellen Sitzung gefunden.</translation></message>

    <message><source>Export commands to file</source><translation>Befehle in Datei exportieren</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>In Datei exportieren… · Befehl: %1
%2</translation></message>
    <message><source>Import</source><translation>Importieren</translation></message>
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>uwfmgr-Befehle in den Entwurf importieren. Änderungen werden erst nach Bestätigung unter „Prüfen und anwenden“ geschrieben.</translation></message>
    <message><source>Load from file…</source><translation>Aus Datei laden…</translation></message>
    <message><source>Load recommended configuration</source><translation>Empfohlene Konfiguration laden</translation></message>
    <message><source>Choose recommended configuration</source><translation>Empfohlene Konfiguration auswählen</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Empfohlene Konfigurationsgruppen auswählen. Einzelne Befehle können vor dem Import geprüft oder gelöscht werden.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Aus der offiziellen Microsoft-Dokumentation.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Von Microsoft empfohlene UWF-Ausschlüsse</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>Prüfen Sie diese Empfehlungen vor dem Import.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Quelle: Offizielle Microsoft-UWF-Dokumentation zu allgemeinen Filterausschlüsse und Antischadsoftware-Unterstützung.</translation></message>
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
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Textdateien (*.txt *.cmd);;Alle Dateien (*)</translation></message>
    <message><source>Status</source><translation>Status</translation></message>
    <message><source>Import failed</source><translation>Import fehlgeschlagen</translation></message>
    <message><source>Command import is not ready.</source><translation>Der Befehlsimport ist noch nicht bereit.</translation></message>
    <message><source>Nothing to import</source><translation>Keine Befehle zu importieren</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Keine uwfmgr-Befehle in der Eingabe gefunden.</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplikat: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Nicht unterstützt: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Importdurchlauf %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Duplikat</translation></message>
    <message><source>Unsupported</source><translation>Nicht unterstützt</translation></message>
    <message><source>File too large</source><translation>Datei zu groß</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Datei %1 ist größer als 5 MB und wurde nicht verarbeitet. Filtern Sie sie zuerst manuell.</translation></message>
    <message><source>Cannot read file</source><translation>Datei nicht lesbar</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Fehlgeschlagen: Datei öffnen (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Ausstehende Änderungen verwerfen?</translation></message>
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>%1 Änderung(en) wurden noch nicht angewendet.

Änderungen verwerfen und fortfahren?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Unvollständiger uwfmgr-Befehl</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Fehlendes Argument: Größe (MB)</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>Die Größe muss eine nicht negative ganze Zahl sein (MB)</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Fehlendes Argument: Overlay · Typ (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Ungültiger Wert: Overlay · Typ: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Fehlendes Argument: Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Volume muss ein Laufwerksbuchstabe wie C: sein</translation></message>
    <message><source>Missing path argument</source><translation>Fehlendes Argument: Pfad</translation></message>
    <message><source>Missing registry key argument</source><translation>Fehlendes Argument: Registrierung · Schlüssel</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Nicht geschlossenes Anführungszeichen im Befehl</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Unerwartetes zusätzliches Argument: %1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>Dieser uwfmgr-Befehl kann nicht importiert werden.</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Zielzustand bereits erreicht; keine Aktion nötig</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Pfad liegt nicht auf diesem Volume oder es unterstützt keine Dateiausschlüsse (z. B. exFAT / ReFS)</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>Dieser Pfad kann nicht als Ausschluss hinzugefügt werden.</translation></message>
    <message><source>Invalid size value: %1</source><translation>Ungültiger Wert: Größe: %1</translation></message>
    <message><source>maximum capacity</source><translation>maximale Kapazität</translation></message>
    <message><source>warning threshold</source><translation>Warnschwelle</translation></message>
    <message><source>critical threshold</source><translation>Kritische Schwelle</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>Die globale Konfiguration ist nicht verfügbar; der Befehl wurde nicht zur Anwendung vorgemerkt.</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>Unbekanntes Volume %1 (keine passende Volume-Konfiguration)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Zielzustand bereits erreicht; keine Aktion nötig</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>Pfad %1 hat keinen Laufwerksbuchstaben; der zugehörige Volume-Tab kann nicht ermittelt werden.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Nicht unterstützt</translation></message>
    <message><source>file exclusion</source><translation>Dateiausschluss</translation></message>
    <message><source>registry exclusion</source><translation>Registrierungsausschluss</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Registrierungsausschlüsse benötigen den hier fehlenden Registrierungstab</translation></message>
    <message><source>Unsupported command</source><translation>Befehl: Nicht unterstützt</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>Overlay-Widget ein- oder ausblenden.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Aktiviert</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Deaktiviert</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Unbekannt</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Belegt %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Erweiterter Modus</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Die automatische Übernahme vorgemerkter Dateien mit dem Herunterfahren und Neustarten von Windows koordinieren.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Fehlgeschlagen: Status des erweiterten Modus lesen (Unbekannter Fehler).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Fehlgeschlagen: Dienststatus lesen:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Fehlgeschlagen: Dienststatus lesen (Unbekannter Fehler).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Der Dienst ist nicht als eigener Prozess konfiguriert.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Der Dienst ist nicht für automatischen Start konfiguriert.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Der Dienst ist nicht für die Ausführung als LocalSystem konfiguriert.</translation></message>
    <message><source>The service is not running.</source><translation>Der Dienst läuft nicht.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Programmpfad oder Startargumente des Dienstes passen nicht zu diesem Programm.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Das Zeitlimit für Dienst-Vorabbenachrichtigungen zum Herunterfahren ist falsch konfiguriert.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Der Dienst deklariert nicht die zum Starten des UI-Agenten nötigen Rechte.</translation></message>
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>Der Dienst kann derzeit keine Windows-Vorabbenachrichtigungen zum Herunterfahren empfangen.</translation></message>
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
    <message><source>Show main window</source><translation>Hauptfenster anzeigen</translation></message>
    <message><source>Restore default position</source><translation>Standardposition wiederherstellen</translation></message>
    <message><source>Show overlay widget</source><translation>Overlay-Widget anzeigen</translation></message>
    <message><source>Hide overlay widget</source><translation>Overlay-Widget ausblenden</translation></message>
    <message><source>Exit application</source><translation>Anwendung beenden</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Fehlgeschlagen: Registrierungswerte lesen (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Overlay-Dateien auflisten: Unbekannter Fehler.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Fehlgeschlagen: Filter %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Fehlgeschlagen: Filterstatus lesen</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Fehlgeschlagen: Warnschwelle festlegen</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Fehlgeschlagen: Kritische Schwelle festlegen</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Fehlgeschlagen: Overlay-Status lesen</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Fehlgeschlagen: Overlay-Typ festlegen</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ Maximale Kapazität konnte nicht festgelegt werden</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Fehlgeschlagen: Overlay-Konfiguration lesen</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Fehlgeschlagen: Volumeschutz %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Fehlgeschlagen: Zuordnung für Volume %1 festlegen</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Fehlgeschlagen: Dateiausschluss hinzufügen %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Fehlgeschlagen: Dateiausschluss entfernen %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Fehlgeschlagen: Volumekonfiguration lesen</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Fehlgeschlagen: Registrierungsausschluss hinzufügen %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Fehlgeschlagen: Registrierungsausschluss entfernen %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Fehlgeschlagen: Registrierungspersistenz umschalten</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Fehlgeschlagen: Registrierungsfilter lesen</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Nicht unterstützt</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Zielstatus lesen: Unbekannter Fehler.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Unbekannter Fehler.</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>Der Zielzustand konnte nicht überprüft werden; das Ergebnis des Vorgangs ist daher unbekannt: %1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>Das System hat die Löschanforderung angenommen, aber das Ziel ist nach der Überprüfung weiterhin vorhanden.</translation></message>
    <message><source>Unknown</source><translation>Unbekannt</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Fehlgeschlagen: Protokoll laden (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Fehlgeschlagen: Protokoll laden (Unbekannter Fehler).</translation></message>
    <message><source>: unknown error</source><translation>: Unbekannter Fehler</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Fehlgeschlagen: Zielvolume ermitteln (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Fehlgeschlagen: Registrierungsschlüssel auflisten (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Fehlgeschlagen: Zielvolume ermitteln (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Übersprungen, da ein vorausgesetzter Vorgang fehlgeschlagen ist oder nicht bestätigt werden konnte.</translation></message>
    <message><source>Copy</source><translation>Kopieren</translation></message>
    <message><source>Select All</source><translation>Alles auswählen</translation></message>
    <message><source>Display language</source><translation>Anzeigesprache</translation></message>
    <message><source>Folder</source><translation>Ordner</translation></message>
    <message><source>File</source><translation>Datei</translation></message>
    <message><source>Export commands</source><translation>Befehle exportieren</translation></message>
    <message><source>Direct restart…</source><translation>Direkt neu starten…</translation></message>
    <message><source>Clear…</source><translation>Leeren…</translation></message>
    <message><source>Committing</source><translation>Wird übernommen</translation></message>
    <message><source> · %1 selected</source><translation> · %1 ausgewählt</translation></message>
    <message><source>%1 changes</source><translation>%1 Änderungen</translation></message>
    <message><source>%1 commands</source><translation>%1 Befehle</translation></message>
    <message><source>%1 items</source><translation>%1 Einträge</translation></message>
    <message><source>%1 items · %2</source><translation>%1 Einträge · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 ausstehende Änderungen</translation></message>
    <message><source>%1 records</source><translation>%1 Datensätze</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 Datensätze · %2 insgesamt</translation></message>


    <message><source>%1 selected</source><translation>%1 ausgewählt</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 Dateien</translation></message>
    <message><source>32-bit</source><translation>32 Bit</translation></message>
    <message><source>64-bit</source><translation>64 Bit</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Veröffentlicht unter der &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Die vollständigen Bedingungen stehen in der mitgelieferten LICENSE-Datei.&lt;/p&gt;&lt;p&gt;Sie dürfen die Software unter GPL v3 weitergeben oder ändern. Sie wird ohne jegliche Gewährleistung bereitgestellt.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows deaktiviert Auslagerungsdateien, Systemwiederherstellung, SuperFetch, Dateiindizierung, Datenträgerdefragmentierung und Schnellstart. Außerdem wird bootstatuspolicy in der BCD auf ignoreallfailures gesetzt.&lt;/p&gt;&lt;p&gt;Sie können diese Einstellungen danach ändern. Verschieben Sie beispielsweise die Auslagerungsdatei auf ein ungeschütztes Volume, bevor Sie sie wieder aktivieren.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Eine Diagnoseaufnahme von System, Berechtigungen und UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>An diesem Ort existiert bereits eine Datei:
%1

Beim Speichern wird ihr Inhalt ersetzt.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Eine grafische Verwaltung für Windows Unified Write Filter.
Verwalten Sie Volumeschutz, Overlay, Ausschlüsse und vorgemerkte Dateien.</translation></message>
    <message><source>Snapshot details</source><translation>Snapshot-Details</translation></message>
    <message><source>Account</source><translation>Konto</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>UWF-Konfiguration aus einem Skript oder Text hinzufügen.</translation></message>
    <message><source>Add exclusion</source><translation>Ausschluss hinzufügen</translation></message>
    <message><source>Add files or folders</source><translation>Dateien oder Ordner hinzufügen</translation></message>
    <message><source>Add selected items</source><translation>Ausgewählte Einträge hinzufügen</translation></message>
    <message><source>Added</source><translation>Hinzugefügt</translation></message>
    <message><source>Administrator</source><translation>Administrator</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>Die ausgewählten Vorgänge wurden abgeschlossen. UWF kann ausgeschlossene Ziele überspringen.</translation></message>
    <message><source>All files (*)</source><translation>Alle Dateien (*)</translation></message>
    <message><source>All levels</source><translation>Alle Stufen</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Eine leere Diagnoseaufnahme bedeutet nicht, dass das Overlay keine Änderungen enthält.</translation></message>
    <message><source>Application path</source><translation>Anwendungspfad</translation></message>
    <message><source>Application result</source><translation>Ergebnis der Anwendung</translation></message>
    <message><source>Apply changes</source><translation>Änderungen anwenden</translation></message>
    <message><source>Architecture</source><translation>Architektur</translation></message>
    <message><source>Automatic</source><translation>Automatisch</translation></message>
    <message><source>Automatic staged file commits</source><translation>Automatische Übernahme vorgemerkter Dateien</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>Automatische Dateiübernahme unvollständig</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>Geeignete vorgemerkte Dateien beim Herunterfahren und Neustarten von Windows automatisch übernehmen.</translation></message>
    <message><source>Available</source><translation>Verfügbar</translation></message>
    <message><source>Back</source><translation>Zurück</translation></message>
    <message><source>Background service</source><translation>Hintergrunddienst</translation></message>
    <message><source>Binding</source><translation>Zuordnung</translation></message>
    <message><source>By drive letter</source><translation>Nach Laufwerkbuchstaben</translation></message>
    <message><source>By volume ID</source><translation>Nach Volume-ID</translation></message>
    <message><source>Cancel reading</source><translation>Einlesen abbrechen</translation></message>
    <message><source>Completed operations cannot be undone.</source><translation>Abgeschlossene Vorgänge können nicht rückgängig gemacht werden.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Wählen Sie den Registrierungsschlüssel aus, der einschließlich seiner Unterschlüssel und Werte vom UWF-Schutz ausgeschlossen werden soll.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Wählen Sie einen Speicherort für die Datei.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Wählen Sie einen Ort innerhalb des zulässigen Laufwerks oder Ordners.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Wählen Sie einen anderen Ordner oder prüfen Sie die Zugriffsrechte und versuchen Sie es erneut.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Wählen Sie einen anderen Schlüssel oder prüfen Sie die Zugriffsrechte und versuchen Sie es erneut.</translation></message>
    <message><source>Clear filters</source><translation>Filter zurücksetzen</translation></message>
    <message><source>Clear logs</source><translation>Protokolle löschen</translation></message>
    <message><source>Clear logs?</source><translation>Protokolle löschen?</translation></message>
    <message><source>Collapse</source><translation>Zuklappen</translation></message>
    <message><source>Commit changes</source><translation>Änderungen übernehmen</translation></message>
    <message><source>Commit changes…</source><translation>Änderungen übernehmen…</translation></message>
    <message><source>Commit file changes?</source><translation>Dateiänderungen übernehmen?</translation></message>
    <message><source>Commit folder changes?</source><translation>Ordneränderungen übernehmen?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Änderungen des Registrierungsschlüssels übernehmen?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Änderungen des Registrierungswerts übernehmen?</translation></message>
    <message><source>Compatibility mode</source><translation>Kompatibilitätsmodus</translation></message>
    <message><source>Configuration applied</source><translation>Konfiguration angewendet</translation></message>
    <message><source>Configuration error</source><translation>Konfigurationsfehler</translation></message>
    <message><source>Configured</source><translation>Konfiguriert</translation></message>
    <message><source>Confirm before continuing</source><translation>Vor dem Fortfahren bestätigen</translation></message>
    <message><source>Connected and authenticated</source><translation>Verbunden und authentifiziert</translation></message>
    <message><source>Connecting…</source><translation>Verbindung wird hergestellt…</translation></message>
    <message><source>Contact author</source><translation>Autor kontaktieren</translation></message>
    <message><source>Continue editing</source><translation>Weiter bearbeiten</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>Wenn Sie den Neustart fortsetzen, können nicht übernommene Dateiänderungen verloren gehen.</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>Wenn Sie das Herunterfahren fortsetzen, können nicht übernommene Dateiänderungen verloren gehen.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Mit dem Herunterfahren von Windows koordinieren</translation></message>
    <message><source>Copy all logs</source><translation>Alle Protokolle kopieren</translation></message>
    <message><source>Copy full report</source><translation>Vollständigen Bericht kopieren</translation></message>
    <message><source>Copy name</source><translation>Namen kopieren</translation></message>
    <message><source>Copy path</source><translation>Pfad kopieren</translation></message>
    <message><source>Copy selection</source><translation>Auswahl kopieren</translation></message>
    <message><source>Copy version</source><translation>Version kopieren</translation></message>
    <message><source>Counting staged files…</source><translation>Vorgemerkte Dateien werden gezählt…</translation></message>
    <message><source>Create</source><translation>Erstellen</translation></message>
    <message><source>Critical %1 MB</source><translation>Kritisch %1 MB</translation></message>
    <message><source>Current filter</source><translation>Aktueller Filter</translation></message>
    <message><source>Current run</source><translation>Aktueller Durchlauf</translation></message>
    <message><source>Current selection</source><translation>Aktuelle Auswahl</translation></message>
    <message><source>Current usage</source><translation>Aktuelle Nutzung</translation></message>
    <message><source>Date modified</source><translation>Änderungsdatum</translation></message>
    <message><source>Debug</source><translation>Fehlersuche</translation></message>
    <message><source>Delete and commit this file?</source><translation>Diese Datei löschen und die Löschung übernehmen?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Diesen Ordner löschen und die Löschung übernehmen?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Diesen Registrierungsschlüssel löschen und die Löschung übernehmen?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Diesen Registrierungswert löschen und die Löschung übernehmen?</translation></message>

    <message><source>Direct restart?</source><translation>Direkt neu starten?</translation></message>
    <message><source>Direct shutdown?</source><translation>Direkt herunterfahren?</translation></message>
    <message><source>Discard changes</source><translation>Änderungen verwerfen</translation></message>
    <message><source>Disk</source><translation>Datenträger</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Persistenz des Domänengeheimnisses</translation></message>
    <message><source>Done</source><translation>Fertig</translation></message>
    <message><source>Edit commands</source><translation>Befehle bearbeiten</translation></message>
    <message><source>Editable</source><translation>Bearbeitbar</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Filter aktivieren und neu starten, um die aktuelle Nutzung anzuzeigen</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>Der erweiterte Modus fügt einen Hintergrunddienst hinzu, der mit Windows startet. Entfernen Sie den Dienst, bevor Sie die Anwendung verschieben oder löschen.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>Der erweiterte Modus ist nicht verfügbar. Verwenden Sie „Sicher herunterfahren“ oder „Sicher neu starten“, um vorgemerkte Dateien zu übernehmen.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>HKLM\SOFTWARE\… eingeben und zum Navigieren die Eingabetaste drücken</translation></message>
    <message><source>Enter a file name</source><translation>Dateinamen eingeben</translation></message>
    <message><source>Enter a valid file name.</source><translation>Geben Sie einen gültigen Dateinamen ein.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Geben Sie einen gültigen Datei- oder Ordnernamen ein.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Geben Sie einen gültigen Ordnernamen ein.</translation></message>
    <message><source>Error</source><translation>Fehler</translation></message>
    <message><source>Error details</source><translation>Fehlerdetails</translation></message>
    <message><source>Error details and suggestions</source><translation>Fehlerdetails und Vorschläge</translation></message>
    <message><source>Expand</source><translation>Aufklappen</translation></message>
    <message><source>Export list</source><translation>Liste exportieren</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Protokolle konnten nicht gelöscht werden: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Protokolle konnten nicht gelöscht werden: unerwarteter Fehler.</translation></message>
    <message><source>File name</source><translation>Dateiname</translation></message>
    <message><source>File or folder</source><translation>Datei oder Ordner</translation></message>
    <message><source>File commits have started and cannot be canceled.</source><translation>Die Dateiübernahme hat begonnen und kann nicht abgebrochen werden.</translation></message>
    <message><source>File commits incomplete</source><translation>Dateiübernahme unvollständig</translation></message>
    <message><source>File type</source><translation>Dateityp</translation></message>
    <message><source>Files and folders</source><translation>Dateien und Ordner</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Dateien und Ordner können gemeinsam ausgewählt werden.</translation></message>
    <message><source>Filter is disabled</source><translation>Filter ist deaktiviert</translation></message>
    <message><source>Fit columns to contents</source><translation>Spalten an Inhalt anpassen</translation></message>
    <message><source>Folder address</source><translation>Ordneradresse</translation></message>
    <message><source>Folder name</source><translation>Ordnername</translation></message>
    <message><source>Folders</source><translation>Ordner</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>Diesen Schlüssel zu den Ausschlüssen der nächsten Sitzung hinzufügen.</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>Diesen Pfad zu den Ausschlüssen der nächsten Sitzung hinzufügen.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Globaler Registrierungsausschluss · einschließlich Unterschlüssel</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Wechseln Sie eine Ordnerebene nach oben oder wählen Sie einen anderen Ort.</translation></message>
    <message><source>Home</source><translation>Benutzerordner</translation></message>
    <message><source>Import configuration</source><translation>Konfiguration importieren</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>Der Import aktualisiert den Entwurf. Bestätigen Sie die Änderungen unter „Prüfen und anwenden“, um sie ins System zu schreiben.</translation></message>
    <message><source>Import results</source><translation>Importergebnisse</translation></message>
    <message><source>Imported</source><translation>Importiert</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>Die Liste zeigt die Ergebnisse der Befehle für den Konfigurationsentwurf. Wählen Sie eine Zeile, um den vollständigen Befehl und Details anzuzeigen.</translation></message>
    <message><source>Imported: %1</source><translation>Importiert: %1</translation></message>
    <message><source>Information</source><translation>Information</translation></message>
    <message><source>Item details</source><translation>Eintragsdetails</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>Die Übernahme vieler vorgemerkter Dateien kann länger dauern.</translation></message>
    <message><source>License and copyright</source><translation>Lizenz und Urheberrecht</translation></message>
    <message><source>Line</source><translation>Zeile</translation></message>
    <message><source>Local disk (%1)</source><translation>Lokaler Datenträger (%1)</translation></message>
    <message><source>Log level</source><translation>Protokollstufe</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Protokolle konnten nicht gelesen werden. Versuchen Sie es erneut oder prüfen Sie den Zugriff auf die Protokolldatei.</translation></message>
    <message><source>Maximize</source><translation>Maximieren</translation></message>
    <message><source>Message details</source><translation>Nachrichtendetails</translation></message>
    <message><source>Minimize</source><translation>Minimieren</translation></message>
    <message><source>Module</source><translation>Modul</translation></message>
    <message><source>More</source><translation>Mehr</translation></message>
    <message><source>Move</source><translation>Verschieben</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Wenn UWF das Laufwerk der Anwendung schützt, stellen Sie zunächst sicher, dass die Anwendungsdateien nach einem Neustart erhalten bleiben. Die Übernahme vieler vorgemerkter Dateien kann das Herunterfahren oder den Neustart verlängern.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Mehrere Dateien können ausgewählt werden.</translation></message>
    <message><source>Name and location</source><translation>Name und Speicherort</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Benannter Wert: %1 · nur dieser Wert</translation></message>
    <message><source>Navigate to registry path</source><translation>Zum Registrierungspfad navigieren</translation></message>
    <message><source>New folder</source><translation>Neuer Ordner</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Nach dem Aktualisieren werden neue Datensätze angezeigt.</translation></message>
    <message><source>Next-session configuration</source><translation>Konfiguration der nächsten Sitzung</translation></message>
    <message><source>No files to commit</source><translation>Keine Dateien zu übernehmen</translation></message>
    <message><source>No logs yet</source><translation>Noch keine Protokolle</translation></message>
    <message><source>No matching items</source><translation>Keine passenden Einträge</translation></message>
    <message><source>No matching logs</source><translation>Keine passenden Protokolle</translation></message>
    <message><source>No overlay files found</source><translation>Keine Overlay-Dateien gefunden</translation></message>
    <message><source>No registry key selected</source><translation>Kein Registrierungsschlüssel ausgewählt</translation></message>
    <message><source>None</source><translation>Keine</translation></message>
    <message><source>Not configured</source><translation>Nicht konfiguriert</translation></message>
    <message><source>Not connected</source><translation>Nicht verbunden</translation></message>
    <message><source>Not installed</source><translation>Nicht installiert</translation></message>
    <message><source>Not written to the system</source><translation>Nicht in das System geschrieben</translation></message>
    <message><source>Off</source><translation>Aus</translation></message>
    <message><source>On</source><translation>Ein</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Ein uwfmgr-Befehl pro Zeile</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Speichern Sie die Anwendung in einem festen Ordner, den Standardbenutzer nicht ändern können. Empfohlener Speicherort:</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>Nur vorgemerkte Dateien werden automatisch übernommen. Andere nicht übernommene Änderungen können nach dem Neustart verloren gehen.</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>Nur vorgemerkte Dateien werden automatisch übernommen. Andere nicht übernommene Änderungen können nach dem Herunterfahren verloren gehen.</translation></message>
    <message><source>Open</source><translation>Öffnen</translation></message>
    <message><source>Open folder</source><translation>Ordner öffnen</translation></message>
    <message><source>Operating system</source><translation>Betriebssystem</translation></message>
    <message><source>Operation and security</source><translation>Betrieb und Sicherheit</translation></message>
    <message><source>Original command</source><translation>Ursprünglicher Befehl</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Kritischer Overlay-Schwellenwert erreicht. Speichern Sie Änderungen und starten Sie sicher neu.</translation></message>
    <message><source>Maximum overlay capacity</source><translation>Maximale Overlay-Kapazität</translation></message>
    <message><source>Overlay type</source><translation>Overlay-Typ</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Overlay-Warnschwellenwert erreicht. Speichern Sie Änderungen und starten Sie sicher neu.</translation></message>
    <message><source>Overview</source><translation>Übersicht</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Befehle einfügen oder aus einer Datei laden…</translation></message>
    <message><source>Pending changes</source><translation>Ausstehende Änderungen</translation></message>
    <message><source>Pending removal</source><translation>Zur Entfernung vorgemerkt</translation></message>
    <message><source>Permissions and protection</source><translation>Berechtigungen und Schutz</translation></message>
    <message><source>Privileges</source><translation>Privilegien</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2 Einträge verarbeitet</translation></message>
    <message><source>Processing result</source><translation>Verarbeitungsergebnis</translation></message>
    <message><source>Protected</source><translation>Geschützt</translation></message>
    <message><source>Protected volumes</source><translation>Geschützte Volumes</translation></message>
    <message><source>Protection and overlay</source><translation>Schutz und Overlay</translation></message>
    <message><source>Quick access</source><translation>Schnellzugriff</translation></message>
    <message><source>Raw report</source><translation>Rohbericht</translation></message>
    <message><source>Read again</source><translation>Erneut lesen</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Lesen Sie erneut ein, um eine Diagnoseaufnahme abzurufen.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Konfiguration einlesen, um die aktuelle Nutzung anzuzeigen</translation></message>
    <message><source>Read-only</source><translation>Schreibgeschützt</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Das Lesen eines großen Overlays kann länger dauern. Sie können jederzeit abbrechen.</translation></message>
    <message><source>Reading canceled</source><translation>Einlesen abgebrochen</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Dateien und Unterordner an diesem Ort werden gelesen…</translation></message>
    <message><source>Reading folder</source><translation>Ordner wird gelesen</translation></message>
    <message><source>Reading logs</source><translation>Protokolle werden gelesen</translation></message>
    <message><source>Reading overlay</source><translation>Overlay wird gelesen</translation></message>
    <message><source>Ready</source><translation>Bereit</translation></message>
    <message><source>Files to commit</source><translation>Zu übernehmende Dateien</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Nach Abschluss des Einlesens erscheinen hier Datensätze des aktuellen Durchlaufs.</translation></message>

    <message><source>Refresh folder</source><translation>Ordner aktualisieren</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Registrierungsschlüssel oder Persistenzeinstellung</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>Beim Abbrechen läuft der aktuelle Vorgang zu Ende; danach werden keine weiteren Aufgaben ausgeführt.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Diesen Schlüssel aus den Ausschlüssen der nächsten Sitzung entfernen.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Diesen Pfad aus den Ausschlüssen der nächsten Sitzung entfernen.</translation></message>
    <message><source>Removed</source><translation>Entfernt</translation></message>
    <message><source>Rename</source><translation>Umbenennen</translation></message>
    <message><source>Rename…</source><translation>Umbenennen…</translation></message>
    <message><source>Repair required</source><translation>Reparatur erforderlich</translation></message>
    <message><source>Replace file</source><translation>Datei ersetzen</translation></message>
    <message><source>Replace the existing file?</source><translation>Vorhandene Datei ersetzen?</translation></message>
    <message><source>Required configuration</source><translation>Erforderliche Konfiguration</translation></message>
    <message><source>Restart anyway</source><translation>Trotzdem neu starten</translation></message>
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>Nach der Übernahme wird der Neustart fortgesetzt. Lassen Sie die Anwendung geöffnet.</translation></message>
    <message><source>Restore</source><translation>Wiederherstellen</translation></message>
    <message><source>Retry</source><translation>Erneut versuchen</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Prüfen Sie die Fehlerdetails, bevor Sie das Herunterfahren fortsetzen.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Prüfen Sie die Fehlerdetails vor dem Neustart.</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>Einige Änderungen wurden möglicherweise bereits geschrieben. Aktualisieren Sie den Systemzustand vor einem erneuten Versuch.</translation></message>
    <message><source>Review the result for each target.</source><translation>Prüfen Sie das Ergebnis für jedes Ziel.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Prüfen Sie diese Änderungen, bevor Sie sie in das System schreiben.</translation></message>
    <message><source>Running</source><translation>Wird ausgeführt</translation></message>
    <message><source>Save</source><translation>Speichern</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>Vorgemerkte Dateien auf den Datenträger übernehmen, dann neu starten.</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>Vorgemerkte Dateien auf den Datenträger übernehmen, dann herunterfahren.</translation></message>

    <message><source>Committing to disk</source><translation>Übernahme auf den Datenträger läuft</translation></message>
    <message><source>Committing…</source><translation>Übernahme läuft…</translation></message>
    <message><source>Scanning staged paths</source><translation>Vorgemerkte Pfade werden durchsucht</translation></message>
    <message><source>Search logs</source><translation>Protokolle durchsuchen</translation></message>
    <message><source>Search messages or modules</source><translation>Nachrichten oder Module durchsuchen</translation></message>
    <message><source>Search paths</source><translation>Pfade durchsuchen</translation></message>
    <message><source>Search this folder</source><translation>Diesen Ordner durchsuchen</translation></message>
    <message><source>Select a file or enter its name</source><translation>Datei auswählen oder Namen eingeben</translation></message>
    <message><source>Select a file to open.</source><translation>Wählen Sie eine Datei zum Öffnen.</translation></message>
    <message><source>Select a folder.</source><translation>Wählen Sie einen Ordner.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Wählen Sie einen Schlüssel, um seinen gesamten Unterbaum zu bearbeiten, oder einen benannten Wert, um nur diesen Wert zu bearbeiten.</translation></message>
    <message><source>Select an available subkey</source><translation>Verfügbaren Unterschlüssel auswählen</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Wählen Sie nur Dateien oder öffnen Sie einen Ordner, um seinen Inhalt zu durchsuchen.</translation></message>
    <message><source>Select files or folders</source><translation>Dateien oder Ordner auswählen</translation></message>
    <message><source>Select folder</source><translation>Ordner auswählen</translation></message>
    <message><source>Select folders only.</source><translation>Wählen Sie nur Ordner.</translation></message>
    <message><source>Select registry key or value</source><translation>Registrierungsschlüssel oder -wert auswählen</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Zu löschenden Registrierungsschlüssel oder -wert auswählen</translation></message>
    <message><source>Select the current folder</source><translation>Aktuellen Ordner auswählen</translation></message>
    <message><source>Select this file</source><translation>Diese Datei auswählen</translation></message>
    <message><source>Select this registry key</source><translation>Diesen Registrierungsschlüssel auswählen</translation></message>
    <message><source>Select this registry value</source><translation>Diesen Registrierungswert auswählen</translation></message>
    <message><source>Selected file</source><translation>Ausgewählte Datei</translation></message>
    <message><source>Selected folder</source><translation>Ausgewählter Ordner</translation></message>
    <message><source>Selection</source><translation>Auswahl</translation></message>
    <message><source>Service account</source><translation>Dienstkonto</translation></message>
    <message><source>Service configuration needs repair</source><translation>Dienstkonfiguration muss repariert werden</translation></message>
    <message><source>Service details</source><translation>Dienstdetails</translation></message>
    <message><source>Service name</source><translation>Dienstname</translation></message>
    <message><source>Service status</source><translation>Dienststatus</translation></message>
    <message><source>Service stopped</source><translation>Dienst angehalten</translation></message>
    <message><source>Shut down anyway</source><translation>Trotzdem herunterfahren</translation></message>
    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>Nach der Übernahme wird das Herunterfahren fortgesetzt. Lassen Sie die Anwendung geöffnet.</translation></message>
    <message><source>Size</source><translation>Größe</translation></message>
    <message><source>Some changes could not be applied</source><translation>Einige Änderungen konnten nicht angewendet werden</translation></message>
    <message><source>Some files could not be committed</source><translation>Einige Dateien konnten nicht übernommen werden</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Einige Hardwareinformationen konnten nicht gelesen werden. Andere Diagnosedaten können weiterhin angezeigt und kopiert werden.</translation></message>
    <message><source>Source code</source><translation>Quellcode</translation></message>

    <message><source>Staged path</source><translation>Vorgemerkter Pfad</translation></message>
    <message><source>Standard user</source><translation>Standardbenutzer</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Startet automatisch mit Windows</translation></message>
    <message><source>Startup type</source><translation>Starttyp</translation></message>
    <message><source>Status unavailable</source><translation>Status nicht verfügbar</translation></message>
    <message><source>Stopped</source><translation>Angehalten</translation></message>
    <message><source>Storage</source><translation>Speicher</translation></message>
    <message><source>System</source><translation>System</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Systemänderungen beim ersten Aktivieren von UWF</translation></message>
    <message><source>System memory</source><translation>Arbeitsspeicher</translation></message>
    <message><source>System metadata</source><translation>Systemmetadaten</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Systemmetadaten dienen nur der Diagnose und können nicht übernommen werden.</translation></message>
    <message><source>Takes effect after restart</source><translation>Wirksam nach Neustart</translation></message>
    <message><source>Takes effect when applied</source><translation>Wirksam beim Anwenden</translation></message>
    <message><source>Used when UWF starts</source><translation>Wird beim Start von UWF verwendet</translation></message>
    <message><source>Mixed activation times</source><translation>Verschiedene Aktivierungszeiten</translation></message>
    <message><source>Target</source><translation>Ziel</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistenz von Terminaldienste-Lizenzen</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Der UWF-Anbieter konnte die Auflistung nicht abschließen. Dies kann bei großem Overlay oder unzureichenden Systemressourcen auftreten.</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>Die Konfiguration wurde in das System geschrieben.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Die aktuellen Protokolldatensätze werden unwiederbringlich gelöscht.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Der Zielordner existiert nicht.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Der Dienststatus des erweiterten Modus konnte nicht gelesen werden. Prüfen Sie die Dienstberechtigungen und versuchen Sie es erneut.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>Die Liste kann Dateien auslassen oder bereits übernommene Dateien enthalten. Verwenden Sie sie nicht als Übernahmeliste.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Der ausgewählte Eintrag existiert nicht mehr: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>Der Dienst ist installiert, aber angehalten. Starten Sie ihn, um die automatische Dateiübernahme fortzusetzen.</translation></message>
    <message><source>This PC</source><translation>Dieser PC</translation></message>
    <message><source>This folder is empty</source><translation>Dieser Ordner ist leer</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Dieser Bericht kann Kontonamen, Gerätekennungen und Dateipfade enthalten. Prüfen Sie ihn vor der Weitergabe.</translation></message>

    <message><source>Try another keyword or log level.</source><translation>Versuchen Sie ein anderes Stichwort oder eine andere Protokollstufe.</translation></message>
    <message><source>Try another search term.</source><translation>Versuchen Sie einen anderen Suchbegriff.</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>Deaktivieren Sie UWF und starten Sie neu, bevor Sie den Overlay-Typ oder die maximale Kapazität ändern.</translation></message>
    <message><source>UI agent</source><translation>UI-Agent</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 Zeilen</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>Die UWF-Konfiguration konnte nicht gelesen werden. Prüfen Sie, ob Unified Write Filter in Windows installiert ist, und lesen Sie erneut ein.</translation></message>
    <message><source>UWF feature</source><translation>UWF-Feature</translation></message>
    <message><source>UWF filter</source><translation>UWF-Filter</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF ist derzeit nicht verfügbar</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Der sichere UWF-Neustart wird direkt ausgeführt.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>Das sichere UWF-Herunterfahren wird direkt ausgeführt.</translation></message>
    <message><source>Unable to access this folder</source><translation>Kein Zugriff auf diesen Ordner</translation></message>
    <message><source>Unable to access this registry key</source><translation>Kein Zugriff auf diesen Registrierungsschlüssel</translation></message>
    <message><source>Unable to read logs</source><translation>Protokolle können nicht gelesen werden</translation></message>
    <message><source>Unable to read service status</source><translation>Dienststatus kann nicht gelesen werden</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Overlay kann derzeit nicht gelesen werden</translation></message>
    <message><source>Unable to read this folder.</source><translation>Dieser Ordner kann nicht gelesen werden.</translation></message>
    <message><source>Unable to select this item</source><translation>Dieser Eintrag kann nicht ausgewählt werden</translation></message>
    <message><source>Unavailable</source><translation>Nicht verfügbar</translation></message>
    <message><source>Undo removal</source><translation>Entfernung rückgängig machen</translation></message>
    <message><source>Unprotected</source><translation>Ungeschützt</translation></message>
    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>Einige Einträge wurden nicht vollständig verarbeitet oder konnten nicht überprüft werden. Wählen Sie einen Eintrag, um den vollständigen Pfad und die Details anzuzeigen.</translation></message>
    <message><source>Up one folder</source><translation>Eine Ordnerebene nach oben</translation></message>
    <message><source>Usage unavailable</source><translation>Nutzung nicht verfügbar</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Gesamten Schlüssel einschließlich aller Unterschlüssel verwenden</translation></message>
    <message><source>Version %1</source><translation>Version %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Anwendungsprotokolle und Diagnosedaten anzeigen.</translation></message>
    <message><source>View commands for these changes</source><translation>Befehle für diese Änderungen anzeigen</translation></message>
    <message><source>View failure details</source><translation>Fehlerdetails anzeigen</translation></message>
    <message><source>View overlay</source><translation>Overlay anzeigen</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Dateidiagnoseaufnahme für %1 anzeigen.</translation></message>
    <message><source>Volume %1 binding</source><translation>Zuordnung für Volume %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Schutz für Volume %1</translation></message>
    <message><source>Volume identity</source><translation>Volume-Identität</translation></message>
    <message><source>Warning</source><translation>Warnung</translation></message>
    <message><source>Warning %1 MB</source><translation>Warnung %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Gesamter Schlüssel und alle Unterschlüssel · rekursiv</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF stellt den Schreibschutz bereit. Diese Anwendung konfiguriert und verwaltet ihn.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>Ohne erweiterten Modus verwenden Sie „Sicher herunterfahren“ oder „Sicher neu starten“ in dieser Anwendung, um vorgemerkte Dateien zu übernehmen.</translation></message>
    <message><source>Write protection</source><translation>Schreibschutz</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>Diese Konfigurationsänderungen in das System schreiben?</translation></message>
    <message><source>staged files</source><translation>vorgemerkte Dateien</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr-Befehle</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Ausgewählte Pfade zum Entfernen vormerken. Wird nach dem Anwenden wirksam.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Nach dem Anwenden aus der Dateivormerkungsliste entfernen.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Nach dem Anwenden zur Dateivormerkungsliste hinzufügen.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Beim Anwenden dieses Plans wird die gespeicherte Dateivormerkungsliste geleert und ihr Registrierungsausschluss entfernt, wenn er nicht mehr benötigt wird.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Änderungen der Dateivormerkung müssen in UWF Manager angewendet werden und können nicht als uwfmgr-Befehle exportiert werden.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Die Dateivormerkungsliste wird nach dem Anwenden gespeichert</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Umfang des Overlay-Snapshots</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Diese Liste dient nur der Diagnose. Sie zeigt nicht, welche Dateien übernommen werden müssen.</translation></message>
    <message><source>NTFS volumes only</source><translation>Nur NTFS-Volumes</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Bei großen Overlays kann das Lesen langsam sein oder wegen unzureichender Ressourcen fehlschlagen.</translation></message>
    <message><source>Small files may be missing</source><translation>Kleine Dateien können fehlen</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Dateien unterhalb der Clustergröße des Datenträgers (meist 4 KB) erscheinen möglicherweise nicht in der Liste.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Zeigt keine ausstehenden Übernahmen an</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Bereits übernommene Dateien, ausgeschlossene Pfade und durch andere Vorgänge betroffene Einträge können ebenfalls erscheinen.</translation></message>

    <message><source>Command processing result</source><translation>Ergebnis der Befehlsverarbeitung</translation></message>
    <message><source>Copy command</source><translation>Befehl kopieren</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 Zweige konnten nicht gelesen werden</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 Zweige konnten nicht gelesen werden. Mit gefundenen Inhalten fortfahren oder nur fehlgeschlagene Einträge erneut versuchen.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 Dateien · %2 Ordner</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 Werte · %2 Schlüssel</translation></message>
    <message><source>Attempts</source><translation>Versuche</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Restliche Aufgaben abbrechen</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Nur erfolgreich durchsuchte Inhalte übernehmen. Nicht gelesene Zweige bleiben unverändert.</translation></message>
    <message><source>Committing changes</source><translation>Änderungen werden übernommen</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Abgeschlossene Löschungen und Übernahmen können nicht rückgängig gemacht werden.</translation></message>
    <message><source>Continue…</source><translation>Weiter…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Nur erfolgreich durchsuchte Ziele löschen und die Löschungen übernehmen; Zweige, die nicht gelesen werden konnten, sowie ihre übergeordneten Ordner oder Schlüssel bleiben erhalten.</translation></message>
    <message><source>Deleting and committing</source><translation>Löschen und Übernehmen läuft</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Löschen und Übernehmen können nicht rückgängig gemacht werden. Nur der gewählte Bereich wird verarbeitet; ausstehende UWF-Einstellungen werden nicht angewendet.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Der Umfang wird ermittelt. Nicht lesbare Zweige werden protokolliert; andere Inhalte werden weiter durchsucht.</translation></message>
    <message><source>Export traversal records</source><translation>Suchprotokoll exportieren</translation></message>
    <message><source>No actionable content</source><translation>Keine verarbeitbaren Inhalte</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Keine verarbeitbaren Ziele gefunden. Es wurde nichts gelöscht oder übernommen.</translation></message>
    <message><source>No changes have been committed.</source><translation>Noch keine Änderungen übernommen</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Noch nichts gelöscht oder übernommen</translation></message>
    <message><source>Path</source><translation>Pfad</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>Die ausgewählten Inhalte dauerhaft löschen und die Löschungen übernehmen.</translation></message>
    <message><source>Preparation complete</source><translation>Vorbereitung abgeschlossen</translation></message>
    <message><source>Preparing commit</source><translation>Übernahme wird vorbereitet</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Löschen und Übernehmen vorbereiten</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Nur erfolgreich durchsuchte Inhalte verarbeiten</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Analysepunkt-Verknüpfungen werden übersprungen, damit die Suche im gewählten Bereich bleibt.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Ein Neustart macht die Übernahme nicht rückgängig. Nur der gewählte Bereich wird verarbeitet; ausstehende UWF-Einstellungen werden nicht angewendet.</translation></message>
    <message><source>Retry failed items</source><translation>Fehlgeschlagene erneut versuchen</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>Änderungen im ausgewählten Bereich übernehmen.</translation></message>
    <message><source>Stopping the operation</source><translation>Vorgang wird angehalten</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Erfolgs- und Fehlerprotokolle bleiben erhalten</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Der Ordner ist nicht leer; unverarbeitete Inhalte bleiben erhalten.</translation></message>
    <message><source>The operation could not be completed</source><translation>Der Vorgang konnte nicht abgeschlossen werden</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>Der Registrierungsschlüssel enthält neue Werte; unverarbeitete Inhalte bleiben erhalten.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>Der Registrierungsschlüssel hat noch Unterschlüssel; unverarbeitete Inhalte bleiben erhalten.</translation></message>
    <message><source>The target no longer exists.</source><translation>Das Ziel ist nicht mehr vorhanden.</translation></message>
    <message><source>Traversal complete</source><translation>Suche abgeschlossen</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Suche abgeschlossen, mit nicht gelesenen Inhalten</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Das Suchprotokoll bleibt erhalten und kann exportiert werden.</translation></message>
    <message><source>Traverse again</source><translation>Erneut durchsuchen</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Erneut durchsuchen oder dieses Fenster schließen.</translation></message>
    <message><source>Traversing the folder</source><translation>Ordner wird durchsucht</translation></message>
    <message><source>Traversing the registry</source><translation>Registrierung wird durchsucht</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Der Vorgang kann nicht vorbereitet werden</translation></message>
    <message><source>View results</source><translation>Ergebnisse anzeigen</translation></message>

    <message><source>All operations succeeded</source><translation>Alle Vorgänge erfolgreich</translation></message>
    <message><source>Some operations succeeded</source><translation>Einige Vorgänge erfolgreich</translation></message>
    <message><source>No operations succeeded</source><translation>Keine Vorgänge erfolgreich</translation></message>
    <message><source>No targets to process</source><translation>Keine Ziele zu verarbeiten</translation></message>
    <message><source>Commit interrupted</source><translation>Übernahme unterbrochen</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 Elemente wurden nicht verarbeitet.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 Registrierungsvorgänge sind fehlgeschlagen.</translation></message>
    <message><source>The folder does not exist.</source><translation>Der Ordner ist nicht vorhanden.</translation></message>
    <message><source>Access denied.</source><translation>Zugriff verweigert.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Ein Element mit diesem Namen ist bereits vorhanden.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Der Ordner konnte nicht erstellt werden. Prüfen Sie den Namen und die Berechtigungen.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Das Element konnte nicht umbenannt werden. Prüfen Sie den Namen und die Berechtigungen.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Dieser Browser unterstützt das Umbenennen nicht.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Der Dateivormerkungsentwurf wurde nach dem Öffnen dieses Plans geändert. Prüfen Sie einen neuen Plan.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Die gespeicherte Dateivormerkungsliste wurde nach dem Öffnen dieses Plans geändert. Aktualisieren und prüfen Sie die Änderungen.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Die gespeicherte Dateivormerkungsliste stimmt nicht mit dem bestätigten Plan überein.</translation></message>
    <message><source>Export interface settings</source><translation>Oberflächeneinstellungen exportieren</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>Aktuelles Design, Sprache und Sichtbarkeit des Overlay-Widgets in UWF.ini neben dieser Anwendung speichern.</translation></message>
    <message><source>Interface settings saved</source><translation>Oberflächeneinstellungen gespeichert</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Die aktuellen Oberflächeneinstellungen wurden gespeichert unter:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini ist bereits vorhanden. Die vorhandene Datei wurde nicht geändert:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>UWF.ini konnte nicht gespeichert werden:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Desktop</translation></message>
    <message><source>Downloads</source><translation>Downloads</translation></message>
    <message><source>Documents</source><translation>Dokumente</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Die Übernahme vorgemerkter Dateien wurde angehalten. Beheben Sie den Fehler vor einem erneuten Versuch.</translation></message>
    <message><source>No net change for this setting</source><translation>Diese Einstellung ist nach dem Import unverändert</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Endgültiger Entwurfswert: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Endgültiger Entwurfswert: %1 → %2 (angefordert: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Angefordert: %1 → %2; durch einen späteren Befehl ersetzt. Endgültiger Entwurfswert → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Entwurf für %1 aktualisiert</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Entwurf der Schutzeinstellung für Volume %1 aktualisiert → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Wählen Sie die zu entfernenden Einträge aus.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Die ausgewählten Einträge können derzeit nicht wiederhergestellt werden.</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>Durchleitung über freien Speicherplatz</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>Persistentes Overlay</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>Persistente Daten beim nächsten Start löschen</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>Modus für schreibgeschützte Medien</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>Nutzt freien Datenträgerspeicher für Schreibvorgänge und reduziert die Belegung des reservierten Overlays; unterstützt RAM (nicht empfohlen) und Disk (empfohlen).</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>Behält Schreibvorgänge im Disk-Overlay über Neustarts hinweg; für RAM-Overlays nicht verfügbar.</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>Setzt das persistente Overlay zurück: Löscht beim nächsten Start dessen Schreibvorgänge und stellt den Zustand bei Aktivierung des Overlays wieder her</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>Administratorrechte erforderlich; die Overlay-Konfiguration kann nur angezeigt werden.</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>Die Konfiguration kann vorübergehend nicht bearbeitet werden. Warten Sie, bis der aktuelle Vorgang abgeschlossen ist.</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>Die Overlay-Konfiguration konnte nicht gelesen werden. Vor dem Bearbeiten aktualisieren.</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>Dieses System unterstützt das Löschen persistenter Daten beim nächsten Start nicht.</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>Die aktuelle Löschanforderung ist unbekannt. Aktualisieren Sie die Konfiguration vor dem Bearbeiten.</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>Zum Ändern dieser Anforderung müssen UWF und das persistente Disk-Overlay in der aktuellen Sitzung aktiv sein.</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>Das Löschen beim nächsten Start erfordert ein persistentes Disk-Overlay. Brechen Sie die Löschanforderung ab oder behalten Sie das persistente Overlay bei.</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>Ein persistentes Overlay erfordert Disk. Deaktivieren Sie es oder wählen Sie Disk.</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>Der Modus für schreibgeschützte Medien erfordert RAM. Deaktivieren Sie ihn oder wählen Sie RAM.</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>UWF ist in der aktuellen Sitzung aktiviert. Deaktivieren Sie UWF und starten Sie neu, bevor Sie diese Einstellung ändern.</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>Die Konfiguration wurde geändert. Schließen und öffnen Sie diesen Dialog vor dem Bearbeiten erneut.</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>Erweiterte Overlay-Einstellungen</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>Aktuelles Overlay</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>Nächstes Overlay</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>Der Entwurf enthält unvereinbare oder nicht verfügbare Overlay-Einstellungen. Kehren Sie zur Bearbeitung zurück und korrigieren Sie die Overlay-Konfiguration.</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>Für den nächsten Start ist ein Löschen geplant. Brechen Sie die Löschanforderung ab, bevor Sie das persistente Overlay deaktivieren.</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>Die nächste Sitzung behält das persistente Disk-Overlay nicht bei. Behalten Sie es bei, bevor Sie das Löschen anfordern.</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>Ein persistentes Overlay ist nur für Disk verfügbar; das nächste Overlay ist RAM.</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>Der Modus für schreibgeschützte Medien erfordert RAM; das nächste Overlay ist Disk.</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>Der aktuelle Zustand des persistenten Overlays konnte nicht bestätigt werden. Lesen Sie die Konfiguration erneut ein.</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>Wird nach dem Anwenden beim nächsten Start wirksam.</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>Löschen geplant</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>Kein Löschen geplant</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>Löscht einmal beim nächsten Start und behält danach Daten bei. Die aktuelle Sitzung wird nicht sofort gelöscht.</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>Bricht das geplante Löschen beim nächsten Start ab und behält die Daten des persistenten Overlays bei.</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>Persistentes Overlay zurücksetzen · Einmalige Anforderung</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Wird bearbeitet…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>Bitte warten Sie, bis der Vorgang abgeschlossen ist.</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · Nächste Sitzung</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>Zurück zu den Einstellungen</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>Voraussetzungen für Änderungen</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>Geändert</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>Konfigurationskonflikte prüfen</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>Konfigurationskonflikt</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>Voraussetzungen zum Deaktivieren</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>UWF und das persistente Disk-Overlay müssen in der aktuellen Sitzung aktiv sein. Zum Löschen der Daten muss das persistente Disk-Overlay auch für die nächste Sitzung beibehalten werden.</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>Voraussetzungen zum Aktivieren</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Funktion</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>Funktionsbeschreibung</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>Informationen zu %1</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>Beheben Sie diese Konfigurationskonflikte, bevor Sie den Entwurf beibehalten.</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>Zurücksetzen</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>Wirksamkeit</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>Diese Einstellung kann derzeit nicht geändert werden</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>UWF muss in der aktuellen Sitzung deaktiviert sein und das Overlay der nächsten Sitzung muss Disk verwenden.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>UWF muss in der aktuellen Sitzung deaktiviert sein und das Overlay der nächsten Sitzung muss RAM verwenden.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>UWF muss in der aktuellen Sitzung deaktiviert sein. Brechen Sie eine geplante Löschung persistenter Daten ab, bevor Sie das persistente Overlay deaktivieren.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>UWF muss in der aktuellen Sitzung deaktiviert sein. Wenn UWF nur für die nächste Sitzung deaktiviert wird, ist vor dem Bearbeiten weiterhin ein Neustart erforderlich.</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>Nutzungsvoraussetzungen</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Details anzeigen</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>Nur ansehen</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>Warum %1 nicht geändert werden kann</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>UWF ist in der aktuellen Sitzung aktiviert. Daher können diese Overlay-Einstellungen nicht geändert werden. Machen Sie die Änderungen rückgängig oder deaktivieren Sie UWF und starten Sie neu.</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>Voraussetzungen für Änderungen anzeigen</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>Es wurden keine Konfigurationsänderungen geschrieben. Sie können es erneut versuchen.</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>Es waren keine Konfigurationsänderungen erforderlich.</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>Verwenden Sie für das Domänencomputerkontogeheimnis ($MACHINE.ACC) die Einstellung „Persistenz des Domänengeheimnisses“.</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>Dieser Schlüssel selbst kann nicht ausgeschlossen werden. Wählen Sie einen seiner Unterschlüssel.</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>Registrierungsausschlüsse müssen einen nicht leeren Unterschlüssel unter HKEY_LOCAL_MACHINE (HKLM) angeben.</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>Dieses Skript enthält nur Einstellungen, die mit uwfmgr-Befehlen darstellbar sind. Nicht enthalten:</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>Volumebindung</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>Einstellungen ohne verlässliche Zuordnung zu einem Laufwerksbuchstaben</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>Befehlsskript</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>Die Konfiguration entspricht dem Ziel für die nächste Sitzung. Unter „Befehlsskript“ können Sie die unterstützten Einstellungen anzeigen oder exportieren.</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>Einstellungen mit Steuerzeichen in Pfaden oder Namen</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>Befehle, die von ausgelassenen Einstellungen abhängen</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>Die UWF-Ausschlusseinstellungen verhindern eine zuverlässige Übernahme dieses Ziels. Entfernen Sie den widersprüchlichen Ausschluss und starten Sie vor der Übernahme neu.</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>Die UWF-Ausschlusseinstellungen verhindern eine zuverlässige Übernahme der Dateivormerkliste. Entfernen Sie den widersprüchlichen Ausschluss und starten Sie vor dem Anwenden dieser Änderung neu.</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF kann nicht sicherstellen, dass die Operation den angegebenen Registrierungsschlüssel unverändert als Ziel verwendet.</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>Bestätigen Sie vor dem Entfernen des Registrierungsausschlusses, dass die gespeicherte Dateivormerkliste leer ist.</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>Bestätigung ausstehend</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Leer</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>Das Systemvolume ist derzeit nicht durch UWF geschützt; es gibt keine zu übernehmenden Registrierungsänderungen.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>Registrierungsänderungen sind bereits dauerhaft gespeichert; eine UWF-Übernahme ist nicht erforderlich.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>Registrierungsänderungen sind bereits dauerhaft gespeichert; UWF kann diese Löschung nicht ausführen.</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>UWF-Ausschlusspfade müssen umgekehrte Schrägstriche verwenden.</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>Dateipfade dürfen keine Nullzeichen enthalten.</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>Aktivieren Sie den Volumeschutz in diesem Entwurf, bevor Sie Dateiausschlüsse hinzufügen.</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>Der Ausschlusspfad enthält Zeichen, die UWF nicht unterstützt.</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>Der Ausschlusspfad überschreitet die Längenbegrenzung von UWF.</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>Solange HORM aktiv ist, können Datei- und Registrierungsänderungen nicht übernommen werden.</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>Solange HORM aktiv ist, sind Schreibvorgänge in der Registrierung nicht möglich.</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>Solange der Modus für schreibgeschützte Medien aktiv ist, können Schreibvorgänge in der Registrierung nicht dauerhaft gespeichert werden.</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>Solange der Modus für schreibgeschützte Medien aktiv ist, können Datei- und Registrierungsänderungen nicht übernommen werden.</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>Fängt Schreibzugriffe auf physische Medien einschließlich Metadaten ab. Ausschlüsse umgehen den Schutz nicht. Einzelne Datei- und Registrierungsänderungen können nicht übernommen werden; Full Commit ist ein separater globaler Vorgang.</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>Solange HORM aktiv ist, können UWF-Einstellungen nicht geändert werden.</translation></message>
    <message><source>Default value · this value only</source><translation>Standardwert · nur dieser Wert</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>Dateiänderungen sind bereits dauerhaft gespeichert; ein UWF-Commit ist nicht erforderlich.</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>Letzte erfolgreiche Erfassung der Overlay-Nutzung: %1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>Erfasster Overlay-Speicher außerhalb des schreibbaren Größen-/Typbereichs</translation></message>
    <message><source>Overlay (last observed)</source><translation>Overlay (zuletzt erfasst)</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>Aktualisierung der Overlay-Nutzung fehlgeschlagen.
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>Aktualisierung der Overlay-Nutzung fehlgeschlagen. Die letzte erfolgreiche Erfassung vom %1 wird angezeigt.
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>Der Pfad konnte keinem lokalen Volume zugeordnet werden.</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF: Aktiviert (zuletzt erfasst)</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>Dateiänderungen sind bereits dauerhaft gespeichert; UWF kann diese Löschung nicht ausführen.</translation></message>
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>%1 GB verfügbar</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>Administratorrechte sind erforderlich.</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>Ein Vorgang läuft. Bitte warten Sie.</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>Versucht, alle seit diesem Start hinzugefügten, geänderten und gelöschten Daten auf sämtlichen geschützten Volumes zu speichern. Dazu gehören Dateien und Systemeinstellungen; einzelne Änderungen können nicht ausgewählt werden.</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>Automatisches Speichern ist im aktuellen Modus nicht verfügbar</translation></message>
    <message><source>Cannot be undone once started</source><translation>Nach dem Start nicht mehr rückgängig zu machen</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>Speicherort für das Disk-Overlay auswählen</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>Wählen Sie den Speicherort für das Disk-Overlay aus.</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>Konfigurationsänderungen sind vorübergehend nicht verfügbar.</translation></message>
    <message><source>Configured, inactive this session</source><translation>Konfiguriert, in dieser Sitzung nicht aktiv</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>Konfiguriert; wird nach einem Neustart wirksam</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>Konfiguriert; wird nach einem Neustart wirksam. Das Overlay dieses Starts wird nicht an den neuen Speicherort verschoben.</translation></message>
    <message><source>Configure…</source><translation>Konfigurieren…</translation></message>
    <message><source>Current UWF</source><translation>UWF in dieser Sitzung</translation></message>
    <message><source>Currently configured storage volume</source><translation>Derzeit konfiguriertes Speichervolume</translation></message>
    <message><source>Default behavior</source><translation>Standardverhalten</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>Deaktivieren Sie HORM und starten Sie den Computer neu, bevor Sie das gesamte Volume übernehmen.</translation></message>
    <message><source>Disabled · restart required</source><translation>Deaktiviert · Neustart erforderlich</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>Das Deaktivieren von HORM schaltet weder UWF noch den Modus für schreibgeschützte Medien aus und speichert keine Änderungen auf geschützten Datenträgern. Starten Sie den Computer neu, bevor Sie die verfügbaren Aktionen prüfen.</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>Aktivieren Sie UWF und starten Sie den Computer neu, bevor Sie HORM aktivieren.</translation></message>

    <message><source>Full volume commit</source><translation>Gesamtes Volume übernehmen</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>Vollständige Volume-Übernahme (Modus bis zum Neustart aktiv)</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>Der Modus zur vollständigen Volume-Übernahme ist bereits aktiviert. Starten Sie den Computer neu, bevor Sie ihn erneut verwenden.</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>Die Übernahme des gesamten Volumes wurde gestartet. Starten Sie den Computer neu, bevor Sie HORM aktivieren.</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>Das gesamte Volume kann nur übernommen werden, wenn der Modus für schreibgeschützte Medien aktiv ist.</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>Der Modus zur vollständigen Volume-Übernahme bleibt bis zum Neustart aktiv. Neue Änderungen werden direkt gespeichert. Einzelne Änderungen können weiterhin nicht übernommen werden.</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>Die Übernahme des gesamten Volumes ist auf diesem System nicht verfügbar.</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM ist aktiv. UWF-Einstellungen und die Übernahme einzelner Änderungen sind nicht verfügbar. Deaktivieren Sie HORM und starten Sie den Computer neu, bevor Sie Änderungen vornehmen.</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM ist deaktiviert. Starten Sie den Computer neu, bevor Sie UWF-Einstellungen ändern.</translation></message>
    <message><source>HORM is not available on this system.</source><translation>HORM ist auf diesem System nicht verfügbar.</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM erfordert eine aktuell aktive RAM-Konfiguration für schreibgeschützte Medien.</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM erfordert ein aktuell aktives RAM-Overlay.</translation></message>

    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>Im Disk-Modus kann die Systemimplementierung diese Option standardmäßig aktivieren; prüfen Sie den in den Einstellungen angezeigten aktuellen Zustand.</translation></message>
    <message><source>Inactive this session</source><translation>In dieser Sitzung nicht aktiv</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>Auch spätere Änderungen werden bis zum Neustart gespeichert</translation></message>

    <message><source>Maintenance and recovery</source><translation>Wartung und Wiederherstellung</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>Vergewissern Sie sich vor dem Fortfahren, dass Sie diesen Zustand beibehalten möchten.</translation></message>

    <message><source>Next startup: %1</source><translation>Nächster Start: %1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>Kein geeignetes Speichervolume verfügbar. Aktualisieren Sie die Volumeliste.</translation></message>
    <message><source>Not enabled</source><translation>Nicht aktiviert</translation></message>
    <message><source>Not selected</source><translation>Nicht ausgewählt</translation></message>
    <message><source>Not supported</source><translation>Nicht unterstützt</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>Nach dem Start des Vorgangs werden neue Änderungen bis zum nächsten Neustart des Computers direkt auf den Datenträger geschrieben.</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>Es kann jeweils nur ein Speichervolume verwendet werden. Die Auswahl eines neuen Volumes ersetzt den bisherigen Speicherort und stellt das Overlay für den nächsten Start auf Disk um.</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>Overlay-Kapazität (aktuelle Einstellung beibehalten)</translation></message>
    <message><source>Overlay configuration</source><translation>Overlay-Konfiguration</translation></message>

    <message><source>Processing…</source><translation>Wird verarbeitet…</translation></message>
    <message><source>Read state again</source><translation>Zustand erneut lesen</translation></message>
    <message><source>Read the current UWF state first.</source><translation>Lesen Sie zuerst den aktuellen UWF-Zustand.</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>Der Modus für schreibgeschützte Medien unterstützt weder die Übernahme einzelner Änderungen noch das automatische Speichern vorgemerkter Dateien.</translation></message>
    <message><source>Reading state…</source><translation>Zustand wird gelesen…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>Starten Sie den Computer neu und lesen Sie den aktuellen Zustand, bevor Sie HORM erneut aktivieren.</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>Bei jedem Start denselben Desktop und dieselben laufenden Programme wiederherstellen.</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>Prüfen Sie die verfügbaren Übernahmevorgänge und ihre Voraussetzungen.</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>Änderungen auf allen geschützten Volumes speichern</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>Änderungen auf allen geschützten Volumes speichern. Prüfen Sie die Auswirkungen, bevor Sie fortfahren.</translation></message>


    <message><source>State unknown</source><translation>Zustand unbekannt</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>Erfolgreich gespeicherte Änderungen lassen sich durch einen Neustart nicht mehr rückgängig machen.</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>Erfolgreich gespeicherte Änderungen bleiben erhalten</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>Der HORM-Zustand ist unbekannt. Lesen Sie den Zustand erneut, bevor Sie fortfahren.</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>Der HORM-Zustand ist unbekannt. Aktualisieren Sie den Zustand, bevor Sie fortfahren.</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>Der Übernahme- oder HORM-Zustand ist unbekannt. Aktualisieren Sie den Zustand, bevor Sie fortfahren.</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>Der aktuelle UWF-Zustand konnte nicht gelesen werden. Aktualisieren Sie den Zustand, bevor Sie fortfahren.</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>Das aktuelle Overlay verwendet den RAM-Modus für schreibgeschützte Medien</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>Der aktuelle Schutzmodus konnte nicht gelesen werden. Aktualisieren Sie den Zustand.</translation></message>
    <message><source>The result could not be confirmed</source><translation>Das Ergebnis konnte nicht bestätigt werden</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>Dieser Ausschluss ist konfiguriert, umgeht den Schutz in der aktuellen Sitzung jedoch nicht.</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>Dadurch können sich auch die Durchleitung über freien Speicherplatz und das persistente Overlay ändern. Prüfen Sie die erweiterten Einstellungen vor dem Neustart.</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>Der UWF-Schutz ist in der aktuellen Sitzung aktiv</translation></message>
    <message><source>%1 (will be replaced)</source><translation>%1 (wird ersetzt)</translation></message>
    <message><source>Add full volume commit to the draft?</source><translation>Die Übernahme des gesamten Volumes zum Entwurf hinzufügen?</translation></message>
    <message><source>Add to draft</source><translation>Zum Entwurf hinzufügen</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>Gilt für alle geschützten Volumes; startet erst beim Anwenden dieses Entwurfs.</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>Beim Anwenden dieses Entwurfs startet die Übernahme des gesamten Volumes. Gespeicherte Änderungen lassen sich durch einen Neustart nicht rückgängig machen. Auch neue Änderungen werden bis zum Neustart gespeichert.</translation></message>
    <message><source>Configure storage location</source><translation>Speicherort konfigurieren</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>Der Speicherort kann auf diesem System nicht konfiguriert werden.</translation></message>
    <message><source>Current: %1</source><translation>Aktuell: %1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>Deaktivieren Sie HORM und starten Sie den Computer neu, bevor Sie den Speicherort konfigurieren.</translation></message>
    <message><source>Disable saved-state startup?</source><translation>Start aus gespeichertem Zustand deaktivieren?</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>Entwurf: %1 · Anwenden erforderlich</translation></message>
    <message><source>Enable saved-state startup?</source><translation>Start aus gespeichertem Zustand aktivieren?</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>Gesamtes Volume übernehmen (vorgemerkt)</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>Die Übernahme des gesamten Volumes ist vorgemerkt. Sie startet erst nach Ihrer Bestätigung unter „Prüfen und anwenden“.</translation></message>
    <message><source>Hibernate once / resume many</source><translation>Einmal Ruhezustand / mehrfach wiederherstellen</translation></message>
    <message><source>Not started</source><translation>Nicht gestartet</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>Die Übernahme des gesamten Volumes aus den ausstehenden Änderungen entfernen.</translation></message>
    <message><source>Save draft</source><translation>Entwurf speichern</translation></message>

    <message><source>Will start when applied</source><translation>Startet beim Anwenden</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>Ein eingebundener fester Datenträger hat die Schutzprüfung nicht bestanden. Prüfen Sie die aufgeführten Datenträger, bevor Sie HORM aktivieren.</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>Prüft feste Datenträger, die weiterhin eingebunden sind und für UWF infrage kommen. Nicht eingebundene Partitionen können übersprungen werden.</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>Die Voraussetzungen für HORM konnten nicht überprüft werden. Lesen Sie den Zustand erneut, bevor Sie fortfahren.</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>Schutzprüfung eingebundener fester Datenträger</translation></message>
    <message><source>Not using a Disk overlay</source><translation>Es wird kein Disk-Overlay verwendet</translation></message>
    <message><source>Selected storage volume</source><translation>Ausgewähltes Speichervolume</translation></message>
    <message><source>System drive default (%1)</source><translation>Systemlaufwerk als Standard (%1)</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>Der Schutzzustand hat sich während dieses Systemstarts geändert. Starten Sie den Computer neu, bevor Sie HORM aktivieren.</translation></message>
    <message><source>Unable to read</source><translation>Lesen nicht möglich</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>Wählen Sie den Datenträger zum Speichern vorübergehender Änderungen. Wird beim nächsten Start wirksam.</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>Deaktivieren Sie UWF und starten Sie den Computer neu, bevor Sie den Speicherort konfigurieren.</translation></message>
    <message><source>Met</source><translation>Erfüllt</translation></message>
    <message><source>Not met</source><translation>Nicht erfüllt</translation></message>
    <message><source>Partition</source><translation>Partition</translation></message>


    <message><source>Protect all</source><translation>Alle schützen</translation></message>
    <message><source>Hibernation is not supported</source><translation>Ruhezustand wird nicht unterstützt</translation></message>
    <message><source>Hibernation is not enabled</source><translation>Ruhezustand ist nicht aktiviert</translation></message>
    <message><source>Fast startup only</source><translation>Nur Schnellstart</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>Das gleichzeitige Ändern des Schutzes aller Volumes ist auf diesem System nicht verfügbar.</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>Deaktivieren Sie HORM und starten Sie den Computer neu, bevor Sie den Volume-Schutz ändern.</translation></message>
    <message><source>Setup and operation</source><translation>Einrichtung und Betrieb</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. Konfiguration vorbereiten</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>Konfigurieren Sie bei derzeit deaktiviertem UWF ein RAM-Overlay, den Modus für schreibgeschützte Medien, den Volume-Schutz und die UWF-Aktivierung für den nächsten Start. Wenden Sie den Entwurf an. Bereiten Sie den S4-Ruhezustand mit einer vorhandenen vollständigen Ruhezustandsdatei vor.</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>Starten Sie neu und prüfen Sie im aktuellen Zustand, ob UWF aktiviert ist, ein RAM-Overlay und der Modus für schreibgeschützte Medien aktiv sind und alle eingebundenen, für UWF geeigneten festen Volumes geschützt sind. Prüfen Sie außerdem, ob der Ruhezustand des Systems verfügbar ist.</translation></message>
    <message><source>2. Restart and verify</source><translation>2. Neu starten und prüfen</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. HORM aktivieren und Ruhezustand starten</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>Fügen Sie „HORM aktivieren“ zum Entwurf hinzu und wählen Sie „Prüfen und anwenden“. Beim Anwenden des Entwurfs wird HORM aktiviert. Dies entspricht:</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>Bereiten Sie nach erfolgreicher Aktivierung den Desktop und die Anwendungen vor. Versetzen Sie den Computer anschließend einmal in den Ruhezustand, um den Ausgangszustand zu speichern:</translation></message>
    <message><source>HORM usage</source><translation>HORM verwenden</translation></message>
    <message><source>Volume protection</source><translation>Volume-Schutz</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>Konfigurieren Sie den Schutz aller Volumes für die nächste Sitzung.</translation></message>
    <message><source>Unprotect all</source><translation>Gesamten Schutz aufheben</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>Nächster Start: alle Volumes geschützt</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>Nächster Start: kein Volume geschützt</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>Nächster Start: individuelle Volume-Einstellungen</translation></message>
    <message><source>System hibernation is available</source><translation>Ruhezustand des Systems ist verfügbar</translation></message>
    <message><source>Usage guide</source><translation>Bedienungsanleitung</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>Schutz aller Volumes wartet auf Anwendung</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>Der Volume-Schutz wird nach dem Neustart wirksam</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>Das Aufheben des Schutzes aller Volumes und das Aktivieren von HORM können nicht zusammen angewendet werden.</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>Das gleichzeitige Ändern des Schutzes aller Volumes wird nicht unterstützt.</translation></message>
    <message><source>Applied</source><translation>Angewendet</translation></message>
    <message><source>Not applied</source><translation>Nicht angewendet</translation></message>
    <message><source>Could not confirm</source><translation>Bestätigung nicht möglich</translation></message>



    <message><source>Storage volume:</source><translation>Speichervolume:</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>Ausnahmen umgehen den Schutz während dieser Startsitzung nicht. Einzelne Übernahmen und die automatische Übernahme vorgemerkter Dateien sind nicht verfügbar. Die vollständige Volumeübernahme betrifft alle geschützten Volumes. Lesen Sie die Beschreibung, bevor Sie fortfahren.</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>Wiederholt aus demselben Ruhezustand fortsetzen</translation></message>
    <message><source>Remove full volume commit draft</source><translation>Entwurf zum Übernehmen des gesamten Volumes entfernen</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>Schreibgeschützter Medienmodus ist aktiv</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>Eine konfigurierte Speichermarkierung konnte nicht gelesen werden.</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>Die Dateivormerkung steht mit einem Dateiausschluss in Konflikt:
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>Auf diesem Computer ist kein vollständiger Ruhezustand verfügbar</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>Mehrere Speicher-Volumes haben eine Markierung ungleich null.</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>Der Datenträgertyp des Overlays konnte nicht bestätigt werden.</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>Der konfigurierte Speicherort konnte nicht gelesen werden.</translation></message>
    <message><source>The system drive could not be identified.</source><translation>Das Systemlaufwerk konnte nicht ermittelt werden.</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>Ein direkter Neustart überspringt die Übernahme vorgemerkter Dateien.</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>Direktes Herunterfahren überspringt die Übernahme vorgemerkter Dateien.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>Vorgemerkte Dateien werden nicht übernommen. Nicht übernommene Änderungen können nach dem Neustart verloren gehen.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>Vorgemerkte Dateien werden nicht übernommen. Nicht übernommene Änderungen können nach dem Herunterfahren verloren gehen.</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>Übernimmt vorgemerkte Dateien vor dem normalen Herunterfahren oder Neustart.</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>Die Übernahme des gesamten Volumes und die Aktivierung von HORM können nicht zusammen angewendet werden.</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>Die Konfiguration des Speicherorts und die Aktivierung von HORM können nicht zusammen angewendet werden.</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>Die Konfiguration des Speicherorts und die Übernahme des gesamten Volumes erfordern unterschiedliche aktuelle UWF-Modi.</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>Wählen Sie vor dem Anwenden ein Speichervolume aus.</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>Die Konfiguration eines Speicherorts auf einem Datenträger steht im Konflikt mit dem angeforderten RAM-Overlay.</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>Wenden Sie die Filteränderung an und starten Sie neu, bevor Sie das gesamte Volume übernehmen.</translation></message>
    <message><source>Canceled</source><translation>Abgebrochen</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>Das geprüfte Speichervolume ist nicht mehr verfügbar. Kehren Sie zur Bearbeitung zurück und wählen Sie ein anderes Volume aus.</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>Dieses Volume kann nicht bei UWF registriert werden, da sein Speicherbus nicht unterstützt wird.</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>Die Unterstützung des physischen Volumes konnte nicht bestätigt werden. Aktualisieren Sie den Status, bevor Sie den Schutz aktivieren.</translation></message>
    <message><source>Paused</source><translation>Pausiert</translation></message>
    <message><source>Partially paused</source><translation>Teilweise pausiert</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>Die Daten des persistenten Overlays bleiben beim nächsten Start erhalten. Automatische Commits sind pausiert.</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>Pausierte Einträge: %1 · Pausierte Dateien: %2</translation></message>
    <message><source>Read-only ROM</source><translation>Schreibgeschütztes ROM</translation></message>
    <message><source>%1 changes · %2 commands</source><translation>%1 Änderungen · %2 Befehle</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>Dateisystem %1: Dateiausschlüsse und dateiweise Übernahmen werden nicht unterstützt.</translation></message>
    <message><source>Applying changes</source><translation>Änderungen werden angewendet</translation></message>
    <message><source>Automatic commits paused</source><translation>Automatische Übernahmen pausiert</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>Nach einer Änderung des HORM-Modus sind automatische Dateiübernahmen bis zum Neustart pausiert.</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>Nach FullCommit sind automatische Dateiübernahmen nicht verfügbar.</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>Bei aktivem HORM sind automatische Dateiübernahmen nicht verfügbar.</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>Automatische Übernahmen vorgemerkter Dateien sind für diesen Neustart pausiert.</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>Automatische Übernahmen vorgemerkter Dateien sind für dieses Herunterfahren pausiert.</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>Die dauerhafte Speicherung der Konfiguration konnte nicht bestätigt werden. Aktualisieren Sie den Systemstatus, bevor Sie diese Einstellung ändern.</translation></message>
    <message><source>Elapsed: %1 s</source><translation>Verstrichen: %1 s</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>Nur geeignete vorgemerkte Dateien werden übernommen.</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>Im schreibgeschützten ROM-Modus können gewöhnliche UWF-Einstellungen auf dem geschützten Systemvolume nicht dauerhaft gespeichert werden. Deaktivieren Sie UWF und starten Sie neu, bevor Sie sie ändern.</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>Übersprungene vorgemerkte Dateien werden durch diesen Vorgang nicht übernommen.</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>Vorgemerkte Dateien liegen bereits auf ungeschütztem Speicher. Eine automatische Übernahme ist nicht erforderlich.</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>Schwellenwertänderungen erfordern eine Registrierungsübernahme, die im schreibgeschützten ROM-Modus nicht verfügbar ist.</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>Windows schließt den Vorgang ab. Dies kann etwas dauern.</translation></message>
    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM ist deaktiviert. Automatische Übernahmen vorgemerkter Dateien sind bis zum Neustart pausiert. Manuelle Vorgänge richten sich nach den aktuellen UWF-Funktionen.</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>Dieser Konfigurationsschnappschuss enthält Einstellungen, für die UWF in der aktuellen Sitzung deaktiviert sein muss. Deaktivieren Sie UWF und starten Sie neu, bevor Sie ihn ausführen.</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>Aktionen der aktuellen Sitzung (HORM, FullCommit und Zurücksetzen des persistenten Overlays; unter „Ausstehende Änderungen“ exportieren)</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>Zum gleichzeitigen Aufheben des Schutzes muss der vollständige Volumeumfang der nächsten Sitzung bestätigt sein.</translation></message>
</context>
</TS>
