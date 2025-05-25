#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bmp_io.h"

// Função de conversão para tons de cinza
void converterParaTonsDeCinza(unsigned char* data, BITMAPINFOHEADER infoHeader) {
    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    // int row_padded = (width * 3 + 3) & (~3);
    int row_padded = width * 3;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int pos = i * row_padded + j * 3;

            unsigned char B = data[pos];
            unsigned char G = data[pos + 1];
            unsigned char R = data[pos + 2];

            // Aplicando a fórmula do cinza
            unsigned char gray = (unsigned char)(0.3 * R + 0.587 * G + 0.114 * B);

            // Define todos os canais para o tom de cinza
            data[pos]     = gray;  // B
            data[pos + 1] = gray;  // G
            data[pos + 2] = gray;  // R
        }
    }
}
