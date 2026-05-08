#include <stdio.h>
#include <string.h>

#define MAX_LENGTH 81

int main()
{
    char str[MAX_LENGTH];
    
    printf("Enter a string (max 80 characters): ");
    fgets(str, MAX_LENGTH, stdin);
    
    // Удаляем символ новой строки, если он есть
    str[strcspn(str, "\n")] = '\0';
    
    // Заменяем 'a' и 'b' на заглавные
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == 'a')
            str[i] = 'A';
        else if (str[i] == 'b')
            str[i] = 'B';
    }
    
    printf("Result: %s\n", str);
    
    return 0;
}