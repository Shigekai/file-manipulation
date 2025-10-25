#include "externalSort.h"
#include "externalSortHandlers.c"

// Realiza o external sort no arquivo de índice
//Merge sort com K-way merge
//Limpa os diretórios de partições quando terminar
int externalSort(int recordsPerPartition, int partitionsToMerge){
    printf("Iniciando external sort...\n");
    printf("Criando partições ordenadas...\n");

    int partitionCount = createSortedPartitions(INDEX_PATH, recordsPerPartition);

    printf("✓ %d partições criadas\n", partitionCount);


    printf("Iniciando K-way merge com K=%d...\n", partitionsToMerge);
    const char *temporaryOutput = "bin/sorted_index.bin";

    if(!kWayMerge(partitionCount, partitionsToMerge, temporaryOutput)){
        fprintf(stderr, "Erro durante o K-way merge.\n");
        cleanupPartitions(partitionCount);
        return 0;
    }

    printf("✓ K-way merge concluído. Arquivo ordenado gerado em '%s'\n", temporaryOutput);

    printf("Removendo arquivos temporários...\n");
    printf("✓ Arquivos temporários removidos.\n");
    printf("Substituindo arquivos antigos...\n");
    remove(INDEX_PATH);
    rename(temporaryOutput, INDEX_PATH);
    cleanupPartitions(partitionCount);

    printf("External sort concluído com sucesso!\n");
    return 1;
}