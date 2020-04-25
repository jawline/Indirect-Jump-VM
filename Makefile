all: build test

CFLAGS= -O1
 
build:
	gcc -Iblakec/src/ ${CFLAGS} ./src/blas.c -o blas
	gcc -Iblakec/src/ -D FAST_MODE ${CFLAGS} ./src/main.c -o vm_fast
	gcc -Iblakec/src/ ${CFLAGS} ./src/main.c -o vm_slow
test:
	@echo "Generating a random program"
	python scripts/randmath.py > example.blas
	@echo "Generating ByteCode"
	./blas example.blas example.bc
	@echo "Running vms"
	time ./vm_fast example.bc
	time ./vm_slow example.bc
