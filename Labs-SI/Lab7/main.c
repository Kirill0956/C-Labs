/*
 * Лабораторная работа 7. Операции сдвига и поразрядные логические
 * операции. Сжатие записи ДНК.
 *
 * Входной файл - последовательность азотистых оснований A, G, C, T, N
 * по 70 символов в строке (последняя строка может быть короче).
 * Имя файла передаётся через argv[1], поэтому можно сжимать любую из
 * 23 хромосом. Строки-заголовки (например, ">..." в формате FASTA или
 * "Chromosome 01") пропускаются, строчные буквы приводятся к прописным.
 *
 * 1) Основное задание - 3-битное кодирование:
 *        N = 000, A = 001, C = 010, G = 100, T = 111.
 *    В одно число long long (64 бита) помещается 21 основание (63 бита).
 *    Результат: файл <имя>.dna3.
 *
 * 2) Дополнительно - 2-битное кодирование (A=00, C=01, G=10, T=11,
 *    32 основания в одном long long). Символ N не кодируется в потоке,
 *    а хранится отдельно списком участков (позиция, длина) - в геноме
 *    N идут длинными сплошными участками, поэтому список короткий.
 *    Результат: файл <имя>.dna2.
 *
 * Программа выводит количество каждого основания в десятичном виде.
 * Режим -d распаковывает файл .dna3 или .dna2 обратно в текст
 * (по 70 символов в строке) - для проверки правильности сжатия.
 *
 * Сборка:  gcc -Wall -Wextra -std=c99 -O2 main.c -o dna
 * Запуск:  ./dna hs_alt_HuRef_chr13.fa
 *          ./dna -d hs_alt_HuRef_chr13.fa.dna3 restored3.txt
 *          ./dna -d hs_alt_HuRef_chr13.fa.dna2 restored2.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_LEN   70
#define PER_LL3    21              /* оснований в long long при 3 битах */
#define PER_LL2    32              /* оснований в long long при 2 битах */
#define MAGIC3     0x33414E44LL    /* "DNA3" */
#define MAGIC2     0x32414E44LL    /* "DNA2" */

typedef unsigned long long ULL;

/* ---------------- коды оснований ---------------- */

/* 3-битный код символа, -1 - не основание */
int code3(int c)
{
    switch (c) {
    case 'N': return 0;            /* 000 */
    case 'A': return 1;            /* 001 */
    case 'C': return 2;            /* 010 */
    case 'G': return 4;            /* 100 */
    case 'T': return 7;            /* 111 */
    }
    return -1;
}

/* Символ по 3-битному коду */
char decode3(int code)
{
    static const char t[8] = {'N', 'A', 'C', '?', 'G', '?', '?', 'T'};
    return t[code & 07];
}

/* 2-битный код (N сюда не попадает) */
int code2(int c)
{
    switch (c) {
    case 'A': return 0;            /* 00 */
    case 'C': return 1;            /* 01 */
    case 'G': return 2;            /* 10 */
    case 'T': return 3;            /* 11 */
    }
    return -1;
}

char decode2(int code)
{
    static const char t[4] = {'A', 'C', 'G', 'T'};
    return t[code & 03];
}

/* ---------------- упаковщики (буфер в один long long) ---------------- */

typedef struct {
    FILE *f;
    ULL word;           /* собираемое число */
    int  n;             /* сколько оснований уже в word */
    int  per;           /* оснований на одно число */
    int  bits;          /* бит на основание */
    long long words;    /* записано чисел */
} PACKER;

void packer_init(PACKER *p, FILE *f, int bits, int per)
{
    p->f = f;
    p->word = 0;
    p->n = 0;
    p->bits = bits;
    p->per = per;
    p->words = 0;
}

void packer_put(PACKER *p, int code)
{
    /* сдвигаем накопленное влево и дописываем код в младшие биты */
    p->word = (p->word << p->bits) | (ULL)code;
    if (++p->n == p->per) {
        fwrite(&p->word, sizeof(ULL), 1, p->f);
        p->words++;
        p->word = 0;
        p->n = 0;
    }
}

void packer_flush(PACKER *p)
{
    if (p->n > 0) {
        /* неполное число: прижимаем коды к старшим разрядам */
        p->word <<= (ULL)(p->bits * (p->per - p->n));
        fwrite(&p->word, sizeof(ULL), 1, p->f);
        p->words++;
        p->word = 0;
        p->n = 0;
    }
}

