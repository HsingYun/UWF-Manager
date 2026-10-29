<?xml version="1.0" encoding="utf-8"?>
<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (c) 2026 HsingYun (iakext@gmail.com) -->
<!DOCTYPE TS>
<TS version="2.1" language="pt_PT" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Preview</source><translation>Pré-visualizar</translation></message>
    <message><source>enable</source><translation>ativar</translation></message>
    <message><source>disable</source><translation>desativar</translation></message>
    <message><source>Enabled</source><translation>Ativado</translation></message>
    <message><source>Disabled</source><translation>Desativado</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Cancelar</translation></message>
    <message><source>Close</source><translation>Fechar</translation></message>
    <message><source>Refresh</source><translation>Atualizar</translation></message>
    <message><source>Add</source><translation>Adicionar</translation></message>
    <message><source>Remove</source><translation>Remover</translation></message>
    <message><source>Yes</source><translation>Sim</translation></message>
    <message><source>No</source><translation>Não</translation></message>
    <message><source>Excluded</source><translation>Excluído</translation></message>
    <message><source>Not excluded</source><translation>Não excluído</translation></message>
    <message><source>File…</source><translation>Ficheiro…</translation></message>
    <message><source>Folder…</source><translation>Pasta…</translation></message>
    <message><source>volume ID</source><translation>ID do volume</translation></message>
    <message><source>drive letter</source><translation>letra da unidade</translation></message>
    <message><source>Drive letter</source><translation>Letra da unidade</translation></message>
    <message><source>Volume ID</source><translation>ID do volume</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Estado de proteção deste volume na sessão atual (somente leitura).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Proteger este volume na próxima sessão. As gravações vão para a sobreposição e são descartadas ao reiniciar.</translation></message>
    <message><source>Current session</source><translation>Sessão atual</translation></message>
    <message><source>Next session</source><translation>Próxima sessão</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identificação do volume: a letra é simples, mas a associação se perde se ela mudar, por exemplo ao adicionar ou remover discos. O ID do volume permanece estável.</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Volume não compatível com UWF: </translation></message>
    <message><source>Global settings</source><translation>Configurações globais</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Estado do filtro UWF na sessão atual (somente leitura).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Ativar UWF na próxima sessão. As gravações nos volumes protegidos vão para a sobreposição e são descartadas ao reiniciar.</translation></message>
    <message><source>Overlay type and maximum size can only be changed while the filter is disabled:
1. Disable the filter using the switch above.
2. Reboot (the filter will be off after reboot).
3. Change this setting.</source><translation>Para alterar o tipo e o tamanho máximo:
1. Desative o filtro acima.
2. Reinicie para desativá-lo efetivamente.
3. Altere a configuração.</translation></message>
    <message><source>Overlay storage location. RAM is faster but consumes memory; Disk uses the system drive and offers more capacity. Both are discarded on reboot.</source><translation>Local da sobreposição: RAM é mais rápida, mas usa memória; Disk usa a unidade do sistema e oferece mais capacidade. Ambas são descartadas ao reiniciar.</translation></message>
    <message><source>Type</source><translation>Tipo</translation></message>
    <message><source>Maximum overlay capacity. In RAM mode, capped by total system memory. Disk mode requires at least 1024 MB and enough free space on the system volume.</source><translation>Capacidade máxima da sobreposição. RAM é limitada pela memória total. Disk exige pelo menos 1024 MB e espaço livre suficiente no volume do sistema.</translation></message>
    <message><source>Maximum size</source><translation>Tamanho máximo</translation></message>
    <message><source>Maximum size · RAM %1</source><translation>Tamanho máximo · RAM %1</translation></message>
    <message><source>Warning threshold</source><translation>Limite de aviso</translation></message>
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be lower than the critical threshold. Set to 0 to disable only the warning event.</source><translation>Regista uma notificação de aviso no registo de eventos quando a utilização da sobreposição atinge este valor. Se ambos os eventos estiverem ativados, este valor tem de ser inferior ao limiar crítico. O valor 0 desativa apenas o evento de aviso.</translation></message>
    <message><source>Critical threshold</source><translation>Limite crítico</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. When both events are enabled, this value must be higher than the warning threshold. Set to 0 to disable only the critical event.</source><translation>Regista uma notificação crítica no registo de eventos quando a utilização da sobreposição atinge este valor. Se ambos os eventos estiverem ativados, este valor tem de ser superior ao limiar de aviso. O valor 0 desativa apenas o evento crítico.</translation></message>
    <message><source>Overlay</source><translation>Sobreposição</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Tipo de unidade incompatível (somente discos locais fixos).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>O volume excede o limite UWF de 16 TiB por volume protegido.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: proteção do volume disponível, mas sem exclusões nem gravação por ficheiro.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Falha: Leitura de informações do volume.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 livres / %2</translation></message>
    <message><source>Commit</source><translation>Gravar em disco</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Gravar em disco as alterações deste ficheiro na sobreposição / Gravar alterações do Registo. Esta ação não pode ser desfeita.</translation></message>
    <message><source>Commit file changes…</source><translation>Gravar alterações do ficheiro…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Selecionar ficheiro para gravar em disco. Gravar em disco as alterações deste ficheiro na sobreposição.</translation></message>
    <message><source>Commit folder changes…</source><translation>Gravar alterações da pasta…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Selecionar pasta para gravar em disco. Gravar em disco as alterações desta pasta na sobreposição. Todas as subpastas são incluídas.</translation></message>
    <message><source>Delete and commit file…</source><translation>Excluir o ficheiro e gravar a exclusão em disco…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Selecionar ficheiro para excluir permanentemente. Excluir o ficheiro e gravar a exclusão em disco.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Excluir a pasta e o conteúdo e gravar as exclusões em disco…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Selecionar pasta para excluir permanentemente. Excluir a pasta e o conteúdo e gravar as exclusões em disco. Todas as subpastas são incluídas.</translation></message>
    <message><source>Commit registry changes…</source><translation>Gravar alterações do Registo…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Informe a chave e, opcionalmente, o nome do valor. Sem nome, todos os valores da chave e das subchaves são gravados permanentemente.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Gravar exclusão do Registo…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Informe a chave a excluir e um nome de valor opcional. Sem nome, toda a árvore é excluída permanentemente: chave, valores e subchaves.</translation></message>
    <message><source>File exclusions</source><translation>Exclusões de ficheiros</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Ficheiros e pastas do volume excluídos da proteção UWF. Clique duas vezes em uma entrada para copiar o caminho.</translation></message>
    <message><source>Registry exclusions</source><translation>Exclusões do Registo</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>As exclusões do Registo são globais e dependem do filtro UWF, não da proteção deste volume. A lista é compartilhada e aparece apenas uma vez nas abas de discos. Clique duas vezes para copiar o caminho.</translation></message>
    <message><source>File staging</source><translation>Ficheiros para gravação automática</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>ficheiros / pastas: Gravação automática antes do desligamento ou reinício seguro. Todas as subpastas são incluídas. Enquanto houver entradas, esta exclusão do Registo preserva a lista entre sessões protegidas: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Adicionar ficheiros ou uma pasta à lista de gravação automática.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Selecionar ficheiro para gravar em disco. Gravação automática antes do desligamento ou reinício seguro.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Selecionar pasta para gravar em disco. Gravação automática antes do desligamento ou reinício seguro. Todas as subpastas são incluídas.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Selecionar ficheiro para gravar em disco · Ficheiros para gravação automática</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Selecionar pasta para gravar em disco · Ficheiros para gravação automática · Todas as subpastas são incluídas.</translation></message>
    <message><source>Cannot stage this path</source><translation>Não foi possível preparar este caminho</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>O caminho se sobrepõe a uma exclusão UWF. Remova primeiro a exclusão:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>O caminho se sobrepõe à gravação automática. Remova primeiro a entrada:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>O caminho se sobrepõe a uma entrada de gravação automática</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Esta exclusão do Registo é necessária enquanto houver entradas de gravação automática</translation></message>
    <message><source>The selected path does not exist or is not the expected file type:
%1</source><translation>O caminho não existe ou não é do tipo esperado:
%1</translation></message>
    <message><source>The selected path could not be inspected:
%1</source><translation>Não foi possível examinar o caminho:
%1</translation></message>
    <message><source>Reparse points cannot be added to automatic file staging:
%1</source><translation>Pontos de nova análise não podem ser adicionados à gravação automática:
%1</translation></message>
    <message><source>The path has no drive letter and cannot be committed by UWF:
%1</source><translation>Sem letra de unidade, o UWF não pode gravar este caminho:
%1</translation></message>
    <message><source>The selected path is on volume %1. Add it from that volume's File staging tab.</source><translation>O caminho está no volume %1. Adicione-o pela aba de gravação automática desse volume.</translation></message>
    <message><source>A volume root cannot be staged because it would recursively commit the entire volume.</source><translation>A raiz do volume não pode ser adicionada, pois gravaria todo o volume recursivamente.</translation></message>
    <message><source>File staging could not be saved</source><translation>Falha: Salvamento da lista de gravação automática</translation></message>
    <message><source>The file staging state was reloaded after the update could not be completed:
