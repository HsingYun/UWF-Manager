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
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Dit volume beveiligen in de volgende sessie. Schrijfacties gaan naar de overlay en worden bij herstarten gewist.</translation></message>
    <message><source>Current session</source><translation>Huidige sessie</translation></message>
    <message><source>Next session</source><translation>Volgende sessie</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Volumeherkenning: een stationsletter is eenvoudiger, maar de koppeling vervalt bij een andere letter, bijvoorbeeld na schijfwijzigingen. Het volume-ID blijft gelijk.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Dit volume wordt niet ondersteund door UWF: </translation></message>
    <message><source>Global settings</source><translation>Algemene instellingen</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>UWF-filterstatus in de huidige sessie (alleen-lezen).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>UWF inschakelen in de volgende sessie. Schrijfacties naar beveiligde volumes gaan naar de overlay en worden bij herstarten gewist.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Type en maximale grootte wijzigen:
1. Schakel het filter hierboven uit.
2. Herstart om het filter uit te schakelen.
3. Wijzig de instelling.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Overlay-opslag: RAM is sneller maar gebruikt geheugen; Disk gebruikt de systeemschijf en biedt meer ruimte. Beide worden bij herstarten gewist.</translation></message>
    <message><source>Type</source><translation>Type</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Maximale overlaycapaciteit. RAM is begrensd door het totale geheugen. Disk vereist minstens 1024 MB en voldoende vrije ruimte op het systeemvolume.</translation></message>
    <message><source>Maximum size</source><translation>Maximale grootte</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Maximale grootte · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Waarschuwingsdrempel</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Schrijft een waarschuwing naar het gebeurtenislogboek wanneer het overlaygebruik deze waarde bereikt. Als beide gebeurtenissen zijn ingeschakeld, moet deze waarde lager zijn dan de kritieke drempel. Met 0 wordt alleen de waarschuwingsgebeurtenis uitgeschakeld.</translation></message>
    <message><source>Critical threshold</source><translation>Kritieke drempel</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Schrijft een kritieke melding naar het gebeurtenislogboek wanneer het overlaygebruik deze waarde bereikt. Als beide gebeurtenissen zijn ingeschakeld, moet deze waarde hoger zijn dan de waarschuwingsdrempel. Met 0 wordt alleen de kritieke gebeurtenis uitgeschakeld.</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Niet-ondersteund stationstype (alleen lokale vaste schijven).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Het volume overschrijdt de UWF-limiet van 16 TiB per beveiligd volume.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: volumebeveiliging mogelijk, maar geen bestandsuitsluitingen of vastlegging per bestand.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Mislukt: Volumegegevens lezen.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 vrij / %2</translation></message>
    <message><source>Commit</source><translation>Vastleggen</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Overlaywijzigingen van dit bestand vastleggen op schijf / Registerwijzigingen vastleggen. Deze handeling kan niet ongedaan worden gemaakt.</translation></message>
    <message><source>Commit file changes…</source><translation>Bestandswijzigingen vastleggen…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Bestand kiezen om vast te leggen op schijf. Overlaywijzigingen van dit bestand vastleggen op schijf.</translation></message>
    <message><source>Commit folder changes…</source><translation>Mapwijzigingen vastleggen…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Map kiezen om vast te leggen op schijf. Overlaywijzigingen van deze map vastleggen op schijf. Inclusief alle submappen.</translation></message>
    <message><source>Delete and commit file…</source><translation>Bestand verwijderen en verwijdering vastleggen op schijf…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Bestand kiezen voor blijvende verwijdering. Bestand verwijderen en verwijdering vastleggen op schijf.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Map en inhoud verwijderen en verwijderingen vastleggen op schijf…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Map kiezen voor blijvende verwijdering. Map en inhoud verwijderen en verwijderingen vastleggen op schijf. Inclusief alle submappen.</translation></message>
    <message><source>Commit registry changes…</source><translation>Registerwijzigingen vastleggen…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Voer een registersleutel en eventueel een waardenaam in. Zonder waardenaam worden alle waarden van de sleutel en subsleutels vastgelegd.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Registerverwijdering vastleggen…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Voer de te verwijderen sleutel en eventueel een waardenaam in. Zonder naam worden de sleutel, alle waarden en subsleutels verwijderd en de verwijderingen vastgelegd.</translation></message>
    <message><source>File exclusions</source><translation>Bestandsuitsluitingen</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Bestanden en mappen op dit volume zonder UWF-beveiliging. Dubbelklik op een vermelding om het pad te kopiëren.</translation></message>
    <message><source>Registry exclusions</source><translation>Registeruitsluitingen</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Registeruitsluitingen zijn algemeen en volgen het UWF-filter, niet de volumebeveiliging. De gedeelde lijst verschijnt maar op één volumetab. Dubbelklik om een pad te kopiëren.</translation></message>
    <message><source>File staging</source><translation>Automatisch vastleggen</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>bestanden / mappen: Automatisch vastleggen vóór veilig afsluiten of herstarten. Inclusief alle submappen. Zolang de lijst vermeldingen bevat, bewaart deze registeruitsluiting de lijst tussen beveiligde sessies: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Bestanden of een map toevoegen aan automatisch vastleggen.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Bestand kiezen om vast te leggen op schijf. Automatisch vastleggen vóór veilig afsluiten of herstarten.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Map kiezen om vast te leggen op schijf. Automatisch vastleggen vóór veilig afsluiten of herstarten. Inclusief alle submappen.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Bestand kiezen om vast te leggen op schijf · Automatisch vastleggen</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Map kiezen om vast te leggen op schijf · Automatisch vastleggen · Inclusief alle submappen.</translation></message>
    <message><source>Cannot stage this path</source><translation>Pad kan niet worden toegevoegd aan automatisch vastleggen</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Dit pad overlapt met een UWF-uitsluiting. Verwijder eerst de uitsluiting:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Dit pad overlapt met automatisch vastleggen. Verwijder eerst die vermelding:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Pad overlapt met een bestaande vermelding voor automatisch vastleggen</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Deze registeruitsluiting is vereist zolang de lijst voor automatisch vastleggen vermeldingen bevat</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Het pad bestaat niet of heeft niet het verwachte type:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Het pad kan niet worden onderzocht:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Reparsepunten kunnen niet automatisch worden vastgelegd:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Zonder stationsletter kan UWF dit pad niet vastleggen:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Het pad staat op volume %1. Voeg het toe via het tabblad voor automatisch vastleggen van dat volume.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>De hoofdmap van een volume kan niet worden toegevoegd: dat zou het hele volume recursief vastleggen.</translation></message>
    <message><source>File staging could not be saved</source><translation>Mislukt: Lijst automatisch vastleggen opslaan</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>De lijst is opnieuw geladen omdat bijwerken niet is gelukt:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Automatisch vastleggen: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Automatisch vastleggen vóór veilig afsluiten of herstarten.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Automatisch vastleggen: %1 · bestanden: %2 · mappen: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Automatisch vastleggen van bestanden vereist deze registeruitsluiting om de lijst na opnieuw opstarten te behouden.</translation></message>
    <message><source>Built-in</source><translation>Ingebouwd</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Het UWF-filter is uitgeschakeld. Er zijn geen overlaywijzigingen verzameld om vast te leggen.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Het bestandssysteem %1 ondersteunt geen vastlegging per bestand.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Dit volume wordt niet ondersteund door UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Dit volume ondersteunt geen bestandsvastlegging. Alleen registervastlegging beschikbaar (registeruitsluitingen zijn algemeen).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Het bestandssysteem %1 ondersteunt geen vastlegging per bestand. Alleen registervastlegging beschikbaar (registeruitsluitingen zijn algemeen).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Dit volume is momenteel niet UWF-beveiligd; er zijn geen bestandswijzigingen om vast te leggen.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Dit volume is momenteel niet UWF-beveiligd; er zijn geen bestandswijzigingen om vast te leggen. Alleen registervastlegging beschikbaar (registeruitsluitingen zijn algemeen).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Bestand kiezen om vast te leggen op schijf</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Map kiezen om vast te leggen op schijf</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Bestand kiezen voor blijvende verwijdering</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Map kiezen voor blijvende verwijdering</translation></message>
    <message><source>Name</source><translation>Naam</translation></message>
    <message><source>Data</source><translation>Gegevens</translation></message>
    <message><source>(Default)</source><translation>(Standaard)</translation></message>
    <message><source>(value not set)</source><translation>(waarde niet ingesteld)</translation></message>
    <message><source>(empty)</source><translation>(leeg)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Onbekend (Waarde: Bestand te groot)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Overlaybestanden bekijken. &lt;b&gt;Alleen een diagnosemomentopname.&lt;/b&gt; Gebruik deze lijst niet om te bepalen wat u vastlegt.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Overlaybestanden kunnen alleen op NTFS-volumes worden weergegeven.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF is uitgeschakeld; geen enkel volume heeft een overlay om te bekijken.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Dit volume is niet UWF-beveiligd en heeft geen overlay om te bekijken.</translation></message>
    <message><source>Failed: %1</source><translation>Mislukt: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>De WMI-provider is vastgelopen tijdens het opsommen. Dit is een bekende instabiliteit van UWF_Overlay.GetOverlayFiles bij grote overlays of hoge I/O-belasting.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Onvoldoende geheugen of niet-ondersteunde bewerking. NTFS en vrij geheugen zijn vereist. Probeer een kleinere overlay of sluit geheugenintensieve toepassingen.</translation></message>
    <message><source>Raw error: %1</source><translation>Foutcode: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Exporteren naar bestand… · Overlay · bestanden</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Tekstbestanden (*.txt);;Alle bestanden (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Opdrachtbestanden (*.cmd);;Alle bestanden (*)</translation></message>
    <message><source>Export failed</source><translation>Exporteren mislukt</translation></message>
    <message><source>Export finished</source><translation>Exporteren voltooid</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Mislukt: Bestand openen (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Mislukt: Bestand schrijven (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Configuratie: Bestand schrijven · vermeldingen: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Een heel volume (%1) kan niet worden uitgesloten.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Wissel-, swap- en slaapstandbestanden kunnen niet worden uitgesloten; UWF is ervan afhankelijk.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>De hele map kan niet worden uitgesloten: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>De hele map kan niet worden uitgesloten: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>De hele map kan niet worden uitgesloten: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Dit essentiële systeembestand kan niet worden uitgesloten: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Het gebruikersregisterbestand NTUSER.DAT kan niet worden uitgesloten.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>De registersleutel mag niet leeg zijn.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Een registersleutel mag niet beginnen met een backslash.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Het pad bevat opeenvolgende backslashes en is ongeldig.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Registerpaden gebruiken `\` als scheidingsteken, geen `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Het pad bevat onzichtbare besturingstekens en is ongeldig.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>Volgens UWF-documentatie mag het geheim van het domeincomputeraccount niet worden uitgesloten: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Alleen specifieke subsleutels van deze hoofdsleutels kunnen worden uitgesloten:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Bestanden of mappen aan uitsluitingen toevoegen. UWF beveiligt uitgesloten items niet.</translation></message>
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
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Alle openstaande wijzigingen controleren en in één keer toepassen. De meeste wijzigingen worden na de volgende herstart van kracht.</translation></message>
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
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Doel niet gevonden; er is niets in de overlay om vast te leggen.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Het doel bestaat alleen in de overlay: gemaakt onder UWF, nooit vastgelegd op schijf of in het permanente register. Het verdwijnt bij herstarten; verwijderen vastleggen is niet nodig en niet mogelijk.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Het systeem heeft een parameter geweigerd (ongeldig pad of argument).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Bewerking mislukt. Een ander proces gebruikt het doel mogelijk, bijvoorbeeld Verkenner of een geopend bestand. Sluit de betrokken programma’s en probeer opnieuw.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Bewerking mislukt; zie het logboek voor details.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Bewerking geweigerd (code %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Onbekend.</translation></message>
    <message><source>Commit canceled</source><translation>Vastleggen geannuleerd</translation></message>

    <message><source>Succeeded</source><translation>Geslaagd</translation></message>
    <message><source>Skipped</source><translation>Overgeslagen</translation></message>
    <message><source>Failed</source><translation>Mislukt</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Geslaagd: %1; Overgeslagen: %2; Mislukt: %3.</translation></message>

    <message><source>Existed before</source><translation>Bestond vooraf</translation></message>
    <message><source>Exists after</source><translation>Bestaat achteraf</translation></message>
    <message><source>Error code</source><translation>Foutcode</translation></message>
    <message><source>Reason</source><translation>Reden</translation></message>
    <message><source>Previous page</source><translation>Vorige pagina</translation></message>
    <message><source>First page</source><translation>Eerste pagina</translation></message>
    <message><source>Last page</source><translation>Laatste pagina</translation></message>
    <message><source>Next page</source><translation>Volgende pagina</translation></message>
    <message><source>Copy selected rows</source><translation>Geselecteerde rijen kopiëren</translation></message>
    <message><source>Copy all</source><translation>Alles kopiëren</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Deze registeruitsluiting is vereist zolang de lijst voor automatisch vastleggen vermeldingen bevat. Automatisch vastleggen: Wissen → Registeruitsluiting verwijderen.</translation></message>
    <message><source>Copy current page</source><translation>Huidige pagina kopiëren</translation></message>
    <message><source>No pending changes</source><translation>Geen openstaande wijzigingen</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Registeruitsluitingen · Algemene instellingen.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: UWF-beveiliging / Bestandsuitsluitingen / Automatisch vastleggen.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Niet ondersteund</translation></message>
    <message><source>Review and apply changes</source><translation>Wijzigingen controleren en toepassen</translation></message>
    <message><source>Confirm apply</source><translation>Toepassen bevestigen</translation></message>
    <message><source>Result</source><translation>Resultaat</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filter: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Waarschuwingsdrempel instellen: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Kritieke drempel instellen: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Overlaytype instellen / Maximale grootte instellen: Type en maximale grootte wijzigen:
1. Schakel het filter hierboven uit.
2. Herstart om het filter uit te schakelen.
3. Wijzig de instelling.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Overlaytype instellen: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Maximale grootte instellen: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Maximale grootte instellen: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · UWF-beveiliging: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Koppelen op: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1 · Bestandsuitsluiting toevoegen: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1 · Bestandsuitsluiting verwijderen: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Registeruitsluiting toevoegen: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Registeruitsluiting verwijderen: %1</translation></message>
    <message><source>Time</source><translation>Tijd</translation></message>
    <message><source>Level</source><translation>Niveau</translation></message>
    <message><source>Tag</source><translation>Tag</translation></message>
    <message><source>Message</source><translation>Bericht</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Bestanden voor automatisch vastleggen bepalen…</translation></message>
    <message><source>Committing staged files</source><translation>Bestanden automatisch vastleggen</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 van %2 bestanden verwerkt.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Geen aanvullende foutdetails beschikbaar.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Automatisch vastleggen is onverwacht gestopt:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Automatisch vastleggen niet voltooid: Onbekende fout.</translation></message>
    <message><source>File staging will be skipped</source><translation>Automatisch vastleggen wordt overgeslagen</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Direct herstarten: Automatisch vastleggen wordt overgeslagen.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Direct afsluiten: Automatisch vastleggen wordt overgeslagen.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Automatisch vastleggen overgeslagen vanwege een directe energieactie.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Veilig afsluiten: Mislukt</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Afsluiten: Mislukt (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Veilig herstarten: Mislukt</translation></message>
    <message><source>Direct restart failed</source><translation>Direct herstarten: Mislukt</translation></message>
    <message><source>Direct shutdown failed</source><translation>Direct afsluiten: Mislukt</translation></message>
    <message><source>Restart failed: %1</source><translation>Herstarten: Mislukt (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>De lijst voor automatisch vastleggen kon niet worden gelezen:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>De lijst voor automatisch vastleggen kon niet worden gelezen: Onbekende fout.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>bestanden: %1 · Geslaagd: %2 · Overgeslagen (bestanden): %3 · Overgeslagen (vermeldingen): %4 · Mislukt: %5</translation></message>
    <message><source>File staging preparation</source><translation>Automatisch vastleggen: Bewerkingsdetails</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Reparsepunten worden niet ondersteund.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Alleen absolute paden op lokale volumes kunnen worden toegevoegd.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Alleen absolute paden op lokale volumes kunnen worden toegevoegd.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Het automatisch vast te leggen pad kon niet worden onderzocht.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Het voorbereide bestandspad verwijst nu naar een ander type.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Het voorbereide mappad verwijst nu naar een ander type.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>De voorbereide map kon niet volledig worden opgesomd.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Het pad heeft geen stationsletter.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>De hoofdmap van een volume mag niet recursief worden verwerkt.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>De UWF-provider heeft het vastleggen geweigerd.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Vastleggen mislukt: Onbekende fout.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>%1 extra fouten zijn in het logboek vastgelegd.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Het pad heeft geen stationsletter. Doelvolume bepalen: Mislukt.</translation></message>

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
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Typ of plak één opdracht per regel. Laad tekst, logboeken of scripts (.txt, .bat, .ps1); regels met deze opdracht worden toegevoegd. Importeren zet wijzigingen klaar. Pas ‘Controleren en toepassen’ op de werkbalk schrijft ze naar het systeem.</translation></message>
    <message><source>Load from file…</source><translation>Uit bestand laden…</translation></message>
    <message><source>Load recommended configuration</source><translation>Aanbevolen configuratie laden</translation></message>
    <message><source>Choose recommended configuration</source><translation>Aanbevolen configuratie kiezen</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Kies de toe te voegen configuratiegroepen. Controleer of verwijder afzonderlijke opdrachten vóór het importeren.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Uit de officiële Microsoft-documentatie.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Door Microsoft aanbevolen UWF-uitsluitingen</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Controleer vóór importeren: mappen moeten bestaan voordat UWF uitsluitingen accepteert.</translation></message>
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
    <message><source>Internal error: no applier registered.</source><translation>Interne fout: geen toepashandler geregistreerd.</translation></message>
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
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Er zijn %1 niet-toegepaste wijzigingen.

Verwerpen en doorgaan?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Onvolledige uwfmgr-opdracht</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Ontbrekend argument: Grootte (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>De grootte moet een niet-negatief geheel getal in MB zijn</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Ontbrekend argument: Overlay · Type (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Ongeldige waarde: Overlay · Type: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Ontbrekend argument: Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Het volume moet een stationsletter zijn, zoals C:</translation></message>
    <message><source>Missing path argument</source><translation>Ontbrekend argument: Pad</translation></message>
    <message><source>Missing registry key argument</source><translation>Ontbrekend argument: Register · Sleutel</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Niet-gesloten dubbele aanhalingstekens in opdracht</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Onverwacht extra argument: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Niet-ondersteunde uwfmgr-opdracht (geen bijbehorende interfaceactie)</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Doelstatus is al bereikt; geen actie nodig</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Pad staat niet op dit volume of dit volume ondersteunt geen uitsluitingen (bijv. exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Geweigerd door UWF-blokkeerlijst (systeembestand / Windows / wisselbestand enz.)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Ongeldige waarde: Grootte: %1</translation></message>
    <message><source>maximum size</source><translation>Maximale grootte</translation></message>
    <message><source>warning threshold</source><translation>Waarschuwingsdrempel</translation></message>
    <message><source>critical threshold</source><translation>Kritieke drempel</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>De algemene configuratie is niet beschikbaar; de opdracht is niet toegevoegd aan de openstaande wijzigingen.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Onbekend volume %1 (geen UWF-geschikte schijf met deze letter)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Doelstatus is al bereikt; geen actie nodig</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Het pad heeft geen stationsletter. Doelvolume bepalen: Mislukt.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Niet ondersteund</translation></message>
    <message><source>file exclusion</source><translation>Bestandsuitsluitingen</translation></message>
    <message><source>registry exclusion</source><translation>Registeruitsluitingen</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Registeruitsluitingen zijn alleen beschikbaar op het ontbrekende registertabblad</translation></message>
    <message><source>Unsupported command</source><translation>Opdracht: Niet ondersteund</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Tonen/verbergen.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Ingeschakeld</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Uitgeschakeld</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Onbekend</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Gebruikt %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Uitgebreide modus</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Automatisch vastleggen afstemmen op afsluiten en herstarten van Windows.</translation></message>
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
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>De service heeft geen Windows-vooruitschakelmeldingen geaccepteerd.</translation></message>
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
    <message><source>File staging completed</source><translation>Automatisch vastleggen: Geslaagd</translation></message>
    <message><source>File staging failed</source><translation>Automatisch vastleggen: Mislukt</translation></message>
    <message><source>All staged targets were processed.</source><translation>Alle voorbereide doelen zijn verwerkt.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Alle doelen zijn verwerkt; één of meer bewerkingen zijn mislukt.</translation></message>
    <message><source>Show main window</source><translation>Hoofdvenster tonen</translation></message>
    <message><source>Restore default position</source><translation>Standaardpositie herstellen</translation></message>
    <message><source>Show overlay hub</source><translation>Overlay-HUB tonen</translation></message>
    <message><source>Hide overlay hub</source><translation>Overlay-HUB verbergen</translation></message>
    <message><source>Exit application</source><translation>Toepassing afsluiten</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Mislukt: Verbinding met het systeem</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Mislukt: Registerwaarden lezen (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Overlaybestanden opsommen: Onbekende fout.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Mislukt: Filteractie: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Mislukt: Filterstatus lezen</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Mislukt: Waarschuwingsdrempel instellen</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Mislukt: Kritieke drempel instellen</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Mislukt: Overlaystatus lezen</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Mislukt: Overlaytype instellen</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Mislukt: Maximale grootte instellen</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Mislukt: Overlayconfiguratie lezen</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Mislukt: Volume %1 bij UWF registreren</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Mislukt: Beveiliging volume %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Mislukt: Koppeling voor volume %1 instellen</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Mislukt: Bestandsuitsluiting toevoegen %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Bestandsuitsluiting op volume %1 was al afwezig: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Mislukt: Bestandsuitsluiting verwijderen %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Mislukt: Volumeconfiguratie lezen</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Mislukt: Registeruitsluiting toevoegen %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Mislukt: Registeruitsluiting verwijderen %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Mislukt: Registerpersistentie wijzigen</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Mislukt: Registerfilter lezen</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Niet ondersteund</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Doelstatus lezen: Onbekende fout.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Het pad bestaat niet; er is niets te verwijderen.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Onbekende fout.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Resultaat kan niet worden bevestigd: opnieuw lezen van de doelstatus mislukte: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>De provider heeft de verwijdering geaccepteerd, maar de gezaghebbende hercontrole toont dat het doel nog bestaat.</translation></message>
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
    <message><source>About this snapshot</source><translation>Over deze momentopname</translation></message>
    <message><source>Account</source><translation>Account</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Voeg UWF-configuratie toe vanuit een script of tekst.</translation></message>
    <message><source>Add exclusion</source><translation>Uitsluiting toevoegen</translation></message>
    <message><source>Add files or folders</source><translation>Bestanden of mappen toevoegen</translation></message>
    <message><source>Add selected items</source><translation>Geselecteerde items toevoegen</translation></message>
    <message><source>Added</source><translation>Toegevoegd</translation></message>
    <message><source>Administrator</source><translation>Beheerder</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Alle wijzigingen zijn blijvend op schijf opgeslagen. Opnieuw opstarten is niet nodig.</translation></message>
    <message><source>All files (*)</source><translation>Alle bestanden (*)</translation></message>
    <message><source>All levels</source><translation>Alle niveaus</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Een lege diagnostische momentopname betekent niet dat de overlay geen wijzigingen bevat.</translation></message>
    <message><source>Application path</source><translation>Toepassingspad</translation></message>
    <message><source>Application result</source><translation>Resultaat van toepassen</translation></message>
    <message><source>Apply changes</source><translation>Wijzigingen toepassen</translation></message>
    <message><source>Architecture</source><translation>Architectuur</translation></message>
    <message><source>Automatic</source><translation>Automatisch</translation></message>
    <message><source>Automatic file staging</source><translation>Klaargezette bestanden automatisch opslaan</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Automatisch opslaan onvolledig</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Sla geschikte klaargezette bestanden automatisch op bij normaal afsluiten en herstarten.</translation></message>
    <message><source>Available</source><translation>Beschikbaar</translation></message>
    <message><source>Back</source><translation>Terug</translation></message>
    <message><source>Background service</source><translation>Achtergrondservice</translation></message>
    <message><source>Binding</source><translation>Koppeling</translation></message>
    <message><source>By drive letter</source><translation>Op stationsletter</translation></message>
    <message><source>By volume ID</source><translation>Op volume-ID</translation></message>
    <message><source>Cancel reading</source><translation>Lezen annuleren</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Reeds vastgelegde wijzigingen blijven behouden.</translation></message>
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
    <message><source>Commit complete</source><translation>Vastleggen voltooid</translation></message>
    <message><source>Commit file changes?</source><translation>Bestandswijzigingen vastleggen?</translation></message>
    <message><source>Commit folder changes?</source><translation>Mapwijzigingen vastleggen?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Wijzigingen aan registersleutel vastleggen?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Wijzigingen aan registerwaarde vastleggen?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Legt klaargezette bestanden vast vóór normaal afsluiten en herstarten</translation></message>
    <message><source>Compatibility mode</source><translation>Compatibiliteitsmodus</translation></message>
    <message><source>Configuration applied</source><translation>Configuratie toegepast</translation></message>
    <message><source>Configuration error</source><translation>Configuratiefout</translation></message>
    <message><source>Configured</source><translation>Geconfigureerd</translation></message>
    <message><source>Confirm before continuing</source><translation>Bevestig voordat u doorgaat</translation></message>
    <message><source>Connected and authenticated</source><translation>Verbonden en geverifieerd</translation></message>
    <message><source>Connecting…</source><translation>Verbinden…</translation></message>
    <message><source>Contact author</source><translation>Contact met auteur</translation></message>
    <message><source>Continue editing</source><translation>Verder bewerken</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Doorgaan met herstarten verwijdert wijzigingen in bestanden die niet konden worden opgeslagen.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Doorgaan met afsluiten verwijdert wijzigingen in bestanden die niet konden worden opgeslagen.</translation></message>
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
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>De uitgebreide modus is niet beschikbaar. Gebruik Veilig afsluiten of Veilig herstarten om klaargezette bestanden op te slaan.</translation></message>
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
    <message><source>File saving has started and cannot be canceled.</source><translation>Het opslaan van bestanden is begonnen en kan niet worden geannuleerd.</translation></message>
    <message><source>File saving incomplete</source><translation>Bestanden onvolledig opgeslagen</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Het opslaan van klaargezette bestanden wordt overgeslagen. Niet-vastgelegde wijzigingen op beveiligde paden gaan na herstarten verloren.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Het opslaan van klaargezette bestanden wordt overgeslagen. Niet-vastgelegde wijzigingen op beveiligde paden gaan na afsluiten verloren.</translation></message>
    <message><source>File type</source><translation>Bestandstype</translation></message>
    <message><source>Files and folders</source><translation>Bestanden en mappen</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Bestanden en mappen kunnen samen worden geselecteerd.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Klaargezette bestanden worden niet opgeslagen.</translation></message>
    <message><source>Filter is disabled</source><translation>Filter is uitgeschakeld</translation></message>
    <message><source>Fit columns to contents</source><translation>Kolommen aanpassen aan inhoud</translation></message>
    <message><source>Folder address</source><translation>Mapadres</translation></message>
    <message><source>Folder name</source><translation>Mapnaam</translation></message>
    <message><source>Folders</source><translation>Mappen</translation></message>
    <message><source>Full configuration</source><translation>Volledige configuratie</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Toekomstige schrijfbewerkingen naar deze sleutel worden niet door UWF beveiligd.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Toekomstige schrijfbewerkingen naar dit pad worden niet door UWF beveiligd.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Globale registeruitsluiting · inclusief subsleutels</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Ga één map omhoog of kies een andere locatie.</translation></message>
    <message><source>Home</source><translation>Persoonlijke map</translation></message>
    <message><source>Import configuration</source><translation>Configuratie importeren</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>Importeren werkt het configuratieconcept bij. Gebruik Controleren en toepassen om wijzigingen naar het systeem te schrijven.</translation></message>
    <message><source>Import results</source><translation>Importresultaten</translation></message>
    <message><source>Imported</source><translation>Geïmporteerd</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>Het rapport toont de opdrachtresultaten voor het configuratieconcept. Selecteer een rij om de volledige opdracht en het resultaat te bekijken.</translation></message>
    <message><source>Imported: %1</source><translation>Geïmporteerd: %1</translation></message>
    <message><source>Information</source><translation>Informatie</translation></message>
    <message><source>Item details</source><translation>Itemdetails</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Het opslaan van grote lijsten met klaargezette bestanden kan langer duren.</translation></message>
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
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Als UWF het station met de toepassing beveiligt, zorg er dan eerst voor dat de toepassingsbestanden na een herstart behouden blijven. Het definitief opslaan van veel klaargezette bestanden kan het afsluiten of herstarten vertragen.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Meerdere bestanden kunnen worden geselecteerd.</translation></message>
    <message><source>Name and location</source><translation>Naam en locatie</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Benoemde waarde: %1 · alleen deze waarde</translation></message>
    <message><source>Navigate to registry path</source><translation>Naar registerpad navigeren</translation></message>
    <message><source>New folder</source><translation>Nieuwe map</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Nieuwe records verschijnen na vernieuwen.</translation></message>
    <message><source>Next-session configuration</source><translation>Configuratie volgende sessie</translation></message>
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
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Alleen klaargezette bestanden worden automatisch opgeslagen. Andere niet-vastgelegde wijzigingen op beveiligde paden gaan na herstarten verloren.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Alleen klaargezette bestanden worden automatisch opgeslagen. Andere niet-vastgelegde wijzigingen op beveiligde paden gaan na afsluiten verloren.</translation></message>
    <message><source>Open</source><translation>Openen</translation></message>
    <message><source>Open folder</source><translation>Map openen</translation></message>
    <message><source>Operating system</source><translation>Besturingssysteem</translation></message>
    <message><source>Operation and security</source><translation>Werking en beveiliging</translation></message>
    <message><source>Original command</source><translation>Oorspronkelijke opdracht</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Kritieke overlaydrempel bereikt. Sla wijzigingen op en herstart veilig.</translation></message>
    <message><source>Overlay maximum size</source><translation>Maximale overlaygrootte</translation></message>
    <message><source>Overlay type</source><translation>Overlaytype</translation></message>
    <message><source>Overlay usage</source><translation>Overlaygebruik</translation></message>
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
    <message><source>Ready to save</source><translation>Gereed om op te slaan</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Records van de huidige uitvoering verschijnen hier zodra het lezen is voltooid.</translation></message>
    <message><source>Recursive staging</source><translation>Recursief klaarzetten</translation></message>
    <message><source>Refresh folder</source><translation>Map vernieuwen</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Registersleutel of instelling voor behoud</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Resterende taken kunnen worden geannuleerd nadat het huidige doel is voltooid.</translation></message>
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
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Herstarten gaat verder na het opslaan. Houd de toepassing geopend.</translation></message>
    <message><source>Restore</source><translation>Herstellen</translation></message>
    <message><source>Retry</source><translation>Opnieuw proberen</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Controleer de foutdetails voordat u doorgaat met afsluiten.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Controleer de foutdetails voordat u herstart.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Controleer de bewerkingsdetails. Lees de huidige systeemstatus voordat u wijzigingen opnieuw probeert die mogelijk al zijn geschreven.</translation></message>
    <message><source>Review the result for each target.</source><translation>Controleer het resultaat voor elk doel.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Controleer deze wijzigingen voordat u ze naar het systeem schrijft.</translation></message>
    <message><source>Running</source><translation>Actief</translation></message>
    <message><source>Save</source><translation>Opslaan</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Sla klaargezette bestanden op schijf op en herstart daarna.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Sla klaargezette bestanden op schijf op en sluit daarna af.</translation></message>
    <message><source>Saving staged files</source><translation>Klaargezette bestanden opslaan</translation></message>
    <message><source>Saving to disk</source><translation>Naar schijf opslaan</translation></message>
    <message><source>Saving…</source><translation>Opslaan…</translation></message>
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
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>Afsluiten gaat verder na het opslaan. Houd de toepassing geopend.</translation></message>
    <message><source>Size</source><translation>Grootte</translation></message>
    <message><source>Some changes could not be applied</source><translation>Sommige wijzigingen konden niet worden toegepast</translation></message>
    <message><source>Some files could not be saved</source><translation>Sommige bestanden konden niet worden opgeslagen</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Sommige hardwaregegevens konden niet worden gelezen. Andere diagnostische gegevens blijven beschikbaar om te bekijken en kopiëren.</translation></message>
    <message><source>Source code</source><translation>Broncode</translation></message>
    <message><source>Staged file</source><translation>Klaargezet bestand</translation></message>
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
    <message><source>Target</source><translation>Doel</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Terminal Services-licenties behouden</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>De UWF-provider kon de opsomming niet voltooien. Dit kan optreden bij een grote overlay of onvoldoende systeembronnen.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>De configuratie is naar het systeem geschreven. De meeste wijzigingen worden actief na herstarten.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>De configuratie komt overeen met het doel van de volgende sessie. Bekijk of exporteer opdrachten in Volledige configuratie.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>De huidige logboekrecords worden onherroepelijk gewist.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>De doelmap bestaat niet.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>De servicestatus van de uitgebreide modus kon niet worden gelezen. Controleer de servicemachtigingen en probeer het opnieuw.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>De lijst kan bestanden missen of reeds vastgelegde bestanden bevatten. Gebruik deze niet als lijst voor vastleggen.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Het geselecteerde item bestaat niet meer: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>De service is geïnstalleerd maar gestopt. Start deze om automatisch opslaan te herstellen.</translation></message>
    <message><source>This PC</source><translation>Deze pc</translation></message>
    <message><source>This folder is empty</source><translation>Deze map is leeg</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Dit rapport kan accountnamen, apparaat-ID’s en bestandspaden bevatten. Controleer het voordat u het deelt.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Sla dit UTF-8 CMD-script op als een .cmd-bestand en voer het uit in de opdrachtprompt. Bij export wordt hetzelfde script gebruikt.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Probeer een ander trefwoord of logboekniveau.</translation></message>
    <message><source>Try another search term.</source><translation>Probeer een andere zoekterm.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Type en maximale grootte kunnen niet worden gewijzigd terwijl het filter actief is. Schakel het filter uit en herstart eerst.</translation></message>
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
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Mislukte items zijn niet opgeslagen. Selecteer een rij voor het volledige pad en de reden.</translation></message>
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
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Gebruik zonder uitgebreide modus Veilig afsluiten of Veilig herstarten in deze toepassing om klaargezette bestanden op te slaan.</translation></message>
    <message><source>Write protection</source><translation>Schrijfbeveiliging</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Deze configuratiewijzigingen naar het systeem schrijven? De meeste wijzigingen worden actief na herstarten.</translation></message>
    <message><source>staged files</source><translation>klaargezette bestanden</translation></message>
    <message><source>uwfmgr commands</source><translation>uwfmgr-opdrachten</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Markeer de geselecteerde paden voor verwijdering. Dit wordt van kracht na toepassen.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Na toepassen uit de lijst voor automatisch opslaan van bestanden verwijderen.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Na toepassen aan de lijst voor automatisch opslaan van bestanden toevoegen.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Door dit plan toe te passen wordt de opgeslagen bestandslijst gewist en wordt de bijbehorende registeruitsluiting verwijderd als deze niet meer nodig is.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Wijzigingen in de bestandslijst moeten in UWF Manager worden toegepast en kunnen niet als uwfmgr-opdrachten worden geëxporteerd.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>De bestandslijst wordt na toepassen opgeslagen</translation></message>
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
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Alleen succesvol doorlopen inhoud vastleggen. Onleesbare takken blijven ongewijzigd.</translation></message>
    <message><source>Committing changes</source><translation>Wijzigingen vastleggen</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Voltooide verwijderingen en vastleggingen kunnen niet ongedaan worden gemaakt.</translation></message>
    <message><source>Continue…</source><translation>Doorgaan…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Alleen gevonden doelen verwijderen en vastleggen. Onleesbare takken en hun bovenliggende mappen of sleutels blijven behouden.</translation></message>
    <message><source>Deleting and committing</source><translation>Verwijderen en vastleggen</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Verwijderen en vastleggen kunnen niet ongedaan worden gemaakt. Alleen het geselecteerde bereik wordt verwerkt; wachtende UWF-instellingen worden niet toegepast.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Bereik bepalen. Onleesbare takken worden vastgelegd terwijl andere inhoud verder wordt doorlopen.</translation></message>
    <message><source>Export traversal records</source><translation>Doorlooplogboek exporteren</translation></message>
    <message><source>No actionable content</source><translation>Geen verwerkbare inhoud</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Geen verwerkbare doelen gevonden. Er is niets verwijderd of vastgelegd.</translation></message>
    <message><source>No changes have been committed.</source><translation>Nog geen wijzigingen vastgelegd</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Niets verwijderd of vastgelegd</translation></message>
    <message><source>Path</source><translation>Pad</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Het geselecteerde bereik definitief verwijderen en de verwijderingen opslaan op het beschermde volume.</translation></message>
    <message><source>Preparation complete</source><translation>Voorbereiding voltooid</translation></message>
    <message><source>Preparing commit</source><translation>Vastleggen voorbereiden</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Verwijderen en vastleggen voorbereiden</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Alleen succesvol doorlopen inhoud verwerken</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Reparsepunten worden overgeslagen om binnen het geselecteerde bereik te blijven.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Opnieuw starten maakt vastleggen niet ongedaan. Alleen het geselecteerde bereik wordt verwerkt; wachtende UWF-instellingen worden niet toegepast.</translation></message>
    <message><source>Retry failed items</source><translation>Mislukte items opnieuw proberen</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Wijzigingen in het geselecteerde bereik opslaan op het beschermde volume.</translation></message>
    <message><source>Stopping the operation</source><translation>Bewerking stoppen</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Geslaagde en mislukte leesacties zijn vastgelegd</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>De map is niet leeg; onverwerkte inhoud is behouden.</translation></message>
    <message><source>The operation could not be completed</source><translation>De bewerking kon niet worden voltooid</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>De registersleutel bevat nieuwe waarden; onverwerkte inhoud is behouden.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>De registersleutel heeft nog subsleutels; onverwerkte inhoud is behouden.</translation></message>
    <message><source>The target no longer exists.</source><translation>Het doel bestaat niet meer.</translation></message>
    <message><source>Traversal complete</source><translation>Doorlopen voltooid</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Doorlopen voltooid met onleesbare inhoud</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Het doorlooplogboek blijft behouden en kan worden geëxporteerd.</translation></message>
    <message><source>Traverse again</source><translation>Opnieuw doorlopen</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Doorloop opnieuw of sluit dit venster.</translation></message>
    <message><source>Traversing the folder</source><translation>Map doorlopen</translation></message>
    <message><source>Traversing the registry</source><translation>Register doorlopen</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Kan de bewerking niet voorbereiden</translation></message>
    <message><source>View results</source><translation>Resultaten bekijken</translation></message>

    <message><source>All changes committed</source><translation>Alle wijzigingen vastgelegd</translation></message>
    <message><source>Some changes committed</source><translation>Wijzigingen gedeeltelijk vastgelegd</translation></message>
    <message><source>No changes could be committed</source><translation>Geen wijzigingen konden worden vastgelegd</translation></message>
    <message><source>No changes to commit</source><translation>Geen wijzigingen om vast te leggen</translation></message>
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
    <message><source>Export configuration</source><translation>Configuratie exporteren</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Sla het huidige thema, de taal en de zichtbaarheid van de HUB op in UWF.ini naast deze toepassing.</translation></message>
    <message><source>Configuration saved</source><translation>Configuratie opgeslagen</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Het automatisch vastleggen van bestanden is gestopt. Verhelp de fout voordat u het opnieuw probeert.</translation></message>
    <message><source>No net change for this setting</source><translation>De eindwaarde van deze instelling is na het importeren ongewijzigd</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Uiteindelijke conceptwaarde: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Uiteindelijke conceptwaarde: %1 → %2 (gevraagd: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Gevraagd: %1 → %2; vervangen door een latere opdracht. Uiteindelijke conceptwaarde → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Concept voor %1 bijgewerkt</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Concept voor de beveiligingsinstelling van volume %1 bijgewerkt → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Selecteer de vermeldingen die u wilt verwijderen.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>De geselecteerde vermeldingen kunnen momenteel niet worden hersteld.</translation></message>
</context>
</TS>
