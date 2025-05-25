#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bmp_io.h"
#include "conversao.h"
#include "filtro_mediana.h"
#include "filtro_laplaciano.h"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Uso: %s <entrada.bmp> <saida.bmp> <tamanho_mascara> <num_processos>\n", argv[0]);
        printf("Exemplo: %s imagens/entrada.bmp imagens/saida.bmp 3 4\n", argv[0]);
        exit(1);
    }

    const char* arquivo_entrada = argv[1];
    const char* arquivo_saida = argv[2];
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

    struct timespec inicio, fim;
    double tempo;

    // Carrega imagem
    unsigned char* image = loadBMP(arquivo_entrada, &fileHeader, &infoHeader);
    printf("Imagem carregada: %dx%d\n", infoHeader.biWidth, infoHeader.biHeight);

    // Conversão para tons de cinza
    converterParaTonsDeCinza(image, infoHeader);
    printf("Conversão para tons de cinza concluída.\n");

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // Filtro de mediana
    aplicarFiltroMedianaProcessos(image, infoHeader, tamanho_mascara, num_processos);
    printf("Filtro de mediana aplicado com máscara %dx%d.\n", tamanho_mascara, tamanho_mascara);




    aplicarFiltroLaplacianoProcessos(image, infoHeader, tamanho_mascara, num_processos);
    printf("Filtro Laplaciano paralelo aplicado com máscara %dx%d e %d processos.\n", tamanho_mascara, tamanho_mascara, num_processos);

    clock_gettime(CLOCK_MONOTONIC, &fim);

    tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    printf("Tempo de execução Laplaciano paralelo: %.6f segundos\n", tempo);

    // Salva saída
    saveBMP(arquivo_saida, fileHeader, infoHeader, image);
    printf("Imagem salva em: %s\n", arquivo_saida);

    free(image);
    return 0;
}
