#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


#include "bmp_io.h"
#include "filtro_laplaciano.h"

// Função para gerar a máscara Laplaciana dependendo do tamanho
void gerarMascaraLaplaciana(int tamanho, int* mascara) {
    if (tamanho == 3) {
        int temp[9] = {
            0, -1, 0,
            -1, 4, -1,
            0, -1, 0
        };
        memcpy(mascara, temp, sizeof(temp));
    } else if (tamanho == 5) {
        int temp[25] = {
             0,  0, -1,  0,  0,
             0, -1, -2, -1,  0,
            -1, -2, 16, -2, -1,
             0, -1, -2, -1,  0,
             0,  0, -1,  0,  0
        };
        memcpy(mascara, temp, sizeof(temp));
    } else if (tamanho == 7) {
        int temp[49] = {
             0,  0,  0, -1,  0,  0,  0,
             0,  0, -1, -2, -1,  0,  0,
             0, -1, -2, -3, -2, -1,  0,
            -1, -2, -3, 36, -3, -2, -1,
             0, -1, -2, -3, -2, -1,  0,
             0,  0, -1, -2, -1,  0,  0,
             0,  0,  0, -1,  0,  0,  0
        };
       memcpy(mascara, temp, sizeof(temp));
    } else {
        printf("Tamanho de máscara inválido no Laplaciano (use 3, 5 ou 7).\n");
        exit(1);
    }
}

// Função que aplica o filtro Laplaciano com processos e memória compartilhada
void aplicarFiltroLaplacianoProcessos(unsigned char* dataOriginal, BITMAPINFOHEADER infoHeader, int tamanhoMascara, int num_processos) {
    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    // int row_padded = (width * 3 + 3) & (~3);
    int row_padded = width * 3;
    int data_size = row_padded * height;

    key_t chave = 1234;
    int shmid = shmget(chave, data_size, 0600 | IPC_CREAT);
    if (shmid < 0) {
        perror("Erro ao criar memória compartilhada");
        exit(1);
    }

    unsigned char* buffer = (unsigned char*)shmat(shmid, NULL, 0);
    if (buffer == (void*)-1) {
        perror("Erro ao mapear memória compartilhada");
        exit(1);
    }

    // Copia os dados originais para a memória compartilhada
    for (int i = 0; i < data_size; i++) {
        buffer[i] = dataOriginal[i];
    }

    // Gera a máscara de acordo com o tamanho
    int num_elementos = tamanhoMascara * tamanhoMascara;
    int* mascara = (int*)malloc(num_elementos * sizeof(int));
    if (mascara == NULL) {
        perror("Erro de alocação da máscara");
        exit(1);
    }
    gerarMascaraLaplaciana(tamanhoMascara, mascara);

    int offset = tamanhoMascara / 2;

    int id_seq = 0;
    for (int i = 1; i < num_processos; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            id_seq = i;
            break;
        }
    }

    unsigned char* copia = (unsigned char*)malloc(data_size);
    if (!copia) {
        perror("Erro de alocação na cópia da imagem");
        exit(1);
    }
    for (int i = 0; i < data_size; i++) {
        copia[i] = buffer[i];
    }

    for (int i = offset + id_seq; i < height - offset; i += num_processos) {
        for (int j = offset; j < width - offset; j++) {
            int soma = 0;
            int count = 0;

            for (int mi = -offset; mi <= offset; mi++) {
                for (int mj = -offset; mj <= offset; mj++) {
                    int y = i + mi;
                    int x = j + mj;

                    // int pos = y * row_padded + x * 3;
                    int pos = y * row_padded + x * 3;
                    unsigned char valor = copia[pos];

                    soma += valor * mascara[count];
                    count++;
                }
            }

            if (soma < 0) soma = 0;
            if (soma > 255) soma = 255;

            int pos = i * row_padded + j * 3;
            buffer[pos]     = (unsigned char)soma;
            buffer[pos + 1] = (unsigned char)soma;
            buffer[pos + 2] = (unsigned char)soma;
        }
    }

    free(copia);
    free(mascara);

    if (id_seq != 0) {
        // Processo filho
        shmdt(buffer);
        exit(0);
    } else {
        // Processo pai
        for (int i = 1; i < num_processos; i++) {
            wait(NULL);
        }

        // Copia o resultado da memória compartilhada de volta
        for (int i = 0; i < data_size; i++) {
            dataOriginal[i] = buffer[i];
        }

        shmdt(buffer);
        shmctl(shmid, IPC_RMID, 0);
    }
}
