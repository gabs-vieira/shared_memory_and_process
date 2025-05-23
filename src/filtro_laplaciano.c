#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bmp_io.h"
#include "filtro_laplaciano.h"

// Definir máscaras Laplacianas para diferentes tamanhos
// Aqui, como exemplo, usamos a máscara 3x3 clássica
void gerarMascaraLaplaciana(int tamanhoMascara, int* mascara) {
    if (tamanhoMascara == 3) {
        int temp[9] = {
             0, -1,  0,
            -1,  4, -1,
             0, -1,  0
        };
        for (int i = 0; i < 9; i++) mascara[i] = temp[i];
    } else {
        // Para tamanhos maiores, uma abordagem mais genérica seria necessária.
        // Podemos discutir depois, mas neste exemplo vamos focar no 3x3.
        printf("Tamanho de máscara não suportado no momento.\n");
        exit(1);
    }
}

void aplicarFiltroLaplaciano(unsigned char* data, BITMAPINFOHEADER infoHeader, int tamanhoMascara) {
    if (tamanhoMascara != 3) {
        printf("Somente tamanho de máscara 3x3 suportado neste exemplo.\n");
        exit(1);
    }

    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    int row_padded = (width * 3 + 3) & (~3);

    int offset = tamanhoMascara / 2;

    unsigned char* copia = (unsigned char*)malloc(row_padded * height);
    if (!copia) {
        perror("Erro de alocação na cópia da imagem");
        exit(1);
    }

    // Copia os dados originais
    for (int i = 0; i < row_padded * height; i++) {
        copia[i] = data[i];
    }

    int mascara[9];
    gerarMascaraLaplaciana(tamanhoMascara, mascara);

    // Percorrer a imagem ignorando as bordas
    for (int i = offset; i < height - offset; i++) {
        for (int j = offset; j < width - offset; j++) {
            int soma = 0;
            int count = 0;

            // Aplicar a máscara
            for (int mi = -offset; mi <= offset; mi++) {
                for (int mj = -offset; mj <= offset; mj++) {
                    int y = i + mi;
                    int x = j + mj;

                    int pos = y * row_padded + x * 3;
                    unsigned char valor = copia[pos]; // Apenas um canal (todos são iguais no cinza)

                    soma += valor * mascara[count];
                    count++;
                }
            }

            // Clamp (limitar entre 0 e 255)
            if (soma < 0) soma = 0;
            if (soma > 255) soma = 255;

            int pos = i * row_padded + j * 3;
            data[pos]     = (unsigned char)soma;
            data[pos + 1] = (unsigned char)soma;
            data[pos + 2] = (unsigned char)soma;
        }
    }

    free(copia);
}
