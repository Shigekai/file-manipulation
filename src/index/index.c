#include "index.h"
#include "indexHandlers.c"
#include "stdbool.h"

//métodos públicos que manipulam o arquivo de índice do banco de dados


//Procura no arquivo de índice por um nome.
//Usa o algoritmo simples de busca sequencial O(N).
int findByName(const char *name, IImage *out) {
    FILE *file = fopen(INDEX_PATH, "rb");
    if(!file){
        return 0;
    }

    IImage currentImage;
    while(readRecord(file, &currentImage)){
        if(currentImage.isAvailable && strcmp(currentImage.name, name) == 0){
            *out = currentImage;
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

//Lista todas as imagens com uma saída amigável para o usuário.
void listAllData(void) {
    FILE *file = fopen(INDEX_PATH, "rb");
    if (!file) {
        printf("Nenhuma imagem cadastrada.\n");
        return;
    }
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║              IMAGENS CADASTRADAS                       ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    IImage image;
    int count = 0;
    
    while (readRecord(file, &image)) {
        if (image.isAvailable) {
            printf("  %d) %s\n", ++count, image.name);
            printf("     Dimensões: %ux%u | maxValue: %u | BPP: %u\n",
                   image.width, image.height, image.maxValue, image.bpp);
            printf("     Offset: %llu | Tamanho: %u bytes\n\n",
                   (unsigned long long)image.offset, image.size);
        }
    }
    
    if (count == 0) {
        printf("  (vazio)\n");
    } else {
        printf("Total: %d imagem(ns)\n", count);
    }
    
    fclose(file);
}

// Esta função marca uma imagem como deletada
// Na prática, é um soft_delete, pois os arquivos são recuperáveis
bool deleteByName(const char *name) {
    FILE *file = fopen(INDEX_PATH, "r+b");
    if (!file) {
        return false;
    }

    IImage image;
    long recordPosition = 0;
    bool found = false;

    while (readRecord(file, &image)) {
        if (image.isAvailable && strcmp(image.name, name) == 0) {
            found = true;
            fseeko(file, recordPosition, SEEK_SET);
            image.isAvailable = false;
            
            writeRecord(file, &image);
            break;
        }
        
        recordPosition = ftello(file);
    }

    fclose(file);
    return found;
}

//Função para varrer o arquivo de índice e imprimir seu conteúdo
void dumpIndex(const char *label) {
    FILE *file = fopen(INDEX_PATH, "rb");
    if (!file) {
        printf("[%s] índice inexistente.\n", label);
        return;
    }

    printf("\n[%s] Conteúdo do índice:\n", label);
    printf("------------------------------------------------------------\n");

    IImage image;
    size_t count = 0;
    while (readRecord(file, &image)) {
        printf("#%zu | nome: %-25s | offset: %10llu | size: %7u | "
               "dim: %4ux%-4u | max: %5u | bpp: %u | disp: %s\n",
               ++count,
               image.name,
               (unsigned long long)image.offset,
               image.size,
               image.width,
               image.height,
               image.maxValue,
               image.bpp,
               image.isAvailable ? "sim" : "não");
    }

    if (count == 0) {
        printf("(vazio)\n");
    }

    printf("------------------------------------------------------------\n\n");
    fclose(file);
}