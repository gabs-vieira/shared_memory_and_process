#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#include "bmp_io.h"
#include "filtro_laplaciano.h"

void aplicarFiltroLaplacianoProcessos(unsigned char* dataOriginal, BITMAPINFOHEADER infoHeader, int tamanhoMascara, int num_processos) {
    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    int row_padded = (width * 3 + 3) & (~3);
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

    // Copiar dados originais para a memória compartilhada
    for (int i = 0; i < data_size; i++) {
        buffer[i] = dataOriginal[i];
    }

    int id_seq = 0;
    for (int i = 1; i < num_processos; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            id_seq = i;
            break;
        }
    }

    // Processo filho ou pai faz sua parte
    unsigned char* copia = (unsigned char*)malloc(data_size);
    for (int i = 0; i < data_size; i++) {
        copia[i] = buffer[i];
    }

    int offset = tamanhoMascara / 2;
    int mascara[9] = {0, -1, 0, -1, 4, -1, 0, -1, 0};

    for (int i = offset + id_seq; i < height - offset; i += num_processos) {
        for (int j = offset; j < width - offset; j++) {
            int soma = 0;
            int count = 0;

            for (int mi = -offset; mi <= offset; mi++) {
                for (int mj = -offset; mj <= offset; mj++) {
                    int y = i + mi;
                    int x = j + mj;

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

    if (id_seq != 0) {
        // Processo filho
        shmdt(buffer);
        exit(0);
    } else {
        // Processo pai
        for (int i = 1; i < num_processos; i++) {
            wait(NULL);
        }

        // Copiar dados da memória compartilhada de volta para a imagem original
        for (int i = 0; i < data_size; i++) {
            dataOriginal[i] = buffer[i];
        }

        // Libera memória compartilhada
        shmdt(buffer);
        shmctl(shmid, IPC_RMID, 0);
    }
}
