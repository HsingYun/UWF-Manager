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
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Dieses Volume in der nächsten Sitzung schützen. Schreibzugriffe werden ins Overlay umgeleitet und beim Neustart verworfen.</translation></message>
    <message><source>Current session</source><translation>Aktuelle Sitzung</translation></message>
    <message><source>Next session</source><translation>Nächste Sitzung</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Volume-Erkennung: Laufwerksbuchstaben sind einfach, die Zuordnung geht bei einer Neuzuweisung aber verloren, etwa nach Festplattenänderungen. Die Volume-ID bleibt bei Buchstabenänderungen stabil.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>UWF unterstützt dieses Volume nicht: </translation></message>
    <message><source>Global settings</source><translation>Globale Einstellungen</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>UWF-Filterstatus in der aktuellen Sitzung (schreibgeschützt).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>UWF-Filter in der nächsten Sitzung aktivieren. Schreibzugriffe auf geschützte Volumes gehen ins Overlay und werden beim Neustart verworfen.</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Schreibt eine Warnmeldung in das Ereignisprotokoll, sobald die Overlay-Belegung diesen Wert erreicht. Wenn beide Ereignisse aktiviert sind, muss dieser Wert unter dem kritischen Schwellenwert liegen. Mit 0 wird nur das Warnereignis deaktiviert.</translation></message>
    <message><source>Critical threshold</source><translation>Kritische Schwelle</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Schreibt eine kritische Meldung in das Ereignisprotokoll, sobald die Overlay-Belegung diesen Wert erreicht. Wenn beide Ereignisse aktiviert sind, muss dieser Wert über dem Warnschwellenwert liegen. Mit 0 wird nur das kritische Ereignis deaktiviert.</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
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
    <message><source>File staging</source><translation>Dateivormerkung</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>Dateien / Ordner: Vor sicherem Herunterfahren oder Neustart automatisch dauerhaft übernehmen. Alle Unterordner werden einbezogen. Solange die Liste Einträge enthält, bleibt sie durch folgende Registrierungsausnahme auch über geschützte Sitzungen erhalten: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Dateien oder einen Ordner zur automatischen Übernahme hinzufügen.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Datei zur dauerhaften Übernahme auswählen. Vor sicherem Herunterfahren oder Neustart automatisch dauerhaft übernehmen.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Ordner zur dauerhaften Übernahme auswählen. Vor sicherem Herunterfahren oder Neustart automatisch dauerhaft übernehmen. Alle Unterordner werden einbezogen.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Datei zur dauerhaften Übernahme auswählen · Dateivormerkung</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Ordner zur dauerhaften Übernahme auswählen · Dateivormerkung · Alle Unterordner werden einbezogen.</translation></message>
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
    <message><source>File staging could not be saved</source><translation>Fehlgeschlagen: Dateivormerkungliste speichern</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Die Übernahmeliste wurde neu geladen, da die Aktualisierung scheiterte:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Dateivormerkung: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Vor sicherem Herunterfahren oder Neustart automatisch dauerhaft übernehmen.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Dateivormerkung: %1 · Dateien: %2 · Ordner: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Die Dateivormerkung benötigt diesen Registrierungsausschluss, damit ihre Liste nach Neustarts erhalten bleibt.</translation></message>
    <message><source>Built-in</source><translation>Integriert</translation></message>
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
    <message><source>Name</source><translation>Name</translation></message>
    <message><source>Data</source><translation>Daten</translation></message>
    <message><source>(Default)</source><translation>(Standard)</translation></message>
    <message><source>(value not set)</source><translation>(Wert nicht festgelegt)</translation></message>
    <message><source>(empty)</source><translation>(leer)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Unbekannt (Wert: Datei zu groß)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Overlay-Dateien anzeigen. &lt;b&gt;Nur eine Diagnoseaufnahme.&lt;/b&gt; Nicht zur Auswahl der zu übernehmenden Dateien verwenden.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Overlay-Dateilisten werden nur auf NTFS-Volumes unterstützt.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>Der UWF-Filter ist deaktiviert; kein Volume hat ein prüfbares Overlay.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Dieses Volume ist nicht UWF-geschützt und hat kein prüfbares Overlay.</translation></message>
    <message><source>Failed: %1</source><translation>Fehlgeschlagen: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Der WMI-Anbieter ist beim Auflisten abgestürzt. Dies ist eine bekannte Instabilität von UWF_Overlay.GetOverlayFiles bei großen Overlays oder hoher E/A-Last.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Nicht genügend Speicher oder Vorgang vom Anbieter nicht unterstützt. Erfordert NTFS und Speicherreserven. Mit kleinerem Overlay oder nach Schließen speicherintensiver Programme erneut versuchen.</translation></message>
    <message><source>Raw error: %1</source><translation>Fehlercode: %1</translation></message>
    <message><source>Export overlay file list</source><translation>In Datei exportieren… · Overlay · Dateien</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Textdateien (*.txt);;Alle Dateien (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Befehlsdateien (*.cmd);;Alle Dateien (*)</translation></message>
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
    <message><source>✓ %1 persistence: %2</source><translation>✓ Dauerhafte Speicherung %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Auswahl entfernen</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Ausgewählte Ausnahmen entfernen. Wird nach dem Anwenden wirksam.</translation></message>
    <message><source>Open containing folder</source><translation>Übergeordneten Ordner öffnen</translation></message>
    <message><source>Copy file path</source><translation>Dateipfad kopieren</translation></message>
    <message><source>Copy registry path</source><translation>Registrierungspfad kopieren</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Overlay-Änderungen dieses Ordners auf Datenträger übernehmen…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Overlay-Änderungen dieser Datei auf Datenträger übernehmen…</translation></message>
    <message><source>Copied to clipboard</source><translation>In die Zwischenablage kopiert</translation></message>
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
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Verwaltung des Unified Write Filter (UWF)</translation></message>
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
    <message><source>Switch display language</source><translation>Anzeigesprache wechseln</translation></message>
    <message><source>Language</source><translation>Sprache</translation></message>
    <message><source>Theme</source><translation>Design</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Helles/dunkles Design wechseln</translation></message>
    <message><source>System information</source><translation>Systeminformationen</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Diagnosesnapshot</translation></message>
    <message><source>Read only</source><translation>Schreibgeschützt</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Ziel nicht gefunden; im Overlay gibt es nichts zu übernehmen.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Das Ziel existiert nur im Overlay, nicht auf Datenträger bzw. in der dauerhaften Registrierung. Es wurde unter UWF erstellt, nie übernommen und verschwindet beim Neustart. Eine Löschübernahme ist weder nötig noch möglich.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Das System hat einen Parameter abgelehnt (ungültiger Pfad oder ungültiges Argument).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Vorgang fehlgeschlagen. Ein anderes Programm könnte das Ziel verwenden, etwa Explorer oder eine geöffnete Datei. Schließen Sie zugreifende Programme und versuchen Sie es erneut.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Vorgang fehlgeschlagen; Details stehen im Protokoll.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Vorgang abgelehnt (Code %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Unbekannt.</translation></message>
    <message><source>Commit canceled</source><translation>Übernahme abgebrochen</translation></message>

    <message><source>Succeeded</source><translation>Erfolgreich</translation></message>
    <message><source>Skipped</source><translation>Übersprungen</translation></message>
    <message><source>Failed</source><translation>Fehlgeschlagen</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Erfolgreich: %1; Übersprungen: %2; Fehlgeschlagen: %3.</translation></message>

    <message><source>Existed before</source><translation>Vorher vorhanden</translation></message>
    <message><source>Exists after</source><translation>Danach vorhanden</translation></message>
    <message><source>Error code</source><translation>Fehlercode</translation></message>
    <message><source>Reason</source><translation>Grund</translation></message>
    <message><source>Previous page</source><translation>Vorherige Seite</translation></message>
    <message><source>First page</source><translation>Erste Seite</translation></message>
    <message><source>Last page</source><translation>Letzte Seite</translation></message>
    <message><source>Next page</source><translation>Nächste Seite</translation></message>
    <message><source>Copy selected rows</source><translation>Ausgewählte Zeilen kopieren</translation></message>
    <message><source>Copy all</source><translation>Alles kopieren</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Diese Registrierungsausnahme ist erforderlich, solange die Übernahmeliste Einträge enthält. Dateivormerkung: Leeren → Registrierungsausnahme entfernen.</translation></message>
    <message><source>Copy current page</source><translation>Aktuelle Seite kopieren</translation></message>
    <message><source>No pending changes</source><translation>Keine ausstehenden Änderungen</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Registrierungsausnahmen · Globale Einstellungen.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: UWF-Schutz / Dateiausnahmen / Dateivormerkung.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Nicht unterstützt</translation></message>
    <message><source>Review and apply changes</source><translation>Änderungen prüfen und anwenden</translation></message>
    <message><source>Confirm apply</source><translation>Anwenden bestätigen</translation></message>
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
    <message><source>Calculating files for automatic commit…</source><translation>Dateien für automatische Übernahme werden ermittelt…</translation></message>
    <message><source>Committing staged files</source><translation>Dateien werden automatisch übernommen</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 von %2 Dateien verarbeitet.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Keine zusätzlichen Fehlerdetails verfügbar.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Dateivormerkung unerwartet beendet:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Dateivormerkung nicht abgeschlossen: Unbekannter Fehler.</translation></message>
    <message><source>File staging will be skipped</source><translation>Dateivormerkung wird übersprungen</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Direkt neu starten: Dateivormerkung wird übersprungen.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Direkt herunterfahren: Dateivormerkung wird übersprungen.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Dateivormerkung wegen direktem Ein-/Ausschaltvorgang übersprungen.</translation></message>
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
    <message><source>File staging preparation</source><translation>Dateivormerkung: Vorgangsdetails</translation></message>
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

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Der Pfad hat keinen Laufwerksbuchstaben. Zielvolume ermitteln: Fehlgeschlagen.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Für Volume %1 wurde kein Datensatz der aktuellen Sitzung gefunden.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Dieser Pfad ist in der Dateiausnahmeliste.
Ausnahme: %1</translation></message>

    <message><source>Continue</source><translation>Weiter</translation></message>
    <message><source>Delete and commit</source><translation>Löschen und übernehmen</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Dieser Schlüssel ist in der Registrierungsausnahmeliste.
Ausnahme: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Kein Registrierungsfilter-Datensatz der aktuellen Sitzung gefunden.</translation></message>

    <message><source>Export commands to file</source><translation>Befehle in Datei exportieren</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>In Datei exportieren… · Befehl: %1
%2</translation></message>
    <message><source>Import</source><translation>Importieren</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Befehle eingeben oder einfügen, einen pro Zeile. Textdateien, Protokolle oder Skripte (.txt, .bat, .ps1) laden; Zeilen mit diesem Befehl werden angehängt. Der Import merkt Änderungen nur vor. Erst „Prüfen und anwenden“ in der Symbolleiste schreibt sie ins System.</translation></message>
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
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Textdateien (*.txt *.cmd);;Alle Dateien (*)</translation></message>
    <message><source>Status</source><translation>Status</translation></message>
    <message><source>Import failed</source><translation>Import fehlgeschlagen</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Interner Fehler: Keine Anwendungsroutine registriert.</translation></message>
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
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Ein-/Ausblenden.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Aktiviert</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Deaktiviert</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Unbekannt</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Belegt %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Erweiterter Modus</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Dateivormerkung mit Windows-Herunterfahren und -Neustart koordinieren.</translation></message>
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
    <message><source>File staging completed</source><translation>Dateivormerkung: Erfolgreich</translation></message>
    <message><source>File staging failed</source><translation>Dateivormerkung: Fehlgeschlagen</translation></message>
    <message><source>All staged targets were processed.</source><translation>Alle vorgemerkten Ziele wurden verarbeitet.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Alle vorgemerkten Ziele verarbeitet; mindestens ein Vorgang ist fehlgeschlagen.</translation></message>
    <message><source>Show main window</source><translation>Hauptfenster anzeigen</translation></message>
    <message><source>Restore default position</source><translation>Standardposition wiederherstellen</translation></message>
    <message><source>Show overlay hub</source><translation>Overlay-HUB anzeigen</translation></message>
    <message><source>Hide overlay hub</source><translation>Overlay-HUB ausblenden</translation></message>
    <message><source>Exit application</source><translation>Anwendung beenden</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Fehlgeschlagen: Mit dem System verbinden</translation></message>
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
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Der Pfad existiert nicht; es gibt nichts zu löschen.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Unbekannter Fehler.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Ergebnis nicht bestätigbar, da das erneute Lesen des Zielstatus scheiterte: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Der Anbieter hat die Löschung angenommen, aber die maßgebliche erneute Prüfung zeigt, dass das Ziel noch existiert.</translation></message>
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
    <message><source>About this snapshot</source><translation>Über diese Aufnahme</translation></message>
    <message><source>Account</source><translation>Konto</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>UWF-Konfiguration aus einem Skript oder Text hinzufügen.</translation></message>
    <message><source>Add exclusion</source><translation>Ausschluss hinzufügen</translation></message>
    <message><source>Add files or folders</source><translation>Dateien oder Ordner hinzufügen</translation></message>
    <message><source>Add selected items</source><translation>Ausgewählte Einträge hinzufügen</translation></message>
    <message><source>Added</source><translation>Hinzugefügt</translation></message>
    <message><source>Administrator</source><translation>Administrator</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Alle Änderungen wurden dauerhaft auf dem Datenträger gespeichert. Ein Neustart ist nicht erforderlich.</translation></message>
    <message><source>All files (*)</source><translation>Alle Dateien (*)</translation></message>
    <message><source>All levels</source><translation>Alle Stufen</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Eine leere Diagnoseaufnahme bedeutet nicht, dass das Overlay keine Änderungen enthält.</translation></message>
    <message><source>Application path</source><translation>Anwendungspfad</translation></message>
    <message><source>Application result</source><translation>Ergebnis der Anwendung</translation></message>
    <message><source>Apply changes</source><translation>Änderungen anwenden</translation></message>
    <message><source>Architecture</source><translation>Architektur</translation></message>
    <message><source>Automatic</source><translation>Automatisch</translation></message>
    <message><source>Automatic file staging</source><translation>Automatische Dateivormerkung</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Automatisches Speichern unvollständig</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Geeignete vorgemerkte Dateien beim normalen Herunterfahren und Neustarten automatisch speichern.</translation></message>
    <message><source>Available</source><translation>Verfügbar</translation></message>
    <message><source>Back</source><translation>Zurück</translation></message>
    <message><source>Background service</source><translation>Hintergrunddienst</translation></message>
    <message><source>Binding</source><translation>Zuordnung</translation></message>
    <message><source>By drive letter</source><translation>Nach Laufwerkbuchstaben</translation></message>
    <message><source>By volume ID</source><translation>Nach Volume-ID</translation></message>
    <message><source>Cancel reading</source><translation>Einlesen abbrechen</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Bereits übernommene Änderungen bleiben erhalten.</translation></message>
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
    <message><source>Commit complete</source><translation>Übernahme abgeschlossen</translation></message>
    <message><source>Commit file changes?</source><translation>Dateiänderungen übernehmen?</translation></message>
    <message><source>Commit folder changes?</source><translation>Ordneränderungen übernehmen?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Änderungen des Registrierungsschlüssels übernehmen?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Änderungen des Registrierungswerts übernehmen?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Übernimmt vorgemerkte Dateien vor normalem Herunterfahren und Neustarten</translation></message>
    <message><source>Compatibility mode</source><translation>Kompatibilitätsmodus</translation></message>
    <message><source>Configuration applied</source><translation>Konfiguration angewendet</translation></message>
    <message><source>Configuration error</source><translation>Konfigurationsfehler</translation></message>
    <message><source>Configured</source><translation>Konfiguriert</translation></message>
    <message><source>Confirm before continuing</source><translation>Vor dem Fortfahren bestätigen</translation></message>
    <message><source>Connected and authenticated</source><translation>Verbunden und authentifiziert</translation></message>
    <message><source>Connecting…</source><translation>Verbindung wird hergestellt…</translation></message>
    <message><source>Contact author</source><translation>Autor kontaktieren</translation></message>
    <message><source>Continue editing</source><translation>Weiter bearbeiten</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Wenn Sie den Neustart fortsetzen, gehen Änderungen in nicht gespeicherten Dateien verloren.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Wenn Sie das Herunterfahren fortsetzen, gehen Änderungen in nicht gespeicherten Dateien verloren.</translation></message>
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
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>Der erweiterte Modus ist nicht verfügbar. Nutzen Sie Sicher herunterfahren oder Sicher neu starten, um vorgemerkte Dateien zu speichern.</translation></message>
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
    <message><source>File saving has started and cannot be canceled.</source><translation>Das Speichern hat begonnen und kann nicht abgebrochen werden.</translation></message>
    <message><source>File saving incomplete</source><translation>Dateien nicht vollständig gespeichert</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Die Dateivormerkung wird übersprungen. Nicht übernommene Änderungen an geschützten Pfaden gehen nach dem Neustart verloren.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Die Dateivormerkung wird übersprungen. Nicht übernommene Änderungen an geschützten Pfaden gehen nach dem Herunterfahren verloren.</translation></message>
    <message><source>File type</source><translation>Dateityp</translation></message>
    <message><source>Files and folders</source><translation>Dateien und Ordner</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Dateien und Ordner können gemeinsam ausgewählt werden.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Dateien in der Dateivormerkung werden nicht gespeichert.</translation></message>
    <message><source>Filter is disabled</source><translation>Filter ist deaktiviert</translation></message>
    <message><source>Fit columns to contents</source><translation>Spalten an Inhalt anpassen</translation></message>
    <message><source>Folder address</source><translation>Ordneradresse</translation></message>
    <message><source>Folder name</source><translation>Ordnername</translation></message>
    <message><source>Folders</source><translation>Ordner</translation></message>
    <message><source>Full configuration</source><translation>Vollständige Konfiguration</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Künftige Schreibzugriffe auf diesen Schlüssel werden nicht durch UWF geschützt.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Künftige Schreibzugriffe auf diesen Pfad werden nicht durch UWF geschützt.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Globaler Registrierungsausschluss · einschließlich Unterschlüssel</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Wechseln Sie eine Ordnerebene nach oben oder wählen Sie einen anderen Ort.</translation></message>
    <message><source>Home</source><translation>Benutzerordner</translation></message>
    <message><source>Import configuration</source><translation>Konfiguration importieren</translation></message>
    <message><source>Import creates pending changes. Use Review and apply to write them to the system.</source><translation>Der Import erzeugt ausstehende Änderungen. Schreiben Sie diese mit Prüfen und anwenden in das System.</translation></message>
    <message><source>Import results</source><translation>Importergebnisse</translation></message>
    <message><source>Imported</source><translation>Importiert</translation></message>
    <message><source>Imported commands still need to be applied. Select a row to view the complete result.</source><translation>Importierte Befehle müssen noch angewendet werden. Wählen Sie eine Zeile, um das vollständige Ergebnis zu sehen.</translation></message>
    <message><source>Imported: %1</source><translation>Importiert: %1</translation></message>
    <message><source>Information</source><translation>Information</translation></message>
    <message><source>Item details</source><translation>Eintragsdetails</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Das Speichern großer Vormerklisten kann länger dauern.</translation></message>
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
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Wenn UWF das Laufwerk der Anwendung schützt, stellen Sie zunächst sicher, dass die Anwendungsdateien nach einem Neustart erhalten bleiben. Das dauerhafte Speichern vieler vorgemerkter Dateien kann das Herunterfahren oder den Neustart verlängern.</translation></message>
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
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Nur vorgemerkte Dateien werden automatisch gespeichert. Andere nicht übernommene Änderungen an geschützten Pfaden gehen nach dem Neustart verloren.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Nur vorgemerkte Dateien werden automatisch gespeichert. Andere nicht übernommene Änderungen an geschützten Pfaden gehen nach dem Herunterfahren verloren.</translation></message>
    <message><source>Open</source><translation>Öffnen</translation></message>
    <message><source>Open folder</source><translation>Ordner öffnen</translation></message>
    <message><source>Operating system</source><translation>Betriebssystem</translation></message>
    <message><source>Operation and security</source><translation>Betrieb und Sicherheit</translation></message>
    <message><source>Original command</source><translation>Ursprünglicher Befehl</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Kritischer Overlay-Schwellenwert erreicht. Speichern Sie Änderungen und starten Sie sicher neu.</translation></message>
    <message><source>Overlay maximum size</source><translation>Maximale Overlay-Größe</translation></message>
    <message><source>Overlay type</source><translation>Overlay-Typ</translation></message>
    <message><source>Overlay usage</source><translation>Overlay-Nutzung</translation></message>
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
    <message><source>Ready to save</source><translation>Bereit zum Speichern</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Nach Abschluss des Einlesens erscheinen hier Datensätze des aktuellen Durchlaufs.</translation></message>
    <message><source>Recursive staging</source><translation>Rekursive Vormerkung</translation></message>
    <message><source>Refresh folder</source><translation>Ordner aktualisieren</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Registrierungsschlüssel oder Persistenzeinstellung</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Verbleibende Aufgaben können nach Abschluss des aktuellen Ziels abgebrochen werden.</translation></message>
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
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Nach dem Speichern wird der Neustart fortgesetzt. Lassen Sie die Anwendung geöffnet.</translation></message>
    <message><source>Restore</source><translation>Wiederherstellen</translation></message>
    <message><source>Retry</source><translation>Erneut versuchen</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Prüfen Sie die Fehlerdetails, bevor Sie das Herunterfahren fortsetzen.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Prüfen Sie die Fehlerdetails vor dem Neustart.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Prüfen Sie die Vorgangsdetails. Lesen Sie den aktuellen Systemzustand ein, bevor Sie möglicherweise bereits geschriebene Änderungen erneut versuchen.</translation></message>
    <message><source>Review the result for each target.</source><translation>Prüfen Sie das Ergebnis für jedes Ziel.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Prüfen Sie diese Änderungen, bevor Sie sie in das System schreiben.</translation></message>
    <message><source>Running</source><translation>Wird ausgeführt</translation></message>
    <message><source>Save</source><translation>Speichern</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Vorgemerkte Dateien auf den Datenträger speichern, dann neu starten.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Vorgemerkte Dateien auf den Datenträger speichern, dann herunterfahren.</translation></message>
    <message><source>Saving staged files</source><translation>Vorgemerkte Dateien werden gespeichert</translation></message>
    <message><source>Saving to disk</source><translation>Daten werden auf den Datenträger geschrieben</translation></message>
    <message><source>Saving…</source><translation>Speichern…</translation></message>
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
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>Nach dem Speichern wird das Herunterfahren fortgesetzt. Lassen Sie die Anwendung geöffnet.</translation></message>
    <message><source>Size</source><translation>Größe</translation></message>
    <message><source>Some changes could not be applied</source><translation>Einige Änderungen konnten nicht angewendet werden</translation></message>
    <message><source>Some files could not be saved</source><translation>Einige Dateien konnten nicht gespeichert werden</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Einige Hardwareinformationen konnten nicht gelesen werden. Andere Diagnosedaten können weiterhin angezeigt und kopiert werden.</translation></message>
    <message><source>Source code</source><translation>Quellcode</translation></message>
    <message><source>Staged file</source><translation>Vorgemerkte Datei</translation></message>
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
    <message><source>Target</source><translation>Ziel</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistenz von Terminaldienste-Lizenzen</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Der UWF-Anbieter konnte die Auflistung nicht abschließen. Dies kann bei großem Overlay oder unzureichenden Systemressourcen auftreten.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>Die Konfiguration wurde in das System geschrieben. Die meisten Änderungen werden nach einem Neustart wirksam.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>Die Konfiguration entspricht dem Ziel der nächsten Sitzung. Unter Vollständige Konfiguration können Sie Befehle anzeigen oder exportieren.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Die aktuellen Protokolldatensätze werden unwiederbringlich gelöscht.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Der Zielordner existiert nicht.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Der Dienststatus des erweiterten Modus konnte nicht gelesen werden. Prüfen Sie die Dienstberechtigungen und versuchen Sie es erneut.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>Die Liste kann Dateien auslassen oder bereits übernommene Dateien enthalten. Verwenden Sie sie nicht als Übernahmeliste.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Der ausgewählte Eintrag existiert nicht mehr: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Der Dienst ist installiert, aber angehalten. Starten Sie ihn, um das automatische Speichern wiederherzustellen.</translation></message>
    <message><source>This PC</source><translation>Dieser PC</translation></message>
    <message><source>This folder is empty</source><translation>Dieser Ordner ist leer</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Dieser Bericht kann Kontonamen, Gerätekennungen und Dateipfade enthalten. Prüfen Sie ihn vor der Weitergabe.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Speichern Sie dieses UTF-8-CMD-Skript als .cmd-Datei und führen Sie es in der Eingabeaufforderung aus. Beim Export wird dasselbe Skript verwendet.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Versuchen Sie ein anderes Stichwort oder eine andere Protokollstufe.</translation></message>
    <message><source>Try another search term.</source><translation>Versuchen Sie einen anderen Suchbegriff.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Typ und maximale Größe können bei aktiviertem Filter nicht geändert werden. Deaktivieren Sie den Filter und starten Sie zuerst neu.</translation></message>
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
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Nicht erfolgreiche Einträge wurden nicht gespeichert. Wählen Sie eine Zeile, um den vollständigen Pfad und den Grund zu sehen.</translation></message>
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
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Ohne erweiterten Modus verwenden Sie Sicher herunterfahren oder Sicher neu starten in dieser Anwendung, um vorgemerkte Dateien zu speichern.</translation></message>
    <message><source>Write protection</source><translation>Schreibschutz</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Diese Konfigurationsänderungen in das System schreiben? Die meisten Änderungen werden nach einem Neustart wirksam.</translation></message>
    <message><source>staged files</source><translation>vorgemerkte Dateien</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr-Befehle</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Ausgewählte Pfade zum Entfernen vormerken. Wird nach dem Anwenden wirksam.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Nach dem Anwenden aus der automatischen Dateibereitstellung entfernen.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Nach dem Anwenden zur automatischen Dateibereitstellung hinzufügen.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Beim Anwenden dieses Plans wird die gespeicherte Bereitstellungsliste geleert und ihr Registrierungsausschluss entfernt, wenn er nicht mehr benötigt wird.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Änderungen der Dateibereitstellung müssen in UWF Manager angewendet werden und können nicht als uwfmgr-Befehle exportiert werden.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Dateibereitstellung wird nach dem Anwenden gespeichert</translation></message>
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
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Nur erfolgreich durchsuchte Inhalte übernehmen. Nicht lesbare Zweige bleiben unverändert.</translation></message>
    <message><source>Committing changes</source><translation>Änderungen werden übernommen</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Abgeschlossene Löschungen und Übernahmen können nicht rückgängig gemacht werden.</translation></message>
    <message><source>Continue…</source><translation>Weiter…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Nur gefundene Ziele löschen und dauerhaft übernehmen. Nicht lesbare Zweige und ihre übergeordneten Ordner oder Schlüssel bleiben erhalten.</translation></message>
    <message><source>Deleting and committing</source><translation>Löschen und Übernehmen läuft</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Löschen und Übernehmen können nicht rückgängig gemacht werden. Nur der gewählte Bereich wird verarbeitet; ausstehende UWF-Einstellungen werden nicht angewendet.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Der Umfang wird ermittelt. Nicht lesbare Zweige werden protokolliert; andere Inhalte werden weiter durchsucht.</translation></message>
    <message><source>Export traversal records</source><translation>Suchprotokoll exportieren</translation></message>
    <message><source>No actionable content</source><translation>Keine verarbeitbaren Inhalte</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Keine verarbeitbaren Ziele gefunden. Es wurde nichts gelöscht oder übernommen.</translation></message>
    <message><source>No changes have been committed.</source><translation>Noch keine Änderungen übernommen</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Noch nichts gelöscht oder übernommen</translation></message>
    <message><source>Path</source><translation>Pfad</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Den gewählten Bereich dauerhaft löschen und die Löschungen auf dem geschützten Volume speichern.</translation></message>
    <message><source>Preparation complete</source><translation>Vorbereitung abgeschlossen</translation></message>
    <message><source>Preparing commit</source><translation>Übernahme wird vorbereitet</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Löschen und Übernehmen vorbereiten</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Nur erfolgreich durchsuchte Inhalte verarbeiten</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Analysepunkt-Verknüpfungen werden übersprungen, damit die Suche im gewählten Bereich bleibt.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Ein Neustart macht die Übernahme nicht rückgängig. Nur der gewählte Bereich wird verarbeitet; ausstehende UWF-Einstellungen werden nicht angewendet.</translation></message>
    <message><source>Retry failed items</source><translation>Fehlgeschlagene erneut versuchen</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Änderungen im gewählten Bereich auf dem geschützten Volume speichern.</translation></message>
    <message><source>Stopping the operation</source><translation>Vorgang wird angehalten</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Erfolgs- und Fehlerprotokolle bleiben erhalten</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Der Ordner ist nicht leer; unverarbeitete Inhalte bleiben erhalten.</translation></message>
    <message><source>The operation could not be completed</source><translation>Der Vorgang konnte nicht abgeschlossen werden</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>Der Registrierungsschlüssel enthält neue Werte; unverarbeitete Inhalte bleiben erhalten.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>Der Registrierungsschlüssel hat noch Unterschlüssel; unverarbeitete Inhalte bleiben erhalten.</translation></message>
    <message><source>The target no longer exists.</source><translation>Das Ziel ist nicht mehr vorhanden.</translation></message>
    <message><source>Traversal complete</source><translation>Suche abgeschlossen</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Suche abgeschlossen, Inhalte teilweise nicht lesbar</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Das Suchprotokoll bleibt erhalten und kann exportiert werden.</translation></message>
    <message><source>Traverse again</source><translation>Erneut durchsuchen</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Erneut durchsuchen oder dieses Fenster schließen.</translation></message>
    <message><source>Traversing the folder</source><translation>Ordner wird durchsucht</translation></message>
    <message><source>Traversing the registry</source><translation>Registrierung wird durchsucht</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Der Vorgang kann nicht vorbereitet werden</translation></message>
    <message><source>View results</source><translation>Ergebnisse anzeigen</translation></message>

    <message><source>All changes committed</source><translation>Alle Änderungen übernommen</translation></message>
    <message><source>Some changes committed</source><translation>Änderungen teilweise übernommen</translation></message>
    <message><source>No changes could be committed</source><translation>Keine Änderungen konnten übernommen werden</translation></message>
    <message><source>No changes to commit</source><translation>Keine Änderungen zu übernehmen</translation></message>
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
    <message><source>Export configuration</source><translation>Konfiguration exportieren</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Die aktuellen Einstellungen für Design, Sprache und HUB-Sichtbarkeit in der UWF.ini neben dieser Anwendung speichern.</translation></message>
    <message><source>Configuration saved</source><translation>Konfiguration gespeichert</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Die Dateivormerkung wurde angehalten. Beheben Sie den Fehler, bevor Sie es erneut versuchen.</translation></message>
</context>
</TS>
