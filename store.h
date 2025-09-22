//
// Created by costa on 21/09/2025.
//

#ifndef STORE_H
#define STORE_H
#include "imgdb.h"

// Garante que bin/ exista e que STORE_PATH/INDEX_PATH possam ser abertos
void ensure_bin_folder(void);

// Acrescenta bytes no final do store e retorna o offset (chave primária)
int append_to_store(const uint8_t *data, uint32_t bytes, uint64_t *offset_out);

// Lê um bloco do store no offset/size para buf (buf deve existir e ter size bytes)
int read_from_store(uint64_t offset, uint32_t size, uint8_t *buf);

#endif //STORE_H
