#include "index.h"
#include "indexHandlers.c"

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
    int success = 
        writeField(indexFile, &nameLength,  sizeof(uint16_t)) &&
        fwrite(name, 1, nameLength, indexFile) == nameLength    &&
        writeField(indexFile, &image->offset, sizeof(uint64_t)) &&
        writeField(indexFile, &image->size,   sizeof(uint32_t)) &&
        writeField(indexFile, &image->width,  sizeof(uint32_t)) &&
        writeField(indexFile, &image->height, sizeof(uint32_t)) &&
        writeField(indexFile, &image->maxValue, sizeof(uint16_t)) &&
        writeField(indexFile, &image->bpp,    sizeof(uint8_t));
    
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
    
    IImage entry;
    while (readNextData(file, &entry)) {
        if (strcmp(entry.name, name) == 0) {
            *out = entry;
            fclose(file);
            return 1; 
        }
        freeData(&entry);
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
    
    IImage entry;
    int count = 0;
    
    while (readNextData(file, &entry)) {
        printf("  %d) %s\n", ++count, entry.name);
        printf("     Dimensões: %ux%u | maxValue: %u | BPP: %u\n",
               entry.width, entry.height, entry.maxValue, entry.bpp);
        printf("     Offset: %llu | Tamanho: %u bytes\n\n",
               (unsigned long long)entry.offset, entry.size);
        freeData(&entry);
    }
    
    if (count == 0) {
        printf("  (vazio)\n");
    } else {
        printf("Total: %d imagem(ns)\n", count);
    }
    
    fclose(file);
}