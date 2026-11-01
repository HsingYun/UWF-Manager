<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="nl" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Voorbeeld</translation></message>
    <message><source>enable</source><translation>inschakelen</translation></message>
    <message><source>disable</source><translation>uitschakelen</translation></message>
    <message><source>Enabled</source><translation>Ingeschakeld</translation></message>
    <message><source>Disabled</source><translation>Uitgeschakeld</translation></message>
    <message><source>Disabled</source><comment>service startup type</comment><translation>Uitgeschakeld</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Annuleren</translation></message>
    <message><source>Close</source><translation>Sluiten</translation></message>
    <message><source>Refresh</source><translation>Vernieuwen</translation></message>
    <message><source>Add</source><translation>Toevoegen</translation></message>
    <message><source>Remove</source><translation>Verwijderen</translation></message>
    <message><source>Yes</source><translation>Ja</translation></message>
    <message><source>No</source><translation>Nee</translation></message>
    <message><source>Excluded</source><translation>Uitgesloten</translation></message>
    <message><source>Not excluded</source><translation>Niet uitgesloten</translation></message>
    <message><source>File…</source><translation>Bestand…</translation></message>
    <message><source>Folder…</source><translation>Map…</translation></message>
    <message><source>volume ID</source><translation>volume-ID</translation></message>
    <message><source>drive letter</source><translation>stationsletter</translation></message>
    <message><source>Drive letter</source><translation>Stationsletter</translation></message>
    <message><source>Volume ID</source><translation>Volume-ID</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Beveiligingsstatus van dit volume in de huidige sessie (alleen-lezen).</translation></message>
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Dit volume in de volgende sessie beveiligen. Schrijfbewerkingen naar beveiligde paden worden naar de overlay omgeleid; behoud na een herstart hangt af van de overlay-instellingen.</translation></message>
    <message><source>Current session</source><translation>Huidige sessie</translation></message>
    <message><source>Next session</source><translation>Volgende sessie</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Volumeherkenning: een stationsletter is eenvoudiger, maar de koppeling vervalt bij een andere letter, bijvoorbeeld na schijfwijzigingen. Het volume-ID blijft gelijk.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Dit volume wordt niet ondersteund door UWF: </translation></message>
    <message><source>Global settings</source><translation>Algemene instellingen</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>UWF-filterstatus in de huidige sessie (alleen-lezen).</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>UWF in de volgende sessie inschakelen. Schrijfbewerkingen naar beveiligde paden worden naar de overlay omgeleid; behoud na een herstart hangt af van de overlay-instellingen.</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>Het overlaytype of de maximale capaciteit wijzigen:
