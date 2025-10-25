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
//
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

//Lê a próxima partição do arquivo
static void advancePartition(PartitionReader *reader){
    if(fread(&reader->currentImage, sizeof(IImage), 1, reader->file) == 1){
        reader->hasData = true;
    } else {
        reader->hasData = false;
    }
}

static int kWayMerge(int totalPartitions, int partitionsToMerge, const char *outputPath){
    FILE *output = fopen(outputPath, "wb");

    int currentPartition = 0;

    while(currentPartition < totalPartitions) {
        int partitionsInThisRound = (totalPartitions - currentPartition < partitionsToMerge)
        ? (totalPartitions - currentPartition)
        : partitionsToMerge;

        PartitionReader *readers = malloc(partitionsInThisRound * sizeof(PartitionReader));

        for (int i = 0; i < partitionsInThisRound; i++){
            char partitionName[256];
            snprintf(partitionName, sizeof(partitionName),
            "bin/partitions/partition_%d.bin", currentPartition + i);
            
            readers[i].file = fopen(partitionName, "rb");
            if(!readers[i].file){
                perror("Erro ao abrir partição para leitura");
                free(readers);
                fclose(output);
                return 0;
            }

            readers[i].partitionId = currentPartition + i;
            advancePartition(&readers[i]);
        }

        int activePartitions = partitionsInThisRound;
        while(activePartitions > 0){
            int minIndex = findMinPartition(readers, partitionsInThisRound);
            if(minIndex == -1) break;

            fwrite(&readers[minIndex].currentImage, sizeof(IImage), 1, output);
            advancePartition(&readers[minIndex]);

            if(!readers[minIndex].hasData){
                activePartitions--;
            }
        }

        for (int i = 0; i < partitionsInThisRound; i++) {
            fclose(readers[i].file);
        }

        free(readers);
        currentPartition += partitionsInThisRound;
    }

    fclose(output);
    return 1;
}

static void cleanupPartitions(int partitionCount) {
    for (int i = 0; i < partitionCount; i++) {
        char partitionName[256];
        snprintf(partitionName, sizeof(partitionName), "bin/partitions/partition_%d.bin", i);
        remove(partitionName);
    }
}