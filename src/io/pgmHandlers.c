#include "pgm.h"

//Funções estáticas auxiliares para manipulação de arquivos PGM

// Pula comentários e espaços em branco no arquivo PGM
static void skipComments(FILE *file) {
    int c;
    do {
        c = fgetc(file);
        
        while (isspace(c)) {
            c = fgetc(file);
        }
        
        if (c == '#') {
            while (c != '\n' && c != EOF) {
                c = fgetc(file);
            }
        } else {
            if (c != EOF) {
                ungetc(c, file);
            }
            break;
        }
    } while (1);
}

// Lê header do arquivo PGM e determina se é P5 ou P2
static int readPGMHeader(FILE *file, int *isP5, uint32_t *width, uint32_t *height, uint32_t *maxValue) {
    char magic[3] = {0};
    if (fread(magic, 1, 2, file) != 2) {
        return 0;
    }
    magic[2] = '\0';
    
    if (strcmp(magic, "P5") == 0) {
        *isP5 = 1;  
    } else if (strcmp(magic, "P2") == 0) {
        *isP5 = 0;
    } else {
        return 0;  
    }

    skipComments(file);
    if (fscanf(file, "%u", width) != 1) return 0;
    
    skipComments(file);
    if (fscanf(file, "%u", height) != 1) return 0;
    
    skipComments(file);
    if (fscanf(file, "%u", maxValue) != 1) return 0;

    int c = fgetc(file);
    if (c == '\r') {
        int c2 = fgetc(file);
        if (c2 != '\n') {
            ungetc(c2, file);
        }
    } else if (c != '\n' && c != ' ' && c != '\t') {
        ungetc(c, file);
    }

    return 1;
}

// Lê dados de imagem P2 e transforma em binário P5
static int readP2Data(FILE *file, uint8_t *buffer, uint32_t width, 
                     uint32_t height, uint8_t bytesPerPixel) {
    const uint64_t totalPixels = (uint64_t)width * height;
    
    if (bytesPerPixel == 1) {
        for (uint64_t i = 0; i < totalPixels; i++) {
            skipComments(file);
            unsigned value;
            if (fscanf(file, "%u", &value) != 1 || value > 255) {
                return 0;
            }
            buffer[i] = (uint8_t)value;
        }
    } else {
        for (uint64_t i = 0; i < totalPixels; i++) {
            skipComments(file);
            unsigned value;
            if (fscanf(file, "%u", &value) != 1 || value > 65535) {
                return 0;
            }
            buffer[2*i]     = (uint8_t)((value >> 8) & 0xFF);
            buffer[2*i + 1] = (uint8_t)(value & 0xFF);
        }
    }
    
    return 1;
}