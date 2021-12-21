CFLAGS=-O3 -mtune=native -I/opt/local/include
LDFLAGS=-L/opt/local/lib

DBCFLAGS=-mtune=native -I/opt/local/include
DBGFLAGS=-g3 -fsanitize=bounds
SRCS=main.c
all: kkutu
	
kkutu: ${SRCS}
	${CC} -o kkutu ${SRCS} ${CFLAGS} ${LDFLAGS}
	
kkutu_debug: ${SRCS}
	${CC} -o kkutu_debug ${SRCS} ${DBCFLAGS} ${LDFLAGS} ${DBGFLAGS}
	
run: kkutu
	./kkutu
	
debug: kkutu_debug
	./kkutu_debug

clean:
	rm -rf ./kkutu ./kkutu_debug
