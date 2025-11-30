#include "handleImages.h"
#include "database.h"
#include "../io/pgm.h"
#include "filters.h"
#include "filters.c"

//Essa função pega uma imagem do banco de dados (formato binário), com base no offset fornecido
//E a exporta para um arquivo PGM, aplicando o filtro solicitado

int exportImage(const IImage *image, const char *outputPath, FilterMode mode, uint32_t thresholdValue) {
    char finalPath[512];
    strcpy(finalPath, outputPath);
    
    if (strlen(outputPath) < 4 || strcmp(outputPath + strlen(outputPath) - 4, ".pgm") != 0) {
        strcat(finalPath, ".pgm");
    }

    uint8_t *buffer = malloc(image->size);
    if (!buffer) {
        perror("Erro ao alocar memória");
        return 0;
    }
    
    if (!readData(image->offset, image->size, buffer)) {
        free(buffer);
        return 0;
    }

    const uint32_t totalPixels = image->width * image->height;
    
    switch (mode) {
        case FILTER_THRESHOLD:
            applyThreshold(buffer, totalPixels, image->bpp, image->maxValue, thresholdValue);
            break;
            
        case FILTER_NEGATIVE:
            applyNegative(buffer, totalPixels, image->bpp, image->maxValue);
            break;
            
        case FILTER_NONE:
        default:
            break;
    }
    
    FILE *output = fopen(finalPath, "wb"); 
    if (!output) {
        perror("Erro ao criar arquivo de saída");
        free(buffer);
        return 0;
    }
    
    if (!writeP5Header(output, image->width, image->height, image->maxValue)) {
        fclose(output);
        free(buffer);
        return 0;
    }

    size_t written = fwrite(buffer, 1, image->size, output);
    
    fclose(output);
    free(buffer);
    
    return written == image->size;
}