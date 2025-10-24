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