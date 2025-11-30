#include "headers.h"
#include "bTree.h"
#include "bTreeHandlers.c"


void initBTree(void) {
    FILE *file = fopen(INDEX_PATH, "rb");
    if (file) {
        fclose(file);
        return;
    }
    
    file = fopen(INDEX_PATH, "wb");
    if (!file) {
        perror("Erro ao criar arquivo da B-tree");
        exit(EXIT_FAILURE);
    }
    
    BTreeHeader header = {
        .rootOffset = 0,
        .nodeCount = 0,
        .nextFreeOffset = sizeof(BTreeHeader)
    };
    
    if (!writeHeader(file, &header)) {
        fprintf(stderr, "Erro ao escrever header da B-tree\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    
    fclose(file);
}

static uint64_t createNode(FILE *file, bool isLeaf) {
    BTreeHeader header;
    if (!readHeader(file, &header)) {
        fprintf(stderr, "Erro ao ler header\n");
        return 0;
    }
    
    uint64_t newOffset = header.nextFreeOffset;
    
    BTreeNode newNode = {
        .keyCount = 0,
        .isLeaf = isLeaf
    };
    
    for (int i = 0; i < BTREE_MAX_CHILDREN; i++) {
        newNode.children[i] = 0;
    }
    
    if (!writeNode(file, newOffset, &newNode)) {
        fprintf(stderr, "Erro ao escrever novo nó\n");
        return 0;
    }
    
    header.nextFreeOffset += sizeof(BTreeNode);
    header.nodeCount++;
    
    if (!writeHeader(file, &header)) {
        fprintf(stderr, "Erro ao atualizar header\n");
        return 0;
    }
    
    return newOffset;
}


static void splitChild(FILE *file, uint64_t parentOffset, int index) {
    BTreeNode parent;
    if (!readNode(file, parentOffset, &parent)) {
        fprintf(stderr, "Erro ao ler nó pai\n");
        return;
    }
    
    uint64_t childOffset = parent.children[index];
    BTreeNode child;
    if (!readNode(file, childOffset, &child)) {
        fprintf(stderr, "Erro ao ler nó filho\n");
        return;
    }
    
    uint64_t newNodeOffset = createNode(file, child.isLeaf);
    BTreeNode newNode;
    if (!readNode(file, newNodeOffset, &newNode)) {
        fprintf(stderr, "Erro ao ler novo nó\n");
        return;
    }
    
    newNode.keyCount = BTREE_MIN_KEYS;
    
    for (int i = 0; i < BTREE_MIN_KEYS; i++) {
        newNode.keys[i] = child.keys[i + BTREE_ORDER/2];
    }
    
    if (!child.isLeaf) {
        for (int i = 0; i < BTREE_ORDER/2; i++) {
            newNode.children[i] = child.children[i + BTREE_ORDER/2];
        }
    }
    
    child.keyCount = BTREE_MIN_KEYS;
    
    // Desloca filhos do pai para abrir espaço
    for (int i = parent.keyCount; i > index; i--) {
        parent.children[i + 1] = parent.children[i];
    }
    
    parent.children[index + 1] = newNodeOffset;
    
    // Desloca chaves do pai para inserir a chave do meio
    for (int i = parent.keyCount - 1; i >= index; i--) {
        parent.keys[i + 1] = parent.keys[i];
    }
    
    parent.keys[index] = child.keys[BTREE_ORDER/2 - 1];
    parent.keyCount++;
    
    // Salva nós modificados
    writeNode(file, childOffset, &child);
    writeNode(file, newNodeOffset, &newNode);
    writeNode(file, parentOffset, &parent);
}

// Insere em um nó não cheio
static void insertNonFull(FILE *file, uint64_t nodeOffset, const IImage *image) {
    BTreeNode node;
    if (!readNode(file, nodeOffset, &node)) {
        fprintf(stderr, "Erro ao ler nó\n");
        return;
    }
    
    int i = node.keyCount - 1;
    
    if (node.isLeaf) {
        // Insere na ordem
        while (i >= 0 && compareImageKeys(&node.keys[i], image) > 0) {
            node.keys[i + 1] = node.keys[i];
            i--;
        }
        node.keys[i + 1] = *image;
        node.keyCount++;
        
        writeNode(file, nodeOffset, &node);
    } else {
        // Encontra filho apropriado
        while (i >= 0 && compareImageKeys(&node.keys[i], image) > 0) {
            i--;
        }
        i++;
        
        BTreeNode child;
        if (!readNode(file, node.children[i], &child)) {
            fprintf(stderr, "Erro ao ler filho\n");
            return;
        }
        
        if (child.keyCount == BTREE_MAX_KEYS) {
            // Divide filho se estiver cheio
            splitChild(file, nodeOffset, i);
            
            // Relê o nó pai (foi modificado)
            readNode(file, nodeOffset, &node);
            
            // Determina qual dos dois filhos usar
            if (compareImageKeys(&node.keys[i], image) < 0) {
                i++;
            }
        }
        insertNonFull(file, node.children[i], image);
    }
}

// Insere uma imagem na B-tree
int insertBTree(const IImage *image) {
    FILE *file = fopen(INDEX_PATH, "r+b");
    if (!file) {
        perror("Erro ao abrir arquivo da B-tree");
        return 0;
    }
    
    BTreeHeader header;
    if (!readHeader(file, &header)) {
        fclose(file);
        return 0;
    }
    
    if (header.rootOffset == 0) {
        // Cria primeira raiz
        uint64_t newRootOffset = createNode(file, true);
        BTreeNode root;
        readNode(file, newRootOffset, &root);
        
        root.keys[0] = *image;
        root.keyCount = 1;
        
        writeNode(file, newRootOffset, &root);
        
        header.rootOffset = newRootOffset;
        writeHeader(file, &header);
        
        fclose(file);
        return 1;
    }
    
    BTreeNode root;
    if (!readNode(file, header.rootOffset, &root)) {
        fclose(file);
        return 0;
    }
    
    if (root.keyCount == BTREE_MAX_KEYS) {
        // Divide raiz se estiver cheia
        uint64_t newRootOffset = createNode(file, false);
        BTreeNode newRoot;
        readNode(file, newRootOffset, &newRoot);
        
        newRoot.children[0] = header.rootOffset;
        writeNode(file, newRootOffset, &newRoot);
        
        splitChild(file, newRootOffset, 0);
        
        header.rootOffset = newRootOffset;
        writeHeader(file, &header);
    }
    
    insertNonFull(file, header.rootOffset, image);
    
    fclose(file);
    return 1;
}

// Busca uma imagem na B-tree
int searchBTree(const char *name, FilterMode filterMode, uint32_t thresholdValue, IImage *out) {
    FILE *file = fopen(INDEX_PATH, "rb");
    if (!file) {
        return 0;
    }
    
    BTreeHeader header;
    if (!readHeader(file, &header)) {
        fclose(file);
        return 0;
    }
    
    if (header.rootOffset == 0) {
        fclose(file);
        return 0; // Árvore vazia
    }
    
    // Cria chave de busca
    IImage searchKey = {0};
    strncpy(searchKey.name, name, MAX_NAME_LENGTH);
    searchKey.filterMode = filterMode;
    searchKey.thresholdValue = thresholdValue;
    
    // Busca recursiva
    uint64_t currentOffset = header.rootOffset;
    
    while (currentOffset != 0) {
        BTreeNode node;
        if (!readNode(file, currentOffset, &node)) {
            fclose(file);
            return 0;
        }
        
        int i = 0;
        while (i < node.keyCount && compareImageKeys(&searchKey, &node.keys[i]) > 0) {
            i++;
        }
        
        if (i < node.keyCount && compareImageKeys(&searchKey, &node.keys[i]) == 0) {
            *out = node.keys[i];
            fclose(file);
            return 1;
        }
        
        if (node.isLeaf) {
            fclose(file);
            return 0;
        }
        
        currentOffset = node.children[i];
    }
    
    fclose(file);
    return 0;
}

// Percorre a B-tree em ordem (para listagem)
static void traverseInOrder(FILE *file, uint64_t nodeOffset) {
    if (nodeOffset == 0) {
        return;
    }
    
    BTreeNode node;
    if (!readNode(file, nodeOffset, &node)) {
        return;
    }
    
    for (int i = 0; i < node.keyCount; i++) {
        traverseInOrder(file, node.children[i]);
        
        // Imprime informações da imagem
        printf("Nome: %s | Filtro: %d", node.keys[i].name, node.keys[i].filterMode);
        if (node.keys[i].filterMode == FILTER_THRESHOLD) {
            printf(" | Threshold: %u", node.keys[i].thresholdValue);
        }
        printf("\n");
    }
    traverseInOrder(file, node.children[node.keyCount]);
}

// Lista todas as imagens da B-tree
void listBTree(void) {
    FILE *file = fopen(INDEX_PATH, "rb");
    if (!file) {
        printf("Árvore vazia ou arquivo não encontrado.\n");
        return;
    }
    
    BTreeHeader header;
    if (!readHeader(file, &header)) {
        fclose(file);
        return;
    }
    
    printf("\n=== Listagem da B-Tree ===\n");
    printf("Total de nós: %u\n\n", header.nodeCount);
    
    traverseInOrder(file, header.rootOffset);
    
    fclose(file);
}

// Função auxiliar recursiva para coletar imagens por nome
//Útil para coletar as variações de imagens com o mesmo nome
static void collectByName(FILE *file, uint64_t nodeOffset, const char *name, 
                          IImage *results, int maxResults, int *count) {
    if (nodeOffset == 0 || *count >= maxResults) {
        return;
    }
    
    BTreeNode node;
    if (!readNode(file, nodeOffset, &node)) {
        return;
    }
    
    for (int i = 0; i < node.keyCount; i++) {
        if (!node.isLeaf) {
            collectByName(file, node.children[i], name, results, maxResults, count);
        }
        
        if (*count < maxResults && strcmp(node.keys[i].name, name) == 0) {
            results[*count] = node.keys[i];
            (*count)++;
        }
    }
    
    if (!node.isLeaf) {
        collectByName(file, node.children[node.keyCount], name, results, maxResults, count);
    }
}

// Busca todas as imagens com um determinado nome
//Utiliza o método recursivo collectByName
int searchByNameBTree(const char *name, IImage *results, int maxResults) {
    FILE *file = fopen(INDEX_PATH, "rb");
    if (!file) {
        return 0;
    }
    
    BTreeHeader header;
    if (!readHeader(file, &header)) {
        fclose(file);
        return 0;
    }
    
    if (header.rootOffset == 0) {
        fclose(file);
        return 0;
    }
    
    int count = 0;
    collectByName(file, header.rootOffset, name, results, maxResults, &count);
    
    fclose(file);
    return count;
}