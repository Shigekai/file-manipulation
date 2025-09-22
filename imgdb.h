//
// Created by costa on 21/09/2025.
//

#ifndef IMGDB_H
#define IMGDB_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(_WIN32)
  #include <direct.h>
  #define MKDIR(path) _mkdir(path)
#else
  #include <sys/stat.h>
  #define MKDIR(path) mkdir(path, 0755)
#endif

#define STORE_PATH "bin/store.bin"
#define INDEX_PATH "bin/index.bin"

typedef struct {
    uint64_t offset;   // chave primária (posição em store.bin)
    uint32_t size;     // bytes de pixel
    uint32_t width;
    uint32_t height;
    uint16_t maxval;
    uint8_t  bpp;      // 1 (<=255) ou 2 (>255)
    char    *name;     // alocado dinamicamente
} KeyEntry;

#endif //IMGDB_H
