#include "headers.h"
#include "menu.h"
#include "../io/pgm.h"
#include "database.h"
#include "index.h"
#include "handleImages.h"
#include "externalSort.h"
#include "../algos/bTree.h"

//Funções estáticas auxiliares do menu

// Lê string do stdin removendo quebras de linha
static int readLine(char *buffer, size_t size) {
    if (!fgets(buffer, size, stdin)) {
        return 0;
    }
    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 1;
}

// Limpa buffer de entrada após scanf
static void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


//Método reparoveitável que permite que o usuário interaja com os filtros disponíveis
static int printFilters(uint16_t maxValue, FilterMode *outMode, uint32_t *outThreshold) {
    printf("\nEscolha o filtro:\n");
    printf("  0) Sem modificação\n");
    printf("  1) Limiarização\n");
    printf("  2) Negativo\n");
    printf("Opção: ");
    
    int modeChoice = 0;
    if (scanf("%d", &modeChoice) != 1) {
        printf("Entrada inválida.\n");
        clearInputBuffer();
        return 0;
    }
    clearInputBuffer();
    
    if (modeChoice < 0 || modeChoice > 2) {
        printf("Opção inválida.\n");
        return 0;
    }
    
    *outMode = (FilterMode)modeChoice;
    *outThreshold = 0;

    if (*outMode == FILTER_THRESHOLD) {
        printf("Valor de limiar (0 a %u): ", maxValue);
        if (scanf("%u", outThreshold) != 1) {
            printf("Entrada inválida.\n");
            clearInputBuffer();
            return 0;
        }
        clearInputBuffer();
        
        if (*outThreshold > maxValue) {
            printf("Valor de limiar inválido.\n");
            return 0;
        }
    }
    
    return 1;
}

// Exibe as variações de uma imagem e permite o usuário escolher
// Retorna o índice escolhido (0-based) ou -1 se cancelado
static int printImageSelection(IImage *images, int count) {
    printf("\nEncontradas %d variações da imagem:\n", count);
    
    for (int i = 0; i < count; i++) {
        printf("  %d) ", i + 1);
        
        switch (images[i].filterMode) {
            case FILTER_NONE:
                printf("Sem filtro");
                break;
            case FILTER_THRESHOLD:
                printf("Limiarização (threshold=%u)", images[i].thresholdValue);
                break;
            case FILTER_NEGATIVE:
                printf("Negativo");
                break;
        }
        
        printf(" | %ux%u | %u bytes\n", 
               images[i].width, images[i].height, images[i].size);
    }
    
    printf("  0) Cancelar\n");
    printf("Escolha: ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Entrada inválida.\n");
        clearInputBuffer();
        return -1;
    }
    clearInputBuffer();
    
    if (choice == 0) {
        printf("Operação cancelada.\n");
        return -1;
    }
    
    if (choice < 1 || choice > count) {
        printf("Opção inválida.\n");
        return -1;
    }
    
    return choice - 1;
}

// Comando para importar imagem PGM para o banco de dados
// Com opções de filtro!
static void commandImport(void) {
    char path[1024], name[512];
    
    printf("Caminho do arquivo PGM: ");
    if (!readLine(path, sizeof(path))) {
        return;
    }   
    if (path[0] == '\0') {
        printf("Caminho não pode ser vazio.\n");
        return;
    }

    printf("Nome de referência (chave secundária): ");
    if (!readLine(name, sizeof(name))) {
        return;
    } 
    if (name[0] == '\0') {
        printf("Nome não pode ser vazio.\n");
        return;
    }

    uint32_t width, height, maxValue, bytes;
    uint8_t *data = NULL;
    uint8_t bytesPerPixel = 0;
    
    if (!loadPGM(path, &width, &height, &maxValue, 
                &data, &bytes, &bytesPerPixel)) {
        printf("Falha ao carregar '%s'.\n", path);
        return;
    }

    FilterMode mode;
    uint32_t thresholdValue;
    if (!printFilters((uint16_t)maxValue, &mode, &thresholdValue)) {
        free(data);
        return;
    }

    IImage existing;
    if (searchBTree(name, mode, thresholdValue, &existing)) {
        printf("Já existe uma imagem com nome '%s'", name);
        if (mode == FILTER_NONE) {
            printf(" sem filtro.\n");
        } else if (mode == FILTER_THRESHOLD) {
            printf(" com limiarização (threshold=%u).\n", thresholdValue);
        } else {
            printf(" com filtro negativo.\n");
        }
        free(data);
        return;
    }

    if (mode == FILTER_THRESHOLD) {
        applyThreshold(data, bytes, bytesPerPixel, thresholdValue, maxValue);
    } else if (mode == FILTER_NEGATIVE) {
        applyNegative(data, bytes, bytesPerPixel, maxValue);
    }

    uint64_t offset = 0;
    if (!addData(data, bytes, &offset)) {
        printf("Falha ao gravar no banco de dados (database.bin)\n");
        free(data);
        return;
    }
    free(data);

    IImage image = {
        .offset = offset,
        .size = bytes,
        .width = width,
        .height = height,
        .maxValue = (uint16_t)maxValue,
        .bpp = bytesPerPixel,
        .filterMode = mode,
        .thresholdValue = thresholdValue
    };
    strncpy(image.name, name, MAX_NAME_LENGTH - 1);
    image.name[MAX_NAME_LENGTH - 1] = '\0';

    if (!insertBTree(&image)) {
        printf("Falha ao gravar no índice.\n");
        return;
    }
    
    printf("Imagem '%s' importada com sucesso!\n", name);
    printf("   Dimensões: %ux%u | Maxval: %u | BPP: %u\n",
           width, height, maxValue, bytesPerPixel);
    printf("   Offset: %llu | Tamanho: %u bytes\n",
           (unsigned long long)offset, bytes);
}

