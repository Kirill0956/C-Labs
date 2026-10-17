CFLAGS = -Wall -Wextra -std=c99 -O2
GTK    = $(shell pkg-config --cflags --libs gtk+-3.0)

all: C

libmat.so: matcreat.c matfill.c matdump.c matarith.c matrix.h
	./makelib.sh

C: C.c molecule.c molecule.h libmat.so
	gcc $(CFLAGS) C.c molecule.c $(GTK) -L. -lmat -lm -o C

run: C
	LD_LIBRARY_PATH=. ./C molecules/benzimid

clean:
	rm -f *.o libmat.so libmat.a C
