<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pt_BR" sourcelanguage="en">
<context>
    <name>uwf</name>
    <message><source>Enable</source><translation>Ativar</translation></message>
    <message><source>Disable</source><translation>Desativar</translation></message>
    <message><source>enable</source><translation>ativar</translation></message>
    <message><source>disable</source><translation>desativar</translation></message>
    <message><source>Enabled</source><translation>Ativado</translation></message>
    <message><source>Disabled</source><translation>Desativado</translation></message>
    <message><source>OK</source><translation>OK</translation></message>
    <message><source>Cancel</source><translation>Cancelar</translation></message>
    <message><source>Close</source><translation>Fechar</translation></message>
    <message><source>Refresh</source><translation>Atualizar</translation></message>
    <message><source>Apply</source><translation>Aplicar</translation></message>
    <message><source>Add</source><translation>Adicionar</translation></message>
    <message><source>Remove</source><translation>Remover</translation></message>
    <message><source>Yes</source><translation>Sim</translation></message>
    <message><source>No</source><translation>Não</translation></message>
    <message><source>Excluded</source><translation>Excluído</translation></message>
    <message><source>Not excluded</source><translation>Não excluído</translation></message>
    <message><source>File…</source><translation>Arquivo…</translation></message>
    <message><source>Folder…</source><translation>Pasta…</translation></message>
    <message><source>volume ID</source><translation>ID do volume</translation></message>
    <message><source>drive letter</source><translation>letra da unidade</translation></message>
    <message><source>Drive letter</source><translation>Letra da unidade</translation></message>
    <message><source>Volume ID</source><translation>ID do volume</translation></message>
    <message><source>The current system "%1" (%2) is not a recognized supported Windows 10/11 edition. UWF Manager is running in compatibility mode and some features may be unavailable.</source><translation>O sistema “%1” (%2) não foi reconhecido como uma edição compatível do Windows 10/11. O UWF Manager está no modo de compatibilidade; alguns recursos podem estar indisponíveis.</translation></message>
    <message><source>Protection state of this volume in the current session (read-only).</source><translation>Estado de proteção deste volume na sessão atual (somente leitura).</translation></message>
    <message><source>Protect this volume in the next session. Writes to this volume are redirected to the overlay and discarded on reboot.</source><translation>Proteger este volume na próxima sessão. As gravações vão para a sobreposição e são descartadas ao reiniciar.</translation></message>
    <message><source>Protection:</source><translation>Proteção:</translation></message>
    <message><source>Current session</source><translation>Sessão atual</translation></message>
    <message><source>Next session</source><translation>Próxima sessão</translation></message>
    <message><source>The currently active session (read-only). Changes you make never take effect in this session.</source><translation>Sessão em execução (somente leitura). As alterações não entram em vigor nesta sessão.</translation></message>
    <message><source>The session that takes effect after a reboot. Changes you make take effect after the system restarts.</source><translation>Sessão após a reinicialização. As alterações entram em vigor ao reiniciar o sistema.</translation></message>
    <message><source>How UWF identifies this volume. Drive letter is simpler, but the binding breaks if the letter is reassigned (e.g. after adding or removing other disks). Volume ID stays stable across drive letter changes.</source><translation>Identificação do volume: a letra é simples, mas a associação se perde se ela mudar, por exemplo ao adicionar ou remover discos. O ID do volume permanece estável.</translation></message>
    <message><source>Bind by:</source><translation>Identificar por:</translation></message>
    <message><source>This volume is not supported by UWF: </source><translation>Volume não compatível com UWF: </translation></message>
    <message><source>Global settings</source><translation>Configurações globais</translation></message>
    <message><source>UWF filter state in the current session (read-only).</source><translation>Estado do filtro UWF na sessão atual (somente leitura).</translation></message>
    <message><source>Enable the UWF filter in the next session. Writes to protected volumes are redirected to the overlay and discarded on reboot.</source><translation>Ativar UWF na próxima sessão. As gravações nos volumes protegidos vão para a sobreposição e são descartadas ao reiniciar.</translation></message>
    <message><source>Filter</source><translation>Filtro</translation></message>
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
    <message><source>Triggers a warning-level event-log notification when overlay usage reaches this value. Must be lower than the critical threshold. Set to 0 to disable this event.</source><translation>Ao atingir este uso, registra um aviso no log de eventos. Deve ser menor que o limite crítico. 0 desativa o evento.</translation></message>
    <message><source>Critical threshold</source><translation>Limite crítico</translation></message>
    <message><source>Triggers a critical-level event-log notification when overlay usage reaches this value. Must be higher than the warning threshold. Set to 0 to disable this event.</source><translation>Ao atingir este uso, registra um evento crítico. Deve ser maior que o limite de aviso. 0 desativa o evento.</translation></message>
    <message><source>Used / total</source><translation>Usado / total</translation></message>
    <message><source>Overlay</source><translation>Sobreposição</translation></message>
    <message><source>&lt;span style='color:%1'&gt;■&lt;/span&gt; Used &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Warning &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Critical</source><translation>&lt;span style='color:%1'&gt;■&lt;/span&gt; Usado &amp;nbsp; &lt;span style='color:%2'&gt;■&lt;/span&gt; Aviso &amp;nbsp; &lt;span style='color:%3'&gt;■&lt;/span&gt; Crítico</translation></message>
    <message><source>UWF status unavailable: </source><translation>Status do UWF indisponível: </translation></message>
    <message><source>Administrator privileges are required to change UWF settings. Restart the program via right-click → "Run as administrator".</source><translation>Para alterar UWF, são necessários privilégios de administrador. Reabra o programa com o botão direito → “Executar como administrador”.</translation></message>
    <message><source>Unsupported drive type (only fixed local disks are supported).</source><translation>Tipo de unidade incompatível (somente discos locais fixos).</translation></message>
    <message><source>Volume capacity exceeds the UWF limit of 16 TiB for a single protected volume.</source><translation>O volume excede o limite UWF de 16 TiB por volume protegido.</translation></message>
    <message><source>%1 file system: this volume can be protected, but file exclusions and per-file commit are not supported.</source><translation>%1: proteção do volume disponível, mas sem exclusões nem gravação por arquivo.</translation></message>
    <message><source>Failed to read volume information.</source><translation>Falha: Leitura de informações do volume.</translation></message>
    <message><source>%1 free / %2</source><translation>%1 livres / %2</translation></message>
    <message><source>Commit</source><translation>Gravar em disco</translation></message>
    <message><source>Commit overlay changes to disk / registry. This action cannot be undone.</source><translation>Gravar em disco as alterações deste arquivo na sobreposição / Gravar alterações do Registro. Esta ação não pode ser desfeita.</translation></message>
    <message><source>Commit file changes…</source><translation>Gravar alterações do arquivo…</translation></message>
    <message><source>Pick a file and commit its overlay changes to disk.</source><translation>Selecionar arquivo para gravar em disco. Gravar em disco as alterações deste arquivo na sobreposição.</translation></message>
    <message><source>Commit folder changes…</source><translation>Gravar alterações da pasta…</translation></message>
    <message><source>Pick a folder and commit overlay changes for every file inside it to disk.</source><translation>Selecionar pasta para gravar em disco. Gravar em disco as alterações desta pasta na sobreposição. Todas as subpastas são incluídas.</translation></message>
    <message><source>Delete and commit file…</source><translation>Excluir o arquivo e gravar a exclusão em disco…</translation></message>
    <message><source>Pick a file to delete, and commit the deletion to disk.</source><translation>Selecionar arquivo para excluir permanentemente. Excluir o arquivo e gravar a exclusão em disco.</translation></message>
    <message><source>Delete and commit folder…</source><translation>Excluir a pasta e o conteúdo e gravar as exclusões em disco…</translation></message>
    <message><source>Pick a folder to delete (recursively, with everything inside it) and commit the deletions to disk.</source><translation>Selecionar pasta para excluir permanentemente. Excluir a pasta e o conteúdo e gravar as exclusões em disco. Todas as subpastas são incluídas.</translation></message>
    <message><source>Commit registry changes…</source><translation>Gravar alterações do Registro…</translation></message>
    <message><source>Enter a registry key (and optional value name) and commit changes to the registry. Leaving the value name empty commits every value under the key and all its subkeys (recursively).</source><translation>Informe a chave e, opcionalmente, o nome do valor. Sem nome, todos os valores da chave e das subchaves são gravados permanentemente.</translation></message>
    <message><source>Delete and commit registry…</source><translation>Gravar exclusão do Registro…</translation></message>
    <message><source>Enter a registry key (and optional value name) to delete, and commit the deletion to the registry. Leaving the value name empty deletes the whole subtree (the key plus all its values and subkeys, recursively).</source><translation>Informe a chave a excluir e um nome de valor opcional. Sem nome, toda a árvore é excluída permanentemente: chave, valores e subchaves.</translation></message>
    <message><source>File exclusions</source><translation>Exclusões de arquivos</translation></message>
    <message><source>Files and folders on this volume excluded from UWF protection. Double-click an entry to copy its path.</source><translation>Arquivos e pastas do volume excluídos da proteção UWF. Clique duas vezes em uma entrada para copiar o caminho.</translation></message>
    <message><source>Registry exclusions</source><translation>Exclusões do Registro</translation></message>
    <message><source>Registry exclusions are global: they are governed by the global UWF filter switch, not by this volume's protection state. The list is shared across all volumes and shown only once in the disk tabs. Double-click an entry to copy its path.</source><translation>As exclusões do Registro são globais e dependem do filtro UWF, não da proteção deste volume. A lista é compartilhada e aparece apenas uma vez nas abas de discos. Clique duas vezes para copiar o caminho.</translation></message>
    <message><source>File staging</source><translation>Arquivos para gravação automática</translation></message>
    <message><source>Files and folders in this list are committed automatically before safe shutdown or restart. Folders are processed recursively. While the list contains data, UWF Manager excludes HKLM\SOFTWARE\HsingYun\UWF Manager so the list persists across protected sessions.</source><translation>arquivos / pastas: Gravação automática antes do desligamento ou reinício seguro. Todas as subpastas são incluídas. Enquanto houver entradas, esta exclusão do Registro preserva a lista entre sessões protegidas: HKLM\SOFTWARE\HsingYun\UWF Manager</translation></message>
    <message><source>Search staged paths…</source><translation>Buscar caminhos de gravação automática…</translation></message>
    <message><source>Add files or a folder to the automatic commit list.</source><translation>Adicionar arquivos ou uma pasta à lista de gravação automática.</translation></message>
    <message><source>Select one or more files to commit automatically before safe shutdown or restart.</source><translation>Selecionar arquivo para gravar em disco. Gravação automática antes do desligamento ou reinício seguro.</translation></message>
    <message><source>Select a folder whose files will be committed recursively before safe shutdown or restart.</source><translation>Selecionar pasta para gravar em disco. Gravação automática antes do desligamento ou reinício seguro. Todas as subpastas são incluídas.</translation></message>
    <message><source>Remove the selected paths from automatic commit immediately.</source><translation>Remover imediatamente os caminhos selecionados da gravação automática.</translation></message>
    <message><source>Select files for automatic commit</source><translation>Selecionar arquivo para gravar em disco · Arquivos para gravação automática</translation></message>
    <message><source>Select a folder for automatic recursive commit</source><translation>Selecionar pasta para gravar em disco · Arquivos para gravação automática · Todas as subpastas são incluídas.</translation></message>
    <message><source>Cannot stage this path</source><translation>Não foi possível preparar este caminho</translation></message>
    <message><source>This path overlaps a UWF file exclusion. Remove the exclusion before adding the staged path:
