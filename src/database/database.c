#include "database.h"

//Observação: usado fseeko/ftello em vez de fseek/ftell para garantir uma abordagem moderna =)

//Garante que o arquivo binário pôde ser aberto/criado com sucesso...
void ensureBin(void) {
    FILE *databaseFile = fopen(DATABASE_PATH, "ab");
    if (!databaseFile) {
        MKDIR("bin");
        databaseFile = fopen(DATABASE_PATH, "ab");
        if (!databaseFile) {
            fprintf(stderr, "Erro ao criar/abrir %s: %s\n", DATABASE_PATH, strerror(errno));
            exit(1);
        }
    }
    fclose(databaseFile);
    
    databaseFile = fopen(INDEX_PATH, "ab");
    if (!databaseFile) {
        MKDIR("bin");
        databaseFile = fopen(INDEX_PATH, "ab");
        if (!databaseFile) {
            fprintf(stderr, "Erro ao criar/abrir %s: %s\n", INDEX_PATH, strerror(errno));
            exit(1);
        }
    }
    fclose(databaseFile);
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