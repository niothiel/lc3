all: build run

.PHONY: build
build:
	gcc -fsanitize=address -g -Werror -Wall -Wextra -pedantic -std=c99 ./src/*.c -o lc3


.PHONY: run
run:
	./lc3 run prog/counter.s
