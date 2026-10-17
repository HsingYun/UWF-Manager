<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>Attiva</translation></message>
    <message><source>Disable</source><translation>Disattiva</translation></message>
    <message><source>enable</source><translation>attivare</translation></message>
    <message><source>disable</source><translation>disattivare</translation></message>
    <message><source>Enabled</source><translation>Attivo</translation></message>
    <message><source>Disabled</source><translation>Disattivo</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Annulla</translation></message>
    <message><source>Close</source><translation>Chiudi</translation></message>
    <message><source>Refresh</source><translation>Aggiorna</translation></message>
    <message><source>Apply</source><translation>Applica</translation></message>
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
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>Il sistema «%1» (%2) non è riconosciuto come edizione Windows 10/11 supportata. UWF Manager usa la modalità compatibilità; alcune funzioni potrebbero non essere disponibili.</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Stato di protezione del volume nella sessione corrente (sola lettura).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Protegge il volume nella prossima sessione. Le scritture vengono reindirizzate all’overlay e scartate al riavvio.</translation></message>
    <message><source>Protection:</source><translation>Protezione:</translation></message>
    <message><source>Current session</source><translation>Sessione corrente</translation></message>
    <message><source>Next session</source><translation>Prossima sessione</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>Sessione attiva (sola lettura). Le modifiche non hanno effetto in questa sessione.</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>Sessione successiva al riavvio. Le modifiche hanno effetto dopo il riavvio del sistema.</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identificazione del volume: la lettera è semplice, ma il collegamento si perde se viene riassegnata, ad esempio aggiungendo o rimuovendo dischi. L’ID volume rimane stabile.</translation></message>
    <message><source>Bind by:</source><translation>Identifica tramite:</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Volume non supportato da UWF: </translation></message>
    <message><source>Global settings</source><translation>Impostazioni globali</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Stato del filtro UWF nella sessione corrente (sola lettura).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Attiva UWF nella prossima sessione. Le scritture sui volumi protetti vanno nell’overlay e vengono scartate al riavvio.</translation></message>
    <message><source>Filter</source><translation>Filtro</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>A questo livello d’uso registra un avviso nel registro eventi. Deve essere inferiore alla soglia critica. 0 disabilita l’evento.</translation></message>
    <message><source>Critical threshold</source><translation>Soglia critica</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>A questo livello d’uso registra un evento critico. Deve essere superiore alla soglia di avviso. 0 disabilita l’evento.</translation></message>
    <message><source>Used / total</source><translation>Utilizzato / totale</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; Utilizzato &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Avviso &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critico</translation></message>
    <message><source>UWF status unavailable: </source><translation>Stato UWF non disponibile: </translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>Per modificare UWF servono privilegi amministrativi. Riavvia il programma con clic destro → «Esegui come amministratore».</translation></message>
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
    <message><source>Search staged paths…</source><translation>Cerca percorsi da salvare…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Aggiungi file o una cartella all’elenco di salvataggio automatico.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Seleziona file da salvare su disco. Salvataggio automatico prima di un arresto o riavvio sicuro.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Seleziona cartella da salvare su disco. Salvataggio automatico prima di un arresto o riavvio sicuro. Sono incluse tutte le sottocartelle.</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>Rimuovi subito i percorsi selezionati dal salvataggio automatico.</translation></message>
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
    <message><source>Required by UWF for File staging.</source><translation>Necessario per il salvataggio automatico UWF.</translation></message>
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
    <message><source>Commit registry changes</source><translation>Salva modifiche del registro</translation></message>
    <message><source>Commit registry deletion</source><translation>Salva eliminazione dal registro</translation></message>
    <message><source>Key path:</source><translation>Chiave · Percorso:</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>Digita o incolla HKLM\Software\... e premi Invio</translation></message>
    <message><source>Name</source><translation>Nome</translation></message>
    <message><source>Data</source><translation>Dati</translation></message>
    <message><source>Key: %1</source><translation>Chiave: %1</translation></message>
    <message><source>Value: %1</source><translation>Valore: %1</translation></message>
    <message><source>(none)</source><translation>(nessuno)</translation></message>
    <message><source>(Default)</source><translation>(Predefinito)</translation></message>
    <message><source>(value not set)</source><translation>(valore non impostato)</translation></message>
    <message><source>(empty)</source><translation>(vuoto)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Sconosciuto (Valore: File troppo grande)</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>(intera chiave e tutte le sottochiavi)</translation></message>
    <message><source>Overlay files - %1</source><translation>Overlay · file - %1</translation></message>
    <message><source>View overlay files</source><translation>Visualizza file overlay</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Visualizza file overlay. &lt;b&gt;Solo istantanea diagnostica.&lt;/b&gt; Non usare l’elenco per decidere cosa salvare.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>L’elenco dei file nell’overlay è disponibile solo sui volumi NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>Il filtro UWF è disattivato; nessun volume ha un overlay da esaminare.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Il volume non è protetto da UWF e non ha overlay da esaminare.</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;Solo istantanea diagnostica.&lt;/b&gt; Non usare l’elenco per decidere cosa salvare.</translation></message>
    <message><source>NTFS volumes only.</source><translation>L’elenco dei file nell’overlay è disponibile solo sui volumi NTFS.</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>Un overlay grande richiede più memoria e tempo; l’operazione può essere lenta o fallire.</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>L’elenco non è esatto: possono mancare file sotto la dimensione del cluster (di solito 4 KB) e comparire file già salvati, esclusi o coinvolti in altre operazioni.</translation></message>
    <message><source>Loading…</source><translation>Caricamento…</translation></message>
    <message><source>Failed: %1</source><translation>Non riuscito: %1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>Overlay · file: %1</translation></message>
    <message><source>No files</source><translation>Nessun file</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>Pagina %1 / %2 · file: %3</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Il provider WMI si è arrestato durante l’elenco dei file. È un’instabilità nota di UWF_Overlay.GetOverlayFiles con overlay grandi o carichi I/O elevati.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Memoria insufficiente o operazione non supportata. Servono NTFS e memoria libera. Riprova con un overlay più piccolo o chiudi applicazioni che usano molta memoria.</translation></message>
    <message><source>Raw error: %1</source><translation>Codice errore: %1</translation></message>
    <message><source>Export to file…</source><translation>Esporta su file…</translation></message>
    <message><source>Export overlay file list</source><translation>Esporta su file… · Overlay · file</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>File di testo (*.txt);;Tutti i file (*)</translation></message>
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
    <message><source>Search file paths…</source><translation>Cerca percorsi file…</translation></message>
    <message><source>Search registry keys…</source><translation>Cerca chiavi registro…</translation></message>
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
    <message><source>· %1 persistence %2</source><translation>· Persistenza %1: %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistenza %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Rimuovi selezionati</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Rimuovi le esclusioni selezionate. Ha effetto dopo l’applicazione.</translation></message>
    <message><source>Open containing folder</source><translation>Apri cartella contenitore</translation></message>
    <message><source>Copy file path</source><translation>Copia percorso file</translation></message>
    <message><source>Copy registry path</source><translation>Copia percorso registro</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Salva su disco le modifiche della cartella nell’overlay…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Salva su disco le modifiche del file nell’overlay…</translation></message>
    <message><source>Copied to clipboard: </source><translation>Copiato negli appunti: </translation></message>
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
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>voci: %1 · Da aggiungere: %2 · Da rimuovere nella prossima sessione: %3 · Modifiche in sospeso: %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Gestore del filtro di scrittura unificato (UWF)</translation></message>
    <message><source>Main toolbar</source><translation>Barra strumenti principale</translation></message>
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
    <message><source>About this program.</source><translation>Informazioni UWF Manager.</translation></message>
    <message><source>Switch display language</source><translation>Cambia lingua di visualizzazione</translation></message>
    <message><source>Language</source><translation>Lingua</translation></message>
    <message><source>Theme</source><translation>Tema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Passa tra tema chiaro e scuro</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Interfaccia grafica per gestire filtro UWF, overlay ed esclusioni di file e registro. La maggior parte delle modifiche ha effetto al prossimo riavvio.&lt;/p&gt;&lt;p&gt;Codice sorgente: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;Licenza: &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; Il testo completo della licenza è nel file LICENSE incluso nel programma.&lt;/p&gt;&lt;p&gt;Software libero: puoi ridistribuirlo e modificarlo secondo la GPL v3. Fornito «così com’è», senza alcuna garanzia.&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>Informazioni UWF Manager</translation></message>
    <message><source>System information</source><translation>Informazioni di sistema</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Istantanea diagnostica</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>Diagnostica di sistema, sicurezza, UWF, modalità avanzata e registri recenti.</translation></message>
    <message><source>Read only</source><translation>Sola lettura</translation></message>
    <message><source>Diagnostic report</source><translation>Rapporto diagnostico</translation></message>
    <message><source>Copied</source><translation>Copiato</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>Diagnostica di sistema, sicurezza, UWF, modalità avanzata e registri recenti.</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>Il rapporto può contenere nomi account, identificativi dispositivi, percorsi e registri recenti. Controllalo prima di condividerlo.</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;Questo programma dipende dal filtro di scrittura unificato Windows (UWF).&lt;/b&gt; La protezione è fornita da UWF integrato in Windows. UWF Manager configura e gestisce UWF, ma non filtra direttamente le scritture. UWF deve essere installato e attivo nel sistema.&lt;/p&gt;&lt;p&gt;Alla prima attivazione di UWF vengono apportate queste modifiche per migliorarne le prestazioni:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;I file di paging vengono disabilitati.&lt;/li&gt;&lt;li&gt;Il ripristino di sistema viene disabilitato.&lt;/li&gt;&lt;li&gt;SuperFetch viene disabilitato.&lt;/li&gt;&lt;li&gt;Il servizio di indicizzazione file viene disattivato.&lt;/li&gt;&lt;li&gt;Il servizio di deframmentazione viene disattivato.&lt;/li&gt;&lt;li&gt;L’avvio rapido viene disabilitato.&lt;/li&gt;&lt;li&gt;L'impostazione BCD bootstatuspolicy diventa ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;Dopo l’attivazione puoi modificare queste impostazioni, ad esempio spostando il file di paging su un volume non protetto e riattivando il paging.&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Destinazione non trovata; nell’overlay non c’è nulla da salvare.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>La destinazione esiste solo nell’overlay: creata sotto UWF e mai salvata sul disco o nel registro persistente. Sparirà al riavvio; salvare l’eliminazione non è necessario né possibile.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Il sistema ha rifiutato un parametro (percorso o argomento non valido).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Operazione non riuscita. La destinazione potrebbe essere in uso, ad esempio in Esplora file o in un file aperto. Chiudi i programmi interessati e riprova.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Operazione non riuscita; consulta il registro per i dettagli.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operazione rifiutata (codice %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Sconosciuto.</translation></message>
    <message><source>Commit canceled</source><translation>Salvataggio annullato</translation></message>
    <message><source>Commit result</source><translation>Risultato del salvataggio</translation></message>
    <message><source>Succeeded</source><translation>Riuscito</translation></message>
    <message><source>Skipped</source><translation>Ignorato</translation></message>
    <message><source>Failed</source><translation>Non riuscito</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Riuscito: %1; Ignorato: %2; Non riuscito: %3.</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
