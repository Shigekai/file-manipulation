#ifndef INDEX_H
#define INDEX_H

#include "headers.h"
#include <stdbool.h>

// Adiciona entrada ao índice
int addDataKey(const char *name, const IImage *entry);

// Busca por nome (O(N) sequencial)
int findByName(const char *name, IImage *out);

// Lista todas as imagens
void listAllData(void);

bool deleteByName(const char *name);

#endif // INDEX_H