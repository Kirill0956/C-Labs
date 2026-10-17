CFLAGS = -Wall -Wextra -std=c99 -O2

all: libmat.so test tesseract

libmat.so libmat.a: matcreat.c matfill.c matdump.c matarith.c matrix.h
	./makelib.sh

test: test.c libmat.so
	gcc $(CFLAGS) test.c -L. -lmat -lm -o test

tesseract: tesseract.c libmat.so
	gcc $(CFLAGS) tesseract.c -L. -lmat -lX11 -lm -o tesseract

run: tesseract
	LD_LIBRARY_PATH=. ./tesseract

clean:
	rm -f *.o libmat.so libmat.a test tesseract
