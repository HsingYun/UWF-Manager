<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="it" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Anteprima</translation></message>
    <message><source>enable</source><translation>attivare</translation></message>
    <message><source>disable</source><translation>disattivare</translation></message>
    <message><source>Enabled</source><translation>Attivo</translation></message>
    <message><source>Disabled</source><translation>Disattivo</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Annulla</translation></message>
    <message><source>Close</source><translation>Chiudi</translation></message>
    <message><source>Refresh</source><translation>Aggiorna</translation></message>
    <message><source>Add</source><translation>Aggiungi</translation></message>
    <message><source>Remove</source><translation>Rimuovi</translation></message>
    <message><source>Yes</source><translation>Sì</translation></message>
    <message><source>No</source><translation>No</translation></message>
    <message><source>Excluded</source><translation>Escluso</translation></message>
    <message><source>Not excluded</source><translation>Non escluso</translation></message>
    <message><source>File…</source><translation>File…</translation></message>
    <message><source>Folder…</source><translation>Cartella…</translation></message>
    <message><source>volume ID</source><translation>ID volume</translation></message>
    <message><source>drive letter</source><translation>lettera di unità</translation></message>
    <message><source>Drive letter</source><translation>Lettera di unità</translation></message>
    <message><source>Volume ID</source><translation>ID volume</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Stato di protezione del volume nella sessione corrente (sola lettura).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Protegge il volume nella prossima sessione. Le scritture vengono reindirizzate all’overlay e scartate al riavvio.</translation></message>
    <message><source>Current session</source><translation>Sessione corrente</translation></message>
    <message><source>Next session</source><translation>Prossima sessione</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identificazione del volume: la lettera è semplice, ma il collegamento si perde se viene riassegnata, ad esempio aggiungendo o rimuovendo dischi. L’ID volume rimane stabile.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Volume non supportato da UWF: </translation></message>
    <message><source>Global settings</source><translation>Impostazioni globali</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Stato del filtro UWF nella sessione corrente (sola lettura).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Attiva UWF nella prossima sessione. Le scritture sui volumi protetti vanno nell’overlay e vengono scartate al riavvio.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Per modificare tipo e dimensione massima:
