#include "human.h"
#include <stdio.h>

void sortByBirthYear(struct Human arr[], int size)
{
    struct Human temp;
    for (int i = 0; i < size - 1; i++)
    {
        for (int j = i + 1; j < size; j++)
        {
            if (arr[i].birthYear > arr[j].birthYear)
            {
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
}

void printHumans(struct Human arr[], int size, const char *title)
{
    printf("\n%s:\n", title);
    for (int i = 0; i < size; i++)
    {
        printf("%d. %s %s, born in %d\n", 
               i + 1, 
               arr[i].name, 
               arr[i].surname, 
               arr[i].birthYear);
    }
}