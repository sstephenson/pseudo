pseudo: pseudo.c pseudo.h
	$(CC) -o pseudo -framework Security pseudo.c

clean:
	rm -f pseudo
