#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bmp_io.h"
#include "filtro_mediana.h"

// Função de comparação para qsort
int compara(const void* a, const void* b) {
    return (*(unsigned char*)a - *(unsigned char*)b);
}

void aplicarFiltroMediana(unsigned char* data, BITMAPINFOHEADER infoHeader, int tamanhoMascara) {
    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    int row_padded = (width * 3 + 3) & (~3);

    int offset = tamanhoMascara / 2;
    int num_pixels = tamanhoMascara * tamanhoMascara;

    unsigned char* copia = (unsigned char*)malloc(row_padded * height);
    if (!copia) {
        perror("Erro de alocação na cópia da imagem");
        exit(1);
    }

    // Faz uma cópia da imagem original
    for (int i = 0; i < row_padded * height; i++) {
        copia[i] = data[i];
    }

    unsigned char* vizinhos = (unsigned char*)malloc(num_pixels);
    if (!vizinhos) {
        perror("Erro de alocação no vetor de vizinhos");
        exit(1);
    }

    // Percorre a imagem ignorando as bordas
    for (int i = offset; i < height - offset; i++) {
        for (int j = offset; j < width - offset; j++) {
            int count = 0;

            // Coleta os pixels da vizinhança
            for (int mi = -offset; mi <= offset; mi++) {
                for (int mj = -offset; mj <= offset; mj++) {
                    int y = i + mi;
                    int x = j + mj;

                    int pos = y * row_padded + x * 3;
                    vizinhos[count++] = copia[pos]; // Só pega o canal B (todos são iguais no cinza)
                }
            }

            // Ordena os valores
            qsort(vizinhos, num_pixels, sizeof(unsigned char), compara);

            unsigned char mediana = vizinhos[num_pixels / 2];

            // Atribui a mediana para os três canais (B, G, R)
            int pos = i * row_padded + j * 3;
            data[pos]     = mediana;
            data[pos + 1] = mediana;
            data[pos + 2] = mediana;
        }
    }

    free(copia);
    free(vizinhos);
}
