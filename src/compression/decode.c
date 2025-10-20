#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "decode.h"

uint8_t* decode(const uint8_t* encoded, uint32_t encodedSize, uint32_t* decodedSize) {
    if (!encoded || encodedSize < 2 || !decodedSize) {
        if (decodedSize) *decodedSize = 0;
        return NULL;
    }

    uint32_t totalSize = 0;
    for (uint32_t i = 1; i < encodedSize; i++) {
        totalSize += encoded[i];
    }
    
    uint8_t* decoded = (uint8_t*)malloc(totalSize * sizeof(uint8_t));
    if (!decoded) {
        *decodedSize = 0;
        return NULL;
    }

    uint8_t currentSymbol = encoded[0];
    uint32_t decIdx = 0;  
    
    for (uint32_t i = 1; i < encodedSize; i++) {
        uint8_t count = encoded[i];
        
        for (uint8_t j = 0; j < count; j++) {
            decoded[decIdx++] = currentSymbol;
        }
        
        currentSymbol = !currentSymbol;
    }
    
    *decodedSize = totalSize;
    return decoded;
}