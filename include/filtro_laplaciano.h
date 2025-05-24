#ifndef FILTRO_LAPLACIANO_H
#define FILTRO_LAPLACIANO_H

#include "bmp_io.h"

void aplicarFiltroLaplacianoProcessos(unsigned char* dataOriginal, BITMAPINFOHEADER infoHeader, int tamanhoMascara, int num_processos);

#endif
