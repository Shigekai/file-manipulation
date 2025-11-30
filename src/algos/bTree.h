#ifndef BTREE_H
#define BTREE_H

#include "headers.h"

// === Funções de ciclo de vida (virtualização da raiz) ===

// Abre a B-tree e carrega header/raiz em memória
int openBTree(void);

// Fecha a B-tree, persistindo alterações pendentes
void closeBTree(void);

// Força sincronização da raiz/header com o disco
int syncBTree(void);

// Inicializa o arquivo da B-tree (cria se não existir)
void initBTree(void);

// Insere uma imagem na B-tree
int insertBTree(const IImage *image);

// Busca uma imagem na B-tree pela chave composta
int searchBTree(const char *name, FilterMode filterMode, uint32_t thresholdValue, IImage *out);

// Lista todas as imagens da B-tree em ordem
void listBTree(void);

// Busca todas as imagens com o mesmo nome (variações de filtro)
int searchByNameBTree(const char *name, IImage *results, int maxResults);

// Callback para iteração na B-tree
typedef void (*BTreeCallback)(const IImage *image, void *userData);

// Percorre toda a B-tree e executa callback para cada imagem
void traverseBTree(BTreeCallback callback, void *userData);

#endif // BTREE_H