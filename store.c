#include "store.h"

void ensure_bin_folder(void) {
    FILE *f = fopen(STORE_PATH, "ab");
    if (!f) {
        MKDIR("bin");
        f = fopen(STORE_PATH, "ab");
        if (!f) {
            fprintf(stderr, "Erro ao criar/abrir %s: %s\n", STORE_PATH, strerror(errno));
            exit(1);
        }
    }
    fclose(f);
    f = fopen(INDEX_PATH, "ab");
    if (!f) {
        MKDIR("bin");
        f = fopen(INDEX_PATH, "ab");
        if (!f) {
            fprintf(stderr, "Erro ao criar/abrir %s: %s\n", INDEX_PATH, strerror(errno));
            exit(1);
        }
    }
    fclose(f);
}

int append_to_store(const uint8_t *data, uint32_t bytes, uint64_t *offset_out) {
    FILE *sf = fopen(STORE_PATH, "ab+");
    if (!sf) { fprintf(stderr, "Erro ao abrir %s: %s\n", STORE_PATH, strerror(errno)); return 0; }
#if defined(_WIN32)
    _fseeki64(sf, 0, SEEK_END);
    long long off = _ftelli64(sf);
    if (off < 0) { fclose(sf); return 0; }
    *offset_out = (uint64_t)off;
#else
    if (fseeko(sf, 0, SEEK_END) != 0) { fclose(sf); return 0; }
    off_t off = ftello(sf);
    if (off < 0) { fclose(sf); return 0; }
    *offset_out = (uint64_t)off;
#endif
    size_t wr = fwrite(data, 1, bytes, sf);
    fclose(sf);
    return wr == bytes;
}

int read_from_store(uint64_t offset, uint32_t size, uint8_t *buf) {
    FILE *sf = fopen(STORE_PATH, "rb");
    if (!sf) { fprintf(stderr, "Erro ao abrir %s\n", STORE_PATH); return 0; }
#if defined(_WIN32)
    _fseeki64(sf, (long long)offset, SEEK_SET);
#else
    if (fseeko(sf, (off_t)offset, SEEK_SET) != 0) { fclose(sf); return 0; }
#endif
    size_t rd = fread(buf, 1, size, sf);
    fclose(sf);
    return rd == size;
}
