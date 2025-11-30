#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>
#include "headers.h"  

// Aplica filtro negativo na imagem
void applyNegative(uint8_t *buffer, uint32_t totalPixels, uint8_t bytesPerPixel, uint32_t maxValue);

// Aplica limiarização (threshold) na imagem
void applyThreshold(uint8_t *buffer, uint32_t totalPixels, uint8_t bytesPerPixel, uint32_t maxValue, uint32_t threshold);

// Exporta imagem com filtro opcional
int exportImage(const IImage *image, const char *outputPath, FilterMode mode, uint32_t thresholdValue);

#endif 