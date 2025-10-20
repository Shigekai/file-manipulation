#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>

uint8_t* decode(const uint8_t* encoded, uint32_t encodedSize, uint32_t* decodedSize);

#endif 