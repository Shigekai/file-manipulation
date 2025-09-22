//
// Created by costa on 21/09/2025.
//

#ifndef INDEXDB_H
#define INDEXDB_H
#include "imgdb.h"

// Adiciona um registro ao índice (name como chave secundária)
int add_key_record(const char *name, const KeyEntry *e);

// Lê o próximo registro do índice aberto; retorna 1 se ok
int read_next_key(FILE *kf, KeyEntry *out);

// Libera o campo name de um KeyEntry carregado do índice
void free_key(KeyEntry *e);

// Busca sequencial O(N) por nome no índice
int find_by_name_seq(const char *name, KeyEntry *found);

// Lista todos os registros do índice
void list_all(void);
#endif //INDEXDB_H
