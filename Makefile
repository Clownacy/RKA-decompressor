CC = gcc
CFLAGS = -O3 -s -std=c11 -fno-ident
NAME = RKA-decompressor.exe

all: src/main.c
	$(CC) $(CFLAGS) -o $(NAME) $^
