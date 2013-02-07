INSTALL = /usr/bin/install
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

all: pseudo pseudo.1

pseudo: pseudo.c pseudo.h
	$(CC) -o pseudo -framework Security pseudo.c

pseudo.1: pseudo.1.ronn
	ronn --roff pseudo.1.ronn

install: pseudo pseudo.1
	$(INSTALL) pseudo $(BINDIR)/pseudo
	$(INSTALL) pseudo.1 $(MANDIR)/man1/pseudo.1

clean:
	rm -f pseudo
