#include "filters.h"
#include "store.h"
#include "pgm_io.h"

static void apply_negative(uint8_t *buf, uint32_t pixels, uint8_t bpp, uint32_t maxval) {
    if (bpp == 1) {
        for (uint32_t i=0;i<pixels;i++) buf[i] = (uint8_t)(maxval - buf[i]);
    } else {
        for (uint32_t i=0;i<pixels;i++) {
            uint16_t v = ((uint16_t)buf[2*i] << 8) | (uint16_t)buf[2*i+1];
            uint16_t nv = (uint16_t)(maxval - v);
            buf[2*i]   = (uint8_t)((nv >> 8) & 0xFF);
            buf[2*i+1] = (uint8_t)(nv & 0xFF);
        }
    }
}

static void apply_threshold(uint8_t *buf, uint32_t pixels, uint8_t bpp, uint32_t maxval, uint32_t th) {
    if (th > maxval) th = maxval;
    if (bpp == 1) {
        for (uint32_t i=0;i<pixels;i++) {
            uint8_t v = buf[i];
            buf[i] = (v >= th) ? (uint8_t)maxval : 0;
        }
    } else {
        for (uint32_t i=0;i<pixels;i++) {
            uint16_t v = ((uint16_t)buf[2*i] << 8) | (uint16_t)buf[2*i+1];
            uint16_t nv = (v >= th) ? (uint16_t)maxval : 0;
            buf[2*i]   = (uint8_t)((nv >> 8) & 0xFF);
            buf[2*i+1] = (uint8_t)(nv & 0xFF);
        }
    }
}

int export_image(const KeyEntry *e, const char *out_path, int mode, uint32_t thresholdVal) {
    uint8_t *buf = (uint8_t*)malloc(e->size);
    if (!buf) return 0;

    if (!read_from_store(e->offset, e->size, buf)) {
        free(buf); return 0;
    }

    const uint32_t pixels = e->width * e->height;
    if (mode == 1) {
        apply_threshold(buf, pixels, e->bpp, e->maxval, thresholdVal);
    } else if (mode == 2) {
        apply_negative(buf, pixels, e->bpp, e->maxval);
    }

    FILE *out = fopen(out_path, "wb");
    if (!out) { fprintf(stderr, "Erro ao criar '%s': %s\n", out_path, strerror(errno)); free(buf); return 0; }
    if (!write_p5_header(out, e->width, e->height, e->maxval)) { fclose(out); free(buf); return 0; }

    size_t wr = fwrite(buf, 1, e->size, out);
    fclose(out);
    free(buf);
    return wr == e->size;
}
