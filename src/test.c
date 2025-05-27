#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma pack(push, 1)
typedef struct {
    unsigned short bfType;       // 'BM' = 0x4D42
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BMPHeader;

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

unsigned char* lerBMP(const char* filename, int* width, int* height, BMPHeader* header, BMPInfoHeader* infoHeader) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Erro ao abrir arquivo BMP\n");
        return NULL;
    }

    fread(header, sizeof(BMPHeader), 1, f);
    if (header->bfType != 0x4D42) {
        printf("Arquivo não é BMP válido\n");
        fclose(f);
        return NULL;
    }

    fread(infoHeader, sizeof(BMPInfoHeader), 1, f);

    if (infoHeader->biBitCount != 24) {
        printf("Somente BMP 24 bits suportado\n");
        fclose(f);
        return NULL;
    }

    *width = infoHeader->biWidth;
    *height = infoHeader->biHeight;

    fseek(f, header->bfOffBits, SEEK_SET);

    int tamanhoLinha = ((*width * 3 + 3) / 4) * 4; // alinhamento DWORD
    unsigned char* dados = (unsigned char*)malloc(tamanhoLinha * (*height));
    fread(dados, 1, tamanhoLinha * (*height), f);

    fclose(f);
    return dados;
}

void salvarBMP(const char* filename, BMPHeader* header, BMPInfoHeader* infoHeader, unsigned char* dados) {
    FILE* f = fopen(filename, "wb");
    fwrite(header, sizeof(BMPHeader), 1, f);
    fwrite(infoHeader, sizeof(BMPInfoHeader), 1, f);

    int tamanhoLinha = ((infoHeader->biWidth * 3 + 3) / 4) * 4;
    fwrite(dados, 1, tamanhoLinha * infoHeader->biHeight, f);

    fclose(f);
}

void converterParaTonsDeCinza(unsigned char* dados, int largura, int altura) {
    int tamanhoLinha = ((largura * 3 + 3) / 4) * 4;
    for (int y = 0; y < altura; y++) {
        unsigned char* linha = dados + y * tamanhoLinha;
        for (int x = 0; x < largura; x++) {
            unsigned char* pixel = linha + x * 3;
            unsigned char cinza = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
            pixel[0] = pixel[1] = pixel[2] = cinza;
        }
    }
}


#include <stdlib.h>

// Função auxiliar para comparar pixels (para qsort)
int comparar(const void* a, const void* b) {
    return (*(unsigned char*)a) - (*(unsigned char*)b);
}

void filtroMediana(unsigned char* dados, unsigned char* saida, int largura, int altura, int tamanhoMascara) {
    int raio = tamanhoMascara / 2;
    int tamanhoJanela = tamanhoMascara * tamanhoMascara;
    unsigned char* janela = (unsigned char*)malloc(tamanhoJanela);

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            int idx_saida = y * largura + x;

            // Coletar pixels da janela ao redor (com tratamento de borda replicando borda)
            int count = 0;
            for (int dy = -raio; dy <= raio; dy++) {
                int yy = y + dy;
                if (yy < 0) yy = 0;
                if (yy >= altura) yy = altura - 1;

                for (int dx = -raio; dx <= raio; dx++) {
                    int xx = x + dx;
                    if (xx < 0) xx = 0;
                    if (xx >= largura) xx = largura - 1;

                    janela[count++] = dados[yy * largura + xx];
                }
            }

            // Ordenar e pegar mediana
            qsort(janela, count, sizeof(unsigned char), comparar);
            saida[idx_saida] = janela[count / 2];
        }
    }

    free(janela);
}



