<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="pl" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Podgląd</translation></message>
    <message><source>enable</source><translation>włącz</translation></message>
    <message><source>disable</source><translation>wyłącz</translation></message>
    <message><source>Enabled</source><translation>Włączone</translation></message>
    <message><source>Disabled</source><translation>Wyłączone</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Anuluj</translation></message>
    <message><source>Close</source><translation>Zamknij</translation></message>
    <message><source>Refresh</source><translation>Odśwież</translation></message>
    <message><source>Add</source><translation>Dodaj</translation></message>
    <message><source>Remove</source><translation>Usuń</translation></message>
    <message><source>Yes</source><translation>Tak</translation></message>
    <message><source>No</source><translation>Nie</translation></message>
    <message><source>Excluded</source><translation>Wykluczone</translation></message>
    <message><source>Not excluded</source><translation>Niewykluczone</translation></message>
    <message><source>File…</source><translation>Plik…</translation></message>
    <message><source>Folder…</source><translation>Folder…</translation></message>
    <message><source>volume ID</source><translation>identyfikator woluminu</translation></message>
    <message><source>drive letter</source><translation>litera dysku</translation></message>
    <message><source>Drive letter</source><translation>Litera dysku</translation></message>
    <message><source>Volume ID</source><translation>Identyfikator woluminu</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Stan ochrony woluminu w bieżącej sesji (tylko do odczytu).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Chroń wolumin w następnej sesji. Zapisy trafiają do nakładki i są odrzucane po restarcie.</translation></message>
    <message><source>Current session</source><translation>Bieżąca sesja</translation></message>
    <message><source>Next session</source><translation>Następna sesja</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Sposób identyfikacji woluminu. Litera dysku jest prostsza, ale jej zmiana, np. po dodaniu dysku, zrywa powiązanie. Identyfikator woluminu pozostaje stały.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Wolumin nieobsługiwany przez UWF: </translation></message>
    <message><source>Global settings</source><translation>Ustawienia globalne</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Stan filtra UWF w bieżącej sesji (tylko do odczytu).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Włącz UWF w następnej sesji. Zapisy na chronionych woluminach trafiają do nakładki i są odrzucane po restarcie.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Aby zmienić typ i maksymalny rozmiar:
