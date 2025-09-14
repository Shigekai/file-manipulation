#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>

int pgmToBinary(const char* filename, char* format, int* width, int* height, int* maxValue);
int thresholdBinaryToPgm(const char* filename, int threshold);

#define MAX_FILES 50
#define MAX_FILENAME 256

char getch() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int listPgmFiles(char files[][MAX_FILENAME]) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;
    
    dir = opendir("assets");
    if (dir == NULL) {
        printf("Erro: Não foi possível abrir a pasta assets!\n");
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL && count < MAX_FILES) {
        char *dot = strrchr(entry->d_name, '.');
        if (dot && strcmp(dot, ".pgm") == 0) {
            // Remove a extensão .pgm
            strncpy(files[count], entry->d_name, strlen(entry->d_name) - 4);
            files[count][strlen(entry->d_name) - 4] = '\0';
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

void displayMenu(char files[][MAX_FILENAME], int fileCount, int selected) {
    system("clear"); 
    
    printf("=== PROCESSADOR DE IMAGENS PGM ===\n");
    printf("Use ↑/↓ para navegar, ENTER para selecionar, Q para sair\n\n");
    printf("Selecione uma imagem para limiarizar:\n\n");
    
    for (int i = 0; i < fileCount; i++) {
        if (i == selected) {
            printf("► %s.pgm\n", files[i]);
        } else {
            printf("  %s.pgm\n", files[i]);
        }
    }
    
    if (fileCount == 0) {
        printf("Nenhum arquivo .pgm encontrado na pasta assets!\n");
    }
}

void processImage(const char* filename) {
    char format[3];
    int width, height, maxValue;
    int threshold;
    
    printf("\n=== PROCESSANDO: %s.pgm ===\n", filename);
    
    printf("Etapa 1/3: Convertendo PGM para binário...\n");
    int result = pgmToBinary(filename, format, &width, &height, &maxValue);
    
    if (result != 0) {
        printf("Erro ao converter arquivo para binário!\n");
        exit(1); 
    }
    
    printf("✓ Conversão concluída! Arquivo salvo em bin/%s.bin\n", filename);
    printf("  Formato: %s, Dimensões: %dx%d, MaxValue: %d\n", format, width, height, maxValue);
    
    printf("\nEtapa 2/3: Definindo valor de limiarização...\n");
    printf("Digite o valor de threshold (0-%d): ", maxValue);
    scanf("%d", &threshold);
    
    if (threshold < 0 || threshold > maxValue) {
        printf("Valor inválido! Usando threshold padrão: %d\n", maxValue/2);
        threshold = maxValue/2;
    }
    
    printf("\nEtapa 3/3: Aplicando limiarização (threshold=%d)...\n", threshold);
    result = thresholdBinaryToPgm(filename, threshold);
    
    if (result != 0) {
        printf("Erro ao aplicar limiarização!\n");
        exit(1); 
    }
    
    printf("✓ Processamento concluído com sucesso!\n");
    printf("\nVocê limiarizou a imagem =), veja o resultado em assets/%s-thresholded.pgm\n", filename);
    exit(0); 
}

int main() {
    char files[MAX_FILES][MAX_FILENAME];
    int fileCount;
    int selected = 0;
    char key;
    
    fileCount = listPgmFiles(files);
    
    if (fileCount == 0) {
        printf("Nenhum arquivo .pgm encontrado na pasta assets!\n");
        printf("Adicione alguns arquivos .pgm e tente novamente.\n");
        return 1;
    }
    
    while (1) {
        displayMenu(files, fileCount, selected);
        
        key = getch();
        
        switch (key) {
            case 27:
                key = getch();
                if (key == 91) { 
                    key = getch();
                    switch (key) {
                        case 65: 
                            selected = (selected - 1 + fileCount) % fileCount;
                            break;
                        case 66:
                            selected = (selected + 1) % fileCount;
                            break;
                    }
                }
                break;
                
            case 10:
            case 13:
                processImage(files[selected]);
                break;
                
            case 'q':
            case 'Q':
                printf("\nSaindo...\n");
                return 0;
                
            default:
                break;
        }
    }
    
    return 0;
}