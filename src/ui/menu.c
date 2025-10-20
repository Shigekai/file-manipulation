#include "headers.h"
#include "menu.h"
#include "pgm.h"
#include "database.h"
#include "index.h"
#include "handleImages.h"
#include "menuHandlers.c"

//Este método usa as funções auxiliares de menuHandlers.c para exibir o menu de interação

void startMenu(void) {
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║       SISTEMA DE BANCO DE IMAGENS     ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    while (1) {
        printf("\n┌─── MENU PRINCIPAL ───┐\n");
        printf("│ 1) Importar imagem   │\n");
        printf("│ 2) Listar imagens    │\n");
        printf("│ 3) Exportar imagem   │\n");
        printf("│ 4) Deletar imagem    │\n");
        printf("│ 0) Sair              │\n");
        printf("└──────────────────────┘\n");
        printf("Escolha: ");
        
        int option = 0;
        if (scanf("%d", &option) != 1) {
            printf("❌ Entrada inválida.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (option) {
            case 0:
                printf("\nEncerrando sistema...\n");
                return;
                
            case 1:
                commandImport();
                break;
                
            case 2:
                commandList();
                break;
                
            case 3:
                commandExport();
                break;
                
            case 4:
                commandDelete();
                break;
                
            default:
                printf("❌ Opção inválida. Tente novamente.\n");
                break;
        }
    }
}