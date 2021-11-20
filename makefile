default: build run

build:
	gcc cc/*.c common/*.c -o 9cc 
	gcc vm/*.c common/*.c -o 9vm

run:
	./9cc
	./9vm
