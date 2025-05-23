#include <stdio.h>
#include <stdlib.h>
#include "bmp_io.h"

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
