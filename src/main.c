#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bmp_io.h"
#include "conversao.h"
#include "filtro_mediana.h"
#include "filtro_laplaciano.h"

#define N_EXECUCOES 5

double medir_tempo_execucao(void (*funcao)(unsigned char*, BITMAPINFOHEADER, int, int),
                            unsigned char* imagem, BITMAPINFOHEADER infoHeader, int tamanho, int processos) {
    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    funcao(imagem, infoHeader, tamanho, processos);

    clock_gettime(CLOCK_MONOTONIC, &fim);

    return (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
}

// Copia uma imagem (alocação + memcpy)
unsigned char* copiarImagem(unsigned char* original, BITMAPINFOHEADER infoHeader) {
    int tamanho = infoHeader.biHeight * infoHeader.biWidth;
    unsigned char* copia = malloc(3 * tamanho); // 3 canais RGB
    if (copia == NULL) {
        perror("Erro ao alocar memória para cópia");
        exit(1);
    }
    memcpy(copia, original, 3 * tamanho);
    return copia;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Uso: %s <entrada.bmp> <saida.bmp> <tamanho_mascara> <num_processos>\n", argv[0]);
        printf("Exemplo: %s images/entrada.bmp images/saida.bmp 3 4\n", argv[0]);
        exit(1);
    }

    const char* arquivo_entrada = argv[1];

    char arquivo_saida[100];
    snprintf(arquivo_saida, sizeof(arquivo_saida), "%s_%s.bmp", argv[2], argv[3]);

    int tamanho_mascara = atoi(argv[3]);
    int num_processos = atoi(argv[4]);

    if (tamanho_mascara != 3 && tamanho_mascara != 5 && tamanho_mascara != 7) {
        printf("Tamanho de máscara inválido. Use 3, 5 ou 7.\n");
        exit(1);
    }

    if (num_processos < 1) {
        printf("Número de processos deve ser maior ou igual a 1.\n");
        exit(1);
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    // Carrega imagem
    unsigned char* image = loadBMP(arquivo_entrada, &fileHeader, &infoHeader);
    printf("Imagem carregada: %dx%d\n", infoHeader.biWidth, infoHeader.biHeight);

    // Conversão para tons de cinza
    converterParaTonsDeCinza(image, infoHeader);
    printf("Conversão para tons de cinza concluída.\n");

    double tempo_total_mediana = 0;
    double tempo_total_laplaciano = 0;

    for (int i = 0; i < N_EXECUCOES; i++) {
        printf("Execução %d/%d:\n", i + 1, N_EXECUCOES);

        unsigned char* copia1 = copiarImagem(image, infoHeader);
        double t1 = medir_tempo_execucao(aplicarFiltroMedianaProcessos, copia1, infoHeader, tamanho_mascara, num_processos);
        printf("  Tempo mediana: %.6f s\n", t1);
        tempo_total_mediana += t1;
        free(copia1);

        unsigned char* copia2 = copiarImagem(image, infoHeader);
        double t2 = medir_tempo_execucao(aplicarFiltroLaplacianoProcessos, copia2, infoHeader, tamanho_mascara, num_processos);
        printf("  Tempo laplaciano: %.6f s\n", t2);
        tempo_total_laplaciano += t2;
        free(copia2);
    }

    double media_mediana = tempo_total_mediana / N_EXECUCOES;
    double media_laplaciano = tempo_total_laplaciano / N_EXECUCOES;

    printf("\nTempo médio filtro mediana: %.6f segundos\n", media_mediana);
    printf("Tempo médio filtro Laplaciano: %.6f segundos\n", media_laplaciano);

    // Aplica filtros em sequência na imagem original para salvar corretamente
    aplicarFiltroMedianaProcessos(image, infoHeader, tamanho_mascara, num_processos);
    aplicarFiltroLaplacianoProcessos(image, infoHeader, tamanho_mascara, num_processos);
    saveBMP(arquivo_saida, fileHeader, infoHeader, image);

    // Log em arquivo
    FILE* log = fopen("output/tempo_execucao.log", "a");
    if (log) {
        fprintf(log, "Arquivo: %s | Mascara: %d | Processos: %d | Média Mediana: %.6f s | Média Laplaciano: %.6f s\n",
                arquivo_entrada, tamanho_mascara, num_processos, media_mediana, media_laplaciano);
        fclose(log);
    } else {
        perror("Erro ao escrever log");
    }

    free(image);
    return 0;
}
