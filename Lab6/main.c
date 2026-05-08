#include <stdio.h>
#include "human.h"

int main()
{
    struct Human arr1[4];
    struct Human arr2[4];
    
    printf("Enter data for 4 people:\n\n");
    for (int i = 0; i < 4; i++)
    {
        printf("Person %d:\n", i + 1);
        printf("  Name: ");
        scanf("%s", arr1[i].name);
        printf("  Surname: ");
        scanf("%s", arr1[i].surname);
        printf("  Birth year: ");
        scanf("%d", &arr1[i].birthYear);
        printf("\n");
    }
    
    // Копирование
    for (int i = 0; i < 4; i++)
        arr2[i] = arr1[i];
    
    // Сортировка
    sortByBirthYear(arr2, 4);
    
    // Вывод
    printHumans(arr1, 4, "ORIGINAL ARRAY");
    printHumans(arr2, 4, "SORTED BY BIRTH YEAR");
    
    return 0;
}