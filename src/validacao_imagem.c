#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <opencv2/opencv.hpp>

#define N_EXECUCOES 5

using namespace cv;

void converterParaTonsDeCinza(Mat& imagem) {
    cvtColor(imagem, imagem, COLOR_BGR2GRAY);
}

void aplicarFiltroMediana(Mat& imagem, int tamanho_mascara) {
    medianBlur(imagem, imagem, tamanho_mascara);
}

void aplicarFiltroLaplaciano(Mat& imagem) {
    Mat laplaciano;
    Laplacian(imagem, laplaciano, CV_16S, 3);
    convertScaleAbs(laplaciano, imagem);
}

double medir_tempo_execucao(void (*filtro)(Mat&, int), Mat imagem, int arg) {
    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    filtro(imagem, arg);

    clock_gettime(CLOCK_MONOTONIC, &fim);
    return (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
}

double medir_tempo_execucao_laplaciano(void (*filtro)(Mat&), Mat imagem) {
    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    filtro(imagem);

    clock_gettime(CLOCK_MONOTONIC, &fim);
    return (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Uso: %s <entrada.bmp> <saida.bmp> <tamanho_mascara>\n", argv[0]);
        return 1;
    }

    const char* arquivo_entrada = argv[1];
    const char* arquivo_saida = argv[2];
    int tamanho_mascara = atoi(argv[3]);

    if (tamanho_mascara != 3 && tamanho_mascara != 5 && tamanho_mascara != 7) {
        printf("Tamanho de máscara inválido. Use 3, 5 ou 7.\n");
        return 1;
    }

    Mat imagem = imread(arquivo_entrada);
    if (imagem.empty()) {
        printf("Erro ao carregar imagem.\n");
        return 1;
    }

    printf("Imagem carregada: %dx%d\n", imagem.cols, imagem.rows);
    converterParaTonsDeCinza(imagem);
    printf("Conversão para tons de cinza concluída.\n");

    double tempo_total_mediana = 0;
    double tempo_total_laplaciano = 0;

    for (int i = 0; i < N_EXECUCOES; i++) {
        printf("Execução %d/%d:\n", i + 1, N_EXECUCOES);

        Mat copia1 = imagem.clone();
        double t1 = medir_tempo_execucao(aplicarFiltroMediana, copia1, tamanho_mascara);
        printf("  Tempo mediana: %.6f s\n", t1);
        tempo_total_mediana += t1;

        Mat copia2 = imagem.clone();
        double t2 = medir_tempo_execucao_laplaciano(aplicarFiltroLaplaciano, copia2);
        printf("  Tempo laplaciano: %.6f s\n", t2);
        tempo_total_laplaciano += t2;
    }

    double media_mediana = tempo_total_mediana / N_EXECUCOES;
    double media_laplaciano = tempo_total_laplaciano / N_EXECUCOES;

    printf("\nTempo médio filtro mediana: %.6f segundos\n", media_mediana);
    printf("Tempo médio filtro Laplaciano: %.6f segundos\n", media_laplaciano);

    // Aplica os filtros para salvar imagem final
    aplicarFiltroMediana(imagem, tamanho_mascara);
    aplicarFiltroLaplaciano(imagem);

    imwrite(arquivo_saida, imagem);
    printf("Imagem salva em: %s\n", arquivo_saida);

    // Log em arquivo
    FILE* log = fopen("tempo_execucao.log", "a");
    if (log) {
        fprintf(log, "Arquivo: %s | Mascara: %d | Média Mediana: %.6f s | Média Laplaciano: %.6f s | Tempo Total: %.6f s\n",
                arquivo_entrada, tamanho_mascara, media_mediana, media_laplaciano, (media_mediana + media_laplaciano));
        fclose(log);
    } else {
        perror("Erro ao escrever log");
    }

    return 0;
}
