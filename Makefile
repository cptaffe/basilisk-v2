CFLAGS=-O2 -Wall
PREFIX=/usr/local

basilisk:
	$(CC) main.c -o basilisk

install: basilisk
	install basilisk $(PREFIX)/bin

test: basilisk
	@./test.sh

redo: clean basilisk test

clean:
	rm -f basilisk