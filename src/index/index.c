#include "index.h"
#include "indexHandlers.c"
#include "stdbool.h"

//métodos públicos que manipulam o arquivo de índice do banco de dados


//Este método adiciona os metadados de um novo arquivo (imagem) ao arquivo de índice.
//Escrevemos cada campo que descreve a nossa imagem a partir da linha 16
//Veja mais sobre a tipagem de image em headers.h
int addDataKey(const char *name, const IImage *image) {
    FILE *indexFile = fopen(INDEX_PATH, "ab");
    if(!indexFile){
        perror("Erro ao abrir o arquivo de índice");
        return 0;
    };

    IImage newImage = *image;
    newImage.isAvailable = true;
    strncpy(newImage.name, name, MAX_NAME_LENGTH);
    newImage.name[MAX_NAME_LENGTH] = '\0';
    fclose(indexFile);

    return writeRecord(indexFile, &newImage);
}

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