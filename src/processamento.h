#ifndef PROCESSAMENTO_H
#define PROCESSAMENTO_H

#include "bmp_io.h"

void aplicarFiltroLaplacianoProcessos(unsigned char* dataOriginal, BITMAPINFOHEADER infoHeader, int tamanhoMascara, int num_processos);

#endif
