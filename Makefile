# Build File for the Task Manager

CC=gcc
CFLAGS=-Wall -Wextra -ggdb -std=c99 -pedantic
LIBS=-lsqlite3

.PHONY: all clean

all: build/taskman

build:
	mkdir -p build

build/taskman: src/tm.c src/taskman.c | build
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf build/ test.db
