#include <stdio.h>
#include <stdlib.h>
#include "bmp_io.h"

// Carregar imagem BMP
unsigned char* loadBMP(const char* filename, BITMAPFILEHEADER *fileHeader, BITMAPINFOHEADER *infoHeader) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    if (fread(fileHeader, sizeof(BITMAPFILEHEADER), 1, f) != 1) {
        perror("Erro ao ler cabeçalho do arquivo BMP");
        exit(1);
    }

    if (fread(infoHeader, sizeof(BITMAPINFOHEADER), 1, f) != 1) {
        perror("Erro ao ler cabeçalho da imagem BMP");
        exit(1);
    }

    if (fileHeader->bfType != 0x4D42) {
        printf("Não é um arquivo BMP válido.\n");
        exit(1);
    }

    int width = infoHeader->biWidth;
    int height = abs(infoHeader->biHeight);
    int row_padded = (width * 3 + 3) & (~3);
    int data_size = row_padded * height;

    unsigned char* data = (unsigned char*)malloc(data_size);
    if (!data) {
        perror("Erro de alocação");
        exit(1);
    }

    fseek(f, fileHeader->bfOffBits, SEEK_SET);
    if (fread(data, sizeof(unsigned char), data_size, f) != data_size) {
        perror("Erro ao ler dados da imagem BMP");
        exit(1);
    }

    fclose(f);
    return data;
}

// Salvar imagem BMP
void saveBMP(const char* filename, BITMAPFILEHEADER fileHeader, BITMAPINFOHEADER infoHeader, unsigned char* data) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Erro ao criar arquivo");
        exit(1);
    }

    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, f);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, f);

    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    int row_padded = (width * 3 + 3) & (~3);
    int data_size = row_padded * height;

    fwrite(data, sizeof(unsigned char), data_size, f);
    fclose(f);
}
