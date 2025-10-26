#include "headers.h"


//Função auxiliar para o qsort - compara duas IImage pelo campo name
static int compareImagesByName(const void *firstImage, const void *secondImage) {
    //Type casting, pois qsort exige uma função com parâmetros do tipo void*
    const IImage *imgA = (const IImage *)firstImage;
    const IImage *imgB = (const IImage *)secondImage;
    
    return strcmp(imgA->name, imgB->name);
}

//Aqui vamos ler o banco de dados, e criar partições de certo tamanho
//Interessante que cada partição já estará ordenada (quick sort para tal)
//Pois isso é essencial para o kway merge depois!
//Observação --> malloc e free fora do while para otimização.
//Observação --> o buffer aponta para o mesmo endereço de memória durante todo o processo.
//Observação --> o que muda é o valor.
//Retornamos o número de partições criadas
static int createSortedPartitions(const char *inputPath, int recordsPerPartition){
    FILE *file = fopen(inputPath, "rb");
    IImage *buffer = malloc(sizeof(IImage) * recordsPerPartition);

    int partitionsCount = 0;
    int recordsRead;

    while((recordsRead = fread(buffer, sizeof(IImage), recordsPerPartition, file)) > 0){
        qsort(buffer, recordsRead, sizeof(IImage), compareImagesByName);
        char partitionName[256];
        snprintf(partitionName, sizeof(partitionName), "bin/partitions/partition_%d.bin", partitionsCount);

        FILE *partitionFile = fopen(partitionName, "wb");

        fwrite(buffer, sizeof(IImage), recordsRead, partitionFile);
        fclose(partitionFile);
        partitionsCount++;
    }

    free(buffer);
    fclose(file);

    return partitionsCount;
}

typedef struct {
    FILE *file;
    IImage currentImage;
    bool hasData;
    int partitionId;
} PartitionReader;

//Aqui foi utilizado aritmética de ponteiros para manipular os dados
//Encontra o registro mínimo entre as partições ativas
//Retorna seu índice
static int findMinPartition(PartitionReader *readers, int totalPartitions){
    int minIndex = -1;

    for(int i = 0; i < totalPartitions; i++){
        //Pular para próxima iteração se não houver dados =)
        if(!readers[i].hasData) continue;

        if(minIndex == -1 || strcmp(readers[i].currentImage.name, readers[minIndex].currentImage.name) < 0){
            minIndex = i;
        }

    }
    return minIndex;
}

//Lê o próximo registro da partição
static void advancePartition(PartitionReader *reader){
    if(fread(&reader->currentImage, sizeof(IImage), 1, reader->file) == 1){
        reader->hasData = true;
    } else {
        reader->hasData = false;
    }
}

static char *duplicateString(const char *source) {
    size_t length = strlen(source) + 1;
    char *copy = malloc(length);
    if (copy) {
        memcpy(copy, source, length);
    }
    return copy;
}


static void freePartitionList(char **list, int count, bool removeFiles) {
    if (!list) return;
    for (int i = 0; i < count; i++) {
        if (!list[i]) continue;
        if (removeFiles) {
            remove(list[i]);
        }
        free(list[i]);
    }
    free(list);
}


//Esta função cuida de realizar o merge de um grupo de partições por vez
//E escrever o resultado final em um arquivo de saída temporário
//Que estará no disco
static int mergePartitionGroup(const char **partitionPaths, int groupSize, const char *outputPath){
    FILE *output = fopen(outputPath, "wb");
    if (!output) {
        perror("Erro ao abrir arquivo de destino para merge");
        return 0;
    }

    PartitionReader *readers = malloc(groupSize * sizeof(PartitionReader));
    if (!readers) {
        perror("Erro de memória ao preparar merge");
        fclose(output);
        return 0;
    }

    for (int i = 0; i < groupSize; i++) {
        readers[i].file = fopen(partitionPaths[i], "rb");
        if (!readers[i].file) {
            perror("Erro ao abrir partição para merge");
            for (int j = 0; j < i; j++) {
                fclose(readers[j].file);
            }
            free(readers);
            fclose(output);
            remove(outputPath);
            return 0;
        }
        readers[i].partitionId = i;
        advancePartition(&readers[i]);
    }

    while (1) {
        int minIndex = findMinPartition(readers, groupSize);
        if (minIndex == -1) {
            break;
        }
        fwrite(&readers[minIndex].currentImage, sizeof(IImage), 1, output);
        advancePartition(&readers[minIndex]);
    }

    for (int i = 0; i < groupSize; i++) {
        fclose(readers[i].file);
    }

    free(readers);
    fclose(output);
    return 1;
}

