#include "headers.h"

//Aqui, temos funções estáticas auxiliares de filters.c
//Elas recebem um buffer, dados da imagem, e aplicam os filtros solicitados em cada pixel
//Ambas conseguem lidar com pixels 1 ou 2 bytes para maior flexibilidade

// Função responsável por aplicar o filtro negativo
static void applyNegative(uint8_t *buffer, uint32_t totalPixels, uint8_t bytesPerPixel, uint32_t maxValueue) {
    if (bytesPerPixel == 1) {
        for (uint32_t i = 0; i < totalPixels; i++) {
            buffer[i] = (uint8_t)(maxValueue - buffer[i]);
        }
    } else {
        for (uint32_t i = 0; i < totalPixels; i++) {
            uint16_t value = ((uint16_t)buffer[2*i] << 8) | 
                            (uint16_t)buffer[2*i + 1];
            
            uint16_t negativeValue = (uint16_t)(maxValueue - value);
            
            buffer[2*i]     = (uint8_t)((negativeValue >> 8) & 0xFF);
            buffer[2*i + 1] = (uint8_t)(negativeValue & 0xFF);
        }
    }
}

// Aplica limiarização (threshold) na imagem
static void applyThreshold(uint8_t *buffer, uint32_t totalPixels,
                          uint8_t bytesPerPixel, uint32_t maxValueue,
                          uint32_t threshold) {
    if (threshold > maxValueue) {
        threshold = maxValueue;
    }
    
    if (bytesPerPixel == 1) {
        for (uint32_t i = 0; i < totalPixels; i++) {
            uint8_t value = buffer[i];
            buffer[i] = (value >= threshold) ? (uint8_t)maxValueue : 0;
        }
    } else {
        for (uint32_t i = 0; i < totalPixels; i++) {
            uint16_t value = ((uint16_t)buffer[2*i] << 8) | 
                            (uint16_t)buffer[2*i + 1];
            
            uint16_t result = (value >= threshold) ? (uint16_t)maxValueue : 0;
            
            buffer[2*i]     = (uint8_t)((result >> 8) & 0xFF);
            buffer[2*i + 1] = (uint8_t)(result & 0xFF);
        }
    }
}