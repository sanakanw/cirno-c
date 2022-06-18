.PHONY=default build debug run tests

default: build run

build:
	gcc src/*/*.c src/*.c -o 9c

debug:
	gcc -g src/*/*.c src/*.c -o 9c
	gdb 9c

run:
	./9c main.9c

tests: build
	./9c tests/bubble.9c
	./9c tests/prime.9c
	./9c tests/selection.9c
	./9c tests/dot.9c
