/*
 * Лабораторная работа 0. Решето Эратосфена.
 *
 * Простые числа, не превосходящие n, вычисляются и сохраняются
 * в массиве целых чисел P[n].
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 main.c -o era
 * Запуск:  ./era          (n вводится с клавиатуры)
 *          ./era 100      (n передаётся параметром)
 */
#include <stdio.h>
#include <stdlib.h>

/* Шаг 7: удалить из массива все нулевые элементы.
   Возвращает новое количество элементов. */
int remove_zeros(int *P, int len)
{
    int k = 0;
    for (int i = 0; i < len; i++)
        if (P[i] != 0)
            P[k++] = P[i];
    return k;
}

/* Решето Эратосфена. P - массив из n элементов.
   Возвращает количество найденных простых чисел (они в начале P). */
int sieve(int *P, int n)
{
    int len = n - 1;                 /* числа 2..n */
    if (len <= 0)
        return 0;

    /* Шаг 1: выписать числа 2, 3, ..., n. Число k лежит в P[k-2]. */
    for (int i = 0; i < len; i++)
        P[i] = i + 2;

    /* Шаг 2: p = 2 - первое простое число. */
    int p = 2;

    /* Шаг 5: повторять, пока p*p <= n (улучшенный вариант). */
    while ((long long)p * p <= n) {
        /* Шаг 3: вычеркнуть p*p, p*p+p, ... (присвоить 0). */
        for (long long m = (long long)p * p; m <= n; m += p)
            P[m - 2] = 0;

        /* Шаг 4: первое невычеркнутое число, большее p. */
        int next = p + 1;
        while (next <= n && P[next - 2] == 0)
            next++;
        p = next;
    }

    /* Шаги 6-7: оставшиеся числа - простые, нули удаляем. */
    return remove_zeros(P, len);
}

int main(int argc, char *argv[])
{
    int n = 0;

    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        printf("Введите n: ");
        if (scanf("%d", &n) != 1) {
            fprintf(stderr, "Ошибка: нужно ввести целое число\n");
            return 1;
        }
    }

    if (n < 2) {
        printf("Простых чисел, не больших %d, нет\n", n);
        return 0;
    }

    int *P = malloc(sizeof(int) * (size_t)n);   /* int P[n] */
    if (P == NULL) {
        fprintf(stderr, "Ошибка: не хватает памяти\n");
        return 1;
    }

    int count = sieve(P, n);

    printf("Простые числа до %d (всего %d):\n", n, count);
    for (int i = 0; i < count; i++)
        printf("%d%c", P[i], (i + 1) % 10 == 0 || i == count - 1 ? '\n' : ' ');

    free(P);
    return 0;
}
