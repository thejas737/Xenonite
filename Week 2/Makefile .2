CC = gcc
CFLAGS = -D_POSIX_C_SOURCE=200809L -std=c11 -Wall -Wextra -Wpedantic -O2
LDLIBS = -lrt

.PHONY: all clean run
all: ui core logger standalone

ui: ui.c common.h
	$(CC) $(CFLAGS) -o ui ui.c $(LDLIBS)

core: core.c common.h
	$(CC) $(CFLAGS) -o core core.c $(LDLIBS)

logger: logger.c common.h
	$(CC) $(CFLAGS) -o logger logger.c $(LDLIBS)

standalone: standalone.c
	$(CC) $(CFLAGS) -o standalone standalone.c $(LDLIBS)

clean:
	rm -f ui core logger standalone simulator.log

run: all
	./run.sh
