/*
 * Лабораторная работа 3. Двоичное сортированное дерево
 * библиографических источников.
 *
 * Программа читает набор файлов в формате BibTeX (имена передаются
 * через параметры командной строки), выделяет записи о книгах и сразу
 * вставляет их в двоичное дерево поиска. Ключ дерева: автор, для одного
 * автора - название книги. Обход дерева слева направо даёт записи
 * в алфавитном порядке, они записываются в файл sorted.bib.
 *
 * Файлы bib записаны в UTF-8. Для сравнения строк поля автора и названия
 * переводятся в кодировку cp1251 функциями библиотеки iconv, а сравнение
 * выполняется собственной функцией cp1251_strncmp (аналог strncmp без
 * учёта регистра, правильно упорядочивает русские буквы, включая Ё).
 *
 * После построения дерева программа ищет книги по первым 5 символам
 * фамилии/имени автора (вводится с клавиатуры).
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 main.c -o bibtree
 *          (на macOS/FreeBSD добавить -liconv)
 * Запуск:  ./bibtree 0000_0000.bib/file1.bib 0000_0000.bib/file2.bib ...
 *          ./bibtree -o out.bib файлы...    (другое имя выходного файла)
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iconv.h>

#define SEARCH_LEN 5            /* поиск по первым 5 символам */

/* Структура, построенная по полям записи bib */
typedef struct book {
    char *key;                  /* метка записи: book:602 */
    char *title;
    char *author;
    char *publisher;
    char *isbn;
    char *year;
    char *series;
    char *edition;
    char *volume;
    char *url;
    char *author_cp;            /* автор в cp1251 - для сравнения */
    char *title_cp;             /* название в cp1251 - для сравнения */
} BOOK;

typedef struct tnode {
    BOOK *book;
    int count;                  /* сколько одинаковых записей */
    struct tnode *left, *right;
} TNODE;

static iconv_t cd;              /* преобразователь UTF-8 -> CP1251 */

/* ------------------------------------------------------------------ */
/*                      Работа с кодировками                           */
/* ------------------------------------------------------------------ */

