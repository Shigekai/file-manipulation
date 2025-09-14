#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char format[3];
    int width;
    int height;
    int maxValue;
    unsigned char* pixels;
} PGMImage;

//Alunos: Lucas Gabriel, Guilherme Zorzam
int thresholdBinaryToPgm(const char* filename, int threshold) {
    FILE *binaryFile, *outputFile;
    PGMImage image;

    char binFileName[1000];
    char outputFileName[1000];
    sprintf(binFileName, "bin/%s.bin", filename);
    sprintf(outputFileName, "assets/%s-thresholded.pgm", filename);

    if (access(outputFileName, F_OK) == 0) {
        printf("Ops! Você já limiarizou essa imagem: %s\n", outputFileName);
        exit(0); 
    }

    binaryFile = fopen(binFileName, "rb");
    if(!binaryFile){
        printf("Err: arquivo %s não encontrado!\n", binFileName);
        return -1;
    }

    fscanf(binaryFile, "%2s", image.format);
    fscanf(binaryFile, "%d %d", &image.width, &image.height);
    fscanf(binaryFile, "%d", &image.maxValue);

    int totalPixels = image.width * image.height;
    image.pixels = (unsigned char*)malloc(totalPixels * sizeof(unsigned char));

    fread(image.pixels, sizeof(unsigned char), totalPixels, binaryFile);
    fclose(binaryFile);

    for(int i = 0; i < totalPixels; i++){
        if(image.pixels[i] >= threshold){
            image.pixels[i] = image.maxValue;
        } else {
            image.pixels[i] = 0;
        }
    }

    outputFile = fopen(outputFileName, "w");

    fprintf(outputFile, "%s\n", image.format);
    fprintf(outputFile, "%d %d\n", image.width, image.height);
    fprintf(outputFile, "%d\n", image.maxValue);

    for(int i = 0; i < totalPixels; i++){
        fprintf(outputFile, "%d\n", image.pixels[i]);
        if((i + 1) % image.width == 0){
            fprintf(outputFile, "\n");
        }
    }

    fclose(outputFile);
    free(image.pixels);

    printf("Limiarização concluída. Confira o arquivo %s\n", outputFileName);

    return 0;
}