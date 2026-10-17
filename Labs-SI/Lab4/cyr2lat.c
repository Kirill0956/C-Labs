/*
 * Лабораторная работа 4. Программа 1: русский текст -> транслит.
 *
 * Транслитерация по ГОСТ 7.79-2000, система Б (файл ГОСТа лежит
 * в каталоге задания). Входные файлы - текст в кодировке cp1251
 * (каталоги History и Detect), до 1000 файлов, имена передаются через
 * параметры командной строки. Каждая строка разбивается на слова,
 * для каждого слова считается количество символов, текст выравнивается
 * по правой границе столбца. Файлы читаются один раз (однопроходный
 * алгоритм).
 *
 * Выход: для каждого файла X создаётся файл X.lat.
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 cyr2lat.c align.c -o cyr2lat
 * Запуск:  ./cyr2lat History/IGOR.TXT History/LICAR.TXT
 *          ./cyr2lat -w 60 файлы...      (ширина столбца 60)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "align.h"

#define MAXFILES 1000

/* Таблица ГОСТ 7.79-2000 (система Б) для букв а..я в порядке cp1251 */
static const char *lat[32] = {
    "a", "b", "v", "g", "d", "e", "zh", "z",            /* а б в г д е ж з */
    "i", "j", "k", "l", "m", "n", "o", "p",             /* и й к л м н о п */
    "r", "s", "t", "u", "f", "x", "cz", "ch",           /* р с т у ф х ц ч */
    "sh", "shh", "``", "y`", "`", "e`", "yu", "ya"      /* ш щ ъ ы ь э ю я */
};
static const char *lat_yo = "yo";                       /* ё */

/* Русская ли это буква (cp1251) */
static int is_cyr(unsigned char c)
{
    return c >= 0xC0 || c == 0xA8 || c == 0xB8;
}

static int is_cyr_upper(unsigned char c)
{
    return (c >= 0xC0 && c <= 0xDF) || c == 0xA8;
}

/* Транслит строчной формы русской буквы */
static const char *translit(unsigned char c)
{
    if (c == 0xA8 || c == 0xB8)
        return lat_yo;
    if (c >= 0xC0 && c <= 0xDF)
        c = (unsigned char)(c + 0x20);
    return lat[c - 0xE0];
}

/* Преобразовать одну строку cp1251 в транслит */
static void convert_line(const unsigned char *in, char *out)
{
    char *o = out;
    for (int i = 0; in[i]; i++) {
        unsigned char c = in[i];

        if (c < 0x80) {                 /* ASCII - без изменений */
            *o++ = (char)c;
            continue;
        }
        if (!is_cyr(c)) {               /* прочие символы cp1251 */
            switch (c) {
            case 0xAB: case 0xBB:       /* « » */
            case 0x93: case 0x94: case 0x84:
                *o++ = '"'; break;
            case 0x91: case 0x92:       /* ‘ ’ */
                *o++ = '\''; break;
            case 0x96: case 0x97:       /* – — */
                *o++ = '-'; break;
            case 0x85:                  /* … */
                strcpy(o, "..."); o += 3; break;
            case 0xB9:                  /* № */
                strcpy(o, "No"); o += 2; break;
            case 0xA0:                  /* неразрывный пробел */
                *o++ = ' '; break;
            default:
                *o++ = '?';
            }
            continue;
        }

        const char *t = translit(c);

        /* ц: "c" перед i, e, y, j, иначе "cz" */
        if ((c == 0xF6 || c == 0xD6) && is_cyr(in[i + 1])) {
            char nx = translit(in[i + 1])[0];
            if (nx == 'i' || nx == 'e' || nx == 'y' || nx == 'j')
                t = "c";
        }

        if (is_cyr_upper(c)) {
            /* прописная: если следующая буква тоже прописная -
               всё сочетание прописными (ZH), иначе только первая (Zh) */
            int all = is_cyr_upper(in[i + 1]) ||
                      (i > 0 && is_cyr_upper(in[i - 1]) && !is_cyr(in[i + 1]));
            for (int k = 0; t[k]; k++)
                *o++ = (char)((k == 0 || all) ? toupper((unsigned char)t[k])
                                              : t[k]);
        } else {
            strcpy(o, t);
            o += strlen(t);
        }
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

    static unsigned char line[MAXLINE];
    static char out_line[MAXLINE * 4];

    for (int i = first; i < argc; i++) {
        FILE *in = fopen(argv[i], "rb");
        if (in == NULL) {
            perror(argv[i]);
            continue;
        }
        char outname[1024];
        snprintf(outname, sizeof(outname), "%s.lat", argv[i]);
        FILE *out = fopen(outname, "w");
        if (out == NULL) {
            perror(outname);
            fclose(in);
            continue;
        }

        WORDSTAT st = {0, 0, 0};
        while (fgets((char *)line, sizeof(line), in) != NULL) {
            convert_line(line, out_line);
            align_line(out, out_line, width, &st);
        }
        fclose(in);
        fclose(out);

        printf("%s -> %s: слов %ld, средняя длина %.2f, самое длинное %d\n",
               argv[i], outname, st.words,
               st.words ? (double)st.chars / st.words : 0.0, st.longest);
    }
    return 0;
}