/* ---------------- участки N для 2-битной схемы ---------------- */

typedef struct {
    long long pos, len;
} RUN;

RUN *runs = NULL;
long long nruns = 0, runs_cap = 0;

void add_n(long long pos)
{
    if (nruns > 0 && runs[nruns - 1].pos + runs[nruns - 1].len == pos) {
        runs[nruns - 1].len++;           /* продолжение участка */
        return;
    }
    if (nruns == runs_cap) {
        runs_cap = runs_cap ? runs_cap * 2 : 64;
        runs = realloc(runs, sizeof(RUN) * (size_t)runs_cap);
        if (runs == NULL) {
            fprintf(stderr, "Не хватает памяти\n");
            exit(1);
        }
    }
    runs[nruns].pos = pos;
    runs[nruns].len = 1;
    nruns++;
}

/* Строка - последовательность ДНК, а не заголовок? */
int is_seq_line(const char *s)
{
    if (*s == '>' || *s == ';')
        return 0;
    int any = 0;
    for (; *s && *s != '\n' && *s != '\r'; s++) {
        if (code3(toupper((unsigned char)*s)) < 0)
            return 0;
        any = 1;
    }
    return any;
}

/* ---------------- сжатие ---------------- */

int compress(const char *fname)
{
    FILE *in = fopen(fname, "r");
    if (in == NULL) {
        perror(fname);
        return 1;
    }

    char name3[1024], name2[1024], tmp2[1024];
    snprintf(name3, sizeof(name3), "%s.dna3", fname);
    snprintf(name2, sizeof(name2), "%s.dna2", fname);
    snprintf(tmp2, sizeof(tmp2), "%s.dna2.tmp", fname);

    FILE *out3 = fopen(name3, "wb");
    FILE *data2 = fopen(tmp2, "wb+");      /* поток 2-битных данных */
    if (out3 == NULL || data2 == NULL) {
        perror("fopen");
        return 1;
    }

    /* заголовок .dna3: сигнатура и число оснований (заполним в конце) */
    long long magic = MAGIC3, total = 0;
    fwrite(&magic, sizeof(magic), 1, out3);
    fwrite(&total, sizeof(total), 1, out3);

    PACKER p3, p2;
    packer_init(&p3, out3, 3, PER_LL3);
    packer_init(&p2, data2, 2, PER_LL2);

    long long cnt[256] = {0};
    long long headers = 0;
    char line[4096];

    while (fgets(line, sizeof(line), in) != NULL) {
        if (!is_seq_line(line)) {
            if (line[0] != '\n' && line[0] != '\r')
                headers++;
            continue;
        }
        /* символ перевода строки (012) не упаковывается */
        for (char *s = line; *s && *s != '\n' && *s != '\r'; s++) {
            int c = toupper((unsigned char)*s);
            cnt[c]++;
            packer_put(&p3, code3(c));
            if (c == 'N')
                add_n(total);
            else
                packer_put(&p2, code2(c));
            total++;
        }
    }
    fclose(in);
    packer_flush(&p3);
    packer_flush(&p2);

    fseek(out3, (long)sizeof(long long), SEEK_SET);
    fwrite(&total, sizeof(total), 1, out3);
    long long size3 = 2 * (long long)sizeof(long long) + p3.words * 8;
    fclose(out3);

    /* .dna2: сигнатура, всего оснований, число участков N, участки,
       число слов данных, сами данные */
    FILE *out2 = fopen(name2, "wb");
    if (out2 == NULL) {
        perror(name2);
        return 1;
    }
    magic = MAGIC2;
    fwrite(&magic, sizeof(magic), 1, out2);
    fwrite(&total, sizeof(total), 1, out2);
    fwrite(&nruns, sizeof(nruns), 1, out2);
    fwrite(runs, sizeof(RUN), (size_t)nruns, out2);
    fwrite(&p2.words, sizeof(p2.words), 1, out2);
    rewind(data2);
    ULL w;
    while (fread(&w, sizeof(w), 1, data2) == 1)
        fwrite(&w, sizeof(w), 1, out2);
    fclose(data2);
    remove(tmp2);
    long long size2 = 4 * (long long)sizeof(long long) +
                      nruns * (long long)sizeof(RUN) + p2.words * 8;
    fclose(out2);

    long long text_size = total + (total + LINE_LEN - 1) / LINE_LEN;

    printf("Файл: %s (строк-заголовков пропущено: %lld)\n", fname, headers);
    printf("Количество оснований:\n");
    printf("  A (аденин):        %lld\n", cnt['A']);
    printf("  G (гуанин):        %lld\n", cnt['G']);
    printf("  C (цитозин):       %lld\n", cnt['C']);
    printf("  T (тимин):         %lld\n", cnt['T']);
    printf("  N (неопределено):  %lld\n", cnt['N']);
    printf("  всего:             %lld\n", total);
    printf("Исходный текст ~%lld байт\n", text_size);
    printf("3 бита: %s, %lld байт (%.1f%%)\n", name3, size3,
           text_size ? 100.0 * size3 / text_size : 0.0);
    printf("2 бита: %s, %lld байт (%.1f%%), участков N: %lld\n", name2, size2,
           text_size ? 100.0 * size2 / text_size : 0.0, nruns);
    free(runs);
    return 0;
}

