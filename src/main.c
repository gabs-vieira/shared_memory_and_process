#include <stdio.h>
#include <stdlib.h>
#include "bmp_io.h"
#include "conversao.h"


int main() {
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    // Carregar a imagem BMP
    unsigned char* image = loadBMP("imagens/entrada.bmp", &fileHeader, &infoHeader);

    printf("Imagem carregada: %dx%d\n", infoHeader.biWidth, infoHeader.biHeight);

    // Conversão para tons de cinza
    converterParaTonsDeCinza(image, infoHeader);
    printf("Conversão para tons de cinza concluída.\n");

    saveBMP("imagens/saida_filtro_cinza.bmp", fileHeader, infoHeader, image);

    // Aplicar filtro de mediana (ex.: máscara 3x3)
    aplicarFiltroMediana(image, infoHeader, 3);
    printf("Filtro de mediana aplicado.\n");

    // Salvar imagem de saída
    saveBMP("imagens/saida_mediana.bmp", fileHeader, infoHeader, image);

    // Aplicar filtro Laplaciano
    //aplicarFiltroLaplaciano(image, infoHeader, 3);
    aplicarFiltroLaplacianoProcessos(image, infoHeader, 3, 4);
    printf("Filtro Laplaciano aplicado.\n");

    // Salvar resultado
    saveBMP("imagens/saida_final.bmp", fileHeader, infoHeader, image);
    
    free(image);
    return 0;
}
