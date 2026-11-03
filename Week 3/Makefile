CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2
all: page_simulator
page_simulator: main.o page_replacement.o report.o
	$(CC) $(CFLAGS) -o $@ $^
main.o: main.c page_replacement.h
	$(CC) $(CFLAGS) -c main.c
page_replacement.o: page_replacement.c page_replacement.h
	$(CC) $(CFLAGS) -c page_replacement.c
report.o: report.c page_replacement.h
	$(CC) $(CFLAGS) -c report.c
clean:
	rm -f page_simulator *.o
.PHONY: all clean
