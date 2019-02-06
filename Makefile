CC=gcc
DEBUG=-g3 -pedantic -Wall -Wextra -Werror
OPT=-pedantic -O3 -Wall -Werror -Wextra

ifneq (,$(filter $(MAKECMDGOALS),debug valgrind))
CFLAGS=$(DEBUG)
else
CFLAGS=$(OPT)
endif

.PHONY: clean

all: clean web_server

debug: all
	
valgrind: clean web_server
	valgrind --leak-check=full --log-file="valgrind.out" --show-reachable=yes -v ./web_server
	
web_server: web_server.o util.o
	$(CC) $(CFLAGS) -o $@ $^
	
web_server.o: web_server.c util.h
	$(CC) $(CFLAGS) -c -o $@ $<

util.o: util.c util.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	rm -rf *.o *.exe