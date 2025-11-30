#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>

// Aplica filtro negativo na imagem
void applyNegative(uint8_t *buffer, uint32_t totalPixels, uint8_t bytesPerPixel, uint32_t maxValue);

// Aplica limiarização (threshold) na imagem
void applyThreshold(uint8_t *buffer, uint32_t totalPixels, uint8_t bytesPerPixel, uint32_t maxValue, uint32_t threshold);

#endif // FILTERS_H
