#!/bin/bash
# Сборка библиотеки для работы с матрицами:
#   libmat.so - динамическая, libmat.a - статическая
rm -f *.o libmat.so libmat.a
for i in matcreat.c matfill.c matdump.c matarith.c
do
    gcc -Wall -Wextra -std=c99 -O2 -c -fPIC "$i" || exit 1
done
gcc *.o -shared -lm -o libmat.so
ar rc libmat.a *.o
ranlib libmat.a
echo "Готово: libmat.so и libmat.a"
# При запуске программ с динамической библиотекой указать путь:
#   export LD_LIBRARY_PATH=.