1. Disattiva il filtro qui sopra.
2. Riavvia per renderlo inattivo.
3. Modifica l’impostazione.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Posizione dell’overlay: RAM è più veloce ma usa memoria; Disk usa il disco di sistema e offre più capacità. Entrambi vengono scartati al riavvio.</translation></message>
    <message><source>Type</source><translation>Tipo</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Capacità massima dell’overlay. RAM è limitata dalla memoria totale. Disk richiede almeno 1024 MB e spazio libero sufficiente sul volume di sistema.</translation></message>
    <message><source>Maximum size</source><translation>Dimensione massima</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Dimensione massima · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Soglia di avviso</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Registra una notifica di avviso nel registro eventi quando l’utilizzo dell’overlay raggiunge questo valore. Se entrambi gli eventi sono abilitati, il valore deve essere inferiore alla soglia critica. Il valore 0 disabilita solo l’evento di avviso.</translation></message>
    <message><source>Critical threshold</source><translation>Soglia critica</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Registra una notifica critica nel registro eventi quando l’utilizzo dell’overlay raggiunge questo valore. Se entrambi gli eventi sono abilitati, il valore deve essere superiore alla soglia di avviso. Il valore 0 disabilita solo l’evento critico.</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Tipo di unità non supportato (solo dischi locali fissi).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Il volume supera il limite UWF di 16 TiB per volume protetto.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: protezione volume disponibile, ma senza esclusioni né salvataggio per singolo file.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Non riuscito: Lettura informazioni volume.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 liberi / %2</translation></message>
    <message><source>Commit</source><translation>Salva su disco</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Salva su disco le modifiche del file nell’overlay / Salva modifiche del registro. Questa operazione non può essere annullata.</translation></message>
    <message><source>Commit file changes…</source><translation>Salva modifiche del file…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Seleziona file da salvare su disco. Salva su disco le modifiche del file nell’overlay.</translation></message>
    <message><source>Commit folder changes…</source><translation>Salva modifiche della cartella…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Seleziona cartella da salvare su disco. Salva su disco le modifiche della cartella nell’overlay. Sono incluse tutte le sottocartelle.</translation></message>
    <message><source>Delete and commit file…</source><translation>Elimina il file e salva l’eliminazione su disco…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Seleziona file da eliminare definitivamente. Elimina il file e salva l’eliminazione su disco.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Elimina la cartella e il contenuto e salva le eliminazioni su disco…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Seleziona cartella da eliminare definitivamente. Elimina la cartella e il contenuto e salva le eliminazioni su disco. Sono incluse tutte le sottocartelle.</translation></message>
    <message><source>Commit registry changes…</source><translation>Salva modifiche del registro…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Inserisci la chiave e, facoltativamente, il nome del valore. Senza nome, vengono salvati tutti i valori della chiave e delle sottochiavi.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Salva eliminazione dal registro…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Inserisci la chiave da eliminare e un eventuale nome di valore. Senza nome, vengono eliminati e salvati l’intera chiave, tutti i valori e le sottochiavi.</translation></message>
    <message><source>File exclusions</source><translation>Esclusioni file</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>File e cartelle del volume esclusi dalla protezione UWF. Fai doppio clic su una voce per copiarne il percorso.</translation></message>
    <message><source>Registry exclusions</source><translation>Esclusioni registro</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Le esclusioni del registro dipendono dal filtro UWF globale, non dalla protezione del volume. L’elenco è condiviso tra i volumi e compare una sola volta nelle schede. Il doppio clic copia il percorso.</translation></message>
    <message><source>File staging</source><translation>File da salvare</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>file / cartelle: Salvataggio automatico prima di un arresto o riavvio sicuro. Sono incluse tutte le sottocartelle. Finché l’elenco contiene voci, questa esclusione del registro lo conserva tra le sessioni protette: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Aggiungi file o una cartella all’elenco di salvataggio automatico.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Seleziona file da salvare su disco. Salvataggio automatico prima di un arresto o riavvio sicuro.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Seleziona cartella da salvare su disco. Salvataggio automatico prima di un arresto o riavvio sicuro. Sono incluse tutte le sottocartelle.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Seleziona file da salvare su disco · File da salvare</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Seleziona cartella da salvare su disco · File da salvare · Sono incluse tutte le sottocartelle.</translation></message>
    <message><source>Cannot stage this path</source><translation>Impossibile preparare questo percorso</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Il percorso si sovrappone a un’esclusione UWF. Rimuovi prima l’esclusione:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Il percorso si sovrappone a un file da salvare automaticamente. Rimuovi prima la voce:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Il percorso si sovrappone a una voce di salvataggio automatico</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Questa esclusione del registro è necessaria finché l’elenco contiene file da salvare</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Il percorso non esiste o non è del tipo previsto:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Impossibile verificare il percorso:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>I reparse point non possono essere aggiunti al salvataggio automatico:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Senza lettera di unità UWF non può salvare il percorso:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Il percorso è sul volume %1. Aggiungilo dalla scheda di salvataggio automatico di quel volume.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>La radice del volume non può essere aggiunta: salverebbe ricorsivamente l’intero volume.</translation></message>
    <message><source>File staging could not be saved</source><translation>Non riuscito: Memorizzazione elenco file da salvare</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>L’elenco è stato ricaricato perché l’aggiornamento non è riuscito:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>File da salvare: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Salvataggio automatico prima di un arresto o riavvio sicuro.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>File da salvare: %1 · file: %2 · cartelle: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Il salvataggio automatico dei file richiede questa esclusione del registro per conservare il proprio elenco dopo i riavvii.</translation></message>
    <message><source>Built-in</source><translation>Integrato</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Il filtro UWF è disattivato. Non ci sono modifiche accumulate nell’overlay da salvare.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Il file system %1 non supporta il salvataggio per singolo file.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Questo volume non è supportato da UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Il volume non supporta il salvataggio dei file. Disponibile solo il salvataggio del registro (le esclusioni sono globali).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Il file system %1 non supporta il salvataggio per singolo file. Disponibile solo il salvataggio del registro (le esclusioni sono globali).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Il volume non è protetto da UWF; non ci sono modifiche ai file da salvare.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Il volume non è protetto da UWF; non ci sono modifiche ai file da salvare. Disponibile solo il salvataggio del registro (le esclusioni sono globali).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Seleziona file da salvare su disco</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Seleziona cartella da salvare su disco</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Seleziona file da eliminare definitivamente</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Seleziona cartella da eliminare definitivamente</translation></message>
    <message><source>Name</source><translation>Nome</translation></message>
    <message><source>Data</source><translation>Dati</translation></message>
    <message><source>(Default)</source><translation>(Predefinito)</translation></message>
    <message><source>(value not set)</source><translation>(valore non impostato)</translation></message>
    <message><source>(empty)</source><translation>(vuoto)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Sconosciuto (Valore: File troppo grande)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Visualizza file overlay. &lt;b&gt;Solo istantanea diagnostica.&lt;/b&gt; Non usare l’elenco per decidere cosa salvare.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>L’elenco dei file nell’overlay è disponibile solo sui volumi NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>Il filtro UWF è disattivato; nessun volume ha un overlay da esaminare.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Il volume non è protetto da UWF e non ha overlay da esaminare.</translation></message>
    <message><source>Failed: %1</source><translation>Non riuscito: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Il provider WMI si è arrestato durante l’elenco dei file. È un’instabilità nota di UWF_Overlay.GetOverlayFiles con overlay grandi o carichi I/O elevati.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Memoria insufficiente o operazione non supportata. Servono NTFS e memoria libera. Riprova con un overlay più piccolo o chiudi applicazioni che usano molta memoria.</translation></message>
    <message><source>Raw error: %1</source><translation>Codice errore: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Esporta su file… · Overlay · file</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>File di testo (*.txt);;Tutti i file (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>File di comandi (*.cmd);;Tutti i file (*)</translation></message>
    <message><source>Export failed</source><translation>Esportazione non riuscita</translation></message>
    <message><source>Export finished</source><translation>Esportazione completata</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Non riuscito: Apertura file (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Non riuscito: Scrittura file (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Configurazione: Scrittura file · voci: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Impossibile escludere un intero volume (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>I file di paging, swap e ibernazione non possono essere esclusi: UWF ne dipende.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Impossibile escludere l’intera directory: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Impossibile escludere l’intera directory: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Impossibile escludere l’intera directory: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Questo file di sistema essenziale non può essere escluso: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Il file di registro utente NTUSER.DAT non può essere escluso.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>La chiave del registro non può essere vuota.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>La chiave del registro non può iniziare con una barra inversa.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Il percorso contiene barre inverse consecutive e non è valido.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>I percorsi del registro usano `\` come separatore, non `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Il percorso contiene caratteri di controllo invisibili e non è valido.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>UWF vieta di escludere il segreto dell’account computer di dominio: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Si possono escludere solo sottochiavi specifiche delle seguenti chiavi principali:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Aggiungi file o cartelle alle esclusioni. Gli elementi esclusi non sono protetti da UWF.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Seleziona file da aggiungere · Esclusioni file.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Seleziona cartella da aggiungere · Esclusioni file. Sono incluse tutte le sottocartelle.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Inserisci il percorso della chiave del registro da escludere.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Inserisci il percorso della chiave del registro da escludere. / Persistenza.</translation></message>
    <message><source>Registry key…</source><translation>Registro · Chiave…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Chiave segreta dominio (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Licenza accesso client Servizi terminal (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Conserva il segreto del dominio (password account computer) tra le sessioni UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Conserva le licenze client Servizi terminal tra le sessioni UWF.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistenza %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Rimuovi selezionati</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Rimuovi le esclusioni selezionate. Ha effetto dopo l’applicazione.</translation></message>
    <message><source>Open containing folder</source><translation>Apri cartella contenitore</translation></message>
    <message><source>Copy file path</source><translation>Copia percorso file</translation></message>
    <message><source>Copy registry path</source><translation>Copia percorso registro</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Salva su disco le modifiche della cartella nell’overlay…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Salva su disco le modifiche del file nell’overlay…</translation></message>
    <message><source>Copied to clipboard</source><translation>Copiato negli appunti</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Seleziona file da aggiungere · Esclusioni file. È possibile selezionare più file.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Seleziona cartella da aggiungere · Esclusioni file</translation></message>
    <message><source>Add registry exclusion</source><translation>Aggiunta esclusione registro</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Il percorso %1 non è sul volume %2 e non può essere aggiunto alle sue esclusioni.</translation></message>
    <message><source>Path is not on this volume</source><translation>Il percorso non è su questo volume</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Impossibile aggiungere l’esclusione</translation></message>
    <message><source>File: %1</source><translation>File… %1</translation></message>
    <message><source>Folder: %1</source><translation>Cartella… %1</translation></message>
    <message><source>Registry: %1</source><translation>Registro: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Escluso per impostazione predefinita; modificabile in qualsiasi momento.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Sessione corrente: %1
Prossima sessione: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Modifiche in sospeso: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Gestore del filtro di scrittura unificato (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Rilegge lo stato UWF attuale e la configurazione della prossima sessione.</translation></message>
    <message><source>Review and apply</source><translation>Verifica e applica</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Verifica tutte le modifiche in sospeso e applicale insieme. La maggior parte delle modifiche ha effetto al prossimo riavvio.</translation></message>
    <message><source>Safe shutdown</source><translation>Arresto sicuro</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Il sistema si arresterà in sicurezza tramite UWF. Disponibile anche con overlay UWF pieno.</translation></message>
    <message><source>Safe restart</source><translation>Riavvio sicuro</translation></message>
    <message><source>Direct restart</source><translation>Riavvio diretto</translation></message>
    <message><source>Direct shutdown</source><translation>Arresto diretto</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Il sistema si riavvierà in sicurezza tramite UWF. Disponibile anche con overlay UWF pieno.</translation></message>
    <message><source>Log</source><translation>Registro attività</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Visualizza il registro interno della sessione per la risoluzione dei problemi.</translation></message>
    <message><source>About</source><translation>Informazioni</translation></message>
    <message><source>Switch display language</source><translation>Cambia lingua di visualizzazione</translation></message>
    <message><source>Language</source><translation>Lingua</translation></message>
    <message><source>Theme</source><translation>Tema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Passa tra tema chiaro e scuro</translation></message>
    <message><source>System information</source><translation>Informazioni di sistema</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Istantanea diagnostica</translation></message>
    <message><source>Read only</source><translation>Sola lettura</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Destinazione non trovata; nell’overlay non c’è nulla da salvare.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>La destinazione esiste solo nell’overlay: creata sotto UWF e mai salvata sul disco o nel registro persistente. Sparirà al riavvio; salvare l’eliminazione non è necessario né possibile.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Il sistema ha rifiutato un parametro (percorso o argomento non valido).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Operazione non riuscita. La destinazione potrebbe essere in uso, ad esempio in Esplora file o in un file aperto. Chiudi i programmi interessati e riprova.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Operazione non riuscita; consulta il registro per i dettagli.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operazione rifiutata (codice %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Sconosciuto.</translation></message>
    <message><source>Commit canceled</source><translation>Salvataggio annullato</translation></message>

    <message><source>Succeeded</source><translation>Riuscito</translation></message>
    <message><source>Skipped</source><translation>Ignorato</translation></message>
    <message><source>Failed</source><translation>Non riuscito</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Riuscito: %1; Ignorato: %2; Non riuscito: %3.</translation></message>

    <message><source>Existed before</source><translation>Presente prima</translation></message>
    <message><source>Exists after</source><translation>Presente dopo</translation></message>
    <message><source>Error code</source><translation>Codice errore</translation></message>
    <message><source>Reason</source><translation>Motivo</translation></message>
    <message><source>Previous page</source><translation>Pagina precedente</translation></message>
    <message><source>First page</source><translation>Prima pagina</translation></message>
    <message><source>Last page</source><translation>Ultima pagina</translation></message>
    <message><source>Next page</source><translation>Pagina successiva</translation></message>
    <message><source>Copy selected rows</source><translation>Copia righe selezionate</translation></message>
    <message><source>Copy all</source><translation>Copia tutto</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Questa esclusione del registro è necessaria finché l’elenco contiene file da salvare. File da salvare: Svuota → Rimozione esclusione registro.</translation></message>
    <message><source>Copy current page</source><translation>Copia pagina corrente</translation></message>
    <message><source>No pending changes</source><translation>Nessuna modifica in sospeso</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Esclusioni registro · Impostazioni globali.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: Protezione UWF / Esclusioni file / File da salvare.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Non supportato</translation></message>
    <message><source>Review and apply changes</source><translation>Verifica e applica le modifiche</translation></message>
    <message><source>Confirm apply</source><translation>Conferma applicazione</translation></message>
    <message><source>Result</source><translation>Risultato</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filtro: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Impostazione soglia di avviso: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Impostazione soglia critica: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Impostazione tipo overlay / Impostazione dimensione massima: Per modificare tipo e dimensione massima:
1. Disattiva il filtro qui sopra.
2. Riavvia per renderlo inattivo.
3. Modifica l’impostazione.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Impostazione tipo overlay: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Impostazione dimensione massima: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Impostazione dimensione massima: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · Protezione UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Identifica tramite: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1 · Aggiunta esclusione file: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1 · Rimozione esclusione file: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Aggiunta esclusione registro: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Rimozione esclusione registro: %1</translation></message>
    <message><source>Time</source><translation>Ora</translation></message>
    <message><source>Level</source><translation>Livello</translation></message>
    <message><source>Tag</source><translation>Tag</translation></message>
    <message><source>Message</source><translation>Messaggio</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Calcolo dei file da salvare automaticamente…</translation></message>
    <message><source>Committing staged files</source><translation>Salvataggio dei file preparati</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>Elaborati %1 file su %2.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Nessun dettaglio aggiuntivo sull’errore.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Salvataggio automatico interrotto in modo imprevisto:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Salvataggio automatico incompleto: Errore sconosciuto.</translation></message>
    <message><source>File staging will be skipped</source><translation>Il salvataggio automatico verrà ignorato</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Riavvio diretto: Il salvataggio automatico verrà ignorato.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Arresto diretto: Il salvataggio automatico verrà ignorato.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Salvataggio automatico ignorato perché è stata scelta un’azione di alimentazione diretta.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Arresto sicuro: Non riuscito</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Arresta: Non riuscito (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Riavvio sicuro: Non riuscito</translation></message>
    <message><source>Direct restart failed</source><translation>Riavvio diretto: Non riuscito</translation></message>
    <message><source>Direct shutdown failed</source><translation>Arresto diretto: Non riuscito</translation></message>
    <message><source>Restart failed: %1</source><translation>Riavvia: Non riuscito (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Impossibile leggere l’elenco di salvataggio automatico:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Impossibile leggere l’elenco di salvataggio automatico: Errore sconosciuto.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>file: %1 · Riuscito: %2 · Ignorato (file): %3 · Ignorato (voci): %4 · Non riuscito: %5</translation></message>
    <message><source>File staging preparation</source><translation>File da salvare: Dettagli operazione</translation></message>
    <message><source>Reparse points are not supported.</source><translation>I reparse point non sono supportati.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Si possono preparare solo percorsi assoluti su volumi locali.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Si possono preparare solo percorsi assoluti su volumi locali.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Impossibile verificare il percorso preparato.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Il percorso del file preparato ora ha un tipo diverso.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Il percorso della cartella preparata ora ha un tipo diverso.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Impossibile elencare tutto il contenuto della cartella preparata.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Il percorso non ha una lettera di unità.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>La radice di un volume non può essere elaborata ricorsivamente.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Il provider UWF ha rifiutato il salvataggio.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Salvataggio non riuscito: Errore sconosciuto.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Altri %1 errori sono stati scritti nel registro.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Il percorso non ha una lettera di unità. Identificazione volume di destinazione: Non riuscito.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Nessun record della sessione corrente per il volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Il percorso è nell’elenco esclusioni file.
Esclusione: %1</translation></message>

    <message><source>Continue</source><translation>Continua</translation></message>
    <message><source>Delete and commit</source><translation>Elimina e salva</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>La chiave è nell’elenco esclusioni registro.
Esclusione: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Nessun record del filtro registro per la sessione corrente.</translation></message>

    <message><source>Export commands to file</source><translation>Esporta comandi su file</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Esporta su file… · Comando: %1
%2</translation></message>
    <message><source>Import</source><translation>Importa</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Digita o incolla un comando per riga. Carica testi, registri o script (.txt, .bat, .ps1); le righe con questo comando vengono aggiunte. L’importazione prepara le modifiche. Solo «Verifica e applica» nella barra strumenti le scrive nel sistema.</translation></message>
    <message><source>Load from file…</source><translation>Carica da file…</translation></message>
    <message><source>Load recommended configuration</source><translation>Carica configurazione consigliata</translation></message>
    <message><source>Choose recommended configuration</source><translation>Scegli configurazione consigliata</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Seleziona i gruppi consigliati da aggiungere. Puoi verificare o eliminare singoli comandi prima dell’importazione.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Dalla documentazione ufficiale Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Esclusioni UWF consigliate da Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Verifica prima dell’importazione: le cartelle devono esistere affinché UWF accetti le esclusioni.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Fonte: documentazione ufficiale Microsoft UWF su esclusioni comuni e supporto antimalware.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Analisi utilizzo software (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Conserva permanentemente: Stato adesione nei criteri</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Conserva permanentemente: Stato adesione locale</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Conserva permanentemente: Indicatore disattivazione caricamento</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Servizio trasferimento intelligente in background (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Conserva permanentemente: File della coda download</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Conserva permanentemente: Indice stato trasferimenti</translation></message>
    <message><source>Network profiles and policies</source><translation>Profili e criteri di rete</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Criteri GPO rete wireless</translation></message>
    <message><source>Wired network GPO policy</source><translation>Criteri GPO rete cablata</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Criteri GPO rete wireless · file</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Criteri GPO rete cablata · file</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Profili interfacce wireless</translation></message>
    <message><source>Wired network interface profiles</source><translation>Profili interfacce cablate</translation></message>
    <message><source>Wireless service configuration</source><translation>Configurazione servizio wireless</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Configurazione servizio banda larga mobile</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Configurazione servizio configurazione automatica reti cablate</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>I profili XML dipendono dal dispositivo. Aggiungi manualmente i percorsi esatti Interfaces\{GUID}\{GUID}.xml se necessario.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Ora legale (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Conserva permanentemente: Definizioni fusi orari</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Conserva permanentemente: Informazioni fuso selezionato</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Conserva permanentemente: File del prodotto e aggiornamenti</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Conserva permanentemente: Firme e stato in ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Conserva permanentemente: Windows Update · Registro attività</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Conserva permanentemente: MpCmdRun · Registro attività</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Conserva permanentemente: Stato del prodotto nel registro</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Conserva permanentemente: WdBoot · Stato del servizio</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Conserva permanentemente: WdFilter · Stato del servizio</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Conserva permanentemente: WdNisSvc · Stato del servizio</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Conserva permanentemente: WdNisDrv · Stato del servizio</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Conserva permanentemente: WinDefend · Stato del servizio</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Conserva permanentemente: File del programma client</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Conserva permanentemente: Windows Update · Registro attività</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Conserva permanentemente: MpCmdRun · Registro attività</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Conserva permanentemente: Firme e stato antimalware</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Conserva permanentemente: Stato antimalware nel registro</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Scegli file contenenti comandi uwfmgr</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>File di testo (*.txt *.cmd);;Tutti i file (*)</translation></message>
    <message><source>Status</source><translation>Stato</translation></message>
    <message><source>Import failed</source><translation>Importazione non riuscita</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Errore interno: nessun gestore di applicazione registrato.</translation></message>
    <message><source>Nothing to import</source><translation>Nessun comando da importare</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Nessun comando uwfmgr trovato nell’input.</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplicato: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Non supportato: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Lotto %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Duplicato</translation></message>
    <message><source>Unsupported</source><translation>Non supportato</translation></message>
    <message><source>File too large</source><translation>File troppo grande</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Il file %1 supera 5 MB e non è stato analizzato. Filtralo prima manualmente.</translation></message>
    <message><source>Cannot read file</source><translation>Impossibile leggere il file</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Non riuscito: Apertura file (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Scartare le modifiche in sospeso?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Ci sono %1 modifiche non applicate.

Scartarle e continuare?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Comando uwfmgr incompleto</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Argomento mancante: Dimensione (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>La dimensione deve essere un intero non negativo in MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Argomento mancante: Overlay · Tipo (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Valore non valido: Overlay · Tipo: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Argomento mancante: Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Il volume deve essere una lettera di unità, ad esempio C:</translation></message>
    <message><source>Missing path argument</source><translation>Argomento mancante: Percorso</translation></message>
    <message><source>Missing registry key argument</source><translation>Argomento mancante: Registro · Chiave</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Virgolette doppie non chiuse nel comando</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Argomento aggiuntivo imprevisto: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Comando uwfmgr non supportato (nessuna azione corrispondente nell’interfaccia)</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Stato desiderato già presente; nessuna operazione necessaria</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Il percorso è fuori dal volume oppure il volume non supporta esclusioni (es. exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Rifiutato dall’elenco di blocco UWF (file di sistema, Windows, paging, ecc.)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valore non valido: Dimensione: %1</translation></message>
    <message><source>maximum size</source><translation>Dimensione massima</translation></message>
    <message><source>warning threshold</source><translation>Soglia di avviso</translation></message>
    <message><source>critical threshold</source><translation>Soglia critica</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>La configurazione globale non è disponibile; il comando non è stato aggiunto alle modifiche in sospeso.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Volume %1 sconosciuto (nessun disco compatibile UWF con questa lettera)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Stato desiderato già presente; nessuna operazione necessaria</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Il percorso non ha una lettera di unità. Identificazione volume di destinazione: Non riuscito.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Non supportato</translation></message>
    <message><source>file exclusion</source><translation>Esclusioni file</translation></message>
    <message><source>registry exclusion</source><translation>Esclusioni registro</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Le esclusioni del registro richiedono la scheda Registro, qui assente</translation></message>
    <message><source>Unsupported command</source><translation>Comando: Non supportato</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Mostra/nascondi.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Attivo</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Disattivo</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Sconosciuto</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Utilizzato %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Modalità avanzata</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordina il salvataggio automatico con l’arresto e il riavvio di Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Non riuscito: Lettura stato modalità avanzata (Errore sconosciuto).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Non riuscito: Lettura stato servizio:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Non riuscito: Lettura stato servizio (Errore sconosciuto).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Il servizio non è configurato come processo indipendente.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Il servizio non è configurato per l’avvio automatico.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Il servizio non è configurato per l’esecuzione come LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>Il servizio non è in esecuzione.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Il percorso o gli argomenti del servizio non corrispondono a questo eseguibile.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Il timeout di prearresto del servizio non è configurato correttamente.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Il servizio non dichiara i privilegi necessari per avviare l’agente dell’interfaccia.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Il servizio non ha accettato le notifiche di prearresto di Windows.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Attiva modalità avanzata</translation></message>
    <message><source>Disable enhanced mode</source><translation>Disattiva modalità avanzata</translation></message>
    <message><source>Start service</source><translation>Avvia servizio</translation></message>
    <message><source>Repair enhanced mode</source><translation>Ripara modalità avanzata</translation></message>
    <message><source>Delete service</source><translation>Elimina servizio</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Arresta ed elimina il servizio avanzato, inclusi i dati residui nel registro.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Il servizio è assente, ma restano dati nel registro.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>In attesa dell’autenticazione dell’agente dell’interfaccia.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Il servizio è attivo, ma nessun agente dell’interfaccia autenticato è connesso.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Servizio ausiliario della modalità avanzata UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Modalità avanzata · Configurazione: Applicato.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Configurazione: Applicato. Segnalazione persistenza UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Modalità avanzata · Configurazione: Non riuscito:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Modalità avanzata · Configurazione: Non riuscito (Errore sconosciuto).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Il servizio avanzato e la configurazione residua sono stati eliminati.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Eliminazione servizio richiesta, ma la pulizia residua ha segnalato:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Modalità avanzata: Elimina servizio: Non riuscito:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Modalità avanzata: Elimina servizio: Non riuscito (Errore sconosciuto).</translation></message>
    <message><source>File staging completed</source><translation>File da salvare: Riuscito</translation></message>
    <message><source>File staging failed</source><translation>File da salvare: Non riuscito</translation></message>
    <message><source>All staged targets were processed.</source><translation>Tutte le destinazioni preparate sono state elaborate.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Tutte le destinazioni elaborate; una o più operazioni non sono riuscite.</translation></message>
    <message><source>Show main window</source><translation>Mostra finestra principale</translation></message>
    <message><source>Restore default position</source><translation>Ripristina posizione predefinita</translation></message>
    <message><source>Show overlay hub</source><translation>Mostra HUB overlay</translation></message>
    <message><source>Hide overlay hub</source><translation>Nascondi HUB overlay</translation></message>
    <message><source>Exit application</source><translation>Esci dall’applicazione</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Non riuscito: Connessione al sistema</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Non riuscito: Lettura valori registro (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Elenco file overlay: Errore sconosciuto.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Non riuscito: Azione sul filtro: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Non riuscito: Lettura stato filtro</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Non riuscito: Impostazione soglia di avviso</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Non riuscito: Impostazione soglia critica</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Non riuscito: Lettura stato overlay</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Non riuscito: Impostazione tipo overlay</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Non riuscito: Impostazione dimensione massima</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Non riuscito: Lettura configurazione overlay</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Non riuscito: Registrazione volume %1 in UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Non riuscito: Protezione volume %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Non riuscito: Impostazione identificazione volume %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Non riuscito: Aggiunta esclusione file %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Esclusione file già assente sul volume %1: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Non riuscito: Rimozione esclusione file %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Non riuscito: Lettura configurazione volume</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Non riuscito: Aggiunta esclusione registro %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Non riuscito: Rimozione esclusione registro %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Non riuscito: Aggiornamento opzioni persistenza registro</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Non riuscito: Lettura filtro registro</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Non supportato</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Lettura stato destinazione: Errore sconosciuto.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Il percorso non esiste; non c’è nulla da eliminare.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Errore sconosciuto.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Risultato non confermabile: rilettura dello stato della destinazione non riuscita: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Il provider ha accettato l’eliminazione, ma la verifica autorevole conferma che la destinazione esiste ancora.</translation></message>
    <message><source>Unknown</source><translation>Sconosciuto</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Non riuscito: Caricamento registro (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Non riuscito: Caricamento registro (Errore sconosciuto).</translation></message>
    <message><source>: unknown error</source><translation>: Errore sconosciuto</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Non riuscito: Identificazione volume di destinazione (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Non riuscito: Elenco chiavi registro (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Non riuscito: Identificazione volume di destinazione (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Operazione ignorata perché un’operazione preliminare non è riuscita o non è stato possibile confermarla.</translation></message>
    <message><source>Copy</source><translation>Copia</translation></message>
    <message><source>Select All</source><translation>Seleziona tutto</translation></message>
    <message><source>Display language</source><translation>Lingua di visualizzazione</translation></message>
    <message><source>Folder</source><translation>Cartella</translation></message>
    <message><source>File</source><translation>File</translation></message>
    <message><source>Export commands</source><translation>Esporta comandi</translation></message>
    <message><source>Direct restart…</source><translation>Riavvio diretto…</translation></message>
    <message><source>Clear…</source><translation>Svuota…</translation></message>
    <message><source>Committing</source><translation>Salvataggio</translation></message>
    <message><source> · %1 selected</source><translation> · %1 selezionati</translation></message>
    <message><source>%1 changes</source><translation>%1 modifiche</translation></message>
    <message><source>%1 commands</source><translation>%1 comandi</translation></message>
    <message><source>%1 items</source><translation>%1 elementi</translation></message>
    <message><source>%1 items · %2</source><translation>%1 elementi · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 modifiche in sospeso</translation></message>
    <message><source>%1 records</source><translation>%1 record</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 record · %2 totali</translation></message>


    <message><source>%1 selected</source><translation>%1 selezionati</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 file</translation></message>
    <message><source>32-bit</source><translation>32 bit</translation></message>
    <message><source>64-bit</source><translation>64 bit</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Distribuito secondo la &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. I termini completi sono nel file LICENSE incluso nell’applicazione.&lt;/p&gt;&lt;p&gt;È possibile ridistribuire o modificare il software secondo GPL v3. Il software è fornito così com’è, senza alcuna garanzia.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows disattiva i file di paging, Ripristino configurazione di sistema, SuperFetch, l’indicizzazione dei file, la deframmentazione e l’avvio rapido. Inoltre imposta bootstatuspolicy su ignoreallfailures nel BCD.&lt;/p&gt;&lt;p&gt;È possibile modificare queste impostazioni in seguito. Ad esempio, spostare il file di paging su un volume non protetto prima di riattivare il paging.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Un’istantanea diagnostica di sistema, privilegi e UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>Esiste già un file in questa posizione:
%1

Il salvataggio ne sostituirà il contenuto.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Un gestore grafico per Windows Unified Write Filter.
Gestisci protezione dei volumi, overlay, esclusioni e file da salvare.</translation></message>
    <message><source>About this snapshot</source><translation>Informazioni sull’istantanea</translation></message>
    <message><source>Account</source><translation>Account</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Aggiungi una configurazione UWF da uno script o testo.</translation></message>
    <message><source>Add exclusion</source><translation>Aggiungi esclusione</translation></message>
    <message><source>Add files or folders</source><translation>Aggiungi file o cartelle</translation></message>
    <message><source>Add selected items</source><translation>Aggiungi elementi selezionati</translation></message>
    <message><source>Added</source><translation>Aggiunto</translation></message>
    <message><source>Administrator</source><translation>Amministratore</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Tutte le modifiche sono state salvate su disco e verranno mantenute senza riavviare.</translation></message>
    <message><source>All files (*)</source><translation>Tutti i file (*)</translation></message>
    <message><source>All levels</source><translation>Tutti i livelli</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Un’istantanea diagnostica vuota non significa che l’overlay non contenga modifiche.</translation></message>
    <message><source>Application path</source><translation>Percorso dell’applicazione</translation></message>
    <message><source>Application result</source><translation>Risultato dell’applicazione</translation></message>
    <message><source>Apply changes</source><translation>Applica modifiche</translation></message>
    <message><source>Architecture</source><translation>Architettura</translation></message>
    <message><source>Automatic</source><translation>Automatico</translation></message>
    <message><source>Automatic file staging</source><translation>Salvataggio automatico dei file in attesa</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Salvataggio automatico incompleto</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Salva automaticamente i file idonei in attesa durante l’arresto e il riavvio normali.</translation></message>
    <message><source>Available</source><translation>Disponibile</translation></message>
    <message><source>Back</source><translation>Indietro</translation></message>
    <message><source>Background service</source><translation>Servizio in background</translation></message>
    <message><source>Binding</source><translation>Associazione</translation></message>
    <message><source>By drive letter</source><translation>Per lettera di unità</translation></message>
    <message><source>By volume ID</source><translation>Per ID del volume</translation></message>
    <message><source>Cancel reading</source><translation>Annulla lettura</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Le modifiche già confermate vengono mantenute.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Selezionare la chiave del Registro di sistema da escludere dalla protezione UWF, incluse le relative sottochiavi e i valori.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Scegli dove salvare il file.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Scegli una posizione nell’unità o cartella consentita.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Scegli un’altra cartella o controlla i permessi di accesso e riprova.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Scegli un’altra chiave o controlla i permessi di accesso e riprova.</translation></message>
    <message><source>Clear filters</source><translation>Azzera filtri</translation></message>
    <message><source>Clear logs</source><translation>Cancella log</translation></message>
    <message><source>Clear logs?</source><translation>Cancellare i log?</translation></message>
    <message><source>Collapse</source><translation>Comprimi</translation></message>
    <message><source>Commit changes</source><translation>Conferma modifiche</translation></message>
    <message><source>Commit changes…</source><translation>Conferma modifiche…</translation></message>
    <message><source>Commit complete</source><translation>Conferma completata</translation></message>
    <message><source>Commit file changes?</source><translation>Confermare le modifiche al file?</translation></message>
    <message><source>Commit folder changes?</source><translation>Confermare le modifiche alla cartella?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Confermare le modifiche alla chiave del registro?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Confermare le modifiche al valore del registro?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Conferma i file in attesa prima dell’arresto e del riavvio normali</translation></message>
    <message><source>Compatibility mode</source><translation>Modalità compatibilità</translation></message>
    <message><source>Configuration applied</source><translation>Configurazione applicata</translation></message>
    <message><source>Configuration error</source><translation>Errore di configurazione</translation></message>
    <message><source>Configured</source><translation>Configurato</translation></message>
    <message><source>Confirm before continuing</source><translation>Conferma prima di continuare</translation></message>
    <message><source>Connected and authenticated</source><translation>Connesso e autenticato</translation></message>
    <message><source>Connecting…</source><translation>Connessione…</translation></message>
    <message><source>Contact author</source><translation>Contatta l’autore</translation></message>
    <message><source>Continue editing</source><translation>Continua a modificare</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Continuando il riavvio si perderanno le modifiche dei file che non è stato possibile salvare.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Continuando l’arresto si perderanno le modifiche dei file che non è stato possibile salvare.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Coordina con l’arresto di Windows</translation></message>
    <message><source>Copy all logs</source><translation>Copia tutti i log</translation></message>
    <message><source>Copy full report</source><translation>Copia rapporto completo</translation></message>
    <message><source>Copy name</source><translation>Copia nome</translation></message>
    <message><source>Copy path</source><translation>Copia percorso</translation></message>
    <message><source>Copy selection</source><translation>Copia selezione</translation></message>
    <message><source>Copy version</source><translation>Copia versione</translation></message>
    <message><source>Counting staged files…</source><translation>Conteggio dei file in attesa…</translation></message>
    <message><source>Create</source><translation>Crea</translation></message>
    <message><source>Critical %1 MB</source><translation>Critico %1 MB</translation></message>
    <message><source>Current filter</source><translation>Filtro attuale</translation></message>
    <message><source>Current run</source><translation>Esecuzione attuale</translation></message>
    <message><source>Current selection</source><translation>Selezione attuale</translation></message>
    <message><source>Current usage</source><translation>Utilizzo attuale</translation></message>
    <message><source>Date modified</source><translation>Data di modifica</translation></message>
    <message><source>Debug</source><translation>Debug</translation></message>
    <message><source>Delete and commit this file?</source><translation>Eliminare questo file e confermare l’eliminazione?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Eliminare questa cartella e confermare l’eliminazione?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Eliminare questa chiave del registro e confermare l’eliminazione?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Eliminare questo valore del registro e confermare l’eliminazione?</translation></message>

    <message><source>Direct restart?</source><translation>Riavviare direttamente?</translation></message>
    <message><source>Direct shutdown?</source><translation>Arrestare direttamente?</translation></message>
    <message><source>Discard changes</source><translation>Scarta modifiche</translation></message>
    <message><source>Disk</source><translation>Disco</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Persistenza della chiave segreta di dominio</translation></message>
    <message><source>Done</source><translation>Fatto</translation></message>
    <message><source>Edit commands</source><translation>Modifica comandi</translation></message>
    <message><source>Editable</source><translation>Modificabile</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Attiva il filtro e riavvia per mostrare l’utilizzo reale</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>La modalità avanzata aggiunge un servizio in background che si avvia con Windows. Rimuovi il servizio prima di spostare o eliminare l’applicazione.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>La modalità avanzata non è disponibile. Usa Arresto sicuro o Riavvio sicuro per salvare i file in attesa.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Immetti HKLM\SOFTWARE\… e premi Invio per spostarti</translation></message>
    <message><source>Enter a file name</source><translation>Immetti un nome file</translation></message>
    <message><source>Enter a valid file name.</source><translation>Immetti un nome file valido.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Immetti un nome file o cartella valido.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Immetti un nome cartella valido.</translation></message>
    <message><source>Error</source><translation>Errore</translation></message>
    <message><source>Error details</source><translation>Dettagli dell’errore</translation></message>
    <message><source>Error details and suggestions</source><translation>Dettagli dell’errore e suggerimenti</translation></message>
    <message><source>Expand</source><translation>Espandi</translation></message>
    <message><source>Export list</source><translation>Esporta elenco</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Impossibile cancellare i log: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Impossibile cancellare i log: errore imprevisto.</translation></message>
    <message><source>File name</source><translation>Nome file</translation></message>
    <message><source>File or folder</source><translation>File o cartella</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>Il salvataggio dei file è iniziato e non può essere annullato.</translation></message>
    <message><source>File saving incomplete</source><translation>Salvataggio dei file incompleto</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Il salvataggio dei file in attesa verrà saltato. Le modifiche non confermate nei percorsi protetti andranno perse dopo il riavvio.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Il salvataggio dei file in attesa verrà saltato. Le modifiche non confermate nei percorsi protetti andranno perse dopo l’arresto.</translation></message>
    <message><source>File type</source><translation>Tipo di file</translation></message>
    <message><source>Files and folders</source><translation>File e cartelle</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>È possibile selezionare file e cartelle insieme.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>I file in attesa non verranno salvati.</translation></message>
    <message><source>Filter is disabled</source><translation>Il filtro è disattivato</translation></message>
    <message><source>Fit columns to contents</source><translation>Adatta colonne al contenuto</translation></message>
    <message><source>Folder address</source><translation>Indirizzo cartella</translation></message>
    <message><source>Folder name</source><translation>Nome cartella</translation></message>
    <message><source>Folders</source><translation>Cartelle</translation></message>
    <message><source>Full configuration</source><translation>Configurazione completa</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Le future scritture in questa chiave non saranno protette da UWF.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Le future scritture in questo percorso non saranno protette da UWF.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Esclusione globale del registro · include sottochiavi</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Sali alla cartella superiore o scegli un’altra posizione.</translation></message>
    <message><source>Home</source><translation>Cartella personale</translation></message>
    <message><source>Import configuration</source><translation>Importa configurazione</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>L’importazione aggiorna la bozza della configurazione. Usa Rivedi e applica per scrivere le modifiche nel sistema.</translation></message>
    <message><source>Import results</source><translation>Risultati dell’importazione</translation></message>
    <message><source>Imported</source><translation>Importato</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>Il rapporto elenca i risultati dei comandi per la bozza della configurazione. Seleziona una riga per visualizzare il comando e il risultato completi.</translation></message>
    <message><source>Imported: %1</source><translation>Importato: %1</translation></message>
    <message><source>Information</source><translation>Informazioni</translation></message>
    <message><source>Item details</source><translation>Dettagli elemento</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Il salvataggio di elenchi lunghi di file in attesa può richiedere più tempo.</translation></message>
    <message><source>License and copyright</source><translation>Licenza e copyright</translation></message>
    <message><source>Line</source><translation>Riga</translation></message>
    <message><source>Local disk (%1)</source><translation>Disco locale (%1)</translation></message>
    <message><source>Log level</source><translation>Livello di log</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Lettura dei log non riuscita. Riprova o controlla l’accesso al file di log.</translation></message>
    <message><source>Maximize</source><translation>Ingrandisci</translation></message>
    <message><source>Message details</source><translation>Dettagli messaggio</translation></message>
    <message><source>Minimize</source><translation>Riduci a icona</translation></message>
    <message><source>Module</source><translation>Modulo</translation></message>
    <message><source>More</source><translation>Altro</translation></message>
    <message><source>Move</source><translation>Sposta</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Se UWF protegge l’unità in cui si trova l’applicazione, assicurati prima che i suoi file vengano conservati dopo un riavvio. Il salvataggio permanente di molti file in attesa può prolungare l’arresto o il riavvio.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>È possibile selezionare più file.</translation></message>
    <message><source>Name and location</source><translation>Nome e posizione</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Valore denominato: %1 · solo questo valore</translation></message>
    <message><source>Navigate to registry path</source><translation>Vai al percorso del registro</translation></message>
    <message><source>New folder</source><translation>Nuova cartella</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>I nuovi record appariranno dopo l’aggiornamento.</translation></message>
    <message><source>Next-session configuration</source><translation>Configurazione della prossima sessione</translation></message>
    <message><source>No files to commit</source><translation>Nessun file da confermare</translation></message>
    <message><source>No logs yet</source><translation>Ancora nessun log</translation></message>
    <message><source>No matching items</source><translation>Nessun elemento corrispondente</translation></message>
    <message><source>No matching logs</source><translation>Nessun log corrispondente</translation></message>
    <message><source>No overlay files found</source><translation>Nessun file nell’overlay trovato</translation></message>
    <message><source>No registry key selected</source><translation>Nessuna chiave del registro selezionata</translation></message>
    <message><source>None</source><translation>Nessuno</translation></message>
    <message><source>Not configured</source><translation>Non configurato</translation></message>
    <message><source>Not connected</source><translation>Non connesso</translation></message>
    <message><source>Not installed</source><translation>Non installato</translation></message>
    <message><source>Not written to the system</source><translation>Non scritto nel sistema</translation></message>
    <message><source>Off</source><translation>Disattivato</translation></message>
    <message><source>On</source><translation>Attivato</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Un comando uwfmgr per riga</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Mantieni l’applicazione in una cartella fissa che gli utenti standard non possano modificare. Posizione consigliata:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Solo i file in attesa vengono salvati automaticamente. Le altre modifiche non confermate nei percorsi protetti andranno perse dopo il riavvio.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Solo i file in attesa vengono salvati automaticamente. Le altre modifiche non confermate nei percorsi protetti andranno perse dopo l’arresto.</translation></message>
    <message><source>Open</source><translation>Apri</translation></message>
    <message><source>Open folder</source><translation>Apri cartella</translation></message>
    <message><source>Operating system</source><translation>Sistema operativo</translation></message>
    <message><source>Operation and security</source><translation>Funzionamento e sicurezza</translation></message>
    <message><source>Original command</source><translation>Comando originale</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Soglia critica dell’overlay raggiunta. Salva le modifiche e riavvia in sicurezza.</translation></message>
    <message><source>Overlay maximum size</source><translation>Dimensione massima dell’overlay</translation></message>
    <message><source>Overlay type</source><translation>Tipo di overlay</translation></message>
    <message><source>Overlay usage</source><translation>Utilizzo dell’overlay</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Soglia di avviso dell’overlay raggiunta. Salva le modifiche e riavvia in sicurezza.</translation></message>
    <message><source>Overview</source><translation>Panoramica</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Incolla i comandi o caricali da un file…</translation></message>
    <message><source>Pending changes</source><translation>Modifiche in sospeso</translation></message>
    <message><source>Pending removal</source><translation>Rimozione in sospeso</translation></message>
    <message><source>Permissions and protection</source><translation>Autorizzazioni e protezione</translation></message>
    <message><source>Privileges</source><translation>Privilegi</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>Elaborati %1 / %2 elementi</translation></message>
    <message><source>Processing result</source><translation>Risultato dell’elaborazione</translation></message>
    <message><source>Protected</source><translation>Protetto</translation></message>
    <message><source>Protected volumes</source><translation>Volumi protetti</translation></message>
    <message><source>Protection and overlay</source><translation>Protezione e overlay</translation></message>
    <message><source>Quick access</source><translation>Accesso rapido</translation></message>
    <message><source>Raw report</source><translation>Rapporto grezzo</translation></message>
    <message><source>Read again</source><translation>Leggi di nuovo</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Leggi di nuovo per ottenere un’istantanea diagnostica.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Leggi la configurazione per mostrare l’utilizzo reale</translation></message>
    <message><source>Read-only</source><translation>Sola lettura</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Leggere un overlay grande può richiedere più tempo. È possibile annullare in qualsiasi momento.</translation></message>
    <message><source>Reading canceled</source><translation>Lettura annullata</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Lettura di file e sottocartelle in questa posizione…</translation></message>
    <message><source>Reading folder</source><translation>Lettura cartella</translation></message>
    <message><source>Reading logs</source><translation>Lettura log</translation></message>
    <message><source>Reading overlay</source><translation>Lettura overlay</translation></message>
    <message><source>Ready</source><translation>Pronto</translation></message>
    <message><source>Ready to save</source><translation>Pronto per salvare</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>I record dell’esecuzione attuale appariranno qui al termine della lettura.</translation></message>
    <message><source>Recursive staging</source><translation>Preparazione ricorsiva</translation></message>
    <message><source>Refresh folder</source><translation>Aggiorna cartella</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Chiave del registro o impostazione di persistenza</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>È possibile annullare le attività rimanenti al termine della destinazione attuale.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Rimuovi questa chiave dalle esclusioni della prossima sessione.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Rimuovi questo percorso dalle esclusioni della prossima sessione.</translation></message>
    <message><source>Removed</source><translation>Rimosso</translation></message>
    <message><source>Rename</source><translation>Rinomina</translation></message>
    <message><source>Rename…</source><translation>Rinomina…</translation></message>
    <message><source>Repair required</source><translation>Riparazione necessaria</translation></message>
    <message><source>Replace file</source><translation>Sostituisci file</translation></message>
    <message><source>Replace the existing file?</source><translation>Sostituire il file esistente?</translation></message>
    <message><source>Required configuration</source><translation>Configurazione necessaria</translation></message>
    <message><source>Restart anyway</source><translation>Riavvia comunque</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Il riavvio proseguirà al termine del salvataggio. Mantieni aperta l’applicazione.</translation></message>
    <message><source>Restore</source><translation>Ripristina</translation></message>
    <message><source>Retry</source><translation>Riprova</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Esamina i dettagli degli errori prima di continuare l’arresto.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Esamina i dettagli degli errori prima di riavviare.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Esamina i dettagli dell’operazione. Leggi lo stato attuale del sistema prima di riprovare modifiche che potrebbero essere state scritte.</translation></message>
    <message><source>Review the result for each target.</source><translation>Controlla il risultato per ogni destinazione.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Controlla queste modifiche prima di scriverle nel sistema.</translation></message>
    <message><source>Running</source><translation>In esecuzione</translation></message>
    <message><source>Save</source><translation>Salva</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Salva i file in attesa su disco e poi riavvia.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Salva i file in attesa su disco e poi arresta.</translation></message>
    <message><source>Saving staged files</source><translation>Salvataggio dei file in attesa</translation></message>
    <message><source>Saving to disk</source><translation>Salvataggio su disco</translation></message>
    <message><source>Saving…</source><translation>Salvataggio…</translation></message>
    <message><source>Scanning staged paths</source><translation>Scansione dei percorsi in attesa</translation></message>
    <message><source>Search logs</source><translation>Cerca nei log</translation></message>
    <message><source>Search messages or modules</source><translation>Cerca messaggi o moduli</translation></message>
    <message><source>Search paths</source><translation>Cerca percorsi</translation></message>
    <message><source>Search this folder</source><translation>Cerca in questa cartella</translation></message>
    <message><source>Select a file or enter its name</source><translation>Seleziona un file o immetti il nome</translation></message>
    <message><source>Select a file to open.</source><translation>Seleziona un file da aprire.</translation></message>
    <message><source>Select a folder.</source><translation>Seleziona una cartella.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Seleziona una chiave per operare sul suo intero sottoalbero oppure un valore denominato per operare solo su quel valore.</translation></message>
    <message><source>Select an available subkey</source><translation>Seleziona una sottochiave disponibile</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Seleziona solo file o apri una cartella per esplorarne il contenuto.</translation></message>
    <message><source>Select files or folders</source><translation>Seleziona file o cartelle</translation></message>
    <message><source>Select folder</source><translation>Seleziona cartella</translation></message>
    <message><source>Select folders only.</source><translation>Seleziona solo cartelle.</translation></message>
    <message><source>Select registry key or value</source><translation>Seleziona chiave o valore del registro</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Seleziona chiave o valore del registro da eliminare</translation></message>
    <message><source>Select the current folder</source><translation>Seleziona la cartella attuale</translation></message>
    <message><source>Select this file</source><translation>Seleziona questo file</translation></message>
    <message><source>Select this registry key</source><translation>Seleziona questa chiave del registro</translation></message>
    <message><source>Select this registry value</source><translation>Seleziona questo valore del registro</translation></message>
    <message><source>Selected file</source><translation>File selezionato</translation></message>
    <message><source>Selected folder</source><translation>Cartella selezionata</translation></message>
    <message><source>Selection</source><translation>Selezione</translation></message>
    <message><source>Service account</source><translation>Account del servizio</translation></message>
    <message><source>Service configuration needs repair</source><translation>La configurazione del servizio richiede una riparazione</translation></message>
    <message><source>Service details</source><translation>Dettagli del servizio</translation></message>
    <message><source>Service name</source><translation>Nome del servizio</translation></message>
    <message><source>Service status</source><translation>Stato del servizio</translation></message>
    <message><source>Service stopped</source><translation>Servizio arrestato</translation></message>
    <message><source>Shut down anyway</source><translation>Arresta comunque</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>L’arresto proseguirà al termine del salvataggio. Mantieni aperta l’applicazione.</translation></message>
    <message><source>Size</source><translation>Dimensione</translation></message>
    <message><source>Some changes could not be applied</source><translation>Impossibile applicare alcune modifiche</translation></message>
    <message><source>Some files could not be saved</source><translation>Impossibile salvare alcuni file</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Impossibile leggere alcune informazioni hardware. Le altre informazioni diagnostiche rimangono disponibili e copiabili.</translation></message>
    <message><source>Source code</source><translation>Codice sorgente</translation></message>
    <message><source>Staged file</source><translation>File in attesa</translation></message>
    <message><source>Staged path</source><translation>Percorso in attesa</translation></message>
    <message><source>Standard user</source><translation>Utente standard</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Avvio automatico con Windows</translation></message>
    <message><source>Startup type</source><translation>Tipo di avvio</translation></message>
    <message><source>Status unavailable</source><translation>Stato non disponibile</translation></message>
    <message><source>Stopped</source><translation>Arrestato</translation></message>
    <message><source>Storage</source><translation>Archiviazione</translation></message>
    <message><source>System</source><translation>Sistema</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Modifiche al sistema alla prima attivazione di UWF</translation></message>
    <message><source>System memory</source><translation>Memoria di sistema</translation></message>
    <message><source>System metadata</source><translation>Metadati di sistema</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>I metadati di sistema sono solo diagnostici e non possono essere confermati.</translation></message>
    <message><source>Takes effect after restart</source><translation>Ha effetto dopo il riavvio</translation></message>
    <message><source>Target</source><translation>Destinazione</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistenza delle licenze Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Il provider UWF non ha completato l’enumerazione. Può accadere con un overlay grande o risorse di sistema insufficienti.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>La configurazione è stata scritta nel sistema. La maggior parte delle modifiche ha effetto dopo il riavvio.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>La configurazione corrisponde alla destinazione della prossima sessione. Visualizza o esporta i comandi in Configurazione completa.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>I record attuali dei log verranno cancellati e non potranno essere recuperati.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>La cartella di destinazione non esiste.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Impossibile leggere lo stato del servizio della modalità avanzata. Controlla le autorizzazioni del servizio e riprova.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>L’elenco può omettere file o includere file già confermati. Non usarlo come elenco di conferma.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>L’elemento selezionato non esiste più: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Il servizio è installato ma arrestato. Avvialo per ripristinare il salvataggio automatico.</translation></message>
    <message><source>This PC</source><translation>Questo PC</translation></message>
    <message><source>This folder is empty</source><translation>Questa cartella è vuota</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Questo rapporto può contenere nomi di account, identificativi dei dispositivi e percorsi di file. Controllalo prima di condividerlo.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Salvare questo script CMD UTF-8 come file .cmd ed eseguirlo nel prompt dei comandi. L’esportazione usa lo stesso script.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Prova un’altra parola chiave o un altro livello di log.</translation></message>
    <message><source>Try another search term.</source><translation>Prova un altro termine di ricerca.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Tipo e dimensione massima non possono essere modificati con il filtro attivo. Disattiva il filtro e riavvia prima.</translation></message>
    <message><source>UI agent</source><translation>Agente dell’interfaccia</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 righe</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>Impossibile leggere la configurazione UWF. Verifica che Unified Write Filter sia installato in Windows e leggi di nuovo.</translation></message>
    <message><source>UWF feature</source><translation>Funzionalità UWF</translation></message>
    <message><source>UWF filter</source><translation>Filtro UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF non è attualmente disponibile</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Il riavvio sicuro UWF verrà eseguito direttamente.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>L’arresto sicuro UWF verrà eseguito direttamente.</translation></message>
    <message><source>Unable to access this folder</source><translation>Impossibile accedere a questa cartella</translation></message>
    <message><source>Unable to access this registry key</source><translation>Impossibile accedere a questa chiave del registro</translation></message>
    <message><source>Unable to read logs</source><translation>Impossibile leggere i log</translation></message>
    <message><source>Unable to read service status</source><translation>Impossibile leggere lo stato del servizio</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Impossibile leggere l’overlay al momento</translation></message>
    <message><source>Unable to read this folder.</source><translation>Impossibile leggere questa cartella.</translation></message>
    <message><source>Unable to select this item</source><translation>Impossibile selezionare questo elemento</translation></message>
    <message><source>Unavailable</source><translation>Non disponibile</translation></message>
    <message><source>Undo removal</source><translation>Annulla rimozione</translation></message>
    <message><source>Unprotected</source><translation>Non protetto</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Gli elementi non riusciti non sono stati salvati. Seleziona una riga per vedere il percorso completo e il motivo.</translation></message>
    <message><source>Up one folder</source><translation>Cartella superiore</translation></message>
    <message><source>Usage unavailable</source><translation>Utilizzo non disponibile</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Usa l’intera chiave, incluse tutte le sottochiavi</translation></message>
    <message><source>Version %1</source><translation>Versione %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Visualizza i record dell’applicazione e le informazioni diagnostiche.</translation></message>
    <message><source>View commands for these changes</source><translation>Visualizza i comandi per queste modifiche</translation></message>
    <message><source>View failure details</source><translation>Visualizza i dettagli degli errori</translation></message>
    <message><source>View overlay</source><translation>Visualizza overlay</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Visualizza l’istantanea diagnostica dei file per %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Associazione del volume %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Protezione del volume %1</translation></message>
    <message><source>Volume identity</source><translation>Identità del volume</translation></message>
    <message><source>Warning</source><translation>Avviso</translation></message>
    <message><source>Warning %1 MB</source><translation>Avviso %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Intera chiave e tutte le sottochiavi · ricorsivo</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF fornisce la protezione da scrittura. Questa applicazione la configura e gestisce.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Senza modalità avanzata, usa Arresto sicuro o Riavvio sicuro in questa applicazione per salvare i file in attesa.</translation></message>
    <message><source>Write protection</source><translation>Protezione da scrittura</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Scrivere queste modifiche di configurazione nel sistema? La maggior parte ha effetto dopo il riavvio.</translation></message>
    <message><source>staged files</source><translation>file in attesa</translation></message>
    <message><source>uwfmgr commands</source><translation>comandi uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Contrassegnare i percorsi selezionati per la rimozione. Ha effetto dopo l’applicazione.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Rimuovere dalla preparazione automatica dei file dopo l’applicazione.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Aggiungere alla preparazione automatica dei file dopo l’applicazione.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>L’applicazione di questo piano svuota l’elenco di preparazione salvato e rimuove la relativa esclusione del Registro se non è più necessaria.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Le modifiche alla preparazione dei file devono essere applicate in UWF Manager e non possono essere esportate come comandi uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>La preparazione dei file viene salvata dopo l’applicazione</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Ambito dell’istantanea dell’overlay</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Questo elenco serve solo alla diagnostica. Non consente di stabilire quali file devono essere confermati.</translation></message>
    <message><source>NTFS volumes only</source><translation>Solo volumi NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>La lettura di overlay grandi può essere lenta o non riuscire per risorse insufficienti.</translation></message>
    <message><source>Small files may be missing</source><translation>Potrebbero mancare file piccoli</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>I file più piccoli della dimensione del cluster del disco (in genere 4 KB) potrebbero non comparire nell’elenco.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Non indica conferme in sospeso</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Possono comparire anche file già confermati, percorsi esclusi ed elementi interessati da altre operazioni.</translation></message>

    <message><source>Command processing result</source><translation>Risultato dell’elaborazione del comando</translation></message>
    <message><source>Copy command</source><translation>Copia comando</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 rami non leggibili</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>Impossibile leggere %1 rami. Continua con il contenuto rilevato oppure riprova solo gli elementi non riusciti.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 file · %2 cartelle</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 valori · %2 chiavi</translation></message>
    <message><source>Attempts</source><translation>Tentativi</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Annulla le attività rimanenti</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Conferma solo il contenuto esplorato correttamente. I rami non leggibili restano invariati.</translation></message>
    <message><source>Committing changes</source><translation>Conferma delle modifiche</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Le eliminazioni e le conferme completate sono irreversibili.</translation></message>
    <message><source>Continue…</source><translation>Continua…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Elimina e conferma solo le destinazioni rilevate. I rami non leggibili e le relative cartelle o chiavi superiori vengono conservati.</translation></message>
    <message><source>Deleting and committing</source><translation>Eliminazione e conferma</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>L’eliminazione e la conferma sono irreversibili. Viene elaborato solo l’ambito selezionato; la configurazione UWF in sospeso non viene applicata.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Determinazione dell’ambito. I rami non leggibili vengono registrati e l’esplorazione prosegue altrove.</translation></message>
    <message><source>Export traversal records</source><translation>Esporta il registro di esplorazione</translation></message>
    <message><source>No actionable content</source><translation>Nessun contenuto elaborabile</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Nessuna destinazione elaborabile trovata. Non è stato eliminato né confermato nulla.</translation></message>
    <message><source>No changes have been committed.</source><translation>Nessuna modifica confermata</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Nessuna eliminazione o conferma eseguita</translation></message>
    <message><source>Path</source><translation>Percorso</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Elimina definitivamente l’ambito selezionato e salva le eliminazioni nel volume protetto.</translation></message>
    <message><source>Preparation complete</source><translation>Preparazione completata</translation></message>
    <message><source>Preparing commit</source><translation>Preparazione della conferma</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Preparazione di eliminazione e conferma</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Elabora solo il contenuto esplorato correttamente</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>I punti di analisi vengono ignorati per mantenere l’esplorazione nell’ambito selezionato.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Un riavvio non annulla una conferma. Viene elaborato solo l’ambito selezionato; la configurazione UWF in sospeso non viene applicata.</translation></message>
    <message><source>Retry failed items</source><translation>Riprova gli elementi non riusciti</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Salva le modifiche dell’ambito selezionato nel volume protetto.</translation></message>
    <message><source>Stopping the operation</source><translation>Arresto dell’operazione</translation></message>
    <message><source>Successful and failed records are retained</source><translation>I risultati riusciti e non riusciti sono conservati</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>La cartella non è vuota; il contenuto non elaborato è stato conservato.</translation></message>
    <message><source>The operation could not be completed</source><translation>Impossibile completare l’operazione</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>La chiave del Registro contiene nuovi valori; il contenuto non elaborato è stato conservato.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>La chiave del Registro contiene ancora sottochiavi; il contenuto non elaborato è stato conservato.</translation></message>
    <message><source>The target no longer exists.</source><translation>La destinazione non esiste più.</translation></message>
    <message><source>Traversal complete</source><translation>Esplorazione completata</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Esplorazione completata con contenuti non leggibili</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Il registro di esplorazione viene conservato e può essere esportato.</translation></message>
    <message><source>Traverse again</source><translation>Esplora di nuovo</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Esplora di nuovo oppure chiudi questa finestra.</translation></message>
    <message><source>Traversing the folder</source><translation>Esplorazione della cartella</translation></message>
    <message><source>Traversing the registry</source><translation>Esplorazione del Registro</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Impossibile preparare l’operazione</translation></message>
    <message><source>View results</source><translation>Visualizza risultati</translation></message>

    <message><source>All changes committed</source><translation>Tutte le modifiche sono state salvate</translation></message>
    <message><source>Some changes committed</source><translation>Alcune modifiche sono state salvate</translation></message>
    <message><source>No changes could be committed</source><translation>Impossibile salvare le modifiche</translation></message>
    <message><source>No changes to commit</source><translation>Nessuna modifica da salvare</translation></message>
    <message><source>Commit interrupted</source><translation>Salvataggio interrotto</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 elementi non sono stati elaborati.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 operazioni del Registro non riuscite.</translation></message>
    <message><source>The folder does not exist.</source><translation>La cartella non esiste.</translation></message>
    <message><source>Access denied.</source><translation>Accesso negato.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Esiste già un elemento con questo nome.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Impossibile creare la cartella. Controllare il nome e le autorizzazioni.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Impossibile rinominare l’elemento. Controllare il nome e le autorizzazioni.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Questo browser non supporta la ridenominazione.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>La bozza dei file da salvare è cambiata dopo l’apertura di questo piano. Esamina un nuovo piano.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>L’elenco salvato dei file da salvare è cambiato dopo l’apertura di questo piano. Aggiorna ed esamina le modifiche.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>L’elenco salvato dei file da salvare non corrisponde al piano accettato.</translation></message>
    <message><source>Export configuration</source><translation>Esporta configurazione</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Salva il tema, la lingua e la visibilità attuali dell’HUB in UWF.ini accanto a questa applicazione.</translation></message>
    <message><source>Configuration saved</source><translation>Configurazione salvata</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Le impostazioni attuali dell’interfaccia sono state salvate in:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini esiste già. Il file esistente non è stato modificato:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>Impossibile salvare UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Desktop</translation></message>
    <message><source>Downloads</source><translation>Download</translation></message>
    <message><source>Documents</source><translation>Documenti</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Il salvataggio dei file è stato interrotto. Risolvere l&apos;errore prima di riprovare.</translation></message>
    <message><source>No net change for this setting</source><translation>Il valore finale di questa impostazione non è cambiato dopo l’importazione</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Valore finale della bozza: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Valore finale della bozza: %1 → %2 (valore richiesto: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Valore richiesto: %1 → %2; sostituito da un comando successivo. Valore finale della bozza → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Bozza di %1 aggiornata</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Bozza della protezione del volume %1 aggiornata → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Seleziona le voci da rimuovere.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Al momento non è possibile ripristinare le voci selezionate.</translation></message>
</context>
</TS>
