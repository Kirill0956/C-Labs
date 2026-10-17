/*
 * matfill.c - матрицы геометрических преобразований.
 *
 * Используются однородные координаты: точка N-мерного пространства -
 * вектор-строка (x1, ..., xN, 1), преобразование - умножение на
 * матрицу (N+1)x(N+1) справа. Несколько преобразований объединяются
 * перемножением матриц.
 */
#include <math.h>
#include "matrix.h"

/* Поворот в плоскости координат c1-c2 на угол angle.
   Для 3D: плоскость 1-2 - поворот вокруг X, 0-2 - вокруг Y, 0-1 - вокруг Z.
   Для 4D (тессеракт) добавляются плоскости с осью W: 0-3, 1-3, 2-3. */
MATRIX mat_fill_R(MATRIX A, int c1, int c2, double angle)
{
    mat_fill(A, UNIT_MATRIX);
    if (c1 == c2)
        return A;
    A[c1][c1] = cos(angle);
    A[c2][c2] = cos(angle);
    A[c1][c2] = sin(angle);
    A[c2][c1] = -sin(angle);
    return A;
}

/* Перенос на вектор dx (длина - размерность пространства).
   Вектор переноса стоит в последней строке матрицы. */
MATRIX mat_fill_T(MATRIX A, double *dx)
{
    mat_fill(A, UNIT_MATRIX);
    int n = MatRow(A) - 1;
    for (int i = 0; i < n; i++)
        A[n][i] = dx[i];
    return A;
}

/* Равномерное масштабирование */
MATRIX mat_fill_S(MATRIX A, double s)
{
    mat_fill(A, UNIT_MATRIX);
    for (int i = 0; i < MatRow(A) - 1; i++)
        A[i][i] = s;
    return A;
}

/* ---- частные случаи для 3D (матрицы 4x4) ---- */

MATRIX mat_fill_x(MATRIX A, double angle) { return mat_fill_R(A, 1, 2, angle); }
MATRIX mat_fill_y(MATRIX A, double angle) { return mat_fill_R(A, 0, 2, angle); }
MATRIX mat_fill_z(MATRIX A, double angle) { return mat_fill_R(A, 0, 1, angle); }
MATRIX mat_fill_s(MATRIX A, double s)     { return mat_fill_S(A, s); }

MATRIX mat_fill_t(MATRIX A, double dx, double dy, double dz)
{
    double d[3] = {dx, dy, dz};
    return mat_fill_T(A, d);
}

/* Центральная проекция с центром на оси Z на расстоянии vz:
   после умножения w' = 1 - z/vz, координаты делятся на w'. */
MATRIX mat_fill_p(MATRIX A, double vx, double vy, double vz)
{
    (void)vx;
    (void)vy;
    mat_fill(A, UNIT_MATRIX);
    if (vz != 0)
        A[2][3] = -1.0 / vz;
    return A;
}
