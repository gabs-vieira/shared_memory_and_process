#ifndef FILTRO_MEDIANA_H
#define FILTRO_MEDIANA_H

#include "../include/bmp_io.h"

void aplicarFiltroMedianaProcessos(unsigned char* dataOriginal, BITMAPINFOHEADER infoHeader, int tamanhoMascara, int num_processos);

#endif
