#include <stdlib.h>
#include <string.h>
#include "align.h"

/* Вывести n слов, выровняв строку по правому краю ширины width */
static void flush_words(FILE *out, char **w, int *len, int n, int width)
{
    int total = 0;
    for (int i = 0; i < n; i++)
        total += len[i] + (i > 0 ? 1 : 0);
    for (int i = total; i < width; i++)
        fputc(' ', out);
    for (int i = 0; i < n; i++) {
        if (i > 0)
            fputc(' ', out);
        fwrite(w[i], 1, (size_t)len[i], out);
    }
    fputc('\n', out);
}

void align_line(FILE *out, const char *line, int width, WORDSTAT *st)
{
    char *words[MAXLINE / 2];
    int lens[MAXLINE / 2];
    int n = 0, cur = 0;           /* слов в текущей строке и её длина */
    const char *p = line;

    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;
        if (*p == '\0')
            break;
        const char *start = p;
        while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
            p++;
        int len = (int)(p - start);   /* количество символов в слове */

        st->words++;
        st->chars += len;
        if (len > st->longest)
            st->longest = len;

        /* слово не помещается в текущую строку - выводим её */
        if (n > 0 && cur + 1 + len > width) {
            flush_words(out, words, lens, n, width);
            n = cur = 0;
        }
        words[n] = (char *)start;
        lens[n] = len;
        cur += (n > 0 ? 1 : 0) + len;
        n++;
    }
    if (n > 0)
        flush_words(out, words, lens, n, width);
    else
        fputc('\n', out);
}

int parse_width(int argc, char *argv[], int *width)
{
    *width = DEFAULT_WIDTH;
    if (argc > 2 && strcmp(argv[1], "-w") == 0) {
        *width = atoi(argv[2]);
        if (*width < 10)
            *width = DEFAULT_WIDTH;
        return 3;
    }
    return 1;
}