static void commandExport(void) {
    char name[512];
    
    printf("Nome da imagem para exportar: ");
    if (!readLine(name, sizeof(name))) {
        return;
    }
    
    if (name[0] == '\0') {
        printf("Nome não pode ser vazio.\n");
        return;
    }

    #define MAX_VARIATIONS 10
    IImage results[MAX_VARIATIONS];
    int count = searchByNameBTree(name, results, MAX_VARIATIONS);
    
    if (count == 0) {
        printf("Imagem '%s' não encontrada.\n", name);
        return;
    }

    IImage *selectedImage;
    
    if (count == 1) {
        selectedImage = &results[0];
        printf("Imagem encontrada: ");
        switch (selectedImage->filterMode) {
            case FILTER_NONE:
                printf("sem filtro\n");
                break;
            case FILTER_THRESHOLD:
                printf("limiarização (threshold=%u)\n", selectedImage->thresholdValue);
                break;
            case FILTER_NEGATIVE:
                printf("negativo\n");
                break;
        }
    } else {
        int selection = printImageSelection(results, count);
        if (selection < 0) {
            return;
        }
        selectedImage = &results[selection];
    }

    // Solicitar opções de filtro adicional na exportação
    FilterMode mode;
    uint32_t thresholdValue;
    if (!printFilters(selectedImage->maxValue, &mode, &thresholdValue)) {
        return;
    }

    char outputPath[1024];
    printf("Arquivo de saída (.pgm): ");
    if (!readLine(outputPath, sizeof(outputPath))) {
        return;
    }
    
    if (outputPath[0] == '\0') {
        printf("Caminho de saída não pode ser vazio.\n");
        return;
    }

    if (exportImage(selectedImage, outputPath, mode, thresholdValue)) {
        printf("Imagem exportada com sucesso em '%s'.\n", outputPath);
    } else {
        printf("Falha na exportação.\n");
    }
}

// Deletar imagem do banco
// static void commandDelete(void) {
//     char name[512];
    
//     printf("Nome da imagem para deletar: ");
//     if (!readLine(name, sizeof(name))) {
//         return;
//     }
    
//     if (name[0] == '\0') {
//         printf("❌ Nome não pode ser vazio.\n");
//         return;
//     }

//     IImage image;
//     if (!findByName(name, &image)) {
//         printf("❌ Imagem '%s' não encontrada.\n", name);
//         return;
//     }

//     printf("⚠️  Tem certeza que deseja deletar '%s'? (S/N): ", name);
//     char confirmation[10];
//     if (!readLine(confirmation, sizeof(confirmation))) {
//         return;
//     }

//     if (toupper(confirmation[0]) != 'S') {
//         printf("Operação cancelada.\n");
//         return;
//     }

//     if (deleteByName(name)) {
//         printf("✅ Imagem '%s' deletada com sucesso!\n", name);
//     } else {
//         printf("❌ Falha ao deletar imagem!\n");
//     }
// }
// Compactar banco de dados
static void commandCompact(void) {
    printf("Iniciando compactação do banco de dados...\n");
    printf("Este procedimento é irreversível\n");
    printf("Os dados apagados não poderão ser recuperados\n");
    printf("⚠️  Deseja continuar? (S/N): ");
    
    char confirmation[10];
    if (!readLine(confirmation, sizeof(confirmation))) {
        return;
    }

    if (toupper(confirmation[0]) != 'S') {
        printf("Operação cancelada.\n");
        return;
    }

    if (compactDatabase()) {
        printf("✅ Banco de dados compactado com sucesso!\n");
    } else {
        printf("❌ Erro durante a compactação do banco de dados.\n");
    }
}


static void commandSort(void) {
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║           ORDENAR ARQUIVO DE ÍNDICE                   ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    int recordsPerPartition = 1000;  
    int partitionsToMerge = 4;                       
    
    printf("Configurações:\n");
    printf("  - Registros por partição: %d\n", recordsPerPartition);
    printf("  - K-way merge: %d vias\n", partitionsToMerge);
    printf("\nEste processo pode demorar. Continuar? (S/N): ");
    
    char confirmation[10];
    if (!readLine(confirmation, sizeof(confirmation))) {
        return;
    }
    
    if (toupper(confirmation[0]) != 'S') {
        printf("Operação cancelada.\n");
        return;
    }
    dumpIndex("--- ÍNDICE ANTES DA ORDENAÇÃO ---\n\n\n");
    if (externalSort(recordsPerPartition, partitionsToMerge)) {
        dumpIndex("--- ÍNDICE DEPOIS DA ORDENAÇÃO ---\n\n\n");
        printf("\n✅ Índice ordenado por nome com sucesso!\n");
        printf("   Agora você pode usar busca binária para consultas mais rápidas.\n");
    } else {
        printf("\n❌ Falha ao ordenar índice.\n");
    }
}