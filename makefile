default: build run

build:
	gcc cc/*.c common/*.c vm/*.c 9c.c -o 9c

debug:
	gcc -g cc/*.c common/*.c vm/*.c 9c.c -o 9c
	gdb 9c

tests: build
	./9c tests/bubble.9c
	./9c tests/prime.9c
	./9c tests/selection.9c
	./9c tests/dot.9c

run:
	./9c test.9c
