# Folders
HEADERS := headers
SOURCES := sources
EXECUTABLES := executables
DATA := data
CORRECT_DATA := $(DATA)/correct
PROD_DATA := $(DATA)/prod

# CCookbook
CCOOKBOOK_FOLDER := ../../helpers/CCookbook/
CCOOKBOOK_SOURCES := $(shell find ../../helpers/CCookbook/ -maxdepth 2 -name "*.c" -type f)

# Compiler
CC := gcc
FLAGS := -O3 -Wall -g -std=gnu99
LD_FLAGS := -lm -lpthread -lcrypto

all: clean build

build: build_search build_freqs build_md5

test: test_search test_freqs test_md5

build_search: $(SOURCES)/search.c
	$(CC) $(FLAGS) -I $(CCOOKBOOK_FOLDER) $(CCOOKBOOK_SOURCES) $(SOURCES)/search.c -o $(EXECUTABLES)/search $(LD_FLAGS)

test_search:
	$(EXECUTABLES)/search 2 $(CORRECT_DATA)/vals.txt $(CORRECT_DATA)/input.txt $(PROD_DATA)/indexes.txt
	diff $(PROD_DATA)/indexes.txt $(CORRECT_DATA)/indexes.txt

build_freqs: $(SOURCES)/freqs.c
	$(CC) $(FLAGS) -I $(CCOOKBOOK_FOLDER) $(CCOOKBOOK_SOURCES) $(SOURCES)/freqs.c -o $(EXECUTABLES)/freqs $(LD_FLAGS)

test_freqs:
	$(EXECUTABLES)/freqs 2 $(PROD_DATA)/indexes.txt $(CORRECT_DATA)/input.txt $(CORRECT_DATA)/real_order.txt $(PROD_DATA)/dictionary.txt
	diff $(PROD_DATA)/dictionary.txt $(CORRECT_DATA)/dictionary.txt

build_md5: $(SOURCES)/md5.c
	$(CC) $(FLAGS) -I $(CCOOKBOOK_FOLDER) $(CCOOKBOOK_SOURCES) $(SOURCES)/md5.c -o $(EXECUTABLES)/md5 $(LD_FLAGS)

test_md5:
	$(EXECUTABLES)/md5 2 $(PROD_DATA)/dictionary.txt $(CORRECT_DATA)/md5_hash.txt $(PROD_DATA)/plaintext.txt
	diff $(PROD_DATA)/plaintext.txt $(CORRECT_DATA)/plaintext.txt

clean:
	rm -f $(EXECUTABLES)/*
	rm -f $(PROD_DATA)/*
	rm -f .gdb*
	rm -f peda*