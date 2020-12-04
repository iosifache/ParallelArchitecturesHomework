# Folders
HEADERS := headers
SOURCES := sources
EXECUTABLES := executables

# Compiler
CC := gcc
FLAGS := -O3 -Wall -g
LD_FLAGS := -lm -lpthread -lcrypto

all: clean build

build: clean homework generator

homework: $(SOURCES)/main.c $(SOURCES)/workerThread.c $(HEADERS)/workerThread.h
	gcc $(FLAGS) -I $(HEADERS) $(SOURCES)/workerThread.c $(SOURCES)/main.c $(SOURCES)/replicated_workers.c $(LD_FLAGS) -o $(EXECUTABLES)/homework

generator: $(SOURCES)/generator.c
	gcc $(FLAGS) $(SOURCES)/generator.c -o $(EXECUTABLES)/generator

clean:
	rm -f $(EXECUTABLES)/*
	rm -f .gdb*
	rm -f peda*