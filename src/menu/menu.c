#include "headers.h"
#include "menu.h"
#include "menuHandlers.c"
#include "../algos/bTree.h" 

void showMenu(void) {
    int choice;
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║          BEM-VINDO AO BANCO DE IMAGENS PGM             ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    do {
        printf("\n╔════════════════════════════════════════════════════════╗\n");
        printf("║                      MENU PRINCIPAL                    ║\n");
        printf("╠════════════════════════════════════════════════════════╣\n");
        printf("║  1) Importar imagem                                    ║\n");
        printf("║  2) Exportar imagem                                    ║\n");
        printf("║  3) Listar imagens                                     ║\n");
        printf("║  4) Deletar imagem                                     ║\n");
        printf("║  5) Compactar banco de dados                           ║\n");
        printf("║  6) Ordenar índice (Merge Sort Externo)                ║\n");
        printf("║  0) Sair                                               ║\n");
        printf("╚════════════════════════════════════════════════════════╝\n");
        printf("➤ Escolha uma opção: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("\n❌ Entrada inválida! Digite um número entre 0 e 6.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        printf("\n"); 
        
        if (choice != 0) {
            handleMenuChoice(choice);
            
            printf("\n[Pressione ENTER para continuar]");
            getchar();
        }
        
    } while (choice != 0);
    
    closeBTree();
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║               Programa encerrado. Até logo!            ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
}

void handleMenuChoice(int choice) {
    switch (choice) {
        case 1: commandImport(); break;
        case 2: commandExport(); break;
        case 3: listBTree(); break;
        case 4: commandDelete(); break;
        case 5: commandCompact(); break;
        case 6: commandSort(); break;
        case 0: 
            break;
        default: 
            printf("❌ Opção inválida! Escolha um número entre 0 e 6.\n");
    }
}