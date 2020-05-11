CFLAGS = -mcx16 -O3 -std=c++17 -march=native -Wall
CFLAGS_DBG = -mcx16 -std=c++17 -march=native -Wall -g

OMPFLAGS = -DOPENMP -fopenmp
CILKFLAGS = -DCILK -fcilkplus
HGFLAGS = -DHOMEGROWN -pthread

RNGFLAGS = -lm

ifdef CLANG
CC = clang++
PFLAGS = $(CILKFLAGS)
else ifdef CILK
CC = g++
PFLAGS = $(CILKFLAGS)
else ifdef OPENMP
CC = g++
PFLAGS = $(OMPFLAGS)
else ifdef HOMEGROWN
CC = g++
PFLAGS = $(HGFLAGS)
else ifdef SERIAL
CC = g++
PFLAGS =
else # default is cilk
CC = g++
PFLAGS = $(CILKFLAGS)
endif

dfs: dfs_main.cpp rng/brg_sha1.c uts.c
	$(CC) $(CFLAGS) $(RNGFLAGS) -o $@ $+

par: parallel_main.cpp rng/brg_sha1.c uts.c
	$(CC) $(CFLAGS) $(PFLAGS) $(RNGFLAGS) -o $@ $+

par.dbg: parallel_main.cpp rng/brg_sha1.c uts.c
	$(CC) $(CFLAGS_DBG) $(PFLAGS) $(RNGFLAGS) -o $@ $+

clean: phony
	rm -f dfs

.PHONY: phony
phony:
