# Folders
HEADERS := headers
SOURCES := sources
EXECUTABLES := executables
DATA := data

# CCookbook, without cryptography
CCOOKBOOK_FOLDER := ../../helpers/CCookbook/
CCOOKBOOK_SOURCES := $(shell find ../../helpers/CCookbook/ -maxdepth 2 -name "*.c" -type f -not -name "hash.c")

# Compiler
CC := gcc
FLAGS := -std=c99 -O3 -Wall -g
LD_FLAGS := -lm

all: clean build

build: homework generator compare

homework: $(SOURCES)/homework.c
	mpicc $(FLAGS) -I $(HEADERS) -I $(CCOOKBOOK_FOLDER) $(CCOOKBOOK_SOURCES) $(SOURCES)/homework.c $(LD_FLAGS) -o $(EXECUTABLES)/homework

test: clean_output compare homework
	./run_tests.sh

generator: $(SOURCES)/generator.c
	gcc $(FLAGS) $(SOURCES)/generator.c -o $(EXECUTABLES)/generatorInput

compare: $(SOURCES)/compare.c
	gcc $(FLAGS) $(SOURCES)/compare.c -o $(EXECUTABLES)/compare

clean_output:
	rm -f $(DATA)/prod/1/*
	rm -f $(DATA)/prod/2/*
	rm -f $(DATA)/prod/4/*

clean: clean_output
	rm -f $(EXECUTABLES)/*
	rm -f .gdb*
	rm -f peda*