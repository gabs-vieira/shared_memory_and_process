CC = gcc
CFLAGS = -Wall -O2
SRC = src
BIN = bin

OBJS = $(SRC)/main.c $(SRC)/bmp_io.c $(SRC)/conversao.c $(SRC)/filtro_mediana.c $(SRC)/filtro_laplaciano.c $(SRC)/processamento.c



all: $(BIN)/main

$(BIN)/main: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(BIN)/main
