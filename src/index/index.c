#include "index.h"
#include "indexHandlers.c"
#include "stdbool.h"

//métodos públicos que manipulam o arquivo de índice do banco de dados


//Este método adiciona os metadados de um novo arquivo (imagem) ao arquivo de índice.
//Escrevemos cada campo que descreve a nossa imagem a partir da linha 16
//Veja mais sobre a tipagem de image em headers.h
int addDataKey(const char *name, const IImage *image) {
    FILE *indexFile = fopen(INDEX_PATH, "ab");
    if (!indexFile) {
        perror("Erro ao abrir índice");
        return 0;
    }
    
    uint16_t nameLength = (uint16_t)strlen(name);
    bool available = true;

    int success = 
        writeField(indexFile, &nameLength,  sizeof(uint16_t)) &&
        fwrite(name, 1, nameLength, indexFile) == nameLength    &&
        writeField(indexFile, &image->offset, sizeof(uint64_t)) &&
        writeField(indexFile, &image->size,   sizeof(uint32_t)) &&
        writeField(indexFile, &image->width,  sizeof(uint32_t)) &&
        writeField(indexFile, &image->height, sizeof(uint32_t)) &&
        writeField(indexFile, &image->maxValue, sizeof(uint16_t)) &&
        writeField(indexFile, &image->bpp,    sizeof(uint8_t)) &&
        writeField(indexFile, &available,    sizeof(uint8_t));
    
    fclose(indexFile);
    return success;
}

//Procura no arquivo de índice por um nome.
//Usa o algoritmo simples de busca sequencial O(N).
int findByName(const char *name, IImage *out) {
    FILE *file = fopen(INDEX_PATH, "rb");
    if (!file) {
        return 0; 
    }
    
    IImage image;
    while (readNextData(file, &image, NULL)) {
        if (image.isAvailable && strcmp(image.name, name) == 0) {
            *out = image;
            fclose(file);
            return 1; 
        }
        freeData(&image);
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
    
    while (readNextData(file, &image, NULL)) {
        if(image.isAvailable) {
            printf("  %d) %s\n", ++count, image.name);
            printf("     Dimensões: %ux%u | maxValue: %u | BPP: %u\n",
                   image.width, image.height, image.maxValue, image.bpp);
            printf("     Offset: %llu | Tamanho: %u bytes\n\n",
                   (unsigned long long)image.offset, image.size);
        }
        freeData(&image);
    }
    
    if (count == 0) {
        printf("  (vazio)\n");
    } else {
        printf("Total: %d imagem(ns)\n", count);
    }
    
    fclose(file);
}

// E então na função deleteByName:
bool deleteByName(const char *name){
    FILE *file = fopen(INDEX_PATH, "r+b");
    if(!file){
        return false;
    }

    IImage currentImage;
    bool found = false;
    long isAvailablePos;

    while (!found){
        if(!readNextData(file, &currentImage, &isAvailablePos)){
            break;
        }

        if(strcmp(currentImage.name, name) == 0) {
            found = true;
            
            fseeko(file, isAvailablePos, SEEK_SET);
            
            bool notAvailable = false;
            writeField(file, &notAvailable, sizeof(bool));
            
            freeData(&currentImage);
            fclose(file);
            return true;
        } else {
            freeData(&currentImage);
        }
    }

    fclose(file);
    return false;
}