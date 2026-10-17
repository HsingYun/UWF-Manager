<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>Activer</translation></message>
    <message><source>Disable</source><translation>Désactiver</translation></message>
    <message><source>enable</source><translation>activer</translation></message>
    <message><source>disable</source><translation>désactiver</translation></message>
    <message><source>Enabled</source><translation>Activé</translation></message>
    <message><source>Disabled</source><translation>Désactivé</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Annuler</translation></message>
    <message><source>Close</source><translation>Fermer</translation></message>
    <message><source>Refresh</source><translation>Actualiser</translation></message>
    <message><source>Apply</source><translation>Appliquer</translation></message>
    <message><source>Add</source><translation>Ajouter</translation></message>
    <message><source>Remove</source><translation>Retirer</translation></message>
    <message><source>Yes</source><translation>Oui</translation></message>
    <message><source>No</source><translation>Non</translation></message>
    <message><source>Excluded</source><translation>Exclu</translation></message>
    <message><source>Not excluded</source><translation>Non exclu</translation></message>
    <message><source>File…</source><translation>Fichier…</translation></message>
    <message><source>Folder…</source><translation>Dossier…</translation></message>
    <message><source>volume ID</source><translation>ID de volume</translation></message>
    <message><source>drive letter</source><translation>lettre de lecteur</translation></message>
    <message><source>Drive letter</source><translation>Lettre de lecteur</translation></message>
    <message><source>Volume ID</source><translation>ID de volume</translation></message>
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>Le système « %1 » (%2) n’est pas reconnu comme une édition Windows 10/11 prise en charge. UWF Manager fonctionne en mode de compatibilité ; certaines fonctions peuvent être indisponibles.</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>État de protection de ce volume pour la session actuelle (lecture seule).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Protéger ce volume à la prochaine session. Les écritures sont redirigées vers l’overlay, puis supprimées au redémarrage.</translation></message>
    <message><source>Protection:</source><translation>Protection :</translation></message>
    <message><source>Current session</source><translation>Session actuelle</translation></message>
    <message><source>Next session</source><translation>Prochaine session</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>Session en cours (lecture seule). Les modifications ne prennent pas effet pendant cette session.</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>Session après redémarrage. Les modifications prennent effet au redémarrage du système.</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identification du volume : la lettre de lecteur est simple, mais l’association est perdue si elle change, notamment après un changement de disques. L’ID de volume reste stable.</translation></message>
    <message><source>Bind by:</source><translation>Identification par :</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Volume non pris en charge par UWF : </translation></message>
    <message><source>Global settings</source><translation>Paramètres généraux</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>État du filtre UWF dans la session actuelle (lecture seule).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Activer UWF à la prochaine session. Les écritures sur les volumes protégés vont dans l’overlay et sont supprimées au redémarrage.</translation></message>
    <message><source>Filter</source><translation>Filtre</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Pour modifier le type et la taille maximale :
