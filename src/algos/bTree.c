#include "headers.h"
#include "bTree.h"
#include "bTreeHandlers.c"

// Contexto global da B-tree com raiz virtualizada
static BTreeContext g_btreeContext = {
    .file = NULL,
    .headerDirty = false,
    .rootDirty = false,
    .isOpen = false
};

int openBTree(void) {
    if (g_btreeContext.isOpen) {
        return 1; 
    }

    initBTree();
    
    g_btreeContext.file = fopen(INDEX_PATH, "r+b");
    if (!g_btreeContext.file) {
        perror("Erro ao abrir arquivo da B-tree");
        return 0;
    }
    
    if (!readHeader(g_btreeContext.file, &g_btreeContext.header)) {
        fprintf(stderr, "Erro ao ler header da B-tree\n");
        fclose(g_btreeContext.file);
        g_btreeContext.file = NULL;
        return 0;
    }
    
    if (g_btreeContext.header.rootOffset != 0) {
        if (!readNode(g_btreeContext.file, g_btreeContext.header.rootOffset, &g_btreeContext.rootNode)) {
            fprintf(stderr, "Erro ao ler raiz da B-tree\n");
            fclose(g_btreeContext.file);
            g_btreeContext.file = NULL;
            return 0;
        }
    } else {
        memset(&g_btreeContext.rootNode, 0, sizeof(BTreeNode));
    }
    
    g_btreeContext.headerDirty = false;
    g_btreeContext.rootDirty = false;
    g_btreeContext.isOpen = true;
    
    return 1;
}

int syncBTree(void) {
    if (!g_btreeContext.isOpen || !g_btreeContext.file) {
        return 0;
    }
    
    if (g_btreeContext.rootDirty && g_btreeContext.header.rootOffset != 0) {
        if (!writeNode(g_btreeContext.file, g_btreeContext.header.rootOffset, &g_btreeContext.rootNode)) {
            fprintf(stderr, "Erro ao persistir raiz\n");
            return 0;
        }
        g_btreeContext.rootDirty = false;
    }
    
    if (g_btreeContext.headerDirty) {
        if (!writeHeader(g_btreeContext.file, &g_btreeContext.header)) {
            fprintf(stderr, "Erro ao persistir header\n");
            return 0;
        }
        g_btreeContext.headerDirty = false;
    }
    
    fflush(g_btreeContext.file);
    return 1;
}

void closeBTree(void) {
    if (!g_btreeContext.isOpen) {
        return;
    }
    
    syncBTree();
    
    if (g_btreeContext.file) {
        fclose(g_btreeContext.file);
        g_btreeContext.file = NULL;
    }
    
    g_btreeContext.isOpen = false;
}

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

static uint64_t createNode(bool isLeaf) {
    if (!g_btreeContext.isOpen || !g_btreeContext.file) {
        fprintf(stderr, "B-tree não está aberta\n");
        return 0;
    }
    
    uint64_t newOffset = g_btreeContext.header.nextFreeOffset;
    
    BTreeNode newNode = {
        .keyCount = 0,
        .isLeaf = isLeaf
    };
    
    for (int i = 0; i < BTREE_MAX_CHILDREN; i++) {
        newNode.children[i] = 0;
    }
    
    if (!writeNode(g_btreeContext.file, newOffset, &newNode)) {
        fprintf(stderr, "Erro ao escrever novo nó\n");
        return 0;
    }
    
    g_btreeContext.header.nextFreeOffset += sizeof(BTreeNode);
    g_btreeContext.header.nodeCount++;
    g_btreeContext.headerDirty = true;
    
    return newOffset;
}


