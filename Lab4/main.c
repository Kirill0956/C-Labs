#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

int main()
{
    int n;
    char op;
    
    // Ввод размера
    printf("Enter size of matrices (n): ");
    scanf("%d", &n);
    
    // Выделение памяти для матриц A и B
    double** A = (double**)malloc(n * sizeof(double*));
    double** B = (double**)malloc(n * sizeof(double*));
    
    for (int i = 0; i < n; i++)
    {
        A[i] = (double*)malloc(n * sizeof(double));
        B[i] = (double*)malloc(n * sizeof(double));
    }
    
    // Ввод матрицы A
    printf("\nEnter matrix A:\n");
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("A[%d][%d] = ", i, j);
            scanf("%lf", &A[i][j]);
        }
    }
    
    // Ввод матрицы B
    printf("\nEnter matrix B:\n");
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("B[%d][%d] = ", i, j);
            scanf("%lf", &B[i][j]);
        }
    }
    
    // Ввод операции
    printf("\nEnter operation (+, -, *): ");
    scanf(" %c", &op);
    
    // Вычисление результата
    double** result;
    
    if (op == '+')
    {
        result = addMatrices(A, B, n);
    }
    else if (op == '-')
    {
        result = subMatrices(A, B, n);
    }
    else if (op == '*')
    {
        result = mulMatrices(A, B, n);
    }
    else
    {
        printf("Error: Unknown operation!\n");
        return 1;
    }
    
    // Вывод результата
    printf("\nResult:\n");
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%8.2f ", result[i][j]);
        }
        printf("\n");
    }
    
    // Освобождение памяти
    for (int i = 0; i < n; i++)
    {
        free(A[i]);
        free(B[i]);
        free(result[i]);
    }
    free(A);
    free(B);
    free(result);
    
    return 0;
}