1. Wyłącz filtr powyżej.
2. Uruchom ponownie, aby filtr był wyłączony.
3. Zmień ustawienie.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Miejsce nakładki: RAM jest szybszy, ale zajmuje pamięć; Disk używa dysku systemowego i daje większą pojemność. Oba są odrzucane po restarcie.</translation></message>
    <message><source>Type</source><translation>Typ</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Maksymalna pojemność nakładki. RAM jest ograniczony wielkością pamięci. Disk wymaga co najmniej 1024 MB i odpowiedniej ilości miejsca na woluminie systemowym.</translation></message>
    <message><source>Maximum size</source><translation>Maksymalny rozmiar</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Maksymalny rozmiar · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Próg ostrzeżenia</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Zapisuje ostrzeżenie w dzienniku zdarzeń, gdy użycie nakładki osiągnie tę wartość. Gdy oba zdarzenia są włączone, wartość ta musi być niższa od progu krytycznego. Wartość 0 wyłącza tylko zdarzenie ostrzeżenia.</translation></message>
    <message><source>Critical threshold</source><translation>Próg krytyczny</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Zapisuje powiadomienie krytyczne w dzienniku zdarzeń, gdy użycie nakładki osiągnie tę wartość. Gdy oba zdarzenia są włączone, wartość ta musi być wyższa od progu ostrzeżenia. Wartość 0 wyłącza tylko zdarzenie krytyczne.</translation></message>
    <message><source>Overlay</source><translation>Nakładka</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Nieobsługiwany typ dysku (tylko stałe dyski lokalne).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>Wolumin przekracza limit UWF: 16 TiB na chroniony wolumin.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: można chronić wolumin, ale wykluczenia i zapis pojedynczych plików nie są obsługiwane.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Niepowodzenie: Odczyt informacji o woluminie.</translation></message>
    <message><source>%1 free / %2</source><translation>Wolne %1 / %2</translation></message>
    <message><source>Commit</source><translation>Zapisz trwale</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Zapisz na dysku zmiany tego pliku z nakładki / Zapisz trwale zmiany rejestru. Tej operacji nie można cofnąć.</translation></message>
    <message><source>Commit file changes…</source><translation>Zapisz trwale zmiany pliku…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Wybierz plik do trwałego zapisu na dysku. Zapisz na dysku zmiany tego pliku z nakładki.</translation></message>
    <message><source>Commit folder changes…</source><translation>Zapisz trwale zmiany folderu…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Wybierz folder do trwałego zapisu na dysku. Zapisz na dysku zmiany tego folderu z nakładki. Obejmuje wszystkie podfoldery.</translation></message>
    <message><source>Delete and commit file…</source><translation>Usuń plik i trwale zapisz usunięcie na dysku…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Wybierz plik do trwałego usunięcia. Usuń plik i trwale zapisz usunięcie na dysku.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Usuń folder z zawartością i trwale zapisz usunięcia na dysku…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Wybierz folder do trwałego usunięcia. Usuń folder z zawartością i trwale zapisz usunięcia na dysku. Obejmuje wszystkie podfoldery.</translation></message>
    <message><source>Commit registry changes…</source><translation>Zapisz trwale zmiany rejestru…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Wpisz klucz i opcjonalną nazwę wartości. Pusta nazwa oznacza trwały zapis wszystkich wartości klucza i podkluczy.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Zapisz trwale usunięcie z rejestru…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Wpisz klucz do usunięcia i opcjonalną nazwę wartości. Pusta nazwa oznacza trwałe usunięcie całego drzewa: klucza, wartości i podkluczy.</translation></message>
    <message><source>File exclusions</source><translation>Wykluczenia plików</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Pliki i foldery woluminu wykluczone z ochrony UWF. Dwuklik kopiuje ścieżkę.</translation></message>
    <message><source>Registry exclusions</source><translation>Wykluczenia rejestru</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>Wykluczenia rejestru są globalne i zależą od filtra UWF, nie od ochrony woluminu. Wspólna lista jest wyświetlana tylko raz na kartach dysków. Dwuklik kopiuje ścieżkę.</translation></message>
    <message><source>File staging</source><translation>Automatyczny zapis plików</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>pliki / foldery: Automatyczny trwały zapis przed bezpiecznym wyłączeniem lub restartem. Obejmuje wszystkie podfoldery. Gdy lista zawiera wpisy, poniższe wykluczenie rejestru zachowuje ją między chronionymi sesjami: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Dodaj pliki lub folder do automatycznego zapisu.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Wybierz plik do trwałego zapisu na dysku. Automatyczny trwały zapis przed bezpiecznym wyłączeniem lub restartem.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Wybierz folder do trwałego zapisu na dysku. Automatyczny trwały zapis przed bezpiecznym wyłączeniem lub restartem. Obejmuje wszystkie podfoldery.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Wybierz plik do trwałego zapisu na dysku · Automatyczny zapis plików</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Wybierz folder do trwałego zapisu na dysku · Automatyczny zapis plików · Obejmuje wszystkie podfoldery.</translation></message>
    <message><source>Cannot stage this path</source><translation>Nie można dodać ścieżki do automatycznego zapisu</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>Ścieżka pokrywa się z wykluczeniem UWF. Najpierw usuń wykluczenie:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>Ścieżka pokrywa się z automatycznym zapisem. Najpierw usuń wpis:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>Ścieżka pokrywa się z wpisem automatycznego zapisu</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>To wykluczenie rejestru jest wymagane, gdy lista automatycznego zapisu nie jest pusta</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>Ścieżka nie istnieje lub ma nieoczekiwany typ:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Nie można sprawdzić ścieżki:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Punktów ponownej analizy nie można dodawać do automatycznego zapisu:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Brak litery dysku uniemożliwia zapis przez UWF:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>Ścieżka jest na woluminie %1. Dodaj ją na karcie automatycznego zapisu tego woluminu.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>Nie można dodać katalogu głównego: spowodowałoby to trwały zapis całego woluminu.</translation></message>
    <message><source>File staging could not be saved</source><translation>Niepowodzenie: Zapis listy automatycznego zapisu</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>Lista została wczytana ponownie po nieudanej aktualizacji:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Automatyczny zapis plików: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Automatyczny trwały zapis przed bezpiecznym wyłączeniem lub restartem.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Automatyczny zapis plików: %1 · pliki: %2 · foldery: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>Automatyczny zapis plików wymaga tego wykluczenia rejestru, aby zachować listę po ponownym uruchomieniu.</translation></message>
    <message><source>Built-in</source><translation>Wbudowane</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>Filtr UWF jest wyłączony. Nakładka nie zgromadziła zmian do trwałego zapisu.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>System plików %1 nie obsługuje trwałego zapisu pojedynczych plików.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Ten wolumin nie jest obsługiwany przez UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Wolumin nie obsługuje trwałego zapisu plików. Dostępny tylko trwały zapis rejestru (wykluczenia są globalne).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>System plików %1 nie obsługuje trwałego zapisu pojedynczych plików. Dostępny tylko trwały zapis rejestru (wykluczenia są globalne).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Wolumin nie jest teraz chroniony przez UWF; brak zmian plików do trwałego zapisu.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Wolumin nie jest teraz chroniony przez UWF; brak zmian plików do trwałego zapisu. Dostępny tylko trwały zapis rejestru (wykluczenia są globalne).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Wybierz plik do trwałego zapisu na dysku</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Wybierz folder do trwałego zapisu na dysku</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Wybierz plik do trwałego usunięcia</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Wybierz folder do trwałego usunięcia</translation></message>
    <message><source>Name</source><translation>Nazwa</translation></message>
    <message><source>Data</source><translation>Dane</translation></message>
    <message><source>(Default)</source><translation>(Domyślna)</translation></message>
    <message><source>(value not set)</source><translation>(wartość nieustawiona)</translation></message>
    <message><source>(empty)</source><translation>(puste)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Nieznane (Wartość: Plik jest za duży)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Pokaż pliki nakładki. &lt;b&gt;Tylko migawka diagnostyczna.&lt;/b&gt; Nie wybieraj na jej podstawie plików do trwałego zapisu.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>Lista plików nakładki działa tylko na woluminach NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>UWF jest wyłączony; żaden wolumin nie ma nakładki do sprawdzenia.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>Wolumin nie jest chroniony przez UWF i nie ma nakładki do sprawdzenia.</translation></message>
    <message><source>Failed: %1</source><translation>Niepowodzenie: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>Dostawca WMI uległ awarii podczas wyliczania. To znana niestabilność UWF_Overlay.GetOverlayFiles przy dużej nakładce lub wysokim obciążeniu wejścia/wyjścia.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Brak pamięci lub operacja nieobsługiwana. Wymagany NTFS i zapas pamięci. Spróbuj z mniejszą nakładką lub zamknij programy zużywające dużo pamięci.</translation></message>
    <message><source>Raw error: %1</source><translation>Kod błędu: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Eksportuj do pliku… · Nakładka · pliki</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Pliki tekstowe (*.txt);;Wszystkie pliki (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Pliki poleceń (*.cmd);;Wszystkie pliki (*)</translation></message>
    <message><source>Export failed</source><translation>Eksport nieudany</translation></message>
    <message><source>Export finished</source><translation>Eksport zakończony</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Niepowodzenie: Otwarcie pliku (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Niepowodzenie: Zapis pliku (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Konfiguracja: Zapis pliku · wpisy: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Nie można wykluczyć całego woluminu (%1).</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Nie można wykluczyć plików stronicowania, wymiany i hibernacji — UWF jest od nich zależny.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Nie można wykluczyć całego katalogu: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Nie można wykluczyć całego katalogu: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Nie można wykluczyć całego katalogu: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Nie można wykluczyć krytycznego pliku systemowego: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>Nie można wykluczyć pliku rejestru użytkownika NTUSER.DAT.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>Klucz rejestru nie może być pusty.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>Klucz rejestru nie może zaczynać się od ukośnika odwrotnego.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>Ścieżka zawiera kolejne ukośniki odwrotne i jest nieprawidłowa.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Separatorem ścieżki rejestru jest `\`, nie `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>Ścieżka zawiera niewidoczne znaki sterujące i jest nieprawidłowa.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>Dokumentacja UWF zabrania wykluczania sekretu konta komputera domenowego: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Można wykluczać tylko określone podklucze następujących kluczy głównych:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Dodaj pliki lub foldery do wykluczeń. UWF nie będzie ich chronić.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Wybierz plik do dodania · Wykluczenia plików.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Wybierz folder do dodania · Wykluczenia plików. Obejmuje wszystkie podfoldery.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Wpisz ścieżkę klucza rejestru do wykluczenia.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Wpisz ścieżkę klucza rejestru do wykluczenia. / Trwałe przechowywanie.</translation></message>
    <message><source>Registry key…</source><translation>Rejestr · Klucz…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Klucz tajny domeny (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Licencja dostępu klienta usług terminalowych (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Zachowaj tajny klucz domeny (hasło konta komputera) między sesjami UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Zachowaj licencje dostępu klienta usług terminalowych między sesjami UWF.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Trwałe przechowywanie %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Usuń zaznaczone</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Usuń zaznaczone wykluczenia. Zadziała po zastosowaniu.</translation></message>
    <message><source>Open containing folder</source><translation>Otwórz folder nadrzędny</translation></message>
    <message><source>Copy file path</source><translation>Kopiuj ścieżkę pliku</translation></message>
    <message><source>Copy registry path</source><translation>Kopiuj ścieżkę rejestru</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Zapisz na dysku zmiany tego folderu z nakładki…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Zapisz na dysku zmiany tego pliku z nakładki…</translation></message>
    <message><source>Copied to clipboard</source><translation>Skopiowano do schowka</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Wybierz plik do dodania · Wykluczenia plików. Można wybrać kilka plików.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Wybierz folder do dodania · Wykluczenia plików</translation></message>
    <message><source>Add registry exclusion</source><translation>Dodanie wykluczenia rejestru</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>Ścieżka %1 nie jest na woluminie %2 i nie można jej dodać do jego wykluczeń.</translation></message>
    <message><source>Path is not on this volume</source><translation>Ścieżka nie jest na tym woluminie</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Nie można dodać wykluczenia</translation></message>
    <message><source>File: %1</source><translation>Plik… %1</translation></message>
    <message><source>Folder: %1</source><translation>Folder… %1</translation></message>
    <message><source>Registry: %1</source><translation>Rejestr: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Domyślnie wykluczone; można to zmienić.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Bieżąca sesja: %1
Następna sesja: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Oczekujące zmiany: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Menedżer ujednoliconego filtra zapisu (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Odczytaj ponownie stan bieżącej i konfigurację następnej sesji UWF.</translation></message>
    <message><source>Review and apply</source><translation>Sprawdź i zastosuj</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Sprawdź wszystkie oczekujące zmiany i zastosuj je zbiorczo. Większość zmian zacznie obowiązywać po następnym restarcie.</translation></message>
    <message><source>Safe shutdown</source><translation>Bezpieczne wyłączenie</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>System zostanie bezpiecznie wyłączony przez UWF. Dostępne nawet przy pełnej nakładce UWF.</translation></message>
    <message><source>Safe restart</source><translation>Bezpieczny restart</translation></message>
    <message><source>Direct restart</source><translation>Bezpośredni restart</translation></message>
    <message><source>Direct shutdown</source><translation>Bezpośrednie wyłączenie</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>System zostanie bezpiecznie uruchomiony ponownie przez UWF. Dostępne nawet przy pełnej nakładce UWF.</translation></message>
    <message><source>Log</source><translation>Dziennik</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Pokaż wewnętrzny dziennik bieżącej sesji do diagnozowania problemów.</translation></message>
    <message><source>About</source><translation>O programie</translation></message>
    <message><source>Switch display language</source><translation>Zmień język interfejsu</translation></message>
    <message><source>Language</source><translation>Język</translation></message>
    <message><source>Theme</source><translation>Motyw</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Przełącz jasny/ciemny motyw</translation></message>
    <message><source>System information</source><translation>Informacje o systemie</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Migawka diagnostyczna</translation></message>
    <message><source>Read only</source><translation>Tylko do odczytu</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Nie znaleziono celu; w nakładce nie ma nic do trwałego zapisu.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>Cel istnieje tylko w nakładce: utworzony pod UWF, nigdy niezapisany na dysku lub w trwałym rejestrze. Zniknie po restarcie; trwały zapis usunięcia jest zbędny i niemożliwy.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>System odrzucił parametr (nieprawidłowa ścieżka lub argument).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>Operacja nieudana. Cel może być używany przez inny proces, np. folder w Eksploratorze lub otwarty plik. Zamknij korzystające z niego programy i ponów próbę.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>Operacja nieudana; szczegóły w dzienniku.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operacja odrzucona (kod %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Nieznane.</translation></message>
    <message><source>Commit canceled</source><translation>Zapis anulowany</translation></message>

    <message><source>Succeeded</source><translation>Powodzenie</translation></message>
    <message><source>Skipped</source><translation>Pominięte</translation></message>
    <message><source>Failed</source><translation>Niepowodzenie</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Powodzenie: %1; Pominięte: %2; Niepowodzenie: %3.</translation></message>

    <message><source>Existed before</source><translation>Istniał przed</translation></message>
    <message><source>Exists after</source><translation>Istnieje po</translation></message>
    <message><source>Error code</source><translation>Kod błędu</translation></message>
    <message><source>Reason</source><translation>Przyczyna</translation></message>
    <message><source>Previous page</source><translation>Poprzednia strona</translation></message>
    <message><source>First page</source><translation>Pierwsza strona</translation></message>
    <message><source>Last page</source><translation>Ostatnia strona</translation></message>
    <message><source>Next page</source><translation>Następna strona</translation></message>
    <message><source>Copy selected rows</source><translation>Kopiuj zaznaczone wiersze</translation></message>
    <message><source>Copy all</source><translation>Kopiuj wszystko</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>To wykluczenie rejestru jest wymagane, gdy lista automatycznego zapisu nie jest pusta. Automatyczny zapis plików: Wyczyść → Usunięcie wykluczenia rejestru.</translation></message>
    <message><source>Copy current page</source><translation>Kopiuj bieżącą stronę</translation></message>
    <message><source>No pending changes</source><translation>Brak oczekujących zmian</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Wykluczenia rejestru · Ustawienia globalne.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Wolumin %1: Ochrona UWF / Wykluczenia plików / Automatyczny zapis plików.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Nieobsługiwane</translation></message>
    <message><source>Review and apply changes</source><translation>Sprawdź i zastosuj zmiany</translation></message>
    <message><source>Confirm apply</source><translation>Potwierdź zastosowanie</translation></message>
    <message><source>Result</source><translation>Wynik</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filtr: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Ustawienie progu ostrzeżenia: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Ustawienie progu krytycznego: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Ustawienie typu nakładki / Ustawienie maksymalnego rozmiaru: Aby zmienić typ i maksymalny rozmiar:
1. Wyłącz filtr powyżej.
2. Uruchom ponownie, aby filtr był wyłączony.
3. Zmień ustawienie.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Ustawienie typu nakładki: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Ustawienie maksymalnego rozmiaru: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Ustawienie maksymalnego rozmiaru: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Wolumin %1 · Ochrona UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Wolumin %1 · Powiąż przez: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Wolumin %1 · Dodanie wykluczenia pliku: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Wolumin %1 · Usunięcie wykluczenia pliku: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Dodanie wykluczenia rejestru: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Usunięcie wykluczenia rejestru: %1</translation></message>
    <message><source>Time</source><translation>Czas</translation></message>
    <message><source>Level</source><translation>Poziom</translation></message>
    <message><source>Tag</source><translation>Tag</translation></message>
    <message><source>Message</source><translation>Komunikat</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Obliczanie plików do automatycznego zapisu…</translation></message>
    <message><source>Committing staged files</source><translation>Automatyczne zapisywanie plików</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>Przetworzono %1 z %2 plików.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Brak dodatkowych informacji o błędzie.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>Automatyczny zapis nieoczekiwanie przerwany:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Automatyczny zapis nie został ukończony: Nieznany błąd.</translation></message>
    <message><source>File staging will be skipped</source><translation>Automatyczny zapis zostanie pominięty</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Bezpośredni restart: Automatyczny zapis zostanie pominięty.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Bezpośrednie wyłączenie: Automatyczny zapis zostanie pominięty.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Pominięto automatyczny zapis, bo wybrano bezpośrednią operację zasilania.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Bezpieczne wyłączenie: Niepowodzenie</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Wyłącz: Niepowodzenie (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Bezpieczny restart: Niepowodzenie</translation></message>
    <message><source>Direct restart failed</source><translation>Bezpośredni restart: Niepowodzenie</translation></message>
    <message><source>Direct shutdown failed</source><translation>Bezpośrednie wyłączenie: Niepowodzenie</translation></message>
    <message><source>Restart failed: %1</source><translation>Uruchom ponownie: Niepowodzenie (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Nie można odczytać listy automatycznego zapisu:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Nie można odczytać listy automatycznego zapisu: Nieznany błąd.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>pliki: %1 · Powodzenie: %2 · Pominięte (pliki): %3 · Pominięte (wpisy): %4 · Niepowodzenie: %5</translation></message>
    <message><source>File staging preparation</source><translation>Automatyczny zapis plików: Szczegóły operacji</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Punkty ponownej analizy nie są obsługiwane.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Można dodawać tylko ścieżki bezwzględne na lokalnych woluminach.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Można dodawać tylko ścieżki bezwzględne na lokalnych woluminach.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Nie można sprawdzić ścieżki automatycznego zapisu.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>Ścieżka dodanego pliku wskazuje teraz obiekt innego typu.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>Ścieżka dodanego folderu wskazuje teraz obiekt innego typu.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Nie można wyliczyć całej zawartości dodanego folderu.</translation></message>
    <message><source>The path has no drive letter.</source><translation>Ścieżka nie ma litery dysku.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>Nie można przetwarzać rekurencyjnie katalogu głównego woluminu.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>Dostawca UWF odrzucił trwały zapis.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Zapis nieudany: Nieznany błąd.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Dodatkowe błędy zapisane w dzienniku: %1.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>Ścieżka nie ma litery dysku. Identyfikacja woluminu docelowego: Niepowodzenie.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Brak rekordu bieżącej sesji dla woluminu %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>Ścieżka znajduje się na liście wykluczeń plików.
Wykluczenie: %1</translation></message>

    <message><source>Continue</source><translation>Kontynuuj</translation></message>
    <message><source>Delete and commit</source><translation>Usuń i zapisz trwale</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>Klucz znajduje się na liście wykluczeń rejestru.
Wykluczenie: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Brak rekordu filtra rejestru dla bieżącej sesji.</translation></message>

    <message><source>Export commands to file</source><translation>Eksportuj polecenia do pliku</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Eksportuj do pliku… · Polecenie: %1
%2</translation></message>
    <message><source>Import</source><translation>Importuj</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Wpisz lub wklej jedno polecenie na wiersz. Wczytaj tekst, dziennik lub skrypt (.txt, .bat, .ps1); wiersze z tym poleceniem zostaną dodane. Import przygotowuje zmiany. Dopiero „Sprawdź i zastosuj” na pasku narzędzi zapisuje je w systemie.</translation></message>
    <message><source>Load from file…</source><translation>Wczytaj z pliku…</translation></message>
    <message><source>Load recommended configuration</source><translation>Wczytaj zalecaną konfigurację</translation></message>
    <message><source>Choose recommended configuration</source><translation>Wybierz zalecaną konfigurację</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Wybierz grupy zalecanych ustawień. Przed importem możesz sprawdzić lub usunąć poszczególne polecenia.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Z oficjalnej dokumentacji Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Wykluczenia UWF zalecane przez Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Sprawdź przed importem: foldery muszą istnieć, aby UWF zaakceptował wykluczenia.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Źródło: oficjalna dokumentacja Microsoft UWF o typowych wykluczeniach i obsłudze ochrony antymalware.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Program poprawy jakości obsługi klienta (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Zachowaj trwale: Stan zgody w zasadach</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Zachowaj trwale: Lokalny stan zgody</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Zachowaj trwale: Flaga wyłączenia wysyłania</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Usługa inteligentnego transferu w tle (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Zachowaj trwale: Pliki kolejki pobierania</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Zachowaj trwale: Indeks stanu transferu</translation></message>
    <message><source>Network profiles and policies</source><translation>Profile i zasady sieciowe</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Zasady grupy sieci bezprzewodowej</translation></message>
    <message><source>Wired network GPO policy</source><translation>Zasady grupy sieci przewodowej</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Zasady grupy sieci bezprzewodowej · pliki</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Zasady grupy sieci przewodowej · pliki</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Profile interfejsów bezprzewodowych</translation></message>
    <message><source>Wired network interface profiles</source><translation>Profile interfejsów przewodowych</translation></message>
    <message><source>Wireless service configuration</source><translation>Konfiguracja usługi bezprzewodowej</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Konfiguracja usługi szerokopasmowej sieci komórkowej</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Konfiguracja usługi autokonfiguracji sieci przewodowej</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>Profile XML zależą od urządzenia. W razie potrzeby dodaj ręcznie konkretne ścieżki Interfaces\{GUID}\{GUID}.xml.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Czas letni (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Zachowaj trwale: Definicje stref czasowych</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Zachowaj trwale: Wybrana strefa czasowa</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Zachowaj trwale: Pliki produktu i aktualizacje</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Zachowaj trwale: Sygnatury i stan w ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Zachowaj trwale: Windows Update · Dziennik</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Zachowaj trwale: MpCmdRun · Dziennik</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Zachowaj trwale: Stan produktu w rejestrze</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Zachowaj trwale: WdBoot · Stan usługi</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Zachowaj trwale: WdFilter · Stan usługi</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Zachowaj trwale: WdNisSvc · Stan usługi</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Zachowaj trwale: WdNisDrv · Stan usługi</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Zachowaj trwale: WinDefend · Stan usługi</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Zachowaj trwale: Pliki programu klienckiego</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Zachowaj trwale: Windows Update · Dziennik</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Zachowaj trwale: MpCmdRun · Dziennik</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Zachowaj trwale: Sygnatury i stan ochrony antymalware</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Zachowaj trwale: Stan ochrony antymalware w rejestrze</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Wybierz pliki zawierające polecenia uwfmgr</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Pliki tekstowe (*.txt *.cmd);;Wszystkie pliki (*)</translation></message>
    <message><source>Status</source><translation>Stan</translation></message>
    <message><source>Import failed</source><translation>Import nieudany</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Błąd wewnętrzny: brak zarejestrowanego modułu stosowania zmian.</translation></message>
    <message><source>Nothing to import</source><translation>Brak poleceń do importu</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Nie znaleziono poleceń uwfmgr w danych wejściowych.</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplikat: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Nieobsługiwane: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Partia %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Duplikat</translation></message>
    <message><source>Unsupported</source><translation>Nieobsługiwane</translation></message>
    <message><source>File too large</source><translation>Plik jest za duży</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>Plik %1 przekracza 5 MB i nie został przeanalizowany. Najpierw odfiltruj go ręcznie.</translation></message>
    <message><source>Cannot read file</source><translation>Nie można odczytać pliku</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Niepowodzenie: Otwarcie pliku (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Odrzucić oczekujące zmiany?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Niezastosowane zmiany: %1.

Odrzucić i kontynuować?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Niepełne polecenie uwfmgr</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Brak argumentu: Rozmiar (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>Rozmiar musi być nieujemną liczbą całkowitą w MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Brak argumentu: Nakładka · Typ (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Nieprawidłowa wartość: Nakładka · Typ: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Brak argumentu: Wolumin (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>Wolumin musi być literą dysku, np. C:</translation></message>
    <message><source>Missing path argument</source><translation>Brak argumentu: Ścieżka</translation></message>
    <message><source>Missing registry key argument</source><translation>Brak argumentu: Rejestr · Klucz</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Niezamknięty cudzysłów w poleceniu</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Nieoczekiwany dodatkowy argument: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Nieobsługiwane polecenie uwfmgr (brak odpowiedniej operacji interfejsu)</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Stan docelowy już ustawiony; brak operacji</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>Ścieżka jest poza woluminem lub wolumin nie obsługuje wykluczeń (np. exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Odrzucone przez listę blokad UWF (pliki systemowe, Windows, plik stronicowania itd.)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Nieprawidłowa wartość: Rozmiar: %1</translation></message>
    <message><source>maximum size</source><translation>Maksymalny rozmiar</translation></message>
    <message><source>warning threshold</source><translation>Próg ostrzeżenia</translation></message>
    <message><source>critical threshold</source><translation>Próg krytyczny</translation></message>
    <message><source>Global configuration is unavailable; the command was not queued.</source><translation>Konfiguracja globalna jest niedostępna; polecenie nie zostało dodane do oczekujących zmian.</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Nieznany wolumin %1 (brak dysku obsługiwanego przez UWF o tej literze)</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Wolumin %1: Stan docelowy już ustawiony; brak operacji</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: Ścieżka nie ma litery dysku. Identyfikacja woluminu docelowego: Niepowodzenie.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Wolumin %1: UWF Nieobsługiwane</translation></message>
    <message><source>file exclusion</source><translation>Wykluczenia plików</translation></message>
    <message><source>registry exclusion</source><translation>Wykluczenia rejestru</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>Brak karty rejestru; wykluczenia rejestru są niedostępne</translation></message>
    <message><source>Unsupported command</source><translation>Polecenie: Nieobsługiwane</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Pokaż lub ukryj.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Włączone</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Wyłączone</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Nieznane</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Zajęte %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Tryb rozszerzony</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Powiąż automatyczny zapis z zamykaniem i restartem Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Niepowodzenie: Odczyt stanu trybu rozszerzonego (Nieznany błąd).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Niepowodzenie: Odczyt stanu usługi:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Niepowodzenie: Odczyt stanu usługi (Nieznany błąd).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>Usługa nie jest skonfigurowana jako osobny proces.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>Usługa nie jest skonfigurowana do automatycznego startu.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>Usługa nie jest skonfigurowana do pracy jako LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>Usługa nie jest uruchomiona.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>Ścieżka lub argumenty usługi nie odpowiadają bieżącemu programowi.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>Nieprawidłowy limit czasu wstępnego zamykania usługi.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>Usługa nie deklaruje uprawnień do uruchomienia agenta interfejsu.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>Usługa nie przyjęła powiadomień Windows o wstępnym zamykaniu.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Włącz tryb rozszerzony</translation></message>
    <message><source>Disable enhanced mode</source><translation>Wyłącz tryb rozszerzony</translation></message>
    <message><source>Start service</source><translation>Uruchom usługę</translation></message>
    <message><source>Repair enhanced mode</source><translation>Napraw tryb rozszerzony</translation></message>
    <message><source>Delete service</source><translation>Usuń usługę</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Zatrzymaj i usuń usługę trybu rozszerzonego wraz z pozostałymi danymi rejestru.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>Usługi nie ma, ale pozostały jej dane w rejestrze.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Oczekiwanie na uwierzytelnienie agenta interfejsu.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>Usługa działa, ale nie ma połączonego uwierzytelnionego agenta interfejsu.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Usługa pomocnicza trybu rozszerzonego UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Tryb rozszerzony · Konfiguracja: Zastosowano.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Konfiguracja: Zastosowano. Komunikat trwałego zapisu UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Tryb rozszerzony · Konfiguracja: Niepowodzenie:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Tryb rozszerzony · Konfiguracja: Niepowodzenie (Nieznany błąd).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>Usługa trybu rozszerzonego i pozostała konfiguracja zostały usunięte.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>Zażądano usunięcia usługi, ale czyszczenie pozostałości zgłosiło:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Tryb rozszerzony: Usuń usługę: Niepowodzenie:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Tryb rozszerzony: Usuń usługę: Niepowodzenie (Nieznany błąd).</translation></message>
    <message><source>File staging completed</source><translation>Automatyczny zapis plików: Powodzenie</translation></message>
    <message><source>File staging failed</source><translation>Automatyczny zapis plików: Niepowodzenie</translation></message>
    <message><source>All staged targets were processed.</source><translation>Przetworzono wszystkie przygotowane cele.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Przetworzono wszystkie cele; co najmniej jedna operacja się nie udała.</translation></message>
    <message><source>Show main window</source><translation>Pokaż okno główne</translation></message>
    <message><source>Restore default position</source><translation>Przywróć pozycję domyślną</translation></message>
    <message><source>Show overlay hub</source><translation>Pokaż HUB nakładki</translation></message>
    <message><source>Hide overlay hub</source><translation>Ukryj HUB nakładki</translation></message>
    <message><source>Exit application</source><translation>Zamknij aplikację</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Niepowodzenie: Połączenie z systemem</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Niepowodzenie: Odczyt wartości rejestru (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Wyliczanie plików nakładki: Nieznany błąd.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Niepowodzenie: Operacja filtra: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Niepowodzenie: Odczyt stanu filtra</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Niepowodzenie: Ustawienie progu ostrzeżenia</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Niepowodzenie: Ustawienie progu krytycznego</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Niepowodzenie: Odczyt stanu nakładki</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Niepowodzenie: Ustawienie typu nakładki</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Niepowodzenie: Ustawienie maksymalnego rozmiaru</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Niepowodzenie: Odczyt konfiguracji nakładki</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Niepowodzenie: Rejestracja woluminu %1 w UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Niepowodzenie: Ochrona woluminu %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Niepowodzenie: Ustawienie powiązania woluminu %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Wolumin %1 · Niepowodzenie: Dodanie wykluczenia pliku %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Wykluczenie pliku na woluminie %1 już nie istnieje: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Wolumin %1 · Niepowodzenie: Usunięcie wykluczenia pliku %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Niepowodzenie: Odczyt konfiguracji woluminu</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Niepowodzenie: Dodanie wykluczenia rejestru %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Niepowodzenie: Usunięcie wykluczenia rejestru %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Niepowodzenie: Zmiana przełączników trwałości rejestru</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Niepowodzenie: Odczyt filtra rejestru</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Nieobsługiwane</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Odczyt stanu celu: Nieznany błąd.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>Ścieżka nie istnieje; nie ma czego usuwać.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Nieznany błąd.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Nie można potwierdzić wyniku: ponowny odczyt stanu celu nie powiódł się: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>Dostawca przyjął usunięcie, ale ponowny miarodajny odczyt potwierdził, że cel nadal istnieje.</translation></message>
    <message><source>Unknown</source><translation>Nieznane</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Niepowodzenie: Wczytywanie dziennika (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Niepowodzenie: Wczytywanie dziennika (Nieznany błąd).</translation></message>
    <message><source>: unknown error</source><translation>: Nieznany błąd</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Niepowodzenie: Identyfikacja woluminu docelowego (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Niepowodzenie: Wyliczanie kluczy rejestru (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Niepowodzenie: Identyfikacja woluminu docelowego (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Pominięto operację, ponieważ wymagana wcześniejsza operacja zakończyła się niepowodzeniem lub nie można było jej potwierdzić.</translation></message>
    <message><source>Copy</source><translation>Skopiuj</translation></message>
    <message><source>Select All</source><translation>Zaznacz wszystko</translation></message>
    <message><source>Display language</source><translation>Język wyświetlania</translation></message>
    <message><source>Folder</source><translation>Folder</translation></message>
    <message><source>File</source><translation>Plik</translation></message>
    <message><source>Export commands</source><translation>Eksportuj polecenia</translation></message>
    <message><source>Direct restart…</source><translation>Bezpośredni restart…</translation></message>
    <message><source>Clear…</source><translation>Wyczyść…</translation></message>
    <message><source>Committing</source><translation>Zapisywanie</translation></message>
    <message><source> · %1 selected</source><translation> · Wybrano %1</translation></message>
    <message><source>%1 changes</source><translation>%1 zmian</translation></message>
    <message><source>%1 commands</source><translation>%1 poleceń</translation></message>
    <message><source>%1 items</source><translation>%1 elementów</translation></message>
    <message><source>%1 items · %2</source><translation>%1 elementów · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 oczekujących zmian</translation></message>
    <message><source>%1 records</source><translation>%1 wpisów</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 wpisów · łącznie %2</translation></message>


    <message><source>%1 selected</source><translation>Wybrano %1</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 plików</translation></message>
    <message><source>32-bit</source><translation>32 bity</translation></message>
    <message><source>64-bit</source><translation>64 bity</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Program jest udostępniany na licencji &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Pełne warunki znajdują się w dołączonym pliku LICENSE.&lt;/p&gt;&lt;p&gt;Oprogramowanie można rozpowszechniać i modyfikować na warunkach GPL v3. Jest ono dostarczane w stanie takim, w jakim jest, bez jakiejkolwiek gwarancji.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;Windows wyłącza pliki stronicowania, Przywracanie systemu, SuperFetch, indeksowanie plików, defragmentację dysku i szybkie uruchamianie. Ustawia też bootstatuspolicy na ignoreallfailures w BCD.&lt;/p&gt;&lt;p&gt;Te ustawienia można później zmienić. Na przykład przed ponownym włączeniem stronicowania można przenieść plik stronicowania na niechroniony wolumin.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Migawka diagnostyczna systemu, uprawnień i UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>W tej lokalizacji istnieje już plik:
%1

Zapisanie zastąpi jego zawartość.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Graficzny menedżer Windows Unified Write Filter.
Zarządzaj ochroną woluminów, nakładką, wykluczeniami i plikami do zapisu.</translation></message>
    <message><source>About this snapshot</source><translation>O tej migawce</translation></message>
    <message><source>Account</source><translation>Konto</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Dodaj konfigurację UWF ze skryptu lub tekstu.</translation></message>
    <message><source>Add exclusion</source><translation>Dodaj wykluczenie</translation></message>
    <message><source>Add files or folders</source><translation>Dodaj pliki lub foldery</translation></message>
    <message><source>Add selected items</source><translation>Dodaj wybrane elementy</translation></message>
    <message><source>Added</source><translation>Dodano</translation></message>
    <message><source>Administrator</source><translation>Administrator</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Wszystkie zmiany zapisano na dysku. Zostaną zachowane bez ponownego uruchamiania.</translation></message>
    <message><source>All files (*)</source><translation>Wszystkie pliki (*)</translation></message>
    <message><source>All levels</source><translation>Wszystkie poziomy</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Pusta migawka diagnostyczna nie oznacza, że nakładka nie zawiera zmian.</translation></message>
    <message><source>Application path</source><translation>Ścieżka aplikacji</translation></message>
    <message><source>Application result</source><translation>Wynik zastosowania</translation></message>
    <message><source>Apply changes</source><translation>Zastosuj zmiany</translation></message>
    <message><source>Architecture</source><translation>Architektura</translation></message>
    <message><source>Automatic</source><translation>Automatycznie</translation></message>
    <message><source>Automatic file staging</source><translation>Automatyczny zapis przygotowanych plików</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Automatyczny zapis nieukończony</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Automatycznie zapisuj odpowiednie przygotowane pliki podczas zwykłego wyłączania i ponownego uruchamiania.</translation></message>
    <message><source>Available</source><translation>Dostępne</translation></message>
    <message><source>Back</source><translation>Wstecz</translation></message>
    <message><source>Background service</source><translation>Usługa w tle</translation></message>
    <message><source>Binding</source><translation>Powiązanie</translation></message>
    <message><source>By drive letter</source><translation>Według litery dysku</translation></message>
    <message><source>By volume ID</source><translation>Według identyfikatora woluminu</translation></message>
    <message><source>Cancel reading</source><translation>Anuluj odczyt</translation></message>
    <message><source>Changes already committed are retained.</source><translation>Utrwalone zmiany zostają zachowane.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Wybierz klucz rejestru, który ma zostać wykluczony z ochrony UWF, wraz z jego podkluczami i wartościami.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Wybierz lokalizację zapisu pliku.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Wybierz lokalizację w dozwolonym dysku lub folderze.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Wybierz inny folder lub sprawdź uprawnienia dostępu i ponów próbę.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Wybierz inny klucz lub sprawdź uprawnienia dostępu i ponów próbę.</translation></message>
    <message><source>Clear filters</source><translation>Wyczyść filtry</translation></message>
    <message><source>Clear logs</source><translation>Wyczyść dzienniki</translation></message>
    <message><source>Clear logs?</source><translation>Wyczyścić dzienniki?</translation></message>
    <message><source>Collapse</source><translation>Zwiń</translation></message>
    <message><source>Commit changes</source><translation>Utrwal zmiany</translation></message>
    <message><source>Commit changes…</source><translation>Utrwal zmiany…</translation></message>
    <message><source>Commit complete</source><translation>Utrwalanie zakończone</translation></message>
    <message><source>Commit file changes?</source><translation>Utrwalić zmiany pliku?</translation></message>
    <message><source>Commit folder changes?</source><translation>Utrwalić zmiany folderu?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Utrwalić zmiany klucza rejestru?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Utrwalić zmiany wartości rejestru?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Utrwala przygotowane pliki przed zwykłym wyłączeniem i ponownym uruchomieniem</translation></message>
    <message><source>Compatibility mode</source><translation>Tryb zgodności</translation></message>
    <message><source>Configuration applied</source><translation>Konfiguracja zastosowana</translation></message>
    <message><source>Configuration error</source><translation>Błąd konfiguracji</translation></message>
    <message><source>Configured</source><translation>Skonfigurowano</translation></message>
    <message><source>Confirm before continuing</source><translation>Potwierdź przed kontynuowaniem</translation></message>
    <message><source>Connected and authenticated</source><translation>Połączono i uwierzytelniono</translation></message>
    <message><source>Connecting…</source><translation>Łączenie…</translation></message>
    <message><source>Contact author</source><translation>Kontakt z autorem</translation></message>
    <message><source>Continue editing</source><translation>Kontynuuj edycję</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Kontynuowanie ponownego uruchamiania odrzuci zmiany w plikach, których nie udało się zapisać.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Kontynuowanie wyłączania odrzuci zmiany w plikach, których nie udało się zapisać.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Koordynuj z wyłączaniem Windows</translation></message>
    <message><source>Copy all logs</source><translation>Kopiuj wszystkie dzienniki</translation></message>
    <message><source>Copy full report</source><translation>Kopiuj pełny raport</translation></message>
    <message><source>Copy name</source><translation>Kopiuj nazwę</translation></message>
    <message><source>Copy path</source><translation>Kopiuj ścieżkę</translation></message>
    <message><source>Copy selection</source><translation>Kopiuj zaznaczenie</translation></message>
    <message><source>Copy version</source><translation>Kopiuj wersję</translation></message>
    <message><source>Counting staged files…</source><translation>Zliczanie plików do zapisu…</translation></message>
    <message><source>Create</source><translation>Utwórz</translation></message>
    <message><source>Critical %1 MB</source><translation>Próg krytyczny %1 MB</translation></message>
    <message><source>Current filter</source><translation>Bieżący filtr</translation></message>
    <message><source>Current run</source><translation>Bieżące uruchomienie</translation></message>
    <message><source>Current selection</source><translation>Bieżący wybór</translation></message>
    <message><source>Current usage</source><translation>Bieżące użycie</translation></message>
    <message><source>Date modified</source><translation>Data modyfikacji</translation></message>
    <message><source>Debug</source><translation>Debugowanie</translation></message>
    <message><source>Delete and commit this file?</source><translation>Usunąć ten plik i utrwalić usunięcie?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Usunąć ten folder i utrwalić usunięcie?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Usunąć ten klucz rejestru i utrwalić usunięcie?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Usunąć tę wartość rejestru i utrwalić usunięcie?</translation></message>

    <message><source>Direct restart?</source><translation>Uruchomić ponownie bezpośrednio?</translation></message>
    <message><source>Direct shutdown?</source><translation>Wyłączyć bezpośrednio?</translation></message>
    <message><source>Discard changes</source><translation>Odrzuć zmiany</translation></message>
    <message><source>Disk</source><translation>Dysk</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Trwałe przechowywanie tajnego klucza domeny</translation></message>
    <message><source>Done</source><translation>Gotowe</translation></message>
    <message><source>Edit commands</source><translation>Edytuj polecenia</translation></message>
    <message><source>Editable</source><translation>Edytowalne</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Włącz filtr i uruchom ponownie, aby wyświetlić bieżące użycie</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>Tryb rozszerzony dodaje usługę działającą w tle, która uruchamia się wraz z systemem Windows. Usuń tę usługę przed przeniesieniem lub usunięciem aplikacji.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>Tryb rozszerzony jest niedostępny. Użyj Bezpiecznego wyłączenia lub Bezpiecznego restartu, aby zapisać przygotowane pliki.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Wpisz HKLM\SOFTWARE\… i naciśnij Enter, aby przejść</translation></message>
    <message><source>Enter a file name</source><translation>Wprowadź nazwę pliku</translation></message>
    <message><source>Enter a valid file name.</source><translation>Wprowadź poprawną nazwę pliku.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Wprowadź poprawną nazwę pliku lub folderu.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Wprowadź poprawną nazwę folderu.</translation></message>
    <message><source>Error</source><translation>Błąd</translation></message>
    <message><source>Error details</source><translation>Szczegóły błędu</translation></message>
    <message><source>Error details and suggestions</source><translation>Szczegóły błędu i sugestie</translation></message>
    <message><source>Expand</source><translation>Rozwiń</translation></message>
    <message><source>Export list</source><translation>Eksportuj listę</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Nie udało się wyczyścić dzienników: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Nie udało się wyczyścić dzienników: nieoczekiwany błąd.</translation></message>
    <message><source>File name</source><translation>Nazwa pliku</translation></message>
    <message><source>File or folder</source><translation>Plik lub folder</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>Rozpoczęto zapisywanie plików. Nie można go anulować.</translation></message>
    <message><source>File saving incomplete</source><translation>Zapisywanie plików nieukończone</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>Zapisywanie przygotowanych plików zostanie pominięte. Nieutrwalone zmiany w chronionych ścieżkach zostaną utracone po ponownym uruchomieniu.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>Zapisywanie przygotowanych plików zostanie pominięte. Nieutrwalone zmiany w chronionych ścieżkach zostaną utracone po wyłączeniu.</translation></message>
    <message><source>File type</source><translation>Typ pliku</translation></message>
    <message><source>Files and folders</source><translation>Pliki i foldery</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Pliki i foldery można wybrać razem.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Pliki przygotowane do zapisu nie zostaną zapisane.</translation></message>
    <message><source>Filter is disabled</source><translation>Filtr jest wyłączony</translation></message>
    <message><source>Fit columns to contents</source><translation>Dopasuj kolumny do zawartości</translation></message>
    <message><source>Folder address</source><translation>Adres folderu</translation></message>
    <message><source>Folder name</source><translation>Nazwa folderu</translation></message>
    <message><source>Folders</source><translation>Foldery</translation></message>
    <message><source>Full configuration</source><translation>Pełna konfiguracja</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>Przyszłe zapisy do tego klucza nie będą chronione przez UWF.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>Przyszłe zapisy do tej ścieżki nie będą chronione przez UWF.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Globalne wykluczenie rejestru · obejmuje podklucze</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Przejdź do folderu nadrzędnego lub wybierz inną lokalizację.</translation></message>
    <message><source>Home</source><translation>Folder osobisty</translation></message>
    <message><source>Import configuration</source><translation>Importuj konfigurację</translation></message>
    <message><source>Import updates the configuration draft. Use Review and apply to write changes to the system.</source><translation>Import aktualizuje roboczą wersję konfiguracji. Użyj Sprawdź i zastosuj, aby zapisać zmiany w systemie.</translation></message>
    <message><source>Import results</source><translation>Wyniki importu</translation></message>
    <message><source>Imported</source><translation>Zaimportowano</translation></message>
    <message><source>The report lists command results for the configuration draft. Select a row to view the complete command and result.</source><translation>Raport zawiera wyniki poleceń dotyczących roboczej wersji konfiguracji. Wybierz wiersz, aby wyświetlić pełne polecenie i wynik.</translation></message>
    <message><source>Imported: %1</source><translation>Zaimportowano: %1</translation></message>
    <message><source>Information</source><translation>Informacja</translation></message>
    <message><source>Item details</source><translation>Szczegóły elementu</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Zapisywanie dużej listy przygotowanych plików może potrwać dłużej.</translation></message>
    <message><source>License and copyright</source><translation>Licencja i prawa autorskie</translation></message>
    <message><source>Line</source><translation>Wiersz</translation></message>
    <message><source>Local disk (%1)</source><translation>Dysk lokalny (%1)</translation></message>
    <message><source>Log level</source><translation>Poziom dziennika</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Nie udało się odczytać dzienników. Ponów próbę lub sprawdź dostęp do pliku dziennika.</translation></message>
    <message><source>Maximize</source><translation>Maksymalizuj</translation></message>
    <message><source>Message details</source><translation>Szczegóły komunikatu</translation></message>
    <message><source>Minimize</source><translation>Minimalizuj</translation></message>
    <message><source>Module</source><translation>Moduł</translation></message>
    <message><source>More</source><translation>Więcej</translation></message>
    <message><source>Move</source><translation>Przenieś</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Jeśli UWF chroni dysk, na którym znajduje się aplikacja, najpierw upewnij się, że jej pliki zostaną zachowane po ponownym uruchomieniu. Trwałe zapisywanie wielu przygotowanych plików może wydłużyć wyłączanie lub ponowne uruchamianie.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Można wybrać wiele plików.</translation></message>
    <message><source>Name and location</source><translation>Nazwa i lokalizacja</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Nazwana wartość: %1 · tylko ta wartość</translation></message>
    <message><source>Navigate to registry path</source><translation>Przejdź do ścieżki rejestru</translation></message>
    <message><source>New folder</source><translation>Nowy folder</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Nowe wpisy pojawią się po odświeżeniu.</translation></message>
    <message><source>Next-session configuration</source><translation>Konfiguracja następnej sesji</translation></message>
    <message><source>No files to commit</source><translation>Brak plików do utrwalenia</translation></message>
    <message><source>No logs yet</source><translation>Brak dzienników</translation></message>
    <message><source>No matching items</source><translation>Brak pasujących elementów</translation></message>
    <message><source>No matching logs</source><translation>Brak pasujących dzienników</translation></message>
    <message><source>No overlay files found</source><translation>Nie znaleziono plików nakładki</translation></message>
    <message><source>No registry key selected</source><translation>Nie wybrano klucza rejestru</translation></message>
    <message><source>None</source><translation>Brak</translation></message>
    <message><source>Not configured</source><translation>Nie skonfigurowano</translation></message>
    <message><source>Not connected</source><translation>Nie połączono</translation></message>
    <message><source>Not installed</source><translation>Nie zainstalowano</translation></message>
    <message><source>Not written to the system</source><translation>Nie zapisano w systemie</translation></message>
    <message><source>Off</source><translation>Wyłączone</translation></message>
    <message><source>On</source><translation>Włączone</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Jedno polecenie uwfmgr na wiersz</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Przechowuj aplikację w stałym folderze, którego standardowi użytkownicy nie mogą modyfikować. Sugerowana lokalizacja:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Tylko przygotowane pliki są zapisywane automatycznie. Inne nieutrwalone zmiany w chronionych ścieżkach zostaną utracone po ponownym uruchomieniu.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Tylko przygotowane pliki są zapisywane automatycznie. Inne nieutrwalone zmiany w chronionych ścieżkach zostaną utracone po wyłączeniu.</translation></message>
    <message><source>Open</source><translation>Otwórz</translation></message>
    <message><source>Open folder</source><translation>Otwórz folder</translation></message>
    <message><source>Operating system</source><translation>System operacyjny</translation></message>
    <message><source>Operation and security</source><translation>Działanie i bezpieczeństwo</translation></message>
    <message><source>Original command</source><translation>Oryginalne polecenie</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>Osiągnięto krytyczny próg nakładki. Zapisz zmiany i bezpiecznie uruchom ponownie.</translation></message>
    <message><source>Overlay maximum size</source><translation>Maksymalny rozmiar nakładki</translation></message>
    <message><source>Overlay type</source><translation>Typ nakładki</translation></message>
    <message><source>Overlay usage</source><translation>Użycie nakładki</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>Osiągnięto próg ostrzeżenia nakładki. Zapisz zmiany i bezpiecznie uruchom ponownie.</translation></message>
    <message><source>Overview</source><translation>Przegląd</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Wklej polecenia lub wczytaj je z pliku…</translation></message>
    <message><source>Pending changes</source><translation>Oczekujące zmiany</translation></message>
    <message><source>Pending removal</source><translation>Oczekuje na usunięcie</translation></message>
    <message><source>Permissions and protection</source><translation>Uprawnienia i ochrona</translation></message>
    <message><source>Privileges</source><translation>Przywileje</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>Przetworzono %1 / %2 elementów</translation></message>
    <message><source>Processing result</source><translation>Wynik przetwarzania</translation></message>
    <message><source>Protected</source><translation>Chroniony</translation></message>
    <message><source>Protected volumes</source><translation>Chronione woluminy</translation></message>
    <message><source>Protection and overlay</source><translation>Ochrona i nakładka</translation></message>
    <message><source>Quick access</source><translation>Szybki dostęp</translation></message>
    <message><source>Raw report</source><translation>Raport surowy</translation></message>
    <message><source>Read again</source><translation>Odczytaj ponownie</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Odczytaj ponownie, aby pobrać migawkę diagnostyczną.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Odczytaj konfigurację, aby wyświetlić bieżące użycie</translation></message>
    <message><source>Read-only</source><translation>Tylko do odczytu</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Odczyt dużej nakładki może potrwać dłużej. Można go anulować w dowolnym momencie.</translation></message>
    <message><source>Reading canceled</source><translation>Odczyt anulowany</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Odczytywanie plików i podfolderów w tej lokalizacji…</translation></message>
    <message><source>Reading folder</source><translation>Odczytywanie folderu</translation></message>
    <message><source>Reading logs</source><translation>Odczytywanie dzienników</translation></message>
    <message><source>Reading overlay</source><translation>Odczytywanie nakładki</translation></message>
    <message><source>Ready</source><translation>Gotowość</translation></message>
    <message><source>Ready to save</source><translation>Gotowe do zapisu</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Po zakończeniu odczytu pojawią się tu wpisy z bieżącego uruchomienia.</translation></message>
    <message><source>Recursive staging</source><translation>Przygotowanie z podfolderami</translation></message>
    <message><source>Refresh folder</source><translation>Odśwież folder</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Klucz rejestru lub ustawienie trwałego zapisu</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>Pozostałe zadania można anulować po zakończeniu bieżącego celu.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Usuń ten klucz z wykluczeń następnej sesji.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Usuń tę ścieżkę z wykluczeń następnej sesji.</translation></message>
    <message><source>Removed</source><translation>Usunięto</translation></message>
    <message><source>Rename</source><translation>Zmień nazwę</translation></message>
    <message><source>Rename…</source><translation>Zmień nazwę…</translation></message>
    <message><source>Repair required</source><translation>Wymagana naprawa</translation></message>
    <message><source>Replace file</source><translation>Zastąp plik</translation></message>
    <message><source>Replace the existing file?</source><translation>Zastąpić istniejący plik?</translation></message>
    <message><source>Required configuration</source><translation>Wymagana konfiguracja</translation></message>
    <message><source>Restart anyway</source><translation>Mimo to uruchom ponownie</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>Ponowne uruchamianie będzie kontynuowane po zapisaniu. Pozostaw aplikację otwartą.</translation></message>
    <message><source>Restore</source><translation>Przywróć</translation></message>
    <message><source>Retry</source><translation>Ponów próbę</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Przejrzyj szczegóły błędów przed kontynuowaniem wyłączania.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Przejrzyj szczegóły błędów przed ponownym uruchomieniem.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Przejrzyj szczegóły operacji. Odczytaj bieżący stan systemu przed ponowieniem zmian, które mogły już zostać zapisane.</translation></message>
    <message><source>Review the result for each target.</source><translation>Sprawdź wynik każdego celu.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Sprawdź te zmiany przed zapisaniem ich w systemie.</translation></message>
    <message><source>Running</source><translation>Uruchomiono</translation></message>
    <message><source>Save</source><translation>Zapisz</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Zapisz przygotowane pliki na dysku i uruchom ponownie.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Zapisz przygotowane pliki na dysku i wyłącz komputer.</translation></message>
    <message><source>Saving staged files</source><translation>Zapisywanie przygotowanych plików</translation></message>
    <message><source>Saving to disk</source><translation>Zapisywanie na dysku</translation></message>
    <message><source>Saving…</source><translation>Zapisywanie…</translation></message>
    <message><source>Scanning staged paths</source><translation>Skanowanie przygotowanych ścieżek</translation></message>
    <message><source>Search logs</source><translation>Przeszukaj dzienniki</translation></message>
    <message><source>Search messages or modules</source><translation>Szukaj komunikatów lub modułów</translation></message>
    <message><source>Search paths</source><translation>Szukaj ścieżek</translation></message>
    <message><source>Search this folder</source><translation>Przeszukaj ten folder</translation></message>
    <message><source>Select a file or enter its name</source><translation>Wybierz plik lub wprowadź jego nazwę</translation></message>
    <message><source>Select a file to open.</source><translation>Wybierz plik do otwarcia.</translation></message>
    <message><source>Select a folder.</source><translation>Wybierz folder.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Wybierz klucz, aby wykonać operację na całym poddrzewie, lub nazwaną wartość, aby objąć tylko tę wartość.</translation></message>
    <message><source>Select an available subkey</source><translation>Wybierz dostępny podklucz</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Wybierz tylko pliki lub otwórz folder, aby przejrzeć jego zawartość.</translation></message>
    <message><source>Select files or folders</source><translation>Wybierz pliki lub foldery</translation></message>
    <message><source>Select folder</source><translation>Wybierz folder</translation></message>
    <message><source>Select folders only.</source><translation>Wybierz tylko foldery.</translation></message>
    <message><source>Select registry key or value</source><translation>Wybierz klucz lub wartość rejestru</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Wybierz klucz lub wartość rejestru do usunięcia</translation></message>
    <message><source>Select the current folder</source><translation>Wybierz bieżący folder</translation></message>
    <message><source>Select this file</source><translation>Wybierz ten plik</translation></message>
    <message><source>Select this registry key</source><translation>Wybierz ten klucz rejestru</translation></message>
    <message><source>Select this registry value</source><translation>Wybierz tę wartość rejestru</translation></message>
    <message><source>Selected file</source><translation>Wybrany plik</translation></message>
    <message><source>Selected folder</source><translation>Wybrany folder</translation></message>
    <message><source>Selection</source><translation>Wybór</translation></message>
    <message><source>Service account</source><translation>Konto usługi</translation></message>
    <message><source>Service configuration needs repair</source><translation>Konfiguracja usługi wymaga naprawy</translation></message>
    <message><source>Service details</source><translation>Szczegóły usługi</translation></message>
    <message><source>Service name</source><translation>Nazwa usługi</translation></message>
    <message><source>Service status</source><translation>Stan usługi</translation></message>
    <message><source>Service stopped</source><translation>Usługa zatrzymana</translation></message>
    <message><source>Shut down anyway</source><translation>Mimo to wyłącz</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>Wyłączanie będzie kontynuowane po zapisaniu. Pozostaw aplikację otwartą.</translation></message>
    <message><source>Size</source><translation>Rozmiar</translation></message>
    <message><source>Some changes could not be applied</source><translation>Nie udało się zastosować niektórych zmian</translation></message>
    <message><source>Some files could not be saved</source><translation>Nie udało się zapisać niektórych plików</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Nie udało się odczytać części informacji o sprzęcie. Pozostałe informacje diagnostyczne nadal można wyświetlić i skopiować.</translation></message>
    <message><source>Source code</source><translation>Kod źródłowy</translation></message>
    <message><source>Staged file</source><translation>Plik do zapisu</translation></message>
    <message><source>Staged path</source><translation>Ścieżka do zapisu</translation></message>
    <message><source>Standard user</source><translation>Użytkownik standardowy</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Uruchamia się automatycznie z Windows</translation></message>
    <message><source>Startup type</source><translation>Typ uruchomienia</translation></message>
    <message><source>Status unavailable</source><translation>Stan niedostępny</translation></message>
    <message><source>Stopped</source><translation>Zatrzymano</translation></message>
    <message><source>Storage</source><translation>Magazyn</translation></message>
    <message><source>System</source><translation>System</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Zmiany systemu przy pierwszym włączeniu UWF</translation></message>
    <message><source>System memory</source><translation>Pamięć systemowa</translation></message>
    <message><source>System metadata</source><translation>Metadane systemu</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Metadane systemu służą tylko do diagnozy i nie można ich utrwalić.</translation></message>
    <message><source>Takes effect after restart</source><translation>Działa po ponownym uruchomieniu</translation></message>
    <message><source>Target</source><translation>Cel</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Trwałe przechowywanie licencji Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>Dostawca UWF nie ukończył wyliczania. Może to wystąpić przy dużej nakładce lub braku zasobów systemowych.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>Konfigurację zapisano w systemie. Większość zmian zacznie działać po ponownym uruchomieniu.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>Konfiguracja odpowiada celowi następnej sesji. Wyświetl lub wyeksportuj polecenia w Pełnej konfiguracji.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Bieżące wpisy dziennika zostaną nieodwracalnie usunięte.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>Folder docelowy nie istnieje.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Nie udało się odczytać stanu usługi trybu rozszerzonego. Sprawdź uprawnienia usługi i ponów próbę.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>Lista może pomijać pliki lub zawierać pliki już utrwalone. Nie używaj jej jako listy do utrwalenia.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>Wybrany element już nie istnieje: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>Usługa jest zainstalowana, ale zatrzymana. Uruchom ją, aby przywrócić automatyczny zapis.</translation></message>
    <message><source>This PC</source><translation>Ten komputer</translation></message>
    <message><source>This folder is empty</source><translation>Ten folder jest pusty</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Raport może zawierać nazwy kont, identyfikatory urządzeń i ścieżki plików. Sprawdź go przed udostępnieniem.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Zapisz ten skrypt CMD w kodowaniu UTF-8 jako plik .cmd i uruchom go w wierszu polecenia. Eksport używa tego samego skryptu.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Spróbuj innego słowa kluczowego lub poziomu dziennika.</translation></message>
    <message><source>Try another search term.</source><translation>Spróbuj innego hasła wyszukiwania.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>Typu i maksymalnego rozmiaru nie można zmieniać przy włączonym filtrze. Najpierw wyłącz filtr i uruchom ponownie.</translation></message>
    <message><source>UI agent</source><translation>Agent interfejsu</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 wierszy</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>Nie udało się odczytać konfiguracji UWF. Sprawdź, czy Unified Write Filter jest zainstalowany w Windows, i odczytaj ponownie.</translation></message>
    <message><source>UWF feature</source><translation>Funkcja UWF</translation></message>
    <message><source>UWF filter</source><translation>Filtr UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>UWF jest obecnie niedostępny</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>Bezpieczny restart UWF zostanie wykonany bezpośrednio.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>Bezpieczne wyłączenie UWF zostanie wykonane bezpośrednio.</translation></message>
    <message><source>Unable to access this folder</source><translation>Nie można uzyskać dostępu do tego folderu</translation></message>
    <message><source>Unable to access this registry key</source><translation>Nie można uzyskać dostępu do tego klucza rejestru</translation></message>
    <message><source>Unable to read logs</source><translation>Nie można odczytać dzienników</translation></message>
    <message><source>Unable to read service status</source><translation>Nie można odczytać stanu usługi</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Nie można teraz odczytać nakładki</translation></message>
    <message><source>Unable to read this folder.</source><translation>Nie można odczytać tego folderu.</translation></message>
    <message><source>Unable to select this item</source><translation>Nie można wybrać tego elementu</translation></message>
    <message><source>Unavailable</source><translation>Niedostępne</translation></message>
    <message><source>Undo removal</source><translation>Cofnij usunięcie</translation></message>
    <message><source>Unprotected</source><translation>Niechroniony</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Elementy zakończone niepowodzeniem nie zostały zapisane. Wybierz wiersz, aby zobaczyć pełną ścieżkę i przyczynę.</translation></message>
    <message><source>Up one folder</source><translation>Folder nadrzędny</translation></message>
    <message><source>Usage unavailable</source><translation>Użycie niedostępne</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Użyj całego klucza, w tym wszystkich podkluczy</translation></message>
    <message><source>Version %1</source><translation>Wersja %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Wyświetl wpisy aplikacji i informacje diagnostyczne.</translation></message>
    <message><source>View commands for these changes</source><translation>Wyświetl polecenia dla tych zmian</translation></message>
    <message><source>View failure details</source><translation>Wyświetl szczegóły błędów</translation></message>
    <message><source>View overlay</source><translation>Wyświetl nakładkę</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Wyświetl migawkę diagnostyczną plików dla %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Powiązanie woluminu %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Ochrona woluminu %1</translation></message>
    <message><source>Volume identity</source><translation>Tożsamość woluminu</translation></message>
    <message><source>Warning</source><translation>Ostrzeżenie</translation></message>
    <message><source>Warning %1 MB</source><translation>Próg ostrzeżenia %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Cały klucz i wszystkie podklucze · rekurencyjnie</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>Windows UWF zapewnia ochronę przed zapisem. Ta aplikacja ją konfiguruje i zarządza nią.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Bez trybu rozszerzonego użyj Bezpiecznego wyłączenia lub Bezpiecznego restartu w tej aplikacji, aby zapisać przygotowane pliki.</translation></message>
    <message><source>Write protection</source><translation>Ochrona przed zapisem</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Zapisać te zmiany konfiguracji w systemie? Większość zacznie działać po ponownym uruchomieniu.</translation></message>
    <message><source>staged files</source><translation>pliki do zapisu</translation></message>
    <message><source>uwfmgr commands</source><translation>polecenia uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Oznacz wybrane ścieżki do usunięcia. Zmiana zacznie obowiązywać po zastosowaniu.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Usuń z listy automatycznego zapisywania plików po zastosowaniu.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Dodaj do listy automatycznego zapisywania plików po zastosowaniu.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Zastosowanie tego planu wyczyści zapisaną listę plików i usunie powiązane wykluczenie rejestru, gdy nie będzie już potrzebne.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>Zmiany listy plików należy zastosować w UWF Manager; nie można ich wyeksportować jako poleceń uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>Lista plików jest zapisywana po zastosowaniu</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Zakres migawki nakładki</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Ta lista służy wyłącznie do diagnostyki. Nie pozwala określić, które pliki trzeba zatwierdzić.</translation></message>
    <message><source>NTFS volumes only</source><translation>Tylko woluminy NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>Odczyt dużych nakładek może być powolny lub zakończyć się błędem z powodu braku zasobów.</translation></message>
    <message><source>Small files may be missing</source><translation>Może brakować małych plików</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Pliki mniejsze niż rozmiar klastra dysku (zwykle 4 KB) mogą nie być widoczne na liście.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Nie wskazuje zmian oczekujących na zatwierdzenie</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Mogą pojawić się również pliki już zatwierdzone, wykluczone ścieżki i elementy objęte innymi operacjami.</translation></message>

    <message><source>Command processing result</source><translation>Wynik przetwarzania polecenia</translation></message>
    <message><source>Copy command</source><translation>Kopiuj polecenie</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · Nie odczytano %1 gałęzi</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>Nie można odczytać %1 gałęzi. Kontynuuj ze znalezioną zawartością lub ponów tylko nieudane odczyty.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>Pliki: %1 · foldery: %2</translation></message>
    <message><source>%1 values · %2 keys</source><translation>Wartości: %1 · klucze: %2</translation></message>
    <message><source>Attempts</source><translation>Próby</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Anuluj pozostałe zadania</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Utrwal tylko poprawnie przejrzaną zawartość. Nieodczytane gałęzie pozostaną bez zmian.</translation></message>
    <message><source>Committing changes</source><translation>Utrwalanie zmian</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>Zakończonych usunięć i utrwaleń nie można cofnąć.</translation></message>
    <message><source>Continue…</source><translation>Kontynuuj…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Usuń i utrwal tylko znalezione obiekty. Nieodczytane gałęzie i ich foldery lub klucze nadrzędne zostaną zachowane.</translation></message>
    <message><source>Deleting and committing</source><translation>Usuwanie i utrwalanie</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Usuwania i utrwalania nie można cofnąć. Przetwarzany jest tylko wybrany zakres; oczekujące ustawienia UWF nie są stosowane.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>Ustalanie zakresu. Nieodczytane gałęzie są zapisywane w dzienniku, a przeglądanie pozostałej zawartości trwa.</translation></message>
    <message><source>Export traversal records</source><translation>Eksportuj dziennik przeglądania</translation></message>
    <message><source>No actionable content</source><translation>Brak zawartości do przetworzenia</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Nie znaleziono obiektów do przetworzenia. Niczego nie usunięto ani nie utrwalono.</translation></message>
    <message><source>No changes have been committed.</source><translation>Nie utrwalono jeszcze zmian</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Niczego nie usunięto ani nie utrwalono</translation></message>
    <message><source>Path</source><translation>Ścieżka</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Trwale usuń wybrany zakres i zapisz usunięcia na chronionym woluminie.</translation></message>
    <message><source>Preparation complete</source><translation>Przygotowanie zakończone</translation></message>
    <message><source>Preparing commit</source><translation>Przygotowywanie utrwalania</translation></message>
    <message><source>Preparing deletion and commit</source><translation>Przygotowywanie usuwania i utrwalania</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Przetwórz tylko poprawnie przejrzaną zawartość</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Punkty ponownej analizy są pomijane, aby przeglądanie nie wykraczało poza wybrany zakres.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Ponowne uruchomienie nie cofa utrwalania. Przetwarzany jest tylko wybrany zakres; oczekujące ustawienia UWF nie są stosowane.</translation></message>
    <message><source>Retry failed items</source><translation>Ponów nieudane odczyty</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Zapisz zmiany z wybranego zakresu na chronionym woluminie.</translation></message>
    <message><source>Stopping the operation</source><translation>Zatrzymywanie operacji</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Zapisy udanych i nieudanych odczytów zachowano</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>Folder nie jest pusty; nieprzetworzona zawartość została zachowana.</translation></message>
    <message><source>The operation could not be completed</source><translation>Nie udało się ukończyć operacji</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>Klucz rejestru ma nowe wartości; nieprzetworzona zawartość została zachowana.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>Klucz rejestru nadal ma podklucze; nieprzetworzona zawartość została zachowana.</translation></message>
    <message><source>The target no longer exists.</source><translation>Obiekt już nie istnieje.</translation></message>
    <message><source>Traversal complete</source><translation>Przeglądanie zakończone</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Przeglądanie zakończone z nieodczytaną zawartością</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>Dziennik przeglądania został zachowany i można go wyeksportować.</translation></message>
    <message><source>Traverse again</source><translation>Przejrzyj ponownie</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Przejrzyj ponownie lub zamknij to okno.</translation></message>
    <message><source>Traversing the folder</source><translation>Przeglądanie folderu</translation></message>
    <message><source>Traversing the registry</source><translation>Przeglądanie rejestru</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Nie można przygotować operacji</translation></message>
    <message><source>View results</source><translation>Wyświetl wyniki</translation></message>

    <message><source>All changes committed</source><translation>Zatwierdzono wszystkie zmiany</translation></message>
    <message><source>Some changes committed</source><translation>Zatwierdzono część zmian</translation></message>
    <message><source>No changes could be committed</source><translation>Nie udało się zatwierdzić żadnych zmian</translation></message>
    <message><source>No changes to commit</source><translation>Brak zmian do zatwierdzenia</translation></message>
    <message><source>Commit interrupted</source><translation>Zatwierdzanie przerwane</translation></message>
    <message><source>%1 items were not processed.</source><translation>Nie przetworzono %1 elementów.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>%1 operacji rejestru nie powiodło się.</translation></message>
    <message><source>The folder does not exist.</source><translation>Folder nie istnieje.</translation></message>
    <message><source>Access denied.</source><translation>Odmowa dostępu.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Element o tej nazwie już istnieje.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Nie udało się utworzyć folderu. Sprawdź nazwę i uprawnienia.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Nie udało się zmienić nazwy elementu. Sprawdź nazwę i uprawnienia.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Ta przeglądarka nie obsługuje zmiany nazw.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>Wersja robocza listy przygotowanych plików zmieniła się po otwarciu tego planu. Sprawdź nowy plan.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>Zapisana lista przygotowanych plików zmieniła się po otwarciu tego planu. Odśwież dane i sprawdź zmiany.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>Zapisana lista przygotowanych plików nie odpowiada zaakceptowanemu planowi.</translation></message>
    <message><source>Export configuration</source><translation>Eksportuj konfigurację</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Zapisz bieżący motyw, język i widoczność HUB w pliku UWF.ini obok tej aplikacji.</translation></message>
    <message><source>Configuration saved</source><translation>Konfiguracja zapisana</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>Bieżące ustawienia interfejsu zapisano w:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>Plik UWF.ini już istnieje. Istniejący plik nie został zmieniony:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>Nie można zapisać pliku UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Pulpit</translation></message>
    <message><source>Downloads</source><translation>Pobrane</translation></message>
    <message><source>Documents</source><translation>Dokumenty</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>Automatyczny zapis plików został zatrzymany. Usuń przyczynę błędu przed ponowną próbą.</translation></message>
    <message><source>No net change for this setting</source><translation>Wartość końcowa tego ustawienia nie zmieniła się po imporcie</translation></message>
    <message><source>Final draft %1 → %2</source><translation>Końcowa wartość wersji roboczej: %1 → %2</translation></message>
    <message><source>Final draft %1 → %2 (requested %3)</source><translation>Końcowa wartość wersji roboczej: %1 → %2 (żądano: %3)</translation></message>
    <message><source>Requested %1 → %2; replaced by a later command. Final draft value → %3</source><translation>Żądano: %1 → %2; zastąpiono późniejszym poleceniem. Końcowa wartość wersji roboczej → %3</translation></message>
    <message><source>Updated the %1 draft</source><translation>Zaktualizowano wersję roboczą %1</translation></message>
    <message><source>Updated volume %1 protection draft → %2</source><translation>Zaktualizowano wersję roboczą ustawienia ochrony woluminu %1 → %2</translation></message>
    <message><source>Select entries to remove.</source><translation>Wybierz wpisy do usunięcia.</translation></message>
    <message><source>Selected entries cannot currently be restored.</source><translation>Obecnie nie można przywrócić wybranych wpisów.</translation></message>
</context>
</TS>
