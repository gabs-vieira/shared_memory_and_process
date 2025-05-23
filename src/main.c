#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Definições dos headers BMP
#pragma pack(push,1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

// Função para carregar BMP
unsigned char* loadBMP(const char* filename, BITMAPFILEHEADER *fileHeader, BITMAPINFOHEADER *infoHeader) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    fread(fileHeader, sizeof(BITMAPFILEHEADER), 1, f);
    fread(infoHeader, sizeof(BITMAPINFOHEADER), 1, f);

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
    fread(data, sizeof(unsigned char), data_size, f);

    fclose(f);
    return data;
}

// Função para salvar BMP
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

int main() {
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    // Carregar a imagem
    unsigned char* image = loadBMP("../imagens/entrada.bmp", &fileHeader, &infoHeader);

    printf("Imagem carregada: %dx%d\n", infoHeader.biWidth, infoHeader.biHeight);

    // (Aqui você aplicaria os filtros)

    // Salvar imagem de saída
    saveBMP("../imagens/saida.bmp", fileHeader, infoHeader, image);

    free(image);
    return 0;
}