1. Schakel UWF uit.
2. Start het apparaat opnieuw op.
3. Wijzig deze instelling.</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM gebruikt het systeemgeheugen. Disk gebruikt het systeemstation en ondersteunt grotere overlays. Een persistente overlay kan gegevens in een Disk-overlay na herstarts behouden.</translation></message>
    <message><source>Type</source><translation>Type</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>De capaciteit van een RAM-overlay wordt beperkt door het systeemgeheugen. Een Disk-overlay vereist minimaal 1024 MB en voldoende vrije ruimte op het systeemstation.</translation></message>
    <message><source>Maximum capacity</source><translation>Maximale capaciteit</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>Maximale capaciteit · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Waarschuwingsdrempel</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Schrijft een waarschuwing naar het gebeurtenislogboek wanneer het overlaygebruik deze waarde bereikt. Als beide gebeurtenissen zijn ingeschakeld, moet deze waarde lager zijn dan de kritieke drempel. Met 0 wordt alleen de waarschuwingsgebeurtenis uitgeschakeld.</translation></message>
    <message><source>Critical threshold</source><translation>Kritieke drempel</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Schrijft een kritieke melding naar het gebeurtenislogboek wanneer het overlaygebruik deze waarde bereikt. Als beide gebeurtenissen zijn ingeschakeld, moet deze waarde hoger zijn dan de waarschuwingsdrempel. Met 0 wordt alleen de kritieke gebeurtenis uitgeschakeld.</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Niet-ondersteund stationstype (alleen lokale vaste schijven).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Het volume overschrijdt de UWF-limiet van 16 TiB per beveiligd volume.</translation></message>

    <message><source>%1 free / %2</source><translation>%1 vrij / %2</translation></message>
    <message><source>Commit</source><translation>Vastleggen</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>Wijzigingen uit de overlay vastleggen op schijf of in het register. Dit kan niet ongedaan worden gemaakt.</translation></message>
    <message><source>Commit file changes…</source><translation>Bestandswijzigingen vastleggen…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Kies een bestand en leg de wijzigingen uit de overlay vast op schijf.</translation></message>
    <message><source>Commit folder changes…</source><translation>Mapwijzigingen vastleggen…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Kies een map en leg de overlaywijzigingen van alle bestanden daarin vast op schijf.</translation></message>
    <message><source>Delete and commit file…</source><translation>Bestand verwijderen en verwijdering vastleggen op schijf…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Kies een bestand om te verwijderen en leg de verwijdering vast op schijf.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Map en inhoud verwijderen en verwijderingen vastleggen op schijf…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>Kies een map om recursief met alle inhoud te verwijderen en leg de verwijderingen vast op schijf.</translation></message>
    <message><source>Commit registry changes…</source><translation>Registerwijzigingen vastleggen…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Voer een registersleutel en eventueel een waardenaam in. Zonder waardenaam worden alle waarden van de sleutel en subsleutels vastgelegd.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Registerverwijdering vastleggen…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>Voer een registersleutel en eventueel een waardenaam in. Laat de waardenaam leeg om de sleutel en alle onderliggende sleutels en waarden te verwijderen en daarna de verwijderingen vast te leggen.</translation></message>
    <message><source>File exclusions</source><translation>Bestandsuitsluitingen</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Bestanden en mappen op dit volume zonder UWF-beveiliging. Dubbelklik op een vermelding om het pad te kopiëren.</translation></message>
    <message><source>Registry exclusions</source><translation>Registeruitsluitingen</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>Registeruitsluitingen zijn globale instellingen en worden alleen weergegeven bij de Windows-partitie.</translation></message>
    <message><source>File staging</source><translation>Klaargezette bestanden</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>Bestanden of een map toevoegen aan automatisch vastleggen.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Selecteer één of meer bestanden om automatisch vast te leggen vóór veilig afsluiten of herstarten.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Selecteer een map waarvan de bestanden recursief worden vastgelegd vóór veilig afsluiten of herstarten.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Bestanden voor automatisch vastleggen selecteren</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Een map voor automatisch recursief vastleggen selecteren</translation></message>
    <message><source>Cannot stage this path</source><translation>Pad kan niet worden toegevoegd aan automatisch vastleggen</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Dit pad overlapt een UWF-bestandsuitsluiting. Verwijder de uitsluiting voordat u het pad klaarzet:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Dit pad overlapt een vermelding in de lijst met klaargezette bestanden. Verwijder het klaargezette pad voordat u de uitsluiting toevoegt:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Het pad overlapt een bestaande vermelding in de lijst met klaargezette bestanden</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Deze registeruitsluiting is vereist zolang de lijst met klaargezette bestanden opgeslagen vermeldingen bevat</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Het pad bestaat niet of heeft niet het verwachte type:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Het pad kan niet worden onderzocht:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Reparsepunten kunnen niet aan de lijst met klaargezette bestanden worden toegevoegd:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>Het pad heeft geen stationsletter en kan niet aan “Klaargezette bestanden” worden toegevoegd:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Het geselecteerde pad staat op volume %1. Voeg het toe via het tabblad ‘Klaargezette bestanden’ van dat volume.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>De hoofdmap van een volume kan niet worden klaargezet, omdat dan het hele volume recursief zou worden vastgelegd.</translation></message>
    <message><source>File staging could not be saved</source><translation>De lijst met klaargezette bestanden kon niet worden opgeslagen</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>De lijst met klaargezette bestanden is opnieuw geladen omdat bijwerken niet kon worden voltooid:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Klaargezette bestanden zijn niet beschikbaar: %1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>%1 klaargezette vermeldingen · %2 bestanden · %3 mappen</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Klaargezette bestanden vereist deze registeruitsluiting om de lijst na herstarts te behouden.</translation></message>
    <message><source>Built-in</source><translation>Ingebouwd</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Het UWF-filter is uitgeschakeld. Er zijn geen overlaywijzigingen verzameld om vast te leggen.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Het bestandssysteem %1 ondersteunt geen vastlegging per bestand.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Dit volume wordt niet ondersteund door UWF.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Het bestandssysteem %1 ondersteunt geen vastlegging per bestand. Alleen registervastlegging beschikbaar (registeruitsluitingen zijn algemeen).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Dit volume is momenteel niet UWF-beveiligd; er zijn geen bestandswijzigingen om vast te leggen.</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Bestand kiezen om vast te leggen op schijf</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Map kiezen om vast te leggen op schijf</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Bestand kiezen voor blijvende verwijdering</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Map kiezen voor blijvende verwijdering</translation></message>
    <message><source>Name</source><translation>Naam</translation></message>
    <message><source>Data</source><translation>Gegevens</translation></message>
    <message><source>(Default)</source><translation>(Standaard)</translation></message>
    <message><source>(value not set)</source><translation>(waarde niet ingesteld)</translation></message>
    <message><source>(empty)</source><translation>(leeg)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Voorbeeld niet beschikbaar (waarde is te groot)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Een diagnoseweergave openen van bestanden die momenteel voor dit volume in de overlay zijn opgeslagen.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Overlaybestanden kunnen alleen op NTFS-volumes worden weergegeven.</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>Het UWF-filter is uitgeschakeld; geen enkel volume heeft een overlay om te bekijken.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Dit volume is niet UWF-beveiligd en heeft geen overlay om te bekijken.</translation></message>
    <message><source>Failed: %1</source><translation>Mislukt: %1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>De vorige inventarisatie is mogelijk nog bezig. Een nieuwe leesbewerking kan nog niet worden gestart.</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>De UWF-service is niet beschikbaar tijdens het opsommen van overlaybestanden.</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>Onvoldoende geheugen of deze bewerking wordt niet door het systeem ondersteund. Deze functie vereist een NTFS-volume; sluit geheugenintensieve toepassingen en probeer het opnieuw.</translation></message>
    <message><source>Raw error: %1</source><translation>Foutcode: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Exporteren naar bestand… · Overlay · bestanden</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Tekstbestanden (*.txt);;Alle bestanden (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Opdrachtbestanden (*.cmd);;Alle bestanden (*)</translation></message>
    <message><source>Export failed</source><translation>Exporteren mislukt</translation></message>
    <message><source>Export finished</source><translation>Exporteren voltooid</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Mislukt: Bestand openen (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Mislukt: Bestand schrijven (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>%1 vermeldingen opgeslagen in:
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Een heel volume (%1) kan niet worden uitgesloten.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>Wissel-, swap- en slaapstandbestanden kunnen niet als uitsluitingen worden toegevoegd.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>De hele map kan niet worden uitgesloten: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>De hele map kan niet worden uitgesloten: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>De hele map kan niet worden uitgesloten: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Dit essentiële systeembestand kan niet worden uitgesloten: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Het gebruikersregisterbestand NTUSER.DAT kan niet worden uitgesloten.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>De registersleutel mag niet leeg zijn.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Een registersleutel mag niet beginnen met een backslash.</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>Registerpaden mogen geen null-tekens bevatten.</translation></message>
    <message><source>A registry key component is too long.</source><translation>Een onderdeel van de registersleutel is te lang.</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>Het registerpad overschrijdt de maximale nestdiepte van UWF.</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>Een bestand of map aan de uitsluitingslijst toevoegen.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Bestand kiezen om toe te voegen · Bestandsuitsluitingen.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Map kiezen om toe te voegen · Bestandsuitsluitingen. Inclusief alle submappen.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Voer het pad van de uit te sluiten registersleutel in.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Voer het pad van de uit te sluiten registersleutel in. / Persistentie.</translation></message>
    <message><source>Registry key…</source><translation>Register · Sleutel…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Geheime domeinsleutel (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Clienttoegangslicentie voor Terminal Services (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Domeingeheim (wachtwoord computeraccount) tussen UWF-sessies bewaren.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Clienttoegangslicenties voor Terminal Services tussen UWF-sessies bewaren.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistentie %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Selectie verwijderen</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Geselecteerde uitsluitingen verwijderen. Wordt actief na toepassen.</translation></message>
    <message><source>Open containing folder</source><translation>Bijbehorende map openen</translation></message>
    <message><source>Copy file path</source><translation>Bestandspad kopiëren</translation></message>
    <message><source>Copy registry path</source><translation>Registerpad kopiëren</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Overlaywijzigingen van deze map vastleggen op schijf…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Overlaywijzigingen van dit bestand vastleggen op schijf…</translation></message>
    <message><source>Copied to clipboard</source><translation>Naar klembord gekopieerd</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Bestand kiezen om toe te voegen · Bestandsuitsluitingen. Meerdere bestanden selecteren is mogelijk.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Map kiezen om toe te voegen · Bestandsuitsluitingen</translation></message>
    <message><source>Add registry exclusion</source><translation>Registeruitsluiting toevoegen</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Pad %1 staat niet op volume %2 en kan niet als uitsluiting worden toegevoegd.</translation></message>
    <message><source>Path is not on this volume</source><translation>Pad staat niet op dit volume</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Uitsluiting kan niet worden toegevoegd</translation></message>
    <message><source>File: %1</source><translation>Bestand… %1</translation></message>
    <message><source>Folder: %1</source><translation>Map… %1</translation></message>
    <message><source>Registry: %1</source><translation>Register: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Standaard uitgesloten; altijd aanpasbaar.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Huidige sessie: %1
Volgende sessie: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Openstaande wijzigingen: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Beheer van Unified Write Filter (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Huidige UWF-status en configuratie van de volgende sessie opnieuw lezen.</translation></message>
    <message><source>Review and apply</source><translation>Controleren en toepassen</translation></message>
    <message><source>Review all pending changes and apply them in one batch.</source><translation>Alle openstaande wijzigingen controleren en in één keer toepassen.</translation></message>
    <message><source>Safe shutdown</source><translation>Veilig afsluiten</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Het systeem wordt via UWF veilig afgesloten. Ook beschikbaar als de UWF-overlay vol is.</translation></message>
    <message><source>Safe restart</source><translation>Veilig herstarten</translation></message>
    <message><source>Direct restart</source><translation>Direct herstarten</translation></message>
    <message><source>Direct shutdown</source><translation>Direct afsluiten</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Het systeem wordt via UWF veilig herstart. Ook beschikbaar als de UWF-overlay vol is.</translation></message>
    <message><source>Log</source><translation>Logboek</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Het interne logboek van deze sessie bekijken voor probleemoplossing.</translation></message>
    <message><source>About</source><translation>Over</translation></message>
    <message><source>Switch display language</source><translation>Weergavetaal wijzigen</translation></message>
    <message><source>Language</source><translation>Taal</translation></message>
    <message><source>Theme</source><translation>Thema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Licht/donker thema wisselen</translation></message>
    <message><source>System information</source><translation>Systeeminformatie</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Diagnosemomentopname</translation></message>
    <message><source>Read only</source><translation>Alleen-lezen</translation></message>
    <message><source>The system did not find the target.</source><translation>Het systeem heeft het doel niet gevonden.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Het systeem heeft een parameter geweigerd (ongeldig pad of argument).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Bewerking mislukt. Een ander proces gebruikt het doel mogelijk, bijvoorbeeld Verkenner of een geopend bestand. Sluit de betrokken programma’s en probeer opnieuw.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Bewerking mislukt; zie het logboek voor details.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Bewerking geweigerd (code %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Onbekend.</translation></message>
    <message><source>Commit canceled</source><translation>Vastleggen geannuleerd</translation></message>

    <message><source>Succeeded</source><translation>Geslaagd</translation></message>
    <message><source>Skipped</source><translation>Overgeslagen</translation></message>
    <message><source>Failed</source><translation>Mislukt</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>Geslaagd: %1; mislukt: %2.</translation></message>

    <message><source>Existed before</source><translation>Bestond vóór verwijdering</translation></message>
    <message><source>Exists after</source><translation>Bestaat na verwijdering</translation></message>
    <message><source>Error code</source><translation>Foutcode</translation></message>
    <message><source>Reason</source><translation>Reden</translation></message>
    <message><source>Previous page</source><translation>Vorige pagina</translation></message>
    <message><source>First page</source><translation>Eerste pagina</translation></message>
    <message><source>Last page</source><translation>Laatste pagina</translation></message>
    <message><source>Next page</source><translation>Volgende pagina</translation></message>
    <message><source>Copy selected rows</source><translation>Geselecteerde rijen kopiëren</translation></message>
    <message><source>Copy all</source><translation>Alles kopiëren</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Deze registeruitsluiting is vereist zolang de lijst met klaargezette bestanden opgeslagen vermeldingen bevat. Wis de lijst voordat u de uitsluiting verwijdert.</translation></message>
    <message><source>Copy current page</source><translation>Huidige pagina kopiëren</translation></message>
    <message><source>No pending changes</source><translation>Geen openstaande wijzigingen</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Registeruitsluitingen · Algemene instellingen.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Ga naar beveiliging, bestandsuitsluitingen en klaargezette bestanden voor volume %1.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Niet ondersteund</translation></message>
    <message><source>Review and apply changes</source><translation>Wijzigingen controleren en toepassen</translation></message>
    <message><source>Confirm apply</source><translation>Toepassen bevestigen</translation></message>
    <message><source>Result</source><translation>Resultaat</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filter: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Overlaywaarschuwingsdrempel ingesteld op %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Kritieke overlaydrempel ingesteld op %1 MB</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ Type / maximale capaciteit niet toegepast: UWF is ingeschakeld. Schakel UWF uit en start eerst opnieuw op.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Overlaytype ingesteld op %1</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ Maximale overlaycapaciteit ingesteld op %1 MB</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · UWF-beveiliging: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Koppelen op: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1: bestandsuitsluiting toegevoegd: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1: bestandsuitsluiting verwijderd: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Registeruitsluiting toegevoegd: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Registeruitsluiting verwijderd: %1</translation></message>
    <message><source>Time</source><translation>Tijd</translation></message>
    <message><source>Level</source><translation>Niveau</translation></message>

    <message><source>Message</source><translation>Bericht</translation></message>
    <message><source>Committing staged files</source><translation>Klaargezette bestanden worden vastgelegd</translation></message>
    <message><source>No additional error details were provided.</source><translation>Geen aanvullende foutdetails beschikbaar.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Het automatisch vastleggen van klaargezette bestanden is onverwacht gestopt:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Het automatisch vastleggen van klaargezette bestanden is gestopt door een onbekende fout.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Veilig afsluiten: Mislukt</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Afsluiten: Mislukt (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Veilig herstarten: Mislukt</translation></message>
    <message><source>Direct restart failed</source><translation>Direct herstarten: Mislukt</translation></message>
    <message><source>Direct shutdown failed</source><translation>Direct afsluiten: Mislukt</translation></message>
    <message><source>Restart failed: %1</source><translation>Herstarten: Mislukt (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>De lijst met klaargezette bestanden kon niet worden gelezen:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>De lijst met klaargezette bestanden kon niet worden gelezen door een onbekende fout.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>Gevonden bestanden: %1 · Vastgelegd: %2 · Overgeslagen bestanden: %3 · Overgeslagen vermeldingen: %4 · Mislukt: %5</translation></message>
    <message><source>File staging preparation</source><translation>Bestanden voorbereiden</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Reparsepunten worden niet ondersteund.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Alleen absolute paden op lokale volumes kunnen worden klaargezet.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Alleen absolute paden op lokale volumes kunnen aan de lijst met klaargezette bestanden worden toegevoegd:
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Het klaargezette pad kon niet worden onderzocht.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Het klaargezette bestand verwijst nu naar een ander padtype.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>De klaargezette map verwijst nu naar een ander padtype.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>De klaargezette map kon niet volledig worden opgesomd.</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>De hoofdmap van een volume mag niet recursief worden verwerkt.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>De UWF-provider heeft het vastleggen geweigerd.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Vastleggen mislukt: Onbekende fout.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>%1 extra fouten zijn in het logboek vastgelegd.</translation></message>



    <message><source>No current-session record found for volume %1.</source><translation>Geen gegevens van de huidige sessie voor volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Dit pad staat in de bestandsuitsluitingen.
Uitsluiting: %1</translation></message>

    <message><source>Continue</source><translation>Doorgaan</translation></message>
    <message><source>Delete and commit</source><translation>Verwijderen en vastleggen</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Deze sleutel staat in de registeruitsluitingen.
Uitsluiting: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Geen registerfiltergegevens voor de huidige sessie.</translation></message>

    <message><source>Export commands to file</source><translation>Opdrachten naar bestand exporteren</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Exporteren naar bestand… · Opdracht: %1
%2</translation></message>
    <message><source>Import</source><translation>Importeren</translation></message>
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>Importeer uwfmgr-opdrachten in het concept. Wijzigingen worden pas geschreven na bevestiging in ‘Controleren en toepassen’.</translation></message>
    <message><source>Load from file…</source><translation>Uit bestand laden…</translation></message>
    <message><source>Load recommended configuration</source><translation>Aanbevolen configuratie laden</translation></message>
    <message><source>Choose recommended configuration</source><translation>Aanbevolen configuratie kiezen</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Kies de toe te voegen configuratiegroepen. Controleer of verwijder afzonderlijke opdrachten vóór het importeren.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Uit de officiële Microsoft-documentatie.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Door Microsoft aanbevolen UWF-uitsluitingen</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>Controleer deze aanbevelingen voordat u ze importeert.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Bron: officiële Microsoft-UWF-documentatie over algemene uitsluitingen en antimalwareondersteuning.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Programma voor kwaliteitsverbetering (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Blijvend bewaren: Deelnamestatus in beleid</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Blijvend bewaren: Lokale deelnamestatus</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Blijvend bewaren: Markering voor uitgeschakelde uploads</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Background Intelligent Transfer Service (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Blijvend bewaren: Downloadwachtrijbestanden</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Blijvend bewaren: Overdrachtsstatusindex</translation></message>
    <message><source>Network profiles and policies</source><translation>Netwerkprofielen en beleidsregels</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Groepsbeleid voor draadloze netwerken</translation></message>
    <message><source>Wired network GPO policy</source><translation>Groepsbeleid voor bekabelde netwerken</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Groepsbeleid voor draadloze netwerken · bestanden</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Groepsbeleid voor bekabelde netwerken · bestanden</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Draadloze interfaceprofielen</translation></message>
    <message><source>Wired network interface profiles</source><translation>Bekabelde interfaceprofielen</translation></message>
    <message><source>Wireless service configuration</source><translation>Draadloze serviceconfiguratie</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Configuratie mobiele breedbandservice</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Configuratie van automatische bekabelde netwerkconfiguratie</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>XML-netwerkprofielen zijn apparaatspecifiek. Voeg zo nodig concrete paden Interfaces\{GUID}\{GUID}.xml handmatig toe.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Zomertijd (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Blijvend bewaren: Tijdzonedefinities</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Blijvend bewaren: Geselecteerde tijdzone</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Blijvend bewaren: Productbestanden en updates</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Blijvend bewaren: Handtekeningen en status in ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Blijvend bewaren: Windows Update · Logboek</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Blijvend bewaren: MpCmdRun · Logboek</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Blijvend bewaren: Productstatus in het register</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Blijvend bewaren: WdBoot · Servicestatus</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Blijvend bewaren: WdFilter · Servicestatus</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Blijvend bewaren: WdNisSvc · Servicestatus</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Blijvend bewaren: WdNisDrv · Servicestatus</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Blijvend bewaren: WinDefend · Servicestatus</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Blijvend bewaren: Clientprogrammabestanden</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Blijvend bewaren: Windows Update · Logboek</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Blijvend bewaren: MpCmdRun · Logboek</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Blijvend bewaren: Antimalwarehandtekeningen en status</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Blijvend bewaren: Antimalwarestatus in het register</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Bestanden met uwfmgr-opdrachten kiezen</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Tekstbestanden (*.txt *.cmd);;Alle bestanden (*)</translation></message>
    <message><source>Status</source><translation>Status</translation></message>
    <message><source>Import failed</source><translation>Importeren mislukt</translation></message>
    <message><source>Command import is not ready.</source><translation>Het importeren van opdrachten is nog niet gereed.</translation></message>
    <message><source>Nothing to import</source><translation>Geen opdrachten om te importeren</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Geen uwfmgr-opdrachten gevonden in de invoer.</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplicaat: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Niet ondersteund: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Batch %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Duplicaat</translation></message>
    <message><source>Unsupported</source><translation>Niet ondersteund</translation></message>
    <message><source>File too large</source><translation>Bestand te groot</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Bestand %1 is groter dan 5 MB en is niet verwerkt. Filter het eerst handmatig.</translation></message>
    <message><source>Cannot read file</source><translation>Bestand kan niet worden gelezen</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Mislukt: Bestand openen (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Openstaande wijzigingen verwerpen?</translation></message>
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>%1 wijziging(en) zijn niet toegepast.

Deze verwerpen en doorgaan?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Onvolledige uwfmgr-opdracht</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Ontbrekend argument: Grootte (MB)</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>De grootte moet een niet-negatief geheel getal zijn (MB)</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Ontbrekend argument: Overlay · Type (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Ongeldige waarde: Overlay · Type: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Ontbrekend argument: Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Het volume moet een stationsletter zijn, zoals C:</translation></message>
    <message><source>Missing path argument</source><translation>Ontbrekend argument: Pad</translation></message>
    <message><source>Missing registry key argument</source><translation>Ontbrekend argument: Register · Sleutel</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Niet-gesloten dubbele aanhalingstekens in opdracht</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Onverwacht extra argument: %1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>Deze uwfmgr-opdracht kan niet worden geïmporteerd.</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Doelstatus is al bereikt; geen actie nodig</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Pad staat niet op dit volume of dit volume ondersteunt geen uitsluitingen (bijv. exFAT / ReFS)</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>Dit pad kan niet als uitsluiting worden toegevoegd.</translation></message>
    <message><source>Invalid size value: %1</source><translation>Ongeldige waarde: Grootte: %1</translation></message>
    <message><source>maximum capacity</source><translation>maximale capaciteit</translation></message>
    <message><source>warning threshold</source><translation>Waarschuwingsdrempel</translation></message>
    <message><source>critical threshold</source><translation>Kritieke drempel</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>De algemene configuratie is niet beschikbaar; de opdracht is niet toegevoegd aan de openstaande wijzigingen.</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>Onbekend volume %1 (geen overeenkomende volumeconfiguratie)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Doelstatus is al bereikt; geen actie nodig</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>Pad %1 heeft geen stationsletter; het bijbehorende volumetabblad kan niet worden bepaald.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Niet ondersteund</translation></message>
    <message><source>file exclusion</source><translation>Bestandsuitsluitingen</translation></message>
    <message><source>registry exclusion</source><translation>Registeruitsluitingen</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Registeruitsluitingen zijn alleen beschikbaar op het ontbrekende registertabblad</translation></message>
    <message><source>Unsupported command</source><translation>Opdracht: Niet ondersteund</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>De overlaywidget tonen of verbergen.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Ingeschakeld</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Uitgeschakeld</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Onbekend</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Gebruikt %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Uitgebreide modus</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Het automatisch vastleggen van klaargezette bestanden afstemmen op afsluiten en herstarten van Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Mislukt: Status uitgebreide modus lezen (Onbekende fout).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Mislukt: Servicestatus lezen:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Mislukt: Servicestatus lezen (Onbekende fout).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>De service is niet als zelfstandig proces ingesteld.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>De service is niet ingesteld op automatisch starten.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>De service is niet ingesteld om als LocalSystem te werken.</translation></message>
    <message><source>The service is not running.</source><translation>De service is niet actief.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Het servicepad of de startargumenten komen niet overeen met dit programma.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>De time-out voor vooruitschakeling van de service is verkeerd ingesteld.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>De service vermeldt niet de rechten om de interfaceagent te starten.</translation></message>
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>De service kan momenteel geen Windows-meldingen voorafgaand aan het afsluiten ontvangen.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Uitgebreide modus inschakelen</translation></message>
    <message><source>Disable enhanced mode</source><translation>Uitgebreide modus uitschakelen</translation></message>
    <message><source>Start service</source><translation>Service starten</translation></message>
    <message><source>Repair enhanced mode</source><translation>Uitgebreide modus herstellen</translation></message>
    <message><source>Delete service</source><translation>Service verwijderen</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>De uitgebreide service stoppen en verwijderen, inclusief achtergebleven registergegevens.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>De service ontbreekt, maar registergegevens zijn achtergebleven.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Wachten op verificatie van de interfaceagent.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>De service is actief, maar er is geen geverifieerde interfaceagent verbonden.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Hulpservice voor de uitgebreide modus van UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Uitgebreide modus · Configuratie: Toegepast.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Configuratie: Toegepast. Melding UWF-persistentie:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Uitgebreide modus · Configuratie: Mislukt:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Uitgebreide modus · Configuratie: Mislukt (Onbekende fout).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>De uitgebreide service en achtergebleven configuratie zijn verwijderd.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Serviceverwijdering aangevraagd, maar opruimen meldde:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Uitgebreide modus: Service verwijderen: Mislukt:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Uitgebreide modus: Service verwijderen: Mislukt (Onbekende fout).</translation></message>
    <message><source>Show main window</source><translation>Hoofdvenster tonen</translation></message>
    <message><source>Restore default position</source><translation>Standaardpositie herstellen</translation></message>
    <message><source>Show overlay widget</source><translation>Overlaywidget tonen</translation></message>
    <message><source>Hide overlay widget</source><translation>Overlaywidget verbergen</translation></message>
    <message><source>Exit application</source><translation>Toepassing afsluiten</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Mislukt: Registerwaarden lezen (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Overlaybestanden opsommen: Onbekende fout.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Mislukt: Filteractie: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Mislukt: Filterstatus lezen</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Mislukt: Waarschuwingsdrempel instellen</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Mislukt: Kritieke drempel instellen</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Mislukt: Overlaystatus lezen</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Mislukt: Overlaytype instellen</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ Maximale capaciteit kon niet worden ingesteld</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Mislukt: Overlayconfiguratie lezen</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Mislukt: Beveiliging volume %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Mislukt: Koppeling voor volume %1 instellen</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Mislukt: Bestandsuitsluiting toevoegen %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Mislukt: Bestandsuitsluiting verwijderen %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Mislukt: Volumeconfiguratie lezen</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Mislukt: Registeruitsluiting toevoegen %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Mislukt: Registeruitsluiting verwijderen %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Mislukt: Registerpersistentie wijzigen</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Mislukt: Registerfilter lezen</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Niet ondersteund</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Doelstatus lezen: Onbekende fout.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Onbekende fout.</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>De doelstatus kon niet worden geverifieerd; het resultaat van de bewerking is daarom onbekend: %1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>Het systeem heeft het verwijderverzoek geaccepteerd, maar het doel bestaat na verificatie nog steeds.</translation></message>
    <message><source>Unknown</source><translation>Onbekend</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Mislukt: Logboek laden (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Mislukt: Logboek laden (Onbekende fout).</translation></message>
    <message><source>: unknown error</source><translation>: Onbekende fout</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Mislukt: Doelvolume bepalen (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Mislukt: Registersleutels opsommen (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Mislukt: Doelvolume bepalen (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Overgeslagen omdat een vereiste voorafgaande bewerking is mislukt of niet kon worden bevestigd.</translation></message>
    <message><source>Copy</source><translation>Kopiëren</translation></message>
    <message><source>Select All</source><translation>Alles selecteren</translation></message>
    <message><source>Display language</source><translation>Weergavetaal</translation></message>
    <message><source>Folder</source><translation>Map</translation></message>
    <message><source>File</source><translation>Bestand</translation></message>
    <message><source>Export commands</source><translation>Opdrachten exporteren</translation></message>
    <message><source>Direct restart…</source><translation>Direct herstarten…</translation></message>
    <message><source>Clear…</source><translation>Wissen…</translation></message>
    <message><source>Committing</source><translation>Vastleggen</translation></message>
    <message><source> · %1 selected</source><translation> · %1 geselecteerd</translation></message>
    <message><source>%1 changes</source><translation>%1 wijzigingen</translation></message>
    <message><source>%1 commands</source><translation>%1 opdrachten</translation></message>
    <message><source>%1 items</source><translation>%1 items</translation></message>
    <message><source>%1 items · %2</source><translation>%1 items · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 openstaande wijzigingen</translation></message>
    <message><source>%1 records</source><translation>%1 records</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 records · %2 totaal</translation></message>


    <message><source>%1 selected</source><translation>%1 geselecteerd</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 bestanden</translation></message>
    <message><source>32-bit</source><translation>32-bits</translation></message>
    <message><source>64-bit</source><translation>64-bits</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Uitgebracht onder de &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. De volledige voorwaarden staan in het meegeleverde LICENSE-bestand.&lt;/p&gt;&lt;p&gt;U mag de software onder GPL v3 verspreiden of wijzigen. De software wordt geleverd zoals deze is, zonder enige garantie.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows schakelt wisselbestanden, Systeemherstel, SuperFetch, bestandsindexering, schijfdefragmentatie en snel opstarten uit. Ook wordt bootstatuspolicy in de BCD ingesteld op ignoreallfailures.&lt;/p&gt;&lt;p&gt;U kunt deze instellingen later wijzigen. Verplaats bijvoorbeeld het wisselbestand naar een onbeveiligd volume voordat u het weer inschakelt.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Een diagnostische momentopname van systeem, bevoegdheden en UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>Op deze locatie bestaat al een bestand:
%1

Opslaan vervangt de inhoud ervan.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Een grafisch beheerprogramma voor Windows Unified Write Filter.
Beheer volumebeveiliging, overlay, uitsluitingen en klaargezette bestanden.</translation></message>
    <message><source>Snapshot details</source><translation>Momentopnamedetails</translation></message>
    <message><source>Account</source><translation>Account</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Voeg UWF-configuratie toe vanuit een script of tekst.</translation></message>
    <message><source>Add exclusion</source><translation>Uitsluiting toevoegen</translation></message>
    <message><source>Add files or folders</source><translation>Bestanden of mappen toevoegen</translation></message>
    <message><source>Add selected items</source><translation>Geselecteerde items toevoegen</translation></message>
    <message><source>Added</source><translation>Toegevoegd</translation></message>
    <message><source>Administrator</source><translation>Beheerder</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>De geselecteerde bewerkingen zijn voltooid. UWF kan uitgesloten doelen overslaan.</translation></message>
    <message><source>All files (*)</source><translation>Alle bestanden (*)</translation></message>
    <message><source>All levels</source><translation>Alle niveaus</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Een lege diagnostische momentopname betekent niet dat de overlay geen wijzigingen bevat.</translation></message>
    <message><source>Application path</source><translation>Toepassingspad</translation></message>
    <message><source>Application result</source><translation>Resultaat van toepassen</translation></message>
    <message><source>Apply changes</source><translation>Wijzigingen toepassen</translation></message>
    <message><source>Architecture</source><translation>Architectuur</translation></message>
    <message><source>Automatic</source><translation>Automatisch</translation></message>
    <message><source>Automatic staged file commits</source><translation>Klaargezette bestanden automatisch vastleggen</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>Automatisch vastleggen van bestanden onvolledig</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>Geschikte klaargezette bestanden automatisch vastleggen bij het afsluiten en herstarten van Windows.</translation></message>
    <message><source>Available</source><translation>Beschikbaar</translation></message>
    <message><source>Back</source><translation>Terug</translation></message>
    <message><source>Background service</source><translation>Achtergrondservice</translation></message>
    <message><source>Binding</source><translation>Koppeling</translation></message>
    <message><source>By drive letter</source><translation>Op stationsletter</translation></message>
    <message><source>By volume ID</source><translation>Op volume-ID</translation></message>
    <message><source>Cancel reading</source><translation>Lezen annuleren</translation></message>
    <message><source>Completed operations cannot be undone.</source><translation>Voltooide bewerkingen kunnen niet ongedaan worden gemaakt.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Selecteer de registersleutel die u wilt uitsluiten van UWF-beveiliging, inclusief de bijbehorende subsleutels en waarden.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Kies een locatie om het bestand op te slaan.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Kies een locatie binnen het toegestane station of de toegestane map.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Kies een andere map of controleer de toegangsrechten en probeer het opnieuw.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Kies een andere sleutel of controleer de toegangsrechten en probeer het opnieuw.</translation></message>
    <message><source>Clear filters</source><translation>Filters wissen</translation></message>
    <message><source>Clear logs</source><translation>Logboeken wissen</translation></message>
    <message><source>Clear logs?</source><translation>Logboeken wissen?</translation></message>
    <message><source>Collapse</source><translation>Inklappen</translation></message>
    <message><source>Commit changes</source><translation>Wijzigingen vastleggen</translation></message>
    <message><source>Commit changes…</source><translation>Wijzigingen vastleggen…</translation></message>
    <message><source>Commit file changes?</source><translation>Bestandswijzigingen vastleggen?</translation></message>
    <message><source>Commit folder changes?</source><translation>Mapwijzigingen vastleggen?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Wijzigingen aan registersleutel vastleggen?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Wijzigingen aan registerwaarde vastleggen?</translation></message>
    <message><source>Compatibility mode</source><translation>Compatibiliteitsmodus</translation></message>
    <message><source>Configuration applied</source><translation>Configuratie toegepast</translation></message>
    <message><source>Configuration error</source><translation>Configuratiefout</translation></message>
    <message><source>Configured</source><translation>Geconfigureerd</translation></message>
    <message><source>Confirm before continuing</source><translation>Bevestig voordat u doorgaat</translation></message>
    <message><source>Connected and authenticated</source><translation>Verbonden en geverifieerd</translation></message>
    <message><source>Connecting…</source><translation>Verbinden…</translation></message>
    <message><source>Contact author</source><translation>Contact met auteur</translation></message>
    <message><source>Continue editing</source><translation>Verder bewerken</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>Als u doorgaat met herstarten, kunnen niet-vastgelegde bestandswijzigingen verloren gaan.</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>Als u doorgaat met afsluiten, kunnen niet-vastgelegde bestandswijzigingen verloren gaan.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Afstemmen op het afsluiten van Windows</translation></message>
    <message><source>Copy all logs</source><translation>Alle logboeken kopiëren</translation></message>
    <message><source>Copy full report</source><translation>Volledig rapport kopiëren</translation></message>
    <message><source>Copy name</source><translation>Naam kopiëren</translation></message>
    <message><source>Copy path</source><translation>Pad kopiëren</translation></message>
    <message><source>Copy selection</source><translation>Selectie kopiëren</translation></message>
    <message><source>Copy version</source><translation>Versie kopiëren</translation></message>
    <message><source>Counting staged files…</source><translation>Klaargezette bestanden tellen…</translation></message>
    <message><source>Create</source><translation>Maken</translation></message>
    <message><source>Critical %1 MB</source><translation>Kritiek %1 MB</translation></message>
    <message><source>Current filter</source><translation>Huidig filter</translation></message>
    <message><source>Current run</source><translation>Huidige uitvoering</translation></message>
    <message><source>Current selection</source><translation>Huidige selectie</translation></message>
    <message><source>Current usage</source><translation>Huidig gebruik</translation></message>
    <message><source>Date modified</source><translation>Gewijzigd op</translation></message>
    <message><source>Debug</source><translation>Foutopsporing</translation></message>
    <message><source>Delete and commit this file?</source><translation>Dit bestand verwijderen en de verwijdering vastleggen?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Deze map verwijderen en de verwijdering vastleggen?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Deze registersleutel verwijderen en de verwijdering vastleggen?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Deze registerwaarde verwijderen en de verwijdering vastleggen?</translation></message>

    <message><source>Direct restart?</source><translation>Direct herstarten?</translation></message>
    <message><source>Direct shutdown?</source><translation>Direct afsluiten?</translation></message>
    <message><source>Discard changes</source><translation>Wijzigingen verwerpen</translation></message>
    <message><source>Disk</source><translation>Schijf</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Domeingeheimsleutel behouden</translation></message>
    <message><source>Done</source><translation>Gereed</translation></message>
    <message><source>Edit commands</source><translation>Opdrachten bewerken</translation></message>
    <message><source>Editable</source><translation>Bewerkbaar</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Schakel het filter in en herstart om het actuele gebruik te tonen</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>De uitgebreide modus voegt een achtergrondservice toe die tegelijk met Windows start. Verwijder de service voordat u de toepassing verplaatst of verwijdert.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>De uitgebreide modus is niet beschikbaar. Gebruik ‘Veilig afsluiten’ of ‘Veilig herstarten’ om klaargezette bestanden vast te leggen.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Voer HKLM\SOFTWARE\… in en druk op Enter om te navigeren</translation></message>
    <message><source>Enter a file name</source><translation>Voer een bestandsnaam in</translation></message>
    <message><source>Enter a valid file name.</source><translation>Voer een geldige bestandsnaam in.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Voer een geldige bestands- of mapnaam in.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Voer een geldige mapnaam in.</translation></message>
    <message><source>Error</source><translation>Fout</translation></message>
    <message><source>Error details</source><translation>Foutdetails</translation></message>
    <message><source>Error details and suggestions</source><translation>Foutdetails en suggesties</translation></message>
    <message><source>Expand</source><translation>Uitklappen</translation></message>
    <message><source>Export list</source><translation>Lijst exporteren</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Logboeken wissen mislukt: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Logboeken wissen mislukt: onverwachte fout.</translation></message>
    <message><source>File name</source><translation>Bestandsnaam</translation></message>
    <message><source>File or folder</source><translation>Bestand of map</translation></message>
    <message><source>File commits have started and cannot be canceled.</source><translation>Het vastleggen van bestanden is begonnen en kan niet worden geannuleerd.</translation></message>
    <message><source>File commits incomplete</source><translation>Vastleggen van bestanden onvolledig</translation></message>
    <message><source>File type</source><translation>Bestandstype</translation></message>
    <message><source>Files and folders</source><translation>Bestanden en mappen</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Bestanden en mappen kunnen samen worden geselecteerd.</translation></message>
    <message><source>Filter is disabled</source><translation>Filter is uitgeschakeld</translation></message>
    <message><source>Fit columns to contents</source><translation>Kolommen aan inhoud aanpassen</translation></message>
    <message><source>Folder address</source><translation>Mapadres</translation></message>
    <message><source>Folder name</source><translation>Mapnaam</translation></message>
    <message><source>Folders</source><translation>Mappen</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>Deze sleutel toevoegen aan de uitsluitingen voor de volgende sessie.</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>Dit pad toevoegen aan de uitsluitingen voor de volgende sessie.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Globale registeruitsluiting · inclusief subsleutels</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Ga één map omhoog of kies een andere locatie.</translation></message>
    <message><source>Home</source><translation>Persoonlijke map</translation></message>
    <message><source>Import configuration</source><translation>Configuratie importeren</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>Importeren werkt het concept bij. Bevestig de wijzigingen in ‘Controleren en toepassen’ om ze naar het systeem te schrijven.</translation></message>
    <message><source>Import results</source><translation>Importresultaten</translation></message>
    <message><source>Imported</source><translation>Geïmporteerd</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>De lijst toont de opdrachtresultaten voor het configuratieconcept; selecteer een rij om de volledige opdracht en details te bekijken.</translation></message>
    <message><source>Imported: %1</source><translation>Geïmporteerd: %1</translation></message>
    <message><source>Information</source><translation>Informatie</translation></message>
    <message><source>Item details</source><translation>Itemdetails</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>Veel klaargezette bestanden vastleggen kan langer duren.</translation></message>
    <message><source>License and copyright</source><translation>Licentie en auteursrecht</translation></message>
    <message><source>Line</source><translation>Regel</translation></message>
    <message><source>Local disk (%1)</source><translation>Lokale schijf (%1)</translation></message>
    <message><source>Log level</source><translation>Logboekniveau</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Logboeken lezen mislukt. Probeer opnieuw of controleer de toegang tot het logbestand.</translation></message>
    <message><source>Maximize</source><translation>Maximaliseren</translation></message>
    <message><source>Message details</source><translation>Berichtdetails</translation></message>
    <message><source>Minimize</source><translation>Minimaliseren</translation></message>
    <message><source>Module</source><translation>Module</translation></message>
    <message><source>More</source><translation>Meer</translation></message>
    <message><source>Move</source><translation>Verplaatsen</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Als UWF het station van de toepassing beveiligt, zorg er dan eerst voor dat de toepassingsbestanden na een herstart behouden blijven. Veel klaargezette bestanden vastleggen kan het afsluiten of herstarten vertragen.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Meerdere bestanden kunnen worden geselecteerd.</translation></message>
    <message><source>Name and location</source><translation>Naam en locatie</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Benoemde waarde: %1 · alleen deze waarde</translation></message>
    <message><source>Navigate to registry path</source><translation>Naar registerpad navigeren</translation></message>
    <message><source>New folder</source><translation>Nieuwe map</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Nieuwe records verschijnen na vernieuwen.</translation></message>
    <message><source>Next-session configuration</source><translation>Configuratie van de volgende sessie</translation></message>
    <message><source>No files to commit</source><translation>Geen bestanden om vast te leggen</translation></message>
    <message><source>No logs yet</source><translation>Nog geen logboeken</translation></message>
    <message><source>No matching items</source><translation>Geen overeenkomende items</translation></message>
    <message><source>No matching logs</source><translation>Geen overeenkomende logboeken</translation></message>
    <message><source>No overlay files found</source><translation>Geen overlaybestanden gevonden</translation></message>
    <message><source>No registry key selected</source><translation>Geen registersleutel geselecteerd</translation></message>
    <message><source>None</source><translation>Geen</translation></message>
    <message><source>Not configured</source><translation>Niet geconfigureerd</translation></message>
    <message><source>Not connected</source><translation>Niet verbonden</translation></message>
    <message><source>Not installed</source><translation>Niet geïnstalleerd</translation></message>
    <message><source>Not written to the system</source><translation>Niet naar het systeem geschreven</translation></message>
    <message><source>Off</source><translation>Uit</translation></message>
    <message><source>On</source><translation>Aan</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Eén uwfmgr-opdracht per regel</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Bewaar de toepassing in een vaste map die standaardgebruikers niet kunnen wijzigen. Voorgestelde locatie:</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>Alleen klaargezette bestanden worden automatisch vastgelegd. Andere niet-vastgelegde wijzigingen kunnen na herstarten verloren gaan.</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>Alleen klaargezette bestanden worden automatisch vastgelegd. Andere niet-vastgelegde wijzigingen kunnen na afsluiten verloren gaan.</translation></message>
    <message><source>Open</source><translation>Openen</translation></message>
    <message><source>Open folder</source><translation>Map openen</translation></message>
    <message><source>Operating system</source><translation>Besturingssysteem</translation></message>
    <message><source>Operation and security</source><translation>Werking en beveiliging</translation></message>
    <message><source>Original command</source><translation>Oorspronkelijke opdracht</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Kritieke overlaydrempel bereikt. Sla wijzigingen op en herstart veilig.</translation></message>
    <message><source>Maximum overlay capacity</source><translation>Maximale overlaycapaciteit</translation></message>
    <message><source>Overlay type</source><translation>Overlaytype</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Waarschuwingsdrempel overlay bereikt. Sla wijzigingen op en herstart veilig.</translation></message>
    <message><source>Overview</source><translation>Overzicht</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Plak opdrachten of laad ze uit een bestand…</translation></message>
    <message><source>Pending changes</source><translation>Openstaande wijzigingen</translation></message>
    <message><source>Pending removal</source><translation>Verwijdering in behandeling</translation></message>
    <message><source>Permissions and protection</source><translation>Machtigingen en beveiliging</translation></message>
    <message><source>Privileges</source><translation>Bevoegdheden</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2 items verwerkt</translation></message>
    <message><source>Processing result</source><translation>Verwerkingsresultaat</translation></message>
    <message><source>Protected</source><translation>Beveiligd</translation></message>
    <message><source>Protected volumes</source><translation>Beveiligde volumes</translation></message>
    <message><source>Protection and overlay</source><translation>Beveiliging en overlay</translation></message>
    <message><source>Quick access</source><translation>Snelle toegang</translation></message>
    <message><source>Raw report</source><translation>Onbewerkt rapport</translation></message>
    <message><source>Read again</source><translation>Opnieuw lezen</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Lees opnieuw om een diagnostische momentopname op te halen.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Lees de configuratie om het actuele gebruik te tonen</translation></message>
    <message><source>Read-only</source><translation>Alleen-lezen</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Het lezen van een grote overlay kan langer duren. U kunt op elk moment annuleren.</translation></message>
    <message><source>Reading canceled</source><translation>Lezen geannuleerd</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Bestanden en submappen op deze locatie lezen…</translation></message>
    <message><source>Reading folder</source><translation>Map lezen</translation></message>
    <message><source>Reading logs</source><translation>Logboeken lezen</translation></message>
    <message><source>Reading overlay</source><translation>Overlay lezen</translation></message>
    <message><source>Ready</source><translation>Gereed voor gebruik</translation></message>
    <message><source>Files to commit</source><translation>Vast te leggen bestanden</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Records van de huidige uitvoering verschijnen hier zodra het lezen is voltooid.</translation></message>

    <message><source>Refresh folder</source><translation>Map vernieuwen</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Registersleutel of instelling voor behoud</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>Bij annuleren stoppen de resterende taken zodra de huidige bewerking is voltooid.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Deze sleutel uit de uitsluitingen van de volgende sessie verwijderen.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Dit pad uit de uitsluitingen van de volgende sessie verwijderen.</translation></message>
    <message><source>Removed</source><translation>Verwijderd</translation></message>
    <message><source>Rename</source><translation>Naam wijzigen</translation></message>
    <message><source>Rename…</source><translation>Naam wijzigen…</translation></message>
    <message><source>Repair required</source><translation>Herstel vereist</translation></message>
    <message><source>Replace file</source><translation>Bestand vervangen</translation></message>
    <message><source>Replace the existing file?</source><translation>Bestaand bestand vervangen?</translation></message>
    <message><source>Required configuration</source><translation>Vereiste configuratie</translation></message>
    <message><source>Restart anyway</source><translation>Toch herstarten</translation></message>
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>Het herstarten gaat verder na het vastleggen. Houd de toepassing open.</translation></message>
    <message><source>Restore</source><translation>Herstellen</translation></message>
    <message><source>Retry</source><translation>Opnieuw proberen</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Controleer de foutdetails voordat u doorgaat met afsluiten.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Controleer de foutdetails voordat u herstart.</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>Sommige wijzigingen zijn mogelijk al geschreven. Vernieuw de systeemstatus voordat u het opnieuw probeert.</translation></message>
    <message><source>Review the result for each target.</source><translation>Controleer het resultaat voor elk doel.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Controleer deze wijzigingen voordat u ze naar het systeem schrijft.</translation></message>
    <message><source>Running</source><translation>Actief</translation></message>
    <message><source>Save</source><translation>Opslaan</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>Klaargezette bestanden vastleggen op schijf en daarna herstarten.</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>Klaargezette bestanden vastleggen op schijf en daarna afsluiten.</translation></message>

    <message><source>Committing to disk</source><translation>Vastleggen op schijf</translation></message>
    <message><source>Committing…</source><translation>Vastleggen…</translation></message>
    <message><source>Scanning staged paths</source><translation>Klaargezette paden scannen</translation></message>
    <message><source>Search logs</source><translation>Logboeken doorzoeken</translation></message>
    <message><source>Search messages or modules</source><translation>Berichten of modules zoeken</translation></message>
    <message><source>Search paths</source><translation>Paden zoeken</translation></message>
    <message><source>Search this folder</source><translation>Deze map doorzoeken</translation></message>
    <message><source>Select a file or enter its name</source><translation>Selecteer een bestand of voer de naam in</translation></message>
    <message><source>Select a file to open.</source><translation>Selecteer een bestand om te openen.</translation></message>
    <message><source>Select a folder.</source><translation>Selecteer een map.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Selecteer een sleutel om de volledige onderliggende structuur te bewerken, of een benoemde waarde om alleen die waarde te bewerken.</translation></message>
    <message><source>Select an available subkey</source><translation>Beschikbare subsleutel selecteren</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Selecteer alleen bestanden of open een map om de inhoud te bekijken.</translation></message>
    <message><source>Select files or folders</source><translation>Bestanden of mappen selecteren</translation></message>
    <message><source>Select folder</source><translation>Map selecteren</translation></message>
    <message><source>Select folders only.</source><translation>Selecteer alleen mappen.</translation></message>
    <message><source>Select registry key or value</source><translation>Registersleutel of -waarde selecteren</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Te verwijderen registersleutel of -waarde selecteren</translation></message>
    <message><source>Select the current folder</source><translation>Huidige map selecteren</translation></message>
    <message><source>Select this file</source><translation>Dit bestand selecteren</translation></message>
    <message><source>Select this registry key</source><translation>Deze registersleutel selecteren</translation></message>
    <message><source>Select this registry value</source><translation>Deze registerwaarde selecteren</translation></message>
    <message><source>Selected file</source><translation>Geselecteerd bestand</translation></message>
    <message><source>Selected folder</source><translation>Geselecteerde map</translation></message>
    <message><source>Selection</source><translation>Selectie</translation></message>
    <message><source>Service account</source><translation>Serviceaccount</translation></message>
    <message><source>Service configuration needs repair</source><translation>Serviceconfiguratie moet worden hersteld</translation></message>
    <message><source>Service details</source><translation>Servicedetails</translation></message>
    <message><source>Service name</source><translation>Servicenaam</translation></message>
    <message><source>Service status</source><translation>Servicestatus</translation></message>
    <message><source>Service stopped</source><translation>Service gestopt</translation></message>
    <message><source>Shut down anyway</source><translation>Toch afsluiten</translation></message>
    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>Het afsluiten gaat verder na het vastleggen. Houd de toepassing open.</translation></message>
    <message><source>Size</source><translation>Grootte</translation></message>
    <message><source>Some changes could not be applied</source><translation>Sommige wijzigingen konden niet worden toegepast</translation></message>
    <message><source>Some files could not be committed</source><translation>Sommige bestanden konden niet worden vastgelegd</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Sommige hardwaregegevens konden niet worden gelezen. Andere diagnostische gegevens blijven beschikbaar om te bekijken en kopiëren.</translation></message>
    <message><source>Source code</source><translation>Broncode</translation></message>

    <message><source>Staged path</source><translation>Klaargezet pad</translation></message>
    <message><source>Standard user</source><translation>Standaardgebruiker</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Start automatisch met Windows</translation></message>
    <message><source>Startup type</source><translation>Opstarttype</translation></message>
    <message><source>Status unavailable</source><translation>Status niet beschikbaar</translation></message>
    <message><source>Stopped</source><translation>Gestopt</translation></message>
    <message><source>Storage</source><translation>Opslag</translation></message>
    <message><source>System</source><translation>Systeem</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Systeemwijzigingen bij eerste inschakeling van UWF</translation></message>
    <message><source>System memory</source><translation>Systeemgeheugen</translation></message>
    <message><source>System metadata</source><translation>Systeemmetagegevens</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Systeemmetagegevens zijn alleen voor diagnose en kunnen niet worden vastgelegd.</translation></message>
    <message><source>Takes effect after restart</source><translation>Wordt actief na herstarten</translation></message>
    <message><source>Takes effect when applied</source><translation>Van kracht bij toepassen</translation></message>
    <message><source>Used when UWF starts</source><translation>Gebruikt bij het starten van UWF</translation></message>
    <message><source>Mixed activation times</source><translation>Verschillende activeringsmomenten</translation></message>
    <message><source>Target</source><translation>Doel</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Terminal Services-licenties behouden</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>De UWF-provider kon de opsomming niet voltooien. Dit kan optreden bij een grote overlay of onvoldoende systeembronnen.</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>De configuratie is naar het systeem geschreven.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>De huidige logboekrecords worden onherroepelijk gewist.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>De doelmap bestaat niet.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>De servicestatus van de uitgebreide modus kon niet worden gelezen. Controleer de servicemachtigingen en probeer het opnieuw.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>De lijst kan bestanden missen of reeds vastgelegde bestanden bevatten. Gebruik deze niet als lijst voor vastleggen.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Het geselecteerde item bestaat niet meer: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>De service is geïnstalleerd, maar gestopt. Start deze om het automatisch vastleggen van bestanden te hervatten.</translation></message>
    <message><source>This PC</source><translation>Deze pc</translation></message>
    <message><source>This folder is empty</source><translation>Deze map is leeg</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Dit rapport kan accountnamen, apparaat-ID’s en bestandspaden bevatten. Controleer het voordat u het deelt.</translation></message>

    <message><source>Try another keyword or log level.</source><translation>Probeer een ander trefwoord of logboekniveau.</translation></message>
    <message><source>Try another search term.</source><translation>Probeer een andere zoekterm.</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>Schakel UWF uit en start opnieuw op voordat u het overlaytype of de maximale capaciteit wijzigt.</translation></message>
    <message><source>UI agent</source><translation>UI-agent</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 regels</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>De UWF-configuratie kon niet worden gelezen. Controleer of Unified Write Filter in Windows is geïnstalleerd en lees opnieuw.</translation></message>
    <message><source>UWF feature</source><translation>UWF-onderdeel</translation></message>
    <message><source>UWF filter</source><translation>UWF-filter</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF is momenteel niet beschikbaar</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Veilig herstarten via UWF wordt direct uitgevoerd.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>Veilig afsluiten via UWF wordt direct uitgevoerd.</translation></message>
    <message><source>Unable to access this folder</source><translation>Geen toegang tot deze map</translation></message>
    <message><source>Unable to access this registry key</source><translation>Geen toegang tot deze registersleutel</translation></message>
    <message><source>Unable to read logs</source><translation>Logboeken kunnen niet worden gelezen</translation></message>
    <message><source>Unable to read service status</source><translation>Servicestatus kan niet worden gelezen</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Overlay kan momenteel niet worden gelezen</translation></message>
    <message><source>Unable to read this folder.</source><translation>Deze map kan niet worden gelezen.</translation></message>
    <message><source>Unable to select this item</source><translation>Dit item kan niet worden geselecteerd</translation></message>
    <message><source>Unavailable</source><translation>Niet beschikbaar</translation></message>
    <message><source>Undo removal</source><translation>Verwijdering ongedaan maken</translation></message>
    <message><source>Unprotected</source><translation>Onbeveiligd</translation></message>
    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>Sommige items zijn niet volledig verwerkt of konden niet worden geverifieerd. Selecteer een item om het volledige pad en de details te bekijken.</translation></message>
    <message><source>Up one folder</source><translation>Eén map omhoog</translation></message>
    <message><source>Usage unavailable</source><translation>Gebruik niet beschikbaar</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Volledige sleutel inclusief alle subsleutels gebruiken</translation></message>
    <message><source>Version %1</source><translation>Versie %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Bekijk toepassingsrecords en diagnostische informatie.</translation></message>
    <message><source>View commands for these changes</source><translation>Opdrachten voor deze wijzigingen bekijken</translation></message>
    <message><source>View failure details</source><translation>Foutdetails bekijken</translation></message>
    <message><source>View overlay</source><translation>Overlay bekijken</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Bekijk de diagnostische bestandsopname voor %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Koppeling voor volume %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Beveiliging voor volume %1</translation></message>
    <message><source>Volume identity</source><translation>Volume-identiteit</translation></message>
    <message><source>Warning</source><translation>Waarschuwing</translation></message>
    <message><source>Warning %1 MB</source><translation>Waarschuwing %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Volledige sleutel en alle subsleutels · recursief</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF biedt schrijfbeveiliging. Deze toepassing configureert en beheert deze.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>Gebruik zonder uitgebreide modus ‘Veilig afsluiten’ of ‘Veilig herstarten’ in deze toepassing om klaargezette bestanden vast te leggen.</translation></message>
    <message><source>Write protection</source><translation>Schrijfbeveiliging</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>Deze configuratiewijzigingen naar het systeem schrijven?</translation></message>
    <message><source>staged files</source><translation>klaargezette bestanden</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr-opdrachten</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Markeer de geselecteerde paden voor verwijdering. Dit wordt van kracht na toepassen.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Na toepassen uit de lijst met klaargezette bestanden verwijderen.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Na toepassen aan de lijst met klaargezette bestanden toevoegen.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Door dit plan toe te passen wordt de opgeslagen lijst met klaargezette bestanden gewist en wordt de bijbehorende registeruitsluiting verwijderd als deze niet meer nodig is.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Wijzigingen in Klaargezette bestanden moeten in UWF Manager worden toegepast en kunnen niet als uwfmgr-opdrachten worden geëxporteerd.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>De lijst met klaargezette bestanden wordt na toepassen opgeslagen</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Bereik van de overlaymomentopname</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Deze lijst is alleen voor diagnose. Hiermee kan niet worden bepaald welke bestanden moeten worden vastgelegd.</translation></message>
    <message><source>NTFS volumes only</source><translation>Alleen NTFS-volumes</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Het lezen van grote overlays kan traag zijn of mislukken door onvoldoende bronnen.</translation></message>
    <message><source>Small files may be missing</source><translation>Kleine bestanden kunnen ontbreken</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Bestanden die kleiner zijn dan de clustergrootte van de schijf (meestal 4 KB) staan mogelijk niet in de lijst.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Geeft geen wachtende vastleggingen aan</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Ook al vastgelegde bestanden, uitgesloten paden en items die door andere bewerkingen zijn beïnvloed, kunnen worden weergegeven.</translation></message>

    <message><source>Command processing result</source><translation>Resultaat van opdrachtverwerking</translation></message>
    <message><source>Copy command</source><translation>Opdracht kopiëren</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 takken konden niet worden gelezen</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 takken konden niet worden gelezen. Ga verder met gevonden inhoud of probeer alleen mislukte items opnieuw.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 bestanden · %2 mappen</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 waarden · %2 sleutels</translation></message>
    <message><source>Attempts</source><translation>Pogingen</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Resterende taken annuleren</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Alleen succesvol doorlopen inhoud vastleggen. Niet-gelezen takken blijven ongewijzigd.</translation></message>
    <message><source>Committing changes</source><translation>Wijzigingen vastleggen</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Voltooide verwijderingen en vastleggingen kunnen niet ongedaan worden gemaakt.</translation></message>
    <message><source>Continue…</source><translation>Doorgaan…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Alleen succesvol doorlopen doelen verwijderen en vastleggen; takken die niet konden worden gelezen en hun bovenliggende mappen of sleutels blijven behouden.</translation></message>
    <message><source>Deleting and committing</source><translation>Verwijderen en vastleggen</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Verwijderen en vastleggen kunnen niet ongedaan worden gemaakt. Alleen het geselecteerde bereik wordt verwerkt; wachtende UWF-instellingen worden niet toegepast.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Bereik bepalen. Onleesbare takken worden vastgelegd terwijl andere inhoud verder wordt doorlopen.</translation></message>
    <message><source>Export traversal records</source><translation>Doorlooplogboek exporteren</translation></message>
    <message><source>No actionable content</source><translation>Geen verwerkbare inhoud</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Geen verwerkbare doelen gevonden. Er is niets verwijderd of vastgelegd.</translation></message>
    <message><source>No changes have been committed.</source><translation>Nog geen wijzigingen vastgelegd</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Niets verwijderd of vastgelegd</translation></message>
    <message><source>Path</source><translation>Pad</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>De geselecteerde inhoud definitief verwijderen en de verwijderingen vastleggen.</translation></message>
    <message><source>Preparation complete</source><translation>Voorbereiding voltooid</translation></message>
    <message><source>Preparing commit</source><translation>Vastleggen voorbereiden</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Verwijderen en vastleggen voorbereiden</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Alleen succesvol doorlopen inhoud verwerken</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Reparsepunten worden overgeslagen om binnen het geselecteerde bereik te blijven.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Opnieuw starten maakt vastleggen niet ongedaan. Alleen het geselecteerde bereik wordt verwerkt; wachtende UWF-instellingen worden niet toegepast.</translation></message>
    <message><source>Retry failed items</source><translation>Mislukte items opnieuw proberen</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>Wijzigingen in het geselecteerde bereik vastleggen.</translation></message>
    <message><source>Stopping the operation</source><translation>Bewerking stoppen</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Geslaagde en mislukte leesacties zijn vastgelegd</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>De map is niet leeg; onverwerkte inhoud is behouden.</translation></message>
    <message><source>The operation could not be completed</source><translation>De bewerking kon niet worden voltooid</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>De registersleutel bevat nieuwe waarden; onverwerkte inhoud is behouden.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>De registersleutel heeft nog subsleutels; onverwerkte inhoud is behouden.</translation></message>
    <message><source>The target no longer exists.</source><translation>Het doel bestaat niet meer.</translation></message>
    <message><source>Traversal complete</source><translation>Doorlopen voltooid</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Doorlopen voltooid met niet-gelezen inhoud</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Het doorlooplogboek blijft behouden en kan worden geëxporteerd.</translation></message>
    <message><source>Traverse again</source><translation>Opnieuw doorlopen</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Doorloop opnieuw of sluit dit venster.</translation></message>
    <message><source>Traversing the folder</source><translation>Map doorlopen</translation></message>
    <message><source>Traversing the registry</source><translation>Register doorlopen</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Kan de bewerking niet voorbereiden</translation></message>
    <message><source>View results</source><translation>Resultaten bekijken</translation></message>

    <message><source>All operations succeeded</source><translation>Alle bewerkingen geslaagd</translation></message>
    <message><source>Some operations succeeded</source><translation>Sommige bewerkingen geslaagd</translation></message>
    <message><source>No operations succeeded</source><translation>Geen bewerkingen geslaagd</translation></message>
    <message><source>No targets to process</source><translation>Geen doelen om te verwerken</translation></message>
    <message><source>Commit interrupted</source><translation>Vastleggen onderbroken</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 items zijn niet verwerkt.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 registerbewerkingen zijn mislukt.</translation></message>
    <message><source>The folder does not exist.</source><translation>De map bestaat niet.</translation></message>
    <message><source>Access denied.</source><translation>Toegang geweigerd.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Er bestaat al een item met deze naam.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>De map kon niet worden gemaakt. Controleer de naam en de machtigingen.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>De naam van het item kon niet worden gewijzigd. Controleer de naam en de machtigingen.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Deze browser ondersteunt het wijzigen van namen niet.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Het concept van de klaargezette bestanden is gewijzigd nadat dit plan is geopend. Controleer een nieuw plan.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>De opgeslagen lijst met klaargezette bestanden is gewijzigd nadat dit plan is geopend. Vernieuw en controleer de wijzigingen.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>De opgeslagen lijst met klaargezette bestanden komt niet overeen met het geaccepteerde plan.</translation></message>
    <message><source>Export interface settings</source><translation>Interface-instellingen exporteren</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>Het huidige thema, de taal en de zichtbaarheid van de overlaywidget opslaan in UWF.ini naast deze toepassing.</translation></message>
    <message><source>Interface settings saved</source><translation>Interface-instellingen opgeslagen</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>De huidige interface-instellingen zijn opgeslagen in:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini bestaat al. Het bestaande bestand is niet gewijzigd:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>UWF.ini kon niet worden opgeslagen:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Bureaublad</translation></message>
    <message><source>Downloads</source><translation>Downloads</translation></message>
    <message><source>Documents</source><translation>Documenten</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Het vastleggen van klaargezette bestanden is gestopt. Verhelp de fout voordat u het opnieuw probeert.</translation></message>
    <message><source>No net change for this setting</source><translation>De eindwaarde van deze instelling is na het importeren ongewijzigd</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Uiteindelijke conceptwaarde: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Uiteindelijke conceptwaarde: %1 → %2 (gevraagd: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Gevraagd: %1 → %2; vervangen door een latere opdracht. Uiteindelijke conceptwaarde → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Concept voor %1 bijgewerkt</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Concept voor de beveiligingsinstelling van volume %1 bijgewerkt → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Selecteer de vermeldingen die u wilt verwijderen.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>De geselecteerde vermeldingen kunnen momenteel niet worden hersteld.</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>Direct schrijven via vrije ruimte</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>Persistente overlay</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>Persistente gegevens bij de volgende start wissen</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>Modus voor alleen-lezenmedia</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>Gebruikt vrije schijfruimte voor schrijfbewerkingen en vermindert het gebruik van de gereserveerde overlay; ondersteunt RAM (niet aanbevolen) en Disk (aanbevolen).</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>Behoudt schrijfbewerkingen in de Disk-overlay na opnieuw opstarten; niet beschikbaar voor RAM-overlays.</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>Stelt de persistente overlay opnieuw in: wist de schrijfbewerkingen bij de volgende start en herstelt de toestand van toen de overlay werd ingeschakeld</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>Beheerdersrechten zijn vereist; de overlayconfiguratie is alleen-lezen.</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>Configuratie bewerken is tijdelijk niet beschikbaar. Wacht tot de huidige bewerking is voltooid.</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>De overlayconfiguratie kon niet worden gelezen. Vernieuw voordat u deze bewerkt.</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>Dit systeem ondersteunt het wissen van persistente gegevens bij de volgende start niet.</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>Het huidige wisverzoek is onbekend. Vernieuw de configuratie voordat u deze bewerkt.</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>Om dit verzoek te wijzigen, moeten UWF en de persistente Disk-overlay in de huidige sessie actief zijn.</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>Wissen bij de volgende start vereist een persistente Disk-overlay. Annuleer het wisverzoek of behoud de persistente overlay.</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>Een persistente overlay vereist Disk. Schakel de persistente overlay uit of selecteer Disk.</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>De modus voor alleen-lezenmedia vereist RAM. Schakel deze uit of selecteer RAM.</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>UWF is ingeschakeld in de huidige sessie. Schakel UWF uit en start opnieuw op voordat u deze instelling wijzigt.</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>De configuratie is gewijzigd. Sluit en heropen dit dialoogvenster voordat u deze bewerkt.</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>Geavanceerde overlayinstellingen</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>Huidige overlay</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>Volgende overlay</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>Het concept bevat onverenigbare of niet-beschikbare overlayinstellingen. Ga terug naar bewerken en corrigeer de overlayconfiguratie.</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>Er is een wisactie gepland bij de volgende start. Annuleer het wisverzoek voordat u de persistente overlay uitschakelt.</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>De volgende sessie behoudt de persistente Disk-overlay niet. Behoud deze voordat u wissen aanvraagt.</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>Een persistente overlay is alleen beschikbaar voor Disk; de volgende overlay is RAM.</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>De modus voor alleen-lezenmedia vereist RAM; de volgende overlay is Disk.</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>De huidige status van de persistente overlay kon niet worden bevestigd. Lees de configuratie opnieuw.</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>Wordt na toepassen bij de volgende start van kracht.</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>Wissen gepland</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>Geen wissen gepland</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>Wist eenmaal bij de volgende start en bewaart daarna gegevens. De huidige sessie wordt niet onmiddellijk gewist.</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>Annuleert het geplande wissen bij de volgende start en behoudt de gegevens van de persistente overlay.</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>Persistente overlay opnieuw instellen · Eenmalig verzoek</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Bezig…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>Wacht totdat de bewerking is voltooid.</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · Volgende sessie</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>Terug naar instellingen</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>Voorwaarden voor wijzigen</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>Gewijzigd</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>Configuratieconflicten controleren</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>Configuratieconflict</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>Voorwaarden voor uitschakelen</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>UWF en de persistente Disk-overlay moeten in de huidige sessie actief zijn. Om gegevens te wissen, moet de persistente Disk-overlay ook voor de volgende sessie behouden blijven.</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>Voorwaarden voor inschakelen</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Functie</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>Functiebeschrijving</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>Meer informatie over %1</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>Los deze configuratieconflicten op voordat u het concept bewaart.</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>Terugdraaien</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>Wanneer van kracht</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>Deze instelling kan momenteel niet worden gewijzigd</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>UWF moet in de huidige sessie zijn uitgeschakeld en de overlay van de volgende sessie moet Disk gebruiken.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>UWF moet in de huidige sessie zijn uitgeschakeld en de overlay van de volgende sessie moet RAM gebruiken.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>UWF moet in de huidige sessie zijn uitgeschakeld. Annuleer een geplande wisactie voor persistente gegevens voordat u de persistente overlay uitschakelt.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>UWF moet in de huidige sessie zijn uitgeschakeld. Als UWF alleen voor de volgende sessie wordt uitgeschakeld, moet u nog steeds opnieuw opstarten voordat u kunt bewerken.</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>Gebruiksvoorwaarden</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Details bekijken</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>Alleen bekijken</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>Waarom %1 niet kan worden gewijzigd</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>UWF is ingeschakeld in de huidige sessie, dus deze overlay-instellingen kunnen niet worden gewijzigd. Draai de wijzigingen terug of schakel UWF uit en start opnieuw op.</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>Voorwaarden voor wijzigen bekijken</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>Er zijn geen configuratiewijzigingen geschreven. U kunt het opnieuw proberen.</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>Er waren geen configuratiewijzigingen nodig.</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>Gebruik “Domeingeheimsleutel behouden” voor het geheim van het domeincomputeraccount ($MACHINE.ACC).</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>Deze sleutel zelf kan niet worden uitgesloten. Selecteer een van de subsleutels.</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>Registeruitsluitingen moeten een niet-lege subsleutel onder HKEY_LOCAL_MACHINE (HKLM) opgeven.</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>Dit script bevat alleen instellingen die met uwfmgr-opdrachten kunnen worden weergegeven. Niet opgenomen:</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>Volumekoppeling</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>Instellingen zonder betrouwbare adressering via een stationsletter</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>Opdrachtscript</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>De configuratie komt overeen met het doel voor de volgende sessie. Bekijk of exporteer de ondersteunde instellingen onder “Opdrachtscript”.</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>Instellingen met besturingstekens in paden of namen</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>Opdrachten die afhankelijk zijn van weggelaten instellingen</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>De UWF-uitsluitingen verhinderen dat dit doel betrouwbaar wordt vastgelegd. Verwijder de conflicterende uitsluiting en start opnieuw voordat u het vastlegt.</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>De UWF-uitsluitingen verhinderen dat de lijst met klaargezette bestanden betrouwbaar wordt vastgelegd. Verwijder de conflicterende uitsluiting en start opnieuw voordat u deze wijziging toepast.</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF kan niet garanderen dat de bewerking precies deze registersleutel als doel behoudt.</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>Bevestig dat de opgeslagen lijst met klaargezette bestanden leeg is voordat u de registeruitsluiting verwijdert.</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>Wacht op bevestiging</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Leeg</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>Het systeemvolume wordt momenteel niet door UWF beschermd; er zijn geen registerwijzigingen om vast te leggen.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>Registerwijzigingen zijn al permanent opgeslagen; vastleggen via UWF is niet nodig.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>Registerwijzigingen zijn al permanent opgeslagen; UWF kan deze verwijdering niet uitvoeren.</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>UWF-uitsluitingspaden moeten backslashes gebruiken.</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>Bestandspaden mogen geen null-tekens bevatten.</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>Schakel volumebeveiliging in dit concept in voordat u bestandsuitsluitingen toevoegt.</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>Het uitsluitingspad bevat tekens die UWF niet ondersteunt.</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>Het uitsluitingspad overschrijdt de maximale lengte van UWF.</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>Bestands- en registerwijzigingen kunnen niet worden vastgelegd terwijl HORM actief is.</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>Schrijven naar het register is niet mogelijk terwijl HORM actief is.</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>Schrijfbewerkingen naar het register kunnen niet permanent worden opgeslagen terwijl de modus voor alleen-lezenmedia actief is.</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>Bestands- en registerwijzigingen kunnen niet worden vastgelegd terwijl de modus voor alleen-lezenmedia actief is.</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>Onderschept schrijfbewerkingen naar fysieke media, inclusief metagegevens. Uitsluitingen omzeilen de beveiliging niet. Afzonderlijke bestands- en registerwijzigingen kunnen niet worden vastgelegd; Full Commit is een aparte globale bewerking.</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>UWF-instellingen kunnen niet worden gewijzigd terwijl HORM actief is.</translation></message>
    <message><source>Default value · this value only</source><translation>Standaardwaarde · alleen deze waarde</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>Bestandswijzigingen zijn al permanent opgeslagen; een UWF-commit is niet vereist.</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>Laatste geslaagde waarneming van overlaygebruik: %1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>Waargenomen overlayopslag valt buiten het beschrijfbare grootte- of typebereik</translation></message>
    <message><source>Overlay (last observed)</source><translation>Overlay (laatst waargenomen)</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>Vernieuwen van overlaygebruik mislukt.
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>Vernieuwen van overlaygebruik mislukt. De laatste geslaagde waarneming van %1 wordt getoond.
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>Het pad kon niet aan een lokaal volume worden gekoppeld.</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF: ingeschakeld (laatst waargenomen)</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>Bestandswijzigingen zijn al permanent opgeslagen; UWF kan deze verwijdering niet uitvoeren.</translation></message>
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>%1 GB beschikbaar</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>Beheerdersrechten zijn vereist.</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>Er wordt een bewerking uitgevoerd. Even geduld.</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>Probeert alle toevoegingen, wijzigingen en verwijderingen op elk beveiligd volume sinds deze opstart op te slaan. Dit omvat bestanden en systeeminstellingen; u kunt geen afzonderlijke wijzigingen selecteren.</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>Automatisch opslaan is niet beschikbaar in de huidige modus</translation></message>
    <message><source>Cannot be undone once started</source><translation>Kan na het starten niet ongedaan worden gemaakt</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>Kies waar de Disk-overlay wordt opgeslagen</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>Kies waar de Disk-overlay wordt opgeslagen.</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>Configuratiewijzigingen zijn tijdelijk niet beschikbaar.</translation></message>
    <message><source>Configured, inactive this session</source><translation>Geconfigureerd, niet actief in deze sessie</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>Geconfigureerd; wordt actief na opnieuw opstarten</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>Geconfigureerd; wordt actief na opnieuw opstarten. De overlay van deze opstart wordt niet naar de nieuwe locatie verplaatst.</translation></message>
    <message><source>Configure…</source><translation>Configureren…</translation></message>
    <message><source>Current UWF</source><translation>UWF in de huidige sessie</translation></message>
    <message><source>Currently configured storage volume</source><translation>Huidig geconfigureerd opslagvolume</translation></message>
    <message><source>Default behavior</source><translation>Standaardgedrag</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>Schakel HORM uit en start opnieuw op voordat u het volledige volume vastlegt.</translation></message>
    <message><source>Disabled · restart required</source><translation>Uitgeschakeld · opnieuw opstarten vereist</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>Het uitschakelen van HORM schakelt UWF of de modus voor alleen-lezenmedia niet uit en slaat geen wijzigingen op beveiligde schijven op. Start opnieuw op voordat u de beschikbare acties controleert.</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>Schakel UWF in en start opnieuw op voordat u HORM inschakelt.</translation></message>

    <message><source>Full volume commit</source><translation>Volledig volume vastleggen</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>Modus voor het vastleggen van het volledige volume (actief tot opnieuw opstarten)</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>De modus voor het vastleggen van het volledige volume is al ingeschakeld. Start opnieuw op voordat u deze weer gebruikt.</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>Het vastleggen van het volledige volume is gestart. Start opnieuw op voordat u HORM inschakelt.</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>Het vastleggen van het volledige volume is alleen beschikbaar als de modus voor alleen-lezenmedia actief is.</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>De modus voor het vastleggen van het volledige volume blijft actief tot het opnieuw opstarten. Nieuwe wijzigingen worden rechtstreeks opgeslagen. Afzonderlijk vastleggen blijft niet beschikbaar.</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>Het vastleggen van het volledige volume is niet beschikbaar op dit systeem.</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM is actief. UWF-instellingen en afzonderlijk vastleggen zijn niet beschikbaar. Schakel HORM uit en start opnieuw op voordat u wijzigingen aanbrengt.</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM is uitgeschakeld. Start de computer opnieuw op voordat u UWF-instellingen wijzigt.</translation></message>
    <message><source>HORM is not available on this system.</source><translation>HORM is niet beschikbaar op dit systeem.</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM vereist de huidige RAM-configuratie voor alleen-lezenmedia.</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM vereist een actieve RAM-overlay in de huidige sessie.</translation></message>

    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>In de modus Disk kan de systeemimplementatie deze optie standaard inschakelen; controleer de huidige status in de instellingen.</translation></message>
    <message><source>Inactive this session</source><translation>Niet actief in deze sessie</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>Ook latere wijzigingen worden opgeslagen totdat u opnieuw opstart</translation></message>

    <message><source>Maintenance and recovery</source><translation>Onderhoud en herstel</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>Controleer voordat u doorgaat of dit de status is die u wilt behouden.</translation></message>

    <message><source>Next startup: %1</source><translation>Volgende opstart: %1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>Er is geen geschikt opslagvolume beschikbaar. Vernieuw de volumelijst.</translation></message>
    <message><source>Not enabled</source><translation>Niet ingeschakeld</translation></message>
    <message><source>Not selected</source><translation>Niet geselecteerd</translation></message>
    <message><source>Not supported</source><translation>Niet ondersteund</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>Na het starten van de bewerking worden nieuwe wijzigingen rechtstreeks naar de schijf geschreven totdat u de computer opnieuw opstart.</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>Er kan maar één opslagvolume tegelijk worden gebruikt. Een nieuw volume kiezen vervangt de vorige locatie en stelt de volgende overlay in op Disk.</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>Overlaycapaciteit (huidige instelling behouden)</translation></message>
    <message><source>Overlay configuration</source><translation>Overlayconfiguratie</translation></message>

    <message><source>Processing…</source><translation>Bezig met verwerken…</translation></message>
    <message><source>Read state again</source><translation>Status opnieuw lezen</translation></message>
    <message><source>Read the current UWF state first.</source><translation>Lees eerst de huidige UWF-status.</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>De modus voor alleen-lezenmedia ondersteunt geen afzonderlijk vastleggen of automatisch opslaan van klaargezette bestanden.</translation></message>
    <message><source>Reading state…</source><translation>Status lezen…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>Start opnieuw op en lees de huidige status voordat u HORM opnieuw inschakelt.</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>Bij elke opstart hetzelfde bureaublad en dezelfde actieve programma’s herstellen.</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>Bekijk de beschikbare vastlegbewerkingen en hun vereisten.</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>Wijzigingen op alle beveiligde volumes opslaan</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>Wijzigingen op alle beveiligde volumes opslaan. Controleer de gevolgen voordat u doorgaat.</translation></message>


    <message><source>State unknown</source><translation>Status onbekend</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>Opgeslagen wijzigingen kunnen niet meer ongedaan worden gemaakt door opnieuw op te starten.</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>Opgeslagen wijzigingen blijven behouden</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>De HORM-status is onbekend. Lees de status opnieuw voordat u doorgaat.</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>De HORM-status is onbekend. Vernieuw de status voordat u doorgaat.</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>De vastlegstatus of HORM-status is onbekend. Vernieuw de status voordat u doorgaat.</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>De huidige UWF-status kon niet worden gelezen. Vernieuw de status voordat u doorgaat.</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>De huidige overlay gebruikt RAM in de modus voor alleen-lezenmedia</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>De huidige beveiligingsmodus kon niet worden gelezen. Vernieuw de status.</translation></message>
    <message><source>The result could not be confirmed</source><translation>Het resultaat kon niet worden bevestigd</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>Deze uitsluiting is geconfigureerd, maar omzeilt de beveiliging in de huidige sessie niet.</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>Hierdoor kunnen ook direct schrijven via vrije ruimte en de persistente overlay veranderen. Controleer de geavanceerde instellingen voordat u opnieuw opstart.</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>UWF-beveiliging is actief in de huidige sessie</translation></message>
    <message><source>%1 (will be replaced)</source><translation>%1 (wordt vervangen)</translation></message>
    <message><source>Add full volume commit to the draft?</source><translation>Het vastleggen van het volledige volume aan het concept toevoegen?</translation></message>
    <message><source>Add to draft</source><translation>Aan concept toevoegen</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>Geldt voor alle beveiligde volumes; start pas wanneer u dit concept toepast.</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>Als u dit concept toepast, wordt het volledige volume vastgelegd. Opgeslagen wijzigingen kunnen niet ongedaan worden gemaakt door opnieuw op te starten. Nieuwe wijzigingen worden tot het opnieuw opstarten opgeslagen.</translation></message>
    <message><source>Configure storage location</source><translation>Opslaglocatie configureren</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>Het configureren van de opslaglocatie is niet beschikbaar op dit systeem.</translation></message>
    <message><source>Current: %1</source><translation>Huidig: %1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>Schakel HORM uit en start opnieuw op voordat u de opslaglocatie configureert.</translation></message>
    <message><source>Disable saved-state startup?</source><translation>Opstarten vanuit opgeslagen status uitschakelen?</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>Concept: %1 · toepassen vereist</translation></message>
    <message><source>Enable saved-state startup?</source><translation>Opstarten vanuit opgeslagen status inschakelen?</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>Volledig volume vastleggen (wacht op toepassen)</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>Het vastleggen van het volledige volume staat klaar. Het start pas nadat u dit bevestigt in ‘Controleren en toepassen’.</translation></message>
    <message><source>Hibernate once / resume many</source><translation>Eenmaal sluimeren / meermaals hervatten</translation></message>
    <message><source>Not started</source><translation>Niet gestart</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>Het vastleggen van het volledige volume uit de openstaande wijzigingen verwijderen.</translation></message>
    <message><source>Save draft</source><translation>Concept opslaan</translation></message>

    <message><source>Will start when applied</source><translation>Start bij toepassen</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>Een gekoppelde vaste schijf heeft de beveiligingscontrole niet doorstaan. Controleer de vermelde schijven voordat u HORM inschakelt.</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>Controleert vaste schijven die voor UWF in aanmerking komen en gekoppeld blijven. Niet-gekoppelde partities kunnen worden overgeslagen.</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>De vereisten voor HORM konden niet worden gecontroleerd. Lees de status opnieuw voordat u doorgaat.</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>Beveiligingscontrole van gekoppelde vaste schijven</translation></message>
    <message><source>Not using a Disk overlay</source><translation>Er wordt geen Disk-overlay gebruikt</translation></message>
    <message><source>Selected storage volume</source><translation>Geselecteerd opslagvolume</translation></message>
    <message><source>System drive default (%1)</source><translation>Standaard op het systeemstation (%1)</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>De beveiligingsstatus is tijdens deze opstart gewijzigd. Start opnieuw op voordat u HORM inschakelt.</translation></message>
    <message><source>Unable to read</source><translation>Kan niet worden gelezen</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>Kies de schijf voor het opslaan van tijdelijke wijzigingen. Wordt actief bij de volgende opstart.</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>Schakel UWF uit en start opnieuw op voordat u de opslaglocatie configureert.</translation></message>
    <message><source>Met</source><translation>Voldaan</translation></message>
    <message><source>Not met</source><translation>Niet voldaan</translation></message>
    <message><source>Partition</source><translation>Partitie</translation></message>


    <message><source>Protect all</source><translation>Alles beveiligen</translation></message>
    <message><source>Applied</source><translation>Toegepast</translation></message>
    <message><source>Not applied</source><translation>Niet toegepast</translation></message>
    <message><source>Could not confirm</source><translation>Kon niet worden bevestigd</translation></message>
    <message><source>Hibernation is not supported</source><translation>Slaapstand wordt niet ondersteund</translation></message>
    <message><source>Hibernation is not enabled</source><translation>Slaapstand is niet ingeschakeld</translation></message>
    <message><source>Fast startup only</source><translation>Alleen snel opstarten</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>Gezamenlijke volumebeveiliging is niet beschikbaar op dit systeem.</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>Schakel HORM uit en start opnieuw op voordat u de volumebeveiliging wijzigt.</translation></message>
    <message><source>Setup and operation</source><translation>Instellen en gebruiken</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. Configuratie voorbereiden</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>Stel terwijl UWF is uitgeschakeld de RAM-overlay, alleen-lezenmediamodus, volumebeveiliging en het inschakelen van UWF voor de volgende start in. Pas het concept toe. Zorg dat S4-slaapstand beschikbaar is met een volledig slaapstandbestand.</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>Start opnieuw op en controleer bij de huidige sessie of UWF is ingeschakeld, de RAM-overlay en alleen-lezenmediamodus actief zijn en alle gekoppelde vaste volumes die UWF kan beveiligen beschermd zijn. Controleer of de slaapstand beschikbaar is.</translation></message>
    <message><source>2. Restart and verify</source><translation>2. Opnieuw opstarten en controleren</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. HORM inschakelen en de slaapstand activeren</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>Voeg HORM inschakelen toe aan het concept en kies Controleren en toepassen. Daarmee wordt HORM ingeschakeld, gelijkwaardig aan:</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>Stel na het inschakelen het bureaublad en de toepassingen in en activeer eenmaal de slaapstand om de uitgangstoestand op te slaan:</translation></message>
    <message><source>HORM usage</source><translation>HORM gebruiken</translation></message>
    <message><source>Volume protection</source><translation>Volumebeveiliging</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>Stel de beveiliging van alle volumes voor de volgende sessie in.</translation></message>
    <message><source>Unprotect all</source><translation>Alle beveiliging uitschakelen</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>Volgende start: alle volumes beveiligd</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>Volgende start: geen volumes beveiligd</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>Volgende start: instellingen per volume</translation></message>
    <message><source>System hibernation is available</source><translation>Slaapstand is beschikbaar</translation></message>
    <message><source>Usage guide</source><translation>Gebruiksaanwijzing</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>Beveiliging van alle volumes wacht op toepassing</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>Volumebeveiliging wordt actief na opnieuw opstarten</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>Alle beveiliging uitschakelen en HORM inschakelen kunnen niet samen worden toegepast.</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>Gezamenlijke volumebeveiliging wordt niet ondersteund.</translation></message>



    <message><source>Storage volume:</source><translation>Opslagvolume:</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>Uitsluitingen omzeilen de beveiliging niet tijdens deze opstartsessie. Afzonderlijk opslaan en automatisch opslaan van klaargezette bestanden zijn niet beschikbaar. Het volledig opslaan van volumes heeft gevolgen voor alle beveiligde volumes; lees de beschrijving voordat u doorgaat.</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>Herhaaldelijk hervatten vanuit dezelfde sluimerstand</translation></message>
    <message><source>Remove full volume commit draft</source><translation>Concept voor het vastleggen van het volledige volume verwijderen</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>De modus voor alleen-lezenmedia is actief</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>Een ingestelde opslagmarkering kon niet worden gelezen.</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>De klaargezette bestanden conflicteren met een bestandsuitsluiting:
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>Volledige slaapstand is niet beschikbaar op deze computer</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>Meerdere opslagvolumes hebben een markering die niet nul is.</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>Het opslagtype Schijf van de overlay kon niet worden bevestigd.</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>De ingestelde opslaglocatie kon niet worden gelezen.</translation></message>
    <message><source>The system drive could not be identified.</source><translation>Het systeemstation kon niet worden geïdentificeerd.</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>Direct herstarten slaat het vastleggen van klaargezette bestanden over.</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>Direct afsluiten slaat het vastleggen van klaargezette bestanden over.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>Klaargezette bestanden worden niet vastgelegd. Niet-vastgelegde wijzigingen kunnen na herstarten verloren gaan.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>Klaargezette bestanden worden niet vastgelegd. Niet-vastgelegde wijzigingen kunnen na afsluiten verloren gaan.</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>Legt klaargezette bestanden vast vóór normaal afsluiten of herstarten.</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>Het vastleggen van het volledige volume en het inschakelen van HORM kunnen niet samen worden toegepast.</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>Het configureren van de opslaglocatie en het inschakelen van HORM kunnen niet samen worden toegepast.</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>Het configureren van de opslaglocatie en het vastleggen van het volledige volume vereisen verschillende huidige UWF-modi.</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>Kies een opslagvolume voordat u de wijzigingen toepast.</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>Het configureren van een opslaglocatie op schijf conflicteert met de gevraagde RAM-overlay.</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>Pas de filterwijziging toe en start opnieuw op voordat u het volledige volume laat vastleggen.</translation></message>
    <message><source>Canceled</source><translation>Geannuleerd</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>Het beoordeelde opslagvolume is niet meer beschikbaar. Ga terug naar bewerken en kies een ander volume.</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>Dit volume kan niet bij UWF worden geregistreerd omdat de opslagbus niet wordt ondersteund.</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>Ondersteuning voor het fysieke volume kon niet worden bevestigd. Vernieuw de status voordat u de beveiliging inschakelt.</translation></message>
    <message><source>Paused</source><translation>Gepauzeerd</translation></message>
    <message><source>Partially paused</source><translation>Gedeeltelijk gepauzeerd</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>De gegevens van de permanente overlay blijven bij de volgende start behouden. Automatisch vastleggen is gepauzeerd.</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>Gepauzeerde items: %1 · Gepauzeerde bestanden: %2</translation></message>
    <message><source>Read-only ROM</source><translation>Alleen-lezen-ROM</translation></message>
    <message><source>%1 changes · %2 commands</source><translation>%1 wijzigingen · %2 opdrachten</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>Bestandssysteem %1: bestandsuitsluitingen en doorvoeren per bestand worden niet ondersteund.</translation></message>
    <message><source>Applying changes</source><translation>Wijzigingen toepassen</translation></message>
    <message><source>Automatic commits paused</source><translation>Automatisch doorvoeren gepauzeerd</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>Na een wijziging van de HORM-modus wordt automatisch doorvoeren van bestanden gepauzeerd tot de herstart.</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>Automatisch doorvoeren van bestanden is niet beschikbaar na FullCommit.</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>Automatisch doorvoeren van bestanden is niet beschikbaar terwijl HORM actief is.</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>Automatisch doorvoeren van klaargezette bestanden wordt gepauzeerd voor deze herstart.</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>Automatisch doorvoeren van klaargezette bestanden wordt gepauzeerd voor dit afsluiten.</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>Het behoud van de configuratie kon niet worden bevestigd. Vernieuw de systeemstatus voordat u deze instelling wijzigt.</translation></message>
    <message><source>Elapsed: %1 s</source><translation>Verstreken: %1 s</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>Alleen geschikte klaargezette bestanden worden doorgevoerd.</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>Alleen-lezen ROM kan gewone UWF-instellingen op het beveiligde systeemvolume niet blijvend opslaan. Schakel UWF uit en start opnieuw voordat u ze wijzigt.</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>Overgeslagen klaargezette bestanden worden door deze bewerking niet doorgevoerd.</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>Klaargezette bestanden staan al op onbeveiligde opslag. Automatisch doorvoeren is niet nodig.</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>Drempelwijzigingen vereisen het doorvoeren van het register, wat niet beschikbaar is in alleen-lezen ROM.</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>Wachten tot Windows klaar is. Dit kan even duren.</translation></message>
    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM is uitgeschakeld. Automatisch doorvoeren van klaargezette bestanden is gepauzeerd tot de herstart. Handmatige bewerkingen volgen de huidige UWF-mogelijkheden.</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>Deze configuratiemomentopname bevat instellingen waarvoor UWF in de huidige sessie uitgeschakeld moet zijn. Schakel UWF uit en start opnieuw op voordat u deze uitvoert.</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>Acties voor de huidige sessie (HORM, FullCommit en het opnieuw instellen van de persistente overlay; exporteer ze via Openstaande wijzigingen)</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>Het in bulk opheffen van de beveiliging vereist bevestiging van het volledige volumebereik voor de volgende sessie.</translation></message>
</context>
</TS>
