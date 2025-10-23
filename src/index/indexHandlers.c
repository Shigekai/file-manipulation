#include "index.h"

//Funções estáticas auxiliares para os métodos em index.c

// Escreve um registro no arquivo com verificação
static int writeRecord(FILE *file, const IImage *image) {
    return fwrite(image, sizeof(IImage), 1, file) == 1;
}

// Lê um registro do arquivo com verificação
static int readRecord(FILE *file, const IImage *image) {
    return fread(image, sizeof(IImage), 1, file) == 1;
}


