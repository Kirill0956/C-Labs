/*
 * matcreat.c - создание, заполнение, копирование и удаление матриц.
 */
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

MATRIX mat_error(int err)
{
    switch (err) {
    case MAT_MALLOC:     fprintf(stderr, "mat: malloc error\n"); break;
    case MAT_FNOTOPEN:   fprintf(stderr, "mat: fileopen error\n"); break;
    case MAT_FNOTGETMAT: fprintf(stderr, "fgetmat: matrix read error\n"); break;
    case MAT_SIZE:       fprintf(stderr, "mat: incompatible sizes\n"); break;
    case MAT_SINGULAR:   fprintf(stderr, "mat: singular matrix\n"); break;
    }
    return NULL;
}

/* Выделить память под матрицу row x col (элементы не заполняются).
   В памяти: [MATHEAD][double *строка0][double *строка1]... */
MATRIX _mat_creat(int row, int col)
{
    if (row <= 0 || col <= 0)
        return mat_error(MAT_SIZE);

    MATBODY *mat = malloc(sizeof(MATHEAD) + sizeof(double *) * (size_t)row);
    if (mat == NULL)
        return mat_error(MAT_MALLOC);

    MATRIX A = &mat->matrix;
    mat->head.row = row;
    mat->head.col = col;
    for (int i = 0; i < row; i++) {
        A[i] = malloc(sizeof(double) * (size_t)col);
        if (A[i] == NULL) {
            for (int k = 0; k < i; k++)
                free(A[k]);
            free(mat);
            return mat_error(MAT_MALLOC);
        }
    }
    return A;
}

MATRIX mat_creat(int row, int col, int type)
{
    MATRIX A = _mat_creat(row, col);
    if (A == NULL)
        return NULL;
    return mat_fill(A, type);
}

/* Заполнить нулевой или единичной матрицей */
MATRIX mat_fill(MATRIX A, int type)
{
    if (type != ZERO_MATRIX && type != UNIT_MATRIX)
        return A;                                   /* UNDEFINED */
    for (int i = 0; i < MatRow(A); i++)
        for (int j = 0; j < MatCol(A); j++)
            A[i][j] = (type == UNIT_MATRIX && i == j) ? 1.0 : 0.0;
    return A;
}

int mat_free(MATRIX A)
{
    if (A == NULL)
        return 0;
    for (int i = 0; i < MatRow(A); i++)
        free(A[i]);
    free(Mathead(A));
    return 1;
}

MATRIX mat_copy(MATRIX A)
{
    MATRIX C = mat_creat(MatRow(A), MatCol(A), UNDEFINED);
    if (C == NULL)
        return NULL;
    for (int i = 0; i < MatRow(A); i++)
        for (int j = 0; j < MatCol(A); j++)
            C[i][j] = A[i][j];
    return C;
}

/* Скопировать столбец colb матрицы B в столбец cola матрицы A */
MATRIX mat_colcopy1(MATRIX A, MATRIX B, int cola, int colb)
{
    for (int i = 0; i < MatRow(A); i++)
        A[i][cola] = B[i][colb];
    return A;
}

/* Прочитать элементы матрицы из файла, вернуть число прочитанных */
int fgetmat(MATRIX A, FILE *fp)
{
    int k = 0;
    for (int i = 0; i < MatRow(A); i++)
        for (int j = 0; j < MatCol(A); j++)
            if (fscanf(fp, "%lf", &A[i][j]) == 1)
                k++;
    return k;
}
