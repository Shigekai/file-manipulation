# Processador de Imagens PGM

## Descrição

Programa em C que processa imagens PGM aplicando limiarização.

## Compilação

```bash
gcc -o imgdb main.c store.c pgm_io.c menu.c filters.c indexdb.c -std=c99 -Wall -Wextra
```

## Execução

```bash
./imgdb
```

## Conceito de Interface Aplicada ao Terminal (CLI)
O projeto possui uma Interface de Linha de Comando (CLI), implementada inteiramente no terminal. Este tipo de interface é controlada por texto, onde o usuário interage com o programa através de comandos e entradas de teclado.

O arquivo menu.c é o coração desta interface. Os conceitos aplicados são:

Menu Orientado a Texto: A função menu() apresenta uma lista de opções numeradas ao usuário. Isso guia o usuário sobre as ações possíveis, sendo uma abordagem simples e eficaz para programas de console.

Loop de Eventos Principal (Main Loop): A interface opera dentro de um loop infinito (for (;;) em menu.c). O programa exibe o menu, aguarda a entrada do usuário, executa a ação correspondente e, em seguida, repete o processo, exibindo o menu novamente. O loop só é quebrado quando o usuário escolhe a opção "Sair" (0).

Entrada e Saída Padrão (Standard I/O):

Saída: A função printf é usada para exibir todo o texto, incluindo o menu, os prompts (Caminho do PGM para importar: ) e as mensagens de status (Exportado com sucesso...).

Entrada: O programa utiliza uma combinação de scanf (para ler números, como a opção do menu) e fgets (para ler texto, como nomes de arquivos).

Tratamento do Buffer de Entrada: Um desafio comum em C é misturar scanf com fgets. Após uma leitura com scanf("%d", &op), o caractere de nova linha (\n) permanece no buffer de entrada, o que faria com que uma chamada subsequente a fgets lesse uma string vazia. O código lida com isso de forma robusta, limpando o buffer com o loop while(getchar()!='\n'); após cada scanf. Isso garante que a próxima leitura de texto funcione como esperado.

Fluxo Baseado em Condicionais: A lógica da interface é controlada por uma estrutura if-else if que verifica a opção escolhida pelo usuário (op) e chama a função apropriada (cmd_import, list_all, cmd_export). Isso funciona como um roteador simples, direcionando o fluxo do programa com base na interação do usuário.

Fluxo do Programa: Da Importação à Exportação com Filtros
Aqui está o passo a passo de como uma imagem PGM flui através do sistema, desde sua entrada até a saída processada.

Etapa 1: Importação de uma Imagem PGM
Seleção do Usuário: O usuário executa o programa e escolhe a opção 1) Importar PGM para o banco no menu principal.

Coleta de Dados: A função cmd_import() é chamada. Ela solicita ao usuário o caminho do arquivo PGM de entrada e um "nome de referência" para a imagem.

Leitura e Conversão do PGM: A função load_pgm_as_p5_bytes() (pgm_io.c) é invocada com o caminho do arquivo.

Ela abre o arquivo PGM e lê seu cabeçalho, identificando as dimensões (largura, altura) e o valor máximo de cinza (maxval).

Ela aloca um buffer de memória para armazenar os dados dos pixels.

Transformação para Binário: Se a imagem for P2 (ASCII), a função lê os valores de pixel como texto e os converte para seus equivalentes numéricos (uint8_t ou uint16_t), salvando-os no buffer. Se a imagem já for P5 (binária), ela simplesmente copia os dados de pixel diretamente para o buffer.

Ao final, a função retorna um ponteiro para este buffer com os dados da imagem em formato binário bruto, junto com seus metadados.

Armazenamento de Dados: A função cmd_import chama append_to_store() (de store.c) para anexar os dados do buffer ao final do arquivo bin/store.bin. Esta função retorna o offset (a posição em bytes) onde os dados da imagem foram gravados.

Indexação: Com o offset e os outros metadados em mãos, uma struct KeyEntry é preenchida. Em seguida, a função add_key_record() (indexdb.c) é chamada para escrever essa entrada de metadados no arquivo bin/index.bin. A imagem está agora oficialmente no banco de dados.

Limpeza: O buffer de memória alocado em load_pgm_as_p5_bytes é liberado com free(data).

Etapa 2: Exportação da Imagem com Filtro
Seleção do Usuário: O usuário escolhe a opção 3) Exportar por nome e, em seguida, seleciona uma operação (ex: 1 para Limiarização ou 2 para Negativação).

Busca no Índice: A função cmd_export() solicita o nome da imagem. Ela chama find_by_name_seq() (indexdb.c), que varre o arquivo bin/index.bin sequencialmente até encontrar a KeyEntry correspondente ao nome fornecido. Se encontrada, os metadados da imagem (incluindo seu offset no store.bin) são carregados.

Chamada para Exportação: A função principal de processamento, export_image() (filters.c), é chamada, recebendo os metadados da imagem, o caminho do arquivo de saída e o modo de filtro escolhido.

Leitura do Armazenamento: Dentro de export_image(), um novo buffer de memória é alocado. A função read_from_store() (de store.c) é usada para ler os dados brutos da imagem do bin/store.bin, usando o offset e o size da KeyEntry para saber exatamente onde ler e quantos bytes extrair.

Aplicação do Filtro: O fluxo então se bifurca com base no mode:

Modo 1 (Limiar): apply_threshold() é chamada. Ela itera sobre cada pixel no buffer. Para cada pixel, compara seu valor com o limiar (th). O pixel é redefinido para maxval (branco) se for maior ou igual ao limiar, ou para 0 (preto) caso contrário.

Modo 2 (Negativo): apply_negative() é chamada. Ela itera sobre cada pixel no buffer e substitui seu valor v pelo resultado de maxval - v.

Modo 0 (Sem modificação): Nenhum filtro é aplicado e os dados no buffer permanecem como foram lidos do store.bin.

Escrita do Novo Arquivo PGM:

export_image() abre o arquivo de saída em modo de escrita binária ("wb").

A função write_p5_header() (pgm_io.c) escreve o cabeçalho PGM (P5, dimensões, maxval) no novo arquivo.

Finalmente, o conteúdo do buffer (agora com os dados originais ou modificados pelo filtro) é escrito no arquivo de saída com fwrite.

Limpeza Final: Os buffers de memória são liberados com free(), e o arquivo é fechado. O controle retorna ao menu principal.
