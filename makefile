default: build run

build:
	gcc src/*.c vm/*.c -o 9c 

run:
	./9c
