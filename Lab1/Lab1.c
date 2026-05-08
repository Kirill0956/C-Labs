#include <stdio.h>

#define SIZE_3 3
#define SIZE_2 2

int main()
{
    // Массив 3x3 (double)
    double matrix3[SIZE_3][SIZE_3];
    
    // Массив 2x2 (int)
    int matrix2[SIZE_2][SIZE_2];
    
    // Ввод матрицы 3x3
    printf("Enter the elements of the 3x3 matrix:\n");
    for (int i = 0; i < SIZE_3; i++)
    {
        for (int j = 0; j < SIZE_3; j++)
        {
            printf("matrix3[%d][%d] = ", i, j);
            scanf("%lf", &matrix3[i][j]);
        }
    }
    
    // Сумма главной диагонали (i == j)
    double sumMain = 0;
    for (int i = 0; i < SIZE_3; i++)
        sumMain += matrix3[i][i];
    
    // Сумма побочной диагонали (i + j == SIZE_3 - 1)
    double sumSecondary = 0;
    for (int i = 0; i < SIZE_3; i++)
        sumSecondary += matrix3[i][SIZE_3 - 1 - i];
    
    printf("Sum of main diagonal: %.2f\n", sumMain);
    printf("Sum of secondary diagonal: %.2f\n", sumSecondary);
    
    // Ввод матрицы 2x2
    printf("\nEnter the elements of the 2x2 matrix:\n");
    for (int i = 0; i < SIZE_2; i++)
    {
        for (int j = 0; j < SIZE_2; j++)
        {
            printf("matrix2[%d][%d] = ", i, j);
            scanf("%d", &matrix2[i][j]);
        }
    }
    
    // Вычисление квадрата матрицы 2x2
    int square[SIZE_2][SIZE_2];
    
    for (int i = 0; i < SIZE_2; i++)
    {
        for (int j = 0; j < SIZE_2; j++)
        {
            square[i][j] = 0;
            for (int k = 0; k < SIZE_2; k++)
                square[i][j] += matrix2[i][k] * matrix2[k][j];
        }
    }
    
    // Вывод квадрата матрицы
    printf("\nSquare of the matrix:\n");
    for (int i = 0; i < SIZE_2; i++)
    {
        for (int j = 0; j < SIZE_2; j++)
            printf("%d ", square[i][j]);
        printf("\n");
    }
    
    return 0;
}