Annullato dall’utente; %1 voci non elaborate.</translation></message>
    <message><source>Category</source><translation>Categoria</translation></message>
    <message><source>Path</source><translation>Percorso</translation></message>
    <message><source>Existed before</source><translation>Presente prima</translation></message>
    <message><source>Exists after</source><translation>Presente dopo</translation></message>
    <message><source>Error code</source><translation>Codice errore</translation></message>
    <message><source>Reason</source><translation>Motivo</translation></message>
    <message><source>Previous page</source><translation>Pagina precedente</translation></message>
    <message><source>First page</source><translation>Prima pagina</translation></message>
    <message><source>Last page</source><translation>Ultima pagina</translation></message>
    <message><source>Next page</source><translation>Pagina successiva</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>Pagina %1 / %2 · voci: %3</translation></message>
    <message><source>Copy selected rows</source><translation>Copia righe selezionate</translation></message>
    <message><source>Copy all</source><translation>Copia tutto</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Questa esclusione del registro è necessaria finché l’elenco contiene file da salvare. File da salvare: Svuota → Rimozione esclusione registro.</translation></message>
    <message><source>0 lines</source><translation>0 righe</translation></message>
    <message><source>%1 lines</source><translation>%1 righe</translation></message>
    <message><source>Loading log entries…</source><translation>Caricamento registro…</translation></message>
    <message><source>No log entries</source><translation>Nessuna voce nel registro</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>Pagina %1 / %2 · righe: %3</translation></message>
    <message><source>Copy current page</source><translation>Copia pagina corrente</translation></message>
    <message><source>Clear</source><translation>Svuota</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>Modifiche in sospeso: %1</translation></message>
    <message><source>No pending changes</source><translation>Nessuna modifica in sospeso</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Esclusioni registro · Impostazioni globali.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: Protezione UWF / Esclusioni file / File da salvare.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Non supportato</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>Aggiorna · Volume: %1</translation></message>
    <message><source>· Filter (global) %1</source><translation>· Filtro (UWF): %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· Overlay · Tipo → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· Overlay · Dimensione massima → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· Overlay · Soglia di avviso → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· Overlay · Soglia critica → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ Per modificare tipo e dimensione massima:
