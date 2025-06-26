build: clean make_build
	./make

make_build: make.c
	gcc -o make make.c

clean: 
	rm -rf build
