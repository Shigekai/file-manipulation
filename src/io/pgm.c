#include "pgm.h"
#include "pgmHandlers.c"

//Essa função lê um arquivo PGM (P2 ou P5) e retorna os dados da imagem em formato binário
// e o aloca em um buffer
int loadPGM(const char *path, uint32_t *width, uint32_t *height, uint32_t *maxValue, uint8_t **data, uint32_t *bytes, uint8_t *bytesPerPixel) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Erro ao abrir '%s': %s\n", path, strerror(errno));
        return 0;
    }

    *width = 0;
    *height = 0;
    *maxValue = 0;
    
    int isP5 = 0;
    
    if (!readPGMHeader(file, &isP5, width, height, maxValue)) {
        fprintf(stderr, "Header PGM inválido em '%s'\n", path);
        fclose(file);
        return 0;
    }
    
    if (*width == 0 || *height == 0 || *maxValue == 0 || *maxValue > 65535) {
        fprintf(stderr, "Valores inválidos em '%s' (w=%u, h=%u, maxval=%u)\n",
                path, *width, *height, *maxValue);
        fclose(file);
        return 0;
    }

    const uint8_t bpp = (*maxValue <= 255) ? 1 : 2;
    const uint64_t totalBytes = (uint64_t)(*width) * (uint64_t)(*height) * (uint64_t)bpp;
    if (totalBytes > 0xFFFFFFFFu) {
        fprintf(stderr, "Imagem muito grande em '%s'\n", path);
        fclose(file);
        return 0;
    }

    uint8_t *buffer = malloc((size_t)totalBytes);
    if (!buffer) {
        perror("Erro ao alocar memória");
        fclose(file);
        return 0;
    }

    int success = 0;
    
    if (isP5) {
        size_t read = fread(buffer, 1, (size_t)totalBytes, file);
        success = (read == (size_t)totalBytes);
        
        if (!success) {
            fprintf(stderr, "Erro ao ler dados de '%s'\n", path);
        }
    } else {
        success = readP2Data(file, buffer, *width, *height, bpp);
        
        if (!success) {
            fprintf(stderr, "Erro ao ler dados P2 de '%s'\n", path);
        }
    }

    fclose(file);
    
    if (!success) {
        free(buffer);
        return 0;
    }
    
    *data = buffer;
    *bytes = (uint32_t)totalBytes;
    *bytesPerPixel = bpp;
    
    return 1;
}

int writeP5Header(FILE *file, uint32_t width, uint32_t height, 
                 uint32_t maxValue) {
    return fprintf(file, "P5\n%u %u\n%u\n", width, height, maxValue) > 0;
}