1. Disattiva il filtro qui sopra.
2. Riavvia per renderlo inattivo.
3. Modifica l’impostazione.</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· Volume %1 · Protezione UWF: %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· Volume %1 · Identifica tramite: %2 (nessun equivalente CLI; solo in questo programma)</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ Esclusioni file  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− Esclusioni file  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ Esclusioni registro  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− Esclusioni registro  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>Filtro (UWF): %1</translation></message>
    <message><source>Overlay type → %1</source><translation>Overlay · Tipo → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>Overlay · Dimensione massima → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>Overlay · Soglia di avviso → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>Overlay · Soglia critica → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>Volume %1 · Protezione UWF: %2</translation></message>
    <message><source>File exclusion %1</source><translation>Esclusioni file: %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>Esclusioni registro: %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>Modifiche in sospeso (%1)</translation></message>
    <message><source>Current session configuration</source><translation>Configurazione della sessione corrente</translation></message>
    <message><source>Review and apply changes</source><translation>Verifica e applica le modifiche</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>Configurazione completa come comandi uwfmgr. Le modifiche in sospeso sono mostrate prima in una sezione separata. &lt;span style='color:%1'&gt;Modifiche in sospeso&lt;/span&gt;. &lt;span style='color:%2'&gt;Applica&lt;/span&gt;: Fai clic per scrivere le modifiche nel sistema. La maggior parte delle modifiche ha effetto al prossimo riavvio.</translation></message>
    <message><source>Confirm apply</source><translation>Conferma applicazione</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>&lt;span style='color:%1'&gt;Queste modifiche verranno scritte nel sistema&lt;/span&gt;. La maggior parte delle modifiche ha effetto al prossimo riavvio.&lt;br&gt;&lt;br&gt;Continua?</translation></message>
    <message><source>Applied changes</source><translation>Modifiche applicate</translation></message>
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
    <message><source>Confirm safe shutdown?</source><translation>Arresto sicuro?</translation></message>
    <message><source>Confirm safe restart?</source><translation>Riavvio sicuro?</translation></message>
    <message><source>Shut down</source><translation>Arresta</translation></message>
    <message><source>Restart</source><translation>Riavvia</translation></message>
    <message><source>Continue shutdown</source><translation>Continua arresto</translation></message>
    <message><source>Continue restart</source><translation>Continua riavvio</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>Il sistema si arresterà in sicurezza tramite UWF.</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>Il sistema si riavvierà in sicurezza tramite UWF.</translation></message>
    <message><source>UWF protection</source><translation>Protezione UWF</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>Disponibile anche con overlay UWF pieno.</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Calcolo dei file da salvare automaticamente…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>Prima dell’operazione saranno salvati automaticamente %1 file. Quelli fuori dai volumi attualmente protetti vengono ignorati.</translation></message>
    <message><source>Committing staged files</source><translation>Salvataggio dei file preparati</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>L’operazione di alimentazione proseguirà dopo il salvataggio automatico.</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>Elaborati %1 file su %2.</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>Salvataggio automatico incompleto</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>Controlla i dettagli prima di decidere se continuare.</translation></message>
    <message><source>Operation details</source><translation>Dettagli operazione</translation></message>
    <message><source>No additional error details were provided.</source><translation>Nessun dettaglio aggiuntivo sull’errore.</translation></message>
    <message><source>Some changes may not be preserved</source><translation>Alcune modifiche potrebbero non essere conservate</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>Continuando, il sistema si arresterà senza conservare i file non salvati.</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>Continuando, il sistema si riavvierà senza conservare i file non salvati.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Salvataggio automatico interrotto in modo imprevisto:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Salvataggio automatico incompleto: Errore sconosciuto.</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>Le modifiche non salvate andranno perse</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>Prima di continuare, salva il lavoro e rendi permanenti tutte le modifiche da conservare.</translation></message>
    <message><source>Confirm direct restart?</source><translation>Riavvio diretto?</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>Arresto diretto?</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>Riavvia. I file preparati non saranno salvati prima del riavvio.</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>Arresta. I file preparati non saranno salvati prima dell’arresto.</translation></message>
    <message><source>File staging will be skipped</source><translation>Il salvataggio automatico verrà ignorato</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>I file preparati non saranno salvati prima del riavvio.</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>I file preparati non saranno salvati prima dell’arresto.</translation></message>
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
    <message><source>Commit failed</source><translation>Salvataggio non riuscito</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Il percorso non ha una lettera di unità. Identificazione volume di destinazione: Non riuscito.</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>Non riuscito: Lettura informazioni volume (%1)</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>Nessun record della sessione corrente per il volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Il percorso è nell’elenco esclusioni file.
