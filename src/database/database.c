#include "database.h"
#include "indexHandlers.c"
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


// Compacta o banco de dados removendo entradas marcadas como deletadas
// Retorna 1 se sucesso, 0 se falha
int compactDatabase(void) {
    // Caminhos para arquivos temporários
    const char *TEMP_DATABASE_PATH = "bin/database_temp.bin";
    const char *TEMP_INDEX_PATH = "bin/index_temp.bin";
    
    FILE *sourceIndex = fopen(INDEX_PATH, "rb");
    if (!sourceIndex) {
        fprintf(stderr, "Erro ao abrir arquivo de índice para compactação\n");
        return 0;
    }
    
    FILE *sourceDatabase = fopen(DATABASE_PATH, "rb");
    if (!sourceDatabase) {
        fprintf(stderr, "Erro ao abrir banco de dados para compactação\n");
        fclose(sourceIndex);
        return 0;
    }
    
    FILE *destinationIndex = fopen(TEMP_INDEX_PATH, "wb");
    if (!destinationIndex) {
        fprintf(stderr, "Erro ao criar índice temporário\n");
        fclose(sourceIndex);
        fclose(sourceDatabase);
        return 0;
    }
    
    FILE *dstDatabase = fopen(TEMP_DATABASE_PATH, "wb");
    if (!dstDatabase) {
        fprintf(stderr, "Erro ao criar banco de dados temporário\n");
        fclose(sourceIndex);
        fclose(sourceDatabase);
        fclose(destinationIndex);
        return 0;
    }


    #define BUFFER_SIZE (64 * 1024) // 64 KB
    uint8_t *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Erro ao alocar memória para buffer\n");
        fclose(sourceIndex);
        fclose(sourceDatabase);
        fclose(destinationIndex);
        fclose(dstDatabase);
        return 0;
    }

    int copied = 0;
    uint64_t newOffset = 0;
    
    IImage image;
    long isAvailablePos;
    
    while (readNextData(sourceIndex, &image, &isAvailablePos)) {
        if (image.isAvailable) {
            if (fseeko(sourceDatabase, (off_t)image.offset, SEEK_SET) != 0) {
                fprintf(stderr, "Erro ao posicionar no banco de dados de origem\n");
                break;
            }
            
            uint32_t bytesRemaining = image.size;
            uint32_t totalCopied = 0;
            
            while (bytesRemaining > 0) {
                size_t blockSize = (bytesRemaining < BUFFER_SIZE) ? bytesRemaining : BUFFER_SIZE;
                
                size_t bytesRead = fread(buffer, 1, blockSize, sourceDatabase);
                if (bytesRead != blockSize) {
                    fprintf(stderr, "Erro ao ler do banco de dados de origem\n");
                    break;
                }
                
                size_t bytesWritten = fwrite(buffer, 1, bytesRead, dstDatabase);
                if (bytesWritten != bytesRead) {
                    fprintf(stderr, "Erro ao escrever no banco de dados temporário\n");
                    break;
                }
                
                bytesRemaining -= (uint32_t)bytesWritten;
                totalCopied += (uint32_t)bytesWritten;
            }
            
            if (totalCopied == image.size) {
                uint64_t oldOffset = image.offset;
                image.offset = newOffset;
                newOffset += image.size;
                
                uint16_t nameLength = (uint16_t)strlen(image.name);
                bool available = true;

                int success = 
                    writeField(destinationIndex, &nameLength, sizeof(uint16_t)) &&
                    fwrite(image.name, 1, nameLength, destinationIndex) == nameLength &&
                    writeField(destinationIndex, &image.offset, sizeof(uint64_t)) &&
                    writeField(destinationIndex, &image.size, sizeof(uint32_t)) &&
                    writeField(destinationIndex, &image.width, sizeof(uint32_t)) &&
                    writeField(destinationIndex, &image.height, sizeof(uint32_t)) &&
                    writeField(destinationIndex, &image.maxValue, sizeof(uint16_t)) &&
                    writeField(destinationIndex, &image.bpp, sizeof(uint8_t)) &&
                    writeField(destinationIndex, &available, sizeof(bool));
                
                if (success) {
                    copied++;
                } else {
                    fprintf(stderr, "Erro ao escrever no índice temporário\n");
                    break;
                }
            }
        }
        
        if (image.name) {
            free(image.name);
            image.name = NULL;
        }
    }
    
    free(buffer);
    fclose(sourceIndex);
    fclose(sourceDatabase);
    fclose(destinationIndex);
    fclose(dstDatabase);
    
    if (remove(DATABASE_PATH) != 0 || remove(INDEX_PATH) != 0 ||
        rename(TEMP_DATABASE_PATH, DATABASE_PATH) != 0 ||
        rename(TEMP_INDEX_PATH, INDEX_PATH) != 0) {
        fprintf(stderr, "Erro ao substituir arquivos originais\n");
        return 0;
    }
    
    printf("Compactação concluída: %d registros mantidos.\n", copied);
    return 1;
}