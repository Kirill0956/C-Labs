/*
 * matdump.c - вывод матрицы на экран или в файл.
 */
#include <stdio.h>
#include "matrix.h"

MATRIX mat_fdumpf(MATRIX A, const char *fmt, FILE *fp)
{
    for (int i = 0; i < MatRow(A); i++) {
        for (int j = 0; j < MatCol(A); j++)
            fprintf(fp, fmt, A[i][j]);
        fprintf(fp, "\n");
    }
    return A;
}

MATRIX mat_dump(MATRIX A)                   { return mat_fdumpf(A, "%10.4f ", stdout); }
MATRIX mat_dumpf(MATRIX A, const char *fmt) { return mat_fdumpf(A, fmt, stdout); }
MATRIX mat_fdump(MATRIX A, FILE *fp)        { return mat_fdumpf(A, "%10.4f ", fp); }