static void splitChild(uint64_t parentOffset, int index) {
    BTreeNode parent;
    bool parentIsRoot = (parentOffset == g_btreeContext.header.rootOffset);
    
    if (parentIsRoot) {
        parent = g_btreeContext.rootNode;
    } else if (!readNode(g_btreeContext.file, parentOffset, &parent)) {
        fprintf(stderr, "Erro ao ler nó pai\n");
        return;
    }
    
    uint64_t childOffset = parent.children[index];
    BTreeNode child;
    if (!readNode(g_btreeContext.file, childOffset, &child)) {
        fprintf(stderr, "Erro ao ler nó filho\n");
        return;
    }
    
    uint64_t newNodeOffset = createNode(child.isLeaf);
    BTreeNode newNode;
    if (!readNode(g_btreeContext.file, newNodeOffset, &newNode)) {
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
    
    for (int i = parent.keyCount; i > index; i--) {
        parent.children[i + 1] = parent.children[i];
    }
    
    parent.children[index + 1] = newNodeOffset;
    
    for (int i = parent.keyCount - 1; i >= index; i--) {
        parent.keys[i + 1] = parent.keys[i];
    }
    
    parent.keys[index] = child.keys[BTREE_ORDER/2 - 1];
    parent.keyCount++;
    
    writeNode(g_btreeContext.file, childOffset, &child);
    writeNode(g_btreeContext.file, newNodeOffset, &newNode);
    
    if (parentIsRoot) {
        g_btreeContext.rootNode = parent;
        g_btreeContext.rootDirty = true;
    } else {
        writeNode(g_btreeContext.file, parentOffset, &parent);
    }
}

// Insere em um nó não cheio
static void insertNonFull(uint64_t nodeOffset, const IImage *image) {
    BTreeNode node;
    bool isRoot = (nodeOffset == g_btreeContext.header.rootOffset);
    
    if (isRoot) {
        node = g_btreeContext.rootNode;
    } else if (!readNode(g_btreeContext.file, nodeOffset, &node)) {
        fprintf(stderr, "Erro ao ler nó\n");
        return;
    }
    
    int i = node.keyCount - 1;
    
    if (node.isLeaf) {
        while (i >= 0 && compareImageKeys(&node.keys[i], image) > 0) {
            node.keys[i + 1] = node.keys[i];
            i--;
        }
        node.keys[i + 1] = *image;
        node.keyCount++;
        
        if (isRoot) {
            g_btreeContext.rootNode = node;
            g_btreeContext.rootDirty = true;
        } else {
            writeNode(g_btreeContext.file, nodeOffset, &node);
        }
    } else {
        while (i >= 0 && compareImageKeys(&node.keys[i], image) > 0) {
            i--;
        }
        i++;
        
        BTreeNode child;
        if (!readNode(g_btreeContext.file, node.children[i], &child)) {
            fprintf(stderr, "Erro ao ler filho\n");
            return;
        }
        
        if (child.keyCount == BTREE_MAX_KEYS) {
            splitChild(nodeOffset, i);
            
            if (isRoot) {
                node = g_btreeContext.rootNode;
            } else {
                readNode(g_btreeContext.file, nodeOffset, &node);
            }
            
            if (compareImageKeys(&node.keys[i], image) < 0) {
                i++;
            }
        }
        insertNonFull(node.children[i], image);
    }
}

// Insere uma imagem na B-tree
int insertBTree(const IImage *image) {
    if (!g_btreeContext.isOpen) {
        if (!openBTree()) {
            return 0;
        }
    }
    
    if (g_btreeContext.header.rootOffset == 0) {
        uint64_t newRootOffset = createNode(true);
        if (newRootOffset == 0) {
            return 0;
        }
        
        g_btreeContext.rootNode.keys[0] = *image;
        g_btreeContext.rootNode.keyCount = 1;
        g_btreeContext.rootNode.isLeaf = true;
        for (int i = 0; i < BTREE_MAX_CHILDREN; i++) {
            g_btreeContext.rootNode.children[i] = 0;
        }
        
        g_btreeContext.header.rootOffset = newRootOffset;
        g_btreeContext.headerDirty = true;
        g_btreeContext.rootDirty = true;
        
        syncBTree();
        return 1;
    }
    
    if (g_btreeContext.rootNode.keyCount == BTREE_MAX_KEYS) {
        uint64_t oldRootOffset = g_btreeContext.header.rootOffset;
        
        writeNode(g_btreeContext.file, oldRootOffset, &g_btreeContext.rootNode);
        
        uint64_t newRootOffset = createNode(false);
        
        BTreeNode newRoot = {0};
        newRoot.keyCount = 0;
        newRoot.isLeaf = false;
        newRoot.children[0] = oldRootOffset;
        
        g_btreeContext.rootNode = newRoot;
        g_btreeContext.header.rootOffset = newRootOffset;
        g_btreeContext.headerDirty = true;
        g_btreeContext.rootDirty = true;
        
        writeNode(g_btreeContext.file, newRootOffset, &g_btreeContext.rootNode);
        
        splitChild(newRootOffset, 0);
    }
    
    insertNonFull(g_btreeContext.header.rootOffset, image);
    
    syncBTree();
    return 1;
}

// Busca uma imagem na B-tree
int searchBTree(const char *name, FilterMode filterMode, uint32_t thresholdValue, IImage *out) {
    if (!g_btreeContext.isOpen) {
        if (!openBTree()) {
            return 0;
        }
    }
    
    if (g_btreeContext.header.rootOffset == 0) {
        return 0;
    }
    
    IImage searchKey = {0};
    strncpy(searchKey.name, name, MAX_NAME_LENGTH);
    searchKey.filterMode = filterMode;
    searchKey.thresholdValue = thresholdValue;
    
    uint64_t currentOffset = g_btreeContext.header.rootOffset;
    BTreeNode node = g_btreeContext.rootNode; 
    bool firstIteration = true;
    
    while (currentOffset != 0) {
        if (!firstIteration) {
            if (!readNode(g_btreeContext.file, currentOffset, &node)) {
                return 0;
            }
        }
        firstIteration = false;
        
        int i = 0;
        while (i < node.keyCount && compareImageKeys(&searchKey, &node.keys[i]) > 0) {
            i++;
        }
        
        if (i < node.keyCount && compareImageKeys(&searchKey, &node.keys[i]) == 0) {
            *out = node.keys[i];
            return 1;
        }
        
        if (node.isLeaf) {
            return 0;
        }
        
        currentOffset = node.children[i];
    }
    
    return 0;
}

// Percorre a B-tree em ordem (para listagem)
static void traverseInOrder(uint64_t nodeOffset, bool isRoot) {
    if (nodeOffset == 0) {
        return;
    }
    
    BTreeNode node;
    if (isRoot) {
        node = g_btreeContext.rootNode;
    } else if (!readNode(g_btreeContext.file, nodeOffset, &node)) {
        return;
    }
    
    for (int i = 0; i < node.keyCount; i++) {
        traverseInOrder(node.children[i], false);
        
        printf("Nome: %s | Filtro: %d", node.keys[i].name, node.keys[i].filterMode);
        if (node.keys[i].filterMode == FILTER_THRESHOLD) {
            printf(" | Threshold: %u", node.keys[i].thresholdValue);
        }
        printf("\n");
    }
    traverseInOrder(node.children[node.keyCount], false);
}

// Lista todas as imagens da B-tree
void listBTree(void) {
    if (!g_btreeContext.isOpen) {
        if (!openBTree()) {
            printf("Árvore vazia ou arquivo não encontrado.\n");
            return;
        }
    }
    
    printf("\n=== Listagem da B-Tree ===\n");
    printf("Total de nós: %u\n\n", g_btreeContext.header.nodeCount);
    
    if (g_btreeContext.header.rootOffset == 0) {
        printf("Árvore vazia.\n");
        return;
    }
    
    traverseInOrder(g_btreeContext.header.rootOffset, true);
}

// Função auxiliar recursiva para coletar imagens por nome
//Útil para coletar as variações de imagens com o mesmo nome
static void collectByName(uint64_t nodeOffset, const char *name, 
                          IImage *results, int maxResults, int *count, bool isRoot) {
    if (nodeOffset == 0 || *count >= maxResults) {
        return;
    }
    
    BTreeNode node;
    if (isRoot) {
        node = g_btreeContext.rootNode;
    } else if (!readNode(g_btreeContext.file, nodeOffset, &node)) {
        return;
    }
    
    for (int i = 0; i < node.keyCount; i++) {
        if (!node.isLeaf) {
            collectByName(node.children[i], name, results, maxResults, count, false);
        }
        
        if (*count < maxResults && strcmp(node.keys[i].name, name) == 0) {
            results[*count] = node.keys[i];
            (*count)++;
        }
    }
    
    if (!node.isLeaf) {
        collectByName(node.children[node.keyCount], name, results, maxResults, count, false);
    }
}

// Busca todas as imagens com um determinado nome
//Utiliza o método recursivo collectByName
int searchByNameBTree(const char *name, IImage *results, int maxResults) {
    if (!g_btreeContext.isOpen) {
        if (!openBTree()) {
            return 0;
        }
    }
    
    if (g_btreeContext.header.rootOffset == 0) {
        return 0;
    }
    
    int count = 0;
    collectByName(g_btreeContext.header.rootOffset, name, results, maxResults, &count, true);
    
    return count;
}