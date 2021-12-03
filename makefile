default: build run

build:
	gcc cc/*.c common/*.c vm/*.c 9c.c -o 9c

run:
	./9c test.9c
