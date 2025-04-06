CC=gcc

CFLAGS=-Wall -Werror -Iinclude -g3

all: pc unit_tests

unit_tests: test_entry.o \
	tcp.o test_tcp.o \
	string_view.o test_string_view.o \
	url.o test_url.o \
	request.o test_request.o \
	http_client.o test_http_client.o
	$(CC) -o $@ $^ $(CFLAGS) -lcunit

pc: proxy.o connection.o tcp.o string_view.o url.o request.o md5.o http_client.o filename.o
	$(CC) -o $@ $^ $(CFLAGS)


proxy.o: src/proxy.c
	$(CC) $(CFLAGS) -c $< -o $@

tcp.o: src/tcp.c include/tcp.h
	$(CC) $(CFLAGS) -c $< -o $@

md5.o: src/md5.c include/md5.h
	$(CC) $(CFLAGS) -c $< -o $@

string_view.o: src/string_view.c include/string_view.h
	$(CC) $(CFLAGS) -c $< -o $@

url.o: src/url.c include/url.h
	$(CC) $(CFLAGS) -c $< -o $@

http_client.o: src/http_client.c include/http_client.h
	$(CC) $(CFLAGS) -c $< -o $@

request.o: src/request.c include/request.h
	$(CC) $(CFLAGS) -c $< -o $@

filename.o: src/filename.c include/filename.h
	$(CC) $(CFLAGS) -c $< -o $@

connection.o: src/connection.c include/connection.h include/address.h
	$(CC) $(CFLAGS) -c $< -o $@


test_tcp.o: test/tcp.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_http_client.o: test/http_client.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_string_view.o: test/string_view.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_url.o: test/url.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@

test_request.o: test/request.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@


test_entry.o: test/entry.c test/entry.h
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf *.o
	rm -f unit_tests pc

.PHONY: all clean

