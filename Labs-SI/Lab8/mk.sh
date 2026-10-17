#!/bin/bash
# Сборка и запуск конвейера ./gen | ./filter | ./osc
# Нужна библиотека FLTK (Ubuntu: sudo apt install libfltk1.3-dev)
gcc gen.c -lm -o gen
gcc -Wall -Wextra -std=c99 filter.c -o filter
g++ -c Cartesian.cpp
g++ osc.cpp Cartesian.o -lfltk -lX11 -lm -lpthread -o osc
if [ $? -eq 0 ]
then
    # параметр filter: номер набора коэффициентов (0..8), см. README.md
    ./gen | ./filter ${1:-3} | ./osc
fi
