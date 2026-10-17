/*
 * Лабораторная работа 2. Частотный словарь.
 *
 * Программа читает до 256 текстовых файлов (имена передаются через
 * параметры командной строки), разбивает текст на слова функцией strtok
 * и подсчитывает, сколько раз встречается каждое слово.
 * Слово - последовательность букв и цифр, начинающаяся с буквы.
 * Все слова приводятся к нижнему регистру.
 * Алгоритм однопроходный: каждый файл читается ровно один раз.
 *
 * Результат - два файла:
 *   dict_alpha.txt - словарь по алфавиту;
 *   dict_freq.txt  - словарь по убыванию частоты.
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 -O2 main.c -o freq
 * Запуск:  ./freq texts/1ws4610.txt texts/2ws0110.txt ...
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXWORDS 100000
#define MAXFILES 256
#define MAXLINE  4096
#define HASHSIZE 262144          /* степень двойки, больше MAXWORDS */

typedef struct word {
    int count;
    char *w;
} WORD;

WORD *words[MAXWORDS];           /* статический массив указателей на WORD */
int totalw = 0;                  /* количество разных слов */

/* Хеш-таблица индексов в words[] - чтобы не искать слово перебором
   по всему массиву (иначе на больших текстах программа работает минутами) */
int hashtab[HASHSIZE];

unsigned hash(const char *s)
{
    unsigned h = 5381;
    while (*s)
        h = h * 33 + (unsigned char)*s++;
    return h & (HASHSIZE - 1);
}

/* Добавить слово или увеличить его счётчик */
int addword(const char *s)
{
    unsigned h = hash(s);
    while (hashtab[h] != -1) {                 /* слово уже есть? */
        if (strcmp(words[hashtab[h]]->w, s) == 0) {
            words[hashtab[h]]->count++;
            return 1;
        }
        h = (h + 1) & (HASHSIZE - 1);
    }
    if (totalw >= MAXWORDS) {
        fprintf(stderr, "Словарь переполнен (больше %d слов)\n", MAXWORDS);
        return 0;
    }
    /* новое слово: сначала создаём структуру WORD, потом строку w */
    WORD *nw = malloc(sizeof(WORD));
    if (nw == NULL) {
        perror("malloc");
        exit(1);
    }
    nw->w = malloc(strlen(s) + 1);
    if (nw->w == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(nw->w, s);
    nw->count = 1;
    words[totalw] = nw;                        /* связываем с массивом */
    hashtab[h] = totalw;
    totalw++;
    return 1;
}

/* Сравнение по алфавиту (для qsort массива указателей) */
int cmpword_alpha(const void *a, const void *b)
{
    const WORD *w1 = *(const WORD * const *)a;
    const WORD *w2 = *(const WORD * const *)b;
    return strcmp(w1->w, w2->w);
}

/* Сравнение по убыванию частоты, при равенстве - по алфавиту */
int cmpword_quant(const void *a, const void *b)
{
    const WORD *w1 = *(const WORD * const *)a;
    const WORD *w2 = *(const WORD * const *)b;
    if (w1->count != w2->count)
        return w2->count - w1->count;
    return strcmp(w1->w, w2->w);
}

/* Обработать одну лексему, полученную от strtok */
void process_token(char *tok)
{
    /* слово должно начинаться с буквы: пропускаем ведущие цифры */
    while (*tok && !isalpha((unsigned char)*tok))
        tok++;
    if (*tok == '\0')
        return;
    for (char *p = tok; *p; p++)
        *p = (char)tolower((unsigned char)*p);
    addword(tok);
}

/* Строка-разделитель для strtok: все символы, кроме латинских букв и цифр */
char delim[256];

void make_delim(void)
{
    int k = 0;
    for (int c = 1; c < 256; c++)
        if (!isalnum(c))
            delim[k++] = (char)c;
    delim[k] = '\0';
}

void print_dict(const char *fname)
{
    FILE *out = fopen(fname, "w");
    if (out == NULL) {
        perror(fname);
        exit(1);
    }
    for (int i = 0; i < totalw; i++)
        fprintf(out, "%-30s %d\n", words[i]->w, words[i]->count);
    fclose(out);
}

int main(int argc, char *argv[])
{
    char line[MAXLINE];

    if (argc < 2) {
        printf("Использование: %s файл1 [файл2 ...]\n", argv[0]);
        return 1;
    }
    if (argc - 1 > MAXFILES) {
        printf("Можно обработать не более %d файлов, будут взяты первые %d\n",
               MAXFILES, MAXFILES);
        argc = MAXFILES + 1;
    }

    memset(hashtab, -1, sizeof(hashtab));
    make_delim();

    for (int i = 1; i < argc; i++) {
        FILE *in = fopen(argv[i], "r");
        if (in == NULL) {
            perror(argv[i]);
            continue;
        }
        printf("Обрабатывается файл %s\n", argv[i]);
        while (fgets(line, sizeof(line), in) != NULL) {
            for (char *tok = strtok(line, delim); tok != NULL;
                 tok = strtok(NULL, delim))
                process_token(tok);
        }
        fclose(in);
    }

    qsort(words, totalw, sizeof(WORD *), cmpword_alpha);
    print_dict("dict_alpha.txt");

    qsort(words, totalw, sizeof(WORD *), cmpword_quant);
    print_dict("dict_freq.txt");

    printf("Разных слов: %d\n", totalw);
    printf("Результат: dict_alpha.txt (по алфавиту), dict_freq.txt (по частоте)\n");

    for (int i = 0; i < totalw; i++) {
        free(words[i]->w);
        free(words[i]);
    }
    return 0;
}
