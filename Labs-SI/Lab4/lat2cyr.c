/*
 * Лабораторная работа 4. Программа 2: транслит -> русский текст.
 *
 * Обратное преобразование по ГОСТ 7.79-2000 (система Б).
 * Вход: файлы с транслитом (например, результат работы cyr2lat),
 * до 1000 файлов через параметры командной строки.
 * Выход: для файла X.lat создаётся X.cyr (для других имён - X.cyr),
 * текст в кодировке cp1251, как в исходных файлах. Строки разбиваются
 * на слова, длина слов подсчитывается, текст выравнивается по правой
 * границе столбца. Каждый файл читается один раз.
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 lat2cyr.c align.c -o lat2cyr
 * Запуск:  ./lat2cyr History/IGOR.TXT.lat
 *          ./lat2cyr -w 60 файлы...
 * Просмотр результата в UTF-8:  iconv -f cp1251 -t utf-8 IGOR.TXT.cyr
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "align.h"

#define MAXFILES 1000

/* Сочетания латинских букв и соответствующие строчные русские буквы
   (cp1251). Более длинные сочетания проверяются раньше. */
typedef struct {
    const char *lat;
    unsigned char cyr;
} PAIR;

static const PAIR table[] = {
    {"shh", 0xF9},                                       /* щ */
    {"sh", 0xF8}, {"ch", 0xF7}, {"zh", 0xE6}, {"cz", 0xF6}, /* ш ч ж ц */
    {"yo", 0xB8}, {"yu", 0xFE}, {"ya", 0xFF},            /* ё ю я */
    {"y`", 0xFB}, {"e`", 0xFD}, {"``", 0xFA}, {"`", 0xFC}, /* ы э ъ ь */
    {"a", 0xE0}, {"b", 0xE1}, {"v", 0xE2}, {"g", 0xE3},
    {"d", 0xE4}, {"e", 0xE5}, {"z", 0xE7}, {"i", 0xE8},
    {"j", 0xE9}, {"k", 0xEA}, {"l", 0xEB}, {"m", 0xEC},
    {"n", 0xED}, {"o", 0xEE}, {"p", 0xEF}, {"r", 0xF0},
    {"s", 0xF1}, {"t", 0xF2}, {"u", 0xF3}, {"f", 0xF4},
    {"x", 0xF5}, {"c", 0xF6},                            /* х ц */
    {NULL, 0}
};

/* Прописная форма русской буквы cp1251 */
static unsigned char cyr_upper(unsigned char c)
{
    if (c == 0xB8)
        return 0xA8;                                     /* Ё */
    return (unsigned char)(c - 0x20);
}

/* Совпадает ли начало строки s с сочетанием pat (без учёта регистра) */
static int match(const char *s, const char *pat)
{
    for (int i = 0; pat[i]; i++)
        if (tolower((unsigned char)s[i]) != pat[i])
            return 0;
    return 1;
}

/* Преобразовать одну строку транслита в cp1251 */
static void convert_line(const char *in, unsigned char *out)
{
    unsigned char *o = out;
    int prev_upper = 0;          /* предыдущая буква была прописной */

    for (int i = 0; in[i]; ) {
        const PAIR *p;
        for (p = table; p->lat != NULL; p++)
            if (match(in + i, p->lat))
                break;

        if (p->lat == NULL) {                    /* не буква транслита */
            *o++ = (unsigned char)in[i];
            prev_upper = 0;
            i++;
            continue;
        }

        int len = (int)strlen(p->lat);
        int upper;
        if (in[i] == '`')     /* ъ, ь: регистр по соседним буквам */
            upper = prev_upper && isupper((unsigned char)in[i + len]);
        else
            upper = isupper((unsigned char)in[i]) != 0;

        *o++ = upper ? cyr_upper(p->cyr) : p->cyr;
        prev_upper = upper;
        i += len;
    }
    *o = '\0';
}

int main(int argc, char *argv[])
{
    int width;
    int first = parse_width(argc, argv, &width);

    if (first >= argc) {
        printf("Использование: %s [-w ширина] файл1 [файл2 ...]\n", argv[0]);
        return 1;
    }
    if (argc - first > MAXFILES) {
        printf("Будут обработаны первые %d файлов\n", MAXFILES);
        argc = first + MAXFILES;
    }

    static char line[MAXLINE];
    static unsigned char out_line[MAXLINE];

    for (int i = first; i < argc; i++) {
        FILE *in = fopen(argv[i], "r");
        if (in == NULL) {
            perror(argv[i]);
            continue;
        }
        char outname[1024];
        size_t n = strlen(argv[i]);
        if (n > 4 && strcmp(argv[i] + n - 4, ".lat") == 0)
            snprintf(outname, sizeof(outname), "%.*s.cyr", (int)(n - 4), argv[i]);
        else
            snprintf(outname, sizeof(outname), "%s.cyr", argv[i]);

        FILE *out = fopen(outname, "wb");
        if (out == NULL) {
            perror(outname);
            fclose(in);
            continue;
        }

        WORDSTAT st = {0, 0, 0};
        while (fgets(line, sizeof(line), in) != NULL) {
            convert_line(line, out_line);
            align_line(out, (const char *)out_line, width, &st);
        }
        fclose(in);
        fclose(out);

        printf("%s -> %s: слов %ld, средняя длина %.2f, самое длинное %d\n",
               argv[i], outname, st.words,
               st.words ? (double)st.chars / st.words : 0.0, st.longest);
    }
    return 0;
}
