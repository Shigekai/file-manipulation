#ifndef ENCODE_H
#define ENCODE_H

#include <stdint.h>

uint8_t* encode(const uint8_t* data, uint32_t dataSize, uint32_t* encodedSize);

#endif 