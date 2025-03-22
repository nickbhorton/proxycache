CC=gcc

CFLAGS=-Wall -Werror -Iinclude -g3

all: unit_tests

unit_tests: test_entry.o \
	utils.o test_utils.o \
	string_view.o test_string_view.o \
	url.o test_url.o
	$(CC) -o $@ $^ $(CFLAGS) -lcunit


utils.o: src/utils.c include/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

string_view.o: src/string_view.c include/string_view.h
	$(CC) $(CFLAGS) -c $< -o $@

url.o: src/url.c include/url.h
	$(CC) $(CFLAGS) -c $< -o $@


test_utils.o: test/utils.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_string_view.o: test/string_view.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_url.o: test/url.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@


test_entry.o: test/entry.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf *.o
	rm -f unit_tests

.PHONY: all clean

