//
// Created by costa on 21/09/2025.
//

#ifndef HEADERS_H
#define HEADERS_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)

#define DATABASE_PATH "bin/database.bin"
#define INDEX_PATH "bin/index.bin"

typedef struct {
    uint64_t offset;   // chave primária (posição em database.bin)
    uint32_t size;     // metadados de bytes de pixel
    uint32_t width;    // metadados de largura
    uint32_t height;   // metadados de altura
    uint16_t maxValue;   // metadados de valor máximo
    uint8_t  bpp;      // metadados de bits por pixel
    bool isAvailable; // flag para controle de soft delete (1 = disponível, 0 = deletado)
    char name[256];     // chave secundária
} IImage;

#define IIMAGE_SIZE sizeof(IImage)
#define MAX_NAME_LENGTH 255

#endif //IMGDB_H
