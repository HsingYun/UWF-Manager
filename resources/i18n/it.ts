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
    <message><source>Disabled</source><translation>Disabilitato</translation></message>
    <message><source>Disabled</source><comment>service startup type</comment><translation>Disabilitato</translation></message>
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
    <message><source>Protect this volume in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Proteggi questo volume nella prossima sessione. Le scritture nei percorsi protetti vengono reindirizzate all’overlay; la conservazione dopo i riavvii dipende dalle impostazioni dell’overlay.</translation></message>
    <message><source>Current session</source><translation>Sessione corrente</translation></message>
    <message><source>Next session</source><translation>Prossima sessione</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identificazione del volume: la lettera è semplice, ma il collegamento si perde se viene riassegnata, ad esempio aggiungendo o rimuovendo dischi. L’ID volume rimane stabile.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Volume non supportato da UWF: </translation></message>
    <message><source>Global settings</source><translation>Impostazioni globali</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Stato del filtro UWF nella sessione corrente (sola lettura).</translation></message>
    <message><source>Enable UWF in the next session. Writes to protected paths are redirected to the overlay; retention across restarts depends on the overlay settings.</source><translation>Abilita UWF nella prossima sessione. Le scritture nei percorsi protetti vengono reindirizzate all’overlay; la conservazione dopo i riavvii dipende dalle impostazioni dell’overlay.</translation></message>
    <message><source>To change the overlay type or maximum capacity:
1. Disable UWF.
2. Restart the device.
3. Change this setting.</source><translation>Per modificare il tipo o la capacità massima dell’overlay:
1. Disabilita UWF.
2. Riavvia il dispositivo.
3. Modifica questa impostazione.</translation></message>
    <message><source>RAM uses system memory. Disk uses the system drive and supports larger overlays. Persistent Overlay can retain Disk overlay data across restarts.</source><translation>RAM usa la memoria di sistema. Disk usa l’unità di sistema e supporta overlay più grandi. L’overlay persistente può conservare i dati di un overlay Disk tra i riavvii.</translation></message>
    <message><source>Type</source><translation>Tipo</translation></message>
    <message><source>RAM overlay capacity is limited by system memory. A Disk overlay requires at least 1024 MB and sufficient free space on the system drive.</source><translation>La capacità di un overlay RAM è limitata dalla memoria di sistema. Un overlay Disk richiede almeno 1024 MB e spazio libero sufficiente sull’unità di sistema.</translation></message>
    <message><source>Maximum capacity</source><translation>Capacità massima</translation></message>
    <message><source>Maximum capacity · RAM %1</source><translation>Capacità massima · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Soglia di avviso</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Registra una notifica di avviso nel registro eventi quando l’utilizzo dell’overlay raggiunge questo valore. Se entrambi gli eventi sono abilitati, il valore deve essere inferiore alla soglia critica. Il valore 0 disabilita solo l’evento di avviso.</translation></message>
    <message><source>Critical threshold</source><translation>Soglia critica</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Registra una notifica critica nel registro eventi quando l’utilizzo dell’overlay raggiunge questo valore. Se entrambi gli eventi sono abilitati, il valore deve essere superiore alla soglia di avviso. Il valore 0 disabilita solo l’evento critico.</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Tipo di unità non supportato (solo dischi locali fissi).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Il volume supera il limite UWF di 16 TiB per volume protetto.</translation></message>

    <message><source>%1 free / %2</source><translation>%1 liberi / %2</translation></message>
    <message><source>Commit</source><translation>Consolida</translation></message>
    <message><source>Commit changes from the overlay to disk or the registry. This cannot be undone.</source><translation>Consolida le modifiche dall’overlay su disco o nel registro. L’operazione è irreversibile.</translation></message>
    <message><source>Commit file changes…</source><translation>Consolida modifiche del file…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Seleziona un file e consolidane le modifiche dall’overlay su disco.</translation></message>
    <message><source>Commit folder changes…</source><translation>Consolida modifiche della cartella…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Seleziona una cartella e consolida su disco le modifiche nell’overlay di tutti i file al suo interno.</translation></message>
    <message><source>Delete and commit file…</source><translation>Elimina e consolida file…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Seleziona un file da eliminare e consolida l’eliminazione su disco.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Elimina e consolida cartella…</translation></message>
    <message><source>Select a folder, recursively delete it and all its contents, and commit the deletions to disk.</source><translation>Seleziona una cartella da eliminare con tutto il suo contenuto, ricorsivamente, e consolida le eliminazioni su disco.</translation></message>
    <message><source>Commit registry changes…</source><translation>Consolida modifiche del registro…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Inserisci una chiave del registro e, facoltativamente, un nome di valore, quindi consolida le modifiche nel registro. Senza nome di valore, vengono consolidati tutti i valori della chiave e di tutte le sottochiavi.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Elimina e consolida nel registro…</translation></message>
    <message><source>Enter a registry key and an optional value name. Leave the value name empty to delete the key and all its subkeys and values, then commit the deletions.</source><translation>Inserisci una chiave del registro e, facoltativamente, un nome di valore. Lascia vuoto il nome del valore per eliminare la chiave, tutte le relative sottochiavi e tutti i valori, quindi consolidare le eliminazioni.</translation></message>
    <message><source>File exclusions</source><translation>Esclusioni file</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>File e cartelle del volume esclusi dalla protezione UWF. Fai doppio clic su una voce per copiarne il percorso.</translation></message>
    <message><source>Registry exclusions</source><translation>Esclusioni registro</translation></message>
    <message><source>Registry exclusions are global and are shown only on the system drive.</source><translation>Le esclusioni del registro sono globali e vengono mostrate solo nella partizione di Windows.</translation></message>
    <message><source>File staging</source><translation>File predisposti</translation></message>

    <message><source>Add files or a folder to the automatic commit list.</source><translation>Aggiungi file o una cartella all’elenco di consolidamento automatico.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Seleziona uno o più file da consolidare automaticamente prima dell’arresto o del riavvio sicuro.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Seleziona una cartella i cui file verranno consolidati ricorsivamente prima dell’arresto o del riavvio sicuro.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Seleziona file per il consolidamento automatico</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Seleziona una cartella per il consolidamento automatico ricorsivo</translation></message>
    <message><source>Cannot stage this path</source><translation>Impossibile preparare questo percorso</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Questo percorso si sovrappone a un’esclusione di file UWF. Rimuovi l’esclusione prima di aggiungere il percorso all’elenco dei file predisposti:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Questo percorso si sovrappone a una voce dell’elenco dei file predisposti. Rimuovi il percorso predisposto prima di aggiungere l’esclusione:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Il percorso si sovrappone a una voce dell’elenco dei file predisposti</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Questa esclusione del registro è necessaria finché l’elenco dei file predisposti contiene voci salvate</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Il percorso non esiste o non è del tipo previsto:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Impossibile verificare il percorso:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>I reparse point non possono essere aggiunti all’elenco dei file predisposti:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be added to file staging:
