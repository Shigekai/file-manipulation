#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "encode.h"


// Codifica um buffer usando Run-Length Encoding modificado
uint8_t* encode(const uint8_t* data, uint32_t dataSize, uint32_t* encodedSize) {
    if (!data || dataSize == 0 || !encodedSize) {
        if (encodedSize) *encodedSize = 0;
        return NULL;
    }

    uint8_t* encoded = (uint8_t*)malloc((dataSize + 1) * sizeof(uint8_t));
    if (!encoded) {
        *encodedSize = 0;
        return NULL;
    }
    
    uint8_t currentSymbol = data[0];
    encoded[0] = currentSymbol;
    
    uint32_t encIdx = 1; 
    uint32_t count = 0; 
    
    for (uint32_t i = 0; i < dataSize; i++) {
        if (data[i] == currentSymbol) {
            count++;
            
            if (count == 255) {
                encoded[encIdx++] = 255; 
                count = 0;              
            }
        } else {
            encoded[encIdx++] = count;
            currentSymbol = !currentSymbol;  
            count = 1;               
        }
    }
    
    if (count > 0) {
        encoded[encIdx++] = count;
    }
    
    *encodedSize = encIdx;
    
    uint8_t* result = (uint8_t*)realloc(encoded, (*encodedSize) * sizeof(uint8_t));
    return result ? result : encoded;
}