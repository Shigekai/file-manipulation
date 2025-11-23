#include "headers.h"

//Compara duas imagens para ordenação na B-Tree
//Nome: prioridade máxima
//FilterMode: prioridade secundária
//ThresholdValue: prioridade terciária (apenas se FilterMode for FILTER_THRESHOLD)
//Duas imagens negativas ou sem filtro de mesmo serão negadas
int compareImageKeys(const IImage *a, const IImage *b) {
    int nameCompare = strcmp(a->name, b->name);
    if (nameCompare != 0) {
        return nameCompare;
    }
    
    if (a->filterMode != b->filterMode) {
        return a->filterMode - b->filterMode;
    }
    
    if (a->filterMode == FILTER_THRESHOLD) {
        return (int)a->thresholdValue - (int)b->thresholdValue;
    }
    
    if (a->filterMode == FILTER_NEGATIVE) {
        return 0; 
    }

    return 0;
}

static int readHeader(FILE *file, BTreeHeader *header) {
    if (fseeko(file, 0, SEEK_SET) != 0) {
        perror("Erro ao posicionar no início do arquivo");
        return 0;
    }
    size_t read = fread(header, sizeof(BTreeHeader), 1, file);
    return read == 1;
}

// Escreve o header da B-tree no arquivo
static int writeHeader(FILE *file, const BTreeHeader *header) {
    if (fseeko(file, 0, SEEK_SET) != 0) {
        perror("Erro ao posicionar no início do arquivo");
        return 0;
    }
    size_t written = fwrite(header, sizeof(BTreeHeader), 1, file);
    return written == 1;
}

// Lê um nó da B-tree a partir de um offset
static int readNode(FILE *file, uint64_t offset, BTreeNode *node) {
    if (offset == 0) {
        return 0; // Offset inválido
    }
    
    if (fseeko(file, (off_t)offset, SEEK_SET) != 0) {
        perror("Erro ao posicionar no offset do nó");
        return 0;
    }
    
    size_t read = fread(node, sizeof(BTreeNode), 1, file);
    return read == 1;
}

// Escreve um nó da B-tree em um offset
static int writeNode(FILE *file, uint64_t offset, const BTreeNode *node) {
    if (fseeko(file, (off_t)offset, SEEK_SET) != 0) {
        perror("Erro ao posicionar no offset do nó");
        return 0;
    }
    
    size_t written = fwrite(node, sizeof(BTreeNode), 1, file);
    return written == 1;
}