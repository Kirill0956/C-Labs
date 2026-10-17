/*
 * Лабораторная работа 1. Посимвольный анализ текстового файла.
 *
 * Имя файла вводится с клавиатуры, файл читается функцией fgetc.
 * Подсчитываются: все символы, строки, цифры, знаки препинания,
 * пробельные символы, буквы (строчные и прописные), гласные и согласные.
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 main.c -o count
 * Запуск:  ./count
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Является ли символ гласной английской буквой */
int isvowel(int c)
{
    switch (tolower(c)) {
    case 'a': case 'e': case 'i':
    case 'o': case 'u': case 'y':
        return 1;
    }
    return 0;
}

/* Является ли символ согласной английской буквой */
int isconsonant(int c)
{
    return isalpha(c) && !isvowel(c);
}

int main(void)
{
    char fname[256];

    printf("Введите имя файла: ");
    if (fgets(fname, sizeof(fname), stdin) == NULL) {
        fprintf(stderr, "Имя файла не введено\n");
        return 1;
    }
    fname[strcspn(fname, "\r\n")] = '\0';     /* убираем перевод строки */

    FILE *f = fopen(fname, "r");
    if (f == NULL) {                           /* проверка открытия */
        perror(fname);
        return 1;
    }

    long total = 0, lines = 0, digits = 0, punct = 0, spaces = 0;
    long letters = 0, lower = 0, upper = 0, vowels = 0, consonants = 0;
    int c, last = '\n';

    while ((c = fgetc(f)) != EOF) {
        total++;
        if (c == '\n')   lines++;
        if (isdigit(c))  digits++;
        if (ispunct(c))  punct++;
        if (isspace(c))  spaces++;
        if (isalpha(c)) {
            letters++;
            if (islower(c)) lower++;
            if (isupper(c)) upper++;
            if (isvowel(c)) vowels++;
            if (isconsonant(c)) consonants++;
        }
        last = c;
    }
    fclose(f);

    /* последняя строка без '\n' в конце тоже считается строкой */
    if (total > 0 && last != '\n')
        lines++;

    printf("Файл: %s\n", fname);
    printf("Всего символов:        %ld\n", total);
    printf("Строк:                 %ld\n", lines);
    printf("Цифр:                  %ld\n", digits);
    printf("Знаков препинания:     %ld\n", punct);
    printf("Пробельных символов:   %ld\n", spaces);
    printf("Букв:                  %ld\n", letters);
    printf("  из них строчных:     %ld\n", lower);
    printf("  из них прописных:    %ld\n", upper);
    printf("Гласных:               %ld\n", vowels);
    printf("Согласных:             %ld\n", consonants);
    return 0;
}