Esclusione: %1</translation></message>
    <message><source>No files were found under %1.</source><translation>Nessun file trovato in %1.</translation></message>
    <message><source>Commit to disk</source><translation>Salva su disco</translation></message>
    <message><source>Committing…</source><translation>Salvataggio…</translation></message>
    <message><source>This action cannot be undone.</source><translation>Questa operazione non può essere annullata.</translation></message>
    <message><source>Continue</source><translation>Continua</translation></message>
    <message><source>Delete and commit</source><translation>Elimina e salva</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>Salva su disco le modifiche del file nell’overlay</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>Salva su disco le modifiche della cartella nell’overlay</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>file: %1. Sono incluse tutte le sottocartelle.</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>Elimina il file e salva l’eliminazione su disco</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>Elimina la cartella e il contenuto e salva le eliminazioni su disco</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>file: %1; cartelle: %2. Elimina e salva.</translation></message>
    <message><source>Commit this registry value to disk</source><translation>Salva il valore del registro su disco</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>Salva su disco la chiave del registro e tutto il sottoalbero</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>Valore: %1. (intera chiave e tutte le sottochiavi)</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>Elimina il valore del registro e salva l’eliminazione su disco</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>Elimina la chiave del registro e il sottoalbero e salva le eliminazioni</translation></message>
    <message><source>Scanning registry keys…</source><translation>Elenco chiavi registro…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>Elenco chiavi registro…
