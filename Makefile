CC=gcc

CFLAGS=-Wall -Werror -Iinclude -g3

all: pc unit_tests

unit_tests: test_entry.o \
	utils.o test_utils.o \
	string_view.o test_string_view.o \
	url.o test_url.o \
	http.o test_http.o
	$(CC) -o $@ $^ $(CFLAGS) -lcunit

pc: proxy.o connection.o utils.o string_view.o url.o http.o
	$(CC) -o $@ $^ $(CFLAGS)


proxy.o: src/proxy.c
	$(CC) $(CFLAGS) -c $< -o $@

utils.o: src/utils.c include/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

string_view.o: src/string_view.c include/string_view.h
	$(CC) $(CFLAGS) -c $< -o $@

url.o: src/url.c include/url.h
	$(CC) $(CFLAGS) -c $< -o $@

http.o: src/http.c include/http.h
	$(CC) $(CFLAGS) -c $< -o $@

connection.o: src/connection.c include/connection.h include/address.h
	$(CC) $(CFLAGS) -c $< -o $@


test_utils.o: test/utils.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_string_view.o: test/string_view.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_url.o: test/url.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_http.o: test/http.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@


test_entry.o: test/entry.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf *.o
	rm -f unit_tests

.PHONY: all clean

