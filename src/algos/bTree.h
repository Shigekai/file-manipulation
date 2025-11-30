#ifndef BTREE_H
#define BTREE_H

#include "headers.h"

// Inicializa o arquivo da B-tree
void initBTree(void);k

// Insere uma imagem na B-tree
int insertBTree(const IImage *image);

// Busca uma imagem na B-tree pela chave composta
int searchBTree(const char *name, FilterMode filterMode, uint32_t thresholdValue, IImage *out);

// Lista todas as imagens da B-tree em ordem
void listBTree(void);

int searchByNameBTree(const char *name, IImage *results, int maxResults);

#endif // BTREE_H