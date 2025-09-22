#include "pgm_io.h"

static void skip_ws_and_comments(FILE *fp) {
    int c;
    do {
        c = fgetc(fp);
        while (isspace(c)) c = fgetc(fp);
        if (c == '#') {
            while (c != '\n' && c != EOF) c = fgetc(fp);
        } else {
            if (c != EOF) ungetc(c, fp);
            break;
        }
    } while (1);
}

static int read_pgm_header(FILE *fp, int *isP5, uint32_t *w, uint32_t *h, uint32_t *maxval) {
    char magic[3] = {0};
    if (fread(magic, 1, 2, fp) != 2) return 0;
    magic[2] = '\0';
    if (strcmp(magic, "P5") == 0) *isP5 = 1;
    else if (strcmp(magic, "P2") == 0) *isP5 = 0;
    else return 0;

    skip_ws_and_comments(fp);
    if (fscanf(fp, "%u", w) != 1) return 0;
    skip_ws_and_comments(fp);
    if (fscanf(fp, "%u", h) != 1) return 0;
    skip_ws_and_comments(fp);
    if (fscanf(fp, "%u", maxval) != 1) return 0;

    int c = fgetc(fp);
    if (c == '\r') { int c2 = fgetc(fp); if (c2 != '\n') ungetc(c2, fp); }
    else if (c != '\n' && c != ' ' && c != '\t') ungetc(c, fp);

    return 1;
}

int load_pgm_as_p5_bytes(const char *path,
                         uint32_t *W, uint32_t *H, uint32_t *MAX,
                         uint8_t **data, uint32_t *bytes, uint8_t *bpp_out)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Falha ao abrir PGM '%s': %s\n", path, strerror(errno));
        return 0;
    }

    int isP5 = 0;
    uint32_t w=0,h=0,maxv=0;
    if (!read_pgm_header(fp, &isP5, &w, &h, &maxv)) {
        fprintf(stderr, "Header PGM inválido em '%s'\n", path);
        fclose(fp);
        return 0;
    }
    if (w == 0 || h == 0 || maxv == 0 || maxv > 65535) {
        fprintf(stderr, "PGM inválido (valores fora do intervalo) em '%s'\n", path);
        fclose(fp);
        return 0;
    }

    const uint8_t bpp = (maxv <= 255) ? 1 : 2;
    const uint64_t total = (uint64_t)w * (uint64_t)h * (uint64_t)bpp;
    if (total > 0xFFFFFFFFu) { fclose(fp); return 0; }

    uint8_t *buf = (uint8_t*)malloc((size_t)total);
    if (!buf) { fclose(fp); return 0; }

    if (isP5) {
        size_t rd = fread(buf, 1, (size_t)total, fp);
        if (rd != (size_t)total) { free(buf); fclose(fp); return 0; }
    } else {
        if (bpp == 1) {
            for (uint64_t i = 0; i < (uint64_t)w*h; i++) {
                skip_ws_and_comments(fp);
                unsigned v;
                if (fscanf(fp, "%u", &v) != 1 || v>255) { free(buf); fclose(fp); return 0; }
                buf[i] = (uint8_t)v;
            }
        } else {
            for (uint64_t i = 0; i < (uint64_t)w*h; i++) {
                skip_ws_and_comments(fp);
                unsigned v;
                if (fscanf(fp, "%u", &v) != 1 || v>65535) { free(buf); fclose(fp); return 0; }
                buf[2*i]   = (uint8_t)((v >> 8) & 0xFF);
                buf[2*i+1] = (uint8_t)(v & 0xFF);
            }
        }
    }

    fclose(fp);
    *W = w; *H = h; *MAX = maxv; *data = buf; *bytes = (uint32_t)total; *bpp_out = bpp;
    return 1;
}

int write_p5_header(FILE *fp, uint32_t w, uint32_t h, uint32_t maxval) {
    return fprintf(fp, "P5\n%u %u\n%u\n", w, h, maxval) > 0;
}
