CC = gcc
WARNS = -Wall -Wextra -pedantic -Werror
DEFS = -D_POSIX_SOURCE
CFLAGS := $(WARNS) $(DEFS) -ansi $(CFLAGS)
TARGS = sexpcode

all: $(TARGS)

sexpcode: *.c
	$(CC) -o $@ $(CFLAGS) $^

.PHONY: clean
clean:
	rm -f $(TARGS)

.PHONY: install
install: $(TARGS)
	mv -i $^ $(HOME)/bin/
