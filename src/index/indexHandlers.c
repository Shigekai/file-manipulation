#include "index.h"

//Funções estáticas auxiliares para os métodos em index.c

// Escreve um campo no arquivo com verificação
static int writeField(FILE *file, const void *data, size_t size) {
    return fwrite(data, size, 1, file) == 1;
}

// Lê um campo do arquivo com verificação
static int readField(FILE *file, void *data, size_t size) {
    return fread(data, size, 1, file) == 1;
}

// Lê próximo registro do índice
// Adicionamos um parâmetro para retornar a posição do campo isAvailable
static int readNextData(FILE *file, IImage *image, long *isAvailablePos) {
    uint16_t name_len;
    
    if (!readField(file, &name_len, sizeof(uint16_t))) {
        return 0;
    }
    
    char *name = malloc(name_len + 1);
    if (!name) return 0;
    
    if (fread(name, 1, name_len, file) != name_len) {
        free(name);
        return 0;
    }
    name[name_len] = '\0';
    image->name = name;
    
    if (!readField(file, &image->offset, sizeof(uint64_t)) ||
        !readField(file, &image->size,   sizeof(uint32_t)) ||
        !readField(file, &image->compressedSize, sizeof(uint32_t)) ||
        !readField(file, &image->width,  sizeof(uint32_t)) ||
        !readField(file, &image->height, sizeof(uint32_t)) ||
        !readField(file, &image->maxValue, sizeof(uint16_t)) ||
        !readField(file, &image->bpp,    sizeof(uint8_t))) {
        free(name);
        return 0;
    }
    
    if (isAvailablePos) {
        *isAvailablePos = ftello(file);
    }
    
    if (!readField(file, &image->isAvailable, sizeof(bool))) {
        free(name);
        return 0;
    }
    
    return 1;
}

static void freeData(IImage *image) {
    if (image && image->name) {
        free(image->name);
        image->name = NULL;  
    }
}

