#include <stdio.h>
#include <stdlib.h>
#include "../include/bmp_io.h"

/**
 * Carrega uma imagem BMP em memória.
 * Lê os cabeçalhos do arquivo e os dados de pixel (sem padding).
 */
unsigned char* loadBMP(const char* filename, BITMAPFILEHEADER *fileHeader, BITMAPINFOHEADER *infoHeader) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    // Lê cabeçalho do arquivo BMP (14 bytes)
    if (fread(fileHeader, sizeof(BITMAPFILEHEADER), 1, f) != 1) {
        perror("Erro ao ler cabeçalho do arquivo BMP");
        fclose(f);
        exit(1);
    }

    // Lê cabeçalho de informações da imagem BMP (40 bytes)
    if (fread(infoHeader, sizeof(BITMAPINFOHEADER), 1, f) != 1) {
        perror("Erro ao ler cabeçalho da imagem BMP");
        fclose(f);
        exit(1);
    }

    // Verifica se é realmente um arquivo BMP
    if (fileHeader->bfType != 0x4D42) { // 'BM' em hexadecimal
        printf("Não é um arquivo BMP válido.\n");
        fclose(f);
        exit(1);
    }

    int width = infoHeader->biWidth;
    int height = abs(infoHeader->biHeight);

    // Remove padding (usa largura real em bytes sem alinhamento múltiplo de 4)
    int row_padded = width * 3;
    int data_size = row_padded * height;

    // Aloca memória para os dados da imagem
    unsigned char* data = (unsigned char*)malloc(data_size);
    if (!data) {
        perror("Erro de alocação");
        fclose(f);
        exit(1);
    }

    // Move o ponteiro de leitura para o início dos dados da imagem
    fseek(f, fileHeader->bfOffBits, SEEK_SET);

    // Lê os dados da imagem linha por linha
    for (int i = 0; i < height; i++) {
        if (fread(data + i * row_padded, sizeof(unsigned char), row_padded, f) != row_padded) {
            perror("Erro ao ler linha da imagem BMP");
            free(data);
            fclose(f);
            exit(1);
        }
    }

    fclose(f);
    return data;
}

/**
 * Salva uma imagem BMP no disco.
 * Assume que a imagem não possui padding entre as linhas.
 */
void saveBMP(const char* filename, BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader, unsigned char* data) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Erro ao criar arquivo");
        exit(1);
    }

    // Escreve os cabeçalhos
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, f);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, f);

    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);

    // Remove padding (mesmo tratamento que no loadBMP)
    int row_padded = width * 3;
    int data_size = row_padded * height;

    // Escreve os dados da imagem
    fwrite(data, sizeof(unsigned char), data_size, f);

    fclose(f);
}
