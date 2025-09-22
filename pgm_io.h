//
// Created by costa on 21/09/2025.
//

#ifndef PGM_IO_H
#define PGM_IO_H
#include "imgdb.h"

// Lê PGM (P2/P5) e entrega os dados como bytes P5 (1B ou 2B big-endian).
// Retorna 1 se ok.
int load_pgm_as_p5_bytes(const char *path,
                         uint32_t *W, uint32_t *H, uint32_t *MAX,
                         uint8_t **data, uint32_t *bytes, uint8_t *bpp_out);

// Escreve o cabeçalho P5 "P5\nW H\nMAX\n"
int write_p5_header(FILE *fp, uint32_t w, uint32_t h, uint32_t maxval);

#endif //PGM_IO_H