//Esta função faz as passadas de kway merge até que reste apenas uma partição ordenada
//A cada passada, ele chama a função mergePartitionGroup para unir grupos de partições
//O resultado final é movido para o caminho especificado por finalOutputPath

static int kWayMerge(int partitionCount, int partitionsToMerge, const char *finalOutputPath) {
    if (partitionCount == 0) {
        FILE *output = fopen(finalOutputPath, "wb");
        if (!output) {
            perror("Erro ao criar arquivo final vazio");
            return 0;
        }
        fclose(output);
        return 1;
    }

    if (partitionCount == 1) {
        char source[256];
        snprintf(source, sizeof(source), "bin/partitions/partition_0.bin");
        remove(finalOutputPath);
        if (rename(source, finalOutputPath) != 0) {
            perror("Erro ao mover partição final");
            return 0;
        }
        return 1;
    }

    if (partitionsToMerge < 2) {
        partitionsToMerge = 2;
    }

    char **currentPartitions = malloc(sizeof(char *) * partitionCount);
    if (!currentPartitions) {
        perror("Erro de memória ao iniciar kway merge");
        return 0;
    }

    for (int i = 0; i < partitionCount; i++) {
        char path[256];
        snprintf(path, sizeof(path), "bin/partitions/partition_%d.bin", i);
        currentPartitions[i] = duplicateString(path);
        if (!currentPartitions[i]) {
            freePartitionList(currentPartitions, i, false);
            perror("Erro de memória ao armazenar caminhos de partições");
            return 0;
        }
    }

    int currentCount = partitionCount;
    int round = 0;

    while (currentCount > 1) {
        int nextCount = (currentCount + partitionsToMerge - 1) / partitionsToMerge;
        char **nextPartitions = malloc(sizeof(char *) * nextCount);
        if (!nextPartitions) {
            perror("Erro de memória ao preparar próxima rodada");
            freePartitionList(currentPartitions, currentCount, true);
            return 0;
        }

        int produced = 0;
        for (int start = 0; start < currentCount; start += partitionsToMerge) {
            int groupSize = (start + partitionsToMerge > currentCount)
                            ? (currentCount - start)
                            : partitionsToMerge;

            char outputPath[256];
            snprintf(outputPath, sizeof(outputPath),
                     "bin/partitions/tmp_r%d_p%d.bin", round, produced);

            const char **group = (const char **)(currentPartitions + start);
            if (!mergePartitionGroup(group, groupSize, outputPath)) {
                freePartitionList(nextPartitions, produced, true);
                freePartitionList(currentPartitions, currentCount, true);
                return 0;
            }

            nextPartitions[produced] = duplicateString(outputPath);
            if (!nextPartitions[produced]) {
                freePartitionList(nextPartitions, produced + 1, true);
                freePartitionList(currentPartitions, currentCount, true);
                perror("Erro de memória ao armazenar partição intermediária");
                return 0;
            }
            produced++;
        }

        freePartitionList(currentPartitions, currentCount, true);
        currentPartitions = nextPartitions;
        currentCount = produced;
        round++;
    }

    remove(finalOutputPath);
    if (rename(currentPartitions[0], finalOutputPath) != 0) {
        perror("Erro ao mover arquivo final ordenado");
        freePartitionList(currentPartitions, 1, true);
        return 0;
    }

    freePartitionList(currentPartitions, 1, false);
    return 1;
}

static void cleanupPartitions(int partitionCount) {
    for (int i = 0; i < partitionCount; i++) {
        char partitionName[256];
        snprintf(partitionName, sizeof(partitionName), "bin/partitions/partition_%d.bin", i);
        remove(partitionName);
    }
}