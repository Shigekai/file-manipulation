#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>
#include "headers.h"  

typedef enum {
    FILTER_NONE = 0,      // Exporta sem modificação
    FILTER_THRESHOLD = 1, // Aplica limiarização
    FILTER_NEGATIVE = 2   // Aplica negativo
} FilterMode;

// Exporta imagem com filtro opcional
int exportImage(const IImage *image, const char *outputPath, FilterMode mode, uint32_t thresholdValue);

#endif 