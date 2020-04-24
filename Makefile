all: build test
 
build:
	gcc -Iblakec/src/ -D FAST_MODE -Ofast ./src/blas.c -o blas
	gcc -Iblakec/src/ -D FAST_MODE -Ofast ./src/main.c -o vm_fast
	gcc -Iblakec/src/ -Ofast ./src/main.c -o vm_slow
test:
	@echo "Generating ByteCode"
	./blas example.blas example.bc
	@echo "Running vms"
	time ./vm_fast example.bc
	time ./vm_slow example.bc