%1</source><translation>O caminho se sobrepõe a uma exclusão UWF. Remova primeiro a exclusão:
%1</translation></message>
    <message><source>This path overlaps a file staging entry. Remove the staged path before adding the exclusion:
%1</source><translation>O caminho se sobrepõe à gravação automática. Remova primeiro a entrada:
%1</translation></message>
    <message><source>The path overlaps an existing file staging entry</source><translation>O caminho se sobrepõe a uma entrada de gravação automática</translation></message>
    <message><source>The registry exclusion is required while File staging contains saved entries</source><translation>Esta exclusão do Registro é necessária enquanto houver entradas de gravação automática</translation></message>
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
    <message><source>File staging is unavailable: %1</source><translation>Arquivos para gravação automática: %1</translation></message>
    <message><source>Committed automatically before safe shutdown or restart.</source><translation>Gravação automática antes do desligamento ou reinício seguro.</translation></message>
    <message><source>%1 staged entries · %2 files · %3 folders</source><translation>Arquivos para gravação automática: %1 · arquivos: %2 · pastas: %3</translation></message>
    <message><source>Required by UWF for File staging.</source><translation>Necessário para gravação automática UWF.</translation></message>
    <message><source>The UWF filter is currently disabled; no overlay changes have been accumulated, so there is nothing to commit.</source><translation>O filtro UWF está desativado. Não há alterações acumuladas na sobreposição para gravar.</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system.</source><translation>O sistema de arquivos %1 não permite gravação por arquivo.</translation></message>
    <message><source>This volume is not supported by UWF.</source><translation>Este volume não é compatível com UWF.</translation></message>
    <message><source>This volume does not support file commit. Only registry commit is available (registry exclusions are global).</source><translation>Este volume não permite gravação de arquivos. Apenas a gravação do Registro está disponível (exclusões globais).</translation></message>
    <message><source>Per-file commit is not supported on the %1 file system. Only registry commit is available (registry exclusions are global).</source><translation>O sistema de arquivos %1 não permite gravação por arquivo. Apenas a gravação do Registro está disponível (exclusões globais).</translation></message>
    <message><source>This volume is not currently protected by UWF; there are no file changes to commit.</source><translation>Este volume não está protegido pelo UWF; não há alterações de arquivos para gravar.</translation></message>
    <message><source>This volume is not currently protected by UWF. Only registry commit is available (registry exclusions are global).</source><translation>Este volume não está protegido pelo UWF; não há alterações de arquivos para gravar. Apenas a gravação do Registro está disponível (exclusões globais).</translation></message>
    <message><source>Select a file to commit to disk</source><translation>Selecionar arquivo para gravar em disco</translation></message>
    <message><source>Select a folder to commit to disk</source><translation>Selecionar pasta para gravar em disco</translation></message>
    <message><source>Select a file whose deletion you want to commit</source><translation>Selecionar arquivo para excluir permanentemente</translation></message>
    <message><source>Select a folder whose deletion you want to commit</source><translation>Selecionar pasta para excluir permanentemente</translation></message>
    <message><source>Commit registry changes</source><translation>Gravar alterações do Registro</translation></message>
    <message><source>Commit registry deletion</source><translation>Gravar exclusão do Registro</translation></message>
    <message><source>Key path:</source><translation>Chave · Caminho:</translation></message>
    <message><source>HKLM\Software\... — type or paste, press Enter to jump</source><translation>Digite ou cole HKLM\Software\... e pressione Enter</translation></message>
    <message><source>Name</source><translation>Nome</translation></message>
    <message><source>Data</source><translation>Dados</translation></message>
    <message><source>Key: %1</source><translation>Chave: %1</translation></message>
    <message><source>Value: %1</source><translation>Valor: %1</translation></message>
    <message><source>(none)</source><translation>(nenhum)</translation></message>
    <message><source>(Default)</source><translation>(Padrão)</translation></message>
    <message><source>(value not set)</source><translation>(valor não definido)</translation></message>
    <message><source>(empty)</source><translation>(vazio)</translation></message>
    <message><source>Preview unavailable (value is too large)</source><translation>Desconhecido (Valor: Arquivo muito grande)</translation></message>
    <message><source>(whole key recursive, including subkeys)</source><translation>(chave inteira e todas as subchaves)</translation></message>
    <message><source>Overlay files - %1</source><translation>Sobreposição · arquivos - %1</translation></message>
    <message><source>View overlay files</source><translation>Ver arquivos da sobreposição</translation></message>
    <message><source>Open a diagnostic view of files currently cached in the overlay for this volume.</source><translation>Ver arquivos da sobreposição. &lt;b&gt;Apenas um instantâneo de diagnóstico.&lt;/b&gt; Não use a lista para decidir o que gravar.</translation></message>
    <message><source>Overlay file listing is only supported on NTFS volumes.</source><translation>A listagem de arquivos da sobreposição exige volumes NTFS.</translation></message>
    <message><source>The UWF filter is currently disabled, so no volume has an overlay to inspect.</source><translation>O filtro UWF está desativado; nenhum volume tem sobreposição para examinar.</translation></message>
    <message><source>This volume is not currently protected by UWF, so it has no overlay to inspect.</source><translation>O volume não está protegido pelo UWF e não tem sobreposição para examinar.</translation></message>
    <message><source>&lt;b&gt;Diagnostic snapshot only.&lt;/b&gt; Don't use this list to decide what to commit.</source><translation>&lt;b&gt;Apenas um instantâneo de diagnóstico.&lt;/b&gt; Não use a lista para decidir o que gravar.</translation></message>
    <message><source>NTFS volumes only.</source><translation>A listagem de arquivos da sobreposição exige volumes NTFS.</translation></message>
    <message><source>Can be slow or fail when the overlay is large — memory and time grow with overlay size.</source><translation>Sobreposições grandes exigem mais memória e tempo; a operação pode ficar lenta ou falhar.</translation></message>
    <message><source>The list is not exact: files smaller than the disk cluster size (typically 4 KB) may be missing; earlier commits, files in excluded paths, and files affected by unrelated operations may appear.</source><translation>A lista é imprecisa: arquivos menores que um cluster (geralmente 4 KB) podem faltar; arquivos já gravados, excluídos da proteção ou afetados por outras operações podem aparecer.</translation></message>
    <message><source>Loading…</source><translation>Carregando…</translation></message>
    <message><source>Failed: %1</source><translation>Falha: %1</translation></message>
    <message><source>%1 file(s) in overlay</source><translation>Sobreposição · arquivos: %1</translation></message>
    <message><source>No files</source><translation>Nenhum arquivo</translation></message>
    <message><source>Page %1 / %2 · %3 file(s) total</source><translation>Página %1 / %2 · arquivos: %3</translation></message>
    <message><source>The WMI provider crashed while enumerating overlay files. This is a known instability of UWF_Overlay.GetOverlayFiles when the overlay is large or under I/O pressure.</source><translation>O provedor WMI falhou durante a enumeração. É uma instabilidade conhecida de UWF_Overlay.GetOverlayFiles com sobreposições grandes ou alta carga de E/S.</translation></message>
    <message><source>Out of memory or operation not supported by the provider. Overlay file enumeration only works on NTFS volumes and requires headroom; try again with a smaller overlay or after closing memory-heavy applications.</source><translation>Memória insuficiente ou operação não compatível. Exige NTFS e memória livre. Tente com uma sobreposição menor ou feche aplicativos que consomem muita memória.</translation></message>
    <message><source>Raw error: %1</source><translation>Código do erro: %1</translation></message>
    <message><source>Export to file…</source><translation>Exportar para arquivo…</translation></message>
    <message><source>Export overlay file list</source><translation>Exportar para arquivo… · Sobreposição · arquivos</translation></message>
    <message><source>Text files (*.txt);;All files (*)</source><translation>Arquivos de texto (*.txt);;Todos os arquivos (*)</translation></message>
    <message><source>Export failed</source><translation>Falha na exportação</translation></message>
    <message><source>Export finished</source><translation>Exportação concluída</translation></message>
    <message><source>Could not open file for writing: %1</source><translation>Falha: Abertura do arquivo (%1).</translation></message>
    <message><source>Could not write file: %1</source><translation>Falha: Gravação do arquivo (%1).</translation></message>
    <message><source>Saved %1 entries to:
