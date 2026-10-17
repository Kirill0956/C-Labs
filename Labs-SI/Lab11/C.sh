#!/bin/bash
# Сборка библиотеки матриц и программы просмотра молекул.
# Нужны пакеты: libgtk-3-dev (Ubuntu: sudo apt install libgtk-3-dev)
./makelib.sh || exit 1
gcc -Wall -std=c99 C.c molecule.c $(pkg-config --cflags --libs gtk+-3.0) \
    -L. -lmat -lm -o C || exit 1
echo "Запуск: LD_LIBRARY_PATH=. ./C molecules/benzimid"
