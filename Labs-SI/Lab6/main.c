/*
 * Лабораторная работа 6. Выпуклая оболочка на плоскости (QuickHull).
 *
 * Точки читаются из файла (каждая строка: номер x y) в массив структур
 * POINT. Массив сортируется по координате X библиотечной функцией qsort,
 * после чего крайняя левая и крайняя правая точки гарантированно
 * принадлежат оболочке. Далее алгоритм QuickHull рекурсивно ищет самую
 * удалённую от отрезка точку, лежащую слева от луча, и отбрасывает точки
 * внутри образованного треугольника.
 *
 * Результат - номера точек оболочки в порядке обхода по часовой стрелке
 * (начиная с самой левой точки) - записывается в файл.
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 main.c -o hull
 * Запуск:  ./hull IN.txt            (результат в OUT.txt)
 *          ./hull IN.txt res.txt    (результат в res.txt)
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct point {
    double x;
    double y;
    int num;                    /* номер точки из файла */
} POINT;

POINT *points;                  /* все точки */
int npoints;

int *hull;                      /* индексы точек оболочки */
int nhull;

/* Сравнение для qsort: по X, при равных X - по Y */
int cmp_x(const void *a, const void *b)
{
    const POINT *p = a, *q = b;
    if (p->x < q->x) return -1;
    if (p->x > q->x) return 1;
    if (p->y < q->y) return -1;
    if (p->y > q->y) return 1;
    return 0;
}

/* Псевдоскалярное (векторное) произведение (B-A) x (C-A).
   > 0 - точка C слева от луча A->B, < 0 - справа, 0 - на прямой.
   Модуль равен удвоенной площади треугольника ABC, т.е. пропорционален
   расстоянию от C до прямой AB. */
double cross(const POINT *A, const POINT *B, const POINT *C)
{
    return (B->x - A->x) * (C->y - A->y) - (B->y - A->y) * (C->x - A->x);
}

/* Находится ли точка C слева от луча A->B */
int is_left(const POINT *A, const POINT *B, const POINT *C)
{
    return cross(A, B, C) > 0;
}

/* Находится ли точка C справа от луча A->B */
int is_right(const POINT *A, const POINT *B, const POINT *C)
{
    return cross(A, B, C) < 0;
}

/* Рекурсивная часть QuickHull.
   set[0..n-1] - индексы точек, лежащих слева от луча a->b.
   Добавляет в hull вершины оболочки между a и b (не включая их). */
void quickhull(int a, int b, int *set, int n)
{
    if (n == 0)
        return;

    /* самая удалённая от прямой ab точка */
    int far = set[0];
    double best = cross(&points[a], &points[b], &points[far]);
    for (int i = 1; i < n; i++) {
        double d = cross(&points[a], &points[b], &points[set[i]]);
        if (d > best) {
            best = d;
            far = set[i];
        }
    }

    /* точки слева от a->far и слева от far->b; точки внутри
       треугольника a-far-b отбрасываются */
    int *left1 = malloc(sizeof(int) * (size_t)n);
    int *left2 = malloc(sizeof(int) * (size_t)n);
    if (left1 == NULL || left2 == NULL) {
        fprintf(stderr, "Не хватает памяти\n");
        exit(1);
    }
    int n1 = 0, n2 = 0;
    for (int i = 0; i < n; i++) {
        int k = set[i];
        if (k == far)
            continue;
        if (is_left(&points[a], &points[far], &points[k]))
            left1[n1++] = k;
        else if (is_left(&points[far], &points[b], &points[k]))
            left2[n2++] = k;
    }

    quickhull(a, far, left1, n1);
    hull[nhull++] = far;
    quickhull(far, b, left2, n2);

    free(left1);
    free(left2);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Использование: %s входной_файл [выходной_файл]\n", argv[0]);
        return 1;
    }
    const char *outname = argc > 2 ? argv[2] : "OUT.txt";

    FILE *in = fopen(argv[1], "r");
    if (in == NULL) {
        perror(argv[1]);
        return 1;
    }

    /* чтение точек в динамический массив структур POINT */
    int cap = 1024, num;
    double x, y;
    points = malloc(sizeof(POINT) * (size_t)cap);
    npoints = 0;
    while (fscanf(in, "%d%lf%lf", &num, &x, &y) == 3) {
        if (npoints == cap) {
            cap *= 2;
            points = realloc(points, sizeof(POINT) * (size_t)cap);
            if (points == NULL) {
                fprintf(stderr, "Не хватает памяти\n");
                return 1;
            }
        }
        points[npoints].x = x;
        points[npoints].y = y;
        points[npoints].num = num;
        npoints++;
    }
    fclose(in);
    printf("Прочитано точек: %d\n", npoints);

    if (npoints == 0) {
        printf("Нет точек\n");
        free(points);
        return 0;
    }

    /* (1) предварительная сортировка по X */
    qsort(points, (size_t)npoints, sizeof(POINT), cmp_x);

    hull = malloc(sizeof(int) * (size_t)npoints);
    int *upper = malloc(sizeof(int) * (size_t)npoints);
    int *lower = malloc(sizeof(int) * (size_t)npoints);
    if (hull == NULL || upper == NULL || lower == NULL) {
        fprintf(stderr, "Не хватает памяти\n");
        return 1;
    }

    int A = 0, B = npoints - 1;     /* самая левая и самая правая точки */
    nhull = 0;
    hull[nhull++] = A;

    if (B != A && (points[A].x != points[B].x || points[A].y != points[B].y)) {
        int nu = 0, nl = 0;
        for (int i = 1; i < npoints - 1; i++) {
            if (is_left(&points[A], &points[B], &points[i]))
                upper[nu++] = i;            /* над прямой AB */
            else if (is_right(&points[A], &points[B], &points[i]))
                lower[nl++] = i;            /* под прямой AB */
        }
        quickhull(A, B, upper, nu);         /* верхняя часть: A -> B */
        hull[nhull++] = B;
        quickhull(B, A, lower, nl);         /* нижняя часть: B -> A */
    }

    /* (3) вывод номеров точек оболочки в файл */
    FILE *out = fopen(outname, "w");
    if (out == NULL) {
        perror(outname);
        return 1;
    }
    for (int i = 0; i < nhull; i++)
        fprintf(out, "%d\n", points[hull[i]].num);
    fclose(out);

    printf("Точек в оболочке: %d, номера:", nhull);
    for (int i = 0; i < nhull; i++)
        printf(" %d", points[hull[i]].num);
    printf("\nРезультат записан в %s\n", outname);

    free(points);
    free(hull);
    free(upper);
    free(lower);
    return 0;
}
