CC=gcc
CFLAGS=-Wall -O2
SRC=src
BIN=bin

all: main

main: $(SRC)/main.c $(SRC)/bmp_io.c
	$(CC) $(CFLAGS) -o $(BIN)/main $(SRC)/main.c $(SRC)/bmp_io.c

clean:
	rm -f $(BIN)/main