HEADERS := headers
HELPER_HEADERS := $(HEADERS)/helpers
SOURCES := sources
HELPER_SOURCES := $(SOURCES)/helpers/*
EXECUTABLES := executables
DATA := data
CORRECT_DATA := $(DATA)/correct
PROD_DATA := $(DATA)/prod

all: clean build

build: build_search build_freqs build_md5

test: test_search test_freqs test_md5

build_search: $(SOURCES)/search.c
	gcc -O3 -Wall -I $(HELPER_HEADERS) $(SOURCES)/search.c $(HELPER_SOURCES) -o $(EXECUTABLES)/search -g -lm -lpthread -lcrypto -std=gnu99

test_search:
	$(EXECUTABLES)/search 2 $(CORRECT_DATA)/vals.txt $(CORRECT_DATA)/input.txt $(PROD_DATA)/indexes.txt
	diff $(PROD_DATA)/indexes.txt $(CORRECT_DATA)/indexes.txt

build_freqs: $(SOURCES)/freqs.c
	gcc -O3 -Wall -I $(HELPER_HEADERS) $(SOURCES)/freqs.c $(HELPER_SOURCES) -o $(EXECUTABLES)/freqs -g -lm -lpthread -lcrypto -std=gnu99

test_freqs:
	$(EXECUTABLES)/freqs 2 $(PROD_DATA)/indexes.txt $(CORRECT_DATA)/input.txt $(CORRECT_DATA)/real_order.txt $(PROD_DATA)/dictionary.txt
	diff $(PROD_DATA)/dictionary.txt $(CORRECT_DATA)/dictionary.txt

build_md5: $(SOURCES)/md5.c
	gcc -O3 -Wall -I $(HELPER_HEADERS) $(SOURCES)/md5.c $(HELPER_SOURCES) -o $(EXECUTABLES)/md5 -g -lm -lpthread -lcrypto -std=gnu99

test_md5:
	$(EXECUTABLES)/md5 2 $(PROD_DATA)/dictionary.txt $(CORRECT_DATA)/md5_hash.txt $(PROD_DATA)/plaintext.txt
	diff $(PROD_DATA)/plaintext.txt $(CORRECT_DATA)/plaintext.txt

clean:
	rm -f $(EXECUTABLES)/*
	rm -f $(PROD_DATA)/*
	rm -f .gdb*
	rm -f peda*