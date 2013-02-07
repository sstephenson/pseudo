all: pseudo pseudo.1

pseudo: pseudo.c pseudo.h
	$(CC) -o pseudo -framework Security pseudo.c

pseudo.1: pseudo.1.ronn
	ronn --roff pseudo.1.ronn

clean:
	rm -f pseudo pseudo.1
