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
#define BTREE_ORDER 3
#define BTREE_MAX_KEYS (BTREE_ORDER - 1)    
#define BTREE_MAX_CHILDREN BTREE_ORDER  
#define BTREE_MIN_KEYS ((BTREE_ORDER / 2) - 1)

typedef enum {
    FILTER_NONE = 0,      // Exporta sem modificação
    FILTER_THRESHOLD = 1, // Aplica limiarização
    FILTER_NEGATIVE = 2   // Aplica negativo
} FilterMode;

typedef struct {
    uint64_t offset;   // chave primária (posição em database.bin)
    uint32_t size;     // metadados de bytes de pixel
    uint32_t width;    // metadados de largura
    uint32_t height;   // metadados de altura
    uint16_t maxValue;   // metadados de valor máximo
    uint8_t  bpp;      // metadados de bits por pixel

    //Chave secundária (composite keys:)
    char name[256];     // Nome da imagem (Prioridade 01)
    FilterMode filterMode; // modo de filtro LIMIARIZAÇÃO | NEGATIVO (Prioridade 02)
    uint32_t thresholdValue; // Valor da limiarização. (Prioridade 03)
} IImage;

typedef struct {
    uint32_t keyCount; // Número max de chaves no nó, como a árvore é de ordem 3, o máximo é 2
    IImage keys[BTREE_MAX_KEYS];   // Array de chaves (imagens)
    uint64_t children[BTREE_MAX_CHILDREN]; // Array de offsets dos filhos
    bool isLeaf;      // Indica se o nó é folha (sem filhos)
} BTreeNode;

typedef struct {
    uint64_t rootOffset;
    uint32_t nodeCount;
    uint64_t nextFreeOffset;
} BTreeHeader;

#define IIMAGE_SIZE sizeof(IImage)
#define MAX_NAME_LENGTH 255

#endif //IMGDB_H
