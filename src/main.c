#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/bmp_io.h"
#include "../include/conversao.h"
#include "../include/filtro_laplaciano.h"

#define N_EXECUCOES 5

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Uso: %s <entrada.bmp> <saida.bmp> <tamanho_mascara> <num_processos>\n", argv[0]);
        return 1;
    }

    const char* arquivo_entrada = argv[1];
    char arquivo_saida[100];
    snprintf(arquivo_saida, sizeof(arquivo_saida), "%s_%s.bmp", argv[2], argv[3]);

    int tamanho_mascara = atoi(argv[3]);
    int num_processos = atoi(argv[4]);

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    unsigned char* image = loadBMP(arquivo_entrada, &fileHeader, &infoHeader);
    if (!image) {
        printf("Erro ao carregar imagem.\n");
        return 1;
    }

    converterParaTonsDeCinza(image, infoHeader);
    printf("Conversão para tons de cinza concluída.\n");

    aplicarFiltroLaplacianoProcessos(image, infoHeader, tamanho_mascara, num_processos);
    saveBMP(arquivo_saida, fileHeader, infoHeader, image);

    free(image);
    printf("Imagem salva em %s\n", arquivo_saida);
    return 0;
}