%1</source><translation>A lista foi recarregada porque a atualização não pôde ser concluída:
%1</translation></message>
    <message><source>File staging is unavailable: %1</source><translation>Ficheiros para gravação automática: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Gravação automática antes do desligamento ou reinício seguro.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Ficheiros para gravação automática: %1 · ficheiros: %2 · pastas: %3</translation></message>
    <message><source>File staging relies on this registry exclusion to preserve its list across restarts.</source><translation>A gravação automática de ficheiros depende desta exclusão do registo para preservar a lista após reiniciar.</translation></message>
    <message><source>Built-in</source><translation>Integrado</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>O filtro UWF está desativado. Não há alterações acumuladas na sobreposição para gravar.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>O sistema de ficheiros %1 não permite gravação por ficheiro.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Este volume não é compatível com UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Este volume não permite gravação de ficheiros. Apenas a gravação do Registo está disponível (exclusões globais).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>O sistema de ficheiros %1 não permite gravação por ficheiro. Apenas a gravação do Registo está disponível (exclusões globais).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Este volume não está protegido pelo UWF; não há alterações de ficheiros para gravar.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Este volume não está protegido pelo UWF; não há alterações de ficheiros para gravar. Apenas a gravação do Registo está disponível (exclusões globais).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Selecionar ficheiro para gravar em disco</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Selecionar pasta para gravar em disco</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Selecionar ficheiro para excluir permanentemente</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Selecionar pasta para excluir permanentemente</translation></message>
    <message><source>Name</source><translation>Nome</translation></message>
    <message><source>Data</source><translation>Dados</translation></message>
    <message><source>(Default)</source><translation>(Padrão)</translation></message>
    <message><source>(value not set)</source><translation>(valor não definido)</translation></message>
    <message><source>(empty)</source><translation>(vazio)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Desconhecido (Valor: Ficheiro muito grande)</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Ver ficheiros da sobreposição. &lt;b&gt;Apenas um instantâneo de diagnóstico.&lt;/b&gt; Não use a lista para decidir o que gravar.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>A listagem de ficheiros da sobreposição exige volumes NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>O filtro UWF está desativado; nenhum volume tem sobreposição para examinar.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>O volume não está protegido pelo UWF e não tem sobreposição para examinar.</translation></message>
    <message><source>Failed: %1</source><translation>Falha: %1</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>O provedor WMI falhou durante a enumeração. É uma instabilidade conhecida de UWF_Overlay.GetOverlayFiles com sobreposições grandes ou alta carga de E/S.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Memória insuficiente ou operação não compatível. Exige NTFS e memória livre. Tente com uma sobreposição menor ou feche aplicações que consomem muita memória.</translation></message>
    <message><source>Raw error: %1</source><translation>Código do erro: %1</translation></message>
    <message><source>Export overlay file list</source><translation>Exportar para ficheiro… · Sobreposição · ficheiros</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Ficheiros de texto (*.txt);;Todos os ficheiros (*)</translation></message>
    <message><source>Command files (*.cmd);;All files (*)</source><translation>Ficheiros de comandos (*.cmd);;Todos os ficheiros (*)</translation></message>
    <message><source>Export failed</source><translation>Falha na exportação</translation></message>
    <message><source>Export finished</source><translation>Exportação concluída</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Falha: Abertura do ficheiro (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Falha: Gravação do ficheiro (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Configuração: Gravação do ficheiro · entradas: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Não é possível excluir um volume inteiro (%1) da proteção.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Ficheiros de paginação, swap e hibernação não podem ser excluídos da proteção: o UWF depende deles.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Não é possível excluir o diretório inteiro da proteção: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Não é possível excluir o diretório inteiro da proteção: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Não é possível excluir o diretório inteiro da proteção: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Este ficheiro essencial do sistema não pode ser excluído da proteção: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>O ficheiro de Registo do utilizador NTUSER.DAT não pode ser excluído da proteção.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>A chave do Registo não pode ficar vazia.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>A chave do Registo não pode começar com barra invertida.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>O caminho contém barras invertidas consecutivas e é inválido.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Caminhos do Registo usam `\` como separador, não `/`.</translation></message>
    <message><source>The path contains invisible control characters; this is not valid.</source><translation>O caminho contém caracteres de controle invisíveis e é inválido.</translation></message>
    <message><source>HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC cannot be excluded; this is the domain machine account secret, which UWF documentation explicitly forbids excluding.</source><translation>A documentação UWF proíbe excluir o segredo da conta de computador do domínio: HKLM\SECURITY\Policy\Secrets\$MACHINE.ACC</translation></message>
    <message><source>UWF only allows exclusions under the following top-level registry keys. Please pick a specific subkey under one of them:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</source><translation>Só é possível excluir subchaves específicas destas chaves principais:
  HKLM\BCD00000000
  HKLM\SYSTEM
  HKLM\SOFTWARE
  HKLM\SAM
  HKLM\SECURITY
  HKLM\COMPONENTS</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Adicionar ficheiros ou pastas às exclusões. Itens excluídos não são protegidos pelo UWF.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Selecionar ficheiro para adicionar · Exclusões de ficheiros.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Selecionar pasta para adicionar · Exclusões de ficheiros. Todas as subpastas são incluídas.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Informe o caminho da chave do Registo a excluir.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Informe o caminho da chave do Registo a excluir. / Persistência.</translation></message>
    <message><source>Registry key…</source><translation>Registo · Chave…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Chave secreta do domínio (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Licença de acesso de cliente dos Serviços de Terminal (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Preservar o segredo do domínio (senha da conta de computador) entre sessões UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Preservar licenças de acesso de cliente dos Serviços de Terminal entre sessões UWF.</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistência %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Remover selecionados</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Remover as exclusões selecionadas. Entra em vigor após aplicar.</translation></message>
    <message><source>Open containing folder</source><translation>Abrir pasta do ficheiro</translation></message>
    <message><source>Copy file path</source><translation>Copiar caminho do ficheiro</translation></message>
    <message><source>Copy registry path</source><translation>Copiar caminho do Registo</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Gravar em disco as alterações desta pasta na sobreposição…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Gravar em disco as alterações deste ficheiro na sobreposição…</translation></message>
    <message><source>Copied to clipboard</source><translation>Copiado para a área de transferência</translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Selecionar ficheiro para adicionar · Exclusões de ficheiros. É possível selecionar vários ficheiros.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Selecionar pasta para adicionar · Exclusões de ficheiros</translation></message>
    <message><source>Add registry exclusion</source><translation>Adição de exclusão do Registo</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>O caminho %1 não está no volume %2 e não pode ser adicionado às suas exclusões.</translation></message>
    <message><source>Path is not on this volume</source><translation>O caminho não pertence a este volume</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Não foi possível adicionar esta exclusão</translation></message>
    <message><source>File: %1</source><translation>Ficheiro… %1</translation></message>
    <message><source>Folder: %1</source><translation>Pasta… %1</translation></message>
    <message><source>Registry: %1</source><translation>Registo: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Excluído por padrão; pode ser alterado a qualquer momento.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Sessão atual: %1
Próxima sessão: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Alterações pendentes: %1</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Gestor do Filtro de Gravação Unificado (UWF)</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Reler o estado atual e a configuração da próxima sessão do UWF.</translation></message>
    <message><source>Review and apply</source><translation>Revisar e aplicar</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Revisar todas as alterações pendentes e aplicá-las em lote. A maioria das alterações entra em vigor após a próxima reinício.</translation></message>
    <message><source>Safe shutdown</source><translation>Desligamento seguro</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>O sistema desligará com segurança pelo UWF. Disponível mesmo com a sobreposição UWF cheia.</translation></message>
    <message><source>Safe restart</source><translation>Reinício seguro</translation></message>
    <message><source>Direct restart</source><translation>Reinício direta</translation></message>
    <message><source>Direct shutdown</source><translation>Desligamento direto</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>O sistema reiniciará com segurança pelo UWF. Disponível mesmo com a sobreposição UWF cheia.</translation></message>
    <message><source>Log</source><translation>Registo</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Ver o registo interno desta sessão para diagnóstico.</translation></message>
    <message><source>About</source><translation>Sobre</translation></message>
    <message><source>Switch display language</source><translation>Alterar idioma de exibição</translation></message>
    <message><source>Language</source><translation>Idioma</translation></message>
    <message><source>Theme</source><translation>Tema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Alternar tema claro/escuro</translation></message>
    <message><source>System information</source><translation>Informações do sistema</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Instantâneo de diagnóstico</translation></message>
    <message><source>Read only</source><translation>Somente leitura</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Destino não encontrado; não há nada para gravar na sobreposição.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>O destino só existe na sobreposição: foi criado sob UWF e nunca gravado no disco ou Registo persistente. Desaparecerá ao reiniciar; gravar sua exclusão não é necessário nem possível.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>O sistema rejeitou um parâmetro (caminho ou argumento inválido).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>A operação falhou. Outro processo pode estar usando o destino, como uma pasta no Explorador ou um ficheiro aberto. Feche esses programas e tente novamente.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>A operação falhou; veja os detalhes no registo.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operação rejeitada (código %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Desconhecido.</translation></message>
    <message><source>Commit canceled</source><translation>Gravação cancelada</translation></message>

    <message><source>Succeeded</source><translation>Sucesso</translation></message>
    <message><source>Skipped</source><translation>Ignorado</translation></message>
    <message><source>Failed</source><translation>Falha</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Sucesso: %1; Ignorado: %2; Falha: %3.</translation></message>

    <message><source>Existed before</source><translation>Existia antes</translation></message>
    <message><source>Exists after</source><translation>Existe depois</translation></message>
    <message><source>Error code</source><translation>Código do erro</translation></message>
    <message><source>Reason</source><translation>Motivo</translation></message>
    <message><source>Previous page</source><translation>Página anterior</translation></message>
    <message><source>First page</source><translation>Primeira página</translation></message>
    <message><source>Last page</source><translation>Última página</translation></message>
    <message><source>Next page</source><translation>Próxima página</translation></message>
    <message><source>Copy selected rows</source><translation>Copiar linhas selecionadas</translation></message>
    <message><source>Copy all</source><translation>Copiar tudo</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Esta exclusão do Registo é necessária enquanto houver entradas de gravação automática. Ficheiros para gravação automática: Limpar → Remoção de exclusão do Registo.</translation></message>
    <message><source>Copy current page</source><translation>Copiar página atual</translation></message>
    <message><source>No pending changes</source><translation>Nenhuma alteração pendente</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Exclusões do Registo · Configurações globais.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: Proteção UWF / Exclusões de ficheiros / Ficheiros para gravação automática.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Não compatível</translation></message>
    <message><source>Review and apply changes</source><translation>Revisar e aplicar alterações</translation></message>
    <message><source>Confirm apply</source><translation>Confirmar aplicação</translation></message>
    <message><source>Result</source><translation>Resultado</translation></message>
    <message><source>✓ Filter: %1</source><translation>✓ Filtro: %1</translation></message>
    <message><source>✓ Overlay warning threshold set to %1 MB</source><translation>✓ Definição do limite de aviso: %1 MB</translation></message>
    <message><source>✓ Overlay critical threshold set to %1 MB</source><translation>✓ Definição do limite crítico: %1 MB</translation></message>
    <message><source>✘ Type / maximum size not applied: the filter is currently enabled. Disable the filter and reboot first.</source><translation>✘ Definição do tipo de sobreposição / Definição do tamanho máximo: Para alterar o tipo e o tamanho máximo:
1. Desative o filtro acima.
2. Reinicie para desativá-lo efetivamente.
3. Altere a configuração.</translation></message>
    <message><source>✓ Overlay type set to %1</source><translation>✓ Definição do tipo de sobreposição: %1</translation></message>
    <message><source>✓ Overlay maximum size set to %1 MB</source><translation>✓ Definição do tamanho máximo: %1 MB</translation></message>
    <message><source>✘ Maximum size not applied: a disk-based overlay requires at least %1 MB.</source><translation>✘ Definição do tamanho máximo: Disk ≥ %1 MB.</translation></message>
    <message><source>✓ Volume %1 protection: %2</source><translation>✓ Volume %1 · Proteção UWF: %2</translation></message>
    <message><source>✓ Volume %1 bind by: %2</source><translation>✓ Volume %1 · Identificar por: %2</translation></message>
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1 · Adição de exclusão de ficheiro: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1 · Remoção de exclusão de ficheiro: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Adição de exclusão do Registo: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Remoção de exclusão do Registo: %1</translation></message>
    <message><source>Time</source><translation>Hora</translation></message>
    <message><source>Level</source><translation>Nível</translation></message>
    <message><source>Tag</source><translation>Tag</translation></message>
    <message><source>Message</source><translation>Mensagem</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Calculando ficheiros para gravação automática…</translation></message>
    <message><source>Committing staged files</source><translation>Gravando ficheiros preparados</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 de %2 ficheiros processados.</translation></message>
    <message><source>No additional error details were provided.</source><translation>Nenhum detalhe adicional do erro.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>A gravação automática foi interrompida inesperadamente:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Gravação automática incompleta: Erro desconhecido.</translation></message>
    <message><source>File staging will be skipped</source><translation>A gravação automática será ignorada</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Reinício direta: A gravação automática será ignorada.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Desligamento direto: A gravação automática será ignorada.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Gravação automática ignorada porque foi escolhida uma ação de energia direta.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Desligamento seguro: Falha</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Desligar: Falha (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Reinício seguro: Falha</translation></message>
    <message><source>Direct restart failed</source><translation>Reinício direta: Falha</translation></message>
    <message><source>Direct shutdown failed</source><translation>Desligamento direto: Falha</translation></message>
    <message><source>Restart failed: %1</source><translation>Reiniciar: Falha (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Não foi possível ler a lista de gravação automática:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Não foi possível ler a lista de gravação automática: Erro desconhecido.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>ficheiros: %1 · Sucesso: %2 · Ignorado (ficheiros): %3 · Ignorado (entradas): %4 · Falha: %5</translation></message>
    <message><source>File staging preparation</source><translation>Ficheiros para gravação automática: Detalhes da operação</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Pontos de nova análise não são compatíveis.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Somente caminhos absolutos em volumes locais podem ser adicionados.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Somente caminhos absolutos em volumes locais podem ser adicionados.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Não foi possível examinar o caminho preparado.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>O caminho do ficheiro preparado agora se refere a outro tipo.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>O caminho da pasta preparada agora se refere a outro tipo.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Não foi possível enumerar completamente a pasta preparada.</translation></message>
    <message><source>The path has no drive letter.</source><translation>O caminho não tem letra de unidade.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>A raiz do volume não pode ser processada recursivamente.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>O provedor UWF rejeitou a gravação.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Falha na gravação: Erro desconhecido.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Mais %1 falhas foram registradas no registo.</translation></message>

    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>O caminho não tem letra de unidade. Identificação do volume de destino: Falha.</translation></message>

    <message><source>No current-session record found for volume %1.</source><translation>Nenhum registo da sessão atual para o volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>O caminho está na lista de exclusões de ficheiros.
Exclusão: %1</translation></message>

    <message><source>Continue</source><translation>Continuar</translation></message>
    <message><source>Delete and commit</source><translation>Excluir e gravar</translation></message>

    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>A chave está na lista de exclusões do Registo.
Exclusão: %1</translation></message>

    <message><source>No current-session registry filter record found.</source><translation>Nenhum registo do filtro do Registo para a sessão atual.</translation></message>

    <message><source>Export commands to file</source><translation>Exportar comandos para ficheiro</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Exportar para ficheiro… · Comando: %1
%2</translation></message>
    <message><source>Import</source><translation>Importar</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Digite ou cole um comando por linha. Carregue textos, registos ou scripts (.txt, .bat, .ps1); as linhas com este comando serão adicionadas. A importação prepara alterações. Apenas “Revisar e aplicar” na barra de ferramentas as grava no sistema.</translation></message>
    <message><source>Load from file…</source><translation>Carregar de ficheiro…</translation></message>
    <message><source>Load recommended configuration</source><translation>Carregar configuração recomendada</translation></message>
    <message><source>Choose recommended configuration</source><translation>Escolher configuração recomendada</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Selecione os grupos recomendados a adicionar. Revise ou exclua comandos individuais antes de importar.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Da documentação oficial da Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Exclusões UWF recomendadas pela Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Revise antes de importar: as pastas devem existir para o UWF aceitar as exclusões.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Fonte: documentação oficial Microsoft UWF sobre exclusões comuns e suporte antimalware.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Programa de Aperfeiçoamento da Experiência do Utilizador (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Preservar permanentemente: Estado de adesão por política</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Preservar permanentemente: Estado de adesão local</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Preservar permanentemente: Sinalizador de envio desativado</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Serviço de Transferência Inteligente em Segundo Plano (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Preservar permanentemente: Ficheiros da fila de transferência</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Preservar permanentemente: Índice do estado de transferência</translation></message>
    <message><source>Network profiles and policies</source><translation>Perfis e políticas de rede</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Política GPO de rede sem fio</translation></message>
    <message><source>Wired network GPO policy</source><translation>Política GPO de rede com fio</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Política GPO de rede sem fio · ficheiros</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Política GPO de rede com fio · ficheiros</translation></message>
    <message><source>Wireless network interface profiles</source><translation>Perfis de interfaces sem fio</translation></message>
    <message><source>Wired network interface profiles</source><translation>Perfis de interfaces com fio</translation></message>
    <message><source>Wireless service configuration</source><translation>Configuração do serviço sem fio</translation></message>
    <message><source>Mobile broadband service configuration</source><translation>Configuração do serviço de banda larga móvel</translation></message>
    <message><source>Wired AutoConfig service configuration</source><translation>Configuração do serviço de configuração automática com fio</translation></message>
    <message><source>Network profile XML files are per-device; add the concrete Interfaces\{GUID}\{GUID}.xml paths manually if needed.</source><translation>Os perfis XML são específicos do dispositivo. Adicione manualmente os caminhos exatos Interfaces\{GUID}\{GUID}.xml se necessário.</translation></message>
    <message><source>Daylight saving time (DST)</source><translation>Horário de verão (DST)</translation></message>
    <message><source>DST: persist time zone definitions</source><translation>DST: Preservar permanentemente: Definições de fuso horário</translation></message>
    <message><source>DST: persist selected time zone information</source><translation>DST: Preservar permanentemente: Informações do fuso selecionado</translation></message>
    <message><source>Microsoft Defender</source><translation>Microsoft Defender</translation></message>
    <message><source>Defender: persist product files and updates</source><translation>Defender: Preservar permanentemente: Ficheiros do produto e atualizações</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Preservar permanentemente: Assinaturas e estado em ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Preservar permanentemente: Windows Update · Registo</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Preservar permanentemente: MpCmdRun · Registo</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Preservar permanentemente: Estado do produto no Registo</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Preservar permanentemente: WdBoot · Estado do serviço</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Preservar permanentemente: WdFilter · Estado do serviço</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Preservar permanentemente: WdNisSvc · Estado do serviço</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Preservar permanentemente: WdNisDrv · Estado do serviço</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Preservar permanentemente: WinDefend · Estado do serviço</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Preservar permanentemente: Ficheiros do programa cliente</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Preservar permanentemente: Windows Update · Registo</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Preservar permanentemente: MpCmdRun · Registo</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Preservar permanentemente: Assinaturas e estado antimalware</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Preservar permanentemente: Estado antimalware no Registo</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Escolher ficheiros com comandos uwfmgr</translation></message>
    <message><source>Text files (*.txt *.cmd);;All files (*)</source><translation>Ficheiros de texto (*.txt *.cmd);;Todos os ficheiros (*)</translation></message>
    <message><source>Status</source><translation>Status</translation></message>
    <message><source>Import failed</source><translation>Falha na importação</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Erro interno: nenhum aplicador registrado.</translation></message>
    <message><source>Nothing to import</source><translation>Nenhum comando para importar</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Nenhum comando uwfmgr encontrado na entrada.</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplicado: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Não compatível: %1</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Lote %1 ──</translation></message>
    <message><source>Duplicate</source><translation>Duplicado</translation></message>
    <message><source>Unsupported</source><translation>Não compatível</translation></message>
    <message><source>File too large</source><translation>Ficheiro muito grande</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>O ficheiro %1 tem mais de 5 MB e não foi analisado. Filtre-o manualmente primeiro.</translation></message>
    <message><source>Cannot read file</source><translation>Não foi possível ler o ficheiro</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Falha: Abertura do ficheiro (%1): %2</translation></message>
    <message><source>Discard pending changes?</source><translation>Descartar alterações pendentes?</translation></message>
    <message><source>There are %1 pending change(s) that have not been applied.

Continue and discard them?</source><translation>Há %1 alterações não aplicadas.

Descartar e continuar?</translation></message>
    <message><source>Incomplete uwfmgr command</source><translation>Comando uwfmgr incompleto</translation></message>
    <message><source>Missing size argument (MB)</source><translation>Argumento ausente: Tamanho (MB)</translation></message>
    <message><source>Size must be a non-negative integer in MB</source><translation>O tamanho deve ser um inteiro não negativo em MB</translation></message>
    <message><source>Missing overlay type argument (RAM or Disk)</source><translation>Argumento ausente: Sobreposição · Tipo (RAM / Disk)</translation></message>
    <message><source>Unknown overlay type %1 (expected RAM or Disk)</source><translation>Valor inválido: Sobreposição · Tipo: %1 (RAM / Disk)</translation></message>
    <message><source>Missing volume argument (e.g. C:)</source><translation>Argumento ausente: Volume (C:)</translation></message>
    <message><source>Volume must be a drive letter such as C:</source><translation>O volume deve ser uma letra de unidade, como C:</translation></message>
    <message><source>Missing path argument</source><translation>Argumento ausente: Caminho</translation></message>
    <message><source>Missing registry key argument</source><translation>Argumento ausente: Registo · Chave</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Aspas duplas não fechadas no comando</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Argumento extra inesperado: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Comando uwfmgr não compatível (sem ação correspondente na interface)</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>Alterações pendentes: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Já está no estado desejado; nenhuma ação necessária</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>O caminho está fora do volume ou o volume não permite exclusões (ex.: exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Rejeitado pela lista de bloqueio UWF (ficheiro de sistema, Windows, paginação etc.)</translation></message>
    <message><source>Pending filter %1</source><translation>Alterações pendentes: Filtro %1</translation></message>
    <message><source>Filter is already in the target state</source><translation>Filtro: Já está no estado desejado; nenhuma ação necessária</translation></message>
    <message><source>Pending overlay type → %1</source><translation>Alterações pendentes: Sobreposição · Tipo → %1</translation></message>
    <message><source>Overlay type already %1</source><translation>Sobreposição · Tipo: %1 (Já está no estado desejado; nenhuma ação necessária)</translation></message>
    <message><source>Invalid size value: %1</source><translation>Valor inválido: Tamanho: %1</translation></message>
    <message><source>maximum size</source><translation>Tamanho máximo</translation></message>
    <message><source>warning threshold</source><translation>Limite de aviso</translation></message>
    <message><source>critical threshold</source><translation>Limite crítico</translation></message>
    <message><source>Pending overlay %1 → %2 MB</source><translation>Alterações pendentes: Sobreposição %1 → %2 MB</translation></message>
    <message><source>Overlay %1 already %2 MB</source><translation>Sobreposição %1: %2 MB (Já está no estado desejado; nenhuma ação necessária)</translation></message>
    <message><source>Unknown volume %1 (no UWF-eligible disk with that drive letter)</source><translation>Volume %1 desconhecido (sem disco compatível com UWF nessa letra)</translation></message>
    <message><source>Pending volume %1 protection %2</source><translation>Alterações pendentes: Volume %1 · Proteção UWF: %2</translation></message>
    <message><source>Volume %1 is already in the target protection state</source><translation>Volume %1: Já está no estado desejado; nenhuma ação necessária</translation></message>
    <message><source>Path %1 has no drive letter; cannot route to a volume tab</source><translation>%1: O caminho não tem letra de unidade. Identificação do volume de destino: Falha.</translation></message>
    <message><source>No UWF-eligible disk for drive letter %1</source><translation>Volume %1: UWF Não compatível</translation></message>
    <message><source>file exclusion</source><translation>Exclusões de ficheiros</translation></message>
    <message><source>registry exclusion</source><translation>Exclusões do Registo</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>As exclusões do Registo exigem a aba Registo, que não está presente</translation></message>
    <message><source>Unsupported command</source><translation>Comando: Não compatível</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Mostrar ou ocultar.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Ativado</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Desativado</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Desconhecido</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Usado %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Modo avançado</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordenar a gravação automática com o desligamento e a reinício do Windows.</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Falha: Leitura do estado do modo avançado (Erro desconhecido).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Falha: Leitura do estado do serviço:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Falha: Leitura do estado do serviço (Erro desconhecido).</translation></message>
    <message><source>The service is not configured as an independent process.</source><translation>O serviço não está configurado como processo independente.</translation></message>
    <message><source>The service is not configured for automatic startup.</source><translation>O serviço não está configurado para início automático.</translation></message>
    <message><source>The service is not configured to run as LocalSystem.</source><translation>O serviço não está configurado para executar como LocalSystem.</translation></message>
    <message><source>The service is not running.</source><translation>O serviço não está em execução.</translation></message>
    <message><source>The service executable path or startup arguments do not match this executable.</source><translation>O caminho ou os argumentos do serviço não correspondem ao executável atual.</translation></message>
    <message><source>The service preshutdown timeout is not configured correctly.</source><translation>O tempo limite de pré-desligamento do serviço está incorreto.</translation></message>
    <message><source>The service does not declare the privileges required to start the UI agent.</source><translation>O serviço não declara os privilégios necessários para iniciar o agente de interface.</translation></message>
    <message><source>The service has not accepted Windows preshutdown notifications.</source><translation>O serviço não aceitou as notificações de pré-desligamento do Windows.</translation></message>
    <message><source>Enable enhanced mode</source><translation>Ativar modo avançado</translation></message>
    <message><source>Disable enhanced mode</source><translation>Desativar modo avançado</translation></message>
    <message><source>Start service</source><translation>Iniciar serviço</translation></message>
    <message><source>Repair enhanced mode</source><translation>Reparar modo avançado</translation></message>
    <message><source>Delete service</source><translation>Excluir serviço</translation></message>
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Parar e excluir o serviço avançado, incluindo dados residuais no Registo.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>O serviço não existe, mas restam dados no Registo.</translation></message>
    <message><source>Waiting for the UI agent to complete authentication.</source><translation>Aguardando a autenticação do agente de interface.</translation></message>
    <message><source>The service is running, but no authenticated UI agent is connected.</source><translation>O serviço está em execução, mas não há agente de interface autenticado conectado.</translation></message>
    <message><source>UWF Manager enhanced mode helper service</source><translation>Serviço auxiliar do modo avançado do UWF Manager</translation></message>
    <message><source>The enhanced mode configuration was applied.</source><translation>Modo avançado · Configuração: Aplicado.</translation></message>
    <message><source>The service configuration was applied, but UWF persistence reported:
%1</source><translation>Configuração: Aplicado. Relato da persistência UWF:
%1</translation></message>
    <message><source>The enhanced mode configuration failed:
%1</source><translation>Modo avançado · Configuração: Falha:
%1</translation></message>
    <message><source>The enhanced mode configuration failed because of an unknown error.</source><translation>Modo avançado · Configuração: Falha (Erro desconhecido).</translation></message>
    <message><source>The enhanced mode service and its remaining configuration were deleted.</source><translation>O serviço avançado e a configuração residual foram excluídos.</translation></message>
    <message><source>The service deletion was requested, but residual cleanup reported:
%1</source><translation>A exclusão do serviço foi solicitada, mas a limpeza residual relatou:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted:
%1</source><translation>Modo avançado: Excluir serviço: Falha:
%1</translation></message>
    <message><source>The enhanced mode service could not be deleted because of an unknown error.</source><translation>Modo avançado: Excluir serviço: Falha (Erro desconhecido).</translation></message>
    <message><source>File staging completed</source><translation>Ficheiros para gravação automática: Sucesso</translation></message>
    <message><source>File staging failed</source><translation>Ficheiros para gravação automática: Falha</translation></message>
    <message><source>All staged targets were processed.</source><translation>Todos os destinos preparados foram processados.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Todos os destinos foram processados; uma ou mais operações falharam.</translation></message>
    <message><source>Show main window</source><translation>Mostrar janela principal</translation></message>
    <message><source>Restore default position</source><translation>Restaurar posição padrão</translation></message>
    <message><source>Show overlay hub</source><translation>Mostrar HUB da sobreposição</translation></message>
    <message><source>Hide overlay hub</source><translation>Ocultar HUB da sobreposição</translation></message>
    <message><source>Exit application</source><translation>Sair do aplicação</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Falha: Conexão ao sistema</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Falha: Leitura de valores do Registo (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Enumeração de ficheiros da sobreposição: Erro desconhecido.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Falha: Ação no filtro: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Falha: Leitura do estado do filtro</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Falha: Definição do limite de aviso</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Falha: Definição do limite crítico</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Falha: Leitura do estado da sobreposição</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Falha: Definição do tipo de sobreposição</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Falha: Definição do tamanho máximo</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Falha: Leitura da configuração da sobreposição</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Falha: Registo do volume %1 no UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Falha: Proteção do volume %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Falha: Definição da identificação do volume %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Falha: Adição de exclusão de ficheiro %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Exclusão de ficheiro já ausente no volume %1: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Falha: Remoção de exclusão de ficheiro %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Falha: Leitura da configuração do volume</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Falha: Adição de exclusão do Registo %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Falha: Remoção de exclusão do Registo %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Falha: Atualização das opções de persistência do Registo</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Falha: Leitura do filtro do Registo</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Não compatível</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Leitura do estado do destino: Erro desconhecido.</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>O caminho não existe; não há nada para excluir.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Erro desconhecido.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Não foi possível confirmar o resultado porque a releitura do estado falhou: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>O provedor aceitou a exclusão, mas a verificação autoritativa confirma que o destino ainda existe.</translation></message>
    <message><source>Unknown</source><translation>Desconhecido</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Falha: Carregamento do registo (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Falha: Carregamento do registo (Erro desconhecido).</translation></message>
    <message><source>: unknown error</source><translation>: Erro desconhecido</translation></message>

    <message><source>Failed to resolve the target volume: %1</source><translation>Falha: Identificação do volume de destino (%1)</translation></message>

    <message><source>Failed to enumerate registry keys: %1</source><translation>Falha: Enumeração de chaves do Registo (%1)</translation></message>

    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Falha: Identificação do volume de destino (%1): %2</translation></message>
    <message><source>Skipped because a prerequisite operation failed or could not be confirmed.</source><translation>Operação ignorada porque uma operação prévia necessária falhou ou não pôde ser confirmada.</translation></message>
    <message><source>Copy</source><translation>Copiar</translation></message>
    <message><source>Select All</source><translation>Selecionar tudo</translation></message>
    <message><source>Display language</source><translation>Idioma de apresentação</translation></message>
    <message><source>Folder</source><translation>Pasta</translation></message>
    <message><source>File</source><translation>Ficheiro</translation></message>
    <message><source>Export commands</source><translation>Exportar comandos</translation></message>
    <message><source>Direct restart…</source><translation>Reinício direta…</translation></message>
    <message><source>Clear…</source><translation>Limpar…</translation></message>
    <message><source>Committing</source><translation>Gravando</translation></message>
    <message><source> · %1 selected</source><translation> · %1 selecionados</translation></message>
    <message><source>%1 changes</source><translation>%1 alterações</translation></message>
    <message><source>%1 commands</source><translation>%1 comandos</translation></message>
    <message><source>%1 items</source><translation>%1 itens</translation></message>
    <message><source>%1 items · %2</source><translation>%1 itens · %2</translation></message>
    <message><source>%1 pending change(s)</source><translation>%1 alterações pendentes</translation></message>
    <message><source>%1 records</source><translation>%1 registos</translation></message>
    <message><source>%1 records · %2 total</source><translation>%1 registos · %2 no total</translation></message>


    <message><source>%1 selected</source><translation>%1 selecionados</translation></message>
    <message><source>/ %1 files</source><translation>/ %1 ficheiros</translation></message>
    <message><source>32-bit</source><translation>32 bits</translation></message>
    <message><source>64-bit</source><translation>64 bits</translation></message>
    <message><source>&lt;p&gt;Released under the &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Full terms are in the LICENSE file included with the application.&lt;/p&gt;&lt;p&gt;You may redistribute or modify it under GPL v3. The software is provided as is, without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Distribuído sob a &lt;a href="https://www.gnu.org/licenses/gpl-3.0.html"&gt;GNU General Public License v3.0&lt;/a&gt;. Os termos completos estão no ficheiro LICENSE incluído na aplicação.&lt;/p&gt;&lt;p&gt;Pode redistribuir ou modificar o software sob a GPL v3. O software é fornecido como está, sem qualquer garantia.&lt;/p&gt;</translation></message>
    <message><source>&lt;p&gt;Windows disables paging files, System Restore, SuperFetch, file indexing, disk defragmentation, and fast startup. It also sets the BCD bootstatuspolicy to ignoreallfailures.&lt;/p&gt;&lt;p&gt;You can adjust these settings afterwards. For example, move the paging file to an unprotected volume before enabling paging again.&lt;/p&gt;</source><translation>&lt;p&gt;O Windows desativa os ficheiros de paginação, a Restauro do Sistema, o SuperFetch, a indexação de ficheiros, a desfragmentação e a arranque rápida. Também define bootstatuspolicy como ignoreallfailures no BCD.&lt;/p&gt;&lt;p&gt;Pode ajustar essas configurações depois. Por exemplo, mova o ficheiro de paginação para um volume desprotegido antes de reativar a paginação.&lt;/p&gt;</translation></message>
    <message><source>A diagnostic snapshot of the system, privileges, and UWF.</source><translation>Um instantâneo de diagnóstico do sistema, dos privilégios e do UWF.</translation></message>
    <message><source>A file already exists at this location:
%1

Saving will replace its contents.</source><translation>Já existe um ficheiro neste local:
%1

Guardar substituirá o conteúdo existente.</translation></message>
    <message><source>A graphical manager for Windows Unified Write Filter.
Manage volume protection, overlay, exclusions, and staged files.</source><translation>Um gestor gráfico para o Windows Unified Write Filter.
Faça a gestão da proteção de volumes, da sobreposição, das exclusões e dos ficheiros pendentes.</translation></message>
    <message><source>About this snapshot</source><translation>Sobre este instantâneo</translation></message>
    <message><source>Account</source><translation>Conta</translation></message>
    <message><source>Add UWF configuration from a script or text.</source><translation>Adicionar uma configuração UWF de um script ou texto.</translation></message>
    <message><source>Add exclusion</source><translation>Adicionar exclusão</translation></message>
    <message><source>Add files or folders</source><translation>Adicionar ficheiros ou pastas</translation></message>
    <message><source>Add selected items</source><translation>Adicionar itens selecionados</translation></message>
    <message><source>Added</source><translation>Adicionado</translation></message>
    <message><source>Administrator</source><translation>Administrador</translation></message>
    <message><source>All changes have been saved to disk and will persist without a restart.</source><translation>Todas as alterações foram guardadas no disco e serão mantidas sem reiniciar.</translation></message>
    <message><source>All files (*)</source><translation>Todos os ficheiros (*)</translation></message>
    <message><source>All levels</source><translation>Todos os níveis</translation></message>

    <message><source>An empty diagnostic snapshot does not mean that the overlay contains no changes.</source><translation>Um instantâneo de diagnóstico vazio não significa que a sobreposição não contém alterações.</translation></message>
    <message><source>Application path</source><translation>Caminho da aplicação</translation></message>
    <message><source>Application result</source><translation>Resultado da aplicação</translation></message>
    <message><source>Apply changes</source><translation>Aplicar alterações</translation></message>
    <message><source>Architecture</source><translation>Arquitetura</translation></message>
    <message><source>Automatic</source><translation>Automático</translation></message>
    <message><source>Automatic file staging</source><translation>Gravação automático de ficheiros pendentes</translation></message>
    <message><source>Automatic saving incomplete</source><translation>Gravação automático incompleto</translation></message>
    <message><source>Automatically save eligible staged files during normal shutdown and restart.</source><translation>Guardar automaticamente os ficheiros pendentes elegíveis durante o desligamento e o reinício normais.</translation></message>
    <message><source>Available</source><translation>Disponível</translation></message>
    <message><source>Back</source><translation>Voltar</translation></message>
    <message><source>Background service</source><translation>Serviço em segundo plano</translation></message>
    <message><source>Binding</source><translation>Vinculação</translation></message>
    <message><source>By drive letter</source><translation>Por letra da unidade</translation></message>
    <message><source>By volume ID</source><translation>Por ID do volume</translation></message>
    <message><source>Cancel reading</source><translation>Cancelar leitura</translation></message>
    <message><source>Changes already committed are retained.</source><translation>As alterações já confirmadas são mantidas.</translation></message>
    <message><source>Select the registry key to exclude from UWF protection, including its subkeys and values.</source><translation>Selecione a chave do Registo a excluir da proteção do UWF, incluindo as respetivas subchaves e valores.</translation></message>
    <message><source>Choose a location to save the file.</source><translation>Escolha um local para guardar o ficheiro.</translation></message>
    <message><source>Choose a location within the allowed drive or folder.</source><translation>Escolha um local dentro da unidade ou pasta permitida.</translation></message>
    <message><source>Choose another folder, or check access permissions and retry.</source><translation>Escolha outra pasta ou verifique as permissões de acesso e tente novamente.</translation></message>
    <message><source>Choose another key, or check access permissions and try again.</source><translation>Escolha outra chave ou verifique as permissões de acesso e tente novamente.</translation></message>
    <message><source>Clear filters</source><translation>Limpar filtros</translation></message>
    <message><source>Clear logs</source><translation>Limpar logs</translation></message>
    <message><source>Clear logs?</source><translation>Limpar os logs?</translation></message>
    <message><source>Collapse</source><translation>Recolher</translation></message>
    <message><source>Commit changes</source><translation>Confirmar alterações</translation></message>
    <message><source>Commit changes…</source><translation>Confirmar alterações…</translation></message>
    <message><source>Commit complete</source><translation>Confirmação concluída</translation></message>
    <message><source>Commit file changes?</source><translation>Confirmar as alterações do ficheiro?</translation></message>
    <message><source>Commit folder changes?</source><translation>Confirmar as alterações da pasta?</translation></message>
    <message><source>Commit registry key changes?</source><translation>Confirmar as alterações da chave do registo?</translation></message>
    <message><source>Commit registry value changes?</source><translation>Confirmar as alterações do valor do registo?</translation></message>
    <message><source>Commits staged files before normal shutdown and restart</source><translation>Confirma os ficheiros pendentes antes do desligamento e do reinício normais</translation></message>
    <message><source>Compatibility mode</source><translation>Modo de compatibilidade</translation></message>
    <message><source>Configuration applied</source><translation>Configuração aplicada</translation></message>
    <message><source>Configuration error</source><translation>Erro de configuração</translation></message>
    <message><source>Configured</source><translation>Configurado</translation></message>
    <message><source>Confirm before continuing</source><translation>Confirmar antes de continuar</translation></message>
    <message><source>Connected and authenticated</source><translation>Ligado e autenticado</translation></message>
    <message><source>Connecting…</source><translation>A ligar…</translation></message>
    <message><source>Contact author</source><translation>Contactar o autor</translation></message>
    <message><source>Continue editing</source><translation>Continuar editando</translation></message>
    <message><source>Continuing restart will discard changes in files that could not be saved.</source><translation>Continuar o reinício descartará as alterações dos ficheiros que não puderam ser guardados.</translation></message>
    <message><source>Continuing shutdown will discard changes in files that could not be saved.</source><translation>Continuar o desligamento descartará as alterações dos ficheiros que não puderam ser guardados.</translation></message>
    <message><source>Coordinate with Windows shutdown</source><translation>Coordenar com o desligamento do Windows</translation></message>
    <message><source>Copy all logs</source><translation>Copiar todos os logs</translation></message>
    <message><source>Copy full report</source><translation>Copiar relatório completo</translation></message>
    <message><source>Copy name</source><translation>Copiar nome</translation></message>
    <message><source>Copy path</source><translation>Copiar caminho</translation></message>
    <message><source>Copy selection</source><translation>Copiar seleção</translation></message>
    <message><source>Copy version</source><translation>Copiar versão</translation></message>
    <message><source>Counting staged files…</source><translation>Contando ficheiros pendentes…</translation></message>
    <message><source>Create</source><translation>Criar</translation></message>
    <message><source>Critical %1 MB</source><translation>Crítico %1 MB</translation></message>
    <message><source>Current filter</source><translation>Filtro atual</translation></message>
    <message><source>Current run</source><translation>Execução atual</translation></message>
    <message><source>Current selection</source><translation>Seleção atual</translation></message>
    <message><source>Current usage</source><translation>Uso atual</translation></message>
    <message><source>Date modified</source><translation>Data de modificação</translation></message>
    <message><source>Debug</source><translation>Depuração</translation></message>
    <message><source>Delete and commit this file?</source><translation>Excluir este ficheiro e confirmar a exclusão?</translation></message>
    <message><source>Delete and commit this folder?</source><translation>Excluir esta pasta e confirmar a exclusão?</translation></message>
    <message><source>Delete and commit this registry key?</source><translation>Excluir esta chave do registo e confirmar a exclusão?</translation></message>
    <message><source>Delete and commit this registry value?</source><translation>Excluir este valor do registo e confirmar a exclusão?</translation></message>

    <message><source>Direct restart?</source><translation>Reiniciar diretamente?</translation></message>
    <message><source>Direct shutdown?</source><translation>Desligar diretamente?</translation></message>
    <message><source>Discard changes</source><translation>Descartar alterações</translation></message>
    <message><source>Disk</source><translation>Disco</translation></message>
    <message><source>Domain Secret Key persistence</source><translation>Persistência da chave secreta de domínio</translation></message>
    <message><source>Done</source><translation>Concluído</translation></message>
    <message><source>Edit commands</source><translation>Editar comandos</translation></message>
    <message><source>Editable</source><translation>Editável</translation></message>
    <message><source>Enable the filter and restart to display live usage</source><translation>Ative o filtro e reinicie para exibir o uso real</translation></message>
    <message><source>Enhanced mode adds a background service that starts with Windows. Remove the service before moving or deleting the application.</source><translation>O modo avançado adiciona um serviço em segundo plano que arranca com o Windows. Remova o serviço antes de mover ou eliminar a aplicação.</translation></message>
    <message><source>Enhanced mode is unavailable. Use Safe shutdown or Safe restart to save staged files.</source><translation>O modo avançado está indisponível. Use Desligamento seguro ou Reinício seguro para guardar os ficheiros pendentes.</translation></message>
    <message><source>Enter HKLM\SOFTWARE\… and press Enter to navigate</source><translation>Introduza HKLM\SOFTWARE\… e prima Enter para navegar</translation></message>
    <message><source>Enter a file name</source><translation>Introduza um nome de ficheiro</translation></message>
    <message><source>Enter a valid file name.</source><translation>Introduza um nome de ficheiro válido.</translation></message>
    <message><source>Enter a valid file or folder name.</source><translation>Introduza um nome de ficheiro ou pasta válido.</translation></message>
    <message><source>Enter a valid folder name.</source><translation>Introduza um nome de pasta válido.</translation></message>
    <message><source>Error</source><translation>Erro</translation></message>
    <message><source>Error details</source><translation>Detalhes do erro</translation></message>
    <message><source>Error details and suggestions</source><translation>Detalhes do erro e sugestões</translation></message>
    <message><source>Expand</source><translation>Expandir</translation></message>
    <message><source>Export list</source><translation>Exportar lista</translation></message>
    <message><source>Failed to clear logs: %1</source><translation>Falha ao limpar os logs: %1</translation></message>
    <message><source>Failed to clear logs: unexpected error.</source><translation>Falha ao limpar os logs: erro inesperado.</translation></message>
    <message><source>File name</source><translation>Nome do ficheiro</translation></message>
    <message><source>File or folder</source><translation>Ficheiro ou pasta</translation></message>
    <message><source>File saving has started and cannot be canceled.</source><translation>O gravação dos ficheiros começou e não pode ser cancelado.</translation></message>
    <message><source>File saving incomplete</source><translation>Gravação de ficheiros incompleto</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after restart.</source><translation>O gravação dos ficheiros pendentes será ignorado. As alterações não confirmadas em caminhos protegidos serão perdidas após o reinício.</translation></message>
    <message><source>File staging will be skipped. Uncommitted changes on protected paths will be lost after shutdown.</source><translation>O gravação dos ficheiros pendentes será ignorado. As alterações não confirmadas em caminhos protegidos serão perdidas após o desligamento.</translation></message>
    <message><source>File type</source><translation>Tipo de ficheiro</translation></message>
    <message><source>Files and folders</source><translation>Ficheiros e pastas</translation></message>
    <message><source>Files and folders can be selected together.</source><translation>Ficheiros e pastas podem ser selecionados juntos.</translation></message>
    <message><source>Files in File staging will not be saved.</source><translation>Os ficheiros pendentes não serão guardados.</translation></message>
    <message><source>Filter is disabled</source><translation>O filtro está desativado</translation></message>
    <message><source>Fit columns to contents</source><translation>Ajustar colunas ao conteúdo</translation></message>
    <message><source>Folder address</source><translation>Endereço da pasta</translation></message>
    <message><source>Folder name</source><translation>Nome da pasta</translation></message>
    <message><source>Folders</source><translation>Pastas</translation></message>
    <message><source>Full configuration</source><translation>Configuração completa</translation></message>
    <message><source>Future writes to this key will not be protected by UWF.</source><translation>As futuras gravações nesta chave não serão protegidas pelo UWF.</translation></message>
    <message><source>Future writes to this path will not be protected by UWF.</source><translation>As futuras gravações neste caminho não serão protegidas pelo UWF.</translation></message>
    <message><source>Global registry exclusion · includes subkeys</source><translation>Exclusão global do registo · inclui subchaves</translation></message>
    <message><source>Go up one folder or choose another location.</source><translation>Suba uma pasta ou escolha outro local.</translation></message>
    <message><source>Home</source><translation>Pasta pessoal</translation></message>
    <message><source>Import configuration</source><translation>Importar configuração</translation></message>
    <message><source>Import creates pending changes. Use Review and apply to write them to the system.</source><translation>A importação cria alterações pendentes. Use Rever e aplicar para gravá-las no sistema.</translation></message>
    <message><source>Import results</source><translation>Resultados da importação</translation></message>
    <message><source>Imported</source><translation>Importado</translation></message>
    <message><source>Imported commands still need to be applied. Select a row to view the complete result.</source><translation>Os comandos importados ainda precisam ser aplicados. Selecione uma linha para ver o resultado completo.</translation></message>
    <message><source>Imported: %1</source><translation>Importado: %1</translation></message>
    <message><source>Information</source><translation>Informação</translation></message>
    <message><source>Item details</source><translation>Detalhes do item</translation></message>
    <message><source>Large staging lists may take longer to save.</source><translation>Guardar listas grandes de ficheiros pendentes pode demorar mais.</translation></message>
    <message><source>License and copyright</source><translation>Licença e direitos autorais</translation></message>
    <message><source>Line</source><translation>Linha</translation></message>
    <message><source>Local disk (%1)</source><translation>Disco local (%1)</translation></message>
    <message><source>Log level</source><translation>Nível de log</translation></message>
    <message><source>Log reading failed. Retry or check access to the log file.</source><translation>Falha ao ler os logs. Tente novamente ou verifique o acesso ao ficheiro de log.</translation></message>
    <message><source>Maximize</source><translation>Maximizar</translation></message>
    <message><source>Message details</source><translation>Detalhes da mensagem</translation></message>
    <message><source>Minimize</source><translation>Minimizar</translation></message>
    <message><source>Module</source><translation>Módulo</translation></message>
    <message><source>More</source><translation>Mais</translation></message>
    <message><source>Move</source><translation>Mover</translation></message>
    <message><source>If UWF protects the application drive, first make sure the application files will remain after a restart. Committing many staged files can make shutdown or restart take longer.</source><translation>Se o UWF protege a unidade onde está a aplicação, certifique-se primeiro de que os ficheiros da aplicação serão mantidos após um reinício. Guardar permanentemente muitos ficheiros pendentes pode prolongar o encerramento ou o reinício.</translation></message>
    <message><source>Multiple files can be selected.</source><translation>Vários ficheiros podem ser selecionados.</translation></message>
    <message><source>Name and location</source><translation>Nome e local</translation></message>
    <message><source>Named value: %1 · this value only</source><translation>Valor nomeado: %1 · somente este valor</translation></message>
    <message><source>Navigate to registry path</source><translation>Ir para o caminho do registo</translation></message>
    <message><source>New folder</source><translation>Nova pasta</translation></message>
    <message><source>New records will appear after refreshing.</source><translation>Novos registos aparecerão após atualizar.</translation></message>
    <message><source>Next-session configuration</source><translation>Configuração da próxima sessão</translation></message>
    <message><source>No files to commit</source><translation>Nenhum ficheiro para confirmar</translation></message>
    <message><source>No logs yet</source><translation>Ainda não há logs</translation></message>
    <message><source>No matching items</source><translation>Nenhum item correspondente</translation></message>
    <message><source>No matching logs</source><translation>Nenhum log correspondente</translation></message>
    <message><source>No overlay files found</source><translation>Nenhum ficheiro de sobreposição encontrado</translation></message>
    <message><source>No registry key selected</source><translation>Nenhuma chave do registo selecionada</translation></message>
    <message><source>None</source><translation>Nenhum</translation></message>
    <message><source>Not configured</source><translation>Não configurado</translation></message>
    <message><source>Not connected</source><translation>Não ligado</translation></message>
    <message><source>Not installed</source><translation>Não instalado</translation></message>
    <message><source>Not written to the system</source><translation>Não gravado no sistema</translation></message>
    <message><source>Off</source><translation>Desativado</translation></message>
    <message><source>On</source><translation>Ativado</translation></message>
    <message><source>One uwfmgr command per line</source><translation>Um comando uwfmgr por linha</translation></message>
    <message><source>Keep the application in a fixed folder that standard users cannot modify. Suggested location:</source><translation>Mantenha a aplicação numa pasta fixa que os utilizadores padrão não possam modificar. Local sugerido:</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after restart.</source><translation>Somente os ficheiros pendentes são guardados automaticamente. As demais alterações não confirmadas em caminhos protegidos serão perdidas após o reinício.</translation></message>
    <message><source>Only staged files are saved automatically. Other uncommitted changes in protected paths will be lost after shutdown.</source><translation>Somente os ficheiros pendentes são guardados automaticamente. As demais alterações não confirmadas em caminhos protegidos serão perdidas após o desligamento.</translation></message>
    <message><source>Open</source><translation>Abrir</translation></message>
    <message><source>Open folder</source><translation>Abrir pasta</translation></message>
    <message><source>Operating system</source><translation>Sistema operacional</translation></message>
    <message><source>Operation and security</source><translation>Operação e segurança</translation></message>
    <message><source>Original command</source><translation>Comando original</translation></message>
    <message><source>Overlay critical threshold reached. Save changes and safely restart.</source><translation>O limite crítico da sobreposição foi atingido. Salve as alterações e reinicie com segurança.</translation></message>
    <message><source>Overlay maximum size</source><translation>Tamanho máximo da sobreposição</translation></message>
    <message><source>Overlay type</source><translation>Tipo de sobreposição</translation></message>
    <message><source>Overlay usage</source><translation>Uso da sobreposição</translation></message>
    <message><source>Overlay warning threshold reached. Save changes and safely restart.</source><translation>O limite de aviso da sobreposição foi atingido. Salve as alterações e reinicie com segurança.</translation></message>
    <message><source>Overview</source><translation>Visão geral</translation></message>
    <message><source>Paste commands, or load them from a file…</source><translation>Cole comandos ou carregue de um ficheiro…</translation></message>
    <message><source>Pending changes</source><translation>Alterações pendentes</translation></message>
    <message><source>Pending removal</source><translation>Remoção pendente</translation></message>
    <message><source>Permissions and protection</source><translation>Permissões e proteção</translation></message>
    <message><source>Privileges</source><translation>Privilégios</translation></message>
    <message><source>Processed %1 / %2 items</source><translation>%1 / %2 itens processados</translation></message>
    <message><source>Processing result</source><translation>Resultado do processamento</translation></message>
    <message><source>Protected</source><translation>Protegido</translation></message>
    <message><source>Protected volumes</source><translation>Volumes protegidos</translation></message>
    <message><source>Protection and overlay</source><translation>Proteção e sobreposição</translation></message>
    <message><source>Quick access</source><translation>Acesso rápido</translation></message>
    <message><source>Raw report</source><translation>Relatório bruto</translation></message>
    <message><source>Read again</source><translation>Ler novamente</translation></message>
    <message><source>Read again to retrieve a diagnostic snapshot.</source><translation>Leia novamente para obter um instantâneo de diagnóstico.</translation></message>
    <message><source>Read the configuration to display live usage</source><translation>Leia a configuração para exibir o uso real</translation></message>
    <message><source>Read-only</source><translation>Somente leitura</translation></message>
    <message><source>Reading a large overlay can take longer. You can cancel at any time.</source><translation>Ler uma sobreposição grande pode demorar mais. Pode cancelar a qualquer momento.</translation></message>
    <message><source>Reading canceled</source><translation>Leitura cancelada</translation></message>
    <message><source>Reading files and subfolders in this location…</source><translation>Lendo ficheiros e subpastas neste local…</translation></message>
    <message><source>Reading folder</source><translation>Lendo pasta</translation></message>
    <message><source>Reading logs</source><translation>Lendo logs</translation></message>
    <message><source>Reading overlay</source><translation>Lendo sobreposição</translation></message>
    <message><source>Ready</source><translation>Pronto</translation></message>
    <message><source>Ready to save</source><translation>Pronto para guardar</translation></message>
    <message><source>Records from the current run will appear here when reading is complete.</source><translation>Os registos da execução atual aparecerão aqui quando a leitura terminar.</translation></message>
    <message><source>Recursive staging</source><translation>Preparação recursiva</translation></message>
    <message><source>Refresh folder</source><translation>Atualizar pasta</translation></message>
    <message><source>Registry key or persistence setting</source><translation>Chave do registo ou configuração de persistência</translation></message>
    <message><source>Remaining tasks can be canceled after the current target finishes.</source><translation>As tarefas restantes podem ser canceladas após o destino atual terminar.</translation></message>
    <message><source>Remove this key from the next-session exclusions.</source><translation>Remover esta chave das exclusões da próxima sessão.</translation></message>
    <message><source>Remove this path from the next-session exclusions.</source><translation>Remover este caminho das exclusões da próxima sessão.</translation></message>
    <message><source>Removed</source><translation>Removido</translation></message>
    <message><source>Rename</source><translation>Renomear</translation></message>
    <message><source>Rename…</source><translation>Renomear…</translation></message>
    <message><source>Repair required</source><translation>Reparação necessário</translation></message>
    <message><source>Replace file</source><translation>Substituir ficheiro</translation></message>
    <message><source>Replace the existing file?</source><translation>Substituir o ficheiro existente?</translation></message>
    <message><source>Required configuration</source><translation>Configuração necessária</translation></message>
    <message><source>Restart anyway</source><translation>Reiniciar mesmo assim</translation></message>
    <message><source>Restart will continue after saving. Keep the application open.</source><translation>O reinício continuará após guardar. Mantenha a aplicação aberta.</translation></message>
    <message><source>Restore</source><translation>Restaurar</translation></message>
    <message><source>Retry</source><translation>Tentar novamente</translation></message>
    <message><source>Review the failure details before continuing shutdown.</source><translation>Reveja os detalhes das falhas antes de continuar o desligamento.</translation></message>
    <message><source>Review the failure details before restarting.</source><translation>Reveja os detalhes das falhas antes de reiniciar.</translation></message>
    <message><source>Review the operation details. Read the current system state before retrying changes that may have been written.</source><translation>Reveja os detalhes da operação. Leia o estado atual do sistema antes de tentar novamente alterações que podem já ter sido gravadas.</translation></message>
    <message><source>Review the result for each target.</source><translation>Reveja o resultado de cada destino.</translation></message>
    <message><source>Review these changes before writing them to the system.</source><translation>Reveja estas alterações antes de gravá-las no sistema.</translation></message>
    <message><source>Running</source><translation>Em execução</translation></message>
    <message><source>Save</source><translation>Guardar</translation></message>
    <message><source>Save staged files to disk, then restart.</source><translation>Guardar os ficheiros pendentes no disco e reiniciar.</translation></message>
    <message><source>Save staged files to disk, then shut down.</source><translation>Guardar os ficheiros pendentes no disco e desligar.</translation></message>
    <message><source>Saving staged files</source><translation>A guardar ficheiros pendentes</translation></message>
    <message><source>Saving to disk</source><translation>A guardar no disco</translation></message>
    <message><source>Saving…</source><translation>A guardar…</translation></message>
    <message><source>Scanning staged paths</source><translation>Verificando caminhos pendentes</translation></message>
    <message><source>Search logs</source><translation>Pesquisar logs</translation></message>
    <message><source>Search messages or modules</source><translation>Pesquisar mensagens ou módulos</translation></message>
    <message><source>Search paths</source><translation>Pesquisar caminhos</translation></message>
    <message><source>Search this folder</source><translation>Pesquisar nesta pasta</translation></message>
    <message><source>Select a file or enter its name</source><translation>Selecione um ficheiro ou introduza o nome</translation></message>
    <message><source>Select a file to open.</source><translation>Selecione um ficheiro para abrir.</translation></message>
    <message><source>Select a folder.</source><translation>Selecione uma pasta.</translation></message>
    <message><source>Select a key to operate on its subtree, or a named value to operate on that value only.</source><translation>Selecione uma chave para operar sobre toda a subárvore ou um valor nomeado para operar somente sobre esse valor.</translation></message>
    <message><source>Select an available subkey</source><translation>Selecionar uma subchave disponível</translation></message>
    <message><source>Select files only, or open a folder to browse its contents.</source><translation>Selecione somente ficheiros ou abra uma pasta para explorar seu conteúdo.</translation></message>
    <message><source>Select files or folders</source><translation>Selecionar ficheiros ou pastas</translation></message>
    <message><source>Select folder</source><translation>Selecionar pasta</translation></message>
    <message><source>Select folders only.</source><translation>Selecione somente pastas.</translation></message>
    <message><source>Select registry key or value</source><translation>Selecionar chave ou valor do registo</translation></message>
    <message><source>Select registry key or value to delete</source><translation>Selecionar chave ou valor do registo para excluir</translation></message>
    <message><source>Select the current folder</source><translation>Selecionar a pasta atual</translation></message>
    <message><source>Select this file</source><translation>Selecionar este ficheiro</translation></message>
    <message><source>Select this registry key</source><translation>Selecionar esta chave do registo</translation></message>
    <message><source>Select this registry value</source><translation>Selecionar este valor do registo</translation></message>
    <message><source>Selected file</source><translation>Ficheiro selecionado</translation></message>
    <message><source>Selected folder</source><translation>Pasta selecionada</translation></message>
    <message><source>Selection</source><translation>Seleção</translation></message>
    <message><source>Service account</source><translation>Conta do serviço</translation></message>
    <message><source>Service configuration needs repair</source><translation>A configuração do serviço precisa de reparação</translation></message>
    <message><source>Service details</source><translation>Detalhes do serviço</translation></message>
    <message><source>Service name</source><translation>Nome do serviço</translation></message>
    <message><source>Service status</source><translation>Status do serviço</translation></message>
    <message><source>Service stopped</source><translation>Serviço parado</translation></message>
    <message><source>Shut down anyway</source><translation>Desligar mesmo assim</translation></message>
    <message><source>Shutdown will continue after saving. Keep the application open.</source><translation>O desligamento continuará após guardar. Mantenha a aplicação aberta.</translation></message>
    <message><source>Size</source><translation>Tamanho</translation></message>
    <message><source>Some changes could not be applied</source><translation>Não foi possível aplicar algumas alterações</translation></message>
    <message><source>Some files could not be saved</source><translation>Não foi possível guardar alguns ficheiros</translation></message>
    <message><source>Some hardware information could not be read. Other diagnostic information remains available to view and copy.</source><translation>Não foi possível ler algumas informações de hardware. As demais informações de diagnóstico continuam disponíveis para visualização e cópia.</translation></message>
    <message><source>Source code</source><translation>Código-fonte</translation></message>
    <message><source>Staged file</source><translation>Ficheiro pendente</translation></message>
    <message><source>Staged path</source><translation>Caminho pendente</translation></message>
    <message><source>Standard user</source><translation>Utilizador padrão</translation></message>
    <message><source>Starts automatically with Windows</source><translation>Inicia automaticamente com o Windows</translation></message>
    <message><source>Startup type</source><translation>Tipo de arranque</translation></message>
    <message><source>Status unavailable</source><translation>Status indisponível</translation></message>
    <message><source>Stopped</source><translation>Parado</translation></message>
    <message><source>Storage</source><translation>Armazenamento</translation></message>
    <message><source>System</source><translation>Sistema</translation></message>
    <message><source>System changes when UWF is first enabled</source><translation>Alterações no sistema ao ativar o UWF pela primeira vez</translation></message>
    <message><source>System memory</source><translation>Memória do sistema</translation></message>
    <message><source>System metadata</source><translation>Metadados do sistema</translation></message>
    <message><source>System metadata is for diagnosis only and cannot be committed.</source><translation>Os metadados do sistema servem somente para diagnóstico e não podem ser confirmados.</translation></message>
    <message><source>Takes effect after restart</source><translation>Entra em vigor após reiniciar</translation></message>
    <message><source>Target</source><translation>Destino</translation></message>
    <message><source>Terminal Services license persistence</source><translation>Persistência de licenças do Terminal Services</translation></message>
    <message><source>The UWF provider could not complete enumeration. This can occur with a large overlay or insufficient system resources.</source><translation>O provedor UWF não conseguiu concluir a enumeração. Isso pode ocorrer com uma sobreposição grande ou recursos do sistema insuficientes.</translation></message>
    <message><source>The configuration has been written to the system. Most changes take effect after restart.</source><translation>A configuração foi gravada no sistema. A maioria das alterações entra em vigor após reiniciar.</translation></message>
    <message><source>The configuration matches the next-session target. View or export commands in Full configuration.</source><translation>A configuração corresponde ao destino da próxima sessão. Veja ou exporte os comandos em Configuração completa.</translation></message>
    <message><source>The current log records will be cleared and cannot be restored.</source><translation>Os registos atuais serão apagados e não poderão ser recuperados.</translation></message>
    <message><source>The destination folder does not exist.</source><translation>A pasta de destino não existe.</translation></message>
    <message><source>The enhanced mode service status could not be read. Check service permissions and retry.</source><translation>Não foi possível ler o status do serviço do modo avançado. Verifique as permissões do serviço e tente novamente.</translation></message>
    <message><source>The list may omit files or include files already committed. Do not use it as a commit list.</source><translation>A lista pode omitir ficheiros ou incluir ficheiros já confirmados. Não a use como lista de confirmação.</translation></message>

    <message><source>The selected item no longer exists: %1</source><translation>O item selecionado não existe mais: %1</translation></message>
    <message><source>The service is installed but stopped. Start it to restore automatic saving.</source><translation>O serviço está instalado, mas parado. Inicie-o para restaurar o gravação automático.</translation></message>
    <message><source>This PC</source><translation>Este PC</translation></message>
    <message><source>This folder is empty</source><translation>Esta pasta está vazia</translation></message>

    <message><source>This report may contain account names, device identifiers, and file paths. Review it before sharing.</source><translation>Este relatório pode conter nomes de contas, identificadores de dispositivos e caminhos de ficheiros. Reveja antes de compartilhar.</translation></message>
    <message><source>Save this UTF-8 CMD script as a .cmd file and run it in Command Prompt. Export uses the same script.</source><translation>Guarde este script CMD em UTF-8 como um ficheiro .cmd e execute-o na Linha de Comandos. A exportação utiliza o mesmo script.</translation></message>
    <message><source>Try another keyword or log level.</source><translation>Tente outra palavra-chave ou nível de log.</translation></message>
    <message><source>Try another search term.</source><translation>Tente outro termo de pesquisa.</translation></message>
    <message><source>Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>O tipo e o tamanho máximo não podem ser alterados com o filtro ativo. Desative o filtro e reinicie primeiro.</translation></message>
    <message><source>UI agent</source><translation>Agente de interface</translation></message>
    <message><source>UTF-8 · %1 lines</source><translation>UTF-8 · %1 linhas</translation></message>
    <message><source>UWF configuration could not be read. Confirm that Unified Write Filter is installed in Windows, then read again.</source><translation>Não foi possível ler a configuração UWF. Confirme que o Unified Write Filter está instalado no Windows e leia novamente.</translation></message>
    <message><source>UWF feature</source><translation>Recurso UWF</translation></message>
    <message><source>UWF filter</source><translation>Filtro UWF</translation></message>
    <message><source>UWF is currently unavailable</source><translation>O UWF está indisponível de momento</translation></message>
    <message><source>UWF safe restart will run directly.</source><translation>O reinício seguro do UWF será executada diretamente.</translation></message>
    <message><source>UWF safe shutdown will run directly.</source><translation>O desligamento seguro do UWF será executado diretamente.</translation></message>
    <message><source>Unable to access this folder</source><translation>Não é possível aceder esta pasta</translation></message>
    <message><source>Unable to access this registry key</source><translation>Não é possível aceder esta chave do registo</translation></message>
    <message><source>Unable to read logs</source><translation>Não é possível ler os logs</translation></message>
    <message><source>Unable to read service status</source><translation>Não é possível ler o status do serviço</translation></message>
    <message><source>Unable to read the overlay right now</source><translation>Não é possível ler a sobreposição agora</translation></message>
    <message><source>Unable to read this folder.</source><translation>Não é possível ler esta pasta.</translation></message>
    <message><source>Unable to select this item</source><translation>Não é possível selecionar este item</translation></message>
    <message><source>Unavailable</source><translation>Indisponível</translation></message>
    <message><source>Undo removal</source><translation>Desfazer remoção</translation></message>
    <message><source>Unprotected</source><translation>Desprotegido</translation></message>
    <message><source>Unsuccessful items have not been saved. Select a row to view the full path and reason.</source><translation>Os itens com falha não foram guardados. Selecione uma linha para ver o caminho completo e o motivo.</translation></message>
    <message><source>Up one folder</source><translation>Subir uma pasta</translation></message>
    <message><source>Usage unavailable</source><translation>Uso indisponível</translation></message>
    <message><source>Use the whole key, including all subkeys</source><translation>Usar a chave inteira, incluindo todas as subchaves</translation></message>
    <message><source>Version %1</source><translation>Versão %1</translation></message>
    <message><source>View application records and diagnostic information.</source><translation>Veja os registos da aplicação e as informações de diagnóstico.</translation></message>
    <message><source>View commands for these changes</source><translation>Ver comandos destas alterações</translation></message>
    <message><source>View failure details</source><translation>Ver detalhes das falhas</translation></message>
    <message><source>View overlay</source><translation>Ver sobreposição</translation></message>
    <message><source>View the file diagnostic snapshot for %1.</source><translation>Veja o instantâneo de diagnóstico de ficheiros de %1.</translation></message>
    <message><source>Volume %1 binding</source><translation>Vinculação do volume %1</translation></message>
    <message><source>Volume %1 protection</source><translation>Proteção do volume %1</translation></message>
    <message><source>Volume identity</source><translation>Identidade do volume</translation></message>
    <message><source>Warning</source><translation>Aviso</translation></message>
    <message><source>Warning %1 MB</source><translation>Aviso %1 MB</translation></message>
    <message><source>Whole key and all subkeys · recursive</source><translation>Chave inteira e todas as subchaves · recursivo</translation></message>
    <message><source>Windows UWF provides write protection. This application configures and manages it.</source><translation>O Windows UWF fornece a proteção contra escrita. Esta aplicação a configura e gere.</translation></message>
    <message><source>Without enhanced mode, use Safe shutdown or Safe restart in this application to save staged files.</source><translation>Sem modo avançado, use Desligamento seguro ou Reinício seguro nesta aplicação para guardar os ficheiros pendentes.</translation></message>
    <message><source>Write protection</source><translation>Proteção contra escrita</translation></message>
    <message><source>Write these configuration changes to the system? Most changes take effect after restart.</source><translation>Gravar estas alterações de configuração no sistema? A maioria entra em vigor após reiniciar.</translation></message>
    <message><source>staged files</source><translation>ficheiros pendentes</translation></message>
    <message><source>uwfmgr commands</source><translation>comandos uwfmgr</translation></message>
    <message><source>Mark selected paths for removal. Takes effect after Apply.</source><translation>Marcar os caminhos selecionados para remoção. Entra em vigor após aplicar.</translation></message>
    <message><source>Remove from automatic file staging after Apply.</source><translation>Remover da preparação automática de ficheiros após aplicar.</translation></message>
    <message><source>Add to automatic file staging after Apply.</source><translation>Adicionar à preparação automática de ficheiros após aplicar.</translation></message>
    <message><source>Applying this plan clears the saved staging list and removes its registry exclusion when no longer required.</source><translation>Aplicar este plano limpa a lista de preparação guardada e remove a exclusão do Registo associada quando já não for necessária.</translation></message>
    <message><source>File staging changes must be applied in UWF Manager and cannot be exported as uwfmgr commands.</source><translation>As alterações à preparação de ficheiros têm de ser aplicadas no UWF Manager e não podem ser exportadas como comandos uwfmgr.</translation></message>
    <message><source>File staging is saved after Apply</source><translation>A preparação de ficheiros é guardada após aplicar</translation></message>
    <message><source>Overlay snapshot scope</source><translation>Âmbito do instantâneo da sobreposição</translation></message>
    <message><source>This list is for diagnostics only. It cannot determine which files must be committed.</source><translation>Esta lista destina-se apenas a diagnóstico. Não permite determinar que ficheiros devem ser confirmados.</translation></message>
    <message><source>NTFS volumes only</source><translation>Apenas volumes NTFS</translation></message>
    <message><source>Large overlays may load slowly or fail because of insufficient resources.</source><translation>A leitura de sobreposições grandes pode ser lenta ou falhar por falta de recursos.</translation></message>
    <message><source>Small files may be missing</source><translation>Podem faltar ficheiros pequenos</translation></message>
    <message><source>Files smaller than the disk cluster size (typically 4 KB) may not appear in the list.</source><translation>Os ficheiros menores do que o tamanho do cluster do disco (normalmente 4 KB) podem não aparecer na lista.</translation></message>
    <message><source>Does not indicate pending commits</source><translation>Não indica confirmações pendentes</translation></message>
    <message><source>Files already committed, excluded paths, and items affected by other operations may also appear.</source><translation>Também podem aparecer ficheiros já confirmados, caminhos excluídos e itens afetados por outras operações.</translation></message>

    <message><source>Command processing result</source><translation>Resultado do processamento do comando</translation></message>
    <message><source>Copy command</source><translation>Copiar comando</translation></message>
    <message><source> · %1 branches could not be read</source><translation> · Não foi possível ler %1 ramos</translation></message>
    <message><source>%1 branches could not be read. Continue with discovered content, or retry only the failed items.</source><translation>Não foi possível ler %1 ramos. Continue com o conteúdo encontrado ou tente novamente apenas os itens com falha.</translation></message>
    <message><source>%1 files · %2 folders</source><translation>%1 ficheiros · %2 pastas</translation></message>
    <message><source>%1 values · %2 keys</source><translation>%1 valores · %2 chaves</translation></message>
    <message><source>Attempts</source><translation>Tentativas</translation></message>
    <message><source>Cancel remaining tasks</source><translation>Cancelar tarefas restantes</translation></message>
    <message><source>Commit only successfully traversed content. Unread branches remain unchanged.</source><translation>Consolidar apenas o conteúdo percorrido com êxito. Os ramos ilegíveis permanecem inalterados.</translation></message>
    <message><source>Committing changes</source><translation>A consolidar alterações</translation></message>
    <message><source>Completed deletions and commits cannot be undone.</source><translation>As eliminações e consolidações concluídas não podem ser anuladas.</translation></message>
    <message><source>Continue…</source><translation>Continuar…</translation></message>
    <message><source>Delete and commit only discovered targets. Unread branches and their parent folders or keys are retained.</source><translation>Eliminar e consolidar apenas os destinos encontrados. Os ramos ilegíveis e as respetivas pastas ou chaves superiores serão mantidos.</translation></message>
    <message><source>Deleting and committing</source><translation>A eliminar e consolidar</translation></message>
    <message><source>Deletion and commit cannot be undone. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>A eliminação e a consolidação não podem ser anuladas. Apenas o âmbito selecionado é processado; a configuração UWF pendente não é aplicada.</translation></message>
    <message><source>Determining the scope. Unread branches are recorded while traversal continues elsewhere.</source><translation>A determinar o âmbito. Os ramos ilegíveis são registados e o percurso continua no restante conteúdo.</translation></message>
    <message><source>Export traversal records</source><translation>Exportar registo do percurso</translation></message>
    <message><source>No actionable content</source><translation>Sem conteúdo processável</translation></message>
    <message><source>No actionable targets were found. Nothing has been deleted or committed.</source><translation>Não foram encontrados destinos processáveis. Nada foi eliminado ou consolidado.</translation></message>
    <message><source>No changes have been committed.</source><translation>Nenhuma alteração foi consolidada</translation></message>
    <message><source>Nothing has been deleted or committed.</source><translation>Nada foi eliminado ou consolidado</translation></message>
    <message><source>Path</source><translation>Caminho</translation></message>
    <message><source>Permanently delete the selected scope and save the deletions to the protected volume.</source><translation>Eliminar permanentemente o âmbito selecionado e guardar as eliminações no volume protegido.</translation></message>
    <message><source>Preparation complete</source><translation>Preparação concluída</translation></message>
    <message><source>Preparing commit</source><translation>A preparar a consolidação</translation></message>
    <message><source>Preparing deletion and commit</source><translation>A preparar a eliminação e consolidação</translation></message>
    <message><source>Process only successfully traversed content</source><translation>Processar apenas o conteúdo percorrido com êxito</translation></message>

    <message><source>Reparse points are skipped to keep traversal within the selected scope.</source><translation>Os pontos de reanálise são ignorados para manter o percurso no âmbito selecionado.</translation></message>
    <message><source>Restarting cannot undo a commit. Only the selected scope is processed; pending UWF configuration is not applied.</source><translation>Reiniciar não anula uma consolidação. Apenas o âmbito selecionado é processado; a configuração UWF pendente não é aplicada.</translation></message>
    <message><source>Retry failed items</source><translation>Repetir os itens com falha</translation></message>
    <message><source>Save changes in the selected scope to the protected volume.</source><translation>Guardar as alterações do âmbito selecionado no volume protegido.</translation></message>
    <message><source>Stopping the operation</source><translation>A parar a operação</translation></message>
    <message><source>Successful and failed records are retained</source><translation>Os registos de êxitos e falhas foram mantidos</translation></message>
    <message><source>The folder is not empty; unprocessed content has been retained.</source><translation>A pasta não está vazia; o conteúdo não processado foi mantido.</translation></message>
    <message><source>The operation could not be completed</source><translation>Não foi possível concluir a operação</translation></message>
    <message><source>The registry key has new values; unprocessed content has been retained.</source><translation>A chave do Registo tem novos valores; o conteúdo não processado foi mantido.</translation></message>
    <message><source>The registry key still has subkeys; unprocessed content has been retained.</source><translation>A chave do Registo ainda tem subchaves; o conteúdo não processado foi mantido.</translation></message>
    <message><source>The target no longer exists.</source><translation>O destino já não existe.</translation></message>
    <message><source>Traversal complete</source><translation>Percurso concluído</translation></message>
    <message><source>Traversal completed with unread content</source><translation>Percurso concluído com conteúdo ilegível</translation></message>
    <message><source>Traversal records are retained and can be exported.</source><translation>O registo do percurso é mantido e pode ser exportado.</translation></message>
    <message><source>Traverse again</source><translation>Percorrer novamente</translation></message>
    <message><source>Traverse again, or close this window.</source><translation>Percorra novamente ou feche esta janela.</translation></message>
    <message><source>Traversing the folder</source><translation>A percorrer a pasta</translation></message>
    <message><source>Traversing the registry</source><translation>A percorrer o Registo</translation></message>
    <message><source>Unable to prepare the operation</source><translation>Não foi possível preparar a operação</translation></message>
    <message><source>View results</source><translation>Ver resultados</translation></message>

    <message><source>All changes committed</source><translation>Todas as alterações foram gravadas</translation></message>
    <message><source>Some changes committed</source><translation>Algumas alterações foram gravadas</translation></message>
    <message><source>No changes could be committed</source><translation>Não foi possível gravar nenhuma alteração</translation></message>
    <message><source>No changes to commit</source><translation>Não há alterações para gravar</translation></message>
    <message><source>Commit interrupted</source><translation>Gravação interrompida</translation></message>
    <message><source>%1 items were not processed.</source><translation>%1 itens não foram processados.</translation></message>
    <message><source>%1 registry operation(s) failed.</source><translation>Falharam %1 operações do registo.</translation></message>
    <message><source>The folder does not exist.</source><translation>A pasta não existe.</translation></message>
    <message><source>Access denied.</source><translation>Acesso negado.</translation></message>
    <message><source>An item with this name already exists.</source><translation>Já existe um item com este nome.</translation></message>
    <message><source>The folder could not be created. Check the name and permissions.</source><translation>Não foi possível criar a pasta. Verifique o nome e as permissões.</translation></message>
    <message><source>The item could not be renamed. Check the name and permissions.</source><translation>Não foi possível mudar o nome do item. Verifique o nome e as permissões.</translation></message>
    <message><source>Renaming is not supported by this browser.</source><translation>Este explorador não permite mudar nomes.</translation></message>
    <message><source>The file staging draft changed after this plan was opened. Review a new plan.</source><translation>O rascunho dos ficheiros preparados foi alterado após a abertura deste plano. Reveja um novo plano.</translation></message>
    <message><source>The saved file staging list changed after this plan was opened. Refresh and review the changes.</source><translation>A lista guardada de ficheiros preparados foi alterada após a abertura deste plano. Atualize e reveja as alterações.</translation></message>
    <message><source>The saved file staging list does not match the accepted plan.</source><translation>A lista guardada de ficheiros preparados não corresponde ao plano aceite.</translation></message>
    <message><source>Export configuration</source><translation>Exportar configuração</translation></message>
    <message><source>Save the current theme, language, and HUB visibility to UWF.ini next to this application.</source><translation>Guarde o tema, o idioma e a visibilidade atuais do HUB em UWF.ini, na mesma pasta desta aplicação.</translation></message>
    <message><source>Configuration saved</source><translation>Configuração guardada</translation></message>
    <message><source>Saved the current interface settings to:
%1</source><translation>As definições atuais da interface foram guardadas em:
%1</translation></message>
    <message><source>UWF.ini already exists. The existing file was not changed:
%1</source><translation>UWF.ini já existe. O ficheiro existente não foi alterado:
%1</translation></message>
    <message><source>Could not save UWF.ini:
%1
%2</source><translation>Não foi possível guardar UWF.ini:
%1
%2</translation></message>
    <message><source>Desktop</source><translation>Ambiente de Trabalho</translation></message>
    <message><source>Downloads</source><translation>Transferências</translation></message>
    <message><source>Documents</source><translation>Documentos</translation></message>
    <message><source>File staging stopped. Resolve the error before trying again.</source><translation>A gravação automática de ficheiros foi interrompida. Resolva o erro antes de tentar novamente.</translation></message>
</context>
</TS>
