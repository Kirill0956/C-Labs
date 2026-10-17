/*
 * Проверка библиотеки libmat (аналог test.c из задания).
 *
 * Сборка:  ./makelib.sh
 *          gcc -Wall -std=c99 test.c -L. -lmat -lm -o test
 *          LD_LIBRARY_PATH=. ./test
 */
#define _DEFAULT_SOURCE      /* для M_PI */
#include <stdio.h>
#include <math.h>
#include "matrix.h"

int main(void)
{
    MATRIX A = _mat_creat(4, 4), B = _mat_creat(4, 4);
    MATRIX C = _mat_creat(4, 4), D = _mat_creat(4, 4);

    mat_fill_x(A, 0.123);
    mat_fill_y(B, 0.23);
    mat_fill_z(C, 0.3);
    mat_fill_t(D, 2.0, 3.0, 4.0);

    printf("Поворот вокруг X:\n");  mat_dump(A);
    printf("\nПоворот вокруг Y:\n"); mat_dump(B);
    printf("\nПоворот вокруг Z:\n"); mat_dump(C);
    printf("\nПеренос (2,3,4):\n");  mat_dump(D);

    /* точка (1,0,0) после поворота на 90 градусов вокруг Z -> (0,1,0) */
    MATRIX v = mat_creat(1, 4, ZERO_MATRIX);
    v[0][0] = 1.0;
    v[0][3] = 1.0;
    mat_fill_z(C, M_PI / 2);
    MATRIX r = mat_mul(v, C);
    printf("\n(1,0,0) повёрнута на 90 град. вокруг Z: (%.3f, %.3f, %.3f)\n",
           r[0][0], r[0][1], r[0][2]);

    /* у матрицы поворота определитель 1, обратная = транспонированная */
    MATRIX M = mat_mul(A, B);
    MATRIX Mi = mat_inv(M);
    MATRIX Mt = mat_tran(M);
    MATRIX diff = mat_sub(Mi, Mt);
    double err = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            err += fabs(diff[i][j]);
    printf("det(Rx*Ry) = %.6f, |inv - tran| = %.2e\n", mat_det(M), err);

    /* 4D: поворот в плоскости X-W, матрица 5x5 */
    MATRIX R4 = _mat_creat(5, 5);
    mat_fill_R(R4, 0, 3, M_PI / 2);
    printf("\nПоворот в плоскости XW (4D):\n");
    mat_dump(R4);

    MATRIX objs[] = {A, B, C, D, v, r, M, Mi, Mt, diff, R4};
    for (unsigned i = 0; i < sizeof(objs) / sizeof(objs[0]); i++)
        mat_free(objs[i]);
    return 0;
}