%2</source><translation>Configuração: Gravação do arquivo · entradas: %1
%2</translation></message>
    <message><source>Cannot add an entire volume (%1) to the exclusion list.</source><translation>Não é possível excluir um volume inteiro (%1) da proteção.</translation></message>
    <message><source>The pagefile, swapfile and hibernation file cannot be excluded; UWF itself depends on these system files.</source><translation>Arquivos de paginação, swap e hibernação não podem ser excluídos da proteção: o UWF depende deles.</translation></message>
    <message><source>The entire \Windows directory cannot be excluded.</source><translation>Não é possível excluir o diretório inteiro da proteção: \Windows.</translation></message>
    <message><source>The entire \Windows\System32 directory cannot be excluded.</source><translation>Não é possível excluir o diretório inteiro da proteção: \Windows\System32.</translation></message>
    <message><source>The entire \Windows\System32\Drivers directory cannot be excluded.</source><translation>Não é possível excluir o diretório inteiro da proteção: \Windows\System32\Drivers.</translation></message>
    <message><source>This critical system file cannot be excluded: %1</source><translation>Este arquivo essencial do sistema não pode ser excluído da proteção: %1</translation></message>
    <message><source>The per-user registry file NTUSER.DAT cannot be excluded.</source><translation>O arquivo de Registro do usuário NTUSER.DAT não pode ser excluído da proteção.</translation></message>
    <message><source>Registry key cannot be empty.</source><translation>A chave do Registro não pode ficar vazia.</translation></message>
    <message><source>Registry key cannot start with a backslash.</source><translation>A chave do Registro não pode começar com barra invertida.</translation></message>
    <message><source>The path contains consecutive backslashes; this is not valid.</source><translation>O caminho contém barras invertidas consecutivas e é inválido.</translation></message>
    <message><source>Registry paths use backslash `\` as the separator; do not use forward slash `/`.</source><translation>Caminhos do Registro usam `\` como separador, não `/`.</translation></message>
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
    <message><source>Search file paths…</source><translation>Buscar caminhos de arquivos…</translation></message>
    <message><source>Search registry keys…</source><translation>Buscar chaves do Registro…</translation></message>
    <message><source>Add a file or folder to the exclusion list. Excluded entries are not protected by UWF.</source><translation>Adicionar arquivos ou pastas às exclusões. Itens excluídos não são protegidos pelo UWF.</translation></message>
    <message><source>Pick a file to add to the exclusion list.</source><translation>Selecionar arquivo para adicionar · Exclusões de arquivos.</translation></message>
    <message><source>Pick a folder (and all of its contents) to add to the exclusion list.</source><translation>Selecionar pasta para adicionar · Exclusões de arquivos. Todas as subpastas são incluídas.</translation></message>
    <message><source>Enter a registry key path to add to the exclusion list.</source><translation>Informe o caminho da chave do Registro a excluir.</translation></message>
    <message><source>Add a registry key to the exclusion list, or enable a persistence switch.</source><translation>Informe o caminho da chave do Registro a excluir. / Persistência.</translation></message>
    <message><source>Registry key…</source><translation>Registro · Chave…</translation></message>
    <message><source>Domain Secret Key (DomainSecretKey)</source><translation>Chave secreta do domínio (DomainSecretKey)</translation></message>
    <message><source>Terminal Services Client Access License (TSCAL)</source><translation>Licença de acesso de cliente dos Serviços de Terminal (TSCAL)</translation></message>
    <message><source>Persist the domain secret key (machine account password) across UWF sessions.</source><translation>Preservar o segredo do domínio (senha da conta de computador) entre sessões UWF.</translation></message>
    <message><source>Persist Terminal Services client access licenses across UWF sessions.</source><translation>Preservar licenças de acesso de cliente dos Serviços de Terminal entre sessões UWF.</translation></message>
    <message><source>· %1 persistence %2</source><translation>· Persistência %1: %2</translation></message>
    <message><source>✓ %1 persistence: %2</source><translation>✓ Persistência %1: %2</translation></message>
    <message><source>Remove selected</source><translation>Remover selecionados</translation></message>
    <message><source>Remove the selected entries from the exclusion list. Takes effect after Apply.</source><translation>Remover as exclusões selecionadas. Entra em vigor após aplicar.</translation></message>
    <message><source>Open containing folder</source><translation>Abrir pasta do arquivo</translation></message>
    <message><source>Copy file path</source><translation>Copiar caminho do arquivo</translation></message>
    <message><source>Copy registry path</source><translation>Copiar caminho do Registro</translation></message>
    <message><source>Commit folder changes to disk…</source><translation>Gravar em disco as alterações desta pasta na sobreposição…</translation></message>
    <message><source>Commit file changes to disk…</source><translation>Gravar em disco as alterações deste arquivo na sobreposição…</translation></message>
    <message><source>Copied to clipboard: </source><translation>Copiado para a área de transferência: </translation></message>
    <message><source>Select files to add to the exclusion list (multiple selection allowed)</source><translation>Selecionar arquivo para adicionar · Exclusões de arquivos. É possível selecionar vários arquivos.</translation></message>
    <message><source>Select a folder to add to the exclusion list</source><translation>Selecionar pasta para adicionar · Exclusões de arquivos</translation></message>
    <message><source>Add registry exclusion</source><translation>Adição de exclusão do Registro</translation></message>
    <message><source>The selected path %1 is not on volume %2, and therefore cannot be added as an exclusion for this volume.</source><translation>O caminho %1 não está no volume %2 e não pode ser adicionado às suas exclusões.</translation></message>
    <message><source>Path is not on this volume</source><translation>O caminho não pertence a este volume</translation></message>
    <message><source>Cannot add this exclusion</source><translation>Não foi possível adicionar esta exclusão</translation></message>
    <message><source>File: %1</source><translation>Arquivo… %1</translation></message>
    <message><source>Folder: %1</source><translation>Pasta… %1</translation></message>
    <message><source>Registry: %1</source><translation>Registro: %1</translation></message>
    <message><source>Excluded by default; change it anytime.</source><translation>Excluído por padrão; pode ser alterado a qualquer momento.</translation></message>
    <message><source>Current session: %1
Next session: %2</source><translation>Sessão atual: %1
Próxima sessão: %2</translation></message>
    <message><source>Pending change: %1</source><translation>Alterações pendentes: %1</translation></message>
    <message><source>%1 entries · %2 to add · %3 to remove in next session · %4 pending</source><translation>entradas: %1 · A adicionar: %2 · A remover na próxima sessão: %3 · Alterações pendentes: %4</translation></message>
    <message><source>Unified Write Filter (UWF) Manager</source><translation>Gerenciador do Filtro de Gravação Unificado (UWF)</translation></message>
    <message><source>Main toolbar</source><translation>Barra de ferramentas principal</translation></message>
    <message><source>Re-read the current session state and next-session configuration of UWF.</source><translation>Reler o estado atual e a configuração da próxima sessão do UWF.</translation></message>
    <message><source>Review and apply</source><translation>Revisar e aplicar</translation></message>
    <message><source>Review all pending changes and apply them in one batch. Most changes take effect after the next reboot.</source><translation>Revisar todas as alterações pendentes e aplicá-las em lote. A maioria das alterações entra em vigor após a próxima reinicialização.</translation></message>
    <message><source>Safe shutdown</source><translation>Desligamento seguro</translation></message>
    <message><source>Shut down safely, even when the UWF overlay is full.</source><translation>O sistema desligará com segurança pelo UWF. Disponível mesmo com a sobreposição UWF cheia.</translation></message>
    <message><source>Safe restart</source><translation>Reinicialização segura</translation></message>
    <message><source>Direct restart</source><translation>Reinicialização direta</translation></message>
    <message><source>Direct shutdown</source><translation>Desligamento direto</translation></message>
    <message><source>Restart safely, even when the UWF overlay is full.</source><translation>O sistema reiniciará com segurança pelo UWF. Disponível mesmo com a sobreposição UWF cheia.</translation></message>
    <message><source>Log</source><translation>Log</translation></message>
    <message><source>View the internal log accumulated during this session, for troubleshooting.</source><translation>Ver o log interno desta sessão para diagnóstico.</translation></message>
    <message><source>About</source><translation>Sobre</translation></message>
    <message><source>About this program.</source><translation>Sobre UWF Manager.</translation></message>
    <message><source>Switch display language</source><translation>Alterar idioma de exibição</translation></message>
    <message><source>Language</source><translation>Idioma</translation></message>
    <message><source>Theme</source><translation>Tema</translation></message>
    <message><source>Toggle light / dark theme</source><translation>Alternar tema claro/escuro</translation></message>
    <message><source>&lt;p&gt;A graphical front-end for managing the UWF filter state, overlay, and file / registry exclusions. Most changes take effect after the next reboot.&lt;/p&gt;&lt;p&gt;Source code: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;This program is released under the &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; the full license text is included in the LICENSE file shipped with this program.&lt;/p&gt;&lt;p&gt;This program is free software: you may redistribute it and / or modify it under the terms of the GPL v3. It is provided "as is", without any warranty.&lt;/p&gt;</source><translation>&lt;p&gt;Interface gráfica para gerenciar o filtro UWF, a sobreposição e as exclusões de arquivos e do Registro. A maioria das alterações entra em vigor após a próxima reinicialização.&lt;/p&gt;&lt;p&gt;Código-fonte: &lt;a href="%3"&gt;%3&lt;/a&gt;&lt;/p&gt;&lt;p&gt;Copyright © 2026 HsingYun &amp;lt;&lt;a href="mailto:%1"&gt;%1&lt;/a&gt;&amp;gt;&lt;/p&gt;&lt;p&gt;Licença: &lt;a href="%2"&gt;GNU General Public License v3.0&lt;/a&gt;; O texto completo da licença está no arquivo LICENSE incluído.&lt;/p&gt;&lt;p&gt;Software livre: pode ser redistribuído e modificado conforme a GPL v3. Fornecido “como está”, sem qualquer garantia.&lt;/p&gt;</translation></message>
    <message><source>About UWF Manager</source><translation>Sobre UWF Manager</translation></message>
    <message><source>System information</source><translation>Informações do sistema</translation></message>
    <message><source>Diagnostic snapshot</source><translation>Instantâneo de diagnóstico</translation></message>
    <message><source>System, security, UWF, enhanced mode, and recent application logs.</source><translation>Diagnóstico do sistema, segurança, UWF, modo avançado e logs recentes.</translation></message>
    <message><source>Read only</source><translation>Somente leitura</translation></message>
    <message><source>Diagnostic report</source><translation>Relatório de diagnóstico</translation></message>
    <message><source>Copied</source><translation>Copiado</translation></message>
    <message><source>View system, security, UWF, enhanced mode, and log diagnostics.</source><translation>Diagnóstico do sistema, segurança, UWF, modo avançado e logs recentes.</translation></message>
    <message><source>This report may contain account names, device identifiers, file paths, and recent application logs. Review it before sharing.</source><translation>O relatório pode conter contas, identificadores de dispositivos, caminhos e logs recentes. Revise antes de compartilhar.</translation></message>
    <message><source>&lt;p&gt;&lt;b&gt;This program depends on the Windows Unified Write Filter (UWF).&lt;/b&gt; UWF Manager does not perform write filtering itself; the actual write protection is provided by the UWF feature built into Windows. This program only configures and manages UWF, and requires UWF to be installed and enabled on the system.&lt;/p&gt;&lt;p&gt;When UWF is first enabled on a device, it makes the following changes to the system to improve UWF performance:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Paging files are disabled.&lt;/li&gt;&lt;li&gt;System Restore is disabled.&lt;/li&gt;&lt;li&gt;SuperFetch is disabled.&lt;/li&gt;&lt;li&gt;The file indexing service is turned off.&lt;/li&gt;&lt;li&gt;The defragmentation service is turned off.&lt;/li&gt;&lt;li&gt;Fast boot is disabled.&lt;/li&gt;&lt;li&gt;The BCD setting bootstatuspolicy is set to ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;After UWF is enabled, these settings can be changed as needed. For example, the paging file can be moved to an unprotected volume and paging re-enabled.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;Este programa depende do Filtro de Gravação Unificado do Windows (UWF).&lt;/b&gt; A proteção é feita pelo UWF integrado ao Windows. UWF Manager só configura e gerencia UWF, sem filtrar gravações. UWF precisa estar instalado e ativado no sistema.&lt;/p&gt;&lt;p&gt;Na primeira ativação do UWF, estas configurações do sistema mudam para melhorar o desempenho:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Arquivos de paginação são desativados.&lt;/li&gt;&lt;li&gt;A Restauração do Sistema é desativada.&lt;/li&gt;&lt;li&gt;SuperFetch é desativado.&lt;/li&gt;&lt;li&gt;O serviço de indexação de arquivos é desativado.&lt;/li&gt;&lt;li&gt;O serviço de desfragmentação é desativado.&lt;/li&gt;&lt;li&gt;A Inicialização Rápida é desativada.&lt;/li&gt;&lt;li&gt;A opção BCD bootstatuspolicy é definida como ignoreallfailures.&lt;/li&gt;&lt;/ul&gt;&lt;p&gt;Depois, esses ajustes podem ser alterados. Por exemplo, mova o arquivo de paginação para um volume não protegido e reative a paginação.&lt;/p&gt;</translation></message>
    <message><source>The target was not found; there is nothing in the overlay to commit.</source><translation>Destino não encontrado; não há nada para gravar na sobreposição.</translation></message>
    <message><source>Target is not on the physical volume / persistent registry — it exists only in the overlay (created under UWF protection, never committed). It will disappear on reboot; commit-delete is neither needed nor possible.</source><translation>O destino só existe na sobreposição: foi criado sob UWF e nunca gravado no disco ou Registro persistente. Desaparecerá ao reiniciar; gravar sua exclusão não é necessário nem possível.</translation></message>
    <message><source>A parameter was rejected by the system (invalid path or argument).</source><translation>O sistema rejeitou um parâmetro (caminho ou argumento inválido).</translation></message>
    <message><source>The operation failed. The target may be in use by another process (e.g. an Explorer window browsing the folder, or the file is open). Close any program holding it and try again.</source><translation>A operação falhou. Outro processo pode estar usando o destino, como uma pasta no Explorador ou um arquivo aberto. Feche esses programas e tente novamente.</translation></message>
    <message><source>The operation failed (see log for details).</source><translation>A operação falhou; veja os detalhes no log.</translation></message>
    <message><source>Operation rejected (code %1).</source><translation>Operação rejeitada (código %1).</translation></message>
    <message><source>Unknown cause.</source><translation>Desconhecido.</translation></message>
    <message><source>Commit canceled</source><translation>Gravação cancelada</translation></message>
    <message><source>Commit result</source><translation>Resultado da gravação</translation></message>
    <message><source>Succeeded</source><translation>Sucesso</translation></message>
    <message><source>Skipped</source><translation>Ignorado</translation></message>
    <message><source>Failed</source><translation>Falha</translation></message>
    <message><source>%1 succeeded; %2 skipped; %3 failed.</source><translation>Sucesso: %1; Ignorado: %2; Falha: %3.</translation></message>
    <message><source>
Canceled by user; %1 entries not processed.</source><translation>
Cancelado pelo usuário; %1 entradas não processadas.</translation></message>
    <message><source>Category</source><translation>Categoria</translation></message>
    <message><source>Path</source><translation>Caminho</translation></message>
    <message><source>Existed before</source><translation>Existia antes</translation></message>
    <message><source>Exists after</source><translation>Existe depois</translation></message>
    <message><source>Error code</source><translation>Código do erro</translation></message>
    <message><source>Reason</source><translation>Motivo</translation></message>
    <message><source>Previous page</source><translation>Página anterior</translation></message>
    <message><source>First page</source><translation>Primeira página</translation></message>
    <message><source>Last page</source><translation>Última página</translation></message>
    <message><source>Next page</source><translation>Próxima página</translation></message>
    <message><source>Page %1 / %2 · %3 entries total</source><translation>Página %1 / %2 · entradas: %3</translation></message>
    <message><source>Copy selected rows</source><translation>Copiar linhas selecionadas</translation></message>
    <message><source>Copy all</source><translation>Copiar tudo</translation></message>
    <message><source>This registry exclusion is required while File staging contains saved entries. Clear the File staging list before removing it.</source><translation>Esta exclusão do Registro é necessária enquanto houver entradas de gravação automática. Arquivos para gravação automática: Limpar → Remoção de exclusão do Registro.</translation></message>
    <message><source>0 lines</source><translation>0 linhas</translation></message>
    <message><source>%1 lines</source><translation>%1 linhas</translation></message>
    <message><source>Loading log entries…</source><translation>Carregando log…</translation></message>
    <message><source>No log entries</source><translation>Nenhuma entrada no log</translation></message>
    <message><source>Page %1 / %2 · %3 lines total</source><translation>Página %1 / %2 · linhas: %3</translation></message>
    <message><source>Copy current page</source><translation>Copiar página atual</translation></message>
    <message><source>Clear</source><translation>Limpar</translation></message>
    <message><source>%1 pending change(s) (not yet written to the system)</source><translation>Alterações pendentes: %1</translation></message>
    <message><source>No pending changes</source><translation>Nenhuma alteração pendente</translation></message>
    <message><source> (Also manages global registry exclusions.)</source><translation> (Exclusões do Registro · Configurações globais.)</translation></message>
    <message><source>Switch to protection settings, file exclusions, and file staging for volume %1.%2</source><translation>Volume %1: Proteção UWF / Exclusões de arquivos / Arquivos para gravação automática.%2</translation></message>
    <message><source>UWF namespace is not available</source><translation>UWF: Não compatível</translation></message>
    <message><source>Refreshed · %1 volumes</source><translation>Atualizar · Volume: %1</translation></message>
    <message><source>· Filter (global) %1</source><translation>· Filtro (UWF): %1</translation></message>
    <message><source>· Overlay type → %1</source><translation>· Sobreposição · Tipo → %1</translation></message>
    <message><source>· Overlay maximum size → %1 MB</source><translation>· Sobreposição · Tamanho máximo → %1 MB</translation></message>
    <message><source>· Overlay warning threshold → %1 MB</source><translation>· Sobreposição · Limite de aviso → %1 MB</translation></message>
    <message><source>· Overlay critical threshold → %1 MB</source><translation>· Sobreposição · Limite crítico → %1 MB</translation></message>
    <message><source>⚠ Type and maximum size cannot be changed while the filter is enabled. Disable the filter and reboot first.</source><translation>⚠ Para alterar o tipo e o tamanho máximo:
1. Desative o filtro acima.
2. Reinicie para desativá-lo efetivamente.
3. Altere a configuração.</translation></message>
    <message><source>· Volume %1 protection %2</source><translation>· Volume %1 · Proteção UWF: %2</translation></message>
    <message><source>· Volume %1 bind by → %2 (no CLI equivalent; this program only)</source><translation>· Volume %1 · Identificar por: %2 (sem equivalente na CLI; somente neste programa)</translation></message>
    <message><source>+ File exclusion  %1</source><translation>+ Exclusões de arquivos  %1</translation></message>
    <message><source>− File exclusion  %1</source><translation>− Exclusões de arquivos  %1</translation></message>
    <message><source>+ Registry exclusion  %1</source><translation>+ Exclusões do Registro  %1</translation></message>
    <message><source>− Registry exclusion  %1</source><translation>− Exclusões do Registro  %1</translation></message>
    <message><source>Filter (global) %1</source><translation>Filtro (UWF): %1</translation></message>
    <message><source>Overlay type → %1</source><translation>Sobreposição · Tipo → %1</translation></message>
    <message><source>Overlay maximum size → %1 MB</source><translation>Sobreposição · Tamanho máximo → %1 MB</translation></message>
    <message><source>Overlay warning threshold → %1 MB</source><translation>Sobreposição · Limite de aviso → %1 MB</translation></message>
    <message><source>Overlay critical threshold → %1 MB</source><translation>Sobreposição · Limite crítico → %1 MB</translation></message>
    <message><source>Volume %1 protection %2</source><translation>Volume %1 · Proteção UWF: %2</translation></message>
    <message><source>File exclusion %1</source><translation>Exclusões de arquivos: %1</translation></message>
    <message><source>Registry exclusion %1</source><translation>Exclusões do Registro: %1</translation></message>
    <message><source>Pending changes (%1)</source><translation>Alterações pendentes (%1)</translation></message>
    <message><source>Current session configuration</source><translation>Configuração da sessão atual</translation></message>
    <message><source>Review and apply changes</source><translation>Revisar e aplicar alterações</translation></message>
    <message><source>Below is the full configuration in uwfmgr command form. &lt;span style='color:%1'&gt;Pending changes&lt;/span&gt;, if any, are shown in a separate section first. Click &lt;span style='color:%2'&gt;Apply&lt;/span&gt; to write the changes to the system (most take effect after the next reboot).</source><translation>Configuração completa em comandos uwfmgr. As alterações pendentes aparecem primeiro em uma seção separada. &lt;span style='color:%1'&gt;Alterações pendentes&lt;/span&gt;. &lt;span style='color:%2'&gt;Aplicar&lt;/span&gt;: Clique para gravar as alterações no sistema. A maioria das alterações entra em vigor após a próxima reinicialização.</translation></message>
    <message><source>Confirm apply</source><translation>Confirmar aplicação</translation></message>
    <message><source>These changes will be &lt;span style='color:%1'&gt;written to the system&lt;/span&gt;; most take effect after the next reboot.&lt;br&gt;&lt;br&gt;Continue?</source><translation>&lt;span style='color:%1'&gt;Estas alterações serão gravadas no sistema&lt;/span&gt;. A maioria das alterações entra em vigor após a próxima reinicialização.&lt;br&gt;&lt;br&gt;Continuar?</translation></message>
    <message><source>Applied changes</source><translation>Alterações aplicadas</translation></message>
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
    <message><source>✓ Volume %1 added file exclusion: %2</source><translation>✓ Volume %1 · Adição de exclusão de arquivo: %2</translation></message>
    <message><source>✓ Volume %1 removed file exclusion: %2</source><translation>✓ Volume %1 · Remoção de exclusão de arquivo: %2</translation></message>
    <message><source>✓ Added registry exclusion: %1</source><translation>✓ Adição de exclusão do Registro: %1</translation></message>
    <message><source>✓ Removed registry exclusion: %1</source><translation>✓ Remoção de exclusão do Registro: %1</translation></message>
    <message><source>Time</source><translation>Hora</translation></message>
    <message><source>Level</source><translation>Nível</translation></message>
    <message><source>Tag</source><translation>Tag</translation></message>
    <message><source>Message</source><translation>Mensagem</translation></message>
    <message><source>Confirm safe shutdown?</source><translation>Desligamento seguro?</translation></message>
    <message><source>Confirm safe restart?</source><translation>Reinicialização segura?</translation></message>
    <message><source>Shut down</source><translation>Desligar</translation></message>
    <message><source>Restart</source><translation>Reiniciar</translation></message>
    <message><source>Continue shutdown</source><translation>Continuar desligamento</translation></message>
    <message><source>Continue restart</source><translation>Continuar reinicialização</translation></message>
    <message><source>The system will shut down safely through UWF.</source><translation>O sistema desligará com segurança pelo UWF.</translation></message>
    <message><source>The system will restart safely through UWF.</source><translation>O sistema reiniciará com segurança pelo UWF.</translation></message>
    <message><source>UWF protection</source><translation>Proteção UWF</translation></message>
    <message><source>This operation remains available even if the UWF overlay is full.</source><translation>Disponível mesmo com a sobreposição UWF cheia.</translation></message>
    <message><source>Calculating files for automatic commit…</source><translation>Calculando arquivos para gravação automática…</translation></message>
    <message><source>%1 file(s) will be committed automatically before this operation. Files outside currently protected volumes are skipped.</source><translation>%1 arquivos serão gravados automaticamente antes da operação. Arquivos fora dos volumes atualmente protegidos são ignorados.</translation></message>
    <message><source>Committing staged files</source><translation>Gravando arquivos preparados</translation></message>
    <message><source>The power action will continue after file staging completes.</source><translation>A ação de energia continuará após a gravação automática.</translation></message>
    <message><source>Processed %1 of %2 file(s).</source><translation>%1 de %2 arquivos processados.</translation></message>
    <message><source>Automatic file staging did not complete</source><translation>Gravação automática incompleta</translation></message>
    <message><source>Review the operation details before deciding whether to continue.</source><translation>Revise os detalhes antes de decidir continuar.</translation></message>
    <message><source>Operation details</source><translation>Detalhes da operação</translation></message>
    <message><source>No additional error details were provided.</source><translation>Nenhum detalhe adicional do erro.</translation></message>
    <message><source>Some changes may not be preserved</source><translation>Algumas alterações podem não ser preservadas</translation></message>
    <message><source>If you continue, the system will shut down without the files that could not be committed.</source><translation>Se continuar, o sistema desligará sem preservar os arquivos cuja gravação falhou.</translation></message>
    <message><source>If you continue, the system will restart without the files that could not be committed.</source><translation>Se continuar, o sistema reiniciará sem preservar os arquivos cuja gravação falhou.</translation></message>
    <message><source>Automatic file staging stopped unexpectedly:
%1</source><translation>A gravação automática foi interrompida inesperadamente:
%1</translation></message>
    <message><source>Automatic file staging stopped because of an unknown error.</source><translation>Gravação automática incompleta: Erro desconhecido.</translation></message>
    <message><source>Uncommitted changes will be lost</source><translation>Alterações não gravadas serão perdidas</translation></message>
    <message><source>Before continuing, save your work and commit any changes that you want to keep permanently.</source><translation>Antes de continuar, salve seu trabalho e grave permanentemente as alterações que deseja manter.</translation></message>
    <message><source>Confirm direct restart?</source><translation>Reinicialização direta?</translation></message>
    <message><source>Confirm direct shutdown?</source><translation>Desligamento direto?</translation></message>
    <message><source>The system will restart without committing files in File staging.</source><translation>Reiniciar. Os arquivos preparados não serão gravados antes da reinicialização.</translation></message>
    <message><source>The system will shut down without committing files in File staging.</source><translation>Desligar. Os arquivos preparados não serão gravados antes do desligamento.</translation></message>
    <message><source>File staging will be skipped</source><translation>A gravação automática será ignorada</translation></message>
    <message><source>Files in File staging will not be committed before restart.</source><translation>Os arquivos preparados não serão gravados antes da reinicialização.</translation></message>
    <message><source>Files in File staging will not be committed before shutdown.</source><translation>Os arquivos preparados não serão gravados antes do desligamento.</translation></message>
    <message><source>Direct restart skips File staging commits.</source><translation>Reinicialização direta: A gravação automática será ignorada.</translation></message>
    <message><source>Direct shutdown skips File staging commits.</source><translation>Desligamento direto: A gravação automática será ignorada.</translation></message>
    <message><source>File staging was skipped because a direct power action was selected.</source><translation>Gravação automática ignorada porque foi escolhida uma ação de energia direta.</translation></message>
    <message><source>Safe shutdown failed</source><translation>Desligamento seguro: Falha</translation></message>
    <message><source>Shutdown failed: %1</source><translation>Desligar: Falha (%1)</translation></message>
    <message><source>Safe restart failed</source><translation>Reinicialização segura: Falha</translation></message>
    <message><source>Direct restart failed</source><translation>Reinicialização direta: Falha</translation></message>
    <message><source>Direct shutdown failed</source><translation>Desligamento direto: Falha</translation></message>
    <message><source>Restart failed: %1</source><translation>Reiniciar: Falha (%1)</translation></message>
    <message><source>The file staging list could not be read:
%1</source><translation>Não foi possível ler a lista de gravação automática:
%1</translation></message>
    <message><source>The file staging list could not be read because of an unknown error.</source><translation>Não foi possível ler a lista de gravação automática: Erro desconhecido.</translation></message>
    <message><source>Discovered files: %1 · Committed: %2 · Skipped files: %3 · Skipped entries: %4 · Failed: %5</source><translation>arquivos: %1 · Sucesso: %2 · Ignorado (arquivos): %3 · Ignorado (entradas): %4 · Falha: %5</translation></message>
    <message><source>File staging preparation</source><translation>Arquivos para gravação automática: Detalhes da operação</translation></message>
    <message><source>Reparse points are not supported.</source><translation>Pontos de nova análise não são compatíveis.</translation></message>
    <message><source>Only absolute paths on local volumes can be staged.</source><translation>Somente caminhos absolutos em volumes locais podem ser adicionados.</translation></message>
    <message><source>Only absolute paths on local volumes can be added to automatic file staging:
%1</source><translation>Somente caminhos absolutos em volumes locais podem ser adicionados.
%1</translation></message>
    <message><source>The staged path could not be inspected.</source><translation>Não foi possível examinar o caminho preparado.</translation></message>
    <message><source>The staged file now refers to a different path type.</source><translation>O caminho do arquivo preparado agora se refere a outro tipo.</translation></message>
    <message><source>The staged folder now refers to a different path type.</source><translation>O caminho da pasta preparada agora se refere a outro tipo.</translation></message>
    <message><source>The staged folder could not be enumerated completely.</source><translation>Não foi possível enumerar completamente a pasta preparada.</translation></message>
    <message><source>The path has no drive letter.</source><translation>O caminho não tem letra de unidade.</translation></message>
    <message><source>A volume root cannot be processed recursively.</source><translation>A raiz do volume não pode ser processada recursivamente.</translation></message>
    <message><source>The UWF provider rejected the commit.</source><translation>O provedor UWF rejeitou a gravação.</translation></message>
    <message><source>The commit failed with an unknown error.</source><translation>Falha na gravação: Erro desconhecido.</translation></message>
    <message><source>%1 additional failure(s) were written to the log.</source><translation>Mais %1 falhas foram registradas no log.</translation></message>
    <message><source>Commit failed</source><translation>Falha na gravação</translation></message>
    <message><source>The path has no drive letter; cannot identify the target volume.</source><translation>O caminho não tem letra de unidade. Identificação do volume de destino: Falha.</translation></message>
    <message><source>Failed to read volume information: %1</source><translation>Falha: Leitura de informações do volume (%1)</translation></message>
    <message><source>No current-session record found for volume %1.</source><translation>Nenhum registro da sessão atual para o volume %1.</translation></message>
    <message><source>This path is in the file exclusion list.
Exclusion: %1</source><translation>O caminho está na lista de exclusões de arquivos.
Exclusão: %1</translation></message>
    <message><source>No files were found under %1.</source><translation>Nenhum arquivo encontrado em %1.</translation></message>
    <message><source>Commit to disk</source><translation>Gravar em disco</translation></message>
    <message><source>Committing…</source><translation>Gravando…</translation></message>
    <message><source>This action cannot be undone.</source><translation>Esta ação não pode ser desfeita.</translation></message>
    <message><source>Continue</source><translation>Continuar</translation></message>
    <message><source>Delete and commit</source><translation>Excluir e gravar</translation></message>
    <message><source>Commit this file's overlay changes to disk</source><translation>Gravar em disco as alterações deste arquivo na sobreposição</translation></message>
    <message><source>Commit this folder's overlay changes to disk</source><translation>Gravar em disco as alterações desta pasta na sobreposição</translation></message>
    <message><source>%1 files in this folder and all its subfolders will be committed.</source><translation>arquivos: %1. Todas as subpastas são incluídas.</translation></message>
    <message><source>Delete this file, and commit the deletion to disk</source><translation>Excluir o arquivo e gravar a exclusão em disco</translation></message>
    <message><source>Delete this folder and its contents, and commit the deletions to disk</source><translation>Excluir a pasta e o conteúdo e gravar as exclusões em disco</translation></message>
    <message><source>%1 files and %2 subfolders will be deleted.</source><translation>arquivos: %1; pastas: %2. Excluir e gravar.</translation></message>
    <message><source>Commit this registry value to disk</source><translation>Gravar este valor do Registro em disco</translation></message>
    <message><source>Commit this registry key and its whole subtree to disk</source><translation>Gravar esta chave do Registro e toda a subárvore em disco</translation></message>
    <message><source>%1 values in this key and all its subkeys will be committed.</source><translation>Valor: %1. (chave inteira e todas as subchaves)</translation></message>
    <message><source>Delete this registry value, and commit the deletion to disk</source><translation>Excluir este valor do Registro e gravar a exclusão em disco</translation></message>
    <message><source>Delete this registry key and its whole subtree, and commit the deletions to disk</source><translation>Excluir esta chave do Registro e a subárvore e gravar as exclusões</translation></message>
    <message><source>Scanning registry keys…</source><translation>Enumeração de chaves do Registro…</translation></message>
    <message><source>Scanning registry keys…
%1 key(s) found</source><translation>Enumeração de chaves do Registro…
Chave: %1</translation></message>
    <message><source>Registry keys that will be deleted:</source><translation>Registro · Chave · Excluir e gravar:</translation></message>
    <message><source>Copy current entry</source><translation>Copiar entrada atual</translation></message>
    <message><source>%1 keys and all values they contain will be recursively deleted.</source><translation>%1 chaves e todos os seus valores serão excluídos recursivamente.</translation></message>
    <message><source>Commit file deletion failed</source><translation>Falha: Gravação da exclusão do arquivo</translation></message>
    <message><source>This path does not exist, so there is nothing to delete.</source><translation>O caminho não existe; não há nada para excluir.</translation></message>
    <message><source>This key is in the registry exclusion list.
Exclusion: %1</source><translation>A chave está na lista de exclusões do Registro.
Exclusão: %1</translation></message>
    <message><source>Failed to read registry filter: %1</source><translation>Falha: Leitura do filtro do Registro (%1)</translation></message>
    <message><source>No current-session registry filter record found.</source><translation>Nenhum registro do filtro do Registro para a sessão atual.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to commit.</source><translation>O valor do Registro não existe; não há nada para gravar.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to commit.</source><translation>A chave do Registro não existe; não há nada para gravar.</translation></message>
    <message><source>This registry value does not exist, so there is nothing to delete.</source><translation>O valor do Registro não existe; não há nada para excluir.</translation></message>
    <message><source>This registry key does not exist, so there is nothing to delete.</source><translation>A chave do Registro não existe; não há nada para excluir.</translation></message>
    <message><source>Export commands…</source><translation>Exportar comandos…</translation></message>
    <message><source>Export commands to file</source><translation>Exportar comandos para arquivo</translation></message>
    <message><source>Exported %1 commands to:
%2</source><translation>Exportar para arquivo… · Comando: %1
%2</translation></message>
    <message><source>Import</source><translation>Importar</translation></message>
    <message><source>Paste, type, or load a script of uwfmgr commands and turn each line into a pending UI change. Nothing is written to the system until you click "Review and apply".</source><translation>uwfmgr: Digite ou cole um comando por linha. Carregue textos, logs ou scripts (.txt, .bat, .ps1); as linhas com este comando serão adicionadas. A importação prepara alterações. Apenas “Revisar e aplicar” na barra de ferramentas as grava no sistema.</translation></message>
    <message><source>Import uwfmgr commands</source><translation>uwfmgr · Importar</translation></message>
    <message><source>&lt;p&gt;Paste or type &lt;b&gt;uwfmgr&lt;/b&gt; commands below; one command per line. Supported categories: &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;Use &lt;b&gt;Load from file…&lt;/b&gt; to pick any text-like file (logs, scripts, .txt, .bat, .ps1); lines containing &lt;code&gt;uwfmgr&lt;/code&gt; will be appended to the box.&lt;/p&gt;&lt;p&gt;Clicking &lt;b&gt;Import&lt;/b&gt; turns each command into a pending UI change — &lt;b&gt;nothing is written to the system yet&lt;/b&gt;. Use &lt;b&gt;Review and apply&lt;/b&gt; in the toolbar to commit them.&lt;/p&gt;</source><translation>&lt;p&gt;&lt;b&gt;uwfmgr&lt;/b&gt;: Digite ou cole um comando por linha. &lt;code&gt;filter&lt;/code&gt; · &lt;code&gt;overlay&lt;/code&gt; · &lt;code&gt;volume&lt;/code&gt; · &lt;code&gt;file&lt;/code&gt; · &lt;code&gt;registry&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Carregar de arquivo…&lt;/b&gt;: Carregue textos, logs ou scripts (.txt, .bat, .ps1); as linhas com este comando serão adicionadas. &lt;code&gt;uwfmgr&lt;/code&gt;.&lt;/p&gt;&lt;p&gt;&lt;b&gt;Importar&lt;/b&gt;: &lt;b&gt;A importação prepara alterações. Apenas “Revisar e aplicar” na barra de ferramentas as grava no sistema.&lt;/b&gt; &lt;b&gt;Revisar e aplicar&lt;/b&gt;.&lt;/p&gt;</translation></message>
    <message><source>Load from file…</source><translation>Carregar de arquivo…</translation></message>
    <message><source>Load recommended configuration</source><translation>Carregar configuração recomendada</translation></message>
    <message><source>Choose recommended configuration</source><translation>Escolher configuração recomendada</translation></message>
    <message><source>Select the recommended configuration groups to append. You can review or delete individual commands before importing.</source><translation>Selecione os grupos recomendados a adicionar. Revise ou exclua comandos individuais antes de importar.</translation></message>
    <message><source>From Microsoft official documentation.</source><translation>Da documentação oficial da Microsoft.</translation></message>
    <message><source>Microsoft recommended UWF exclusions</source><translation>Exclusões UWF recomendadas pela Microsoft</translation></message>
    <message><source>Review these recommendations before importing; folders should exist before UWF accepts file exclusions.</source><translation>Revise antes de importar: as pastas devem existir para o UWF aceitar as exclusões.</translation></message>
    <message><source>Source: Microsoft official UWF documentation, including common write filter exclusions and antimalware support.</source><translation>Fonte: documentação oficial Microsoft UWF sobre exclusões comuns e suporte antimalware.</translation></message>
    <message><source>Customer Experience Improvement Program (CEIP)</source><translation>Programa de Aperfeiçoamento da Experiência do Usuário (CEIP)</translation></message>
    <message><source>CEIP: persist policy opt-in state</source><translation>CEIP: Preservar permanentemente: Estado de adesão por política</translation></message>
    <message><source>CEIP: persist local opt-in state</source><translation>CEIP: Preservar permanentemente: Estado de adesão local</translation></message>
    <message><source>CEIP: persist upload-disable flag</source><translation>CEIP: Preservar permanentemente: Sinalizador de envio desativado</translation></message>
    <message><source>Background Intelligent Transfer Service (BITS)</source><translation>Serviço de Transferência Inteligente em Segundo Plano (BITS)</translation></message>
    <message><source>BITS: persist downloader queue files</source><translation>BITS: Preservar permanentemente: Arquivos da fila de download</translation></message>
    <message><source>BITS: persist transfer state index</source><translation>BITS: Preservar permanentemente: Índice do estado de transferência</translation></message>
    <message><source>Network profiles and policies</source><translation>Perfis e políticas de rede</translation></message>
    <message><source>Wireless network GPO policy</source><translation>Política GPO de rede sem fio</translation></message>
    <message><source>Wired network GPO policy</source><translation>Política GPO de rede com fio</translation></message>
    <message><source>Wireless network GPO policy files</source><translation>Política GPO de rede sem fio · arquivos</translation></message>
    <message><source>Wired network GPO policy files</source><translation>Política GPO de rede com fio · arquivos</translation></message>
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
    <message><source>Defender: persist product files and updates</source><translation>Defender: Preservar permanentemente: Arquivos do produto e atualizações</translation></message>
    <message><source>Defender: persist ProgramData signatures and state</source><translation>Defender: Preservar permanentemente: Assinaturas e estado em ProgramData</translation></message>
    <message><source>Defender: persist Windows Update log</source><translation>Defender: Preservar permanentemente: Windows Update · Log</translation></message>
    <message><source>Defender: persist MpCmdRun log</source><translation>Defender: Preservar permanentemente: MpCmdRun · Log</translation></message>
    <message><source>Defender: persist product registry state</source><translation>Defender: Preservar permanentemente: Estado do produto no Registro</translation></message>
    <message><source>Defender: persist WdBoot service state</source><translation>Defender: Preservar permanentemente: WdBoot · Estado do serviço</translation></message>
    <message><source>Defender: persist WdFilter service state</source><translation>Defender: Preservar permanentemente: WdFilter · Estado do serviço</translation></message>
    <message><source>Defender: persist WdNisSvc service state</source><translation>Defender: Preservar permanentemente: WdNisSvc · Estado do serviço</translation></message>
    <message><source>Defender: persist WdNisDrv service state</source><translation>Defender: Preservar permanentemente: WdNisDrv · Estado do serviço</translation></message>
    <message><source>Defender: persist WinDefend service state</source><translation>Defender: Preservar permanentemente: WinDefend · Estado do serviço</translation></message>
    <message><source>System Center Endpoint Protection</source><translation>System Center Endpoint Protection</translation></message>
    <message><source>SCEP: persist client program files</source><translation>SCEP: Preservar permanentemente: Arquivos do programa cliente</translation></message>
    <message><source>SCEP: persist Windows Update log</source><translation>SCEP: Preservar permanentemente: Windows Update · Log</translation></message>
    <message><source>SCEP: persist MpCmdRun log</source><translation>SCEP: Preservar permanentemente: MpCmdRun · Log</translation></message>
    <message><source>SCEP: persist antimalware signatures and state</source><translation>SCEP: Preservar permanentemente: Assinaturas e estado antimalware</translation></message>
    <message><source>SCEP: persist antimalware registry state</source><translation>SCEP: Preservar permanentemente: Estado antimalware no Registro</translation></message>
    <message><source>Choose files containing uwfmgr commands</source><translation>Escolher arquivos com comandos uwfmgr</translation></message>
    <message><source>All files (*);;Text files (*.txt *.bat *.ps1 *.log *.cmd)</source><translation>Todos os arquivos (*);;Arquivos de texto (*.txt *.bat *.ps1 *.log *.cmd)</translation></message>
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
    <message><source>Status</source><translation>Status</translation></message>
    <message><source>Command</source><translation>Comando</translation></message>
    <message><source>Detail</source><translation>Detalhes</translation></message>
    <message><source>Import failed</source><translation>Falha na importação</translation></message>
    <message><source>Internal error: no applier registered.</source><translation>Erro interno: nenhum aplicador registrado.</translation></message>
    <message><source>Nothing to import</source><translation>Nenhum comando para importar</translation></message>
    <message><source>No uwfmgr commands found in the input.</source><translation>Nenhum comando uwfmgr encontrado na entrada.</translation></message>
    <message><source>Applied: %1</source><translation>Aplicado: %1</translation></message>
    <message><source>Duplicates: %1</source><translation>Duplicado: %1</translation></message>
    <message><source>Unsupported: %1</source><translation>Não compatível: %1</translation></message>
    <message><source>Cumulative after %1 batch(es):</source><translation>Acumulado após %1 lotes:</translation></message>
    <message><source>── Batch %1 ──</source><translation>── Lote %1 ──</translation></message>
    <message><source>Applied</source><translation>Aplicado</translation></message>
    <message><source>Duplicate</source><translation>Duplicado</translation></message>
    <message><source>Unsupported</source><translation>Não compatível</translation></message>
    <message><source>File too large</source><translation>Arquivo muito grande</translation></message>
    <message><source>File %1 is larger than 5 MB and was not parsed. Please filter it manually first.</source><translation>O arquivo %1 tem mais de 5 MB e não foi analisado. Filtre-o manualmente primeiro.</translation></message>
    <message><source>Cannot read file</source><translation>Não foi possível ler o arquivo</translation></message>
    <message><source>Could not open file %1: %2</source><translation>Falha: Abertura do arquivo (%1): %2</translation></message>
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
    <message><source>Missing registry key argument</source><translation>Argumento ausente: Registro · Chave</translation></message>
    <message><source>Unclosed double quote in command</source><translation>Aspas duplas não fechadas no comando</translation></message>
    <message><source>Unexpected extra argument: %1</source><translation>Argumento extra inesperado: %1</translation></message>
    <message><source>Unsupported uwfmgr command (cannot be mapped to a UI action)</source><translation>Comando uwfmgr não compatível (sem ação correspondente na interface)</translation></message>
    <message><source>Queued as a pending %1 change</source><translation>Alterações pendentes: %1</translation></message>
    <message><source>Already in the target state — no-op</source><translation>Já está no estado desejado; nenhuma ação necessária</translation></message>
    <message><source>Path is not on this volume, or this volume does not support file exclusions (e.g. exFAT / ReFS)</source><translation>O caminho está fora do volume ou o volume não permite exclusões (ex.: exFAT / ReFS)</translation></message>
    <message><source>Rejected by UWF's blacklist (system file / Windows / pagefile / etc.)</source><translation>Rejeitado pela lista de bloqueio UWF (arquivo de sistema, Windows, paginação etc.)</translation></message>
    <message><source>Same command was already issued earlier in this batch</source><translation>O mesmo comando já foi processado neste lote</translation></message>
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
    <message><source>file exclusion</source><translation>Exclusões de arquivos</translation></message>
    <message><source>registry exclusion</source><translation>Exclusões do Registro</translation></message>
    <message><source>Registry exclusions are only available on the registry tab, which is not present</source><translation>As exclusões do Registro exigem a aba Registro, que não está presente</translation></message>
    <message><source>Unsupported command</source><translation>Comando: Não compatível</translation></message>
    <message><source>Exit</source><translation>Sair</translation></message>
    <message><source>Show or hide the overlay hub.</source><translation>HUB: Mostrar ou ocultar.</translation></message>
    <message><source>UWF: Enabled</source><translation>UWF: Ativado</translation></message>
    <message><source>UWF: Disabled</source><translation>UWF: Desativado</translation></message>
    <message><source>UWF status unavailable</source><translation>UWF: Desconhecido</translation></message>
    <message><source>Used %1 MB / Total %2 MB</source><translation>Usado %1 MB / %2 MB</translation></message>
    <message><source>Enhanced mode</source><translation>Modo avançado</translation></message>
    <message><source>Coordinate automatic file staging with Windows shutdown and restart.</source><translation>Coordenar a gravação automática com o desligamento e a reinicialização do Windows.</translation></message>
    <message><source>Enhanced mode installs an automatically started LocalSystem service. It launches UWF Manager after Windows starts and attempts to commit changes to user-configured staged files before shutdown or restart, improving compatibility for applications that conflict with UWF.</source><translation>Instala um serviço LocalSystem automático. Ele inicia o UWF Manager com o Windows e tenta gravar os arquivos preparados antes de desligar ou reiniciar, melhorando a compatibilidade com aplicativos em conflito com UWF.</translation></message>
    <message><source>System changes</source><translation>Alterações no sistema</translation></message>
    <message><source>Creates the automatic service UWFManagerEnhanced and binds it to the current executable path</source><translation>Criar o serviço automático UWFManagerEnhanced vinculado ao executável atual</translation></message>
    <message><source>Writes and commits the service configuration under HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced</source><translation>Configuração: Registro → Gravar em disco (HKLM\SYSTEM\CurrentControlSet\Services\UWFManagerEnhanced)</translation></message>
    <message><source>Starts UWF Manager automatically after the next Windows startup; moving the executable can break automatic startup</source><translation>Iniciar UWF Manager na próxima inicialização do Windows; mover o executável pode impedir o início automático</translation></message>
    <message><source>Commits eligible staged files during normal Windows shutdown and restart</source><translation>Gravar arquivos preparados elegíveis durante o desligamento ou reinício normal do Windows</translation></message>
    <message><source>Operational and security considerations</source><translation>Considerações operacionais e de segurança</translation></message>
    <message><source>Registers a service in the system registry when enhanced mode is enabled, so the application no longer remains fully portable</source><translation>Registra um serviço no Registro do sistema; o aplicativo deixa de ser totalmente portátil</translation></message>
    <message><source>A large number of staged files can significantly extend shutdown or restart; allow UWF Manager enough time to preserve user changes</source><translation>Muitos arquivos podem prolongar o desligamento ou reinício; permita tempo suficiente para preservar as alterações</translation></message>
    <message><source>Enhanced mode runs this application (UWF Manager) as SYSTEM. If a non-administrator tampers with the application or replaces it with a malicious binary, privileges could be elevated to SYSTEM. Enable enhanced mode only from an administrator-protected location (recommended location: %ProgramFiles%\UWF Manager\Service\UWF.exe)</source><translation>O modo avançado executa UWF Manager como SYSTEM. Se um não administrador adulterar ou substituir o programa por um executável malicioso, poderá obter privilégios SYSTEM. Ative apenas em um local protegido por permissões de administrador (recomendado: %ProgramFiles%\UWF Manager\Service\UWF.exe).</translation></message>
    <message><source>Enhanced mode status could not be read</source><translation>Falha: Leitura do estado do modo avançado</translation></message>
    <message><source>The enhanced mode status read failed because of an unknown error.</source><translation>Falha: Leitura do estado do modo avançado (Erro desconhecido).</translation></message>
    <message><source>The current service state could not be read:
%1</source><translation>Falha: Leitura do estado do serviço:
%1</translation></message>
    <message><source>The current service state could not be read because of an unknown error.</source><translation>Falha: Leitura do estado do serviço (Erro desconhecido).</translation></message>
    <message><source>Status: Disabled</source><translation>Status: Desativado</translation></message>
    <message><source>Status: Enabled</source><translation>Status: Ativado</translation></message>
    <message><source>Status: Service stopped</source><translation>Status: serviço parado</translation></message>
    <message><source>Status: Repair required</source><translation>Status: reparo necessário</translation></message>
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
    <message><source>Stop and delete the enhanced mode service, including any remaining service registry data.</source><translation>Parar e excluir o serviço avançado, incluindo dados residuais no Registro.</translation></message>
    <message><source>The service is absent, but its service registry data remains.</source><translation>O serviço não existe, mas restam dados no Registro.</translation></message>
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
    <message><source>File staging completed</source><translation>Arquivos para gravação automática: Sucesso</translation></message>
    <message><source>File staging failed</source><translation>Arquivos para gravação automática: Falha</translation></message>
    <message><source>All staged targets were processed.</source><translation>Todos os destinos preparados foram processados.</translation></message>
    <message><source>All staged targets were processed; one or more operations failed.</source><translation>Todos os destinos foram processados; uma ou mais operações falharam.</translation></message>
    <message><source>Show main window</source><translation>Mostrar janela principal</translation></message>
    <message><source>Restore default position</source><translation>Restaurar posição padrão</translation></message>
    <message><source>Hide overlay hub</source><translation>Ocultar HUB da sobreposição</translation></message>
    <message><source>Exit application</source><translation>Sair do aplicativo</translation></message>
    <message><source>✘ Failed to connect to the system</source><translation>✘ Falha: Conexão ao sistema</translation></message>
    <message><source>Registry access failed</source><translation>Falha: Acesso ao Registro</translation></message>
    <message><source>Failed to read registry values: %1</source><translation>Falha: Leitura de valores do Registro (%1)</translation></message>
    <message><source>Unexpected error while enumerating overlay files.</source><translation>Enumeração de arquivos da sobreposição: Erro desconhecido.</translation></message>
    <message><source>✘ Failed to %1 filter</source><translation>✘ Falha: Ação no filtro: %1</translation></message>
    <message><source>✘ Failed to read filter state</source><translation>✘ Falha: Leitura do estado do filtro</translation></message>
    <message><source>✘ Failed to set warning threshold</source><translation>✘ Falha: Definição do limite de aviso</translation></message>
    <message><source>✘ Failed to set critical threshold</source><translation>✘ Falha: Definição do limite crítico</translation></message>
    <message><source>✘ Failed to read overlay state</source><translation>✘ Falha: Leitura do estado da sobreposição</translation></message>
    <message><source>✘ Failed to set overlay type</source><translation>✘ Falha: Definição do tipo de sobreposição</translation></message>
    <message><source>✘ Failed to set maximum size</source><translation>✘ Falha: Definição do tamanho máximo</translation></message>
    <message><source>✘ Failed to read overlay configuration</source><translation>✘ Falha: Leitura da configuração da sobreposição</translation></message>
    <message><source>✘ Volume %1: failed to register with UWF</source><translation>✘ Falha: Registro do volume %1 no UWF</translation></message>
    <message><source>✘ Failed to %1 protection on volume %2</source><translation>✘ Falha: Proteção do volume %2: %1</translation></message>
    <message><source>✘ Failed to set binding for volume %1</source><translation>✘ Falha: Definição da identificação do volume %1</translation></message>
    <message><source>✘ Volume %1 failed to add file exclusion %2</source><translation>✘ Volume %1 · Falha: Adição de exclusão de arquivo %2</translation></message>
    <message><source>✓ Volume %1 file exclusion already absent: %2</source><translation>✓ Exclusão de arquivo já ausente no volume %1: %2</translation></message>
    <message><source>✘ Volume %1 failed to remove file exclusion %2</source><translation>✘ Volume %1 · Falha: Remoção de exclusão de arquivo %2</translation></message>
    <message><source>✘ Failed to read volume configuration</source><translation>✘ Falha: Leitura da configuração do volume</translation></message>
    <message><source>✘ Failed to add registry exclusion %1</source><translation>✘ Falha: Adição de exclusão do Registro %1</translation></message>
    <message><source>✘ Failed to remove registry exclusion %1</source><translation>✘ Falha: Remoção de exclusão do Registro %1</translation></message>
    <message><source>✘ Failed to update registry persistence switches</source><translation>✘ Falha: Atualização das opções de persistência do Registro</translation></message>
    <message><source>✘ Failed to read registry filter</source><translation>✘ Falha: Leitura do filtro do Registro</translation></message>
    <message><source>UWF is not available</source><translation>UWF: Não compatível</translation></message>
    <message><source>Unknown error while reading the target state.</source><translation>Leitura do estado do destino: Erro desconhecido.</translation></message>
    <message><source>The target state could not be read before the operation: %1</source><translation>Não foi possível ler o estado do destino antes da operação: %1</translation></message>
    <message><source>The target no longer exists, so there is nothing to delete.</source><translation>O caminho não existe; não há nada para excluir.</translation></message>
    <message><source>The operation failed with an unknown error.</source><translation>Erro desconhecido.</translation></message>
    <message><source>The operation result could not be confirmed because the target state reread failed: %1</source><translation>Não foi possível confirmar o resultado porque a releitura do estado falhou: %1</translation></message>
    <message><source>The provider accepted the deletion, but the target still exists after the authoritative reread.</source><translation>O provedor aceitou a exclusão, mas a verificação autoritativa confirma que o destino ainda existe.</translation></message>
    <message><source>Unknown</source><translation>Desconhecido</translation></message>
    <message><source>Failed to load logs: %1</source><translation>Falha: Carregamento do log (%1)</translation></message>
    <message><source>Failed to load logs: unexpected error.</source><translation>Falha: Carregamento do log (Erro desconhecido).</translation></message>
    <message><source>: unknown error</source><translation>: Erro desconhecido</translation></message>
    <message><source>Failed to read the target state: %1</source><translation>Falha: Leitura do estado do destino (%1)</translation></message>
    <message><source>Failed to read the target state because of an unknown error.</source><translation>Falha: Leitura do estado do destino (Erro desconhecido).</translation></message>
    <message><source>Failed to resolve the target volume: %1</source><translation>Falha: Identificação do volume de destino (%1)</translation></message>
    <message><source>Failed to enumerate registry values: %1</source><translation>Falha: Enumeração de valores do Registro (%1)</translation></message>
    <message><source>Failed to enumerate registry keys: %1</source><translation>Falha: Enumeração de chaves do Registro (%1)</translation></message>
    <message><source>Delete and commit failed</source><translation>Excluir e gravar: Falha</translation></message>
    <message><source>Failed to resolve the volume for path %1: %2</source><translation>Falha: Identificação do volume de destino (%1): %2</translation></message>
    <message><source>%1/%2 slots</source><translation>Slots usados: %1/%2</translation></message>
</context>
</TS>
