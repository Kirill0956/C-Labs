/*
 * Библиотека для работы с матрицами (libmat).
 * Основа - библиотека из каталога задания (ko shu pui, patrick),
 * переписанная в стиле ANSI C, с добавленными функциями для
 * многомерных преобразований (mat_fill_R/T/S), определителем
 * и обратной матрицей.
 *
 * Матрица хранится как массив указателей на строки, перед которым
 * в памяти лежит заголовок MATHEAD с размерами. Поэтому к элементам
 * можно обращаться как A[i][j], а размеры узнавать через MatRow/MatCol.
 */
#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>

typedef struct {
    int row;
    int col;
} MATHEAD;

typedef struct {
    MATHEAD head;
    double *matrix;
} MATBODY;

typedef double **MATRIX;

#define Mathead(a)  ((MATHEAD *)((MATHEAD *)(a) - 1))
#define MatRow(a)   (Mathead(a)->row)
#define MatCol(a)   (Mathead(a)->col)

/* коды ошибок */
#define MAT_MALLOC      1
#define MAT_FNOTOPEN    2
#define MAT_FNOTGETMAT  3
#define MAT_SIZE        4
#define MAT_SINGULAR    5

/* типы матриц для mat_creat / mat_fill */
#define UNDEFINED   -1
#define ZERO_MATRIX  0
#define UNIT_MATRIX  1

/* создание, заполнение, удаление (matcreat.c) */
MATRIX mat_error   (int err);
MATRIX _mat_creat  (int row, int col);
MATRIX mat_creat   (int row, int col, int type);
MATRIX mat_fill    (MATRIX A, int type);
int    mat_free    (MATRIX A);
MATRIX mat_copy    (MATRIX A);
MATRIX mat_colcopy1(MATRIX A, MATRIX B, int cola, int colb);
int    fgetmat     (MATRIX A, FILE *fp);

/* матрицы преобразований 3D в однородных координатах 4x4 (matfill.c),
   вектор-строка умножается на матрицу справа: v' = v * M */
MATRIX mat_fill_x  (MATRIX A, double angle);    /* поворот вокруг X */
MATRIX mat_fill_y  (MATRIX A, double angle);    /* поворот вокруг Y */
MATRIX mat_fill_z  (MATRIX A, double angle);    /* поворот вокруг Z */
MATRIX mat_fill_s  (MATRIX A, double s);        /* масштаб */
MATRIX mat_fill_t  (MATRIX A, double dx, double dy, double dz); /* перенос */
MATRIX mat_fill_p  (MATRIX A, double vx, double vy, double vz); /* перспектива */

/* то же для пространства любой размерности N (матрица (N+1)x(N+1)) */
MATRIX mat_fill_R  (MATRIX A, int c1, int c2, double angle); /* поворот в плоскости c1-c2 */
MATRIX mat_fill_T  (MATRIX A, double *dx);      /* перенос на вектор dx[N] */
MATRIX mat_fill_S  (MATRIX A, double s);        /* масштаб */

/* вывод (matdump.c) */
MATRIX mat_dump    (MATRIX A);
MATRIX mat_dumpf   (MATRIX A, const char *fmt);
MATRIX mat_fdump   (MATRIX A, FILE *fp);
MATRIX mat_fdumpf  (MATRIX A, const char *fmt, FILE *fp);

/* арифметика: результат - новая матрица (не забыть mat_free) */
MATRIX mat_add     (MATRIX A, MATRIX B);        /* matadd.c */
MATRIX mat_sub     (MATRIX A, MATRIX B);        /* matsub.c */
MATRIX mat_mul     (MATRIX A, MATRIX B);        /* matmul.c */
MATRIX mat_tran    (MATRIX A);                  /* mattran.c */
double mat_det     (MATRIX A);                  /* matdet.c */
MATRIX mat_inv     (MATRIX A);                  /* matinv.c */

#endif
