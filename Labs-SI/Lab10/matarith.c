/*
 * matarith.c - сложение, вычитание, умножение, транспонирование,
 * определитель и обратная матрица.
 * Все функции возвращают НОВУЮ матрицу (её нужно освободить mat_free)
 * или NULL при ошибке.
 */
#include <math.h>
#include "matrix.h"

MATRIX mat_add(MATRIX A, MATRIX B)
{
    if (MatRow(A) != MatRow(B) || MatCol(A) != MatCol(B))
        return mat_error(MAT_SIZE);
    MATRIX C = mat_creat(MatRow(A), MatCol(A), UNDEFINED);
    if (C == NULL)
        return NULL;
    for (int i = 0; i < MatRow(A); i++)
        for (int j = 0; j < MatCol(A); j++)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

MATRIX mat_sub(MATRIX A, MATRIX B)
{
    if (MatRow(A) != MatRow(B) || MatCol(A) != MatCol(B))
        return mat_error(MAT_SIZE);
    MATRIX C = mat_creat(MatRow(A), MatCol(A), UNDEFINED);
    if (C == NULL)
        return NULL;
    for (int i = 0; i < MatRow(A); i++)
        for (int j = 0; j < MatCol(A); j++)
            C[i][j] = A[i][j] - B[i][j];
    return C;
}

MATRIX mat_mul(MATRIX A, MATRIX B)
{
    if (MatCol(A) != MatRow(B))
        return mat_error(MAT_SIZE);
    MATRIX C = mat_creat(MatRow(A), MatCol(B), UNDEFINED);
    if (C == NULL)
        return NULL;
    for (int i = 0; i < MatRow(A); i++)
        for (int j = 0; j < MatCol(B); j++) {
            double s = 0.0;
            for (int k = 0; k < MatCol(A); k++)
                s += A[i][k] * B[k][j];
            C[i][j] = s;
        }
    return C;
}

MATRIX mat_tran(MATRIX A)
{
    MATRIX At = mat_creat(MatCol(A), MatRow(A), UNDEFINED);
    if (At == NULL)
        return NULL;
    for (int i = 0; i < MatCol(A); i++)
        for (int j = 0; j < MatRow(A); j++)
            At[i][j] = A[j][i];
    return At;
}

/* Определитель методом Гаусса с выбором главного элемента */
double mat_det(MATRIX A)
{
    int n = MatRow(A);
    if (n != MatCol(A)) {
        mat_error(MAT_SIZE);
        return 0.0;
    }
    MATRIX T = mat_copy(A);
    if (T == NULL)
        return 0.0;
    double det = 1.0;
    for (int c = 0; c < n; c++) {
        int p = c;
        for (int r = c + 1; r < n; r++)
            if (fabs(T[r][c]) > fabs(T[p][c]))
                p = r;
        if (fabs(T[p][c]) < 1e-12) {
            det = 0.0;
            break;
        }
        if (p != c) {                         /* перестановка строк */
            double *tmp = T[p];
            T[p] = T[c];
            T[c] = tmp;
            det = -det;
        }
        det *= T[c][c];
        for (int r = c + 1; r < n; r++) {
            double f = T[r][c] / T[c][c];
            for (int k = c; k < n; k++)
                T[r][k] -= f * T[c][k];
        }
    }
    mat_free(T);
    return det;
}

/* Обратная матрица методом Гаусса-Жордана */
MATRIX mat_inv(MATRIX A)
{
    int n = MatRow(A);
    if (n != MatCol(A))
        return mat_error(MAT_SIZE);
    MATRIX T = mat_copy(A);
    MATRIX I = mat_creat(n, n, UNIT_MATRIX);
    if (T == NULL || I == NULL) {
        mat_free(T);
        mat_free(I);
        return NULL;
    }
    for (int c = 0; c < n; c++) {
        int p = c;
        for (int r = c + 1; r < n; r++)
            if (fabs(T[r][c]) > fabs(T[p][c]))
                p = r;
        if (fabs(T[p][c]) < 1e-12) {
            mat_free(T);
            mat_free(I);
            return mat_error(MAT_SINGULAR);
        }
        double *tmp;
        tmp = T[p]; T[p] = T[c]; T[c] = tmp;
        tmp = I[p]; I[p] = I[c]; I[c] = tmp;

        double d = T[c][c];
        for (int k = 0; k < n; k++) {
            T[c][k] /= d;
            I[c][k] /= d;
        }
        for (int r = 0; r < n; r++) {
            if (r == c)
                continue;
            double f = T[r][c];
            for (int k = 0; k < n; k++) {
                T[r][k] -= f * T[c][k];
                I[r][k] -= f * I[c][k];
            }
        }
    }
    mat_free(T);
    return I;
}
