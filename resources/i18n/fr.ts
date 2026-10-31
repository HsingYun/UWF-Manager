<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="fr" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Aperçu</translation></message>
    <message><source>enable</source><translation>activer</translation></message>
    <message><source>disable</source><translation>désactiver</translation></message>
    <message><source>Enabled</source><translation>Activé</translation></message>
    <message><source>Disabled</source><translation>Désactivé</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Annuler</translation></message>
    <message><source>Close</source><translation>Fermer</translation></message>
    <message><source>Refresh</source><translation>Actualiser</translation></message>
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
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>État de protection de ce volume pour la session actuelle (lecture seule).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Protéger ce volume à la prochaine session. Les écritures sont redirigées vers l’overlay, puis supprimées au redémarrage.</translation></message>
    <message><source>Current session</source><translation>Session actuelle</translation></message>
    <message><source>Next session</source><translation>Prochaine session</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identification du volume : la lettre de lecteur est simple, mais l’association est perdue si elle change, notamment après un changement de disques. L’ID de volume reste stable.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Volume non pris en charge par UWF : </translation></message>
    <message><source>Global settings</source><translation>Paramètres généraux</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>État du filtre UWF dans la session actuelle (lecture seule).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Activer UWF à la prochaine session. Les écritures sur les volumes protégés vont dans l’overlay et sont supprimées au redémarrage.</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Inscrit une notification de niveau avertissement dans le journal des événements lorsque l’utilisation de la surcouche atteint cette valeur. Lorsque les deux événements sont activés, cette valeur doit être inférieure au seuil critique. La valeur 0 désactive uniquement l’événement d’avertissement.</translation></message>
    <message><source>Critical threshold</source><translation>Seuil critique</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Inscrit une notification de niveau critique dans le journal des événements lorsque l’utilisation de la surcouche atteint cette valeur. Lorsque les deux événements sont activés, cette valeur doit être supérieure au seuil d’avertissement. La valeur 0 désactive uniquement l’événement critique.</translation></message>
    <message><source>Overlay</source><translation>Overlay</translation></message>
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
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Ajouter des fichiers ou un dossier à la liste d’enregistrement automatique.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Choisir un fichier à enregistrer sur disque. Enregistrement automatique avant un arrêt ou redémarrage sécurisé.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Choisir un dossier à enregistrer sur disque. Enregistrement automatique avant un arrêt ou redémarrage sécurisé. Tous les sous-dossiers sont inclus.</translation></message>
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
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>L’enregistrement automatique des fichiers dépend de cette exclusion de registre pour conserver sa liste après les redémarrages.</translation></message>
    <message><source>Built-in</source><translation>Intégré</translation></message>
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
    <message><source>Name</source><translation>Nom</translation></message>
    <message><source>Data</source><translation>Données</translation></message>
    <message><source>(Default)</source><translation>(par défaut)</translation></message>
    <message><source>(value not set)</source><translation>(valeur non définie)</translation></message>
    <message><source>(empty)</source><translation>(vide)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Inconnu (Valeur: Fichier trop volumineux)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Voir les fichiers de l’overlay. &lt;b&gt;Instantané de diagnostic uniquement.&lt;/b&gt; N’utilisez pas cette liste pour décider quoi enregistrer.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>La liste des fichiers de l’overlay nécessite un volume NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>Le filtre UWF est désactivé ; aucun volume n’a d’overlay à examiner.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Ce volume n’est pas protégé par UWF et n’a pas d’overlay à examiner.</translation></message>
    <message><source>Failed: %1</source><translation>Échec: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Le fournisseur WMI a planté pendant l’énumération. C’est une instabilité connue de UWF_Overlay.GetOverlayFiles avec un overlay volumineux ou une forte charge d’E/S.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Mémoire insuffisante ou opération non prise en charge. NTFS et de la mémoire libre sont nécessaires. Réessayez avec un overlay plus petit ou après avoir fermé les applications gourmandes.</translation></message>
    <message><source>Raw error: %1</source><translation>Code d’erreur: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Exporter dans un fichier… · Overlay · fichiers</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Fichiers texte (*.txt);;Tous les fichiers (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Fichiers de commandes (*.cmd);;Tous les fichiers (*)</translation></message>
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
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistance %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Retirer la sélection</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Retirer les exclusions sélectionnées. Prend effet après application.</translation></message>
    <message><source>Open containing folder</source><translation>Ouvrir le dossier parent</translation></message>
    <message><source>Copy file path</source><translation>Copier le chemin du fichier</translation></message>
    <message><source>Copy registry path</source><translation>Copier le chemin du Registre</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Enregistrer sur disque les modifications de ce dossier dans l’overlay…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Enregistrer sur disque les modifications de ce fichier dans l’overlay…</translation></message>
    <message><source>Copied to clipboard</source><translation>Copié dans le presse-papiers</translation></message>
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
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Gestionnaire du filtre d’écriture unifié (UWF)</translation></message>
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
    <message><source>Switch display language</source><translation>Changer la langue d’affichage</translation></message>
    <message><source>Language</source><translation>Langue</translation></message>
    <message><source>Theme</source><translation>Thème</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Basculer entre thème clair et sombre</translation></message>
    <message><source>System information</source><translation>Informations système</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Instantané de diagnostic</translation></message>
    <message><source>Read only</source><translation>Lecture seule</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Cible introuvable ; rien à enregistrer dans l’overlay.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>La cible existe uniquement dans l’overlay, créée sous UWF et jamais enregistrée sur le support ou dans le Registre persistant. Elle disparaîtra au redémarrage ; enregistrer sa suppression n’est ni nécessaire ni possible.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>Le système a refusé un paramètre (chemin ou argument invalide).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Échec de l’opération. Un autre processus utilise peut-être la cible : dossier ouvert dans l’Explorateur ou fichier ouvert. Fermez les programmes concernés et réessayez.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Échec de l’opération ; consultez le journal pour plus de détails.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Opération refusée (code %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Inconnu.</translation></message>
    <message><source>Commit canceled</source><translation>Enregistrement annulé</translation></message>

    <message><source>Succeeded</source><translation>Réussi</translation></message>
    <message><source>Skipped</source><translation>Ignoré</translation></message>
    <message><source>Failed</source><translation>Échec</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Réussi: %1; Ignoré: %2; Échec: %3.</translation></message>

    <message><source>Existed before</source><translation>Présent avant</translation></message>
    <message><source>Exists after</source><translation>Présent après</translation></message>
    <message><source>Error code</source><translation>Code d’erreur</translation></message>
    <message><source>Reason</source><translation>Motif</translation></message>
    <message><source>Previous page</source><translation>Page précédente</translation></message>
    <message><source>First page</source><translation>Première page</translation></message>
    <message><source>Last page</source><translation>Dernière page</translation></message>
    <message><source>Next page</source><translation>Page suivante</translation></message>
    <message><source>Copy selected rows</source><translation>Copier les lignes sélectionnées</translation></message>
    <message><source>Copy all</source><translation>Tout copier</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Cette exclusion du Registre est nécessaire tant que la liste d’enregistrement contient des entrées. Fichiers à enregistrer: Effacer → Retrait d’une exclusion du Registre.</translation></message>
    <message><source>Copy current page</source><translation>Copier la page actuelle</translation></message>
    <message><source>No pending changes</source><translation>Aucune modification en attente</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Exclusions du Registre · Paramètres généraux.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: Protection UWF / Exclusions de fichiers / Fichiers à enregistrer.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Non pris en charge</translation></message>
    <message><source>Review and apply changes</source><translation>Vérifier et appliquer les modifications</translation></message>
    <message><source>Confirm apply</source><translation>Confirmer l’application</translation></message>
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
    <message><source>Calculating files for automatic commit…</source><translation>Calcul des fichiers à enregistrer automatiquement…</translation></message>
    <message><source>Committing staged files</source><translation>Enregistrement des fichiers préparés</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 fichiers traités sur %2.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Aucun détail d’erreur supplémentaire.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>L’enregistrement automatique s’est arrêté de façon inattendue :
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Enregistrement automatique incomplet: Erreur inconnue.</translation></message>
    <message><source>File staging will be skipped</source><translation>L’enregistrement automatique sera ignoré</translation></message>
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

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Le chemin n’a pas de lettre de lecteur. Identification du volume cible: Échec.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Aucun enregistrement de session actuelle pour le volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Ce chemin figure dans les exclusions de fichiers.
Exclusion : %1</translation></message>

    <message><source>Continue</source><translation>Continuer</translation></message>
    <message><source>Delete and commit</source><translation>Supprimer et enregistrer</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Cette clé figure dans les exclusions du Registre.
Exclusion : %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Aucun enregistrement du filtre du Registre pour la session actuelle.</translation></message>

    <message><source>Export commands to file</source><translation>Exporter les commandes dans un fichier</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Exporter dans un fichier… · Commande: %1
%2</translation></message>
    <message><source>Import</source><translation>Importer</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Saisissez ou collez une commande par ligne. Charger un texte, journal ou script (.txt, .bat, .ps1) ; les lignes contenant cette commande sont ajoutées. L’importation prépare les modifications. Seul « Vérifier et appliquer » dans la barre d’outils les écrit dans le système.</translation></message>
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
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Fichiers texte (*.txt *.cmd);;Tous les fichiers (*)</translation></message>
    <message><source>Status</source><translation>État</translation></message>
    <message><source>Import failed</source><translation>Échec de l’importation</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Erreur interne : aucun gestionnaire d’application enregistré.</translation></message>
    <message><source>Nothing to import</source><translation>Aucune commande à importer</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Aucune commande uwfmgr trouvée dans la saisie.</translation></message>
    <message><source>Duplicates: %1</source><translation>Doublon: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Non pris en charge: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Lot %1 ──</translation></message>
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
    <message><source>Already in the target state — no-op</source><translation>État cible déjà atteint ; aucune action nécessaire</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Chemin hors de ce volume, ou exclusions de fichiers non prises en charge (exFAT / ReFS, par exemple)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Refusé par la liste de blocage UWF (fichiers système, Windows, fichier d’échange, etc.)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valeur invalide : Taille: %1</translation></message>
    <message><source>maximum size</source><translation>Taille maximale</translation></message>
    <message><source>warning threshold</source><translation>Seuil d’avertissement</translation></message>
    <message><source>critical threshold</source><translation>Seuil critique</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>La configuration globale est indisponible ; la commande n’a pas été ajoutée aux modifications en attente.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Volume %1 inconnu (aucun disque compatible UWF avec cette lettre)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: État cible déjà atteint ; aucune action nécessaire</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Le chemin n’a pas de lettre de lecteur. Identification du volume cible: Échec.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Non pris en charge</translation></message>
    <message><source>file exclusion</source><translation>Exclusions de fichiers</translation></message>
    <message><source>registry exclusion</source><translation>Exclusions du Registre</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Les exclusions du Registre nécessitent l’onglet Registre, qui est absent</translation></message>
    <message><source>Unsupported command</source><translation>Commande: Non pris en charge</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Afficher/masquer.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Activé</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Désactivé</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Inconnu</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Utilisé %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Mode avancé</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordonner l’enregistrement automatique avec l’arrêt et le redémarrage de Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Échec: Lecture de l’état du mode avancé (Erreur inconnue).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Échec: Lecture de l’état du service:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Échec: Lecture de l’état du service (Erreur inconnue).</translation></message>
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
    <message><source>Show overlay hub</source><translation>Afficher le HUB d’overlay</translation></message>
    <message><source>Hide overlay hub</source><translation>Masquer le HUB d’overlay</translation></message>
    <message><source>Exit application</source><translation>Quitter l’application</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Échec: Connexion au système</translation></message>
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
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Le chemin n’existe pas ; rien à supprimer.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Erreur inconnue.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Résultat impossible à confirmer : la relecture de l’état de la cible a échoué : %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Le fournisseur a accepté la suppression, mais la vérification de référence confirme que la cible existe encore.</translation></message>
    <message><source>Unknown</source><translation>Inconnu</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Échec: Chargement du journal (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Échec: Chargement du journal (Erreur inconnue).</translation></message>
    <message><source>: unknown error</source><translation>: Erreur inconnue</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Échec: Identification du volume cible (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Échec: Énumération des clés du Registre (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Échec: Identification du volume cible (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Opération ignorée, car une opération préalable a échoué ou n’a pas pu être confirmée.</translation></message>
    <message><source>Copy</source><translation>Copier</translation></message>
    <message><source>Select All</source><translation>Tout sélectionner</translation></message>
    <message><source>Display language</source><translation>Langue d’affichage</translation></message>
    <message><source>Folder</source><translation>Dossier</translation></message>
    <message><source>File</source><translation>Fichier</translation></message>
    <message><source>Export commands</source><translation>Exporter les commandes</translation></message>
    <message><source>Direct restart…</source><translation>Redémarrage direct…</translation></message>
    <message><source>Clear…</source><translation>Effacer…</translation></message>
    <message><source>Committing</source><translation>Enregistrement</translation></message>
    <message><source> · %1 selected</source><translation> · %1 sélectionnés</translation></message>
    <message><source>%1 changes</source><translation>%1 modifications</translation></message>
    <message><source>%1 commands</source><translation>%1 commandes</translation></message>
    <message><source>%1 items</source><translation>%1 éléments</translation></message>
    <message><source>%1 items · %2</source><translation>%1 éléments · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 modifications en attente</translation></message>
    <message><source>%1 records</source><translation>%1 enregistrements</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 enregistrements · %2 au total</translation></message>


    <message><source>%1 selected</source><translation>%1 sélectionnés</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 fichiers</translation></message>
    <message><source>32-bit</source><translation>32 bits</translation></message>
    <message><source>64-bit</source><translation>64 bits</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Distribué sous la &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Les conditions complètes figurent dans le fichier LICENSE fourni avec l’application.&lt;/p&gt;&lt;p&gt;Vous pouvez redistribuer ou modifier ce logiciel sous GPL v3. Il est fourni en l’état, sans aucune garantie.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows désactive les fichiers d’échange, la restauration du système, SuperFetch, l’indexation des fichiers, la défragmentation et le démarrage rapide. Il définit aussi bootstatuspolicy sur ignoreallfailures dans le BCD.&lt;/p&gt;&lt;p&gt;Vous pouvez modifier ces réglages ensuite. Par exemple, déplacez le fichier d’échange vers un volume non protégé avant de le réactiver.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Un instantané de diagnostic du système, des privilèges et d’UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>Un fichier existe déjà à cet emplacement :
%1

L’enregistrement remplacera son contenu.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Un gestionnaire graphique de Windows Unified Write Filter.
Gérez la protection des volumes, la surcouche, les exclusions et les fichiers en attente.</translation></message>
    <message><source>About this snapshot</source><translation>À propos de cet instantané</translation></message>
    <message><source>Account</source><translation>Compte</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Ajouter une configuration UWF à partir d’un script ou d’un texte.</translation></message>
    <message><source>Add exclusion</source><translation>Ajouter une exclusion</translation></message>
    <message><source>Add files or folders</source><translation>Ajouter des fichiers ou dossiers</translation></message>
    <message><source>Add selected items</source><translation>Ajouter les éléments sélectionnés</translation></message>
    <message><source>Added</source><translation>Ajouté</translation></message>
    <message><source>Administrator</source><translation>Administrateur</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Toutes les modifications ont été enregistrées durablement sur le disque, sans redémarrage nécessaire.</translation></message>
    <message><source>All files (*)</source><translation>Tous les fichiers (*)</translation></message>
    <message><source>All levels</source><translation>Tous les niveaux</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Un instantané de diagnostic vide ne signifie pas que la surcouche ne contient aucune modification.</translation></message>
    <message><source>Application path</source><translation>Chemin de l’application</translation></message>
    <message><source>Application result</source><translation>Résultat de l’application</translation></message>
    <message><source>Apply changes</source><translation>Appliquer les modifications</translation></message>
    <message><source>Architecture</source><translation>Architecture</translation></message>
    <message><source>Automatic</source><translation>Automatique</translation></message>
    <message><source>Automatic file staging</source><translation>Mise en attente automatique des fichiers</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Enregistrement automatique incomplet</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Enregistrer automatiquement les fichiers admissibles en attente lors d’un arrêt ou redémarrage normal.</translation></message>
    <message><source>Available</source><translation>Disponible</translation></message>
    <message><source>Back</source><translation>Précédent</translation></message>
    <message><source>Background service</source><translation>Service en arrière-plan</translation></message>
    <message><source>Binding</source><translation>Association</translation></message>
    <message><source>By drive letter</source><translation>Par lettre de lecteur</translation></message>
    <message><source>By volume ID</source><translation>Par identifiant de volume</translation></message>
    <message><source>Cancel reading</source><translation>Annuler la lecture</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Les modifications déjà validées sont conservées.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Sélectionnez la clé de Registre à exclure de la protection UWF, y compris ses sous-clés et ses valeurs.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Choisissez où enregistrer le fichier.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Choisissez un emplacement dans le lecteur ou dossier autorisé.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Choisissez un autre dossier ou vérifiez les autorisations d’accès, puis réessayez.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Choisissez une autre clé ou vérifiez les autorisations d’accès, puis réessayez.</translation></message>
    <message><source>Clear filters</source><translation>Effacer les filtres</translation></message>
    <message><source>Clear logs</source><translation>Effacer les journaux</translation></message>
    <message><source>Clear logs?</source><translation>Effacer les journaux ?</translation></message>
    <message><source>Collapse</source><translation>Réduire</translation></message>
    <message><source>Commit changes</source><translation>Valider les modifications</translation></message>
    <message><source>Commit changes…</source><translation>Valider les modifications…</translation></message>
    <message><source>Commit complete</source><translation>Validation terminée</translation></message>
    <message><source>Commit file changes?</source><translation>Valider les modifications du fichier ?</translation></message>
    <message><source>Commit folder changes?</source><translation>Valider les modifications du dossier ?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Valider les modifications de la clé de registre ?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Valider les modifications de la valeur de registre ?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Valide les fichiers en attente avant un arrêt ou redémarrage normal</translation></message>
    <message><source>Compatibility mode</source><translation>Mode de compatibilité</translation></message>
    <message><source>Configuration applied</source><translation>Configuration appliquée</translation></message>
    <message><source>Configuration error</source><translation>Erreur de configuration</translation></message>
    <message><source>Configured</source><translation>Configuré</translation></message>
    <message><source>Confirm before continuing</source><translation>Confirmer avant de continuer</translation></message>
    <message><source>Connected and authenticated</source><translation>Connecté et authentifié</translation></message>
    <message><source>Connecting…</source><translation>Connexion…</translation></message>
    <message><source>Contact author</source><translation>Contacter l’auteur</translation></message>
    <message><source>Continue editing</source><translation>Continuer la modification</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Poursuivre le redémarrage supprimera les modifications des fichiers qui n’ont pas pu être enregistrés.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Poursuivre l’arrêt supprimera les modifications des fichiers qui n’ont pas pu être enregistrés.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Coordonner avec l’arrêt de Windows</translation></message>
    <message><source>Copy all logs</source><translation>Copier tous les journaux</translation></message>
    <message><source>Copy full report</source><translation>Copier le rapport complet</translation></message>
    <message><source>Copy name</source><translation>Copier le nom</translation></message>
    <message><source>Copy path</source><translation>Copier le chemin</translation></message>
    <message><source>Copy selection</source><translation>Copier la sélection</translation></message>
    <message><source>Copy version</source><translation>Copier la version</translation></message>
    <message><source>Counting staged files…</source><translation>Comptage des fichiers en attente…</translation></message>
    <message><source>Create</source><translation>Créer</translation></message>
    <message><source>Critical %1 MB</source><translation>Critique %1 Mo</translation></message>
    <message><source>Current filter</source><translation>Filtre actuel</translation></message>
    <message><source>Current run</source><translation>Exécution actuelle</translation></message>
    <message><source>Current selection</source><translation>Sélection actuelle</translation></message>
    <message><source>Current usage</source><translation>Utilisation actuelle</translation></message>
    <message><source>Date modified</source><translation>Date de modification</translation></message>
    <message><source>Debug</source><translation>Débogage</translation></message>
    <message><source>Delete and commit this file?</source><translation>Supprimer ce fichier et valider sa suppression ?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Supprimer ce dossier et valider sa suppression ?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Supprimer cette clé de registre et valider sa suppression ?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Supprimer cette valeur de registre et valider sa suppression ?</translation></message>

    <message><source>Direct restart?</source><translation>Redémarrer directement ?</translation></message>
    <message><source>Direct shutdown?</source><translation>Arrêter directement ?</translation></message>
    <message><source>Discard changes</source><translation>Abandonner les modifications</translation></message>
    <message><source>Disk</source><translation>Disque</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Persistance du secret de domaine</translation></message>
    <message><source>Done</source><translation>Terminé</translation></message>
    <message><source>Edit commands</source><translation>Modifier les commandes</translation></message>
    <message><source>Editable</source><translation>Modifiable</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Activez le filtre et redémarrez pour afficher l’utilisation réelle</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>Le mode avancé ajoute un service en arrière-plan qui démarre avec Windows. Supprimez ce service avant de déplacer ou de supprimer l’application.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>Le mode avancé est indisponible. Utilisez Arrêt sécurisé ou Redémarrage sécurisé pour enregistrer les fichiers en attente.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Saisissez HKLM\SOFTWARE\… et appuyez sur Entrée pour naviguer</translation></message>
    <message><source>Enter a file name</source><translation>Saisir un nom de fichier</translation></message>
    <message><source>Enter a valid file name.</source><translation>Saisissez un nom de fichier valide.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Saisissez un nom de fichier ou de dossier valide.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Saisissez un nom de dossier valide.</translation></message>
    <message><source>Error</source><translation>Erreur</translation></message>
    <message><source>Error details</source><translation>Détails de l’erreur</translation></message>
    <message><source>Error details and suggestions</source><translation>Détails de l’erreur et suggestions</translation></message>
    <message><source>Expand</source><translation>Développer</translation></message>
    <message><source>Export list</source><translation>Exporter la liste</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Impossible d’effacer les journaux : %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Impossible d’effacer les journaux : erreur inattendue.</translation></message>
    <message><source>File name</source><translation>Nom du fichier</translation></message>
    <message><source>File or folder</source><translation>Fichier ou dossier</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>L’enregistrement des fichiers a commencé et ne peut plus être annulé.</translation></message>
    <message><source>File saving incomplete</source><translation>Enregistrement des fichiers incomplet</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>La mise en attente sera ignorée. Les modifications non validées des chemins protégés seront perdues après le redémarrage.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>La mise en attente sera ignorée. Les modifications non validées des chemins protégés seront perdues après l’arrêt.</translation></message>
    <message><source>File type</source><translation>Type de fichier</translation></message>
    <message><source>Files and folders</source><translation>Fichiers et dossiers</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Les fichiers et dossiers peuvent être sélectionnés ensemble.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Les fichiers en attente ne seront pas enregistrés.</translation></message>
    <message><source>Filter is disabled</source><translation>Le filtre est désactivé</translation></message>
    <message><source>Fit columns to contents</source><translation>Ajuster les colonnes au contenu</translation></message>
    <message><source>Folder address</source><translation>Adresse du dossier</translation></message>
    <message><source>Folder name</source><translation>Nom du dossier</translation></message>
    <message><source>Folders</source><translation>Dossiers</translation></message>
    <message><source>Full configuration</source><translation>Configuration complète</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Les futures écritures dans cette clé ne seront pas protégées par UWF.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Les futures écritures dans ce chemin ne seront pas protégées par UWF.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Exclusion globale du registre · sous-clés incluses</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Remontez d’un dossier ou choisissez un autre emplacement.</translation></message>
    <message><source>Home</source><translation>Dossier personnel</translation></message>
    <message><source>Import configuration</source><translation>Importer la configuration</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>L’importation met à jour le brouillon de configuration. Utilisez Vérifier et appliquer pour écrire les modifications dans le système.</translation></message>
    <message><source>Import results</source><translation>Résultats d’importation</translation></message>
    <message><source>Imported</source><translation>Importé</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>Le rapport présente les résultats des commandes pour le brouillon de configuration. Sélectionnez une ligne pour afficher la commande et le résultat complets.</translation></message>
    <message><source>Imported: %1</source><translation>Importé : %1</translation></message>
    <message><source>Information</source><translation>Information</translation></message>
    <message><source>Item details</source><translation>Détails de l’élément</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>L’enregistrement d’une longue liste de fichiers en attente peut prendre plus de temps.</translation></message>
    <message><source>License and copyright</source><translation>Licence et droits d’auteur</translation></message>
    <message><source>Line</source><translation>Ligne</translation></message>
    <message><source>Local disk (%1)</source><translation>Disque local (%1)</translation></message>
    <message><source>Log level</source><translation>Niveau de journalisation</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Échec de lecture des journaux. Réessayez ou vérifiez l’accès au fichier journal.</translation></message>
    <message><source>Maximize</source><translation>Agrandir</translation></message>
    <message><source>Message details</source><translation>Détails du message</translation></message>
    <message><source>Minimize</source><translation>Réduire la fenêtre</translation></message>
    <message><source>Module</source><translation>Module</translation></message>
    <message><source>More</source><translation>Plus</translation></message>
    <message><source>Move</source><translation>Déplacer</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Si UWF protège le lecteur où se trouve l’application, assurez-vous d’abord que les fichiers de l’application seront conservés après un redémarrage. L’enregistrement définitif d’un grand nombre de fichiers en attente peut prolonger l’arrêt ou le redémarrage.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Plusieurs fichiers peuvent être sélectionnés.</translation></message>
    <message><source>Name and location</source><translation>Nom et emplacement</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Valeur nommée : %1 · cette valeur uniquement</translation></message>
    <message><source>Navigate to registry path</source><translation>Accéder au chemin du registre</translation></message>
    <message><source>New folder</source><translation>Nouveau dossier</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Les nouveaux enregistrements apparaîtront après actualisation.</translation></message>
    <message><source>Next-session configuration</source><translation>Configuration de la prochaine session</translation></message>
    <message><source>No files to commit</source><translation>Aucun fichier à valider</translation></message>
    <message><source>No logs yet</source><translation>Aucun journal pour l’instant</translation></message>
    <message><source>No matching items</source><translation>Aucun élément correspondant</translation></message>
    <message><source>No matching logs</source><translation>Aucun journal correspondant</translation></message>
    <message><source>No overlay files found</source><translation>Aucun fichier de surcouche trouvé</translation></message>
    <message><source>No registry key selected</source><translation>Aucune clé de registre sélectionnée</translation></message>
    <message><source>None</source><translation>Aucun</translation></message>
    <message><source>Not configured</source><translation>Non configuré</translation></message>
    <message><source>Not connected</source><translation>Non connecté</translation></message>
    <message><source>Not installed</source><translation>Non installé</translation></message>
    <message><source>Not written to the system</source><translation>Non écrit dans le système</translation></message>
    <message><source>Off</source><translation>Désactivé</translation></message>
    <message><source>On</source><translation>Activé</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Une commande uwfmgr par ligne</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Conservez l’application dans un dossier fixe que les utilisateurs standard ne peuvent pas modifier. Emplacement conseillé :</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Seuls les fichiers en attente sont enregistrés automatiquement. Les autres modifications non validées des chemins protégés seront perdues après le redémarrage.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Seuls les fichiers en attente sont enregistrés automatiquement. Les autres modifications non validées des chemins protégés seront perdues après l’arrêt.</translation></message>
    <message><source>Open</source><translation>Ouvrir</translation></message>
    <message><source>Open folder</source><translation>Ouvrir le dossier</translation></message>
    <message><source>Operating system</source><translation>Système d’exploitation</translation></message>
    <message><source>Operation and security</source><translation>Fonctionnement et sécurité</translation></message>
    <message><source>Original command</source><translation>Commande d’origine</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Seuil critique de la surcouche atteint. Enregistrez les modifications et redémarrez en toute sécurité.</translation></message>
    <message><source>Overlay maximum size</source><translation>Taille maximale de la surcouche</translation></message>
    <message><source>Overlay type</source><translation>Type de surcouche</translation></message>
    <message><source>Overlay usage</source><translation>Utilisation de la surcouche</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Seuil d’avertissement de la surcouche atteint. Enregistrez les modifications et redémarrez en toute sécurité.</translation></message>
    <message><source>Overview</source><translation>Vue d’ensemble</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Collez des commandes ou chargez-les depuis un fichier…</translation></message>
    <message><source>Pending changes</source><translation>Modifications en attente</translation></message>
    <message><source>Pending removal</source><translation>Suppression en attente</translation></message>
    <message><source>Permissions and protection</source><translation>Autorisations et protection</translation></message>
    <message><source>Privileges</source><translation>Privilèges</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2 éléments traités</translation></message>
    <message><source>Processing result</source><translation>Résultat du traitement</translation></message>
    <message><source>Protected</source><translation>Protégé</translation></message>
    <message><source>Protected volumes</source><translation>Volumes protégés</translation></message>
    <message><source>Protection and overlay</source><translation>Protection et surcouche</translation></message>
    <message><source>Quick access</source><translation>Accès rapide</translation></message>
    <message><source>Raw report</source><translation>Rapport brut</translation></message>
    <message><source>Read again</source><translation>Relire</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Relisez pour obtenir un instantané de diagnostic.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Lire la configuration pour afficher l’utilisation réelle</translation></message>
    <message><source>Read-only</source><translation>Lecture seule</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>La lecture d’une grande surcouche peut être longue. Vous pouvez annuler à tout moment.</translation></message>
    <message><source>Reading canceled</source><translation>Lecture annulée</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Lecture des fichiers et sous-dossiers à cet emplacement…</translation></message>
    <message><source>Reading folder</source><translation>Lecture du dossier</translation></message>
    <message><source>Reading logs</source><translation>Lecture des journaux</translation></message>
    <message><source>Reading overlay</source><translation>Lecture de la surcouche</translation></message>
    <message><source>Ready</source><translation>Prêt</translation></message>
    <message><source>Ready to save</source><translation>Prêt à enregistrer</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Les enregistrements de l’exécution actuelle apparaîtront ici à la fin de la lecture.</translation></message>
    <message><source>Recursive staging</source><translation>Mise en attente récursive</translation></message>
    <message><source>Refresh folder</source><translation>Actualiser le dossier</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Clé de registre ou réglage de persistance</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Les tâches restantes peuvent être annulées après le traitement de la cible actuelle.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Retirer cette clé des exclusions de la prochaine session.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Retirer ce chemin des exclusions de la prochaine session.</translation></message>
    <message><source>Removed</source><translation>Supprimé</translation></message>
    <message><source>Rename</source><translation>Renommer</translation></message>
    <message><source>Rename…</source><translation>Renommer…</translation></message>
    <message><source>Repair required</source><translation>Réparation nécessaire</translation></message>
    <message><source>Replace file</source><translation>Remplacer le fichier</translation></message>
    <message><source>Replace the existing file?</source><translation>Remplacer le fichier existant ?</translation></message>
    <message><source>Required configuration</source><translation>Configuration requise</translation></message>
    <message><source>Restart anyway</source><translation>Redémarrer quand même</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Le redémarrage reprendra après l’enregistrement. Gardez l’application ouverte.</translation></message>
    <message><source>Restore</source><translation>Restaurer</translation></message>
    <message><source>Retry</source><translation>Réessayer</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Consultez les détails des échecs avant de poursuivre l’arrêt.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Consultez les détails des échecs avant de redémarrer.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Consultez les détails de l’opération. Lisez l’état actuel du système avant de réessayer des modifications qui ont pu être écrites.</translation></message>
    <message><source>Review the result for each target.</source><translation>Vérifiez le résultat pour chaque cible.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Vérifiez ces modifications avant de les écrire dans le système.</translation></message>
    <message><source>Running</source><translation>En cours d’exécution</translation></message>
    <message><source>Save</source><translation>Enregistrer</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Enregistrer les fichiers en attente sur le disque, puis redémarrer.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Enregistrer les fichiers en attente sur le disque, puis arrêter.</translation></message>
    <message><source>Saving staged files</source><translation>Enregistrement des fichiers en attente</translation></message>
    <message><source>Saving to disk</source><translation>Écriture sur le disque</translation></message>
    <message><source>Saving…</source><translation>Enregistrement…</translation></message>
    <message><source>Scanning staged paths</source><translation>Analyse des chemins en attente</translation></message>
    <message><source>Search logs</source><translation>Rechercher dans les journaux</translation></message>
    <message><source>Search messages or modules</source><translation>Rechercher des messages ou modules</translation></message>
    <message><source>Search paths</source><translation>Rechercher des chemins</translation></message>
    <message><source>Search this folder</source><translation>Rechercher dans ce dossier</translation></message>
    <message><source>Select a file or enter its name</source><translation>Sélectionner un fichier ou saisir son nom</translation></message>
    <message><source>Select a file to open.</source><translation>Sélectionnez un fichier à ouvrir.</translation></message>
    <message><source>Select a folder.</source><translation>Sélectionnez un dossier.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Sélectionnez une clé pour agir sur son arborescence, ou une valeur nommée pour agir uniquement sur cette valeur.</translation></message>
    <message><source>Select an available subkey</source><translation>Sélectionner une sous-clé disponible</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Sélectionnez uniquement des fichiers ou ouvrez un dossier pour parcourir son contenu.</translation></message>
    <message><source>Select files or folders</source><translation>Sélectionner des fichiers ou dossiers</translation></message>
    <message><source>Select folder</source><translation>Sélectionner un dossier</translation></message>
    <message><source>Select folders only.</source><translation>Sélectionnez uniquement des dossiers.</translation></message>
    <message><source>Select registry key or value</source><translation>Sélectionner une clé ou valeur de registre</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Sélectionner une clé ou valeur de registre à supprimer</translation></message>
    <message><source>Select the current folder</source><translation>Sélectionner le dossier actuel</translation></message>
    <message><source>Select this file</source><translation>Sélectionner ce fichier</translation></message>
    <message><source>Select this registry key</source><translation>Sélectionner cette clé de registre</translation></message>
    <message><source>Select this registry value</source><translation>Sélectionner cette valeur de registre</translation></message>
    <message><source>Selected file</source><translation>Fichier sélectionné</translation></message>
    <message><source>Selected folder</source><translation>Dossier sélectionné</translation></message>
    <message><source>Selection</source><translation>Sélection</translation></message>
    <message><source>Service account</source><translation>Compte du service</translation></message>
    <message><source>Service configuration needs repair</source><translation>La configuration du service nécessite une réparation</translation></message>
    <message><source>Service details</source><translation>Détails du service</translation></message>
    <message><source>Service name</source><translation>Nom du service</translation></message>
    <message><source>Service status</source><translation>État du service</translation></message>
    <message><source>Service stopped</source><translation>Service arrêté</translation></message>
    <message><source>Shut down anyway</source><translation>Arrêter quand même</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>L’arrêt reprendra après l’enregistrement. Gardez l’application ouverte.</translation></message>
    <message><source>Size</source><translation>Taille</translation></message>
    <message><source>Some changes could not be applied</source><translation>Certaines modifications n’ont pas pu être appliquées</translation></message>
    <message><source>Some files could not be saved</source><translation>Certains fichiers n’ont pas pu être enregistrés</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Certaines informations matérielles n’ont pas pu être lues. Les autres informations de diagnostic restent consultables et copiables.</translation></message>
    <message><source>Source code</source><translation>Code source</translation></message>
    <message><source>Staged file</source><translation>Fichier en attente</translation></message>
    <message><source>Staged path</source><translation>Chemin en attente</translation></message>
    <message><source>Standard user</source><translation>Utilisateur standard</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Démarre automatiquement avec Windows</translation></message>
    <message><source>Startup type</source><translation>Type de démarrage</translation></message>
    <message><source>Status unavailable</source><translation>État indisponible</translation></message>
    <message><source>Stopped</source><translation>Arrêté</translation></message>
    <message><source>Storage</source><translation>Stockage</translation></message>
    <message><source>System</source><translation>Système</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Modifications du système à la première activation d’UWF</translation></message>
    <message><source>System memory</source><translation>Mémoire système</translation></message>
    <message><source>System metadata</source><translation>Métadonnées système</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Les métadonnées système servent uniquement au diagnostic et ne peuvent pas être validées.</translation></message>
    <message><source>Takes effect after restart</source><translation>Prend effet après redémarrage</translation></message>
    <message><source>Target</source><translation>Cible</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistance des licences Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Le fournisseur UWF n’a pas pu terminer l’énumération. Cela peut arriver avec une grande surcouche ou des ressources système insuffisantes.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>La configuration a été écrite dans le système. La plupart des modifications prennent effet après redémarrage.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>La configuration correspond à la cible de la prochaine session. Consultez ou exportez les commandes dans Configuration complète.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Les enregistrements actuels du journal seront effacés définitivement.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Le dossier de destination n’existe pas.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>L’état du service du mode avancé n’a pas pu être lu. Vérifiez les autorisations du service, puis réessayez.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>La liste peut omettre des fichiers ou inclure des fichiers déjà validés. Ne l’utilisez pas comme liste de validation.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>L’élément sélectionné n’existe plus : %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Le service est installé mais arrêté. Démarrez-le pour rétablir l’enregistrement automatique.</translation></message>
    <message><source>This PC</source><translation>Ce PC</translation></message>
    <message><source>This folder is empty</source><translation>Ce dossier est vide</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Ce rapport peut contenir des noms de comptes, des identifiants d’appareils et des chemins de fichiers. Vérifiez-le avant de le partager.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Enregistrez ce script CMD UTF-8 dans un fichier .cmd et exécutez-le dans l’invite de commandes. L’exportation utilise le même script.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Essayez un autre mot-clé ou niveau de journalisation.</translation></message>
    <message><source>Try another search term.</source><translation>Essayez un autre terme de recherche.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Le type et la taille maximale ne peuvent pas être modifiés lorsque le filtre est actif. Désactivez le filtre et redémarrez d’abord.</translation></message>
    <message><source>UI agent</source><translation>Agent d’interface</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 lignes</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>La configuration UWF n’a pas pu être lue. Vérifiez que Unified Write Filter est installé dans Windows, puis relisez.</translation></message>
    <message><source>UWF feature</source><translation>Fonctionnalité UWF</translation></message>
    <message><source>UWF filter</source><translation>Filtre UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF est actuellement indisponible</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Le redémarrage sécurisé UWF sera exécuté directement.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>L’arrêt sécurisé UWF sera exécuté directement.</translation></message>
    <message><source>Unable to access this folder</source><translation>Impossible d’accéder à ce dossier</translation></message>
    <message><source>Unable to access this registry key</source><translation>Impossible d’accéder à cette clé de registre</translation></message>
    <message><source>Unable to read logs</source><translation>Impossible de lire les journaux</translation></message>
    <message><source>Unable to read service status</source><translation>Impossible de lire l’état du service</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Impossible de lire la surcouche pour le moment</translation></message>
    <message><source>Unable to read this folder.</source><translation>Impossible de lire ce dossier.</translation></message>
    <message><source>Unable to select this item</source><translation>Impossible de sélectionner cet élément</translation></message>
    <message><source>Unavailable</source><translation>Indisponible</translation></message>
    <message><source>Undo removal</source><translation>Annuler la suppression</translation></message>
    <message><source>Unprotected</source><translation>Non protégé</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Les éléments en échec n’ont pas été enregistrés. Sélectionnez une ligne pour voir le chemin complet et la raison.</translation></message>
    <message><source>Up one folder</source><translation>Dossier parent</translation></message>
    <message><source>Usage unavailable</source><translation>Utilisation indisponible</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Utiliser la clé entière, avec toutes ses sous-clés</translation></message>
    <message><source>Version %1</source><translation>Version %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Consulter les enregistrements de l’application et les informations de diagnostic.</translation></message>
    <message><source>View commands for these changes</source><translation>Voir les commandes de ces modifications</translation></message>
    <message><source>View failure details</source><translation>Voir les détails des échecs</translation></message>
    <message><source>View overlay</source><translation>Voir la surcouche</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Afficher l’instantané de diagnostic des fichiers de %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Association du volume %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Protection du volume %1</translation></message>
    <message><source>Volume identity</source><translation>Identité du volume</translation></message>
    <message><source>Warning</source><translation>Avertissement</translation></message>
    <message><source>Warning %1 MB</source><translation>Avertissement %1 Mo</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Clé entière et toutes ses sous-clés · récursif</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF fournit la protection en écriture. Cette application la configure et la gère.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Sans mode avancé, utilisez Arrêt sécurisé ou Redémarrage sécurisé dans cette application pour enregistrer les fichiers en attente.</translation></message>
    <message><source>Write protection</source><translation>Protection en écriture</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Écrire ces modifications de configuration dans le système ? La plupart prennent effet après redémarrage.</translation></message>
    <message><source>staged files</source><translation>fichiers en attente</translation></message>
    <message><source>uwfmgr commands</source><translation>commandes uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Marquer les chemins sélectionnés pour suppression. Prend effet après application.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Retirer de la liste de préparation automatique des fichiers après application.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Ajouter à la liste de préparation automatique des fichiers après application.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Appliquer ce plan vide la liste de préparation enregistrée et supprime son exclusion du Registre si elle n’est plus nécessaire.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Les modifications de préparation des fichiers doivent être appliquées dans UWF Manager et ne peuvent pas être exportées sous forme de commandes uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>La préparation des fichiers est enregistrée après application</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Portée de l’instantané de la surcouche</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Cette liste sert uniquement au diagnostic. Elle ne permet pas de déterminer quels fichiers doivent être validés.</translation></message>
    <message><source>NTFS volumes only</source><translation>Volumes NTFS uniquement</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>La lecture des surcouches volumineuses peut être lente ou échouer par manque de ressources.</translation></message>
    <message><source>Small files may be missing</source><translation>Des petits fichiers peuvent manquer</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Les fichiers plus petits que la taille d’un cluster du disque (généralement 4 Ko) peuvent ne pas figurer dans la liste.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>N’indique pas les validations en attente</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Des fichiers déjà validés, des chemins exclus et des éléments affectés par d’autres opérations peuvent aussi apparaître.</translation></message>

    <message><source>Command processing result</source><translation>Résultat du traitement de la commande</translation></message>
    <message><source>Copy command</source><translation>Copier la commande</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · %1 branches n’ont pas pu être lues</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>%1 branches n’ont pas pu être lues. Continuez avec le contenu découvert ou réessayez uniquement les éléments en échec.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 fichiers · %2 dossiers</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 valeurs · %2 clés</translation></message>
    <message><source>Attempts</source><translation>Tentatives</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Annuler les tâches restantes</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Enregistrer uniquement le contenu parcouru avec succès. Les branches illisibles restent inchangées.</translation></message>
    <message><source>Committing changes</source><translation>Enregistrement des modifications</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Les suppressions et enregistrements effectués sont irréversibles.</translation></message>
    <message><source>Continue…</source><translation>Continuer…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Supprimer et enregistrer uniquement les cibles découvertes. Les branches illisibles et leurs dossiers ou clés parents sont conservés.</translation></message>
    <message><source>Deleting and committing</source><translation>Suppression et enregistrement</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>La suppression et l’enregistrement sont irréversibles. Seul le périmètre sélectionné est traité ; la configuration UWF en attente n’est pas appliquée.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Détermination du périmètre. Les branches illisibles sont consignées et le parcours continue ailleurs.</translation></message>
    <message><source>Export traversal records</source><translation>Exporter le journal du parcours</translation></message>
    <message><source>No actionable content</source><translation>Aucun contenu exploitable</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Aucune cible exploitable trouvée. Rien n’a été supprimé ni enregistré.</translation></message>
    <message><source>No changes have been committed.</source><translation>Aucune modification enregistrée</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Rien n’a été supprimé ni enregistré</translation></message>
    <message><source>Path</source><translation>Chemin</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Supprimer définitivement le périmètre sélectionné et enregistrer les suppressions sur le volume protégé.</translation></message>
    <message><source>Preparation complete</source><translation>Préparation terminée</translation></message>
    <message><source>Preparing commit</source><translation>Préparation de l’enregistrement</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Préparation de la suppression et de l’enregistrement</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Traiter uniquement le contenu parcouru avec succès</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Les points d’analyse sont ignorés pour rester dans le périmètre sélectionné.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Un redémarrage n’annule pas un enregistrement. Seul le périmètre sélectionné est traité ; la configuration UWF en attente n’est pas appliquée.</translation></message>
    <message><source>Retry failed items</source><translation>Réessayer les échecs</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Enregistrer les modifications du périmètre sélectionné sur le volume protégé.</translation></message>
    <message><source>Stopping the operation</source><translation>Arrêt de l’opération</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Les réussites et les échecs sont consignés</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Le dossier n’est pas vide ; le contenu non traité est conservé.</translation></message>
    <message><source>The operation could not be completed</source><translation>L’opération n’a pas pu être terminée</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>La clé de registre contient de nouvelles valeurs ; le contenu non traité est conservé.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>La clé de registre contient encore des sous-clés ; le contenu non traité est conservé.</translation></message>
    <message><source>The target no longer exists.</source><translation>La cible n’existe plus.</translation></message>
    <message><source>Traversal complete</source><translation>Parcours terminé</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Parcours terminé, certains contenus sont illisibles</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Le journal du parcours est conservé et peut être exporté.</translation></message>
    <message><source>Traverse again</source><translation>Parcourir à nouveau</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Parcourez à nouveau ou fermez cette fenêtre.</translation></message>
    <message><source>Traversing the folder</source><translation>Parcours du dossier</translation></message>
    <message><source>Traversing the registry</source><translation>Parcours du registre</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Impossible de préparer l’opération</translation></message>
    <message><source>View results</source><translation>Afficher les résultats</translation></message>

    <message><source>All changes committed</source><translation>Toutes les modifications ont été enregistrées</translation></message>
    <message><source>Some changes committed</source><translation>Modifications partiellement enregistrées</translation></message>
    <message><source>No changes could be committed</source><translation>Aucune modification n’a pu être enregistrée</translation></message>
    <message><source>No changes to commit</source><translation>Aucune modification à enregistrer</translation></message>
    <message><source>Commit interrupted</source><translation>Enregistrement interrompu</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 éléments n’ont pas été traités.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 opération(s) du registre ont échoué.</translation></message>
    <message><source>The folder does not exist.</source><translation>Le dossier n’existe pas.</translation></message>
    <message><source>Access denied.</source><translation>Accès refusé.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Un élément portant ce nom existe déjà.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Impossible de créer le dossier. Vérifiez le nom et les autorisations.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Impossible de renommer cet élément. Vérifiez le nom et les autorisations.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Ce navigateur ne prend pas en charge le renommage.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Le brouillon des fichiers à enregistrer a changé depuis l’ouverture de ce plan. Vérifiez un nouveau plan.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>La liste enregistrée des fichiers à enregistrer a changé depuis l’ouverture de ce plan. Actualisez et vérifiez les modifications.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>La liste enregistrée des fichiers à enregistrer ne correspond pas au plan accepté.</translation></message>
    <message><source>Export configuration</source><translation>Exporter la configuration</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Enregistrer le thème, la langue et la visibilité actuels du HUB dans UWF.ini, à côté de cette application.</translation></message>
    <message><source>Configuration saved</source><translation>Configuration enregistrée</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Les paramètres actuels de l’interface ont été enregistrés dans :
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini existe déjà. Le fichier existant n’a pas été modifié :
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>Impossible d’enregistrer UWF.ini :
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Bureau</translation></message>
    <message><source>Downloads</source><translation>Téléchargements</translation></message>
    <message><source>Documents</source><translation>Documents</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>L’enregistrement des fichiers s’est arrêté. Corrigez l’erreur avant de réessayer.</translation></message>
    <message><source>No net change for this setting</source><translation>La valeur finale de ce paramètre reste inchangée après l’importation</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Valeur finale du brouillon : %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Valeur finale du brouillon : %1 → %2 (valeur demandée : %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Valeur demandée : %1 → %2 ; remplacée par une commande ultérieure. Valeur finale du brouillon → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Brouillon de %1 mis à jour</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Brouillon du paramètre de protection du volume %1 mis à jour → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Sélectionnez les entrées à supprimer.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Les entrées sélectionnées ne peuvent pas être restaurées pour le moment.</translation></message>
</context>
</TS>
