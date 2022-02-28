DEP=

CFLAGS=-O3 -arch arm64 -mtune=native `pkg-config --cflags ${DEP}` -pthread -std=c11
LDFLAGS=`pkg-config --libs ${DEP}`
all: finder

finder: main.c
	${CC} main.c -o finder ${CFLAGS} ${LDFLAGS} ${DEBUGFLAGS}

clean:
	rm -rf trie
