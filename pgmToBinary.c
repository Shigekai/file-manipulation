#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pgmToBinary(const char* filename, char* format, int* width, int* height, int* maxValue) {
    FILE * pgmFile, * binaryFile;
    int pixel;

    char pgmFileName[1000];
    char binFileName[1000];

    sprintf(pgmFileName, "assets/%s.pgm", filename);
    sprintf(binFileName, "bin/%s.bin", filename);

    pgmFile = fopen(pgmFileName, "r");
    binaryFile = fopen(binFileName, "wb");

    fscanf(pgmFile, "%s", format);
    fscanf(pgmFile, "%d %d", width, height);
    fscanf(pgmFile, "%d", maxValue);

    fprintf(binaryFile, "P5\n%d %d\n%d\n", *width, *height, *maxValue);

    for (int i = 0; i < (*width) * (*height); i++) {
        fscanf(pgmFile, "%d", &pixel);
        unsigned char byte = (unsigned char) pixel;
        fwrite(&byte, sizeof(unsigned char), 1, binaryFile);
    }

    fclose(pgmFile);
    fclose(binaryFile);
    return 0;
}

int main() {
    char format[3];
    int width, height, maxValue;

    pgmToBinary("barbara", format, &width, &height, &maxValue);

    return 0;
}
