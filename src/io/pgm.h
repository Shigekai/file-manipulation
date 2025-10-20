#ifndef PGM_H
#define PGM_H

#include "headers.h"

// Lê arquivo PGM (P2 ou P5) e retorna dados como bytes P5
int loadPGM(const char *path, uint32_t *width, uint32_t *height, 
            uint32_t *maxValue, uint8_t **data, uint32_t *bytes, 
            uint8_t *bytesPerPixel);

// Escreve header PGM formato P5 (binário)
int writeP5Header(FILE *file, uint32_t width, uint32_t height, 
                  uint32_t maxValue);

#endif // PGM_H