static char *xstrdup(const char *s)
{
    char *p = malloc(strlen(s) + 1);
    if (p == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(p, s);
    return p;
}

/* Перевод строки из UTF-8 в cp1251 (новая строка в динамической памяти) */
char *utf8_to_cp1251(const char *src)
{
    size_t inleft = strlen(src);
    size_t outsize = inleft + 1, outleft = inleft;
    char *out = malloc(outsize);
    if (out == NULL) {
        perror("malloc");
        exit(1);
    }
    char *in = (char *)src, *op = out;

    iconv(cd, NULL, NULL, NULL, NULL);          /* сброс состояния */
    while (inleft > 0) {
        if (iconv(cd, &in, &inleft, &op, &outleft) == (size_t)-1) {
            /* символ, которого нет в cp1251: заменяем на '?' */
            if (outleft == 0)
                break;
            *op++ = '?';
            outleft--;
            /* пропускаем один символ UTF-8 */
            in++;
            inleft--;
            while (inleft > 0 && ((unsigned char)*in & 0xC0) == 0x80) {
                in++;
                inleft--;
            }
        }
    }
    *op = '\0';
    return out;
}

/* Вес символа cp1251 для сравнения без учёта регистра.
   Латиница идёт раньше кириллицы, Ё стоит между Е и Ж. */
static int cp1251_weight(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    if (c >= 0xC0 && c <= 0xDF)             /* А..Я -> а..я */
        c = (unsigned char)(c + 0x20);
    if (c == 0xA8)                          /* Ё -> ё */
        c = 0xB8;
    if (c >= 0xE0)                          /* а..я */
        return 0x200 + (c - 0xE0) * 2;
    if (c == 0xB8)                          /* ё - сразу после е */
        return 0x200 + (0xE5 - 0xE0) * 2 + 1;
    return c;
}

/* Аналог strncmp для кодировки cp1251 (без учёта регистра).
   n < 0 - сравнивать строки целиком. */
int cp1251_strncmp(const char *s1, const char *s2, int n)
{
    for (int i = 0; n < 0 || i < n; i++) {
        int w1 = cp1251_weight((unsigned char)s1[i]);
        int w2 = cp1251_weight((unsigned char)s2[i]);
        if (w1 != w2)
            return w1 < w2 ? -1 : 1;
        if (s1[i] == '\0')
            return 0;
    }
    return 0;
}

/* Сравнение книг: по автору, при одинаковом авторе - по названию */
int book_cmp(const BOOK *a, const BOOK *b)
{
    int r = cp1251_strncmp(a->author_cp, b->author_cp, -1);
    if (r != 0)
        return r;
    return cp1251_strncmp(a->title_cp, b->title_cp, -1);
}

/* ------------------------------------------------------------------ */
/*                          Двоичное дерево                            */
/* ------------------------------------------------------------------ */

/* Вставка с одновременной сортировкой */
TNODE *tree_insert(TNODE *p, BOOK *b)
{
    if (p == NULL) {
        p = malloc(sizeof(TNODE));
        if (p == NULL) {
            perror("malloc");
            exit(1);
        }
        p->book = b;
        p->count = 1;
        p->left = p->right = NULL;
        return p;
    }
    int r = book_cmp(b, p->book);
    if (r < 0)
        p->left = tree_insert(p->left, b);
    else if (r > 0)
        p->right = tree_insert(p->right, b);
    else
        p->count++;               /* точный дубликат записи */
    return p;
}

void print_field(FILE *f, const char *name, const char *val)
{
    fprintf(f, "   %-10s = {%s},\n", name, val ? val : "");
}

void print_book(FILE *f, const BOOK *b)
{
    fprintf(f, "@book{%s,\n", b->key ? b->key : "");
    print_field(f, "author", b->author);
    print_field(f, "title", b->title);
    print_field(f, "publisher", b->publisher);
    print_field(f, "isbn", b->isbn);
    print_field(f, "year", b->year);
    print_field(f, "series", b->series);
    print_field(f, "edition", b->edition);
    print_field(f, "volume", b->volume);
    fprintf(f, "   %-10s = {%s}\n}\n\n", "url", b->url ? b->url : "");
}

/* Обход слева направо - вывод в алфавитном порядке */
int tree_print(FILE *f, const TNODE *p)
{
    if (p == NULL)
        return 0;
    int n = tree_print(f, p->left);
    print_book(f, p->book);
    n++;
    n += tree_print(f, p->right);
    return n;
}

/* Поиск всех книг, у которых первые SEARCH_LEN символов автора
   совпадают с образцом. Лишние ветви дерева не просматриваются. */
int tree_search(const TNODE *p, const char *pattern_cp)
{
    if (p == NULL)
        return 0;
    int len = (int)strlen(pattern_cp);
    if (len > SEARCH_LEN)
        len = SEARCH_LEN;
    int r = cp1251_strncmp(pattern_cp, p->book->author_cp, len);
    int found = 0;
    if (r <= 0)
        found += tree_search(p->left, pattern_cp);
    if (r == 0) {
        printf("  %s - %s (%s)\n", p->book->author, p->book->title,
               p->book->year ? p->book->year : "");
        found++;
    }
    if (r >= 0)
        found += tree_search(p->right, pattern_cp);
    return found;
}

int tree_height(const TNODE *p)
{
    if (p == NULL)
        return 0;
    int l = tree_height(p->left), r = tree_height(p->right);
    return 1 + (l > r ? l : r);
}

void tree_free(TNODE *p)
{
    if (p == NULL)
        return;
    tree_free(p->left);
    tree_free(p->right);
    BOOK *b = p->book;
    free(b->key); free(b->title); free(b->author); free(b->publisher);
    free(b->isbn); free(b->year); free(b->series); free(b->edition);
    free(b->volume); free(b->url); free(b->author_cp); free(b->title_cp);
    free(b);
    free(p);
}

/* ------------------------------------------------------------------ */
/*                        Разбор формата bib                           */
/* ------------------------------------------------------------------ */

/* Прочитать весь файл в память */
char *read_file(const char *fname)
{
    FILE *f = fopen(fname, "rb");
    if (f == NULL) {
        perror(fname);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)size + 1);
    if (buf == NULL) {
        fclose(f);
        return NULL;
    }
    size_t got = fread(buf, 1, (size_t)size, f);
    buf[got] = '\0';
    fclose(f);
    return buf;
}

/* Значение поля в фигурных скобках (учитываются вложенные скобки)
   или в кавычках. *pp указывает на открывающую скобку/кавычку. */
char *read_value(char **pp)
{
    char *p = *pp, *start, *end;
    if (*p == '{') {
        int depth = 1;
        start = ++p;
        while (*p && depth > 0) {
            if (*p == '{') depth++;
            else if (*p == '}') depth--;
            p++;
        }
        end = p - 1;
    } else if (*p == '"') {
        start = ++p;
        while (*p && *p != '"')
            p++;
        end = p;
        if (*p) p++;
    } else {                      /* значение без скобок (число) */
        start = p;
        while (*p && *p != ',' && *p != '}' && !isspace((unsigned char)*p))
            p++;
        end = p;
    }
    *pp = p;

    /* обрезаем пробелы по краям */
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)end[-1])) end--;
    size_t len = (size_t)(end - start);
    char *v = malloc(len + 1);
    if (v == NULL) {
        perror("malloc");
        exit(1);
    }
    memcpy(v, start, len);
    v[len] = '\0';
    return v;
}

/* Разбор одной записи @type{key, field = {value}, ... }.
   p указывает на '{' после типа. Возвращает указатель за записью. */