%1</source><translation>Il percorso non ha una lettera di unità e non può essere aggiunto a «File predisposti»:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Il percorso selezionato si trova sul volume %1. Aggiungilo dalla scheda «File predisposti» di quel volume.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Non è possibile predisporre la radice di un volume, perché verrebbe consolidato ricorsivamente l’intero volume.</translation></message>
    <message><source>File staging could not be saved</source><translation>Impossibile salvare l’elenco dei file predisposti</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>L’elenco dei file predisposti è stato ricaricato perché l’aggiornamento non è stato completato:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>La funzione File predisposti non è disponibile: %1</translation></message>

    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>%1 voci predisposte · %2 file · %3 cartelle</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>File predisposti richiede questa esclusione del registro per conservare il proprio elenco tra i riavvii.</translation></message>
    <message><source>Built-in</source><translation>Integrato</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Il filtro UWF è attualmente disabilitato; non si sono accumulate modifiche nell’overlay da consolidare.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Il file system %1 non supporta il consolidamento per singolo file.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Questo volume non è supportato da UWF.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Il file system %1 non supporta il consolidamento per singolo file. È disponibile solo quello del registro (le esclusioni del registro sono globali).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Questo volume non è attualmente protetto da UWF; non ci sono modifiche ai file da consolidare.</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Seleziona un file da consolidare su disco</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Seleziona una cartella da consolidare su disco</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Seleziona un file di cui consolidare l’eliminazione</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Seleziona una cartella di cui consolidare l’eliminazione</translation></message>
    <message><source>Name</source><translation>Nome</translation></message>
    <message><source>Data</source><translation>Dati</translation></message>
    <message><source>(Default)</source><translation>(Predefinito)</translation></message>
    <message><source>(value not set)</source><translation>(valore non impostato)</translation></message>
    <message><source>(empty)</source><translation>(vuoto)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Anteprima non disponibile (valore troppo grande)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Apri una vista diagnostica dei file attualmente memorizzati nell’overlay per questo volume.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>L’elenco dei file nell’overlay è disponibile solo sui volumi NTFS.</translation></message>
    <message><source>The UWF filter is disabled; no volume has an overlay to inspect.</source><translation>Il filtro UWF è disattivato; nessun volume ha un overlay da esaminare.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Il volume non è protetto da UWF e non ha overlay da esaminare.</translation></message>
    <message><source>Failed: %1</source><translation>Non riuscito: %1</translation></message>
    <message><source>The previous enumeration may still be running. Another read cannot start yet.</source><translation>L’enumerazione precedente potrebbe essere ancora in corso. Non è ancora possibile avviare una nuova lettura.</translation></message><message><source>The UWF service is unavailable while enumerating overlay files.</source><translation>Il servizio UWF non è disponibile durante l’enumerazione dei file dell’overlay.</translation></message>
    <message><source>Out of memory, or this operation is not supported. This feature requires an NTFS volume; close memory-intensive applications and try again.</source><translation>Memoria insufficiente o operazione non supportata dal sistema. Questa funzione richiede un volume NTFS; chiudi le applicazioni che usano molta memoria e riprova.</translation></message>
    <message><source>Raw error: %1</source><translation>Codice errore: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Esporta su file… · Overlay · file</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>File di testo (*.txt);;Tutti i file (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>File di comandi (*.cmd);;Tutti i file (*)</translation></message>
    <message><source>Export failed</source><translation>Esportazione non riuscita</translation></message>
    <message><source>Export finished</source><translation>Esportazione completata</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Non riuscito: Apertura file (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Non riuscito: Scrittura file (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>%1 voci salvate in:
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Impossibile escludere un intero volume (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be added as exclusions.</source><translation>I file di paging, swap e ibernazione non possono essere aggiunti alle esclusioni.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Impossibile escludere l’intera directory: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Impossibile escludere l’intera directory: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Impossibile escludere l’intera directory: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Questo file di sistema essenziale non può essere escluso: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Il file di registro utente NTUSER.DAT non può essere escluso.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>La chiave del registro non può essere vuota.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>La chiave del registro non può iniziare con una barra inversa.</translation></message>
    <message><source>Registry paths cannot contain null characters.</source><translation>I percorsi del Registro non possono contenere caratteri nulli.</translation></message>
    <message><source>A registry key component is too long.</source><translation>Un componente della chiave del Registro è troppo lungo.</translation></message>
    <message><source>The registry path exceeds UWF's nesting limit.</source><translation>Il percorso del Registro supera il limite di annidamento di UWF.</translation></message>
    <message><source>Add a file or folder to the exclusion list.</source><translation>Aggiungi un file o una cartella all’elenco delle esclusioni.</translation></message>
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
    <message><source>Commit folder changes to disk…</source><translation>Consolida modifiche della cartella su disco…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Consolida modifiche del file su disco…</translation></message>
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
    <message><source>Review all pending changes and apply them in one batch.</source><translation>Verifica tutte le modifiche in sospeso e applicale insieme.</translation></message>
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
    <message><source>The system did not find the target.</source><translation>Il sistema non ha trovato la destinazione.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Il sistema ha rifiutato un parametro (percorso o argomento non valido).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Operazione non riuscita. La destinazione potrebbe essere in uso, ad esempio in Esplora file o in un file aperto. Chiudi i programmi interessati e riprova.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Operazione non riuscita; consulta il registro per i dettagli.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operazione rifiutata (codice %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Sconosciuto.</translation></message>
    <message><source>Commit canceled</source><translation>Consolidamento annullato</translation></message>

    <message><source>Succeeded</source><translation>Riuscito</translation></message>
    <message><source>Skipped</source><translation>Ignorato</translation></message>
    <message><source>Failed</source><translation>Non riuscito</translation></message>
    <message><source>%1 succeeded; %2 failed.</source><translation>Riusciti: %1; non riusciti: %2.</translation></message>

    <message><source>Existed before</source><translation>Esisteva prima dell’eliminazione</translation></message>
    <message><source>Exists after</source><translation>Esiste dopo l’eliminazione</translation></message>
    <message><source>Error code</source><translation>Codice errore</translation></message>
    <message><source>Reason</source><translation>Motivo</translation></message>
    <message><source>Previous page</source><translation>Pagina precedente</translation></message>
    <message><source>First page</source><translation>Prima pagina</translation></message>
    <message><source>Last page</source><translation>Ultima pagina</translation></message>
    <message><source>Next page</source><translation>Pagina successiva</translation></message>
    <message><source>Copy selected rows</source><translation>Copia righe selezionate</translation></message>
    <message><source>Copy all</source><translation>Copia tutto</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Questa esclusione del registro è necessaria finché l’elenco dei file predisposti contiene voci salvate. Svuota l’elenco prima di rimuovere l’esclusione.</translation></message>
    <message><source>Copy current page</source><translation>Copia pagina corrente</translation></message>
    <message><source>No pending changes</source><translation>Nessuna modifica in sospeso</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Esclusioni registro · Impostazioni globali.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Vai alla protezione, alle esclusioni di file e ai file predisposti per il volume %1.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Non supportato</translation></message>
    <message><source>Review and apply changes</source><translation>Verifica e applica le modifiche</translation></message>
    <message><source>Confirm apply</source><translation>Conferma applicazione</translation></message>
    <message><source>Result</source><translation>Risultato</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filtro: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Soglia di avviso dell’overlay impostata su %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Soglia critica dell’overlay impostata su %1 MB</translation></message>
    <message><source>✘ Type / maximum capacity not applied: UWF is enabled. Disable UWF and restart first.</source><translation>✘ Tipo / capacità massima non applicati: UWF è abilitato. Disabilita UWF e riavvia prima di procedere.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Tipo di overlay impostato su %1</translation></message>
    <message><source>✓ Maximum overlay capacity set to %1 MB</source><translation>✓ Capacità massima dell’overlay impostata su %1 MB</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · Protezione UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Identifica tramite: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1: esclusione di file aggiunta: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1: esclusione di file rimossa: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Esclusione del registro aggiunta: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Esclusione del registro rimossa: %1</translation></message>
    <message><source>Time</source><translation>Ora</translation></message>
    <message><source>Level</source><translation>Livello</translation></message>

    <message><source>Message</source><translation>Messaggio</translation></message>
    <message><source>Committing staged files</source><translation>Consolidamento dei file predisposti</translation></message>
    <message><source>No additional error details were provided.</source><translation>Nessun dettaglio aggiuntivo sull’errore.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Il consolidamento automatico dei file predisposti si è interrotto in modo imprevisto:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Il consolidamento automatico dei file predisposti si è interrotto per un errore sconosciuto.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Arresto sicuro non riuscito</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Arresta: Non riuscito (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Riavvio sicuro non riuscito</translation></message>
    <message><source>Direct restart failed</source><translation>Riavvio diretto: Non riuscito</translation></message>
    <message><source>Direct shutdown failed</source><translation>Arresto diretto: Non riuscito</translation></message>
    <message><source>Restart failed: %1</source><translation>Riavvia: Non riuscito (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Impossibile leggere l’elenco dei file predisposti:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Impossibile leggere l’elenco dei file predisposti a causa di un errore sconosciuto.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>File rilevati: %1 · Consolidati: %2 · File ignorati: %3 · Voci ignorate: %4 · Non riusciti: %5</translation></message>
    <message><source>File staging preparation</source><translation>Preparazione dei file</translation></message>
    <message><source>Reparse points are not supported.</source><translation>I reparse point non sono supportati.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>È possibile predisporre solo percorsi assoluti su volumi locali.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>È possibile aggiungere all’elenco dei file predisposti solo percorsi assoluti su volumi locali:
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Impossibile esaminare il percorso predisposto.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Il file predisposto ora corrisponde a un altro tipo di percorso.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>La cartella predisposta ora corrisponde a un altro tipo di percorso.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Impossibile enumerare completamente la cartella predisposta.</translation></message>

    <message><source>A volume root cannot be processed recursively.</source><translation>La radice di un volume non può essere elaborata ricorsivamente.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Il provider UWF ha rifiutato il consolidamento.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Il consolidamento non è riuscito a causa di un errore sconosciuto.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Altri %1 errori sono stati scritti nel registro.</translation></message>



    <message><source>No current-session record found for volume %1.</source><translation>Nessun record della sessione corrente per il volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Il percorso è nell’elenco esclusioni file.
Esclusione: %1</translation></message>

    <message><source>Continue</source><translation>Continua</translation></message>
    <message><source>Delete and commit</source><translation>Elimina e consolida</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>La chiave è nell’elenco esclusioni registro.
Esclusione: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Nessun record del filtro registro per la sessione corrente.</translation></message>

    <message><source>Export commands to file</source><translation>Esporta comandi su file</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Esporta su file… · Comando: %1
%2</translation></message>
    <message><source>Import</source><translation>Importa</translation></message>
    <message><source>Import uwfmgr commands into the draft. Changes are written only after confirmation in Review and apply.</source><translation>Importa i comandi uwfmgr nella bozza. Le modifiche vengono scritte solo dopo la conferma in «Verifica e applica».</translation></message>
    <message><source>Load from file…</source><translation>Carica da file…</translation></message>
    <message><source>Load recommended configuration</source><translation>Carica configurazione consigliata</translation></message>
    <message><source>Choose recommended configuration</source><translation>Scegli configurazione consigliata</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Seleziona i gruppi consigliati da aggiungere. Puoi verificare o eliminare singoli comandi prima dell’importazione.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Dalla documentazione ufficiale Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Esclusioni UWF consigliate da Microsoft</translation></message>
    <message><source>Review these recommendations before importing.</source><translation>Verificare queste raccomandazioni prima di importarle.</translation></message>
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
    <message><source>Command import is not ready.</source><translation>L’importazione dei comandi non è ancora pronta.</translation></message>
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
    <message><source>%1 change(s) have not been applied.

Discard them and continue?</source><translation>Modifiche non applicate: %1.

Scartarle e continuare?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Comando uwfmgr incompleto</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Argomento mancante: Dimensione (MB)</translation></message>
    <message><source>Size must be a non-negative integer (MB)</source><translation>La dimensione deve essere un numero intero non negativo (MB)</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Argomento mancante: Overlay · Tipo (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Valore non valido: Overlay · Tipo: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Argomento mancante: Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Il volume deve essere una lettera di unità, ad esempio C:</translation></message>
    <message><source>Missing path argument</source><translation>Argomento mancante: Percorso</translation></message>
    <message><source>Missing registry key argument</source><translation>Argomento mancante: Registro · Chiave</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Virgolette doppie non chiuse nel comando</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Argomento aggiuntivo imprevisto: %1</translation></message>
    <message><source>This uwfmgr command cannot be imported.</source><translation>Questo comando uwfmgr non può essere importato.</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Stato desiderato già presente; nessuna operazione necessaria</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Il percorso è fuori dal volume oppure il volume non supporta esclusioni (es. exFAT / ReFS)</translation></message>
    <message><source>This path cannot be added as an exclusion.</source><translation>Questo percorso non può essere aggiunto come esclusione.</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valore non valido: Dimensione: %1</translation></message>
    <message><source>maximum capacity</source><translation>capacità massima</translation></message>
    <message><source>warning threshold</source><translation>Soglia di avviso</translation></message>
    <message><source>critical threshold</source><translation>Soglia critica</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>La configurazione globale non è disponibile; il comando non è stato aggiunto alle modifiche in sospeso.</translation></message>
    <message><source>Unknown volume %1 (no matching volume configuration)</source><translation>Volume %1 sconosciuto (nessuna configurazione di volume corrispondente)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Stato desiderato già presente; nessuna operazione necessaria</translation></message>
    <message><source>Path %1 has no drive letter; its volume tab cannot be identified.</source><translation>Il percorso %1 non ha una lettera di unità; impossibile identificare la scheda del volume.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Non supportato</translation></message>
    <message><source>file exclusion</source><translation>Esclusioni file</translation></message>
    <message><source>registry exclusion</source><translation>Esclusioni registro</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Le esclusioni del registro richiedono la scheda Registro, qui assente</translation></message>
    <message><source>Unsupported command</source><translation>Comando: Non supportato</translation></message>
    <message><source>Show or hide the overlay widget.</source><translation>Mostra o nascondi il widget dell’overlay.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Attivo</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Disabilitato</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Sconosciuto</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Utilizzato %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Modalità avanzata</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordina il consolidamento automatico dei file predisposti con l’arresto e il riavvio di Windows.</translation></message>
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
    <message><source>The service cannot receive Windows preshutdown notifications.</source><translation>Il servizio non può attualmente ricevere le notifiche di prearresto di Windows.</translation></message>
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
    <message><source>Show main window</source><translation>Mostra finestra principale</translation></message>
    <message><source>Restore default position</source><translation>Ripristina posizione predefinita</translation></message>
    <message><source>Show overlay widget</source><translation>Mostra widget dell’overlay</translation></message>
    <message><source>Hide overlay widget</source><translation>Nascondi widget dell’overlay</translation></message>
    <message><source>Exit application</source><translation>Esci dall’applicazione</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Non riuscito: Lettura valori registro (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Elenco file overlay: Errore sconosciuto.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Non riuscito: Azione sul filtro: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Non riuscito: Lettura stato filtro</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Non riuscito: Impostazione soglia di avviso</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Non riuscito: Impostazione soglia critica</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Non riuscito: Lettura stato overlay</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Non riuscito: Impostazione tipo overlay</translation></message>
    <message><source>✘ Failed to set maximum capacity</source><translation>✘ Impossibile impostare la capacità massima</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Non riuscito: Lettura configurazione overlay</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Non riuscito: Protezione volume %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Non riuscito: Impostazione identificazione volume %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Non riuscito: Aggiunta esclusione file %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Non riuscito: Rimozione esclusione file %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Non riuscito: Lettura configurazione volume</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Non riuscito: Aggiunta esclusione registro %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Non riuscito: Rimozione esclusione registro %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Non riuscito: Aggiornamento opzioni persistenza registro</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Non riuscito: Lettura filtro registro</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Non supportato</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Lettura stato destinazione: Errore sconosciuto.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Errore sconosciuto.</translation></message>
    <message><source>The target state could not be verified, so the operation result is unknown: %1</source><translation>Impossibile verificare lo stato della destinazione; il risultato dell’operazione è quindi sconosciuto: %1</translation></message>
    <message><source>The system accepted the deletion request, but the target still exists after verification.</source><translation>Il sistema ha accettato la richiesta di eliminazione, ma la destinazione esiste ancora dopo la verifica.</translation></message>
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
    <message><source>Committing</source><translation>Consolidamento</translation></message>
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
Gestisci protezione dei volumi, overlay, esclusioni e file predisposti.</translation></message>
    <message><source>Snapshot details</source><translation>Dettagli dell’istantanea</translation></message>
    <message><source>Account</source><translation>Account</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Aggiungi una configurazione UWF da uno script o testo.</translation></message>
    <message><source>Add exclusion</source><translation>Aggiungi esclusione</translation></message>
    <message><source>Add files or folders</source><translation>Aggiungi file o cartelle</translation></message>
    <message><source>Add selected items</source><translation>Aggiungi elementi selezionati</translation></message>
    <message><source>Added</source><translation>Aggiunto</translation></message>
    <message><source>Administrator</source><translation>Amministratore</translation></message>
    <message><source>The selected operations completed. UWF may skip excluded targets.</source><translation>Le operazioni selezionate sono terminate. UWF può ignorare le destinazioni escluse.</translation></message>
    <message><source>All files (*)</source><translation>Tutti i file (*)</translation></message>
    <message><source>All levels</source><translation>Tutti i livelli</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Un’istantanea diagnostica vuota non significa che l’overlay non contenga modifiche.</translation></message>
    <message><source>Application path</source><translation>Percorso dell’applicazione</translation></message>
    <message><source>Application result</source><translation>Risultato dell’applicazione</translation></message>
    <message><source>Apply changes</source><translation>Applica modifiche</translation></message>
    <message><source>Architecture</source><translation>Architettura</translation></message>
    <message><source>Automatic</source><translation>Automatico</translation></message>
    <message><source>Automatic staged file commits</source><translation>Consolidamento automatico dei file predisposti</translation></message>
    <message><source>Automatic file commits incomplete</source><translation>Consolidamento automatico dei file incompleto</translation></message>
    <message><source>Automatically commit eligible staged files during Windows shutdown and restart.</source><translation>Consolida automaticamente i file predisposti idonei durante l’arresto e il riavvio di Windows.</translation></message>
    <message><source>Available</source><translation>Disponibile</translation></message>
    <message><source>Back</source><translation>Indietro</translation></message>
    <message><source>Background service</source><translation>Servizio in background</translation></message>
    <message><source>Binding</source><translation>Associazione</translation></message>
    <message><source>By drive letter</source><translation>Per lettera di unità</translation></message>
    <message><source>By volume ID</source><translation>Per ID del volume</translation></message>
    <message><source>Cancel reading</source><translation>Annulla lettura</translation></message>
    <message><source>Completed operations cannot be undone.</source><translation>Le operazioni completate non possono essere annullate.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Selezionare la chiave del Registro di sistema da escludere dalla protezione UWF, incluse le relative sottochiavi e i valori.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Scegli dove salvare il file.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Scegli una posizione nell’unità o cartella consentita.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Scegli un’altra cartella o controlla i permessi di accesso e riprova.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Scegli un’altra chiave o controlla i permessi di accesso e riprova.</translation></message>
    <message><source>Clear filters</source><translation>Azzera filtri</translation></message>
    <message><source>Clear logs</source><translation>Cancella log</translation></message>
    <message><source>Clear logs?</source><translation>Cancellare i log?</translation></message>
    <message><source>Collapse</source><translation>Comprimi</translation></message>
    <message><source>Commit changes</source><translation>Consolida modifiche</translation></message>
    <message><source>Commit changes…</source><translation>Consolida modifiche…</translation></message>
    <message><source>Commit file changes?</source><translation>Consolidare le modifiche al file?</translation></message>
    <message><source>Commit folder changes?</source><translation>Consolidare le modifiche alla cartella?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Consolidare le modifiche alla chiave del registro?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Consolidare le modifiche al valore del registro?</translation></message>
    <message><source>Compatibility mode</source><translation>Modalità compatibilità</translation></message>
    <message><source>Configuration applied</source><translation>Configurazione applicata</translation></message>
    <message><source>Configuration error</source><translation>Errore di configurazione</translation></message>
    <message><source>Configured</source><translation>Configurato</translation></message>
    <message><source>Confirm before continuing</source><translation>Conferma prima di continuare</translation></message>
    <message><source>Connected and authenticated</source><translation>Connesso e autenticato</translation></message>
    <message><source>Connecting…</source><translation>Connessione…</translation></message>
    <message><source>Contact author</source><translation>Contatta l’autore</translation></message>
    <message><source>Continue editing</source><translation>Continua a modificare</translation></message>
    <message><source>If you continue restart, uncommitted file changes may be lost.</source><translation>Se continui il riavvio, le modifiche ai file non consolidate potrebbero andare perse.</translation></message>
    <message><source>If you continue shutdown, uncommitted file changes may be lost.</source><translation>Se continui l’arresto, le modifiche ai file non consolidate potrebbero andare perse.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Coordina con l’arresto di Windows</translation></message>
    <message><source>Copy all logs</source><translation>Copia tutti i log</translation></message>
    <message><source>Copy full report</source><translation>Copia rapporto completo</translation></message>
    <message><source>Copy name</source><translation>Copia nome</translation></message>
    <message><source>Copy path</source><translation>Copia percorso</translation></message>
    <message><source>Copy selection</source><translation>Copia selezione</translation></message>
    <message><source>Copy version</source><translation>Copia versione</translation></message>
    <message><source>Counting staged files…</source><translation>Conteggio dei file predisposti…</translation></message>
    <message><source>Create</source><translation>Crea</translation></message>
    <message><source>Critical %1 MB</source><translation>Critico %1 MB</translation></message>
    <message><source>Current filter</source><translation>Filtro attuale</translation></message>
    <message><source>Current run</source><translation>Esecuzione attuale</translation></message>
    <message><source>Current selection</source><translation>Selezione attuale</translation></message>
    <message><source>Current usage</source><translation>Utilizzo attuale</translation></message>
    <message><source>Date modified</source><translation>Data di modifica</translation></message>
    <message><source>Debug</source><translation>Debug</translation></message>
    <message><source>Delete and commit this file?</source><translation>Eliminare questo file e consolidare l’eliminazione?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Eliminare questa cartella e consolidare l’eliminazione?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Eliminare questa chiave del registro e consolidare l’eliminazione?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Eliminare questo valore del registro e consolidare l’eliminazione?</translation></message>

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
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to commit staged files.</source><translation>La modalità avanzata non è disponibile. Usa «Arresto sicuro» o «Riavvio sicuro» per consolidare i file predisposti.</translation></message>
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
    <message><source>File commits have started and cannot be canceled.</source><translation>Il consolidamento dei file è iniziato e non può essere annullato.</translation></message>
    <message><source>File commits incomplete</source><translation>Consolidamento dei file incompleto</translation></message>
    <message><source>File type</source><translation>Tipo di file</translation></message>
    <message><source>Files and folders</source><translation>File e cartelle</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>È possibile selezionare file e cartelle insieme.</translation></message>
    <message><source>Filter is disabled</source><translation>Il filtro è disattivato</translation></message>
    <message><source>Fit columns to contents</source><translation>Adatta colonne al contenuto</translation></message>
    <message><source>Folder address</source><translation>Indirizzo cartella</translation></message>
    <message><source>Folder name</source><translation>Nome cartella</translation></message>
    <message><source>Folders</source><translation>Cartelle</translation></message>
    <message><source>Add this key to the next-session exclusions.</source><translation>Aggiungi questa chiave alle esclusioni della prossima sessione.</translation></message>
    <message><source>Add this path to the next-session exclusions.</source><translation>Aggiungi questo percorso alle esclusioni della prossima sessione.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Esclusione globale del registro · include sottochiavi</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Sali alla cartella superiore o scegli un’altra posizione.</translation></message>
    <message><source>Home</source><translation>Cartella personale</translation></message>
    <message><source>Import configuration</source><translation>Importa configurazione</translation></message>
    <message><source>Import updates the draft. Confirm the changes in Review and apply to write them to the system.</source><translation>L’importazione aggiorna la bozza. Conferma le modifiche in «Verifica e applica» per scriverle nel sistema.</translation></message>
    <message><source>Import results</source><translation>Risultati dell’importazione</translation></message>
    <message><source>Imported</source><translation>Importato</translation></message>
    <message><source>The list shows command results for the configuration draft. Select a row to view the complete command and details.</source><translation>L’elenco mostra i risultati dei comandi sulla bozza di configurazione; seleziona una riga per visualizzare il comando completo e i dettagli.</translation></message>
    <message><source>Imported: %1</source><translation>Importato: %1</translation></message>
    <message><source>Information</source><translation>Informazioni</translation></message>
    <message><source>Item details</source><translation>Dettagli elemento</translation></message>
    <message><source>Committing many staged files may take longer.</source><translation>Il consolidamento di molti file predisposti può richiedere più tempo.</translation></message>
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
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Se UWF protegge l’unità dell’applicazione, assicurati prima che i suoi file vengano conservati dopo un riavvio. Il consolidamento di molti file predisposti può prolungare l’arresto o il riavvio.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>È possibile selezionare più file.</translation></message>
    <message><source>Name and location</source><translation>Nome e posizione</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Valore denominato: %1 · solo questo valore</translation></message>
    <message><source>Navigate to registry path</source><translation>Vai al percorso del registro</translation></message>
    <message><source>New folder</source><translation>Nuova cartella</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>I nuovi record appariranno dopo l’aggiornamento.</translation></message>
    <message><source>Next-session configuration</source><translation>Configurazione della prossima sessione</translation></message>
    <message><source>No files to commit</source><translation>Nessun file da consolidare</translation></message>
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
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after restart.</source><translation>Solo i file predisposti vengono consolidati automaticamente. Le altre modifiche non consolidate potrebbero andare perse dopo il riavvio.</translation></message>
    <message><source>Only staged files are committed automatically. Other uncommitted changes may be lost after shutdown.</source><translation>Solo i file predisposti vengono consolidati automaticamente. Le altre modifiche non consolidate potrebbero andare perse dopo l’arresto.</translation></message>
    <message><source>Open</source><translation>Apri</translation></message>
    <message><source>Open folder</source><translation>Apri cartella</translation></message>
    <message><source>Operating system</source><translation>Sistema operativo</translation></message>
    <message><source>Operation and security</source><translation>Funzionamento e sicurezza</translation></message>
    <message><source>Original command</source><translation>Comando originale</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Soglia critica dell’overlay raggiunta. Salva le modifiche e riavvia in sicurezza.</translation></message>
    <message><source>Maximum overlay capacity</source><translation>Capacità massima dell’overlay</translation></message>
    <message><source>Overlay type</source><translation>Tipo di overlay</translation></message>
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
    <message><source>Files to commit</source><translation>File da consolidare</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>I record dell’esecuzione attuale appariranno qui al termine della lettura.</translation></message>

    <message><source>Refresh folder</source><translation>Aggiorna cartella</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Chiave del registro o impostazione di persistenza</translation></message>
    <message><source>Canceling stops remaining tasks after the current operation finishes.</source><translation>Se annulli, le attività rimanenti si fermano al termine dell’operazione in corso.</translation></message>
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
    <message><source>Restart will continue after committing. Keep the application open.</source><translation>Il riavvio continuerà dopo il consolidamento. Mantieni aperta l’applicazione.</translation></message>
    <message><source>Restore</source><translation>Ripristina</translation></message>
    <message><source>Retry</source><translation>Riprova</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Esamina i dettagli degli errori prima di continuare l’arresto.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Esamina i dettagli degli errori prima di riavviare.</translation></message>
    <message><source>Some changes may already have been written. Refresh the system state before retrying.</source><translation>Alcune modifiche potrebbero essere già state scritte. Aggiorna lo stato del sistema prima di riprovare.</translation></message>
    <message><source>Review the result for each target.</source><translation>Controlla il risultato per ogni destinazione.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Controlla queste modifiche prima di scriverle nel sistema.</translation></message>
    <message><source>Running</source><translation>In esecuzione</translation></message>
    <message><source>Save</source><translation>Salva</translation></message>
    <message><source>Commit staged files to disk, then restart.</source><translation>Consolida i file predisposti su disco, quindi riavvia.</translation></message>
    <message><source>Commit staged files to disk, then shut down.</source><translation>Consolida i file predisposti su disco, quindi arresta il sistema.</translation></message>

    <message><source>Committing to disk</source><translation>Consolidamento su disco</translation></message>
    <message><source>Committing…</source><translation>Consolidamento…</translation></message>
    <message><source>Scanning staged paths</source><translation>Analisi dei percorsi predisposti</translation></message>
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
    <message><source>Shutdown will continue after committing. Keep the application open.</source><translation>L’arresto continuerà dopo il consolidamento. Mantieni aperta l’applicazione.</translation></message>
    <message><source>Size</source><translation>Dimensione</translation></message>
    <message><source>Some changes could not be applied</source><translation>Impossibile applicare alcune modifiche</translation></message>
    <message><source>Some files could not be committed</source><translation>Impossibile consolidare alcuni file</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Impossibile leggere alcune informazioni hardware. Le altre informazioni diagnostiche rimangono disponibili e copiabili.</translation></message>
    <message><source>Source code</source><translation>Codice sorgente</translation></message>

    <message><source>Staged path</source><translation>Percorso predisposto</translation></message>
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
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>I metadati di sistema servono solo alla diagnostica e non possono essere consolidati.</translation></message>
    <message><source>Takes effect after restart</source><translation>Ha effetto dopo il riavvio</translation></message>
    <message><source>Takes effect when applied</source><translation>Ha effetto all’applicazione</translation></message>
    <message><source>Used when UWF starts</source><translation>Usato all’avvio di UWF</translation></message>
    <message><source>Mixed activation times</source><translation>Tempi di attivazione diversi</translation></message>
    <message><source>Target</source><translation>Destinazione</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistenza delle licenze Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Il provider UWF non ha completato l’enumerazione. Può accadere con un overlay grande o risorse di sistema insufficienti.</translation></message>
    <message><source>The configuration has been written to the system.</source><translation>La configurazione è stata scritta nel sistema.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>I record attuali dei log verranno cancellati e non potranno essere recuperati.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>La cartella di destinazione non esiste.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Impossibile leggere lo stato del servizio della modalità avanzata. Controlla le autorizzazioni del servizio e riprova.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>L’elenco può omettere file o includere file già consolidati. Non usarlo come elenco di consolidamento.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>L’elemento selezionato non esiste più: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to resume automatic file commits.</source><translation>Il servizio è installato ma arrestato. Avvialo per riprendere il consolidamento automatico dei file.</translation></message>
    <message><source>This PC</source><translation>Questo PC</translation></message>
    <message><source>This folder is empty</source><translation>Questa cartella è vuota</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Questo rapporto può contenere nomi di account, identificativi dei dispositivi e percorsi di file. Controllalo prima di condividerlo.</translation></message>

    <message><source>Try another keyword or log level.</source><translation>Prova un’altra parola chiave o un altro livello di log.</translation></message>
    <message><source>Try another search term.</source><translation>Prova un altro termine di ricerca.</translation></message>
    <message><source>Disable UWF and restart before changing the overlay type or maximum capacity.</source><translation>Disabilita UWF e riavvia prima di modificare il tipo o la capacità massima dell’overlay.</translation></message>
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
    <message><source>Some items were not completed or could not be verified. Select an item to view its full path and details.</source><translation>Alcuni elementi non sono stati completati o non è stato possibile verificarli. Seleziona un elemento per visualizzarne il percorso completo e i dettagli.</translation></message>
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
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to commit staged files.</source><translation>Senza la modalità avanzata, usa «Arresto sicuro» o «Riavvio sicuro» in questa applicazione per consolidare i file predisposti.</translation></message>
    <message><source>Write protection</source><translation>Protezione da scrittura</translation></message>
    <message><source>Write these configuration changes to the system?</source><translation>Scrivere queste modifiche di configurazione nel sistema?</translation></message>
    <message><source>staged files</source><translation>file predisposti</translation></message>
    <message><source>uwfmgr commands</source><translation>comandi uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Contrassegnare i percorsi selezionati per la rimozione. Ha effetto dopo l’applicazione.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Rimuovi dall’elenco dei file predisposti dopo l’applicazione.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Aggiungi all’elenco dei file predisposti dopo l’applicazione.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>L’applicazione di questo piano svuota l’elenco salvato dei file predisposti e rimuove la relativa esclusione del registro quando non è più necessaria.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Le modifiche a File predisposti devono essere applicate in UWF Manager e non possono essere esportate come comandi uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>L’elenco dei file predisposti verrà salvato dopo l’applicazione</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Ambito dell’istantanea dell’overlay</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Questo elenco serve solo alla diagnostica. Non consente di stabilire quali file devono essere consolidati.</translation></message>
    <message><source>NTFS volumes only</source><translation>Solo volumi NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>La lettura di overlay grandi può essere lenta o non riuscire per risorse insufficienti.</translation></message>
    <message><source>Small files may be missing</source><translation>Potrebbero mancare file piccoli</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>I file più piccoli della dimensione del cluster del disco (in genere 4 KB) potrebbero non comparire nell’elenco.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Non indica consolidamenti in sospeso</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Possono comparire anche file già consolidati, percorsi esclusi ed elementi interessati da altre operazioni.</translation></message>

    <message><source>Command processing result</source><translation>Risultato dell’elaborazione del comando</translation></message>
    <message><source>Copy command</source><translation>Copia comando</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 rami non leggibili</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>Impossibile leggere %1 rami. Continua con il contenuto rilevato oppure riprova solo gli elementi non riusciti.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 file · %2 cartelle</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 valori · %2 chiavi</translation></message>
    <message><source>Attempts</source><translation>Tentativi</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Annulla le attività rimanenti</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Consolida solo il contenuto esplorato correttamente. I rami non letti restano invariati.</translation></message>
    <message><source>Committing changes</source><translation>Consolidamento delle modifiche</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Le eliminazioni e i consolidamenti completati sono irreversibili.</translation></message>
    <message><source>Continue…</source><translation>Continua…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Elimina e consolida solo le destinazioni esplorate correttamente; i rami che non è stato possibile leggere e le relative cartelle o chiavi superiori vengono conservati.</translation></message>
    <message><source>Deleting and committing</source><translation>Eliminazione e consolidamento</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>L’eliminazione e il consolidamento sono irreversibili. Viene elaborato solo l’ambito selezionato; la configurazione UWF in sospeso non viene applicata.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Determinazione dell’ambito. I rami non leggibili vengono registrati e l’esplorazione prosegue altrove.</translation></message>
    <message><source>Export traversal records</source><translation>Esporta il registro di esplorazione</translation></message>
    <message><source>No actionable content</source><translation>Nessun contenuto elaborabile</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Nessuna destinazione elaborabile trovata. Non è stato eliminato né consolidato nulla.</translation></message>
    <message><source>No changes have been committed.</source><translation>Nessuna modifica consolidata.</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Non è stato eliminato né consolidato nulla.</translation></message>
    <message><source>Path</source><translation>Percorso</translation></message>
    <message><source>Permanently delete the selected content and commit the deletions.</source><translation>Elimina definitivamente il contenuto selezionato e consolida le eliminazioni.</translation></message>
    <message><source>Preparation complete</source><translation>Preparazione completata</translation></message>
    <message><source>Preparing commit</source><translation>Preparazione del consolidamento</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Preparazione di eliminazione e consolidamento</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Elabora solo il contenuto esplorato correttamente</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>I punti di analisi vengono ignorati per mantenere l’esplorazione nell’ambito selezionato.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Un riavvio non annulla un consolidamento. Viene elaborato solo l’ambito selezionato; la configurazione UWF in sospeso non viene applicata.</translation></message>
    <message><source>Retry failed items</source><translation>Riprova gli elementi non riusciti</translation></message>
    <message><source>Commit changes in the selected scope.</source><translation>Consolida le modifiche nell’ambito selezionato.</translation></message>
    <message><source>Stopping the operation</source><translation>Arresto dell’operazione</translation></message>
    <message><source>Successful and failed records are retained</source><translation>I risultati riusciti e non riusciti sono conservati</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>La cartella non è vuota; il contenuto non elaborato è stato conservato.</translation></message>
    <message><source>The operation could not be completed</source><translation>Impossibile completare l’operazione</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>La chiave del Registro contiene nuovi valori; il contenuto non elaborato è stato conservato.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>La chiave del Registro contiene ancora sottochiavi; il contenuto non elaborato è stato conservato.</translation></message>
    <message><source>The target no longer exists.</source><translation>La destinazione non esiste più.</translation></message>
    <message><source>Traversal complete</source><translation>Esplorazione completata</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Esplorazione completata con contenuti non letti</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Il registro di esplorazione viene conservato e può essere esportato.</translation></message>
    <message><source>Traverse again</source><translation>Esplora di nuovo</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Esplora di nuovo oppure chiudi questa finestra.</translation></message>
    <message><source>Traversing the folder</source><translation>Esplorazione della cartella</translation></message>
    <message><source>Traversing the registry</source><translation>Esplorazione del Registro</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Impossibile preparare l’operazione</translation></message>
    <message><source>View results</source><translation>Visualizza risultati</translation></message>

    <message><source>All operations succeeded</source><translation>Tutte le operazioni sono riuscite</translation></message>
    <message><source>Some operations succeeded</source><translation>Alcune operazioni sono riuscite</translation></message>
    <message><source>No operations succeeded</source><translation>Nessuna operazione riuscita</translation></message>
    <message><source>No targets to process</source><translation>Nessuna destinazione da elaborare</translation></message>
    <message><source>Commit interrupted</source><translation>Consolidamento interrotto</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 elementi non sono stati elaborati.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 operazioni del Registro non riuscite.</translation></message>
    <message><source>The folder does not exist.</source><translation>La cartella non esiste.</translation></message>
    <message><source>Access denied.</source><translation>Accesso negato.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Esiste già un elemento con questo nome.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Impossibile creare la cartella. Controllare il nome e le autorizzazioni.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Impossibile rinominare l’elemento. Controllare il nome e le autorizzazioni.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Questo browser non supporta la ridenominazione.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>La bozza dei file predisposti è cambiata dopo l’apertura di questo piano. Esamina un nuovo piano.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>L’elenco salvato dei file predisposti è cambiato dopo l’apertura di questo piano. Aggiorna ed esamina le modifiche.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>L’elenco salvato dei file predisposti non corrisponde al piano accettato.</translation></message>
    <message><source>Export interface settings</source><translation>Esporta impostazioni dell’interfaccia</translation></message>
    <message><source>Save the current theme, language, and overlay widget visibility to UWF.ini beside this application.</source><translation>Salva il tema, la lingua e la visibilità attuali del widget dell’overlay in UWF.ini accanto all’applicazione.</translation></message>
    <message><source>Interface settings saved</source><translation>Impostazioni dell’interfaccia salvate</translation></message>
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
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Il consolidamento dei file predisposti si è interrotto. Risolvi l’errore prima di riprovare.</translation></message>
    <message><source>No net change for this setting</source><translation>Il valore finale di questa impostazione non è cambiato dopo l’importazione</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Valore finale della bozza: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Valore finale della bozza: %1 → %2 (valore richiesto: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Valore richiesto: %1 → %2; sostituito da un comando successivo. Valore finale della bozza → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Bozza di %1 aggiornata</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Bozza della protezione del volume %1 aggiornata → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Seleziona le voci da rimuovere.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Al momento non è possibile ripristinare le voci selezionate.</translation></message>
    <message>
        <source>Freespace Passthrough</source>
        <translation>Passaggio diretto tramite spazio libero</translation>
    </message>
    <message>
        <source>Persistent Overlay</source>
        <translation>Overlay persistente</translation>
    </message>
    <message>
        <source>Clear persistent data on next startup</source>
        <translation>Cancella i dati persistenti al prossimo avvio</translation>
    </message>
    <message>
        <source>Read-Only Media mode</source>
        <translation>Modalità supporto di sola lettura</translation>
    </message>
    <message>
        <source>Uses free disk space for writes, reducing reserved overlay usage; supports RAM (not recommended) and Disk (recommended).</source>
        <translation>Utilizza lo spazio libero del disco per le scritture, riducendo l’uso dell’overlay riservato; supporta RAM (sconsigliata) e Disk (consigliato).</translation>
    </message>
    <message>
        <source>Preserves writes in the Disk overlay across restarts; not available for RAM overlays.</source>
        <translation>Conserva le scritture nell’overlay Disk dopo i riavvii; non disponibile per gli overlay RAM.</translation>
    </message>
    <message>
        <source>Resets the persistent overlay: clears its writes at the next startup, restoring the state when the overlay was enabled</source>
        <translation>Reimposta l’overlay persistente: cancella le scritture al prossimo avvio e ripristina lo stato in cui è stato abilitato l’overlay</translation>
    </message>


    <message>
        <source>Administrator privileges are required; the overlay configuration is read-only.</source>
        <translation>Sono necessari privilegi di amministratore; la configurazione dell’overlay è di sola lettura.</translation>
    </message>
    <message>
        <source>Configuration editing is temporarily unavailable. Wait for the current operation to finish.</source>
        <translation>La modifica è temporaneamente non disponibile. Attendi il completamento dell’operazione corrente.</translation>
    </message>
    <message>
        <source>The overlay configuration could not be read. Refresh before editing.</source>
        <translation>Impossibile leggere la configurazione dell’overlay. Aggiorna prima di modificarla.</translation>
    </message>
    <message>
        <source>This system does not support clearing persistent data at the next startup.</source>
        <translation>Questo sistema non supporta la cancellazione dei dati persistenti al prossimo avvio.</translation>
    </message>
    <message>
        <source>The current clear request is unknown. Refresh the configuration before editing.</source>
        <translation>La richiesta di cancellazione corrente è sconosciuta. Aggiorna la configurazione prima di modificarla.</translation>
    </message>
    <message>
        <source>Changing this request requires UWF and Disk Persistent Overlay to be active in the current session.</source>
        <translation>Per modificare questa richiesta, UWF e l’overlay persistente Disk devono essere attivi nella sessione corrente.</translation>
    </message>
    <message>
        <source>Clearing persistent data at the next startup requires Disk Persistent Overlay. Cancel the clear request or retain Persistent Overlay.</source>
        <translation>La cancellazione al prossimo avvio richiede di mantenere l’overlay persistente Disk. Annulla la richiesta o mantieni l’overlay persistente.</translation>
    </message>
    <message>
        <source>Persistent Overlay requires Disk. Disable Persistent Overlay or select Disk.</source>
        <translation>L’overlay persistente richiede Disk. Disabilitalo o seleziona Disk.</translation>
    </message>
    <message>
        <source>Read-Only Media mode requires RAM. Disable Read-Only Media mode or select RAM.</source>
        <translation>La modalità supporto di sola lettura richiede RAM. Disabilitala o seleziona RAM.</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session. Disable UWF and restart before changing this setting.</source>
        <translation>UWF è abilitato nella sessione corrente. Disabilitare UWF e riavviare prima di modificare questa impostazione.</translation>
    </message>
    <message>
        <source>The configuration has changed. Close and reopen this dialog before editing.</source>
        <translation>La configurazione è cambiata. Chiudi e riapri questa finestra prima di modificarla.</translation>
    </message>
    <message>
        <source>Overlay advanced settings</source>
        <translation>Impostazioni avanzate dell’overlay</translation>
    </message>

    <message>
        <source>Current overlay</source>
        <translation>Overlay corrente</translation>
    </message>
    <message>
        <source>Next overlay</source>
        <translation>Overlay successivo</translation>
    </message>





    <message>
        <source>The draft contains incompatible or unavailable overlay settings. Return to editing and correct the overlay configuration.</source>
        <translation>La bozza contiene impostazioni dell’overlay incompatibili o non disponibili. Torna alla modifica e correggi la configurazione.</translation>
    </message>
    <message>
        <source>A clear is scheduled for the next startup. Cancel the clear request before disabling Persistent Overlay.</source>
        <translation>È pianificata una cancellazione al prossimo avvio. Annulla la richiesta prima di disabilitare l’overlay persistente.</translation>
    </message>
    <message>
        <source>The next session does not retain Disk Persistent Overlay. Retain it before requesting a clear.</source>
        <translation>La prossima sessione non manterrà l’overlay persistente Disk. Mantienilo prima di richiedere una cancellazione.</translation>
    </message>
    <message>
        <source>Persistent Overlay is only available for Disk; the next overlay is RAM.</source>
        <translation>L’overlay persistente è disponibile solo per Disk; il prossimo overlay è RAM.</translation>
    </message>
    <message>
        <source>Enabling Read-Only Media mode requires RAM; the next overlay is Disk.</source>
        <translation>La modalità supporto di sola lettura richiede RAM; il prossimo overlay è Disk.</translation>
    </message>
    <message>
        <source>The current Persistent Overlay state could not be confirmed. Read the configuration again.</source>
        <translation>Impossibile confermare lo stato attuale dell’overlay persistente. Leggi di nuovo la configurazione.</translation>
    </message>
    <message>
        <source>Will take effect at the next startup after applying.</source>
        <translation>Avrà effetto al prossimo avvio dopo l’applicazione.</translation>
    </message>
    <message>
        <source>Scheduled to clear</source>
        <translation>Cancellazione pianificata</translation>
    </message>
    <message>
        <source>Not scheduled to clear</source>
        <translation>Nessuna cancellazione pianificata</translation>
    </message>
    <message>
        <source>Clears once at the next startup, then preserves data. The current session is not cleared immediately.</source>
        <translation>Cancella una sola volta al prossimo avvio, quindi conserva i dati. La sessione corrente non viene cancellata immediatamente.</translation>
    </message>
    <message>
        <source>Cancels the scheduled clear at the next startup, preserving persistent overlay data.</source>
        <translation>Annulla la cancellazione pianificata al prossimo avvio e conserva i dati dell’overlay persistente.</translation>
    </message>
    <message>
        <source>Reset persistent overlay · One-time request</source>
        <translation>Reimposta overlay persistente · Richiesta singola</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Operazione in corso…</translation>
    </message>
    <message>
        <source>Please wait for the operation to finish.</source>
        <translation>Attendere il completamento dell'operazione.</translation>
    </message>
    <message>
        <source>%1 · Next session</source>
        <translation>%1 · Prossima sessione</translation>
    </message>
    <message>
        <source>Back to settings</source>
        <translation>Torna alle impostazioni</translation>
    </message>
    <message>
        <source>Change requirements</source>
        <translation>Requisiti per la modifica</translation>
    </message>
    <message>
        <source>Changed</source>
        <translation>Modificato</translation>
    </message>
    <message>
        <source>Check configuration conflicts</source>
        <translation>Verifica conflitti di configurazione</translation>
    </message>
    <message>
        <source>Configuration conflict</source>
        <translation>Conflitto di configurazione</translation>
    </message>
    <message>
        <source>Disable requirements</source>
        <translation>Requisiti per la disattivazione</translation>
    </message>
    <message>
        <source>UWF and Disk Persistent Overlay must be active in the current session. Clearing data also requires retaining Disk Persistent Overlay for the next session.</source>
        <translation>UWF e l’overlay persistente Disk devono essere attivi nella sessione corrente. Per cancellare i dati, è necessario mantenere l’overlay persistente Disk anche per la sessione successiva.</translation>
    </message>
    <message>
        <source>Enable requirements</source>
        <translation>Requisiti per l’attivazione</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Funzione</translation>
    </message>
    <message>
        <source>Feature details</source>
        <translation>Descrizione della funzione</translation>
    </message>
    <message>
        <source>Learn about %1</source>
        <translation>Informazioni su %1</translation>
    </message>
    <message>
        <source>Resolve these configuration conflicts before keeping the draft.</source>
        <translation>Risolvi questi conflitti di configurazione prima di mantenere la bozza.</translation>
    </message>
    <message>
        <source>Revert</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <source>Takes effect</source>
        <translation>Entrata in vigore</translation>
    </message>
    <message>
        <source>This setting cannot be changed right now</source>
        <translation>Al momento non è possibile modificare questa impostazione</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use Disk.</source>
        <translation>UWF deve essere disabilitato nella sessione corrente e l’overlay della prossima sessione deve usare Disk.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session, and the next-session overlay must use RAM.</source>
        <translation>UWF deve essere disabilitato nella sessione corrente e l’overlay della prossima sessione deve usare RAM.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Cancel any scheduled persistent-data clear before disabling Persistent Overlay.</source>
        <translation>UWF deve essere disabilitato nella sessione corrente. Annulla l’eventuale cancellazione pianificata dei dati persistenti prima di disabilitare l’overlay persistente.</translation>
    </message>
    <message>
        <source>UWF must be disabled in the current session. Setting only the next session to disabled still requires a restart before editing.</source>
        <translation>UWF deve essere disabilitato nella sessione corrente. Se lo disabiliti solo per la prossima sessione, devi comunque riavviare prima di modificare le impostazioni.</translation>
    </message>
    <message>
        <source>Usage requirements</source>
        <translation>Requisiti per l’utilizzo</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Mostra dettagli</translation>
    </message>
    <message>
        <source>View only</source>
        <translation>Sola visualizzazione</translation>
    </message>
    <message>
        <source>Why %1 cannot be changed</source>
        <translation>Perché non è possibile modificare %1</translation>
    </message>
    <message>
        <source>UWF is enabled in the current session, so these overlay settings cannot be changed. Revert the changes, or disable UWF and restart.</source>
        <translation>UWF è abilitato nella sessione corrente, quindi queste impostazioni di overlay non possono essere modificate. Annullare le modifiche oppure disabilitare UWF e riavviare.</translation>
    </message>
    <message>
        <source>View change requirements</source>
        <translation>Mostra i requisiti per la modifica</translation>
    </message>
    <message>
        <source>No configuration changes were written. You can retry.</source>
        <translation>Non è stata scritta alcuna modifica alla configurazione. È possibile riprovare.</translation>
    </message>
    <message>
        <source>No configuration changes were needed.</source>
        <translation>Non erano necessarie modifiche alla configurazione.</translation>
    </message>
    <message>
        <source>Use Domain Secret Key persistence for the domain machine account secret ($MACHINE.ACC).</source>
        <translation>Usare «Persistenza della chiave segreta di dominio» per il segreto dell’account computer del dominio ($MACHINE.ACC).</translation>
    </message>
    <message>
        <source>This key cannot be excluded itself. Select one of its subkeys.</source>
        <translation>Questa chiave non può essere esclusa direttamente. Selezionare una delle sue sottochiavi.</translation>
    </message>
    <message>
        <source>Registry exclusions must specify a nonempty subkey under HKEY_LOCAL_MACHINE (HKLM).</source>
        <translation>Le esclusioni del registro devono specificare una sottochiave non vuota in HKEY_LOCAL_MACHINE (HKLM).</translation>
    </message>
    <message>
        <source>This script contains only settings supported by uwfmgr commands. Not included:</source>
        <translation>Questo script include solo le impostazioni esprimibili tramite comandi uwfmgr. Non include:</translation>
    </message>
    <message>
        <source>Volume binding</source>
        <translation>Associazione dei volumi</translation>
    </message>
    <message>
        <source>Settings without a reliable drive-letter address</source>
        <translation>Impostazioni senza un indirizzo affidabile tramite lettera di unità</translation>
    </message>
    <message>
        <source>Command script</source>
        <translation>Script di comandi</translation>
    </message>
    <message>
        <source>The configuration matches the next-session target. View or export the supported settings in Command script.</source>
        <translation>La configurazione corrisponde all’obiettivo della prossima sessione. È possibile visualizzare o esportare le impostazioni supportate in «Script di comandi».</translation>
    </message>
    <message>
        <source>Settings with control characters in paths or names</source>
        <translation>Impostazioni con caratteri di controllo nei percorsi o nei nomi</translation>
    </message>
    <message>
        <source>Commands that depend on omitted settings</source>
        <translation>Comandi che dipendono da impostazioni omesse</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of this target. Remove the conflicting exclusion and restart before committing it.</source>
        <translation>Le esclusioni UWF impediscono di consolidare in modo affidabile questa destinazione. Rimuovere l’esclusione in conflitto e riavviare prima di consolidarla.</translation>
    </message>
    <message>
        <source>UWF exclusion settings prevent a reliable commit of the file staging list. Remove the conflicting exclusion and restart before applying this change.</source>
        <translation>Le esclusioni UWF impediscono di consolidare in modo affidabile l’elenco dei file predisposti. Rimuovere l’esclusione in conflitto e riavviare prima di applicare questa modifica.</translation>
    </message>
    <message>
        <source>UWF cannot preserve the exact registry target for this key.</source>
        <translation>UWF non può garantire che l’operazione mantenga esattamente questa chiave come destinazione del registro.</translation>
    </message>
    <message>
        <source>Confirm that the saved staging list is empty before removing its registry exclusion.</source>
        <translation>Confermare che l’elenco salvato dei file predisposti sia vuoto prima di rimuoverne l’esclusione dal registro.</translation>
    </message>
    <message>
        <source>Pending confirmation</source>
        <translation>In attesa di conferma</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Vuoto</translation>
    </message>
    <message>
        <source>The system volume is not currently protected by UWF; there are no registry changes to commit.</source>
        <translation>Il volume di sistema non è attualmente protetto da UWF; non ci sono modifiche al registro da consolidare.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; no UWF commit is required.</source>
        <translation>Le modifiche al registro sono già persistenti; non è necessario consolidarle tramite UWF.</translation>
    </message>
    <message>
        <source>Registry changes are already persistent; UWF cannot perform this deletion.</source>
        <translation>Le modifiche al registro sono già persistenti; UWF non può eseguire questa eliminazione.</translation>
    </message>
    <message><source>UWF exclusion paths must use backslashes.</source><translation>I percorsi di esclusione UWF devono usare barre rovesciate.</translation></message>
    <message><source>File paths cannot contain null characters.</source><translation>I percorsi dei file non possono contenere caratteri nulli.</translation></message>
    <message><source>Enable volume protection in this draft before adding file exclusions.</source><translation>Attivare la protezione del volume in questa bozza prima di aggiungere esclusioni di file.</translation></message>
    <message><source>The exclusion path contains characters that UWF does not support.</source><translation>Il percorso di esclusione contiene caratteri non supportati da UWF.</translation></message>
    <message><source>The exclusion path exceeds UWF's length limit.</source><translation>Il percorso di esclusione supera il limite di lunghezza di UWF.</translation></message>
    <message><source>File and registry commits are unavailable while HORM is active.</source><translation>Non è possibile confermare le modifiche a file e Registro mentre HORM è attivo.</translation></message>
    <message><source>Registry writes are unavailable while HORM is active.</source><translation>Non è possibile scrivere nel Registro mentre HORM è attivo.</translation></message>
    <message><source>Registry writes cannot be persisted while Read-Only Media mode is active.</source><translation>Le scritture nel Registro non possono essere salvate in modo permanente mentre è attiva la modalità supporti di sola lettura.</translation></message>
    <message><source>File and registry commits are unavailable while Read-Only Media mode is active.</source><translation>Non è possibile confermare le modifiche a file e Registro mentre è attiva la modalità supporti di sola lettura.</translation></message>
    <message><source>Intercepts physical media writes, including metadata. Exclusions do not bypass protection. Individual file and registry commits are unavailable; Full Commit is a separate global operation.</source><translation>Intercetta le scritture sui supporti fisici, inclusi i metadati. Le esclusioni non aggirano la protezione. Non è possibile confermare singole modifiche a file e Registro; Full Commit è un’operazione globale separata.</translation></message>
    <message><source>UWF settings cannot be changed while HORM is active.</source><translation>Non è possibile modificare le impostazioni UWF mentre HORM è attivo.</translation></message>
    <message><source>Default value · this value only</source><translation>Valore predefinito · solo questo valore</translation></message>
    <message><source>File changes are already persistent; no UWF commit is required.</source><translation>Le modifiche ai file sono già persistenti; non è necessario confermarle con UWF.</translation></message>
    <message><source>Last successful overlay usage observation: %1</source><translation>Ultima osservazione riuscita dell’utilizzo dell’overlay: %1</translation></message>
    <message><source>Observed overlay storage outside the writable size/type range</source><translation>L’archiviazione overlay osservata non rientra nei limiti di dimensione o tipo scrivibili</translation></message>
    <message><source>Overlay (last observed)</source><translation>Overlay (ultima osservazione)</translation></message>
    <message><source>Overlay usage refresh failed.
%1</source><translation>Aggiornamento dell’utilizzo dell’overlay non riuscito.
%1</translation></message>
    <message><source>Overlay usage refresh failed. Showing the last successful observation from %1.
%2</source><translation>Aggiornamento dell’utilizzo dell’overlay non riuscito. Viene mostrata l’ultima osservazione riuscita del %1.
%2</translation></message>
    <message><source>The path could not be resolved to a local volume.</source><translation>Impossibile associare il percorso a un volume locale.</translation></message>
    <message><source>UWF: Enabled (last observed)</source><translation>UWF: abilitato (ultima osservazione)</translation></message>
    <message><source>File changes are already persistent; UWF cannot perform this deletion.</source><translation>Le modifiche ai file sono già persistenti; UWF non può eseguire questa eliminazione.</translation></message>
    <message><source>%1 (%2)</source><translation>%1 (%2)</translation></message>
    <message><source>%1 GB available</source><translation>%1 GB disponibili</translation></message>
    <message><source>%1 MB</source><translation>%1 MB</translation></message>
    <message><source>%1 · %2</source><translation>%1 · %2</translation></message>

    <message><source>Administrator privileges are required.</source><translation>Sono necessari i privilegi di amministratore.</translation></message>


    <message><source>An operation is in progress. Please wait.</source><translation>È in corso un’operazione. Attendere.</translation></message>
    <message><source>Attempts to save all additions, changes and deletions made on every protected volume since this startup. This includes files and system settings; you cannot select only some changes.</source><translation>Tenta di salvare tutte le aggiunte, modifiche ed eliminazioni effettuate su ogni volume protetto dall’avvio corrente. Sono inclusi file e impostazioni di sistema; non è possibile selezionare solo alcune modifiche.</translation></message>

    <message><source>Automatic saving is unavailable in the current mode</source><translation>Il salvataggio automatico non è disponibile nella modalità attuale</translation></message>
    <message><source>Cannot be undone once started</source><translation>Non può essere annullato dopo l’avvio</translation></message>
    <message><source>Choose where to store the disk overlay</source><translation>Scegliere dove archiviare l’overlay Disk</translation></message>
    <message><source>Choose where to store the disk overlay.</source><translation>Scegliere dove archiviare l’overlay Disk.</translation></message>
    <message><source>Configuration changes are temporarily unavailable.</source><translation>Le modifiche alla configurazione non sono temporaneamente disponibili.</translation></message>
    <message><source>Configured, inactive this session</source><translation>Configurato, inattivo in questa sessione</translation></message>
    <message><source>Configured; takes effect after restarting</source><translation>Configurato; avrà effetto dopo il riavvio</translation></message>
    <message><source>Configured; takes effect after restarting. The overlay from this startup will not be moved to the new location.</source><translation>Configurato; avrà effetto dopo il riavvio. L’overlay dell’avvio corrente non verrà spostato nella nuova posizione.</translation></message>
    <message><source>Configure…</source><translation>Configura…</translation></message>
    <message><source>Current UWF</source><translation>UWF nella sessione attuale</translation></message>
    <message><source>Currently configured storage volume</source><translation>Volume di archiviazione attualmente configurato</translation></message>
    <message><source>Default behavior</source><translation>Comportamento predefinito</translation></message>
    <message><source>Disable HORM and restart before using full volume commit.</source><translation>Disattivare HORM e riavviare prima di consolidare l’intero volume.</translation></message>
    <message><source>Disabled · restart required</source><translation>Disattivato · riavvio necessario</translation></message>
    <message><source>Disabling HORM does not turn off UWF or Read-Only Media mode and does not save changes on protected disks. Restart before checking the available actions.</source><translation>La disattivazione di HORM non disattiva UWF o la modalità per supporti di sola lettura e non salva le modifiche sui dischi protetti. Riavviare prima di controllare le azioni disponibili.</translation></message>

    <message><source>Enable UWF and restart before enabling HORM.</source><translation>Attivare UWF e riavviare prima di attivare HORM.</translation></message>

    <message><source>Full volume commit</source><translation>Consolida l’intero volume</translation></message>
    <message><source>Full volume commit mode (enabled until restart)</source><translation>Modalità di consolidamento dell’intero volume (attiva fino al riavvio)</translation></message>
    <message><source>Full volume commit mode is already enabled. Restart before using it again.</source><translation>La modalità di consolidamento dell’intero volume è già attiva. Riavviare prima di utilizzarla di nuovo.</translation></message>
    <message><source>Full volume commit has started. Restart before enabling HORM.</source><translation>Il consolidamento dell’intero volume è iniziato. Riavviare prima di attivare HORM.</translation></message>
    <message><source>Full volume commit is available only while Read-Only Media mode is active.</source><translation>Il consolidamento dell’intero volume è disponibile solo quando è attiva la modalità per supporti di sola lettura.</translation></message>
    <message><source>Full volume commit mode remains enabled until restart. New changes are saved directly. Individual commits remain unavailable.</source><translation>La modalità di consolidamento dell’intero volume rimane attiva fino al riavvio. Le nuove modifiche vengono salvate direttamente. I consolidamenti individuali rimangono non disponibili.</translation></message>
    <message><source>Full volume commit is not available on this system.</source><translation>Il consolidamento dell’intero volume non è disponibile su questo sistema.</translation></message>

    <message><source>HORM is active. UWF settings and individual commits are unavailable. Disable HORM and restart before changing them.</source><translation>HORM è attivo. Le impostazioni UWF e i consolidamenti individuali non sono disponibili. Disattivare HORM e riavviare prima di apportare modifiche.</translation></message>
    <message><source>HORM is disabled. Restart the computer before changing UWF settings.</source><translation>HORM è disattivato. Riavviare il computer prima di modificare le impostazioni UWF.</translation></message>
    <message><source>HORM is not available on this system.</source><translation>HORM non è disponibile su questo sistema.</translation></message>
    <message><source>HORM requires the current RAM Read-Only Media configuration.</source><translation>HORM richiede che la configurazione attuale sia RAM con la modalità per supporti di sola lettura.</translation></message>
    <message><source>HORM requires the current RAM overlay.</source><translation>HORM richiede che l’overlay attuale sia RAM.</translation></message>

    <message><source>In Disk mode, the system implementation may enable this option by default; check the current state shown in settings.</source><translation>In modalità Disk, l’implementazione del sistema può attivare questa opzione per impostazione predefinita; controllare lo stato attuale mostrato nelle impostazioni.</translation></message>
    <message><source>Inactive this session</source><translation>Inattivo in questa sessione</translation></message>
    <message><source>Later changes will also be saved until you restart</source><translation>Anche le modifiche successive verranno salvate fino al riavvio</translation></message>

    <message><source>Maintenance and recovery</source><translation>Manutenzione e ripristino</translation></message>
    <message><source>Make sure this is the state you want to keep before continuing.</source><translation>Prima di continuare, assicurarsi che questo sia lo stato da conservare.</translation></message>

    <message><source>Next startup: %1</source><translation>Prossimo avvio: %1</translation></message>
    <message><source>No suitable storage volume is available. Refresh the volume list.</source><translation>Non è disponibile alcun volume di archiviazione adatto. Aggiornare l’elenco dei volumi.</translation></message>
    <message><source>Not enabled</source><translation>Non attivato</translation></message>
    <message><source>Not selected</source><translation>Non selezionato</translation></message>
    <message><source>Not supported</source><translation>Non supportato</translation></message>
    <message><source>Once started, new changes are written directly to disk until you restart the computer.</source><translation>Dopo l’avvio dell’operazione, le nuove modifiche vengono scritte direttamente sul disco fino al riavvio del computer.</translation></message>
    <message><source>Only one storage volume can be used at a time. Choosing a new volume replaces the previous location and sets the next overlay to Disk.</source><translation>È possibile usare un solo volume di archiviazione alla volta. La scelta di un nuovo volume sostituisce la posizione precedente e imposta il prossimo overlay su Disk.</translation></message>
    <message><source>Overlay capacity (keep current setting)</source><translation>Capacità overlay (mantieni l’impostazione attuale)</translation></message>
    <message><source>Overlay configuration</source><translation>Configurazione overlay</translation></message>

    <message><source>Processing…</source><translation>Elaborazione…</translation></message>
    <message><source>Read state again</source><translation>Rileggi lo stato</translation></message>
    <message><source>Read the current UWF state first.</source><translation>Leggere prima lo stato attuale di UWF.</translation></message>

    <message><source>Read-only media mode does not support individual commits or automatic saving of staged files.</source><translation>La modalità per supporti di sola lettura non supporta i consolidamenti individuali o il salvataggio automatico dei file predisposti.</translation></message>
    <message><source>Reading state…</source><translation>Lettura dello stato…</translation></message>
    <message><source>Restart and read the current state before enabling HORM again.</source><translation>Riavviare e leggere lo stato attuale prima di attivare nuovamente HORM.</translation></message>
    <message><source>Restore the same desktop and running programs at every startup.</source><translation>Ripristina lo stesso desktop e gli stessi programmi in esecuzione a ogni avvio.</translation></message>
    <message><source>Review available commit operations and their requirements.</source><translation>Esaminare le operazioni di consolidamento disponibili e i relativi requisiti.</translation></message>
    <message><source>Save changes on all protected volumes</source><translation>Salva le modifiche su tutti i volumi protetti</translation></message>
    <message><source>Save changes on all protected volumes. Review the effects before continuing.</source><translation>Salva le modifiche su tutti i volumi protetti. Esaminare gli effetti prima di continuare.</translation></message>


    <message><source>State unknown</source><translation>Stato sconosciuto</translation></message>

    <message><source>Successfully saved changes can no longer be undone by restarting.</source><translation>Le modifiche salvate correttamente non possono più essere annullate con un riavvio.</translation></message>
    <message><source>Successfully saved changes will remain</source><translation>Le modifiche salvate correttamente verranno conservate</translation></message>
    <message><source>The HORM state is unknown. Read the state again before continuing.</source><translation>Lo stato di HORM è sconosciuto. Rileggere lo stato prima di continuare.</translation></message>
    <message><source>The HORM state is unknown. Refresh before continuing.</source><translation>Lo stato di HORM è sconosciuto. Aggiornare lo stato prima di continuare.</translation></message>
    <message><source>The commit or HORM state is unknown. Refresh before continuing.</source><translation>Lo stato di consolidamento o di HORM è sconosciuto. Aggiornare lo stato prima di continuare.</translation></message>
    <message><source>The current UWF state could not be read. Refresh before continuing.</source><translation>Impossibile leggere lo stato attuale di UWF. Aggiornare lo stato prima di continuare.</translation></message>
    <message><source>The current overlay uses RAM Read-Only Media mode</source><translation>L’overlay attuale usa RAM con la modalità per supporti di sola lettura</translation></message>
    <message><source>The current protection mode could not be read. Refresh the state.</source><translation>Impossibile leggere la modalità di protezione attuale. Aggiornare lo stato.</translation></message>
    <message><source>The result could not be confirmed</source><translation>Impossibile confermare il risultato</translation></message>
    <message><source>This exclusion is configured but does not bypass protection in the current session.</source><translation>Questa esclusione è configurata, ma non aggira la protezione nella sessione attuale.</translation></message>
    <message><source>This may also change Freespace Passthrough and Persistent Overlay. Review advanced settings before restarting.</source><translation>Questo può modificare anche il passaggio diretto tramite spazio libero e l’overlay persistente. Controllare le impostazioni avanzate prima di riavviare.</translation></message>
    <message><source>UWF protection is active in the current session</source><translation>La protezione UWF è attiva nella sessione attuale</translation></message>
    <message><source>%1 (will be replaced)</source><translation>%1 (verrà sostituito)</translation></message>
    <message><source>Add full volume commit to the draft?</source><translation>Aggiungere il consolidamento dell’intero volume alla bozza?</translation></message>
    <message><source>Add to draft</source><translation>Aggiungi alla bozza</translation></message>
    <message><source>Applies to every protected volume; starts only when this draft is applied.</source><translation>Si applica a tutti i volumi protetti; inizia solo quando viene applicata questa bozza.</translation></message>
    <message><source>Applying this draft starts full volume commit. Saved changes cannot be undone by restarting, and new changes are saved until restart.</source><translation>L’applicazione di questa bozza avvia il consolidamento dell’intero volume. Le modifiche salvate non possono essere annullate con un riavvio e le nuove modifiche vengono salvate fino al riavvio.</translation></message>
    <message><source>Configure storage location</source><translation>Configura la posizione di archiviazione</translation></message>
    <message><source>Configuring the storage location is not available on this system.</source><translation>La configurazione della posizione di archiviazione non è disponibile su questo sistema.</translation></message>
    <message><source>Current: %1</source><translation>Attuale: %1</translation></message>
    <message><source>Disable HORM and restart before configuring the storage location.</source><translation>Disattivare HORM e riavviare prima di configurare la posizione di archiviazione.</translation></message>
    <message><source>Disable saved-state startup?</source><translation>Disattivare l’avvio dallo stato salvato?</translation></message>
    <message><source>Draft: %1 · apply required</source><translation>Bozza: %1 · da applicare</translation></message>
    <message><source>Enable saved-state startup?</source><translation>Attivare l’avvio dallo stato salvato?</translation></message>
    <message><source>Full volume commit (pending apply)</source><translation>Consolida l’intero volume (da applicare)</translation></message>
    <message><source>Full volume commit is pending. It will start only after you confirm Review and apply.</source><translation>Il consolidamento dell’intero volume è in attesa. Inizierà solo dopo la conferma in «Verifica e applica».</translation></message>
    <message><source>Hibernate once / resume many</source><translation>Iberna una volta / riprendi più volte</translation></message>
    <message><source>Not started</source><translation>Non avviato</translation></message>
    <message><source>Remove full volume commit from pending changes.</source><translation>Rimuovi il consolidamento dell’intero volume dalle modifiche in sospeso.</translation></message>
    <message><source>Save draft</source><translation>Salva bozza</translation></message>

    <message><source>Will start when applied</source><translation>Inizierà all’applicazione</translation></message>
    <message><source>A mounted fixed disk failed the protection check. Review the listed disks before enabling HORM.</source><translation>Un disco fisso montato non ha superato il controllo di protezione. Controllare i dischi elencati prima di attivare HORM.</translation></message>
    <message><source>Checks UWF-eligible fixed disks that remain mounted. Unmounted partitions may be skipped.</source><translation>Controlla i dischi fissi idonei per UWF che rimangono montati. Le partizioni non montate possono essere ignorate.</translation></message>
    <message><source>HORM prerequisites could not be verified. Read the state again before continuing.</source><translation>Impossibile verificare i prerequisiti di HORM. Rileggere lo stato prima di continuare.</translation></message>
    <message><source>Mounted fixed-disk protection check</source><translation>Controllo di protezione dei dischi fissi montati</translation></message>
    <message><source>Not using a Disk overlay</source><translation>Non è in uso un overlay Disk</translation></message>
    <message><source>Selected storage volume</source><translation>Volume di archiviazione selezionato</translation></message>
    <message><source>System drive default (%1)</source><translation>Predefinito: unità di sistema (%1)</translation></message>

    <message><source>The protection state changed during this startup. Restart before enabling HORM.</source><translation>Lo stato di protezione è cambiato durante questo avvio. Riavviare prima di attivare HORM.</translation></message>
    <message><source>Unable to read</source><translation>Impossibile leggere</translation></message>

    <message><source>Choose the disk used to store temporary changes. Takes effect at the next startup.</source><translation>Scegliere il disco su cui archiviare le modifiche temporanee. Avrà effetto al prossimo avvio.</translation></message>
    <message><source>Disable UWF and restart before configuring the storage location.</source><translation>Disattivare UWF e riavviare prima di configurare la posizione di archiviazione.</translation></message>
    <message><source>Met</source><translation>Soddisfatto</translation></message>
    <message><source>Not met</source><translation>Non soddisfatto</translation></message>
    <message><source>Partition</source><translation>Partizione</translation></message>


    <message><source>Protect all</source><translation>Proteggi tutti</translation></message>
    <message><source>Hibernation is not supported</source><translation>Ibernazione non supportata</translation></message>
    <message><source>Hibernation is not enabled</source><translation>Ibernazione non attivata</translation></message>
    <message><source>Fast startup only</source><translation>Solo avvio rapido</translation></message>
    <message><source>Bulk volume protection is not available on this system.</source><translation>La protezione dei volumi in blocco non è disponibile su questo sistema.</translation></message>
    <message><source>Disable HORM and restart before changing volume protection.</source><translation>Disattivare HORM e riavviare prima di modificare la protezione dei volumi.</translation></message>
    <message><source>Setup and operation</source><translation>Configurazione e funzionamento</translation></message>

    <message><source>1. Prepare the configuration</source><translation>1. Preparare la configurazione</translation></message>
    <message><source>With UWF currently disabled, configure RAM overlay, read-only media mode, volume protection, and UWF enablement for the next startup. Apply the draft. Prepare S4 hibernation with a present, full hibernation file.</source><translation>Con UWF disattivato nella sessione attuale, configurare l’overlay RAM, la modalità supporto di sola lettura, la protezione dei volumi e l’attivazione di UWF per il prossimo avvio. Applicare la bozza. Preparare l’ibernazione S4 con un file di ibernazione completo e presente.</translation></message>
    <message><source>Restart, then confirm that Current shows UWF enabled, RAM overlay, read-only media mode, and protection for all UWF-eligible mounted fixed volumes. Confirm that system hibernation is available.</source><translation>Riavviare, quindi verificare nello stato attuale che UWF sia attivo, che siano in uso l’overlay RAM e la modalità supporto di sola lettura e che tutti i volumi fissi montati idonei a UWF siano protetti. Verificare che l’ibernazione del sistema sia disponibile.</translation></message>
    <message><source>2. Restart and verify</source><translation>2. Riavviare e verificare</translation></message>
    <message><source>3. Enable HORM and hibernate</source><translation>3. Attivare HORM e ibernare</translation></message>
    <message><source>Add Enable HORM to the draft, then use Review and apply. Applying the draft performs the HORM enable operation, equivalent to:</source><translation>Aggiungere l’attivazione di HORM alla bozza, quindi usare «Verifica e applica». L’applicazione della bozza attiva HORM, con un’operazione equivalente a:</translation></message>
    <message><source>After enablement succeeds, prepare the desktop and applications, then hibernate once to save the baseline state:</source><translation>Dopo l’attivazione riuscita, preparare il desktop e le applicazioni, quindi ibernare una volta per salvare lo stato di riferimento:</translation></message>
    <message><source>HORM usage</source><translation>Uso di HORM</translation></message>
    <message><source>Volume protection</source><translation>Protezione dei volumi</translation></message>
    <message><source>Configure protection for all volumes in the next session.</source><translation>Configurare la protezione di tutti i volumi per la prossima sessione.</translation></message>
    <message><source>Unprotect all</source><translation>Rimuovi protezione da tutti</translation></message>
    <message><source>Next startup: all volumes protected</source><translation>Prossimo avvio: tutti i volumi protetti</translation></message>
    <message><source>Next startup: all volumes unprotected</source><translation>Prossimo avvio: nessun volume protetto</translation></message>
    <message><source>Next startup: individual volume settings</source><translation>Prossimo avvio: impostazioni dei singoli volumi</translation></message>
    <message><source>System hibernation is available</source><translation>Ibernazione del sistema disponibile</translation></message>
    <message><source>Usage guide</source><translation>Guida all’uso</translation></message>

    <message><source>Protection for all volumes is pending apply</source><translation>La protezione di tutti i volumi è in attesa di applicazione</translation></message>
    <message><source>Volume protection takes effect after restarting</source><translation>La protezione dei volumi ha effetto dopo il riavvio</translation></message>

    <message><source>Unprotect all and enabling HORM cannot be applied together.</source><translation>Non è possibile rimuovere la protezione da tutti i volumi e attivare HORM nella stessa applicazione.</translation></message>
    <message><source>Batch volume protection is not supported.</source><translation>La protezione dei volumi in blocco non è supportata.</translation></message>
    <message><source>Applied</source><translation>Applicato</translation></message>
    <message><source>Not applied</source><translation>Non applicato</translation></message>
    <message><source>Could not confirm</source><translation>Impossibile confermare</translation></message>



    <message><source>Storage volume:</source><translation>Volume di archiviazione:</translation></message>
    <message><source>Exclusions do not bypass protection during this startup. Individual commits and automatic file staging commits are unavailable. Full volume commit affects all protected volumes; review its description before continuing.</source><translation>Le esclusioni non aggirano la protezione durante questa sessione di avvio. Il salvataggio dei singoli elementi e quello automatico dei file preparati non sono disponibili. Il salvataggio completo dei volumi interessa tutti i volumi protetti; leggere la descrizione prima di continuare.</translation></message>
    <message><source>Resume the same hibernation state repeatedly</source><translation>Riprendere più volte lo stesso stato di ibernazione</translation></message>
    <message><source>Remove full volume commit draft</source><translation>Rimuovi la bozza di consolidamento dell’intero volume</translation></message>
    <message><source>Read-Only Media mode is active</source><translation>La modalità supporti di sola lettura è attiva</translation></message>
    <message><source>%1 → %2</source><translation>%1 → %2</translation></message>
    <message><source>A configured storage marker could not be read.</source><translation>Impossibile leggere un indicatore di archiviazione configurato.</translation></message>
    <message><source>File staging conflicts with a file exclusion:
%1
%2</source><translation>I file predisposti sono in conflitto con un’esclusione di file:
%1
%2</translation></message>
    <message><source>Full hibernation is not available on this computer</source><translation>L’ibernazione completa non è disponibile su questo computer</translation></message>
    <message><source>More than one storage volume has a nonzero marker.</source><translation>Più volumi di archiviazione hanno un indicatore diverso da zero.</translation></message>
    <message><source>The Disk overlay type could not be established.</source><translation>Impossibile confermare che la sovrapposizione sia di tipo Disco.</translation></message>
    <message><source>The configured storage location could not be read.</source><translation>Impossibile leggere la posizione di archiviazione configurata.</translation></message>
    <message><source>The system drive could not be identified.</source><translation>Impossibile identificare l’unità di sistema.</translation></message>
    <message><source>Direct restart skips staged file commits.</source><translation>Il riavvio diretto salta il consolidamento dei file predisposti.</translation></message>
    <message><source>Direct shutdown skips staged file commits.</source><translation>L’arresto diretto salta il consolidamento dei file predisposti.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after restart.</source><translation>I file predisposti non verranno consolidati. Le modifiche non consolidate potrebbero andare perse dopo il riavvio.</translation></message>
    <message><source>Staged files will not be committed. Uncommitted changes may be lost after shutdown.</source><translation>I file predisposti non verranno consolidati. Le modifiche non consolidate potrebbero andare perse dopo l’arresto.</translation></message>
    <message><source>Commits staged files before normal shutdown or restart.</source><translation>Consolida i file predisposti prima del normale arresto o riavvio.</translation></message>
    <message><source>Full volume commit and enabling HORM cannot be applied together.</source><translation>Il consolidamento dell’intero volume e l’attivazione di HORM non possono essere applicati insieme.</translation></message>
    <message><source>Configuring the storage location and enabling HORM cannot be applied together.</source><translation>La configurazione della posizione di archiviazione e l’attivazione di HORM non possono essere applicate insieme.</translation></message>
    <message><source>Configuring the storage location and full volume commit require different current UWF modes.</source><translation>La configurazione della posizione di archiviazione e il consolidamento dell’intero volume richiedono modalità UWF correnti diverse.</translation></message>
    <message><source>Choose a storage volume before applying.</source><translation>Scegli un volume di archiviazione prima di applicare le modifiche.</translation></message>
    <message><source>Configuring a disk storage location conflicts with a requested RAM overlay.</source><translation>La configurazione di una posizione di archiviazione su disco è in conflitto con l’overlay RAM richiesto.</translation></message>
    <message><source>Apply the filter change and restart before requesting full volume commit.</source><translation>Applica la modifica del filtro e riavvia prima di richiedere il consolidamento dell’intero volume.</translation></message>
    <message><source>Canceled</source><translation>Annullato</translation></message>
    <message><source>The reviewed storage volume is no longer available. Return to editing and choose another volume.</source><translation>Il volume di archiviazione esaminato non è più disponibile. Torna alla modifica e scegli un altro volume.</translation></message>
    <message><source>This volume cannot be registered with UWF because its storage bus is not supported.</source><translation>Questo volume non può essere registrato in UWF perché il suo bus di archiviazione non è supportato.</translation></message>
    <message><source>Physical volume support could not be confirmed. Refresh before enabling protection.</source><translation>Impossibile confermare il supporto del volume fisico. Aggiorna lo stato prima di attivare la protezione.</translation></message>
    <message><source>Paused</source><translation>In pausa</translation></message>
    <message><source>Partially paused</source><translation>Parzialmente in pausa</translation></message>
    <message><source>Persistent overlay data will be retained at the next startup. Automatic commits are paused.</source><translation>I dati dell’overlay persistente verranno conservati al prossimo avvio. I commit automatici sono in pausa.</translation></message>

    <message><source>Paused entries: %1 · Paused files: %2</source><translation>Elementi in pausa: %1 · File in pausa: %2</translation></message>
    <message><source>Read-only ROM</source><translation>ROM di sola lettura</translation></message>
    <message><source>%1 changes · %2 commands</source><translation>%1 modifiche · %2 comandi</translation></message>
    <message><source>%1 file system: file exclusions and per-file commit are not supported.</source><translation>File system %1: esclusioni di file e commit per file non supportati.</translation></message>
    <message><source>Applying changes</source><translation>Applicazione delle modifiche</translation></message>
    <message><source>Automatic commits paused</source><translation>Commit automatici sospesi</translation></message>
    <message><source>Automatic file commits are paused until restart after a HORM mode change.</source><translation>I commit automatici dei file sono sospesi fino al riavvio dopo una modifica della modalità HORM.</translation></message>
    <message><source>Automatic file commits are unavailable after FullCommit.</source><translation>I commit automatici dei file non sono disponibili dopo FullCommit.</translation></message>
    <message><source>Automatic file commits are unavailable while HORM is active.</source><translation>I commit automatici dei file non sono disponibili mentre HORM è attivo.</translation></message>
    <message><source>Automatic staged-file commits are paused for this restart.</source><translation>I commit automatici dei file predisposti sono sospesi per questo riavvio.</translation></message>
    <message><source>Automatic staged-file commits are paused for this shutdown.</source><translation>I commit automatici dei file predisposti sono sospesi per questo arresto.</translation></message>
    <message><source>Configuration persistence could not be confirmed. Refresh the system state before changing this setting.</source><translation>Impossibile confermare la persistenza della configurazione. Aggiornare lo stato del sistema prima di modificare questa impostazione.</translation></message>
    <message><source>Elapsed: %1 s</source><translation>Tempo trascorso: %1 s</translation></message>
    <message><source>Only eligible staged files will be committed.</source><translation>Verrà eseguito il commit solo dei file predisposti idonei.</translation></message>
    <message><source>Read-only ROM cannot persist ordinary UWF settings on the protected system volume. Disable UWF and restart before changing them.</source><translation>La modalità ROM di sola lettura non può rendere persistenti le normali impostazioni UWF sul volume di sistema protetto. Disabilitare UWF e riavviare prima di modificarle.</translation></message>
    <message><source>Skipped staged files are not committed by this operation.</source><translation>Questa operazione non esegue il commit dei file predisposti ignorati.</translation></message>
    <message><source>Staged files are already on unprotected storage. No automatic commit is needed.</source><translation>I file predisposti sono già su un archivio non protetto. Non è necessario un commit automatico.</translation></message>
    <message><source>Threshold changes require a registry commit, which is unavailable in read-only ROM.</source><translation>Le modifiche delle soglie richiedono un commit del registro, non disponibile in modalità ROM di sola lettura.</translation></message>
    <message><source>Waiting for Windows to finish. This may take a while.</source><translation>In attesa del completamento di Windows. Potrebbe richiedere tempo.</translation></message>
    <message><source>HORM is disabled. Automatic staged-file commits are paused until restart. Manual operations follow the current UWF capabilities.</source><translation>HORM è disabilitato. I commit automatici dei file predisposti sono sospesi fino al riavvio. Le operazioni manuali dipendono dalle funzionalità UWF attuali.</translation></message>
    <message><source>This configuration snapshot includes settings that require UWF disabled in the current session. Disable UWF and restart before running it.</source><translation>Questa istantanea della configurazione include impostazioni che richiedono UWF disabilitato nella sessione corrente. Disabilitare UWF e riavviare prima di eseguirla.</translation></message>
    <message><source>Current-session actions (HORM, FullCommit, and persistent overlay reset; export them from Pending changes)</source><translation>Azioni della sessione corrente (HORM, FullCommit e reimpostazione dell’overlay persistente; esportarle da Modifiche in sospeso)</translation></message>
    <message><source>Bulk unprotection requires a confirmed complete next-session volume scope.</source><translation>La rimozione collettiva della protezione richiede la conferma dell’insieme completo dei volumi della prossima sessione.</translation></message>
</context>
</TS>
