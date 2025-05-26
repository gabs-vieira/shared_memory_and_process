#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

#include "../include/filtro_mediana.h"
#include "../include/bmp_io.h"


/**
 * @brief Calcula a mediana dos valores em uma janela.
 *
 * A função realiza uma ordenação simples dos valores e retorna o valor central.
 *
 * @param janela Vetor de valores da vizinhança (tons de cinza).
 * @param tamanho Número de elementos na janela (deve ser ímpar).
 * @return unsigned char Valor mediano.
 */
unsigned char calcularMediana(unsigned char* janela, int tamanho) {
    // Ordenação bubble sort simples (poderia ser otimizado com counting sort) --> se der tempo a gnt implementa
    for (int i = 0; i < tamanho - 1; i++) {
        for (int j = i + 1; j < tamanho; j++) {
            if (janela[i] > janela[j]) {
                unsigned char tmp = janela[i];
                janela[i] = janela[j];
                janela[j] = tmp;
            }
        }
    }
    return janela[tamanho / 2];
}


unsigned char acessarPixel(unsigned char* data, int x, int y, int width, int height) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    }
    int pos = (y * width + x) * 3;
    return data[pos]; // Como está em tons de cinza, B=G=R
}




/**
 * @brief Aplica o filtro da mediana de forma paralela usando múltiplos processos e memória compartilhada.
 *
 * A função divide a imagem entre vários processos, cada um processando linhas específicas.
 * A imagem é em tons de cinza, então apenas um canal é considerado (R = G = B).
 *
 * @param dataOriginal Ponteiro para os dados da imagem BMP em tons de cinza.
 * @param infoHeader Informações da imagem (largura, altura, etc).
 * @param tamanhoMascara Tamanho da máscara (3, 5 ou 7).
 * @param num_processos Número de processos a serem usados.
 */

void aplicarFiltroMedianaProcessos(unsigned char* dataOriginal, BITMAPINFOHEADER infoHeader, int tamanhoMascara, int num_processos) {
    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);

    // int row_padded = (width * 3 + 3) & (~3); // largura da linha com padding de 4 bytes
    int row_padded = width * 3;
    int data_size = row_padded * height;


    // Criação da memória compartilhada para a imagem
    key_t chave = 5678; // chave arbitrária
    int shmid = shmget(chave, data_size, IPC_CREAT | 0600);
    if (shmid < 0) {
        perror("Erro ao criar memória compartilhada");
        exit(1);
    }

    // Mapeia memória compartilhada para o espaço de endereço do processo
    unsigned char* buffer = (unsigned char*)shmat(shmid, NULL, 0);
    if (buffer == (void*)-1) {
        perror("Erro ao mapear memória compartilhada");
        exit(1);
    }

    // Copia a imagem original para a memória compartilhada
    memcpy(buffer, dataOriginal, data_size);


    int offset = tamanhoMascara / 2;
    int janela_tam = tamanhoMascara * tamanhoMascara;

    // Criação dos processos filhos
    int id_seq = 0;
    for (int i = 1; i < num_processos; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            id_seq = i;  // Cada processo filho identifica seu número
            break;
        }
    }


    // Cópia local da imagem (para leitura segura)
    unsigned char* copia = (unsigned char*)malloc(data_size);
    if (!copia) {
        perror("Erro de alocação");
        exit(1);
    }
    memcpy(copia, buffer, data_size);


    // Vetor para armazenar a vizinhança de cada pixel
    unsigned char* janela = (unsigned char*)malloc(janela_tam);
    if (!janela) {
        perror("Erro de alocação da janela");
        exit(1);
    }


    /**
     * Cada processo irá processar linhas alternadas da imagem:
     * processo 0: linha 1, 1+num_processos, ...
     * processo 1: linha 2, 2+num_processos, ...
     */
    // for (int i = offset + id_seq; i < height - offset; i += num_processos) {
    //     for (int j = offset; j < width - offset; j++) {
    //         int k = 0;

    //         // Monta a janela de vizinhança
    //         for (int mi = -offset; mi <= offset; mi++) {
    //             for (int mj = -offset; mj <= offset; mj++) {
    //                 int y = i + mi;
    //                 int x = j + mj;
    //                 //int pos = y * row_padded + x * 3;
    //                 //janela[k++] = copia[pos];  // R, G e B são iguais em tons de cinza
    //                 janela[k++] = acessarPixel(copia, x, y, width, height);

    //             }
    //         }

    //         // Calcula a mediana da janela
    //         unsigned char mediana = calcularMediana(janela, janela_tam);

    //         // Escreve o valor nos três canais da imagem (tons de cinza)
    //         int pos = i * row_padded + j * 3;
    //         buffer[pos]     = mediana;
    //         buffer[pos + 1] = mediana;
    //         buffer[pos + 2] = mediana;
    //     }
    // }

    for (int i = id_seq; i < height; i += num_processos) {
        for (int j = 0; j < width; j++) {
            int k = 0;

            // Monta a janela
            for (int mi = -offset; mi <= offset; mi++) {
                for (int mj = -offset; mj <= offset; mj++) {
                    int y = i + mi;
                    int x = j + mj;
                    janela[k++] = acessarPixel(copia, x, y, width, height);
                }
            }

            // Calcula a mediana
            unsigned char mediana = calcularMediana(janela, janela_tam);

            // Grava o resultado nos 3 canais
            int pos = (i * width + j) * 3;
            buffer[pos]     = mediana;
            buffer[pos + 1] = mediana;
            buffer[pos + 2] = mediana;
        }
    }



    free(copia);
    free(janela);

    if (id_seq != 0) {
        // Processo filho encerra e libera memória
        shmdt(buffer);
        exit(0);
    } else {
        // Processo pai aguarda todos os filhos
        for (int i = 1; i < num_processos; i++) wait(NULL);

        // Copia os dados processados de volta para a imagem original
        memcpy(dataOriginal, buffer, data_size);

        // Libera a memória compartilhada
        shmdt(buffer);
        shmctl(shmid, IPC_RMID, NULL);
    }

}
