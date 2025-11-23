#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>
#include "headers.h"  

// Exporta imagem com filtro opcional
int exportImage(const IImage *image, const char *outputPath, FilterMode mode, uint32_t thresholdValue);

#endif 