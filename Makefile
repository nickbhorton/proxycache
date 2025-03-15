CC=gcc

CFLAGS=-Wall -Werror -Iinclude

all: unit_tests

unit_tests: test_entry.o utils.o test_utils.o
	$(CC) -o $@ $^ $(CFLAGS) -lcunit


utils.o: src/utils.c include/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

test_utils.o: test/utils.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_entry.o: test/entry.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf *.o
	rm -f unit_tests

.PHONY: all clean