char *parse_record(char *p, BOOK *b)
{
    p++;                                          /* пропускаем '{' */
    char *k = p;
    while (*p && *p != ',' && *p != '}')
        p++;
    size_t klen = (size_t)(p - k);
    b->key = malloc(klen + 1);
    memcpy(b->key, k, klen);
    b->key[klen] = '\0';

    while (*p && *p != '}') {
        if (*p == ',' || isspace((unsigned char)*p)) {
            p++;
            continue;
        }
        /* имя поля */
        char name[32];
        int n = 0;
        while (*p && (isalnum((unsigned char)*p) || *p == '_' || *p == '-')) {
            if (n < (int)sizeof(name) - 1)
                name[n++] = (char)tolower((unsigned char)*p);
            p++;
        }
        name[n] = '\0';
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p != '=') {                          /* испорченная запись */
            while (*p && *p != ',' && *p != '}') p++;
            continue;
        }
        p++;
        while (*p && isspace((unsigned char)*p)) p++;
        char *val = read_value(&p);

        char **dst = NULL;
        if      (!strcmp(name, "title"))     dst = &b->title;
        else if (!strcmp(name, "author"))    dst = &b->author;
        else if (!strcmp(name, "publisher")) dst = &b->publisher;
        else if (!strcmp(name, "isbn"))      dst = &b->isbn;
        else if (!strcmp(name, "year"))      dst = &b->year;
        else if (!strcmp(name, "series"))    dst = &b->series;
        else if (!strcmp(name, "edition"))   dst = &b->edition;
        else if (!strcmp(name, "volume"))    dst = &b->volume;
        else if (!strcmp(name, "url"))       dst = &b->url;
        if (dst != NULL && *dst == NULL)
            *dst = val;
        else
            free(val);
    }
    if (*p == '}')
        p++;
    return p;
}

/* Разобрать один файл, все найденные книги вставить в дерево */
TNODE *load_file(TNODE *root, const char *fname, int *nbooks)
{
    char *text = read_file(fname);
    if (text == NULL)
        return root;

    char *p = text;
    while ((p = strchr(p, '@')) != NULL) {
        char type[32];
        int n = 0;
        p++;
        while (isalpha((unsigned char)*p) && n < (int)sizeof(type) - 1)
            type[n++] = (char)tolower((unsigned char)*p++);
        type[n] = '\0';
        while (isspace((unsigned char)*p)) p++;
        if (*p != '{')
            continue;
        if (strcmp(type, "comment") == 0 || strcmp(type, "preamble") == 0 ||
            strcmp(type, "string") == 0)
            continue;

        BOOK *b = calloc(1, sizeof(BOOK));
        if (b == NULL) {
            perror("calloc");
            exit(1);
        }
        p = parse_record(p, b);
        if (b->author == NULL) b->author = xstrdup("");
        if (b->title == NULL)  b->title = xstrdup("");
        b->author_cp = utf8_to_cp1251(b->author);
        b->title_cp = utf8_to_cp1251(b->title);
        root = tree_insert(root, b);
        (*nbooks)++;
    }
    free(text);
    return root;
}

int main(int argc, char *argv[])
{
    const char *outname = "sorted.bib";
    int first = 1;

    if (argc > 2 && strcmp(argv[1], "-o") == 0) {
        outname = argv[2];
        first = 3;
    }
    if (first >= argc) {
        printf("Использование: %s [-o выходной_файл] файл1.bib [файл2.bib ...]\n",
               argv[0]);
        return 1;
    }

    cd = iconv_open("CP1251", "UTF-8");
    if (cd == (iconv_t)-1) {
        perror("iconv_open");
        return 1;
    }

    TNODE *root = NULL;
    int nbooks = 0;
    for (int i = first; i < argc; i++)
        root = load_file(root, argv[i], &nbooks);

    FILE *out = fopen(outname, "w");
    if (out == NULL) {
        perror(outname);
        return 1;
    }
    int written = tree_print(out, root);
    fclose(out);

    printf("Прочитано записей: %d, уникальных: %d, высота дерева: %d\n",
           nbooks, written, tree_height(root));
    printf("Отсортированный список записан в %s\n", outname);

    /* Поиск по первым 5 символам автора */
    char query[256];
    for (;;) {
        printf("\nПоиск по автору (первые %d символов, Enter - выход): ",
               SEARCH_LEN);
        fflush(stdout);
        if (fgets(query, sizeof(query), stdin) == NULL)
            break;
        query[strcspn(query, "\r\n")] = '\0';
        if (query[0] == '\0')
            break;
        char *q = utf8_to_cp1251(query);
        int found = tree_search(root, q);
        printf("Найдено: %d\n", found);
        free(q);
    }

    tree_free(root);
    iconv_close(cd);
    return 0;
}