/* ---------------- распаковка ---------------- */

void put_base(FILE *out, char c, long long *col)
{
    fputc(c, out);
    if (++*col == LINE_LEN) {
        fputc('\n', out);
        *col = 0;
    }
}

int decompress(const char *fname, const char *outname)
{
    FILE *in = fopen(fname, "rb");
    if (in == NULL) {
        perror(fname);
        return 1;
    }
    FILE *out = fopen(outname, "w");
    if (out == NULL) {
        perror(outname);
        return 1;
    }

    long long magic, total, col = 0;
    if (fread(&magic, sizeof(magic), 1, in) != 1 ||
        fread(&total, sizeof(total), 1, in) != 1) {
        fprintf(stderr, "Неверный формат файла\n");
        return 1;
    }

    ULL w;
    long long done = 0;

    if (magic == MAGIC3) {
        while (done < total && fread(&w, sizeof(w), 1, in) == 1) {
            /* коды лежат начиная со старших бит: 21 тройка = 63 бита */
            for (int k = PER_LL3 - 1; k >= 0 && done < total; k--, done++)
                put_base(out, decode3((int)((w >> (3 * k)) & 07)), &col);
        }
    } else if (magic == MAGIC2) {
        long long nr, nwords;
        if (fread(&nr, sizeof(nr), 1, in) != 1) {
            fprintf(stderr, "Неверный формат файла\n");
            return 1;
        }
        RUN *r = malloc(sizeof(RUN) * (size_t)(nr ? nr : 1));
        if (r == NULL || fread(r, sizeof(RUN), (size_t)nr, in) != (size_t)nr ||
            fread(&nwords, sizeof(nwords), 1, in) != 1) {
            fprintf(stderr, "Неверный формат файла\n");
            return 1;
        }
        long long ri = 0;
        int k = -1;
        while (done < total) {
            if (ri < nr && done == r[ri].pos) {      /* участок из N */
                for (long long j = 0; j < r[ri].len; j++, done++)
                    put_base(out, 'N', &col);
                ri++;
                continue;
            }
            if (k < 0) {
                if (fread(&w, sizeof(w), 1, in) != 1)
                    break;
                k = PER_LL2 - 1;
            }
            put_base(out, decode2((int)((w >> (2 * k)) & 03)), &col);
            k--;
            done++;
        }
        free(r);
    } else {
        fprintf(stderr, "%s: это не файл .dna3/.dna2\n", fname);
        return 1;
    }
    if (col != 0)
        fputc('\n', out);
    fclose(in);
    fclose(out);
    printf("Распаковано оснований: %lld -> %s\n", done, outname);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc >= 4 && strcmp(argv[1], "-d") == 0)
        return decompress(argv[2], argv[3]);
    if (argc >= 2)
        return compress(argv[1]);

    printf("Использование:\n");
    printf("  %s файл_ДНК                - сжать (создаются .dna3 и .dna2)\n",
           argv[0]);
    printf("  %s -d файл.dna3 вывод.txt  - распаковать\n", argv[0]);
    return 1;
}