int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void filtroLaplaciano(unsigned char* dados, unsigned char* saida, int largura, int altura) {
    // int kernel[3][3] = {
    //     { 0,  1,  0},
    //     { 1, -4,  1},
    //     { 0,  1,  0}
    // };

    // int kernel[5][5] = {
    //   {0,  0, -1,  0,  0},
    //   {0, -1, -2, -1,  0},
    //   {-1, -2, 16, -2, -1},
    //   {0, -1, -2, -1,  0},
    //   {0,  0, -1,  0,  0}
    // };
    int kernel[7][7] = {
      {0,  0,  0, -1,  0,  0,  0},
      {0,  0, -1, -2, -1,  0,  0},
      {0, -1, -2, -3, -2, -1,  0},
      {1, -2, -3, 36, -3, -2, -1},
      {0, -1, -2, -3, -2, -1,  0},
      {0,  0, -1, -2, -1,  0,  0},
      {0,  0,  0, -1,  0,  0,  0}
    };

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            int soma = 0;
            for (int ky = -1; ky <= 1; ky++) {
                int yy = y + ky;
                if (yy < 0) yy = 0;
                if (yy >= altura) yy = altura -1;

                for (int kx = -1; kx <= 1; kx++) {
                    int xx = x + kx;
                    if (xx < 0) xx = 0;
                    if (xx >= largura) xx = largura -1;

                    int pixel = dados[yy * largura + xx];
                    soma += pixel * kernel[ky + 1][kx + 1];
                }
            }
            // O resultado pode ser negativo, faz abs para destacar bordas
            soma = abs(soma);

            saida[y * largura + x] = (unsigned char)clamp(soma, 0, 255);
        }
    }
}



int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Uso: %s <entrada.bmp> <saida.bmp> <tamanho_mascara>\n", argv[0]);
        return 1;
    }

    int largura, altura;
    BMPHeader header;
    BMPInfoHeader infoHeader;

    unsigned char* dados = lerBMP(argv[1], &largura, &altura, &header, &infoHeader);
    if (!dados) return 1;

    converterParaTonsDeCinza(dados, largura, altura);

    int tamanhoLinha = ((largura * 3 + 3) / 4) * 4;

    // Criar buffer de tons de cinza sem padding (1 byte por pixel)
    unsigned char* cinza = malloc(largura * altura);
    if (!cinza) {
        printf("Erro de memória\n");
        free(dados);
        return 1;
    }

    // Copiar do dados (3 canais iguais) para cinza (1 canal)
    for (int y = 0; y < altura; y++) {
        unsigned char* linha = dados + y * tamanhoLinha;
        for (int x = 0; x < largura; x++) {
            cinza[y * largura + x] = linha[x * 3]; // pegar canal azul (B)
        }
    }

    // Buffers temporários para os filtros
    unsigned char* tmpMediana = malloc(largura * altura);
    unsigned char* tmpLaplaciano = malloc(largura * altura);
    if (!tmpMediana || !tmpLaplaciano) {
        printf("Erro de memória\n");
        free(dados);
        free(cinza);
        free(tmpMediana);
        free(tmpLaplaciano);
        return 1;
    }

    int tamanhoMascara = atoi(argv[3]);
    if (tamanhoMascara % 2 == 0 || tamanhoMascara < 3) {
        printf("Tamanho da máscara deve ser ímpar e >= 3\n");
        free(dados);
        free(cinza);
        free(tmpMediana);
        free(tmpLaplaciano);
        return 1;
    }

    // Aplicar filtro mediana
    filtroMediana(cinza, tmpMediana, largura, altura, tamanhoMascara);

    // Aplicar filtro laplaciano no resultado da mediana
    filtroLaplaciano(tmpMediana, tmpLaplaciano, largura, altura);

    // Copiar resultado final para dados (3 canais iguais e com padding)
    for (int y = 0; y < altura; y++) {
        unsigned char* linha = dados + y * tamanhoLinha;
        for (int x = 0; x < largura; x++) {
            unsigned char val = tmpLaplaciano[y * largura + x];
            linha[x * 3 + 0] = val; // B
            linha[x * 3 + 1] = val; // G
            linha[x * 3 + 2] = val; // R
        }
    }

    salvarBMP(argv[2], &header, &infoHeader, dados);

    free(dados);
    free(cinza);
    free(tmpMediana);
    free(tmpLaplaciano);

    return 0;
}
