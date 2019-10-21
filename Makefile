CC = gcc
CFLAGS = -O3 -s -std=c99 -Wall -Wextra -pedantic

RKA-decompressor: src/main.c
	$(CC) $(CFLAGS) -o $@ $^
