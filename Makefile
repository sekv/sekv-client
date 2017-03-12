all: *.c *.h
	gcc -O3 *.c -Wall -levent -std=c99 -lcrypto -pthread -lm -D_GNU_SOURCE -o loader

clean:
	rm loader
