#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PAGE_SIZE    256
#define NUM_PAGES    256
#define MAX_ENDERECO 65535
#ifndef FRAMES
#define FRAMES       128
#endif

enum { FIFO, LRU };

static int tabela_paginas[NUM_PAGES];
static signed char memoria[FRAMES * PAGE_SIZE];
static int pagina_do_frame[FRAMES];
static long ultimo_uso[FRAMES];
static long relogio = 0;
static int proximo_frame = 0;
static int proximo_fifo = 0;
static int page_faults = 0;

static int escolhe_vitima(int alg)
{
    if (alg == FIFO) {
        int v = proximo_fifo;
        proximo_fifo = (proximo_fifo + 1) % FRAMES;
        return v;
    }
    int v = 0;
    for (int f = 1; f < FRAMES; f++)
        if (ultimo_uso[f] < ultimo_uso[v])
            v = f;
    return v;
}

static int le_algoritmo(const char *s)
{
    if (strcmp(s, "fifo") == 0) return FIFO;
    if (strcmp(s, "lru") == 0)  return LRU;
    return -1;
}

static void limpa_fim(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t'))
        s[--n] = '\0';
}

static int *le_enderecos(const char *nome, int *qtd)
{
    FILE *f = fopen(nome, "r");
    if (f == NULL) {
        fprintf(stderr, "Erro: não foi possível abrir '%s': %s\n", nome, strerror(errno));
        return NULL;
    }

    int cap = 1024, n = 0, num_linha = 0;
    int *v = malloc(cap * sizeof(int));
    if (v == NULL) { fclose(f); fprintf(stderr, "Erro: memória insuficiente\n"); return NULL; }

    char linha[256];
    while (fgets(linha, sizeof linha, f) != NULL) {
        num_linha++;
        limpa_fim(linha);

        char *fim;
        errno = 0;
        long x = strtol(linha, &fim, 10);

        if (linha[0] == '\0' || *fim != '\0' || linha[0] == '-' || linha[0] == '+' || errno == ERANGE || x < 0 || x > MAX_ENDERECO) {
            fprintf(stderr, "Erro: formato inválido em '%s', linha %d: \"%s\" "
                            "(esperado um inteiro entre 0 e %d por linha)\n",
                    nome, num_linha, linha, MAX_ENDERECO);
            free(v); fclose(f); return NULL;
        }

        if (n == cap) {
            cap *= 2;
            int *novo = realloc(v, cap * sizeof(int));
            if (novo == NULL) { free(v); fclose(f); fprintf(stderr, "Erro: memória insuficiente\n"); return NULL; }
            v = novo;
        }
        v[n++] = (int) x;
    }
    fclose(f);

    if (n == 0) {
        fprintf(stderr, "Erro: o arquivo '%s' está vazio\n", nome);
        free(v);
        return NULL;
    }
    *qtd = n;
    return v;
}

static int carrega_pagina(FILE *bs, int pagina, int alg)
{
    int frame;
    if (proximo_frame < FRAMES) {
        frame = proximo_frame++;
    } else {
        frame = escolhe_vitima(alg);
    }
    if (fseek(bs, (long) pagina * PAGE_SIZE, SEEK_SET) != 0 || fread(&memoria[frame * PAGE_SIZE], 1, PAGE_SIZE, bs) != PAGE_SIZE) {
        fprintf(stderr, "Erro: falha ao ler a página %d do BACKING_STORE.bin\n", pagina);
        return -1;
    }
    tabela_paginas[pagina] = frame;
    pagina_do_frame[frame] = pagina;
    page_faults++;
    return frame;
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <arquivo_de_enderecos> <fifo|lru> <fifo|lru>\n", argv[0]);
        return 1;
    }
    int alg_pagina = le_algoritmo(argv[2]);
    int alg_tlb    = le_algoritmo(argv[3]);
    if (alg_pagina < 0 || alg_tlb < 0) {
        fprintf(stderr, "Erro: algoritmo inválido ('%s', '%s'); use fifo ou lru\n", argv[2], argv[3]);
        return 1;
    }

    int qtd;
    int *enderecos = le_enderecos(argv[1], &qtd);
    if (enderecos == NULL) return 1;

    FILE *bs = fopen("BACKING_STORE.bin", "rb");
    if (bs == NULL) {
        fprintf(stderr, "Erro: não foi possível abrir 'BACKING_STORE.bin': %s\n", strerror(errno));
        free(enderecos);
        return 1;
    }
    FILE *saida = fopen("correct.txt", "w");
    if (saida == NULL) {
        fprintf(stderr, "Erro: não foi possível criar 'correct.txt': %s\n", strerror(errno));
        fclose(bs);
        free(enderecos);
        return 1;
    }

    for (int i = 0; i < NUM_PAGES; i++)
        tabela_paginas[i] = -1;

    for (int i = 0; i < qtd; i++) {
        int pagina = (enderecos[i] >> 8) & 0xFF;
        int offset = enderecos[i] & 0xFF;

        int frame = tabela_paginas[pagina];
        if (frame == -1) {
            frame = carrega_pagina(bs, pagina, alg_pagina);
            if (frame == -1) {
                fclose(saida); fclose(bs); free(enderecos);
                return 1;
            }
        }

        ultimo_uso[frame] = ++relogio;

        int fisico = frame * PAGE_SIZE + offset;
        fprintf(saida, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
                enderecos[i], -1, fisico, memoria[fisico]);
    }

    fprintf(saida, "Number of Translated Addresses = %d\n", qtd);
    fprintf(saida, "Page Faults = %d\n", page_faults);
    fprintf(saida, "Page Fault Rate = %.3f\n", (double) page_faults / qtd);
    fprintf(saida, "TLB Hits = %d\n", 0);
    fprintf(saida, "TLB Hit Rate = %.3f\n", 0.0);

    fclose(saida);
    fclose(bs);
    free(enderecos);
    return 0;
}
