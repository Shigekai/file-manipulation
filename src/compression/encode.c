#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "encode.h"

uint8_t* encode(const uint8_t* data, uint32_t dataSize, uint32_t* encodedSize) {
    if (!data || dataSize == 0 || !encodedSize) {
        if (encodedSize) *encodedSize = 0;
        return NULL;
    }

    uint8_t* encoded = (uint8_t*)malloc((dataSize * 2 + 1) * sizeof(uint8_t));
    if (!encoded) {
        *encodedSize = 0;
        return NULL;
    }
    
    uint8_t currentSymbol = data[0];
    encoded[0] = currentSymbol;
    
    uint32_t encIdx = 1; 
    uint32_t count = 1; 
    
    for (uint32_t i = 1; i < dataSize; i++) {
        if (data[i] == currentSymbol && count < 255) {
            count++;
        } else {
            encoded[encIdx++] = (uint8_t)count;
            currentSymbol = data[i];
            count = 1;
        }
    }
    
    encoded[encIdx++] = (uint8_t)count;
    
    *encodedSize = encIdx;
    
    uint8_t* result = (uint8_t*)realloc(encoded, (*encodedSize) * sizeof(uint8_t));
    return result ? result : encoded;
}