HEADERS := headers
HELPER_HEADERS := $(HEADERS)/helpers
SOURCES := sources
HELPER_SOURCES := $(SOURCES)/helpers/*
EXECUTABLES := executables
OUTPUT := output

all: clean build

build: build_search build_freqs build_md5

test: test_search test_freqs test_md5

build_search: $(SOURCES)/search.c
	gcc -O3 -Wall -I $(HELPER_HEADERS) $(SOURCES)/search.c $(HELPER_SOURCES) -o $(EXECUTABLES)/search -g -lm -lpthread -lcrypto -std=gnu99
	cp $(EXECUTABLES)/search search

test_search:
	$(EXECUTABLES)/search 2 data/vals.txt data/input.txt output/indexes.txt
	diff output/indexes.txt data/indexes.txt

build_freqs: $(SOURCES)/freqs.c
	gcc -O3 -Wall -I $(HELPER_HEADERS) $(SOURCES)/freqs.c $(HELPER_SOURCES) -o $(EXECUTABLES)/freqs -g -lm -lpthread -lcrypto -std=gnu99
	cp $(EXECUTABLES)/freqs freqs

test_freqs:
	$(EXECUTABLES)/freqs 2 output/indexes.txt data/input.txt data/real_order.txt output/dictionary.txt
	diff output/dictionary.txt data/dictionary.txt

build_md5: $(SOURCES)/md5.c
	gcc -O3 -Wall -I $(HELPER_HEADERS) $(SOURCES)/md5.c $(HELPER_SOURCES) -o $(EXECUTABLES)/md5 -g -lm -lpthread -lcrypto -std=gnu99
	cp $(EXECUTABLES)/md5 md5

test_md5:
	$(EXECUTABLES)/md5 2 output/dictionary.txt data/md5_hash.txt output/plaintext.txt
	diff output/plaintext.txt data/plaintext.txt

clean:
	rm -f $(EXECUTABLES)/*
	rm -f $(OUTPUT)/*
	rm -f freqs
	rm -f search
	rm -f md5
	rm -f .gdb*
	rm -f peda*