all: build test

CFLAGS=-O1
 
build:
	gcc -Iblakec/src/ ${CFLAGS} ./src/blas.c -o blas
	gcc -Iblakec/src/ -D FAST_MODE ${CFLAGS} ./src/main.c -o vm_fast
	gcc -Iblakec/src/ ${CFLAGS} ./src/main.c -o vm_slow
test:
	./blas example.blas ./example.bc
	./vm_fast example.bc
	./vm_slow example.bc