Chiave: %1</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>Registro · Chiave · Elimina e salva:</translation></message>
    <message><source>Copy current entry</source><translation>Copia voce corrente</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>Verranno eliminate ricorsivamente %1 chiavi e tutti i loro valori.</translation></message>
    <message><source>Commit file deletion failed</source><translation>Non riuscito: Salvataggio eliminazione file</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>Il percorso non esiste; non c’è nulla da eliminare.</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>La chiave è nell’elenco esclusioni registro.
Esclusione: %1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>Non riuscito: Lettura filtro registro (%1)</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>Nessun record del filtro registro per la sessione corrente.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>Il valore del registro non esiste; non c’è nulla da salvare.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>La chiave del registro non esiste; non c’è nulla da salvare.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>Il valore del registro non esiste; non c’è nulla da eliminare.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>La chiave del registro non esiste; non c’è nulla da eliminare.</translation></message>
    <message><source>Export commands…</source><translation>Esporta comandi…</translation></message>
    <message><source>Export commands to file</source><translation>Esporta comandi su file</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Esporta su file… · Comando: %1
%2</translation></message>
    <message><source>Import</source><translation>Importa</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Digita o incolla un comando per riga. Carica testi, registri o script (.txt, .bat, .ps1); le righe con questo comando vengono aggiunte. L’importazione prepara le modifiche. Solo «Verifica e applica» nella barra strumenti le scrive nel sistema.</translation></message>
    <message><source>Import uwfmgr commands</source><translation>uwfmgr · Importa</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;uwfmgr&lt;/b&gt;: Digita o incolla un comando per riga. &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Carica da file…&lt;/b&gt;: Carica testi, registri o script (.txt, .bat, .ps1); le righe con questo comando vengono aggiunte. &lt;code&gt;uwfmgr&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Importa&lt;/b&gt;: &lt;b&gt;L’importazione prepara le modifiche. Solo «Verifica e applica» nella barra strumenti le scrive nel sistema.&lt;/b&gt; &lt;b&gt;Verifica e applica&lt;/b&gt;.&lt;/p&gt;</translation></message>
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
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>Tutti i file (*);;File di testo (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
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
    <message><source>Status</source><translation>Stato</translation></message>
    <message><source>Command</source><translation>Comando</translation></message>
    <message><source>Detail</source><translation>Dettagli</translation></message>
    <message><source>Import failed</source><translation>Importazione non riuscita</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Errore interno: nessun gestore di applicazione registrato.</translation></message>
    <message><source>Nothing to import</source><translation>Nessun comando da importare</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Nessun comando uwfmgr trovato nell’input.</translation></message>
    <message><source>Applied: %1</source><translation>Applicato: %1</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplicato: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Non supportato: %1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>Totale dopo %1 lotti:</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Lotto %1 ──</translation></message>
    <message><source>Applied</source><translation>Applicato</translation></message>
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
    <message><source>Queued as a pending %1 change</source><translation>Modifiche in sospeso: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Stato desiderato già presente; nessuna operazione necessaria</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Il percorso è fuori dal volume oppure il volume non supporta esclusioni (es. exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Rifiutato dall’elenco di blocco UWF (file di sistema, Windows, paging, ecc.)</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>Il comando è già stato elaborato in questo lotto</translation></message>
    <message><source>Pending filter %1</source><translation>Modifiche in sospeso: Filtro %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>Filtro: Stato desiderato già presente; nessuna operazione necessaria</translation></message>
    <message><source>Pending overlay type → %1</source><translation>Modifiche in sospeso: Overlay · Tipo → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>Overlay · Tipo: %1 (Stato desiderato già presente; nessuna operazione necessaria)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valore non valido: Dimensione: %1</translation></message>
    <message><source>maximum size</source><translation>Dimensione massima</translation></message>
    <message><source>warning threshold</source><translation>Soglia di avviso</translation></message>
    <message><source>critical threshold</source><translation>Soglia critica</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>Modifiche in sospeso: Overlay %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>Overlay %1: %2 MB (Stato desiderato già presente; nessuna operazione necessaria)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Volume %1 sconosciuto (nessun disco compatibile UWF con questa lettera)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>Modifiche in sospeso: Volume %1 · Protezione UWF: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Stato desiderato già presente; nessuna operazione necessaria</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Il percorso non ha una lettera di unità. Identificazione volume di destinazione: Non riuscito.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Non supportato</translation></message>
    <message><source>file exclusion</source><translation>Esclusioni file</translation></message>
    <message><source>registry exclusion</source><translation>Esclusioni registro</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Le esclusioni del registro richiedono la scheda Registro, qui assente</translation></message>
    <message><source>Unsupported command</source><translation>Comando: Non supportato</translation></message>
    <message><source>Exit</source><translation>Esci</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Mostra/nascondi.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Attivo</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Disattivo</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Sconosciuto</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Utilizzato %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Modalità avanzata</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordina il salvataggio automatico con l’arresto e il riavvio di Windows.</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>Installa un servizio LocalSystem automatico. Avvia UWF Manager all’avvio di Windows e tenta di salvare i file preparati prima dell’arresto o riavvio, migliorando la compatibilità con le applicazioni in conflitto con UWF.</translation></message>
    <message><source>System changes</source><translation>Modifiche al sistema</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>Crea il servizio automatico UWFManagerEnhanced associato all’eseguibile attuale</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>Configurazione: Registro → Salva su disco (HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>Avvia UWF Manager al prossimo avvio di Windows; spostare l’eseguibile può interrompere l’avvio automatico</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>Salva i file preparati idonei durante il normale arresto o riavvio di Windows</translation></message>
    <message><source>Operational and security considerations</source><translation>Considerazioni operative e di sicurezza</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>Registra un servizio nel registro di sistema; l’applicazione non è più completamente portatile</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>Molti file preparati possono allungare l’arresto o il riavvio; lascia tempo sufficiente per conservare le modifiche</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>La modalità avanzata esegue UWF Manager come SYSTEM. Se un non amministratore modifica o sostituisce il programma con un eseguibile malevolo, può ottenere privilegi SYSTEM. Attivala solo da una posizione protetta da permessi amministrativi (consigliata: %ProgramFiles%\UWF Manager\Service\UWF.exe).</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>Non riuscito: Lettura stato modalità avanzata</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Non riuscito: Lettura stato modalità avanzata (Errore sconosciuto).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Non riuscito: Lettura stato servizio:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Non riuscito: Lettura stato servizio (Errore sconosciuto).</translation></message>
    <message><source>Status: Disabled</source><translation>Stato: Disattivo</translation></message>
    <message><source>Status: Enabled</source><translation>Stato: Attivo</translation></message>
    <message><source>Status: Service stopped</source><translation>Stato: servizio arrestato</translation></message>
    <message><source>Status: Repair required</source><translation>Stato: riparazione necessaria</translation></message>
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
    <message><source>Hide overlay hub</source><translation>Nascondi HUB overlay</translation></message>
    <message><source>Exit application</source><translation>Esci dall’applicazione</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Non riuscito: Connessione al sistema</translation></message>
    <message><source>Registry access failed</source><translation>Non riuscito: Accesso al registro</translation></message>
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
    <message><source>The target state could not be read before the operation: %1</source><translation>Impossibile leggere lo stato della destinazione prima dell’operazione: %1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Il percorso non esiste; non c’è nulla da eliminare.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Errore sconosciuto.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Risultato non confermabile: rilettura dello stato della destinazione non riuscita: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Il provider ha accettato l’eliminazione, ma la verifica autorevole conferma che la destinazione esiste ancora.</translation></message>
    <message><source>Unknown</source><translation>Sconosciuto</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Non riuscito: Caricamento registro (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Non riuscito: Caricamento registro (Errore sconosciuto).</translation></message>
    <message><source>: unknown error</source><translation>: Errore sconosciuto</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>Non riuscito: Lettura stato destinazione (%1)</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>Non riuscito: Lettura stato destinazione (Errore sconosciuto).</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>Non riuscito: Identificazione volume di destinazione (%1)</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>Non riuscito: Elenco valori registro (%1)</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>Non riuscito: Elenco chiavi registro (%1)</translation></message>
    <message><source>Delete and commit failed</source><translation>Elimina e salva: Non riuscito</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Non riuscito: Identificazione volume di destinazione (%1): %2</translation></message>
    <message><source>%1/%2 slots</source><translation>Slot utilizzati: %1/%2</translation></message>
</context>
</TS>
