# Folders
HEADERS := headers
SOURCES := sources
EXECUTABLES := executables

# CCookbook, without replicated workers
CCOOKBOOK_FOLDER := ../../helpers/CCookbook/
CCOOKBOOK_SOURCES := $(shell find ../../helpers/CCookbook/ -maxdepth 2 -name "*.c" -type f -not -name "replicated_workers.c")

# Compiler
CC := gcc
FLAGS := -O3 -Wall -g
LD_FLAGS := -lm -lpthread -lcrypto

all: clean build

build: homework generator

homework: $(SOURCES)/main.c $(SOURCES)/worker_thread.c $(HEADERS)/worker_thread.h
	$(CC) $(FLAGS) -I $(HEADERS) -I $(CCOOKBOOK_FOLDER) $(CCOOKBOOK_SOURCES) $(SOURCES)/worker_thread.c $(SOURCES)/replicated_workers.c $(SOURCES)/main.c $(LD_FLAGS) -o $(EXECUTABLES)/homework

generator: $(SOURCES)/generator.c
	$(CC) $(FLAGS) $(SOURCES)/generator.c -o $(EXECUTABLES)/generator

clean:
	rm -f $(EXECUTABLES)/*
	rm -f .gdb*
	rm -f peda*