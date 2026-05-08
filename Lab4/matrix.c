#include "matrix.h"
#include <stdlib.h>

// Сложение матриц
double** addMatrices(double** A, double** B, int n)
{
    double** C = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++)
    {
        C[i] = (double*)malloc(n * sizeof(double));
        for (int j = 0; j < n; j++)
        {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
    return C;
}

// Вычитание матриц
double** subMatrices(double** A, double** B, int n)
{
    double** C = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++)
    {
        C[i] = (double*)malloc(n * sizeof(double));
        for (int j = 0; j < n; j++)
        {
            C[i][j] = A[i][j] - B[i][j];
        }
    }
    return C;
}

// Умножение матриц
double** mulMatrices(double** A, double** B, int n)
{
    double** C = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++)
    {
        C[i] = (double*)malloc(n * sizeof(double));
        for (int j = 0; j < n; j++)
        {
            C[i][j] = 0;
            for (int k = 0; k < n; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}