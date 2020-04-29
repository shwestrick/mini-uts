CC=g++

main: main.c rng/brg_sha1.c uts.c
	$(CC) -O3 -Wall -DBRG_C99_TYPES -std=gnu++11 -lm -DBRG_RNG -o $@ $+
