#include "imgdb.h"
#include "menu.h"
#include "pgm_io.h"
#include "store.h"
#include "indexdb.h"
#include "filters.h"

static void cmd_import(void) {
    char path[1024], name[512];
    printf("Caminho do PGM para importar: ");
    if (!fgets(path, sizeof(path), stdin)) return;
    path[strcspn(path, "\r\n")] = 0;

    printf("Nome de referência (chave secundária): ");
    if (!fgets(name, sizeof(name), stdin)) return;
    name[strcspn(name, "\r\n")] = 0;
    if (name[0] == '\0') { printf("Nome não pode ser vazio.\n"); return; }

    uint32_t W,H,MAX, BYTES;
    uint8_t *data=NULL; uint8_t BPP=0;
    if (!load_pgm_as_p5_bytes(path, &W, &H, &MAX, &data, &BYTES, &BPP)) {
        printf("Falha ao importar.\n"); return;
    }

    uint64_t off=0;
    if (!append_to_store(data, BYTES, &off)) {
        printf("Falha ao gravar no store.\n"); free(data); return;
    }
    free(data);

    KeyEntry e = {0};
    e.offset = off;
    e.size = BYTES;
    e.width = W; e.height = H; e.maxval = (uint16_t)MAX; e.bpp = BPP;

    if (!add_key_record(name, &e)) {
        printf("Falha ao gravar índice.\n"); return;
    }
    printf("OK! Importado '%s' -> offset=%llu, %ux%u, max=%u, bpp=%u, size=%u\n",
           name, (unsigned long long)off, W, H, MAX, BPP, BYTES);
}

static void cmd_export(void) {
    char name[512];
    printf("Nome da imagem (chave secundária) para exportar: ");
    if (!fgets(name, sizeof(name), stdin)) return;
    name[strcspn(name, "\r\n")] = 0;
    if (name[0]=='\0') { printf("Nome vazio.\n"); return; }

    KeyEntry e;
    if (!find_by_name_seq(name, &e)) {
        printf("Imagem '%s' não encontrada.\n", name);
        return;
    }

    printf("Escolha a operação:\n");
    printf("  0) Sem modificação\n");
    printf("  1) Limiarização\n");
    printf("  2) Negativação\n");
    printf("Opção: ");
    int mode = 0;
    if (scanf("%d", &mode) != 1) { free_key(&e); while(getchar()!='\n'); return; }
    while(getchar()!='\n');

    uint32_t th = 0;
    if (mode == 1) {
        printf("Valor de limiar (0..%u): ", e.maxval);
        if (scanf("%u", &th) != 1) { free_key(&e); while(getchar()!='\n'); return; }
        while(getchar()!='\n');
    }

    char out[1024];
    printf("Arquivo de saída (.pgm): ");
    if (!fgets(out, sizeof(out), stdin)) { free_key(&e); return; }
    out[strcspn(out, "\r\n")] = 0;

    if (export_image(&e, out, mode, th)) {
        printf("Exportado com sucesso em '%s'.\n", out);
    } else {
        printf("Falha na exportação.\n");
    }
    free_key(&e);
}

void menu(void) {
    for (;;) {
        printf("\n== IMGDB ==\n");
        printf("1) Importar PGM para o banco\n");
        printf("2) Listar imagens\n");
        printf("3) Exportar por nome (sem/limiar/negativo)\n");
        printf("0) Sair\n");
        printf("Escolha: ");
        int op=0;
        if (scanf("%d", &op) != 1) { while(getchar()!='\n'); continue; }
        while(getchar()!='\n');

        if (op==0) break;
        else if (op==1) cmd_import();
        else if (op==2) list_all();
        else if (op==3) cmd_export();
        else printf("Opcao invalida.\n");
    }
}
