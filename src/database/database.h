#include "headers.h"

// Garante que bin/ exista e que STORE_PATH/INDEX_PATH possam ser abertos
void ensureBin(void);

// Acrescenta bytes no final do store e retorna o offset (chave primária)
int addData(const uint8_t *data, uint32_t bytes, uint64_t *offset_out);

// Lê um bloco do store no offset/size para buf (buf deve existir e ter size bytes)
int readData(uint64_t offset, uint32_t size, uint8_t *buf);
