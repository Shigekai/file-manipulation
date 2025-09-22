#include "indexdb.h"

int add_key_record(const char *name, const KeyEntry *e) {
    FILE *kf = fopen(INDEX_PATH, "ab");
    if (!kf) { fprintf(stderr, "Erro ao abrir %s: %s\n", INDEX_PATH, strerror(errno)); return 0; }
    uint16_t nlen = (uint16_t)strlen(name);
    if (fwrite(&nlen, sizeof(uint16_t), 1, kf) != 1) { fclose(kf); return 0; }
    if (fwrite(name, 1, nlen, kf) != nlen) { fclose(kf); return 0; }
    if (fwrite(&e->offset, sizeof(uint64_t), 1, kf) != 1) { fclose(kf); return 0; }
    if (fwrite(&e->size,   sizeof(uint32_t), 1, kf) != 1) { fclose(kf); return 0; }
    if (fwrite(&e->width,  sizeof(uint32_t), 1, kf) != 1) { fclose(kf); return 0; }
    if (fwrite(&e->height, sizeof(uint32_t), 1, kf) != 1) { fclose(kf); return 0; }
    if (fwrite(&e->maxval, sizeof(uint16_t), 1, kf) != 1) { fclose(kf); return 0; }
    if (fwrite(&e->bpp,    sizeof(uint8_t),  1, kf) != 1) { fclose(kf); return 0; }
    fclose(kf);
    return 1;
}

int read_next_key(FILE *kf, KeyEntry *out) {
    uint16_t nlen;
    if (fread(&nlen, sizeof(uint16_t), 1, kf) != 1) return 0;
    char *name = (char*)malloc(nlen + 1);
    if (!name) return 0;
    if (fread(name, 1, nlen, kf) != nlen) { free(name); return 0; }
    name[nlen] = '\0';
    out->name = name;
    if (fread(&out->offset, sizeof(uint64_t), 1, kf) != 1) { free(name); return 0; }
    if (fread(&out->size,   sizeof(uint32_t), 1, kf) != 1) { free(name); return 0; }
    if (fread(&out->width,  sizeof(uint32_t), 1, kf) != 1) { free(name); return 0; }
    if (fread(&out->height, sizeof(uint32_t), 1, kf) != 1) { free(name); return 0; }
    if (fread(&out->maxval, sizeof(uint16_t), 1, kf) != 1) { free(name); return 0; }
    if (fread(&out->bpp,    sizeof(uint8_t),  1, kf) != 1) { free(name); return 0; }
    return 1;
}

void free_key(KeyEntry *e) {
    if (e && e->name) { free(e->name); e->name = NULL; }
}

int find_by_name_seq(const char *name, KeyEntry *found) {
    FILE *kf = fopen(INDEX_PATH, "rb");
    if (!kf) { fprintf(stderr, "Erro ao abrir %s\n", INDEX_PATH); return 0; }
    KeyEntry e;
    while (read_next_key(kf, &e)) {
        if (strcmp(e.name, name) == 0) {
            *found = e;
            fclose(kf);
            return 1;
        }
        free_key(&e);
    }
    fclose(kf);
    return 0;
}

void list_all(void) {
    FILE *kf = fopen(INDEX_PATH, "rb");
    if (!kf) { printf("(nenhuma imagem indexada ainda)\n"); return; }
    printf("Imagens indexadas:\n");
    KeyEntry e; int i=0;
    while (read_next_key(kf, &e)) {
        printf("  %d) '%s'  [%ux%u max=%u, bpp=%u, offset=%llu, size=%u]\n",
               ++i, e.name, e.width, e.height, e.maxval, e.bpp,
               (unsigned long long)e.offset, e.size);
        free_key(&e);
    }
    if (i==0) printf("  (vazio)\n");
    fclose(kf);
}
