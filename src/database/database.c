#include "database.h"
#include "../algos/bTree.h"
//Observação: usado fseeko/ftello em vez de fseek/ftell para garantir uma abordagem moderna =)

//Garante que o arquivo binário pôde ser aberto/criado com sucesso...
//Garante a criação da pasta bin/ e bin/partitions/
//E dos arquivos binários necessários
void ensureBin(void) {
    MKDIR("bin");
    MKDIR("bin/partitions");
    
    FILE *databaseFile = fopen(DATABASE_PATH, "ab");
    if (!databaseFile) {
        fprintf(stderr, "Erro ao criar/abrir %s: %s\n", 
                DATABASE_PATH, strerror(errno));
        exit(1);
    }
    fclose(databaseFile);
    
    // Inicializa a B-tree (cria arquivo de índice se não existir)
    initBTree();
}
// Para não ter problema de tamanhos em diferentes platformas, usei tipos inteiros com tamanhos fixos:
//uint8_t  → 8 bits -> 1 byte
//uint32_t → 32 bits -> 4 bytes
//uint64_t → 64 bits -> 8 bytes

//Grava o arquivo no database, no final do arquivo, e retorna o offset onde foi gravado
int addData(const uint8_t *data, uint32_t bytes, uint64_t *offset_out) {
    FILE *databaseFile = fopen(DATABASE_PATH, "ab+");
    if (!databaseFile) {
        fprintf(stderr, "Erro ao abrir %s: %s\n", DATABASE_PATH, strerror(errno));
        return 0;
    }
    
    if (fseeko(databaseFile, 0, SEEK_END) != 0) {
        fclose(databaseFile);
        return 0;
    }
    
    off_t offset = ftello(databaseFile);
    if (offset == -1) {
        fclose(databaseFile);
        return 0;
    }
    
    *offset_out = (uint64_t)offset;
    size_t wr = fwrite(data, 1, bytes, databaseFile);
    fclose(databaseFile);
    
    return wr == bytes;
}

//Lê o arquivo no database, guarda em buffer
// Retorna 1 se leu tudo, 0 se deu erro
int readData(uint64_t offset, uint32_t size, uint8_t *buffer) {
    FILE *databaseFile = fopen(DATABASE_PATH, "rb");
    if (!databaseFile) {
        fprintf(stderr, "Erro ao abrir %s\n", DATABASE_PATH);
        return 0;
    }
    
    if (fseeko(databaseFile, (off_t)offset, SEEK_SET) != 0) {
        fclose(databaseFile);
        return 0;
    }
    
    size_t fileRead = fread(buffer, 1, size, databaseFile);
    fclose(databaseFile);
    
    return fileRead == size;
}

#include "../algos/bTree.h"

// Estrutura auxiliar para compactação
typedef struct {
    FILE *newDatabase;
    FILE *oldDatabase;
    FILE *newIndex;
    int copied;
    int errors;
} CompactContext;

// Callback para compactar cada imagem da B-tree
static void compactCallback(const IImage *image, void *userData) {
    CompactContext *ctx = (CompactContext *)userData;
    
    uint8_t *buffer = malloc(image->size);
    if (!buffer) {
        fprintf(stderr, "Erro ao alocar memória para compactação\n");
        ctx->errors++;
        return;
    }
    
    if (fseeko(ctx->oldDatabase, (off_t)image->offset, SEEK_SET) != 0 ||
        fread(buffer, 1, image->size, ctx->oldDatabase) != image->size) {
        fprintf(stderr, "Erro ao ler dados do database\n");
        free(buffer);
        ctx->errors++;
        return;
    }
    
    off_t newOffset = ftello(ctx->newDatabase);
    if (newOffset == -1) {
        fprintf(stderr, "Erro ao obter offset\n");
        free(buffer);
        ctx->errors++;
        return;
    }
    
    if (fwrite(buffer, 1, image->size, ctx->newDatabase) != image->size) {
        fprintf(stderr, "Erro ao escrever no novo database\n");
        free(buffer);
        ctx->errors++;
        return;
    }
    
    free(buffer);
    
    IImage updatedImage = *image;
    updatedImage.offset = (uint64_t)newOffset;
    
    if (fwrite(&updatedImage, sizeof(IImage), 1, ctx->newIndex) != 1) {
        fprintf(stderr, "Erro ao escrever registro temporário\n");
        ctx->errors++;
        return;
    }
    
    ctx->copied++;
}

int compactDatabase(void) {
    const char *TEMP_DATABASE_PATH = "bin/database_temp.bin";
    const char *TEMP_INDEX_PATH = "bin/index_temp.bin";
    
    if (!openBTree()) {
        fprintf(stderr, "Erro ao abrir B-tree para compactação\n");
        return 0;
    }
    
    FILE *oldDatabase = fopen(DATABASE_PATH, "rb");
    if (!oldDatabase) {
        fprintf(stderr, "Erro ao abrir banco de dados para compactação\n");
        return 0;
    }
    
    FILE *newDatabase = fopen(TEMP_DATABASE_PATH, "wb");
    if (!newDatabase) {
        fprintf(stderr, "Erro ao criar banco de dados temporário\n");
        fclose(oldDatabase);
        return 0;
    }
    
    FILE *tempIndex = fopen(TEMP_INDEX_PATH, "wb");
    if (!tempIndex) {
        fprintf(stderr, "Erro ao criar índice temporário\n");
        fclose(oldDatabase);
        fclose(newDatabase);
        return 0;
    }
    
    CompactContext ctx = {
        .newDatabase = newDatabase,
        .oldDatabase = oldDatabase,
        .newIndex = tempIndex,
        .copied = 0,
        .errors = 0
    };
    
    traverseBTree(compactCallback, &ctx);
    
    fclose(oldDatabase);
    fclose(newDatabase);
    fclose(tempIndex);
    
    if (ctx.errors > 0) {
        fprintf(stderr, "Ocorreram %d erros durante a compactação\n", ctx.errors);
        remove(TEMP_DATABASE_PATH);
        remove(TEMP_INDEX_PATH);
        return 0;
    }
    
    closeBTree();

    if (remove(DATABASE_PATH) != 0) {
        fprintf(stderr, "Erro ao remover database antigo\n");
        return 0;
    }
    if (rename(TEMP_DATABASE_PATH, DATABASE_PATH) != 0) {
        fprintf(stderr, "Erro ao renomear database\n");
        return 0;
    }
    
    remove(INDEX_PATH);
    initBTree();
    
    if (!openBTree()) {
        fprintf(stderr, "Erro ao reabrir B-tree\n");
        return 0;
    }
    
    tempIndex = fopen(TEMP_INDEX_PATH, "rb");
    if (tempIndex) {
        IImage image;
        while (fread(&image, sizeof(IImage), 1, tempIndex) == 1) {
            insertBTree(&image);
        }
        fclose(tempIndex);
    }
    
    remove(TEMP_INDEX_PATH);
    
    printf("Compactação concluída: %d registros copiados.\n", ctx.copied);
    return 1;
}

