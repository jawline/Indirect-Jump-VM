all: build test
 
build:
	gcc -D FAST_MODE -Ofast ./src/main.c -o vm_fast
	gcc -Ofast ./src/main.c -o vm_slow
test:
	time ./vm_fast
	time ./vm_slow
