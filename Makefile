CC ?= cc
CFLAGS ?= -O2 -std=c99 -Wall -Wextra -pedantic

dracula: dracula.c
	$(CC) $(CFLAGS) -o $@ dracula.c

clean:
	rm -f dracula

.PHONY: clean