1. Désactivez le filtre ci-dessus.
2. Redémarrez pour le désactiver effectivement.
3. Modifiez le réglage.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Stockage de l’overlay : RAM est plus rapide mais consomme de la mémoire ; Disk utilise le disque système et offre plus de capacité. Les deux sont effacés au redémarrage.</translation></message>
    <message><source>Type</source><translation>Type</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Capacité maximale de l’overlay. RAM est limitée par la mémoire totale. Disk exige au moins 1024 MB et assez d’espace libre sur le volume système.</translation></message>
    <message><source>Maximum size</source><translation>Taille maximale</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Taille maximale · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Seuil d’avertissement</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>À ce niveau d’utilisation, écrit un avertissement dans le journal d’événements. Doit être inférieur au seuil critique. 0 désactive l’événement.</translation></message>
    <message><source>Critical threshold</source><translation>Seuil critique</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>À ce niveau d’utilisation, écrit un événement critique dans le journal. Doit être supérieur au seuil d’avertissement. 0 désactive l’événement.</translation></message>
    <message><source>Used / total</source><translation>Utilisé / total</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; Utilisé &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Avertissement &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critique</translation></message>
    <message><source>UWF status unavailable: </source><translation>État UWF indisponible : </translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>Les droits administrateur sont nécessaires pour modifier UWF. Relancez le programme par clic droit → « Exécuter en tant qu’administrateur ».</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Type de lecteur non pris en charge (disques locaux fixes uniquement).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Ce volume dépasse la limite UWF de 16 TiB par volume protégé.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1 : protection du volume possible, mais sans exclusions ni enregistrement par fichier.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Échec: Lecture des informations du volume.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 libres / %2</translation></message>
    <message><source>Commit</source><translation>Enregistrer sur disque</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Enregistrer sur disque les modifications de ce fichier dans l’overlay / Enregistrer les modifications du Registre. Cette opération est irréversible.</translation></message>
    <message><source>Commit file changes…</source><translation>Enregistrer les modifications du fichier…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Choisir un fichier à enregistrer sur disque. Enregistrer sur disque les modifications de ce fichier dans l’overlay.</translation></message>
    <message><source>Commit folder changes…</source><translation>Enregistrer les modifications du dossier…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Choisir un dossier à enregistrer sur disque. Enregistrer sur disque les modifications de ce dossier dans l’overlay. Tous les sous-dossiers sont inclus.</translation></message>
    <message><source>Delete and commit file…</source><translation>Supprimer ce fichier et enregistrer sa suppression sur disque…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Choisir un fichier à supprimer définitivement. Supprimer ce fichier et enregistrer sa suppression sur disque.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Supprimer ce dossier et son contenu, puis enregistrer les suppressions sur disque…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Choisir un dossier à supprimer définitivement. Supprimer ce dossier et son contenu, puis enregistrer les suppressions sur disque. Tous les sous-dossiers sont inclus.</translation></message>
    <message><source>Commit registry changes…</source><translation>Enregistrer les modifications du Registre…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Saisissez la clé et, si nécessaire, le nom d’une valeur. Sans nom de valeur, toutes les valeurs de la clé et de ses sous-clés seront enregistrées.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Enregistrer la suppression dans le Registre…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Saisissez la clé à supprimer et, éventuellement, le nom d’une valeur. Sans nom, la clé, ses valeurs et toutes ses sous-clés sont supprimées, puis les suppressions enregistrées.</translation></message>
    <message><source>File exclusions</source><translation>Exclusions de fichiers</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Fichiers et dossiers du volume exclus de la protection UWF. Double-cliquez sur une entrée pour copier son chemin.</translation></message>
    <message><source>Registry exclusions</source><translation>Exclusions du Registre</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Les exclusions du Registre dépendent du filtre UWF global, pas de la protection de ce volume. La liste commune à tous les volumes n’est affichée qu’une fois. Double-cliquez pour copier un chemin.</translation></message>
    <message><source>File staging</source><translation>Fichiers à enregistrer</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>fichiers / dossiers: Enregistrement automatique avant un arrêt ou redémarrage sécurisé. Tous les sous-dossiers sont inclus. Tant que la liste contient des entrées, cette exclusion du Registre permet de la conserver entre les sessions protégées : HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Search staged paths…</source><translation>Rechercher un chemin à enregistrer…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Ajouter des fichiers ou un dossier à la liste d’enregistrement automatique.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Choisir un fichier à enregistrer sur disque. Enregistrement automatique avant un arrêt ou redémarrage sécurisé.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Choisir un dossier à enregistrer sur disque. Enregistrement automatique avant un arrêt ou redémarrage sécurisé. Tous les sous-dossiers sont inclus.</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>Retirer immédiatement les chemins sélectionnés de l’enregistrement automatique.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Choisir un fichier à enregistrer sur disque · Fichiers à enregistrer</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Choisir un dossier à enregistrer sur disque · Fichiers à enregistrer · Tous les sous-dossiers sont inclus.</translation></message>
    <message><source>Cannot stage this path</source><translation>Impossible d’ajouter ce chemin à l’enregistrement automatique</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Ce chemin recoupe une exclusion UWF. Retirez d’abord cette exclusion :
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Ce chemin recoupe un fichier à enregistrer automatiquement. Retirez d’abord cette entrée :
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Le chemin recoupe une entrée d’enregistrement automatique</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Cette exclusion du Registre est nécessaire tant que la liste d’enregistrement contient des entrées</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Le chemin n’existe pas ou n’a pas le type attendu :
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Impossible d’examiner le chemin :
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Les points d’analyse ne peuvent pas être ajoutés à l’enregistrement automatique :
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Sans lettre de lecteur, UWF ne peut pas enregistrer ce chemin :
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Ce chemin est sur le volume %1. Ajoutez-le depuis l’onglet d’enregistrement de ce volume.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>La racine d’un volume ne peut pas être ajoutée, car cela enregistrerait récursivement tout le volume.</translation></message>
    <message><source>File staging could not be saved</source><translation>Échec: Sauvegarde de la liste des fichiers à enregistrer</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>La liste a été rechargée après l’échec de la mise à jour :
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Fichiers à enregistrer: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Enregistrement automatique avant un arrêt ou redémarrage sécurisé.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Fichiers à enregistrer: %1 · fichiers: %2 · dossiers: %3</translation></message>
    <message><source>Required by UWF for File staging.</source><translation>Nécessaire à l’enregistrement automatique UWF.</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Le filtre UWF est désactivé. Aucune modification d’overlay n’a été accumulée ni n’est à enregistrer.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>Le système de fichiers %1 ne permet pas l’enregistrement par fichier.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Ce volume n’est pas pris en charge par UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Ce volume ne permet pas l’enregistrement de fichiers. Seul l’enregistrement du Registre est disponible (exclusions globales).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>Le système de fichiers %1 ne permet pas l’enregistrement par fichier. Seul l’enregistrement du Registre est disponible (exclusions globales).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Ce volume n’est pas protégé par UWF ; aucune modification de fichier n’est à enregistrer.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Ce volume n’est pas protégé par UWF ; aucune modification de fichier n’est à enregistrer. Seul l’enregistrement du Registre est disponible (exclusions globales).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Choisir un fichier à enregistrer sur disque</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Choisir un dossier à enregistrer sur disque</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Choisir un fichier à supprimer définitivement</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Choisir un dossier à supprimer définitivement</translation></message>
    <message><source>Commit registry changes</source><translation>Enregistrer les modifications du Registre</translation></message>
    <message><source>Commit registry deletion</source><translation>Enregistrer la suppression dans le Registre</translation></message>
    <message><source>Key path:</source><translation>Clé · Chemin:</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>Saisissez ou collez HKLM\Software\... puis appuyez sur Entrée</translation></message>
    <message><source>Name</source><translation>Nom</translation></message>
    <message><source>Data</source><translation>Données</translation></message>
    <message><source>Key: %1</source><translation>Clé: %1</translation></message>
    <message><source>Value: %1</source><translation>Valeur: %1</translation></message>
    <message><source>(none)</source><translation>(aucun)</translation></message>
    <message><source>(Default)</source><translation>(par défaut)</translation></message>
    <message><source>(value not set)</source><translation>(valeur non définie)</translation></message>
    <message><source>(empty)</source><translation>(vide)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Inconnu (Valeur: Fichier trop volumineux)</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>(clé entière et toutes ses sous-clés)</translation></message>
    <message><source>Overlay files - %1</source><translation>Overlay · fichiers - %1</translation></message>
    <message><source>View overlay files</source><translation>Voir les fichiers de l’overlay</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Voir les fichiers de l’overlay. &lt;b&gt;Instantané de diagnostic uniquement.&lt;/b&gt; N’utilisez pas cette liste pour décider quoi enregistrer.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>La liste des fichiers de l’overlay nécessite un volume NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>Le filtre UWF est désactivé ; aucun volume n’a d’overlay à examiner.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Ce volume n’est pas protégé par UWF et n’a pas d’overlay à examiner.</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;Instantané de diagnostic uniquement.&lt;/b&gt; N’utilisez pas cette liste pour décider quoi enregistrer.</translation></message>
    <message><source>NTFS volumes only.</source><translation>La liste des fichiers de l’overlay nécessite un volume NTFS.</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>Un overlay volumineux augmente la mémoire et le temps nécessaires ; l’opération peut ralentir ou échouer.</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>Liste non exacte : les fichiers plus petits qu’un cluster (souvent 4 KB) peuvent manquer ; des fichiers déjà enregistrés, exclus ou touchés par d’autres opérations peuvent apparaître.</translation></message>
    <message><source>Loading…</source><translation>Chargement…</translation></message>
    <message><source>Failed: %1</source><translation>Échec: %1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>Overlay · fichiers: %1</translation></message>
    <message><source>No files</source><translation>Aucun fichier</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>Page %1 / %2 · fichiers: %3</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Le fournisseur WMI a planté pendant l’énumération. C’est une instabilité connue de UWF_Overlay.GetOverlayFiles avec un overlay volumineux ou une forte charge d’E/S.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Mémoire insuffisante ou opération non prise en charge. NTFS et de la mémoire libre sont nécessaires. Réessayez avec un overlay plus petit ou après avoir fermé les applications gourmandes.</translation></message>
    <message><source>Raw error: %1</source><translation>Code d’erreur: %1</translation></message>
    <message><source>Export to file…</source><translation>Exporter dans un fichier…</translation></message>
    <message><source>Export overlay file list</source><translation>Exporter dans un fichier… · Overlay · fichiers</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Fichiers texte (*.txt);;Tous les fichiers (*)</translation></message>
    <message><source>Export failed</source><translation>Échec de l’exportation</translation></message>
    <message><source>Export finished</source><translation>Exportation terminée</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Échec: Ouverture du fichier (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Échec: Écriture du fichier (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Configuration: Écriture du fichier · entrées: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Impossible d’exclure un volume entier (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Les fichiers d’échange, swap et d’hibernation ne peuvent pas être exclus : UWF en dépend.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Impossible d’exclure tout le répertoire : \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Impossible d’exclure tout le répertoire : \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Impossible d’exclure tout le répertoire : \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Ce fichier système essentiel ne peut pas être exclu : %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Le fichier de Registre utilisateur NTUSER.DAT ne peut pas être exclu.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>La clé du Registre ne peut pas être vide.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Une clé du Registre ne peut pas commencer par une barre oblique inverse.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Le chemin contient des barres obliques inverses consécutives ; il est invalide.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Les chemins du Registre utilisent `\` comme séparateur, pas `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Le chemin contient des caractères de contrôle invisibles ; il est invalide.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>La documentation UWF interdit d’exclure le secret du compte ordinateur du domaine : HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Seules des sous-clés précises des clés racines suivantes peuvent être exclues :
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Search file paths…</source><translation>Rechercher un chemin de fichier…</translation></message>
    <message><source>Search registry keys…</source><translation>Rechercher une clé du Registre…</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Ajouter des fichiers ou dossiers aux exclusions. Ils ne seront pas protégés par UWF.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Choisir un fichier à ajouter · Exclusions de fichiers.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Choisir un dossier à ajouter · Exclusions de fichiers. Tous les sous-dossiers sont inclus.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Saisissez le chemin de la clé du Registre à exclure.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Saisissez le chemin de la clé du Registre à exclure. / Persistance.</translation></message>
    <message><source>Registry key…</source><translation>Registre · Clé…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Clé secrète du domaine (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Licence d’accès client aux services Terminal Server (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Conserver le secret du domaine (mot de passe du compte ordinateur) entre les sessions UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Conserver les licences d’accès client Terminal Server entre les sessions UWF.</translation></message>
    <message><source>· %1 persistence %2</source><translation>· Persistance %1: %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistance %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Retirer la sélection</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Retirer les exclusions sélectionnées. Prend effet après application.</translation></message>
    <message><source>Open containing folder</source><translation>Ouvrir le dossier parent</translation></message>
    <message><source>Copy file path</source><translation>Copier le chemin du fichier</translation></message>
    <message><source>Copy registry path</source><translation>Copier le chemin du Registre</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Enregistrer sur disque les modifications de ce dossier dans l’overlay…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Enregistrer sur disque les modifications de ce fichier dans l’overlay…</translation></message>
    <message><source>Copied to clipboard: </source><translation>Copié dans le presse-papiers : </translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Choisir un fichier à ajouter · Exclusions de fichiers. Sélection multiple possible.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Choisir un dossier à ajouter · Exclusions de fichiers</translation></message>
    <message><source>Add registry exclusion</source><translation>Ajout d’une exclusion du Registre</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Le chemin %1 n’est pas sur le volume %2 et ne peut pas y être ajouté comme exclusion.</translation></message>
    <message><source>Path is not on this volume</source><translation>Chemin hors de ce volume</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Impossible d’ajouter cette exclusion</translation></message>
    <message><source>File: %1</source><translation>Fichier… %1</translation></message>
    <message><source>Folder: %1</source><translation>Dossier… %1</translation></message>
    <message><source>Registry: %1</source><translation>Registre: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Exclu par défaut ; modifiable à tout moment.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Session actuelle: %1
Prochaine session: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Modifications en attente: %1</translation></message>
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>entrées: %1 · À ajouter: %2 · À retirer à la prochaine session: %3 · Modifications en attente: %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Gestionnaire du filtre d’écriture unifié (UWF)</translation></message>
    <message><source>Main toolbar</source><translation>Barre d’outils principale</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Relire l’état actuel d’UWF et la configuration de la prochaine session.</translation></message>
    <message><source>Review and apply</source><translation>Vérifier et appliquer</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Vérifier toutes les modifications en attente et les appliquer ensemble. La plupart des modifications prennent effet au prochain redémarrage.</translation></message>
    <message><source>Safe shutdown</source><translation>Arrêt sécurisé</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>Le système s’arrêtera de façon sécurisée via UWF. Disponible même si l’overlay UWF est plein.</translation></message>
    <message><source>Safe restart</source><translation>Redémarrage sécurisé</translation></message>
    <message><source>Direct restart</source><translation>Redémarrage direct</translation></message>
    <message><source>Direct shutdown</source><translation>Arrêt direct</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>Le système redémarrera de façon sécurisée via UWF. Disponible même si l’overlay UWF est plein.</translation></message>
    <message><source>Log</source><translation>Journal</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Afficher le journal interne de cette session pour le dépannage.</translation></message>
    <message><source>About</source><translation>À propos</translation></message>
    <message><source>About this program.</source><translation>À propos UWF Manager.</translation></message>
    <message><source>Switch display language</source><translation>Changer la langue d’affichage</translation></message>
    <message><source>Language</source><translation>Langue</translation></message>
    <message><source>Theme</source><translation>Thème</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Basculer entre thème clair et sombre</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Interface graphique de gestion du filtre UWF, de l’overlay et des exclusions de fichiers et du Registre. La plupart des modifications prennent effet au prochain redémarrage.&lt;/p&gt;&lt;p&gt;Code source: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;Licence: &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; Le texte intégral de la licence figure dans le fichier LICENSE fourni avec le programme.&lt;/p&gt;&lt;p&gt;Logiciel libre : redistribution et modification autorisées selon la GPL v3. Fourni « en l’état », sans aucune garantie.&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>À propos UWF Manager</translation></message>
    <message><source>System information</source><translation>Informations système</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Instantané de diagnostic</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>Diagnostic du système, de la sécurité, d’UWF, du mode avancé et des journaux récents.</translation></message>
    <message><source>Read only</source><translation>Lecture seule</translation></message>
    <message><source>Diagnostic report</source><translation>Rapport de diagnostic</translation></message>
    <message><source>Copied</source><translation>Copié</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>Diagnostic du système, de la sécurité, d’UWF, du mode avancé et des journaux récents.</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>Le rapport peut contenir des noms de comptes, identifiants d’appareils, chemins et journaux récents. Vérifiez-le avant de le partager.</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;Ce programme dépend du filtre d’écriture unifié de Windows (UWF).&lt;/b&gt; La protection est assurée par UWF, intégré à Windows. UWF Manager ne filtre pas lui-même les écritures : il configure et gère UWF, qui doit être installé et activé sur le système.&lt;/p&gt;&lt;p&gt;À la première activation d’UWF, les paramètres système suivants sont modifiés pour améliorer les performances :&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Les fichiers d’échange sont désactivés.&lt;/li&gt;&lt;li&gt;La restauration du système est désactivée.&lt;/li&gt;&lt;li&gt;SuperFetch est désactivé.&lt;/li&gt;&lt;li&gt;Le service d’indexation des fichiers est arrêté.&lt;/li&gt;&lt;li&gt;Le service de défragmentation est arrêté.&lt;/li&gt;&lt;li&gt;Le démarrage rapide est désactivé.&lt;/li&gt;&lt;li&gt;Le paramètre BCD bootstatuspolicy est défini sur ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;Ces réglages restent modifiables. Par exemple, déplacez le fichier d’échange vers un volume non protégé et réactivez la pagination.&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Cible introuvable ; rien à enregistrer dans l’overlay.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>La cible existe uniquement dans l’overlay, créée sous UWF et jamais enregistrée sur le support ou dans le Registre persistant. Elle disparaîtra au redémarrage ; enregistrer sa suppression n’est ni nécessaire ni possible.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Le système a refusé un paramètre (chemin ou argument invalide).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Échec de l’opération. Un autre processus utilise peut-être la cible : dossier ouvert dans l’Explorateur ou fichier ouvert. Fermez les programmes concernés et réessayez.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Échec de l’opération ; consultez le journal pour plus de détails.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Opération refusée (code %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Inconnu.</translation></message>
    <message><source>Commit canceled</source><translation>Enregistrement annulé</translation></message>
    <message><source>Commit result</source><translation>Résultat de l’enregistrement</translation></message>
    <message><source>Succeeded</source><translation>Réussi</translation></message>
    <message><source>Skipped</source><translation>Ignoré</translation></message>
    <message><source>Failed</source><translation>Échec</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Réussi: %1; Ignoré: %2; Échec: %3.</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
Annulé par l’utilisateur ; %1 entrées non traitées.</translation></message>
    <message><source>Category</source><translation>Catégorie</translation></message>
    <message><source>Path</source><translation>Chemin</translation></message>
    <message><source>Existed before</source><translation>Présent avant</translation></message>
    <message><source>Exists after</source><translation>Présent après</translation></message>
    <message><source>Error code</source><translation>Code d’erreur</translation></message>
    <message><source>Reason</source><translation>Motif</translation></message>
    <message><source>Previous page</source><translation>Page précédente</translation></message>
    <message><source>First page</source><translation>Première page</translation></message>
    <message><source>Last page</source><translation>Dernière page</translation></message>
    <message><source>Next page</source><translation>Page suivante</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>Page %1 / %2 · entrées: %3</translation></message>
    <message><source>Copy selected rows</source><translation>Copier les lignes sélectionnées</translation></message>
    <message><source>Copy all</source><translation>Tout copier</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Cette exclusion du Registre est nécessaire tant que la liste d’enregistrement contient des entrées. Fichiers à enregistrer: Effacer → Retrait d’une exclusion du Registre.</translation></message>
    <message><source>0 lines</source><translation>0 lignes</translation></message>
    <message><source>%1 lines</source><translation>%1 lignes</translation></message>
    <message><source>Loading log entries…</source><translation>Chargement du journal…</translation></message>
    <message><source>No log entries</source><translation>Aucune entrée de journal</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>Page %1 / %2 · lignes: %3</translation></message>
    <message><source>Copy current page</source><translation>Copier la page actuelle</translation></message>
    <message><source>Clear</source><translation>Effacer</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>Modifications en attente: %1</translation></message>
    <message><source>No pending changes</source><translation>Aucune modification en attente</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Exclusions du Registre · Paramètres généraux.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: Protection UWF / Exclusions de fichiers / Fichiers à enregistrer.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Non pris en charge</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>Actualiser · Volume: %1</translation></message>
    <message><source>· Filter (global) %1</source><translation>· Filtre (UWF): %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· Overlay · Type → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· Overlay · Taille maximale → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· Overlay · Seuil d’avertissement → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· Overlay · Seuil critique → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ Pour modifier le type et la taille maximale :
1. Désactivez le filtre ci-dessus.
2. Redémarrez pour le désactiver effectivement.
3. Modifiez le réglage.</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· Volume %1 · Protection UWF: %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· Volume %1 · Identification par : %2 (sans équivalent en ligne de commande ; propre à ce programme)</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ Exclusions de fichiers  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− Exclusions de fichiers  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ Exclusions du Registre  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− Exclusions du Registre  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>Filtre (UWF): %1</translation></message>
    <message><source>Overlay type → %1</source><translation>Overlay · Type → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>Overlay · Taille maximale → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>Overlay · Seuil d’avertissement → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>Overlay · Seuil critique → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>Volume %1 · Protection UWF: %2</translation></message>
    <message><source>File exclusion %1</source><translation>Exclusions de fichiers: %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>Exclusions du Registre: %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>Modifications en attente (%1)</translation></message>
    <message><source>Current session configuration</source><translation>Configuration de la session actuelle</translation></message>
    <message><source>Review and apply changes</source><translation>Vérifier et appliquer les modifications</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>Configuration complète sous forme de commandes uwfmgr. Les modifications en attente figurent d’abord dans une section distincte. &lt;span style='color:%1'&gt;Modifications en attente&lt;/span&gt;. &lt;span style='color:%2'&gt;Appliquer&lt;/span&gt;: Cliquer pour écrire les modifications dans le système. La plupart des modifications prennent effet au prochain redémarrage.</translation></message>
    <message><source>Confirm apply</source><translation>Confirmer l’application</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>&lt;span style='color:%1'&gt;Ces modifications seront écrites dans le système&lt;/span&gt;. La plupart des modifications prennent effet au prochain redémarrage.&lt;br&gt;&lt;br&gt;Continuer?</translation></message>
    <message><source>Applied changes</source><translation>Modifications appliquées</translation></message>
    <message><source>Result</source><translation>Résultat</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filtre: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Définition du seuil d’avertissement: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Définition du seuil critique: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Définition du type d’overlay / Définition de la taille maximale: Pour modifier le type et la taille maximale :
1. Désactivez le filtre ci-dessus.
2. Redémarrez pour le désactiver effectivement.
3. Modifiez le réglage.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Définition du type d’overlay: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Définition de la taille maximale: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Définition de la taille maximale: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · Protection UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Identification par : %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1 · Ajout d’une exclusion de fichier: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1 · Retrait d’une exclusion de fichier: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Ajout d’une exclusion du Registre: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Retrait d’une exclusion du Registre: %1</translation></message>
    <message><source>Time</source><translation>Heure</translation></message>
    <message><source>Level</source><translation>Niveau</translation></message>
    <message><source>Tag</source><translation>Étiquette</translation></message>
    <message><source>Message</source><translation>Message</translation></message>
    <message><source>Confirm safe shutdown?</source><translation>Arrêt sécurisé?</translation></message>
    <message><source>Confirm safe restart?</source><translation>Redémarrage sécurisé?</translation></message>
    <message><source>Shut down</source><translation>Arrêter</translation></message>
    <message><source>Restart</source><translation>Redémarrer</translation></message>
    <message><source>Continue shutdown</source><translation>Poursuivre l’arrêt</translation></message>
    <message><source>Continue restart</source><translation>Poursuivre le redémarrage</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>Le système s’arrêtera de façon sécurisée via UWF.</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>Le système redémarrera de façon sécurisée via UWF.</translation></message>
    <message><source>UWF protection</source><translation>Protection UWF</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>Disponible même si l’overlay UWF est plein.</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Calcul des fichiers à enregistrer automatiquement…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>%1 fichiers seront enregistrés avant cette opération. Les fichiers hors des volumes actuellement protégés seront ignorés.</translation></message>
    <message><source>Committing staged files</source><translation>Enregistrement des fichiers préparés</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>L’opération d’alimentation se poursuivra après l’enregistrement automatique.</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 fichiers traités sur %2.</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>Enregistrement automatique incomplet</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>Consultez les détails avant de décider de poursuivre.</translation></message>
    <message><source>Operation details</source><translation>Détails de l’opération</translation></message>
    <message><source>No additional error details were provided.</source><translation>Aucun détail d’erreur supplémentaire.</translation></message>
    <message><source>Some changes may not be preserved</source><translation>Certaines modifications risquent d’être perdues</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>En poursuivant, le système s’arrêtera sans conserver les fichiers dont l’enregistrement a échoué.</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>En poursuivant, le système redémarrera sans conserver les fichiers dont l’enregistrement a échoué.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>L’enregistrement automatique s’est arrêté de façon inattendue :
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Enregistrement automatique incomplet: Erreur inconnue.</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>Les modifications non enregistrées seront perdues</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>Avant de poursuivre, sauvegardez votre travail et enregistrez durablement les modifications à conserver.</translation></message>
    <message><source>Confirm direct restart?</source><translation>Redémarrage direct?</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>Arrêt direct?</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>Redémarrer. Les fichiers de la liste ne seront pas enregistrés avant le redémarrage.</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>Arrêter. Les fichiers de la liste ne seront pas enregistrés avant l’arrêt.</translation></message>
    <message><source>File staging will be skipped</source><translation>L’enregistrement automatique sera ignoré</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>Les fichiers de la liste ne seront pas enregistrés avant le redémarrage.</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>Les fichiers de la liste ne seront pas enregistrés avant l’arrêt.</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Redémarrage direct: L’enregistrement automatique sera ignoré.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Arrêt direct: L’enregistrement automatique sera ignoré.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Enregistrement automatique ignoré car une opération d’alimentation directe a été choisie.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Arrêt sécurisé: Échec</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Arrêter: Échec (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Redémarrage sécurisé: Échec</translation></message>
    <message><source>Direct restart failed</source><translation>Redémarrage direct: Échec</translation></message>
    <message><source>Direct shutdown failed</source><translation>Arrêt direct: Échec</translation></message>
    <message><source>Restart failed: %1</source><translation>Redémarrer: Échec (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Impossible de lire la liste d’enregistrement automatique:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Impossible de lire la liste d’enregistrement automatique: Erreur inconnue.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>fichiers: %1 · Réussi: %2 · Ignoré (fichiers): %3 · Ignoré (entrées): %4 · Échec: %5</translation></message>
    <message><source>File staging preparation</source><translation>Fichiers à enregistrer: Détails de l’opération</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Les points d’analyse ne sont pas pris en charge.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Seuls les chemins absolus sur des volumes locaux peuvent être ajoutés.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Seuls les chemins absolus sur des volumes locaux peuvent être ajoutés.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Impossible d’examiner le chemin à enregistrer.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Le chemin du fichier préparé désigne maintenant un autre type d’élément.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Le chemin du dossier préparé désigne maintenant un autre type d’élément.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Impossible d’énumérer entièrement le dossier préparé.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Le chemin n’a pas de lettre de lecteur.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>La racine d’un volume ne peut pas être traitée récursivement.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Le fournisseur UWF a refusé l’enregistrement.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Échec de l’enregistrement: Erreur inconnue.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>%1 échecs supplémentaires ont été consignés.</translation></message>
    <message><source>Commit failed</source><translation>Échec de l’enregistrement</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Le chemin n’a pas de lettre de lecteur. Identification du volume cible: Échec.</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>Échec: Lecture des informations du volume (%1)</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>Aucun enregistrement de session actuelle pour le volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Ce chemin figure dans les exclusions de fichiers.
Exclusion : %1</translation></message>
    <message><source>No files were found under %1.</source><translation>Aucun fichier trouvé dans %1.</translation></message>
    <message><source>Commit to disk</source><translation>Enregistrer sur disque</translation></message>
    <message><source>Committing…</source><translation>Enregistrement…</translation></message>
    <message><source>This action cannot be undone.</source><translation>Cette opération est irréversible.</translation></message>
    <message><source>Continue</source><translation>Continuer</translation></message>
    <message><source>Delete and commit</source><translation>Supprimer et enregistrer</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>Enregistrer sur disque les modifications de ce fichier dans l’overlay</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>Enregistrer sur disque les modifications de ce dossier dans l’overlay</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>fichiers: %1. Tous les sous-dossiers sont inclus.</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>Supprimer ce fichier et enregistrer sa suppression sur disque</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>Supprimer ce dossier et son contenu, puis enregistrer les suppressions sur disque</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>fichiers: %1; dossiers: %2. Supprimer et enregistrer.</translation></message>
    <message><source>Commit this registry value to disk</source><translation>Enregistrer cette valeur du Registre sur disque</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>Enregistrer cette clé du Registre et toute son arborescence sur disque</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>Valeur: %1. (clé entière et toutes ses sous-clés)</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>Supprimer cette valeur du Registre et enregistrer la suppression sur disque</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>Supprimer cette clé du Registre et son arborescence, puis enregistrer les suppressions</translation></message>
    <message><source>Scanning registry keys…</source><translation>Énumération des clés du Registre…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>Énumération des clés du Registre…
Clé: %1</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>Registre · Clé · Supprimer et enregistrer:</translation></message>
    <message><source>Copy current entry</source><translation>Copier l’entrée actuelle</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>%1 clés et toutes leurs valeurs seront supprimées récursivement.</translation></message>
    <message><source>Commit file deletion failed</source><translation>Échec: Enregistrement de la suppression du fichier</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>Le chemin n’existe pas ; rien à supprimer.</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Cette clé figure dans les exclusions du Registre.
Exclusion : %1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>Échec: Lecture du filtre du Registre (%1)</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>Aucun enregistrement du filtre du Registre pour la session actuelle.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>Cette valeur du Registre n’existe pas ; rien à enregistrer.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>Cette clé du Registre n’existe pas ; rien à enregistrer.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>Cette valeur du Registre n’existe pas ; rien à supprimer.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>Cette clé du Registre n’existe pas ; rien à supprimer.</translation></message>
    <message><source>Export commands…</source><translation>Exporter les commandes…</translation></message>
    <message><source>Export commands to file</source><translation>Exporter les commandes dans un fichier</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Exporter dans un fichier… · Commande: %1
%2</translation></message>
    <message><source>Import</source><translation>Importer</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Saisissez ou collez une commande par ligne. Charger un texte, journal ou script (.txt, .bat, .ps1) ; les lignes contenant cette commande sont ajoutées. L’importation prépare les modifications. Seul « Vérifier et appliquer » dans la barre d’outils les écrit dans le système.</translation></message>
    <message><source>Import uwfmgr commands</source><translation>uwfmgr · Importer</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;uwfmgr&lt;/b&gt;: Saisissez ou collez une commande par ligne. &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Charger un fichier…&lt;/b&gt;: Charger un texte, journal ou script (.txt, .bat, .ps1) ; les lignes contenant cette commande sont ajoutées. &lt;code&gt;uwfmgr&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Importer&lt;/b&gt;: &lt;b&gt;L’importation prépare les modifications. Seul « Vérifier et appliquer » dans la barre d’outils les écrit dans le système.&lt;/b&gt; &lt;b&gt;Vérifier et appliquer&lt;/b&gt;.&lt;/p&gt;</translation></message>
    <message><source>Load from file…</source><translation>Charger un fichier…</translation></message>
    <message><source>Load recommended configuration</source><translation>Charger une configuration recommandée</translation></message>
    <message><source>Choose recommended configuration</source><translation>Choisir une configuration recommandée</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Choisissez les groupes à ajouter. Vous pouvez vérifier ou supprimer chaque commande avant l’importation.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>D’après la documentation officielle Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Exclusions UWF recommandées par Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Vérifiez avant l’importation : les dossiers doivent exister pour qu’UWF accepte les exclusions.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Source : documentation officielle Microsoft UWF sur les exclusions courantes et la prise en charge antimalware.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Programme d’amélioration de l’expérience utilisateur (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Conserver durablement: État de participation défini par stratégie</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Conserver durablement: État de participation local</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Conserver durablement: Indicateur de désactivation des envois</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Service de transfert intelligent en arrière-plan (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Conserver durablement: Fichiers de la file de téléchargement</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Conserver durablement: Index de l’état des transferts</translation></message>
    <message><source>Network profiles and policies</source><translation>Profils et stratégies réseau</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Stratégie de groupe du réseau sans fil</translation></message>
    <message><source>Wired network GPO policy</source><translation>Stratégie de groupe du réseau filaire</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Stratégie de groupe du réseau sans fil · fichiers</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Stratégie de groupe du réseau filaire · fichiers</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Profils d’interfaces sans fil</translation></message>
    <message><source>Wired network interface profiles</source><translation>Profils d’interfaces filaires</translation></message>
    <message><source>Wireless service configuration</source><translation>Configuration du service sans fil</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Configuration du service haut débit mobile</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Configuration du service de configuration automatique filaire</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>Les profils XML dépendent de l’appareil. Ajoutez au besoin les chemins exacts Interfaces\{GUID}\{GUID}.xml manuellement.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Heure d’été (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Conserver durablement: Définitions des fuseaux horaires</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Conserver durablement: Informations du fuseau sélectionné</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Conserver durablement: Fichiers du produit et mises à jour</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Conserver durablement: Signatures et état dans ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Conserver durablement: Windows Update · Journal</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Conserver durablement: MpCmdRun · Journal</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Conserver durablement: État du produit dans le Registre</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Conserver durablement: WdBoot · État du service</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Conserver durablement: WdFilter · État du service</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Conserver durablement: WdNisSvc · État du service</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Conserver durablement: WdNisDrv · État du service</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Conserver durablement: WinDefend · État du service</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Conserver durablement: Fichiers du programme client</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Conserver durablement: Windows Update · Journal</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Conserver durablement: MpCmdRun · Journal</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Conserver durablement: Signatures et état antimalware</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Conserver durablement: État antimalware dans le Registre</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Choisir des fichiers contenant des commandes uwfmgr</translation></message>
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>Tous les fichiers (*);;Fichiers texte (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
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
    <message><source>Status</source><translation>État</translation></message>
    <message><source>Command</source><translation>Commande</translation></message>
    <message><source>Detail</source><translation>Détail</translation></message>
    <message><source>Import failed</source><translation>Échec de l’importation</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Erreur interne : aucun gestionnaire d’application enregistré.</translation></message>
    <message><source>Nothing to import</source><translation>Aucune commande à importer</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Aucune commande uwfmgr trouvée dans la saisie.</translation></message>
    <message><source>Applied: %1</source><translation>Appliqué: %1</translation></message>
    <message><source>Duplicates: %1</source><translation>Doublon: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Non pris en charge: %1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>Cumul après %1 lots :</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Lot %1 ──</translation></message>
    <message><source>Applied</source><translation>Appliqué</translation></message>
    <message><source>Duplicate</source><translation>Doublon</translation></message>
    <message><source>Unsupported</source><translation>Non pris en charge</translation></message>
    <message><source>File too large</source><translation>Fichier trop volumineux</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Le fichier %1 dépasse 5 MB et n’a pas été analysé. Filtrez-le d’abord manuellement.</translation></message>
    <message><source>Cannot read file</source><translation>Lecture du fichier impossible</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Échec: Ouverture du fichier (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Abandonner les modifications en attente ?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>%1 modifications n’ont pas été appliquées.

Les abandonner et continuer ?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Commande uwfmgr incomplète</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Argument manquant : Taille (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>La taille doit être un entier positif ou nul en MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Argument manquant : Overlay · Type (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Valeur invalide : Overlay · Type: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Argument manquant : Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Le volume doit être une lettre de lecteur, par exemple C:</translation></message>
    <message><source>Missing path argument</source><translation>Argument manquant : Chemin</translation></message>
    <message><source>Missing registry key argument</source><translation>Argument manquant : Registre · Clé</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Guillemet double non fermé dans la commande</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Argument supplémentaire inattendu : %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Commande uwfmgr non prise en charge (aucune action d’interface correspondante)</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>Modifications en attente: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>État cible déjà atteint ; aucune action nécessaire</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Chemin hors de ce volume, ou exclusions de fichiers non prises en charge (exFAT / ReFS, par exemple)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Refusé par la liste de blocage UWF (fichiers système, Windows, fichier d’échange, etc.)</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>Cette commande a déjà été traitée dans ce lot</translation></message>
    <message><source>Pending filter %1</source><translation>Modifications en attente: Filtre %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>Filtre: État cible déjà atteint ; aucune action nécessaire</translation></message>
    <message><source>Pending overlay type → %1</source><translation>Modifications en attente: Overlay · Type → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>Overlay · Type: %1 (État cible déjà atteint ; aucune action nécessaire)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valeur invalide : Taille: %1</translation></message>
    <message><source>maximum size</source><translation>Taille maximale</translation></message>
    <message><source>warning threshold</source><translation>Seuil d’avertissement</translation></message>
    <message><source>critical threshold</source><translation>Seuil critique</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>Modifications en attente: Overlay %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>Overlay %1: %2 MB (État cible déjà atteint ; aucune action nécessaire)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Volume %1 inconnu (aucun disque compatible UWF avec cette lettre)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>Modifications en attente: Volume %1 · Protection UWF: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: État cible déjà atteint ; aucune action nécessaire</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Le chemin n’a pas de lettre de lecteur. Identification du volume cible: Échec.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Non pris en charge</translation></message>
    <message><source>file exclusion</source><translation>Exclusions de fichiers</translation></message>
    <message><source>registry exclusion</source><translation>Exclusions du Registre</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Les exclusions du Registre nécessitent l’onglet Registre, qui est absent</translation></message>
    <message><source>Unsupported command</source><translation>Commande: Non pris en charge</translation></message>
    <message><source>Exit</source><translation>Quitter</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Afficher/masquer.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Activé</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Désactivé</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Inconnu</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Utilisé %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Mode avancé</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordonner l’enregistrement automatique avec l’arrêt et le redémarrage de Windows.</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>Installe un service LocalSystem à démarrage automatique. Il lance UWF Manager au démarrage de Windows et tente d’enregistrer les fichiers préparés avant l’arrêt ou le redémarrage, pour améliorer la compatibilité des applications en conflit avec UWF.</translation></message>
    <message><source>System changes</source><translation>Modifications du système</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>Créer le service automatique UWFManagerEnhanced lié à cet exécutable</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>Configuration: Registre → Enregistrer sur disque (HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>Démarrer UWF Manager au prochain démarrage de Windows ; déplacer l’exécutable peut empêcher le démarrage automatique</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>Enregistrer les fichiers préparés admissibles lors de l’arrêt ou du redémarrage normal de Windows</translation></message>
    <message><source>Operational and security considerations</source><translation>Considérations de fonctionnement et de sécurité</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>Enregistre un service dans le Registre système ; l’application n’est plus entièrement portable</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>Un grand nombre de fichiers peut prolonger fortement l’arrêt ou le redémarrage ; laissez le temps nécessaire à leur enregistrement</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>Le mode avancé exécute UWF Manager en tant que SYSTEM. Si un non-administrateur peut modifier ou remplacer l’exécutable par un programme malveillant, il peut obtenir les droits SYSTEM. Activez ce mode uniquement depuis un emplacement protégé par les droits administrateur (recommandé : %ProgramFiles%\UWF Manager\Service\UWF.exe).</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>Échec: Lecture de l’état du mode avancé</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Échec: Lecture de l’état du mode avancé (Erreur inconnue).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Échec: Lecture de l’état du service:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Échec: Lecture de l’état du service (Erreur inconnue).</translation></message>
    <message><source>Status: Disabled</source><translation>État: Désactivé</translation></message>
    <message><source>Status: Enabled</source><translation>État: Activé</translation></message>
    <message><source>Status: Service stopped</source><translation>État : service arrêté</translation></message>
    <message><source>Status: Repair required</source><translation>État : réparation nécessaire</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Le service n’est pas configuré en processus indépendant.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Le service n’est pas configuré pour le démarrage automatique.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Le service n’est pas configuré pour s’exécuter sous LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>Le service n’est pas en cours d’exécution.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Le chemin ou les arguments du service ne correspondent pas à cet exécutable.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Le délai de préarrêt du service est mal configuré.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Le service ne déclare pas les privilèges requis pour lancer l’agent d’interface.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Le service n’a pas accepté les notifications de préarrêt de Windows.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Activer le mode avancé</translation></message>
    <message><source>Disable enhanced mode</source><translation>Désactiver le mode avancé</translation></message>
    <message><source>Start service</source><translation>Démarrer le service</translation></message>
    <message><source>Repair enhanced mode</source><translation>Réparer le mode avancé</translation></message>
    <message><source>Delete service</source><translation>Supprimer le service</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Arrêter et supprimer le service avancé, y compris ses données résiduelles dans le Registre.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Le service est absent, mais ses données subsistent dans le Registre.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>En attente de l’authentification de l’agent d’interface.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Le service fonctionne, mais aucun agent d’interface authentifié n’est connecté.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Service auxiliaire du mode avancé UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Mode avancé · Configuration: Appliqué.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Configuration: Appliqué. Message de la persistance UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Mode avancé · Configuration: Échec:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Mode avancé · Configuration: Échec (Erreur inconnue).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Le service avancé et sa configuration résiduelle ont été supprimés.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Suppression du service demandée, mais le nettoyage résiduel a signalé :
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Mode avancé: Supprimer le service: Échec:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Mode avancé: Supprimer le service: Échec (Erreur inconnue).</translation></message>
    <message><source>File staging completed</source><translation>Fichiers à enregistrer: Réussi</translation></message>
    <message><source>File staging failed</source><translation>Fichiers à enregistrer: Échec</translation></message>
    <message><source>All staged targets were processed.</source><translation>Toutes les cibles préparées ont été traitées.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Toutes les cibles ont été traitées ; au moins une opération a échoué.</translation></message>
    <message><source>Show main window</source><translation>Afficher la fenêtre principale</translation></message>
    <message><source>Restore default position</source><translation>Rétablir la position par défaut</translation></message>
    <message><source>Hide overlay hub</source><translation>Masquer le HUB d’overlay</translation></message>
    <message><source>Exit application</source><translation>Quitter l’application</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Échec: Connexion au système</translation></message>
    <message><source>Registry access failed</source><translation>Échec: Accès au Registre</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Échec: Lecture des valeurs du Registre (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Énumération des fichiers de l’overlay: Erreur inconnue.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Échec: Action sur le filtre : %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Échec: Lecture de l’état du filtre</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Échec: Définition du seuil d’avertissement</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Échec: Définition du seuil critique</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Échec: Lecture de l’état de l’overlay</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Échec: Définition du type d’overlay</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Échec: Définition de la taille maximale</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Échec: Lecture de la configuration de l’overlay</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Échec: Enregistrement du volume %1 auprès d’UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Échec: Protection du volume %2 : %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Échec: Définition de l’identification du volume %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Échec: Ajout d’une exclusion de fichier %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Exclusion de fichier déjà absente sur le volume %1 : %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Échec: Retrait d’une exclusion de fichier %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Échec: Lecture de la configuration du volume</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Échec: Ajout d’une exclusion du Registre %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Échec: Retrait d’une exclusion du Registre %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Échec: Mise à jour des options de persistance du Registre</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Échec: Lecture du filtre du Registre</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Non pris en charge</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Lecture de l’état de la cible: Erreur inconnue.</translation></message>
    <message><source>The target state could not be read before the operation: %1</source><translation>Impossible de lire l’état de la cible avant l’opération : %1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Le chemin n’existe pas ; rien à supprimer.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Erreur inconnue.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Résultat impossible à confirmer : la relecture de l’état de la cible a échoué : %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Le fournisseur a accepté la suppression, mais la vérification de référence confirme que la cible existe encore.</translation></message>
    <message><source>Unknown</source><translation>Inconnu</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Échec: Chargement du journal (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Échec: Chargement du journal (Erreur inconnue).</translation></message>
    <message><source>: unknown error</source><translation>: Erreur inconnue</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>Échec: Lecture de l’état de la cible (%1)</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>Échec: Lecture de l’état de la cible (Erreur inconnue).</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>Échec: Identification du volume cible (%1)</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>Échec: Énumération des valeurs du Registre (%1)</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>Échec: Énumération des clés du Registre (%1)</translation></message>
    <message><source>Delete and commit failed</source><translation>Supprimer et enregistrer: Échec</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Échec: Identification du volume cible (%1): %2</translation></message>
    <message><source>%1/%2 slots</source><translation>%1/%2 emplacements</translation></message>
</context>
</TS>
