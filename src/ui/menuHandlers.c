#include "headers.h"
#include "menu.h"
#include "pgm.h"
#include "database.h"
#include "index.h"
#include "handleImages.h"

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


// Comando: Importar imagem PGM para o banco
static void commandImport(void) {
    char path[1024], name[512];
    
    printf("Caminho do arquivo PGM: ");
    if (!readLine(path, sizeof(path))) {
        return;
    }   
    if (path[0] == '\0') {
        printf("❌ Caminho não pode ser vazio.\n");
        return;
    }

    printf("Nome de referência (chave secundária): ");
    if (!readLine(name, sizeof(name))) {
        return;
    } 
    if (name[0] == '\0') {
        printf("❌ Nome não pode ser vazio.\n");
        return;
    }

    uint32_t width, height, maxValue, bytes;
    uint8_t *data = NULL;
    uint8_t bytesPerPixel = 0;
    
    if (!loadPGM(path, &width, &height, &maxValue, 
                &data, &bytes, &bytesPerPixel)) {
        printf("❌ Falha ao carregar '%s'.\n", path);
        return;
    }

    uint64_t offset = 0;
    if (!addData(data, bytes, &offset)) {
        printf("❌ Falha ao gravar no banco de dados (database.bin)\n");
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
        .name = NULL 
    };

    if (!addDataKey(name, &image)) {
        printf("❌ Falha ao gravar no índice.\n");
        return;
    }
    
    printf("✅ Imagem '%s' importada com sucesso!\n", name);
    printf("   Dimensões: %ux%u | Maxval: %u | BPP: %u\n",
           width, height, maxValue, bytesPerPixel);
    printf("   Offset: %llu | Tamanho: %u bytes\n",
           (unsigned long long)offset, bytes);
}

// Comando: Exportar imagem por nome
static void commandExport(void) {
    char name[512];
    
    printf("Nome da imagem para exportar: ");
    if (!readLine(name, sizeof(name))) {
        return;
    }
    
    if (name[0] == '\0') {
        printf("❌ Nome não pode ser vazio.\n");
        return;
    }

    IImage image;
    if (!findByName(name, &image)) {
        printf("❌ Imagem '%s' não encontrada.\n", name);
        return;
    }

    printf("\nEscolha o filtro:\n");
    printf("  0) Sem modificação\n");
    printf("  1) Limiarização\n");
    printf("  2) Negativo\n");
    printf("Opção: ");
    
    int modeChoice = 0;
    if (scanf("%d", &modeChoice) != 1) {
        printf("❌ Entrada inválida.\n");
        if (image.name) free(image.name);
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    
    if (modeChoice < 0 || modeChoice > 2) {
        printf("❌ Opção inválida.\n");
        if (image.name) free(image.name);
        return;
    }
    
    FilterMode mode = (FilterMode)modeChoice;

    uint32_t thresholdValue = 0;
    if (mode == FILTER_THRESHOLD) {
        printf("Valor de limiar (0 a %u): ", image.maxValue);
        if (scanf("%u", &thresholdValue) != 1) {
            printf("❌ Entrada inválida.\n");
            if (image.name) free(image.name);
            clearInputBuffer();
            return;
        }
        clearInputBuffer();
        
        if (thresholdValue > image.maxValue) {
            printf("⚠️  Ajustando threshold de %u para %u (maxval)\n",
                   thresholdValue, image.maxValue);
            thresholdValue = image.maxValue;
        }
    }

    char outputPath[1024];
    printf("Arquivo de saída (.pgm): ");
    if (!readLine(outputPath, sizeof(outputPath))) {
        if (image.name) free(image.name);
        return;
    }
    
    if (outputPath[0] == '\0') {
        printf("❌ Caminho de saída não pode ser vazio.\n");
        if (image.name) free(image.name);
        return;
    }

    if (exportImage(&image, outputPath, mode, thresholdValue)) {
        printf("✅ Imagem exportada com sucesso em '%s'.\n", outputPath);
    } else {
        printf("❌ Falha na exportação.\n");
    }
    
    if (image.name) {
        free(image.name);
    }
}

static void commandList(void) {
    listAllData();
}

static void commandDelete(void) {
    char name[512];
    
    printf("Nome da imagem para deletar: ");
    if (!readLine(name, sizeof(name))) {
        return;
    }
    
    if (name[0] == '\0') {
        printf("❌ Nome não pode ser vazio.\n");
        return;
    }

    IImage image;
    if (!findByName(name, &image)) {
        printf("❌ Imagem '%s' não encontrada.\n", name);
        return;
    }
    
    if (image.name) {
        free(image.name);
    }

    printf("⚠️  Tem certeza que deseja deletar '%s'? (S/N): ", name);
    char confirmation[10];
    if (!readLine(confirmation, sizeof(confirmation))) {
        return;
    }

    if (toupper(confirmation[0]) != 'S') {
        printf("Operação cancelada.\n");
        return;
    }

    if (deleteByName(name)) {
        printf("Imagem '%s' deletada com sucesso!\n", name);
    } else {
        printf("Falha ao deletar imagem!\n");
    }
}